
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

#include "validation/validator/regex.h"
#include "validation/validator.h"
#include "validation/validatorinterface.h"
#include "validation/message.h"
#include "validation/exception.h"
#include "validation.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/concat.h"

#include "interned-strings.h"

/**
 * Phalcon\Validation\Validator\Regex
 *
 * Allows validate if the value of a field matches a regular expression
 *
 *<code>
 *use Phalcon\Validation\Validator\Regex as RegexValidator;
 *
 *$validator->add('created_at', new RegexValidator(array(
 *   'pattern' => '/^[0-9]{4}[-\/](0[1-9]|1[12])[-\/](0[1-9]|[12][0-9]|3[01])$/',
 *   'message' => 'The creation date is invalid'
 *)));
 *</code>
 */
zend_class_entry *phalcon_validation_validator_regex_ce;

PHP_METHOD(Phalcon_Validation_Validator_Regex, validate);
PHP_METHOD(Phalcon_Validation_Validator_Regex, valid);

static const zend_function_entry phalcon_validation_validator_regex_method_entry[] = {
	PHP_ME(Phalcon_Validation_Validator_Regex, validate, arginfo_phalcon_validation_validatorinterface_validate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Validator_Regex, valid, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Validation\Validator\Regex initializer
 */
PHALCON_INIT_CLASS(Phalcon_Validation_Validator_Regex){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Validation\\Validator, Regex, validation_validator_regex, phalcon_validation_validator_ce, phalcon_validation_validator_regex_method_entry, 0);

	zend_class_implements(phalcon_validation_validator_regex_ce, 1, phalcon_validation_validatorinterface_ce);

	return SUCCESS;
}

/**
 * Executes the validation
 *
 * @param Phalcon\Validation $validator
 * @param string $attribute
 * @return boolean
 */
PHP_METHOD(Phalcon_Validation_Validator_Regex, validate){

	zval *validaton, *attribute, *_allow_empty = NULL, value = {}, allow_empty = {}, pattern = {}, valid = {}, label = {}, pairs = {}, message_str = {}, code = {}, prepared = {}, message = {};
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

	/* The regular expression is set in the option 'pattern' */
	RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&pattern, ce, getThis(), "pattern"));

	PHALCON_CALL_SELF(&valid, "valid", &value, &pattern);
	zval_ptr_dtor(&value);
	zval_ptr_dtor(&pattern);

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
			RETURN_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validaton), validaton, "Regex"));
		}

		RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&code, ce, getThis(), ISV(code)));
		if (Z_TYPE(code) == IS_NULL) {
			ZVAL_LONG(&code, 0);
		}

		PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);
		zval_ptr_dtor(&message_str);
		zval_ptr_dtor(&pairs);

		phalcon_validation_message_construct_helper(&message, &prepared, attribute, "Regex", &code);
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
PHP_METHOD(Phalcon_Validation_Validator_Regex, valid){

	zval *value, *pattern, matches = {}, match_pattern = {}, match_zero = {}, valid = {};

	phalcon_fetch_params(0, 2, 0, &value, &pattern);

	/* Check if the value match using preg_match in the PHP userland */
	ZVAL_NULL(&matches);
	RETURN_ON_FAILURE(phalcon_preg_match(&match_pattern, pattern, value, &matches, 0, 0));
	RETVAL_FALSE;

	if (zend_is_true(&match_pattern)) {
		phalcon_array_fetch_long(&match_zero, &matches, 0, PH_NOISY|PH_READONLY);

		is_not_equal_function(&valid, &match_zero, value);

		if (PHALCON_IS_FALSE(&valid)) {
			RETVAL_TRUE;
		}
	}
	zval_ptr_dtor(&matches);

}
