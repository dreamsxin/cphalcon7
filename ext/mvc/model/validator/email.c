
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

#include "mvc/model/validator/email.h"
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
 * Phalcon\Mvc\Model\Validator\Email
 *
 * Allows to validate if email fields has correct values
 *
 *<code>
 *	use Phalcon\Mvc\Model\Validator\Email as EmailValidator;
 *
 *	class Subscriptors extends Phalcon\Mvc\Model
 *	{
 *
 *		public function validation()
 *		{
 *			$this->validate(new EmailValidator(array(
 *				'field' => 'electronic_mail'
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
zend_class_entry *phalcon_mvc_model_validator_email_ce;

PHP_METHOD(Phalcon_Mvc_Model_Validator_Email, validate);

static const zend_function_entry phalcon_mvc_model_validator_email_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Validator_Email, validate, arginfo_phalcon_mvc_model_validatorinterface_validate, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Validator\Email initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Validator_Email){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\Validator, Email, mvc_model_validator_email, phalcon_mvc_model_validator_ce, phalcon_mvc_model_validator_email_method_entry, 0);

	zend_class_implements(phalcon_mvc_model_validator_email_ce, 1, phalcon_mvc_model_validatorinterface_ce);

	return SUCCESS;
}

/**
 * Executes the validator
 *
 * @param Phalcon\Mvc\ModelInterface $record
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Validator_Email, validate){

	zval *record, option, field_name, allow_empty, regs, invalid, value, pattern, match_pattern, match_zero, message, type, is_set_code, code;

	phalcon_fetch_params(0, 1, 0, &record);

	ZVAL_STRING(&option, "field");
	
	PHALCON_CALL_METHODW(&field_name, getThis(), "getoption", &option);
	if (Z_TYPE(field_name) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Field name must be a string");
		return;
	}

	ZVAL_FALSE(&invalid);
	
	PHALCON_CALL_METHOD(&value, record, "readattribute", &field_name);

	/*
	 * Allow empty
	 */
	ZVAL_STRING(&option, "allowEmpty");

	PHALCON_CALL_METHODW(&allow_empty, getThis(), "getoption", &option);
	if (zend_is_true(&allow_empty) && PHALCON_IS_EMPTY(&value)) {
		RETURN_TRUE;
	}
	
	/*
	 * Allow empty
	 */
	ZVAL_STRING(&option, "allowEmpty");

	PHALCON_CALL_METHODW(&allow_empty, getThis(), "getoption", &option);

	if (zend_is_true(&allow_empty)) {
		if (PHALCON_IS_EMPTY(&value)) {
			RETURN_TRUE;
		}
	}
	
	/** 
	 * We check if the email has a valid format using a regular expression
	 */
	ZVAL_STRING(&pattern, "/^([^\\x00-\\x20\\x22\\x28\\x29\\x2c\\x2e\\x3a-\\x3c\\x3e\\x40\\x5b-\\x5d\\x7f-\\xff]+|\\x22([^\\x0d\\x22\\x5c\\x80-\\xff]|\\x5c[\\x00-\\x7f])*\\x22)(\\x2e([^\\x00-\\x20\\x22\\x28\\x29\\x2c\\x2e\\x3a-\\x3c\\x3e\\x40\\x5b-\\x5d\\x7f-\\xff]+|\\x22([^\\x0d\\x22\\x5c\\x80-\\xff]|\\x5c[\\x00-\\x7f])*\\x22))*\\x40([^\\x00-\\x20\\x22\\x28\\x29\\x2c\\x2e\\x3a-\\x3c\\x3e\\x40\\x5b-\\x5d\\x7f-\\xff]+|\\x5b([^\\x0d\\x5b-\\x5d\\x80-\\xff]|\\x5c[\\x00-\\x7f])*\\x5d)(\\x2e([^\\x00-\\x20\\x22\\x28\\x29\\x2c\\x2e\\x3a-\\x3c\\x3e\\x40\\x5b-\\x5d\\x7f-\\xff]+|\\x5b([^\\x0d\\x5b-\\x5d\\x80-\\xff]|\\x5c[\\x00-\\x7f])*\\x5d))*$/");

	RETURN_ON_FAILURE(phalcon_preg_match(&match_pattern, &pattern, &value, &regs));
	
	if (zend_is_true(&match_pattern)) {
		phalcon_array_fetch_long(&match_zero, &regs, 0, PH_NOISY);
	
		is_not_equal_function(&invalid, &match_zero, &value);
	} else {
		ZVAL_TRUE(&invalid);
	}
	
	if (PHALCON_IS_TRUE(&invalid)) {
		/** 
		 * Check if the developer has defined a custom message
		 */
		ZVAL_STRING(&option, ISV(message));
	
		PHALCON_CALL_METHODW(&message, getThis(), "getoption", &option);
		if (!zend_is_true(&message)) {
			PHALCON_CONCAT_SVS(&message, "Value of field '", &field_name, "' must have a valid e-mail format");
		}

		ZVAL_STRING(&type, "Email");

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

