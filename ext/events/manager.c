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
#include "events/eventinterface.h"
#include "events/listener.h"
#include "events/exception.h"
#include "events/../debug.h"

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
#include "kernel/debug.h"

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
PHP_METHOD(Phalcon_Events_Manager, createEvent);
PHP_METHOD(Phalcon_Events_Manager, fire);
PHP_METHOD(Phalcon_Events_Manager, hasListeners);
PHP_METHOD(Phalcon_Events_Manager, getListeners);
PHP_METHOD(Phalcon_Events_Manager, getEvents);
PHP_METHOD(Phalcon_Events_Manager, getCurrentEvent);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_events_manager_enablepriorities, 0, 0, 1)
	ZEND_ARG_INFO(0, enablePriorities)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_events_manager_collectresponses, 0, 0, 1)
	ZEND_ARG_INFO(0, collect)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_events_manager_firequeue, 0, 0, 2)
	ZEND_ARG_INFO(0, queue)
	ZEND_ARG_INFO(0, event)
	ZEND_ARG_INFO(0, flag)
	ZEND_ARG_INFO(0, prevData)
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
	PHP_ME(Phalcon_Events_Manager, createEvent, arginfo_phalcon_events_managerinterface_createevent, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Manager, fire, arginfo_phalcon_events_managerinterface_fire, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Manager, hasListeners, arginfo_phalcon_events_manager_haslisteners, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Manager, getListeners, arginfo_phalcon_events_managerinterface_getlisteners, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Manager, getEvents, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Events_Manager, getCurrentEvent, NULL, ZEND_ACC_PUBLIC)
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
	zend_declare_property_null(phalcon_events_manager_ce, SL("_currentEvent"), ZEND_ACC_PROTECTED);
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

	zval *event_type, *handler, *_priority = NULL, event = {}, priority = {}, events = {}, listener = {}, enable_priorities = {}, priority_queue = {};

	phalcon_fetch_params(1, 2, 1, &event_type, &handler, &_priority);

	if (!_priority) {
		ZVAL_LONG(&priority, 100);
	} else {
		ZVAL_COPY_VALUE(&priority, _priority);
	}

	if (Z_TYPE_P(handler) != IS_OBJECT && !phalcon_is_callable(handler)) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_events_exception_ce, "Event handler must be an object or callable");
		return;
	}

	if (!phalcon_memnstr_str(event_type, SL(":"))) {
		ZVAL_COPY_VALUE(&event, event_type);
	} else {
		zval event_parts ={}, name = {}, type = {};
		phalcon_fast_explode_str(&event_parts, SL(":"), event_type);
		phalcon_array_fetch_long(&name, &event_parts, 0, PH_READONLY);
		phalcon_array_fetch_long(&type, &event_parts, 1, PH_READONLY);
		if (PHALCON_IS_STRING(&type, "*") || PHALCON_IS_EMPTY_STRING(&type)) {
			PHALCON_MM_ZVAL_COPY(&event, &name);
		} else {
			PHALCON_MM_ZVAL_COPY(&event, event_type);
		}
		zval_ptr_dtor(&event_parts);
	}

	if (phalcon_instance_of_ev(handler, phalcon_events_listener_ce)) {
		ZVAL_COPY_VALUE(&listener, handler);
		PHALCON_MM_CALL_METHOD(NULL, &listener, "setpriority", &priority);
		PHALCON_MM_CALL_METHOD(NULL, &listener, "setevent", &event);
	} else {
		object_init_ex(&listener, phalcon_events_listener_ce);
		PHALCON_MM_ADD_ENTRY(&listener);
		PHALCON_MM_CALL_METHOD(NULL, &listener, "__construct", handler, &priority, &event);
	}

	phalcon_read_property(&events, getThis(), SL("_events"), PH_READONLY);
	if (Z_TYPE(events) != IS_ARRAY) {
		array_init(&events);
		phalcon_update_property(getThis(), SL("_events"), &events);
		zval_ptr_dtor(&events);
	}

	if (!phalcon_array_isset_fetch(&priority_queue, &events, &event, PH_READONLY)) {
		phalcon_read_property(&enable_priorities, getThis(), SL("_enablePriorities"), PH_READONLY);
		if (zend_is_true(&enable_priorities)) {
			/**
			 * Create a SplPriorityQueue to store the events with priorities
			 */
			object_init_ex(&priority_queue, spl_ce_SplPriorityQueue);
			PHALCON_MM_ADD_ENTRY(&priority_queue);
			if (phalcon_has_constructor(&priority_queue)) {
				PHALCON_MM_CALL_METHOD(NULL, &priority_queue, "__construct");
			}

			/**
			 * Extract only the Data, Set extraction flags
			 */
			PHALCON_MM_CALL_METHOD(NULL, &priority_queue, "setextractflags", &PHALCON_GLOBAL(z_one));
		} else {
			array_init(&priority_queue);
			PHALCON_MM_ADD_ENTRY(&priority_queue);
		}
	}

	/**
	 * Insert the handler in the queue
	 */
	if (unlikely(Z_TYPE(priority_queue) == IS_OBJECT)) {
		PHALCON_MM_CALL_METHOD(NULL, &priority_queue, "insert", &listener, &priority);
	} else {
		phalcon_array_append(&priority_queue, &listener, PH_COPY);
	}

	/**
	 * Append the events to the queue
	 */
	phalcon_array_update(&events, &event, &priority_queue, PH_COPY);

	RETURN_MM();
}

/**
 * Set if priorities are enabled in the EventsManager
 *
 * @param boolean $enablePriorities
 */
PHP_METHOD(Phalcon_Events_Manager, enablePriorities){

	zval *enable_priorities;

	phalcon_fetch_params(0, 1, 0, &enable_priorities);

	phalcon_update_property(getThis(), SL("_enablePriorities"), enable_priorities);

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

	phalcon_update_property(getThis(), SL("_collect"), collect);

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
		PHALCON_THROW_EXCEPTION_STR(phalcon_events_exception_ce, "Event handler must be an object or callable");
		return;
	}

	phalcon_read_property(&events, getThis(), SL("_events"), PH_READONLY);
	if (Z_TYPE(events) != IS_ARRAY) {
		RETURN_FALSE;
	}

	if (!phalcon_array_isset_fetch(&queue, &events, type, PH_READONLY)) {
		RETURN_FALSE;
	}

	if (Z_TYPE(queue) == IS_OBJECT) {
		object_init_ex(&priority_queue, spl_ce_SplPriorityQueue);
		if (phalcon_has_constructor(&priority_queue)) {
			PHALCON_CALL_METHOD(NULL, &priority_queue, "__construct");
		}

		PHALCON_CALL_METHOD(NULL, &queue, "top");

		while (1) {
			zval r0 = {}, listener0 = {}, handler_embeded = {}, priority = {};
			PHALCON_CALL_METHOD(&r0, &queue, "valid");
			if (!zend_is_true(&r0)) {
				break;
			}

			PHALCON_CALL_METHOD(&listener0, &queue, "current");
			PHALCON_CALL_METHOD(&handler_embeded, &listener0, "getlistener");

			if (!phalcon_is_equal(&handler_embeded, handler)) {
				PHALCON_CALL_METHOD(&priority, &listener0, "getpriority");
				PHALCON_CALL_METHOD(NULL, &priority_queue, "insert", &listener0, &priority);
				zval_ptr_dtor(&priority);
			}
			zval_ptr_dtor(&listener0);
			zval_ptr_dtor(&handler_embeded);

			PHALCON_CALL_METHOD(NULL, &queue, "next");
		}
	} else {
		ZVAL_DUP(&priority_queue, &queue);
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(queue), idx, str_key, listener) {
			zval key = {}, handler_embeded = {};
			if (str_key) {
				ZVAL_STR(&key, str_key);
			} else {
				ZVAL_LONG(&key, idx);
			}

			PHALCON_CALL_METHOD(&handler_embeded, listener, "getlistener");

			if (phalcon_is_equal_object(&handler_embeded, handler)) {
				phalcon_array_unset(&priority_queue, &key, 0);
			}
			zval_ptr_dtor(&handler_embeded);

		} ZEND_HASH_FOREACH_END();
	}

	phalcon_array_update(&events, type, &priority_queue, 0);
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

	phalcon_read_property(&events, getThis(), SL("_events"), PH_READONLY);
	if (Z_TYPE_P(type) != IS_NULL && phalcon_array_isset(&events, type)) {
		phalcon_array_unset(&events, type, 0);
	}
}

/**
 * Internal handler to call a queue of events
 *
 * @param \SplPriorityQueue $queue
 * @param Phalcon\Events\Event $event
 * @param boolean $flag
 * @param mixed $prevData
 * @return mixed
 */
PHP_METHOD(Phalcon_Events_Manager, fireQueue){

	zval *queue, *event, *flag = NULL, *_prev_data = NULL, event_name = {}, source = {}, data = {}, cancelable = {}, collect = {}, iterator = {}, *listener;
	zval status = {};
	zend_class_entry *ce, *weakref_ce;

	phalcon_fetch_params(0, 2, 1, &queue, &event, &flag, &_prev_data);

	if (!flag) {
		flag = &PHALCON_GLOBAL(z_false);
	}

	if (_prev_data) {
		ZVAL_COPY(&status, _prev_data);
	} else {
		ZVAL_NULL(&status);
	}

	if (unlikely(Z_TYPE_P(queue) != IS_ARRAY)) {
		if (Z_TYPE_P(queue) == IS_OBJECT) {
			ce = Z_OBJCE_P(queue);
			if (!instanceof_function_ex(ce, phalcon_events_event_ce, 0) && !instanceof_function_ex(ce, spl_ce_SplPriorityQueue, 0)) {
				PHALCON_THROW_EXCEPTION_FORMAT(phalcon_events_exception_ce, "Unexpected value type: expected object of type Phalcon\\Events\\Event or SplPriorityQueue, %s given", ce->name->val);
				return;
			}
		} else {
			PHALCON_THROW_EXCEPTION_FORMAT(phalcon_events_exception_ce, "Unexpected value type: expected object of type Phalcon\\Events\\Event or SplPriorityQueue, %s given", zend_zval_type_name(queue));
			return;
		}
	}

	PHALCON_VERIFY_CLASS_EX(event, phalcon_events_event_ce, phalcon_events_exception_ce);

	weakref_ce = phalcon_class_str_exists(SL("WeakRef"), 0);

	/**
	 * Get the event type
	 */
	PHALCON_CALL_METHOD(&event_name, event, "gettype");

	if (unlikely(Z_TYPE(event_name) != IS_STRING)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_events_exception_ce, "The event type not valid");
		return;
	}

	/**
	 * Get the object who triggered the event
	 */
	PHALCON_CALL_METHOD(&source, event, "getsource");

	/**
	 * Get extra data passed to the event
	 */
	PHALCON_CALL_METHOD(&data, event, "getdata");

	/**
	 * Tell if the event is cancelable
	 */
	PHALCON_CALL_METHOD(&cancelable, event, "iscancelable");

	/**
	 * Responses need to be traced?
	 */
	phalcon_read_property(&collect, getThis(), SL("_collect"), PH_READONLY);
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
		PHALCON_CALL_METHOD(NULL, &iterator, "top");

		while (1) {
			zval r0 = {}, listener0 = {}, handler_embeded = {}, handler_referenced = {}, handler = {}, arguments = {}, is_stopped = {};

			PHALCON_CALL_METHOD(&r0, &iterator, "valid");
			if (!zend_is_true(&r0)) {
				break;
			}

			/**
			 * Get the current data
			 */
			PHALCON_CALL_METHOD(&listener0, &iterator, "current");
			PHALCON_CALL_METHOD(&handler_embeded, &listener0, "getlistener");

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
					PHALCON_CALL_METHOD(&handler_referenced, &handler_embeded, "valid");

					if (zend_is_true(&handler_referenced)) {
						PHALCON_CALL_METHOD(&handler, &handler_embeded, "get");
					} else {
						/**
						 * Move the queue to the next handler
						 */
						PHALCON_CALL_METHOD(NULL, &iterator, "next");
						zval_ptr_dtor(&handler_embeded);
						continue;
					}

				} else {
					ZVAL_COPY(&handler, &handler_embeded);
				}

				/**
				 * Check if the event is a closure
				 */
				assert(Z_TYPE(handler) == IS_OBJECT);
				if (instanceof_function(Z_OBJCE(handler), zend_ce_closure)) {
					/**
					 * Create the closure arguments
					 */
					array_init_size(&arguments, 4);
					phalcon_array_append(&arguments, event, PH_COPY);
					phalcon_array_append(&arguments, &source, PH_COPY);
					phalcon_array_append(&arguments, &data, PH_COPY);
					phalcon_array_append(&arguments, &status, 0);

					/**
					 * Call the function in the PHP userland
					 */
					PHALCON_CALL_USER_FUNC_ARRAY(&status, &handler, &arguments);
					zval_ptr_dtor(&arguments);
					if (zend_is_true(flag) && PHALCON_IS_FALSE(&status)){
						break;
					}

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
						PHALCON_CALL_METHOD(&is_stopped, event, "isstopped");
						if (zend_is_true(&is_stopped)) {
							break;
						}
					}
				} else {
					/**
					 * Check if the listener has implemented an event with the same name
					 */
					if (phalcon_method_exists(&handler, &event_name) == SUCCESS || phalcon_method_exists_ex(&handler, SL("__call")) == SUCCESS) {
						zval prev_data = {};
						ZVAL_COPY(&prev_data, &status);
						zval_ptr_dtor(&status);
						/**
						 * Call the function in the PHP userland
						 */
						PHALCON_CALL_METHOD(&status, &handler, Z_STRVAL(event_name), event, &source, &data, &prev_data);
						zval_ptr_dtor(&prev_data);
						if (zend_is_true(flag) && PHALCON_IS_FALSE(&status)){
							break;
						}

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
							PHALCON_CALL_METHOD(&is_stopped, event, "isstopped");
							if (zend_is_true(&is_stopped)) {
								zval_ptr_dtor(&handler_embeded);
								zval_ptr_dtor(&listener0);
								break;
							}
						}
					}
				}
				zval_ptr_dtor(&handler);
			}
			zval_ptr_dtor(&handler_embeded);
			zval_ptr_dtor(&listener0);

			/**
			 * Move the queue to the next handler
			 */
			PHALCON_CALL_METHOD(NULL, &iterator, "next");
		}
	} else {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(queue), listener) {
			zval handler_embeded = {}, handler_referenced = {}, handler = {}, arguments = {}, is_stopped = {};

			PHALCON_CALL_METHOD(&handler_embeded, listener, "getlistener");
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
					PHALCON_CALL_METHOD(&handler_referenced, &handler_embeded, "valid");

					if (zend_is_true(&handler_referenced)) {
						PHALCON_CALL_METHOD(&handler, &handler_embeded, "get");
					} else {
						continue;
					}

				} else {
					ZVAL_COPY(&handler, &handler_embeded);
				}

				/**
				 * Check if the event is a closure
				 */
				assert(Z_TYPE(handler) == IS_OBJECT);
				if (instanceof_function(Z_OBJCE(handler), zend_ce_closure)) {
					/**
					 * Create the closure arguments
					 */
					array_init_size(&arguments, 4);
					phalcon_array_append(&arguments, event, PH_COPY);
					phalcon_array_append(&arguments, &source, PH_COPY);
					phalcon_array_append(&arguments, &data, PH_COPY);
					phalcon_array_append(&arguments, &status, 0);

					/**
					 * Call the function in the PHP userland
					 */
					PHALCON_CALL_USER_FUNC_ARRAY(&status, &handler, &arguments);
					zval_ptr_dtor(&arguments);
					if (zend_is_true(flag) && PHALCON_IS_FALSE(&status)){
						break;
					}

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
						PHALCON_CALL_METHOD(&is_stopped, event, "isstopped");
						if (zend_is_true(&is_stopped)) {
							break;
						}
					}
				} else {
					/**
					 * Check if the listener has implemented an event with the same name
					 */
					if (phalcon_method_exists(&handler, &event_name) == SUCCESS || phalcon_method_exists_ex(&handler, SL("__call")) == SUCCESS) {
						zval prev_data = {};
						ZVAL_COPY(&prev_data, &status);
						zval_ptr_dtor(&status);
						/**
						 * Call the function in the PHP userland
						 */
						PHALCON_CALL_METHOD(&status, &handler, Z_STRVAL(event_name), event, &source, &data, &prev_data);
						zval_ptr_dtor(&prev_data);
						if (zend_is_true(flag) && PHALCON_IS_FALSE(&status)){
							break;
						}

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
							PHALCON_CALL_METHOD(&is_stopped, event, "isstopped");
							if (zend_is_true(&is_stopped)) {
								break;
							}
						}
					}
				}
				zval_ptr_dtor(&handler);
			}
			zval_ptr_dtor(&handler_embeded);
		} ZEND_HASH_FOREACH_END();
	}
	zval_ptr_dtor(&event_name);
	zval_ptr_dtor(&source);
	zval_ptr_dtor(&data);
	zval_ptr_dtor(&cancelable);
	RETURN_NCTOR(&status);
}

/**
 * Create an event
 *
 * @param string $event_type
 * @param object $source
 * @param mixed $data
 * @param boolean $cancelable
 * @param boolean $flag
 * @return Phalcon\Events\Event
 */
PHP_METHOD(Phalcon_Events_Manager, createEvent){

	zval *event_type, *source, *data = NULL, *cancelable = NULL, *flag = NULL, exception_message = {};
	zval event_parts = {}, name = {}, type = {};

	phalcon_fetch_params(0, 2, 3, &event_type, &source, &data, &cancelable, &flag);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!cancelable) {
		cancelable = &PHALCON_GLOBAL(z_true);
	}

	if (!flag) {
		flag = &PHALCON_GLOBAL(z_false);
	}

	/**
	 * All valid events must have a colon separator
	 */
	if (!phalcon_memnstr_str(event_type, SL(":"))) {
		PHALCON_CONCAT_SV(&exception_message, "Invalid event type ", event_type);
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_events_exception_ce, &exception_message);
		zval_ptr_dtor(&exception_message);
		return;
	}

	phalcon_fast_explode_str(&event_parts, SL(":"), event_type);
	phalcon_array_fetch_long(&name, &event_parts, 0, PH_READONLY);
	phalcon_array_fetch_long(&type, &event_parts, 1, PH_READONLY);

	object_init_ex(return_value, phalcon_events_event_ce);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", &type, source, data, cancelable, flag);
	PHALCON_CALL_METHOD(NULL, return_value, "setname", &name);
	zval_ptr_dtor(&event_parts);
}

/**
 * Fires an event in the events manager causing that active listeners be notified about it
 *
 *<code>
 *	$eventsManager->fire('db', $connection);
 *</code>
 *
 * @param string|Phalcon\Events\Event $event_type
 * @param object $source
 * @param mixed $data
 * @param int $cancelable
 * @param int $flag
 * @return mixed
 */
PHP_METHOD(Phalcon_Events_Manager, fire){

	zval *_event_type, *source, *data = NULL, *cancelable = NULL, *flag = NULL, debug_message = {};
	zval event_type = {}, events = {}, name = {}, type = {}, status = {}, collect = {}, any_type = {}, event = {}, fire_events = {};

	phalcon_fetch_params(1, 2, 3, &_event_type, &source, &data, &cancelable, &flag);

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_CONCAT_SV(&debug_message, "--Events Manager(fire): ", _event_type);
		PHALCON_DEBUG_LOG(&debug_message);
		zval_ptr_dtor(&debug_message);
	}

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!cancelable || Z_TYPE_P(cancelable) == IS_NULL) {
		cancelable = &PHALCON_GLOBAL(z_true);
	}

	if (!flag || Z_TYPE_P(flag) == IS_NULL) {
		flag = &PHALCON_GLOBAL(z_false);
	}

	if (unlikely(Z_TYPE_P(_event_type) != IS_STRING)) {
		if (Z_TYPE_P(_event_type) != IS_OBJECT) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_events_exception_ce, "Event type must be a string");
			return;
		} else {
			PHALCON_MM_VERIFY_INTERFACE_EX(_event_type, phalcon_events_eventinterface_ce, phalcon_events_exception_ce);
			PHALCON_MM_ZVAL_COPY(&event, _event_type);
			PHALCON_MM_CALL_METHOD(&name, &event, "getname");
			PHALCON_MM_ADD_ENTRY(&name);
			PHALCON_MM_CALL_METHOD(&type, &event, "gettype");
			PHALCON_MM_ADD_ENTRY(&type);
			PHALCON_CONCAT_VSV(&event_type, &name, ":", &type);
			PHALCON_MM_ADD_ENTRY(&event_type);
		}
	} else {
		PHALCON_MM_ZVAL_COPY(&event_type, _event_type);
		PHALCON_MM_CALL_METHOD(&event, getThis(), "createevent", &event_type, source, data, cancelable, flag);
		PHALCON_MM_ADD_ENTRY(&event);
		PHALCON_MM_CALL_METHOD(&name, &event, "getname");
		PHALCON_MM_ADD_ENTRY(&name);
	}

	phalcon_update_property(getThis(), SL("_currentEvent"), &event);

	phalcon_read_property(&events, getThis(), SL("_events"), PH_READONLY);
	if (Z_TYPE(events) != IS_ARRAY) {
		RETURN_MM_NULL();
	}

	ZVAL_NULL(&status);

	/**
	 * Should responses be traced?
	 */
	phalcon_read_property(&collect, getThis(), SL("_collect"), PH_READONLY);
	if (zend_is_true(&collect)) {
		phalcon_update_property_null(getThis(), SL("_responses"));
	}

	/**
	 * Check if there are listeners for the all event type
	 */
	PHALCON_MM_ZVAL_STRING(&any_type, "*");
	if (phalcon_array_isset_fetch(&fire_events, &events, &any_type, PH_READONLY)) {
		if (Z_TYPE(fire_events) == IS_ARRAY || Z_TYPE(fire_events) == IS_OBJECT) {
			/**
			 * Call the events queue
			 */
			PHALCON_MM_CALL_METHOD(&status, getThis(), "firequeue", &fire_events, &event);
			if (zend_is_true(flag) && PHALCON_IS_FALSE(&status)) {
				RETURN_MM_FALSE;
			}
		}
	}

	/**
	 * Check if events are grouped by name
	 */
	if (phalcon_array_isset_fetch(&fire_events, &events, &name, PH_READONLY)) {
		if (Z_TYPE(fire_events) == IS_ARRAY || Z_TYPE(fire_events) == IS_OBJECT) {
			/**
			 * Call the events queue
			 */
			PHALCON_MM_CALL_METHOD(&status, getThis(), "firequeue", &fire_events, &event);
			if (zend_is_true(flag) && PHALCON_IS_FALSE(&status)) {
				RETURN_MM_FALSE;
			}
		}
	}

	/**
	 * Check if there are listeners for the event type itself
	 */
	if (phalcon_array_isset_fetch(&fire_events, &events, &event_type, PH_READONLY)) {
		if (Z_TYPE(fire_events) == IS_ARRAY || Z_TYPE(fire_events) == IS_OBJECT) {
			/**
			 * Call the events queue
			 */
			PHALCON_MM_CALL_METHOD(&status, getThis(), "firequeue", &fire_events, &event);
			if (zend_is_true(flag) && PHALCON_IS_FALSE(&status)) {
				RETURN_MM_FALSE;
			}
		}
	}
	RETURN_MM_NCTOR(&status);
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

	phalcon_read_property(&events, getThis(), SL("_events"), PH_READONLY);
	if (phalcon_array_isset(&events, type)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Returns all the attached listeners of a certain type
 *
 * @param string $type
 * @param boolean $full
 * @return array
 */
PHP_METHOD(Phalcon_Events_Manager, getListeners){

	zval *type, *full = NULL, events = {}, queue = {}, iterator = {}, *listener;

	phalcon_fetch_params(0, 1, 1, &type, &full);

	if (!full) {
		full = &PHALCON_GLOBAL(z_false);
	}

	phalcon_read_property(&events, getThis(), SL("_events"), PH_READONLY);
	if (Z_TYPE(events) != IS_ARRAY) {
		RETURN_EMPTY_ARRAY();
	}

	if (!phalcon_array_isset(&events, type)) {
		RETURN_EMPTY_ARRAY();
	}

	array_init(return_value);

	phalcon_array_fetch(&queue, &events, type, PH_NOISY|PH_READONLY);

	if (zend_is_true(full)) {
		RETURN_CTOR(&queue);
	}

	if (Z_TYPE(queue) == IS_OBJECT) {
		if (phalcon_clone(&iterator, &queue) == FAILURE) {
			return;
		}

		PHALCON_CALL_METHOD(NULL, &iterator, "top");

		while (1) {
			zval r0 = {}, listener0 = {}, handler_embeded = {};

			PHALCON_CALL_METHOD(&r0, &iterator, "valid");
			if (!zend_is_true(&r0)) {
				break;
			}

			PHALCON_CALL_METHOD(&listener0, &iterator, "current");
			PHALCON_CALL_METHOD(&handler_embeded, &listener0, "getlistener");
			zval_ptr_dtor(&listener0);

			phalcon_array_append(return_value, &handler_embeded, 0);

			PHALCON_CALL_METHOD(NULL, &iterator, "next");
		}
		zval_ptr_dtor(&iterator);
	} else {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(queue), listener) {
			zval handler_embeded = {};
			PHALCON_CALL_METHOD(&handler_embeded, listener, "getlistener");
			phalcon_array_append(return_value, &handler_embeded, 0);
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

	phalcon_read_property(&events, getThis(), SL("_events"), PH_READONLY);
	phalcon_array_keys(return_value, &events);
}

/**
 * Gets current event
 *
 * @return Phalcon\Events\Event
 */
PHP_METHOD(Phalcon_Events_Manager, getCurrentEvent){

	RETURN_MEMBER(getThis(), "_currentEvent");
}
