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
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#include "cli/console.h"
#include "cli/../application.h"
#include "cli/console/exception.h"
#include "cli/../routerinterface.h"
#include "diinterface.h"
#include "dispatcherinterface.h"
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
 * Phalcon\Cli\Console
 *
 * This component allows to create CLI applications using Phalcon
 */
zend_class_entry *phalcon_cli_console_ce;

PHP_METHOD(Phalcon_Cli_Console, __construct);
PHP_METHOD(Phalcon_Cli_Console, addModules);
PHP_METHOD(Phalcon_Cli_Console, getModules);
PHP_METHOD(Phalcon_Cli_Console, handle);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cli_console___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, dependencyInjector)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_cli_console_addmodules, 0, 0, 1)
	ZEND_ARG_INFO(0, modules)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_cli_console_method_entry[] = {
	PHP_ME(Phalcon_Cli_Console, __construct, arginfo_phalcon_cli_console___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Cli_Console, addModules, arginfo_phalcon_cli_console_addmodules, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Cli_Console, handle, arginfo_phalcon_application_handle, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Cli\Console initializer
 */
PHALCON_INIT_CLASS(Phalcon_Cli_Console){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Cli, Console, cli_console, phalcon_application_ce, phalcon_cli_console_method_entry, 0);

	zend_declare_property_null(phalcon_cli_console_ce, SL("_modules"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_cli_console_ce, SL("_moduleObject"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Cli\Console constructor
 */
PHP_METHOD(Phalcon_Cli_Console, __construct){

	zval *dependency_injector = NULL;

	phalcon_fetch_params(0, 0, 1, &dependency_injector);

	if (dependency_injector && Z_TYPE_P(dependency_injector) != IS_NULL) {
		PHALCON_CALL_METHOD(NULL, getThis(), "setdi", dependency_injector);
	}
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
PHP_METHOD(Phalcon_Cli_Console, addModules){

	zval *modules;

	phalcon_fetch_params(0, 1, 0, &modules);

	PHALCON_CALL_METHOD(return_value, getThis(), "registermodules", modules, &PHALCON_GLOBAL(z_true));
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
PHP_METHOD(Phalcon_Cli_Console, handle){

	zval *_arguments = NULL, arguments = {}, dependency_injector = {}, event_name = {}, service = {}, router = {}, module_name = {};
	zval status = {}, modules = {}, exception_msg = {}, module = {}, path = {}, class_name = {}, module_object = {};
	zval namespace_name = {}, task_name = {}, action_name = {}, params = {}, dispatcher = {};

	phalcon_fetch_params(1, 0, 1, &_arguments);

	if (!_arguments) {
		ZVAL_NULL(&arguments);
	} else {
		PHALCON_MM_ZVAL_COPY(&arguments, _arguments);
	}

	ZVAL_STR(&service, IS(router));

	PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi");
	PHALCON_MM_ADD_ENTRY(&dependency_injector);
	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_MM_THROW_EXCEPTION_FORMAT(phalcon_cli_console_exception_ce, "A dependency injection container is required to access the '%s' service", Z_STRVAL(service));
		return;
	}

	PHALCON_MM_CALL_METHOD(&router, &dependency_injector, "getshared", &service);
	PHALCON_MM_ADD_ENTRY(&router);
	PHALCON_MM_VERIFY_INTERFACE(&router, phalcon_routerinterface_ce);

	PHALCON_MM_CALL_METHOD(NULL, &router, "handle", &arguments);
	PHALCON_MM_CALL_METHOD(&module_name, &router, "getmodulename");
	PHALCON_MM_ADD_ENTRY(&module_name);
	if (zend_is_true(&module_name)) {
		PHALCON_MM_ZVAL_STRING(&event_name, "console:beforeStartModule");
		PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name, &module_name);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_MM_FALSE;
		}
		zval_ptr_dtor(&status);

		phalcon_read_property(&modules, getThis(), SL("_modules"), PH_NOISY|PH_READONLY);
		if (!phalcon_array_isset_fetch(&module, &modules, &module_name, PH_READONLY)) {
			PHALCON_CONCAT_SVS(&exception_msg, "Module '", &module_name, "' isn't registered in the console container");
			PHALCON_MM_ADD_ENTRY(&exception_msg);
			PHALCON_MM_THROW_EXCEPTION_ZVAL(phalcon_cli_console_exception_ce, &exception_msg);
			return;
		}

		if (Z_TYPE(module) != IS_ARRAY) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_cli_console_exception_ce, "Invalid module definition path");
			return;
		}

		if (phalcon_array_isset_fetch_str(&path, &module, SL("path"), PH_READONLY)) {
			convert_to_string_ex(&path);

			if (phalcon_file_exists(&path) == SUCCESS) {
				RETURN_MM_ON_FAILURE(phalcon_require(Z_STRVAL(path)));
			} else {
				PHALCON_MM_THROW_EXCEPTION_FORMAT(phalcon_cli_console_exception_ce, "Modules definition path '%s' does not exist", Z_STRVAL(path));
				return;
			}
		}

		if (!phalcon_array_isset_fetch_str(&class_name, &module, SL("className"), PH_COPY)) {
			PHALCON_MM_ZVAL_STRING(&class_name, "Module");
		}

		PHALCON_MM_CALL_METHOD(&module_object, &dependency_injector, "getshared", &class_name);
		PHALCON_MM_ADD_ENTRY(&module_object);
		PHALCON_MM_CALL_METHOD(NULL, &module_object, "registerautoloaders");
		PHALCON_MM_CALL_METHOD(NULL, &module_object, "registerservices", &dependency_injector);
		phalcon_update_property(getThis(), SL("_moduleObject"), &module_object);

		PHALCON_MM_ZVAL_STRING(&event_name, "console:afterStartModule");
		PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name, &module_name);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_MM_FALSE;
		}
		zval_ptr_dtor(&status);
	}

	PHALCON_MM_CALL_METHOD(&namespace_name, &router, "getnamespacename");
	PHALCON_MM_ADD_ENTRY(&namespace_name);
	PHALCON_MM_CALL_METHOD(&task_name, &router, "gethandlername");
	PHALCON_MM_ADD_ENTRY(&task_name);
	PHALCON_MM_CALL_METHOD(&action_name, &router, "getactionname");
	PHALCON_MM_ADD_ENTRY(&action_name);
	PHALCON_MM_CALL_METHOD(&params, &router, "getparams");
	PHALCON_MM_ADD_ENTRY(&params);

	ZVAL_STR(&service, IS(dispatcher));

	PHALCON_MM_CALL_METHOD(&dispatcher, &dependency_injector, "getshared", &service);
	PHALCON_MM_ADD_ENTRY(&dispatcher);
	PHALCON_MM_VERIFY_INTERFACE(&dispatcher, phalcon_dispatcherinterface_ce);

	PHALCON_MM_CALL_METHOD(NULL, &dispatcher, "setnamespacename", &namespace_name);
	PHALCON_MM_CALL_METHOD(NULL, &dispatcher, "sethandlername", &task_name);
	PHALCON_MM_CALL_METHOD(NULL, &dispatcher, "setactionname", &action_name);
	PHALCON_MM_CALL_METHOD(NULL, &dispatcher, "setparams", &params);

	PHALCON_MM_ZVAL_STRING(&event_name, "console:beforeHandleTask");
	PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name, &dispatcher);

	if (PHALCON_IS_FALSE(&status)) {
		RETURN_MM_FALSE;
	}
	zval_ptr_dtor(&status);

	PHALCON_MM_CALL_METHOD(&status, &dispatcher, "dispatch");

	PHALCON_MM_ZVAL_STRING(&event_name, "console:afterHandleTask");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name, &status);

	RETURN_MM_NCTOR(&status);
}
