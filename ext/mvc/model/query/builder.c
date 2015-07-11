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

#include "interned-strings.h"

/**
 * Phalcon\Mvc\Model\Query\Builder
 *
 * Helps to create PHQL queries using an OO interface
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
zend_class_entry *phalcon_mvc_model_query_builder_ce;

PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, __construct);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, distinct);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getDistinct);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, columns);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getColumns);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, from);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, addFrom);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getFrom);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, join);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, innerJoin);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, leftJoin);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, rightJoin);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, where);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, andWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, orWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, betweenWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, notBetweenWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, inWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, notInWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getWhere);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, orderBy);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getOrderBy);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, having);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getHaving);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, limit);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getLimit);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, offset);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getOffset);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, groupBy);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getGroupBy);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getPhql);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getQuery);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getConditions);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_offset, 0, 0, 1)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_query_builder_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, __construct, arginfo_phalcon_mvc_model_query_builder___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, distinct, arginfo_phalcon_mvc_model_query_builderinterface_distinct, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getDistinct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, columns, arginfo_phalcon_mvc_model_query_builderinterface_columns, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getColumns, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, from, arginfo_phalcon_mvc_model_query_builderinterface_from, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, addFrom, arginfo_phalcon_mvc_model_query_builderinterface_addfrom, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getFrom, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, join, arginfo_phalcon_mvc_model_query_builderinterface_join, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, innerJoin, arginfo_phalcon_mvc_model_query_builderinterface_innerjoin, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, leftJoin, arginfo_phalcon_mvc_model_query_builderinterface_leftjoin, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, rightJoin, arginfo_phalcon_mvc_model_query_builderinterface_rightjoin, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, where, arginfo_phalcon_mvc_model_query_builderinterface_where, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, andWhere, arginfo_phalcon_mvc_model_query_builderinterface_andwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, orWhere, arginfo_phalcon_mvc_model_query_builderinterface_orwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, betweenWhere, arginfo_phalcon_mvc_model_query_builderinterface_betweenwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, notBetweenWhere, arginfo_phalcon_mvc_model_query_builderinterface_notbetweenwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, inWhere, arginfo_phalcon_mvc_model_query_builderinterface_inwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, notInWhere, arginfo_phalcon_mvc_model_query_builderinterface_notinwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getWhere, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, orderBy, arginfo_phalcon_mvc_model_query_builderinterface_orderby, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getOrderBy, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, having, arginfo_phalcon_mvc_model_query_builderinterface_having, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getHaving, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, limit, arginfo_phalcon_mvc_model_query_builderinterface_limit, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getLimit, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, offset, arginfo_phalcon_mvc_model_query_builder_offset, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getOffset, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, groupBy, arginfo_phalcon_mvc_model_query_builderinterface_groupby, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getGroupBy, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getPhql, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getQuery, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder, getConditions, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Query\Builder initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Query_Builder){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\Query, Builder, mvc_model_query_builder, phalcon_di_injectable_ce, phalcon_mvc_model_query_builder_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_columns"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_models"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_joins"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_conditions"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_group"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_having"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_order"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_limit"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_offset"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_forUpdate"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_sharedLock"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_bindParams"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_bindTypes"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_ce, SL("_distinct"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_mvc_model_query_builder_ce, SL("_hiddenParamNumber"), 0, ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_mvc_model_query_builder_ce, 1, phalcon_mvc_model_query_builderinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Model\Query\Builder constructor
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
 *$queryBuilder = new Phalcon\Mvc\Model\Query\Builder($params);
 *</code> 
 *
 * @param array $params
 * @param Phalcon\DI $dependencyInjector
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, __construct){

	zval *params = NULL, *dependency_injector = NULL, *conditions = NULL;
	zval *models, *columns, *group_clause, *joins;
	zval *having_clause, *order_clause, *limit_clause;
	zval *offset_clause, *for_update, *shared_lock;
	zval *limit, *offset, *single_condition_array = NULL;
	zval *condition_string = NULL, *new_condition_string = NULL, *bind_params, *bind_types;	
	zval *merged_conditions, *merged_bind_params, *merged_bind_types;
	zval *current_bind_params, *current_bind_types;
	zend_string *str_key;
	ulong idx;

	PHALCON_MM_GROW();

	phalcon_fetch_params(0, 0, 2, &params, &dependency_injector);

	if (params && Z_TYPE_P(params) == IS_ARRAY) {
		/** 
		 * Process conditions
		 */
		if (phalcon_array_isset_str_fetch(&conditions, params, SL("conditions")) || phalcon_array_isset_long_fetch(&conditions, params, 0)) {
			if (Z_TYPE_P(conditions) == IS_ARRAY) {

				/* ----------- INITIALIZING LOOP VARIABLES ----------- */

				/*
				 * array containing single condition for example:
				 * array(
				 *      'status = :status:',
				 *      array('status' => 5),
				 *      array('status' => PDO::PARAM_INT),
				 * )
				 */
				PHALCON_INIT_VAR(single_condition_array);
				array_init(single_condition_array);

				/* ----------- INITIALIZING MERGED VARIABLES ----------- */

				PHALCON_INIT_VAR(merged_conditions);
				array_init(merged_conditions);

				PHALCON_INIT_VAR(merged_bind_params);
				array_init(merged_bind_params);

				PHALCON_INIT_VAR(merged_bind_types);
				array_init(merged_bind_types);

				ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(conditions), idx, str_key, single_condition_array) {
					zval single_condition_key;
					if (str_key) {
						ZVAL_STR(&single_condition_key, str_key);
					} else {
						ZVAL_LONG(&single_condition_key, idx);
					}
					if (Z_TYPE_P(single_condition_array) == IS_ARRAY
						&& phalcon_array_isset_long_fetch(&condition_string, single_condition_array, 0)
						&& phalcon_array_isset_long_fetch(&bind_params, single_condition_array, 1)
						&& Z_TYPE_P(condition_string) == IS_STRING
						&& Z_TYPE_P(bind_params) == IS_ARRAY
					) {	
						phalcon_array_update_zval(merged_conditions, condition_string, condition_string, PH_COPY);

						phalcon_array_merge_recursive_n(merged_bind_params, bind_params);

						if (phalcon_array_isset_long_fetch(&bind_types, single_condition_array, 2) && Z_TYPE_P(bind_types) == IS_ARRAY) {
							phalcon_array_merge_recursive_n(merged_bind_types, bind_types);
						}
					} else if (Z_TYPE(single_condition_key) == IS_STRING) {
						PHALCON_INIT_NVAR(new_condition_string);
						PHALCON_CONCAT_VSVS(new_condition_string, &single_condition_key, " = :", &single_condition_key, ":");

						phalcon_array_update_zval(merged_conditions, &single_condition_key, new_condition_string, PH_COPY);

						if (Z_TYPE_P(single_condition_array) == IS_ARRAY) {
							phalcon_array_merge_recursive_n(merged_bind_params, single_condition_array);
						} else {
							phalcon_array_update_zval(merged_bind_params, &single_condition_key, single_condition_array, PH_COPY);
						}
					}
				} ZEND_HASH_FOREACH_END();

				PHALCON_INIT_NVAR(new_condition_string);
				phalcon_fast_join_str(new_condition_string, SL(" AND "), merged_conditions);

				phalcon_update_property_this(getThis(), SL("_conditions"), new_condition_string);
				phalcon_update_property_this(getThis(), SL("_bindParams"), merged_bind_params);
				phalcon_update_property_this(getThis(), SL("_bindTypes"), merged_bind_types);
			} else {
				phalcon_update_property_this(getThis(), SL("_conditions"), conditions);		
			}	
		}

		if (phalcon_array_isset_str_fetch(&bind_params, params, SL("bind"))) {
			if (Z_TYPE_P(bind_params) == IS_ARRAY) {
				current_bind_params = phalcon_read_property(getThis(), SL("_bindParams"), PH_NOISY);
				if (Z_TYPE_P(current_bind_params) == IS_ARRAY) { 
					PHALCON_INIT_NVAR(merged_bind_params);
					phalcon_add_function(merged_bind_params, bind_params, current_bind_params);
					phalcon_update_property_this(getThis(), SL("_bindParams"), merged_bind_params);
				} else {
					phalcon_update_property_this(getThis(), SL("_bindParams"), bind_params);
				}
			}
		}

		if (phalcon_array_isset_str_fetch(&bind_types, params, SL("bindTypes"))) {
			if (Z_TYPE_P(bind_types) == IS_ARRAY) {
				current_bind_types = phalcon_read_property(getThis(), SL("_bindTypes"), PH_NOISY);
				if (Z_TYPE_P(current_bind_types) == IS_ARRAY) { 
					PHALCON_INIT_VAR(merged_bind_types);
					phalcon_add_function(merged_bind_types, bind_types, current_bind_types);
					phalcon_update_property_this(getThis(), SL("_bindTypes"), merged_bind_types);
				} else {
					phalcon_update_property_this(getThis(), SL("_bindTypes"), bind_types);
				}
			}
		}

		/** 
		 * Assign 'FROM' clause
		 */
		if (phalcon_array_isset_str_fetch(&models, params, SL("models"))) {
			phalcon_update_property_this(getThis(), SL("_models"), models);
		}

		/** 
		 * Assign COLUMNS clause
		 */
		if (phalcon_array_isset_str_fetch(&columns, params, SL("columns"))) {
			phalcon_update_property_this(getThis(), SL("_columns"), columns);
		}

		/**
		 * Assign JOIN clause
		 */
		if (phalcon_array_isset_str_fetch(&joins, params, SL("joins"))) {
			phalcon_update_property_this(getThis(), SL("_joins"), joins);
		}

		/** 
		 * Assign GROUP clause
		 */
		if (phalcon_array_isset_str_fetch(&group_clause, params, SL("group"))) {
			phalcon_update_property_this(getThis(), SL("_group"), group_clause);
		}

		/** 
		 * Assign HAVING clause
		 */
		if (phalcon_array_isset_str_fetch(&having_clause, params, SL("having"))) {
			phalcon_update_property_this(getThis(), SL("_having"), having_clause);
		}

		/** 
		 * Assign ORDER clause
		 */
		if (phalcon_array_isset_str_fetch(&order_clause, params, SL("order"))) {
			phalcon_update_property_this(getThis(), SL("_order"), order_clause);
		}

		/** 
		 * Assign LIMIT clause
		 */
		if (phalcon_array_isset_str_fetch(&limit_clause, params, SL("limit"))) {
			if (Z_TYPE_P(limit_clause) == IS_ARRAY
				&& phalcon_array_isset_long_fetch(&limit, limit_clause, 0)
				&& phalcon_array_isset_long_fetch(&offset, limit_clause, 1)
			) {
				phalcon_update_property_this(getThis(), SL("_limit"), limit);
				phalcon_update_property_this(getThis(), SL("_offset"), offset);
			} else {
				phalcon_update_property_this(getThis(), SL("_limit"), limit_clause);
			}
		}

		/** 
		 * Assign OFFSET clause
		 */
		if (phalcon_array_isset_str_fetch(&offset_clause, params, SL("offset"))) {
			phalcon_update_property_this(getThis(), SL("_offset"), offset_clause);
		}

		/** 
		 * Assign FOR UPDATE clause
		 */
		if (phalcon_array_isset_str_fetch(&for_update, params, SL("for_update"))) {
			phalcon_update_property_this(getThis(), SL("_forUpdate"), for_update);
		}

		/** 
		 * Assign SHARED LOCK clause
		 */
		if (phalcon_array_isset_str_fetch(&shared_lock, params, SL("shared_lock"))) {
			phalcon_update_property_this(getThis(), SL("_sharedLock"), shared_lock);
		}
	}

	/** 
	 * Update the dependency injector if any
	 */
	if (dependency_injector && Z_TYPE_P(dependency_injector) == IS_OBJECT) {
		phalcon_update_property_this(getThis(), SL("_dependencyInjector"), dependency_injector);
	}

	PHALCON_MM_RESTORE();	
}

/**
 * Sets SELECT DISTINCT / SELECT ALL flag
 *
 * @param bool|null distinct
 * @return Phalcon\Mvc\Model\Query\BuilderInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, distinct){

	zval *distinct;

	phalcon_fetch_params(0, 1, 0, &distinct);

	if (Z_TYPE_P(distinct) != IS_NULL && !PHALCON_IS_BOOL(distinct)) {
		PHALCON_ENSURE_IS_BOOL(distinct);
	}

	phalcon_update_property_this(getThis(), SL("_distinct"), distinct);
	RETURN_THISW();
}

/**
 * Returns SELECT DISTINCT / SELECT ALL flag
 *
 * @return bool
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getDistinct){


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
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, columns){

	zval *columns;

	phalcon_fetch_params(0, 1, 0, &columns);

	phalcon_update_property_this(getThis(), SL("_columns"), columns);
	RETURN_THISW();
}

/**
 * Return the columns to be queried
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getColumns){


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
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, from){

	zval *models;

	phalcon_fetch_params(0, 1, 0, &models);

	phalcon_update_property_this(getThis(), SL("_models"), models);
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
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, addFrom){

	zval *model, *alias = NULL, *models = NULL, *current_model = NULL;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 1, &model, &alias);

	if (!alias) {
		alias = &PHALCON_GLOBAL(z_null);
	}

	models = phalcon_read_property(getThis(), SL("_models"), PH_NOISY);
	if (Z_TYPE_P(models) != IS_ARRAY) { 
		if (Z_TYPE_P(models) != IS_NULL) {
			PHALCON_CPY_WRT(current_model, models);

			PHALCON_INIT_NVAR(models);
			array_init(models);
			phalcon_array_append(models, current_model, 0);
		} else {
			PHALCON_INIT_NVAR(models);
			array_init(models);
		}
	} else {
		SEPARATE_ZVAL(models);
	}

	if (Z_TYPE_P(alias) == IS_STRING) {
		phalcon_array_update_zval(models, alias, model, PH_COPY | 0);
	} else {
		phalcon_array_append(models, model, 0);
	}

	phalcon_update_property_this(getThis(), SL("_models"), models);

	RETURN_THIS();
}

/**
 * Return the models who makes part of the query
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getFrom){


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
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, join){

	zval *model, *conditions = NULL, *alias = NULL, *type = NULL, *join;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 3, &model, &conditions, &alias, &type);

	if (!conditions) {
		conditions = &PHALCON_GLOBAL(z_null);
	}

	if (!alias) {
		alias = &PHALCON_GLOBAL(z_null);
	}

	if (!type) {
		type = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_INIT_VAR(join);
	array_init_size(join, 4);
	phalcon_array_append(join, model, 0);
	phalcon_array_append(join, conditions, 0);
	phalcon_array_append(join, alias, 0);
	phalcon_array_append(join, type, 0);
	phalcon_update_property_array_append(getThis(), SL("_joins"), join);
	RETURN_THIS();
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
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, innerJoin){

	zval *model, *conditions = NULL, *alias = NULL, *type, *join;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 2, &model, &conditions, &alias);

	if (!conditions) {
		conditions = &PHALCON_GLOBAL(z_null);
	}

	if (!alias) {
		alias = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_INIT_VAR(type);
	ZVAL_STRING(type, "INNER");

	PHALCON_INIT_VAR(join);
	array_init_size(join, 4);
	phalcon_array_append(join, model, 0);
	phalcon_array_append(join, conditions, 0);
	phalcon_array_append(join, alias, 0);
	phalcon_array_append(join, type, 0);
	phalcon_update_property_array_append(getThis(), SL("_joins"), join);
	RETURN_THIS();
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
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, leftJoin){

	zval *model, *conditions = NULL, *alias = NULL, *type, *join;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 2, &model, &conditions, &alias);

	if (!conditions) {
		conditions = &PHALCON_GLOBAL(z_null);
	}

	if (!alias) {
		alias = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_INIT_VAR(type);
	ZVAL_STRING(type, "LEFT");

	PHALCON_INIT_VAR(join);
	array_init_size(join, 4);
	phalcon_array_append(join, model, PH_SEPARATE);
	phalcon_array_append(join, conditions, PH_SEPARATE);
	phalcon_array_append(join, alias, PH_SEPARATE);
	phalcon_array_append(join, type, PH_SEPARATE);
	phalcon_update_property_array_append(getThis(), SL("_joins"), join);
	RETURN_THIS();
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
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, rightJoin){

	zval *model, *conditions = NULL, *alias = NULL, *type, *join;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 2, &model, &conditions, &alias);

	if (!conditions) {
		conditions = &PHALCON_GLOBAL(z_null);
	}

	if (!alias) {
		alias = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_INIT_VAR(type);
	ZVAL_STRING(type, "RIGHT");

	PHALCON_INIT_VAR(join);
	array_init_size(join, 4);
	phalcon_array_append(join, model, 0);
	phalcon_array_append(join, conditions, 0);
	phalcon_array_append(join, alias, 0);
	phalcon_array_append(join, type, 0);
	phalcon_update_property_this(getThis(), SL("_joins"), join);
	RETURN_THIS();
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
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, where){

	zval *conditions, *bind_params = NULL, *bind_types = NULL;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 2, &conditions, &bind_params, &bind_types);

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	phalcon_update_property_this(getThis(), SL("_conditions"), conditions);

	/** 
	 * Override the bind params and bind types to the current ones
	 */
	phalcon_update_property_this(getThis(), SL("_bindParams"), bind_params);
	phalcon_update_property_this(getThis(), SL("_bindTypes"), bind_types);

	RETURN_THIS();
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
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, andWhere){

	zval *conditions, *bind_params = NULL, *bind_types = NULL;
	zval *current_conditions = NULL, *new_conditions = NULL;
	zval *current_bind_params, *merged_params = NULL;
	zval *current_bind_types, *merged_types = NULL;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 2, &conditions, &bind_params, &bind_types);

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_SELF(&current_conditions, "getConditions");

	/** 
	 * Nest the condition to current ones or set as unique
	 */
	if (zend_is_true(current_conditions)) {
		PHALCON_INIT_VAR(new_conditions);
		PHALCON_CONCAT_SVSVS(new_conditions, "(", current_conditions, ") AND (", conditions, ")");
	} else {
		PHALCON_CPY_WRT(new_conditions, conditions);
	}

	phalcon_update_property_this(getThis(), SL("_conditions"), new_conditions);

	/** 
	 * Merge the bind params to the current ones
	 */
	if (Z_TYPE_P(bind_params) == IS_ARRAY) {
		current_bind_params = phalcon_read_property(getThis(), SL("_bindParams"), PH_NOISY);
		if (Z_TYPE_P(current_bind_params) == IS_ARRAY) { 
			PHALCON_INIT_VAR(merged_params);
			phalcon_add_function(merged_params, bind_params, current_bind_params);
		} else {
			PHALCON_CPY_WRT(merged_params, bind_params);
		}

		phalcon_update_property_this(getThis(), SL("_bindParams"), merged_params);
	}

	/** 
	 * Merge the bind types to the current ones
	 */
	if (Z_TYPE_P(bind_types) == IS_ARRAY) {
		current_bind_types = phalcon_read_property(getThis(), SL("_bindTypes"), PH_NOISY);
		if (Z_TYPE_P(current_bind_types) == IS_ARRAY) { 
			PHALCON_INIT_VAR(merged_types);
			phalcon_add_function(merged_params, bind_types, current_bind_types);
		} else {
			PHALCON_CPY_WRT(merged_types, bind_types);
		}

		phalcon_update_property_this(getThis(), SL("_bindTypes"), merged_types);
	}

	RETURN_THIS();
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
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, orWhere){

	zval *conditions, *bind_params = NULL, *bind_types = NULL;
	zval *current_conditions = NULL, *new_conditions = NULL;
	zval *current_bind_params, *merged_params = NULL;
	zval *current_bind_types, *merged_types = NULL;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 2, &conditions, &bind_params, &bind_types);

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_SELF(&current_conditions, "getConditions");

	/** 
	 * Nest the condition to current ones or set as unique
	 */
	if (zend_is_true(current_conditions)) {
		PHALCON_INIT_VAR(new_conditions);
		PHALCON_CONCAT_SVSVS(new_conditions, "(", current_conditions, ") OR (", conditions, ")");
	} else {
		PHALCON_CPY_WRT(new_conditions, conditions);
	}

	phalcon_update_property_this(getThis(), SL("_conditions"), new_conditions);

	/** 
	 * Merge the bind params to the current ones
	 */
	if (Z_TYPE_P(bind_params) == IS_ARRAY) {
		current_bind_params = phalcon_read_property(getThis(), SL("_bindParams"), PH_NOISY);
		if (Z_TYPE_P(current_bind_params) == IS_ARRAY) { 
			PHALCON_INIT_VAR(merged_params);
			phalcon_add_function(merged_params, bind_params, current_bind_params);
		} else {
			PHALCON_CPY_WRT(merged_params, bind_params);
		}

		phalcon_update_property_this(getThis(), SL("_bindParams"), merged_params);
	}

	/** 
	 * Merge the bind types to the current ones
	 */
	if (Z_TYPE_P(bind_types) == IS_ARRAY) {
		current_bind_types = phalcon_read_property(getThis(), SL("_bindTypes"), PH_NOISY);
		if (Z_TYPE_P(current_bind_types) == IS_ARRAY) { 
			PHALCON_INIT_VAR(merged_types);
			phalcon_add_function(merged_types, bind_types, current_bind_types);
		} else {
			PHALCON_CPY_WRT(merged_types, bind_types);
		}

		phalcon_update_property_this(getThis(), SL("_bindTypes"), merged_types);
	}

	RETURN_THIS();
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
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, betweenWhere){

	zval *expr, *minimum, *maximum, *use_orwhere = NULL, *hidden_param, *z_one;
	zval *next_hidden_param, *minimum_key, *maximum_key;
	zval *conditions, *bind_params;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 3, 1, &expr, &minimum, &maximum, &use_orwhere);

	if (!use_orwhere) {
		use_orwhere = &PHALCON_GLOBAL(z_false);
	}

	hidden_param = phalcon_read_property(getThis(), SL("_hiddenParamNumber"), PH_NOISY);

	z_one = &PHALCON_GLOBAL(z_one);

	PHALCON_INIT_VAR(next_hidden_param);
	phalcon_add_function(next_hidden_param, hidden_param, z_one);

	/** 
	 * Minimum key with auto bind-params
	 */
	PHALCON_INIT_VAR(minimum_key);
	PHALCON_CONCAT_SV(minimum_key, "phb", hidden_param);

	/** 
	 * Maximum key with auto bind-params
	 */
	PHALCON_INIT_VAR(maximum_key);
	PHALCON_CONCAT_SV(maximum_key, "phb", next_hidden_param);

	/** 
	 * Create a standard BETWEEN condition with bind params
	 */
	PHALCON_INIT_VAR(conditions);
	PHALCON_CONCAT_VSVSVS(conditions, expr, " BETWEEN :", minimum_key, ": AND :", maximum_key, ":");

	PHALCON_INIT_VAR(bind_params);
	array_init_size(bind_params, 2);
	phalcon_array_update_zval(bind_params, minimum_key, minimum, PH_COPY);
	phalcon_array_update_zval(bind_params, maximum_key, maximum, PH_COPY);

	/** 
	 * Append the BETWEEN to the current conditions using and 'and'
	 */
	if (zend_is_true(use_orwhere)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "orwhere", conditions, bind_params);
	} else {
		PHALCON_CALL_METHOD(NULL, getThis(), "andwhere", conditions, bind_params);
	}

	phalcon_increment(next_hidden_param);
	phalcon_update_property_this(getThis(), SL("_hiddenParamNumber"), next_hidden_param);
	RETURN_THIS();
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
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, notBetweenWhere){

	zval *expr, *minimum, *maximum, *use_orwhere = NULL, *hidden_param, *z_one;
	zval *next_hidden_param, *minimum_key, *maximum_key;
	zval *conditions, *bind_params;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 3, 1, &expr, &minimum, &maximum, &use_orwhere);

	if (!use_orwhere) {
		use_orwhere = &PHALCON_GLOBAL(z_false);
	}

	hidden_param = phalcon_read_property(getThis(), SL("_hiddenParamNumber"), PH_NOISY);

	z_one = &PHALCON_GLOBAL(z_one);

	PHALCON_INIT_VAR(next_hidden_param);
	phalcon_add_function(next_hidden_param, hidden_param, z_one);

	/** 
	 * Minimum key with auto bind-params
	 */
	PHALCON_INIT_VAR(minimum_key);
	PHALCON_CONCAT_SV(minimum_key, "phb", hidden_param);

	/** 
	 * Maximum key with auto bind-params
	 */
	PHALCON_INIT_VAR(maximum_key);
	PHALCON_CONCAT_SV(maximum_key, "phb", next_hidden_param);

	/** 
	 * Create a standard BETWEEN condition with bind params
	 */
	PHALCON_INIT_VAR(conditions);
	PHALCON_CONCAT_VSVSVS(conditions, expr, " NOT BETWEEN :", minimum_key, ": AND :", maximum_key, ":");

	PHALCON_INIT_VAR(bind_params);
	array_init_size(bind_params, 2);
	phalcon_array_update_zval(bind_params, minimum_key, minimum, PH_COPY);
	phalcon_array_update_zval(bind_params, maximum_key, maximum, PH_COPY);

	/** 
	 * Append the BETWEEN to the current conditions using and 'and'
	 */
	if (zend_is_true(use_orwhere)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "orwhere", conditions, bind_params);
	} else {
		PHALCON_CALL_METHOD(NULL, getThis(), "andwhere", conditions, bind_params);
	}

	phalcon_increment(next_hidden_param);
	phalcon_update_property_this(getThis(), SL("_hiddenParamNumber"), next_hidden_param);
	RETURN_THIS();
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
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, inWhere){

	zval *expr, *values, *use_orwhere = NULL, *hidden_param, *bind_params;
	zval *bind_keys, *value = NULL, *key = NULL, *query_key = NULL, *joined_keys;
	zval *conditions;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 2, 1, &expr, &values, &use_orwhere);

	if (!use_orwhere) {
		use_orwhere = &PHALCON_GLOBAL(z_false);
	}

	if (Z_TYPE_P(values) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Values must be an array");
		return;
	}

	hidden_param = phalcon_read_property(getThis(), SL("_hiddenParamNumber"), PH_NOISY);
	SEPARATE_ZVAL(hidden_param);

	PHALCON_INIT_VAR(bind_params);
	array_init(bind_params);

	PHALCON_INIT_VAR(bind_keys);
	array_init(bind_keys);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(values), value) {
		/** 
		 * Key with auto bind-params
		 */
		PHALCON_INIT_NVAR(key);
		PHALCON_CONCAT_SV(key, "phi", hidden_param);

		PHALCON_INIT_NVAR(query_key);
		PHALCON_CONCAT_SVS(query_key, ":", key, ":");
		phalcon_array_append(bind_keys, query_key, 0);
		phalcon_array_update_zval(bind_params, key, value, PH_COPY);
		phalcon_increment(hidden_param);
	} ZEND_HASH_FOREACH_END();

	PHALCON_INIT_VAR(joined_keys);
	phalcon_fast_join_str(joined_keys, SL(", "), bind_keys);

	/** 
	 * Create a standard IN condition with bind params
	 */
	PHALCON_INIT_VAR(conditions);
	PHALCON_CONCAT_VSVS(conditions, expr, " IN (", joined_keys, ")");

	/** 
	 * Append the IN to the current conditions using and 'and'
	 */
	if (zend_is_true(use_orwhere)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "orwhere", conditions, bind_params);
	} else {
		PHALCON_CALL_METHOD(NULL, getThis(), "andwhere", conditions, bind_params);
	}
	phalcon_update_property_this(getThis(), SL("_hiddenParamNumber"), hidden_param);

	RETURN_THIS();
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
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, notInWhere){

	zval *expr, *values, *use_orwhere = NULL, *hidden_param, *bind_params;
	zval *bind_keys, *value = NULL, *key = NULL, *query_key = NULL, *joined_keys;
	zval *conditions;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 2, 1, &expr, &values, &use_orwhere);

	if (!use_orwhere) {
		use_orwhere = &PHALCON_GLOBAL(z_false);
	}

	if (Z_TYPE_P(values) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Values must be an array");
		return;
	}

	hidden_param = phalcon_read_property(getThis(), SL("_hiddenParamNumber"), PH_NOISY);

	PHALCON_INIT_VAR(bind_params);
	array_init(bind_params);

	PHALCON_INIT_VAR(bind_keys);
	array_init(bind_keys);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(values), value) {
		/** 
		 * Key with auto bind-params
		 */
		PHALCON_INIT_NVAR(key);
		PHALCON_CONCAT_SV(key, "phi", hidden_param);

		PHALCON_INIT_NVAR(query_key);
		PHALCON_CONCAT_SVS(query_key, ":", key, ":");
		phalcon_array_append(bind_keys, query_key, 0);
		phalcon_array_update_zval(bind_params, key, value, PH_COPY);
		phalcon_increment(hidden_param);
	} ZEND_HASH_FOREACH_END();

	PHALCON_INIT_VAR(joined_keys);
	phalcon_fast_join_str(joined_keys, SL(", "), bind_keys);

	/** 
	 * Create a standard IN condition with bind params
	 */
	PHALCON_INIT_VAR(conditions);
	PHALCON_CONCAT_VSVS(conditions, expr, " NOT IN (", joined_keys, ")");

	/** 
	 * Append the IN to the current conditions using and 'and'
	 */
	if (zend_is_true(use_orwhere)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "orwhere", conditions, bind_params);
	} else {
		PHALCON_CALL_METHOD(NULL, getThis(), "andwhere", conditions, bind_params);
	}
	phalcon_update_property_this(getThis(), SL("_hiddenParamNumber"), hidden_param);

	RETURN_THIS();
}

/**
 * Return the conditions for the query
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getWhere){


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
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, orderBy){

	zval *order_by;

	phalcon_fetch_params(0, 1, 0, &order_by);

	phalcon_update_property_this(getThis(), SL("_order"), order_by);
	RETURN_THISW();
}

/**
 * Returns the set ORDER BY clause
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getOrderBy){


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
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, having){

	zval *having;

	phalcon_fetch_params(0, 1, 0, &having);

	phalcon_update_property_this(getThis(), SL("_having"), having);
	RETURN_THISW();
}

/**
 * Return the current having clause
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getHaving){


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
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, limit){

	zval *limit, *offset = NULL;

	phalcon_fetch_params(0, 1, 1, &limit, &offset);

	phalcon_update_property_this(getThis(), SL("_limit"), limit);

	if (offset) {
		phalcon_update_property_this(getThis(), SL("_offset"), offset);
	}

	RETURN_THISW();
}

/**
 * Returns the current LIMIT clause
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getLimit){


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
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, offset){

	zval *offset;

	phalcon_fetch_params(0, 1, 0, &offset);

	phalcon_update_property_this(getThis(), SL("_offset"), offset);
	RETURN_THISW();
}

/**
 * Returns the current OFFSET clause
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getOffset){


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
 * @return Phalcon\Mvc\Model\Query\Builder
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, groupBy){

	zval *group;

	phalcon_fetch_params(0, 1, 0, &group);

	phalcon_update_property_this(getThis(), SL("_group"), group);
	RETURN_THISW();
}

/**
 * Returns the GROUP BY clause
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getGroupBy){


	RETURN_MEMBER(getThis(), "_group");
}

/**
 * Returns a PHQL statement built based on the builder parameters
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getPhql){

	zval *dependency_injector = NULL, *models, *conditions = NULL, *distinct;
	zval *model = NULL, *phql, *columns;
	zval *selected_columns = NULL, *column = NULL;
	zval *aliased_column = NULL, *joined_columns = NULL;
	zval *selected_column = NULL, *selected_models;
	zval *selected_model = NULL, *joined_models, *joins;
	zval *join = NULL, *join_model = NULL, *join_conditions = NULL, *join_alias = NULL;
	zval *join_type = NULL, *group, *group_items, *group_item = NULL;
	zval *escaped_item = NULL, *joined_items = NULL, *having, *order;
	zval *order_items, *order_item = NULL, *limit, *number, *for_update;
	zend_string *str_key;
	ulong idx;

	PHALCON_MM_GROW();

	dependency_injector = phalcon_read_property(getThis(), SL("_dependencyInjector"), PH_NOISY);
	if (Z_TYPE_P(dependency_injector) != IS_OBJECT) {
		dependency_injector = NULL;
		PHALCON_CALL_CE_STATIC(&dependency_injector, phalcon_di_ce, "getdefault");
		phalcon_update_property_this(getThis(), SL("_dependencyInjector"), dependency_injector);
	}

	models = phalcon_read_property(getThis(), SL("_models"), PH_NOISY);
	if (Z_TYPE_P(models) == IS_ARRAY) { 
		if (!phalcon_fast_count_ev(models)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "At least one model is required to build the query");
			return;
		}
	} else if (!zend_is_true(models)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "At least one model is required to build the query");
		return;
	}

	PHALCON_CALL_SELF(&conditions, "getConditions");

	PHALCON_INIT_VAR(phql);

	distinct = phalcon_read_property(getThis(), SL("_distinct"), PH_NOISY);
	if (PHALCON_IS_BOOL(distinct)) {
		if (Z_TYPE_P(distinct) == IS_TRUE) {
			ZVAL_STRING(phql, "SELECT DISTINCT ");
		} else {
			ZVAL_STRING(phql, "SELECT ALL ");
		}
	}
	else {
		ZVAL_STRING(phql, "SELECT ");
	}

	columns = phalcon_read_property(getThis(), SL("_columns"), PH_NOISY);
	if (Z_TYPE_P(columns) != IS_NULL) {

		/** 
		 * Generate PHQL for columns
		 */
		if (Z_TYPE_P(columns) == IS_ARRAY) { 

			PHALCON_INIT_VAR(selected_columns);
			array_init(selected_columns);
			
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(columns), idx, str_key, column) {
				zval column_alias;
				if (str_key) {
					ZVAL_STR(&column_alias, str_key);
				} else {
					ZVAL_LONG(&column_alias, idx);
				}

				if (Z_TYPE(column_alias) == IS_LONG) {
					phalcon_array_append(selected_columns, column, PH_SEPARATE);
				} else {
					PHALCON_INIT_NVAR(aliased_column);
					PHALCON_CONCAT_VSV(aliased_column, column, " AS ", &column_alias);
					phalcon_array_append(selected_columns, aliased_column, PH_SEPARATE);
				}
			} ZEND_HASH_FOREACH_END();

			PHALCON_INIT_VAR(joined_columns);
			phalcon_fast_join_str(joined_columns, SL(", "), selected_columns);
			phalcon_concat_self(phql, joined_columns);
		} else {
			phalcon_concat_self(phql, columns);
		}
	} else {
		/** 
		 * Automatically generate an array of models
		 */
		if (Z_TYPE_P(models) == IS_ARRAY) { 

			PHALCON_INIT_NVAR(selected_columns);
			array_init(selected_columns);

			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(models), idx, str_key, model) {
				zval model_column_alias;
				if (str_key) {
					ZVAL_STR(&model_column_alias, str_key);
				} else {
					ZVAL_LONG(&model_column_alias, idx);
				}

				if (Z_TYPE(model_column_alias) == IS_LONG) {
					PHALCON_INIT_NVAR(selected_column);
					PHALCON_CONCAT_SVS(selected_column, "[", model, "].*");
				} else {
					PHALCON_INIT_NVAR(selected_column);
					PHALCON_CONCAT_SVS(selected_column, "[", &model_column_alias, "].*");
				}
				phalcon_array_append(selected_columns, selected_column, PH_SEPARATE);
			} ZEND_HASH_FOREACH_END();

			PHALCON_INIT_NVAR(joined_columns);
			phalcon_fast_join_str(joined_columns, SL(", "), selected_columns);
			phalcon_concat_self(phql, joined_columns);
		} else {
			PHALCON_SCONCAT_SVS(phql, "[", models, "].*");
		}
	}

	/** 
	 * Join multiple models or use a single one if it is a string
	 */
	if (Z_TYPE_P(models) == IS_ARRAY) { 

		PHALCON_INIT_VAR(selected_models);
		array_init(selected_models);

		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(models), idx, str_key, model) {
			zval model_alias;
			if (str_key) {
				ZVAL_STR(&model_alias, str_key);
			} else {
				ZVAL_LONG(&model_alias, idx);
			}

			if (Z_TYPE(model_alias) == IS_STRING) {
				PHALCON_INIT_NVAR(selected_model);
				PHALCON_CONCAT_SVSVS(selected_model, "[", model, "] AS [", &model_alias, "]");
			} else {
				PHALCON_INIT_NVAR(selected_model);
				PHALCON_CONCAT_SVS(selected_model, "[", model, "]");
			}
			phalcon_array_append(selected_models, selected_model, PH_SEPARATE);
		} ZEND_HASH_FOREACH_END();

		PHALCON_INIT_VAR(joined_models);
		phalcon_fast_join_str(joined_models, SL(", "), selected_models);
		PHALCON_SCONCAT_SV(phql, " FROM ", joined_models);
	} else {
		PHALCON_SCONCAT_SVS(phql, " FROM [", models, "]");
	}

	/** 
	 * Check if joins were passed to the builders
	 */
	joins = phalcon_read_property(getThis(), SL("_joins"), PH_NOISY);
	if (Z_TYPE_P(joins) == IS_ARRAY) { 

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(joins), join) {
			/** 
			 * The joined table is in the first place of the array
			 */
			PHALCON_OBS_NVAR(join_model);
			phalcon_array_fetch_long(&join_model, join, 0, PH_NOISY);

			/** 
			 * The join conditions are in the second place of the array
			 */
			PHALCON_OBS_NVAR(join_conditions);
			phalcon_array_fetch_long(&join_conditions, join, 1, PH_NOISY);

			/** 
			 * The join alias is in the second place of the array
			 */
			PHALCON_OBS_NVAR(join_alias);
			phalcon_array_fetch_long(&join_alias, join, 2, PH_NOISY);

			/** 
			 * Join type
			 */
			PHALCON_OBS_NVAR(join_type);
			phalcon_array_fetch_long(&join_type, join, 3, PH_NOISY);

			/** 
			 * Create the join according to the type
			 */
			if (zend_is_true(join_type)) {
				PHALCON_SCONCAT_SVSVS(phql, " ", join_type, " JOIN [", join_model, "]");
			} else {
				PHALCON_SCONCAT_SVS(phql, " JOIN [", join_model, "]");
			}

			/** 
			 * Alias comes first
			 */
			if (zend_is_true(join_alias)) {
				PHALCON_SCONCAT_SVS(phql, " AS [", join_alias, "]");
			}

			/** 
			 * Conditions then
			 */
			if (zend_is_true(join_conditions)) {
				PHALCON_SCONCAT_SV(phql, " ON ", join_conditions);
			}
		} ZEND_HASH_FOREACH_END();

	}

	/** 
	 * Only append conditions if it's string
	 */
	if (Z_TYPE_P(conditions) == IS_STRING) {
		if (PHALCON_IS_NOT_EMPTY(conditions)) {
			PHALCON_SCONCAT_SV(phql, " WHERE ", conditions);
		}
	}

	/** 
	 * Process group parameters
	 */
	group = phalcon_read_property(getThis(), SL("_group"), PH_NOISY);
	if (Z_TYPE_P(group) != IS_NULL) {
		if (Z_TYPE_P(group) == IS_ARRAY) { 

			PHALCON_INIT_VAR(group_items);
			array_init(group_items);

			ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(group), group_item) {
				if (phalcon_is_numeric(group_item)) {
					phalcon_array_append(group_items, group_item, PH_SEPARATE);
				} else {
					if (phalcon_memnstr_str(group_item, SL("."))) {
						phalcon_array_append(group_items, group_item, PH_SEPARATE);
					} else {
						PHALCON_INIT_NVAR(escaped_item);
						PHALCON_CONCAT_SVS(escaped_item, "[", group_item, "]");
						phalcon_array_append(group_items, escaped_item, PH_SEPARATE);
					}
				}
			} ZEND_HASH_FOREACH_END();

			PHALCON_INIT_VAR(joined_items);
			phalcon_fast_join_str(joined_items, SL(", "), group_items);
			PHALCON_SCONCAT_SV(phql, " GROUP BY ", joined_items);
		} else {
			if (phalcon_is_numeric(group)) {
				PHALCON_SCONCAT_SV(phql, " GROUP BY ", group);
			} else {
				if (phalcon_memnstr_str(group, SL("."))) {
					PHALCON_SCONCAT_SV(phql, " GROUP BY ", group);
				} else if (phalcon_memnstr_str(group, SL(","))) {
					PHALCON_INIT_VAR(group_items);
					phalcon_fast_explode_str(group_items, SL(", "), group);

					PHALCON_INIT_VAR(joined_items);
					phalcon_fast_join_str(joined_items, SL("], ["), group_items);

					PHALCON_SCONCAT_SVS(phql, " GROUP BY [", joined_items, "]");
				} else {
					PHALCON_SCONCAT_SVS(phql, " GROUP BY [", group, "]");
				}
			}
		}
	}

	/* Process HAVING clause */
	having = phalcon_read_property(getThis(), SL("_having"), PH_NOISY);
	if (Z_TYPE_P(having) != IS_NULL) {
		if (PHALCON_IS_NOT_EMPTY(having)) {
			PHALCON_SCONCAT_SV(phql, " HAVING ", having);
		}
	}

	/** 
	 * Process order clause
	 */
	order = phalcon_read_property(getThis(), SL("_order"), PH_NOISY);
	if (PHALCON_IS_NOT_EMPTY(order)) {
		if (Z_TYPE_P(order) == IS_ARRAY) { 

			PHALCON_INIT_VAR(order_items);
			array_init(order_items);

			ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(order), order_item) {
				if (phalcon_is_numeric(order_item)) {
					phalcon_array_append(order_items, order_item, PH_SEPARATE);
				} else {
					if (phalcon_memnstr_str(order_item, SL("."))) {
						phalcon_array_append(order_items, order_item, PH_SEPARATE);
					} else {
						PHALCON_INIT_NVAR(escaped_item);
						PHALCON_CONCAT_SVS(escaped_item, "[", order_item, "]");
						phalcon_array_append(order_items, escaped_item, PH_SEPARATE);
					}
				}
			} ZEND_HASH_FOREACH_END();

			PHALCON_INIT_NVAR(joined_items);
			phalcon_fast_join_str(joined_items, SL(", "), order_items);
			PHALCON_SCONCAT_SV(phql, " ORDER BY ", joined_items);
		} else {
			PHALCON_SCONCAT_SV(phql, " ORDER BY ", order);
		}
	}

	/** 
	 * Process limit parameters
	 */
	limit = phalcon_read_property(getThis(), SL("_limit"), PH_NOISY);
	if (Z_TYPE_P(limit) != IS_NULL) {
		if (Z_TYPE_P(limit) == IS_ARRAY) { 
			zval *offset;

			PHALCON_OBS_VAR(number);
			phalcon_array_fetch_str(&number, limit, SL("number"), PH_NOISY);
			if (phalcon_array_isset_str_fetch(&offset, limit, SL("offset")) && Z_TYPE_P(offset) != IS_NULL) {
				PHALCON_SCONCAT_SVSV(phql, " LIMIT ", number, " OFFSET ", offset);
			} else {
				PHALCON_SCONCAT_SV(phql, " LIMIT ", number);
			}
		} else {
			zval *offset;

			PHALCON_SCONCAT_SV(phql, " LIMIT ", limit);

			offset = phalcon_read_property(getThis(), SL("_offset"), PH_NOISY);
			if (Z_TYPE_P(offset) != IS_NULL) {
				PHALCON_SCONCAT_SV(phql, " OFFSET ", offset);
			}
		}
	}

	/** 
	 * Process FOR UPDATE clause
	 */
	for_update = phalcon_read_property(getThis(), SL("_forUpdate"), PH_NOISY);
	if (zend_is_true(for_update)) {
		phalcon_concat_self_str(phql, SL(" FOR UPDATE"));
	}

	RETURN_CTOR(phql);
}

/**
 * Returns the query built
 *
 * @return Phalcon\Mvc\Model\Query
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getQuery){

	zval *phql = NULL, *dependency_injector, *service_name, *has = NULL, *parameters, *query = NULL, *bind_params, *bind_types;

	PHALCON_MM_GROW();

	/** 
	 * Process the PHQL
	 */
	PHALCON_CALL_METHOD(&phql, getThis(), "getphql");

	bind_params = phalcon_read_property(getThis(), SL("_bindParams"), PH_NOISY);
	bind_types = phalcon_read_property(getThis(), SL("_bindTypes"), PH_NOISY);

	dependency_injector = phalcon_read_property(getThis(), SL("_dependencyInjector"), PH_NOISY);

	PHALCON_INIT_VAR(service_name);
	ZVAL_STRING(service_name, "modelsQuery");

	PHALCON_CALL_METHOD(&has, dependency_injector, "has", service_name);
	if (zend_is_true(has)) {
		PHALCON_INIT_VAR(parameters);
		array_init(parameters);

		phalcon_array_append(parameters, phql, 0);
		phalcon_array_append(parameters, dependency_injector, 0);

		PHALCON_CALL_METHOD(&query, dependency_injector, "get", service_name, parameters);
	} else {
		PHALCON_INIT_NVAR(query);
		object_init_ex(query, phalcon_mvc_model_query_ce);
		PHALCON_CALL_METHOD(NULL, query, "__construct", phql, dependency_injector);
	}

	/** 
	 * Set default bind params
	 */
	if (Z_TYPE_P(bind_params) == IS_ARRAY) { 
		PHALCON_CALL_METHOD(NULL, query, "setbindparams", bind_params);
	}

	/** 
	 * Set default bind params
	 */
	if (Z_TYPE_P(bind_types) == IS_ARRAY) { 
		PHALCON_CALL_METHOD(NULL, query, "setbindtypes", bind_types);
	}

	RETURN_CTOR(query);
}

/**
 * Returns the conditions, If the conditions is a single numeric field. We internally create a condition
 * using the related primary key
 *
 *<code>
 *	$builder->getConditions();
 *</code>
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder, getConditions){

	zval *conditions, *dependency_injector = NULL, *models;
	zval *z_one, *number_models, *invalid_condition;
	zval *model = NULL, *service_name, *has = NULL, *meta_data = NULL, *model_instance;
	zval *no_primary = NULL, *primary_keys = NULL, *first_primary_key;
	zval *column_map = NULL, *attribute_field = NULL, *exception_message;
	zval *primary_key_condition;
	zend_class_entry *ce0;

	PHALCON_MM_GROW();

	conditions = phalcon_read_property(getThis(), SL("_conditions"), PH_NOISY);

	if (phalcon_is_numeric(conditions)) {
		dependency_injector = phalcon_read_property(getThis(), SL("_dependencyInjector"), PH_NOISY);
		if (Z_TYPE_P(dependency_injector) != IS_OBJECT) {
			dependency_injector = NULL;
			PHALCON_CALL_CE_STATIC(&dependency_injector, phalcon_di_ce, "getdefault");
			phalcon_update_property_this(getThis(), SL("_dependencyInjector"), dependency_injector);
		}

		models = phalcon_read_property(getThis(), SL("_models"), PH_NOISY);
		if (Z_TYPE_P(models) == IS_ARRAY) { 
			if (!phalcon_fast_count_ev(models)) {
				PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "At least one model is required to build the query");
				return;
			}
		} else if (!zend_is_true(models)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "At least one model is required to build the query");
			return;
		}

		/** 
		 * If the conditions is a single numeric field. We internally create a condition
		 * using the related primary key
		 */
		if (Z_TYPE_P(models) == IS_ARRAY) { 

			z_one = &PHALCON_GLOBAL(z_one);

			PHALCON_INIT_VAR(number_models);
			phalcon_fast_count(number_models, models);

			PHALCON_INIT_VAR(invalid_condition);
			is_smaller_function(invalid_condition, z_one, number_models);
			if (PHALCON_IS_TRUE(invalid_condition)) {
				PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Cannot build the query. Invalid condition");
				return;
			}

			PHALCON_OBS_VAR(model);
			phalcon_array_fetch_long(&model, models, 0, PH_NOISY);
		} else {
			PHALCON_CPY_WRT(model, models);
		}

		PHALCON_INIT_VAR(service_name);
		ZVAL_STR(service_name, IS(modelsMetadata));

		PHALCON_CALL_METHOD(&has, dependency_injector, "has", service_name);
		if (zend_is_true(has)) {
			/** 
			 * Get the models metadata service to obtain the column names, column map and
			 * primary key
			 */
			PHALCON_CALL_METHOD(&meta_data, dependency_injector, "getshared", service_name);
			PHALCON_VERIFY_INTERFACE(meta_data, phalcon_mvc_model_metadatainterface_ce);
		} else {
			PHALCON_INIT_NVAR(meta_data);
			object_init_ex(meta_data, phalcon_mvc_model_metadata_memory_ce);
		}

		ce0 = phalcon_fetch_class(model);

		PHALCON_INIT_VAR(model_instance);
		object_init_ex(model_instance, ce0);
		if (phalcon_has_constructor(model_instance)) {
			PHALCON_CALL_METHOD(NULL, model_instance, "__construct", dependency_injector);
		}

		PHALCON_INIT_VAR(no_primary);
		ZVAL_TRUE(no_primary);

		PHALCON_CALL_METHOD(&primary_keys, meta_data, "getprimarykeyattributes", model_instance);
		if (phalcon_fast_count_ev(primary_keys)) {
			if (phalcon_array_isset_long(primary_keys, 0)) {

				PHALCON_OBS_VAR(first_primary_key);
				phalcon_array_fetch_long(&first_primary_key, primary_keys, 0, PH_NOISY);

				/** 
				 * The PHQL contains the renamed columns if available
				 */
				if (PHALCON_GLOBAL(orm).column_renaming) {
					PHALCON_CALL_METHOD(&column_map, meta_data, "getcolumnmap", model_instance);
				} else {
					PHALCON_INIT_VAR(column_map);
				}

				if (Z_TYPE_P(column_map) == IS_ARRAY) { 
					if (phalcon_array_isset(column_map, first_primary_key)) {
						PHALCON_OBS_VAR(attribute_field);
						phalcon_array_fetch(&attribute_field, column_map, first_primary_key, PH_NOISY);
					} else {
						PHALCON_INIT_VAR(exception_message);
						PHALCON_CONCAT_SVS(exception_message, "Column '", first_primary_key, "\" isn't part of the column map");
						PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
						return;
					}
				} else {
					PHALCON_CPY_WRT(attribute_field, first_primary_key);
				}

				PHALCON_INIT_VAR(primary_key_condition);
				PHALCON_CONCAT_SVSVSV(primary_key_condition, "[", model, "].[", attribute_field, "] = ", conditions);
				PHALCON_CPY_WRT(conditions, primary_key_condition);

				ZVAL_FALSE(no_primary);
			}
		}

		/** 
		 * A primary key is mandatory in these cases
		 */
		if (PHALCON_IS_TRUE(no_primary)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Source related to this model does not have a primary key defined");
			return;
		}

		phalcon_update_property_this(getThis(), SL("_conditions"), conditions);
	}

	RETURN_CTOR(conditions);
}

