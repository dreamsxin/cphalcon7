
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

#include "validation/validator/uniqueness.h"
#include "validation/validator.h"
#include "validation/validatorinterface.h"
#include "validation/message.h"
#include "validation/exception.h"
#include "validation.h"
#include "mvc/modelinterface.h"
#include "mvc/model.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/array.h"
#include "kernel/exception.h"
#include "kernel/string.h"

#include "interned-strings.h"

/**
 * Phalcon\Validation\Validator\Uniqueness
 *
 * Check that a field is unique in the related table
 *
 *<code>
 *use Phalcon\Validation\Validator\Uniqueness as UniquenessValidator;
 *
 *$validator->add('name', new UniquenessValidator(array(
 *   'model' => new Users(),
 *   'message' => ':field must be unique'
 *)));
 *
 *$validator->add(array('user_id', 'created'), new UniquenessValidator(array(
 *   'model' => new Users(),
 *   'message' => ':field must be unique'
 *)));
 *</code>
 */
zend_class_entry *phalcon_validation_validator_uniqueness_ce;

PHP_METHOD(Phalcon_Validation_Validator_Uniqueness, validate);
PHP_METHOD(Phalcon_Validation_Validator_Uniqueness, valid);

static const zend_function_entry phalcon_validation_validator_uniqueness_method_entry[] = {
	PHP_ME(Phalcon_Validation_Validator_Uniqueness, validate, arginfo_phalcon_validation_validatorinterface_validate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Validator_Uniqueness, valid, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Validation\Validator\Uniqueness initializer
 */
PHALCON_INIT_CLASS(Phalcon_Validation_Validator_Uniqueness){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Validation\\Validator, Uniqueness, validation_validator_uniqueness, phalcon_validation_validator_ce, phalcon_validation_validator_uniqueness_method_entry, 0);

	zend_class_implements(phalcon_validation_validator_uniqueness_ce, 1, phalcon_validation_validatorinterface_ce);

	return SUCCESS;
}

/**
 * Executes the validation
 *
 * @param Phalcon\Validation $validator
 * @param string $attribute
 * @return boolean
 */
PHP_METHOD(Phalcon_Validation_Validator_Uniqueness, validate){

	zval *validaton, *attribute, record = {}, meta_data = {}, except = {}, operation_made = {}, excepts = {}, column_map = {}, primary_fields = {};
	zval *primary_field, values = {}, *field = NULL, value = {}, valid = {}, label = {}, pairs = {}, message_str = {}, code = {}, prepared = {};
	zval message = {}, exception_message = {};
	zend_class_entry *ce = Z_OBJCE_P(getThis());

	phalcon_fetch_params(0, 2, 0, &validaton, &attribute);
	PHALCON_VERIFY_INTERFACE_EX(validaton, phalcon_validationinterface_ce, phalcon_validation_exception_ce);

	RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&record, ce, getThis(), ISV(model)));

	if (Z_TYPE(record) != IS_OBJECT) {
		PHALCON_CALL_METHOD(&record, validaton, "getentity");
	}
	PHALCON_VERIFY_INTERFACE_EX(&record, phalcon_mvc_modelinterface_ce, phalcon_validation_exception_ce);

	PHALCON_CALL_METHOD(&meta_data, &record, "getmodelsmetadata");

	RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&except, ce, getThis(), ISV(except)));

	array_init(&excepts);

	if (Z_TYPE(except) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(except), field) {
			zval field_value = {};

			PHALCON_CALL_METHOD(&field_value, &record, "readattribute", field);

			phalcon_array_update_zval(&excepts, field, &field_value, PH_COPY);
		} ZEND_HASH_FOREACH_END();

	} else if (PHALCON_IS_NOT_EMPTY(&except)) {
		PHALCON_CALL_METHOD(&value, &record, "readattribute", &except);
		phalcon_array_update_zval(&excepts, &except, &value, PH_COPY);
	}

	PHALCON_CALL_METHOD(&operation_made, &record, "getoperationmade");

	if (PHALCON_IS_LONG(&operation_made, PHALCON_MODEL_OP_UPDATE)) {
		/**
		 * We build a query with the primary key attributes
		 */
		PHALCON_CALL_METHOD(&column_map, &meta_data, "getcolumnmap", &record);
		PHALCON_CALL_METHOD(&primary_fields, &meta_data, "getprimarykeyattributes", &record);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(primary_fields), primary_field) {
			zval attribute_field = {}, attribute_value = {};

			/**
			 * Rename the column if there is a column map
			 */
			if (Z_TYPE(column_map) == IS_ARRAY) {
				if (!phalcon_array_isset_fetch(&attribute_field, &column_map, primary_field, 0)) {
					PHALCON_CONCAT_SVS(&exception_message, "Column '", primary_field, "\" isn't part of the column map");
					PHALCON_THROW_EXCEPTION_ZVAL(phalcon_validation_exception_ce, &exception_message);
					return;
				}
			} else {
				PHALCON_CPY_WRT(&attribute_field, primary_field);
			}

			/**
			 * Create a condition based on the renamed primary key
			 */
			PHALCON_CALL_METHOD(&attribute_value, &record, "readattribute", primary_field);

			phalcon_array_update_zval(&excepts, &attribute_field, &attribute_value, PH_COPY);
		} ZEND_HASH_FOREACH_END();
	}

	array_init(&values);
	if (Z_TYPE_P(attribute) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(attribute), field) {
			zval field_value = {};

			PHALCON_CALL_METHOD(&field_value, &record, "readattribute", field);

			phalcon_array_update_zval(&values, field, &field_value, PH_COPY);
		} ZEND_HASH_FOREACH_END();

	} else {
		PHALCON_CALL_METHOD(&value, &record, "readattribute", attribute);
		phalcon_array_update_zval(&values, attribute, &value, PH_COPY);
	}

	PHALCON_CALL_SELF(&valid, "valid", &record, &values, &excepts);

	if (PHALCON_IS_FALSE(&valid)) {
		RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&label, ce, getThis(), ISV(label)));
		if (!zend_is_true(&label)) {
			PHALCON_CALL_METHOD(&label, validaton, "getlabel", attribute);
		}

		array_init_size(&pairs, 1);
		phalcon_array_update_str(&pairs, SL(":field"), &label, PH_COPY);

		RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
		if (!zend_is_true(&message_str)) {
			RETURN_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validaton), validaton, "Uniqueness"));
		}

		RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&code, ce, getThis(), ISV(code)));
		if (Z_TYPE_P(&code) == IS_NULL) {
			ZVAL_LONG(&code, 0);
		}

		PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

		phalcon_validation_message_construct_helper(&message, &prepared, attribute, "Uniqueness", &code);

		PHALCON_CALL_METHOD(NULL, validaton, "appendmessage", &message);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

/**
 * Executes the validation
 *
 * @param Phalcon\Mvc\ModelInterface $record
 * @param array $values
 * @return boolean
 */
PHP_METHOD(Phalcon_Validation_Validator_Uniqueness, valid){

	zval *record, *values, *excepts = NULL, conditions = {}, bind_params = {}, except_conditions = {}, join_except_conditions = {};
	zval tmp_condition = {}, number = {}, *value = NULL, join_conditions = {}, params = {};
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 2, 1, &record, &values, &excepts);

	PHALCON_VERIFY_INTERFACE_EX(record, phalcon_mvc_modelinterface_ce, phalcon_validation_exception_ce);

	if (Z_TYPE_P(values) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_validation_exception_ce, "Values must be array");
		return;
	}

	/**
	 * PostgreSQL check if the compared constant has the same type as the column, so we
	 * make cast to the data passed to match those column types
	 */
	array_init(&conditions);
	array_init(&bind_params);

	ZVAL_LONG(&number, 0);

	if (excepts && Z_TYPE_P(excepts) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(excepts))) {
		array_init(&except_conditions);
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(excepts), idx, str_key, value) {
			zval field = {}, condition = {};

			if (str_key) {
				ZVAL_STR(&field, str_key);
			} else {
				ZVAL_LONG(&field, idx);
			}

			PHALCON_CONCAT_SVSV(&condition, "[", &field, "] <> ?", &number);
			phalcon_array_append(&except_conditions, &condition, PH_COPY);
			phalcon_array_append(&bind_params, value, PH_COPY);

			phalcon_increment(&number);
		} ZEND_HASH_FOREACH_END();

		phalcon_fast_join_str(&join_except_conditions, SL(" OR "), &except_conditions);
		PHALCON_CONCAT_SVS(&tmp_condition, "(", &join_except_conditions, ")");
		phalcon_array_append(&conditions, &tmp_condition, PH_COPY);
	}

	/**
	 * The field can be an array of values
	 */
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(values), idx, str_key, value) {
		zval field = {}, condition = {};

		if (str_key) {
			ZVAL_STR(&field, str_key);
		} else {
			ZVAL_LONG(&field, idx);
		}

		PHALCON_CONCAT_SVSV(&condition, "[", &field, "] = ?", &number);
		phalcon_array_append(&conditions, &condition, PH_COPY);
		phalcon_array_append(&bind_params, value, PH_COPY);

		phalcon_increment(&number);
	} ZEND_HASH_FOREACH_END();

	phalcon_fast_join_str(&join_conditions, SL(" AND "), &conditions);

	/**
	 * We don't trust the user, so we pass the parameters as bound parameters
	 */
	array_init_size(&params, 2);
	phalcon_array_update_str(&params, SL("conditions"), &join_conditions, PH_COPY);
	phalcon_array_update_str(&params, SL("bind"), &bind_params, PH_COPY);

	/**
	 * Check using a standard count
	 */
	PHALCON_CALL_CE_STATIC(&number, Z_OBJCE_P(record), "count", &params);
	if (!PHALCON_IS_LONG(&number, 0)) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
