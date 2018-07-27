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

#include "mvc/model/query/builder/select.h"
#include "mvc/model/query/builder/join.h"
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
#include "kernel/debug.h"

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
	ZEND_ARG_TYPE_INFO(0, params, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select_distinct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, distinct, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select_columns, 0, 0, 1)
	ZEND_ARG_INFO(0, columns)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select_from, 0, 0, 1)
	ZEND_ARG_INFO(0, model)
	ZEND_ARG_TYPE_INFO(0, alias, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, merge, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select_addfrom, 0, 0, 1)
	ZEND_ARG_INFO(0, model)
	ZEND_ARG_TYPE_INFO(0, alias, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select_orderby, 0, 0, 1)
	ZEND_ARG_INFO(0, orderBy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select_having, 0, 0, 1)
	ZEND_ARG_INFO(0, having)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select_limit, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, limit, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_select_offset, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, offset, IS_LONG, 1)
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
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Select, _compile, NULL, ZEND_ACC_PROTECTED)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Query\Builder\Select initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Query_Builder_Select){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\Query\\Builder, Select, mvc_model_query_builder_select, phalcon_mvc_model_query_builder_join_ce, phalcon_mvc_model_query_builder_select_method_entry, 0);

	zend_declare_property_long(phalcon_mvc_model_query_builder_select_ce, SL("_type"), PHQL_T_SELECT, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_columns"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_models"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_group"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_having"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_order"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_limit"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_offset"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_forUpdate"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_sharedLock"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_distinct"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_query_builder_select_ce, SL("_cache"), ZEND_ACC_PROTECTED);

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
 * );
 * $queryBuilder = new Phalcon\Mvc\Model\Query\Builder\Select($params);
 *</code>
 *
 * @param array $params
 * @param Phalcon\Di $dependencyInjector
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, __construct){

	zval *params = NULL, *dependency_injector = NULL;

	phalcon_fetch_params(0, 0, 2, &params, &dependency_injector);

	/**
	 * Update the dependency injector if any
	 */
	if (dependency_injector && Z_TYPE_P(dependency_injector) != IS_NULL) {
		PHALCON_CALL_METHOD(NULL, getThis(), "setdi", dependency_injector);
	}

	if (params && Z_TYPE_P(params) == IS_ARRAY) {
		zval cache = {}, conditions = {}, bind_params = {}, bind_types = {}, models = {}, index = {}, columns = {}, group_clause = {}, joins = {};
		zval having_clause = {}, order_clause = {}, limit_clause = {}, offset_clause = {}, limit = {}, offset = {}, for_update = {}, shared_lock = {};

		if (phalcon_array_isset_fetch_str(&cache, params, SL("cache"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_cache"), &cache);
		}

		/**
		 * Process conditions
		 */
		if (phalcon_array_isset_fetch_str(&conditions, params, SL("conditions"), PH_READONLY)
			|| phalcon_array_isset_fetch_long(&conditions, params, 0, PH_READONLY)) {
			PHALCON_CALL_METHOD(NULL, getThis(), "setconditions", &conditions);
		}

		if (phalcon_array_isset_fetch_str(&bind_params, params, SL("bind"), PH_READONLY)) {
			PHALCON_CALL_METHOD(NULL, getThis(), "setbindparams", &bind_params);
		} else if (phalcon_array_isset_fetch_str(&bind_params, params, SL("bindParams"), PH_READONLY)) {
			PHALCON_CALL_METHOD(NULL, getThis(), "setbindparams", &bind_params);
		}

		if (phalcon_array_isset_fetch_str(&bind_types, params, SL("bindTypes"), PH_READONLY)) {
			PHALCON_CALL_METHOD(NULL, getThis(), "setbindtypes", &bind_types);
		}

		/**
		 * Assign 'FROM' clause
		 */
		if (phalcon_array_isset_fetch_str(&models, params, SL("models"), PH_READONLY)) {
			PHALCON_CALL_METHOD(NULL, getThis(), "from", &models);
		}

		if (phalcon_array_isset_fetch_str(&index, params, SL("index"), PH_READONLY)) {
			PHALCON_CALL_METHOD(NULL, getThis(), "setindex", &index);
		}

		/**
		 * Assign COLUMNS clause
		 */
		if (phalcon_array_isset_fetch_str(&columns, params, SL("columns"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_columns"), &columns);
		}

		/**
		 * Assign JOIN clause
		 */
		if (phalcon_array_isset_fetch_str(&joins, params, SL("joins"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_joins"), &joins);
		}

		/**
		 * Assign GROUP clause
		 */
		if (phalcon_array_isset_fetch_str(&group_clause, params, SL("group"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_group"), &group_clause);
		}

		/**
		 * Assign HAVING clause
		 */
		if (phalcon_array_isset_fetch_str(&having_clause, params, SL("having"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_having"), &having_clause);
		}

		/**
		 * Assign ORDER clause
		 */
		if (phalcon_array_isset_fetch_str(&order_clause, params, SL("order"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_order"), &order_clause);
		}

		/**
		 * Assign LIMIT clause
		 */
		if (phalcon_array_isset_fetch_str(&limit_clause, params, SL("limit"), PH_READONLY)) {
			if (Z_TYPE(limit_clause) == IS_ARRAY
				&& phalcon_array_isset_fetch_long(&limit, &limit_clause, 0, PH_READONLY)
				&& phalcon_array_isset_fetch_long(&offset, &limit_clause, 1, PH_READONLY)
			) {
				phalcon_update_property(getThis(), SL("_limit"), &limit);
				phalcon_update_property(getThis(), SL("_offset"), &offset);
			} else {
				phalcon_update_property(getThis(), SL("_limit"), &limit_clause);
			}
		}

		/**
		 * Assign OFFSET clause
		 */
		if (phalcon_array_isset_fetch_str(&offset_clause, params, SL("offset"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_offset"), &offset_clause);
		}

		/**
		 * Assign FOR UPDATE clause
		 */
		if (phalcon_array_isset_fetch_str(&for_update, params, SL("for_update"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_forUpdate"), &for_update);
		} else if (phalcon_array_isset_fetch_str(&for_update, params, SL("forUpdate"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_forUpdate"), &for_update);
		}

		/**
		 * Assign SHARED LOCK clause
		 */
		if (phalcon_array_isset_fetch_str(&shared_lock, params, SL("shared_lock"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_sharedLock"), &shared_lock);
		} else if (phalcon_array_isset_fetch_str(&shared_lock, params, SL("sharedLock"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_sharedLock"), &shared_lock);
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

	phalcon_update_property(getThis(), SL("_distinct"), distinct);
	RETURN_THIS();
}

/**
 * Returns SELECT DISTINCT / SELECT ALL flag
 *
 * @return boolean
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

	phalcon_update_property(getThis(), SL("_columns"), columns);
	RETURN_THIS();
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
 * @param string|array $model
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, from){

	zval *model, *alias = NULL, *merge = NULL, models = {};

	phalcon_fetch_params(0, 1, 2, &model, &alias, &merge);

	if (!alias) {
		alias = &PHALCON_GLOBAL(z_null);
	}

	if (!merge) {
		merge = &PHALCON_GLOBAL(z_false);
	}

	if (zend_is_true(merge)) {
		phalcon_read_property(&models, getThis(), SL("_models"), PH_COPY);
		if (Z_TYPE(models) != IS_ARRAY) {
			array_init(&models);
		}
	} else {
		array_init(&models);
	}

	if (Z_TYPE_P(model) == IS_ARRAY) {
		phalcon_array_merge_recursive_n(&models, model);
	} else if (Z_TYPE_P(model) != IS_NULL) {
		if (Z_TYPE_P(alias) == IS_STRING) {
			phalcon_array_update(&models, alias, model, PH_COPY);
		} else {
			phalcon_array_append(&models, model, PH_COPY);
		}
	}

	phalcon_update_property(getThis(), SL("_models"), &models);
	zval_ptr_dtor(&models);

	RETURN_THIS();
}

/**
 * Add a model to take part of the query
 *
 *<code>
 *	$builder->addFrom('Robots', 'r');
 *</code>
 *
 * @param string|array $model
 * @param string $alias
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, addFrom){

	zval *model, *alias = NULL;

	phalcon_fetch_params(0, 1, 1, &model, &alias);

	if (!alias) {
		alias = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_METHOD(NULL, getThis(), "from", model, alias, &PHALCON_GLOBAL(z_true));

	RETURN_THIS();
}

/**
 * Return the models who makes part of the query
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, getFrom){


	RETURN_MEMBER(getThis(), "_models");
}

/**
 * Sets a ORDER BY condition clause
 *
 *<code>
 *	$builder->orderBy('Robots.name');
 *	$builder->orderBy(array('1', 'Robots.name'));
 *</code>
 *
 * @param string|array $orderBy
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, orderBy){

	zval *order_by;

	phalcon_fetch_params(0, 1, 0, &order_by);

	phalcon_update_property(getThis(), SL("_order"), order_by);
	RETURN_THIS();
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

	phalcon_update_property(getThis(), SL("_having"), having);
	RETURN_THIS();
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

	phalcon_update_property(getThis(), SL("_limit"), limit);

	if (offset) {
		phalcon_update_property(getThis(), SL("_offset"), offset);
	}

	RETURN_THIS();
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
 * @param int $offset
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, offset){

	zval *offset;

	phalcon_fetch_params(0, 1, 0, &offset);

	phalcon_update_property(getThis(), SL("_offset"), offset);
	RETURN_THIS();
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
 * @param string|array $group
 * @return Phalcon\Mvc\Model\Query\Builder\Select
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Select, groupBy){

	zval *group;

	phalcon_fetch_params(0, 1, 0, &group);

	phalcon_update_property(getThis(), SL("_group"), group);
	RETURN_THIS();
}

/**
 * Returns the GROUP BY clause
 *
 * @return string|array
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

	zval models = {}, *model, conditions = {}, distinct = {}, phql = {}, columns = {}, selected_columns = {};
	zval *column, joined_columns = {}, selected_models = {}, joined_models = {}, joins = {}, *join, group = {};
	zval having = {}, order = {}, limit = {}, offset = {}, for_update = {}, bind_params = {}, bind_types = {};
	zend_string *str_key;
	ulong idx;

	PHALCON_MM_INIT();
	phalcon_read_property(&models, getThis(), SL("_models"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(models) == IS_ARRAY) {
		if (!phalcon_fast_count_ev(&models)) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_query_exception_ce, "At least one model is required to build the query");
			return;
		}
	} else {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_query_exception_ce, "At least one model is required to build the query");
		return;
	}
	PHALCON_MM_CALL_SELF(&conditions, "getConditions");
	PHALCON_MM_ADD_ENTRY(&conditions);
	phalcon_read_property(&distinct, getThis(), SL("_distinct"), PH_NOISY|PH_READONLY);
	if (PHALCON_IS_BOOL(&distinct)) {
		if (Z_TYPE(distinct) == IS_TRUE) {
			PHALCON_MM_ZVAL_STRING(&phql, "SELECT DISTINCT ");
		} else {
			PHALCON_MM_ZVAL_STRING(&phql, "SELECT ALL ");
		}
	} else {
		PHALCON_MM_ZVAL_STRING(&phql, "SELECT ");
	}

	phalcon_read_property(&columns, getThis(), SL("_columns"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(columns) != IS_NULL) {
		/**
		 * Generate PHQL for columns
		 */
		if (Z_TYPE(columns) == IS_ARRAY) {

			array_init(&selected_columns);
			PHALCON_MM_ADD_ENTRY(&selected_columns);
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
					phalcon_array_append(&selected_columns, &aliased_column, 0);
				}
			} ZEND_HASH_FOREACH_END();

			phalcon_fast_join_str(&joined_columns, SL(", "), &selected_columns);
			phalcon_concat_self(&phql, &joined_columns);
			zval_ptr_dtor(&joined_columns);
			PHALCON_MM_ADD_ENTRY(&phql);
		} else {
			phalcon_concat_self(&phql, &columns);
			PHALCON_MM_ADD_ENTRY(&phql);
		}
	} else {
		/**
		 * Automatically generate an array of models
		 */
		if (Z_TYPE(models) == IS_ARRAY) {
			array_init(&selected_columns);
			PHALCON_MM_ADD_ENTRY(&selected_columns);
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
				phalcon_array_append(&selected_columns, &selected_column, 0);
			} ZEND_HASH_FOREACH_END();

			phalcon_fast_join_str(&joined_columns, SL(", "), &selected_columns);
			phalcon_concat_self(&phql, &joined_columns);
			zval_ptr_dtor(&joined_columns);
			PHALCON_MM_ADD_ENTRY(&phql);
		} else {
			PHALCON_SCONCAT_SVS(&phql, "[", &models, "].*");
			PHALCON_MM_ADD_ENTRY(&phql);
		}
	}

	/**
	 * Join multiple models or use a single one if it is a string
	 */
	if (Z_TYPE(models) == IS_ARRAY) {
		array_init(&selected_models);
		PHALCON_MM_ADD_ENTRY(&selected_models);
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
			phalcon_array_append(&selected_models, &selected_model, 0);
		} ZEND_HASH_FOREACH_END();

		phalcon_fast_join_str(&joined_models, SL(", "), &selected_models);
		PHALCON_SCONCAT_SV(&phql, " FROM ", &joined_models);
		zval_ptr_dtor(&joined_models);
		PHALCON_MM_ADD_ENTRY(&phql);
	} else {
		PHALCON_SCONCAT_SVS(&phql, " FROM [", &models, "]");
		PHALCON_MM_ADD_ENTRY(&phql);
	}

	/**
	 * Check if joins were passed to the builders
	 */
	phalcon_read_property(&joins, getThis(), SL("_joins"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(joins) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(joins), join) {
			zval join_model = {}, join_conditions = {}, join_alias = {}, join_type = {};
			/**
			 * The joined table is in the first place of the array
			 */
			phalcon_array_fetch_long(&join_model, join, 0, PH_NOISY|PH_READONLY);

			/**
			 * The join conditions are in the second place of the array
			 */
			phalcon_array_fetch_long(&join_conditions, join, 1, PH_NOISY|PH_READONLY);

			/**
			 * The join alias is in the second place of the array
			 */
			phalcon_array_fetch_long(&join_alias, join, 2, PH_NOISY|PH_READONLY);

			/**
			 * Join type
			 */
			phalcon_array_fetch_long(&join_type, join, 3, PH_NOISY|PH_READONLY);

			/**
			 * Create the join according to the type
			 */
			if (zend_is_true(&join_type)) {
				PHALCON_SCONCAT_SVSVS(&phql, " ", &join_type, " JOIN [", &join_model, "]");
			} else {
				PHALCON_SCONCAT_SVS(&phql, " JOIN [", &join_model, "]");
			}
			PHALCON_MM_ADD_ENTRY(&phql);

			/**
			 * Alias comes first
			 */
			if (zend_is_true(&join_alias)) {
				PHALCON_SCONCAT_SVS(&phql, " AS [", &join_alias, "]");
				PHALCON_MM_ADD_ENTRY(&phql);
			}

			/**
			 * Conditions then
			 */
			if (zend_is_true(&join_conditions)) {
				PHALCON_SCONCAT_SV(&phql, " ON ", &join_conditions);
				PHALCON_MM_ADD_ENTRY(&phql);
			}
		} ZEND_HASH_FOREACH_END();
	}

	/**
	 * Only append conditions if it's string
	 */
	if (Z_TYPE(conditions) == IS_STRING && PHALCON_IS_NOT_EMPTY(&conditions)) {
		PHALCON_SCONCAT_SV(&phql, " WHERE ", &conditions);
		PHALCON_MM_ADD_ENTRY(&phql);
	}

	/**
	 * Process group parameters
	 */
	phalcon_read_property(&group, getThis(), SL("_group"), PH_NOISY|PH_READONLY);
	if (PHALCON_IS_NOT_EMPTY(&group)) {
		phalcon_orm_phql_build_group(&phql, &group);
		PHALCON_MM_ADD_ENTRY(&phql);
	}

	/* Process HAVING clause */
	phalcon_read_property(&having, getThis(), SL("_having"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(having) != IS_NULL) {
		if (PHALCON_IS_NOT_EMPTY(&having)) {
			PHALCON_SCONCAT_SV(&phql, " HAVING ", &having);
			PHALCON_MM_ADD_ENTRY(&phql);
		}
	}

	/**
	 * Process order clause
	 */
	phalcon_read_property(&order, getThis(), SL("_order"), PH_NOISY|PH_READONLY);
	if (PHALCON_IS_NOT_EMPTY(&order)) {
		phalcon_orm_phql_build_order(&phql, &order);
		PHALCON_MM_ADD_ENTRY(&phql);
	}

	/**
	 * Process limit parameters
	 */
	phalcon_read_property(&limit, getThis(), SL("_limit"), PH_NOISY|PH_READONLY);
	if (PHALCON_IS_NOT_EMPTY(&limit)) {
		if (Z_TYPE(limit) != IS_ARRAY) {
			zval tmp = {};
			phalcon_read_property(&offset, getThis(), SL("_offset"), PH_READONLY);
			if (PHALCON_IS_NOT_EMPTY(&offset)) {
				PHALCON_CONCAT_VSV(&tmp, &limit, " OFFSET ", &offset);
				PHALCON_MM_ADD_ENTRY(&tmp);
			} else {
				ZVAL_COPY_VALUE(&tmp, &limit);
			}
			if (PHALCON_IS_NOT_EMPTY(&tmp)) {
				phalcon_orm_phql_build_limit(&phql, &tmp);
				PHALCON_MM_ADD_ENTRY(&phql);
			}
		} else if (PHALCON_IS_NOT_EMPTY(&limit)) {
			phalcon_orm_phql_build_limit(&phql, &limit);
			PHALCON_MM_ADD_ENTRY(&phql);
		}
	}

	/**
	 * Process FOR UPDATE clause
	 */
	phalcon_read_property(&for_update, getThis(), SL("_forUpdate"), PH_NOISY|PH_READONLY);
	if (zend_is_true(&for_update)) {
		phalcon_concat_self_str(&phql, SL(" FOR UPDATE"));
		PHALCON_MM_ADD_ENTRY(&phql);
	}

	PHALCON_MM_CALL_SELF(&bind_params, "getbindparams");
	phalcon_update_property(getThis(), SL("_mergeBindParams"), &bind_params);
	zval_ptr_dtor(&bind_params);

	PHALCON_MM_CALL_SELF(&bind_types, "getbindtypes");
	phalcon_update_property(getThis(), SL("_mergeBindTypes"), &bind_types);
	zval_ptr_dtor(&bind_types);

	phalcon_update_property(getThis(), SL("_phql"), &phql);
	RETURN_MM();
}
