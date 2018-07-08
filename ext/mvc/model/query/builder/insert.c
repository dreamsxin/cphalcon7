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

#include "mvc/model/query/builder/insert.h"
#include "mvc/model/query/builder.h"
#include "mvc/model/query/builderinterface.h"
#include "mvc/model/query/exception.h"
#include "mvc/model/metadatainterface.h"
#include "mvc/model/metadata/memory.h"
#include "mvc/model/query.h"
#include "mvc/model/query/scanner.h"
#include "di.h"
#include "diinterface.h"
#include "di/injectable.h"
#include "db/rawvalue.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/file.h"
#include "kernel/hash.h"
#include "kernel/framework/orm.h"

#include "interned-strings.h"

/**
 * Phalcon\Mvc\Model\Query\Builder\Insert
 *
 *<code>
 *$resultset = Phalcon\Mvc\Model\Query\Builder::createInsertBuilder()
 *   ->table('Robots')
 *   ->columns(array('name'))
 *   ->values(array(array('name' => 'Google'), array('name' => 'Baidu')))
 *   ->getQuery()
 *   ->execute();
 *</code>
 */
zend_class_entry *phalcon_mvc_model_query_builder_insert_ce;

PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Insert, __construct);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Insert, table);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Insert, getTable);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Insert, columns);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Insert, getColumns);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Insert, values);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Insert, getValues);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Insert, setConflict);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Insert, _compile);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_insert___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, params, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_insert_table, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, table, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_insert_columns, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, columns, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_insert_values, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, values, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_insert_setconflict, 0, 0, 1)
	ZEND_ARG_INFO(0, conflict)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_query_builder_insert_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Insert, __construct, arginfo_phalcon_mvc_model_query_builder_insert___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Insert, table, arginfo_phalcon_mvc_model_query_builder_insert_table, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Insert, getTable, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Insert, columns, arginfo_phalcon_mvc_model_query_builder_insert_columns, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Insert, getColumns, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Insert, values, arginfo_phalcon_mvc_model_query_builder_insert_values, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Insert, getValues, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Insert, setConflict, arginfo_phalcon_mvc_model_query_builder_insert_setconflict, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Insert, _compile, NULL, ZEND_ACC_PROTECTED)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Query\Builder\Insert initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Query_Builder_Insert){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\Query\\Builder, Insert, mvc_model_query_builder_insert, phalcon_mvc_model_query_builder_ce, phalcon_mvc_model_query_builder_insert_method_entry, 0);

	zend_declare_property_long(phalcon_mvc_model_query_builder_insert_ce, SL("_type"), PHQL_T_INSERT, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_insert_ce, SL("_table"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_insert_ce, SL("_columns"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_insert_ce, SL("_flipColumns"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_insert_ce, SL("_values"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_mvc_model_query_builder_insert_ce, SL("_useColumnName"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_insert_ce, SL("_conflict"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_mvc_model_query_builder_insert_ce, 1, phalcon_mvc_model_query_builderinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Model\Query\Builder\Insert constructor
 *
 * @param array $params
 * @param Phalcon\Di $dependencyInjector
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Insert, __construct){

	zval *params = NULL, *dependency_injector = NULL, table = {}, columns = {}, values = {};

	phalcon_fetch_params(0, 0, 2, &params, &dependency_injector);

	/**
	 * Update the dependency injector if any
	 */
	if (dependency_injector && Z_TYPE_P(dependency_injector) != IS_NULL) {
		PHALCON_CALL_METHOD(NULL, getThis(), "setdi", dependency_injector);
	}

	if (params && Z_TYPE_P(params) == IS_ARRAY) {
		if (phalcon_array_isset_fetch_str(&table, params, SL("table"), PH_READONLY)) {
			PHALCON_CALL_METHOD(NULL, getThis(), "settable", &table);
		}

		if (phalcon_array_isset_fetch_str(&columns, params, SL("columns"), PH_READONLY)) {
			PHALCON_CALL_METHOD(NULL, getThis(), "setcolumns", &columns);
		}

		if (phalcon_array_isset_fetch_str(&values, params, SL("values"), PH_READONLY)) {
			PHALCON_CALL_METHOD(NULL, getThis(), "setvalues", &values);
		}
	}
}

/**
 * Sets the table to insert into
 *
 * @param string table
 * @return Phalcon\Mvc\Model\Query\Builder\Insert
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Insert, table){

	zval *table;

	phalcon_fetch_params(0, 1, 0, &table);

	phalcon_update_property(getThis(), SL("_table"), table);
	RETURN_THIS();
}

/**
 * Gets the table to insert into
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Insert, getTable){


	RETURN_MEMBER(getThis(), "_table");
}

/**
 * Set the columns that will be inserted
 *
 * @param array $columns
 * @return Phalcon\Mvc\Model\Query\Builder\Insert
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Insert, columns){

	zval *columns, flip_columns = {}, *entry, data;
	zend_ulong num_idx;
	zend_string *str_idx;

	phalcon_fetch_params(1, 1, 0, &columns);

	array_init_size(&flip_columns, zend_hash_num_elements(Z_ARRVAL_P(columns)));
	PHALCON_MM_ADD_ENTRY(&flip_columns);
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(columns), num_idx, str_idx, entry) {
		if (Z_TYPE_P(entry) == IS_LONG) {
			if (str_idx) {
				ZVAL_STR_COPY(&data, str_idx);
			} else {
				ZVAL_LONG(&data, num_idx);
			}
			zend_hash_index_update(Z_ARRVAL(flip_columns), Z_LVAL_P(entry), &data);
		} else if (Z_TYPE_P(entry) == IS_STRING) {
			if (str_idx) {
				ZVAL_STR_COPY(&data, str_idx);
			} else {
				ZVAL_LONG(&data, num_idx);
			}
			zend_symtable_update(Z_ARRVAL(flip_columns), Z_STR_P(entry), &data);
		} else {
			php_error_docref(NULL, E_WARNING, "Columns must be STRING and INTEGER values!");
		}
	} ZEND_HASH_FOREACH_END();

	phalcon_update_property(getThis(), SL("_columns"), columns);
	phalcon_update_property(getThis(), SL("_flipColumns"), &flip_columns);

	RETURN_MM_THIS();
}

/**
 * Gets the columns that will be inserted
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Insert, getColumns){


	RETURN_MEMBER(getThis(), "_columns");
}

/**
 * Sets the values to insert
 *
 * @param array $values
 * @return Phalcon\Mvc\Model\Query\Builder\Insert
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Insert, values){

	zval *values, use_columnname = {}, *row;

	phalcon_fetch_params(0, 1, 0, &values);

	if (!zend_hash_num_elements(Z_ARRVAL_P(values))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_query_exception_ce, "Values must be not empty");
		RETURN_FALSE;
	}

	ZVAL_TRUE(&use_columnname);
	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(values), row) {
		if (Z_TYPE_P(row) != IS_ARRAY) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_query_exception_ce, "Value every row must be array");
			return;
		}

		if (!phalcon_array_is_associative(row, 1)) {
			ZVAL_FALSE(&use_columnname);
		}
	} ZEND_HASH_FOREACH_END();

	phalcon_update_property(getThis(), SL("_useColumnName"), &use_columnname);
	phalcon_update_property(getThis(), SL("_values"), values);
	RETURN_THIS();
}

/**
 * Gets the values to insert
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Insert, getValues){

	RETURN_MEMBER(getThis(), "_values");
}

/**
 * Sets conflict
 *
 * @param array $conflict
 * @return Phalcon\Mvc\Model\Query\Builder\Insert
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Insert, setConflict){

	zval *conflict;

	phalcon_fetch_params(0, 1, 0, &conflict);

	phalcon_update_property(getThis(), SL("_conflict"), conflict);
	RETURN_THIS();
}

/**
 * Returns a PHQL statement built based on the builder parameters
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Insert, _compile){

	zval table = {}, columns = {}, values = {}, phql = {}, joined_columns = {}, hidden_param = {}, use_columnname = {};
	zval *row = NULL, insert_rows = {}, joined_rows = {}, bind_params = {}, bind_types = {};
	zend_string *str_key;
	ulong idx;

	PHALCON_MM_INIT();

	PHALCON_MM_CALL_SELF(&table, "gettable");
	PHALCON_MM_ADD_ENTRY(&table);

	if (PHALCON_IS_EMPTY(&table)) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_query_exception_ce, "Table is required");
		return;
	}

	PHALCON_MM_CALL_SELF(&columns, "getcolumns");
	PHALCON_MM_ADD_ENTRY(&columns);

	if (Z_TYPE(columns) != IS_ARRAY || !zend_hash_num_elements(Z_ARRVAL(columns))) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_query_exception_ce, "Columns must be array");
		return;
	}

	PHALCON_MM_CALL_SELF(&values, "getvalues");
	PHALCON_MM_ADD_ENTRY(&values);

	if (Z_TYPE(values) != IS_ARRAY || !zend_hash_num_elements(Z_ARRVAL(values))) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_query_exception_ce, "Values must be array");
		return;
	}

	PHALCON_MM_CALL_SELF(&bind_params, "getbindparams");
	PHALCON_MM_ADD_ENTRY(&bind_params);
	if (Z_TYPE(bind_params) != IS_ARRAY) {
		array_init(&bind_params);
		PHALCON_MM_ADD_ENTRY(&bind_params);
	}

	PHALCON_CONCAT_SVS(&phql, "INSERT INTO [", &table, "]");

	phalcon_fast_join_str(&joined_columns, SL("], ["), &columns);
	PHALCON_SCONCAT_SVS(&phql, " ([", &joined_columns, "]) VALUES ");
	zval_ptr_dtor(&joined_columns);

	phalcon_read_property(&hidden_param, getThis(), SL("_hiddenParamNumber"), PH_READONLY);
	phalcon_read_property(&use_columnname, getThis(), SL("_useColumnName"), PH_READONLY);

	array_init(&insert_rows);
	PHALCON_MM_ADD_ENTRY(&insert_rows);

	if (zend_is_true(&use_columnname)) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(values), row) {
			zval *column, insert_values = {}, joined_values = {};

			array_init(&insert_values);

			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(columns), column) {
				zval key = {}, insert_value = {}, value = {};
				PHALCON_CONCAT_SVSV(&key, "phi_", &hidden_param, "_", column);
				PHALCON_CONCAT_SVS(&insert_value, " :", &key, ":");
				phalcon_array_append(&insert_values, &insert_value, 0);
				if (phalcon_array_isset_fetch(&value, row, column, PH_READONLY)) {
					phalcon_array_update(&bind_params, &key, &value, PH_COPY);
				} else {
					phalcon_array_update(&bind_params, &key, &PHALCON_GLOBAL(z_null), 0);
				}
				zval_ptr_dtor(&key);
			} ZEND_HASH_FOREACH_END();
			phalcon_increment(&hidden_param);
			phalcon_fast_join_str(&joined_values, SL(", "), &insert_values);
			zval_ptr_dtor(&insert_values);
			phalcon_array_append(&insert_rows, &joined_values, 0);
		} ZEND_HASH_FOREACH_END();
	} else {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(values), row) {
			zval insert_values = {}, *value, joined_values = {};

			array_init(&insert_values);
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(row), idx, str_key, value) {
				zval column = {}, key = {}, insert_value = {};
				if (str_key) {
					ZVAL_STR(&column, str_key);
				} else {
					ZVAL_LONG(&column, idx);
				}

				PHALCON_CONCAT_SVSV(&key, "phi_", &hidden_param, "_", &column);
				PHALCON_CONCAT_SVS(&insert_value, " :", &key, ":");

				phalcon_array_append(&insert_values, &insert_value, 0);
				phalcon_array_update(&bind_params, &key, value, PH_COPY);
				zval_ptr_dtor(&key);
			} ZEND_HASH_FOREACH_END();
			phalcon_increment(&hidden_param);
			phalcon_fast_join_str(&joined_values, SL(", "), &insert_values);
			zval_ptr_dtor(&insert_values);
			phalcon_array_append(&insert_rows, &joined_values, 0);
		} ZEND_HASH_FOREACH_END();
	}

	phalcon_fast_join_str(&joined_rows, SL("), ("), &insert_rows);

	PHALCON_SCONCAT_SVS(&phql, "(", &joined_rows, ")");
	zval_ptr_dtor(&joined_rows);
	PHALCON_MM_ADD_ENTRY(&phql);

	phalcon_update_property(getThis(), SL("_mergeBindParams"), &bind_params);

	PHALCON_MM_CALL_SELF(&bind_types, "getbindtypes");
	PHALCON_MM_ADD_ENTRY(&bind_types);
	phalcon_update_property(getThis(), SL("_mergeBindTypes"), &bind_types);


	phalcon_update_property(getThis(), SL("_phql"), &phql);
	RETURN_MM();
}
