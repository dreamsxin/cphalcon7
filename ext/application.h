
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

#ifndef PHALCON_APPLICATION_H
#define PHALCON_APPLICATION_H

#include "php_phalcon.h"

extern zend_class_entry *phalcon_application_ce;

PHALCON_INIT_CLASS(Phalcon_Application);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_application_registermodules, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, modules, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, merge, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_application_setdefaultmodule, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, defaultModule, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_application_handle, 0, 0, 0)
	ZEND_ARG_INFO(0, uri)
ZEND_END_ARG_INFO()
/*
#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_phalcon_application_handle, 0, 0, "Phalcon\\Http\\ResponseInterface", 0)
	ZEND_ARG_INFO(0, uri)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_phalcon_application_handle, 0, 0, IS_OBJECT, "Phalcon\\Http\\ResponseInterface", 0)
	ZEND_ARG_INFO(0, uri)
ZEND_END_ARG_INFO()
#endif
*/

#endif /* PHALCON_APPLICATION_H */
