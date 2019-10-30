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
#include "kernel/backend.h"

ASYNC_API zend_class_entry *async_poll_ce;

static zend_object_handlers async_poll_handlers;

typedef struct _async_poll {
	/* PHP object handle. */
	zend_object std;

	/* Poll close error (undef by default). */
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
} async_poll;


static zend_always_inline void sync_poll(async_poll *poll);

ASYNC_CALLBACK trigger_poll(uv_poll_t *handle, int status, int events)
{
	async_poll *poll;
	async_op *op;
	async_op *read;
	async_op *write;
	zend_bool cont;

	poll = (async_poll *) handle->data;

	ZEND_ASSERT(poll != NULL);

	if (EXPECTED(Z_TYPE_P(&poll->error) == IS_UNDEF)) {
		read = poll->reads.first;
		write = poll->writes.first;
	
		if (events & (UV_READABLE | UV_DISCONNECT)) {
			cont = 1;
		
			while (cont && poll->reads.first != NULL) {
				ASYNC_NEXT_OP(&poll->reads, op);
				
				cont = (op != read);
				
				ASYNC_FINISH_OP(op);
			}
		}
	
		if (events & (UV_WRITABLE | UV_DISCONNECT)) {
			cont = 1;
			
			while (cont && poll->writes.first != NULL) {
				ASYNC_NEXT_OP(&poll->writes, op);
				
				cont = (op != write);
				
				ASYNC_FINISH_OP(op);
			}
		}
	} else {
		while (poll->reads.first != NULL) {
			ASYNC_NEXT_OP(&poll->reads, op);
			ASYNC_FAIL_OP(op, &poll->error);
		}
		
		while (poll->writes.first != NULL) {
			ASYNC_NEXT_OP(&poll->writes, op);
			ASYNC_FAIL_OP(op, &poll->error);
		}
	}
	
	if (!uv_is_closing((uv_handle_t *) handle)) {
		sync_poll(poll);
	}
}

ASYNC_CALLBACK close_poll(uv_handle_t *handle)
{
	async_poll *poll;

	poll = (async_poll *) handle->data;

	ZEND_ASSERT(poll != NULL);

	ASYNC_DELREF(&poll->std);
}

ASYNC_CALLBACK shutdown_poll(void *obj, zval *error)
{
	async_poll *poll;
	async_op *op;

	poll = (async_poll *) obj;

	ZEND_ASSERT(poll != NULL);
	
	poll->cancel.func = NULL;
	
	if (error != NULL && Z_TYPE_P(&poll->error) == IS_UNDEF) {
		ZVAL_COPY(&poll->error, error);
	}
	
	ASYNC_UV_TRY_CLOSE_REF(&poll->std, &poll->handle, close_poll);

	if (error != NULL) {
		while (poll->reads.first != NULL) {
			ASYNC_NEXT_OP(&poll->reads, op);
			ASYNC_FAIL_OP(op, &poll->error);
		}
		
		while (poll->writes.first != NULL) {
			ASYNC_NEXT_OP(&poll->writes, op);
			ASYNC_FAIL_OP(op, &poll->error);
		}
	}
}

static zend_always_inline void sync_poll(async_poll *poll)
{
	int events;
	
	events = UV_DISCONNECT;
	
	if (poll->reads.first != NULL) {
		events |= UV_READABLE;
	}
	
	if (poll->writes.first != NULL) {
		events |= UV_WRITABLE;
	}
	
	if (events != poll->events) {
		if (events == UV_DISCONNECT) {
			uv_poll_stop(&poll->handle);
		} else {	
			uv_poll_start(&poll->handle, events, trigger_poll);
		}
		
		poll->events = events;
	}
}

static zend_always_inline void suspend(async_poll *poll, async_op_list *q)
{
	async_context *context;
	async_op *op;
	
	context = async_context_get();
	
	ASYNC_ALLOC_OP(op);
	ASYNC_APPEND_OP(q, op);
	
	sync_poll(poll);

	ASYNC_UNREF_ENTER(context, poll);

	if (UNEXPECTED(async_await_op(op) == FAILURE)) {
		ASYNC_FORWARD_OP_ERROR(op);
	}

	ASYNC_UNREF_EXIT(context, poll);
	ASYNC_FREE_OP(op);
}


static zend_object *async_poll_object_create(zend_class_entry *ce)
{
	async_poll *poll;

	poll = ecalloc(1, sizeof(async_poll));

	zend_object_std_init(&poll->std, ce);
	poll->std.handlers = &async_poll_handlers;

	poll->scheduler = async_task_scheduler_ref();

	ZVAL_UNDEF(&poll->error);
	ZVAL_NULL(&poll->resource);
	
	poll->cancel.object = poll;
	poll->cancel.func = shutdown_poll;

	return &poll->std;
}

static void async_poll_object_dtor(zend_object *object)
{
	async_poll *poll;

	poll = (async_poll *) object;

	if (poll->cancel.func != NULL) {
		ASYNC_LIST_REMOVE(&poll->scheduler->shutdown, &poll->cancel);
	
		poll->cancel.func(poll, NULL);
	}
}

static void async_poll_object_destroy(zend_object *object)
{
	async_poll *poll;

	poll = (async_poll *) object;

	zval_ptr_dtor(&poll->error);
	zval_ptr_dtor(&poll->resource);
	
	async_task_scheduler_unref(poll->scheduler);

	zend_object_std_dtor(&poll->std);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_poll_ctor, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, resource, IS_RESOURCE, 0)
ZEND_END_ARG_INFO();

PHP_METHOD(Poll, __construct)
{
	async_poll *poll;

	php_socket_t fd;
	zend_string *error;

	zval *val;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_RESOURCE(val)
	ZEND_PARSE_PARAMETERS_END();

	if (UNEXPECTED(FAILURE == async_get_poll_fd(val, &fd, &error))) {
		zend_throw_error(NULL, "%s", ZSTR_VAL(error));
		zend_string_release(error);
		return;
	}

	poll = (async_poll *) Z_OBJ_P(getThis());

	poll->fd = fd;

	ZVAL_COPY(&poll->resource, val);

#ifdef PHP_WIN32
	uv_poll_init_socket(&poll->scheduler->loop, &poll->handle, (uv_os_sock_t) fd);
#else
	uv_poll_init(&poll->scheduler->loop, &poll->handle, (int) fd);
#endif

	uv_unref((uv_handle_t *) &poll->handle);

	poll->handle.data = poll;
	
	ASYNC_LIST_APPEND(&poll->scheduler->shutdown, &poll->cancel);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_poll_close, 0, 0, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 1)
ZEND_END_ARG_INFO();

PHP_METHOD(Poll, close)
{
	async_poll *poll;

	zval error;
	zval *val;

	val = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_OBJECT_OF_CLASS_EX(val, zend_ce_throwable, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	poll = (async_poll *) Z_OBJ_P(getThis());

	if (poll->cancel.func == NULL) {
		return;
	}

	ASYNC_PREPARE_ERROR(&error, execute_data, "Poll has been closed");
	
	if (val != NULL && Z_TYPE_P(val) != IS_NULL) {
		zend_exception_set_previous(Z_OBJ_P(&error), Z_OBJ_P(val));
		GC_ADDREF(Z_OBJ_P(val));
	}
	
	ASYNC_LIST_REMOVE(&poll->scheduler->shutdown, &poll->cancel);
	
	poll->cancel.func(poll, &error);
	
	zval_ptr_dtor(&error);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_poll_await_readable, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO();

PHP_METHOD(Poll, awaitReadable)
{
	async_poll *poll;

	ZEND_PARSE_PARAMETERS_NONE();

	poll = (async_poll *) Z_OBJ_P(getThis());

	if (UNEXPECTED(Z_TYPE_P(&poll->error) != IS_UNDEF)) {
		ASYNC_FORWARD_ERROR(&poll->error);
		return;
	}

	ZEND_ASSERT(zend_rsrc_list_get_rsrc_type(Z_RES_P(&poll->resource)));

	suspend(poll, &poll->reads);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_poll_await_writable, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO();

PHP_METHOD(Poll, awaitWritable)
{
	async_poll *poll;

	ZEND_PARSE_PARAMETERS_NONE();

	poll = (async_poll *) Z_OBJ_P(getThis());

	if (UNEXPECTED(Z_TYPE_P(&poll->error) != IS_UNDEF)) {
		ASYNC_FORWARD_ERROR(&poll->error);
		return;
	}

	ZEND_ASSERT(zend_rsrc_list_get_rsrc_type(Z_RES_P(&poll->resource)));

	suspend(poll, &poll->writes);
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_WAKEUP(Poll, async_poll_ce)
//LCOV_EXCL_STOP

static const zend_function_entry async_poll_functions[] = {
	PHP_ME(Poll, __construct, arginfo_poll_ctor, ZEND_ACC_PUBLIC)
	PHP_ME(Poll, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(Poll, close, arginfo_poll_close, ZEND_ACC_PUBLIC)
	PHP_ME(Poll, awaitReadable, arginfo_poll_await_readable, ZEND_ACC_PUBLIC)
	PHP_ME(Poll, awaitWritable, arginfo_poll_await_writable, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


void async_poll_ce_register()
{
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async", "Poll", async_poll_functions);
	async_poll_ce = zend_register_internal_class(&ce);
	async_poll_ce->ce_flags |= ZEND_ACC_FINAL;
	async_poll_ce->create_object = async_poll_object_create;
	async_poll_ce->serialize = zend_class_serialize_deny;
	async_poll_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_poll_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_poll_handlers.free_obj = async_poll_object_destroy;
	async_poll_handlers.dtor_obj = async_poll_object_dtor;
	async_poll_handlers.clone_obj = NULL;
}
