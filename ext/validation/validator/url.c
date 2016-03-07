
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

#include "validation/validator/between.h"
#include "validation/validator.h"
#include "validation/validatorinterface.h"
#include "validation/message.h"
#include "validation/exception.h"
#include "validation.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/array.h"

#include "interned-strings.h"

/**
 * Phalcon\Validation\Validator\Url
 *
 * Checks if a value has a correct URL format
 *
 *<code>
 *use Phalcon\Validation\Validator\Url as UrlValidator;
 *
 *$validator->add('url', new UrlValidator(array(
 *   'message' => 'The url is not valid'
 *)));
 *</code>
 */
zend_class_entry *phalcon_validation_validator_url_ce;

PHP_METHOD(Phalcon_Validation_Validator_Url, validate);
PHP_METHOD(Phalcon_Validation_Validator_Url, valid);

static const zend_function_entry phalcon_validation_validator_url_method_entry[] = {
	PHP_ME(Phalcon_Validation_Validator_Url, validate, arginfo_phalcon_validation_validatorinterface_validate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Validator_Url, valid, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Validation\Validator\Url initializer
 */
PHALCON_INIT_CLASS(Phalcon_Validation_Validator_Url){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Validation\\Validator, Url, validation_validator_url, phalcon_validation_validator_ce, phalcon_validation_validator_url_method_entry, 0);

	zend_class_implements(phalcon_validation_validator_url_ce, 1, phalcon_validation_validatorinterface_ce);

	return SUCCESS;
}

/**
 * Executes the validation
 *
 * @param Phalcon\Validation $validator
 * @param string $attribute
 * @return boolean
 */
PHP_METHOD(Phalcon_Validation_Validator_Url, validate){

	zval *validator, *attribute, value, allow_empty, valid, label, pairs, message_str, code, prepared, message;
	zend_class_entry *ce = Z_OBJCE_P(getThis());

	phalcon_fetch_params(0, 2, 0, &validator, &attribute);

	PHALCON_VERIFY_CLASS_EX(validator, phalcon_validation_ce, phalcon_validation_exception_ce, 1);

	PHALCON_CALL_METHODW(&value, validator, "getvalue", attribute);

	RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&allow_empty, ce, getThis(), ISV(allowEmpty)));
	if (zend_is_true(&allow_empty) && phalcon_validation_validator_isempty_helper(&value)) {
		RETURN_TRUE;
	}

	PHALCON_CALL_SELFW(&valid, "valid", &value);

	if (PHALCON_IS_FALSE(&valid)) {
		RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&label, ce, getThis(), ISV(label)));
		if (!zend_is_true(&label)) {
			PHALCON_CALL_METHODW(&label, validator, "getlabel", attribute);
			if (!zend_is_true(&label)) {
				ZVAL_COPY_VALUE(&label, attribute);
			}
		}

		array_init_size(&pairs, 1);
		phalcon_array_update_str(&pairs, SL(":field"), &label, PH_COPY);

		RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
		if (!zend_is_true(&message_str)) {
			RETURN_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validator), validator, "Url"));
		}

		RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&code, ce, getThis(), ISV(code)));
		if (Z_TYPE_P(&code) == IS_NULL) {
			ZVAL_LONG(&code, 0);
		}

		PHALCON_CALL_FUNCTIONW(&prepared, "strtr", &message_str, &pairs);

		phalcon_validation_message_construct_helper(&message, &prepared, attribute, "Url", &code);

		PHALCON_CALL_METHODW(NULL, validator, "appendmessage", &message);
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
PHP_METHOD(Phalcon_Validation_Validator_Url, valid){

	zval *value, validate_url, valid;

	phalcon_fetch_params(0, 1, 0, &value);

	ZVAL_LONG(&validate_url, 273);

	PHALCON_CALL_FUNCTIONW(&valid, "filter_var", value, &validate_url);
	if (!zend_is_true(&valid)) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
