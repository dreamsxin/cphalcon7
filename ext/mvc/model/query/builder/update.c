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

#include "mvc/model/query/builder/update.h"
#include "mvc/model/query/builder/where.h"
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
 * Phalcon\Mvc\Model\Query\Builder\Update
 *
 *<code>
 *$resultset = Phalcon\Mvc\Model\Query\Builder::createUpdateBuilder()
 *   ->table('Robots')
 *   ->set(array('name' => 'Google'))
 *   ->getQuery()
 *   ->execute();
 *</code>
 */
zend_class_entry *phalcon_mvc_model_query_builder_update_ce;

PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Update, __construct);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Update, table);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Update, getTable);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Update, set);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Update, getSet);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Update, setConflict);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Update, _compile);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_update___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, params, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_update_table, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, table, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_update_set, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, set, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_update_setconflict, 0, 0, 1)
	ZEND_ARG_INFO(0, conflict)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_query_builder_update_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Update, __construct, arginfo_phalcon_mvc_model_query_builder_update___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Update, table, arginfo_phalcon_mvc_model_query_builder_update_table, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Update, getTable, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Update, set, arginfo_phalcon_mvc_model_query_builder_update_set, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Update, getSet, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Update, setConflict, arginfo_phalcon_mvc_model_query_builder_update_setconflict, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Update, _compile, NULL, ZEND_ACC_PROTECTED)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Query\Builder\Update initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Query_Builder_Update){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\Query\\Builder, Update, mvc_model_query_builder_update, phalcon_mvc_model_query_builder_where_ce, phalcon_mvc_model_query_builder_update_method_entry, 0);

	zend_declare_property_long(phalcon_mvc_model_query_builder_update_ce, SL("_type"), PHQL_T_UPDATE, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_update_ce, SL("_table"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_update_ce, SL("_set"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_update_ce, SL("_conflict"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_mvc_model_query_builder_update_ce, 1, phalcon_mvc_model_query_builderinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Model\Query\Builder\Update constructor
 *
 * @param array $params
 * @param Phalcon\Di $dependencyInjector
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Update, __construct){

	zval *params = NULL, *dependency_injector = NULL;

	phalcon_fetch_params(0, 0, 2, &params, &dependency_injector);

	/**
	 * Update the dependency injector if any
	 */
	if (dependency_injector && Z_TYPE_P(dependency_injector) != IS_NULL) {
		PHALCON_CALL_METHOD(NULL, getThis(), "setdi", dependency_injector);
	}

	if (params && Z_TYPE_P(params) == IS_ARRAY) {
		zval conditions = {}, bind_params = {}, bind_types = {}, table = {}, index = {}, set = {};
		/**
		 * Process conditions
		 */
		if (phalcon_array_isset_fetch_str(&conditions, params, SL("conditions"), PH_READONLY)
			|| phalcon_array_isset_fetch_long(&conditions, params, 0, PH_READONLY)) {
			PHALCON_CALL_METHOD(NULL, getThis(), "setconditions", &conditions);
		}

		if (phalcon_array_isset_fetch_str(&bind_params, params, SL("bind"), PH_READONLY)) {
			PHALCON_CALL_METHOD(NULL, getThis(), "setbindparams", &bind_params);
		}

		if (phalcon_array_isset_fetch_str(&bind_types, params, SL("bindTypes"), PH_READONLY)) {
			PHALCON_CALL_METHOD(NULL, getThis(), "setbindtypes", &bind_types);
		}

		if (phalcon_array_isset_fetch_str(&table, params, SL("table"), PH_READONLY)) {
			PHALCON_CALL_METHOD(NULL, getThis(), "settable", &table);
		}

		if (phalcon_array_isset_fetch_str(&index, params, SL("index"), PH_READONLY)) {
			PHALCON_CALL_METHOD(NULL, getThis(), "setindex", &index);
		}

		if (phalcon_array_isset_fetch_str(&set, params, SL("set"), PH_READONLY)) {
			PHALCON_CALL_METHOD(NULL, getThis(), "set", &set);
		}
	}
}

/**
 * Sets the table to update
 *
 * @param string table
 * @return Phalcon\Mvc\Model\Query\Builder\Update
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Update, table){

	zval *table;

	phalcon_fetch_params(0, 1, 0, &table);

	phalcon_update_property(getThis(), SL("_table"), table);
	RETURN_THIS();
}

/**
 * Gets the table to update
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Update, getTable){


	RETURN_MEMBER(getThis(), "_table");
}

/**
 * Sets the values to update with an associative array
 *
 *<code>
 *	$builder->set(array('id' => 1, 'name' => 'Google'));
 *</code>
 *
 * @param array $set
 * @return Phalcon\Mvc\Model\Query\Builder\Update
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Update, set){

	zval *set;

	phalcon_fetch_params(0, 1, 0, &set);

	phalcon_update_property(getThis(), SL("_set"), set);
	RETURN_THIS();
}

/**
 * Return the values to update with an associative array
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Update, getSet){


	RETURN_MEMBER(getThis(), "_set");
}

/**
 * Sets conflict
 *
 * @param array $conflict
 * @return Phalcon\Mvc\Model\Query\Builder\Update
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Update, setConflict){

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
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Update, _compile){

	zval table = {}, conditions = {}, set = {}, phql = {}, *value = NULL, update_columns = {}, joined_columns = {}, bind_params = {}, bind_types = {};
	zend_string *str_key;
	ulong idx;

	PHALCON_MM_INIT();

	PHALCON_MM_CALL_SELF(&table, "gettable");
	PHALCON_MM_ADD_ENTRY(&table);

	PHALCON_MM_CALL_SELF(&conditions, "getconditions");
	PHALCON_MM_ADD_ENTRY(&conditions);

	PHALCON_MM_CALL_SELF(&set, "getset");
	PHALCON_MM_ADD_ENTRY(&set);

	PHALCON_CONCAT_SVS(&phql, "UPDATE [", &table, "] SET ");
	PHALCON_MM_ADD_ENTRY(&phql);

	if (Z_TYPE(set) != IS_ARRAY) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_query_exception_ce, "Values must be array");
		return;
	}

	PHALCON_MM_CALL_SELF(&bind_params, "getbindparams");
	PHALCON_MM_ADD_ENTRY(&bind_params);
	if (Z_TYPE(bind_params) != IS_ARRAY) {
		array_init(&bind_params);
		PHALCON_MM_ADD_ENTRY(&bind_params);
	}

	array_init(&update_columns);
	PHALCON_MM_ADD_ENTRY(&update_columns);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(set), idx, str_key, value) {
		zval column = {}, key = {}, update_column = {};
		if (str_key) {
			ZVAL_STR(&column, str_key);
		} else {
			ZVAL_LONG(&column, idx);
		}

		PHALCON_CONCAT_SV(&key, "phu_", &column);

		PHALCON_CONCAT_SVSVSVS(&update_column, "[", &table, "].", &column, " = :", &key, ":");
		phalcon_array_append(&update_columns, &update_column, 0);

		phalcon_array_update(&bind_params, &key, value, PH_COPY);
		zval_ptr_dtor(&key);
	} ZEND_HASH_FOREACH_END();

	phalcon_fast_join_str(&joined_columns, SL(", "), &update_columns);
	phalcon_concat_self(&phql, &joined_columns);
	zval_ptr_dtor(&joined_columns);
	PHALCON_MM_ADD_ENTRY(&phql);

	/**
	 * Only append conditions if it's string
	 */
	if (Z_TYPE(conditions) == IS_STRING && PHALCON_IS_NOT_EMPTY(&conditions)) {
		PHALCON_SCONCAT_SV(&phql, " WHERE ", &conditions);
		PHALCON_MM_ADD_ENTRY(&phql);
	}

	phalcon_update_property(getThis(), SL("_mergeBindParams"), &bind_params);

	PHALCON_MM_CALL_SELF(&bind_types, "getbindtypes");
	PHALCON_MM_ADD_ENTRY(&bind_types);
	phalcon_update_property(getThis(), SL("_mergeBindTypes"), &bind_types);

	phalcon_update_property(getThis(), SL("_phql"), &phql);
	RETURN_MM();
}
