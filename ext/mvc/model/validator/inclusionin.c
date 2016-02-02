
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

#include "mvc/model/validator/inclusionin.h"
#include "mvc/model/validator.h"
#include "mvc/model/validatorinterface.h"
#include "mvc/model/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/exception.h"
#include "kernel/operators.h"
#include "kernel/array.h"
#include "kernel/string.h"
#include "kernel/concat.h"

#include "interned-strings.h"

/**
 * Phalcon\Mvc\Model\Validator\InclusionIn
 *
 * Check if a value is included into a list of values
 *
 *<code>
 *	use Phalcon\Mvc\Model\Validator\InclusionIn as InclusionInValidator;
 *
 *	class Subscriptors extends Phalcon\Mvc\Model
 *	{
 *
 *		public function validation()
 *		{
 *			$this->validate(new InclusionInValidator(array(
 *				'field' => 'status',
 *				'domain' => array('A', 'I')
 *			)));
 *			if ($this->validationHasFailed() == true) {
 *				return false;
 *			}
 *		}
 *
 *	}
 *</code>
 */
zend_class_entry *phalcon_mvc_model_validator_inclusionin_ce;

PHP_METHOD(Phalcon_Mvc_Model_Validator_Inclusionin, validate);

static const zend_function_entry phalcon_mvc_model_validator_inclusionin_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Validator_Inclusionin, validate, arginfo_phalcon_mvc_model_validatorinterface_validate, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Validator\Inclusionin initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Validator_Inclusionin){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\Validator, Inclusionin, mvc_model_validator_inclusionin, phalcon_mvc_model_validator_ce, phalcon_mvc_model_validator_inclusionin_method_entry, 0);

	zend_class_implements(phalcon_mvc_model_validator_inclusionin_ce, 1, phalcon_mvc_model_validatorinterface_ce);

	return SUCCESS;
}

/**
 * Executes validator
 *
 * @param Phalcon\Mvc\ModelInterface $record
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Validator_Inclusionin, validate){

	zval *record, option, field, is_set, domain, value, allow_empty, message, joined_domain, is_set_code, code, type;

	phalcon_fetch_params(0, 1, 0, &record);

	ZVAL_STRING(&option, "field");

	PHALCON_CALL_METHODW(&field, getThis(), "getoption", &option);
	if (Z_TYPE(field) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Field name must be a string");
		return;
	}

	/** 
	 * The 'domain' option must be a valid array of not allowed values
	 */
	ZVAL_STRING(&option, "domain");

	PHALCON_CALL_METHODW(&is_set, getThis(), "issetoption", &option);
	if (PHALCON_IS_FALSE(&is_set)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "The option 'domain' is required for this validator");
		return;
	}

	ZVAL_STRING(&option, "domain");

	PHALCON_CALL_METHODW(&domain, getThis(), "getoption", &option);
	if (Z_TYPE(&domain) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Option 'domain' must be an array");
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

	/** 
	 * Check if the value is contained in the array
	 */
	if (!phalcon_fast_in_array(&value, &domain)) {
		/** 
		 * Check if the developer has defined a custom message
		 */
		ZVAL_STRING(&option, ISV(message));

		PHALCON_CALL_METHODW(&message, getThis(), "getoption", &option);
		if (!zend_is_true(&message)) {
			phalcon_fast_join_str(&joined_domain, SL(", "), &domain);

			PHALCON_CONCAT_SVSV(&message, "Value of field '", &field, "' must be part of list: ", &joined_domain);
		}

		ZVAL_STRING(&type, "Inclusion");

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
