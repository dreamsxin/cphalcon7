
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

#include "routerinterface.h"
#include "kernel/main.h"

zend_class_entry *phalcon_routerinterface_ce;

static const zend_function_entry phalcon_routerinterface_method_entry[] = {
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, setDefaultModule, arginfo_phalcon_routerinterface_setdefaultmodule)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, getDefaultModule, NULL)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, setDefaultNamespace, arginfo_phalcon_routerinterface_setdefaultnamespace)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, getDefaultNamespace, NULL)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, setDefaultHandler, arginfo_phalcon_routerinterface_setdefaulthandler)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, getDefaultHandler, NULL)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, setDefaultAction, arginfo_phalcon_routerinterface_setdefaultaction)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, getDefaultAction, NULL)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, setDefaultParams, arginfo_phalcon_routerinterface_setdefaultparams)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, getDefaultParams, NULL)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, setCaseSensitive, arginfo_phalcon_routerinterface_setcasesensitive)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, getCaseSensitive, NULL)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, setMode, arginfo_phalcon_routerinterface_setmode)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, getMode, NULL)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, setModuleName, arginfo_phalcon_routerinterface_setmodulename)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, getModuleName, NULL)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, setNamespaceName, arginfo_phalcon_routerinterface_setnamespacename)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, getNamespaceName, NULL)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, setHandlerName, arginfo_phalcon_routerinterface_sethandlername)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, getHandlerName, NULL)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, setActionName, arginfo_phalcon_routerinterface_setactionname)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, getActionName, NULL)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, setParams, arginfo_phalcon_routerinterface_setparams)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, getParams, NULL)
	PHP_ABSTRACT_ME(Phalcon_RouterInterface, handle, arginfo_phalcon_routerinterface_handle)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\RouterInterface initializer
 */
PHALCON_INIT_CLASS(Phalcon_RouterInterface){

	PHALCON_REGISTER_INTERFACE(Phalcon, RouterInterface, routerinterface, phalcon_routerinterface_method_entry);

	return SUCCESS;
}

/**
 * Sets the name of the default module
 *
 * @param string $moduleName
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, setDefaultModule);

/**
 * Gets the name of the default module
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, getDefaultModule);

/**
 * Sets the default handle name
 *
 * @param string $handlerName
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, setDefaultHandler);

/**
 * Gets the default handle name
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, getDefaultHandler);

/**
 * Sets the default action name
 *
 * @param string $actionName
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, setDefaultAction);

/**
 * Gets the default action name
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, getDefaultAction);


/**
 * Sets the default extra params
 *
 * @param array $actionName
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, setDefaultParams);

/**
 * Gets the default extra params
 *
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, getDefaultParams);

/**
 * Sets the case sensitive
 *
 * @param boolean $caseSensitive
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, setCaseSensitive);

/**
 * Gets the case sensitive
 *
 * @return int
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, getCaseSensitive);

/**
 * Sets the mode
 *
 * @param int $mode
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, setMode);

/**
 * Gets the mode
 *
 * @return int
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, getMode);

/**
 * Sets processed module name
 *
 * @param string $moduleName
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, setModuleName);

/**
 * Returns processed module name
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, getModuleName);

/**
 * Sets processed namespace name
 *
 * @param string $namespaceName
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, setNamespaceName);

/**
 * Returns processed namespace name
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, getNamespaceName);

/**
 * Sets processed handle name
 *
 * @param string $handleName
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, setHandlerName);

/**
 * Returns processed handle name
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, getHandlerName);

/**
 * Sets processed action name
 *
 * @param string $actionName
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, setActionName);

/**
 * Returns processed action name
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, getActionName);

/**
 * Sets processed extra params
 *
 * @param array $params
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, setParams);

/**
 * Returns processed extra params
 *
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, getParams);

/**
 * Handles routing information received from the rewrite engine
 *
 * @param string $uri
 */
PHALCON_DOC_METHOD(Phalcon_RouterInterface, handle);
