
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
  |          Jason Rice                                                    |
  +------------------------------------------------------------------------+
*/

#include "mvc/model/validator/stringlength.h"
#include "mvc/model/validator.h"
#include "mvc/model/validatorinterface.h"
#include "mvc/model/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/exception.h"
#include "kernel/string.h"
#include "kernel/operators.h"
#include "kernel/concat.h"

#include "interned-strings.h"

/**
 * Phalcon\Mvc\Model\Validator\StringLength
 *
 * Simply validates specified string length constraints
 *
 *<code>
 *use Phalcon\Mvc\Model\Validator\StringLength as StringLengthValidator;
 *
 *class Subscriptors extends Phalcon\Mvc\Model
 *{
 *
 *	public function validation()
 *	{
 *		$this->validate(new StringLengthValidator(array(
 *			'field' => 'name_last',
 *			'max' => 50,
 *			'min' => 2,
 *			'messageMaximum' => 'We don\'t like really long names',
 *			'messageMinimum' => 'We want more than just their initials'
 *		)));
 *		if ($this->validationHasFailed() == true) {
 *			return false;
 *		}
 *	}
 *
 *}
 *</code>
 *
 */
zend_class_entry *phalcon_mvc_model_validator_stringlength_ce;

PHP_METHOD(Phalcon_Mvc_Model_Validator_StringLength, validate);

static const zend_function_entry phalcon_mvc_model_validator_stringlength_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Validator_StringLength, validate, arginfo_phalcon_mvc_model_validatorinterface_validate, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Validator\StringLength initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Validator_StringLength){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\Validator, StringLength, mvc_model_validator_stringlength, phalcon_mvc_model_validator_ce, phalcon_mvc_model_validator_stringlength_method_entry, 0);

	zend_class_implements(phalcon_mvc_model_validator_stringlength_ce, 1, phalcon_mvc_model_validatorinterface_ce);

	return SUCCESS;
}

/**
 * Executes the validator
 *
 * @param Phalcon\Mvc\ModelInterface $record
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Validator_StringLength, validate){

	zval *record, option = {}, field = {}, allow_empty = {}, is_set_min = {}, is_set_max = {}, value = {}, length = {}, invalid_maximum = {}, invalid_minimum = {}, maximum = {}, message = {}, type = {}, minimum = {}, is_set_code = {}, code = {};

	phalcon_fetch_params(0, 1, 0, &record);

	ZVAL_STRING(&option, "field");

	PHALCON_CALL_METHODW(&field, getThis(), "getoption", &option);
	if (Z_TYPE(field) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Field name must be a string");
		return;
	}

	/** 
	 * At least one of 'min' or 'max' must be set
	 */
	ZVAL_STRING(&option, "min");
	PHALCON_CALL_METHODW(&is_set_min, getThis(), "issetoption", &option);

	ZVAL_STRING(&option, "max");
	PHALCON_CALL_METHODW(&is_set_max, getThis(), "issetoption", &option);
	if (!zend_is_true(&is_set_min)) {
		if (!zend_is_true(&is_set_max)) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "A minimum or maximum must be set");
			return;
		}
	}

	PHALCON_CALL_METHODW(&value, record, "readattribute", &field);

	/*
	 * Allow empty
	 */
	ZVAL_STRING(&option, "allowEmpty");

	PHALCON_CALL_METHODW(&allow_empty, getThis(), "getoption", &option);

	if (zend_is_true(&allow_empty) && PHALCON_IS_EMPTY(&value)) {
		RETURN_TRUE;
	}

	/** 
	 * Check if mbstring is available to calculate the correct length
	 */
	if (phalcon_function_exists_ex(SL("mb_strlen")) == SUCCESS) {
		PHALCON_CALL_FUNCTIONW(&length, "mb_strlen", &value);
	} else {
		phalcon_fast_strlen(&length, &value);
	}

	ZVAL_FALSE(&invalid_maximum);
	ZVAL_FALSE(&invalid_minimum);

	/** 
	 * Maximum length
	 */
	if (zend_is_true(&is_set_max)) {
		ZVAL_STRING(&option, "max");

		PHALCON_CALL_METHODW(&maximum, getThis(), "getoption", &option);

		is_smaller_function(&invalid_maximum, &maximum, &length);
		if (PHALCON_IS_TRUE(&invalid_maximum)) {
			/** 
			 * Check if the developer has defined a custom message
			 */
			ZVAL_STRING(&option, "messageMaximum");

			PHALCON_CALL_METHODW(&message, getThis(), "getoption", &option);
			if (!zend_is_true(&message)) {
				PHALCON_CONCAT_SVSVS(&message, "Value of field '", &field, "' exceeds the maximum ", &maximum, " characters");
			}

			ZVAL_STRING(&type, "TooLong");

			/*
			 * Is code set
			 */
			ZVAL_STRING(&option, ISV(code));

			PHALCON_CALL_METHODW(&is_set_code, getThis(), "issetoption", &option);
			if (zend_is_true(&is_set_code)) {
				PHALCON_CALL_METHODW(&code, getThis(), "getoption", &option);
			} else {
				ZVAL_LONG(&code, 0);
			}

			PHALCON_CALL_METHODW(NULL, getThis(), "appendmessage", &message, &field, &type, &code);
			RETURN_FALSE;
		}
	}

	/** 
	 * Minimum length
	 */
	if (zend_is_true(&is_set_min)) {
		ZVAL_STRING(&option, "min");
		PHALCON_CALL_METHODW(&minimum, getThis(), "getoption", &option);

		is_smaller_function(&invalid_minimum, &length, &minimum);
		if (PHALCON_IS_TRUE(&invalid_minimum)) {
			/** 
			 * Check if the developer has defined a custom message
			 */
			ZVAL_STRING(&option, "messageMinimum");

			PHALCON_CALL_METHODW(&message, getThis(), "getoption", &option);
			if (!zend_is_true(&message)) {
				PHALCON_CONCAT_SVSVS(&message, "Value of field '", &field, "' is less than the minimum ", &minimum, " characters");
			}

			ZVAL_STRING(&type, "TooShort");

			/*
			 * Is code set
			 */
			ZVAL_STRING(&option, ISV(code));

			PHALCON_CALL_METHODW(&is_set_code, getThis(), "issetoption", &option);
			if (zend_is_true(&is_set_code)) {
				PHALCON_CALL_METHODW(&code, getThis(), "getoption", &option);
			} else {
				ZVAL_LONG(&code, 0);
			}

			PHALCON_CALL_METHODW(NULL, getThis(), "appendmessage", &message, &field, &type, &code);
			RETURN_FALSE;
		}
	}

	RETURN_TRUE;
}
