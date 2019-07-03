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
#include "async/async_event.h"
#include "kernel/backend.h"

ASYNC_API zend_class_entry *async_poll_event_ce;
ASYNC_API zend_class_entry *async_tick_event_ce;
ASYNC_API zend_class_entry *async_timer_event_ce;

static zend_object_handlers async_poll_event_handlers;
static zend_object_handlers async_tick_event_handlers;
static zend_object_handlers async_timer_event_handlers;

#define ASYNC_POLL_EVENT_CONST(const_name, value) \
	zend_declare_class_constant_long(async_poll_event_ce, const_name, sizeof(const_name) - 1, (zend_long) value);

#define INIT_CALLBACK(val, event) do { \
	char *_err; \
	if (UNEXPECTED(!zend_is_callable_ex(val, NULL, 0, NULL, &(event)->fcc, &_err))) { \
		zend_throw_error(NULL, "Callable required: %s", _err); \
		efree(_err); \
		efree(event); \
		return NULL; \
	} \
} while (0)


ASYNC_CALLBACK close_poll(uv_handle_t *handle)
{
	async_poll_event *event;

	event = (async_poll_event *) handle->data;

	ZEND_ASSERT(event != NULL);

	ASYNC_DELREF(&event->std);
}

ASYNC_CALLBACK shutdown_poll(void *object, zval *error)
{
	async_poll_event *event;

	event = (async_poll_event *) object;

	event->shutdown.func = NULL;

	ASYNC_UV_TRY_CLOSE_REF(&event->std, &event->handle, close_poll);

	if (event->flags & ASYNC_POLL_EVENT_FLAG_SCHEDULED) {
		event->flags &= ~ASYNC_POLL_EVENT_FLAG_SCHEDULED;

		ASYNC_DELREF(&event->std);
	}
}

ASYNC_CALLBACK poll_cb(uv_poll_t *handle, int status, int events)
{
	async_poll_event *event;

	zend_fcall_info fci;

	zval retval;
	zval args[3];

	int code;

	event = (async_poll_event *) handle->data;

	ZEND_ASSERT(event != NULL);
	ZEND_ASSERT(event->flags & ASYNC_POLL_EVENT_FLAG_SCHEDULED);

	if (UNEXPECTED(status < 0)) {
		if (status != UV_ECANCELED) {
			zend_throw_error(NULL, "Failed to poll: %s", uv_strerror(status));

			async_task_scheduler_handle_error(event->scheduler, EG(exception));
			zend_clear_exception();
		}

		return;
	}

	ZVAL_LONG(&args[0], events);
	ZVAL_COPY(&args[1], &event->stream);
	ZVAL_OBJ(&args[2], &event->std);

	ASYNC_ADDREF(&event->std);

	fci.size = sizeof(zend_fcall_info);
	fci.object = event->fcc.object;
	fci.no_separation = 1;

	ZVAL_COPY_VALUE(&fci.function_name, &event->callback);

	fci.retval = &retval;
	fci.param_count = 3;
	fci.params = args;

	zend_try {
		code = async_call_nowait(ASYNC_G(exec), &fci, &event->fcc);
	} zend_catch {
		async_task_scheduler_handle_exit(event->scheduler);
		ASYNC_DELREF(&event->std);
		return;
	} zend_end_try();

	if (UNEXPECTED(code == FAILURE)) {
		ASYNC_ENSURE_ERROR("Failed to invoke poll callback");

		async_task_scheduler_handle_error(event->scheduler, EG(exception));
		zend_clear_exception();
	} else {
		zval_ptr_dtor(&retval);
	}

	ASYNC_DELREF(&event->std);
}

async_poll_event *async_poll_event_object_create(async_task_scheduler *scheduler, zval *callback, zval *val, php_socket_t fd)
{
	async_poll_event *event;

	event = ecalloc(1, sizeof(async_poll_event));

	INIT_CALLBACK(callback, event);

	zend_object_std_init(&event->std, async_poll_event_ce);
	event->std.handlers = &async_poll_event_handlers;

	event->scheduler = async_task_scheduler_ref();
	event->context = ASYNC_G(foreground);

	ASYNC_ADDREF(&event->context->std);

	ZVAL_COPY(&event->callback, callback);
	ZVAL_COPY(&event->stream, val);

#ifdef PHP_WIN32
	uv_poll_init_socket(&event->scheduler->loop, &event->handle, (uv_os_sock_t) fd);
#else
	uv_poll_init(&event->scheduler->loop, &event->handle, (int) fd);
#endif

	event->handle.data = event;

	event->shutdown.object = event;
	event->shutdown.func = shutdown_poll;

	ASYNC_LIST_APPEND(&event->scheduler->shutdown, &event->shutdown);

	return event;
}

static void async_poll_event_object_dtor(zend_object *object)
{
	async_poll_event *event;

	event = (async_poll_event *) object;

	if (event->shutdown.func) {
		ASYNC_LIST_REMOVE(&event->scheduler->shutdown, &event->shutdown);

		event->shutdown.func(event, NULL);
	}
}

static void async_poll_event_object_destroy(zend_object *object)
{
	async_poll_event *event;

	event = (async_poll_event *) object;

	zval_ptr_dtor(&event->callback);
	zval_ptr_dtor(&event->stream);

	async_task_scheduler_unref(event->scheduler);
	async_context_unref(event->context);

	zend_object_std_dtor(&event->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_poll_event_close, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(PollEvent, close)
{
	async_poll_event *event;

	ZEND_PARSE_PARAMETERS_NONE();

	event = (async_poll_event *) Z_OBJ_P(getThis());

	if (EXPECTED(!(event->flags & ASYNC_POLL_EVENT_FLAG_CLOSED))) {
		event->flags |= ASYNC_POLL_EVENT_FLAG_CLOSED;

		if (event->shutdown.func) {
			ASYNC_LIST_REMOVE(&event->scheduler->shutdown, &event->shutdown);

			event->shutdown.func(event, NULL);
		}
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_poll_event_start, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(PollEvent, start)
{
	async_poll_event *event;

	zend_long mode;
	int events;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_LONG(mode)
	ZEND_PARSE_PARAMETERS_END();

	ASYNC_CHECK_ERROR(mode & ~ASYNC_POLL_EVENT_MASK, "Invalid mode, use class constants only (binary or them as needed)");

	events = UV_DISCONNECT;

	if (mode & ASYNC_POLL_EVENT_FLAG_READABLE) {
		events |= UV_READABLE;
	}

	if (mode & ASYNC_POLL_EVENT_FLAG_WRITABLE) {
		events |= UV_WRITABLE;
	}

	ASYNC_CHECK_ERROR(events == UV_DISCONNECT, "You have to specify at least one of READABLE or WRITABLE as mode");

	event = (async_poll_event *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(event->flags & ASYNC_POLL_EVENT_FLAG_CLOSED, "Poll has been closed");

	event->flags = (event->flags & ~ASYNC_POLL_EVENT_MASK) | (((uint8_t) mode) & ASYNC_POLL_EVENT_MASK);

	uv_poll_start(&event->handle, events, poll_cb);

	if (!(event->flags & ASYNC_POLL_EVENT_FLAG_SCHEDULED)) {
		event->flags |= ASYNC_POLL_EVENT_FLAG_SCHEDULED;

		ASYNC_ADDREF(&event->std);
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_poll_event_stop, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(PollEvent, stop)
{
	async_poll_event *event;

	ZEND_PARSE_PARAMETERS_NONE();

	event = (async_poll_event *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(event->flags & ASYNC_POLL_EVENT_FLAG_CLOSED, "Poll has been closed");

	uv_poll_stop(&event->handle);

	if (event->flags & ASYNC_POLL_EVENT_FLAG_SCHEDULED) {
		event->flags &= ~ASYNC_POLL_EVENT_FLAG_SCHEDULED;

		ASYNC_DELREF(&event->std);
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_poll_event_ref, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(PollEvent, ref)
{
	async_poll_event *event;

	ZEND_PARSE_PARAMETERS_NONE();

	event = (async_poll_event *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(event->flags & ASYNC_POLL_EVENT_FLAG_CLOSED, "Poll has been closed");

	uv_ref((uv_handle_t *) &event->handle);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_poll_event_unref, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(PollEvent, unref)
{
	async_poll_event *event;

	ZEND_PARSE_PARAMETERS_NONE();

	event = (async_poll_event *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(event->flags & ASYNC_POLL_EVENT_FLAG_CLOSED, "Poll has been closed");

	uv_unref((uv_handle_t *) &event->handle);
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_CTOR(PollEvent, async_poll_event_ce)
ASYNC_METHOD_NO_WAKEUP(PollEvent, async_poll_event_ce)
//LCOV_EXCL_STOP

static const zend_function_entry poll_event_functions[] = {
	PHP_ME(PollEvent, __construct, arginfo_no_ctor, ZEND_ACC_PRIVATE)
	PHP_ME(PollEvent, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(PollEvent, close, arginfo_poll_event_close, ZEND_ACC_PUBLIC)
	PHP_ME(PollEvent, start, arginfo_poll_event_start, ZEND_ACC_PUBLIC)
	PHP_ME(PollEvent, stop, arginfo_poll_event_stop, ZEND_ACC_PUBLIC)
	PHP_ME(PollEvent, ref, arginfo_poll_event_ref, ZEND_ACC_PUBLIC)
	PHP_ME(PollEvent, unref, arginfo_poll_event_unref, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


static zend_always_inline void activate_tick(async_tick_event *event)
{
	ZEND_ASSERT(event->list == NULL);

	if (event->scheduler->ticks.first == NULL) {
		if (event->scheduler->flags & ASYNC_TASK_SCHEDULER_FLAG_ACTIVE) {
			uv_stop(&event->scheduler->loop);
		}
	}

	event->list = &event->scheduler->ticks;

	ASYNC_LIST_APPEND(event->list, event);
	ASYNC_ADDREF(&event->std);

	if (EXPECTED(event->flags & ASYNC_TICK_EVENT_FLAG_REFERENCED)) {
		event->scheduler->refticks++;
	}
}

static zend_always_inline void deactivate_tick(async_tick_event *event)
{
	ZEND_ASSERT(event->list != NULL);

	ASYNC_LIST_REMOVE(event->list, event);
	ASYNC_DELREF(&event->std);

	event->list = NULL;

	if (EXPECTED(event->flags & ASYNC_TICK_EVENT_FLAG_REFERENCED)) {
		event->scheduler->refticks--;
	}
}

async_tick_event *async_tick_event_object_create(async_task_scheduler *scheduler, zval *callback)
{
	async_tick_event *event;

	event = ecalloc(1, sizeof(async_tick_event));

	INIT_CALLBACK(callback, event);

	zend_object_std_init(&event->std, async_tick_event_ce);
	event->std.handlers = &async_tick_event_handlers;

	event->scheduler = async_task_scheduler_ref();
	event->context = ASYNC_G(foreground);

	ASYNC_ADDREF(&event->context->std);

	ZVAL_COPY(&event->callback, callback);

	event->flags |= ASYNC_TICK_EVENT_FLAG_REFERENCED;

	activate_tick(event);

	return event;
}

static void async_tick_event_object_destroy(zend_object *object)
{
	async_tick_event *event;

	event = (async_tick_event *) object;

	zval_ptr_dtor(&event->callback);

	async_task_scheduler_unref(event->scheduler);
	async_context_unref(event->context);

	zend_object_std_dtor(&event->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tick_event_close, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(TickEvent, close)
{
	async_tick_event *event;

	ZEND_PARSE_PARAMETERS_NONE();

	event = (async_tick_event *) Z_OBJ_P(getThis());

	event->flags |= ASYNC_TICK_EVENT_FLAG_CLOSED;

	if (EXPECTED(event->list)) {
		deactivate_tick(event);
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tick_event_start, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(TickEvent, start)
{
	async_tick_event *event;

	ZEND_PARSE_PARAMETERS_NONE();

	event = (async_tick_event *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(event->flags & ASYNC_TICK_EVENT_FLAG_CLOSED, "Tick has been closed");

	if (EXPECTED(event->list == NULL)) {
		activate_tick(event);
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tick_event_stop, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(TickEvent, stop)
{
	async_tick_event *event;

	ZEND_PARSE_PARAMETERS_NONE();

	event = (async_tick_event *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(event->flags & ASYNC_TICK_EVENT_FLAG_CLOSED, "Tick has been closed");

	if (EXPECTED(event->list)) {
		deactivate_tick(event);
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tick_event_ref, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(TickEvent, ref)
{
	async_tick_event *event;

	ZEND_PARSE_PARAMETERS_NONE();

	event = (async_tick_event *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(event->flags & ASYNC_TICK_EVENT_FLAG_CLOSED, "Tick has been closed");

	if (EXPECTED(!(event->flags & ASYNC_TICK_EVENT_FLAG_REFERENCED))) {
		event->flags |= ASYNC_TICK_EVENT_FLAG_REFERENCED;

		if (EXPECTED(event->list)) {
			event->scheduler->refticks++;
		}
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tick_event_unref, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(TickEvent, unref)
{
	async_tick_event *event;

	ZEND_PARSE_PARAMETERS_NONE();

	event = (async_tick_event *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(event->flags & ASYNC_TICK_EVENT_FLAG_CLOSED, "Tick has been closed");

	if (EXPECTED(event->flags & ASYNC_TICK_EVENT_FLAG_REFERENCED)) {
		event->flags ^= ASYNC_TICK_EVENT_FLAG_REFERENCED;

		if (EXPECTED(event->list)) {
			event->scheduler->refticks--;
		}
	}
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_CTOR(TickEvent, async_tick_event_ce)
ASYNC_METHOD_NO_WAKEUP(TickEvent, async_tick_event_ce)
//LCOV_EXCL_STOP

static const zend_function_entry tick_event_functions[] = {
	PHP_ME(TickEvent, __construct, arginfo_no_ctor, ZEND_ACC_PRIVATE)
	PHP_ME(TickEvent, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(TickEvent, close, arginfo_tick_event_close, ZEND_ACC_PUBLIC)
	PHP_ME(TickEvent, start, arginfo_tick_event_start, ZEND_ACC_PUBLIC)
	PHP_ME(TickEvent, stop, arginfo_tick_event_stop, ZEND_ACC_PUBLIC)
	PHP_ME(TickEvent, ref, arginfo_tick_event_ref, ZEND_ACC_PUBLIC)
	PHP_ME(TickEvent, unref, arginfo_tick_event_unref, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


ASYNC_CALLBACK close_timer(uv_handle_t *handle)
{
	async_timer_event *event;

	event = (async_timer_event *) handle->data;

	ZEND_ASSERT(event != NULL);

	ASYNC_DELREF(&event->std);
}

ASYNC_CALLBACK shutdown_timer(void *object, zval *error)
{
	async_timer_event *event;

	event = (async_timer_event *) object;

	event->shutdown.func = NULL;
	event->flags |= ASYNC_TIMER_EVENT_FLAG_CLOSED;

	ASYNC_UV_TRY_CLOSE_REF(&event->std, &event->handle, close_timer);

	if (event->flags & ASYNC_TIMER_EVENT_FLAG_SCHEDULED) {
		event->flags &= ~ASYNC_TIMER_EVENT_FLAG_SCHEDULED;

		ASYNC_DELREF(&event->std);
	}
}

ASYNC_CALLBACK timer_cb(uv_timer_t *handle)
{
	async_timer_event *event;

	zend_fcall_info fci;

	zval args[1];
	zval retval;

	int code;

	event = (async_timer_event *) handle->data;

	ZEND_ASSERT(event != NULL);
	ZEND_ASSERT(event->flags & ASYNC_TIMER_EVENT_FLAG_SCHEDULED);

	ZVAL_OBJ(&args[0], &event->std);
	ASYNC_ADDREF(&event->std);

	if (!(event->flags & ASYNC_TIMER_EVENT_FLAG_PERIODIC)) {
		event->flags &= ~ASYNC_TIMER_EVENT_FLAG_SCHEDULED;

		ASYNC_DELREF(&event->std);
	}

	fci.size = sizeof(zend_fcall_info);
	fci.object = event->fcc.object;
	fci.no_separation = 1;

	ZVAL_COPY_VALUE(&fci.function_name, &event->callback);

	fci.params = args;
	fci.param_count = 1;
	fci.retval = &retval;

	zend_try {
		code = async_call_nowait(ASYNC_G(exec), &fci, &event->fcc);
	} zend_catch {
		async_task_scheduler_handle_exit(event->scheduler);
		ASYNC_DELREF(&event->std);
		return;
	} zend_end_try();

	if (UNEXPECTED(code == FAILURE)) {
		ASYNC_ENSURE_ERROR("Failed to invoke timer callback");

		async_task_scheduler_handle_error(event->scheduler, EG(exception));
		zend_clear_exception();
	} else {
		zval_ptr_dtor(&retval);
	}

	ASYNC_DELREF(&event->std);
}

async_timer_event *async_timer_event_object_create(async_task_scheduler *scheduler, zval *callback)
{
	async_timer_event *event;

	event = ecalloc(1, sizeof(async_timer_event));

	INIT_CALLBACK(callback, event);

	zend_object_std_init(&event->std, async_timer_event_ce);
	event->std.handlers = &async_timer_event_handlers;

	event->scheduler = async_task_scheduler_ref();
	event->context = ASYNC_G(foreground);

	ASYNC_ADDREF(&event->context->std);

	ZVAL_COPY(&event->callback, callback);

	uv_timer_init(&event->scheduler->loop, &event->handle);

	event->handle.data = event;

	event->shutdown.object = event;
	event->shutdown.func = shutdown_timer;

	ASYNC_LIST_APPEND(&event->scheduler->shutdown, &event->shutdown);

	return event;
}

static void async_timer_event_object_dtor(zend_object *object)
{
	async_timer_event *event;

	event = (async_timer_event *) object;

	if (event->shutdown.func) {
		ASYNC_LIST_REMOVE(&event->scheduler->shutdown, &event->shutdown);

		event->shutdown.func(event, NULL);
	}
}

static void async_timer_event_object_destroy(zend_object *object)
{
	async_timer_event *event;

	event = (async_timer_event *) object;

	zval_ptr_dtor(&event->callback);

	async_task_scheduler_unref(event->scheduler);
	async_context_unref(event->context);

	zend_object_std_dtor(&event->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_timer_event_close, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(TimerEvent, close)
{
	async_timer_event *event;

	ZEND_PARSE_PARAMETERS_NONE();

	event = (async_timer_event *) Z_OBJ_P(getThis());

	if (EXPECTED(!(event->flags & ASYNC_TIMER_EVENT_FLAG_CLOSED))) {
		event->flags |= ASYNC_TIMER_EVENT_FLAG_CLOSED;

		if (event->shutdown.func) {
			ASYNC_LIST_REMOVE(&event->scheduler->shutdown, &event->shutdown);

			event->shutdown.func(event, NULL);
		}
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_timer_event_start, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, delay, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, periodic, _IS_BOOL, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(TimerEvent, start)
{
	async_timer_event *event;

	zend_long delay;
	zend_bool periodic;

	periodic = 0;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 2)
		Z_PARAM_LONG(delay)
		Z_PARAM_OPTIONAL
		Z_PARAM_BOOL(periodic)
	ZEND_PARSE_PARAMETERS_END();

	ASYNC_CHECK_ERROR(delay < 0, "Timer delay must not be negative");

	event = (async_timer_event *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(event->flags & ASYNC_TIMER_EVENT_FLAG_CLOSED, "Timer has been closed");

	event->delay = (uint64_t) delay;

	if (periodic) {
		event->flags |= ASYNC_TIMER_EVENT_FLAG_PERIODIC;
	} else {
		event->flags &= ~ASYNC_TIMER_EVENT_FLAG_PERIODIC;
	}

	uv_timer_start(&event->handle, timer_cb, event->delay, (event->flags & ASYNC_TIMER_EVENT_FLAG_PERIODIC) ? event->delay : 0);

	if (!(event->flags & ASYNC_TIMER_EVENT_FLAG_SCHEDULED)) {
		event->flags |= ASYNC_TIMER_EVENT_FLAG_SCHEDULED;

		ASYNC_ADDREF(&event->std);
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_timer_event_stop, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(TimerEvent, stop)
{
	async_timer_event *event;

	ZEND_PARSE_PARAMETERS_NONE();

	event = (async_timer_event *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(event->flags & ASYNC_TIMER_EVENT_FLAG_CLOSED, "Timer has been closed");

	uv_timer_stop(&event->handle);

	if (event->flags & ASYNC_TIMER_EVENT_FLAG_SCHEDULED) {
		event->flags &= ~ASYNC_TIMER_EVENT_FLAG_SCHEDULED;

		ASYNC_DELREF(&event->std);
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_timer_event_ref, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(TimerEvent, ref)
{
	async_timer_event *event;

	ZEND_PARSE_PARAMETERS_NONE();

	event = (async_timer_event *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(event->flags & ASYNC_TIMER_EVENT_FLAG_CLOSED, "Timer has been closed");

	uv_ref((uv_handle_t *) &event->handle);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_timer_event_unref, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(TimerEvent, unref)
{
	async_timer_event *event;

	ZEND_PARSE_PARAMETERS_NONE();

	event = (async_timer_event *) Z_OBJ_P(getThis());

	ASYNC_CHECK_ERROR(event->flags & ASYNC_TIMER_EVENT_FLAG_CLOSED, "Timer has been closed");

	uv_unref((uv_handle_t *) &event->handle);
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_CTOR(TimerEvent, async_timer_event_ce)
ASYNC_METHOD_NO_WAKEUP(TimerEvent, async_timer_event_ce)
//LCOV_EXCL_STOP

static const zend_function_entry timer_event_functions[] = {
	PHP_ME(TimerEvent, __construct, arginfo_no_ctor, ZEND_ACC_PRIVATE)
	PHP_ME(TimerEvent, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(TimerEvent, close, arginfo_timer_event_close, ZEND_ACC_PUBLIC)
	PHP_ME(TimerEvent, start, arginfo_timer_event_start, ZEND_ACC_PUBLIC)
	PHP_ME(TimerEvent, stop, arginfo_timer_event_stop, ZEND_ACC_PUBLIC)
	PHP_ME(TimerEvent, ref, arginfo_timer_event_ref, ZEND_ACC_PUBLIC)
	PHP_ME(TimerEvent, unref, arginfo_timer_event_unref, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


void async_event_ce_register()
{
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async", "PollEvent", poll_event_functions);
	async_poll_event_ce = zend_register_internal_class(&ce);
	async_poll_event_ce->ce_flags |= ZEND_ACC_FINAL;
	async_poll_event_ce->serialize = zend_class_serialize_deny;
	async_poll_event_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_poll_event_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_poll_event_handlers.dtor_obj = async_poll_event_object_dtor;
	async_poll_event_handlers.free_obj = async_poll_event_object_destroy;
	async_poll_event_handlers.clone_obj = NULL;

	ASYNC_POLL_EVENT_CONST("READABLE", ASYNC_POLL_EVENT_FLAG_READABLE);
	ASYNC_POLL_EVENT_CONST("WRITABLE", ASYNC_POLL_EVENT_FLAG_WRITABLE);
	ASYNC_POLL_EVENT_CONST("DISCONNECT", ASYNC_POLL_EVENT_FLAG_DISCONNECT);

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async", "TickEvent", tick_event_functions);
	async_tick_event_ce = zend_register_internal_class(&ce);
	async_tick_event_ce->ce_flags |= ZEND_ACC_FINAL;
	async_tick_event_ce->serialize = zend_class_serialize_deny;
	async_tick_event_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_tick_event_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_tick_event_handlers.free_obj = async_tick_event_object_destroy;
	async_tick_event_handlers.clone_obj = NULL;

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async", "TimerEvent", timer_event_functions);
	async_timer_event_ce = zend_register_internal_class(&ce);
	async_timer_event_ce->ce_flags |= ZEND_ACC_FINAL;
	async_timer_event_ce->serialize = zend_class_serialize_deny;
	async_timer_event_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_timer_event_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_timer_event_handlers.dtor_obj = async_timer_event_object_dtor;
	async_timer_event_handlers.free_obj = async_timer_event_object_destroy;
	async_timer_event_handlers.clone_obj = NULL;
}
