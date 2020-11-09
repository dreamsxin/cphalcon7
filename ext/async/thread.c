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
#include "async/async_pipe.h"


#include <main/SAPI.h>
#include <main/php_main.h>
#include <Zend/zend_inheritance.h>

ASYNC_API zend_class_entry *async_thread_ce;

static zend_object_handlers async_thread_handlers;

#ifdef ZTS

static zend_string *str_main;

static void (*prev_interrupt_handler)(zend_execute_data*);
typedef int (*php_sapi_deactivate_t)(void);

static php_sapi_deactivate_t sapi_deactivate_func;

#endif

#define ASYNC_THREAD_FLAG_RUNNING 1
#define ASYNC_THREAD_FLAG_TERMINATED (1 << 1)
#define ASYNC_THREAD_FLAG_KILLED (1 << 2)

typedef struct _async_thread {
	zend_object std;
	
	uint16_t flags;
	
	async_task_scheduler *scheduler;
	async_cancel_cb shutdown;
	
	uv_thread_t impl;
	uv_async_t handle;
	uv_mutex_t mutex;
	
	async_pipe *master;
	async_pipe *slave;
	
#ifdef PHP_WIN32
	uv_pipe_t server;
	async_uv_op accept;
	async_uv_op connect;
	char ipc[128];
#else
	uv_file pipes[2];
#endif
	
	zend_string *bootstrap;
	
	zval error;
	int exit_code;
	zend_bool *interrupt;
	
	async_op_list join;
} async_thread;


#ifdef ZTS

static void run_bootstrap(async_thread *thread)
{
	zend_file_handle handle;
	zend_op_array *ops;
	
	zval retval;
	
	if (SUCCESS != php_stream_open_for_zend_ex(ZSTR_VAL(thread->bootstrap), &handle, USE_PATH | REPORT_ERRORS | STREAM_OPEN_FOR_INCLUDE)) {
		return;
	}
	
	if (!handle.opened_path) {
		handle.opened_path = zend_string_dup(thread->bootstrap, 0);
	}
	
	zend_hash_add_empty_element(&EG(included_files), handle.opened_path);
	
	ops = zend_compile_file(&handle, ZEND_REQUIRE);
	zend_destroy_file_handle(&handle);
	
	if (ops) {
		ZVAL_UNDEF(&retval);
		zend_execute(ops, &retval);
		destroy_op_array(ops);
		efree(ops);
		
		if (!EG(exception)) {
			zval_ptr_dtor(&retval);
		}
	}
	
	if (EG(exception)) {
		EG(exit_status) = 1;
		
		zend_clear_exception();
	}
}

#ifdef PHP_WIN32

ASYNC_CALLBACK ipc_connect_cb(uv_connect_t *req, int status)
{
	async_thread *thread;
	
	thread = (async_thread *) req->data;
	
	ZEND_ASSERT(thread != NULL);
	
	thread->connect.code = status;
	
	ASYNC_FINISH_OP(&thread->connect);
}

static int establish_ipc_slave(async_thread *thread)
{
	async_pipe *pipe;
	uv_connect_t *req;
	
	int code;
	
	pipe = async_pipe_init_ipc();
	
	req = emalloc(sizeof(uv_connect_t));
	req->data = thread;
	
	uv_pipe_connect(req, &pipe->handle, thread->ipc, ipc_connect_cb);
	
	if (UNEXPECTED(FAILURE == async_await_op((async_op *) &thread->connect))) {
		ASYNC_RESET_OP(&thread->connect);
		ASYNC_DELREF(&pipe->std);
		
		return FAILURE;
	}
	
	code = thread->connect.code;
	ASYNC_RESET_OP(&thread->connect);
	
	if (UNEXPECTED(code < 0)) {
		ASYNC_DELREF(&pipe->std);
	} else {
		thread->slave = pipe;
	}
	
	return code;
}

#endif

ASYNC_CALLBACK run_thread(void *arg)
{
	async_thread *thread;
	async_task_scheduler *scheduler;
	
	thread = (async_thread *) arg;
	
	ts_resource(0);

	TSRMLS_CACHE_UPDATE();
	
	uv_mutex_lock(&thread->mutex);	
	thread->interrupt = &EG(vm_interrupt);	
	uv_mutex_unlock(&thread->mutex);
	
	PG(expose_php) = 0;
	PG(auto_globals_jit) = 1;
	
	php_request_startup();
#if PHP_VERSION_ID < 80000
	zend_disable_function(ZEND_STRL("setlocale"));
	zend_disable_function(ZEND_STRL("dl"));
	
# if PHP_VERSION_ID < 70400
	zend_disable_function(ZEND_STRL("putenv"));
# endif
#else
	zend_disable_functions("setlocale,dl"));
#endif
	
	PG(during_request_startup) = 0;
	SG(sapi_started) = 0;
	SG(headers_sent) = 1;
	SG(request_info).no_headers = 1;
	
	ASYNC_G(cli) = 1;
	ASYNC_G(thread) = &thread->std;
	
#ifdef PHP_WIN32
	if (establish_ipc_slave(thread)) {
		abort();
	}
#else
	async_pipe *pipe;
	int code;

	pipe = async_pipe_init_ipc();
	code = uv_pipe_open(&pipe->handle, thread->pipes[1]);
	
	if (UNEXPECTED(code < 0)) {
		abort();
	}
	
	thread->slave = pipe;
#endif
	
	scheduler = async_task_scheduler_get();

	zend_first_try {
		run_bootstrap(thread);
	} zend_catch {
		async_task_scheduler_handle_exit(scheduler);
	} zend_end_try();
	
	uv_mutex_lock(&thread->mutex);
	thread->interrupt = NULL;
	uv_mutex_unlock(&thread->mutex);
	
	if (async_stream_call_close_obj(&thread->slave->std)) {
		ASYNC_DELREF(&thread->slave->std);
	}
	
	uv_mutex_lock(&thread->mutex);
	thread->exit_code = EG(exit_status);
	uv_mutex_unlock(&thread->mutex);
	
	php_request_shutdown(NULL);
	ts_free_thread();
	
	uv_async_send(&thread->handle);
}

#ifdef PHP_WIN32

ASYNC_CALLBACK ipc_close_cb(uv_handle_t *handle)
{
	async_thread *thread;
	
	thread = (async_thread *) handle->data;
	
	ZEND_ASSERT(thread != NULL);
	
	ASYNC_DELREF(&thread->std);
}

#endif

ASYNC_CALLBACK shutdown_cb(void *arg, zval *error)
{
	async_thread *thread;
	
	thread = (async_thread *) arg;
	
	thread->shutdown.func = NULL;
	
#ifdef PHP_WIN32
	ASYNC_UV_TRY_CLOSE_REF(&thread->std, &thread->server, ipc_close_cb);
#endif
}

ASYNC_CALLBACK close_async_cb(uv_handle_t *handle)
{
	async_thread *thread;
	
	thread = (async_thread *) handle->data;
	
	ZEND_ASSERT(thread != NULL);
	
	ASYNC_DELREF(&thread->std);
}

ASYNC_CALLBACK notify_thread_cb(uv_async_t *handle)
{
	async_thread *thread;
	
	thread = (async_thread *) handle->data;
	
	ZEND_ASSERT(thread != NULL);
	
	thread->flags |= ASYNC_THREAD_FLAG_TERMINATED;
	
	if (EXPECTED(thread->shutdown.func)) {
		thread->shutdown.func = NULL;
		
		ASYNC_LIST_REMOVE(&thread->scheduler->shutdown, &thread->shutdown);
	}
	
	while (thread->join.first != NULL) {
		ASYNC_FINISH_OP(thread->join.first);
	}
	
#ifdef PHP_WIN32
	if (thread->accept.base.status == ASYNC_STATUS_RUNNING) {
		thread->accept.code = UV_ECONNREFUSED;
		
		ASYNC_FINISH_OP(&thread->accept);
	}
#else
	closesocket(thread->pipes[1]);
#endif

	ASYNC_UV_TRY_CLOSE(handle, close_async_cb);
}

#endif

static zend_object *async_thread_object_create(zend_class_entry *ce)
{
	async_thread *thread;
	
	thread = ecalloc(1, sizeof(async_thread));
	
	zend_object_std_init(&thread->std, ce);
	thread->std.handlers = &async_thread_handlers;
	
#ifdef ZTS
	thread->scheduler = async_task_scheduler_ref();
	
	thread->shutdown.func = shutdown_cb;
	thread->shutdown.object = thread;
	
	ASYNC_LIST_APPEND(&thread->scheduler->shutdown, &thread->shutdown);
	
	uv_async_init(&thread->scheduler->loop, &thread->handle, notify_thread_cb);
	uv_mutex_init_recursive(&thread->mutex);
	
	thread->handle.data = thread;
	
	ASYNC_ADDREF(&thread->std);
#endif

	return &thread->std;
}

static void async_thread_object_dtor(zend_object *object)
{
#ifdef ZTS
	async_thread *thread;
	
	thread = (async_thread *) object;
	
	if (thread->shutdown.func != NULL) {
		ASYNC_LIST_REMOVE(&thread->scheduler->shutdown, &thread->shutdown);
		
		thread->shutdown.func(thread, NULL);
	}
#endif
}

static void async_thread_object_destroy(zend_object *object)
{
	async_thread *thread;
	
	thread = (async_thread *) object;
	
#ifdef ZTS
	if (thread->flags & ASYNC_THREAD_FLAG_RUNNING) {
		thread->flags &= ~ASYNC_THREAD_FLAG_RUNNING;
		
		uv_thread_join(&thread->impl);
	}
	
	uv_mutex_destroy(&thread->mutex);
	
	if (EXPECTED(thread->master)) {
		ASYNC_DELREF(&thread->master->std);
	}
	
	async_task_scheduler_unref(thread->scheduler);
	
	zval_ptr_dtor(&thread->error);
	
	if (thread->bootstrap != NULL) {
		zend_string_release(thread->bootstrap);
	}
#endif
	
	zend_object_std_dtor(&thread->std);
}

#ifdef ZTS
#ifdef PHP_WIN32

ASYNC_CALLBACK ipc_listen_cb(uv_stream_t *handle, int status)
{
	async_thread *thread;
	async_pipe *pipe;
	
	zval obj;
	
	thread = (async_thread *) handle->data;
	
	ZEND_ASSERT(thread != NULL);
	
	if (thread->accept.base.status != ASYNC_STATUS_RUNNING) {
		return;
	}
	
	thread->accept.code = status;
	
	if (UNEXPECTED(status < 0)) {
		ASYNC_FINISH_OP(&thread->accept);
	} else {
		pipe = async_pipe_init_ipc();
		uv_accept(handle, (uv_stream_t *) &pipe->handle);
		
		ZVAL_OBJ(&obj, &pipe->std);
		ASYNC_RESOLVE_OP(&thread->accept, &obj);
		zval_ptr_dtor(&obj);
	}
}

#endif
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_thread_ctor, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, file, IS_STRING, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Thread, __construct)
{
#ifndef ZTS
	zend_throw_error(NULL, "Threads require PHP to be compiled in thread safe mode (ZTS)");
#else
	async_thread *thread;
	async_pipe *pipe;
	zend_string *file;
	
	char path[MAXPATHLEN];
	int code;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STR(file)
	ZEND_PARSE_PARAMETERS_END();
	
	ASYNC_CHECK_ERROR(!ASYNC_G(cli), "Threads are only supported if PHP is run from the command line (cli)");
	
	thread = (async_thread *) Z_OBJ_P(getThis());
	
	if (UNEXPECTED(!VCWD_REALPATH(ZSTR_VAL(file), path))) {
		ASYNC_UV_TRY_CLOSE(&thread->handle, close_async_cb);

		zend_throw_error(NULL, "Failed to locate thread bootstrap file: %s", ZSTR_VAL(file));
		return;
	}
	
	ASYNC_CHECK_ERROR(!VCWD_REALPATH(ZSTR_VAL(file), path), "Failed to locate thread bootstrap file: %s", ZSTR_VAL(file));
	
	thread->bootstrap = zend_string_init(path, strlen(path), 1);
	
#ifdef PHP_WIN32
	int i;
	
	code = uv_pipe_init(&thread->scheduler->loop, &thread->server, 0);
	
	ASYNC_CHECK_ERROR(code < 0, "Failed to create IPC pipe: %s", uv_strerror(code));
	
	thread->server.data = thread;
	
	for (i = 0; i < 5; i++) {
		sprintf(thread->ipc, "\\\\.\\pipe\\php\\%p-%lu.sock", (void *)(((char *) thread) + i), GetCurrentProcessId());
		
		code = uv_pipe_bind(&thread->server, thread->ipc);
		
		if (EXPECTED(code != UV_EADDRINUSE)) {
			break;
		}
	}
	
	if (UNEXPECTED(code != 0)) {
		ASYNC_UV_TRY_CLOSE(&thread->handle, close_async_cb);
	
		zend_throw_error(NULL, "Failed to create IPC pipe: %s", uv_strerror(code));
		return;
	}
	
	uv_pipe_pending_instances(&thread->server, 1);
	
	code = uv_listen((uv_stream_t *) &thread->server, 0, ipc_listen_cb);
	
	if (UNEXPECTED(code < 0)) {
		ASYNC_UV_TRY_CLOSE_REF(&thread->std, &thread->server, ipc_close_cb);
		
		zend_throw_error(NULL, "Failed to create IPC pipe: %s", uv_strerror(code));
		return;
	}
	
	thread->flags |= ASYNC_THREAD_FLAG_RUNNING;
	
	code = uv_thread_create(&thread->impl, run_thread, thread);
	
	if (UNEXPECTED(code < 0)) {
		ASYNC_UV_TRY_CLOSE_REF(&thread->std, &thread->server, ipc_close_cb);
		ASYNC_UV_TRY_CLOSE(&thread->handle, close_async_cb);
	
		zend_throw_error(NULL, "Failed to create thread: %s", uv_strerror(code));
		return;
	}
	
	if (UNEXPECTED(FAILURE == async_await_op((async_op *) &thread->accept))) {
		ASYNC_FORWARD_OP_ERROR(&thread->accept);
		ASYNC_RESET_OP(&thread->accept);
		
		ASYNC_UV_TRY_CLOSE_REF(&thread->std, &thread->server, ipc_close_cb);
		
		return;
	}
	
	code = thread->accept.code;
	
	if (UNEXPECTED(code < 0)) {
		ASYNC_RESET_OP(&thread->accept);
		ASYNC_UV_TRY_CLOSE_REF(&thread->std, &thread->server, ipc_close_cb);
		
		zend_throw_error(NULL, "Failed to create IPC pipe: %s", uv_strerror(code));
		return;
	}
	
	pipe = (async_pipe *) Z_OBJ_P(&thread->accept.base.result);
	
	ASYNC_ADDREF(&pipe->std);	
	ASYNC_RESET_OP(&thread->accept);

	ASYNC_UV_TRY_CLOSE_REF(&thread->std, &thread->server, ipc_close_cb);
	
	thread->master = pipe;
#else
	code = socketpair(AF_UNIX, SOCK_STREAM, 0, thread->pipes);
	
	if (UNEXPECTED(code == -1)) {
		ASYNC_UV_TRY_CLOSE(&thread->handle, close_async_cb);
	
		zend_throw_error(NULL, "Failed to create IPC pipe: %s", uv_strerror(uv_translate_sys_error(errno)));
		return;
	}
	
	thread->flags |= ASYNC_THREAD_FLAG_RUNNING;
	
	code = uv_thread_create(&thread->impl, run_thread, thread);
	
	if (UNEXPECTED(code < 0)) {
		ASYNC_UV_TRY_CLOSE(&thread->handle, close_async_cb);
	
		zend_throw_error(NULL, "Failed to create thread: %s", uv_strerror(code));
		return;
	}

	pipe = async_pipe_init_ipc();
	code = uv_pipe_open(&pipe->handle, thread->pipes[0]);
	
	if (UNEXPECTED(code < 0)) {
		ASYNC_DELREF(&pipe->std);
		
		zend_throw_error(NULL, "Failed to open IPC pipe: %s", uv_strerror(code));
		return;
	}

	thread->master = pipe;
#endif
#endif
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_thread_is_available, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Thread, isAvailable)
{
	ZEND_PARSE_PARAMETERS_NONE();
	
#ifdef ZTS
	RETURN_BOOL(ASYNC_G(cli));
#else
	RETURN_FALSE;
#endif
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_thread_is_worker, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Thread, isWorker)
{
	ZEND_PARSE_PARAMETERS_NONE();
	
	RETURN_BOOL(ASYNC_G(thread) != NULL);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_thread_connect, 0, 0, Phalcon\\Async\\Network\\Pipe, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Thread, connect)
{
	async_thread *thread;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	ASYNC_CHECK_ERROR(ASYNC_G(thread) == NULL, "Only threads can connect to the parent process");
	
	thread = (async_thread *) ASYNC_G(thread);
	
	ASYNC_CHECK_ERROR(!thread->slave, "IPC pipe is not available");
	
	RETVAL_OBJ(&thread->slave->std);
	Z_ADDREF_P(return_value);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_thread_get_ipc, 0, 0, Phalcon\\Async\\Network\\Pipe, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Thread, getIpc)
{
	async_thread *thread;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	thread = (async_thread *) Z_OBJ_P(getThis());
	
	ASYNC_CHECK_ERROR(!thread->master, "IPC pipe is not available");
	
	RETVAL_OBJ(&thread->master->std);
	Z_ADDREF_P(return_value);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_thread_kill, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Thread, kill)
{
#ifdef ZTS
	async_thread *thread;
#endif
	
	ZEND_PARSE_PARAMETERS_NONE();
	
#ifdef ZTS
	thread = (async_thread *) Z_OBJ_P(getThis());
	
	uv_mutex_lock(&thread->mutex);
	
	thread->flags |= ASYNC_THREAD_FLAG_KILLED;
	
	if (thread->interrupt != NULL) {
		*thread->interrupt = 1;
	}
	
	uv_mutex_unlock(&thread->mutex);
#endif
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_thread_join, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Thread, join)
{
#ifdef ZTS
	async_thread *thread;
	async_op *op;
#endif
	
	ZEND_PARSE_PARAMETERS_NONE();
	
#ifdef ZTS
	thread = (async_thread *) Z_OBJ_P(getThis());
	
	if (!(thread->flags & ASYNC_THREAD_FLAG_TERMINATED)) {
		ASYNC_ALLOC_OP(op);
		ASYNC_APPEND_OP(&thread->join, op);
		
		if (UNEXPECTED(FAILURE == async_await_op(op))) {
			ASYNC_FORWARD_OP_ERROR(op);
		}
		
		ASYNC_FREE_OP(op);
	}
	
	RETURN_LONG(thread->exit_code);
#endif
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_WAKEUP(Thread, async_thread_ce)
//LCOV_EXCL_STOP

static const zend_function_entry thread_functions[] = {
	PHP_ME(Thread, __construct, arginfo_thread_ctor, ZEND_ACC_PUBLIC)
	PHP_ME(Thread, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(Thread, isAvailable, arginfo_thread_is_available, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Thread, isWorker, arginfo_thread_is_worker, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Thread, connect, arginfo_thread_connect, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Thread, getIpc, arginfo_thread_get_ipc, ZEND_ACC_PUBLIC)
	PHP_ME(Thread, kill, arginfo_thread_kill, ZEND_ACC_PUBLIC)
	PHP_ME(Thread, join, arginfo_thread_join, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

#ifdef ZTS

static void interrupt_thread(zend_execute_data *exec)
{
	async_thread *thread;
	
	if (ASYNC_G(thread)) {
		thread = (async_thread *) ASYNC_G(thread);

		if (thread->flags & ASYNC_THREAD_FLAG_KILLED) {
			zend_bailout();
		}
	}
	
	if (prev_interrupt_handler) {
		prev_interrupt_handler(exec);
	}
}

#endif

void async_thread_ce_register()
{
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async", "Thread", thread_functions);
	async_thread_ce = zend_register_internal_class(&ce);
	async_thread_ce->ce_flags |= ZEND_ACC_FINAL;
	async_thread_ce->create_object = async_thread_object_create;
	async_thread_ce->serialize = zend_class_serialize_deny;
	async_thread_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_thread_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_thread_handlers.free_obj = async_thread_object_destroy;
	async_thread_handlers.dtor_obj = async_thread_object_dtor;
	async_thread_handlers.clone_obj = NULL;

#ifdef ZTS
	str_main = zend_new_interned_string(zend_string_init(ZEND_STRL("main"), 1));
	
	if (ASYNC_G(cli)) {
		sapi_deactivate_func = sapi_module.deactivate;
		sapi_module.deactivate = NULL;
		
		prev_interrupt_handler = zend_interrupt_function;
		zend_interrupt_function = interrupt_thread;
	}
#endif
}

void async_thread_ce_unregister()
{
#ifdef ZTS
	if (ASYNC_G(cli)) {
		zend_interrupt_function = prev_interrupt_handler;
		sapi_module.deactivate = sapi_deactivate_func;
	}

	zend_string_release(str_main);
#endif
}
