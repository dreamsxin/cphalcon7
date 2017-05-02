
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
  +------------------------------------------------------------------------+
*/

#include "mvc/../application.h"
#include "application/exception.h"
#include "di/injectable.h"

#include <Zend/zend_closures.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/exception.h"
#include "kernel/operators.h"
#include "kernel/fcall.h"
#include "kernel/array.h"

/**
 * Phalcon\Application
 *
 * Base class for Phalcon\Cli\Console and Phalcon\Mvc\Application
 *
 *</code>
 */
zend_class_entry *phalcon_application_ce;

PHP_METHOD(Phalcon_Application, registerModules);
PHP_METHOD(Phalcon_Application, getModules);
PHP_METHOD(Phalcon_Application, setDefaultModule);
PHP_METHOD(Phalcon_Application, getDefaultModule);

static const zend_function_entry phalcon_application_method_entry[] = {
	PHP_ME(Phalcon_Application, registerModules, arginfo_phalcon_application_registermodules, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Application, getModules, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Application, setDefaultModule, arginfo_phalcon_application_setdefaultmodule, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Application, getDefaultModule, NULL, ZEND_ACC_PUBLIC)
	ZEND_FENTRY(handle, NULL, arginfo_phalcon_application_handle, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	PHP_FE_END
};

/**
 * Phalcon\Application initializer
 */
PHALCON_INIT_CLASS(Phalcon_Application){

	PHALCON_REGISTER_CLASS_EX(Phalcon, Application, application, phalcon_di_injectable_ce, phalcon_application_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_application_ce, SL("_defaultModule"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_application_ce, SL("_modules"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_application_ce, SL("_implicitView"), 1, ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Register an array of modules present in the application
 *
 *<code>
 *	$this->registerModules(array(
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
 * @param boolean $merge
 * @param Phalcon\Application
 */
PHP_METHOD(Phalcon_Application, registerModules){

	zval *modules, *merge = NULL, registered_modules = {}, merged_modules = {};

	phalcon_fetch_params(0, 1, 1, &modules, &merge);

	if (!merge) {
		merge = &PHALCON_GLOBAL(z_false);
	}

	if (PHALCON_IS_FALSE(merge)) {
		phalcon_update_property(getThis(), SL("_modules"), modules);
	} else {
		phalcon_read_property(&registered_modules, getThis(), SL("_modules"), PH_NOISY|PH_READONLY);
		if (Z_TYPE(registered_modules) == IS_ARRAY) {
			phalcon_fast_array_merge(&merged_modules, &registered_modules, modules);
		} else {
			ZVAL_COPY(&merged_modules, modules);
		}

		phalcon_update_property(getThis(), SL("_modules"), &merged_modules);
		zval_ptr_dtor(&merged_modules);
	}

	RETURN_THIS();
}

/**
 * Return the modules registered in the application
 *
 * @return array
 */
PHP_METHOD(Phalcon_Application, getModules){


	RETURN_MEMBER(getThis(), "_modules");
}

/**
 * Sets the module name to be used if the router doesn't return a valid module
 *
 * @param string $defaultModule
 * @return Phalcon\Application
 */
PHP_METHOD(Phalcon_Application, setDefaultModule){

	zval *default_module;

	phalcon_fetch_params(0, 1, 0, &default_module);

	phalcon_update_property(getThis(), SL("_defaultModule"), default_module);
	RETURN_THIS();
}

/**
 * Returns the default module name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Application, getDefaultModule){


	RETURN_MEMBER(getThis(), "_defaultModule");
}
