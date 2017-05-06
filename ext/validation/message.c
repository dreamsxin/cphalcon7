
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

#include "validation/message.h"
#include "validation/messageinterface.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/fcall.h"

/**
 * Phalcon\Validation\Message
 *
 * Encapsulates validation info generated in the validation process
 */
zend_class_entry *phalcon_validation_message_ce;

PHP_METHOD(Phalcon_Validation_Message, __construct);
PHP_METHOD(Phalcon_Validation_Message, setType);
PHP_METHOD(Phalcon_Validation_Message, getType);
PHP_METHOD(Phalcon_Validation_Message, setCode);
PHP_METHOD(Phalcon_Validation_Message, getCode);
PHP_METHOD(Phalcon_Validation_Message, setMessage);
PHP_METHOD(Phalcon_Validation_Message, getMessage);
PHP_METHOD(Phalcon_Validation_Message, setField);
PHP_METHOD(Phalcon_Validation_Message, getField);
PHP_METHOD(Phalcon_Validation_Message, __toString);
PHP_METHOD(Phalcon_Validation_Message, __set_state);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_message___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, message)
	ZEND_ARG_INFO(0, field)
	ZEND_ARG_INFO(0, type)
	ZEND_ARG_INFO(0, code)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_message___set_state, 0, 0, 1)
	ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_validation_message_method_entry[] = {
	PHP_ME(Phalcon_Validation_Message, __construct, arginfo_phalcon_validation_message___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Validation_Message, setType, arginfo_phalcon_validation_messageinterface_settype, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message, getType, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message, setCode, arginfo_phalcon_validation_messageinterface_setcode, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message, getCode, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message, setMessage, arginfo_phalcon_validation_messageinterface_setmessage, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message, getMessage, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message, setField, arginfo_phalcon_validation_messageinterface_setfield, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message, getField, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message, __toString, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Message, __set_state, arginfo_phalcon_validation_message___set_state, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Validation\Message initializer
 */
PHALCON_INIT_CLASS(Phalcon_Validation_Message){

	PHALCON_REGISTER_CLASS(Phalcon\\Validation, Message, validation_message, phalcon_validation_message_method_entry, 0);

	zend_declare_property_null(phalcon_validation_message_ce, SL("_type"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_validation_message_ce, SL("_message"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_validation_message_ce, SL("_field"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_validation_message_ce, SL("_code"), 0, ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_validation_message_ce, 1, phalcon_validation_messageinterface_ce);

	return SUCCESS;
}

void phalcon_validation_message_construct_helper(zval *result, zval *message, zval *field, const char *type, zval *code)
{
	zval *params[4], tmp = {};

	object_init_ex(result, phalcon_validation_message_ce);

	ZVAL_STRING(&tmp, type);

	params[0] = message;
	params[1] = field;
	params[2] = &tmp;
	params[3] = code;

	if (FAILURE == phalcon_call_method(NULL, result, "__construct", 4, params)) {
		zval_ptr_dtor(result);
		ZVAL_NULL(result);
	}
	zval_ptr_dtor(&tmp);
}

/**
 * Phalcon\Validation\Message constructor
 *
 * @param string $message
 * @param string $field
 * @param string $type
 * @param int    $code
 */
PHP_METHOD(Phalcon_Validation_Message, __construct){

	zval *message, *field = NULL, *type = NULL, *code = NULL;

	phalcon_fetch_params(0, 1, 3, &message, &field, &type, &code);

	if (!field) {
		field = &PHALCON_GLOBAL(z_null);
	}

	if (!type) {
		type = &PHALCON_GLOBAL(z_null);
	}

	if (!code) {
		code = &PHALCON_GLOBAL(z_zero);
	}

	phalcon_update_property(getThis(), SL("_message"), message);
	phalcon_update_property(getThis(), SL("_field"), field);
	phalcon_update_property(getThis(), SL("_type"), type);
	phalcon_update_property(getThis(), SL("_code"), code);
}

/**
 * Sets message type
 *
 * @param string $type
 * @return Phalcon\Validation\Message
 */
PHP_METHOD(Phalcon_Validation_Message, setType){

	zval *type;

	phalcon_fetch_params(0, 1, 0, &type);

	phalcon_update_property(getThis(), SL("_type"), type);
	RETURN_THIS();
}

/**
 * Returns message type
 *
 * @return string
 */
PHP_METHOD(Phalcon_Validation_Message, getType){


	RETURN_MEMBER(getThis(), "_type");
}

/**
 * Sets message code
 *
 * @param string $code
 * @return Phalcon\Validation\Message
 */
PHP_METHOD(Phalcon_Validation_Message, setCode){

	zval *code;

	phalcon_fetch_params(0, 1, 0, &code);

	phalcon_update_property(getThis(), SL("_code"), code);
	RETURN_THIS();
}

/**
 * Returns message code
 *
 * @return string
 */
PHP_METHOD(Phalcon_Validation_Message, getCode){

	RETURN_MEMBER(getThis(), "_code");
}

/**
 * Sets verbose message
 *
 * @param string $message
 * @return Phalcon\Validation\Message
 */
PHP_METHOD(Phalcon_Validation_Message, setMessage){

	zval *message;

	phalcon_fetch_params(0, 1, 0, &message);

	phalcon_update_property(getThis(), SL("_message"), message);
	RETURN_THIS();
}

/**
 * Returns verbose message
 *
 * @return string
 */
PHP_METHOD(Phalcon_Validation_Message, getMessage){


	RETURN_MEMBER(getThis(), "_message");
}

/**
 * Sets field name related to message
 *
 * @param string $field
 * @return Phalcon\Validation\Message
 */
PHP_METHOD(Phalcon_Validation_Message, setField){

	zval *field;

	phalcon_fetch_params(0, 1, 0, &field);

	phalcon_update_property(getThis(), SL("_field"), field);
	RETURN_THIS();
}

/**
 * Returns field name related to message
 *
 * @return string
 */
PHP_METHOD(Phalcon_Validation_Message, getField){


	RETURN_MEMBER(getThis(), "_field");
}

/**
 * Magic __toString method returns verbose message
 *
 * @return string
 */
PHP_METHOD(Phalcon_Validation_Message, __toString){


	RETURN_MEMBER(getThis(), "_message");
}

/**
 * Magic __set_state helps to recover messsages from serialization
 *
 * @param array $message
 * @return Phalcon\Validation\Message
 */
PHP_METHOD(Phalcon_Validation_Message, __set_state){

	zval *message, message_text = {}, field = {}, type = {}, code = {};

	phalcon_fetch_params(0, 1, 0, &message);

	phalcon_array_fetch_str(&message_text, message, SL("_message"), PH_NOISY|PH_READONLY);
	phalcon_array_fetch_str(&field, message, SL("_field"), PH_NOISY|PH_READONLY);
	phalcon_array_fetch_str(&type, message, SL("_type"), PH_NOISY|PH_READONLY);
	phalcon_array_fetch_str(&code, message, SL("_code"), PH_NOISY|PH_READONLY);

	object_init_ex(return_value, phalcon_validation_message_ce);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", &message_text, &field, &type, &code);
}
