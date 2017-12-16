
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

#include "events/event.h"
#include "events/eventinterface.h"
#include "events/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/operators.h"
#include "kernel/exception.h"

/**
 * Phalcon\Events\Event
 *
 * This class offers contextual information of a fired event in the EventsManager
 */
zend_class_entry *phalcon_events_event_ce;

PHP_METHOD(Phalcon_Events_Event, __construct);
PHP_METHOD(Phalcon_Events_Event, setName);
PHP_METHOD(Phalcon_Events_Event, getName);
PHP_METHOD(Phalcon_Events_Event, setType);
PHP_METHOD(Phalcon_Events_Event, getType);
PHP_METHOD(Phalcon_Events_Event, setSource);
PHP_METHOD(Phalcon_Events_Event, getSource);
PHP_METHOD(Phalcon_Events_Event, setData);
PHP_METHOD(Phalcon_Events_Event, getData);
PHP_METHOD(Phalcon_Events_Event, setCancelable);
PHP_METHOD(Phalcon_Events_Event, isCancelable);
PHP_METHOD(Phalcon_Events_Event, getFlag);
PHP_METHOD(Phalcon_Events_Event, stop);
PHP_METHOD(Phalcon_Events_Event, isStopped);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_events_event___construct, 0, 0, 2)
	ZEND_ARG_INFO(0, type)
	ZEND_ARG_INFO(0, source)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, cancelable)
	ZEND_ARG_INFO(0, flag)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_events_event_method_entry[] = {
	PHP_ME(Phalcon_Events_Event, __construct, arginfo_phalcon_events_event___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Events_Event, setName, arginfo_phalcon_events_eventinterface_setname, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Event, getName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Event, setType, arginfo_phalcon_events_eventinterface_settype, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Event, getType, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Event, setSource, arginfo_phalcon_events_eventinterface_setsource, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Event, getSource, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Event, setData, arginfo_phalcon_events_eventinterface_setdata, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Event, getData, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Event, setCancelable, arginfo_phalcon_events_eventinterface_setcancelable, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Event, isCancelable, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Event, getFlag, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Event, stop, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Event, isStopped, NULL, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Events_Event, getCancelable, isCancelable, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Events\Event initializer
 */
PHALCON_INIT_CLASS(Phalcon_Events_Event){

	PHALCON_REGISTER_CLASS(Phalcon\\Events, Event, events_event, phalcon_events_event_method_entry, 0);

	zend_declare_property_null(phalcon_events_event_ce, SL("_name"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_events_event_ce, SL("_type"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_events_event_ce, SL("_source"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_events_event_ce, SL("_data"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_events_event_ce, SL("_stopped"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_events_event_ce, SL("_cancelable"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_events_event_ce, SL("_flag"), 0, ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_events_event_ce, 1, phalcon_events_eventinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Events\Event constructor
 *
 * @param string $type
 * @param object $source
 * @param mixed $data
 * @param boolean $cancelable
 */
PHP_METHOD(Phalcon_Events_Event, __construct){

	zval *type, *source, *data = NULL, *cancelable = NULL, *flag = NULL;

	phalcon_fetch_params(0, 2, 3, &type, &source, &data, &cancelable, &flag);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!cancelable) {
		cancelable = &PHALCON_GLOBAL(z_true);
	}

	if (!flag) {
		flag = &PHALCON_GLOBAL(z_false);
	}

	phalcon_update_property(getThis(), SL("_type"), type);
	phalcon_update_property(getThis(), SL("_source"), source);
	if (Z_TYPE_P(data) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_data"), data);
	}

	if (PHALCON_IS_NOT_TRUE(cancelable)) {
		phalcon_update_property(getThis(), SL("_cancelable"), cancelable);
	}
	if (PHALCON_IS_TRUE(flag)) {
		phalcon_update_property(getThis(), SL("_flag"), flag);
	}
}

/**
 * Set the event's name
 *
 * @param string $eventName
 */
PHP_METHOD(Phalcon_Events_Event, setName){

	zval *event_name;

	phalcon_fetch_params(0, 1, 0, &event_name);

	phalcon_update_property(getThis(), SL("_name"), event_name);

}

/**
 * Returns the event's name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Events_Event, getName){


	RETURN_MEMBER(getThis(), "_name");
}

/**
 * Set the event's type
 *
 * @param string $eventType
 */
PHP_METHOD(Phalcon_Events_Event, setType){

	zval *event_type;

	phalcon_fetch_params(0, 1, 0, &event_type);

	phalcon_update_property(getThis(), SL("_type"), event_type);

}

/**
 * Returns the event's type
 *
 * @return string
 */
PHP_METHOD(Phalcon_Events_Event, getType){


	RETURN_MEMBER(getThis(), "_type");
}

/**
 * Sets the event's source
 *
 * @return object
 */
PHP_METHOD(Phalcon_Events_Event, setSource){

	zval *event_source;

	phalcon_fetch_params(0, 1, 0, &event_source);

	phalcon_update_property(getThis(), SL("_source"), event_source);
}

/**
 * Returns the event's source
 *
 * @return object
 */
PHP_METHOD(Phalcon_Events_Event, getSource){


	RETURN_MEMBER(getThis(), "_source");
}

/**
 * Set the event's data
 *
 * @param string $data
 */
PHP_METHOD(Phalcon_Events_Event, setData){

	zval *data;

	phalcon_fetch_params(0, 1, 0, &data);

	phalcon_update_property(getThis(), SL("_data"), data);

}

/**
 * Returns the event's data
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Events_Event, getData){


	RETURN_MEMBER(getThis(), "_data");
}

/**
 * Sets if the event is cancelable
 *
 * @param boolean $cancelable
 */
PHP_METHOD(Phalcon_Events_Event, setCancelable){

	zval *cancelable;

	phalcon_fetch_params(0, 1, 0, &cancelable);

	phalcon_update_property(getThis(), SL("_cancelable"), cancelable);
}

/**
 * Check whether the event is cancelable
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Events_Event, isCancelable){


	RETURN_MEMBER(getThis(), "_cancelable");
}

/**
 * Returns the event's flag
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Events_Event, getFlag){


	RETURN_MEMBER(getThis(), "_flag");
}

/**
 * Stops the event preventing propagation
 */
PHP_METHOD(Phalcon_Events_Event, stop){

	zval cancelable = {};

	phalcon_read_property(&cancelable, getThis(), SL("_cancelable"), PH_NOISY|PH_READONLY);
	if (zend_is_true(&cancelable)) {
		phalcon_update_property_bool(getThis(), SL("_stopped"), 1);
	} else {
		PHALCON_THROW_EXCEPTION_STR(phalcon_events_exception_ce, "Trying to cancel a non-cancelable event");
		return;
	}
}

/**
 * Check whether the event is currently stopped
 */
PHP_METHOD(Phalcon_Events_Event, isStopped){


	RETURN_MEMBER(getThis(), "_stopped");
}
