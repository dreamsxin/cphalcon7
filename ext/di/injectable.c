
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
#include "diinterface.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/string.h"

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
PHP_METHOD(Phalcon_Di_Injectable, getResolveService);
PHP_METHOD(Phalcon_Di_Injectable, __get);
PHP_METHOD(Phalcon_Di_Injectable, __sleep);
PHP_METHOD(Phalcon_Di_Injectable, __debugInfo);

static const zend_function_entry phalcon_di_injectable_method_entry[] = {
	PHP_ME(Phalcon_Di_Injectable, setDI, arginfo_phalcon_di_injectionawareinterface_setdi, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Di_Injectable, getDI, arginfo_phalcon_di_injectionawareinterface_getdi, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Di_Injectable, setEventsManager, arginfo_phalcon_events_eventsawareinterface_seteventsmanager, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Di_Injectable, getEventsManager, arginfo_phalcon_events_eventsawareinterface_geteventsmanager, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Di_Injectable, fireEvent, arginfo_phalcon_di_injectable_fireevent, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Di_Injectable, fireEventCancel, arginfo_phalcon_di_injectable_fireeventcancel, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Di_Injectable, hasService, arginfo_phalcon_di_injectable_hasservice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Di_Injectable, getResolveService, arginfo_phalcon_di_injectable_getresolveservice, ZEND_ACC_PUBLIC)
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
		phalcon_update_property_zval(getThis(), SL("_dependencyInjector"), &di);
	} else {
		PHALCON_VERIFY_INTERFACE_OR_NULL_EX(dependency_injector, phalcon_diinterface_ce, phalcon_di_exception_ce);
		phalcon_update_property_zval(getThis(), SL("_dependencyInjector"), dependency_injector);
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
	zval *error = NULL, *not_use_default = NULL, dependency_injector = {};

	phalcon_fetch_params(0, 0, 2, &error, &not_use_default);

	if (!error) {
		error = &PHALCON_GLOBAL(z_false);
	}

	if (!not_use_default) {
		not_use_default = &PHALCON_GLOBAL(z_false);
	}

	phalcon_return_property(&dependency_injector, getThis(), SL("_dependencyInjector"));
	if (Z_TYPE(dependency_injector) != IS_OBJECT && !zend_is_true(not_use_default)) {
		PHALCON_CALL_CE_STATIC(&dependency_injector, phalcon_di_ce, "getdefault", &dependency_injector);
	}

	if (Z_TYPE(dependency_injector) != IS_OBJECT && zend_is_true(error)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_di_exception_ce, "A dependency injection container is not object");
		return;
	}

	RETURN_CTOR(&dependency_injector);
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

	phalcon_update_property_zval(getThis(), SL("_eventsManager"), events_manager);

	RETURN_THIS();
}

/**
 * Returns the internal event manager
 *
 * @return Phalcon\Events\ManagerInterface
 */
PHP_METHOD(Phalcon_Di_Injectable, getEventsManager){


	RETURN_MEMBER(getThis(), "_eventsManager");
}

/**
 * Fires an event, implicitly calls behaviors and listeners in the events manager are notified
 *
 * @param string $eventName
 * @return boolean
 */
PHP_METHOD(Phalcon_Di_Injectable, fireEvent){

	zval *eventname, *data = NULL, *cancelable = NULL, events_manager = {}, lower = {}, event_parts = {}, name = {}, status = {};

	phalcon_fetch_params(0, 1, 2, &eventname, &data, &cancelable);
	PHALCON_ENSURE_IS_STRING(eventname);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!cancelable) {
		cancelable = &PHALCON_GLOBAL(z_true);
	}

	phalcon_fast_strtolower(&lower, eventname);

	if (phalcon_memnstr_str(&lower, SL(":"))) {
		phalcon_fast_explode_str(&event_parts, SL(":"), &lower);
		phalcon_array_fetch_long(&name, &event_parts, 1, PH_NOISY);
	} else {
		PHALCON_CPY_WRT(&name, &lower);
	}

	/**
	 * Check if there is a method with the same name of the event
	 */
	if (phalcon_method_exists(getThis(), &name) == SUCCESS) {
		PHALCON_CALL_METHOD(NULL, getThis(), Z_STRVAL(name), data);
	}

	phalcon_return_property(&events_manager, getThis(), SL("_eventsManager"));

	if (Z_TYPE(events_manager) != IS_NULL) {
		PHALCON_VERIFY_INTERFACE_EX(&events_manager, phalcon_events_managerinterface_ce, phalcon_di_exception_ce);

		/**
		 * Send a notification to the events manager
		 */
		PHALCON_CALL_METHOD(&status, &events_manager, "fire", eventname, getThis(), data, cancelable);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}
	}

	RETURN_TRUE;
}

/**
 * Fires an event, implicitly calls behaviors and listeners in the events manager are notified
 * This method stops if one of the callbacks/listeners returns boolean false
 *
 * @param string $eventName
 * @return boolean
 */
PHP_METHOD(Phalcon_Di_Injectable, fireEventCancel){

	zval *eventname, *data = NULL, *cancelable = NULL, status = {}, events_manager = {}, lower = {}, event_parts = {}, name = {};

	phalcon_fetch_params(0, 1, 2, &eventname, &data, &cancelable);
	PHALCON_ENSURE_IS_STRING(eventname);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!cancelable) {
		cancelable = &PHALCON_GLOBAL(z_true);
	}

	phalcon_fast_strtolower(&lower, eventname);

	if (phalcon_memnstr_str(&lower, SL(":"))) {
		phalcon_fast_explode_str(&event_parts, SL(":"), &lower);
		phalcon_array_fetch_long(&name, &event_parts, 1, PH_NOISY);
	} else {
		PHALCON_CPY_WRT(&name, &lower);
	}

	/**
	 * Check if there is a method with the same name of the event
	 */
	if (phalcon_method_exists(getThis(), &name) == SUCCESS) {
		PHALCON_CALL_METHOD(&status, getThis(), Z_STRVAL(name), data);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}
	}

	phalcon_return_property(&events_manager, getThis(), SL("_eventsManager"));
	if (Z_TYPE(events_manager) != IS_NULL) {
		PHALCON_VERIFY_INTERFACE_EX(&events_manager, phalcon_events_managerinterface_ce, phalcon_di_exception_ce);

		/**
		 * Send a notification to the events manager
		 */
		PHALCON_CALL_METHOD(&status, &events_manager, "fire", eventname, getThis(), data, cancelable);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}
	}

	RETURN_TRUE;
}

/**
 * Check whether the DI contains a service by a name
 *
 * @param string $name
 * @return boolean
 */
PHP_METHOD(Phalcon_Di_Injectable, hasService){

	zval *service_name, dependency_injector = {};

	phalcon_fetch_params(0, 1, 0, &service_name);

	PHALCON_CALL_METHOD(&dependency_injector, getThis(), "getdi");
	if (Z_TYPE(dependency_injector) == IS_OBJECT) {
		PHALCON_CALL_METHOD(return_value, &dependency_injector, "has", service_name);
	} else {
		RETURN_FALSE;
	}
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

	phalcon_fetch_params(0, 1, 3, &name, &args, &noerror, &noshared);

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
	if (Z_TYPE(dependency_injector) == IS_OBJECT) {
		if (zend_is_true(noshared)) {
			PHALCON_RETURN_CALL_METHOD(&dependency_injector, "get", name, args, noerror);
		} else {
			PHALCON_RETURN_CALL_METHOD(&dependency_injector, "getshared", name, args, noerror);
		}
	}
}

/**
 * Magic method __get
 *
 * @param string $propertyName
 */
PHP_METHOD(Phalcon_Di_Injectable, __get){

	zval *property_name, dependency_injector = {}, has_service = {}, service = {}, class_name = {}, arguments = {}, result = {};

	phalcon_fetch_params(0, 1, 0, &property_name);
	PHALCON_ENSURE_IS_STRING(property_name);

	PHALCON_CALL_METHOD(&dependency_injector, getThis(), "getdi");
	PHALCON_CALL_METHOD(&has_service, &dependency_injector, "has", property_name);

	if (zend_is_true(&has_service)) {
		PHALCON_CALL_METHOD(&result, &dependency_injector, "getshared", property_name);
		RETURN_CTOR(&result);
	}

	if (Z_STRLEN_P(property_name) == sizeof("di")-1 && !memcmp(Z_STRVAL_P(property_name), "di", sizeof("di")-1)) {
		zend_update_property(phalcon_di_injectable_ce, getThis(), SL("di"), &dependency_injector);
		RETURN_CTOR(&dependency_injector);
	}

	/**
	 * Accessing the persistent property will create a session bag in any class
	 */
	if (Z_STRLEN_P(property_name) == sizeof("persistent")-1 && !memcmp(Z_STRVAL_P(property_name), "persistent", sizeof("persistent")-1)) {
		PHALCON_STR(&class_name, Z_OBJCE_P(getThis())->name->val);

		array_init_size(&arguments, 1);
		add_next_index_zval(&arguments, &class_name);

		PHALCON_STR(&service, "sessionBag");
		PHALCON_CALL_METHOD(&result, &dependency_injector, "get", &service, &arguments);

		zend_update_property(phalcon_di_injectable_ce, getThis(), SL("persistent"), &result);
		RETURN_CTOR(&result);
	}

	/**
	 * A notice is shown if the property is not defined or is not a valid service
	 */
	php_error_docref(NULL, E_WARNING, "Access to undefined property %s::%s", Z_OBJCE_P(getThis())->name->val, Z_STRVAL_P(property_name));
	RETURN_NULL();
}

PHP_METHOD(Phalcon_Di_Injectable, __sleep){

	zval dependency_injector = {}, dependency_name = {};

	phalcon_return_property(&dependency_injector, getThis(), SL("_dependencyInjector"));
	if (Z_TYPE(dependency_injector) == IS_OBJECT) {
		PHALCON_CALL_METHOD(&dependency_name, &dependency_injector, "getname");
	} else {
		PHALCON_CPY_WRT(&dependency_name, &dependency_injector);
	}

	phalcon_update_property_zval(getThis(), SL("_dependencyInjector"), &dependency_name);

	phalcon_get_object_members(return_value, getThis(), 0);
}

PHP_METHOD(Phalcon_Di_Injectable, __debugInfo){

	phalcon_get_object_vars(return_value, getThis(), 0);

	if (likely(!PHALCON_GLOBAL(debug).enable_debug)) {
		phalcon_array_unset_str(return_value, SL("_dependencyInjector"), 0);
		phalcon_array_unset_str(return_value, SL("_eventsManager"), 0);
	}
}
