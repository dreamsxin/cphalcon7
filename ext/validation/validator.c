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

#include "validation/validator.h"
#include "validation/validatorinterface.h"
#include "validation/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/fcall.h"

#include "interned-strings.h"

/**
 * Phalcon\Validation\Validator
 *
 * This is a base class for validators
 */
zend_class_entry *phalcon_validation_validator_ce;

PHP_METHOD(Phalcon_Validation_Validator, __construct);
PHP_METHOD(Phalcon_Validation_Validator, isSetOption);
PHP_METHOD(Phalcon_Validation_Validator, getOption);
PHP_METHOD(Phalcon_Validation_Validator, setOption);
PHP_METHOD(Phalcon_Validation_Validator, getType);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_validator___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_validation_validator_method_entry[] = {
	PHP_ME(Phalcon_Validation_Validator, __construct, arginfo_phalcon_validation_validator___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Validation_Validator, isSetOption, arginfo_phalcon_validation_validatorinterface_issetoption, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Validator, getOption, arginfo_phalcon_validation_validatorinterface_getoption, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Validator, setOption, arginfo_phalcon_validation_validatorinterface_setoption, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Validator, getType, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Validation\Validator initializer
 */
PHALCON_INIT_CLASS(Phalcon_Validation_Validator){

	PHALCON_REGISTER_CLASS(Phalcon\\Validation, Validator, validation_validator, phalcon_validation_validator_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_validation_validator_ce, SL("_options"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_validation_validator_ce, SL("_type"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_validation_validator_ce, 1, phalcon_validation_validatorinterface_ce);

	return SUCCESS;
}

int phalcon_validation_validator_getoption_helper(zval *retval, const zend_class_entry *ce, zval *this_ptr, const char *option)
{
	zval opt = {}, options = {};
	zval *params[1];
	int ret;

	if (is_phalcon_class(ce)) {
		phalcon_read_property(&options, this_ptr, SL("_options"), PH_NOISY|PH_READONLY);

		if (!phalcon_array_isset_fetch_str(retval, &options, option, strlen(option), PH_COPY)) {
			ZVAL_NULL(retval);
		}

		return SUCCESS;
	}

	ZVAL_STRING(&opt, option);
	params[0] = &opt;

	ret = phalcon_call_method(retval, this_ptr, "getoption", 1, params);

	zval_ptr_dtor(&opt);
	return ret;
}

/**
 * Phalcon\Validation\Validator constructor
 *
 * @param array $options
 */
PHP_METHOD(Phalcon_Validation_Validator, __construct){

	zval *options = NULL;

	phalcon_fetch_params(0, 0, 1, &options);

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(options) != IS_ARRAY) {
		if (Z_TYPE_P(options) != IS_NULL) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_validation_exception_ce, "Options must be an array");
			return;
		}
	} else {
		phalcon_update_property(getThis(), SL("_options"), options);
	}
}

/**
 * Checks if an option is defined
 *
 * @param string $key
 * @return mixed
 */
PHP_METHOD(Phalcon_Validation_Validator, isSetOption){

	zval *key, options = {};

	phalcon_fetch_params(0, 1, 0, &key);

	phalcon_read_property(&options, getThis(), SL("_options"), PH_NOISY|PH_READONLY);
	RETURN_BOOL(phalcon_array_isset(&options, key));
}

/**
 * Returns an option in the validator's options
 * Returns null if the option hasn't been set
 *
 * @param string $key
 * @return mixed
 */
PHP_METHOD(Phalcon_Validation_Validator, getOption){

	zval *key;

	phalcon_fetch_params(0, 1, 0, &key);
	PHALCON_ENSURE_IS_STRING(key);
	phalcon_validation_validator_getoption_helper(return_value, phalcon_validation_validator_ce, getThis(), Z_STRVAL_P(key));
}

/**
 * Sets an option in the validator
 *
 * @param string $key
 * @param mixed $value
 */
PHP_METHOD(Phalcon_Validation_Validator, setOption){

	zval *key, *value;

	phalcon_fetch_params(0, 2, 0, &key, &value);

	phalcon_update_property_array(getThis(), SL("_options"), key, value);

}

/**
 * Gets an type in the validator
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Validation_Validator, getType){


	RETURN_MEMBER(getThis(), "_type");
}
