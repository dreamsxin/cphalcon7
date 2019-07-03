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

#ifndef PHALCON_ASYNC_CORE_H
#define PHALCON_ASYNC_CORE_H

#include "php_phalcon.h"
#include "kernel/memory.h"

#include <Zend/zend_inheritance.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef PHP_WIN32
#include <Winsock2.h>
#include <Mswsock.h>
#include <psapi.h>
#include <Iphlpapi.h>

#undef HAVE_ASYNC_SSL
#define HAVE_ASYNC_SSL 1

#endif

#ifdef HAVE_ASYNC_SSL
#include <openssl/opensslv.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#endif


#include "async/thirdparty/libuv/uv.h"
#include "php.h"

#ifdef PHP_WIN32
#define ASYNC_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#define ASYNC_API __attribute__ ((visibility("default")))
#else
#define ASYNC_API
#endif

#define ASYNC_CALLBACK static void

#ifdef ZTS
#include "TSRM.h"
#endif

#ifdef PHP_WIN32
typedef ULONG uv_buf_size_t;
#else
typedef size_t uv_buf_size_t;
#endif

#ifdef __GNUC__
#define ASYNC_VA_ARGS(...) , ##__VA_ARGS__
#else
#define ASYNC_VA_ARGS(...) , __VA_ARGS__
#endif

#define ASYNC_STR(target, message) target = zend_string_init(message, sizeof(message)-1, 0)
#define ASYNC_STRP(target, message) target = zend_string_init(message, sizeof(message)-1, 1)

#define ASYNC_STRF(target, message, ...) do { \
	char *_buf; \
	int _len; \
	_len = ap_php_asprintf(&_buf, message ASYNC_VA_ARGS(__VA_ARGS__)); \
	target = zend_string_init(_buf, _len, 0); \
	free(_buf); \
} while (0)

#include "zend.h"
#include "zend_API.h"
#include "zend_vm.h"
#include "zend_interfaces.h"
#include "zend_exceptions.h"
#include "zend_closures.h"

#ifdef PHP_WIN32
#include "win32/winutil.h"
#endif

#include "php_network.h"
#include "php_streams.h"

#if defined(HAVE_SOCKETS) && !defined(COMPILE_DL_SOCKETS)
#define ASYNC_SOCKETS 1
#include "ext/sockets/php_sockets.h"
#elif !defined(PHP_WIN32)
#define ASYNC_SOCKETS 0
typedef struct _php_socket {
	int bsd_socket;
} php_socket;
#else
#define ASYNC_SOCKETS 0
#endif

#define ASYNC_OP_PENDING 0
#define ASYNC_OP_RESOLVED 64
#define ASYNC_OP_FAILED 65

#define ASYNC_SIGNAL_SIGHUP 1
#define ASYNC_SIGNAL_SIGINT 2

#ifdef PHP_WIN32
#define ASYNC_SIGNAL_SIGQUIT -1
#define ASYNC_SIGNAL_SIGKILL -2
#define ASYNC_SIGNAL_SIGTERM -3
#else
#define ASYNC_SIGNAL_SIGQUIT 3
#define ASYNC_SIGNAL_SIGKILL 9
#define ASYNC_SIGNAL_SIGTERM 15
#endif

#ifdef SIGUSR1
#define ASYNC_SIGNAL_SIGUSR1 SIGUSR1
#else
#define ASYNC_SIGNAL_SIGUSR1 -4
#endif

#ifdef SIGUSR2
#define ASYNC_SIGNAL_SIGUSR2 SIGUSR2
#else
#define ASYNC_SIGNAL_SIGUSR2 -5
#endif

ASYNC_API extern zend_class_entry *async_awaitable_ce;
ASYNC_API extern zend_class_entry *async_awaitable_impl_ce;
ASYNC_API extern zend_class_entry *async_cancellation_exception_ce;
ASYNC_API extern zend_class_entry *async_cancellation_handler_ce;
ASYNC_API extern zend_class_entry *async_channel_ce;
ASYNC_API extern zend_class_entry *async_channel_closed_exception_ce;
ASYNC_API extern zend_class_entry *async_channel_group_ce;
ASYNC_API extern zend_class_entry *async_channel_iterator_ce;
ASYNC_API extern zend_class_entry *async_channel_select_ce;
ASYNC_API extern zend_class_entry *async_component_ce;
ASYNC_API extern zend_class_entry *async_context_ce;
ASYNC_API extern zend_class_entry *async_context_var_ce;
ASYNC_API extern zend_class_entry *async_deferred_ce;
ASYNC_API extern zend_class_entry *async_deferred_awaitable_ce;
ASYNC_API extern zend_class_entry *async_dns_config_ce;
ASYNC_API extern zend_class_entry *async_dns_query_ce;
ASYNC_API extern zend_class_entry *async_dns_resolver_ce;
ASYNC_API extern zend_class_entry *async_duplex_stream_ce;
ASYNC_API extern zend_class_entry *async_job_failed_ce;
ASYNC_API extern zend_class_entry *async_monitor_ce;
ASYNC_API extern zend_class_entry *async_monitor_event_ce;
ASYNC_API extern zend_class_entry *async_pending_read_exception_ce;
ASYNC_API extern zend_class_entry *async_pipe_ce;
ASYNC_API extern zend_class_entry *async_pipe_server_ce;
ASYNC_API extern zend_class_entry *async_poll_ce;
ASYNC_API extern zend_class_entry *async_poll_event_ce;
ASYNC_API extern zend_class_entry *async_process_builder_ce;
ASYNC_API extern zend_class_entry *async_process_ce;
ASYNC_API extern zend_class_entry *async_readable_console_stream_ce;
ASYNC_API extern zend_class_entry *async_readable_pipe_ce;
ASYNC_API extern zend_class_entry *async_readable_process_pipe_ce;
ASYNC_API extern zend_class_entry *async_readable_memory_stream_ce;
ASYNC_API extern zend_class_entry *async_readable_stream_ce;
ASYNC_API extern zend_class_entry *async_server_ce;
ASYNC_API extern zend_class_entry *async_socket_ce;
ASYNC_API extern zend_class_entry *async_socket_accept_exception_ce;
ASYNC_API extern zend_class_entry *async_socket_bind_exception_ce;
ASYNC_API extern zend_class_entry *async_socket_connect_exception_ce;
ASYNC_API extern zend_class_entry *async_socket_disconnect_exception_ce;
ASYNC_API extern zend_class_entry *async_socket_exception_ce;
ASYNC_API extern zend_class_entry *async_socket_listen_exception_ce;
ASYNC_API extern zend_class_entry *async_socket_stream_ce;
ASYNC_API extern zend_class_entry *async_stream_closed_exception_ce;
ASYNC_API extern zend_class_entry *async_stream_exception_ce;
ASYNC_API extern zend_class_entry *async_stream_reader_ce;
ASYNC_API extern zend_class_entry *async_stream_writer_ce;
ASYNC_API extern zend_class_entry *async_signal_ce;
ASYNC_API extern zend_class_entry *async_sync_condition_ce;
ASYNC_API extern zend_class_entry *async_task_ce;
ASYNC_API extern zend_class_entry *async_task_scheduler_ce;
ASYNC_API extern zend_class_entry *async_tcp_server_ce;
ASYNC_API extern zend_class_entry *async_tcp_socket_ce;
ASYNC_API extern zend_class_entry *async_thread_pool_ce;
ASYNC_API extern zend_class_entry *async_tls_client_encryption_ce;
ASYNC_API extern zend_class_entry *async_tls_info_ce;
ASYNC_API extern zend_class_entry *async_tls_server_encryption_ce;
ASYNC_API extern zend_class_entry *async_tick_event_ce;
ASYNC_API extern zend_class_entry *async_timeout_ce;
ASYNC_API extern zend_class_entry *async_timeout_exception_ce;
ASYNC_API extern zend_class_entry *async_timer_ce;
ASYNC_API extern zend_class_entry *async_timer_event_ce;
ASYNC_API extern zend_class_entry *async_udp_datagram_ce;
ASYNC_API extern zend_class_entry *async_udp_socket_ce;
ASYNC_API extern zend_class_entry *async_writable_console_stream_ce;
ASYNC_API extern zend_class_entry *async_writable_pipe_ce;
ASYNC_API extern zend_class_entry *async_writable_process_pipe_ce;
ASYNC_API extern zend_class_entry *async_writable_memory_stream_ce;
ASYNC_API extern zend_class_entry *async_writable_stream_ce;

typedef struct _async_cancel_cb                     async_cancel_cb;
typedef struct _async_cancellation_handler          async_cancellation_handler;
typedef struct _async_context                       async_context;
typedef struct _async_context_cancellation          async_context_cancellation;
typedef struct _async_context_timeout               async_context_timeout;
typedef struct _async_context_var                   async_context_var;
typedef struct _async_fiber                         async_fiber;
typedef struct _async_op                            async_op;
typedef struct _async_task                          async_task;
typedef struct _async_task_scheduler                async_task_scheduler;
typedef struct _async_tick_event                    async_tick_event;

#define ASYNC_FIBER_FLAG_QUEUED 1

typedef struct _async_op_list {
	async_op *first;
	async_op *last;
} async_op_list;

typedef struct _async_tick_list {
	async_tick_event *first;
	async_tick_event *last;
} async_tick_list;

struct _async_cancel_cb {
	/* Struct being passed to callback as first arg. */
	void *object;
	
	/* Pointer to cancel function. */
	void (* func)(void *obj, zval *error);
	
	/* List pointers. */
	async_cancel_cb *prev;
	async_cancel_cb *next;
};

#define ASYNC_OP_FLAG_CANCELLED 1
#define ASYNC_OP_FLAG_DEFER 2
#define ASYNC_OP_FLAG_ATOMIC 4

typedef enum _async_status {
	ASYNC_STATUS_PENDING,
	ASYNC_STATUS_RUNNING,
	ASYNC_STATUS_RESOLVED,
	ASYNC_STATUS_FAILED
} async_status;

struct _async_op {
	/* One of the ASYNC_STATUS_ constants. */
	async_status status;
	
	/* Callback being used to continue the suspended execution. */
	void (* callback)(async_op *op);
	
	/* Callback being used to mark the operation as cancelled and continue the suspended execution. */
	async_cancel_cb cancel;
	
	/* Opaque pointer that can be used to pass data to the continuation callback. */
	void *arg;
	
	/* Result variable, will hold an error object if an operation fails or is cancelled. */
	zval result;
	
	/* Combined ASYNC_OP flags. */
	uint8_t flags;
	
	/* Refers to an operation list if the operation is queued for execution. */
	async_op_list *list;
	async_op *next;
	async_op *prev;
};

typedef struct _async_uv_op {
	/* Async operation structure, must be first element to allow for casting to async_op. */
	async_op base;
	
	/* Result status code provided by libuv. */
	int code;
} async_uv_op;

typedef struct _async_awaitable_impl {
	uint8_t status;
	zval result;
	async_op_list operations;

	void *arg;
	zend_object std;
} async_awaitable_impl;

typedef void (* async_interceptor)(async_context *context, zend_object *obj, INTERNAL_FUNCTION_PARAMETERS);

#define ASYNC_AWAIT_HANDLER_INLINE 1

typedef struct _async_await_handler {
	/* Must populate result with a zval copy if ASYNC_OP_RESOLVED or ASYNC_OP_FAILED is returned. */
	int (* delegate)(zend_object *obj, zval *result, async_task *caller, async_context *context, int flags);

	/* Enqueue the given op to await completion. */
	void (* schedule)(zend_object *obj, async_op *op, async_task *caller, async_context *context, int flags);
} async_await_handler;

struct _async_cancellation_handler {
	/* PHP object handle. */
	zend_object std;
	
	/* Refers to a contextual cancellation instance. */
	async_context_cancellation *cancel;
};

#define ASYNC_CONTEXT_FLAG_BACKGROUND 1

struct _async_context {
	/* PHP object handle. */
	zend_object std;

	/* Refers to the parent context. */
	async_context *parent;

	/* Context flags. */
	uint8_t flags;

	/* Context var or NULL. */
	async_context_var *var;

	/* Value of the context var, defaults to zval NULL. */
	zval value;

	/* Refers to the contextual cancellation handler. */
	async_context_cancellation *cancel;
	
	struct {
		async_context *context;
		zend_output_globals *handler;
	} output;
};

#define ASYNC_CONTEXT_CANCELLATION_FLAG_TRIGGERED 1
#define ASYNC_CONTEXT_CANCELLATION_FLAG_TIMEOUT 2

struct _async_context_cancellation {
	/* Internal refcount shared by context and cancellation components. */
	uint32_t refcount;
	
	/* Cancellation flags. */
	uint8_t flags;

	/* Error that caused cancellation, UNDEF by default. */
	zval error;

	/* Chain handler that connects the cancel handler to the parent handler. */
	async_cancel_cb chain;

	/* Linked list of cancellation callbacks. */
	struct {
		async_cancel_cb *first;
		async_cancel_cb *last;
	} callbacks;
};

struct _async_context_timeout {
	async_context_cancellation base;

	/* Task scheduler instance (only != NULL if timeout is active). */
	async_task_scheduler *scheduler;

	/* Timeout instance (unreferenced). */
	uv_timer_t timer;
};

struct _async_context_var {
	/* PHP object handle. */
	zend_object std;
};

#define ASYNC_TASK_FLAG_DISPOSED 1
#define ASYNC_TASK_FLAG_NOWAIT (1 << 1)
#define ASYNC_TASK_FLAG_ROOT (1 << 2)

struct _async_task {
	/* Underlying fiber being used to run the task. */
	async_fiber *fiber;
	
	/* Associated task scheduler. */
	async_task_scheduler *scheduler;
	
	/* Task flags. */
	uint8_t flags;

	/* Async operation status. */
	zend_uchar status;

	/* Fiber C stack size. */
	zend_long stack_size;

	/* PHP callback to be run as task. */
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	
	/* Async execution context provided to the task. */
	async_context *context;

	/* Return value of the task, may also be an error object, check status for outcome. */
	zval result;
	
	/* Queued operations waiting for task completion. */
	async_op_list operations;
	
	/* Current await operation. */
	async_op op;
	
	/* Pointers related to the scheduler's task ready queue. */
	async_task *prev;
	async_task *next;

	/* PHP object handle. */
	zend_object std;
};

#define ASYNC_TASK_SCHEDULER_FLAG_RUNNING 1
#define ASYNC_TASK_SCHEDULER_FLAG_DISPOSED (1 << 1)
#define ASYNC_TASK_SCHEDULER_FLAG_NOWAIT (1 << 2)
#define ASYNC_TASK_SCHEDULER_FLAG_ERROR (1 << 3)
#define ASYNC_TASK_SCHEDULER_FLAG_ACTIVE (1 << 4)

struct _async_task_scheduler {
	/* PHP object handle. */
	zend_object std;
	
	/* Flags being used to control scheduler state. */
	uint16_t flags;

	/* Ready to run fibers. */
	struct {
		async_fiber *first;
		async_fiber *last;
	} fibers;
	
	/* Fiber runnung the event loop. */
	async_fiber *runner;
	
	/* Fiber that caused the scheduler to run. */
	async_fiber *caller;

	/* Tasks ready to be started or resumed. */
	struct {
		async_task *first;
		async_task *last;
	} ready;
	
	/* Pending operations that have not completed yet. */
	async_op_list operations;
	
	/* Root level awaited operation. */
	async_op op;
	
	/** Queue of shutdown callbacks that need to be executed when the scheduler is disposed. */
	struct {
		async_cancel_cb *first;
		async_cancel_cb *last;
	} shutdown;

	/* Libuv event loop. */
	uv_loop_t loop;

	/* Idle handler being used to dispatch tasks from within a running event loop. */
	uv_idle_t idle;
	
	/* Timer being used to keep the loop busy when needed. */
	uv_timer_t busy;
	zend_ulong busy_count;

	/* Instantiated scheduler-scoped objects created by factories. */
	HashTable components;

	/* Tick callbacks scheduled for next tick and running now. */
	async_tick_list ticks;
	async_tick_list running;
	uint32_t refticks;

	/* Holds the error that caused the scheduler to exit (UNDEF by default). */
	zval error;

	/* Embedded pseudo task that is used to schedule the root execution in deferred op mode. */
	struct {
		async_fiber *fiber;
		async_task_scheduler *scheduler;
		uint8_t flags;
		zend_uchar status;
	} root;
};

#define ASYNC_DEBUG_LOG(message, ...)
#define ASYNC_ADDREF(obj) GC_ADDREF(obj)
#define ASYNC_DELREF(obj) OBJ_RELEASE(obj)

#define ASYNC_ADDREF_CB(fci) do { \
	Z_TRY_ADDREF((fci).function_name); \
	if ((fci).object) { \
		ASYNC_ADDREF((fci).object); \
	} \
} while (0)

#define ASYNC_DELREF_CB(fci) do { \
	if ((fci).object) { \
		ASYNC_DELREF((fci).object); \
	} \
	zval_ptr_dtor(&(fci).function_name); \
} while (0)

static zend_always_inline async_task_scheduler *async_task_scheduler_ref()
{
	async_task_scheduler *scheduler;
	
	scheduler = ASYNC_G(scheduler);
	ASYNC_ADDREF(&scheduler->std);
	
	return scheduler;
}

#define async_task_scheduler_get() ASYNC_G(scheduler)
#define async_task_scheduler_unref(scheduler) ASYNC_DELREF(&(scheduler)->std)

static zend_always_inline async_context *async_context_ref()
{
	async_context *context;
	
	context = ASYNC_G(context);
	ASYNC_ADDREF(&context->std);
	
	return context;
}

#define async_context_get() ASYNC_G(context)
#define async_context_unref(context) ASYNC_DELREF(&(context)->std)
#define async_context_is_background(context) (context->flags & ASYNC_CONTEXT_FLAG_BACKGROUND)

#define async_loop_get() &ASYNC_G(scheduler)->loop

ASYNC_API void async_task_scheduler_run(async_task_scheduler *scheduler, zend_execute_data *execute_data);
ASYNC_API int async_await_op(async_op *op);

ASYNC_API void async_task_scheduler_handle_exit(async_task_scheduler *scheduler);
ASYNC_API void async_task_scheduler_handle_error(async_task_scheduler *scheduler, zend_object *error);

ASYNC_API void async_prepare_throwable(zval *error, zend_execute_data *exec, zend_class_entry *ce, const char *message, ...);
ASYNC_API int async_call_nowait(zend_execute_data *exec, zend_fcall_info *fci, zend_fcall_info_cache *fcc);

ASYNC_API async_awaitable_impl *async_create_awaitable(zend_execute_data *call, void *arg);

ASYNC_API async_awaitable_impl *async_create_resolved_awaitable(zend_execute_data *call, zval *result);
ASYNC_API async_awaitable_impl *async_create_failed_awaitable(zend_execute_data *call, zval *error);

ASYNC_API void async_awaitable_resolve(async_awaitable_impl *awaitable, zval *result);
ASYNC_API void async_awaitable_fail(async_awaitable_impl *awaitable, zval *error);

ASYNC_API void async_register_interceptor(zend_function *func, async_interceptor interceptor);

ASYNC_API void async_register_awaitable(zend_class_entry *ce, async_await_handler *handler);
ASYNC_API async_await_handler *async_get_await_handler(zend_class_entry *ce);

ASYNC_API zval *async_get_component(async_task_scheduler *scheduler, zend_string *type, zend_execute_data *exec);

ASYNC_API int async_dns_lookup_ip(char *name, php_sockaddr_storage *dest, int proto);

#define ASYNC_ALLOC_OP(op) do { \
	op = ecalloc(1, sizeof(async_op)); \
} while (0)

#define ASYNC_ALLOC_CUSTOM_OP(op, size) do { \
	op = ecalloc(1, size); \
} while (0)

#define ASYNC_FINISH_OP(op) do { \
	async_op *tmp; \
	tmp = (async_op *) op; \
	if (UNEXPECTED(tmp->list != NULL)) { \
		ASYNC_LIST_REMOVE(tmp->list, tmp); \
		tmp->list = NULL; \
	} \
	tmp->status = ASYNC_STATUS_RESOLVED; \
	if (EXPECTED(tmp->callback)) { \
		tmp->callback(tmp); \
	} \
} while (0)

#define ASYNC_RESOLVE_OP(op, val) do { \
	async_op *tmp; \
	tmp = (async_op *) op; \
	if (UNEXPECTED(tmp->list != NULL)) { \
		ASYNC_LIST_REMOVE(tmp->list, tmp); \
		tmp->list = NULL; \
	} \
	ZVAL_COPY(&tmp->result, val); \
	tmp->status = ASYNC_STATUS_RESOLVED; \
	if (EXPECTED(tmp->callback)) { \
		tmp->callback(tmp); \
	} \
} while (0)

#define ASYNC_FAIL_OP(op, error) do { \
	async_op *tmp; \
	tmp = (async_op *) op; \
	if (UNEXPECTED(tmp->list != NULL)) { \
		ASYNC_LIST_REMOVE(tmp->list, tmp); \
		tmp->list = NULL; \
	} \
	ZVAL_COPY(&tmp->result, error); \
	tmp->status = ASYNC_STATUS_FAILED; \
	if (EXPECTED(tmp->callback)) { \
		tmp->callback(tmp); \
	} \
} while (0)

#define ASYNC_APPEND_OP(operations, op) do { \
	async_op *tmp; \
	tmp = (async_op *) op; \
	tmp->list = operations; \
	ASYNC_LIST_APPEND(operations, tmp); \
} while (0)

#define ASYNC_PREPEND_OP(operations, op) do { \
	async_op *tmp; \
	tmp = (async_op *) op; \
	tmp->list = operations; \
	ASYNC_LIST_PREPEND(operations, tmp); \
} while (0)

#define ASYNC_NEXT_OP(operations, op) do { \
	ASYNC_LIST_EXTRACT_FIRST(operations, op); \
	op->list = NULL; \
} while (0)

#define ASYNC_NEXT_CUSTOM_OP(operations, op, type) do { \
	async_op *tmp; \
	ASYNC_LIST_EXTRACT_FIRST(operations, tmp); \
	tmp->list = NULL; \
	op = (type *) tmp; \
} while (0)

#define ASYNC_RESET_OP(op) do { \
	async_op *tmp; \
	tmp = (async_op *) op; \
	tmp->status = ASYNC_STATUS_PENDING; \
	tmp->flags = 0; \
	zval_ptr_dtor(&tmp->result); \
	ZVAL_UNDEF(&tmp->result); \
	if (UNEXPECTED(tmp->list != NULL)) { \
		ASYNC_LIST_REMOVE(tmp->list, tmp); \
		tmp->list = NULL; \
	} \
} while (0)

#define ASYNC_FREE_OP(op) do { \
	async_op *tmp; \
	tmp = (async_op *) op; \
	if (UNEXPECTED(tmp->list != NULL)) { \
		ASYNC_LIST_REMOVE(tmp->list, tmp); \
		tmp->list = NULL; \
	} \
	zval_ptr_dtor(&tmp->result); \
	efree(op); \
} while (0)

#define ASYNC_FORWARD_OP_ERROR(op) do { \
	Z_ADDREF_P(&((async_op *) op)->result); \
	EG(current_execute_data)->opline--; \
	zend_throw_exception_internal(&((async_op *) op)->result); \
	EG(current_execute_data)->opline++; \
} while (0)

#define ASYNC_FORWARD_ERROR(error) do { \
	Z_ADDREF_P(error); \
	execute_data->opline--; \
	zend_throw_exception_internal(error); \
	execute_data->opline++; \
} while (0)

#define ASYNC_FORWARD_EXIT() do { \
	if (UNEXPECTED(ASYNC_G(exit))) { \
		zend_bailout(); \
	} \
} while (0)

#define ASYNC_UV_CLOSE(handle, callback) do { \
	/*ASYNC_DEBUG_LOG("CLOSE %s: %s:%d\n", uv_handle_type_name(uv_handle_get_type((uv_handle_t *)(handle))), __FILE__, __LINE__);*/ \
	uv_close((uv_handle_t *)(handle), (callback)); \
} while (0)

#define ASYNC_UV_CLOSE_REF(ref, handle, callback) do { \
	ASYNC_ADDREF(ref); \
	ASYNC_UV_CLOSE(handle, callback); \
} while (0)

#define ASYNC_UV_TRY_CLOSE(handle, callback) do { \
	if (!uv_is_closing((uv_handle_t *)(handle))) { \
		ASYNC_UV_CLOSE(handle, callback); \
	} \
} while (0)

#define ASYNC_UV_TRY_CLOSE_REF(ref, handle, callback) do { \
	if (!uv_is_closing((uv_handle_t *)(handle))) { \
		ASYNC_ADDREF(ref); \
		ASYNC_UV_CLOSE(handle, callback); \
	} \
} while (0)

#define ASYNC_UNREF_ENTER(ctx, obj) do { \
	if (!async_context_is_background(ctx)) { \
		if (++(obj)->ref_count == 1) { \
			uv_ref((uv_handle_t *) &(obj)->handle); \
		} \
	} \
} while (0)

#define ASYNC_UNREF_EXIT(ctx, obj) do { \
	if (!async_context_is_background(ctx)) { \
		if (--(obj)->ref_count == 0) { \
			uv_unref((uv_handle_t *) &(obj)->handle); \
		} \
	} \
} while (0)

#define ASYNC_BUSY_ENTER(scheduler) do { \
	if (++(scheduler)->busy_count == 1) { \
		uv_ref((uv_handle_t *) &(scheduler)->busy); \
	} \
} while(0)

#define ASYNC_BUSY_EXIT(scheduler) do { \
	if (--(scheduler)->busy_count == 0) { \
		uv_unref((uv_handle_t *) &(scheduler)->busy); \
	} \
} while(0)

#define ASYNC_PREPARE_ERROR(error, exec, message, ...) async_prepare_throwable(error, exec, zend_ce_error, message ASYNC_VA_ARGS(__VA_ARGS__))
#define ASYNC_PREPARE_EXCEPTION(error, exec, ce, message, ...) async_prepare_throwable(error, exec, ce, message ASYNC_VA_ARGS(__VA_ARGS__))

#define ASYNC_PREPARE_SCHEDULER_ERROR(error, message, ...) async_prepare_throwable(error, NULL, zend_ce_error, message ASYNC_VA_ARGS(__VA_ARGS__))
#define ASYNC_PREPARE_SCHEDULER_EXCEPTION(error, ce, message, ...) async_prepare_throwable(error, NULL, ce, message ASYNC_VA_ARGS(__VA_ARGS__))

#define ASYNC_CHECK_ERROR(expr, message, ...) do { \
    if (UNEXPECTED(expr)) { \
    	zend_throw_error(NULL, message ASYNC_VA_ARGS(__VA_ARGS__)); \
    	return; \
    } \
} while (0)

#define ASYNC_CHECK_EXCEPTION(expr, ce, message, ...) do { \
    if (UNEXPECTED(expr)) { \
    	zend_throw_exception_ex(ce, 0, message ASYNC_VA_ARGS(__VA_ARGS__)); \
    	return; \
    } \
} while (0)

#define ASYNC_CHECK_FATAL(expr, message, ...) do { \
	if (UNEXPECTED(expr)) { \
		printf(message ASYNC_VA_ARGS(__VA_ARGS__)); \
		zend_error_noreturn(E_CORE_ERROR, message ASYNC_VA_ARGS(__VA_ARGS__)); \
	} \
} while (0)

#define ASYNC_ENSURE_ERROR(message, ...) do { \
    if (UNEXPECTED(!EG(exception))) { \
    	zend_throw_error(NULL, message ASYNC_VA_ARGS(__VA_ARGS__)); \
    } \
} while (0)

#define ASYNC_RETURN_ON_ERROR() do { \
	if (UNEXPECTED(EG(exception))) { \
		return; \
	} \
} while (0)

#define ASYNC_BREAK_ON_ERROR() do { \
	if (UNEXPECTED(EG(exception))) { \
		break; \
	} \
} while (0)

#define ASYNC_CONTINUE_ON_ERROR() do { \
	if (UNEXPECTED(EG(exception))) { \
		continue; \
	} \
} while (0)

ZEND_BEGIN_ARG_INFO(arginfo_no_ctor, 0)
ZEND_END_ARG_INFO();

#define ASYNC_METHOD_NO_CTOR(type, ce) static PHP_METHOD(type, __construct) { \
	ZEND_PARSE_PARAMETERS_NONE(); \
	zend_throw_error(NULL, "Construction %s is not allowed", ZSTR_VAL(ce->name)); \
}

ZEND_BEGIN_ARG_INFO(arginfo_no_wakeup, 0)
ZEND_END_ARG_INFO();

#define ASYNC_METHOD_NO_WAKEUP(type, ce) static PHP_METHOD(type, __wakeup) { \
	ZEND_PARSE_PARAMETERS_NONE(); \
	zend_throw_error(NULL, "Unserialization of %s is not allowed", ZSTR_VAL(ce->name)); \
}

/*
 * List macros require a "q" pointer with fields "first" and "last" of same pointer type as "v".
 * The "v" pointer must have fields "prev" and "next" of the same pointer type as "v".
 */

#define ASYNC_LIST_APPEND(q, v) do { \
	(v)->next = NULL; \
	if ((q)->last == NULL) { \
		(v)->prev = NULL; \
		(q)->first = v; \
		(q)->last = v; \
	} else { \
		(v)->prev = (q)->last; \
		(q)->last->next = v; \
		(q)->last = v; \
	} \
} while (0)

#define ASYNC_LIST_PREPEND(q, v) do { \
	(v)->prev = NULL; \
	if ((q)->first == NULL) { \
		(v)->next = NULL; \
		(q)->first = v; \
		(q)->last = v; \
	} else { \
		(v)->next = (q)->first; \
		(q)->first->prev = v; \
		(q)->first = v; \
	} \
} while (0)

#define ASYNC_LIST_EXTRACT_FIRST(q, v) do { \
	if ((q)->first == NULL) { \
		v = NULL; \
	} else { \
		v = (q)->first; \
		(q)->first = (v)->next; \
		if ((q)->first != NULL) { \
			(q)->first->prev = NULL; \
		} \
		if ((q)->last == v) { \
			(q)->last = NULL; \
		} \
		(v)->prev = NULL; \
		(v)->next = NULL; \
	} \
} while (0)

#define ASYNC_LIST_EXTRACT_LAST(q, v) do { \
	if ((q)->last == NULL) { \
		v = NULL; \
	} else { \
		v = (q)->last; \
		(q)->last = (v)->prev; \
		if ((q)->last != NULL) { \
			(q)->last->next = NULL; \
		} \
		if ((q)->first == v) { \
			(q)->first = NULL; \
		} \
		(v)->prev = NULL; \
		(v)->next = NULL; \
	} \
} while (0)

#define ASYNC_LIST_REMOVE(q, v) do { \
	if ((v)->prev != NULL) { \
		(v)->prev->next = (v)->next; \
	} \
	if ((v)->next != NULL) { \
		(v)->next->prev = (v)->prev; \
	} \
	if ((q)->first == v) { \
		(q)->first = (v)->next; \
	} \
	if ((q)->last == v) { \
		(q)->last = (v)->prev; \
	} \
	(v)->prev = NULL; \
	(v)->next = NULL; \
} while (0)

ZEND_API void async_execute_ex(zend_execute_data *exec);

#endif


/* PHP 7.3 */
#if PHP_VERSION_ID < 70200
/* zend_internal_function_handler */
typedef void (ZEND_FASTCALL *zif_handler)(INTERNAL_FUNCTION_PARAMETERS);
#endif

#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() 
#endif

#ifndef ZEND_PROPERTY_EXISTS
#define ZEND_PROPERTY_ISSET     0x0          /* Property exists and is not NULL */
#define ZEND_PROPERTY_NOT_EMPTY ZEND_ISEMPTY /* Property is not empty */
#define ZEND_PROPERTY_EXISTS    0x2          /* Property exists */
#endif
