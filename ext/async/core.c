
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

ASYNC_API zend_class_entry *async_awaitable_ce;

static int implements_awaitable(zend_class_entry *entry, zend_class_entry *implementor)
{
	if (implementor == async_deferred_awaitable_ce) {
		return SUCCESS;
	}

	if (implementor == async_task_ce) {
		return SUCCESS;
	}

	zend_error_noreturn(
		E_CORE_ERROR,
		"Class %s must not implement interface %s, create an awaitable using %s instead",
		ZSTR_VAL(implementor->name),
		ZSTR_VAL(entry->name),
		ZSTR_VAL(async_deferred_ce->name)
	);

	return FAILURE;
}

static const zend_function_entry empty_funcs[] = {
	ZEND_FE_END
};

void async_awaitable_ce_register()
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Awaitable", empty_funcs);
	async_awaitable_ce = zend_register_internal_interface(&ce);
	async_awaitable_ce->interface_gets_implemented = implements_awaitable;
}

extern ZEND_API void (*async_orig_execute_ex)(zend_execute_data *exec);

static void execute_root(zend_execute_data *exec)
{
	zend_object *error;
	
	error = NULL;

	zend_try {
		async_orig_execute_ex(exec);
	} zend_catch {
		ASYNC_G(exit) = 1;
	} zend_end_try();
	
	error = EG(exception);
	
	if (UNEXPECTED(error != NULL)) {
		ASYNC_ADDREF(error);
		zend_clear_exception();
	}
	
	async_task_scheduler_run();
	
	EG(exception) = error;
}

/* Custom executor being used to run the task scheduler before shutdown functions. */
ZEND_API void async_execute_ex(zend_execute_data *exec)
{
	if (UNEXPECTED(exec->prev_execute_data == NULL && ASYNC_G(task) == NULL)) {
		execute_root(exec);
	} else {
		async_orig_execute_ex(exec);
	}
}
#endif
