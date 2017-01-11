
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

#include "websocket/eventloopinterface.h"

#include "kernel/main.h"

/**
 * Phalcon\Websocket\EventloopInterface
 *
 */
zend_class_entry *phalcon_websocket_eventloopinterface_ce;

PHP_METHOD(Phalcon_Websocket_EventLoopInterface, add);
PHP_METHOD(Phalcon_Websocket_EventLoopInterface, delete);
PHP_METHOD(Phalcon_Websocket_EventLoopInterface, setMode);
PHP_METHOD(Phalcon_Websocket_EventLoopInterface, lock);
PHP_METHOD(Phalcon_Websocket_EventLoopInterface, unlock);

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

const zend_function_entry phalcon_websocket_eventloopinterface_method_entry[] = {
	PHP_ABSTRACT_ME(Phalcon_Websocket_EventLoopInterface, add, arginfo_phalcon_websocket_eventloopinterface_add)
	PHP_ABSTRACT_ME(Phalcon_Websocket_EventLoopInterface, delete, arginfo_phalcon_websocket_eventloopinterface_delete)
	PHP_ABSTRACT_ME(Phalcon_Websocket_EventLoopInterface, setMode, arginfo_phalcon_websocket_eventloopinterface_setMode)
	PHP_ABSTRACT_ME(Phalcon_Websocket_EventLoopInterface, lock, arginfo_phalcon_websocket_eventloopinterface_lock)
	PHP_ABSTRACT_ME(Phalcon_Websocket_EventLoopInterface, unlock, arginfo_phalcon_websocket_eventloopinterface_unlock)
	PHP_FE_END
};

zend_object_handlers ws_eventloop_object_handlers;


/**
 * Phalcon\Websocket\Server initializer
 */
PHALCON_INIT_CLASS(Phalcon_Websocket_EventLoopInterface){

	PHALCON_REGISTER_INTERFACE(Phalcon\\Websocket, EventLoopInterface, websocket_eventloopinterface, phalcon_websocket_eventloopinterface_method_entry);

	zend_declare_class_constant_long(phalcon_websocket_eventloopinterface_ce, SL("EVENT_READ"), 1<<0);
	zend_declare_class_constant_long(phalcon_websocket_eventloopinterface_ce, SL("EVENT_WRITE"), 1<<2);
	return SUCCESS;
}
