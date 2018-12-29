
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

#include "validation/message/group.h"
#include "validation/exception.h"
#include "validation/message.h"
#include "validation/messageinterface.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/hash.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"

/**
 * Phalcon\Validation\Message\Group
 *
 * Represents a group of validation messages
 */
zend_class_entry *phalcon_validation_message_group_ce;

PHP_METHOD(Phalcon_Validation_Message_Group, __construct);
PHP_METHOD(Phalcon_Validation_Message_Group, offsetGet);
PHP_METHOD(Phalcon_Validation_Message_Group, offsetSet);
PHP_METHOD(Phalcon_Validation_Message_Group, offsetExists);
PHP_METHOD(Phalcon_Validation_Message_Group, offsetUnset);
PHP_METHOD(Phalcon_Validation_Message_Group, appendMessage);
PHP_METHOD(Phalcon_Validation_Message_Group, appendMessages);
PHP_METHOD(Phalcon_Validation_Message_Group, filter);
PHP_METHOD(Phalcon_Validation_Message_Group, count);
PHP_METHOD(Phalcon_Validation_Message_Group, rewind);
PHP_METHOD(Phalcon_Validation_Message_Group, current);
PHP_METHOD(Phalcon_Validation_Message_Group, key);
PHP_METHOD(Phalcon_Validation_Message_Group, next);
PHP_METHOD(Phalcon_Validation_Message_Group, valid);
PHP_METHOD(Phalcon_Validation_Message_Group, __set_state);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_message_group___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, messages)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_message_group_offsetget, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_message_group_offsetset, 0, 0, 2)
	ZEND_ARG_INFO(0, index)
	ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_message_group_offsetexists, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_message_group_offsetunset, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_message_group_appendmessage, 0, 0, 1)
	ZEND_ARG_INFO(0, message)
	ZEND_ARG_INFO(0, field)
	ZEND_ARG_INFO(0, type)
	ZEND_ARG_INFO(0, code)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_message_group_appendmessages, 0, 0, 1)
	ZEND_ARG_INFO(0, messages)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_message_group_filter, 0, 0, 1)
	ZEND_ARG_INFO(0, fieldName)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_message_group___set_state, 0, 0, 1)
	ZEND_ARG_INFO(0, group)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_validation_message_group_method_entry[] = {
	PHP_ME(Phalcon_Validation_Message_Group, __construct, arginfo_phalcon_validation_message_group___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Validation_Message_Group, offsetGet, arginfo_phalcon_validation_message_group_offsetget, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message_Group, offsetSet, arginfo_phalcon_validation_message_group_offsetset, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message_Group, offsetExists, arginfo_phalcon_validation_message_group_offsetexists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message_Group, offsetUnset, arginfo_phalcon_validation_message_group_offsetunset, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message_Group, appendMessage, arginfo_phalcon_validation_message_group_appendmessage, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message_Group, appendMessages, arginfo_phalcon_validation_message_group_appendmessages, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message_Group, filter, arginfo_phalcon_validation_message_group_filter, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message_Group, count, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message_Group, rewind, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message_Group, current, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message_Group, key, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message_Group, next, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message_Group, valid, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message_Group, __set_state, arginfo_phalcon_validation_message_group___set_state, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Validation\Message\Group initializer
 */
PHALCON_INIT_CLASS(Phalcon_Validation_Message_Group){

	PHALCON_REGISTER_CLASS(Phalcon\\Validation\\Message, Group, validation_message_group, phalcon_validation_message_group_method_entry, 0);

	zend_declare_property_null(phalcon_validation_message_group_ce, SL("_position"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_validation_message_group_ce, SL("_messages"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_validation_message_group_ce, 3, spl_ce_Countable, zend_ce_arrayaccess, zend_ce_iterator);

	return SUCCESS;
}

/**
 * Phalcon\Validation\Message\Group constructor
 *
 * @param array $messages
 */
PHP_METHOD(Phalcon_Validation_Message_Group, __construct){

	zval *messages = NULL;

	phalcon_fetch_params(0, 0, 1, &messages);
	if (messages && Z_TYPE_P(messages) == IS_ARRAY) {
		phalcon_update_property(getThis(), SL("_messages"), messages);
	} else {
		phalcon_update_property_empty_array(getThis(), SL("_messages"));
	}
}

/**
 * Gets an attribute a message using the array syntax
 *
 *<code>
 * print_r($messages[0]);
 *</code>
 *
 * @param string $index
 * @return Phalcon\Validation\Message
 */
PHP_METHOD(Phalcon_Validation_Message_Group, offsetGet){

	zval *index;

	phalcon_fetch_params(0, 1, 0, &index);
	phalcon_read_property_array(return_value, getThis(), SL("_messages"), index, PH_COPY);
}

/**
 * Sets an attribute using the array-syntax
 *
 *<code>
 * $messages[0] = new Phalcon\Validation\Message('This is a message');
 *</code>
 *
 * @param string $index
 * @param Phalcon\Validation\Message $message
 */
PHP_METHOD(Phalcon_Validation_Message_Group, offsetSet){

	zval *index, *message;

	phalcon_fetch_params(0, 2, 0, &index, &message);
	phalcon_update_property_array(getThis(), SL("_messages"), index, message);
}

/**
 * Checks if an index exists
 *
 *<code>
 * var_dump(isset($message['database']));
 *</code>
 *
 * @param string $index
 * @return boolean
 */
PHP_METHOD(Phalcon_Validation_Message_Group, offsetExists){

	zval *index;

	phalcon_fetch_params(0, 1, 0, &index);

	if (phalcon_isset_property_array(getThis(), SL("_messages"), index)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Removes a message from the list
 *
 *<code>
 * unset($message['database']);
 *</code>
 *
 * @param string $index
 */
PHP_METHOD(Phalcon_Validation_Message_Group, offsetUnset){

	zval *index;
	phalcon_fetch_params(0, 1, 0, &index);
	phalcon_unset_property_array(getThis(), SL("_messages"), index);
}

/**
 * Appends a message to the group
 *
 *<code>
 * $messages->appendMessage(new Phalcon\Validation\Message('This is a message'));
 *</code>
 *
 * @param Phalcon\Validation\Message $message
 */
PHP_METHOD(Phalcon_Validation_Message_Group, appendMessage){

	zval *message, *field = NULL, *type = NULL, *code = NULL;

	phalcon_fetch_params(1, 1, 3, &message, &field, &type, &code);

	if (!field) {
		field = &PHALCON_GLOBAL(z_null);
	}

	if (!type) {
		type = &PHALCON_GLOBAL(z_null);
	}

	if (!code) {
		code = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(message) == IS_OBJECT) {
		PHALCON_MM_VERIFY_INTERFACE_EX(message, phalcon_validation_messageinterface_ce, phalcon_validation_exception_ce);
		phalcon_update_property_array_append(getThis(), SL("_messages"), message);
	} else {
		zval new_message = {};
		object_init_ex(&new_message, phalcon_validation_message_ce);
		PHALCON_MM_CALL_METHOD(NULL, &new_message, "__construct", message, field, type, code);
		PHALCON_MM_ADD_ENTRY(&new_message);
		phalcon_update_property_array_append(getThis(), SL("_errorMessages"), &new_message);
	}
	RETURN_MM_THIS();
}

/**
 * Appends an array of messages to the group
 *
 *<code>
 * $messages->appendMessages($messagesArray);
 *</code>
 *
 * @param Phalcon\Validation\MessageInterface[] $messages
 */
PHP_METHOD(Phalcon_Validation_Message_Group, appendMessages){

	zval *messages, *message;

	phalcon_fetch_params(0, 1, 0, &messages);

	if (Z_TYPE_P(messages) != IS_ARRAY) {
		if (Z_TYPE_P(messages) != IS_OBJECT) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_validation_exception_ce, "The messages must be array or object");
			return;
		}
	}

	if (Z_TYPE_P(messages) == IS_ARRAY) {
		/**
		 * An array of messages is simply merged into the current one
		 */
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(messages), message) {
			PHALCON_CALL_SELF(NULL, "appendmessage", message);
		} ZEND_HASH_FOREACH_END();
	} else {
		PHALCON_VERIFY_INTERFACE_EX(messages, zend_ce_iterator, phalcon_validation_exception_ce);
		zend_class_entry *ce     = Z_OBJCE_P(messages);
		zend_object_iterator *it = ce->get_iterator(ce, messages, 0);

		assert(it != NULL);
		assert(it->funcs->rewind != NULL);
		assert(it->funcs->valid != NULL);
		assert(it->funcs->get_current_data != NULL);
		assert(it->funcs->move_forward != NULL);

		/**
		 * A group of messages is iterated and appended one-by-one to the current list
		 */
		it->funcs->rewind(it);
		while (!EG(exception) && SUCCESS == it->funcs->valid(it)) {
			zval *message;
			zval *params[1];

			message = it->funcs->get_current_data(it);
			if (!EG(exception)) {
				params[0] = message;
				if (FAILURE == phalcon_call_method(NULL, getThis(), "appendmessage", 1, params)) {
					break;
				}
			}

			if (!EG(exception)) {
				it->funcs->move_forward(it);
			}
		}

		it->funcs->dtor(it);
		//efree(it);
	}
}

/**
 * Filters the message group by field name
 *
 * @param string $fieldName
 * @return array
 */
PHP_METHOD(Phalcon_Validation_Message_Group, filter){

	zval *field_name, filtered = {}, messages = {}, *message;

	phalcon_fetch_params(0, 1, 0, &field_name);

	array_init(&filtered);

	phalcon_read_property(&messages, getThis(), SL("_messages"), PH_READONLY);
	if (Z_TYPE(messages) == IS_ARRAY) {
		/**
		 * A group of messages is iterated and appended one-by-one to the current list
		 */
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(messages), message) {
			zval field = {};
			/**
			 * Get the field name
			 */
			if (phalcon_method_exists_ex(message, SL("getfield")) == SUCCESS) {
				PHALCON_CALL_METHOD(&field, message, "getfield");
				if (PHALCON_IS_EQUAL(field_name, &field)) {
					phalcon_array_append(&filtered, message, PH_COPY);
				}
				zval_ptr_dtor(&field);
			}
		} ZEND_HASH_FOREACH_END();

	}

	RETVAL_ZVAL(&filtered, 0, 0);
}

/**
 * Returns the number of messages in the list
 *
 * @return int
 */
PHP_METHOD(Phalcon_Validation_Message_Group, count){

	zval messages = {};

	phalcon_read_property(&messages, getThis(), SL("_messages"), PH_NOISY|PH_READONLY);

	phalcon_fast_count(return_value, &messages);
}

/**
 * Rewinds the internal iterator
 */
PHP_METHOD(Phalcon_Validation_Message_Group, rewind){

	zval messages = {};

	phalcon_read_property(&messages, getThis(), SL("_messages"), PH_NOISY|PH_READONLY);
	zend_hash_internal_pointer_reset(Z_ARRVAL(messages));
}

/**
 * Returns the current message in the iterator
 *
 * @return Phalcon\Validation\Message
 */
PHP_METHOD(Phalcon_Validation_Message_Group, current){

	zval messages = {}, *message;

	phalcon_read_property(&messages, getThis(), SL("_messages"), PH_NOISY|PH_READONLY);
	if ((message = zend_hash_get_current_data(Z_ARRVAL(messages))) != NULL) {
		RETURN_CTOR(message);
	}

	RETURN_FALSE;
}

/**
 * Returns the current position/key in the iterator
 *
 * @return int
 */
PHP_METHOD(Phalcon_Validation_Message_Group, key){

	zval messages = {};

	phalcon_read_property(&messages, getThis(), SL("_messages"), PH_NOISY|PH_READONLY);
	zend_hash_get_current_key_zval(Z_ARRVAL(messages), return_value);
}

/**
 * Moves the internal iteration pointer to the next position
 *
 */
PHP_METHOD(Phalcon_Validation_Message_Group, next){

	zval messages = {}, *message;

	phalcon_read_property(&messages, getThis(), SL("_messages"), PH_NOISY|PH_READONLY);
	zend_hash_move_forward(Z_ARRVAL(messages));

	if ((message = zend_hash_get_current_data(Z_ARRVAL(messages))) != NULL) {
		RETURN_CTOR(message);
	}

	RETURN_FALSE;
}

/**
 * Check if the current message in the iterator is valid
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Validation_Message_Group, valid){

	zval messages = {};

	phalcon_read_property(&messages, getThis(), SL("_messages"), PH_NOISY|PH_READONLY);

	RETURN_BOOL(zend_hash_has_more_elements(Z_ARRVAL(messages)) == SUCCESS);
}

/**
 * Magic __set_state helps to re-build messages variable when exporting
 *
 * @param array $group
 * @return Phalcon\Mvc\Model\Message\Group
 */
PHP_METHOD(Phalcon_Validation_Message_Group, __set_state){

	zval *group, messages = {};

	phalcon_fetch_params(0, 1, 0, &group);

	if (phalcon_array_isset_fetch_str(&messages, group, SL("_messages"), PH_READONLY)) {
		object_init_ex(return_value, phalcon_validation_message_group_ce);
		PHALCON_CALL_METHOD(NULL, return_value, "__construct", &messages);
	} else {
		zend_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Invalid arguments passed to %s", "Phalcon\\Mvc\\Model\\Message\\Group::__set_state()");
	}
}
