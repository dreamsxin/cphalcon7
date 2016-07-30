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

#include "mvc/model/resultset/simple.h"
#include "mvc/model/resultset.h"
#include "mvc/model/resultsetinterface.h"
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
#include "kernel/exception.h"
#include "kernel/variables.h"

#include "internal/arginfo.h"

/**
 * Phalcon\Mvc\Model\Resultset\Simple
 *
 * Simple resultsets only contains complete objects.
 * This class builds every complete object as it is required
 */
zend_class_entry *phalcon_mvc_model_resultset_simple_ce;

PHP_METHOD(Phalcon_Mvc_Model_Resultset_Simple, __construct);
PHP_METHOD(Phalcon_Mvc_Model_Resultset_Simple, valid);
PHP_METHOD(Phalcon_Mvc_Model_Resultset_Simple, toArray);
PHP_METHOD(Phalcon_Mvc_Model_Resultset_Simple, serialize);
PHP_METHOD(Phalcon_Mvc_Model_Resultset_Simple, unserialize);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_resultset_simple___construct, 0, 0, 3)
	ZEND_ARG_INFO(0, columnMap)
	ZEND_ARG_INFO(0, model)
	ZEND_ARG_INFO(0, result)
	ZEND_ARG_INFO(0, cache)
	ZEND_ARG_INFO(0, keepSnapshots)
	ZEND_ARG_INFO(0, sourceModel)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_resultset_simple_toarray, 0, 0, 0)
	ZEND_ARG_INFO(0, renameColumns)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_resultset_simple_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Resultset_Simple, __construct, arginfo_phalcon_mvc_model_resultset_simple___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Model_Resultset_Simple, valid, arginfo_iterator_valid, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Resultset_Simple, toArray, arginfo_phalcon_mvc_model_resultset_simple_toarray, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Resultset_Simple, serialize, arginfo_serializable_serialize, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Resultset_Simple, unserialize, arginfo_serializable_unserialize, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Resultset\Simple initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Resultset_Simple){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\Resultset, Simple, mvc_model_resultset_simple, phalcon_mvc_model_resultset_ce, phalcon_mvc_model_resultset_simple_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_model_resultset_simple_ce, SL("_sourceModel"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_resultset_simple_ce, SL("_model"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_resultset_simple_ce, SL("_columnMap"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_model_resultset_simple_ce, SL("_keepSnapshots"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_resultset_simple_ce, SL("_rowsModels"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_resultset_simple_ce, SL("_rowsObjects"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_mvc_model_resultset_simple_ce, 5, zend_ce_iterator, spl_ce_SeekableIterator, spl_ce_Countable, zend_ce_arrayaccess, zend_ce_serializable);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Model\Resultset\Simple constructor
 *
 * @param array $columnMap
 * @param Phalcon\Mvc\ModelInterface $model
 * @param Phalcon\Db\Result\Pdo $result
 * @param Phalcon\Cache\BackendInterface $cache
 * @param boolean $keepSnapshots
 */
PHP_METHOD(Phalcon_Mvc_Model_Resultset_Simple, __construct){

	zval *column_map, *model, *result, *cache = NULL, *keep_snapshots = NULL, *source_model = NULL, fetch_assoc = {}, limit = {}, row_count = {}, big_resultset = {};

	phalcon_fetch_params(0, 3, 3, &column_map, &model, &result, &cache, &keep_snapshots, &source_model);

	if (!cache) {
		cache = &PHALCON_GLOBAL(z_null);
	}

	if (!keep_snapshots) {
		keep_snapshots = &PHALCON_GLOBAL(z_null);
	}

	if (!source_model) {
		source_model = &PHALCON_GLOBAL(z_null);
	}

	phalcon_update_property_zval(getThis(), SL("_model"), model);
	phalcon_update_property_zval(getThis(), SL("_result"), result);
	phalcon_update_property_zval(getThis(), SL("_cache"), cache);
	phalcon_update_property_zval(getThis(), SL("_columnMap"), column_map);
	phalcon_update_property_zval(getThis(), SL("_sourceModel"), source_model);

	if (Z_TYPE_P(result) != IS_OBJECT) {
		RETURN_NULL();
	}

	/** 
	 * Use only fetch assoc
	 */
	ZVAL_LONG(&fetch_assoc, PDO_FETCH_ASSOC);
	PHALCON_CALL_METHODW(NULL, result, "setfetchmode", &fetch_assoc);

	ZVAL_LONG(&limit, 32);

	PHALCON_CALL_METHODW(&row_count, result, "numrows");

	/** 
	 * Check if it's a big resultset
	 */
	is_smaller_function(&big_resultset, &limit, &row_count);
	if (PHALCON_IS_TRUE(&big_resultset)) {
		phalcon_update_property_long(getThis(), SL("_type"), 1);
	} else {
		phalcon_update_property_long(getThis(), SL("_type"), 0);
	}

	/** 
	 * Update the row-count
	 */
	phalcon_update_property_zval(getThis(), SL("_count"), &row_count);

	/** 
	 * Set if the returned resultset must keep the record snapshots
	 */
	phalcon_update_property_zval(getThis(), SL("_keepSnapshots"), keep_snapshots);

	phalcon_update_property_empty_array(getThis(), SL("_models"));
	phalcon_update_property_empty_array(getThis(), SL("_others"));
}

/**
 * Check whether the internal resource has rows to fetch
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Resultset_Simple, valid){

	zval *type, *result, row = {}, rows = {}, dirty_state = {}, *hydrate_mode, *keep_snapshots, *column_map, key = {}, *source_model, *model, active_row = {}, *rows_objects;
	zend_class_entry *ce;

	type = phalcon_read_property(getThis(), SL("_type"), PH_NOISY);
	if (zend_is_true(type)) {
		result = phalcon_read_property(getThis(), SL("_result"), PH_NOISY);
		if (Z_TYPE_P(result) == IS_OBJECT) {
			PHALCON_CALL_METHODW(&row, result, "fetch");
		} else {
			ZVAL_FALSE(&row);
		}
	} else {
		phalcon_return_property(&rows, getThis(), SL("_rows"));
		if (Z_TYPE(rows) != IS_ARRAY) {
			result = phalcon_read_property(getThis(), SL("_result"), PH_NOISY);
			if (Z_TYPE_P(result) == IS_OBJECT) {
				PHALCON_CALL_METHODW(&rows, result, "fetchall");
				phalcon_update_property_zval(getThis(), SL("_rows"), &rows);
			}
		}

		if (Z_TYPE(rows) == IS_ARRAY) {
			phalcon_array_get_current(&row, &rows);
			if (PHALCON_IS_NOT_FALSE(&row)) {
				zend_hash_move_forward(Z_ARRVAL(rows));
			}
		} else {
			ZVAL_FALSE(&row);
		}
	}

	if (Z_TYPE(row) != IS_ARRAY) {
		phalcon_update_property_bool(getThis(), SL("_activeRow"), 0);
		RETURN_FALSE;
	}

	/** 
	 * Set records as dirty state PERSISTENT by default
	 */
	ZVAL_LONG(&dirty_state, 0);

	/** 
	 * Get current hydration mode
	 */
	hydrate_mode = phalcon_read_property(getThis(), SL("_hydrateMode"), PH_NOISY);

	/** 
	 * Tell if the resultset is keeping snapshots
	 */
	keep_snapshots = phalcon_read_property(getThis(), SL("_keepSnapshots"), PH_NOISY);

	/** 
	 * Get the resultset column map
	 */
	column_map = phalcon_read_property(getThis(), SL("_columnMap"), PH_NOISY);

	PHALCON_CALL_SELFW(&key, "key");

	source_model = phalcon_read_property(getThis(), SL("_sourceModel"), PH_NOISY);

	if (Z_TYPE_P(source_model) == IS_OBJECT) {
		ce = Z_OBJCE_P(source_model);
	} else {
		ce = phalcon_mvc_model_ce;
	}

	/** 
	 * Hydrate based on the current hydration
	 */
	switch (phalcon_get_intval(hydrate_mode)) {

		case 0:
			rows_objects = phalcon_read_property(getThis(), SL("_rowsModels"), PH_NOISY);
			if (!phalcon_array_isset_fetch(&active_row, rows_objects, &key)) {
				/** 
				 * this_ptr->model is the base entity
				 */
				model = phalcon_read_property(getThis(), SL("_model"), PH_NOISY);

				/** 
				 * Performs the standard hydration based on objects
				 */
				PHALCON_CALL_CE_STATICW(&active_row, ce, "cloneresultmap", model, &row, column_map, &dirty_state, keep_snapshots, source_model);

				phalcon_update_property_array(getThis(), SL("_rowsModels"), &key, &active_row);
			}
			break;

		default:
			rows_objects = phalcon_read_property(getThis(), SL("_rowsOthers"), PH_NOISY);
			if (!phalcon_array_isset_fetch(&active_row, rows_objects, &key)) {
				/** 
				 * Other kinds of hydrations
				 */
				PHALCON_CALL_CE_STATICW(&active_row, ce, "cloneresultmaphydrate", &row, column_map, hydrate_mode, source_model);

				phalcon_update_property_array(getThis(), SL("_rowsModels"), &key, &active_row);
			}
			break;
	}

	PHALCON_PTR_DTOR(&row);
	PHALCON_PTR_DTOR(&key);

	phalcon_update_property_zval(getThis(), SL("_activeRow"), &active_row);
	PHALCON_PTR_DTOR(&active_row);
	RETURN_TRUE;
}

/**
 * Returns a complete resultset as an array, if the resultset has a big number of rows
 * it could consume more memory than it currently does. Exporting the resultset to an array
 * couldn't be faster with a large number of records
 *
 * @param boolean $renameColumns
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_Resultset_Simple, toArray){

	zval *rename_columns = NULL, records = {};

	phalcon_fetch_params(0, 0, 1, &rename_columns);

	if (!rename_columns) {
		rename_columns = &PHALCON_GLOBAL(z_true);
	}

	array_init(&records);

	PHALCON_CALL_METHODW(NULL, getThis(), "rewind");

	while (1) {
		zval valid = {}, current = {}, arr = {};

		PHALCON_CALL_METHODW(&valid, getThis(), "valid");
		if (!PHALCON_IS_NOT_FALSE(&valid)) {
			break;
		}

		PHALCON_CALL_METHODW(&current, getThis(), "current");
		if (Z_TYPE(current) == IS_OBJECT && phalcon_method_exists_ex(&current, SL("toarray")) == SUCCESS) {
			PHALCON_CALL_METHODW(&arr, &current, "toarray", &PHALCON_GLOBAL(z_null), rename_columns);
			phalcon_array_append(&records, &arr, PH_COPY);
		} else {
			phalcon_array_append(&records, &current, PH_COPY);
		}
		PHALCON_CALL_METHODW(NULL, getThis(), "next");
	}

	RETURN_CTORW(&records);
}

/**
 * Serializing a resultset will dump all related rows into a big array
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Resultset_Simple, serialize){

	zval records = {}, *model, *cache, *column_map, *hydrate_mode, data = {};

	PHALCON_CALL_METHODW(&records, getThis(), "toarray", &PHALCON_GLOBAL(z_false));

	model = phalcon_read_property(getThis(), SL("_model"), PH_NOISY);
	cache = phalcon_read_property(getThis(), SL("_cache"), PH_NOISY);
	column_map = phalcon_read_property(getThis(), SL("_columnMap"), PH_NOISY);
	hydrate_mode = phalcon_read_property(getThis(), SL("_hydrateMode"), PH_NOISY);

	array_init_size(&data, 5);
	phalcon_array_update_str(&data, SL("model"), model, PH_COPY);
	phalcon_array_update_str(&data, SL("cache"), cache, PH_COPY);
	phalcon_array_update_str(&data, SL("rows"), &records, PH_COPY);
	phalcon_array_update_str(&data, SL("columnMap"), column_map, PH_COPY);
	phalcon_array_update_str(&data, SL("hydrateMode"), hydrate_mode, PH_COPY);

	/** 
	 * Force to re-execute the query
	 */
	phalcon_update_property_bool(getThis(), SL("_activeRow"), 0);

	/** 
	 * Serialize the cache using the serialize function
	 */
	phalcon_serialize(return_value, &data);
}

/**
 * Unserializing a resultset only works on the rows present in the saved state
 *
 * @param string $data
 */
PHP_METHOD(Phalcon_Mvc_Model_Resultset_Simple, unserialize){

	zval *data, resultset = {}, model = {}, rows = {}, cache = {}, column_map = {}, hydrate_mode = {};

	phalcon_fetch_params(0, 1, 0, &data);

	phalcon_update_property_long(getThis(), SL("_type"), 0);

	phalcon_unserialize(&resultset, data);
	if (Z_TYPE(resultset) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Invalid serialization data");
		return;
	}

	phalcon_array_fetch_str(&model, &resultset, SL("model"), PH_NOISY);
	phalcon_update_property_zval(getThis(), SL("_model"), &model);

	phalcon_array_fetch_str(&rows, &resultset, SL("rows"), PH_NOISY);
	phalcon_update_property_zval(getThis(), SL("_rows"), &rows);

	phalcon_array_fetch_str(&cache, &resultset, SL("cache"), PH_NOISY);
	phalcon_update_property_zval(getThis(), SL("_cache"), &cache);

	phalcon_array_fetch_str(&column_map, &resultset, SL("columnMap"), PH_NOISY);
	phalcon_update_property_zval(getThis(), SL("_columnMap"), &column_map);

	phalcon_array_fetch_str(&hydrate_mode, &resultset, SL("hydrateMode"), PH_NOISY);
	phalcon_update_property_zval(getThis(), SL("_hydrateMode"), &hydrate_mode);
}
