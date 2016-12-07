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

#include "events/manager.h"
#include "events/managerinterface.h"
#include "events/event.h"
#include "events/listener.h"
#include "events/exception.h"

#include <Zend/zend_closures.h>
#include <ext/spl/spl_heap.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/hash.h"

/**
 * Phalcon\Events\Manager
 *
 * Phalcon Events Manager, offers an easy way to intercept and manipulate, if needed,
 * the normal flow of operation. With the EventsManager the developer can create hooks or
 * plugins that will offer monitoring of data, manipulation, conditional execution and much more.
 *
 */
zend_class_entry *phalcon_events_manager_ce;

PHP_METHOD(Phalcon_Events_Manager, attach);
PHP_METHOD(Phalcon_Events_Manager, enablePriorities);
PHP_METHOD(Phalcon_Events_Manager, arePrioritiesEnabled);
PHP_METHOD(Phalcon_Events_Manager, collectResponses);
PHP_METHOD(Phalcon_Events_Manager, isCollecting);
PHP_METHOD(Phalcon_Events_Manager, getResponses);
PHP_METHOD(Phalcon_Events_Manager, detach);
PHP_METHOD(Phalcon_Events_Manager, detachAll);
PHP_METHOD(Phalcon_Events_Manager, fireQueue);
PHP_METHOD(Phalcon_Events_Manager, fire);
PHP_METHOD(Phalcon_Events_Manager, hasListeners);
PHP_METHOD(Phalcon_Events_Manager, getListeners);
PHP_METHOD(Phalcon_Events_Manager, getEvents);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_events_manager_enablepriorities, 0, 0, 1)
	ZEND_ARG_INFO(0, enablePriorities)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_events_manager_collectresponses, 0, 0, 1)
	ZEND_ARG_INFO(0, collect)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_events_manager_firequeue, 0, 0, 2)
	ZEND_ARG_INFO(0, queue)
	ZEND_ARG_INFO(0, event)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_events_manager_haslisteners, 0, 0, 1)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_events_manager_method_entry[] = {
	PHP_ME(Phalcon_Events_Manager, attach, arginfo_phalcon_events_managerinterface_attach, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Manager, enablePriorities, arginfo_phalcon_events_manager_enablepriorities, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Manager, arePrioritiesEnabled, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Manager, collectResponses, arginfo_phalcon_events_manager_collectresponses, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Manager, isCollecting, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Manager, getResponses, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Manager, detach, arginfo_phalcon_events_managerinterface_detach, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Manager, detachAll, arginfo_phalcon_events_managerinterface_detachall, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Manager, fireQueue, arginfo_phalcon_events_manager_firequeue, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Manager, fire, arginfo_phalcon_events_managerinterface_fire, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Manager, hasListeners, arginfo_phalcon_events_manager_haslisteners, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Manager, getListeners, arginfo_phalcon_events_managerinterface_getlisteners, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Manager, getEvents, NULL, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Events_Manager, dettachAll, detachAll, arginfo_phalcon_events_managerinterface_detachall, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
	PHP_MALIAS(Phalcon_Events_Manager, clearListeners, detachAll, arginfo_phalcon_events_managerinterface_detachall, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
	PHP_FE_END
};


/**
 * Phalcon\Events\Manager initializer
 */
PHALCON_INIT_CLASS(Phalcon_Events_Manager){

	PHALCON_REGISTER_CLASS(Phalcon\\Events, Manager, events_manager, phalcon_events_manager_method_entry, 0);

	zend_declare_property_null(phalcon_events_manager_ce, SL("_events"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_events_manager_ce, SL("_collect"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_events_manager_ce, SL("_enablePriorities"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_events_manager_ce, SL("_responses"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_events_manager_ce, 1, phalcon_events_managerinterface_ce);

	return SUCCESS;
}

/**
 * Attach a listener to the events manager
 *
 * @param string $eventType
 * @param callable $handler
 * @param int $priority
 */
PHP_METHOD(Phalcon_Events_Manager, attach){

	zval *event_type, *handler, *_priority = NULL, priority = {}, events = {}, listener = {}, enable_priorities = {}, priority_queue = {};

	phalcon_fetch_params(0, 2, 1, &event_type, &handler, &_priority);

	if (!_priority) {
		ZVAL_LONG(&priority, 100);
	} else {
		PHALCON_CPY_WRT(&priority, _priority);
	}

	if (unlikely(Z_TYPE_P(event_type) != IS_STRING)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_events_exception_ce, "Event type must be a string");
		return;
	}

	if (Z_TYPE_P(handler) != IS_OBJECT && !phalcon_is_callable(handler)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_events_exception_ce, "Event handler must be an object or callable");
		return;
	}

	if (phalcon_instance_of_ev(handler, phalcon_events_listener_ce)) {
		PHALCON_CPY_WRT(&listener, handler);
		PHALCON_CALL_METHODW(NULL, &listener, "setpriority", &priority);
		PHALCON_CALL_METHODW(NULL, &listener, "setevent", event_type);
	} else {
		object_init_ex(&listener, phalcon_events_listener_ce);
		PHALCON_CALL_METHODW(NULL, &listener, "__construct", handler, &priority, event_type);
	}

	phalcon_return_property(&events, getThis(), SL("_events"));
	if (Z_TYPE(events) != IS_ARRAY) {
		array_init(&events);
	}

	if (!phalcon_array_isset_fetch(&priority_queue, &events, event_type, 0)) {
		phalcon_return_property(&enable_priorities, getThis(), SL("_enablePriorities"));
		if (zend_is_true(&enable_priorities)) {
			/** 
			 * Create a SplPriorityQueue to store the events with priorities
			 */
			object_init_ex(&priority_queue, spl_ce_SplPriorityQueue);
			if (phalcon_has_constructor(&priority_queue)) {
				PHALCON_CALL_METHODW(NULL, &priority_queue, "__construct");
			}

			/** 
			 * Extract only the Data, Set extraction flags
			 */
			PHALCON_CALL_METHODW(NULL, &priority_queue, "setextractflags", &PHALCON_GLOBAL(z_one));

			/** 
			 * Append the events to the queue
			 */
			phalcon_array_update_zval(&events, event_type, &priority_queue, PH_COPY);
			phalcon_update_property_zval(getThis(), SL("_events"), &events);
		} else {
			array_init(&priority_queue);
		}
	}

	/** 
	 * Insert the handler in the queue
	 */
	if (unlikely(Z_TYPE(priority_queue) == IS_OBJECT)) {
		PHALCON_CALL_METHODW(NULL, &priority_queue, "insert", &listener, &priority);
	} else {
		phalcon_array_append(&priority_queue, &listener, PH_COPY);
	}

	/** 
	 * Append the events to the queue
	 */
	phalcon_array_update_zval(&events, event_type, &priority_queue, PH_COPY);
	phalcon_update_property_zval(getThis(), SL("_events"), &events);
}

/**
 * Set if priorities are enabled in the EventsManager
 *
 * @param boolean $enablePriorities
 */
PHP_METHOD(Phalcon_Events_Manager, enablePriorities){

	zval *enable_priorities;

	phalcon_fetch_params(0, 1, 0, &enable_priorities);

	phalcon_update_property_zval(getThis(), SL("_enablePriorities"), enable_priorities);

}

/**
 * Returns if priorities are enabled
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Events_Manager, arePrioritiesEnabled){


	RETURN_MEMBER(getThis(), "_enablePriorities");
}

/**
 * Tells the event manager if it needs to collect all the responses returned by every
 * registered listener in a single fire
 *
 * @param boolean $collect
 */
PHP_METHOD(Phalcon_Events_Manager, collectResponses){

	zval *collect;

	phalcon_fetch_params(0, 1, 0, &collect);

	phalcon_update_property_zval(getThis(), SL("_collect"), collect);

}

/**
 * Check if the events manager is collecting all all the responses returned by every
 * registered listener in a single fire
 */
PHP_METHOD(Phalcon_Events_Manager, isCollecting){


	RETURN_MEMBER(getThis(), "_collect");
}

/**
 * Returns all the responses returned by every handler executed by the last 'fire' executed
 *
 * @return array
 */
PHP_METHOD(Phalcon_Events_Manager, getResponses){


	RETURN_MEMBER(getThis(), "_responses");
}

/**
 * Detach a listener from the events manager
 *
 * @param object|callable $handler
 */
PHP_METHOD(Phalcon_Events_Manager, detach){

	zval *type, *handler, events = {}, queue = {}, priority_queue = {}, *listener;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 2, 0, &type, &handler);

	if (Z_TYPE_P(handler) != IS_OBJECT && !phalcon_is_callable(handler)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_events_exception_ce, "Event handler must be an object or callable");
		return;
	}

	phalcon_return_property(&events, getThis(), SL("_events"));
	if (Z_TYPE(events) != IS_ARRAY) {
		RETURN_FALSE;
	}

	if (!phalcon_array_isset_fetch(&queue, &events, type, 0)) {
		RETURN_FALSE;
	}

	if (Z_TYPE(queue) == IS_OBJECT) {
		object_init_ex(&priority_queue, spl_ce_SplPriorityQueue);
		if (phalcon_has_constructor(&priority_queue)) {
			PHALCON_CALL_METHODW(NULL, &priority_queue, "__construct");
		}

		PHALCON_CALL_METHODW(NULL, &queue, "top");

		while (1) {
			zval r0 = {}, listener0 = {}, handler_embeded = {}, priority = {};
			PHALCON_CALL_METHODW(&r0, &queue, "valid");
			if (!zend_is_true(&r0)) {
				break;
			}

			PHALCON_CALL_METHODW(&listener0, &queue, "current");
			PHALCON_CALL_METHODW(&handler_embeded, &listener0, "getlistener");

			if (!phalcon_is_equal(&handler_embeded, handler)) {
				PHALCON_CALL_METHODW(&priority, &listener0, "getpriority");
				PHALCON_CALL_METHODW(NULL, &priority_queue, "insert", &listener0, &priority);
			}
			
			PHALCON_CALL_METHODW(NULL, &queue, "next");
		}
	} else {
		PHALCON_CPY_WRT_CTOR(&priority_queue, &queue);
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(queue), idx, str_key, listener) {
			zval key = {}, handler_embeded = {};
			if (str_key) {
				ZVAL_STR(&key, str_key);
			} else {
				ZVAL_LONG(&key, idx);
			}

			PHALCON_CALL_METHODW(&handler_embeded, listener, "getlistener");

			if (phalcon_is_equal_object(&handler_embeded, handler)) {
				phalcon_array_unset(&priority_queue, &key, PH_COPY);
			}

		} ZEND_HASH_FOREACH_END();
	}

	phalcon_array_update_zval(&events, type, &priority_queue, PH_COPY);
	phalcon_update_property_zval(getThis(), SL("_events"), &events);
}

/**
 * Removes all events from the EventsManager
 *
 * @param string $type
 */
PHP_METHOD(Phalcon_Events_Manager, detachAll){

	zval *type = NULL, events = {};

	phalcon_fetch_params(0, 0, 1, &type);

	if (!type) {
		type = &PHALCON_GLOBAL(z_null);
	}

	phalcon_return_property(&events, getThis(), SL("_events"));
	if (Z_TYPE_P(type) != IS_NULL && phalcon_array_isset(&events, type)) {
		phalcon_array_unset(&events, type, PH_COPY);
	}

	phalcon_update_property_zval(getThis(), SL("_events"), &events);
}

/**
 * Internal handler to call a queue of events
 *
 * @param \SplPriorityQueue $queue
 * @param Phalcon\Events\Event $event
 * @return mixed
 */
PHP_METHOD(Phalcon_Events_Manager, fireQueue){

	zval *queue, *event, event_name = {}, source = {}, data = {}, cancelable = {}, collect = {}, iterator = {}, *listener, status = {};
	zend_class_entry *ce, *weakref_ce;

	phalcon_fetch_params(0, 2, 0, &queue, &event);
	ZVAL_NULL(&status);

	if (unlikely(Z_TYPE_P(queue) != IS_ARRAY)) {
		if (Z_TYPE_P(queue) == IS_OBJECT) {
			ce = Z_OBJCE_P(queue);
			if (!instanceof_function_ex(ce, phalcon_events_event_ce, 0) && !instanceof_function_ex(ce, spl_ce_SplPriorityQueue, 0)) {
				PHALCON_THROW_EXCEPTION_FORMATW(phalcon_events_exception_ce, "Unexpected value type: expected object of type Phalcon\\Events\\Event or SplPriorityQueue, %s given", ce->name->val);
				return;
			}
		} else {
			PHALCON_THROW_EXCEPTION_FORMATW(phalcon_events_exception_ce, "Unexpected value type: expected object of type Phalcon\\Events\\Event or SplPriorityQueue, %s given", zend_zval_type_name(queue));
			return;
		}
	}

	PHALCON_VERIFY_CLASS_EX(event, phalcon_events_event_ce, phalcon_events_exception_ce, 0);

	weakref_ce = zend_lookup_class_ex(zend_string_init(SL("WeakRef"), 0), NULL, 0);

	/** 
	 * Get the event type
	 */
	PHALCON_CALL_METHODW(&event_name, event, "gettype");
	if (unlikely(Z_TYPE(event_name) != IS_STRING)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_events_exception_ce, "The event type not valid");
		return;
	}

	/** 
	 * Get the object who triggered the event
	 */
	PHALCON_CALL_METHODW(&source, event, "getsource");

	/** 
	 * Get extra data passed to the event
	 */
	PHALCON_CALL_METHODW(&data, event, "getdata");

	/** 
	 * Tell if the event is cancelable
	 */
	PHALCON_CALL_METHODW(&cancelable, event, "iscancelable");

	/** 
	 * Responses need to be traced?
	 */
	phalcon_return_property(&collect, getThis(), SL("_collect"));
	if (Z_TYPE_P(queue) == IS_OBJECT) {
		/** 
		 * We need to clone the queue before iterate over it
		 */
		if (phalcon_clone(&iterator, queue) == FAILURE) {
			return;
		}

		/** 
		 * Move the queue to the top
		 */
		PHALCON_CALL_METHODW(NULL, &iterator, "top");

		while (1) {
			zval r0 = {}, listener0 = {}, handler_embeded = {}, handler_referenced = {}, handler = {}, arguments = {}, is_stopped = {};

			PHALCON_CALL_METHODW(&r0, &iterator, "valid");
			if (!zend_is_true(&r0)) {
				break;
			}

			/** 
			 * Get the current data
			 */
			PHALCON_CALL_METHODW(&listener0, &iterator, "current");
			PHALCON_CALL_METHODW(&handler_embeded, &listener0, "getlistener");

			/** 
			 * Only handler objects are valid
			 */
			if (Z_TYPE(handler_embeded) == IS_OBJECT) {
				/**
				 * Check if the event is a weak reference.
				 */
				if (weakref_ce && instanceof_function(Z_OBJCE(handler_embeded), weakref_ce)) {
					/**
					 * Checks whether the object referenced still exists.
					 */
					PHALCON_CALL_METHODW(&handler_referenced, &handler_embeded, "valid");

					if (zend_is_true(&handler_referenced)) {
						PHALCON_CALL_METHODW(&handler, &handler_embeded, "get");
					} else {
						/**
						 * Move the queue to the next handler
						 */
						PHALCON_CALL_METHODW(NULL, &iterator, "next");
						continue;
					}

				} else {
					PHALCON_CPY_WRT(&handler, &handler_embeded);
				}

				/** 
				 * Check if the event is a closure
				 */
				assert(Z_TYPE(handler) == IS_OBJECT);
				if (instanceof_function(Z_OBJCE(handler), zend_ce_closure)) {
					/** 
					 * Create the closure arguments
					 */
					if (Z_TYPE(arguments) <= IS_NULL) {
						array_init_size(&arguments, 3);
						phalcon_array_append(&arguments, event, PH_COPY);
						phalcon_array_append(&arguments, &source, PH_COPY);
						phalcon_array_append(&arguments, &data, PH_COPY);
					}

					/** 
					 * Call the function in the PHP userland
					 */
					PHALCON_CALL_USER_FUNC_ARRAYW(&status, &handler, &arguments);

					/** 
					 * Trace the response
					 */
					if (zend_is_true(&collect)) {
						phalcon_update_property_array_append(getThis(), SL("_responses"), &status);
					}

					if (zend_is_true(&cancelable)) {
						/** 
						 * Check if the event was stopped by the user
						 */
						PHALCON_CALL_METHODW(&is_stopped, event, "isstopped");
						if (zend_is_true(&is_stopped)) {
							break;
						}
					}
				} else {
					/** 
					 * Check if the listener has implemented an event with the same name
					 */
					if (phalcon_method_exists(&handler, &event_name) == SUCCESS) {
						/** 
						 * Call the function in the PHP userland
						 */
						PHALCON_CALL_METHODW(&status, &handler, Z_STRVAL(event_name), event, &source, &data);

						/** 
						 * Collect the response
						 */
						if (zend_is_true(&collect)) {
							phalcon_update_property_array_append(getThis(), SL("_responses"), &status);
						}

						if (zend_is_true(&cancelable)) {

							/** 
							 * Check if the event was stopped by the user
							 */
							PHALCON_CALL_METHODW(&is_stopped, event, "isstopped");
							if (zend_is_true(&is_stopped)) {
								break;
							}
						}
					}
				}
			}

			/** 
			 * Move the queue to the next handler
			 */
			PHALCON_CALL_METHODW(NULL, &iterator, "next");
		}
	} else {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(queue), listener) {
			zval handler_embeded = {}, handler_referenced = {}, handler = {}, arguments = {}, is_stopped = {};

			PHALCON_CALL_METHODW(&handler_embeded, listener, "getlistener");
			/** 
			 * Only handler objects are valid
			 */
			if (Z_TYPE(handler_embeded) == IS_OBJECT) {

				/**
				  * Check if the event is a weak reference.
				  */
				if (weakref_ce && instanceof_function(Z_OBJCE(handler_embeded), weakref_ce)) {
					/**
					 * Checks whether the object referenced still exists.
					 */
					PHALCON_CALL_METHODW(&handler_referenced, &handler_embeded, "valid");

					if (zend_is_true(&handler_referenced)) {
						PHALCON_CALL_METHODW(&handler, &handler_embeded, "get");
					} else {
						continue;
					}

				} else {
					PHALCON_CPY_WRT(&handler, &handler_embeded);
				}

				/** 
				 * Check if the event is a closure
				 */
				assert(Z_TYPE(handler) == IS_OBJECT);
				if (instanceof_function(Z_OBJCE(handler), zend_ce_closure)) {
					/** 
					 * Create the closure arguments
					 */
					if (Z_TYPE(arguments) <= IS_NULL) {
						array_init_size(&arguments, 3);
						phalcon_array_append(&arguments, event, PH_COPY);
						phalcon_array_append(&arguments, &source, PH_COPY);
						phalcon_array_append(&arguments, &data, PH_COPY);
					}

					/** 
					 * Call the function in the PHP userland
					 */
					PHALCON_CALL_USER_FUNC_ARRAYW(&status, &handler, &arguments);

					/** 
					 * Trace the response
					 */
					if (zend_is_true(&collect)) {
						phalcon_update_property_array_append(getThis(), SL("_responses"), &status);
					}

					if (zend_is_true(&cancelable)) {

						/** 
						 * Check if the event was stopped by the user
						 */
						PHALCON_CALL_METHODW(&is_stopped, event, "isstopped");
						if (zend_is_true(&is_stopped)) {
							break;
						}
					}
				} else {
					/** 
					 * Check if the listener has implemented an event with the same name
					 */
					if (phalcon_method_exists(&handler, &event_name) == SUCCESS) {
						/** 
						 * Call the function in the PHP userland
						 */
						PHALCON_CALL_METHODW(&status, &handler, Z_STRVAL(event_name), event, &source, &data);

						/** 
						 * Collect the response
						 */
						if (zend_is_true(&collect)) {
							phalcon_update_property_array_append(getThis(), SL("_responses"), &status);
						}

						if (zend_is_true(&cancelable)) {
							/** 
							 * Check if the event was stopped by the user
							 */
							PHALCON_CALL_METHODW(&is_stopped, event, "isstopped");
							if (zend_is_true(&is_stopped)) {
								break;
							}
						}
					}
				}
			}
		} ZEND_HASH_FOREACH_END();
	}

	RETURN_CTORW(&status);
}

/**
 * Fires an event in the events manager causing that active listeners be notified about it
 *
 *<code>
 *	$eventsManager->fire('db', $connection);
 *</code>
 *
 * @param string $eventType
 * @param object $source
 * @param mixed  $data
 * @param int $cancelable
 * @return mixed
 */
PHP_METHOD(Phalcon_Events_Manager, fire){

	zval *event_type, *source, *data = NULL, *cancelable = NULL, events = {}, exception_message = {}, event_parts = {}, type = {};
	zval event_name = {}, status = {}, collect = {}, event = {}, fire_events = {};

	phalcon_fetch_params(0, 2, 2, &event_type, &source, &data, &cancelable);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!cancelable) {
		cancelable = &PHALCON_GLOBAL(z_true);
	}

	if (unlikely(Z_TYPE_P(event_type) != IS_STRING)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_events_exception_ce, "Event type must be a string");
		return;
	}

	phalcon_return_property(&events, getThis(), SL("_events"));
	if (Z_TYPE(events) != IS_ARRAY) { 
		RETURN_NULL();
	}

	ZVAL_NULL(&status);

	/** 
	 * All valid events must have a colon separator
	 */
	if (!phalcon_memnstr_str(event_type, SL(":"))) {
		PHALCON_CONCAT_SV(&exception_message, "Invalid event type ", event_type);
		PHALCON_THROW_EXCEPTION_ZVALW(phalcon_events_exception_ce, &exception_message);
		return;
	}

	phalcon_fast_explode_str(&event_parts, SL(":"), event_type);
	phalcon_array_fetch_long(&type, &event_parts, 0, PH_NOISY);
	phalcon_array_fetch_long(&event_name, &event_parts, 1, PH_NOISY);

	/** 
	 * Should responses be traced?
	 */
	phalcon_return_property(&collect, getThis(), SL("_collect"));
	if (zend_is_true(&collect)) {
		phalcon_update_property_null(getThis(), SL("_responses"));
	}

	/** 
	 * Check if events are grouped by type
	 */
	if (phalcon_array_isset_fetch(&fire_events, &events, &type, 0)) {
		if (Z_TYPE(fire_events) == IS_ARRAY || Z_TYPE(fire_events) == IS_OBJECT) {
			/** 
			 * Create the event context
			 */
			PHALCON_OBJECT_INIT(&event, phalcon_events_event_ce);
			PHALCON_CALL_METHODW(NULL, &event, "__construct", &event_name, source, data, cancelable);

			/** 
			 * Call the events queue
			 */
			PHALCON_CALL_METHODW(&status, getThis(), "firequeue", &fire_events, &event);
		}
	}

	/** 
	 * Check if there are listeners for the event type itself
	 */
	if (phalcon_array_isset_fetch(&fire_events, &events, event_type, 0)) {
		if (Z_TYPE(fire_events) == IS_ARRAY || Z_TYPE(fire_events) == IS_OBJECT) {
			/** 
			 * Create the event if it wasn't created before
			 */
			if (Z_TYPE(event) != IS_OBJECT) {
				object_init_ex(&event, phalcon_events_event_ce);
				PHALCON_CALL_METHODW(NULL, &event, "__construct", &event_name, source, data, cancelable);

			}

			/** 
			 * Call the events queue
			 */
			PHALCON_CALL_METHODW(&status, getThis(), "firequeue", &fire_events, &event);
		}
	}

	RETURN_CTORW(&status);
}

/**
 * Check whether certain type of event has listeners
 *
 * @param string $type
 * @return boolean
 */
PHP_METHOD(Phalcon_Events_Manager, hasListeners){

	zval *type, events = {};

	phalcon_fetch_params(0, 1, 0, &type);

	phalcon_return_property(&events, getThis(), SL("_events"));
	if (phalcon_array_isset(&events, type)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Returns all the attached listeners of a certain type
 *
 * @param string $type
 * @return array
 */
PHP_METHOD(Phalcon_Events_Manager, getListeners){

	zval *type, *full = NULL, events = {}, queue = {}, iterator = {}, *listener;

	phalcon_fetch_params(0, 1, 1, &type, &full);

	if (!full) {
		full = &PHALCON_GLOBAL(z_false);
	}

	phalcon_return_property(&events, getThis(), SL("_events"));
	if (Z_TYPE(events) != IS_ARRAY) {
		RETURN_EMPTY_ARRAY();
	}

	if (!phalcon_array_isset(&events, type)) {
		RETURN_EMPTY_ARRAY();
	}

	array_init(return_value);

	phalcon_array_fetch(&queue, &events, type, PH_NOISY);

	if (zend_is_true(full)) {
		RETURN_CTORW(&queue);
	}

	if (Z_TYPE(queue) == IS_OBJECT) {
		if (phalcon_clone(&iterator, &queue) == FAILURE) {
			return;
		}

		PHALCON_CALL_METHODW(NULL, &iterator, "top");

		while (1) {
			zval r0 = {}, listener0 = {}, handler_embeded = {};

			PHALCON_CALL_METHODW(&r0, &iterator, "valid");
			if (!zend_is_true(&r0)) {
				break;
			}

			PHALCON_CALL_METHODW(&listener0, &iterator, "current");
			PHALCON_CALL_METHODW(&handler_embeded, &listener0, "getlistener");

			phalcon_array_append(return_value, &handler_embeded, PH_COPY);

			PHALCON_CALL_METHODW(NULL, &iterator, "next");
		}
	} else {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(queue), listener) {
			zval handler_embeded = {};
			PHALCON_CALL_METHODW(&handler_embeded, listener, "getlistener");
			phalcon_array_append(return_value, &handler_embeded, PH_COPY);
		} ZEND_HASH_FOREACH_END();
	}
}

/**
 * Retrieve all registered events
 *
 * @return array
 */
PHP_METHOD(Phalcon_Events_Manager, getEvents){

	zval events = {};

	phalcon_return_property(&events, getThis(), SL("_events"));
	phalcon_array_keys(return_value, &events);
}
