
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

#include "validation/validator/exclusionin.h"
#include "validation/validator.h"
#include "validation/validatorinterface.h"
#include "validation/message.h"
#include "validation/exception.h"
#include "validation.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/exception.h"
#include "kernel/array.h"
#include "kernel/string.h"
#include "kernel/concat.h"
#include "kernel/operators.h"

#include "interned-strings.h"

/**
 * Phalcon\Validation\Validator\ExclusionIn
 *
 * Check if a value is not included into a list of values
 *
 *<code>
 *use Phalcon\Validation\Validator\ExclusionIn;
 *
 *$validator->add('status', new ExclusionIn(array(
 *   'message' => 'The status must not be A or B',
 *   'domain' => array('A', 'B')
 *)));
 *</code>
 */
zend_class_entry *phalcon_validation_validator_exclusionin_ce;

PHP_METHOD(Phalcon_Validation_Validator_ExclusionIn, validate);
PHP_METHOD(Phalcon_Validation_Validator_ExclusionIn, valid);

static const zend_function_entry phalcon_validation_validator_exclusionin_method_entry[] = {
	PHP_ME(Phalcon_Validation_Validator_ExclusionIn, validate, arginfo_phalcon_validation_validatorinterface_validate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Validator_ExclusionIn, valid, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Validation\Validator\ExclusionIn initializer
 */
PHALCON_INIT_CLASS(Phalcon_Validation_Validator_ExclusionIn){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Validation\\Validator, ExclusionIn, validation_validator_exclusionin, phalcon_validation_validator_ce, phalcon_validation_validator_exclusionin_method_entry, 0);

	zend_class_implements(phalcon_validation_validator_exclusionin_ce, 1, phalcon_validation_validatorinterface_ce);

	return SUCCESS;
}

/**
 * Executes the validation
 *
 * @param Phalcon\Validation $validator
 * @param string $attribute
 * @return boolean
 */
PHP_METHOD(Phalcon_Validation_Validator_ExclusionIn, validate){

	zval *validaton, *attribute, *_allow_empty = NULL, value = {}, allow_empty = {}, domain = {}, valid = {}, label = {}, joined_domain = {}, pairs = {}, message_str = {}, code = {}, prepared = {}, message = {};
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

	/* A domain is an array with a list of valid values */
	RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&domain, ce, getThis(), ISV(domain)));
	if (Z_TYPE(domain) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_validation_exception_ce, "Option 'domain' must be an array");
		zval_ptr_dtor(&value);
		return;
	}

	PHALCON_CALL_SELF(&valid, "valid", &value, &domain);
	zval_ptr_dtor(&value);

	if (PHALCON_IS_FALSE(&valid)) {
		RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&label, ce, getThis(), ISV(label)));
		if (!zend_is_true(&label)) {
			PHALCON_CALL_METHOD(&label, validaton, "getlabel", attribute);
		}

		phalcon_fast_join_str(&joined_domain, SL(", "), &domain);

		array_init_size(&pairs, 2);
		phalcon_array_update_str(&pairs, SL(":field"), &label, PH_COPY);
		phalcon_array_update_str(&pairs, SL(":domain"), &joined_domain, PH_COPY);
		zval_ptr_dtor(&label);
		zval_ptr_dtor(&joined_domain);

		RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
		if (!zend_is_true(&message_str)) {
			RETURN_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validaton), validaton, "ExclusionIn"));
		}

		RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&code, ce, getThis(), ISV(code)));
		if (Z_TYPE(code) <= IS_NULL) {
			ZVAL_LONG(&code, 0);
		}

		PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);
		zval_ptr_dtor(&message_str);
		zval_ptr_dtor(&pairs);

		phalcon_validation_message_construct_helper(&message, &prepared, attribute, "ExclusionIn", &code);
		zval_ptr_dtor(&prepared);

		PHALCON_CALL_METHOD(NULL, validaton, "appendmessage", &message);
		zval_ptr_dtor(&message);
		RETVAL_FALSE;
	} else {
		RETVAL_TRUE;
	}
	zval_ptr_dtor(&domain);
}

/**
 * Executes the validation
 *
 * @param string $value
 * @return boolean
 */
PHP_METHOD(Phalcon_Validation_Validator_ExclusionIn, valid){

	zval *value, *domain;

	phalcon_fetch_params(0, 2, 0, &value, &domain);

	/**
	 * Check if the value is contained by the array
	 */
	if (phalcon_fast_in_array(value, domain)) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
