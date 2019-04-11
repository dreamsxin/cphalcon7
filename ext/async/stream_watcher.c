
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2014 Phalcon Team (http://www.phalconphp.com)       |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled     |
  | with this package in the file docs/LICENSE.txt.                        |
  |                                                                        |
  | If you did not receive a copy of the license and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@phalconphp.com so we can send you a copy immediately.       |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@phalconphp.com>                      |
  |          Eduar Carvajal <eduar@phalconphp.com>                         |
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  |          Martin Schröder <m.schroeder2007@gmail.com>                   |
  +------------------------------------------------------------------------+
*/

#include "async/core.h"
#include "kernel/backend.h"

#if PHALCON_USE_UV

ASYNC_API zend_class_entry *async_stream_watcher_ce;

static zend_object_handlers async_stream_watcher_handlers;

typedef struct {
	/* PHP object handle. */
	zend_object std;

	/* Error being set as the watcher was closed (undef by default). */
	zval error;

	/* PHP stream or socket being observed. */
	zval resource;

	/** File descriptor being polled by libuv. */
	php_socket_t fd;

	/* Libuv poll instance being used to receive events. */
	uv_poll_t handle;
	
	/* Mask of active events. */
	int events;
	
	/* Task scheduler running the event loop. */
	async_task_scheduler *scheduler;

	/* Queue of tasks wanting to be notified when the stream is readable. */
	async_op_list reads;

	/* Queue of tasks wanting to be notified when the stream is writable. */
	async_op_list writes;

	/* Number of pending referenced read / write operations. */
	zend_uchar ref_count;

	/* Shutdown callback. */
	async_cancel_cb cancel;
} async_stream_watcher;


#if ASYNC_SOCKETS
static int (*le_socket)(void);
#endif

static php_socket_t get_poll_fd(zval *val)
{
	php_socket_t fd;
	php_stream *stream;

	stream = (php_stream *) zend_fetch_resource_ex(val, NULL, php_file_le_stream());

#if ASYNC_SOCKETS
	php_socket *socket;

	if (!stream && le_socket && (socket = (php_socket *) zend_fetch_resource_ex(val, NULL, php_sockets_le_socket()))) {
		return socket->bsd_socket;
	}
#endif

	if (UNEXPECTED(!stream)) {
		return -1;
	}

	if (stream->wrapper) {
		if (!strcmp((char *)stream->wrapper->wops->label, "PHP")) {
			if (!stream->orig_path || (strncmp(stream->orig_path, "php://std", sizeof("php://std") - 1) && strncmp(stream->orig_path, "php://fd", sizeof("php://fd") - 1))) {
				return -1;
			}
		}
	}

	if (UNEXPECTED(php_stream_cast(stream, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL, (void *) &fd, 1) != SUCCESS)) {
		return -1;
	}

	if (UNEXPECTED(fd < 1)) {
		return -1;
	}

	if (stream->wrapper && !strcmp((char *) stream->wrapper->wops->label, "plainfile")) {
#ifndef PHP_WIN32
		struct stat stat;
		fstat(fd, &stat);

		if (!S_ISFIFO(stat.st_mode)) {
			return -1;
		}
#else
		return -1;
#endif
	}

	return fd;
}

static zend_always_inline void sync_poll(async_stream_watcher *watcher);

ASYNC_CALLBACK trigger_poll(uv_poll_t *handle, int status, int events)
{
	async_stream_watcher *watcher;
	async_op *op;
	async_op *read;
	async_op *write;
	zend_bool cont;

	watcher = (async_stream_watcher *) handle->data;

	ZEND_ASSERT(watcher != NULL);

	if (EXPECTED(Z_TYPE_P(&watcher->error) == IS_UNDEF)) {
		read = watcher->reads.first;
		write = watcher->writes.first;
	
		if (events & (UV_READABLE | UV_DISCONNECT)) {
			cont = 1;
		
			while (cont && watcher->reads.first != NULL) {
				ASYNC_NEXT_OP(&watcher->reads, op);
				
				cont = (op != read);
				
				ASYNC_FINISH_OP(op);
			}
		}
	
		if (events & (UV_WRITABLE | UV_DISCONNECT)) {
			cont = 1;
			
			while (cont && watcher->writes.first != NULL) {
				ASYNC_NEXT_OP(&watcher->writes, op);
				
				cont = (op != write);
				
				ASYNC_FINISH_OP(op);
			}
		}
	} else {
		while (watcher->reads.first != NULL) {
			ASYNC_NEXT_OP(&watcher->reads, op);
			ASYNC_FAIL_OP(op, &watcher->error);
		}
		
		while (watcher->writes.first != NULL) {
			ASYNC_NEXT_OP(&watcher->writes, op);
			ASYNC_FAIL_OP(op, &watcher->error);
		}
	}
	
	if (!uv_is_closing((uv_handle_t *) handle)) {
		sync_poll(watcher);
	}
}

ASYNC_CALLBACK close_poll(uv_handle_t *handle)
{
	async_stream_watcher *watcher;

	watcher = (async_stream_watcher *) handle->data;

	ZEND_ASSERT(watcher != NULL);

	ASYNC_DELREF(&watcher->std);
}

ASYNC_CALLBACK shutdown_poll(void *obj, zval *error)
{
	async_stream_watcher *watcher;
	async_op *op;

	watcher = (async_stream_watcher *) obj;

	ZEND_ASSERT(watcher != NULL);
	
	watcher->cancel.func = NULL;
	
	if (error != NULL && Z_TYPE_P(&watcher->error) == IS_UNDEF) {
		ZVAL_COPY(&watcher->error, error);
	}

	if (!uv_is_closing((uv_handle_t *) &watcher->handle)) {
		ASYNC_ADDREF(&watcher->std);

		uv_close((uv_handle_t *) &watcher->handle, close_poll);
	}
	
	if (error != NULL) {
		while (watcher->reads.first != NULL) {
			ASYNC_NEXT_OP(&watcher->reads, op);
			ASYNC_FAIL_OP(op, &watcher->error);
		}
		
		while (watcher->writes.first != NULL) {
			ASYNC_NEXT_OP(&watcher->writes, op);
			ASYNC_FAIL_OP(op, &watcher->error);
		}
	}
}

static zend_always_inline void sync_poll(async_stream_watcher *watcher)
{
	int events;
	
	events = UV_DISCONNECT;
	
	if (watcher->reads.first != NULL) {
		events |= UV_READABLE;
	}
	
	if (watcher->writes.first != NULL) {
		events |= UV_WRITABLE;
	}
	
	if (events != watcher->events) {
		if (events == UV_DISCONNECT) {
			uv_poll_stop(&watcher->handle);
		} else {	
			uv_poll_start(&watcher->handle, events, trigger_poll);
		}
		
		watcher->events = events;
	}
}

static zend_always_inline void suspend(async_stream_watcher *watcher, async_op_list *q)
{
	async_context *context;
	async_op *op;
	
	context = async_context_get();
	
	ASYNC_ALLOC_OP(op);
	ASYNC_APPEND_OP(q, op);
	
	sync_poll(watcher);

	ASYNC_UNREF_ENTER(context, watcher);

	if (UNEXPECTED(async_await_op(op) == FAILURE)) {
		ASYNC_FORWARD_OP_ERROR(op);
	}

	ASYNC_UNREF_EXIT(context, watcher);
	ASYNC_FREE_OP(op);
}


static zend_object *async_stream_watcher_object_create(zend_class_entry *ce)
{
	async_stream_watcher *watcher;

	watcher = ecalloc(1, sizeof(async_stream_watcher));

	zend_object_std_init(&watcher->std, ce);
	watcher->std.handlers = &async_stream_watcher_handlers;

	watcher->scheduler = async_task_scheduler_ref();

	ZVAL_UNDEF(&watcher->error);
	ZVAL_NULL(&watcher->resource);
	
	watcher->cancel.object = watcher;
	watcher->cancel.func = shutdown_poll;

	return &watcher->std;
}

static void async_stream_watcher_object_dtor(zend_object *object)
{
	async_stream_watcher *watcher;

	watcher = (async_stream_watcher *) object;

	if (watcher->cancel.func != NULL) {
		ASYNC_LIST_REMOVE(&watcher->scheduler->shutdown, &watcher->cancel);
	
		watcher->cancel.func(watcher, NULL);
	}
}

static void async_stream_watcher_object_destroy(zend_object *object)
{
	async_stream_watcher *watcher;

	watcher = (async_stream_watcher *) object;

	zval_ptr_dtor(&watcher->error);
	zval_ptr_dtor(&watcher->resource);
	
	async_task_scheduler_unref(watcher->scheduler);

	zend_object_std_dtor(&watcher->std);
}

ZEND_METHOD(StreamWatcher, __construct)
{
	async_stream_watcher *watcher;
	php_socket_t fd;

	zval *val;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();

	watcher = (async_stream_watcher *) Z_OBJ_P(getThis());

	fd = get_poll_fd(val);

	ASYNC_CHECK_ERROR(fd < 0, "Cannot cast resource to file descriptor");

	watcher->fd = fd;

	ZVAL_COPY(&watcher->resource, val);

#ifdef PHP_WIN32
	uv_poll_init_socket(&watcher->scheduler->loop, &watcher->handle, (uv_os_sock_t) fd);
#else
	uv_poll_init(&watcher->scheduler->loop, &watcher->handle, fd);
#endif

	uv_unref((uv_handle_t *) &watcher->handle);

	watcher->handle.data = watcher;
	
	ASYNC_LIST_APPEND(&watcher->scheduler->shutdown, &watcher->cancel);
}

ZEND_METHOD(StreamWatcher, close)
{
	async_stream_watcher *watcher;

	zval error;
	zval *val;

	val = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();

	watcher = (async_stream_watcher *) Z_OBJ_P(getThis());

	if (watcher->cancel.func == NULL) {
		return;
	}
	
	ASYNC_PREPARE_ERROR(&error, "Stream watcher has been closed");
	
	if (val != NULL && Z_TYPE_P(val) != IS_NULL) {
		zend_exception_set_previous(Z_OBJ_P(&error), Z_OBJ_P(val));
		GC_ADDREF(Z_OBJ_P(val));
	}
	
	ASYNC_LIST_REMOVE(&watcher->scheduler->shutdown, &watcher->cancel);
	
	watcher->cancel.func(watcher, &error);
	
	zval_ptr_dtor(&error);
}

ZEND_METHOD(StreamWatcher, awaitReadable)
{
	async_stream_watcher *watcher;

	ZEND_PARSE_PARAMETERS_NONE();

	watcher = (async_stream_watcher *) Z_OBJ_P(getThis());

	if (UNEXPECTED(Z_TYPE_P(&watcher->error) != IS_UNDEF)) {
		ASYNC_FORWARD_ERROR(&watcher->error);
		return;
	}

	ZEND_ASSERT(zend_rsrc_list_get_rsrc_type(Z_RES_P(&watcher->resource)));

	suspend(watcher, &watcher->reads);
}

ZEND_METHOD(StreamWatcher, awaitWritable)
{
	async_stream_watcher *watcher;

	ZEND_PARSE_PARAMETERS_NONE();

	watcher = (async_stream_watcher *) Z_OBJ_P(getThis());

	if (UNEXPECTED(Z_TYPE_P(&watcher->error) != IS_UNDEF)) {
		ASYNC_FORWARD_ERROR(&watcher->error);
		return;
	}

	ZEND_ASSERT(zend_rsrc_list_get_rsrc_type(Z_RES_P(&watcher->resource)));

	suspend(watcher, &watcher->writes);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_stream_watcher_ctor, 0, 0, 1)
	ZEND_ARG_INFO(0, resource)
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_watcher_close, 0, 0, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_watcher_await_readable, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_watcher_await_writable, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_watcher_close, 0, 0, IS_VOID, NULL, 0)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_watcher_await_readable, 0, 0, IS_VOID, NULL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stream_watcher_await_writable, 0, 0, IS_VOID, NULL, 0)
ZEND_END_ARG_INFO()
#endif

static const zend_function_entry async_stream_watcher_functions[] = {
	ZEND_ME(StreamWatcher, __construct, arginfo_stream_watcher_ctor, ZEND_ACC_PUBLIC)
	ZEND_ME(StreamWatcher, close, arginfo_stream_watcher_close, ZEND_ACC_PUBLIC)
	ZEND_ME(StreamWatcher, awaitReadable, arginfo_stream_watcher_await_readable, ZEND_ACC_PUBLIC)
	ZEND_ME(StreamWatcher, awaitWritable, arginfo_stream_watcher_await_writable, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


void async_stream_watcher_ce_register()
{
#if ASYNC_SOCKETS
	zend_module_entry *sockets;
#endif

	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\StreamWatcher", async_stream_watcher_functions);
	async_stream_watcher_ce = zend_register_internal_class(&ce);
	async_stream_watcher_ce->ce_flags |= ZEND_ACC_FINAL;
	async_stream_watcher_ce->create_object = async_stream_watcher_object_create;
	async_stream_watcher_ce->serialize = zend_class_serialize_deny;
	async_stream_watcher_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_stream_watcher_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_stream_watcher_handlers.free_obj = async_stream_watcher_object_destroy;
	async_stream_watcher_handlers.dtor_obj = async_stream_watcher_object_dtor;
	async_stream_watcher_handlers.clone_obj = NULL;

#if ASYNC_SOCKETS
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

#endif
