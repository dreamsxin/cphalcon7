
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

#include "mvc/model/validator/uniqueness.h"
#include "mvc/model/validator.h"
#include "mvc/model/validatorinterface.h"
#include "mvc/model/exception.h"
#include "mvc/model/metadatainterface.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/concat.h"
#include "kernel/exception.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/object.h"

#include "interned-strings.h"

/**
 * Phalcon\Mvc\Model\Validator\Uniqueness
 *
 * Validates that a field or a combination of a set of fields are not
 * present more than once in the existing records of the related table
 *
 *<code>
 *use Phalcon\Mvc\Model\Validator\Uniqueness as Uniqueness;
 *
 *class Subscriptors extends Phalcon\Mvc\Model
 *{
 *
 *  public function validation()
 *  {
 *      $this->validate(new Uniqueness(array(
 *          'field' => 'email'
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
zend_class_entry *phalcon_mvc_model_validator_uniqueness_ce;

PHP_METHOD(Phalcon_Mvc_Model_Validator_Uniqueness, validate);

static const zend_function_entry phalcon_mvc_model_validator_uniqueness_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Validator_Uniqueness, validate, arginfo_phalcon_mvc_model_validatorinterface_validate, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Validator\Uniqueness initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Validator_Uniqueness){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\Validator, Uniqueness, mvc_model_validator_uniqueness, phalcon_mvc_model_validator_ce, phalcon_mvc_model_validator_uniqueness_method_entry, 0);

	zend_class_implements(phalcon_mvc_model_validator_uniqueness_ce, 1, phalcon_mvc_model_validatorinterface_ce);

	return SUCCESS;
}

/**
 * Executes the validator
 *
 * @param Phalcon\Mvc\ModelInterface $record
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Validator_Uniqueness, validate){

	zval *record, option, field, dependency_injector, service, meta_data, bind_types, bind_data_types, column_map, conditions, bind_params;
	zval number, *compose_field, column_field, exception_message, value, compose_condition, bind_type, condition, operation_made;
	zval primary_fields, *primary_field, join_conditions, params, message, join_fields, type, is_set_code, code;

	phalcon_fetch_params(0, 1, 0, &record);

	ZVAL_STRING(&option, "field");

	PHALCON_CALL_METHODW(&field, getThis(), "get&option", &option);
	PHALCON_CALL_METHODW(&dependency_injector, record, "getdi");

	ZVAL_STRING(&service, "modelsMetadata");

	PHALCON_CALL_METHODW(&meta_data, &dependency_injector, "getshared", &service);
	PHALCON_VERIFY_INTERFACEW(&meta_data, phalcon_mvc_model_metadatainterface_ce);

	/** 
	 * PostgreSQL check if the compared constant has the same type as the column, so we
	 * make cast to the data passed to match those column types
	 */
	array_init(&bind_types);

	PHALCON_CALL_METHODW(&bind_data_types, &meta_data, "getbindtypes", record);
	if (PHALCON_GLOBAL(orm).column_renaming) {
		PHALCON_CALL_METHODW(&column_map, &meta_data, "getreversecolumnmap", record);
	}

	array_init(&conditions);
	array_init(&bind_params);

	ZVAL_LONG(&number, 0);
	if (Z_TYPE(field) == IS_ARRAY) {
		/** 
		 * The field can be an array of values
		 */
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(field), compose_field) {
			/** 
			 * The reversed column map is used in the case to get real column name
			 */
			if (Z_TYPE(column_map) == IS_ARRAY) { 
				if (!phalcon_array_isset_fetch(&column_field, &column_map, compose_field)) {
					PHALCON_CONCAT_SVS(&exception_message, "Column '", compose_field, "\" isn't part of the column map");
					PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}
			} else {
				ZVAL_COPY(&column_field, compose_field);
			}

			/** 
			 * Some database systems require that we pass the values using bind casting
			 */
			if (!phalcon_array_isset(&bind_data_types, &column_field)) {
				PHALCON_CONCAT_SVS(&exception_message, "Column '", &column_field, "\" isn't part of the table columns");
				PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
				return;
			}

			/** 
			 * The attribute could be "protected" so we read using "readattribute"
			 */
			PHALCON_CALL_METHODW(&value, record, "readattribute", compose_field);

			PHALCON_CONCAT_SVSV(&compose_condition, "[", compose_field, "] = ?", &number);
			phalcon_array_append(&conditions, &compose_condition, PH_COPY);
			phalcon_array_append(&bind_params, &value, PH_COPY);

			phalcon_array_fetch(&bind_type, &bind_data_types, &column_field, PH_NOISY);
			phalcon_array_append(&bind_types, &bind_type, PH_COPY);
			phalcon_increment(&number);
		} ZEND_HASH_FOREACH_END();

	} else {
		/** 
		 * The reversed column map is used in the case to get real column name
		 */
		if (Z_TYPE(column_map) == IS_ARRAY) { 
			if (!phalcon_array_isset_fetch(&column_field, &column_map, &field)) {
				PHALCON_CONCAT_SVS(&exception_message, "Column '", &field, "\" isn't part of the column map");
				PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
				return;
			}
		} else {
			ZVAL_COPY(&column_field, &field);
		}

		/** 
		 * Some database systems require that we pass the values using bind casting
		 */
		if (!phalcon_array_isset(&bind_data_types, &column_field)) {
			PHALCON_CONCAT_SVS(&exception_message, "Column '", &column_field, "\" isn't part of the table columns");
			PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
			return;
		}

		/** 
		 * We're checking the uniqueness with only one field
		 */
		PHALCON_CALL_METHODW(&value, record, "readattribute", &field);

		PHALCON_CONCAT_SVS(&condition, "[", &field, "] = ?0");
		phalcon_array_append(&conditions, &condition, PH_COPY);
		phalcon_array_append(&bind_params, &value, PH_COPY);

		phalcon_array_fetch(&bind_type, &bind_data_types, &column_field, PH_NOISY);
		phalcon_array_append(&bind_types, &bind_type, PH_COPY);
		phalcon_increment(&number);
	}

	/** 
	 * If the operation is update, there must be values in the object
	 */
	PHALCON_CALL_METHODW(&operation_made, record, "getoperationmade");
	if (PHALCON_IS_LONG(&operation_made, 2)) {
		/** 
		 * We build a query with the primary key attributes
		 */
		if (PHALCON_GLOBAL(orm).column_renaming) {
			PHALCON_CALL_METHODW(&column_map, &meta_data, "getcolumnmap", record);
		}

		PHALCON_CALL_METHODW(&primary_fields, &meta_data, "getprimarykeyattributes", record);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(primary_fields), primary_field) {
			zval attribute_field;

			if (!phalcon_array_isset(&bind_data_types, primary_field)) {
				PHALCON_CONCAT_SVS(&exception_message, "Column '", primary_field, "\" isn't part of the table columns");
				PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
				return;
			}

			/** 
			 * Rename the column if there is a column map
			 */
			if (Z_TYPE(column_map) == IS_ARRAY) { 
				if (!phalcon_array_isset_fetch(&attribute_field, &column_map, primary_field)) {
					PHALCON_CONCAT_SVS(&exception_message, "Column '", primary_field, "\" isn't part of the column map");
					PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}
			} else {
				ZVAL_COPY(&attribute_field, primary_field);
			}

			/** 
			 * Create a condition based on the renamed primary key
			 */
			PHALCON_CALL_METHODW(&value, record, "readattribute", primary_field);

			PHALCON_CONCAT_SVSV(&condition, "[", &attribute_field, "] <> ?", &number);
			phalcon_array_append(&conditions, &condition, PH_COPY);
			phalcon_array_append(&bind_params, &value, PH_COPY);

			phalcon_array_fetch(&bind_type, &bind_data_types, primary_field, PH_NOISY);
			phalcon_array_append(&bind_types, &bind_type, PH_COPY);
			phalcon_increment(&number);
		} ZEND_HASH_FOREACH_END();

	}

	phalcon_fast_join_str(&join_conditions, SL(" AND "), &conditions);

	/** 
	 * We don't trust the user, so we pass the parameters as bound parameters
	 */
	array_init_size(&params, 4);
	phalcon_array_update_str(&params, SL("di"), &dependency_injector, PH_COPY);
	phalcon_array_update_str(&params, SL("&conditions"), &join_conditions, PH_COPY);
	phalcon_array_update_str(&params, SL("bind"), &bind_params, PH_COPY);
	phalcon_array_update_str(&params, SL("bindTypes"), &bind_types, PH_COPY);

	/** 
	 * Check using a standard count
	 */
	PHALCON_CALL_CE_STATIC(&number, Z_OBJCE_P(record), "count", &params);
	if (!PHALCON_IS_LONG(&number, 0)) {

		/** 
		 * Check if the developer has defined a custom message
		 */
		ZVAL_STRING(&option, ISV(message));

		PHALCON_CALL_METHODW(&message, getThis(), "get&option", &option);
		if (!zend_is_true(&message)) {
			if (Z_TYPE(field) == IS_ARRAY) { 
				phalcon_fast_join_str(&join_fields, SL(", "), &field);

				PHALCON_CONCAT_SVS(&message, "Value of fields: '", &join_fields, "' are already present in another record");
			} else {
				PHALCON_CONCAT_SVS(&message, "Value of field: '", &field, "' is already present in another record");
			}
		}

		/** 
		 * Append the message to the validator
		 */
		ZVAL_STRING(&type, "Unique");

		/*
		 * Is code set
		 */
		ZVAL_STRING(&option, ISV(code));

		PHALCON_CALL_METHODW(&is_set_code, getThis(), "isset&option", &option);
		if (zend_is_true(&is_set_code)) {
			PHALCON_CALL_METHODW(&code, getThis(), "get&option", &option);
		} else {
			ZVAL_LONG(&code, 0);
		}

		PHALCON_CALL_METHODW(NULL, getThis(), "appendmessage", &message, &field, &type, &code);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
