
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

#ifndef PHALCON_ROUTERINTERFACE_H
#define PHALCON_ROUTERINTERFACE_H

#include "php_phalcon.h"

extern zend_class_entry *phalcon_routerinterface_ce;

PHALCON_INIT_CLASS(Phalcon_RouterInterface);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_routerinterface_setdefaultmodule, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, moduleName, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_routerinterface_setdefaultnamespace, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, namespaceName, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_routerinterface_setdefaulthandler, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, handlerName, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_routerinterface_setdefaultaction, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, actionName, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_routerinterface_setdefaultparams, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, params, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_routerinterface_setcasesensitive, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, caseSensitive, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_routerinterface_setmode, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_routerinterface_setmodulename, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, moduleName, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_routerinterface_setnamespacename, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, namespaceName, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_routerinterface_sethandlername, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, handlerName, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_routerinterface_setactionname, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, actionName, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_routerinterface_setparams, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, params, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_routerinterface_handle, 0, 0, 0)
	ZEND_ARG_INFO(0, uri)
ZEND_END_ARG_INFO()

#endif /* PHALCON_ROUTERINTERFACE_H */
