
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

#include "dispatcher.h"
#include "dispatcherinterface.h"
#include "diinterface.h"
#include "di/injectable.h"
#include "events/eventsawareinterface.h"
#include "exception.h"
#include "continueexception.h"
#include "filterinterface.h"
#include "user/logic.h"
#include "debug.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/exception.h"
#include "kernel/debug.h"

#include "interned-strings.h"

/**
 * Phalcon\Dispatcher
 *
 * This is the base class for Phalcon\Mvc\Dispatcher and Phalcon\Cli\Dispatcher.
 * This class can't be instantiated directly, you can use it to create your own dispatchers
 */
zend_class_entry *phalcon_dispatcher_ce;

PHP_METHOD(Phalcon_Dispatcher, __construct);
PHP_METHOD(Phalcon_Dispatcher, setActionSuffix);
PHP_METHOD(Phalcon_Dispatcher, getActionSuffix);
PHP_METHOD(Phalcon_Dispatcher, setHandlerSuffix);
PHP_METHOD(Phalcon_Dispatcher, getHandlerSuffix);
PHP_METHOD(Phalcon_Dispatcher, setDefaultModule);
PHP_METHOD(Phalcon_Dispatcher, getDefaultModule);
PHP_METHOD(Phalcon_Dispatcher, setDefaultNamespace);
PHP_METHOD(Phalcon_Dispatcher, getDefaultNamespace);
PHP_METHOD(Phalcon_Dispatcher, setDefaultHandler);
PHP_METHOD(Phalcon_Dispatcher, getDefaultHandler);
PHP_METHOD(Phalcon_Dispatcher, setDefaultAction);
PHP_METHOD(Phalcon_Dispatcher, getDefaultAction);
PHP_METHOD(Phalcon_Dispatcher, setModuleName);
PHP_METHOD(Phalcon_Dispatcher, getModuleName);
PHP_METHOD(Phalcon_Dispatcher, setNamespaceName);
PHP_METHOD(Phalcon_Dispatcher, getNamespaceName);
PHP_METHOD(Phalcon_Dispatcher, setHandlerName);
PHP_METHOD(Phalcon_Dispatcher, getHandlerName);
PHP_METHOD(Phalcon_Dispatcher, setActionName);
PHP_METHOD(Phalcon_Dispatcher, getActionName);
PHP_METHOD(Phalcon_Dispatcher, setLogicBinding);
PHP_METHOD(Phalcon_Dispatcher, isLogicBinding);
PHP_METHOD(Phalcon_Dispatcher, setParams);
PHP_METHOD(Phalcon_Dispatcher, getParams);
PHP_METHOD(Phalcon_Dispatcher, hasParam);
PHP_METHOD(Phalcon_Dispatcher, setParam);
PHP_METHOD(Phalcon_Dispatcher, getParam);
PHP_METHOD(Phalcon_Dispatcher, getActiveHandler);
PHP_METHOD(Phalcon_Dispatcher, getActiveMethod);
PHP_METHOD(Phalcon_Dispatcher, isFinished);
PHP_METHOD(Phalcon_Dispatcher, setFinished);
PHP_METHOD(Phalcon_Dispatcher, setReturnedValue);
PHP_METHOD(Phalcon_Dispatcher, getReturnedValue);
PHP_METHOD(Phalcon_Dispatcher, dispatch);
PHP_METHOD(Phalcon_Dispatcher, forward);
PHP_METHOD(Phalcon_Dispatcher, wasForwarded);
PHP_METHOD(Phalcon_Dispatcher, getHandlerClass);
PHP_METHOD(Phalcon_Dispatcher, camelizeNamespace);
PHP_METHOD(Phalcon_Dispatcher, camelizeController);
PHP_METHOD(Phalcon_Dispatcher, setErrorHandler);
PHP_METHOD(Phalcon_Dispatcher, getErrorHandler);
PHP_METHOD(Phalcon_Dispatcher, getLastException);
PHP_METHOD(Phalcon_Dispatcher, getLastHandler);
PHP_METHOD(Phalcon_Dispatcher, getPreviousNamespaceName);
PHP_METHOD(Phalcon_Dispatcher, getPreviousActionName);
PHP_METHOD(Phalcon_Dispatcher, getPreviousParams);
PHP_METHOD(Phalcon_Dispatcher, getPreviousParam);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_dispatcher_setreturnedvalue, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_dispatcher_setfinished, 0, 0, 1)
	ZEND_ARG_INFO(0, finished)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_dispatcher_method_entry[] = {
	PHP_ME(Phalcon_Dispatcher, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Dispatcher, setActionSuffix, arginfo_phalcon_dispatcherinterface_setactionsuffix, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getActionSuffix, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, setHandlerSuffix, arginfo_phalcon_dispatcherinterface_sethandlersuffix, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getHandlerSuffix, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, setDefaultModule, arginfo_phalcon_dispatcherinterface_setdefaultmodule, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getDefaultModule, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, setDefaultNamespace, arginfo_phalcon_dispatcherinterface_setdefaultnamespace, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getDefaultNamespace, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, setDefaultHandler, arginfo_phalcon_dispatcherinterface_setdefaulthandler, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getDefaultHandler, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, setDefaultAction, arginfo_phalcon_dispatcherinterface_setdefaultaction, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getDefaultAction, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, setModuleName, arginfo_phalcon_dispatcherinterface_setmodulename, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getModuleName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, setNamespaceName, arginfo_phalcon_dispatcherinterface_setnamespacename, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getNamespaceName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, setHandlerName, arginfo_phalcon_dispatcherinterface_sethandlername, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getHandlerName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, setActionName, arginfo_phalcon_dispatcherinterface_setactionname, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getActionName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, setLogicBinding, arginfo_phalcon_dispatcherinterface_setlogicbinding, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, isLogicBinding, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, setParams, arginfo_phalcon_dispatcherinterface_setparams, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getParams, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, hasParam, arginfo_phalcon_dispatcherinterface_hasparam, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, setParam, arginfo_phalcon_dispatcherinterface_setparam, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getParam, arginfo_phalcon_dispatcherinterface_getparam, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getActiveHandler, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getActiveMethod, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, isFinished, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, setFinished, arginfo_phalcon_dispatcher_setfinished, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, setReturnedValue, arginfo_phalcon_dispatcher_setreturnedvalue, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getReturnedValue, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, dispatch, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, forward, arginfo_phalcon_dispatcherinterface_forward, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, wasForwarded, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getHandlerClass, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, camelizeNamespace, arginfo_phalcon_dispatcherinterface_camelizenamespace, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, camelizeController, arginfo_phalcon_dispatcherinterface_camelizecontroller, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, setErrorHandler, arginfo_phalcon_dispatcherinterface_seterrorhandler, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getErrorHandler, arginfo_phalcon_dispatcherinterface_geterrorhandler, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getLastException, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getLastHandler, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getPreviousNamespaceName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getPreviousActionName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getPreviousParams, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Dispatcher, getPreviousParam, arginfo_phalcon_dispatcherinterface_getparam, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Dispatcher initializer
 */
PHALCON_INIT_CLASS(Phalcon_Dispatcher){

	PHALCON_REGISTER_CLASS_EX(Phalcon, Dispatcher, dispatcher, phalcon_di_injectable_ce, phalcon_dispatcher_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_dispatcher_ce, SL("_activeHandler"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_dispatcher_ce, SL("_finished"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_dispatcher_ce, SL("_forwarded"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_dispatcher_ce, SL("_moduleName"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_dispatcher_ce, SL("_namespaceName"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_dispatcher_ce, SL("_handlerName"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_dispatcher_ce, SL("_actionName"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_dispatcher_ce, SL("_logicBinding"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_dispatcher_ce, SL("_params"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_dispatcher_ce, SL("_returnedValue"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_dispatcher_ce, SL("_lastHandler"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_dispatcher_ce, SL("_defaultModule"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_dispatcher_ce, SL("_defaultNamespace"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_dispatcher_ce, SL("_defaultHandler"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_dispatcher_ce, SL("_defaultAction"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_dispatcher_ce, SL("_handlerSuffix"), ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_dispatcher_ce, SL("_actionSuffix"), "Action", ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_dispatcher_ce, SL("_isExactHandler"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_dispatcher_ce, SL("_previousNamespaceName"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_dispatcher_ce, SL("_previousHandlerName"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_dispatcher_ce, SL("_previousActionName"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_dispatcher_ce, SL("_previousParams"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_dispatcher_ce, SL("_camelizeNamespace"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_dispatcher_ce, SL("_camelizeController"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_dispatcher_ce, SL("_errorHandlers"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_dispatcher_ce, SL("_lastException"), ZEND_ACC_PROTECTED);

	zend_declare_class_constant_long(phalcon_dispatcher_ce, SL("EXCEPTION_NO_DI"), PHALCON_EXCEPTION_NO_DI);
	zend_declare_class_constant_long(phalcon_dispatcher_ce, SL("EXCEPTION_CYCLIC_ROUTING"), PHALCON_EXCEPTION_CYCLIC_ROUTING);
	zend_declare_class_constant_long(phalcon_dispatcher_ce, SL("EXCEPTION_HANDLER_NOT_FOUND"), PHALCON_EXCEPTION_HANDLER_NOT_FOUND);
	zend_declare_class_constant_long(phalcon_dispatcher_ce, SL("EXCEPTION_INVALID_HANDLER"), PHALCON_EXCEPTION_INVALID_HANDLER);
	zend_declare_class_constant_long(phalcon_dispatcher_ce, SL("EXCEPTION_INVALID_PARAMS"), PHALCON_EXCEPTION_INVALID_PARAMS);
	zend_declare_class_constant_long(phalcon_dispatcher_ce, SL("EXCEPTION_ACTION_NOT_FOUND"), PHALCON_EXCEPTION_ACTION_NOT_FOUND);

	zend_class_implements(phalcon_dispatcher_ce, 1, phalcon_dispatcherinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Dispatcher constructor
 */
PHP_METHOD(Phalcon_Dispatcher, __construct){

	phalcon_update_property_empty_array(getThis(), SL("_params"));
}

/**
 * Sets the default action suffix
 *
 * @param string $actionSuffix
 */
PHP_METHOD(Phalcon_Dispatcher, setActionSuffix){

	zval *action_suffix;

	phalcon_fetch_params(0, 1, 0, &action_suffix);

	phalcon_update_property(getThis(), SL("_actionSuffix"), action_suffix);

}

/**
 * Gets the default action suffix
 *
 * @return string
 */
PHP_METHOD(Phalcon_Dispatcher, getActionSuffix){

	RETURN_MEMBER(getThis(), "_actionSuffix");
}

/**
 * Sets the default handler suffix
 *
 * @param string $handlerSuffix
 */
PHP_METHOD(Phalcon_Dispatcher, setHandlerSuffix){

	zval *handler_suffix;

	phalcon_fetch_params(0, 1, 0, &handler_suffix);

	phalcon_update_property(getThis(), SL("_handlerSuffix"), handler_suffix);

}

/**
 * Gets the default handler suffix
 *
 * @return string
 */
PHP_METHOD(Phalcon_Dispatcher, getHandlerSuffix){

	RETURN_MEMBER(getThis(), "_handlerSuffix");
}

/**
 * Sets the default module
 *
 * @param string $module
 */
PHP_METHOD(Phalcon_Dispatcher, setDefaultModule){

	zval *module;

	phalcon_fetch_params(0, 1, 0, &module);

	phalcon_update_property(getThis(), SL("_defaultModule"), module);

}

/**
 * Returns the default module
 *
 * @return string
 */
PHP_METHOD(Phalcon_Dispatcher, getDefaultModule){


	RETURN_MEMBER(getThis(), "_defaultModule");
}

/**
 * Sets the default namespace
 *
 * @param string $namespace
 */
PHP_METHOD(Phalcon_Dispatcher, setDefaultNamespace){

	zval *namespace;

	phalcon_fetch_params(0, 1, 0, &namespace);

	phalcon_update_property(getThis(), SL("_defaultNamespace"), namespace);

}

/**
 * Returns the default namespace
 *
 * @return string
 */
PHP_METHOD(Phalcon_Dispatcher, getDefaultNamespace){


	RETURN_MEMBER(getThis(), "_defaultNamespace");
}

/**
 * Sets the default handler
 *
 * @param string $handler
 */
PHP_METHOD(Phalcon_Dispatcher, setDefaultHandler){

	zval *handler;

	phalcon_fetch_params(0, 1, 0, &handler);

	phalcon_update_property(getThis(), SL("_defaultHandler"), handler);

}

/**
 * Returns the default handler
 *
 * @return string
 */
PHP_METHOD(Phalcon_Dispatcher, getDefaultHandler){


	RETURN_MEMBER(getThis(), "_defaultHandler");
}

/**
 * Sets the default action name
 *
 * @param string $actionName
 */
PHP_METHOD(Phalcon_Dispatcher, setDefaultAction){

	zval *action_name;

	phalcon_fetch_params(0, 1, 0, &action_name);

	phalcon_update_property(getThis(), SL("_defaultAction"), action_name);

}

/**
 * Returns the default action
 *
 * @return string
 */
PHP_METHOD(Phalcon_Dispatcher, getDefaultAction){


	RETURN_MEMBER(getThis(), "_defaultAction");
}

/**
 * Sets the module where the controller is (only informative)
 *
 * @param string $moduleName
 */
PHP_METHOD(Phalcon_Dispatcher, setModuleName){

	zval *module_name;

	phalcon_fetch_params(0, 1, 0, &module_name);

	phalcon_update_property(getThis(), SL("_moduleName"), module_name);

}

/**
 * Gets the module where the controller class is
 *
 * @return string
 */
PHP_METHOD(Phalcon_Dispatcher, getModuleName){


	RETURN_MEMBER(getThis(), "_moduleName");
}

/**
 * Sets the namespace where the controller class is
 *
 * @param string $namespaceName
 */
PHP_METHOD(Phalcon_Dispatcher, setNamespaceName){

	zval *namespace_name;

	phalcon_fetch_params(0, 1, 0, &namespace_name);

	phalcon_update_property(getThis(), SL("_namespaceName"), namespace_name);

}

/**
 * Gets a namespace to be prepended to the current handler name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Dispatcher, getNamespaceName){


	RETURN_MEMBER(getThis(), "_namespaceName");
}

/**
 * Sets the action name to be dispatched
 *
 * @param string $handlerName
 */
PHP_METHOD(Phalcon_Dispatcher, setHandlerName){

	zval *handler_name;

	phalcon_fetch_params(0, 1, 0, &handler_name);

	phalcon_update_property(getThis(), SL("_handlerName"), handler_name);

}

/**
 * Gets the lastest dispatched handler name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Dispatcher, getHandlerName){


	RETURN_MEMBER(getThis(), "_handlerName");
}

/**
 * Sets the action name to be dispatched
 *
 * @param string $actionName
 */
PHP_METHOD(Phalcon_Dispatcher, setActionName){

	zval *action_name;

	phalcon_fetch_params(0, 1, 0, &action_name);

	phalcon_update_property(getThis(), SL("_actionName"), action_name);

}

/**
 * Gets the lastest dispatched action name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Dispatcher, getActionName){


	RETURN_MEMBER(getThis(), "_actionName");
}

/**
 * Enable/Disable logic binding during dispatch
 *
 * @param boolean $value
 */
PHP_METHOD(Phalcon_Dispatcher, setLogicBinding){

	zval *value;

	phalcon_fetch_params(0, 1, 0, &value);

	if (PHALCON_IS_TRUE(value)) {
		phalcon_update_property(getThis(), SL("_logicBinding"), &PHALCON_GLOBAL(z_true));
	} else {
		phalcon_update_property(getThis(), SL("_logicBinding"), &PHALCON_GLOBAL(z_false));
	}
}

/**
 * Check if logic binding
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Dispatcher, isLogicBinding){


	RETURN_MEMBER(getThis(), "_logicBinding");
}

/**
 * Sets action params to be dispatched
 *
 * @param array $params
 */
PHP_METHOD(Phalcon_Dispatcher, setParams){

	zval *params;

	phalcon_fetch_params(0, 1, 0, &params);
	PHALCON_SEPARATE_PARAM(params);

	phalcon_update_property(getThis(), SL("_params"), params);
}

/**
 * Gets action params
 *
 * @return array
 */
PHP_METHOD(Phalcon_Dispatcher, getParams){


	RETURN_MEMBER(getThis(), "_params");
}

/**
 * Check if a param exists
 *
 * @param mixed param
 * @return boolean
 */
PHP_METHOD(Phalcon_Dispatcher, hasParam){

	zval *param, params = {};

	phalcon_fetch_params(0, 1, 0, &param);

	phalcon_read_property(&params, getThis(), SL("_params"), PH_NOISY|PH_READONLY);
	if (phalcon_array_isset(&params, param)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Set a param by its name or numeric index
 *
 * @param mixed $param
 * @param mixed $value
 */
PHP_METHOD(Phalcon_Dispatcher, setParam){

	zval *param, *value;

	phalcon_fetch_params(0, 2, 0, &param, &value);

	phalcon_update_property_array(getThis(), SL("_params"), param, value);

}

/**
 * Gets a param by its name or numeric index
 *
 * @param mixed $param
 * @param string|array $filters
 * @param mixed $defaultValue
 * @return mixed
 */
PHP_METHOD(Phalcon_Dispatcher, getParam){

	zval *param, *filters = NULL, *default_value = NULL, params = {}, param_value = {}, dependency_injector = {}, exception_code = {}, exception_message = {};
	zval service = {}, filter = {};

	phalcon_fetch_params(1, 1, 2, &param, &filters, &default_value);

	phalcon_read_property(&params, getThis(), SL("_params"), PH_NOISY|PH_READONLY);
	if (phalcon_array_isset_fetch(&param_value, &params, param, PH_READONLY)) {
		if (filters && Z_TYPE_P(filters) != IS_NULL) {
			PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi");
			PHALCON_MM_ADD_ENTRY(&dependency_injector);
			if (Z_TYPE(dependency_injector) != IS_OBJECT) {
				ZVAL_LONG(&exception_code, PHALCON_EXCEPTION_NO_DI);
				PHALCON_MM_ZVAL_STRING(&exception_message, "A dependency injection object is required to access the 'filter' service");
				PHALCON_MM_CALL_METHOD(NULL, getThis(), "_throwdispatchexception", &exception_message, &exception_code);
				RETURN_MM();
			}

			ZVAL_STR(&service, IS(filter));

			PHALCON_MM_CALL_METHOD(&filter, &dependency_injector, "getshared", &service);
			PHALCON_MM_ADD_ENTRY(&filter);
			PHALCON_MM_VERIFY_INTERFACE(&filter, phalcon_filterinterface_ce);
			PHALCON_MM_RETURN_CALL_METHOD(&filter, "sanitize", &param_value, filters);
			RETURN_MM();
		} else {
			RETURN_MM_CTOR(&param_value);
		}
	}

	if (default_value) {
		RETURN_MM_CTOR(default_value);
	}

	RETURN_MM_NULL();
}

/**
 * Returns the current handler to be/executed in the dispatcher
 *
 * @return Phalcon\Mvc\Controller
 */
PHP_METHOD(Phalcon_Dispatcher, getActiveHandler){

	RETURN_MEMBER(getThis(), "_activeHandler");
}

/**
 * Returns the current method to be/executed in the dispatcher
 *
 * @return string
 */
PHP_METHOD(Phalcon_Dispatcher, getActiveMethod){

	zval suffix = {}, action_name = {};

	phalcon_read_property(&suffix, getThis(), SL("_actionSuffix"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&action_name, getThis(), SL("_actionName"), PH_NOISY|PH_READONLY);

	PHALCON_CONCAT_VV(return_value, &action_name, &suffix);
}

/**
 * Checks if the dispatch loop is finished or has more pendent controllers/tasks to disptach
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Dispatcher, isFinished){


	RETURN_MEMBER(getThis(), "_finished");
}

/**
 * Sets the finished
 *
 * @param boolean $finished
 */
PHP_METHOD(Phalcon_Dispatcher, setFinished){

	zval *finished;

	phalcon_fetch_params(0, 1, 0, &finished);

	if (PHALCON_IS_TRUE(finished)) {
		phalcon_update_property(getThis(), SL("_finished"), &PHALCON_GLOBAL(z_true) TSRMLS_CC);
	} else {
		phalcon_update_property(getThis(), SL("_finished"), &PHALCON_GLOBAL(z_false) TSRMLS_CC);
	}
}

/**
 * Sets the latest returned value by an action manually
 *
 * @param mixed $value
 */
PHP_METHOD(Phalcon_Dispatcher, setReturnedValue){

	zval *value;

	phalcon_fetch_params(0, 1, 0, &value);

	phalcon_update_property(getThis(), SL("_returnedValue"), value);

}

/**
 * Returns value returned by the lastest dispatched action
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Dispatcher, getReturnedValue){


	RETURN_MEMBER(getThis(), "_returnedValue");
}

/**
 * Dispatches a handle action taking into account the routing parameters
 *
 * @return object
 */
PHP_METHOD(Phalcon_Dispatcher, dispatch){

	zval dependency_injector = {}, events_manager = {}, event_name = {}, exception_code = {}, exception_message = {}, status = {}, handler = {};
	zval handler_suffix = {}, action_suffix = {}, debug_message = {};
	int number_dispatches = 0, max_dispatches = 256;

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		ZVAL_STRING(&debug_message, "Dispatch: ");
		PHALCON_DEBUG_LOG(&debug_message);
		zval_ptr_dtor(&debug_message);
	}

	PHALCON_MM_INIT();

	PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi");
	PHALCON_MM_ADD_ENTRY(&dependency_injector);
	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		ZVAL_LONG(&exception_code, PHALCON_EXCEPTION_NO_DI);
		PHALCON_MM_ZVAL_STRING(&exception_message, "A dependency injection container is required to access related dispatching services");
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "_throwdispatchexception", &exception_message, &exception_code);
		RETURN_MM();
	}

	PHALCON_MM_CALL_METHOD(&events_manager, getThis(), "geteventsmanager");
	PHALCON_MM_ADD_ENTRY(&events_manager);

	/**
	 * Calling beforeDispatchLoop
	 */
	PHALCON_MM_ZVAL_STRING(&event_name, "dispatch:beforeDispatchLoop");
	PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);

	if (PHALCON_IS_FALSE(&status)) {
		RETURN_MM_FALSE;
	}
	zval_ptr_dtor(&status);

	phalcon_read_property(&handler_suffix, getThis(), SL("_handlerSuffix"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&action_suffix, getThis(), SL("_actionSuffix"), PH_NOISY|PH_READONLY);

	/**
	 * Do at least one dispatch
	 */
	phalcon_update_property(getThis(), SL("_finished"), &PHALCON_GLOBAL(z_false));

	do {
		zval finished = {}, namespace_name = {}, handler_name = {}, action_name = {}, camelize = {}, camelized_class = {};
		zval handler_class = {}, has_service = {}, was_fresh = {}, action_method = {}, action_params = {}, params = {}, tmp_params = {}, *param, logic_binding = {}, reflection_method = {};
		zval reflection_parameters = {}, *reflection_parameter, call_object = {}, value = {}, exception = {};
		zend_class_entry *reflection_method_ce;
		zend_string *param_key;
		ulong param_idx;
		long int count_action_params = 0;

		if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
			zval times = {};
			ZVAL_LONG(&times, number_dispatches);
			PHALCON_CONCAT_SV(&debug_message, "--number_dispatches: ", &times);
			PHALCON_DEBUG_LOG(&debug_message);
			zval_ptr_dtor(&debug_message);
		}

		/**
		 * Loop until finished is false
		 */
		phalcon_read_property(&finished, getThis(), SL("_finished"), PH_READONLY);
		if (zend_is_true(&finished)) {
			break;
		}

		++number_dispatches;

		/**
		 * Throw an exception after 256 consecutive forwards
		 */
		if (number_dispatches == max_dispatches) {
			ZVAL_LONG(&exception_code, PHALCON_EXCEPTION_CYCLIC_ROUTING);
			PHALCON_MM_ZVAL_STRING(&exception_message, "Dispatcher has detected a cyclic routing causing stability problems");
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "_throwdispatchexception", &exception_message, &exception_code);
			break;
		}

		phalcon_update_property(getThis(), SL("_finished"), &PHALCON_GLOBAL(z_true));

		/**
		 * If the current namespace is null we used the set in this_ptr::_defaultNamespace
		 */
		phalcon_read_property(&namespace_name, getThis(), SL("_namespaceName"), PH_READONLY);
		if (!zend_is_true(&namespace_name)) {
			phalcon_read_property(&namespace_name, getThis(), SL("_defaultNamespace"), PH_READONLY);
			phalcon_update_property(getThis(), SL("_namespaceName"), &namespace_name);
		}

		/**
		 * If the handler is null we use the set in this_ptr::_defaultHandler
		 */
		phalcon_read_property(&handler_name, getThis(), SL("_handlerName"), PH_READONLY);
		if (!zend_is_true(&handler_name)) {
			phalcon_read_property(&handler_name, getThis(), SL("_defaultHandler"), PH_READONLY);
			phalcon_update_property(getThis(), SL("_handlerName"), &handler_name);
		}

		/**
		 * If the action is null we use the set in this_ptr::_defaultAction
		 */
		phalcon_read_property(&action_name, getThis(), SL("_actionName"), PH_READONLY);
		if (!zend_is_true(&action_name)) {
			phalcon_read_property(&action_name, getThis(), SL("_defaultAction"), PH_READONLY);
			phalcon_update_property(getThis(), SL("_actionName"), &action_name);
		}

		if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
			PHALCON_CONCAT_SV(&debug_message, "--namespace: ", &namespace_name);
			PHALCON_DEBUG_LOG(&debug_message);
			zval_ptr_dtor(&debug_message);
			PHALCON_CONCAT_SV(&debug_message, "--handler name: ", &handler_name);
			PHALCON_DEBUG_LOG(&debug_message);
			zval_ptr_dtor(&debug_message);
			PHALCON_CONCAT_SV(&debug_message, "--action name: ", &action_name);
			PHALCON_DEBUG_LOG(&debug_message);
			zval_ptr_dtor(&debug_message);
		}

		/**
		 * Calling beforeDispatch
		 */
		if (Z_TYPE(events_manager) == IS_OBJECT) {
			PHALCON_MM_ZVAL_STRING(&event_name, "dispatch:beforeDispatch");
			PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
			if (PHALCON_IS_FALSE(&status)) {
				continue;
			}
			zval_ptr_dtor(&status);

			/**
			 * Check if the user made a forward in the listener
			 */
			phalcon_read_property(&finished, getThis(), SL("_finished"), PH_READONLY);
			if (PHALCON_IS_FALSE(&finished)) {
				continue;
			}
		}

		/**
		 * We don't camelize the classes if they are in namespaces
		 */
		if (!phalcon_memnstr_str(&handler_name, SL("\\"))) {
			phalcon_read_property(&camelize, getThis(), SL("_camelizeController"), PH_READONLY);
			if (!zend_is_true(&camelize)) {
				PHALCON_MM_ZVAL_COPY(&camelized_class, &handler_name);
			} else {
				phalcon_camelize(&camelized_class, &handler_name);
				PHALCON_MM_ADD_ENTRY(&camelized_class);
			}
		} else if (phalcon_start_with_str(&handler_name, SL("\\"))) {
			PHALCON_MM_ZVAL_STRINGL(&camelized_class, Z_STRVAL(handler_name)+1, Z_STRLEN(handler_name)-1);
		} else {
			PHALCON_MM_ZVAL_COPY(&camelized_class, &handler_name);
		}

		/**
		 * Create the complete controller class name prepending the namespace
		 */
		if (zend_is_true(&namespace_name)) {
			zval camelized_namespace = {};
			phalcon_read_property(&camelize, getThis(), SL("_camelizeNamespace"), PH_READONLY);
			if (!zend_is_true(&camelize)) {
				ZVAL_COPY(&camelized_namespace, &namespace_name);
			} else {
				phalcon_camelize(&camelized_namespace, &namespace_name);
			}
			if (phalcon_end_with_str(&camelized_namespace, SL("\\"))) {
				PHALCON_CONCAT_VVV(&handler_class, &camelized_namespace, &camelized_class, &handler_suffix);
			} else {
				PHALCON_CONCAT_VSVV(&handler_class, &camelized_namespace, "\\", &camelized_class, &handler_suffix);
			}
			zval_ptr_dtor(&camelized_namespace);
		} else {
			PHALCON_CONCAT_VV(&handler_class, &camelized_class, &handler_suffix);
		}
		PHALCON_MM_ADD_ENTRY(&handler_class);

		/**
		 * Handlers are retrieved as shared instances from the Service Container
		 */
		PHALCON_MM_CALL_METHOD(&has_service, &dependency_injector, "has", &handler_class);
		if (!zend_is_true(&has_service)) {
			/**
			 * DI doesn't have a service with that name, try to load it using an autoloader
			 */
			assert(Z_TYPE(handler_class) == IS_STRING);
			ZVAL_BOOL(&has_service, (phalcon_class_exists(&handler_class, 1) != NULL) ? 1 : 0);

		}

		/**
		 * If the service cannot be loaded we throw an exception
		 */
		if (!zend_is_true(&has_service)) {
			ZVAL_LONG(&exception_code, PHALCON_EXCEPTION_HANDLER_NOT_FOUND);
			PHALCON_CONCAT_VS(&exception_message, &handler_class, " handler class cannot be loaded");
			PHALCON_MM_ADD_ENTRY(&exception_message);
			PHALCON_MM_CALL_METHOD(&status, getThis(), "_throwdispatchexception", &exception_message, &exception_code);
			if (PHALCON_IS_FALSE(&status)) {
				/**
				 * Check if the user made a forward in the listener
				 */
				phalcon_read_property(&finished, getThis(), SL("_finished"), PH_READONLY);
				if (PHALCON_IS_FALSE(&finished)) {
					continue;
				}
			}

			break;
		}

		/**
		 * Handlers must be only objects
		 */
		PHALCON_MM_CALL_METHOD(&handler, &dependency_injector, "getshared", &handler_class);
		PHALCON_MM_ADD_ENTRY(&handler);
		if (Z_TYPE(handler) != IS_OBJECT) {
			ZVAL_LONG(&exception_code, PHALCON_EXCEPTION_INVALID_HANDLER);
			PHALCON_MM_ZVAL_STRING(&exception_message, "Invalid handler returned from the services container");
			PHALCON_MM_CALL_METHOD(&status, getThis(), "_throwdispatchexception", &exception_message, &exception_code);
			if (PHALCON_IS_FALSE(&status)) {
				phalcon_read_property(&finished, getThis(), SL("_finished"), PH_READONLY);
				if (PHALCON_IS_FALSE(&finished)) {
					continue;
				}
			}
			zval_ptr_dtor(&status);

			break;
		}

		/**
		 * If the object was recently created in the DI we initialize it
		 */
		PHALCON_MM_CALL_METHOD(&was_fresh, &dependency_injector, "wasfreshinstance");

		/**
		 * Update the active handler making it available for events
		 */
		phalcon_update_property(getThis(), SL("_activeHandler"), &handler);

		/**
		 * Check if the method exists in the handler
		 */
		PHALCON_CONCAT_VV(&action_method, &action_name, &action_suffix);
		PHALCON_MM_ADD_ENTRY(&action_method);
		if (phalcon_method_exists(&handler, &action_method) == FAILURE) {
			/**
			 * Call beforeNotFoundAction
			 */
			if (Z_TYPE(events_manager) == IS_OBJECT) {
				PHALCON_MM_ZVAL_STRING(&event_name, "dispatch:beforeNotFoundAction");
				PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);

				if (PHALCON_IS_FALSE(&status)) {
					continue;
				}
				zval_ptr_dtor(&status);

				phalcon_read_property(&finished, getThis(), SL("_finished"), PH_READONLY);
				if (PHALCON_IS_FALSE(&finished)) {
					continue;
				}
			}

			ZVAL_LONG(&exception_code, PHALCON_EXCEPTION_ACTION_NOT_FOUND);
			PHALCON_CONCAT_SVSVS(&exception_message, "Action '", &action_name, "' was not found on handler '", &handler_name, "'");
			PHALCON_MM_ADD_ENTRY(&exception_message);

			/**
			 * Try to throw an exception when an action isn't defined on the object
			 */
			PHALCON_MM_CALL_METHOD(&status, getThis(), "_throwdispatchexception", &exception_message, &exception_code);
			if (PHALCON_IS_FALSE(&status)) {
				phalcon_read_property(&finished, getThis(), SL("_finished"), PH_READONLY);
				if (PHALCON_IS_FALSE(&finished)) {
					continue;
				}
			}
			zval_ptr_dtor(&status);
			break;
		}

		/**
		 * Calling beforeExecuteRoute
		 */
		if (Z_TYPE(events_manager) == IS_OBJECT) {
			PHALCON_MM_ZVAL_STRING(&event_name, "dispatch:beforeExecuteRoute");
			PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
			if (PHALCON_IS_FALSE(&status)) {
				continue;
			}
			zval_ptr_dtor(&status);

			/**
			 * Check if the user made a forward in the listener
			 */
			phalcon_read_property(&finished, getThis(), SL("_finished"), PH_READONLY);
			if (PHALCON_IS_FALSE(&finished)) {
				continue;
			}
		}

		/**
		 * Calling beforeExecuteRoute as callback and event
		 */
		if (phalcon_method_exists_ex(&handler, SL("beforeexecuteroute")) == SUCCESS) {
			PHALCON_MM_CALL_METHOD(&status, &handler, "beforeexecuteroute", getThis());
			if (PHALCON_IS_FALSE(&status)) {
				continue;
			}

			/**
			 * Check if the user made a forward in the listener
			 */
			phalcon_read_property(&finished, getThis(), SL("_finished"), PH_READONLY);
			if (PHALCON_IS_FALSE(&finished)) {
				continue;
			}
		}

		/**
		 * Call the 'initialize' method just once per request
		 */
		if (PHALCON_IS_TRUE(&was_fresh)) {
			/**
			 * Calling afterInitialize
			 */
			if (unlikely(Z_TYPE(events_manager) == IS_OBJECT)) {
				PHALCON_MM_ZVAL_STRING(&event_name, "dispatch:beforeInitialize");
				PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
				if (PHALCON_IS_FALSE(&status)) {
					continue;
				}
				zval_ptr_dtor(&status);

				/**
				 * Check if the user made a forward in the listener
				 */
				phalcon_read_property(&finished, getThis(), SL("_finished"), PH_READONLY);
				if (PHALCON_IS_FALSE(&finished)) {
					continue;
				}
			}

			if (phalcon_method_exists_ex(&handler, SL("initialize")) == SUCCESS) {
				int flag = 0;
				PHALCON_CALL_METHOD_FLAG(flag, NULL, &handler, "initialize");
				if (flag && EG(exception)) {
					zval e = {};
					ZVAL_OBJ(&e, EG(exception));
					
					if (likely(!instanceof_function_ex(Z_OBJCE(e), phalcon_continueexception_ce, 0))) {
						RETURN_MM();
					}

					ZVAL_OBJ(&exception, zend_objects_clone_obj(&e));
					PHALCON_MM_ADD_ENTRY(&exception);
					phalcon_update_property(getThis(), SL("_lastException"), &exception);

					/* Clear the exception  */
					zend_clear_exception();

					/**
					 * Calling afterInitialize
					 */
					if (unlikely(Z_TYPE(events_manager) == IS_OBJECT)) {
						PHALCON_MM_ZVAL_STRING(&event_name, "dispatch:afterInitialize");
						PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
						if (PHALCON_IS_FALSE(&status)) {
							continue;
						}
						zval_ptr_dtor(&status);

						/**
						 * Check if the user made a forward in the listener
						 */
						phalcon_read_property(&finished, getThis(), SL("_finished"), PH_READONLY);
						if (PHALCON_IS_FALSE(&finished)) {
							continue;
						}
					}

					phalcon_update_property(getThis(), SL("_lastHandler"), &handler);
					goto afterexecuteroute;
				}
			}

			/**
			 * Calling afterInitialize
			 */
			if (unlikely(Z_TYPE(events_manager) == IS_OBJECT)) {
				PHALCON_MM_ZVAL_STRING(&event_name, "dispatch:afterInitialize");
				PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
				if (PHALCON_IS_FALSE(&status)) {
					continue;
				}
				zval_ptr_dtor(&status);

				/**
				 * Check if the user made a forward in the listener
				 */
				phalcon_read_property(&finished, getThis(), SL("_finished"), PH_READONLY);
				if (PHALCON_IS_FALSE(&finished)) {
					continue;
				}
			}
		}

		/**
		 * Check if the params is an array
		 */
		phalcon_read_property(&action_params, getThis(), SL("_params"), PH_READONLY);
		if (Z_TYPE(action_params) != IS_ARRAY) {
			ZVAL_LONG(&exception_code, PHALCON_EXCEPTION_INVALID_PARAMS);
			PHALCON_MM_ZVAL_STRING(&exception_message, "Action parameters must be an Array");

			/**
			 * An invalid parameter variable was passed throw an exception
			 */
			PHALCON_MM_CALL_METHOD(&status, getThis(), "_throwdispatchexception", &exception_message, &exception_code);
			if (PHALCON_IS_FALSE(&status)) {
				phalcon_read_property(&finished, getThis(), SL("_finished"), PH_READONLY);
				if (PHALCON_IS_FALSE(&finished)) {
					continue;
				}
			}
			break;
		}

		/**
		 * Check if logic binding
		 */
		phalcon_read_property(&logic_binding, getThis(), SL("_logicBinding"), PH_READONLY);
		if (zend_is_true(&logic_binding)) {
			PHALCON_MM_ZVAL_DUP(&tmp_params, &action_params);

			count_action_params = phalcon_fast_count_int(&action_params);

			array_init(&params);
			PHALCON_MM_ADD_ENTRY(&params);

			reflection_method_ce = phalcon_fetch_str_class(SL("ReflectionMethod"), ZEND_FETCH_CLASS_AUTO);
			object_init_ex(&reflection_method, reflection_method_ce);
			PHALCON_MM_ADD_ENTRY(&reflection_method);
			PHALCON_MM_CALL_METHOD(NULL, &reflection_method, "__construct", &handler, &action_method);
			PHALCON_MM_CALL_METHOD(&reflection_parameters, &reflection_method, "getparameters");
			PHALCON_MM_ADD_ENTRY(&reflection_parameters);

			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(reflection_parameters), param_idx, param_key, reflection_parameter) {
				zval key = {}, reflection_class = {}, logic_classname = {}, logic = {}, var_name = {}, var_value = {};
				zend_class_entry *logic_ce;
				if (param_key) {
					ZVAL_STR(&key, param_key);
				} else {
					ZVAL_LONG(&key, param_idx);
				}

				PHALCON_MM_CALL_METHOD(&reflection_class, reflection_parameter, "getclass");
				PHALCON_MM_ADD_ENTRY(&reflection_class);
				if (Z_TYPE(reflection_class) == IS_OBJECT) {
					PHALCON_MM_CALL_METHOD(&logic_classname, &reflection_class, "getname");
					PHALCON_MM_ADD_ENTRY(&logic_classname);
					if (Z_TYPE(logic_classname) == IS_STRING) {
						logic_ce = phalcon_fetch_class(&logic_classname, ZEND_FETCH_CLASS_AUTO);
						if (instanceof_function_ex(logic_ce, phalcon_user_logic_ce, 0)) {
							PHALCON_MM_CALL_CE_STATIC(&logic, logic_ce, "call", &action_name, &action_params);

							phalcon_array_update(&params, &key, &logic, 0);

							if (phalcon_method_exists_ex(&logic, SL("start")) == SUCCESS) {
								PHALCON_MM_CALL_METHOD(NULL, &logic, "start");
							}
						}
					}
				} else {
					PHALCON_MM_CALL_METHOD(&var_name, reflection_parameter, "getname");
					if (phalcon_array_isset_fetch(&var_value, &action_params, &var_name, 0)) {
						phalcon_array_update(&params, &var_name, &var_value, PH_COPY);
						phalcon_array_unset(&tmp_params, &var_name, 0);
					} else if (count_action_params >= 0 && phalcon_array_isset_fetch(&var_value, &action_params, &key, 0)) {
						phalcon_array_update(&params, &key, &var_value, PH_COPY);
						phalcon_array_unset(&tmp_params, &key, 0);
					} else if (count_action_params) {
						phalcon_array_get_current(&var_value, &action_params);
						phalcon_array_update(&params, &key, &var_value, 0);

						phalcon_array_get_key(&var_name, &action_params);
						phalcon_array_unset(&tmp_params, &var_name, 0);
					}
					zval_ptr_dtor(&var_name);
				}
				if (count_action_params) {
					zend_hash_move_forward(Z_ARRVAL_P(&action_params));
					count_action_params -= 1;
				}
			} ZEND_HASH_FOREACH_END();

			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(tmp_params), param) {
				phalcon_array_append(&params, param, PH_COPY);
			} ZEND_HASH_FOREACH_END();
		} else {
			PHALCON_MM_ZVAL_COPY(&params, &action_params);
		}

		/**
		 * Create a call handler
		 */
		array_init_size(&call_object, 2);
		phalcon_array_append(&call_object, &handler, PH_COPY);
		phalcon_array_append(&call_object, &action_method, PH_COPY);
		PHALCON_MM_ADD_ENTRY(&call_object);

		/* Call the method allowing exceptions */
		phalcon_call_user_func_array_noex(&value, &call_object, &params);
		PHALCON_MM_ADD_ENTRY(&value);

		/* Check if an exception has ocurred */
		if (EG(exception)) {
			zval e = {};
			ZVAL_OBJ(&e, EG(exception));
			ZVAL_OBJ(&exception, zend_objects_clone_obj(&e));
			PHALCON_MM_ADD_ENTRY(&exception);
			phalcon_update_property(getThis(), SL("_lastException"), &exception);

			/* Clear the exception  */
			zend_clear_exception();

			/* Try to handle the exception */
			PHALCON_MM_CALL_METHOD(&status, getThis(), "_handleexception", &exception);

			if (likely(!instanceof_function_ex(Z_OBJCE(exception), phalcon_continueexception_ce, 0))) {
				if (PHALCON_IS_FALSE(&status)) {
					phalcon_read_property(&finished, getThis(), SL("_finished"), PH_READONLY);
					if (PHALCON_IS_FALSE(&finished)) {
						continue;
					}
				} else {
					/* Exception was not handled, rethrow it */
					phalcon_throw_exception(&exception);
					goto end;
				}
			}
		} else {
			/* Update the latest value produced by the latest handler */
			phalcon_update_property(getThis(), SL("_returnedValue"), &value);
		}

		phalcon_update_property(getThis(), SL("_lastHandler"), &handler);

		if (zend_is_true(&logic_binding)) {
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(params), param) {
				if (Z_TYPE_P(param) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(param), phalcon_user_logic_ce, 0)) {
					if (phalcon_method_exists_ex(param, SL("finish")) == SUCCESS) {
						PHALCON_MM_CALL_METHOD(NULL, param, "finish");
					}
				}
			} ZEND_HASH_FOREACH_END();
		}

afterexecuteroute:

		if (Z_TYPE(events_manager) == IS_OBJECT) {
			/**
			 * Call afterExecuteRoute
			 */
			PHALCON_MM_ZVAL_STRING(&event_name, "dispatch:afterExecuteRoute");
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);

			phalcon_read_property(&finished, getThis(), SL("_finished"), PH_READONLY);
			if (PHALCON_IS_FALSE(&finished)) {
				continue;
			}

			/**
			 * Call afterDispatch
			 */
			PHALCON_MM_ZVAL_STRING(&event_name, "dispatch:afterDispatch");
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
		}

		/**
		 * Calling afterExecuteRoute as callback and event
		 */
		if (phalcon_method_exists_ex(&handler, SL("afterexecuteroute")) == SUCCESS) {
			PHALCON_MM_CALL_METHOD(&status, &handler, "afterexecuteroute", getThis(), &value);
			if (PHALCON_IS_FALSE(&status)) {
				continue;
			}
			zval_ptr_dtor(&status);

			phalcon_read_property(&finished, getThis(), SL("_finished"), PH_READONLY);
			if (PHALCON_IS_FALSE(&finished)) {
				continue;
			}
		}
	} while (number_dispatches <= max_dispatches);

	RETVAL_ZVAL(&handler, 1, 0);

end:

	/**
	 * Call afterDispatchLoop
	 */
	PHALCON_MM_ZVAL_STRING(&event_name, "dispatch:afterDispatchLoop");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
	RETURN_MM();
}

/**
 * Forwards the execution flow to another controller/action
 * Dispatchers are unique per module. Forwarding between modules is not allowed
 *
 *<code>
 *  $this->dispatcher->forward(array('controller' => 'posts', 'action' => 'index'));
 *</code>
 *
 * @param string|array $forward
 * @return bool
 */
PHP_METHOD(Phalcon_Dispatcher, forward){

	zval *forward, event_name = {}, forward_parts = {}, parts = {}, number_parts = {}, controller_part = {}, real_namespace_name = {}, real_controller_name = {}, action_part = {}, exception_code = {}, exception_message = {};
	zval namespace_name = {}, controller_name = {}, task_name = {}, action_name = {}, params = {}, previous_namespace_name = {}, previous_controller_name = {}, previous_action_name = {}, previous_params = {};
	int num = 0;

	phalcon_fetch_params(1, 1, 0, &forward);

	PHALCON_MM_ZVAL_STRING(&event_name, "dispatch:beforeForward");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name, forward);

	if (Z_TYPE_P(forward) == IS_STRING) {
		array_init(&forward_parts);
		PHALCON_MM_ADD_ENTRY(&forward_parts);

		phalcon_fast_explode_str(&parts, SL("::"), forward);
		phalcon_fast_count(&number_parts, &parts);

		num = phalcon_get_intval(&number_parts);

		if (num > 0 && num <= 3) {
			switch (num) {

				case 3:
					phalcon_array_fetch_long(&controller_part, &parts, 1, PH_NOISY|PH_READONLY);
					phalcon_array_fetch_long(&action_part, &parts, 2, PH_NOISY|PH_READONLY);
					break;

				case 2:
					phalcon_array_fetch_long(&controller_part, &parts, 0, PH_NOISY|PH_READONLY);
					phalcon_array_fetch_long(&action_part, &parts, 1, PH_NOISY|PH_READONLY);
					break;

				case 1:
					phalcon_array_fetch_long(&controller_part, &parts, 0, PH_NOISY|PH_READONLY);
					break;

			}

			if (phalcon_memnstr_str(&controller_part, SL("\\"))) {
				phalcon_get_class_ns(&real_controller_name, &controller_part, 0);

				phalcon_array_update_str(&forward_parts, SL("controller"), &real_controller_name, 0);
				phalcon_get_ns_class(&real_namespace_name, &controller_name, 0);

				if (zend_is_true(&namespace_name)) {
					phalcon_array_update_str(&forward_parts, SL("namespace"), &real_namespace_name, 0);
				}
			} else {
				ZVAL_COPY(&real_controller_name, &controller_part);
			}

			phalcon_uncamelize(&controller_part, &real_controller_name);
			zval_ptr_dtor(&real_controller_name);
			phalcon_array_update_str(&forward_parts, SL("controller"), &controller_part, 0);

			if (Z_TYPE(action_part) != IS_NULL) {
				phalcon_array_update_str(&forward_parts, SL("action"), &action_part, PH_COPY);
			}
		}
		zval_ptr_dtor(&parts);
	} else {
		PHALCON_MM_ZVAL_COPY(&forward_parts, forward);
	}

	if (Z_TYPE(forward_parts) != IS_ARRAY) {
		ZVAL_LONG(&exception_code, PHALCON_EXCEPTION_INVALID_PARAMS);
		PHALCON_MM_ZVAL_STRING(&exception_message, "Forward parameter must be an Array");
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "_throwdispatchexception", &exception_message, &exception_code);
		RETURN_MM();
	}

	phalcon_read_property(&previous_namespace_name, getThis(), SL("_namespaceName"), PH_READONLY);
	phalcon_update_property(getThis(), SL("_previousNamespaceName"), &previous_namespace_name);

	phalcon_read_property(&previous_controller_name, getThis(), SL("_handlerName"), PH_READONLY);
	phalcon_update_property(getThis(), SL("_previousHandlerName"), &previous_controller_name);

	phalcon_read_property(&previous_action_name, getThis(), SL("_actionName"), PH_READONLY);
	phalcon_update_property(getThis(), SL("_previousActionName"), &previous_action_name);

	phalcon_read_property(&previous_params, getThis(), SL("_params"), PH_READONLY);
	phalcon_update_property(getThis(), SL("_previousParams"), &previous_params);

	/**
	 * Check if we need to forward to another namespace
	 */
	if (phalcon_array_isset_fetch_str(&namespace_name, &forward_parts, SL("namespace"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_namespaceName"), &namespace_name);
	}

	/**
	 * Check if we need to forward to another controller
	 */
	if (phalcon_array_isset_fetch_str(&controller_name, &forward_parts, SL("controller"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_handlerName"), &controller_name);
	} else if (phalcon_array_isset_fetch_str(&task_name, &forward_parts, SL("task"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_handlerName"), &task_name);
	}

	/**
	 * Check if we need to forward to another action
	 */
	if (phalcon_array_isset_fetch_str(&action_name, &forward_parts, SL("action"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_actionName"), &action_name);
	}

	/**
	 * Check if we need to forward changing the current parameters
	 */
	if (phalcon_array_isset_fetch_str(&params, &forward_parts, SL("params"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_params"), &params);
	}

	phalcon_update_property(getThis(), SL("_finished"), &PHALCON_GLOBAL(z_false));
	phalcon_update_property(getThis(), SL("_forwarded"), &PHALCON_GLOBAL(z_true));

	PHALCON_MM_ZVAL_STRING(&event_name, "dispatch:afterForward");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name, forward);
	RETURN_MM();
}

/**
 * Check if the current executed action was forwarded by another one
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Dispatcher, wasForwarded){


	RETURN_MEMBER(getThis(), "_forwarded");
}

/**
 * Possible class name that will be located to dispatch the request
 *
 * @return string
 */
PHP_METHOD(Phalcon_Dispatcher, getHandlerClass){

	zval handler_suffix = {}, namespace_name = {}, handler_name = {}, camelized_class = {}, camelize = {};

	/**
	 * The handler suffix
	 */
	phalcon_read_property(&handler_suffix, getThis(), SL("_handlerSuffix"), PH_NOISY|PH_READONLY);

	/**
	 * If the current namespace is null we used the set in this_ptr::_defaultNamespace
	 */
	phalcon_read_property(&namespace_name, getThis(), SL("_namespaceName"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&namespace_name)) {
		phalcon_read_property(&namespace_name, getThis(), SL("_defaultNamespace"), PH_NOISY|PH_READONLY);
		phalcon_update_property(getThis(), SL("_namespaceName"), &namespace_name);
	}

	/**
	 * If the handler is null we use the set in this_ptr::_defaultHandler
	 */
	phalcon_read_property(&handler_name, getThis(), SL("_handlerName"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&handler_name)) {
		phalcon_read_property(&handler_name, getThis(), SL("_defaultHandler"), PH_NOISY|PH_READONLY);
		phalcon_update_property(getThis(), SL("_handlerName"), &handler_name);
	}

	/**
	 * We don't camelize the classes if they are in namespaces
	 */
	if (!phalcon_memnstr_str(&handler_name, SL("\\"))) {
		phalcon_camelize(&camelized_class, &handler_name);
	} else if (phalcon_start_with_str(&handler_name, SL("\\"))) {
		ZVAL_STRINGL(&camelized_class, Z_STRVAL(handler_name)+1, Z_STRLEN(handler_name)-1);
	} else {
		ZVAL_COPY(&camelized_class, &handler_name);
	}

	/**
	 * Create the complete controller class name prepending the namespace
	 */
	if (zend_is_true(&namespace_name)) {
		zval camelized_namespace = {};
		phalcon_read_property(&camelize, getThis(), SL("_camelizeNamespace"), PH_NOISY|PH_READONLY);
		if (!zend_is_true(&camelize)) {
			ZVAL_COPY(&camelized_namespace, &namespace_name);
		} else {
			phalcon_camelize(&camelized_namespace, &namespace_name);
		}
		if (phalcon_end_with_str(&camelized_namespace, SL("\\"))) {
			PHALCON_CONCAT_VVV(return_value, &camelized_namespace, &camelized_class, &handler_suffix);
		} else {
			PHALCON_CONCAT_VSVV(return_value, &camelized_namespace, "\\", &camelized_class, &handler_suffix);
		}
		zval_ptr_dtor(&camelized_namespace);
	} else {
		PHALCON_CONCAT_VV(return_value, &camelized_class, &handler_suffix);
	}
	zval_ptr_dtor(&camelized_class);

	phalcon_update_property(getThis(), SL("_isExactHandler"), &PHALCON_GLOBAL(z_false));
}

/**
 * Enables/Disables automatically camelize namespace
 *
 *<code>
 *  $this->dispatcher->camelizeNamespace(FALSE);
 *</code>
 *
 * @param bool $camelize
 */
PHP_METHOD(Phalcon_Dispatcher, camelizeNamespace){

	zval *camelize;

	phalcon_fetch_params(0, 1, 0, &camelize);

	if (PHALCON_IS_TRUE(camelize)) {
		phalcon_update_property(getThis(), SL("_camelizeNamespace"), &PHALCON_GLOBAL(z_true));
	} else {
		phalcon_update_property(getThis(), SL("_camelizeNamespace"), &PHALCON_GLOBAL(z_false));
	}
}

/**
 * Enables/Disables automatically camelize controller
 *
 *<code>
 *  $this->dispatcher->camelizeController(FALSE);
 *</code>
 *
 * @param bool $camelize
 */
PHP_METHOD(Phalcon_Dispatcher, camelizeController){

	zval *camelize;

	phalcon_fetch_params(0, 1, 0, &camelize);

	if (PHALCON_IS_TRUE(camelize)) {
		phalcon_update_property(getThis(), SL("_camelizeController"), &PHALCON_GLOBAL(z_true));
	} else {
		phalcon_update_property(getThis(), SL("_camelizeController"), &PHALCON_GLOBAL(z_false));
	}
}

/**
 * Set error handler
 *
 * @param mixed $error_handler
 * @param int $exception_code
 * @return Phalcon\DispatcherInterface
 */
PHP_METHOD(Phalcon_Dispatcher, setErrorHandler){

	zval *error_handler, *exception_code = NULL;

	phalcon_fetch_params(0, 1, 1, &error_handler, &exception_code);

	if (!exception_code) {
		exception_code = &PHALCON_GLOBAL(z_zero);
	}

	phalcon_update_property_array(getThis(), SL("_errorHandlers"), exception_code, error_handler);

	RETURN_THIS();
}

/**
 * Get error handler
 *
 * @param int $exception_code
 * @return mixed
 */
PHP_METHOD(Phalcon_Dispatcher, getErrorHandler){

	zval *exception_code = NULL, error_handlers = {}, error_handler = {};

	phalcon_fetch_params(0, 0, 1, &exception_code);

	if (!exception_code) {
		exception_code = &PHALCON_GLOBAL(z_zero);
	}

	phalcon_read_property(&error_handlers, getThis(), SL("_errorHandlers"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(error_handlers) == IS_ARRAY) {
		if (phalcon_array_isset_fetch(&error_handler, &error_handlers, exception_code, PH_READONLY)) {
			RETURN_CTOR(&error_handler);
		}
	}

	RETURN_NULL();
}

/**
 * Returns the last exception
 *
 * @return \Exception
 */
PHP_METHOD(Phalcon_Dispatcher, getLastException){


	RETURN_MEMBER(getThis(), "_lastException");
}

/**
 * Returns the last handler
 *
 * @return Object
 */
PHP_METHOD(Phalcon_Dispatcher, getLastHandler){


	RETURN_MEMBER(getThis(), "_lastHandler");
}

/**
 * Returns the previons namespace
 *
 * @return string
 */
PHP_METHOD(Phalcon_Dispatcher, getPreviousNamespaceName){


	RETURN_MEMBER(getThis(), "_previousNamespaceName");
}

/**
 * Returns the previons action
 *
 * @return string
 */
PHP_METHOD(Phalcon_Dispatcher, getPreviousActionName){


	RETURN_MEMBER(getThis(), "_previousActionName");
}

/**
 * Returns the previons action params
 *
 * @return array
 */
PHP_METHOD(Phalcon_Dispatcher, getPreviousParams){


	RETURN_MEMBER(getThis(), "_previousParams");
}

/**
 * Gets a previons param by its name or numeric index
 *
 * @param mixed $param
 * @param string|array $filters
 * @param mixed $defaultValue
 * @return mixed
 */
PHP_METHOD(Phalcon_Dispatcher, getPreviousParam){

	zval *param, *filters = NULL, *default_value = NULL, params = {}, param_value = {}, exception_code = {}, exception_message = {};
	zval service = {}, filter = {};

	phalcon_fetch_params(1, 1, 2, &param, &filters, &default_value);

	phalcon_read_property(&params, getThis(), SL("_previousParams"), PH_NOISY|PH_READONLY);
	if (phalcon_array_isset_fetch(&param_value, &params, param, PH_READONLY)) {
		if (filters && Z_TYPE_P(filters) != IS_NULL) {
			zval dependency_injector = {};
			PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi");
			PHALCON_MM_ADD_ENTRY(&dependency_injector);
			if (Z_TYPE(dependency_injector) != IS_OBJECT) {
				ZVAL_LONG(&exception_code, PHALCON_EXCEPTION_NO_DI);
				PHALCON_MM_ZVAL_STRING(&exception_message, "A dependency injection object is required to access the 'filter' service");
				PHALCON_MM_CALL_METHOD(NULL, getThis(), "_throwdispatchexception", &exception_message, &exception_code);
				RETURN_MM();
			}

			ZVAL_STR(&service, IS(filter));

			PHALCON_MM_CALL_METHOD(&filter, &dependency_injector, "getshared", &service);
			PHALCON_MM_ADD_ENTRY(&filter);
			PHALCON_MM_VERIFY_INTERFACE(&filter, phalcon_filterinterface_ce);
			PHALCON_MM_CALL_METHOD(return_value, &filter, "sanitize", &param_value, filters);
			RETURN_MM();
		} else {
			RETURN_MM_CTOR(&param_value);
		}
	}

	if (default_value) {
		RETURN_MM_CTOR(default_value);
	}

	RETURN_MM_NULL();
}
