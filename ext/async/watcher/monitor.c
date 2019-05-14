
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

ASYNC_API zend_class_entry *async_monitor_ce;
ASYNC_API zend_class_entry *async_monitor_event_ce;

static zend_object_handlers async_monitor_handlers;
static zend_object_handlers async_monitor_event_handlers;

typedef struct {
	zend_string *path;
	
	zend_object std;
} async_monitor_event;

static zend_string *str_events;
static zend_string *str_path;

static async_monitor_event *async_monitor_event_object_create(int events, char *path, int len);

typedef struct {
	/* PHP object handle. */
	zend_object std;
	
	uv_fs_event_t handle;
	zend_string *path;
	zend_bool recursive;
	
	async_task_scheduler *scheduler;	
	async_cancel_cb cancel;
	async_op_list listeners;
	
	zval error;
} async_monitor;

typedef struct {
	async_op base;
	int status;
	int events;
	int len;
	char path[MAXPATHLEN];
} async_monitor_op;

#define ASYNC_MONITOR_EVENT_CONST(name, value) \
	zend_declare_class_constant_long(async_monitor_event_ce, name, sizeof(name)-1, (zend_long)value);


ASYNC_CALLBACK close_cb(uv_handle_t *handle)
{
	async_monitor *monitor;
	
	monitor = (async_monitor *) handle->data;
	
	ZEND_ASSERT(monitor != NULL);
	
	ASYNC_DELREF(&monitor->std);
}

ASYNC_CALLBACK shutdown_cb(void *arg, zval *error)
{
	async_monitor *monitor;
	
	monitor = (async_monitor *) arg;
	
	ZEND_ASSERT(monitor != NULL);
	
	monitor->cancel.func = NULL;
	
	if (error != NULL && Z_TYPE_P(&monitor->error) == IS_UNDEF) {
		ZVAL_COPY(&monitor->error, error);
	}
	
	if (!uv_is_closing((uv_handle_t *) &monitor->handle)) {
		ASYNC_ADDREF(&monitor->std);
		
		uv_close((uv_handle_t *) &monitor->handle, close_cb);
	}
	
	while (monitor->listeners.first) {
		ASYNC_FAIL_OP(monitor->listeners.first, &monitor->error);
	}
}

static zend_object *async_monitor_object_create(zend_class_entry *ce)
{
	async_monitor *monitor;

	monitor = ecalloc(1, sizeof(async_monitor));

	zend_object_std_init(&monitor->std, ce);
	monitor->std.handlers = &async_monitor_handlers;
	
	monitor->scheduler = async_task_scheduler_ref();
	
	monitor->cancel.func = shutdown_cb;
	monitor->cancel.object = monitor;
	
	ASYNC_LIST_APPEND(&monitor->scheduler->shutdown, &monitor->cancel);
	
	uv_fs_event_init(&monitor->scheduler->loop, &monitor->handle);
	
	monitor->handle.data = monitor;
	
	return &monitor->std;
}

static void async_monitor_object_dtor(zend_object *object)
{
	async_monitor *monitor;

	monitor = (async_monitor *) object;

	if (monitor->cancel.func != NULL) {
		ASYNC_LIST_REMOVE(&monitor->scheduler->shutdown, &monitor->cancel);
		
		monitor->cancel.func(monitor, NULL);
	}
}

static void async_monitor_object_destroy(zend_object *object)
{
	async_monitor *monitor;

	monitor = (async_monitor *) object;
	
	zval_ptr_dtor(&monitor->error);
	
	async_task_scheduler_unref(monitor->scheduler);
	
	if (monitor->path != NULL) {
		zend_string_release(monitor->path);
	}
	
	zend_object_std_dtor(&monitor->std);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_monitor_ctor, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, recursive, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

static ZEND_METHOD(Monitor, __construct)
{
	async_monitor *monitor;
	
	zend_string *target;
	zend_bool recursive;
	
	char path[MAXPATHLEN];
	
	recursive = 0;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 2)
		Z_PARAM_STR(target)
		Z_PARAM_OPTIONAL
		Z_PARAM_BOOL(recursive)
	ZEND_PARSE_PARAMETERS_END();

	ASYNC_CHECK_ERROR(!VCWD_REALPATH(ZSTR_VAL(target), path), "Failed to verify path: %s", ZSTR_VAL(target));
	
#ifndef PHP_WIN32
	ASYNC_CHECK_ERROR(recursive, "Recursive file monitoring is only supported on Windows");
#endif

	monitor = (async_monitor *) Z_OBJ_P(getThis());
	
	monitor->path = zend_string_init(path, strlen(path), 0);
	monitor->recursive = recursive;
}

ASYNC_CALLBACK event_cb(uv_fs_event_t *handle, const char *name, int events, int status)
{
	async_monitor *monitor;
	async_monitor_op *op;
	
	monitor = (async_monitor *) handle->data;
	
	ZEND_ASSERT(monitor != NULL);
	
	if (monitor->listeners.first != NULL) {
		op = (async_monitor_op *) monitor->listeners.first;
		
#ifdef PHP_WIN32
		op->len = snprintf(op->path, MAXPATHLEN, "%s\\%s", ZSTR_VAL(monitor->path), name);
#else
		op->len = snprintf(op->path, MAXPATHLEN, "%s/%s", ZSTR_VAL(monitor->path), name);
#endif

		op->status = status;
		op->events = events;
		
		ASYNC_FINISH_OP(op);
	}
	
	if (monitor->listeners.first == NULL) {
		uv_fs_event_stop(&monitor->handle);
	}
}

#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_monitor_close, 0, 0, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 1)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_monitor_close, 0, 0, IS_VOID, NULL, 0)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 1)
ZEND_END_ARG_INFO()
#endif

static ZEND_METHOD(Monitor, close)
{
	async_monitor *monitor;

	zval error;
	zval *val;

	val = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();

	monitor = (async_monitor *) Z_OBJ_P(getThis());

	if (monitor->cancel.func == NULL) {
		return;
	}
	
	ASYNC_PREPARE_ERROR(&error, "Monitor has been closed");
	
	if (val != NULL && Z_TYPE_P(val) != IS_NULL) {
		zend_exception_set_previous(Z_OBJ_P(&error), Z_OBJ_P(val));
		GC_ADDREF(Z_OBJ_P(val));
	}
	
	ASYNC_LIST_REMOVE(&monitor->scheduler->shutdown, &monitor->cancel);
	
	monitor->cancel.func(monitor, &error);
	
	zval_ptr_dtor(&error);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_monitor_await_event, 0, 0, Phalcon\\Async\\MonitorEvent, 0)
ZEND_END_ARG_INFO()

static ZEND_METHOD(Monitor, awaitEvent)
{
	async_monitor *monitor;
	async_monitor_event *event;
	async_monitor_op *op;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	monitor = (async_monitor *) Z_OBJ_P(getThis());
	
	uv_fs_event_start(&monitor->handle, event_cb, ZSTR_VAL(monitor->path), monitor->recursive ? UV_FS_EVENT_RECURSIVE : 0);
	
	ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_monitor_op));
	ASYNC_APPEND_OP(&monitor->listeners, op);
	
	if (UNEXPECTED(FAILURE == async_await_op((async_op *) op))) {
		ASYNC_FORWARD_OP_ERROR(op);
		ASYNC_FREE_OP(op);
		
		return;
	}
	
	event = async_monitor_event_object_create(op->events, op->path, op->len);
	
	ASYNC_FREE_OP(op);
	
	RETURN_OBJ(&event->std);
}

static const zend_function_entry async_monitor_functions[] = {
	ZEND_ME(Monitor, __construct, arginfo_monitor_ctor, ZEND_ACC_PUBLIC)
	ZEND_ME(Monitor, close, arginfo_monitor_close, ZEND_ACC_PUBLIC)
	ZEND_ME(Monitor, awaitEvent, arginfo_monitor_await_event, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static zend_always_inline async_monitor_event *async_monitor_event_obj(zend_object *object)
{
	return (async_monitor_event *)((char *) object - XtOffsetOf(async_monitor_event, std));
}

static zend_always_inline uint32_t async_monitor_event_prop_offset(zend_string *name)
{
	return zend_get_property_info(async_monitor_event_ce, name, 1)->offset;
}

static async_monitor_event *async_monitor_event_object_create(int events, char *path, int len)
{
	async_monitor_event *event;
	
	event = ecalloc(1, sizeof(async_monitor_event) + zend_object_properties_size(async_monitor_event_ce));
	
	zend_object_std_init(&event->std, async_monitor_event_ce);	
	event->std.handlers = &async_monitor_event_handlers;
	
	object_properties_init(&event->std, async_monitor_event_ce);
	
	event->path = zend_string_init(path, len, 0);
	
	ZVAL_LONG(OBJ_PROP(&event->std, async_monitor_event_prop_offset(str_events)), events);
	ZVAL_STR_COPY(OBJ_PROP(&event->std, async_monitor_event_prop_offset(str_path)), event->path);

	return event;
}

static void async_monitor_event_object_destroy(zend_object *object)
{
	async_monitor_event *event;
	
	event = async_monitor_event_obj(object);
	
	if (event->path != NULL) {
		zend_string_release(event->path);
	}
	
	zend_object_std_dtor(&event->std);
}

static const zend_function_entry async_monitor_event_functions[] = {
	ZEND_FE_END
};


void async_monitor_ce_register()
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Monitor", async_monitor_functions);
	async_monitor_ce = zend_register_internal_class(&ce);
	async_monitor_ce->ce_flags |= ZEND_ACC_FINAL;
	async_monitor_ce->create_object = async_monitor_object_create;
	async_monitor_ce->serialize = zend_class_serialize_deny;
	async_monitor_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_monitor_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_monitor_handlers.free_obj = async_monitor_object_destroy;
	async_monitor_handlers.dtor_obj = async_monitor_object_dtor;
	async_monitor_handlers.clone_obj = NULL;
	
	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\MonitorEvent", async_monitor_event_functions);
	async_monitor_event_ce = zend_register_internal_class(&ce);
	async_monitor_event_ce->ce_flags |= ZEND_ACC_FINAL;
	async_monitor_event_ce->create_object = NULL;
	async_monitor_event_ce->serialize = zend_class_serialize_deny;
	async_monitor_event_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_monitor_event_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_monitor_event_handlers.offset = XtOffsetOf(async_monitor_event, std);
	async_monitor_event_handlers.free_obj = async_monitor_event_object_destroy;
	async_monitor_event_handlers.clone_obj = NULL;
	
	ASYNC_MONITOR_EVENT_CONST("RENAMED", UV_RENAME);
	ASYNC_MONITOR_EVENT_CONST("CHANGED", UV_CHANGE);
	
	zend_declare_property_null(async_monitor_event_ce, ZEND_STRL("events"), ZEND_ACC_PUBLIC);
	zend_declare_property_null(async_monitor_event_ce, ZEND_STRL("path"), ZEND_ACC_PUBLIC);
	
	str_events = zend_new_interned_string(zend_string_init(ZEND_STRL("events"), 1));
	str_path = zend_new_interned_string(zend_string_init(ZEND_STRL("path"), 1));
}

void async_monitor_ce_unregister()
{
	zend_string_release(str_events);
	zend_string_release(str_path);
}

#endif
