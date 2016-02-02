
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

#include "mvc/model/validator/url.h"
#include "mvc/model/validator.h"
#include "mvc/model/validatorinterface.h"
#include "mvc/model/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/exception.h"
#include "kernel/concat.h"
#include "kernel/operators.h"

#include "interned-strings.h"

/**
 * Phalcon\Mvc\Model\Validator\Url
 *
 * Allows to validate if a field has a url format
 *
 *<code>
 *use Phalcon\Mvc\Model\Validator\Url as UrlValidator;
 *
 *class Posts extends Phalcon\Mvc\Model
 *{
 *
 *  public function validation()
 *  {
 *      $this->validate(new UrlValidator(array(
 *          'field' => 'source_url'
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
zend_class_entry *phalcon_mvc_model_validator_url_ce;

PHP_METHOD(Phalcon_Mvc_Model_Validator_Url, validate);

static const zend_function_entry phalcon_mvc_model_validator_url_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Validator_Url, validate, arginfo_phalcon_mvc_model_validatorinterface_validate, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Validator\Url initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Validator_Url){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\Validator, Url, mvc_model_validator_url, phalcon_mvc_model_validator_ce, phalcon_mvc_model_validator_url_method_entry, 0);

	zend_class_implements(phalcon_mvc_model_validator_url_ce, 1, phalcon_mvc_model_validatorinterface_ce);

	return SUCCESS;
}

/**
 * Executes the validator
 *
 * @param Phalcon\Mvc\ModelInterface $record
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Validator_Url, validate){

	zval *record, option, field, allow_empty, value, flag, is_valid, message, type, is_set_code, code;

	phalcon_fetch_params(0, 1, 0, &record);

	ZVAL_STRING(&option, "field");

	PHALCON_CALL_METHODW(&field, getThis(), "getoption", &option);
	if (Z_TYPE(field) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Field name must be a string");
		return;
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

	ZVAL_LONG(&flag, 273);

	/** 
	 * Filters the format using FILTER_VALIDATE_URL
	 */
	PHALCON_CALL_FUNCTIONW(&is_valid, "filter_var", &value, &flag);
	if (!zend_is_true(&is_valid)) {
		/** 
		 * Check if the developer has defined a custom message
		 */
		ZVAL_STRING(&option, ISV(message));

		PHALCON_CALL_METHODW(&message, getThis(), "getoption", &option);
		if (!zend_is_true(&message)) {
			PHALCON_CONCAT_SVS(&message, "'", &field, "' does not have a valid url format");
		}

		ZVAL_STRING(&type, "Url");

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

	RETURN_TRUE;
}
