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

#include "mvc/model/query/builder/select.h"
#include "mvc/model/query/builder.h"
#include "mvc/model/query/builderinterface.h"
#include "mvc/model/exception.h"
#include "mvc/model/metadatainterface.h"
#include "mvc/model/metadata/memory.h"
#include "mvc/model/query.h"
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
 * Phalcon\Mvc\Model\Query\Builder\Select
 *
 *<code>
 *$resultset = $this->modelsManager->createBuilder()
 *   ->from('Robots')
 *   ->join('RobotsParts')
 *   ->limit(20)
 *   ->orderBy('Robots.name')
 *   ->getQuery()
 *   ->execute();
 *</code>
 */
zend_class_entry *phalcon_mvc_model_query_builder_select_ce;

PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, __construct);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, distinct);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, getDistinct);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, columns);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, getColumns);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, from);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, addFrom);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, getFrom);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, join);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, innerJoin);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, leftJoin);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, rightJoin);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, where);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, andWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, orWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, betweenWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, notBetweenWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, inWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, notInWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, getWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, orderBy);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, getOrderBy);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, having);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, getHaving);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, limit);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, getLimit);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, offset);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, getOffset);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, groupBy);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, getGroupBy);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, _compile);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select_distinct, 0, 0, 1)
	ZEND_ARG_INFO(0, distinct)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select_columns, 0, 0, 1)
	ZEND_ARG_INFO(0, columns)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select_from, 0, 0, 1)
	ZEND_ARG_INFO(0, models)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select_addfrom, 0, 0, 1)
	ZEND_ARG_INFO(0, model)
	ZEND_ARG_INFO(0, alias)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select_join, 0, 0, 1)
	ZEND_ARG_INFO(0, model)
	ZEND_ARG_INFO(0, conditions)
	ZEND_ARG_INFO(0, alias)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select_innerjoin, 0, 0, 1)
	ZEND_ARG_INFO(0, model)
	ZEND_ARG_INFO(0, conditions)
	ZEND_ARG_INFO(0, alias)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select_leftjoin, 0, 0, 1)
	ZEND_ARG_INFO(0, model)
	ZEND_ARG_INFO(0, conditions)
	ZEND_ARG_INFO(0, alias)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select_rightjoin, 0, 0, 1)
	ZEND_ARG_INFO(0, model)
	ZEND_ARG_INFO(0, conditions)
	ZEND_ARG_INFO(0, alias)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select_orderby, 0, 0, 1)
	ZEND_ARG_INFO(0, orderBy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select_having, 0, 0, 1)
	ZEND_ARG_INFO(0, having)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select_limit, 0, 0, 1)
	ZEND_ARG_INFO(0, limit)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select_offset, 0, 0, 1)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select_groupby, 0, 0, 1)
	ZEND_ARG_INFO(0, group)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_query_builder_select_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, __construct, arginfo_phalcon_mvc_model_query_builder_select___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, distinct, arginfo_phalcon_mvc_model_query_builder_select_distinct, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, getDistinct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, columns, arginfo_phalcon_mvc_model_query_builder_select_columns, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, getColumns, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, from, arginfo_phalcon_mvc_model_query_builder_select_from, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, addFrom, arginfo_phalcon_mvc_model_query_builder_select_addfrom, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, getFrom, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, join, arginfo_phalcon_mvc_model_query_builder_select_join, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, innerJoin, arginfo_phalcon_mvc_model_query_builder_select_innerjoin, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, leftJoin, arginfo_phalcon_mvc_model_query_builder_select_leftjoin, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, rightJoin, arginfo_phalcon_mvc_model_query_builder_select_rightjoin, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, orderBy, arginfo_phalcon_mvc_model_query_builder_select_orderby, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, getOrderBy, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, having, arginfo_phalcon_mvc_model_query_builder_select_having, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, getHaving, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, limit, arginfo_phalcon_mvc_model_query_builder_select_limit, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, getLimit, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, offset, arginfo_phalcon_mvc_model_query_builder_select_offset, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, getOffset, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, groupBy, arginfo_phalcon_mvc_model_query_builder_select_groupby, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, getGroupBy, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, _compile, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Query\Builder\Select initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Query_Builder_Select){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\Query\\Builder, Select, mvc_model_query_builder_select, phalcon_mvc_model_query_builder_ce, phalcon_mvc_model_query_builder_select_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_columns"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_models"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_joins"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_conditions"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_group"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_having"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_order"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_limit"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_offset"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_forUpdate"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_sharedLock"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_bindParams"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_bindTypes"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_distinct"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_mvc_model_query_builder_select_ce, SL("_hiddenParamNumber"), 0, ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_mvc_model_query_builder_select_ce, 1, phalcon_mvc_model_query_builderinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Model\Query\Builder\Select constructor
 *
 *<code>
 * $params = array(
 *    'models'     => array('Users'),
 *    'columns'    => array('id', 'name', 'status'),
 *    'conditions' => array(
 *        array(
 *            "created > :min: AND created < :max:",
 *            array("min" => '2013-01-01',   'max' => '2014-01-01'),
 *            array("min" => PDO::PARAM_STR, 'max' => PDO::PARAM_STR),
 *        ),
 *    ),
 *    // or 'conditions' => "created > '2013-01-01' AND created < '2014-01-01'",
 *    'group'      => array('id', 'name'),
 *    'having'     => "name = 'Kamil'",
 *    'order'      => array('name', 'id'),
 *    'limit'      => 20,
 *    'offset'     => 20,
 *    // or 'limit' => array(20, 20),
 *);
 *$queryBuilder = new Phalcon\Mvc\Model\Query\Builder\Select($params);
 *</code> 
 *
 * @param array $params
 * @param Phalcon\DI $dependencyInjector
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, __construct){

	zval *params = NULL, *dependency_injector = NULL, conditions = {}, *single_condition_array;
	zval merged_conditions = {}, merged_bind_params = {}, merged_bind_types = {}, new_condition_string = {};
	zval current_bind_params = {}, current_bind_types = {}, bind_params = {}, bind_types = {}, models = {}, columns = {}, group_clause = {}, joins = {};
	zval having_clause = {}, order_clause = {}, limit_clause = {}, offset_clause = {}, limit = {}, offset = {}, for_update = {}, shared_lock = {};
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 0, 2, &params, &dependency_injector);

	/** 
	 * Update the dependency injector if any
	 */
	if (dependency_injector && Z_TYPE_P(dependency_injector) != IS_NULL) {
		PHALCON_CALL_METHODW(NULL, getThis(), "setdi", dependency_injector);
	}

	if (params && Z_TYPE_P(params) == IS_ARRAY) {
		/** 
		 * Process conditions
		 */
		if (phalcon_array_isset_fetch_str(&conditions, params, SL("conditions")) || phalcon_array_isset_fetch_long(&conditions, params, 0)) {
			if (Z_TYPE(conditions) == IS_ARRAY) {

				/* ----------- INITIALIZING LOOP VARIABLES ----------- */

				/*
				 * array containing single condition for example:
				 * array(
				 *      'status = :status:',
				 *      array('status' => 5),
				 *      array('status' => PDO::PARAM_INT),
				 * )
				 */
				array_init(&merged_conditions);
				array_init(&merged_bind_params);
				array_init(&merged_bind_types);

				ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(conditions), idx, str_key, single_condition_array) {
					zval single_condition_key = {}, condition_string = {}, tmp_bind_params = {}, tmp_bind_types = {};
					if (str_key) {
						ZVAL_STR(&single_condition_key, str_key);
					} else {
						ZVAL_LONG(&single_condition_key, idx);
					}
					if (Z_TYPE_P(single_condition_array) == IS_ARRAY
						&& phalcon_array_isset_fetch_long(&condition_string, single_condition_array, 0)
						&& phalcon_array_isset_fetch_long(&tmp_bind_params, single_condition_array, 1)
						&& Z_TYPE(condition_string) == IS_STRING
						&& Z_TYPE(tmp_bind_params) == IS_ARRAY
					) {	
						phalcon_array_update_zval(&merged_conditions, &condition_string, &condition_string, PH_COPY);
						phalcon_array_merge_recursive_n(&merged_bind_params, &tmp_bind_params);

						if (phalcon_array_isset_fetch_long(&tmp_bind_types, single_condition_array, 2) && Z_TYPE(tmp_bind_types) == IS_ARRAY) {
							phalcon_array_merge_recursive_n(&merged_bind_types, &tmp_bind_types);
						}
					} else if (Z_TYPE(single_condition_key) == IS_STRING) {
						PHALCON_CONCAT_VSVS(&new_condition_string, &single_condition_key, " = :", &single_condition_key, ":");

						phalcon_array_update_zval(&merged_conditions, &single_condition_key, &new_condition_string, PH_COPY);

						if (Z_TYPE_P(single_condition_array) == IS_ARRAY) {
							phalcon_array_merge_recursive_n(&merged_bind_params, single_condition_array);
						} else {
							phalcon_array_update_zval(&merged_bind_params, &single_condition_key, single_condition_array, PH_COPY);
						}
					}
				} ZEND_HASH_FOREACH_END();

				phalcon_fast_join_str(&new_condition_string, SL(" AND "), &merged_conditions);

				phalcon_update_property_zval(getThis(), SL("_conditions"), &new_condition_string);
				phalcon_update_property_zval(getThis(), SL("_bindParams"), &merged_bind_params);
				phalcon_update_property_zval(getThis(), SL("_bindTypes"), &merged_bind_types);
			} else {
				phalcon_update_property_zval(getThis(), SL("_conditions"), &conditions);		
			}
		}

		if (phalcon_array_isset_fetch_str(&bind_params, params, SL("bind"))) {
			if (Z_TYPE(bind_params) == IS_ARRAY) {
				phalcon_read_property(&current_bind_params, getThis(), SL("_bindParams"), PH_NOISY);
				if (Z_TYPE(current_bind_params) == IS_ARRAY) {
					phalcon_add_function(&merged_bind_params, &bind_params, &current_bind_params);
					phalcon_update_property_zval(getThis(), SL("_bindParams"), &merged_bind_params);
				} else {
					phalcon_update_property_zval(getThis(), SL("_bindParams"), &bind_params);
				}
			}
		}

		if (phalcon_array_isset_fetch_str(&bind_types, params, SL("bindTypes"))) {
			if (Z_TYPE(bind_types) == IS_ARRAY) {
				phalcon_read_property(&current_bind_types, getThis(), SL("_bindTypes"), PH_NOISY);
				if (Z_TYPE(current_bind_types) == IS_ARRAY) {
					phalcon_add_function(&merged_bind_types, &bind_types, &current_bind_types);
					phalcon_update_property_zval(getThis(), SL("_bindTypes"), &merged_bind_types);
				} else {
					phalcon_update_property_zval(getThis(), SL("_bindTypes"), &bind_types);
				}
			}
		}

		/** 
		 * Assign 'FROM' clause
		 */
		if (phalcon_array_isset_fetch_str(&models, params, SL("models"))) {
			phalcon_update_property_zval(getThis(), SL("_models"), &models);
		}

		/** 
		 * Assign COLUMNS clause
		 */
		if (phalcon_array_isset_fetch_str(&columns, params, SL("columns"))) {
			phalcon_update_property_zval(getThis(), SL("_columns"), &columns);
		}

		/**
		 * Assign JOIN clause
		 */
		if (phalcon_array_isset_fetch_str(&joins, params, SL("joins"))) {
			phalcon_update_property_zval(getThis(), SL("_joins"), &joins);
		}

		/** 
		 * Assign GROUP clause
		 */
		if (phalcon_array_isset_fetch_str(&group_clause, params, SL("group"))) {
			phalcon_update_property_zval(getThis(), SL("_group"), &group_clause);
		}

		/** 
		 * Assign HAVING clause
		 */
		if (phalcon_array_isset_fetch_str(&having_clause, params, SL("having"))) {
			phalcon_update_property_zval(getThis(), SL("_having"), &having_clause);
		}

		/** 
		 * Assign ORDER clause
		 */
		if (phalcon_array_isset_fetch_str(&order_clause, params, SL("order"))) {
			phalcon_update_property_zval(getThis(), SL("_order"), &order_clause);
		}

		/** 
		 * Assign LIMIT clause
		 */
		if (phalcon_array_isset_fetch_str(&limit_clause, params, SL("limit"))) {
			if (Z_TYPE(limit_clause) == IS_ARRAY
				&& phalcon_array_isset_fetch_long(&limit, &limit_clause, 0)
				&& phalcon_array_isset_fetch_long(&offset, &limit_clause, 1)
			) {
				phalcon_update_property_zval(getThis(), SL("_limit"), &limit);
				phalcon_update_property_zval(getThis(), SL("_offset"), &offset);
			} else {
				phalcon_update_property_zval(getThis(), SL("_limit"), &limit_clause);
			}
		}

		/** 
		 * Assign OFFSET clause
		 */
		if (phalcon_array_isset_fetch_str(&offset_clause, params, SL("offset"))) {
			phalcon_update_property_zval(getThis(), SL("_offset"), &offset_clause);
		}

		/** 
		 * Assign FOR UPDATE clause
		 */
		if (phalcon_array_isset_fetch_str(&for_update, params, SL("for_update"))) {
			phalcon_update_property_zval(getThis(), SL("_forUpdate"), &for_update);
		}

		/** 
		 * Assign SHARED LOCK clause
		 */
		if (phalcon_array_isset_fetch_str(&shared_lock, params, SL("shared_lock"))) {
			phalcon_update_property_zval(getThis(), SL("_sharedLock"), &shared_lock);
		}
	}
}

/**
 * Sets SELECT DISTINCT / SELECT ALL flag
 *
 * @param bool|null distinct
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, distinct){

	zval *distinct;

	phalcon_fetch_params(0, 1, 0, &distinct);

	if (Z_TYPE_P(distinct) != IS_NULL && !PHALCON_IS_BOOL(distinct)) {
		PHALCON_ENSURE_IS_BOOL(distinct);
	}

	phalcon_update_property_zval(getThis(), SL("_distinct"), distinct);
	RETURN_THISW();
}

/**
 * Returns SELECT DISTINCT / SELECT ALL flag
 *
 * @return bool
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, getDistinct){


	RETURN_MEMBER(getThis(), "_distinct");
}

/**
 * Sets the columns to be queried
 *
 *<code>
 *	$builder->columns(array('id', 'name'));
 *</code>
 *
 * @param string|array $columns
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, columns){

	zval *columns;

	phalcon_fetch_params(0, 1, 0, &columns);

	phalcon_update_property_zval(getThis(), SL("_columns"), columns);
	RETURN_THISW();
}

/**
 * Return the columns to be queried
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, getColumns){


	RETURN_MEMBER(getThis(), "_columns");
}

/**
 * Sets the models who makes part of the query
 *
 *<code>
 *	$builder->from('Robots');
 *	$builder->from(array('Robots', 'RobotsParts'));
 *</code>
 *
 * @param string|array $models
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, from){

	zval *models;

	phalcon_fetch_params(0, 1, 0, &models);

	phalcon_update_property_zval(getThis(), SL("_models"), models);
	RETURN_THISW();
}

/**
 * Add a model to take part of the query
 *
 *<code>
 *	$builder->addFrom('Robots', 'r');
 *</code>
 *
 * @param string $model
 * @param string $alias
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, addFrom){

	zval *model, *alias = NULL, models = {}, current_model = {};

	phalcon_fetch_params(0, 1, 1, &model, &alias);

	if (!alias) {
		alias = &PHALCON_GLOBAL(z_null);
	}

	phalcon_return_property(&models, getThis(), SL("_models"));
	if (Z_TYPE(models) != IS_ARRAY) { 
		if (Z_TYPE(models) != IS_NULL) {
			PHALCON_CPY_WRT(&current_model, &models);

			array_init(&models);
			phalcon_array_append(&models, &current_model, PH_COPY);
		} else {
			array_init(&models);
		}
	} else {
		SEPARATE_ZVAL(&models);
	}

	if (Z_TYPE_P(alias) == IS_STRING) {
		phalcon_array_update_zval(&models, alias, model, PH_COPY);
	} else {
		phalcon_array_append(&models, model, PH_COPY);
	}

	phalcon_update_property_zval(getThis(), SL("_models"), &models);

	RETURN_THISW();
}

/**
 * Return the models who makes part of the query
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, getFrom){


	RETURN_MEMBER(getThis(), "_models");
}

/**
 * Adds a join to the query
 *
 *<code>
 *	$builder->join('Robots');
 *	$builder->join('Robots', 'r.id = RobotsParts.robots_id');
 *	$builder->join('Robots', 'r.id = RobotsParts.robots_id', 'r');
 *	$builder->join('Robots', 'r.id = RobotsParts.robots_id', 'r', 'LEFT');
 *</code>
 *
 * @param string $model
 * @param string $conditions
 * @param string $alias
 * @param string $type
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, join){

	zval *model, *conditions = NULL, *alias = NULL, *type = NULL, join = {};

	phalcon_fetch_params(0, 1, 3, &model, &conditions, &alias, &type);

	if (!conditions) {
		conditions = &PHALCON_GLOBAL(z_null);
	}

	if (!alias) {
		alias = &PHALCON_GLOBAL(z_null);
	}

	if (!type) {
		type = &PHALCON_GLOBAL(z_null);
	}

	array_init_size(&join, 4);
	phalcon_array_append(&join, model, PH_COPY);
	phalcon_array_append(&join, conditions, PH_COPY);
	phalcon_array_append(&join, alias, PH_COPY);
	phalcon_array_append(&join, type, PH_COPY);
	phalcon_update_property_array_append(getThis(), SL("_joins"), &join);
	RETURN_THISW();
}

/**
 * Adds a INNER join to the query
 *
 *<code>
 *	$builder->innerJoin('Robots');
 *	$builder->innerJoin('Robots', 'r.id = RobotsParts.robots_id');
 *	$builder->innerJoin('Robots', 'r.id = RobotsParts.robots_id', 'r');
 *</code>
 *
 * @param string $model
 * @param string $conditions
 * @param string $alias
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, innerJoin){

	zval *model, *conditions = NULL, *alias = NULL, type = {}, join = {};

	phalcon_fetch_params(0, 1, 2, &model, &conditions, &alias);

	if (!conditions) {
		conditions = &PHALCON_GLOBAL(z_null);
	}

	if (!alias) {
		alias = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_STR(&type, "INNER");

	array_init_size(&join, 4);
	phalcon_array_append(&join, model, PH_COPY);
	phalcon_array_append(&join, conditions, PH_COPY);
	phalcon_array_append(&join, alias, PH_COPY);
	phalcon_array_append(&join, &type, PH_COPY);
	phalcon_update_property_array_append(getThis(), SL("_joins"), &join);
	RETURN_THISW();
}

/**
 * Adds a LEFT join to the query
 *
 *<code>
 *	$builder->leftJoin('Robots', 'r.id = RobotsParts.robots_id', 'r');
 *</code>
 *
 * @param string $model
 * @param string $conditions
 * @param string $alias
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, leftJoin){

	zval *model, *conditions = NULL, *alias = NULL, type = {}, join = {};

	phalcon_fetch_params(0, 1, 2, &model, &conditions, &alias);

	if (!conditions) {
		conditions = &PHALCON_GLOBAL(z_null);
	}

	if (!alias) {
		alias = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_STR(&type, "LEFT");

	array_init_size(&join, 4);
	phalcon_array_append(&join, model, PH_COPY);
	phalcon_array_append(&join, conditions, PH_COPY);
	phalcon_array_append(&join, alias, PH_COPY);
	phalcon_array_append(&join, &type, PH_COPY);
	phalcon_update_property_array_append(getThis(), SL("_joins"), &join);
	RETURN_THISW();
}

/**
 * Adds a RIGHT join to the query
 *
 *<code>
 *	$builder->rightJoin('Robots', 'r.id = RobotsParts.robots_id', 'r');
 *</code>
 *
 * @param string $model
 * @param string $conditions
 * @param string $alias
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, rightJoin){

	zval *model, *conditions = NULL, *alias = NULL, type = {}, join = {};

	phalcon_fetch_params(0, 1, 2, &model, &conditions, &alias);

	if (!conditions) {
		conditions = &PHALCON_GLOBAL(z_null);
	}

	if (!alias) {
		alias = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_STR(&type, "RIGHT");

	array_init_size(&join, 4);
	phalcon_array_append(&join, model, PH_COPY);
	phalcon_array_append(&join, conditions, PH_COPY);
	phalcon_array_append(&join, alias, PH_COPY);
	phalcon_array_append(&join, &type, PH_COPY);
	phalcon_update_property_zval(getThis(), SL("_joins"), &join);
	RETURN_THISW();
}

/**
 * Sets the query conditions
 *
 *<code>
 *	$builder->where('name = "Peter"');
 *	$builder->where('name = :name: AND id > :id:', array('name' => 'Peter', 'id' => 100));
 *</code>
 *
 * @param string $conditions
 * @param array $bindParams
 * @param array $bindTypes
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, where){

	zval *conditions, *bind_params = NULL, *bind_types = NULL;

	phalcon_fetch_params(0, 1, 2, &conditions, &bind_params, &bind_types);

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	phalcon_update_property_zval(getThis(), SL("_conditions"), conditions);

	/** 
	 * Override the bind params and bind types to the current ones
	 */
	phalcon_update_property_zval(getThis(), SL("_bindParams"), bind_params);
	phalcon_update_property_zval(getThis(), SL("_bindTypes"), bind_types);

	RETURN_THISW();
}

/**
 * Appends a condition to the current conditions using a AND operator
 *
 *<code>
 *	$builder->andWhere('name = "Peter"');
 *	$builder->andWhere('name = :name: AND id > :id:', array('name' => 'Peter', 'id' => 100));
 *</code>
 *
 * @param string $conditions
 * @param array $bindParams
 * @param array $bindTypes
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, andWhere){

	zval *conditions, *bind_params = NULL, *bind_types = NULL, current_conditions = {}, new_conditions = {}, current_bind_params = {}, merged_params = {};
	zval current_bind_types = {}, merged_types = {};

	phalcon_fetch_params(0, 1, 2, &conditions, &bind_params, &bind_types);

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_SELFW(&current_conditions, "getConditions");

	/** 
	 * Nest the condition to current ones or set as unique
	 */
	if (zend_is_true(&current_conditions)) {
		PHALCON_CONCAT_SVSVS(&new_conditions, "(", &current_conditions, ") AND (", conditions, ")");
	} else {
		PHALCON_CPY_WRT(&new_conditions, conditions);
	}

	phalcon_update_property_zval(getThis(), SL("_conditions"), &new_conditions);

	/** 
	 * Merge the bind params to the current ones
	 */
	if (Z_TYPE_P(bind_params) == IS_ARRAY) {
		phalcon_read_property(&current_bind_params, getThis(), SL("_bindParams"), PH_NOISY);
		if (Z_TYPE(current_bind_params) == IS_ARRAY) { 
			phalcon_add_function(&merged_params, bind_params, &current_bind_params);
		} else {
			PHALCON_CPY_WRT(&merged_params, bind_params);
		}

		phalcon_update_property_zval(getThis(), SL("_bindParams"), &merged_params);
	}

	/** 
	 * Merge the bind types to the current ones
	 */
	if (Z_TYPE_P(bind_types) == IS_ARRAY) {
		phalcon_read_property(&current_bind_types, getThis(), SL("_bindTypes"), PH_NOISY);
		if (Z_TYPE(current_bind_types) == IS_ARRAY) { 
			phalcon_add_function(&merged_params, bind_types, &current_bind_types);
		} else {
			PHALCON_CPY_WRT(&merged_types, bind_types);
		}

		phalcon_update_property_zval(getThis(), SL("_bindTypes"), &merged_types);
	}

	RETURN_THISW();
}

/**
 * Appends a condition to the current conditions using a OR operator
 *
 *<code>
 *	$builder->orWhere('name = "Peter"');
 *	$builder->orWhere('name = :name: AND id > :id:', array('name' => 'Peter', 'id' => 100));
 *</code>
 *
 * @param string $conditions
 * @param array $bindParams
 * @param array $bindTypes
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, orWhere){

	zval *conditions, *bind_params = NULL, *bind_types = NULL, current_conditions = {}, new_conditions = {}, current_bind_params = {}, merged_params = {};
	zval current_bind_types = {}, merged_types = {};

	phalcon_fetch_params(0, 1, 2, &conditions, &bind_params, &bind_types);

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_SELFW(&current_conditions, "getConditions");

	/** 
	 * Nest the condition to current ones or set as unique
	 */
	if (zend_is_true(&current_conditions)) {
		PHALCON_CONCAT_SVSVS(&new_conditions, "(", &current_conditions, ") OR (", conditions, ")");
	} else {
		PHALCON_CPY_WRT(&new_conditions, conditions);
	}

	phalcon_update_property_zval(getThis(), SL("_conditions"), &new_conditions);

	/** 
	 * Merge the bind params to the current ones
	 */
	if (Z_TYPE_P(bind_params) == IS_ARRAY) {
		phalcon_read_property(&current_bind_params, getThis(), SL("_bindParams"), PH_NOISY);
		if (Z_TYPE(current_bind_params) == IS_ARRAY) { 
			phalcon_add_function(&merged_params, bind_params, &current_bind_params);
		} else {
			PHALCON_CPY_WRT(&merged_params, bind_params);
		}

		phalcon_update_property_zval(getThis(), SL("_bindParams"), &merged_params);
	}

	/** 
	 * Merge the bind types to the current ones
	 */
	if (Z_TYPE_P(bind_types) == IS_ARRAY) {
		phalcon_read_property(&current_bind_types, getThis(), SL("_bindTypes"), PH_NOISY);
		if (Z_TYPE(current_bind_types) == IS_ARRAY) {
			phalcon_add_function(&merged_types, bind_types, &current_bind_types);
		} else {
			PHALCON_CPY_WRT(&merged_types, bind_types);
		}

		phalcon_update_property_zval(getThis(), SL("_bindTypes"), &merged_types);
	}

	RETURN_THISW();
}

/**
 * Appends a BETWEEN condition to the current conditions
 *
 *<code>
 *	$builder->betweenWhere('price', 100.25, 200.50);
 *</code>
 *
 * @param string $expr
 * @param mixed $minimum
 * @param mixed $maximum
 * @param boolean $useOrWhere
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, betweenWhere){

	zval *expr, *minimum, *maximum, *use_orwhere = NULL, hidden_param = {}, next_hidden_param = {}, minimum_key = {}, maximum_key = {}, conditions = {}, bind_params = {};

	phalcon_fetch_params(0, 3, 1, &expr, &minimum, &maximum, &use_orwhere);

	if (!use_orwhere) {
		use_orwhere = &PHALCON_GLOBAL(z_false);
	}

	phalcon_read_property(&hidden_param, getThis(), SL("_hiddenParamNumber"), PH_NOISY);
	phalcon_add_function(&next_hidden_param, &hidden_param, &PHALCON_GLOBAL(z_one));

	/** 
	 * Minimum key with auto bind-params
	 */
	PHALCON_CONCAT_SV(&minimum_key, "phb", &hidden_param);

	/** 
	 * Maximum key with auto bind-params
	 */
	PHALCON_CONCAT_SV(&maximum_key, "phb", &next_hidden_param);

	/** 
	 * Create a standard BETWEEN condition with bind params
	 */
	PHALCON_CONCAT_VSVSVS(&conditions, expr, " BETWEEN :", &minimum_key, ": AND :", &maximum_key, ":");

	array_init_size(&bind_params, 2);
	phalcon_array_update_zval(&bind_params, &minimum_key, minimum, PH_COPY);
	phalcon_array_update_zval(&bind_params, &maximum_key, maximum, PH_COPY);

	/** 
	 * Append the BETWEEN to the current conditions using and 'and'
	 */
	if (zend_is_true(use_orwhere)) {
		PHALCON_CALL_METHODW(NULL, getThis(), "orwhere", &conditions, &bind_params);
	} else {
		PHALCON_CALL_METHODW(NULL, getThis(), "andwhere", &conditions, &bind_params);
	}

	phalcon_increment(&next_hidden_param);
	phalcon_update_property_zval(getThis(), SL("_hiddenParamNumber"), &next_hidden_param);
	RETURN_THISW();
}

/**
 * Appends a NOT BETWEEN condition to the current conditions
 *
 *<code>
 *	$builder->notBetweenWhere('price', 100.25, 200.50);
 *</code>
 *
 * @param string $expr
 * @param mixed $minimum
 * @param mixed $maximum
 * @param boolean $useOrWhere
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, notBetweenWhere){

	zval *expr, *minimum, *maximum, *use_orwhere = NULL, hidden_param = {}, next_hidden_param = {}, minimum_key = {}, maximum_key = {}, conditions = {}, bind_params = {};

	phalcon_fetch_params(0, 3, 1, &expr, &minimum, &maximum, &use_orwhere);

	if (!use_orwhere) {
		use_orwhere = &PHALCON_GLOBAL(z_false);
	}

	phalcon_read_property(&hidden_param, getThis(), SL("_hiddenParamNumber"), PH_NOISY);
	phalcon_add_function(&next_hidden_param, &hidden_param, &PHALCON_GLOBAL(z_one));

	/** 
	 * Minimum key with auto bind-params
	 */
	PHALCON_CONCAT_SV(&minimum_key, "phb", &hidden_param);

	/** 
	 * Maximum key with auto bind-params
	 */
	PHALCON_CONCAT_SV(&maximum_key, "phb", &next_hidden_param);

	/** 
	 * Create a standard BETWEEN condition with bind params
	 */
	PHALCON_CONCAT_VSVSVS(&conditions, expr, " NOT BETWEEN :", &minimum_key, ": AND :", &maximum_key, ":");

	array_init_size(&bind_params, 2);
	phalcon_array_update_zval(&bind_params, &minimum_key, minimum, PH_COPY);
	phalcon_array_update_zval(&bind_params, &maximum_key, maximum, PH_COPY);

	/** 
	 * Append the BETWEEN to the current conditions using and 'and'
	 */
	if (zend_is_true(use_orwhere)) {
		PHALCON_CALL_METHODW(NULL, getThis(), "orwhere", &conditions, &bind_params);
	} else {
		PHALCON_CALL_METHODW(NULL, getThis(), "andwhere", &conditions, &bind_params);
	}

	phalcon_increment(&next_hidden_param);
	phalcon_update_property_zval(getThis(), SL("_hiddenParamNumber"), &next_hidden_param);
	RETURN_THISW();
}

/**
 * Appends an IN condition to the current conditions
 *
 *<code>
 *	$builder->inWhere('id', [1, 2, 3]);
 *</code>
 *
 * @param string $expr
 * @param array $values
 * @param boolean $useOrWhere
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, inWhere){

	zval *expr, *values, *use_orwhere = NULL, hidden_param = {}, bind_params = {}, bind_keys = {}, *value, joined_keys = {}, conditions = {};

	phalcon_fetch_params(0, 2, 1, &expr, &values, &use_orwhere);

	if (!use_orwhere) {
		use_orwhere = &PHALCON_GLOBAL(z_false);
	}

	if (Z_TYPE_P(values) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Values must be an array");
		return;
	}

	phalcon_read_property(&hidden_param, getThis(), SL("_hiddenParamNumber"), PH_READONLY);

	array_init(&bind_params);
	array_init(&bind_keys);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(values), value) {
		zval key = {}, query_key = {};
		/** 
		 * Key with auto bind-params
		 */
		PHALCON_CONCAT_SV(&key, "phi", &hidden_param);

		PHALCON_CONCAT_SVS(&query_key, ":", &key, ":");
		phalcon_array_append(&bind_keys, &query_key, PH_COPY);
		phalcon_array_update_zval(&bind_params, &key, value, PH_COPY);
		phalcon_increment(&hidden_param);
	} ZEND_HASH_FOREACH_END();

	phalcon_fast_join_str(&joined_keys, SL(", "), &bind_keys);

	/** 
	 * Create a standard IN condition with bind params
	 */
	PHALCON_CONCAT_VSVS(&conditions, expr, " IN (", &joined_keys, ")");

	/** 
	 * Append the IN to the current conditions using and 'and'
	 */
	if (zend_is_true(use_orwhere)) {
		PHALCON_CALL_METHODW(NULL, getThis(), "orwhere", &conditions, &bind_params);
	} else {
		PHALCON_CALL_METHODW(NULL, getThis(), "andwhere", &conditions, &bind_params);
	}
	phalcon_update_property_zval(getThis(), SL("_hiddenParamNumber"), &hidden_param);

	RETURN_THISW();
}

/**
 * Appends a NOT IN condition to the current conditions
 *
 *<code>
 *	$builder->notInWhere('id', [1, 2, 3]);
 *</code>
 *
 * @param string $expr
 * @param array $values
 * @param boolean $useOrWhere
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, notInWhere){

	zval *expr, *values, *use_orwhere = NULL, hidden_param = {}, bind_params = {}, bind_keys = {}, *value, joined_keys = {}, conditions = {};

	phalcon_fetch_params(0, 2, 1, &expr, &values, &use_orwhere);

	if (!use_orwhere) {
		use_orwhere = &PHALCON_GLOBAL(z_false);
	}

	if (Z_TYPE_P(values) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Values must be an array");
		return;
	}

	phalcon_read_property(&hidden_param, getThis(), SL("_hiddenParamNumber"), PH_NOISY);

	array_init(&bind_params);
	array_init(&bind_keys);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(values), value) {
		zval key = {}, query_key = {};
		/** 
		 * Key with auto bind-params
		 */
		PHALCON_CONCAT_SV(&key, "phi", &hidden_param);

		PHALCON_CONCAT_SVS(&query_key, ":", &key, ":");
		phalcon_array_append(&bind_keys, &query_key, PH_COPY);
		phalcon_array_update_zval(&bind_params, &key, value, PH_COPY);
		phalcon_increment(&hidden_param);
	} ZEND_HASH_FOREACH_END();

	phalcon_fast_join_str(&joined_keys, SL(", "), &bind_keys);

	/** 
	 * Create a standard IN condition with bind params
	 */
	PHALCON_CONCAT_VSVS(&conditions, expr, " NOT IN (", &joined_keys, ")");

	/** 
	 * Append the IN to the current conditions using and 'and'
	 */
	if (zend_is_true(use_orwhere)) {
		PHALCON_CALL_METHODW(NULL, getThis(), "orwhere", &conditions, &bind_params);
	} else {
		PHALCON_CALL_METHODW(NULL, getThis(), "andwhere", &conditions, &bind_params);
	}
	phalcon_update_property_zval(getThis(), SL("_hiddenParamNumber"), &hidden_param);

	RETURN_THISW();
}

/**
 * Return the conditions for the query
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, getWhere){


	RETURN_MEMBER(getThis(), "_conditions");
}

/**
 * Sets a ORDER BY condition clause
 *
 *<code>
 *	$builder->orderBy('Robots.name');
 *	$builder->orderBy(array('1', 'Robots.name'));
 *</code>
 *
 * @param string $orderBy
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, orderBy){

	zval *order_by;

	phalcon_fetch_params(0, 1, 0, &order_by);

	phalcon_update_property_zval(getThis(), SL("_order"), order_by);
	RETURN_THISW();
}

/**
 * Returns the set ORDER BY clause
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, getOrderBy){


	RETURN_MEMBER(getThis(), "_order");
}

/**
 * Sets a HAVING condition clause. You need to escape PHQL reserved words using [ and ] delimiters
 *
 *<code>
 *	$builder->having('SUM(Robots.price) > 0');
 *</code>
 *
 * @param string $having
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, having){

	zval *having;

	phalcon_fetch_params(0, 1, 0, &having);

	phalcon_update_property_zval(getThis(), SL("_having"), having);
	RETURN_THISW();
}

/**
 * Return the current having clause
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, getHaving){


	RETURN_MEMBER(getThis(), "_having");
}

/**
 * Sets a LIMIT clause, optionally a offset clause
 *
 *<code>
 *	$builder->limit(100);
 *	$builder->limit(100, 20);
 *</code>
 *
 * @param int $limit
 * @param int $offset
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, limit){

	zval *limit, *offset = NULL;

	phalcon_fetch_params(0, 1, 1, &limit, &offset);

	phalcon_update_property_zval(getThis(), SL("_limit"), limit);

	if (offset) {
		phalcon_update_property_zval(getThis(), SL("_offset"), offset);
	}

	RETURN_THISW();
}

/**
 * Returns the current LIMIT clause
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, getLimit){


	RETURN_MEMBER(getThis(), "_limit");
}

/**
 * Sets an OFFSET clause
 *
 *<code>
 *	$builder->offset(30);
 *</code>
 *
 * @param int $limit
 * @param int $offset
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, offset){

	zval *offset;

	phalcon_fetch_params(0, 1, 0, &offset);

	phalcon_update_property_zval(getThis(), SL("_offset"), offset);
	RETURN_THISW();
}

/**
 * Returns the current OFFSET clause
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, getOffset){


	RETURN_MEMBER(getThis(), "_offset");
}

/**
 * Sets a GROUP BY clause
 *
 *<code>
 *	$builder->groupBy(array('Robots.name'));
 *</code>
 *
 * @param string $group
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, groupBy){

	zval *group;

	phalcon_fetch_params(0, 1, 0, &group);

	phalcon_update_property_zval(getThis(), SL("_group"), group);
	RETURN_THISW();
}

/**
 * Returns the GROUP BY clause
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, getGroupBy){


	RETURN_MEMBER(getThis(), "_group");
}

/**
 * Returns a PHQL statement built based on the builder parameters
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, _compile){

	zval dependency_injector = {}, models = {}, *model, model_instance = {}, conditions = {}, distinct = {}, phql = {}, columns = {}, selected_columns = {};
	zval *column, joined_columns = {}, selected_models = {}, joined_models = {}, joins = {}, *join, group = {};
	zval having = {}, order = {}, limit = {}, offset = {}, for_update = {};
	zend_string *str_key;
	ulong idx;
	zend_class_entry *ce0;

	PHALCON_CALL_METHODW(&dependency_injector, getThis(), "getdi", &PHALCON_GLOBAL(z_true));

	phalcon_read_property(&models, getThis(), SL("_models"), PH_NOISY);
	if (Z_TYPE(models) == IS_ARRAY) { 
		if (!phalcon_fast_count_ev(&models)) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "At least one model is required to build the query");
			return;
		}
	} else if (!zend_is_true(&models)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "At least one model is required to build the query");
		return;
	}

	if (Z_TYPE(models) == IS_ARRAY) { 
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(models), idx, str_key, model) {
			zval model_alias = {};
			if (str_key) {
				ZVAL_STR(&model_alias, str_key);
			} else {
				ZVAL_LONG(&model_alias, idx);
			}

			ce0 = phalcon_fetch_class(model, ZEND_FETCH_CLASS_DEFAULT);

			if (phalcon_method_exists_ce_ex(ce0, SS("beforequery") TSRMLS_CC) == SUCCESS) {
				object_init_ex(&model_instance, ce0);
				if (phalcon_has_constructor(&model_instance TSRMLS_CC)) {
					PHALCON_CALL_METHODW(NULL, &model_instance, "__construct", &dependency_injector);
				}

				PHALCON_CALL_METHODW(NULL, &model_instance, "beforequery", getThis(), &model_alias);
			}
		} ZEND_HASH_FOREACH_END();
	} else {
		ce0 = phalcon_fetch_class(&models TSRMLS_CC, ZEND_FETCH_CLASS_DEFAULT);
		if (phalcon_method_exists_ce_ex(ce0, SS("beforequery") TSRMLS_CC) == SUCCESS) {
			object_init_ex(&model_instance, ce0);
			if (phalcon_has_constructor(&model_instance TSRMLS_CC)) {
				PHALCON_CALL_METHODW(NULL, &model_instance, "__construct", &dependency_injector);
			}

			PHALCON_CALL_METHODW(NULL, &model_instance, "beforequery", getThis());
		}
	}

	PHALCON_CALL_SELFW(&conditions, "getConditions");

	phalcon_read_property(&distinct, getThis(), SL("_distinct"), PH_NOISY);
	if (PHALCON_IS_BOOL(&distinct)) {
		if (Z_TYPE(distinct) == IS_TRUE) {
			PHALCON_STR(&phql, "SELECT DISTINCT ");
		} else {
			PHALCON_STR(&phql, "SELECT ALL ");
		}
	} else {
		PHALCON_STR(&phql, "SELECT ");
	}

	phalcon_read_property(&columns, getThis(), SL("_columns"), PH_NOISY);
	if (Z_TYPE(columns) != IS_NULL) {
		/** 
		 * Generate PHQL for columns
		 */
		if (Z_TYPE(columns) == IS_ARRAY) { 

			array_init(&selected_columns);
			
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(columns), idx, str_key, column) {
				zval column_alias = {}, aliased_column = {};
				if (str_key) {
					ZVAL_STR(&column_alias, str_key);
				} else {
					ZVAL_LONG(&column_alias, idx);
				}

				if (Z_TYPE(column_alias) == IS_LONG) {
					phalcon_array_append(&selected_columns, column, PH_COPY);
				} else {
					PHALCON_CONCAT_VSV(&aliased_column, column, " AS ", &column_alias);
					phalcon_array_append(&selected_columns, &aliased_column, PH_COPY);
				}
			} ZEND_HASH_FOREACH_END();

			phalcon_fast_join_str(&joined_columns, SL(", "), &selected_columns);
			phalcon_concat_self(&phql, &joined_columns);
		} else {
			phalcon_concat_self(&phql, &columns);
		}
	} else {
		/** 
		 * Automatically generate an array of models
		 */
		if (Z_TYPE(models) == IS_ARRAY) {
			array_init(&selected_columns);

			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(models), idx, str_key, model) {
				zval model_column_alias = {}, selected_column = {};
				if (str_key) {
					ZVAL_STR(&model_column_alias, str_key);
				} else {
					ZVAL_LONG(&model_column_alias, idx);
				}

				if (Z_TYPE(model_column_alias) == IS_LONG) {
					PHALCON_CONCAT_SVS(&selected_column, "[", model, "].*");
				} else {
					PHALCON_CONCAT_SVS(&selected_column, "[", &model_column_alias, "].*");
				}
				phalcon_array_append(&selected_columns, &selected_column, PH_COPY);
			} ZEND_HASH_FOREACH_END();

			phalcon_fast_join_str(&joined_columns, SL(", "), &selected_columns);
			phalcon_concat_self(&phql, &joined_columns);
		} else {
			PHALCON_SCONCAT_SVS(&phql, "[", &models, "].*");
		}
	}

	/** 
	 * Join multiple models or use a single one if it is a string
	 */
	if (Z_TYPE(models) == IS_ARRAY) { 
		array_init(&selected_models);

		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(models), idx, str_key, model) {
			zval model_alias = {}, selected_model = {};
			if (str_key) {
				ZVAL_STR(&model_alias, str_key);
			} else {
				ZVAL_LONG(&model_alias, idx);
			}

			if (Z_TYPE(model_alias) == IS_STRING) {
				PHALCON_CONCAT_SVSVS(&selected_model, "[", model, "] AS [", &model_alias, "]");
			} else {
				PHALCON_CONCAT_SVS(&selected_model, "[", model, "]");
			}
			phalcon_array_append(&selected_models, &selected_model, PH_COPY);
		} ZEND_HASH_FOREACH_END();

		phalcon_fast_join_str(&joined_models, SL(", "), &selected_models);
		PHALCON_SCONCAT_SV(&phql, " FROM ", &joined_models);
	} else {
		PHALCON_SCONCAT_SVS(&phql, " FROM [", &models, "]");
	}

	/** 
	 * Check if joins were passed to the builders
	 */
	phalcon_read_property(&joins, getThis(), SL("_joins"), PH_NOISY);
	if (Z_TYPE(joins) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(joins), join) {
			zval join_model = {}, join_conditions = {}, join_alias = {}, join_type = {};
			/** 
			 * The joined table is in the first place of the array
			 */
			phalcon_array_fetch_long(&join_model, join, 0, PH_NOISY);

			/** 
			 * The join conditions are in the second place of the array
			 */
			phalcon_array_fetch_long(&join_conditions, join, 1, PH_NOISY);

			/** 
			 * The join alias is in the second place of the array
			 */
			phalcon_array_fetch_long(&join_alias, join, 2, PH_NOISY);

			/** 
			 * Join type
			 */
			phalcon_array_fetch_long(&join_type, join, 3, PH_NOISY);

			/** 
			 * Create the join according to the type
			 */
			if (zend_is_true(&join_type)) {
				PHALCON_SCONCAT_SVSVS(&phql, " ", &join_type, " JOIN [", &join_model, "]");
			} else {
				PHALCON_SCONCAT_SVS(&phql, " JOIN [", &join_model, "]");
			}

			/** 
			 * Alias comes first
			 */
			if (zend_is_true(&join_alias)) {
				PHALCON_SCONCAT_SVS(&phql, " AS [", &join_alias, "]");
			}

			/** 
			 * Conditions then
			 */
			if (zend_is_true(&join_conditions)) {
				PHALCON_SCONCAT_SV(&phql, " ON ", &join_conditions);
			}
		} ZEND_HASH_FOREACH_END();
	}

	/** 
	 * Only append conditions if it's string
	 */
	if (Z_TYPE(conditions) == IS_STRING && PHALCON_IS_NOT_EMPTY(&conditions)) {
		PHALCON_SCONCAT_SV(&phql, " WHERE ", &conditions);
	}

	/** 
	 * Process group parameters
	 */
	phalcon_read_property(&group, getThis(), SL("_group"), PH_NOISY);
	phalcon_orm_phql_build_group(&phql, &group);

	/* Process HAVING clause */
	phalcon_read_property(&having, getThis(), SL("_having"), PH_NOISY);
	if (Z_TYPE(having) != IS_NULL) {
		if (PHALCON_IS_NOT_EMPTY(&having)) {
			PHALCON_SCONCAT_SV(&phql, " HAVING ", &having);
		}
	}

	/** 
	 * Process order clause
	 */
	phalcon_read_property(&order, getThis(), SL("_order"), PH_NOISY);
	phalcon_orm_phql_build_order(&phql, &order);

	/** 
	 * Process limit parameters
	 */
	phalcon_read_property(&limit, getThis(), SL("_limit"), PH_NOISY);
	if (PHALCON_IS_NOT_EMPTY(&limit) && Z_TYPE(limit) != IS_ARRAY) {
		phalcon_return_property(&offset, getThis(), SL("_offset"));
		if (PHALCON_IS_NOT_EMPTY(&offset)) {
			PHALCON_SCONCAT_SV(&limit, " OFFSET ", &offset);
		}
	}

	phalcon_orm_phql_build_limit(&phql, &limit);

	/** 
	 * Process FOR UPDATE clause
	 */
	phalcon_read_property(&for_update, getThis(), SL("_forUpdate"), PH_NOISY);
	if (zend_is_true(&for_update)) {
		phalcon_concat_self_str(&phql, SL(" FOR UPDATE"));
	}

	phalcon_update_property_zval(getThis(), SL("_phql"), &phql);
}
