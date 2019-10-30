/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Martin Schröder <m.schroeder2007@gmail.com>                 |
  +----------------------------------------------------------------------+
*/

#include "async/core.h"
#include "async/async_helper.h"

#include "zend_smart_str.h"

#if ASYNC_SOCKETS
static int (*le_socket)(void);
#endif

char *async_status_label(zend_uchar status)
{
	if (status == ASYNC_OP_RESOLVED) {
		return "RESOLVED";
	}

	if (status == ASYNC_OP_FAILED) {
		return "FAILED";
	}

	return "PENDING";
}

#if PHP_VERSION_ID < 70400
void async_prop_write_handler_readonly(zval *object, zval *member, zval *value, void **cache_slot)
{
	zend_throw_error(NULL, "Cannot write to property \"%s\" of %s", ZSTR_VAL(Z_STR_P(member)), ZSTR_VAL(Z_OBJCE_P(object)->name));
}
#else
zval *async_prop_write_handler_readonly(zval *object, zval *member, zval *value, void **cache_slot)
{
	zend_throw_error(NULL, "Cannot write to property \"%s\" of %s", ZSTR_VAL(Z_STR_P(member)), ZSTR_VAL(Z_OBJCE_P(object)->name));

	return NULL;
}
#endif

ASYNC_API void async_prepare_throwable(zval *error, zend_execute_data *exec, zend_class_entry *ce, const char *message, ...)
{
	zend_execute_data *prev;

	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
#if PHP_VERSION_ID >= 70200
	smart_str str = {0};
#else
	zend_string *str;
#endif
	va_list argv;

	zend_object *p1;
	zend_object *p2;

	zval arg;
	zval retval;
	zval tmp;

	prev = EG(current_execute_data);
	p1 = EG(exception);
	p2 = EG(prev_exception);

	EG(current_execute_data) = exec;
	EG(exception) = NULL;
	EG(prev_exception) = NULL;

	object_init_ex(error, ce);

	if (UNEXPECTED(EG(exception))) {
		zval_ptr_dtor(error);

		ZVAL_OBJ(error, EG(exception));
		EG(exception) = NULL;
	}

	va_start(argv, message);

	if (!ce->constructor) {
		va_end(argv);

		EG(current_execute_data) = prev;
		EG(exception) = p1;
		EG(prev_exception) = p2;

		return;
	}

	fci = empty_fcall_info;
	fcc = empty_fcall_info_cache;

	ZVAL_STR(&fci.function_name, ce->constructor->common.function_name);
#if PHP_VERSION_ID >= 70200
	php_printf_to_smart_str(&str, message, argv);
	ZVAL_STR(&arg, smart_str_extract(&str));
#else
	str = vstrpprintf(0, message, argv);
	ZVAL_STR(&arg, str);
#endif

	va_end(argv);

	zend_fcall_info_argp(&fci, 1, &arg);
	zval_ptr_dtor(&arg);

	fci.param_count = 1;
	fci.retval = &retval;
	fci.object = Z_OBJ_P(error);

	fci.size = sizeof(fci);

	fcc.function_handler = ce->constructor;
	fcc.called_scope = ce;
	fcc.object = Z_OBJ_P(error);

	ZVAL_UNDEF(&retval);

	zend_try {
		zend_call_function(&fci, &fcc);
	} zend_catch {
		async_task_scheduler_handle_exit(async_task_scheduler_get());
	} zend_end_try();

	zval_ptr_dtor(&retval);

	zend_fcall_info_args_clear(&fci, 1);
	zval_ptr_dtor(&fci.function_name);

	if (UNEXPECTED(EG(exception))) {
		zval_ptr_dtor(error);

		ZVAL_OBJ(error, EG(exception));
		EG(exception) = NULL;
	}

	EG(current_execute_data) = prev;
	EG(exception) = p1;
	EG(prev_exception) = p2;

	if (exec == NULL) {
		ZVAL_STRING(&tmp, zend_get_executed_filename());
		zend_update_property_ex(ce, error, ZSTR_KNOWN(ZEND_STR_FILE), &tmp);
		zval_ptr_dtor(&tmp);
		ZVAL_LONG(&tmp, zend_get_executed_lineno());
		zend_update_property_ex(ce, error, ZSTR_KNOWN(ZEND_STR_LINE), &tmp);
	}
}

int async_get_poll_fd(zval *val, php_socket_t *sock, zend_string **error)
{
	php_socket_t fd;
	php_stream *stream;

	stream = (php_stream *) zend_fetch_resource_ex(val, NULL, php_file_le_stream());
	*error = NULL;

#if ASYNC_SOCKETS
	php_socket *socket;

	if (!stream && le_socket && (socket = (php_socket *) zend_fetch_resource_ex(val, NULL, php_sockets_le_socket()))) {
#ifdef PHP_WIN32
		if (UNEXPECTED(socket->bsd_socket == INVALID_SOCKET)) {
#else
		if (UNEXPECTED(socket->bsd_socket < 1)) {
#endif
			*error = zend_string_init(ZEND_STRL("Underlying file descriptor is invalid (or closed)"), 0);
			return FAILURE;
		}

		*sock = socket->bsd_socket;

		return SUCCESS;
	}
#endif

	if (UNEXPECTED(!stream)) {
		*error = zend_string_init(ZEND_STRL("Unable to fetch stream resource"), 0);
		return FAILURE;
	}

	if (stream->wrapper) {
		if (!strcmp((char *)stream->wrapper->wops->label, "PHP")) {
			if (!stream->orig_path || (strncmp(stream->orig_path, "php://std", sizeof("php://std") - 1) && strncmp(stream->orig_path, "php://fd", sizeof("php://fd") - 1))) {
				ASYNC_STRF(*error, "Unsupported stream type: %s", stream->wrapper->wops->label);
				return FAILURE;
			}
		}
	}

	if (UNEXPECTED(php_stream_cast(stream, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL, (void *) &fd, 1) != SUCCESS)) {
		*error = zend_string_init(ZEND_STRL("Stream could not be cast to fd"), 0);
		return FAILURE;
	}

#ifdef PHP_WIN32
	if (UNEXPECTED(fd == INVALID_SOCKET)) {
#else
	if (UNEXPECTED(fd < 1)) {
#endif
		*error = zend_string_init(ZEND_STRL("Underlying file descriptor is invalid (or closed)"), 0);
		return FAILURE;
	}

	if (stream->wrapper && !strcmp((char *) stream->wrapper->wops->label, "plainfile")) {
#ifndef PHP_WIN32
		struct stat stat;
		fstat(fd, &stat);

		if (!S_ISFIFO(stat.st_mode)) {
			*error = zend_string_init(ZEND_STRL("Invalid file descriptor"), 0);
			return FAILURE;
		}
#else
		*error = zend_string_init(ZEND_STRL("Invalid file descriptor"), 0);
		return FAILURE;
#endif
	}

	*sock = fd;

	return SUCCESS;
}

void async_helper_init()
{
#if ASYNC_SOCKETS
	zend_module_entry *sockets;

	le_socket = NULL;

	if ((sockets = zend_hash_str_find_ptr(&module_registry, ZEND_STRL("sockets")))) {
		if (sockets->handle) { // shared
			le_socket = (int (*)(void)) DL_FETCH_SYMBOL(sockets->handle, "php_sockets_le_socket");

			if (le_socket == NULL) {
				le_socket = (int (*)(void)) DL_FETCH_SYMBOL(sockets->handle, "_php_sockets_le_socket");
			}
		}
	}
#endif
}
