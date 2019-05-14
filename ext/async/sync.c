
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

ASYNC_API zend_class_entry *async_sync_condition_ce;

static zend_object_handlers async_sync_condition_handlers;

typedef struct {
	zend_object std;
	
	async_task_scheduler *scheduler;
	async_cancel_cb shutdown;
	zval error;
	
	async_op_list waiting;
	uint32_t broadcast;
} async_sync_condition;


ASYNC_CALLBACK shutdown_cond_cb(void *arg, zval *error)
{
	async_sync_condition *cond;
	
	cond = (async_sync_condition *) arg;
	
	cond->shutdown.func = NULL;
	
	if (Z_TYPE_P(&cond->error) == IS_UNDEF) {
		if (error) {
			ZVAL_COPY(&cond->error, error);
		} else {
			ASYNC_PREPARE_ERROR(&cond->error, "Condition has been closed");
		}
	}

	while (cond->waiting.first) {
		ASYNC_FAIL_OP(cond->waiting.first, &cond->error);
	}
}

static zend_object *async_sync_condition_object_create(zend_class_entry *ce)
{
	async_sync_condition *cond;
	
	cond = ecalloc(1, sizeof(async_sync_condition));
	
	zend_object_std_init(&cond->std, ce);
	cond->std.handlers = &async_sync_condition_handlers;
	
	cond->scheduler = async_task_scheduler_ref();
	
	cond->shutdown.func = shutdown_cond_cb;
	cond->shutdown.object = cond;
	
	ASYNC_LIST_APPEND(&cond->scheduler->shutdown, &cond->shutdown);
	
	return &cond->std;
}

static void async_sync_condition_object_dtor(zend_object *object)
{
	async_sync_condition *cond;
	
	cond = (async_sync_condition *) object;
	
	if (cond->shutdown.func) {	
		ASYNC_LIST_REMOVE(&cond->scheduler->shutdown, &cond->shutdown);
		
		cond->shutdown.func(cond, NULL);
	}
}

static void async_sync_condition_object_destroy(zend_object *object)
{
	async_sync_condition *cond;
	
	cond = (async_sync_condition *) object;
	
	zval_ptr_dtor(&cond->error);
	
	async_task_scheduler_unref(cond->scheduler);
	
	zend_object_std_dtor(&cond->std);
}

static ZEND_METHOD(Condition, close)
{
	async_sync_condition *cond;
	
	zval *val;
	zval error;

	val = NULL;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();
	
	cond = (async_sync_condition *) Z_OBJ_P(getThis());
	
	if (cond->shutdown.func) {
		ASYNC_PREPARE_ERROR(&error, "Condition has been closed");
		
		if (val != NULL && Z_TYPE_P(val) != IS_NULL) {
			zend_exception_set_previous(Z_OBJ_P(&error), Z_OBJ_P(val));
			GC_ADDREF(Z_OBJ_P(val));
		}
		
		ASYNC_LIST_REMOVE(&cond->scheduler->shutdown, &cond->shutdown);
		
		cond->shutdown.func(cond, &error);
		
		zval_ptr_dtor(&error);
	}
}

static ZEND_METHOD(Condition, wait)
{
	async_sync_condition *cond;
	async_context *context;
	async_op *op;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	cond = (async_sync_condition *) Z_OBJ_P(getThis());
	
	if (UNEXPECTED(Z_TYPE_P(&cond->error) != IS_UNDEF)) {
		ASYNC_FORWARD_ERROR(&cond->error);
		return;
	}
	
	context = async_context_get();
	
	ASYNC_ALLOC_OP(op);
	ASYNC_APPEND_OP(&cond->waiting, op);
	
	if (!async_context_is_background(context)) {
		ASYNC_BUSY_ENTER(cond->scheduler);
	}
	
	if (UNEXPECTED(FAILURE == async_await_op(op))) {
		ASYNC_FORWARD_OP_ERROR(op);
	}
	
	if (!async_context_is_background(context)) {
		ASYNC_BUSY_EXIT(cond->scheduler);
	}
	
	if (op->flags & ASYNC_OP_FLAG_DEFER) {
		cond->broadcast--;
	}

	ASYNC_FREE_OP(op);
}

static ZEND_METHOD(Condition, signal)
{
	async_sync_condition *cond;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	cond = (async_sync_condition *) Z_OBJ_P(getThis());
	
	if (UNEXPECTED(Z_TYPE_P(&cond->error) != IS_UNDEF)) {
		ASYNC_FORWARD_ERROR(&cond->error);
		return;
	}
	
	if (cond->waiting.first) {
		if (cond->broadcast) {
			cond->broadcast++;
			cond->waiting.first->flags |= ASYNC_OP_FLAG_DEFER;
		}

		ASYNC_FINISH_OP(cond->waiting.first);

		RETURN_LONG(1);
	}

	RETURN_LONG(0);
}

static ZEND_METHOD(Condition, broadcast)
{
	async_sync_condition *cond;
	
	int count;

	ZEND_PARSE_PARAMETERS_NONE();
	
	cond = (async_sync_condition *) Z_OBJ_P(getThis());
	
	if (UNEXPECTED(Z_TYPE_P(&cond->error) != IS_UNDEF)) {
		ASYNC_FORWARD_ERROR(&cond->error);
		return;
	}
	
	count = 0;

	while (cond->waiting.first) {
		cond->broadcast++;
		cond->waiting.first->flags |= ASYNC_OP_FLAG_DEFER;
		
		ASYNC_FINISH_OP(cond->waiting.first);

		count++;
	}

	RETURN_LONG(count);
}

#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sync_condition_close, 0, 0, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sync_condition_wait, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sync_condition_signal, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sync_condition_broadcast, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sync_condition_close, 0, 0, IS_VOID, NULL, 0)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sync_condition_wait, 0, 0, IS_VOID, NULL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sync_condition_signal, 0, 0, IS_LONG, NULL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sync_condition_broadcast, 0, 0, IS_LONG, NULL, 0)
ZEND_END_ARG_INFO()
#endif

static const zend_function_entry sync_condition_funcs[] = {
	ZEND_ME(Condition, close, arginfo_sync_condition_close, ZEND_ACC_PUBLIC)
	ZEND_ME(Condition, wait, arginfo_sync_condition_wait, ZEND_ACC_PUBLIC)
	ZEND_ME(Condition, signal, arginfo_sync_condition_signal, ZEND_ACC_PUBLIC)
	ZEND_ME(Condition, broadcast, arginfo_sync_condition_broadcast, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


void async_sync_init()
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Sync\\Condition", sync_condition_funcs);
	async_sync_condition_ce = zend_register_internal_class(&ce);
	async_sync_condition_ce->ce_flags |= ZEND_ACC_FINAL;
	async_sync_condition_ce->create_object = async_sync_condition_object_create;
	async_sync_condition_ce->serialize = zend_class_serialize_deny;
	async_sync_condition_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_sync_condition_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_sync_condition_handlers.free_obj = async_sync_condition_object_destroy;
	async_sync_condition_handlers.dtor_obj = async_sync_condition_object_dtor;
	async_sync_condition_handlers.clone_obj = NULL;
}

#endif
