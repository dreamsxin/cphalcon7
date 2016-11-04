
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

#include "events/eventinterface.h"
#include "kernel/main.h"

zend_class_entry *phalcon_events_eventinterface_ce;

static const zend_function_entry phalcon_events_eventinterface_method_entry[] = {
	PHP_ABSTRACT_ME(Phalcon_Events_EventInterface, setType, arginfo_phalcon_events_eventinterface_settype)
	PHP_ABSTRACT_ME(Phalcon_Events_EventInterface, getType, NULL)
	PHP_ABSTRACT_ME(Phalcon_Events_EventInterface, setSource, arginfo_phalcon_events_eventinterface_setsource)
	PHP_ABSTRACT_ME(Phalcon_Events_EventInterface, getSource, NULL)
	PHP_ABSTRACT_ME(Phalcon_Events_EventInterface, setData, arginfo_phalcon_events_eventinterface_setdata)
	PHP_ABSTRACT_ME(Phalcon_Events_EventInterface, getData, NULL)
	PHP_ABSTRACT_ME(Phalcon_Events_EventInterface, setCancelable, arginfo_phalcon_events_eventinterface_setcancelable)
	PHP_ABSTRACT_ME(Phalcon_Events_EventInterface, isCancelable, NULL)
	PHP_ABSTRACT_ME(Phalcon_Events_EventInterface, stop, NULL)
	PHP_ABSTRACT_ME(Phalcon_Events_EventInterface, isStopped, NULL)
	PHP_FE_END
};


/**
 * Phalcon\Events\EventInterface initializer
 */
PHALCON_INIT_CLASS(Phalcon_Events_EventInterface){

	PHALCON_REGISTER_INTERFACE(Phalcon\\Events, EventInterface, events_eventinterface, phalcon_events_eventinterface_method_entry);

	return SUCCESS;
}

/**
 * Sets event type
 */
PHALCON_DOC_METHOD(Phalcon_Events_EventInterface, setType);

/**
 * Gets event type
 */
PHALCON_DOC_METHOD(Phalcon_Events_EventInterface, getType);

/**
 * Sets event source
 */
PHALCON_DOC_METHOD(Phalcon_Events_EventInterface, setSource);

/**
 * Gets event source
 */
PHALCON_DOC_METHOD(Phalcon_Events_EventInterface, getSource);

/**
 * Sets event data
 */
PHALCON_DOC_METHOD(Phalcon_Events_EventInterface, setData);

/**
 * Gets event data
 */
PHALCON_DOC_METHOD(Phalcon_Events_EventInterface, getData);

/**
 * Sets event is cancelable
 */
PHALCON_DOC_METHOD(Phalcon_Events_EventInterface, setCancelable);

/**
 * Check whether the event is cancelable
 */
PHALCON_DOC_METHOD(Phalcon_Events_EventInterface, isCancelable);

/**
 * Stops the event preventing propagation
 */
PHALCON_DOC_METHOD(Phalcon_Events_EventInterface, stop);

/**
 * Check whether the event is currently stopped
 */
PHALCON_DOC_METHOD(Phalcon_Events_EventInterface, isStopped);

