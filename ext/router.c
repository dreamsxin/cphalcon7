
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

#include "router.h"
#include "routerinterface.h"
#include "router/exception.h"
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
 * Phalcon\Router
 *
 * Base class for Phalcon\Router and Phalcon\Mvc\Router
 *
 */
zend_class_entry *phalcon_router_ce;

PHP_METHOD(Phalcon_Router, setDefaultModule);
PHP_METHOD(Phalcon_Router, getDefaultModule);
PHP_METHOD(Phalcon_Router, setDefaultNamespace);
PHP_METHOD(Phalcon_Router, getDefaultNamespace);
PHP_METHOD(Phalcon_Router, setDefaultHandler);
PHP_METHOD(Phalcon_Router, getDefaultHandler);
PHP_METHOD(Phalcon_Router, setDefaultAction);
PHP_METHOD(Phalcon_Router, getDefaultAction);
PHP_METHOD(Phalcon_Router, setDefaultParams);
PHP_METHOD(Phalcon_Router, getDefaultParams);
PHP_METHOD(Phalcon_Router, setCaseSensitive);
PHP_METHOD(Phalcon_Router, getCaseSensitive);
PHP_METHOD(Phalcon_Router, setMode);
PHP_METHOD(Phalcon_Router, getMode);
PHP_METHOD(Phalcon_Router, setModuleName);
PHP_METHOD(Phalcon_Router, getModuleName);
PHP_METHOD(Phalcon_Router, setNamespaceName);
PHP_METHOD(Phalcon_Router, getNamespaceName);
PHP_METHOD(Phalcon_Router, setHandlerName);
PHP_METHOD(Phalcon_Router, getHandlerName);
PHP_METHOD(Phalcon_Router, setActionName);
PHP_METHOD(Phalcon_Router, getActionName);
PHP_METHOD(Phalcon_Router, setParams);
PHP_METHOD(Phalcon_Router, getParams);

static const zend_function_entry phalcon_router_method_entry[] = {
	PHP_ME(Phalcon_Router, setDefaultModule, arginfo_phalcon_routerinterface_setdefaultmodule, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, getDefaultModule, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, setDefaultNamespace, arginfo_phalcon_routerinterface_setdefaultnamespace, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, getDefaultNamespace, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, setDefaultHandler, arginfo_phalcon_routerinterface_setdefaulthandler, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, getDefaultHandler, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, setDefaultAction, arginfo_phalcon_routerinterface_setdefaultaction, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, getDefaultAction, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, setDefaultParams, arginfo_phalcon_routerinterface_setdefaultparams, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, getDefaultParams, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, setCaseSensitive, arginfo_phalcon_routerinterface_setcasesensitive, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, getCaseSensitive, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, setMode, arginfo_phalcon_routerinterface_setmode, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, getMode, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, setModuleName, arginfo_phalcon_routerinterface_setmodulename, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, getModuleName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, setNamespaceName, arginfo_phalcon_routerinterface_setnamespacename, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, getNamespaceName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, setHandlerName, arginfo_phalcon_routerinterface_sethandlername, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, getHandlerName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, setActionName, arginfo_phalcon_routerinterface_setactionname, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, getActionName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, setParams, arginfo_phalcon_routerinterface_setparams, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Router, getParams, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Router initializer
 */
PHALCON_INIT_CLASS(Phalcon_Router){

	PHALCON_REGISTER_CLASS_EX(Phalcon, Router, router, phalcon_di_injectable_ce, phalcon_router_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_router_ce, SL("_module"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_router_ce, SL("_namespace"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_router_ce, SL("_handler"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_router_ce, SL("_action"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_router_ce, SL("_params"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_router_ce, SL("_defaultModule"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_router_ce, SL("_defaultNamespace"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_router_ce, SL("_defaultHandler"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_router_ce, SL("_defaultAction"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_router_ce, SL("_defaultParams"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_router_ce, SL("_caseSensitive"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_router_ce, SL("_mode"), PHALCON_ROUTER_MODE_DEFAULT, ZEND_ACC_PROTECTED);

	zend_declare_class_constant_long(phalcon_router_ce, SL("MODE_DEFAULT"),   PHALCON_ROUTER_MODE_DEFAULT);
	zend_declare_class_constant_long(phalcon_router_ce, SL("MODE_NONE"),   PHALCON_ROUTER_MODE_NONE);
	zend_declare_class_constant_long(phalcon_router_ce, SL("MODE_REST"),   PHALCON_ROUTER_MODE_REST);

	zend_class_implements(phalcon_router_ce, 1, phalcon_routerinterface_ce);

	return SUCCESS;
}

/**
 * Sets the name of the default module
 *
 * @param string $moduleName
 */
PHP_METHOD(Phalcon_Router, setDefaultModule){

	zval *module_name;

	phalcon_fetch_params(0, 1, 0, &module_name);

	phalcon_update_property(getThis(), SL("_defaultModule"), module_name);
	RETURN_THIS();
}

/**
 * Gets the name of the default module
 *
 * @return string
 */
PHP_METHOD(Phalcon_Router, getDefaultModule){


	RETURN_MEMBER(getThis(), "_defaultModule");
}

/**
 * Sets the name of the default namespace
 *
 * @param string $namespaceName
 */
PHP_METHOD(Phalcon_Router, setDefaultNamespace){

	zval *namespace_name;

	phalcon_fetch_params(0, 1, 0, &namespace_name);

	phalcon_update_property(getThis(), SL("_defaultNamespace"), namespace_name);
	RETURN_THIS();
}

/**
 * Gets the name of the default namespace
 *
 * @return string
 */
PHP_METHOD(Phalcon_Router, getDefaultNamespace){


	RETURN_MEMBER(getThis(), "_defaultNamespace");
}

/**
 * Sets the default handle name
 *
 * @param string $handleName
 */
PHP_METHOD(Phalcon_Router, setDefaultHandler){

	zval *handle_name;

	phalcon_fetch_params(0, 1, 0, &handle_name);

	phalcon_update_property(getThis(), SL("_defaultHandler"), handle_name);
	RETURN_THIS();
}

/**
 * Gets the default handle name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Router, getDefaultHandler){


	RETURN_MEMBER(getThis(), "_defaultHandler");
}

/**
 * Sets the default action name
 *
 * @param string $actionName
 */
PHP_METHOD(Phalcon_Router, setDefaultAction){

	zval *action_name;

	phalcon_fetch_params(0, 1, 0, &action_name);

	phalcon_update_property(getThis(), SL("_defaultAction"), action_name);
	RETURN_THIS();
}

/**
 * Gets the default action name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Router, getDefaultAction){


	RETURN_MEMBER(getThis(), "_defaultAction");
}

/**
 * Sets the default extra params
 *
 * @param string $actionName
 */
PHP_METHOD(Phalcon_Router, setDefaultParams){

	zval *action_name;

	phalcon_fetch_params(0, 1, 0, &action_name);

	phalcon_update_property(getThis(), SL("_defaultParams"), action_name);
	RETURN_THIS();
}

/**
 * Gets the default extra params
 *
 * @return string
 */
PHP_METHOD(Phalcon_Router, getDefaultParams){


	RETURN_MEMBER(getThis(), "_defaultParams");
}

/**
 * Sets the case sensitive
 * @param boolean $caseSensitive
 * @return string
 */
PHP_METHOD(Phalcon_Router, setCaseSensitive){

	zval *case_sensitive;

	phalcon_fetch_params(0, 1, 0, &case_sensitive);

	phalcon_update_property_bool(getThis(), SL("_caseSensitive"), zend_is_true(case_sensitive));

	RETURN_THIS();
}

/**
 * Returns the case sensitive
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Router, getCaseSensitive){

	RETURN_MEMBER(getThis(), "_caseSensitive");
}

/**
 * Sets the mode
 *
 * @param int $mode
 */
PHP_METHOD(Phalcon_Router, setMode){

	zval *mode;

	phalcon_fetch_params(0, 1, 0, &mode);

	phalcon_update_property(getThis(), SL("_mode"), mode);
	RETURN_THIS();
}

/**
 * Gets the mode
 *
 * @param int $mode
 */
PHP_METHOD(Phalcon_Router, getMode){


	RETURN_MEMBER(getThis(), "_mode");
}

/**
 * Sets proccesed module name
 *
 * @param string $moduleName
 */
PHP_METHOD(Phalcon_Router, setModuleName){

	zval *module_name;

	phalcon_fetch_params(0, 1, 0, &module_name);

	phalcon_update_property(getThis(), SL("_module"), module_name);
	RETURN_THIS();
}

/**
 * Returns proccesed module name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Router, getModuleName){


	RETURN_MEMBER(getThis(), "_module");
}

/**
 * Sets proccesed namespace name
 *
 * @param string $namespaceName
 */
PHP_METHOD(Phalcon_Router, setNamespaceName){

	zval *namespace_name;

	phalcon_fetch_params(0, 1, 0, &namespace_name);

	phalcon_update_property(getThis(), SL("_namespace"), namespace_name);
	RETURN_THIS();
}

/**
 * Returns proccesed namespace name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Router, getNamespaceName){


	RETURN_MEMBER(getThis(), "_namespace");
}

/**
 * Sets proccesed handle name
 *
 * @param string $handleName
 */
PHP_METHOD(Phalcon_Router, setHandlerName){

	zval *handler_name;

	phalcon_fetch_params(0, 1, 0, &handler_name);

	phalcon_update_property(getThis(), SL("_handler"), handler_name);
	RETURN_THIS();
}

/**
 * Returns proccesed handle name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Router, getHandlerName){


	RETURN_MEMBER(getThis(), "_handler");
}

/**
 * Sets proccesed action name
 *
 * @param string $actionName
 */
PHP_METHOD(Phalcon_Router, setActionName){

	zval *action_name;

	phalcon_fetch_params(0, 1, 0, &action_name);

	phalcon_update_property(getThis(), SL("_action"), action_name);
	RETURN_THIS();
}

/**
 * Returns proccesed action name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Router, getActionName){


	RETURN_MEMBER(getThis(), "_action");
}

/**
 * Sets proccesed extra params
 *
 * @param array $params
 */
PHP_METHOD(Phalcon_Router, setParams){

	zval *params;

	phalcon_fetch_params(0, 1, 0, &params);

	phalcon_update_property(getThis(), SL("_params"), params);
	RETURN_THIS();
}

/**
 * Returns proccesed extra params
 *
 * @return array
 */
PHP_METHOD(Phalcon_Router, getParams){


	RETURN_MEMBER(getThis(), "_params");
}
