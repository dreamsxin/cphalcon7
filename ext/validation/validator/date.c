
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

#include "validation/validator/date.h"
#include "validation/validator.h"
#include "validation/validatorinterface.h"
#include "validation/message.h"
#include "validation/exception.h"
#include "validation.h"
#include "../../date.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/array.h"
#include "kernel/string.h"

#include "interned-strings.h"

/**
 * Phalcon\Validation\Validator\Date
 *
 * Checks if a value has a correct DATE format
 *
 *<code>
 *use Phalcon\Validation\Validator\Date as DateValidator;
 *
 *$validator->add('date', new DateValidator(array(
 *   'message' => 'The date is not valid'
 *)));
 *</code>
 */
zend_class_entry *phalcon_validation_validator_date_ce;

PHP_METHOD(Phalcon_Validation_Validator_Date, validate);
PHP_METHOD(Phalcon_Validation_Validator_Date, valid);

static const zend_function_entry phalcon_validation_validator_date_method_entry[] = {
	PHP_ME(Phalcon_Validation_Validator_Date, validate, arginfo_phalcon_validation_validatorinterface_validate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Validator_Date, valid, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Validation\Validator\Date initializer
 */
PHALCON_INIT_CLASS(Phalcon_Validation_Validator_Date){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Validation\\Validator, Date, validation_validator_date, phalcon_validation_validator_ce, phalcon_validation_validator_date_method_entry, 0);

	zend_class_implements(phalcon_validation_validator_date_ce, 1, phalcon_validation_validatorinterface_ce);

	return SUCCESS;
}

/**
 * Executes the validation
 *
 * @param Phalcon\Validation $validator
 * @param string $attribute
 * @return boolean
 */
PHP_METHOD(Phalcon_Validation_Validator_Date, validate){

	zval *validaton, *attribute, *_allow_empty = NULL, value = {}, allow_empty = {}, format = {}, valid = {}, label = {}, pairs = {}, message_str = {}, code = {}, prepared = {}, message = {};
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
	if (PHALCON_IS_EMPTY_STRING(&value)) {
		if (zend_is_true(&allow_empty)) {
			zval_ptr_dtor(&allow_empty);
			RETURN_TRUE;
		}
	}
	zval_ptr_dtor(&allow_empty);

	RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&format, ce, getThis(), "format"));

	PHALCON_CALL_SELF(&valid, "valid", &value, &format);
	zval_ptr_dtor(&value);
	zval_ptr_dtor(&format);

	if (PHALCON_IS_FALSE(&valid)) {
		RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&label, ce, getThis(), ISV(label)));
		if (!zend_is_true(&label)) {
			PHALCON_CALL_METHOD(&label, validaton, "getlabel", attribute);
		}

		array_init_size(&pairs, 1);
		phalcon_array_update_str(&pairs, SL(":field"), &label, PH_COPY);
		zval_ptr_dtor(&label);

		RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
		if (!zend_is_true(&message_str)) {
			RETURN_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validaton), validaton, "Date"));
		}

		RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&code, ce, getThis(), ISV(code)));
		if (Z_TYPE(code) == IS_NULL) {
			ZVAL_LONG(&code, 0);
		}

		PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);
		zval_ptr_dtor(&message_str);
		zval_ptr_dtor(&pairs);

		phalcon_validation_message_construct_helper(&message, &prepared, attribute, "Date", &code);
		zval_ptr_dtor(&prepared);

		PHALCON_CALL_METHOD(NULL, validaton, "appendmessage", &message);
		zval_ptr_dtor(&message);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

/**
 * Executes the validation
 *
 * @param string $value
 * @return boolean
 */
PHP_METHOD(Phalcon_Validation_Validator_Date, valid){

	zval *value, *format = NULL, valid = {};

	phalcon_fetch_params(0, 1, 1, &value, &format);

	if (!format) {
		format = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(value) != IS_STRING || Z_STRLEN_P(value) <= 0) {
		RETURN_FALSE;
	}

	PHALCON_CALL_CE_STATIC(&valid, phalcon_date_ce, "valid", value, format);
	if (!zend_is_true(&valid)) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
