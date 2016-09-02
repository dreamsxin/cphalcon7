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

#include "cli/console.h"
#include "cli/console/exception.h"
#include "cli/router.h"
#include "diinterface.h"
#include "dispatcherinterface.h"
#include "diinterface.h"
#include "di/injectable.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/concat.h"
#include "kernel/file.h"
#include "kernel/require.h"

#include "interned-strings.h"

/**
 * Phalcon\CLI\Console
 *
 * This component allows to create CLI applications using Phalcon
 */
zend_class_entry *phalcon_cli_console_ce;

PHP_METHOD(Phalcon_CLI_Console, __construct);
PHP_METHOD(Phalcon_CLI_Console, registerModules);
PHP_METHOD(Phalcon_CLI_Console, addModules);
PHP_METHOD(Phalcon_CLI_Console, getModules);
PHP_METHOD(Phalcon_CLI_Console, handle);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cli_console___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, dependencyInjector)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cli_console_registermodules, 0, 0, 1)
	ZEND_ARG_INFO(0, modules)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cli_console_addmodules, 0, 0, 1)
	ZEND_ARG_INFO(0, modules)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cli_console_handle, 0, 0, 0)
	ZEND_ARG_INFO(0, arguments)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_cli_console_method_entry[] = {
	PHP_ME(Phalcon_CLI_Console, __construct, arginfo_phalcon_cli_console___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_CLI_Console, registerModules, arginfo_phalcon_cli_console_registermodules, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_CLI_Console, addModules, arginfo_phalcon_cli_console_addmodules, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_CLI_Console, getModules, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_CLI_Console, handle, arginfo_phalcon_cli_console_handle, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\CLI\Console initializer
 */
PHALCON_INIT_CLASS(Phalcon_CLI_Console){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\CLI, Console, cli_console, phalcon_di_injectable_ce, phalcon_cli_console_method_entry, 0);

	zend_declare_property_null(phalcon_cli_console_ce, SL("_modules"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_console_ce, SL("_moduleObject"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\CLI\Console constructor
 */
PHP_METHOD(Phalcon_CLI_Console, __construct){

	zval *dependency_injector = NULL;

	phalcon_fetch_params(0, 0, 1, &dependency_injector);

	if (dependency_injector && Z_TYPE_P(dependency_injector) == IS_OBJECT) {
		phalcon_update_property_zval(getThis(), SL("_dependencyInjector"), dependency_injector);
	}
}

/**
 * Register an array of modules present in the console
 *
 *<code>
 *	$application->registerModules(array(
 *		'frontend' => array(
 *			'className' => 'Multiple\Frontend\Module',
 *			'path' => '../apps/frontend/Module.php'
 *		),
 *		'backend' => array(
 *			'className' => 'Multiple\Backend\Module',
 *			'path' => '../apps/backend/Module.php'
 *		)
 *	));
 *</code>
 *
 * @param array $modules
 */
PHP_METHOD(Phalcon_CLI_Console, registerModules){

	zval *modules;

	phalcon_fetch_params(0, 1, 0, &modules);

	if (Z_TYPE_P(modules) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_cli_console_exception_ce, "Modules must be an Array");
		return;
	}
	phalcon_update_property_zval(getThis(), SL("_modules"), modules);

}

/**
 * Merge modules with the existing ones
 *
 *<code>
 *	$application->addModules(array(
 *		'admin' => array(
 *			'className' => 'Multiple\Admin\Module',
 *			'path' => '../apps/admin/Module.php'
 *		)
 *	));
 *</code>
 *
 * @param array $modules
 */
PHP_METHOD(Phalcon_CLI_Console, addModules){

	zval *modules, original_modules = {}, register_modules = {};

	phalcon_fetch_params(0, 1, 0, &modules);

	if (Z_TYPE_P(modules) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_cli_console_exception_ce, "Modules must be an Array");
		return;
	}

	phalcon_read_property(&original_modules, getThis(), SL("_modules"), PH_NOISY);

	phalcon_fast_array_merge(&register_modules, modules, &original_modules);
	phalcon_update_property_zval(getThis(), SL("_modules"), &register_modules);
}

/**
 * Return the modules registered in the console
 *
 * @return array
 */
PHP_METHOD(Phalcon_CLI_Console, getModules){


	RETURN_MEMBER(getThis(), "_modules");
}

/**
 * Handle the command-line arguments.
 *  
 * 
 * <code>
 * 	$arguments = array(
 * 		'task' => 'taskname',
 * 		'action' => 'action',
 * 		'params' => array('parameter1', 'parameter2')
 * 	);
 * 	$console->handle($arguments);
 * </code>
 *
 * @param array $arguments
 * @return mixed
 */
PHP_METHOD(Phalcon_CLI_Console, handle){

	zval *_arguments = NULL, arguments = {}, dependency_injector = {}, events_manager = {}, event_name = {}, service = {}, router = {}, module_name = {};
	zval status = {}, modules = {}, exception_msg = {}, module = {}, path = {}, class_name = {}, module_object = {};
	zval namespace_name = {}, task_name = {}, action_name = {}, params = {}, dispatcher = {};

	phalcon_fetch_params(0, 0, 1, &_arguments);

	if (!_arguments) {
		array_init(&arguments);
	} else {
		PHALCON_CPY_WRT(&arguments, _arguments);
	}

	phalcon_return_property(&events_manager, getThis(), SL("_eventsManager"));

	ZVAL_STRING(&service, ISV(router));

	PHALCON_CALL_METHODW(&dependency_injector, getThis(), "getdi");
	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_FORMATW(phalcon_cli_console_exception_ce, "A dependency injection container is required to access the '%s' service", Z_STRVAL(service));
		return;
	}

	PHALCON_CALL_METHODW(&router, &dependency_injector, "getshared", &service);
	PHALCON_VERIFY_CLASSW(&router, phalcon_cli_router_ce);

	PHALCON_CALL_METHODW(NULL, &router, "handle", &arguments);
	PHALCON_CALL_METHODW(&module_name, &router, "getmodulename");
	if (zend_is_true(&module_name)) {
		if (Z_TYPE(events_manager) == IS_OBJECT) {
			ZVAL_STRING(&event_name, "console:beforeStartModule");

			PHALCON_CALL_METHODW(&status, &events_manager, "fire", &event_name, getThis(), &module_name);
			if (PHALCON_IS_FALSE(&status)) {
				RETURN_FALSE;
			}
		}

		phalcon_read_property(&modules, getThis(), SL("_modules"), PH_NOISY);
		if (!phalcon_array_isset_fetch(&module, &modules, &module_name, 0)) {
			PHALCON_CONCAT_SVS(&exception_msg, "Module '", &module_name, "' isn't registered in the console container");
			PHALCON_THROW_EXCEPTION_ZVALW(phalcon_cli_console_exception_ce, &exception_msg);
			return;
		}

		if (Z_TYPE_P(&module) != IS_ARRAY) { 
			PHALCON_THROW_EXCEPTION_STRW(phalcon_cli_console_exception_ce, "Invalid module definition path");
			return;
		}

		if (phalcon_array_isset_fetch_str(&path, &module, SL("path"))) {
			convert_to_string_ex(&path);

			if (phalcon_file_exists(&path) == SUCCESS) {
				RETURN_ON_FAILURE(phalcon_require(Z_STRVAL(path)));
			} else {
				zend_throw_exception_ex(phalcon_cli_console_exception_ce, 0, "Modules definition path '%s' does not exist", Z_STRVAL(path));
				return;
			}
		}

		if (!phalcon_array_isset_fetch_str(&class_name, &module, SL("className"))) {
			ZVAL_STRING(&class_name, "Module");
		}

		PHALCON_CALL_METHODW(&module_object, &dependency_injector, "getshared", &class_name);
		PHALCON_CALL_METHODW(NULL, &module_object, "registerautoloaders");
		PHALCON_CALL_METHODW(NULL, &module_object, "registerservices", &dependency_injector);
		if (Z_TYPE(events_manager) == IS_OBJECT) {
			phalcon_update_property_zval(getThis(), SL("_moduleObject"), &module_object);

			ZVAL_STRING(&event_name, "console:afterStartModule");

			PHALCON_CALL_METHODW(&status, &events_manager, "fire", &event_name, getThis(), &module_name);
			if (PHALCON_IS_FALSE(&status)) {
				RETURN_FALSE;
			}
		}
	}

	PHALCON_CALL_METHODW(&namespace_name, &router, "getnamespacename");
	PHALCON_CALL_METHODW(&task_name, &router, "gettaskname");
	PHALCON_CALL_METHODW(&action_name, &router, "getactionname");
	PHALCON_CALL_METHODW(&params, &router, "getparams");

	ZVAL_STRING(&service, ISV(dispatcher));

	PHALCON_CALL_METHODW(&dispatcher, &dependency_injector, "getshared", &service);
	PHALCON_VERIFY_INTERFACEW(&dispatcher, phalcon_dispatcherinterface_ce);

	PHALCON_CALL_METHODW(NULL, &dispatcher, "setnamespacename", &namespace_name);
	PHALCON_CALL_METHODW(NULL, &dispatcher, "settaskname", &task_name);
	PHALCON_CALL_METHODW(NULL, &dispatcher, "setactionname", &action_name);
	PHALCON_CALL_METHODW(NULL, &dispatcher, "setparams", &params);
	if (Z_TYPE(events_manager) == IS_OBJECT) {
		ZVAL_STRING(&event_name, "console:beforeHandleTask");

		PHALCON_CALL_METHODW(&status, &events_manager, "fire", &event_name, getThis(), &dispatcher);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}
	}

	PHALCON_CALL_METHODW(&status, &dispatcher, "dispatch");

	if (Z_TYPE(events_manager) == IS_OBJECT) {
		ZVAL_STRING(&event_name, "console:afterHandleTask");
		PHALCON_CALL_METHODW(NULL, &events_manager, "fire", &event_name, getThis(), &status);
	}

	RETURN_CTORW(&status);
}
