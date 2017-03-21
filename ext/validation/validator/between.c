
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

#include "validation/validator/between.h"
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
#include "kernel/array.h"

#include "interned-strings.h"

/**
 * Phalcon\Validation\Validator\Between
 *
 * Validates that a value is between a range of two values
 *
 *<code>
 *use Phalcon\Validation\Validator\Between;
 *
 *$validator->add('name', new Between(array(
 *   'minimum' => 0,
 *   'maximum' => 100,
 *   'message' => 'The price must be between 0 and 100'
 *)));
 *</code>
 */
zend_class_entry *phalcon_validation_validator_between_ce;

PHP_METHOD(Phalcon_Validation_Validator_Between, validate);
PHP_METHOD(Phalcon_Validation_Validator_Between, valid);

static const zend_function_entry phalcon_validation_validator_between_method_entry[] = {
	PHP_ME(Phalcon_Validation_Validator_Between, validate, arginfo_phalcon_validation_validatorinterface_validate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Validator_Between, valid, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Validation\Validator\Between initializer
 */
PHALCON_INIT_CLASS(Phalcon_Validation_Validator_Between){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Validation\\Validator, Between, validation_validator_between, phalcon_validation_validator_ce, phalcon_validation_validator_between_method_entry, 0);

	zend_class_implements(phalcon_validation_validator_between_ce, 1, phalcon_validation_validatorinterface_ce);

	return SUCCESS;
}

/**
 * Executes the validation
 *
 * @param Phalcon\Validation $validator
 * @param string $attribute
 * @return boolean
 */
PHP_METHOD(Phalcon_Validation_Validator_Between, validate){

	zval *validaton, *attribute, value = {}, allow_empty = {}, minimum = {}, maximum = {}, label = {}, pairs = {}, valid = {}, message_str = {}, code = {}, prepared = {}, message = {};
	zend_class_entry *ce = Z_OBJCE_P(getThis());

	phalcon_fetch_params(0, 2, 0, &validaton, &attribute);
	PHALCON_VERIFY_INTERFACE_EX(validaton, phalcon_validationinterface_ce, phalcon_validation_exception_ce);

	PHALCON_CALL_METHOD(&value, validaton, "getvalue", attribute);

	RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&allow_empty, ce, getThis(), ISV(allowEmpty)));
	if (zend_is_true(&allow_empty) && phalcon_validation_validator_isempty_helper(&value)) {
		RETURN_TRUE;
	}

	RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&minimum, ce, getThis(), "minimum"));
	RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&maximum, ce, getThis(), "maximum"));

	PHALCON_CALL_SELF(&valid, "valid", &value, &minimum, &maximum);

	if (PHALCON_IS_FALSE(&valid)) {
		RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&label, ce, getThis(), ISV(label)));
		if (!zend_is_true(&label)) {
			PHALCON_CALL_METHOD(&label, validaton, "getlabel", attribute);
		}

		array_init_size(&pairs, 3);
		phalcon_array_update_str(&pairs, SL(":field"), &label, PH_COPY);
		phalcon_array_update_str(&pairs, SL(":min"), &minimum, PH_COPY);
		phalcon_array_update_str(&pairs, SL(":max"), &maximum, PH_COPY);

		RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
		if (!zend_is_true(&message_str)) {
			RETURN_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validaton), validaton, "Between"));
		}

		RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&code, ce, getThis(), ISV(code)));
		if (Z_TYPE_P(&code) == IS_NULL) {
			ZVAL_LONG(&code, 0);
		}

		PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

		phalcon_validation_message_construct_helper(&message, &prepared, attribute, "Between", &code);

		PHALCON_CALL_METHOD(NULL, validaton, "appendmessage", &message);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

/**
 * Executes the validation
 *
 * @param int $value
 * @param int $minimum
 * @param int $maximum
 * @return boolean
 */
PHP_METHOD(Phalcon_Validation_Validator_Between, valid){

	zval *value, *minimum = NULL, *maximum = NULL, valid = {};

	phalcon_fetch_params(0, 3, 0, &value, &minimum, &maximum);

	is_smaller_or_equal_function(&valid, minimum, value);
	if (zend_is_true(&valid)) {
		is_smaller_or_equal_function(&valid, value, maximum);
	}

	if (PHALCON_IS_FALSE(&valid)) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
