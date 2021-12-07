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
#include "kernel/backend.h"

ASYNC_API zend_class_entry *async_sync_condition_ce;

static zend_object_handlers async_sync_condition_handlers;

typedef struct _async_sync_condition {
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
			ASYNC_PREPARE_SCHEDULER_ERROR(&cond->error, "Condition has been closed");
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

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sync_condition_close, 0, 0, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 1)
ZEND_END_ARG_INFO();

static PHP_METHOD(Condition, close)
{
	async_sync_condition *cond;
	
	zval *val;
	zval error;

	val = NULL;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_OBJECT_OF_CLASS_EX(val, zend_ce_throwable, 1, 0)
	ZEND_PARSE_PARAMETERS_END();
	
	cond = (async_sync_condition *) Z_OBJ_P(getThis());
	
	if (cond->shutdown.func) {
		ASYNC_PREPARE_ERROR(&error, execute_data, "Condition has been closed");
		
		if (val != NULL && Z_TYPE_P(val) != IS_NULL) {
			zend_exception_set_previous(Z_OBJ_P(&error), Z_OBJ_P(val));
			GC_ADDREF(Z_OBJ_P(val));
		}
		
		ASYNC_LIST_REMOVE(&cond->scheduler->shutdown, &cond->shutdown);
		
		cond->shutdown.func(cond, &error);
		
		zval_ptr_dtor(&error);
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sync_condition_wait, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Condition, wait)
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

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sync_condition_signal, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Condition, signal)
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

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_sync_condition_broadcast, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Condition, broadcast)
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

//LCOV_EXCL_START
ASYNC_METHOD_NO_WAKEUP(Condition, async_sync_condition_ce)
//LCOV_EXCL_STOP

static const zend_function_entry sync_condition_funcs[] = {
	PHP_ME(Condition, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(Condition, close, arginfo_sync_condition_close, ZEND_ACC_PUBLIC)
	PHP_ME(Condition, wait, arginfo_sync_condition_wait, ZEND_ACC_PUBLIC)
	PHP_ME(Condition, signal, arginfo_sync_condition_signal, ZEND_ACC_PUBLIC)
	PHP_ME(Condition, broadcast, arginfo_sync_condition_broadcast, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


void async_sync_ce_register()
{
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Sync", "Condition", sync_condition_funcs);
	async_sync_condition_ce = zend_register_internal_class(&ce);
	async_sync_condition_ce->ce_flags |= ZEND_ACC_FINAL;
	async_sync_condition_ce->create_object = async_sync_condition_object_create;
#if PHP_VERSION_ID >= 80100
	async_sync_condition_ce->ce_flags |= ZEND_ACC_NOT_SERIALIZABLE;
#else
	async_sync_condition_ce->serialize = zend_class_serialize_deny;
	async_sync_condition_ce->unserialize = zend_class_unserialize_deny;
#endif
	memcpy(&async_sync_condition_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_sync_condition_handlers.free_obj = async_sync_condition_object_destroy;
	async_sync_condition_handlers.dtor_obj = async_sync_condition_object_dtor;
	async_sync_condition_handlers.clone_obj = NULL;
}
