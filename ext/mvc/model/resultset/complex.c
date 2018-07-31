
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

#include "mvc/model/resultset/complex.h"
#include "mvc/model/resultset.h"
#include "mvc/model/resultsetinterface.h"
#include "mvc/model/row.h"
#include "mvc/model/exception.h"
#include "mvc/model.h"

#include <ext/pdo/php_pdo_driver.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"
#include "kernel/array.h"
#include "kernel/hash.h"
#include "kernel/concat.h"
#include "kernel/string.h"
#include "kernel/variables.h"
#include "kernel/exception.h"

#include "internal/arginfo.h"

#include "interned-strings.h"

/**
 * Phalcon\Mvc\Model\Resultset\Complex
 *
 * Complex resultsets may include complete objects and scalar values.
 * This class builds every complex row as it is required
 */
zend_class_entry *phalcon_mvc_model_resultset_complex_ce;

PHP_METHOD(Phalcon_Mvc_Model_Resultset_Complex, __construct);
PHP_METHOD(Phalcon_Mvc_Model_Resultset_Complex, valid);
PHP_METHOD(Phalcon_Mvc_Model_Resultset_Complex, toArray);
PHP_METHOD(Phalcon_Mvc_Model_Resultset_Complex, serialize);
PHP_METHOD(Phalcon_Mvc_Model_Resultset_Complex, unserialize);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_resultset_complex___construct, 0, 0, 2)
	ZEND_ARG_INFO(0, columnsTypes)
	ZEND_ARG_INFO(0, result)
	ZEND_ARG_INFO(0, cache)
	ZEND_ARG_INFO(0, sourceModel)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_resultset_complex_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Resultset_Complex, __construct, arginfo_phalcon_mvc_model_resultset_complex___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Model_Resultset_Complex, valid, arginfo_iterator_valid, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Resultset_Complex, toArray, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Resultset_Complex, serialize, arginfo_serializable_serialize, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Resultset_Complex, unserialize, arginfo_serializable_unserialize, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Resultset\Complex initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Resultset_Complex){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\Resultset, Complex, mvc_model_resultset_complex, phalcon_mvc_model_resultset_ce, phalcon_mvc_model_resultset_complex_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_model_resultset_complex_ce, SL("_sourceModel"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_resultset_complex_ce, SL("_columnTypes"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_resultset_complex_ce, SL("_rowsModels"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_resultset_complex_ce, SL("_rowsObjects"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_resultset_complex_ce, SL("_rowsArrays"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Model\Resultset\Complex constructor
 *
 * @param Phalcon\Mvc\ModelInterface $sourceModel
 * @param array $columnsTypes
 * @param Phalcon\Db\ResultInterface $result
 * @param Phalcon\Cache\BackendInterface $cache
 */
PHP_METHOD(Phalcon_Mvc_Model_Resultset_Complex, __construct){

	zval *columns_types, *result, *cache = NULL, *source_model = NULL, *count = NULL, fetch_assoc = {};

	phalcon_fetch_params(0, 2, 3, &columns_types, &result, &cache, &source_model, &count);

	if (!cache) {
		cache = &PHALCON_GLOBAL(z_null);
	}

	if (!source_model) {
		source_model = &PHALCON_GLOBAL(z_null);
	}

	/**
	 * Column types, tell the resultset how to build the result
	 */
	phalcon_update_property(getThis(), SL("_columnTypes"), columns_types);

	/**
	 * Valid resultsets are Phalcon\Db\ResultInterface instances
	 * FIXME: or Phalcon\Db\Result\Pdo?
	 */
	phalcon_update_property(getThis(), SL("_result"), result);

	/**
	 * Update the related cache if any
	 */
	if (Z_TYPE_P(cache) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_cache"), cache);
	}

	phalcon_update_property(getThis(), SL("_sourceModel"), source_model);

	/**
	 * Resultsets type 1 are traversed one-by-one
	 */
	phalcon_update_property_long(getThis(), SL("_type"), PHALCON_MVC_MODEL_RESULTSET_TYPE_PARTIAL);

	/**
	 * If the database result is an object, change it to fetch assoc
	 */
	if (Z_TYPE_P(result) == IS_OBJECT) {
		ZVAL_LONG(&fetch_assoc, PDO_FETCH_ASSOC);
		PHALCON_CALL_METHOD(NULL, result, "setfetchmode", &fetch_assoc);
	}
}

/**
 * Check whether internal resource has rows to fetch
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Resultset_Complex, valid){

	zval key = {}, source_model = {}, type = {}, row = {}, rows = {}, hydrate_mode = {}, columns_types = {}, underscore = {}, empty_str = {};
	zval dependency_injector = {}, service_name = {}, has = {}, active_row = {}, dirty_state = {}, *column;
	zend_class_entry *ce;
	zend_string *str_key;
	ulong idx;
	int i_type, is_partial, i_hydrate_mode;

	PHALCON_MM_INIT();

	PHALCON_MM_CALL_SELF(&key, "key");
	PHALCON_MM_ADD_ENTRY(&key);

	phalcon_read_property(&source_model, getThis(), SL("_sourceModel"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&type, getThis(), SL("_type"), PH_NOISY|PH_READONLY);
	i_type = (Z_TYPE(type) == IS_LONG) ? Z_LVAL(type) : phalcon_get_intval(&type);
	is_partial = (i_type == PHALCON_MVC_MODEL_RESULTSET_TYPE_PARTIAL);

	if (Z_TYPE(source_model) == IS_OBJECT) {
		ce = Z_OBJCE(source_model);
	} else {
		ce = phalcon_mvc_model_ce;
	}

	if (is_partial) {
		/**
		 * The result is bigger than 32 rows so it's retrieved one by one
		 */

		if (!phalcon_property_array_isset_fetch(&row, getThis(), SL("_rows"), &key, PH_READONLY)) {
			zval result = {};
			phalcon_read_property(&result, getThis(), SL("_result"), PH_NOISY|PH_READONLY);
			if (Z_TYPE(result) == IS_OBJECT) {
				PHALCON_MM_CALL_METHOD(&row, &result, "fetch");
				PHALCON_MM_ADD_ENTRY(&row);
			} else {
				ZVAL_FALSE(&row);
			}
			phalcon_update_property_array(getThis(), SL("_rows"), &key, &row);
		}
	} else {
		/**
		 * The full rows are dumped in this_ptr->rows
		 */
		phalcon_read_property(&rows, getThis(), SL("_rows"), PH_READONLY);
		if (Z_TYPE(rows) != IS_ARRAY) {
			zval result = {};
			phalcon_read_property(&result, getThis(), SL("_result"), PH_NOISY|PH_READONLY);
			if (Z_TYPE(result) == IS_OBJECT) {
				PHALCON_MM_CALL_METHOD(&rows, &result, "fetchall");
				PHALCON_MM_ADD_ENTRY(&rows);
				phalcon_update_property(getThis(), SL("_rows"), &rows);
			}
		}
		if (Z_TYPE(rows) == IS_ARRAY) {
			phalcon_array_get_current(&row, &rows);
			if (Z_TYPE(row) == IS_OBJECT) {
				zend_hash_move_forward(Z_ARRVAL(rows));
			}
		} else {
			ZVAL_FALSE(&row);
		}
	}

	/**
	 * Valid records are arrays
	 */
	if (Z_TYPE(row) == IS_ARRAY || Z_TYPE(row) == IS_OBJECT) {

		/**
		 * The result type=1 so we need to build every row
		 */
		if (!is_partial) {
			/**
			 * The row is already built so we just assign it to the activeRow
			 */
			phalcon_update_property(getThis(), SL("_activeRow"), &row);
			RETURN_MM_TRUE;
		}

		/**
		 * Get current hydration mode
		 */
		phalcon_read_property(&hydrate_mode, getThis(), SL("_hydrateMode"), PH_NOISY|PH_READONLY);
		phalcon_read_property(&columns_types, getThis(), SL("_columnTypes"), PH_NOISY|PH_READONLY);
		i_hydrate_mode  = phalcon_get_intval(&hydrate_mode);

		PHALCON_MM_ZVAL_STRING(&underscore, "_");
		PHALCON_MM_ZVAL_EMPTY_STRING(&empty_str);

		/**
		 * Each row in a complex result is a Phalcon\Mvc\Model\Row instance
		 */
		switch (i_hydrate_mode) {
			case 0:
			{
				zval tmp = {};
				phalcon_read_property(&tmp, getThis(), SL("_rowsModels"), PH_NOISY|PH_READONLY);
				if (phalcon_array_isset_fetch(&active_row, &tmp, &key, PH_READONLY)) {
					phalcon_update_property(getThis(), SL("_activeRow"), &active_row);
					RETURN_MM_TRUE;
				}
				ZVAL_STR(&service_name, IS(modelsRow));

				PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi");
				PHALCON_MM_ADD_ENTRY(&dependency_injector);
				PHALCON_MM_CALL_METHOD(&has, &dependency_injector, "has", &service_name);
				if (zend_is_true(&has)) {
					PHALCON_MM_CALL_METHOD(&active_row, &dependency_injector, "get", &service_name);
					PHALCON_MM_ADD_ENTRY(&active_row);
					PHALCON_MM_VERIFY_CLASS_EX(&active_row, phalcon_mvc_model_row_ce, phalcon_mvc_model_row_ce);
				} else {
					object_init_ex(&active_row, phalcon_mvc_model_row_ce);
					PHALCON_MM_ADD_ENTRY(&active_row);
				}
				break;

			}
			case 1:
			{
				zval tmp = {};
				phalcon_read_property(&tmp, getThis(), SL("_rowsArrays"), PH_NOISY|PH_READONLY);
				if (phalcon_array_isset_fetch(&active_row, &tmp, &key, PH_READONLY)) {
					phalcon_update_property(getThis(), SL("_activeRow"), &active_row);
					RETURN_MM_TRUE;
				}
				array_init(&active_row);
				PHALCON_MM_ADD_ENTRY(&active_row);
				break;
			}
			case 2:
			default:
			{
				zval tmp = {};
				phalcon_read_property(&tmp, getThis(), SL("_rowsObjects"), PH_NOISY|PH_READONLY);
				if (phalcon_array_isset_fetch(&active_row, &tmp, &key, PH_READONLY)) {
					phalcon_update_property(getThis(), SL("_activeRow"), &active_row);
					RETURN_MM_TRUE;
				}
				object_init(&active_row);
				PHALCON_MM_ADD_ENTRY(&active_row);
				break;
			}
		}

		/**
		 * Create every record according to the column types
		 */

		/**
		 * Set records as dirty state PERSISTENT by default
		 */
		ZVAL_LONG(&dirty_state, 0);

		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(columns_types), idx, str_key, column) {
			zval alias = {}, n_alias = {}, column_type = {}, source = {}, attributes = {}, column_map = {}, row_model = {}, *attribute, sql_alias = {}, value = {};

			phalcon_array_fetch_str(&column_type, column, SL("type"), PH_NOISY|PH_READONLY);

			if (PHALCON_IS_STRING(&column_type, "object")) {

				/**
				 * Object columns are assigned column by column
				 */
				phalcon_array_fetch_str(&source, column, SL("column"), PH_NOISY|PH_READONLY);
				phalcon_array_fetch_str(&attributes, column, SL("attributes"), PH_NOISY|PH_READONLY);
				phalcon_array_fetch_str(&column_map, column, SL("columnMap"), PH_NOISY|PH_READONLY);

				/**
				 * Assign the values from the _source_attribute notation to its real column name
				 */
				array_init(&row_model);
				PHALCON_MM_ADD_ENTRY(&row_model);
				ZEND_HASH_FOREACH_VAL(Z_ARRVAL(attributes), attribute) {
					zval column_alias = {}, column_value = {};
					/**
					 * Columns are supposed to be in the form _table_field
					 */
					PHALCON_CONCAT_VVVV(&column_alias, &underscore, &source, &underscore, attribute);

					if (!phalcon_array_isset_fetch(&column_value, &row, &column_alias, PH_NOISY|PH_READONLY)) {
						ZVAL_NULL(&column_value);
					}

					phalcon_array_update(&row_model, attribute, &column_value, PH_COPY);
					zval_ptr_dtor(&column_alias);
				} ZEND_HASH_FOREACH_END();

				/**
				 * Generate the column value according to the hydration type
				 */
				switch (i_hydrate_mode) {

					case 0: {
						zval instance = {};

						/**
						 * Get the base instance
						 */
						if (!phalcon_array_isset_fetch_str(&instance, column, SL("instance"), PH_READONLY)) {
							php_error_docref(NULL, E_NOTICE, "Undefined index: instance");
							ZVAL_NULL(&instance);
						}

						/**
						 * Assign the values to the attributes using a column map
						 */
						PHALCON_MM_CALL_CE_STATIC(&value, ce, "cloneresultmap", &instance, &row_model, &column_map, &dirty_state, &source_model);
						PHALCON_MM_ADD_ENTRY(&value);
						break;
					}

					default:
						/**
						 * Other kinds of hydrations
						 */
						PHALCON_MM_CALL_CE_STATIC(&value, ce, "cloneresultmaphydrate", &row_model, &column_map, &hydrate_mode, &source_model);
						PHALCON_MM_ADD_ENTRY(&value);
						break;
				}

				/**
				 * The complete object is assigned to an attribute with the name of the alias or
				 * the model name
				 */
				if (!phalcon_array_isset_fetch_str(&alias, column, SL("balias"), PH_READONLY)) {
					ZVAL_NULL(&alias);
				}
			} else {
				if (str_key) {
					ZVAL_STR(&alias, str_key);
				} else {
					ZVAL_LONG(&alias, idx);
				}

				/**
				 * Scalar columns are simply assigned to the result object
				 */
				if (phalcon_array_isset_fetch_str(&sql_alias, column, SL("sqlAlias"), PH_READONLY)) {
					if (!phalcon_array_isset_fetch(&value, &row, &sql_alias, PH_READONLY)) {
						ZVAL_NULL(&value);
					}
				} else if (!phalcon_array_isset_fetch(&value, &row, &alias, PH_READONLY)) {
					ZVAL_NULL(&value);
				}

				/**
				 * If a 'balias' is defined is not an unnamed scalar
				 */
				if (!phalcon_array_isset_str(column, SL("balias"))) {
					PHALCON_STR_REPLACE(&n_alias, &underscore, &empty_str, &alias);
					PHALCON_MM_ADD_ENTRY(&n_alias);
					ZVAL_COPY_VALUE(&alias, &n_alias);
				}
			}

			/**
			 * Assign the instance according to the hydration type
			 */
			if (unlikely(Z_TYPE(alias) == IS_NULL)) {
				zend_throw_exception_ex(phalcon_mvc_model_exception_ce, 0, "Unexpected inconsistency: attribute is NULL");
				RETURN_MM();
			}

			switch (i_hydrate_mode) {

				case 1:
					phalcon_array_update(&active_row, &alias, &value, PH_COPY);
					break;

				default:
					phalcon_update_property_zval_zval(&active_row, &alias, &value);
					break;

			}
		} ZEND_HASH_FOREACH_END();

		if (Z_TYPE(active_row) == IS_OBJECT && phalcon_method_exists_ex(&active_row, SL("afterfetch")) == SUCCESS) {
			PHALCON_MM_CALL_METHOD(NULL, &active_row, "afterfetch");
		}
		switch (i_hydrate_mode) {
			case 0:
			{
				phalcon_update_property_array(getThis(), SL("_rowsModels"), &key, &active_row);
				break;
			}
			case 1:
			{
				phalcon_update_property_array(getThis(), SL("_rowsArrays"), &key, &active_row);
				break;
			}
			case 2:
			default:
			{
				phalcon_update_property_array(getThis(), SL("_rowsObjects"), &key, &active_row);
				break;
			}
		}
		/**
		 * Store the generated row in this_ptr->activeRow to be retrieved by 'current'
		 */
		phalcon_update_property(getThis(), SL("_activeRow"), &active_row);
		RETURN_MM_TRUE;
	}

	/**
	 * There are no results to retrieve so we update this_ptr->activeRow as false
	 */
	phalcon_update_property_bool(getThis(), SL("_activeRow"), 0);
	RETURN_MM_FALSE;
}

/**
 * Returns a complete resultset as an array, if the resultset has a big number of rows
 * it could consume more memory than currently it does.
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_Resultset_Complex, toArray){

	array_init(return_value);

	PHALCON_MM_INIT();

	PHALCON_MM_CALL_METHOD(NULL, getThis(), "rewind");

	while (1) {
		zval valid = {}, current = {}, arr = {};

		PHALCON_MM_CALL_METHOD(&valid, getThis(), "valid");
		if (!PHALCON_IS_NOT_FALSE(&valid)) {
			break;
		}

		PHALCON_MM_CALL_METHOD(&current, getThis(), "current");
		PHALCON_MM_ADD_ENTRY(&current);
		if (Z_TYPE(current) == IS_OBJECT && phalcon_method_exists_ex(&current, SL("toarray")) == SUCCESS) {
			PHALCON_MM_CALL_METHOD(&arr, &current, "toarray");
			phalcon_array_append(return_value, &arr, 0);
		} else {
			phalcon_array_append(return_value, &current, PH_COPY);
		}
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "next");
	}
	RETURN_MM();
}

/**
 * Serializing a resultset will dump all related rows into a big array
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Resultset_Complex, serialize){

	zval records = {}, count = {}, cache = {}, column_types = {}, hydrate_mode = {}, data = {};

	/**
	 * Obtain the records as an array
	 */
	PHALCON_CALL_METHOD(&records, getThis(), "toarray");
	phalcon_fast_count(&count, &records);

	phalcon_read_property(&cache, getThis(), SL("_cache"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&column_types, getThis(), SL("_columnTypes"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&hydrate_mode, getThis(), SL("_hydrateMode"), PH_NOISY|PH_READONLY);

	array_init_size(&data, 4);
	phalcon_array_update_str(&data, SL("cache"), &cache, PH_COPY);
	phalcon_array_update_str(&data, SL("rows"), &records, 0);
	phalcon_array_update_str(&data, SL("count"), &count, 0);
	phalcon_array_update_str(&data, SL("columnTypes"), &column_types, PH_COPY);
	phalcon_array_update_str(&data, SL("hydrateMode"), &hydrate_mode, PH_COPY);

	phalcon_serialize(return_value, &data);
	zval_ptr_dtor(&data);
}

/**
 * Unserializing a resultset will allow to only works on the rows present in the saved state
 *
 * @param string $data
 */
PHP_METHOD(Phalcon_Mvc_Model_Resultset_Complex, unserialize){

	zval *data, resultset = {}, rows = {}, count = {}, cache = {}, column_types = {}, hydrate_mode = {};

	phalcon_fetch_params(0, 1, 0, &data);

	phalcon_update_property_long(getThis(), SL("_type"), PHALCON_MVC_MODEL_RESULTSET_TYPE_FULL);

	phalcon_unserialize(&resultset, data);
	if (Z_TYPE(resultset) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Invalid serialization data");
		return;
	}

	phalcon_array_fetch_str(&rows, &resultset, SL("rows"), PH_NOISY|PH_READONLY);
	phalcon_update_property(getThis(), SL("_rows"), &rows);

	phalcon_array_fetch_str(&rows, &resultset, SL("count"), PH_NOISY|PH_READONLY);
	phalcon_update_property(getThis(), SL("_count"), &count);

	phalcon_array_fetch_str(&cache, &resultset, SL("cache"), PH_NOISY|PH_READONLY);
	phalcon_update_property(getThis(), SL("_cache"), &cache);

	phalcon_array_fetch_str(&column_types, &resultset, SL("columnTypes"), PH_NOISY|PH_READONLY);
	phalcon_update_property(getThis(), SL("_columnTypes"), &column_types);

	phalcon_array_fetch_str(&hydrate_mode, &resultset, SL("hydrateMode"), PH_NOISY|PH_READONLY);
	phalcon_update_property(getThis(), SL("_hydrateMode"), &hydrate_mode);
}
