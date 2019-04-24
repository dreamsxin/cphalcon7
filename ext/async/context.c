
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
#include "async/async_fiber.h"

ASYNC_API zend_class_entry *async_context_ce;
ASYNC_API zend_class_entry *async_context_var_ce;
ASYNC_API zend_class_entry *async_cancellation_handler_ce;
ASYNC_API zend_class_entry *async_cancellation_exception_ce;

static zend_object_handlers async_context_handlers;
static zend_object_handlers async_context_var_handlers;
static zend_object_handlers async_cancellation_handler_handlers;

static async_context *async_context_object_create(async_context_var *var, zval *value);
static async_cancellation_handler *async_cancellation_handler_object_create(async_context_cancellation *cancel);


static zend_always_inline async_context_cancellation *init_cancellation()
{
	async_context_cancellation *cancel;
	
	cancel = ecalloc(1, sizeof(async_context_cancellation));
	
	ZVAL_UNDEF(&cancel->error);
	
	return cancel;
}

static zend_always_inline async_context_timeout *init_timeout_cancellation()
{
	async_context_timeout *cancel;
	
	cancel = ecalloc(1, sizeof(async_context_timeout));
	
	cancel->base.flags = ASYNC_CONTEXT_CANCELLATION_FLAG_TIMEOUT;	
	cancel->scheduler = async_task_scheduler_ref();
	
	ZVAL_UNDEF(&cancel->base.error);
	
	return cancel;
}

static zend_always_inline void chain_cancellation(void *obj, zval *error)
{
	async_context_cancellation *cancel;
	async_cancel_cb *callback;

	cancel = (async_context_cancellation *) obj;
	cancel->flags |= ASYNC_CONTEXT_CANCELLATION_FLAG_TRIGGERED;
	
	ZVAL_COPY(&cancel->error, error);
	
	while (cancel->callbacks.first != NULL) {
		ASYNC_LIST_EXTRACT_FIRST(&cancel->callbacks, callback);

		callback->func(callback->object, error);
	}
}

static zend_always_inline async_context *create_cancellable_context(async_context_cancellation *cancel, async_context *parent)
{
	async_context *context;
	
	ZEND_ASSERT(parent->output.context != NULL);
	
	context = async_context_object_create(NULL, NULL);
	context->parent = parent;
	context->flags = parent->flags;
	context->output.context = parent->output.context;
	context->cancel = cancel;
	
	cancel->refcount++;
	
	ASYNC_ADDREF(&parent->std);
	
	// Connect cancellation to parent cancellation.
	if (UNEXPECTED(parent->cancel != NULL)) {
		if (parent->cancel->flags & ASYNC_CONTEXT_CANCELLATION_FLAG_TRIGGERED) {
			cancel->flags |= ASYNC_CONTEXT_CANCELLATION_FLAG_TRIGGERED;
			
			ZVAL_COPY(&cancel->error, &parent->cancel->error);
		} else {
			cancel->chain.object = cancel;
			cancel->chain.func = chain_cancellation;
	
			ASYNC_LIST_APPEND(&parent->cancel->callbacks, &cancel->chain);
		}
	}
	
	return context;
}

ASYNC_CALLBACK close_timeout(uv_handle_t *handle)
{
	async_context_timeout *cancel;

	cancel = (async_context_timeout *) handle->data;

	ZEND_ASSERT(cancel != NULL);
	
	async_task_scheduler_unref(cancel->scheduler);

	efree(cancel);
}

static zend_always_inline void release_cancellation(async_context_cancellation *cancel)
{
	if (--cancel->refcount != 0) {
		return;
	}
	
	if (cancel->flags & ASYNC_CONTEXT_CANCELLATION_FLAG_TRIGGERED) {
		zval_ptr_dtor(&cancel->error);
	}
	
	if (cancel->flags & ASYNC_CONTEXT_CANCELLATION_FLAG_TIMEOUT) {
		uv_close((uv_handle_t *) &((async_context_timeout *) cancel)->timer, close_timeout);
	} else {
		efree(cancel);
	}
}

ASYNC_CALLBACK timed_out(uv_timer_t *timer)
{
	async_context_cancellation *cancel;
	async_cancel_cb *callback;

	cancel = (async_context_cancellation *) timer->data;

	ZEND_ASSERT(cancel != NULL);

	if (UNEXPECTED(cancel->flags & ASYNC_CONTEXT_CANCELLATION_FLAG_TRIGGERED)) {
		return;
	}
	
	cancel->flags |= ASYNC_CONTEXT_CANCELLATION_FLAG_TRIGGERED;
	
	ASYNC_PREPARE_EXCEPTION(&cancel->error, async_cancellation_exception_ce, "Context has timed out");
	
	while (cancel->callbacks.first != NULL) {
		ASYNC_LIST_EXTRACT_FIRST(&cancel->callbacks, callback);

		callback->func(callback->object, &cancel->error);
	}
	
	release_cancellation(cancel);
}


static async_context *async_context_object_create(async_context_var *var, zval *value)
{
	async_context *context;

	context = ecalloc(1, sizeof(async_context));

	zend_object_std_init(&context->std, async_context_ce);
	context->std.handlers = &async_context_handlers;

	if (var != NULL) {
		context->var = var;
		ASYNC_ADDREF(&var->std);
	}

	if (value == NULL) {
		ZVAL_NULL(&context->value);
	} else {
		ZVAL_COPY(&context->value, value);
	}

	return context;
}

static void async_context_object_destroy(zend_object *object)
{
	async_context *context;
	zend_output_globals og;

	context = (async_context *) object;

	if (context->var != NULL) {
		ASYNC_DELREF(&context->var->std);
	}

	zval_ptr_dtor(&context->value);
	
	if (context->cancel != NULL) {
		release_cancellation(context->cancel);
	}
	
	if (EXPECTED(context->parent != NULL)) {
		ASYNC_DELREF(&context->parent->std);
	}
	
	if (UNEXPECTED(context->output.handler != NULL)) {
		if (context->parent != NULL) {
			async_fiber_copy_og(&og, &OG(handlers));
			async_fiber_copy_og(&OG(handlers), context->output.handler);
			
			if (OG(active)) {
				php_output_end_all();
			}
			
			php_output_deactivate();
			
			async_fiber_copy_og(&OG(handlers), &og);
		}
		
		efree(context->output.handler);
	}	

	zend_object_std_dtor(&context->std);
}

static zval *read_context_prop(zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
	async_context *context;
	
	char *key;
	
	context = (async_context *) Z_OBJ_P(object);
	
	key = Z_STRVAL_P(member);
	
	if (strcmp(key, "cancelled") == 0) {
		ZVAL_BOOL(rv, context->cancel != NULL && context->cancel->flags & ASYNC_CONTEXT_CANCELLATION_FLAG_TRIGGERED);
	} else {
		rv = &EG(uninitialized_zval);
	}
	
	return rv;
}

static int has_context_prop(zval *object, zval *member, int has_set_exists, void **cache_slot)
{
	zval rv;
	zval *val;

    val = read_context_prop(object, member, 0, cache_slot, &rv);

    if (val == &EG(uninitialized_zval)) {
    	return 0;
    }

    switch (has_set_exists) {
    	case ZEND_PROPERTY_EXISTS:
    	case ZEND_PROPERTY_ISSET:
    		zval_ptr_dtor(val);
    		return 1;
    }

    convert_to_boolean(val);

    return (Z_TYPE_P(val) == IS_TRUE) ? 1 : 0;
}

static ZEND_METHOD(Context, __construct)
{
	ZEND_PARSE_PARAMETERS_NONE();

	zend_throw_error(NULL, "Context must not be constructed from userland code");
}

static ZEND_METHOD(Context, with)
{
	async_context *context;
	async_context *current;
	async_context_var *var;

	zval *key;
	zval *value;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_ZVAL(key)
		Z_PARAM_ZVAL(value)
	ZEND_PARSE_PARAMETERS_END();

	current = (async_context *) Z_OBJ_P(getThis());
	var = (async_context_var *) Z_OBJ_P(key);
	
	ZEND_ASSERT(current->output.context != NULL);

	context = async_context_object_create(var, value);
	context->parent = current;
	context->flags = current->flags;
	context->output.context = current->output.context;
	context->cancel = current->cancel;
	
	if (context->cancel != NULL) {
		context->cancel->refcount++;
	}

	ASYNC_ADDREF(&current->std);

	RETURN_OBJ(&context->std);
}

static ZEND_METHOD(Context, withIsolatedOutput)
{
	async_context *context;
	async_context *current;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	current = (async_context *) Z_OBJ_P(getThis());
	
	ZEND_ASSERT(current->output.context != NULL);
	
	context = async_context_object_create(NULL, NULL);
	context->parent = current;
	context->flags = current->flags;
	context->cancel = current->cancel;
	
	if (context->cancel != NULL) {
		context->cancel->refcount++;
	}
	
	context->output.context = context;
	
	RETURN_OBJ(&context->std);
}

static ZEND_METHOD(Context, withTimeout)
{
	async_context *parent;
	async_context *context;
	async_context_timeout *cancel;

	zend_long timeout;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_LONG(timeout)
	ZEND_PARSE_PARAMETERS_END();

	parent = (async_context *) Z_OBJ_P(getThis());
	cancel = init_timeout_cancellation();
	
	context = create_cancellable_context((async_context_cancellation *) cancel, parent);
	
	uv_timer_init(&cancel->scheduler->loop, &cancel->timer);

	cancel->timer.data = cancel;

	uv_timer_start(&cancel->timer, timed_out, (uint64_t) timeout, 0);
	uv_unref((uv_handle_t *) &cancel->timer);

	RETURN_OBJ(&context->std);
}

static ZEND_METHOD(Context, withCancel)
{
	async_context *parent;
	async_context *context;
	async_context_cancellation *cancel;

	zval *val;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL_DEREF(val)
	ZEND_PARSE_PARAMETERS_END();
	
	zval_ptr_dtor(val);

	parent = (async_context *) Z_OBJ_P(getThis());
	cancel = init_cancellation();

	context = create_cancellable_context(cancel, parent);
	
	ZVAL_OBJ(val, &async_cancellation_handler_object_create(cancel)->std);

	RETURN_OBJ(&context->std);
}

static ZEND_METHOD(Context, shield)
{
	async_context *prev;
	async_context *context;

	ZEND_PARSE_PARAMETERS_NONE();

	prev = (async_context *) Z_OBJ_P(getThis());
	
	ZEND_ASSERT(prev->output.context != NULL);

	context = async_context_object_create(NULL, NULL);
	context->parent = prev;
	context->flags = prev->flags;
	context->output.context = prev->output.context;

	ASYNC_ADDREF(&prev->std);

	RETURN_OBJ(&context->std);
}

static ZEND_METHOD(Context, throwIfCancelled)
{
	async_context *context;

	ZEND_PARSE_PARAMETERS_NONE();

	context = (async_context *) Z_OBJ_P(getThis());

	if (context->cancel != NULL && context->cancel->flags & ASYNC_CONTEXT_CANCELLATION_FLAG_TRIGGERED) {
		ASYNC_FORWARD_ERROR(&context->cancel->error);
	}
}

static ZEND_METHOD(Context, run)
{
	async_context *context;
	async_context *prev;

	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	uint32_t count;

	zval *params;
	zval result;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, -1)
		Z_PARAM_FUNC_EX(fci, fcc, 1, 0)
		Z_PARAM_OPTIONAL
		Z_PARAM_VARIADIC('+', params, count)
	ZEND_PARSE_PARAMETERS_END();

	context = (async_context *) Z_OBJ_P(getThis());

	if (count == 0) {
		fci.param_count = 0;
	} else {
		zend_fcall_info_argp(&fci, count, params);
	}

	fci.retval = &result;
	fci.no_separation = 1;

	prev = ASYNC_G(context);
	ASYNC_G(context) = context;
	
	async_fiber_capture_og(prev);
	async_fiber_restore_og(context);

	zend_call_function(&fci, &fcc);

	async_fiber_capture_og(context);
	async_fiber_restore_og(prev);

	ASYNC_G(context) = prev;

	if (count > 0) {
		zend_fcall_info_args_clear(&fci, 1);
	}

	RETURN_ZVAL(&result, 1, 1);
}

static ZEND_METHOD(Context, current)
{
	zval obj;

	ZEND_PARSE_PARAMETERS_NONE();

	ZVAL_OBJ(&obj, &async_context_get()->std);

	RETURN_ZVAL(&obj, 1, 0);
}

static ZEND_METHOD(Context, background)
{
	zval obj;

	ZEND_PARSE_PARAMETERS_NONE();

	ZVAL_OBJ(&obj, &ASYNC_G(background)->std);

	RETURN_ZVAL(&obj, 1, 0);
}

ZEND_BEGIN_ARG_INFO(arginfo_context_ctor, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_context_with, 0, 2, Phalcon\\Async\\Context, 0)
	ZEND_ARG_OBJ_INFO(0, var, Phalcon\\Async\\ContextVar, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_context_with_isolated_output, 0, 0, Phalcon\\Async\\Context, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_context_with_timeout, 0, 1, Phalcon\\Async\\Context, 0)
	ZEND_ARG_TYPE_INFO(0, milliseconds, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_context_with_cancel, 0, 1, Phalcon\\Async\\Context, 0)
	ZEND_ARG_INFO(1, cancel)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_context_shield, 0, 0, Phalcon\\Async\\Context, 0)
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_context_throw_if_cancelled, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_context_throw_if_cancelled, 0, 0, IS_VOID, NULL, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_context_run, 0, 0, 1)
	ZEND_ARG_CALLABLE_INFO(0, callback, 0)
	ZEND_ARG_VARIADIC_INFO(0, arguments)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_context_current, 0, 0, Phalcon\\Async\\Context, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_context_background, 0, 0, Phalcon\\Async\\Context, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry async_context_functions[] = {
	ZEND_ME(Context, __construct, arginfo_context_ctor, ZEND_ACC_PRIVATE)
	ZEND_ME(Context, with, arginfo_context_with, ZEND_ACC_PUBLIC)
	ZEND_ME(Context, withIsolatedOutput, arginfo_context_with_isolated_output, ZEND_ACC_PUBLIC)
	ZEND_ME(Context, withTimeout, arginfo_context_with_timeout, ZEND_ACC_PUBLIC)
	ZEND_ME(Context, withCancel, arginfo_context_with_cancel, ZEND_ACC_PUBLIC)
	ZEND_ME(Context, shield, arginfo_context_shield, ZEND_ACC_PUBLIC)
	ZEND_ME(Context, throwIfCancelled, arginfo_context_throw_if_cancelled, ZEND_ACC_PUBLIC)
	ZEND_ME(Context, run, arginfo_context_run, ZEND_ACC_PUBLIC)
	ZEND_ME(Context, current, arginfo_context_current, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(Context, background, arginfo_context_background, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_FE_END
};


static zend_object *async_context_var_object_create(zend_class_entry *ce)
{
	async_context_var *var;

	var = ecalloc(1, sizeof(async_context_var));

	zend_object_std_init(&var->std, ce);
	var->std.handlers = &async_context_var_handlers;

	return &var->std;
}

static void async_context_var_object_destroy(zend_object *object)
{
	async_context_var *var;

	var = (async_context_var *) object;

	zend_object_std_dtor(&var->std);
}

static ZEND_METHOD(ContextVar, __construct)
{
	ZEND_PARSE_PARAMETERS_NONE();
}

static ZEND_METHOD(ContextVar, get)
{
	async_context_var *var;
	async_context *context;

	zval *val;

	val = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();

	var = (async_context_var *) Z_OBJ_P(getThis());

	if (val == NULL || Z_TYPE_P(val) == IS_NULL) {
		context = async_context_get();
	} else {
		context = (async_context *) Z_OBJ_P(val);
	}

	do {
		if (context->var == var) {
			RETURN_ZVAL(&context->value, 1, 0);
		}

		context = context->parent;
	} while (context != NULL);
}

ZEND_BEGIN_ARG_INFO(arginfo_context_var_ctor, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_context_var_get, 0, 0, 0)
	ZEND_ARG_OBJ_INFO(0, context, Phalcon\\Async\\Context, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry async_context_var_functions[] = {
	ZEND_ME(ContextVar, __construct, arginfo_context_var_ctor, ZEND_ACC_PUBLIC)
	ZEND_ME(ContextVar, get, arginfo_context_var_get, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static async_cancellation_handler *async_cancellation_handler_object_create(async_context_cancellation *cancel)
{
	async_cancellation_handler *handler;

	handler = ecalloc(1, sizeof(async_cancellation_handler));

	zend_object_std_init(&handler->std, async_cancellation_handler_ce);
	handler->std.handlers = &async_cancellation_handler_handlers;
	
	cancel->refcount++;
	
	handler->cancel = cancel;

	return handler;
}

static void async_cancellation_handler_object_destroy(zend_object *object)
{
	async_cancellation_handler *handler;
	
	handler = (async_cancellation_handler *) object;

	release_cancellation(handler->cancel);

	zend_object_std_dtor(&handler->std);
}

static ZEND_METHOD(CancellationHandler, __invoke)
{
	async_cancellation_handler *handler;
	async_context_cancellation *cancel;
	async_cancel_cb *callback;

	zval *err;

	err = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(err)
	ZEND_PARSE_PARAMETERS_END();

	handler = (async_cancellation_handler *) Z_OBJ_P(getThis());
	cancel = handler->cancel;

	if (UNEXPECTED(cancel->flags & ASYNC_CONTEXT_CANCELLATION_FLAG_TRIGGERED)) {
		return;
	}
	
	cancel->flags |= ASYNC_CONTEXT_CANCELLATION_FLAG_TRIGGERED;

	ASYNC_PREPARE_EXCEPTION(&cancel->error, async_cancellation_exception_ce, "Context has been cancelled");

	if (err != NULL && Z_TYPE_P(err) != IS_NULL) {
		zend_exception_set_previous(Z_OBJ_P(&cancel->error), Z_OBJ_P(err));
		GC_ADDREF(Z_OBJ_P(err));
	}

	while (cancel->callbacks.first != NULL) {
		ASYNC_LIST_EXTRACT_FIRST(&cancel->callbacks, callback);

		callback->func(callback->object, &cancel->error);
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_cancellation_handler_invoke, 0, 0, 0)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry async_cancellation_handler_functions[] = {
	ZEND_ME(CancellationHandler, __invoke, arginfo_cancellation_handler_invoke, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static const zend_function_entry empty_funcs[] = {
	ZEND_FE_END
};

void async_context_ce_register()
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Context", async_context_functions);
	async_context_ce = zend_register_internal_class(&ce);
	async_context_ce->ce_flags |= ZEND_ACC_FINAL;
	async_context_ce->serialize = zend_class_serialize_deny;
	async_context_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_context_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_context_handlers.free_obj = async_context_object_destroy;
	async_context_handlers.clone_obj = NULL;
	async_context_handlers.has_property = has_context_prop;
	async_context_handlers.read_property = read_context_prop;

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\ContextVar", async_context_var_functions);
	async_context_var_ce = zend_register_internal_class(&ce);
	async_context_var_ce->ce_flags |= ZEND_ACC_FINAL;
	async_context_var_ce->create_object = async_context_var_object_create;
	async_context_var_ce->serialize = zend_class_serialize_deny;
	async_context_var_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_context_var_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_context_var_handlers.free_obj = async_context_var_object_destroy;
	async_context_var_handlers.clone_obj = NULL;

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\CancellationHandler", async_cancellation_handler_functions);
	async_cancellation_handler_ce = zend_register_internal_class(&ce);
	async_cancellation_handler_ce->ce_flags |= ZEND_ACC_FINAL;
	async_cancellation_handler_ce->create_object = NULL;
	async_cancellation_handler_ce->serialize = zend_class_serialize_deny;
	async_cancellation_handler_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_cancellation_handler_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_cancellation_handler_handlers.free_obj = async_cancellation_handler_object_destroy;
	async_cancellation_handler_handlers.clone_obj = NULL;
	
	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\CancellationException", empty_funcs);
	async_cancellation_exception_ce = zend_register_internal_class(&ce);

	zend_do_inheritance(async_cancellation_exception_ce, zend_ce_exception);
}

void async_context_init()
{
	async_context *context;
	
	context = async_context_object_create(NULL, NULL);
	
	context->output.context = context;
	context->output.handler = emalloc(sizeof(zend_output_globals));
	
	ASYNC_G(context) = context;
	ASYNC_G(foreground) = context;
	
	context = async_context_object_create(NULL, NULL);
	context->flags |= ASYNC_CONTEXT_FLAG_BACKGROUND;
	
	context->output.context = ASYNC_G(foreground);
	
	ASYNC_G(background) = context;
}

void async_context_shutdown()
{
	async_context *context;

	context = ASYNC_G(foreground);
	
	if (context != NULL) {
		ASYNC_DELREF(&context->std);
	}
	
	context = ASYNC_G(background);

	if (context != NULL) {
		ASYNC_DELREF(&context->std);
	}
}

#endif
