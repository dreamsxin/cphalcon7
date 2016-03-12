
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

#include "mvc/model/validator/json.h"
#include "mvc/model/validator.h"
#include "mvc/model/validatorinterface.h"
#include "mvc/model/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/exception.h"
#include "kernel/string.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/concat.h"

#include "interned-strings.h"

/**
 * Phalcon\Mvc\Model\Validator\Json
 *
 * Allows to validate if json fields has correct values
 *
 *<code>
 *	use Phalcon\Mvc\Model\Validator\Json as JsonValidator;
 *
 *	class Subscriptors extends Phalcon\Mvc\Model
 *	{
 *
 *		public function validation()
 *		{
 *			$this->validate(new JsonValidator(array(
 *				'field' => 'json_data'
 *      	)));
 *      	if ($this->validationHasFailed() == true) {
 *				return false;
 *      	}
 *  	}
 *
 *	}
 *</code>
 *
 */
zend_class_entry *phalcon_mvc_model_validator_json_ce;

PHP_METHOD(Phalcon_Mvc_Model_Validator_Json, validate);

static const zend_function_entry phalcon_mvc_model_validator_json_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Validator_Json, validate, arginfo_phalcon_mvc_model_validatorinterface_validate, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Validator\Json initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Validator_Json){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\Validator, Json, mvc_model_validator_json, phalcon_mvc_model_validator_ce, phalcon_mvc_model_validator_json_method_entry, 0);

	zend_class_implements(phalcon_mvc_model_validator_json_ce, 1, phalcon_mvc_model_validatorinterface_ce);

	return SUCCESS;
}

/**
 * Executes the validator
 *
 * @param Phalcon\Mvc\ModelInterface $record
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Validator_Json, validate){

	zval *record, option  = {}, field_name = {}, invalid = {}, value = {}, keys = {}, assoc = {}, json = {}, *constant, ret = {}, message = {}, type = {}, is_set_code = {}, code = {};

	phalcon_fetch_params(0, 1, 0, &record);

	ZVAL_STRING(&option, "field");

	PHALCON_CALL_METHODW(&field_name, getThis(), "getoption", &option);
	if (Z_TYPE(field_name) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Field name must be a string");
		return;
	}

	ZVAL_FALSE(&invalid);

	PHALCON_CALL_METHODW(&value, record, "readattribute", &field_name);

	ZVAL_TRUE(&assoc);

	PHALCON_CALL_FUNCTIONW(&json, "json_decode", &value, &assoc);

	if (Z_TYPE(json) == IS_NULL) {
		if ((constant = zend_get_constant_str(SL("JSON_ERROR_NONE"))) != NULL) {
			PHALCON_CALL_FUNCTIONW(&ret, "json_last_error");

			if (!PHALCON_IS_EQUAL(&ret, constant)) {
				ZVAL_TRUE(&invalid);
			}
		}
	}

	if (!PHALCON_IS_TRUE(&invalid)) {
		ZVAL_STRING(&option, "keys");

		PHALCON_CALL_METHODW(&keys, getThis(), "getoption", &option);

		if (Z_TYPE(keys) != IS_NULL) {
			PHALCON_CALL_FUNCTIONW(&ret, "array_key_exists", &keys, &json);
			if (!zend_is_true(&ret)) {
				ZVAL_TRUE(&invalid);
			}
		}
	}

	if (PHALCON_IS_TRUE(&invalid)) {
		/** 
		 * Check if the developer has defined a custom message
		 */
		ZVAL_STRING(&option, ISV(message));

		PHALCON_CALL_METHODW(&message, getThis(), "getoption", &option);
		if (!zend_is_true(&message)) {
			PHALCON_CONCAT_SVS(&message, "Value of field '", &field_name, "' must have a valid json format");
		}

		ZVAL_STRING(&type, "Json");

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

		PHALCON_CALL_METHODW(NULL, getThis(), "appendmessage", &message, &field_name, &type, &code);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
