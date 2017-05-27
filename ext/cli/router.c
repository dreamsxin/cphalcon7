
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
  |          Rack Lin <racklin@gmail.com>                                  |
  +------------------------------------------------------------------------+
*/

#include "cli/router.h"
#include "cli/../router.h"
#include "cli/../routerinterface.h"
#include "cli/router/exception.h"
#include "diinterface.h"
#include "di/injectable.h"

#include <main/SAPI.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/array.h"
#include "kernel/string.h"
#include "kernel/object.h"
#include "kernel/fcall.h"

/**
 * Phalcon\Cli\Router
 *
 * <p>Phalcon\Cli\Router is the standard framework router. Routing is the
 * process of taking a command-line arguments and
 * decomposing it into parameters to determine which module, task, and
 * action of that task should receive the request</p>
 *
 *<code>
 *	$router = new Phalcon\Cli\Router();
 *	$router->handle(array(
 *		'module' => 'main',
 *		'task' => 'videos',
 *		'action' => 'process'
 *	));
 *	echo $router->getTaskName();
 *</code>
 *
 */
zend_class_entry *phalcon_cli_router_ce;

PHP_METHOD(Phalcon_Cli_Router, __construct);
PHP_METHOD(Phalcon_Cli_Router, setDefaultTask);
PHP_METHOD(Phalcon_Cli_Router, getTaskName);
PHP_METHOD(Phalcon_Cli_Router, handle);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cli_router_handle, 0, 0, 0)
	ZEND_ARG_INFO(0, arguments)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_cli_router_method_entry[] = {
	PHP_ME(Phalcon_Cli_Router, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Cli_Router, setDefaultTask, arginfo_phalcon_routerinterface_setdefaulthandler, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cli_Router, getTaskName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cli_Router, handle, arginfo_phalcon_cli_router_handle, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Cli\Router initializer
 */
PHALCON_INIT_CLASS(Phalcon_Cli_Router){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Cli, Router, cli_router, phalcon_router_ce, phalcon_cli_router_method_entry, 0);

	return SUCCESS;
}

/**
 * Phalcon\Cli\Router constructor
 */
PHP_METHOD(Phalcon_Cli_Router, __construct){

	phalcon_update_property_empty_array(getThis(), SL("_params"));
	phalcon_update_property_empty_array(getThis(), SL("_defaultParams"));
}

/**
 * Sets the default task name
 *
 * @param string $taskName
 */
PHP_METHOD(Phalcon_Cli_Router, setDefaultTask){

	zval *task_name;

	phalcon_fetch_params(0, 1, 0, &task_name);

	phalcon_update_property(getThis(), SL("_defaultHandler"), task_name);
	RETURN_THIS();
}

/**
 * Returns proccesed task name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Cli_Router, getTaskName){


	RETURN_MEMBER(getThis(), "_handler");
}

/**
 * Handles routing information received from command-line arguments
 *
 * @param array $arguments
 */
PHP_METHOD(Phalcon_Cli_Router, handle){

	zval *arguments = NULL, longopts = {}, options = {}, module_name = {}, namespace_name = {}, task_name = {}, action_name = {}, params = {};

	phalcon_fetch_params(0, 0, 1, &arguments);

	if (!arguments || Z_TYPE_P(arguments) != IS_ARRAY) {
		if (likely(!strcmp(sapi_module.name, "cli"))) {
			array_init(&longopts);
			phalcon_array_append_str(&longopts, SL("module::"), 0);
			phalcon_array_append_str(&longopts, SL("namespace::"), 0);
			phalcon_array_append_str(&longopts, SL("task::"), 0);
			phalcon_array_append_str(&longopts, SL("action::"), 0);
			phalcon_array_append_str(&longopts, SL("params::"), 0);
			PHALCON_CALL_FUNCTION(&options, "getopt", &PHALCON_GLOBAL(z_null), &longopts);
			zval_ptr_dtor(&longopts);
		} else {
			array_init(&options);
		}
	} else {
		ZVAL_DUP(&options, arguments);
	}

	/**
	 * Check for a module
	 */
	if (phalcon_array_isset_fetch_str(&module_name, &options, SL("module"), PH_COPY)) {
		phalcon_array_unset_str(&options, SL("module"), 0);
	} else {
		ZVAL_NULL(&module_name);
	}
	phalcon_update_property(getThis(), SL("_module"), &module_name);
	zval_ptr_dtor(&module_name);

	/**
	 * Check for a namespace
	 */
	if (phalcon_array_isset_fetch_str(&namespace_name, &options, SL("namespace"), PH_COPY)) {
		phalcon_array_unset_str(&options, SL("namespace"), 0);
	} else {
		ZVAL_NULL(&namespace_name);
	}
	phalcon_update_property(getThis(), SL("_namespace"), &namespace_name);
	zval_ptr_dtor(&namespace_name);

	/**
	 * Check for a task
	 */
	if (phalcon_array_isset_fetch_str(&task_name, &options, SL("task"), PH_COPY)) {
		phalcon_array_unset_str(&options, SL("task"), 0);
	} else {
		ZVAL_NULL(&task_name);
	}
	phalcon_update_property(getThis(), SL("_handler"), &task_name);
	zval_ptr_dtor(&task_name);

	/**
	 * Check for an action
	 */
	if (phalcon_array_isset_fetch_str(&action_name, &options, SL("action"), PH_COPY)) {
		phalcon_array_unset_str(&options, SL("action"), 0);
	} else {
		ZVAL_NULL(&action_name);
	}
	phalcon_update_property(getThis(), SL("_action"), &action_name);
	zval_ptr_dtor(&action_name);

	if (phalcon_array_isset_fetch_str(&params, &options, SL("params"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_params"), &params);
	} else {
		phalcon_update_property(getThis(), SL("_params"), &options);
	}
	zval_ptr_dtor(&options);
}
