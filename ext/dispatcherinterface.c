
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
  +------------------------------------------------------------------------+
*/

#include "dispatcherinterface.h"
#include "kernel/main.h"

zend_class_entry *phalcon_dispatcherinterface_ce;

static const zend_function_entry phalcon_dispatcherinterface_method_entry[] = {
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, setActionSuffix, arginfo_phalcon_dispatcherinterface_setactionsuffix)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, setDefaultModule, arginfo_phalcon_dispatcherinterface_setdefaultmodule)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, getDefaultModule, NULL)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, setDefaultNamespace, arginfo_phalcon_dispatcherinterface_setdefaultnamespace)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, getDefaultNamespace, NULL)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, setDefaultHandler, arginfo_phalcon_dispatcherinterface_setdefaulthandler)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, getDefaultHandler, NULL)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, setDefaultAction, arginfo_phalcon_dispatcherinterface_setdefaultaction)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, getDefaultAction, NULL)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, setModuleName, arginfo_phalcon_dispatcherinterface_setmodulename)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, getModuleName, NULL)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, setNamespaceName, arginfo_phalcon_dispatcherinterface_setnamespacename)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, getNamespaceName, NULL)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, setHandlerName, arginfo_phalcon_dispatcherinterface_sethandlername)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, getHandlerName, NULL)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, setActionName, arginfo_phalcon_dispatcherinterface_setactionname)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, getActionName, NULL)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, setParams, arginfo_phalcon_dispatcherinterface_setparams)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, getParams, NULL)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, hasParam, arginfo_phalcon_dispatcherinterface_hasparam)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, setParam, arginfo_phalcon_dispatcherinterface_setparam)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, getParam, arginfo_phalcon_dispatcherinterface_getparam)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, isFinished, NULL)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, getReturnedValue, NULL)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, dispatch, NULL)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, forward, arginfo_phalcon_dispatcherinterface_forward)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, camelizeNamespace, arginfo_phalcon_dispatcherinterface_camelizenamespace)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, setErrorHandler, arginfo_phalcon_dispatcherinterface_seterrorhandler)
	PHP_ABSTRACT_ME(Phalcon_DispatcherInterface, getErrorHandler, arginfo_phalcon_dispatcherinterface_geterrorhandler)
	PHP_FE_END
};

/**
 * Phalcon\DispatcherInterface initializer
 */
PHALCON_INIT_CLASS(Phalcon_DispatcherInterface){

	PHALCON_REGISTER_INTERFACE(Phalcon, DispatcherInterface, dispatcherinterface, phalcon_dispatcherinterface_method_entry);

	return SUCCESS;
}

/**
 * Sets the default action suffix
 *
 * @param string $actionSuffix
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, setActionSuffix);

/**
 * Sets the default module
 *
 * @param string $module
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, setDefaultModule);

/**
 * Gets the default module
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, getDefaultModule);

/**
 * Sets the default namespace
 *
 * @param string $namespace
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, setDefaultNamespace);

/**
 * Gets the default namespace
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, getDefaultNamespace);

/**
 * Sets the default handler name
 *
 * @param string $handlerName
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, setDefaultHandler);

/**
 * Gets the default handler name
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, getDefaultHandler);

/**
 * Sets the default action name
 *
 * @param string $actionName
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, setDefaultAction);

/**
 * Gets the default action name
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, getDefaultAction);

/**
 * Sets the module name to be dispatched
 *
 * @param string $moduleName
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, setModuleName);

/**
 * Gets last dispatched module name
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, getModuleName);

/**
 * Sets the namespace to be dispatched
 *
 * @param string $namespaceName
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, setNamespaceName);

/**
 * Gets last dispatched namespace
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, getNamespaceName);

/**
 * Sets the handler name to be dispatched
 *
 * @param string $handlerName
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, setHandlerName);

/**
 * Gets last dispatched handler name
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, getHandlerName);

/**
 * Sets the action name to be dispatched
 *
 * @param string $actionName
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, setActionName);

/**
 * Gets last dispatched action name
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, getActionName);

/**
 * Sets action params to be dispatched
 *
 * @param array $params
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, setParams);

/**
 * Gets action params
 *
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, getParams);

/**
 * Check if a param exists
 *
 * @param mixed param
 * @return boolean
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, setParam);

/**
 * Set a param by its name or numeric index
 *
 * @param  mixed $param
 * @param  mixed $value
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, setParam);

/**
 * Gets a param by its name or numeric index
 *
 * @param  mixed $param
 * @param  string|array $filters
 * @return mixed
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, getParam);

/**
 * Checks if the dispatch loop is finished or has more pendent controllers/tasks to disptach
 *
 * @return boolean
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, isFinished);

/**
 * Returns value returned by the lastest dispatched action
 *
 * @return mixed
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, getReturnedValue);

/**
 * Dispatches a handle action taking into account the routing parameters
 *
 * @return object
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, dispatch);

/**
 * Forwards the execution flow to another controller/action
 *
 * @param array $forward
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, forward);

/**
 * Forwards the execution flow to another controller/action
 *
 * @param array $forward
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, camelizeNamespace);

/**
 * Set error handler
 *
 * @param mixed $handler
 * @param int $exception_code
 * @return Phalcon\DispatcherInterface
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, setErrorHandler);

/**
 * Get error handler
 *
 * @param int $exception_code
 * @return mixed
 */
PHALCON_DOC_METHOD(Phalcon_DispatcherInterface, getErrorHandler);
