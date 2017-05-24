
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

#ifndef PHALCON_WEBSOCKET_EVENTLOOPINTERFACE_H
#define PHALCON_WEBSOCKET_EVENTLOOPINTERFACE_H

#include "php_phalcon.h"

extern zend_class_entry *phalcon_websocket_eventloopinterface_ce;

PHALCON_INIT_CLASS(Phalcon_Websocket_EventloopInterface);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_eventloopinterface_add, 0, 0, 2)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_eventloopinterface_delete, 0, 0, 1)
	ZEND_ARG_INFO(0, fd)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_eventloopinterface_setMode, 0, 0, 2)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_eventloopinterface_lock, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_eventloopinterface_unlock, 0, 0, 0)
ZEND_END_ARG_INFO()

#endif /* PHALCON_WEBSOCKET_EVENTLOOPINTERFACE_H */
