
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

#include "mvc/model/validator/regex.h"
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
 * Phalcon\Mvc\Model\Validator\Regex
 *
 * Allows validate if the value of a field matches a regular expression
 *
 *<code>
 *use Phalcon\Mvc\Model\Validator\Regex as RegexValidator;
 *
 *class Subscriptors extends Phalcon\Mvc\Model
 *{
 *
 *  public function validation()
 *  {
 *      $this->validate(new RegexValidator(array(
 *          'field' => 'created_at',
 *          'pattern' => '/^[0-9]{4}[-\/](0[1-9]|1[12])[-\/](0[1-9]|[12][0-9]|3[01])$/'
 *      )));
 *      if ($this->validationHasFailed() == true) {
 *          return false;
 *      }
 *  }
 *
 *}
 *</code>
 *
 */
zend_class_entry *phalcon_mvc_model_validator_regex_ce;

PHP_METHOD(Phalcon_Mvc_Model_Validator_Regex, validate);

static const zend_function_entry phalcon_mvc_model_validator_regex_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Validator_Regex, validate, arginfo_phalcon_mvc_model_validatorinterface_validate, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Validator\Regex initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Validator_Regex){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\Validator, Regex, mvc_model_validator_regex, phalcon_mvc_model_validator_ce, phalcon_mvc_model_validator_regex_method_entry, 0);

	zend_class_implements(phalcon_mvc_model_validator_regex_ce, 1, phalcon_mvc_model_validatorinterface_ce);

	return SUCCESS;
}

/**
 * Executes the validator
 *
 * @param Phalcon\Mvc\ModelInterface $record
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Validator_Regex, validate){

	zval *record, option = {}, field_name = {}, allow_empty = {}, is_set = {}, value = {}, failed = {}, matches = {}, pattern = {}, match_pattern = {}, match_zero = {}, message = {}, type = {}, is_set_code = {}, code = {};

	phalcon_fetch_params(0, 1, 0, &record);

	ZVAL_STRING(&option, "field");

	PHALCON_CALL_METHODW(&field_name, getThis(), "getoption", &option);
	if (Z_TYPE(field_name) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Field name must be a string");
		return;
	}

	/** 
	 * The 'pattern' option must be a valid regular expression
	 */
	ZVAL_STRING(&option, "pattern");

	PHALCON_CALL_METHODW(&is_set, getThis(), "issetoption", &option);
	if (!zend_is_true(&is_set)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Validator requires a perl-compatible regex pattern");
		return;
	}

	PHALCON_CALL_METHODW(&value, record, "readattribute", &field_name);

	/*
	 * Allow empty
	 */
	ZVAL_STRING(&option, "allowEmpty");

	PHALCON_CALL_METHODW(&allow_empty, getThis(), "getoption", &option);
	if (zend_is_true(&allow_empty) && PHALCON_IS_EMPTY(&value)) {
		RETURN_TRUE;
	}

	ZVAL_FALSE(&failed);

	/** 
	 * The regular expression is set in the option 'pattern'
	 */
	ZVAL_STRING(&option, "pattern");

	PHALCON_CALL_METHODW(&pattern, getThis(), "getoption", &option);

	/** 
	 * Check if the value matches using preg_match
	 */
	RETURN_ON_FAILURE(phalcon_preg_match(&match_pattern, &pattern, &value, &matches));

	if (zend_is_true(&match_pattern)) {
		phalcon_array_fetch_long(&match_zero, &matches, 0, PH_NOISY);

		is_not_equal_function(&failed, &match_zero, &value);
	} else {
		ZVAL_TRUE(&failed);
	}

	if (PHALCON_IS_TRUE(&failed)) {
		/** 
		 * Check if the developer has defined a custom message
		 */
		ZVAL_STRING(&option, ISV(message));

		PHALCON_CALL_METHODW(&message, getThis(), "getoption", &option);
		if (!zend_is_true(&message)) {
			PHALCON_CONCAT_SVS(&message, "Value of field '", &field_name, "' doesn't match regular expression");
		}

		ZVAL_STRING(&type, "Regex");

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
