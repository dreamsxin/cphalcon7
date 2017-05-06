
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

#include "validation/validator/stringlength.h"
#include "validation/validator.h"
#include "validation/validatorinterface.h"
#include "validation/message.h"
#include "validation/exception.h"
#include "validation.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"
#include "kernel/concat.h"
#include "kernel/string.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/array.h"

#include "interned-strings.h"

/**
 * Phalcon\Validation\Validator\StringLength
 *
 * Validates that a string has the specified maximum and minimum constraints
 *
 *<code>
 *use Phalcon\Validation\Validator\StringLength as StringLength;
 *
 *$validation->add('name_last', new StringLength(array(
 *      'max' => 50,
 *      'min' => 2,
 *      'messageMaximum' => 'We don\'t like really long names',
 *      'messageMinimum' => 'We want more than just their initials'
 *)));
 *</code>
 *
 */
zend_class_entry *phalcon_validation_validator_stringlength_ce;

PHP_METHOD(Phalcon_Validation_Validator_StringLength, validate);
PHP_METHOD(Phalcon_Validation_Validator_StringLength, valid);

static const zend_function_entry phalcon_validation_validator_stringlength_method_entry[] = {
	PHP_ME(Phalcon_Validation_Validator_StringLength, validate, arginfo_phalcon_validation_validatorinterface_validate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Validator_StringLength, valid, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Validation\Validator\StringLength initializer
 */
PHALCON_INIT_CLASS(Phalcon_Validation_Validator_StringLength){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Validation\\Validator, StringLength, validation_validator_stringlength, phalcon_validation_validator_ce, phalcon_validation_validator_stringlength_method_entry, 0);

	zend_class_implements(phalcon_validation_validator_stringlength_ce, 1, phalcon_validation_validatorinterface_ce);

	return SUCCESS;
}

/**
 * Executes the validation
 *
 * @param Phalcon\Validation $validator
 * @param string $attribute
 * @return boolean
 */
PHP_METHOD(Phalcon_Validation_Validator_StringLength, validate)
{
	zval *validaton, *attribute, *_allow_empty = NULL, value = {}, allow_empty = {}, valid = {}, type = {}, maximum = {}, minimum = {}, label = {};
	zval code = {}, pairs = {}, message_str = {}, prepared = {}, message = {};
	zend_class_entry *ce = Z_OBJCE_P(getThis());

	phalcon_fetch_params(0, 2, 1, &validaton, &attribute, &_allow_empty);
	PHALCON_VERIFY_INTERFACE_EX(validaton, phalcon_validationinterface_ce, phalcon_validation_exception_ce);

	PHALCON_CALL_METHOD(&value, validaton, "getvalue", attribute);

	RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&allow_empty, ce, getThis(), ISV(allowEmpty)));
	if (Z_TYPE(allow_empty) == IS_NULL) {
		if (_allow_empty && zend_is_true(_allow_empty)) {
			ZVAL_COPY(&allow_empty, _allow_empty);
		}
	}
	if (zend_is_true(&allow_empty) && PHALCON_IS_EMPTY_STRING(&value)) {
		zval_ptr_dtor(&allow_empty);
		zval_ptr_dtor(&value);
		RETURN_TRUE;
	}
	zval_ptr_dtor(&allow_empty);

	RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&maximum, ce, getThis(), "max"));
	RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&minimum, ce, getThis(), "min"));

	PHALCON_CALL_SELF(&valid, "valid", &value, &minimum, &maximum);

	if (PHALCON_IS_FALSE(&valid)) {
		phalcon_read_property(&type, getThis(), SL("_type"), PH_READONLY);

		RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&label, ce, getThis(), ISV(label)));
		if (!zend_is_true(&label)) {
			PHALCON_CALL_METHOD(&label, validaton, "getlabel", attribute);
		}

		RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&code, ce, getThis(), ISV(code)));
		if (Z_TYPE(code) <= IS_NULL) {
			ZVAL_LONG(&code, 0);
		}

		array_init(&pairs);
		phalcon_array_update_str(&pairs, SL(":field"), &label, PH_COPY);
		zval_ptr_dtor(&label);

		if (phalcon_compare_strict_string(&type, SL("TooLong"))) {
			phalcon_array_update_str(&pairs, SL(":max"), &maximum, PH_COPY);

			RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), "messageMaximum"));
			if (!zend_is_true(&message_str)) {
				RETURN_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validaton), validaton, "TooLong"));
			}

			PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

			phalcon_validation_message_construct_helper(&message, &prepared, attribute, "TooLong", &code);
		} else {
			phalcon_array_update_str(&pairs, SL(":min"), &minimum, PH_COPY);

			RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), "messageMinimum"));
			if (!zend_is_true(&message_str)) {
				RETURN_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validaton), validaton, "TooShort"));
			}

			PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

			phalcon_validation_message_construct_helper(&message, &prepared, attribute, "TooShort", &code);
		}
		zval_ptr_dtor(&message_str);
		zval_ptr_dtor(&prepared);
		zval_ptr_dtor(&pairs);

		PHALCON_CALL_METHOD(NULL, validaton, "appendmessage", &message);
		zval_ptr_dtor(&message);
		RETVAL_FALSE;
	} else {
		RETVAL_TRUE;
	}
	zval_ptr_dtor(&minimum);
	zval_ptr_dtor(&maximum);
}

/**
 * Executes the validation
 *
 * @param string $value
 * @param int $minimum
 * @param int $maximum
 * @return boolean
 */
PHP_METHOD(Phalcon_Validation_Validator_StringLength, valid){

	zval *value, *minimum = NULL, *maximum = NULL, length = {}, valid = {};

	phalcon_fetch_params(0, 1, 2, &value, &minimum, &maximum);

	if (!minimum) {
		minimum = &PHALCON_GLOBAL(z_null);
	}

	if (!maximum) {
		maximum = &PHALCON_GLOBAL(z_null);
	}

	/* At least one of 'min' or 'max' must be set */
	if (Z_TYPE_P(minimum) == IS_NULL && Z_TYPE_P(maximum) == IS_NULL) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_validation_exception_ce, "A minimum or maximum must be set");
		return;
	}

#ifdef PHALCON_USE_PHP_MBSTRING
	phalcon_strlen(&length, &value);
#else
	/* Check if mbstring is available to calculate the correct length */
	if (phalcon_function_exists_ex(SL("mb_strlen")) == SUCCESS) {
		PHALCON_CALL_FUNCTION(&length, "mb_strlen", value);
	} else {
		phalcon_fast_strlen(&length, value);
	}
#endif

	/* Maximum length */
	if (Z_TYPE_P(maximum) != IS_NULL) {
		is_smaller_function(&valid, maximum, &length);
		if (PHALCON_IS_TRUE(&valid)) {
			phalcon_update_property_str(getThis(), SL("_type"), SL("TooLong"));
			RETURN_FALSE;
		}
	}

	/* Minimum length */
	if (Z_TYPE_P(minimum) != IS_NULL) {
		is_smaller_function(&valid, &length, minimum);
		if (PHALCON_IS_TRUE(&valid)) {
			phalcon_update_property_str(getThis(), SL("_type"), SL("TooShort"));
			RETURN_FALSE;
		}
	}

	RETURN_TRUE;
}
