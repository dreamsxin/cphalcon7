
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

#if PHALCON_USE_UV

#include "kernel/backend.h"

ASYNC_API zend_class_entry *async_deferred_ce;
ASYNC_API zend_class_entry *async_deferred_awaitable_ce;

static zend_object_handlers async_deferred_handlers;
static zend_object_handlers async_deferred_awaitable_handlers;
static zend_object_handlers async_deferred_custom_awaitable_handlers;

#define ASYNC_DEFERRED_STATUS_PENDING 0
#define ASYNC_DEFERRED_STATUS_RESOLVED ASYNC_OP_RESOLVED
#define ASYNC_DEFERRED_STATUS_FAILED ASYNC_OP_FAILED


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
			ASYNC_PREPARE_ERROR(&state->result, "Awaitable has been disposed before it was resolved");

			while (state->operations.first != NULL) {
				ASYNC_NEXT_OP(&state->operations, op);
				ASYNC_FAIL_OP(op, &state->result);
			}
		}
	}

	zval_ptr_dtor(&state->result);

	ASYNC_DELREF(&state->context->std);
	async_task_scheduler_unref(state->scheduler);

	efree(state);
}

static zend_always_inline void cleanup_cancel(async_deferred *defer)
{
	if (defer->cancel.object != NULL && Z_TYPE_P(&defer->fci.function_name) != IS_UNDEF) {
		ASYNC_LIST_REMOVE(&defer->state->context->cancel->callbacks, &defer->cancel);
		defer->cancel.object = NULL;
	}
}

static zend_always_inline void register_defer_op(async_op *op, async_deferred_state *state)
{
	if (state->status == ASYNC_OP_RESOLVED) {
		ASYNC_RESOLVE_OP(op, &state->result);
	} else if (state->status == ASYNC_OP_FAILED) {
		ASYNC_FAIL_OP(op, &state->result);
	} else {
		ASYNC_APPEND_OP(&state->operations, op);
	}
}

static zend_always_inline void register_task_op(async_op *op, async_task *task)
{
	if (task->status == ASYNC_OP_RESOLVED) {
		ASYNC_RESOLVE_OP(op, &task->result);
	} else if (task->status == ASYNC_OP_FAILED) {
		ASYNC_FAIL_OP(op, &task->result);
	} else {
		ASYNC_APPEND_OP(&task->operations, op);
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
	defer->fci.no_separation = 1;

	async_call_nowait(&defer->fci, &defer->fcc);

	zval_ptr_dtor(&args[0]);
	zval_ptr_dtor(&args[1]);
	zval_ptr_dtor(&retval);

	ASYNC_DELREF_CB(defer->fci);

	ASYNC_CHECK_FATAL(EG(exception), "Must not throw an error from cancellation handler");
}


static async_deferred_awaitable *async_deferred_awaitable_object_create(async_deferred_state *state)
{
	async_deferred_awaitable *awaitable;

	awaitable = ecalloc(1, sizeof(async_deferred_awaitable));

	zend_object_std_init(&awaitable->std, async_deferred_awaitable_ce);
	awaitable->std.handlers = &async_deferred_awaitable_handlers;

	awaitable->state = state;

	state->refcount++;

	return awaitable;
}

static void async_deferred_awaitable_object_destroy(zend_object *object)
{
	async_deferred_awaitable *awaitable;

	awaitable = (async_deferred_awaitable *) object;

	release_state(awaitable->state);

	zend_object_std_dtor(&awaitable->std);
}

static ZEND_METHOD(DeferredAwaitable, __construct)
{
	ZEND_PARSE_PARAMETERS_NONE();

	zend_throw_error(NULL, "Deferred awaitable must not be created from userland code");
}

static ZEND_METHOD(DeferredAwaitable, __debugInfo)
{
	async_deferred_state *state;

	ZEND_PARSE_PARAMETERS_NONE();

	state = ((async_deferred_awaitable *) Z_OBJ_P(getThis()))->state;

	if (USED_RET()) {
		array_init(return_value);

		add_assoc_string(return_value, "status", async_status_label(state->status));
		
		if (state->status != ASYNC_DEFERRED_STATUS_PENDING) {
			Z_TRY_ADDREF_P(&state->result);
		
			add_assoc_zval(return_value, "result", &state->result);
		}
	}
}

ZEND_BEGIN_ARG_INFO(arginfo_deferred_awaitable_debug_info, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_deferred_awaitable_ctor, 0, 0, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry deferred_awaitable_functions[] = {
	ZEND_ME(DeferredAwaitable, __construct, arginfo_deferred_awaitable_ctor, ZEND_ACC_PRIVATE)
	ZEND_ME(DeferredAwaitable, __debugInfo, arginfo_deferred_awaitable_debug_info, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static zend_object *async_deferred_object_create(zend_class_entry *ce)
{
	async_deferred *defer;

	defer = ecalloc(1, sizeof(async_deferred));

	zend_object_std_init(&defer->std, ce);
	defer->std.handlers = &async_deferred_handlers;
	
	defer->state = create_state(async_context_get());

	return &defer->std;
}

static void async_deferred_object_destroy(zend_object *object)
{
	async_deferred *defer;

	defer = (async_deferred *) object;

	cleanup_cancel(defer);
	
	if (defer->state->status == ASYNC_DEFERRED_STATUS_PENDING) {
		defer->state->status = ASYNC_OP_FAILED;
		ASYNC_PREPARE_ERROR(&defer->state->result, "Awaitable has been disposed before it was resolved");

		trigger_ops(defer->state);
	}
	
	release_state(defer->state);

	zend_object_std_dtor(&defer->std);
}

static ZEND_METHOD(Deferred, __construct)
{
	async_deferred *defer;
	async_context *context;
	
	defer = (async_deferred *) Z_OBJ_P(getThis());

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_FUNC_EX(defer->fci, defer->fcc, 1, 0)
	ZEND_PARSE_PARAMETERS_END();
	
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

static ZEND_METHOD(Deferred, __debugInfo)
{
	async_deferred *defer;

	ZEND_PARSE_PARAMETERS_NONE();

	defer = (async_deferred *) Z_OBJ_P(getThis());

	if (USED_RET()) {
		array_init(return_value);

		add_assoc_string(return_value, "status", async_status_label(defer->state->status));
		
		if (defer->state->status != ASYNC_DEFERRED_STATUS_PENDING) {
			Z_TRY_ADDREF_P(&defer->state->result);
		
			add_assoc_zval(return_value, "result", &defer->state->result);
		}
	}
}

static ZEND_METHOD(Deferred, awaitable)
{
	async_deferred *defer;

	ZEND_PARSE_PARAMETERS_NONE();

	defer = (async_deferred *) Z_OBJ_P(getThis());

	RETURN_OBJ(&async_deferred_awaitable_object_create(defer->state)->std);
}

static ZEND_METHOD(Deferred, resolve)
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

	defer = (async_deferred *) Z_OBJ_P(getThis());

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

static ZEND_METHOD(Deferred, fail)
{
	async_deferred *defer;

	zval *error;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(error)
	ZEND_PARSE_PARAMETERS_END();

	defer = (async_deferred *) Z_OBJ_P(getThis());

	if (UNEXPECTED(defer->state->status != ASYNC_DEFERRED_STATUS_PENDING)) {
		return;
	}

	ZVAL_COPY(&defer->state->result, error);

	defer->state->status = ASYNC_DEFERRED_STATUS_FAILED;

	cleanup_cancel(defer);
	
	trigger_ops(defer->state);
}

static ZEND_METHOD(Deferred, value)
{
	async_deferred_state *state;
	async_deferred_awaitable *awaitable;

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

	state = create_state(async_context_get());
	awaitable = async_deferred_awaitable_object_create(state);

	state->status = ASYNC_DEFERRED_STATUS_RESOLVED;
	state->refcount--;

	if (EXPECTED(val != NULL)) {
		ZVAL_COPY(&state->result, val);
	}

	RETURN_OBJ(&awaitable->std);
}

static ZEND_METHOD(Deferred, error)
{
	async_deferred_state *state;
	async_deferred_awaitable *awaitable;

	zval *error;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(error)
	ZEND_PARSE_PARAMETERS_END();

	state = create_state(async_context_get());
	awaitable = async_deferred_awaitable_object_create(state);
	
	state->status = ASYNC_DEFERRED_STATUS_FAILED;
	state->refcount--;

	ZVAL_COPY(&state->result, error);

	RETURN_OBJ(&awaitable->std);
}

typedef struct {
	async_deferred *defer;
	uint32_t counter;
	uint32_t started;
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
} async_defer_combine;

typedef struct {
	async_op base;
	async_defer_combine *combine;
	async_deferred_awaitable *awaitable;
	zval key;
} async_defer_combine_op;

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

	async_call_nowait(&combined->fci, &combined->fcc);

	for (i = 0; i < 5; i++) {
		zval_ptr_dtor(&args[i]);
	}

	zval_ptr_dtor(&retval);

	if (cb->awaitable != NULL) {
		ASYNC_DELREF(&cb->awaitable->std);
	}

	ASYNC_FREE_OP(op);

	state = combined->defer->state;

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

			ASYNC_PREPARE_ERROR(&state->result, "Awaitable has been disposed before it was resolved");

			trigger_ops(state);
		}

		ASYNC_DELREF_CB(combined->fci);
		ASYNC_DELREF(&combined->defer->std);

		efree(combined);
	}

	EG(exception) = error;
}

static ZEND_METHOD(Deferred, combine)
{
	async_deferred *defer;
	async_deferred_awaitable *awaitable;
	async_defer_combine *combined;
	async_defer_combine_op *op;

	zend_class_entry *ce;
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	uint32_t count;

	zval *args;
	zend_ulong i;
	zend_string *k;
	zval *entry;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_ARRAY(args)
		Z_PARAM_FUNC_EX(fci, fcc, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	count = zend_array_count(Z_ARRVAL_P(args));

	if (UNEXPECTED(count == 0)) {
#if PHP_VERSION_ID < 70100
#else
		zend_throw_error(zend_ce_error, "At least one awaitable is required");
#endif
		return;
	}

	fci.no_separation = 1;

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(args), entry) {
		ce = (Z_TYPE_P(entry) == IS_OBJECT) ? Z_OBJCE_P(entry) : NULL;

		if (UNEXPECTED(ce != async_task_ce && ce != async_deferred_awaitable_ce)) {
			zend_throw_error(zend_ce_type_error, "All input elements must be awaitable");
			return;
		}
	} ZEND_HASH_FOREACH_END();

	defer = (async_deferred *) async_deferred_object_create(async_deferred_ce);
	awaitable = async_deferred_awaitable_object_create(defer->state);

	combined = emalloc(sizeof(async_defer_combine));
	combined->defer = defer;
	combined->counter = count;
	combined->started = count;
	combined->fci = fci;
	combined->fcc = fcc;

	ASYNC_ADDREF_CB(combined->fci);

	ZEND_HASH_FOREACH_KEY_VAL_IND(Z_ARRVAL_P(args), i, k, entry) {
		ce = Z_OBJCE_P(entry);
		
		ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_defer_combine_op));
		
		op->combine = combined;
		op->base.callback = combine_cb;

		if (k == NULL) {
			ZVAL_LONG(&op->key, i);
		} else {
			ZVAL_STR_COPY(&op->key, k);
		}

		if (ce == async_task_ce) {
			register_task_op((async_op *) op, (async_task *) Z_OBJ_P(entry));
		} else {
			op->awaitable = (async_deferred_awaitable *) Z_OBJ_P(entry);

			// Keep a reference to the awaitable because the args array could be garbage collected before combine resolves.
			ASYNC_ADDREF(&op->awaitable->std);

			register_defer_op((async_op *) op, op->awaitable->state);
		}
	} ZEND_HASH_FOREACH_END();

	RETURN_OBJ(&awaitable->std);
}

typedef struct {
	async_op base;
	async_deferred_state *state;
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;	
} async_defer_transform_op;

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

	async_call_nowait(&trans->fci, &trans->fcc);

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

static ZEND_METHOD(Deferred, transform)
{
	async_deferred_state *state;
	async_deferred_awaitable *awaitable;
	async_defer_transform_op *op;

	zend_class_entry *ce;
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;

	zval *val;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_ZVAL(val)
		Z_PARAM_FUNC_EX(fci, fcc, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	state = create_state(async_context_get());
	awaitable = async_deferred_awaitable_object_create(state);
	
	ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_defer_transform_op));
	
	op->base.callback = transform_cb;
	op->state = state;
	op->fci = fci;
	op->fcc = fcc;

	ASYNC_ADDREF_CB(op->fci);

	ce = Z_OBJCE_P(val);

	if (ce == async_task_ce) {
		register_task_op((async_op *) op, (async_task *) Z_OBJ_P(val));
	} else {
		register_defer_op((async_op *) op, ((async_deferred_awaitable *) Z_OBJ_P(val))->state);
	}

	RETURN_OBJ(&awaitable->std);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_deferred_ctor, 0, 0, 0)
	ZEND_ARG_CALLABLE_INFO(0, cancel, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_deferred_debug_info, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_deferred_awaitable, 0, 0, Phalcon\\Async\\Awaitable, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_deferred_resolve, 0, 0, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_deferred_fail, 0, 0, IS_VOID, 1)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 0)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_deferred_fail, 0, 0, IS_VOID, NULL, 1)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_deferred_value, 0, 0, Phalcon\\Async\\Awaitable, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_deferred_error, 0, 1, Phalcon\\Async\\Awaitable, 0)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_deferred_combine, 0, 2, Phalcon\\Async\\Awaitable, 0)
	ZEND_ARG_ARRAY_INFO(0, awaitables, 0)
	ZEND_ARG_CALLABLE_INFO(0, continuation, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_deferred_transform, 0, 2, Phalcon\\Async\\Awaitable, 0)
	ZEND_ARG_OBJ_INFO(0, awaitable, Phalcon\\Async\\Awaitable, 0)
	ZEND_ARG_CALLABLE_INFO(0, continuation, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry deferred_functions[] = {
	ZEND_ME(Deferred, __construct, arginfo_deferred_ctor, ZEND_ACC_PUBLIC)
	ZEND_ME(Deferred, __debugInfo, arginfo_deferred_debug_info, ZEND_ACC_PUBLIC)
	ZEND_ME(Deferred, awaitable, arginfo_deferred_awaitable, ZEND_ACC_PUBLIC)
	ZEND_ME(Deferred, resolve, arginfo_deferred_resolve, ZEND_ACC_PUBLIC)
	ZEND_ME(Deferred, fail, arginfo_deferred_fail, ZEND_ACC_PUBLIC)
	ZEND_ME(Deferred, value, arginfo_deferred_value, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(Deferred, error, arginfo_deferred_error, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(Deferred, combine, arginfo_deferred_combine, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(Deferred, transform, arginfo_deferred_transform, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_FE_END
};


ASYNC_API void async_init_awaitable(async_deferred_custom_awaitable *awaitable, void (* dtor)(async_deferred_awaitable *awaitable), async_context *context)
{
	if (EXPECTED(context == NULL)) {
		context = async_context_get();
	}
	
	zend_object_std_init(&awaitable->base.std, async_deferred_awaitable_ce);
	awaitable->base.std.handlers = &async_deferred_custom_awaitable_handlers;

	awaitable->base.state = create_state(context);
	awaitable->dtor = dtor;
}

ASYNC_API void async_resolve_awaitable(async_deferred_awaitable *awaitable, zval *val)
{
	if (EXPECTED(awaitable->state->status == ASYNC_DEFERRED_STATUS_PENDING)) {
		awaitable->state->status = ASYNC_DEFERRED_STATUS_RESOLVED;
		
		if (UNEXPECTED(val == NULL)) {
			ZVAL_NULL(&awaitable->state->result);
		} else {
			ZVAL_COPY(&awaitable->state->result, val);
		}
		
		trigger_ops(awaitable->state);
	}
}

ASYNC_API void async_fail_awaitable(async_deferred_awaitable *awaitable, zval *error)
{
	if (EXPECTED(awaitable->state->status == ASYNC_DEFERRED_STATUS_PENDING)) {
		awaitable->state->status = ASYNC_DEFERRED_STATUS_FAILED;
		
		ZVAL_COPY(&awaitable->state->result, error);
		
		trigger_ops(awaitable->state);
	}
}

static void async_deferred_custom_awaitable_object_destroy(zend_object *object)
{
	async_deferred_custom_awaitable *awaitable;
	
	awaitable = (async_deferred_custom_awaitable *) object;
	
	if (UNEXPECTED(awaitable->base.state->status == ASYNC_DEFERRED_STATUS_PENDING)) {
		awaitable->dtor((async_deferred_awaitable *) awaitable);
	}
}


void async_deferred_ce_register()
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Deferred", deferred_functions);
	async_deferred_ce = zend_register_internal_class(&ce);
	async_deferred_ce->ce_flags |= ZEND_ACC_FINAL;
	async_deferred_ce->create_object = async_deferred_object_create;
	async_deferred_ce->serialize = zend_class_serialize_deny;
	async_deferred_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_deferred_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_deferred_handlers.free_obj = async_deferred_object_destroy;
	async_deferred_handlers.clone_obj = NULL;

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\DeferredAwaitable", deferred_awaitable_functions);
	async_deferred_awaitable_ce = zend_register_internal_class(&ce);
	async_deferred_awaitable_ce->ce_flags |= ZEND_ACC_FINAL;
	async_deferred_awaitable_ce->serialize = zend_class_serialize_deny;
	async_deferred_awaitable_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_deferred_awaitable_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_deferred_awaitable_handlers.free_obj = async_deferred_awaitable_object_destroy;
	async_deferred_awaitable_handlers.clone_obj = NULL;
	
	memcpy(&async_deferred_custom_awaitable_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_deferred_custom_awaitable_handlers.free_obj = async_deferred_awaitable_object_destroy;
	async_deferred_custom_awaitable_handlers.dtor_obj = async_deferred_custom_awaitable_object_destroy;
	async_deferred_custom_awaitable_handlers.clone_obj = NULL;

	zend_class_implements(async_deferred_awaitable_ce, 1, async_awaitable_ce);
}

#endif
