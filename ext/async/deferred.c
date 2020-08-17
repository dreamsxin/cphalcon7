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

ASYNC_API zend_class_entry *async_deferred_ce;
ASYNC_API zend_class_entry *async_deferred_awaitable_ce;

static zend_object_handlers async_deferred_handlers;
static zend_object_handlers async_deferred_awaitable_handlers;

static async_await_handler deferred_await_handler;

static zend_string *str_status;
static zend_string *str_file;
static zend_string *str_line;

static uint32_t off_defer_status;
static uint32_t off_defer_file;
static uint32_t off_defer_line;

static uint32_t off_awaitable_status;
static uint32_t off_awaitable_file;
static uint32_t off_awaitable_line;

#define ASYNC_DEFERRED_STATUS_PENDING 0
#define ASYNC_DEFERRED_STATUS_RESOLVED ASYNC_OP_RESOLVED
#define ASYNC_DEFERRED_STATUS_FAILED ASYNC_OP_FAILED

typedef struct _async_deferred_state {
	/* Deferred status, one of PENDING, RESOLVED or FAILED. */
	zend_uchar status;

	/* Internal refcount being used to control deferred lifecycle. */
	uint32_t refcount;

	zend_string *file;
	zend_ulong line;

	/* Holds the result value or error. */
	zval result;

	/* Reference to the task scheduler. */
	async_task_scheduler *scheduler;

	/* Associated async context. */
	async_context *context;

	/* Queue of waiting async operations. */
	async_op_list operations;

	/* Cancel callback being called when the task scheduler is disposed. */
	async_cancel_cb cancel;
} async_deferred_state;

typedef struct _async_deferred {
	/* Refers to the deferred state being shared by deferred and awaitable. */
	async_deferred_state *state;

	/* Inlined cancellation handler called during contextual cancellation. */
	async_cancel_cb cancel;

	/* Function call info & cache of the cancel callback. */
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;

	/* PHP object handle. */
	zend_object std;
} async_deferred;

typedef struct _async_deferred_awaitable {
	/* Refers to the deferred state being shared by deferred and awaitable. */
	async_deferred_state *state;

	/* PHP object handle. */
	zend_object std;
} async_deferred_awaitable;

typedef struct _async_defer_combine {
	async_deferred *defer;
	uint32_t counter;
	uint32_t started;
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
} async_defer_combine;

typedef struct _async_defer_combine_op {
	async_op base;
	async_defer_combine *combine;
	async_deferred_awaitable *awaitable;
	zval key;
} async_defer_combine_op;

typedef struct _async_defer_transform_op {
	async_op base;
	async_deferred_state *state;
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
} async_defer_transform_op;


static zend_always_inline async_deferred *async_deferred_obj(zend_object *object)
{
	return (async_deferred *)((char *) object - XtOffsetOf(async_deferred, std));
}

static zend_always_inline uint32_t async_deferred_prop_offset(zend_string *name)
{
	return zend_get_property_info(async_deferred_ce, name, 1)->offset;
}

static zend_always_inline async_deferred_awaitable *async_deferred_awaitable_obj(zend_object *object)
{
	return (async_deferred_awaitable *)((char *) object - XtOffsetOf(async_deferred_awaitable, std));
}

static zend_always_inline uint32_t async_deferred_awaitable_prop_offset(zend_string *name)
{
	return zend_get_property_info(async_deferred_awaitable_ce, name, 1)->offset;
}

static void refresh_props(zend_object *obj, async_deferred_state *state, uint32_t (* accessor)(zend_string *name))
{
	zval *tmp;

	if (!obj->properties) {
		rebuild_object_properties(obj);
	}

	tmp = OBJ_PROP(obj, accessor(str_status));
	zval_ptr_dtor(tmp);
	ZVAL_STRING(tmp, async_status_label(state->status));

	tmp = OBJ_PROP(obj, accessor(str_file));
	zval_ptr_dtor(tmp);

	if (state->file) {
		ZVAL_STR_COPY(tmp, state->file);
	} else {
		ZVAL_NULL(tmp);
	}

	tmp = OBJ_PROP(obj, accessor(str_line));
	zval_ptr_dtor(tmp);
	ZVAL_LONG(tmp, state->line);
}

static zend_always_inline void debug_deferred_state(async_deferred_state *state, zval *info)
{
	array_init(info);

	add_assoc_string(info, "status", async_status_label(state->status));

	if (state->file) {
		add_assoc_str(info, "file", zend_string_copy(state->file));
	} else {
		add_assoc_null(info, "file");
	}

	add_assoc_long(info, "line", state->line);
}


static zend_always_inline void trigger_ops(async_deferred_state *state)
{
	async_op *op;
	
	if (state->cancel.object != NULL) {
		ASYNC_LIST_REMOVE(&state->scheduler->shutdown, &state->cancel);
		state->cancel.object = NULL;
	}
	
	while (state->operations.first != NULL) {
		ASYNC_NEXT_OP(&state->operations, op);
		
		if (state->status == ASYNC_OP_RESOLVED) {
			ASYNC_RESOLVE_OP(op, &state->result);
		} else {
			ASYNC_FAIL_OP(op, &state->result);
		}
	}
}

ASYNC_CALLBACK shutdown_state(void *obj, zval *error)
{
	async_deferred_state *state;
	
	state = (async_deferred_state *) obj;
	
	ZEND_ASSERT(state != NULL);

	state->cancel.object = NULL;
	state->status = ASYNC_DEFERRED_STATUS_FAILED;
	
	ZVAL_COPY(&state->result, error);
	
	trigger_ops(state);
}

static zend_always_inline async_deferred_state *create_state(async_context *context)
{
	async_deferred_state *state;

	state = ecalloc(1, sizeof(async_deferred_state));

	ZVAL_NULL(&state->result);

	state->refcount = 1;
	state->scheduler = async_task_scheduler_ref();
	state->context = context;
	
	state->cancel.object = state;
	state->cancel.func = shutdown_state;
	
	ASYNC_LIST_APPEND(&state->scheduler->shutdown, &state->cancel);
	
	ASYNC_ADDREF(&context->std);

	return state;
}

static zend_always_inline void capture_call_context(async_deferred_state *state, zend_execute_data *call)
{
	if (call != NULL && call->func && ZEND_USER_CODE(call->func->common.type)) {
		if (call->func->op_array.filename != NULL) {
			state->file = zend_string_copy(call->func->op_array.filename);
		}

		state->line = call->opline->lineno;
	}
}

static zend_always_inline void release_state(async_deferred_state *state)
{
	async_op *op;

	if (0 != --state->refcount) {
		return;
	}
	
	ZEND_ASSERT(state->status == ASYNC_DEFERRED_STATUS_PENDING || state->operations.first == NULL);

	if (state->cancel.object != NULL) {
		ASYNC_LIST_REMOVE(&state->scheduler->shutdown, &state->cancel);
		state->cancel.object = NULL;
	}

	if (UNEXPECTED(state->status == ASYNC_DEFERRED_STATUS_PENDING)) {
		state->status = state->status == ASYNC_DEFERRED_STATUS_FAILED;

		if (state->operations.first != NULL) {
			ASYNC_PREPARE_SCHEDULER_ERROR(&state->result, "Awaitable has been disposed before it was resolved");

			while (state->operations.first != NULL) {
				ASYNC_NEXT_OP(&state->operations, op);
				ASYNC_FAIL_OP(op, &state->result);
			}
		}
	}

	zval_ptr_dtor(&state->result);

	ASYNC_DELREF(&state->context->std);
	
	async_task_scheduler_unref(state->scheduler);

	if (state->file) {
		zend_string_release(state->file);
	}

	efree(state);
}

static zend_always_inline void cleanup_cancel(async_deferred *defer)
{
	if (defer->cancel.object != NULL && Z_TYPE_P(&defer->fci.function_name) != IS_UNDEF) {
		ASYNC_LIST_REMOVE(&defer->state->context->cancel->callbacks, &defer->cancel);
		defer->cancel.object = NULL;
	}
}

ASYNC_CALLBACK cancel_defer(void *obj, zval* error)
{
	async_deferred *defer;

	zval args[2];
	zval retval;

	defer = (async_deferred *) obj;

	ZEND_ASSERT(defer != NULL);

	defer->cancel.object = NULL;

	ZVAL_OBJ(&args[0], &defer->std);
	ASYNC_ADDREF(&defer->std);

	ZVAL_COPY(&args[1], error);

	defer->fci.param_count = 2;
	defer->fci.params = args;
	defer->fci.retval = &retval;

#if PHP_VERSION_ID < 80000
	defer->fci.no_separation = 1;
#endif

	async_call_nowait(EG(current_execute_data), &defer->fci, &defer->fcc);

	zval_ptr_dtor(&args[0]);
	zval_ptr_dtor(&args[1]);
	zval_ptr_dtor(&retval);

	ASYNC_DELREF_CB(defer->fci);

	ASYNC_CHECK_FATAL(EG(exception), "Must not throw an error from cancellation handler");
}


static int deferred_await_delegate(zend_object *obj, zval *result, async_task *caller, async_context *context, int flags)
{
	async_deferred_state *state;

	state = (async_deferred_awaitable_obj(obj))->state;

	switch (state->status) {
	case ASYNC_OP_RESOLVED:
	case ASYNC_OP_FAILED:
		ZVAL_COPY(result, &state->result);

		return state->status;
	}

	return 0;
}

static void deferred_await_schedule(zend_object *obj, async_op *op, async_task *caller, async_context *context, int flags)
{
	async_deferred_state *state;

	state = (async_deferred_awaitable_obj(obj))->state;

	ASYNC_APPEND_OP(&state->operations, op);
}

static async_deferred_awaitable *async_deferred_awaitable_object_create(async_deferred_state *state)
{
	async_deferred_awaitable *awaitable;

	awaitable = ecalloc(1, sizeof(async_deferred_awaitable) + zend_object_properties_size(async_deferred_awaitable_ce));

	zend_object_std_init(&awaitable->std, async_deferred_awaitable_ce);
	awaitable->std.handlers = &async_deferred_awaitable_handlers;

	object_properties_init(&awaitable->std, async_deferred_awaitable_ce);

	awaitable->state = state;

	state->refcount++;

	return awaitable;
}

static void async_deferred_awaitable_object_destroy(zend_object *object)
{
	async_deferred_awaitable *awaitable;

	awaitable = async_deferred_awaitable_obj(object);

	release_state(awaitable->state);

	zend_object_std_dtor(&awaitable->std);
}

static int deferred_awaitable_has_prop(zval *object, zval *member, int has_set_exists, void **cache_slot)
{
	async_deferred_awaitable *awaitable;

	zend_string *name;
	zend_property_info *info;

	awaitable = async_deferred_awaitable_obj(Z_OBJ_P(object));

	name = Z_STR_P(member);
	info = zend_get_property_info(Z_OBJCE_P(object), name, 0);

	if (info == NULL) {
		return 0;
	}

	if (info->offset == off_awaitable_status) {
		return 1;
	}

	if (has_set_exists != ZEND_PROPERTY_EXISTS) {
		if (info->offset == off_awaitable_line) {
			return (has_set_exists == ZEND_PROPERTY_NOT_EMPTY) ? (awaitable->state->line > 0) : 1;
		}

		if (info->offset == off_awaitable_file) {
			return awaitable->state->file ? 1 : 0;
		}
	}

	return 1;
}

static zval *deferred_awaitable_read_prop(zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
	async_deferred_awaitable *awaitable;

	zend_string *name;
	zend_property_info *info;

	awaitable = async_deferred_awaitable_obj(Z_OBJ_P(object));

	name = Z_STR_P(member);
	info = zend_get_property_info(Z_OBJCE_P(object), name, 0);

	if (info == NULL) {
		rv = &EG(uninitialized_zval);
	} else if (info->offset == off_awaitable_status) {
		ZVAL_STRING(rv, async_status_label(awaitable->state->status));
	} else if (info->offset == off_awaitable_file) {
		if (awaitable->state->file) {
			ZVAL_STR_COPY(rv, awaitable->state->file);
		} else {
			ZVAL_NULL(rv);
		}
	} else if (info->offset == off_awaitable_line) {
		ZVAL_LONG(rv, awaitable->state->line);
	} else {
		rv = &EG(uninitialized_zval);
	}

	return rv;
}

static ASYNC_DEBUG_INFO_HANDLER(deferred_awaitable_debug_info)
{
	async_deferred_awaitable *awaitable;
	zval info;

	*temp = 1;

	awaitable = async_deferred_awaitable_obj(ASYNC_DEBUG_INFO_OBJ());

	debug_deferred_state(awaitable->state, &info);

	return Z_ARRVAL(info);
}

static HashTable *deferred_awaitable_get_props(zval *obj)
{
	async_deferred_awaitable *awaitable;

	awaitable = async_deferred_awaitable_obj(Z_OBJ_P(obj));

	refresh_props(&awaitable->std, awaitable->state, async_deferred_awaitable_prop_offset);

	return awaitable->std.properties;
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_CTOR(DeferredAwaitable, async_deferred_awaitable_ce)
ASYNC_METHOD_NO_WAKEUP(DeferredAwaitable, async_deferred_awaitable_ce)
//LCOV_EXCL_STOP

static const zend_function_entry deferred_awaitable_functions[] = {
	PHP_ME(DeferredAwaitable, __construct, arginfo_no_ctor, ZEND_ACC_PRIVATE)
	PHP_ME(DeferredAwaitable, __wakeup, arginfo_no_wakeup, ZEND_ACC_PRIVATE)
	PHP_FE_END
};


static zend_object *async_deferred_object_create(zend_class_entry *ce)
{
	async_deferred *defer;

	defer = ecalloc(1, sizeof(async_deferred) + zend_object_properties_size(async_deferred_ce));

	zend_object_std_init(&defer->std, ce);
	defer->std.handlers = &async_deferred_handlers;
	
	object_properties_init(&defer->std, async_deferred_ce);

	defer->state = create_state(async_context_get());

	return &defer->std;
}

static void async_deferred_object_destroy(zend_object *object)
{
	async_deferred *defer;

	defer = async_deferred_obj(object);

	cleanup_cancel(defer);
	
	if (defer->state->status == ASYNC_DEFERRED_STATUS_PENDING) {
		defer->state->status = ASYNC_OP_FAILED;
		ASYNC_PREPARE_SCHEDULER_ERROR(&defer->state->result, "Awaitable has been disposed before it was resolved");

		trigger_ops(defer->state);
	}
	
	release_state(defer->state);

	zend_object_std_dtor(&defer->std);
}

#if PHP_VERSION_ID < 80000
static int deferred_has_prop(zval *object, zval *member, int has_set_exists, void **cache_slot)
{
	async_deferred *defer;

	zend_string *name;
	zend_property_info *info;

	defer = async_deferred_obj(Z_OBJ_P(object));

	name = Z_STR_P(member);
	info = zend_get_property_info(Z_OBJCE_P(object), name, 0);

	if (info == NULL) {
		return 0;
	}

	if (info->offset == off_defer_status) {
		return 1;
	}

	if (has_set_exists != ZEND_PROPERTY_EXISTS) {
		if (info->offset == off_defer_line) {
			return (has_set_exists == ZEND_PROPERTY_NOT_EMPTY) ? (defer->state->line > 0) : 1;
		}

		if (info->offset == off_defer_line) {
			return defer->state->file ? 1 : 0;
		}
	}

	return 1;
}
#else
static int deferred_has_prop(zend_object *object, zend_string *member, int has_set_exists, void **cache_slot)
{
	async_deferred *defer;
	zend_property_info *info;

	defer = async_deferred_obj(object);

	info = zend_get_property_info(object->ce, member, 0);

	if (info == NULL) {
		return 0;
	}

	if (info->offset == off_defer_status) {
		return 1;
	}

	if (has_set_exists != ZEND_PROPERTY_EXISTS) {
		if (info->offset == off_defer_line) {
			return (has_set_exists == ZEND_PROPERTY_NOT_EMPTY) ? (defer->state->line > 0) : 1;
		}

		if (info->offset == off_defer_line) {
			return defer->state->file ? 1 : 0;
		}
	}

	return 1;
}
#endif

#if PHP_VERSION_ID < 80000
static zval *deferred_read_prop(zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
	async_deferred *defer;

	zend_string *name;
	zend_property_info *info;

	defer = async_deferred_obj(Z_OBJ_P(object));

	name = Z_STR_P(member);
	info = zend_get_property_info(Z_OBJCE_P(object), name, 0);

	if (info == NULL) {
		rv = &EG(uninitialized_zval);
	} else if (info->offset == off_defer_status) {
		ZVAL_STRING(rv, async_status_label(defer->state->status));
	} else if (info->offset == off_defer_file) {
		if (defer->state->file) {
			ZVAL_STR_COPY(rv, defer->state->file);
		} else {
			ZVAL_NULL(rv);
		}
	} else if (info->offset == off_defer_line) {
		ZVAL_LONG(rv, defer->state->line);
	} else {
		rv = &EG(uninitialized_zval);
	}

	return rv;
}
#else
static zval *deferred_read_prop(zend_object *object, zend_string *member, int type, void **cache_slot, zval *rv)
{
	async_deferred *defer;
	zend_property_info *info;

	defer = async_deferred_obj(object);

	info = zend_get_property_info(object->ce, member, 0);

	if (info == NULL) {
		rv = &EG(uninitialized_zval);
	} else if (info->offset == off_defer_status) {
		ZVAL_STRING(rv, async_status_label(defer->state->status));
	} else if (info->offset == off_defer_file) {
		if (defer->state->file) {
			ZVAL_STR_COPY(rv, defer->state->file);
		} else {
			ZVAL_NULL(rv);
		}
	} else if (info->offset == off_defer_line) {
		ZVAL_LONG(rv, defer->state->line);
	} else {
		rv = &EG(uninitialized_zval);
	}

	return rv;
}
#endif

static ASYNC_DEBUG_INFO_HANDLER(deferred_debug_info)
{
	async_deferred *defer;
	zval info;

	*temp = 1;

	defer = async_deferred_obj(ASYNC_DEBUG_INFO_OBJ());

	debug_deferred_state(defer->state, &info);

	return Z_ARRVAL(info);
}

static HashTable *deferred_get_props(zval *obj)
{
	async_deferred *defer;

	defer = async_deferred_obj(Z_OBJ_P(obj));

	refresh_props(&defer->std, defer->state, async_deferred_prop_offset);

	return defer->std.properties;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_deferred_ctor, 0, 0, 0)
	ZEND_ARG_CALLABLE_INFO(0, cancel, 1)
ZEND_END_ARG_INFO();

static PHP_METHOD(Deferred, __construct)
{
	async_deferred *defer;
	async_context *context;
	
	defer = async_deferred_obj(Z_OBJ_P(getThis()));

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_FUNC_EX(defer->fci, defer->fcc, 1, 0)
	ZEND_PARSE_PARAMETERS_END();
	
	capture_call_context(defer->state, EX(prev_execute_data));

	if (ZEND_NUM_ARGS() > 0) {
		context = defer->state->context;

		if (context->cancel != NULL) {
			if (context->cancel->flags & ASYNC_CONTEXT_CANCELLATION_FLAG_TRIGGERED) {
				cancel_defer(defer, &context->cancel->error);
			} else {
				defer->cancel.object = defer;
				defer->cancel.func = cancel_defer;
				
				ASYNC_ADDREF_CB(defer->fci);

				ASYNC_LIST_APPEND(&context->cancel->callbacks, &defer->cancel);
			}
		}
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_deferred_awaitable, 0, 0, Phalcon\\Async\\Awaitable, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Deferred, awaitable)
{
	async_deferred *defer;

	ZEND_PARSE_PARAMETERS_NONE();

	defer = async_deferred_obj(Z_OBJ_P(getThis()));

	RETURN_OBJ(&async_deferred_awaitable_object_create(defer->state)->std);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_deferred_resolve, 0, 0, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO();

static PHP_METHOD(Deferred, resolve)
{
	async_deferred *defer;

	zval *val;

	val = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();

	if (val != NULL && Z_TYPE_P(val) == IS_OBJECT) {
		if (UNEXPECTED(instanceof_function(Z_OBJCE_P(val), async_awaitable_ce) != 0)) {
			zend_throw_error(NULL, "Deferred must not be resolved with an object implementing Awaitable");
			return;
		}
	}

	defer = async_deferred_obj(Z_OBJ_P(getThis()));

	if (UNEXPECTED(defer->state->status != ASYNC_DEFERRED_STATUS_PENDING)) {
		return;
	}

	if (EXPECTED(val != NULL)) {
		ZVAL_COPY(&defer->state->result, val);
	}

	defer->state->status = ASYNC_DEFERRED_STATUS_RESOLVED;

	cleanup_cancel(defer);
	
	trigger_ops(defer->state);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_deferred_fail, 0, 0, IS_VOID, 1)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Deferred, fail)
{
	async_deferred *defer;

	zval *error;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_OBJECT_OF_CLASS(error, zend_ce_throwable)
	ZEND_PARSE_PARAMETERS_END();

	defer = async_deferred_obj(Z_OBJ_P(getThis()));

	if (UNEXPECTED(defer->state->status != ASYNC_DEFERRED_STATUS_PENDING)) {
		return;
	}

	ZVAL_COPY(&defer->state->result, error);

	defer->state->status = ASYNC_DEFERRED_STATUS_FAILED;

	cleanup_cancel(defer);
	
	trigger_ops(defer->state);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_deferred_value, 0, 0, Phalcon\\Async\\Awaitable, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO();

static PHP_METHOD(Deferred, value)
{
	zval *val;

	val = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();

	if (val != NULL && Z_TYPE_P(val) == IS_OBJECT) {
		if (UNEXPECTED(instanceof_function(Z_OBJCE_P(val), async_awaitable_ce) != 0)) {
			zend_throw_error(NULL, "Deferred must not be resolved with an object implementing Awaitable");
			return;
		}
	}

	RETURN_OBJ(&async_create_resolved_awaitable(EX(prev_execute_data), val)->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_deferred_error, 0, 1, Phalcon\\Async\\Awaitable, 0)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Deferred, error)
{
	zval *error;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(error)
	ZEND_PARSE_PARAMETERS_END();
	
	RETURN_OBJ(&async_create_failed_awaitable(EX(prev_execute_data), error)->std);
}

ASYNC_CALLBACK combine_cb(async_op *op)
{
	async_defer_combine_op *cb;
	async_defer_combine *combined;
	async_deferred_state *state;
	uint32_t i;

	zval args[5];
	zval retval;
	zend_object *error;

	cb = (async_defer_combine_op *) op;

	ZEND_ASSERT(cb != NULL);

	combined = cb->combine;

	ZVAL_OBJ(&args[0], &combined->defer->std);
	ASYNC_ADDREF(&combined->defer->std);

	ZVAL_BOOL(&args[1], 0 == --combined->started);
	ZVAL_COPY(&args[2], &cb->key);

	zval_ptr_dtor(&cb->key);

	if (op->status == ASYNC_STATUS_RESOLVED) {
		ZVAL_NULL(&args[3]);
		ZVAL_COPY(&args[4], &op->result);
	} else {
		ZVAL_COPY(&args[3], &op->result);
		ZVAL_NULL(&args[4]);
	}

	combined->fci.param_count = 5;
	combined->fci.params = args;
	combined->fci.retval = &retval;

	error = EG(exception);

	if (UNEXPECTED(error != NULL)) {
		ASYNC_ADDREF(error);
		zend_clear_exception();
	}

	state = combined->defer->state;

	zend_try {
		async_call_nowait(EG(current_execute_data), &combined->fci, &combined->fcc);
	} zend_catch {
		async_task_scheduler_handle_exit(state->scheduler);
		return;
	} zend_end_try();

	for (i = 0; i < 5; i++) {
		zval_ptr_dtor(&args[i]);
	}

	zval_ptr_dtor(&retval);

	if (cb->awaitable != NULL) {
		ASYNC_DELREF(&cb->awaitable->std);
	}

	ASYNC_FREE_OP(op);

	if (UNEXPECTED(EG(exception))) {
		if (state->status == ASYNC_DEFERRED_STATUS_PENDING) {
			state->status = ASYNC_DEFERRED_STATUS_FAILED;

			ZVAL_OBJ(&state->result, EG(exception));
			Z_ADDREF(state->result);

			zend_clear_exception();

			trigger_ops(state);
		} else {
			zend_clear_exception();
		}
	}

	if (0 == --combined->counter) {
		if (state->status == ASYNC_DEFERRED_STATUS_PENDING) {
			state->status = ASYNC_DEFERRED_STATUS_FAILED;

			ASYNC_PREPARE_SCHEDULER_ERROR(&state->result, "Awaitable has been disposed before it was resolved");

			trigger_ops(state);
		}

		ASYNC_DELREF_CB(combined->fci);
		ASYNC_DELREF(&combined->defer->std);

		efree(combined);
	}

	EG(exception) = error;
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_deferred_combine, 0, 2, Phalcon\\Async\\Awaitable, 0)
	ZEND_ARG_ARRAY_INFO(0, awaitables, 0)
	ZEND_ARG_CALLABLE_INFO(0, continuation, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Deferred, combine)
{
	async_deferred *defer;
	async_deferred_awaitable *awaitable;
	async_defer_combine *combined;
	async_defer_combine_op *op;

	async_await_handler *handler;
	async_context *context;

	zend_class_entry *ce;
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	uint32_t count;

	zval *args;
	zend_ulong i;
	zend_string *k;
	zval *entry;
	zval result;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_ARRAY(args)
		Z_PARAM_FUNC_EX(fci, fcc, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	count = zend_array_count(Z_ARRVAL_P(args));

	if (UNEXPECTED(count == 0)) {
		zend_throw_error(zend_ce_argument_count_error, "At least one awaitable is required");
		return;
	}

#if PHP_VERSION_ID < 80000
	fci.no_separation = 1;
#endif

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(args), entry) {
		ce = (Z_TYPE_P(entry) == IS_OBJECT) ? Z_OBJCE_P(entry) : NULL;

		if (UNEXPECTED(ce == NULL || !instanceof_function(ce, async_awaitable_ce))) {
			zend_throw_error(zend_ce_type_error, "All input elements must be awaitable");
			return;
		}
	} ZEND_HASH_FOREACH_END();

	defer = async_deferred_obj(async_deferred_object_create(async_deferred_ce));
	awaitable = async_deferred_awaitable_object_create(defer->state);

	capture_call_context(defer->state, EX(prev_execute_data));

	combined = emalloc(sizeof(async_defer_combine));
	combined->defer = defer;
	combined->counter = count;
	combined->started = count;
	combined->fci = fci;
	combined->fcc = fcc;

	ASYNC_ADDREF_CB(combined->fci);

	context = async_context_get();

	ZEND_HASH_FOREACH_KEY_VAL_IND(Z_ARRVAL_P(args), i, k, entry) {
		handler = async_get_await_handler(Z_OBJCE_P(entry));

		ZEND_ASSERT(handler != NULL);

		ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_defer_combine_op));
		
		op->combine = combined;
		op->base.callback = combine_cb;

		if (k == NULL) {
			ZVAL_LONG(&op->key, i);
		} else {
			ZVAL_STR_COPY(&op->key, k);
		}

		switch (handler->delegate(Z_OBJ_P(entry), &result, NULL, context, 0)) {
		case ASYNC_OP_RESOLVED:
			ASYNC_RESOLVE_OP(op, &result);
			zval_ptr_dtor(&result);
			break;
		case ASYNC_OP_FAILED:
			ASYNC_FAIL_OP(op, &result);
			zval_ptr_dtor(&result);
			break;
		default:
			handler->schedule(Z_OBJ_P(entry), (async_op *) op, NULL, context, 0);
		}
	} ZEND_HASH_FOREACH_END();

	RETURN_OBJ(&awaitable->std);
}

ASYNC_CALLBACK transform_cb(async_op *op)
{
	async_defer_transform_op *trans;
	
	trans = (async_defer_transform_op *) op;
	
	zval args[2];
	zval retval;
	zend_object *error;
	
	if (op->status == ASYNC_STATUS_RESOLVED) {
		ZVAL_NULL(&args[0]);
		ZVAL_COPY(&args[1], &op->result);
	} else {		
		ZVAL_COPY(&args[0], &op->result);
		ZVAL_NULL(&args[1]);
	}

	trans->fci.param_count = 2;
	trans->fci.params = args;
	trans->fci.retval = &retval;

	error = EG(exception);

	if (UNEXPECTED(error != NULL)) {
		ASYNC_ADDREF(error);
		zend_clear_exception();
	}

	zend_try {
		async_call_nowait(EG(current_execute_data), &trans->fci, &trans->fcc);
	} zend_catch {
		async_task_scheduler_handle_exit(trans->state->scheduler);
		return;
	} zend_end_try();

	zval_ptr_dtor(&args[0]);
	zval_ptr_dtor(&args[1]);

	if (UNEXPECTED(EG(exception))) {
		trans->state->status = ASYNC_DEFERRED_STATUS_FAILED;

		ZVAL_OBJ(&trans->state->result, EG(exception));
		Z_ADDREF(trans->state->result);

		zend_clear_exception();
	} else {
		trans->state->status = ASYNC_DEFERRED_STATUS_RESOLVED;

		ZVAL_COPY(&trans->state->result, &retval);
	}
	
	trigger_ops(trans->state);

	zval_ptr_dtor(&retval);

	release_state(trans->state);
	
	ASYNC_DELREF_CB(trans->fci);

	EG(exception) = error;
	
	ASYNC_FREE_OP(op);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_deferred_transform, 0, 2, Phalcon\\Async\\Awaitable, 0)
	ZEND_ARG_OBJ_INFO(0, awaitable, Phalcon\\Async\\Awaitable, 0)
	ZEND_ARG_CALLABLE_INFO(0, continuation, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Deferred, transform)
{
	async_deferred_state *state;
	async_deferred_awaitable *awaitable;
	async_defer_transform_op *op;

	async_context *context;
	async_await_handler *handler;

	zend_fcall_info fci;
	zend_fcall_info_cache fcc;

	zval *val;
	zval result;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_OBJECT_OF_CLASS(val, async_awaitable_ce)
		Z_PARAM_FUNC_EX(fci, fcc, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	handler = async_get_await_handler(Z_OBJCE_P(val));
	context = async_context_get();

	ZEND_ASSERT(handler != NULL);

	state = create_state(context);
	capture_call_context(state, EX(prev_execute_data));

	awaitable = async_deferred_awaitable_object_create(state);
	
	ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_defer_transform_op));
	
	op->base.callback = transform_cb;
	op->state = state;
	op->fci = fci;
	op->fcc = fcc;

	ASYNC_ADDREF_CB(op->fci);

	switch (handler->delegate(Z_OBJ_P(val), &result, NULL, context, 0)) {
	case ASYNC_OP_RESOLVED:
		ASYNC_RESOLVE_OP(op, &result);
		zval_ptr_dtor(&result);
		break;
	case ASYNC_OP_FAILED:
		ASYNC_FAIL_OP(op, &result);
		zval_ptr_dtor(&result);
		break;
	default:
		handler->schedule(Z_OBJ_P(val), (async_op *) op, NULL, context, 0);
	}

	RETURN_OBJ(&awaitable->std);
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_WAKEUP(Deferred, async_deferred_ce)
//LCOV_EXCL_STOP

static const zend_function_entry deferred_functions[] = {
	PHP_ME(Deferred, __construct, arginfo_deferred_ctor, ZEND_ACC_PUBLIC)
	PHP_ME(Deferred, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(Deferred, awaitable, arginfo_deferred_awaitable, ZEND_ACC_PUBLIC)
	PHP_ME(Deferred, resolve, arginfo_deferred_resolve, ZEND_ACC_PUBLIC)
	PHP_ME(Deferred, fail, arginfo_deferred_fail, ZEND_ACC_PUBLIC)
	PHP_ME(Deferred, value, arginfo_deferred_value, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Deferred, error, arginfo_deferred_error, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Deferred, combine, arginfo_deferred_combine, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Deferred, transform, arginfo_deferred_transform, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_FE_END
};


void async_deferred_ce_register()
{
	zend_class_entry ce;

#if PHP_VERSION_ID >= 70400
	zval tmp;
#endif

	str_status = zend_new_interned_string(zend_string_init(ZEND_STRL("status"), 1));
	str_file = zend_new_interned_string(zend_string_init(ZEND_STRL("file"), 1));
	str_line = zend_new_interned_string(zend_string_init(ZEND_STRL("line"), 1));

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async", "Deferred", deferred_functions);
	async_deferred_ce = zend_register_internal_class(&ce);
	async_deferred_ce->ce_flags |= ZEND_ACC_FINAL;
	async_deferred_ce->create_object = async_deferred_object_create;
	async_deferred_ce->serialize = zend_class_serialize_deny;
	async_deferred_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_deferred_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_deferred_handlers.offset = XtOffsetOf(async_deferred, std);
	async_deferred_handlers.free_obj = async_deferred_object_destroy;
	async_deferred_handlers.clone_obj = NULL;
	async_deferred_handlers.has_property = deferred_has_prop;
	async_deferred_handlers.read_property = deferred_read_prop;
	async_deferred_handlers.write_property = async_prop_write_handler_readonly;
	async_deferred_handlers.get_debug_info = deferred_debug_info;
	async_deferred_handlers.get_properties = deferred_get_props;

#if PHP_VERSION_ID >= 80000
	ZVAL_STRING(&tmp, "");
	zend_declare_typed_property(async_deferred_ce, str_status, &tmp, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CODE(IS_STRING, 0, 0));
	zval_ptr_dtor(&tmp);

	ZVAL_NULL(&tmp);
	zend_declare_typed_property(async_deferred_ce, str_file, &tmp, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CODE(IS_STRING, 1, 0));
	zend_declare_typed_property(async_deferred_ce, str_line, &tmp, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CODE(IS_LONG, 1, 0));
#elif PHP_VERSION_ID < 70400
	zend_declare_property_null(async_deferred_ce, ZEND_STRL("status"), ZEND_ACC_PUBLIC);
	zend_declare_property_null(async_deferred_ce, ZEND_STRL("file"), ZEND_ACC_PUBLIC);
	zend_declare_property_null(async_deferred_ce, ZEND_STRL("line"), ZEND_ACC_PUBLIC);
#else
	ZVAL_STRING(&tmp, "");
	zend_declare_typed_property(async_deferred_ce, str_status, &tmp, ZEND_ACC_PUBLIC, NULL, IS_STRING);
	zval_ptr_dtor(&tmp);

	ZVAL_NULL(&tmp);
	zend_declare_typed_property(async_deferred_ce, str_file, &tmp, ZEND_ACC_PUBLIC, NULL, ZEND_TYPE_ENCODE(IS_STRING, 1));
	zend_declare_typed_property(async_deferred_ce, str_line, &tmp, ZEND_ACC_PUBLIC, NULL, ZEND_TYPE_ENCODE(IS_LONG, 1));
#endif

	off_defer_status = async_deferred_prop_offset(str_status);
	off_defer_file = async_deferred_prop_offset(str_file);
	off_defer_line = async_deferred_prop_offset(str_line);

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async", "DeferredAwaitable", deferred_awaitable_functions);
	async_deferred_awaitable_ce = zend_register_internal_class(&ce);
	async_deferred_awaitable_ce->ce_flags |= ZEND_ACC_FINAL;
	async_deferred_awaitable_ce->serialize = zend_class_serialize_deny;
	async_deferred_awaitable_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_deferred_awaitable_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_deferred_awaitable_handlers.offset = XtOffsetOf(async_deferred_awaitable, std);
	async_deferred_awaitable_handlers.free_obj = async_deferred_awaitable_object_destroy;
	async_deferred_awaitable_handlers.clone_obj = NULL;
	async_deferred_awaitable_handlers.has_property = deferred_awaitable_has_prop;
	async_deferred_awaitable_handlers.read_property = deferred_awaitable_read_prop;
	async_deferred_awaitable_handlers.write_property = async_prop_write_handler_readonly;
	async_deferred_awaitable_handlers.get_debug_info = deferred_awaitable_debug_info;
	async_deferred_awaitable_handlers.get_properties = deferred_awaitable_get_props;

#if PHP_VERSION_ID >= 80000
	ZVAL_STRING(&tmp, "");
	zend_declare_typed_property(async_deferred_awaitable_ce, str_status, &tmp, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CODE(IS_STRING, 0, 0));
	zval_ptr_dtor(&tmp);

	ZVAL_NULL(&tmp);
	zend_declare_typed_property(async_deferred_awaitable_ce, str_file, &tmp, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CODE(IS_STRING, 1, 0));
	zend_declare_typed_property(async_deferred_awaitable_ce, str_line, &tmp, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CODE(IS_LONG, 1, 0));
#elif PHP_VERSION_ID < 70400
	zend_declare_property_null(async_deferred_awaitable_ce, ZEND_STRL("status"), ZEND_ACC_PUBLIC);
	zend_declare_property_null(async_deferred_awaitable_ce, ZEND_STRL("file"), ZEND_ACC_PUBLIC);
	zend_declare_property_null(async_deferred_awaitable_ce, ZEND_STRL("line"), ZEND_ACC_PUBLIC);
#else
	ZVAL_STRING(&tmp, "");
	zend_declare_typed_property(async_deferred_awaitable_ce, str_status, &tmp, ZEND_ACC_PUBLIC, NULL, IS_STRING);
	zval_ptr_dtor(&tmp);

	ZVAL_NULL(&tmp);
	zend_declare_typed_property(async_deferred_awaitable_ce, str_file, &tmp, ZEND_ACC_PUBLIC, NULL, ZEND_TYPE_ENCODE(IS_STRING, 1));
	zend_declare_typed_property(async_deferred_awaitable_ce, str_line, &tmp, ZEND_ACC_PUBLIC, NULL, ZEND_TYPE_ENCODE(IS_LONG, 1));
#endif

	off_awaitable_status = async_deferred_awaitable_prop_offset(str_status);
	off_awaitable_file = async_deferred_awaitable_prop_offset(str_file);
	off_awaitable_line = async_deferred_awaitable_prop_offset(str_line);

	deferred_await_handler.delegate = deferred_await_delegate;
	deferred_await_handler.schedule = deferred_await_schedule;

	async_register_awaitable(async_deferred_awaitable_ce, &deferred_await_handler);
}

void async_deferred_ce_unregister()
{
	zend_string_release(str_status);
	zend_string_release(str_file);
	zend_string_release(str_line);
}
