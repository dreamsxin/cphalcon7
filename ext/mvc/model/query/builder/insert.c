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

static const zend_function_entry phalcon_mvc_model_query_builder_insert_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Insert, __construct, arginfo_phalcon_mvc_model_query_builder_insert___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Insert, table, arginfo_phalcon_mvc_model_query_builder_insert_table, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Insert, getTable, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Insert, columns, arginfo_phalcon_mvc_model_query_builder_insert_columns, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Insert, getColumns, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Insert, values, arginfo_phalcon_mvc_model_query_builder_insert_values, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Insert, getValues, NULL, ZEND_ACC_PUBLIC)
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
	zend_declare_property_null(phalcon_mvc_model_query_builder_insert_ce, SL("_values"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_mvc_model_query_builder_insert_ce, 1, phalcon_mvc_model_query_builderinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Model\Query\Builder\Insert constructor
 *
 * @param array $params
 * @param Phalcon\DI $dependencyInjector
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Insert, __construct){

	zval *params = NULL, *dependency_injector = NULL, table = {}, columns = {}, values = {};

	phalcon_fetch_params(0, 0, 2, &params, &dependency_injector);

	/** 
	 * Update the dependency injector if any
	 */
	if (dependency_injector && Z_TYPE_P(dependency_injector) != IS_NULL) {
		PHALCON_CALL_METHODW(NULL, getThis(), "setdi", dependency_injector);
	}

	if (params && Z_TYPE_P(params) == IS_ARRAY) {
		if (phalcon_array_isset_fetch_str(&table, params, SL("table"))) {
			PHALCON_CALL_METHODW(NULL, getThis(), "settable", &table);
		}

		if (phalcon_array_isset_fetch_str(&columns, params, SL("columns"))) {
			PHALCON_CALL_METHODW(NULL, getThis(), "setcolumns", &columns);
		}

		if (phalcon_array_isset_fetch_str(&values, params, SL("values"))) {
			PHALCON_CALL_METHODW(NULL, getThis(), "setvalues", &values);
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

	phalcon_update_property_zval(getThis(), SL("_table"), table);
	RETURN_THISW();
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

	zval *columns;

	phalcon_fetch_params(0, 1, 0, &columns);

	phalcon_update_property_zval(getThis(), SL("_columns"), columns);
	RETURN_THISW();
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

	zval *values;

	phalcon_fetch_params(0, 1, 0, &values);

	phalcon_update_property_zval(getThis(), SL("_values"), values);
	RETURN_THISW();
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
 * Returns a PHQL statement built based on the builder parameters
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Insert, _compile){

	zval table = {}, columns = {}, values = {}, phql = {}, joined_columns = {}, hidden_param = {}, *row = NULL, insert_rows = {}, joined_rows = {};
	zval bind_params = {}, bind_types = {};
	zend_string *str_key;
	ulong idx;

	PHALCON_CALL_SELFW(&table, "gettable");
	PHALCON_CALL_SELFW(&columns, "getcolumns");
	PHALCON_CALL_SELFW(&values, "getvalues");

	if (Z_TYPE(columns) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_query_exception_ce, "Columns must be array");
		return;
	}

	if (Z_TYPE(values) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_query_exception_ce, "Values must be array");
		return;
	}

	PHALCON_CALL_SELFW(&bind_params, "getbindparams");
	if (Z_TYPE(bind_params) != IS_ARRAY) {
		array_init(&bind_params);
	}

	PHALCON_CONCAT_SVS(&phql, "INSERT INTO [", &table, "]");

	phalcon_fast_join_str(&joined_columns, SL("], ["), &columns);

	PHALCON_SCONCAT_SVS(&phql, " ([", &joined_columns, "]) VALUES ");

	phalcon_read_property(&hidden_param, getThis(), SL("_hiddenParamNumber"), PH_READONLY);

	array_init(&insert_rows);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(values), row) {
		zval insert_values = {}, *value, joined_values = {};

		if (Z_TYPE_P(row) != IS_ARRAY) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_query_exception_ce, "Value every row must be array");
			return;
		}
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
			
			phalcon_array_append(&insert_values, &insert_value, PH_COPY);
			phalcon_array_update_zval(&bind_params, &key, value, PH_COPY);
		} ZEND_HASH_FOREACH_END();
		phalcon_increment(&hidden_param);
		phalcon_fast_join_str(&joined_values, SL(", "), &insert_values);
		phalcon_array_append(&insert_rows, &joined_values, PH_COPY);
	} ZEND_HASH_FOREACH_END();

	phalcon_fast_join_str(&joined_rows, SL("), ("), &insert_rows);

	PHALCON_SCONCAT_SVS(&phql, "(", &joined_rows, ")");

	phalcon_update_property_zval(getThis(), SL("_mergeBindParams"), &bind_params);

	PHALCON_CALL_SELFW(&bind_types, "getbindtypes");
	phalcon_update_property_zval(getThis(), SL("_mergeBindTypes"), &bind_types);

	phalcon_update_property_zval(getThis(), SL("_phql"), &phql);
}
