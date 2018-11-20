
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

#include "di/injectable.h"
#include "di/exception.h"
#include "di/injectionawareinterface.h"
#include "diinterface.h"
#include "di.h"
#include "events/eventsawareinterface.h"
#include "events/managerinterface.h"
#include "events/event.h"
#include "diinterface.h"
#include "debug.h"

#include <Zend/zend_closures.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/debug.h"

#include "internal/arginfo.h"
#include "interned-strings.h"

/**
 * Phalcon\Di\Injectable
 *
 * This class allows to access services in the services container by just only accessing a public property
 * with the same name of a registered service
 */
zend_class_entry *phalcon_di_injectable_ce;

PHP_METHOD(Phalcon_Di_Injectable, setDI);
PHP_METHOD(Phalcon_Di_Injectable, getDI);
PHP_METHOD(Phalcon_Di_Injectable, setEventsManager);
PHP_METHOD(Phalcon_Di_Injectable, getEventsManager);
PHP_METHOD(Phalcon_Di_Injectable, fireEvent);
PHP_METHOD(Phalcon_Di_Injectable, fireEventCancel);
PHP_METHOD(Phalcon_Di_Injectable, hasService);
PHP_METHOD(Phalcon_Di_Injectable, setService);
PHP_METHOD(Phalcon_Di_Injectable, getService);
PHP_METHOD(Phalcon_Di_Injectable, getResolveService);
PHP_METHOD(Phalcon_Di_Injectable, attachEvent);
PHP_METHOD(Phalcon_Di_Injectable, __get);
PHP_METHOD(Phalcon_Di_Injectable, __sleep);
PHP_METHOD(Phalcon_Di_Injectable, __debugInfo);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_di_injectable_attachevent, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, eventType, IS_STRING, 0)
	ZEND_ARG_OBJ_INFO(0, callback, Closure, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_di_injectable_method_entry[] = {
	PHP_ME(Phalcon_Di_Injectable, setDI, arginfo_phalcon_di_injectionawareinterface_setdi, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Di_Injectable, getDI, arginfo_phalcon_di_injectionawareinterface_getdi, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Di_Injectable, setEventsManager, arginfo_phalcon_events_eventsawareinterface_seteventsmanager, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Di_Injectable, getEventsManager, arginfo_phalcon_events_eventsawareinterface_geteventsmanager, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Di_Injectable, fireEvent, arginfo_phalcon_di_injectable_fireevent, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Di_Injectable, fireEventCancel, arginfo_phalcon_di_injectable_fireeventcancel, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Di_Injectable, hasService, arginfo_phalcon_di_injectable_hasservice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Di_Injectable, setService, arginfo_phalcon_di_injectable_setservice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Di_Injectable, getService, arginfo_phalcon_di_injectable_getservice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Di_Injectable, getResolveService, arginfo_phalcon_di_injectable_getresolveservice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Di_Injectable, attachEvent, arginfo_phalcon_di_injectable_attachevent, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Di_Injectable, __get, arginfo___get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Di_Injectable, __sleep, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Di_Injectable, __debugInfo, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


/**
 * Phalcon\Di\Injectable initializer
 */
PHALCON_INIT_CLASS(Phalcon_Di_Injectable){

	PHALCON_REGISTER_CLASS(Phalcon\\Di, Injectable, di_injectable, phalcon_di_injectable_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_di_injectable_ce, SL("_dependencyInjector"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_di_injectable_ce, SL("_eventsManager"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_di_injectable_ce, SL("_eventCallbacks"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_di_injectable_ce, 2, phalcon_di_injectionawareinterface_ce, phalcon_events_eventsawareinterface_ce);

	return SUCCESS;
}

/**
 * Sets the dependency injector
 *
 * @param Phalcon\DiInterface $dependencyInjector
 * @throw Phalcon\Di\Exception
 */
PHP_METHOD(Phalcon_Di_Injectable, setDI){

	zval *dependency_injector, di = {};

	phalcon_fetch_params(0, 1, 0, &dependency_injector);

	if (Z_TYPE_P(dependency_injector) != IS_OBJECT && PHALCON_IS_NOT_EMPTY(dependency_injector)) {
		PHALCON_CALL_METHOD(&di, getThis(), "getdi", dependency_injector);
		phalcon_update_property(getThis(), SL("_dependencyInjector"), &di);
	} else {
		PHALCON_VERIFY_INTERFACE_OR_NULL_EX(dependency_injector, phalcon_diinterface_ce, phalcon_di_exception_ce);
		phalcon_update_property(getThis(), SL("_dependencyInjector"), dependency_injector);
	}

	RETURN_THIS();
}

/**
 * Returns the internal dependency injector
 *
 * @return Phalcon\DiInterface
 */
PHP_METHOD(Phalcon_Di_Injectable, getDI)
{
	zval *error = NULL, *not_use_default = NULL;

	phalcon_fetch_params(0, 0, 2, &error, &not_use_default);

	if (!error) {
		error = &PHALCON_GLOBAL(z_false);
	}

	if (!not_use_default) {
		not_use_default = &PHALCON_GLOBAL(z_false);
	}

	phalcon_read_property(return_value, getThis(), SL("_dependencyInjector"), PH_COPY);
	if (Z_TYPE_P(return_value) != IS_OBJECT && !zend_is_true(not_use_default)) {
		PHALCON_CALL_CE_STATIC(return_value, phalcon_di_ce, "getdefault", return_value);
	}

	if (Z_TYPE_P(return_value) != IS_OBJECT && zend_is_true(error)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_di_exception_ce, "A dependency injection container is not object");
		return;
	}
}

/**
 * Sets the event manager
 *
 * @param Phalcon\Events\ManagerInterface $eventsManager
 */
PHP_METHOD(Phalcon_Di_Injectable, setEventsManager)
{
	zval *events_manager;

	phalcon_fetch_params(0, 1, 0, &events_manager);
	PHALCON_VERIFY_INTERFACE_OR_NULL_EX(events_manager, phalcon_events_managerinterface_ce, phalcon_di_exception_ce);

	phalcon_update_property(getThis(), SL("_eventsManager"), events_manager);

	RETURN_THIS();
}

/**
 * Returns the internal event manager
 *
 * @return Phalcon\Events\ManagerInterface
 */
PHP_METHOD(Phalcon_Di_Injectable, getEventsManager){

	zval service_name = {};

	phalcon_read_property(return_value, getThis(), SL("_eventsManager"), PH_COPY);
	if (Z_TYPE_P(return_value) != IS_OBJECT) {
		ZVAL_STR(&service_name, IS(eventsManager));
		PHALCON_CALL_METHOD(return_value, getThis(), "getservice", &service_name);
	}
}

/**
 * Fires an event, implicitly calls behaviors and listeners in the events manager are notified
 *
 * @param string $eventName
 * @param mixed $data
 * @return boolean
 */
PHP_METHOD(Phalcon_Di_Injectable, fireEvent){

	zval *eventname, *data = NULL, *cancelable = NULL, *flag = NULL, eventtype = {}, callback = {}, events_manager = {}, event = {};
	zval is_stopped = {}, lower = {}, event_parts = {}, name = {}, status = {}, debug_message = {};

	phalcon_fetch_params(1, 1, 2, &eventname, &data, &cancelable);

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_CONCAT_SV(&debug_message, "--Event: ", eventname);
		PHALCON_DEBUG_LOG(&debug_message);
		zval_ptr_dtor(&debug_message);
	}

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!cancelable) {
		cancelable = &PHALCON_GLOBAL(z_true);
	}

	if (!flag) {
		flag = &PHALCON_GLOBAL(z_false);
	}

	phalcon_fast_strtolower(&lower, eventname);
	PHALCON_MM_ADD_ENTRY(&lower);
	if (phalcon_memnstr_str(&lower, SL(":"))) {
		phalcon_fast_explode_str(&event_parts, SL(":"), &lower);
		phalcon_array_fetch_long(&name, &event_parts, 1, PH_COPY);
		PHALCON_MM_ADD_ENTRY(&name);
		zval_ptr_dtor(&event_parts);
		PHALCON_MM_ZVAL_COPY(&eventtype, eventname);
	} else {
		zval class_name = {};
		PHALCON_MM_ZVAL_COPY(&name, &lower);

		phalcon_get_called_class(&class_name);
		PHALCON_CONCAT_VSV(&eventtype, &class_name, ":", eventname);
		PHALCON_MM_ADD_ENTRY(&eventtype);
		zval_ptr_dtor(&class_name);
	}

	PHALCON_MM_CALL_METHOD(&events_manager, getThis(), "geteventsmanager");
	PHALCON_MM_ADD_ENTRY(&events_manager);

	if (Z_TYPE(events_manager) != IS_NULL) {
		PHALCON_MM_VERIFY_INTERFACE_EX(&events_manager, phalcon_events_managerinterface_ce, phalcon_di_exception_ce);

		/**
		 * Send a notification to the events manager
		 */
		PHALCON_MM_CALL_METHOD(&status, &events_manager, "fire", &eventtype, getThis(), data, cancelable);
		PHALCON_MM_ADD_ENTRY(&status);
		PHALCON_MM_CALL_METHOD(&event, &events_manager, "getcurrentevent");
		PHALCON_MM_ADD_ENTRY(&event);
		if (Z_TYPE(event) == IS_OBJECT) {
			PHALCON_MM_CALL_METHOD(&is_stopped, &event, "isstopped");
			if (zend_is_true(&is_stopped)) {
				RETURN_MM_NCTOR(&status);
			}
		}
	} else {
		ZVAL_NULL(&event);
	}

	if (phalcon_property_array_isset_fetch(&callback, getThis(), SL("_eventCallbacks"), &name, PH_READONLY)) {
		zval arguments = {};
		array_init_size(&arguments, 3);
		phalcon_array_append(&arguments, &event, PH_COPY);
		phalcon_array_append(&arguments, data, PH_COPY);
		phalcon_array_append(&arguments, &status, 0);

		PHALCON_MM_ADD_ENTRY(&arguments);
		PHALCON_MM_CALL_USER_FUNC_ARRAY(&status, &callback, &arguments);
		PHALCON_MM_ADD_ENTRY(&status);

		if (Z_TYPE(event) == IS_OBJECT) {
			PHALCON_MM_CALL_METHOD(&is_stopped, &event, "isstopped");
			if (zend_is_true(&is_stopped)) {
				RETURN_MM_NCTOR(&status);
			}
		}
	}

	/**
	 * Check if there is a method with the same name of the event
	 */
	if (phalcon_method_exists(getThis(), &name) == SUCCESS) {
		zval prev_data = {};
		PHALCON_MM_ZVAL_COPY(&prev_data, &status);
		PHALCON_MM_CALL_METHOD(&status, getThis(), Z_STRVAL(name), &event, data, &prev_data);
		PHALCON_MM_ADD_ENTRY(&status);
	}

	RETURN_MM_CTOR(&status);
}

/**
 * Fires an event, can stop the event by returning to the false
 *
 * @param string $eventName
 * @param mixed $data
 * @return mixed
 */
PHP_METHOD(Phalcon_Di_Injectable, fireEventCancel){

	zval *eventname, *data = NULL, *cancelable = NULL, eventtype = {}, callback = {}, events_manager = {}, lower = {}, name = {};
	zval event = {}, is_stopped = {}, status = {}, debug_message = {};

	phalcon_fetch_params(1, 1, 2, &eventname, &data, &cancelable);

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_CONCAT_SV(&debug_message, "--Event(Cancel): ", eventname);
		PHALCON_DEBUG_LOG(&debug_message);
		zval_ptr_dtor(&debug_message);
	}

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!cancelable) {
		cancelable = &PHALCON_GLOBAL(z_true);
	}

	phalcon_fast_strtolower(&lower, eventname);
	PHALCON_MM_ADD_ENTRY(&lower);
	if (phalcon_memnstr_str(&lower, SL(":"))) {
		zval event_parts = {};
		phalcon_fast_explode_str(&event_parts, SL(":"), &lower);
		phalcon_array_fetch_long(&name, &event_parts, 1, PH_COPY);
		PHALCON_MM_ADD_ENTRY(&name);
		zval_ptr_dtor(&event_parts);
		PHALCON_MM_ZVAL_COPY(&eventtype, eventname);
	} else {
		zval class_name = {};
		PHALCON_MM_ZVAL_COPY(&name, &lower);

		phalcon_get_called_class(&class_name);
		PHALCON_CONCAT_VSV(&eventtype, &class_name, ":", eventname);
		PHALCON_MM_ADD_ENTRY(&eventtype);
		zval_ptr_dtor(&class_name);
	}

	PHALCON_MM_CALL_METHOD(&events_manager, getThis(), "geteventsmanager");
	PHALCON_MM_ADD_ENTRY(&events_manager);
	if (Z_TYPE(events_manager) != IS_NULL) {
		PHALCON_MM_VERIFY_INTERFACE_EX(&events_manager, phalcon_events_managerinterface_ce, phalcon_di_exception_ce);

		PHALCON_MM_CALL_METHOD(&status, &events_manager, "fire", &eventtype, getThis(), data, cancelable, &PHALCON_GLOBAL(z_true));
		if (PHALCON_IS_FALSE(&status)){
			RETURN_MM_FALSE;
		}
		PHALCON_MM_ADD_ENTRY(&status);

		PHALCON_MM_CALL_METHOD(&event, &events_manager, "getcurrentevent");
		PHALCON_MM_ADD_ENTRY(&event);
		if (Z_TYPE(event) == IS_OBJECT) {
			PHALCON_MM_CALL_METHOD(&is_stopped, &event, "isstopped");
			if (zend_is_true(&is_stopped)) {
				RETURN_MM_CTOR(&status);
			}
		}
	} else {
		ZVAL_NULL(&event);
	}

	if (phalcon_property_array_isset_fetch(&callback, getThis(), SL("_eventCallbacks"), &name, PH_READONLY)) {
		zval arguments = {};
		array_init_size(&arguments, 2);
		phalcon_array_append(&arguments, &event, PH_COPY);
		phalcon_array_append(&arguments, data, PH_COPY);
		phalcon_array_append(&arguments, &status, PH_COPY);

		PHALCON_MM_ADD_ENTRY(&arguments);
		PHALCON_MM_CALL_USER_FUNC_ARRAY(&status, &callback, &arguments);

		if (PHALCON_IS_FALSE(&status)){
			RETURN_MM_FALSE;
		}
		PHALCON_MM_ADD_ENTRY(&status);
		if (Z_TYPE(event) == IS_OBJECT) {
			PHALCON_MM_CALL_METHOD(&is_stopped, &event, "isstopped");
			if (zend_is_true(&is_stopped)) {
				RETURN_MM_CTOR(&status);
			}
		}
	}

	/**
	 * Check if there is a method with the same name of the event
	 */
	if (phalcon_method_exists(getThis(), &name) == SUCCESS) {
		zval prev_data = {};
		PHALCON_MM_ZVAL_COPY(&prev_data, &status);
		PHALCON_MM_CALL_METHOD(&status, getThis(), Z_STRVAL(name), &event, data, &prev_data);

		if (PHALCON_IS_FALSE(&status)){
			RETURN_MM_FALSE;
		}
		PHALCON_MM_ADD_ENTRY(&status);
	}

	RETURN_MM_CTOR(&status);
}

/**
 * Check whether the DI contains a service by a name
 *
 * @param string $name
 * @return boolean
 */
PHP_METHOD(Phalcon_Di_Injectable, hasService){

	zval *service_name, dependency_injector = {};

	phalcon_fetch_params(1, 1, 0, &service_name);

	PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi");
	PHALCON_MM_ADD_ENTRY(&dependency_injector);
	if (Z_TYPE(dependency_injector) == IS_OBJECT) {
		PHALCON_MM_CALL_METHOD(return_value, &dependency_injector, "has", service_name);
	} else {
		RETVAL_FALSE;
	}
	RETURN_MM();
}

/**
 * Sets a service from the DI
 *
 * @param string $serviceName
 * @param mixed $definition
 * @param boolean $shared
 * @return Phalcon\Di\ServiceInterface
 */
PHP_METHOD(Phalcon_Di_Injectable, setService){

	zval *service_name, *definition, *shared = NULL, dependency_injector = {};

	phalcon_fetch_params(1, 2, 1, &service_name, &definition, &shared);

	if (!shared) {
		shared = &PHALCON_GLOBAL(z_false);
	}

	PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi", &PHALCON_GLOBAL(z_true));
	PHALCON_MM_ADD_ENTRY(&dependency_injector);
	PHALCON_MM_CALL_METHOD(return_value, &dependency_injector, "set", service_name, definition, shared);
	RETURN_MM();
}

/**
 * Obtains a service from the DI
 *
 * @param string $serviceName
 * @return object|null
 */
PHP_METHOD(Phalcon_Di_Injectable, getService){

	zval *service_name, dependency_injector = {};

	phalcon_fetch_params(1, 1, 0, &service_name);

	PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi", &PHALCON_GLOBAL(z_true));
	PHALCON_MM_ADD_ENTRY(&dependency_injector);
	PHALCON_MM_CALL_METHOD(return_value, &dependency_injector, "get", service_name, &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_true));
	RETURN_MM();
}

/**
 * Resolves the service based on its configuration
 *
 * @param string $name
 * @param array $parameters
 * @param boolean $noError
 * @param boolean $noShared
 * @return mixed
 */
PHP_METHOD(Phalcon_Di_Injectable, getResolveService){

	zval *name, *args = NULL, *noerror = NULL, *noshared = NULL, dependency_injector = {};

	phalcon_fetch_params(1, 1, 3, &name, &args, &noerror, &noshared);

	if (!args) {
		args = &PHALCON_GLOBAL(z_null);
	}

	if (!noerror) {
		noerror = &PHALCON_GLOBAL(z_false);
	}

	if (!noshared) {
		noshared = &PHALCON_GLOBAL(z_false);
	}

	ZVAL_NULL(return_value);

	PHALCON_CALL_METHOD(&dependency_injector, getThis(), "getdi", noerror);
	PHALCON_MM_ADD_ENTRY(&dependency_injector);
	if (Z_TYPE(dependency_injector) == IS_OBJECT) {
		if (zend_is_true(noshared)) {
			PHALCON_MM_CALL_METHOD(return_value, &dependency_injector, "get", name, args, noerror);
		} else {
			PHALCON_MM_CALL_METHOD(return_value, &dependency_injector, "getshared", name, args, noerror);
		}
	}
	RETURN_MM();
}

/**
 * Attach a listener to the events
 *
 * @param string $eventType
 * @param Closure $callback
 */
PHP_METHOD(Phalcon_Di_Injectable, attachEvent){

	zval *event_type, *callback, prefixed = {}, new_callback = {};

	phalcon_fetch_params(1, 2, 0, &event_type, &callback);

	phalcon_fast_strtolower(&prefixed, event_type);
	PHALCON_MM_ADD_ENTRY(&prefixed);

	PHALCON_MM_CALL_CE_STATIC(&new_callback, zend_ce_closure, "bind", callback, getThis());
	PHALCON_MM_ADD_ENTRY(&new_callback);
	phalcon_update_property_array(getThis(), SL("_eventCallbacks"), &prefixed, &new_callback);

	RETURN_MM_THIS();
}

/**
 * Magic method __get
 *
 * @param string $propertyName
 */
PHP_METHOD(Phalcon_Di_Injectable, __get){

	zval *property_name, dependency_injector = {}, has_service = {}, service = {}, class_name = {}, arguments = {};

	phalcon_fetch_params(1, 1, 0, &property_name);
	PHALCON_ENSURE_IS_STRING(property_name);

	PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi");
	PHALCON_MM_ADD_ENTRY(&dependency_injector);
	PHALCON_MM_CALL_METHOD(&has_service, &dependency_injector, "has", property_name);

	if (zend_is_true(&has_service)) {
		PHALCON_MM_CALL_METHOD(return_value, &dependency_injector, "getshared", property_name);
		RETURN_MM();
	}

	if (Z_STRLEN_P(property_name) == sizeof("di")-1 && !memcmp(Z_STRVAL_P(property_name), "di", sizeof("di")-1)) {
		zend_update_property(phalcon_di_injectable_ce, getThis(), SL("di"), &dependency_injector);
		RETURN_MM_CTOR(&dependency_injector);
	}

	/**
	 * Accessing the persistent property will create a session bag in any class
	 */
	if (Z_STRLEN_P(property_name) == sizeof("persistent")-1 && !memcmp(Z_STRVAL_P(property_name), "persistent", sizeof("persistent")-1)) {
		ZVAL_STRING(&class_name, Z_OBJCE_P(getThis())->name->val);

		array_init_size(&arguments, 1);
		add_next_index_zval(&arguments, &class_name);
		PHALCON_MM_ADD_ENTRY(&arguments);

		ZVAL_STR(&service, IS(sessionBag));
		PHALCON_MM_CALL_METHOD(return_value, &dependency_injector, "get", &service, &arguments);
		zend_update_property(phalcon_di_injectable_ce, getThis(), SL("persistent"), return_value);
		RETURN_MM();
	}

	/**
	 * A notice is shown if the property is not defined or is not a valid service
	 */
	php_error_docref(NULL, E_WARNING, "Access to undefined property %s::%s", Z_OBJCE_P(getThis())->name->val, Z_STRVAL_P(property_name));
	RETURN_MM_NULL();
}

PHP_METHOD(Phalcon_Di_Injectable, __sleep){

	zval dependency_injector = {}, dependency_name = {};

	phalcon_read_property(&dependency_injector, getThis(), SL("_dependencyInjector"), PH_READONLY);
	if (Z_TYPE(dependency_injector) == IS_OBJECT) {
		PHALCON_CALL_METHOD(&dependency_name, &dependency_injector, "getname");
	} else {
		ZVAL_COPY(&dependency_name, &dependency_injector);
	}

	phalcon_update_property(getThis(), SL("_dependencyInjector"), &dependency_name);
	zval_ptr_dtor(&dependency_name);

	phalcon_get_object_members(return_value, getThis(), 0);
}

PHP_METHOD(Phalcon_Di_Injectable, __debugInfo){

	phalcon_get_object_vars(return_value, getThis(), 1);

	if (likely(!PHALCON_GLOBAL(debug).enable_debug)) {
		phalcon_array_unset_str(return_value, SL("_dependencyInjector"), 0);
		phalcon_array_unset_str(return_value, SL("_eventsManager"), 0);
	}
}
