
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

#include "mvc/model/criteria.h"
#include "mvc/model/criteriainterface.h"
#include "mvc/model/metadatainterface.h"
#include "mvc/modelinterface.h"
#include "mvc/model/exception.h"
#include "mvc/model/query.h"
#include "db/column.h"
#include "di.h"
#include "diinterface.h"
#include "di/injectable.h"
#include "mvc/model/query/scanner.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/file.h"
#include "kernel/hash.h"
#include "kernel/debug.h"
#include "kernel/framework/orm.h"

#include "interned-strings.h"
#include "../modelinterface.h"

/**
 * Phalcon\Mvc\Model\Criteria
 *
 * This class allows to build the array parameter required by Phalcon\Mvc\Model::find
 * and Phalcon\Mvc\Model::findFirst using an object-oriented interface
 *
 *<code>
 *$robots = Robots::query()
 *    ->where("type = :type:")
 *    ->andWhere("year < 2000")
 *    ->bind(array("type" => "mechanical"))
 *    ->order("name")
 *    ->execute();
 *</code>
 */
zend_class_entry *phalcon_mvc_model_criteria_ce;

PHP_METHOD(Phalcon_Mvc_Model_Criteria, setModelName);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, getModelName);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, bind);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, bindTypes);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, columns);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, getColumns);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, join);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, innerJoin);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, leftJoin);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, rightJoin);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, where);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, andWhere);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, orWhere);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, betweenWhere);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, notBetweenWhere);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, inWhere);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, notInWhere);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, getWhere);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, conditions);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, getConditions);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, orderBy);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, getOrder);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, limit);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, getLimit);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, setUniqueRow);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, getUniqueRow);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, forUpdate);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, setIndex);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, getIndex);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, sharedLock);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, getParams);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, fromInput);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, groupBy);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, having);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, execute);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, count);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, cache);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, insert);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, update);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, delete);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, getPhql);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, _generateSelect);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, _generateInsert);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, _generateUpdate);
PHP_METHOD(Phalcon_Mvc_Model_Criteria, _generateDelete);


ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_criteria_innerjoin, 0, 0, 1)
	ZEND_ARG_INFO(0, model)
	ZEND_ARG_INFO(0, conditions)
	ZEND_ARG_INFO(0, alias)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_criteria_leftjoin, 0, 0, 1)
	ZEND_ARG_INFO(0, model)
	ZEND_ARG_INFO(0, conditions)
	ZEND_ARG_INFO(0, alias)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_criteria_rightjoin, 0, 0, 1)
	ZEND_ARG_INFO(0, model)
	ZEND_ARG_INFO(0, conditions)
	ZEND_ARG_INFO(0, alias)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_criteria_groupby, 0, 0, 1)
	ZEND_ARG_INFO(0, group)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_criteria_having, 0, 0, 1)
	ZEND_ARG_INFO(0, having)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_criteria_cache, 0, 0, 1)
	ZEND_ARG_ARRAY_INFO(0, options, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_criteria_count, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, column, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_criteria_setindex, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, index, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_criteria_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Criteria, setModelName, arginfo_phalcon_mvc_model_criteriainterface_setmodelname, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, getModelName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, bind, arginfo_phalcon_mvc_model_criteriainterface_bind, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, bindTypes, arginfo_phalcon_mvc_model_criteriainterface_bindtypes, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Mvc_Model_Criteria, select, columns, arginfo_phalcon_mvc_model_criteriainterface_columns, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
	PHP_ME(Phalcon_Mvc_Model_Criteria, columns, arginfo_phalcon_mvc_model_criteriainterface_columns, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, getColumns, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, join, arginfo_phalcon_mvc_model_criteriainterface_join, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, innerJoin, arginfo_phalcon_mvc_model_criteria_innerjoin, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, leftJoin, arginfo_phalcon_mvc_model_criteria_leftjoin, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, rightJoin, arginfo_phalcon_mvc_model_criteria_rightjoin, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, where, arginfo_phalcon_mvc_model_criteriainterface_where, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Mvc_Model_Criteria, addWhere, andWhere, arginfo_phalcon_mvc_model_criteriainterface_andwhere, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
	PHP_ME(Phalcon_Mvc_Model_Criteria, andWhere, arginfo_phalcon_mvc_model_criteriainterface_andwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, orWhere, arginfo_phalcon_mvc_model_criteriainterface_orwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, betweenWhere, arginfo_phalcon_mvc_model_criteriainterface_betweenwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, notBetweenWhere, arginfo_phalcon_mvc_model_criteriainterface_notbetweenwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, inWhere, arginfo_phalcon_mvc_model_criteriainterface_inwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, notInWhere, arginfo_phalcon_mvc_model_criteriainterface_notinwhere, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, getWhere, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, conditions, arginfo_phalcon_mvc_model_criteriainterface_conditions, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, getConditions, NULL, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Mvc_Model_Criteria, order, orderBy, arginfo_phalcon_mvc_model_criteriainterface_orderby, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
	PHP_ME(Phalcon_Mvc_Model_Criteria, orderBy, arginfo_phalcon_mvc_model_criteriainterface_orderby, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, getOrder, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, limit, arginfo_phalcon_mvc_model_criteriainterface_limit, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, getLimit, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, setUniqueRow, arginfo_phalcon_mvc_model_criteriainterface_setuniquerow, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, getUniqueRow, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, forUpdate, arginfo_phalcon_mvc_model_criteriainterface_forupdate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, setIndex, arginfo_phalcon_mvc_model_criteria_setindex, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, getIndex, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, sharedLock, arginfo_phalcon_mvc_model_criteriainterface_sharedlock, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, getParams, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, fromInput, arginfo_phalcon_mvc_model_criteriainterface_frominput, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, groupBy, arginfo_phalcon_mvc_model_criteria_groupby, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, having, arginfo_phalcon_mvc_model_criteria_having, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, execute, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, count, arginfo_phalcon_mvc_model_criteria_count, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, cache, arginfo_phalcon_mvc_model_criteria_cache, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, insert, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, update, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, delete, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, getPhql, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, _generateSelect, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, _generateInsert, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, _generateUpdate, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Criteria, _generateDelete, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Criteria initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Criteria) {

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model, Criteria, mvc_model_criteria, phalcon_di_injectable_ce, phalcon_mvc_model_criteria_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_model_criteria_ce, SL("_type"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_criteria_ce, SL("_model"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_criteria_ce, SL("_bindParams"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_criteria_ce, SL("_bindTypes"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_criteria_ce, SL("_columns"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_criteria_ce, SL("_values"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_criteria_ce, SL("_joins"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_criteria_ce, SL("_conditions"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_criteria_ce, SL("_order"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_criteria_ce, SL("_limit"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_criteria_ce, SL("_offset"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_criteria_ce, SL("_forUpdate"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_criteria_ce, SL("_index"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_criteria_ce, SL("_sharedLock"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_mvc_model_criteria_ce, SL("_hiddenParamNumber"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_criteria_ce, SL("_cacheOptions"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_criteria_ce, SL("_group"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_criteria_ce, SL("_having"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_criteria_ce, SL("_uniqueRow"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_mvc_model_criteria_ce, 1, phalcon_mvc_model_criteriainterface_ce);

	return SUCCESS;
}

/**
 * Set a model on which the query will be executed
 *
 * @param string $modelName
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, setModelName) {

	zval *model_name;
	zend_class_entry *ce0;

	phalcon_fetch_params(0, 1, 0, &model_name);

	if (Z_TYPE_P(model_name) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Model name must be string");
		return;
	}

	ce0 = phalcon_fetch_class(model_name, ZEND_FETCH_CLASS_DEFAULT);
	PHALCON_VERIFY_INTERFACE_CE_EX(ce0, phalcon_mvc_modelinterface_ce, phalcon_mvc_model_exception_ce);

	phalcon_update_property(getThis(), SL("_model"), model_name);

	RETURN_THIS();
}

/**
 * Returns an internal model name on which the criteria will be applied
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, getModelName) {

	RETURN_MEMBER(getThis(), "_model");
}

/**
 * Sets the bound parameters in the criteria
 * This method replaces all previously set bound parameters
 *
 * @param string $bindParams
 * @param boolean $merge
 *
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, bind) {

	zval *bind_params, *merge = NULL, current_bind_params = {}, merged_params = {};

	phalcon_fetch_params(0, 1, 1, &bind_params, &merge);

	if (!merge) {
		merge = &PHALCON_GLOBAL(z_false);
	}

	if (zend_is_true(merge)) {
		phalcon_read_property(&current_bind_params, getThis(), SL("_bindParams"), PH_NOISY|PH_READONLY);
		if (Z_TYPE(current_bind_params) == IS_ARRAY) {
			phalcon_fast_array_merge(&merged_params, &current_bind_params, bind_params);
		} else {
			ZVAL_COPY(&merged_params, bind_params);
		}

		phalcon_update_property(getThis(), SL("_bindParams"), &merged_params);
		zval_ptr_dtor(&merged_params);
	} else {
		phalcon_update_property(getThis(), SL("_bindParams"), bind_params);
	}

	RETURN_THIS();
}

/**
 * Sets the bind types in the criteria
 * This method replaces all previously set bound parameters
 *
 * @param array $bindTypes
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, bindTypes) {

	zval *bind_types, *merge = NULL, current_bind_types = {}, merged_types = {};

	phalcon_fetch_params(0, 1, 1, &bind_types, &merge);

	if (!merge) {
		merge = &PHALCON_GLOBAL(z_false);
	}

	if (zend_is_true(merge)) {
		phalcon_read_property(&current_bind_types, getThis(), SL("_bindTypes"), PH_NOISY|PH_READONLY);
		if (Z_TYPE(current_bind_types) == IS_ARRAY) {
			phalcon_fast_array_merge(&merged_types, &current_bind_types, bind_types);
		} else {
			ZVAL_COPY(&merged_types, bind_types);
		}

		phalcon_update_property(getThis(), SL("_bindTypes"), &merged_types);
		zval_ptr_dtor(&merged_types);
	} else {
		phalcon_update_property(getThis(), SL("_bindTypes"), bind_types);
	}

	RETURN_THIS();
}

/**
 * Sets the columns to be queried
 *
 *<code>
 *	$criteria->columns(array('id', 'name'));
 *</code>
 *
 * @param string|array $columns
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, columns) {

	zval *columns, type = {};

	phalcon_fetch_params(0, 1, 0, &columns);

	ZVAL_LONG(&type, PHQL_T_SELECT);

	phalcon_update_property(getThis(), SL("_type"), &type);
	phalcon_update_property(getThis(), SL("_columns"), columns);
	RETURN_THIS();
}

/**
 * Adds a join to the query
 *
 *<code>
 *	$criteria->join('Robots');
 *	$criteria->join('Robots', 'r.id = RobotsParts.robots_id');
 *	$criteria->join('Robots', 'r.id = RobotsParts.robots_id', 'r');
 *	$criteria->join('Robots', 'r.id = RobotsParts.robots_id', 'r', 'LEFT');
 *</code>
 *
 * @param string $model
 * @param string $conditions
 * @param string $alias
 * @param string $type
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, join) {

	zval *model, *conditions = NULL, *alias = NULL, *type = NULL, new_join = {}, joins = {};

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

	array_init_size(&new_join, 4);

	phalcon_array_append(&new_join, model, PH_COPY);
	phalcon_array_append(&new_join, conditions, PH_COPY);
	phalcon_array_append(&new_join, alias, PH_COPY);
	phalcon_array_append(&new_join, type, PH_COPY);

	phalcon_read_property(&joins, getThis(), SL("_joins"), PH_COPY);

	if (Z_TYPE(joins) != IS_ARRAY) {
		array_init_size(&joins, 1);
	}

	phalcon_array_append(&joins, &new_join, 0);

	phalcon_update_property(getThis(), SL("_joins"), &joins);
	zval_ptr_dtor(&joins);

	RETURN_THIS();
}

/**
 * Adds a INNER join to the query
 *
 *<code>
 *	$criteria->innerJoin('Robots');
 *	$criteria->innerJoin('Robots', 'r.id = RobotsParts.robots_id');
 *	$criteria->innerJoin('Robots', 'r.id = RobotsParts.robots_id', 'r');
 *	$criteria->innerJoin('Robots', 'r.id = RobotsParts.robots_id', 'r', 'LEFT');
 *</code>
 *
 * @param string $model
 * @param string $conditions
 * @param string $alias
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, innerJoin) {

	zval *model, *conditions = NULL, *alias = NULL, type = {};

	phalcon_fetch_params(0, 1, 2, &model, &conditions, &alias);

	if (!conditions) {
		conditions = &PHALCON_GLOBAL(z_null);
	}

	if (!alias) {
		alias = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STRING(&type, "INNER");

	PHALCON_CALL_SELF(NULL, "join", model, conditions, alias, &type);
	zval_ptr_dtor(&type);

	RETURN_THIS();
}

/**
 * Adds a LEFT join to the query
 *
 *<code>
 *	$criteria->leftJoin('Robots', 'r.id = RobotsParts.robots_id', 'r');
 *</code>
 *
 * @param string $model
 * @param string $conditions
 * @param string $alias
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, leftJoin) {

	zval *model, *conditions = NULL, *alias = NULL, type = {};

	phalcon_fetch_params(0, 1, 2, &model, &conditions, &alias);

	if (!conditions) {
		conditions = &PHALCON_GLOBAL(z_null);
	}

	if (!alias) {
		alias = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STRING(&type, "LEFT");

	PHALCON_CALL_SELF(NULL, "join", model, conditions, alias, &type);
	zval_ptr_dtor(&type);

	RETURN_THIS();
}

/**
 * Adds a RIGHT join to the query
 *
 *<code>
 *	$criteria->rightJoin('Robots', 'r.id = RobotsParts.robots_id', 'r');
 *</code>
 *
 * @param string $model
 * @param string $conditions
 * @param string $alias
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, rightJoin) {

	zval *model, *conditions = NULL, *alias = NULL, type = {};

	phalcon_fetch_params(0, 1, 2, &model, &conditions, &alias);

	if (!conditions) {
		conditions = &PHALCON_GLOBAL(z_null);
	}

	if (!alias) {
		alias = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STRING(&type, "RIGHT");

	PHALCON_CALL_SELF(NULL, "join", model, conditions, alias, &type);
	zval_ptr_dtor(&type);

	RETURN_THIS();
}

/**
 * Sets the conditions parameter in the criteria
 *
 * @param string $conditions
 * @param array $bindParams
 * @param array $bindTypes
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, where) {

	zval *conditions, *bind_params = NULL, *bind_types = NULL, current_bind_params = {}, merged_params = {}, current_bind_types = {}, merged_params_types = {};

	phalcon_fetch_params(0, 1, 2, &conditions, &bind_params, &bind_types);

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(conditions) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Conditions must be string");
		return;
	}
	phalcon_update_property(getThis(), SL("_conditions"), conditions);

	/**
	 * Update or merge existing bound parameters
	 */
	if (Z_TYPE_P(bind_params) == IS_ARRAY) {
		phalcon_read_property(&current_bind_params, getThis(), SL("_bindParams"), PH_NOISY|PH_READONLY);
		if (Z_TYPE(current_bind_params) == IS_ARRAY) {
			phalcon_fast_array_merge(&merged_params, &current_bind_params, bind_params);
		} else {
			ZVAL_COPY(&merged_params, bind_params);
		}

		phalcon_update_property(getThis(), SL("_bindParams"), &merged_params);
		zval_ptr_dtor(&merged_params);
	}

	/**
	 * Update or merge existing bind types parameters
	 */
	if (Z_TYPE_P(bind_types) == IS_ARRAY) {
		phalcon_read_property(&current_bind_types, getThis(), SL("_bindTypes"), PH_NOISY|PH_READONLY);
		if (Z_TYPE(current_bind_types) == IS_ARRAY) {
			phalcon_fast_array_merge(&merged_params_types, &current_bind_types, bind_types);
		} else {
			ZVAL_COPY(&merged_params_types, bind_types);
		}

		phalcon_update_property(getThis(), SL("_bindTypes"), &merged_params_types);
		zval_ptr_dtor(&merged_params_types);
	}

	RETURN_THIS();
}

/**
 * Appends a condition to the current conditions using an AND operator (deprecated)
 *
 * @deprecated 1.0.0
 * @see \Phalcon\Mvc\Model\Criteria::andWhere()
 * @param string $conditions
 * @param array $bindParams
 * @param array $bindTypes
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_Model_Criteria, addWhere);

/**
 * Appends a condition to the current conditions using an AND operator
 *
 * @param string $conditions
 * @param array $bindParams
 * @param array $bindTypes
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, andWhere) {

	zval *conditions, *bind_params = NULL, *bind_types = NULL, current_conditions = {}, new_conditions = {};
	zval current_bind_params = {}, merged_params = {}, current_bind_types = {}, merged_params_types = {};

	phalcon_fetch_params(0, 1, 2, &conditions, &bind_params, &bind_types);

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(conditions) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Conditions must be string");
		return;
	}

	phalcon_read_property(&current_conditions, getThis(), SL("_conditions"), PH_NOISY|PH_READONLY);
	if (!PHALCON_IS_EMPTY(&current_conditions)) {
		PHALCON_CONCAT_SVSVS(&new_conditions, "(", &current_conditions, ") AND (", conditions, ")");
	} else {
		ZVAL_COPY(&new_conditions, conditions);
	}

	phalcon_update_property(getThis(), SL("_conditions"), &new_conditions);
	zval_ptr_dtor(&new_conditions);

	/**
	 * Update or merge existing bound parameters
	 */
	if (Z_TYPE_P(bind_params) == IS_ARRAY) {
		phalcon_read_property(&current_bind_params, getThis(), SL("_bindParams"), PH_NOISY|PH_READONLY);
		if (Z_TYPE(current_bind_params) == IS_ARRAY) {
			phalcon_fast_array_merge(&merged_params, &current_bind_params, bind_params);
		} else {
			ZVAL_COPY(&merged_params, bind_params);
		}

		phalcon_update_property(getThis(), SL("_bindParams"), &merged_params);
		zval_ptr_dtor(&merged_params);
	}

	/**
	 * Update or merge existing bind types parameters
	 */
	if (Z_TYPE_P(bind_types) == IS_ARRAY) {
		phalcon_read_property(&current_bind_types, getThis(), SL("_bindTypes"), PH_NOISY|PH_READONLY);
		if (Z_TYPE(current_bind_types) == IS_ARRAY) {
			phalcon_fast_array_merge(&merged_params_types, &current_bind_types, bind_types);
		} else {
			ZVAL_COPY(&merged_params_types, bind_types);
		}

		phalcon_update_property(getThis(), SL("_bindTypes"), &merged_params_types);
		zval_ptr_dtor(&merged_params_types);
	}

	RETURN_THIS();
}

/**
 * Appends a condition to the current conditions using an OR operator
 *
 * @param string $conditions
 * @param array $bindParams
 * @param array $bindTypes
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, orWhere) {

	zval *conditions, *bind_params = NULL, *bind_types = NULL, current_conditions = {}, new_conditions = {};
	zval current_bind_params = {}, merged_params = {}, current_bind_types = {}, merged_params_types = {};

	phalcon_fetch_params(0, 1, 2, &conditions, &bind_params, &bind_types);

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(conditions) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Conditions must be string");
		return;
	}

	phalcon_read_property(&current_conditions, getThis(), SL("_conditions"), PH_NOISY|PH_READONLY);
	if (!PHALCON_IS_EMPTY(&current_conditions)) {
		PHALCON_CONCAT_SVSVS(&new_conditions, "(", &current_conditions, ") OR (", conditions, ")");
	} else {
		ZVAL_COPY(&new_conditions, conditions);
	}

	phalcon_update_property(getThis(), SL("_conditions"), &new_conditions);
	zval_ptr_dtor(&new_conditions);

	/**
	 * Update or merge existing bound parameters
	 */
	if (Z_TYPE_P(bind_params) == IS_ARRAY) {
		phalcon_read_property(&current_bind_params, getThis(), SL("_bindParams"), PH_NOISY|PH_READONLY);
		if (Z_TYPE(current_bind_params) == IS_ARRAY) {
			phalcon_fast_array_merge(&merged_params, &current_bind_params, bind_params);
		} else {
			ZVAL_COPY(&merged_params, bind_params);
		}

		phalcon_update_property(getThis(), SL("_bindParams"), &merged_params);
		zval_ptr_dtor(&merged_params);
	}

	/**
	 * Update or merge existing bind types parameters
	 */
	if (Z_TYPE_P(bind_types) == IS_ARRAY) {
		phalcon_read_property(&current_bind_types, getThis(), SL("_bindTypes"), PH_NOISY|PH_READONLY);
		if (Z_TYPE(current_bind_types) == IS_ARRAY) {
			phalcon_fast_array_merge(&merged_params_types, &current_bind_types, bind_types);
		} else {
			ZVAL_COPY(&merged_params_types, bind_types);
		}

		phalcon_update_property(getThis(), SL("_bindTypes"), &merged_params_types);
		zval_ptr_dtor(&merged_params_types);
	}

	RETURN_THIS();
}

/**
 * Appends a BETWEEN condition to the current conditions
 *
 *<code>
 *	$criteria->betweenWhere('price', 100.25, 200.50);
 *</code>
 *
 * @param string $expr
 * @param mixed $minimum
 * @param mixed $maximum
 * @param boolean $useOrWhere
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, betweenWhere) {

	zval *expr, *minimum, *maximum, *use_orwhere = NULL, hidden_param = {}, next_hidden_param = {}, minimum_key = {}, maximum_key = {};
	zval conditions = {}, bind_params = {};

	phalcon_fetch_params(0, 3, 1, &expr, &minimum, &maximum, &use_orwhere);

	if (!use_orwhere) {
		use_orwhere = &PHALCON_GLOBAL(z_false);
	}

	phalcon_read_property(&hidden_param, getThis(), SL("_hiddenParamNumber"), PH_NOISY|PH_READONLY);

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
	phalcon_array_update(&bind_params, &minimum_key, minimum, PH_COPY);
	phalcon_array_update(&bind_params, &maximum_key, maximum, PH_COPY);
	zval_ptr_dtor(&minimum_key);
	zval_ptr_dtor(&maximum_key);

	/**
	 * Append the BETWEEN to the current conditions using and 'and'
	 */
	if (zend_is_true(use_orwhere)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "orwhere", &conditions, &bind_params);
	} else {
		PHALCON_CALL_METHOD(NULL, getThis(), "andwhere", &conditions, &bind_params);
	}
	zval_ptr_dtor(&conditions);
	zval_ptr_dtor(&bind_params);
	phalcon_increment(&next_hidden_param);
	phalcon_update_property(getThis(), SL("_hiddenParamNumber"), &next_hidden_param);
	RETURN_THIS();
}

/**
 * Appends a NOT BETWEEN condition to the current conditions
 *
 *<code>
 *	$criteria->notBetweenWhere('price', 100.25, 200.50);
 *</code>
 *
 * @param string $expr
 * @param mixed $minimum
 * @param mixed $maximum
 * @param boolean $useOrWhere
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, notBetweenWhere) {

	zval *expr, *minimum, *maximum, *use_orwhere = NULL, hidden_param = {}, next_hidden_param = {}, minimum_key = {}, maximum_key = {};
	zval conditions = {}, bind_params = {};

	phalcon_fetch_params(0, 3, 1, &expr, &minimum, &maximum, &use_orwhere);

	if (!use_orwhere) {
		use_orwhere = &PHALCON_GLOBAL(z_false);
	}

	phalcon_read_property(&hidden_param, getThis(), SL("_hiddenParamNumber"), PH_NOISY|PH_READONLY);

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
	phalcon_array_update(&bind_params, &minimum_key, minimum, PH_COPY);
	phalcon_array_update(&bind_params, &maximum_key, maximum, PH_COPY);
	zval_ptr_dtor(&minimum_key);
	zval_ptr_dtor(&maximum_key);

	/**
	 * Append the BETWEEN to the current conditions using and 'and'
	 */
	if (zend_is_true(use_orwhere)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "orwhere", &conditions, &bind_params);
	} else {
		PHALCON_CALL_METHOD(NULL, getThis(), "andwhere", &conditions, &bind_params);
	}
	zval_ptr_dtor(&conditions);
	zval_ptr_dtor(&bind_params);
	phalcon_increment(&next_hidden_param);
	phalcon_update_property(getThis(), SL("_hiddenParamNumber"), &next_hidden_param);
	RETURN_THIS();
}

/**
 * Appends an IN condition to the current conditions
 *
 *<code>
 *	$criteria->inWhere('id', [1, 2, 3]);
 *</code>
 *
 * @param string $expr
 * @param array $values
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, inWhere) {

	zval *expr, *values, *use_orwhere = NULL, hidden_param = {}, bind_params = {}, bind_keys = {}, *value, joined_keys = {}, conditions = {};


	phalcon_fetch_params(0, 2, 1, &expr, &values, &use_orwhere);

	if (!use_orwhere) {
		use_orwhere = &PHALCON_GLOBAL(z_false);
	}

	if (Z_TYPE_P(values) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Values must be an array");
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
		phalcon_array_append(&bind_keys, &query_key, 0);
		phalcon_array_update(&bind_params, &key, value, PH_COPY);
		zval_ptr_dtor(&key);
		phalcon_increment(&hidden_param);
	} ZEND_HASH_FOREACH_END();

	phalcon_fast_join_str(&joined_keys, SL(", "), &bind_keys);
	zval_ptr_dtor(&bind_keys);

	/**
	 * Create a standard IN condition with bind params
	 */
	PHALCON_CONCAT_VSVS(&conditions, expr, " IN (", &joined_keys, ")");
	zval_ptr_dtor(&joined_keys);

	/**
	 * Append the IN to the current conditions using and 'and'
	 */
	if (zend_is_true(use_orwhere)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "orwhere", &conditions, &bind_params);
	} else {
		PHALCON_CALL_METHOD(NULL, getThis(), "andwhere", &conditions, &bind_params);
	}
	zval_ptr_dtor(&bind_params);
	zval_ptr_dtor(&conditions);
	phalcon_update_property(getThis(), SL("_hiddenParamNumber"), &hidden_param);

	RETURN_THIS();
}

/**
 * Appends a NOT IN condition to the current conditions
 *
 *<code>
 *	$criteria->notInWhere('id', [1, 2, 3]);
 *</code>
 *
 * @param string $expr
 * @param array $values
 * @param boolean $useOrWhere
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, notInWhere) {

	zval *expr, *values, *use_orwhere = NULL, hidden_param = {}, bind_params = {}, bind_keys = {}, *value, joined_keys = {}, conditions = {};

	phalcon_fetch_params(0, 2, 1, &expr, &values, &use_orwhere);

	if (!use_orwhere) {
		use_orwhere = &PHALCON_GLOBAL(z_false);
	}

	if (Z_TYPE_P(values) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Values must be an array");
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
		phalcon_array_append(&bind_keys, &query_key, 0);
		phalcon_array_update(&bind_params, &key, value, PH_COPY);
		zval_ptr_dtor(&key);
		phalcon_increment(&hidden_param);
	} ZEND_HASH_FOREACH_END();

	phalcon_fast_join_str(&joined_keys, SL(", "), &bind_keys);
	zval_ptr_dtor(&bind_keys);

	/**
	 * Create a standard IN condition with bind params
	 */
	PHALCON_CONCAT_VSVS(&conditions, expr, " NOT IN (", &joined_keys, ")");
	zval_ptr_dtor(&joined_keys);

	/**
	 * Append the IN to the current conditions using and 'and'
	 */
	if (zend_is_true(use_orwhere)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "orwhere", &conditions, &bind_params);
	} else {
		PHALCON_CALL_METHOD(NULL, getThis(), "andwhere", &conditions, &bind_params);
	}
	zval_ptr_dtor(&bind_params);
	zval_ptr_dtor(&conditions);
	phalcon_update_property(getThis(), SL("_hiddenParamNumber"), &hidden_param);

	RETURN_THIS();
}

/**
 * Adds the conditions parameter to the criteria
 *
 * @param string $conditions
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, conditions) {

	zval *conditions;

	phalcon_fetch_params(0, 1, 0, &conditions);

	if (Z_TYPE_P(conditions) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Conditions must be string");
		return;
	}
	phalcon_update_property(getThis(), SL("_conditions"), conditions);

	RETURN_THIS();
}

/**
 * Adds the order-by parameter to the criteria (deprecated)
 *
 * @deprecated 1.2.1
 * @see \Phalcon\Mvc\Model\Criteria::orderBy()
 * @param string $orderColumns
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_Model_Criteria, order)

/**
 * Adds the order-by parameter to the criteria
 *
 * @param string $orderColumns
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, orderBy) {

	zval *order_columns;

	phalcon_fetch_params(0, 1, 0, &order_columns);

	if (Z_TYPE_P(order_columns) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Order columns must be string");
		return;
	}
	phalcon_update_property(getThis(), SL("_order"), order_columns);

	RETURN_THIS();
}

/**
 * Adds the limit parameter to the criteria
 *
 * @param int $limit
 * @param int $offset
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, limit) {

	zval *limit, *offset = NULL;

	phalcon_fetch_params(0, 1, 1, &limit, &offset);

	if (!offset) {
		offset = &PHALCON_GLOBAL(z_null);
	}

	if (!phalcon_is_numeric(limit)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Row limit parameter must be integer");
		return;
	}

	phalcon_update_property(getThis(), SL("_limit"), limit);
	phalcon_update_property(getThis(), SL("_offset"), offset);

	RETURN_THIS();
}

/**
 * Tells to the query if only the first row in the resultset must be returned
 *
 * @param boolean $uniqueRow
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, setUniqueRow) {

	zval *unique_row;

	phalcon_fetch_params(0, 1, 0, &unique_row);

	phalcon_update_property(getThis(), SL("_uniqueRow"), unique_row);
	RETURN_THIS();
}

/**
 * Check if the query is programmed to get only the first row in the resultset
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, getUniqueRow){


	RETURN_MEMBER(getThis(), "_uniqueRow");
}

/**
 * Adds the "for_update" parameter to the criteria
 *
 * @param boolean $forUpdate
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, forUpdate) {

	zval *for_update = NULL;

	phalcon_fetch_params(0, 0, 1, &for_update);

	if (!for_update) {
		for_update = &PHALCON_GLOBAL(z_true);
	}

	phalcon_update_property(getThis(), SL("_forUpdate"), for_update);
	RETURN_THIS();
}

/**
 * Adds the "index" parameter to the criteria
 *
 * @param string $index
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, setIndex) {

	zval *index;

	phalcon_fetch_params(0, 1, 0, &index);

	phalcon_update_property(getThis(), SL("_index"), index);
	RETURN_THIS();
}

/**
 * Gets the index
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, getIndex){


	RETURN_MEMBER(getThis(), "_index");
}

/**
 * Adds the "shared_lock" parameter to the criteria
 *
 * @param boolean $sharedLock
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, sharedLock) {

	zval *shared_lock = NULL;

	phalcon_fetch_params(0, 0, 1, &shared_lock);

	if (!shared_lock) {
		shared_lock = &PHALCON_GLOBAL(z_true);
	}

	phalcon_update_property(getThis(), SL("_sharedLock"), shared_lock);
	RETURN_THIS();
}

/**
 * Returns the conditions parameter in the criteria
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, getWhere) {


	RETURN_MEMBER(getThis(), "_conditions");
}

/**
 * Returns the columns to be queried
 *
 * @return string|array
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, getColumns) {


	RETURN_MEMBER(getThis(), "_columns");
}

/**
 * Returns the conditions parameter in the criteria
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, getConditions) {


	RETURN_MEMBER(getThis(), "_conditions");
}

/**
 * Returns the limit parameter in the criteria
 *
 * @return int
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, getLimit) {


	RETURN_MEMBER(getThis(), "_limit");
}

/**
 * Returns the order parameter in the criteria
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, getOrder) {

	RETURN_MEMBER(getThis(), "_order");
}

/**
 * Returns all the parameters defined in the criteria
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, getParams) {

	zval params = {}, conditions = {}, bind_params = {}, bind_types = {}, order = {}, limit = {}, offset = {}, cache = {};

	array_init(&params);

	phalcon_read_property(&conditions, getThis(), SL("_conditions"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(conditions) != IS_NULL) {
		phalcon_array_update_str(&params, SL("conditions"), &conditions, PH_COPY);
	}

	phalcon_read_property(&bind_params, getThis(), SL("_bindParams"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(bind_params) != IS_NULL) {
		phalcon_array_update_str(&params, SL("bind"), &bind_params, PH_COPY);
	}

	phalcon_read_property(&bind_types, getThis(), SL("_bindTypes"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(bind_types) != IS_NULL) {
		phalcon_array_update_str(&params, SL("bindTypes"), &bind_types, PH_COPY);
	}

	phalcon_read_property(&order, getThis(), SL("_order"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(order) != IS_NULL) {
		phalcon_array_update_str(&params, SL("order"), &order, PH_COPY);
	}

	phalcon_read_property(&limit, getThis(), SL("_limit"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(limit) != IS_NULL) {
		phalcon_array_update_str(&params, SL("limit"), &limit, PH_COPY);
	}

	phalcon_read_property(&offset, getThis(), SL("_offset"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(offset) != IS_NULL) {
		phalcon_array_update_str(&params, SL("offset"), &offset, PH_COPY);
	}

	phalcon_read_property(&cache, getThis(), SL("_cacheOptions"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(cache) != IS_NULL) {
		phalcon_array_update_str(&params, SL("cache"), &cache, PH_COPY);
	}

	RETVAL_ZVAL(&params, 0, 0);
}

/**
 * Builds a Phalcon\Mvc\Model\Criteria based on an input array like $_POST
 *
 * @param Phalcon\DiInterface $dependencyInjector
 * @param string $modelName
 * @param array $data
 * @return Phalcon\Mvc\Model\Criteria
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, fromInput) {

	zval *dependency_injector, *model_name, *data, service = {}, meta_data = {}, model = {}, column_map = {}, data_types = {}, bind = {};
	zval conditions = {}, *value, join_conditions = {};
	zend_string *str_key;
	zend_class_entry *ce0;
	ulong idx;

	phalcon_fetch_params(0, 3, 0, &dependency_injector, &model_name, &data);

	if (Z_TYPE_P(data) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Input data must be an Array");
		return;
	}

	if (Z_TYPE_P(dependency_injector) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "A dependency injector container is required to obtain the ORM services");
		return;
	}

	object_init_ex(return_value, phalcon_mvc_model_criteria_ce);

	if (zend_hash_num_elements(Z_ARRVAL_P(data))) {
		ZVAL_STR(&service, IS(modelsMetadata));

		PHALCON_CALL_METHOD(&meta_data, dependency_injector, "getshared", &service);
		PHALCON_VERIFY_INTERFACE(&meta_data, phalcon_mvc_model_metadatainterface_ce);
		ce0 = phalcon_fetch_class(model_name, ZEND_FETCH_CLASS_DEFAULT);

		PHALCON_OBJECT_INIT(&model, ce0);
		if (phalcon_has_constructor(&model)) {
			PHALCON_CALL_METHOD(NULL, &model, "__construct");
		}

		PHALCON_VERIFY_INTERFACE_EX(&model, phalcon_mvc_modelinterface_ce, phalcon_mvc_model_exception_ce);

		PHALCON_CALL_METHOD(&column_map, &meta_data, "getreversecolumnmap", &model);
		PHALCON_CALL_METHOD(&data_types, &meta_data, "getdatatypes", &model);
		zval_ptr_dtor(&meta_data);
		zval_ptr_dtor(&model);

		array_init(&bind);
		array_init(&conditions);

		/**
		 * We look for attributes in the array passed as data
		 */
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(data), idx, str_key, value) {
			zval field = {}, real_field = {}, type = {}, condition = {}, value_pattern = {};
			if (str_key) {
				ZVAL_STR(&field, str_key);
			} else {
				ZVAL_LONG(&field, idx);
			}

			if (Z_TYPE(column_map) != IS_ARRAY || !phalcon_array_isset_fetch(&real_field, &column_map, &field, PH_READONLY)) {
				ZVAL_COPY_VALUE(&real_field, &field);
			}

			if (phalcon_array_isset_fetch(&type, &data_types, &real_field, PH_READONLY)) {
				if (Z_TYPE_P(value) != IS_NULL && !PHALCON_IS_STRING(value, "")) {
					if (PHALCON_IS_LONG(&type, PHALCON_DB_COLUMN_TYPE_VARCHAR)) {
						/**
						 * For varchar types we use LIKE operator
						 */
						PHALCON_CONCAT_VSVS(&condition, &field, " LIKE :", &field, ":");

						PHALCON_CONCAT_SVS(&value_pattern, "%", value, "%");
						phalcon_array_update(&bind, &field, &value_pattern, 0);
					} else {
						/**
						 * For the rest of data types we use a plain = operator
						 */
						PHALCON_CONCAT_VSVS(&condition, &field, "=:", &field, ":");
						phalcon_array_update(&bind, &field, value, PH_COPY);
					}

					phalcon_array_append(&conditions, &condition, 0);
				}
			}
		} ZEND_HASH_FOREACH_END();
		zval_ptr_dtor(&column_map);
		zval_ptr_dtor(&data_types);

		if (zend_hash_num_elements(Z_ARRVAL(conditions))) {
			phalcon_fast_join_str(&join_conditions, SL(" AND "), &conditions);
			PHALCON_CALL_METHOD(NULL, return_value, "where", &join_conditions, &bind);
			zval_ptr_dtor(&join_conditions);
		}
		zval_ptr_dtor(&bind);
		zval_ptr_dtor(&conditions);
	}

	PHALCON_CALL_METHOD(NULL, return_value, "setmodelname", model_name);
}

/**
 * Sets a GROUP BY clause
 *
 * @param string $group
 * @return Phalcon\Mvc\Model\Criteria
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, groupBy) {

	zval *group;

	phalcon_fetch_params(0, 1, 0, &group);

	phalcon_update_property(getThis(), SL("_group"), group);
	RETURN_THIS();
}

/**
 * Sets a HAVING condition clause. You need to escape PHQL reserved words using [ and ] delimiters
 *
 * @param string $having
 * @return Phalcon\Mvc\Model\Criteria
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, having) {

	zval *having;

	phalcon_fetch_params(0, 1, 0, &having);

	phalcon_update_property(getThis(), SL("_having"), having);
	RETURN_THIS();
}

/**
 * Executes a find using the parameters built with the criteria
 *
 * @return Phalcon\Mvc\Model\ResultsetInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, execute) {

	zval phql = {}, dependency_injector = {}, cache_options = {}, unique_row = {}, index = {};
	zval query = {}, bind_params = {}, bind_types = {};

	PHALCON_MM_INIT();
	PHALCON_MM_CALL_SELF(&phql, "getphql");
	PHALCON_MM_ADD_ENTRY(&phql);

	PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi");
	PHALCON_MM_ADD_ENTRY(&dependency_injector);
	phalcon_read_property(&cache_options, getThis(), SL("_cacheOptions"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&unique_row, getThis(), SL("_uniqueRow"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&index, getThis(), SL("_index"), PH_NOISY|PH_READONLY);

	object_init_ex(&query, phalcon_mvc_model_query_ce);
	PHALCON_MM_ADD_ENTRY(&query);
	PHALCON_MM_CALL_METHOD(NULL, &query, "__construct", &phql, &dependency_injector);

	if (Z_TYPE(cache_options) == IS_ARRAY) {
		PHALCON_MM_CALL_METHOD(NULL, &query, "cache", &cache_options);
	}

	if (Z_TYPE(unique_row) != IS_NULL) {
		PHALCON_MM_CALL_METHOD(NULL, &query, "setuniquerow", &unique_row);
	}

	if (Z_TYPE(index) != IS_NULL) {
		PHALCON_MM_CALL_METHOD(NULL, &query, "setindex", &index);
	}

	phalcon_read_property(&bind_params, getThis(), SL("_bindParams"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&bind_types, getThis(), SL("_bindTypes"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(bind_params) == IS_ARRAY) {
		PHALCON_MM_CALL_METHOD(NULL, &query, "setbindparams", &bind_params);
	}

	if (Z_TYPE(bind_types) == IS_ARRAY) {
		PHALCON_MM_CALL_METHOD(NULL, &query, "setbindtypes", &bind_types);
	}

	PHALCON_MM_CALL_METHOD(return_value, &query, "execute");
	RETURN_MM();
}


/**
 * Auto sets columns and return execute
 *
 * @return int
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, count) {

	zval *column = NULL, count_sql = {}, result = {};

	phalcon_fetch_params(0, 0, 1, &column);

	if (column && Z_TYPE_P(column) == IS_STRING) {
		PHALCON_CONCAT_SVS(&count_sql, "COUNT(", column, ") AS num");
	} else {
		ZVAL_STRING(&count_sql, "COUNT(*) AS num");
	}
	PHALCON_CALL_METHOD(NULL, getThis(), "columns", &count_sql);
	zval_ptr_dtor(&count_sql);

	PHALCON_CALL_METHOD(NULL, getThis(), "setuniquerow", &PHALCON_GLOBAL(z_true));

	PHALCON_CALL_METHOD(&result, getThis(), "execute");

	if (Z_TYPE(result) != IS_OBJECT) {
		RETVAL_ZVAL(&result, 0, 0);
		return;
	}

	phalcon_read_property(return_value, &result, SL("num"), PH_NOISY|PH_COPY);
	convert_to_long(return_value);
	zval_ptr_dtor(&result);
}

/**
 * Sets the cache options in the criteria
 * This method replaces all previously set cache options
 *
 * @param array $options
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, cache) {

	zval *options;

	phalcon_fetch_params(0, 1, 0, &options);

	if (Z_TYPE_P(options) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Cache options must be an Array");
		return;
	}

	phalcon_update_property(getThis(), SL("_cacheOptions"), options);

	RETURN_THIS();
}

/**
 * Sets insert type of PHQL statement to be executed
 *
 * @param array $columns
 * @param array $values
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, insert) {

	zval *columns, *values, type = {};

	phalcon_fetch_params(0, 2, 0, &columns, &values);

	ZVAL_LONG(&type, PHQL_T_INSERT);

	phalcon_update_property(getThis(), SL("_type"), &type);

	if (Z_TYPE_P(columns) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Columns must be an array");
		return;
	}

	if (Z_TYPE_P(values) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Values must be an array");
		return;
	}

	phalcon_update_property(getThis(), SL("_columns"), columns);
	phalcon_update_property(getThis(), SL("_values"), values);

	RETURN_THIS();
}

/**
 * Sets update type of PHQL statement to be executed
 *
 * @param array $columns
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, update) {

	zval *columns, type = {};

	phalcon_fetch_params(0, 1, 0, &columns);

	ZVAL_LONG(&type, PHQL_T_UPDATE);

	phalcon_update_property(getThis(), SL("_type"), &type);

	if (columns && Z_TYPE_P(columns) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Columns must be an array");
		return;
	}

	phalcon_update_property(getThis(), SL("_columns"), columns);

	RETURN_THIS();
}

/**
 * Sets update type of PHQL statement to be executed
 *
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, delete) {

	zval type = {};

	ZVAL_LONG(&type, PHQL_T_DELETE);

	phalcon_update_property(getThis(), SL("_type"), &type);

	RETURN_THIS();
}

/**
 * Returns a PHQL statement built with the criteria
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, getPhql) {

	zval type = {};

	phalcon_read_property(&type, getThis(), SL("_type"), PH_NOISY|PH_READONLY);

	switch (phalcon_get_intval(&type)) {
		case PHQL_T_INSERT:
			PHALCON_RETURN_CALL_METHOD(getThis(), "_generateinsert");
			break;

		case PHQL_T_UPDATE:
			PHALCON_RETURN_CALL_METHOD(getThis(), "_generateupdate");
			break;

		case PHQL_T_DELETE:
			PHALCON_RETURN_CALL_METHOD(getThis(), "_generatedelete");
			break;

		default:
			PHALCON_RETURN_CALL_METHOD(getThis(), "_generateselect");
			break;

	}
}

/**
 * Returns a PHQL statement built with the criteria
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, _generateSelect) {

	zval dependency_injector = {}, model = {}, conditions = {}, service_name = {}, meta_data = {}, model_instance = {}, no_primary = {}, primary_keys = {}, first_primary_key = {};
	zval column_map = {}, attribute_field = {}, exception_message = {}, primary_key_condition = {}, phql = {}, columns = {}, selected_columns = {}, *column, joined_columns = {};
	zval joins = {}, *join, group = {}, having = {}, order = {}, limit = {}, offset = {}, for_update = {};
	zend_string *str_key;
	ulong idx;
	zend_class_entry *ce0;

	PHALCON_MM_INIT();

	PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi", &PHALCON_GLOBAL(z_true));
	PHALCON_MM_ADD_ENTRY(&dependency_injector);
	phalcon_read_property(&model, getThis(), SL("_model"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&model)) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "At least one model is required to build the query");
		return;
	}

	phalcon_read_property(&conditions, getThis(), SL("_conditions"), PH_READONLY);
	if (phalcon_is_numeric(&conditions)) {

		/**
		 * If the conditions is a single numeric field. We internally create a condition
		 * using the related primary key
		 */
		ZVAL_STR(&service_name, IS(modelsMetadata));

		/**
		 * Get the models metadata service to obtain the column names, column map and
		 * primary key
		 */
		PHALCON_MM_CALL_METHOD(&meta_data, &dependency_injector, "getshared", &service_name);
		PHALCON_MM_ADD_ENTRY(&meta_data);
		PHALCON_MM_VERIFY_INTERFACE(&meta_data, phalcon_mvc_model_metadatainterface_ce);
		ce0 = phalcon_fetch_class(&model, ZEND_FETCH_CLASS_DEFAULT);

		object_init_ex(&model_instance, ce0);
		PHALCON_MM_ADD_ENTRY(&model_instance);
		if (phalcon_has_constructor(&model_instance)) {
			PHALCON_MM_CALL_METHOD(NULL, &model_instance, "__construct", &PHALCON_GLOBAL(z_null), &dependency_injector);
		}

		ZVAL_TRUE(&no_primary);

		PHALCON_MM_CALL_METHOD(&primary_keys, &meta_data, "getprimarykeyattributes", &model_instance);
		PHALCON_MM_ADD_ENTRY(&primary_keys);
		if (phalcon_fast_count_ev(&primary_keys)) {
			if (phalcon_array_isset_fetch_long(&first_primary_key, &primary_keys, 0, PH_READONLY)) {
				/**
				 * The PHQL contains the renamed columns if available
				 */
				PHALCON_MM_CALL_METHOD(&column_map, &meta_data, "getcolumnmap", &model_instance);
				PHALCON_MM_ADD_ENTRY(&column_map);
				if (Z_TYPE(column_map) == IS_ARRAY) {
					if (!phalcon_array_isset_fetch(&attribute_field, &column_map, &first_primary_key, PH_READONLY)) {
						PHALCON_CONCAT_SVS(&exception_message, "Column '", &first_primary_key, "\" isn't part of the column map");
						PHALCON_MM_ADD_ENTRY(&exception_message);
						PHALCON_MM_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
						return;
					}
				} else {
					ZVAL_COPY_VALUE(&attribute_field, &first_primary_key);
				}

				PHALCON_CONCAT_SVSVSV(&primary_key_condition, "[", &model, "].[", &attribute_field, "] = ", &conditions);
				PHALCON_MM_ADD_ENTRY(&primary_key_condition);
				ZVAL_COPY_VALUE(&conditions, &primary_key_condition);

				ZVAL_FALSE(&no_primary);
			}
		}

		/**
		 * A primary key is mandatory in these cases
		 */
		if (PHALCON_IS_TRUE(&no_primary)) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Source related to this model does not have a primary key defined");
			return;
		}
	}

	PHALCON_MM_ZVAL_STRING(&phql, "SELECT ");

	phalcon_read_property(&columns, getThis(), SL("_columns"), PH_NOISY|PH_READONLY);
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

				if (Z_TYPE_P(column) == IS_LONG) {
					phalcon_array_append(&selected_columns, column, PH_COPY);
				} else {
					PHALCON_CONCAT_VSV(&aliased_column, column, " AS ", &column_alias);
					phalcon_array_append(&selected_columns, &aliased_column, 0);
				}
			} ZEND_HASH_FOREACH_END();

			phalcon_fast_join_str(&joined_columns, SL(", "), &selected_columns);
			zval_ptr_dtor(&selected_columns);
			phalcon_concat_self(&phql, &joined_columns);
			zval_ptr_dtor(&joined_columns);
			PHALCON_MM_ADD_ENTRY(&phql);
		} else {
			phalcon_concat_self(&phql, &columns);
			PHALCON_MM_ADD_ENTRY(&phql);
		}
	} else {
		/**
		 * Automatically generate an array of model
		 */
		PHALCON_SCONCAT_SVS(&phql, "[", &model, "].*");
		PHALCON_MM_ADD_ENTRY(&phql);
	}

	PHALCON_SCONCAT_SVS(&phql, " FROM [", &model, "]");
	PHALCON_MM_ADD_ENTRY(&phql);

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
			 * Create the join according to the type
			 */
			if (phalcon_array_isset_fetch_long(&join_type, join, 3, PH_READONLY) && zend_is_true(&join_type)) {
				PHALCON_SCONCAT_SVSVS(&phql, " ", &join_type, " JOIN [", &join_model, "]");
			} else {
				PHALCON_SCONCAT_SVS(&phql, " JOIN [", &join_model, "]");
			}
			PHALCON_MM_ADD_ENTRY(&phql);

			/**
			 * Alias comes first
			 */
			if (phalcon_array_isset_fetch_long(&join_alias, join, 2, PH_READONLY) && zend_is_true(&join_alias)) {
				PHALCON_SCONCAT_SVS(&phql, " AS [", &join_alias, "]");
				PHALCON_MM_ADD_ENTRY(&phql);
			}

			/**
			 * Conditions then
			 */
			if (phalcon_array_isset_fetch_long(&join_conditions, join, 1, PH_READONLY) && zend_is_true(&join_conditions)) {
				PHALCON_SCONCAT_SV(&phql, " ON ", &join_conditions);
				PHALCON_MM_ADD_ENTRY(&phql);
			}
		} ZEND_HASH_FOREACH_END();

	}

	/**
	 * Only append conditions if it's string
	 */
	if (Z_TYPE(conditions) == IS_STRING) {
		if (PHALCON_IS_NOT_EMPTY(&conditions)) {
			PHALCON_SCONCAT_SV(&phql, " WHERE ", &conditions);
			PHALCON_MM_ADD_ENTRY(&phql);
		}
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
	if (PHALCON_IS_NOT_EMPTY(&having)) {
		PHALCON_SCONCAT_SV(&phql, " HAVING ", &having);
		PHALCON_MM_ADD_ENTRY(&phql);
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
	phalcon_read_property(&limit, getThis(), SL("_limit"), PH_NOISY|PH_COPY);
	if (PHALCON_IS_NOT_EMPTY(&limit) && Z_TYPE(limit) != IS_ARRAY) {
		phalcon_read_property(&offset, getThis(), SL("_offset"), PH_READONLY);
		if (PHALCON_IS_NOT_EMPTY(&offset)) {
			PHALCON_SCONCAT_SV(&limit, " OFFSET ", &offset);
			PHALCON_MM_ADD_ENTRY(&limit);
		}
	}
	if (PHALCON_IS_NOT_EMPTY(&limit)) {
		phalcon_orm_phql_build_limit(&phql, &limit);
		PHALCON_MM_ADD_ENTRY(&phql);
	}

	/**
	 * Process FOR UPDATE clause
	 */
	phalcon_read_property(&for_update, getThis(), SL("_forUpdate"), PH_NOISY|PH_READONLY);
	if (zend_is_true(&for_update)) {
		phalcon_concat_self_str(&phql, SL(" FOR UPDATE"));
		PHALCON_MM_ADD_ENTRY(&phql);
	}

	RETURN_MM_CTOR(&phql);
}

/**
 * Returns a PHQL statement built with the criteria
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, _generateInsert) {

	zval phql = {}, model = {}, selected_columns = {}, columns = {}, *column, joined_columns = {}, rows = {}, *row, insert_sqls = {}, bind_params = {}, joined_insert_sqls = {};
	zend_string *str_key;
	ulong idx;

	ZVAL_STRING(&phql, "INSERT INTO ");

	phalcon_read_property(&model, getThis(), SL("_model"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&model)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "At least one model is required to build the query");
		return;
	}

	PHALCON_SCONCAT_SVS(&phql, "[", &model, "] ");

	array_init(&selected_columns);

	phalcon_read_property(&columns, getThis(), SL("_columns"), PH_NOISY|PH_READONLY);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(columns), column) {
		phalcon_array_append(&selected_columns, column, PH_COPY);
	} ZEND_HASH_FOREACH_END();

	phalcon_fast_join_str(&joined_columns, SL(", "), &selected_columns);
	zval_ptr_dtor(&selected_columns);

	PHALCON_SCONCAT_SVS(&phql, "(", &joined_columns, ")");
	zval_ptr_dtor(&joined_columns);

	phalcon_read_property(&rows, getThis(), SL("_values"), PH_NOISY|PH_READONLY);

	array_init(&insert_sqls);
	array_init(&bind_params);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(rows), idx, str_key, row) {
		zval tmp = {}, keys = {}, joined_keys = {}, insert_sql = {};
		zend_string *str_key2;
		ulong idx2;
		if (str_key) {
			ZVAL_STR(&tmp, str_key);
		} else {
			ZVAL_LONG(&tmp, idx);
		}

		array_init(&keys);

		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(columns), idx2, str_key2, column) {
			zval tmp2 = {}, value = {}, exception_message = {}, key = {};
			if (str_key2) {
				ZVAL_STR(&tmp2, str_key2);
			} else {
				ZVAL_LONG(&tmp2, idx2);
			}

			if (phalcon_array_isset(row, column)) {
				phalcon_array_fetch(&value, row, column, PH_NOISY|PH_READONLY);
			} else if (phalcon_array_isset(row, &tmp2)) {
				phalcon_array_fetch(&value, row, &tmp2, PH_NOISY|PH_READONLY);
			} else {
				PHALCON_CONCAT_SVS(&exception_message, "Values can't find column '", column, "' value");
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
				return;
			}

			PHALCON_CONCAT_VV(&key, column, &tmp);
			phalcon_array_update(&bind_params, &key, &value, PH_COPY);
			zval_ptr_dtor(&key);

			PHALCON_CONCAT_SVVS(&key, ":", column, &tmp, ":");
			phalcon_array_append(&keys, &key, 0);

		} ZEND_HASH_FOREACH_END();

		phalcon_fast_join_str(&joined_keys, SL(", "), &keys);
		zval_ptr_dtor(&keys);

		PHALCON_SCONCAT_SVS(&insert_sql, "(", &joined_keys, ")");
		zval_ptr_dtor(&joined_keys);

		phalcon_array_append(&insert_sqls, &insert_sql, 0);

	} ZEND_HASH_FOREACH_END();

	PHALCON_CALL_SELF(NULL, "bind", &bind_params);

	phalcon_fast_join_str(&joined_insert_sqls, SL(", "), &insert_sqls);
	zval_ptr_dtor(&insert_sqls);

	PHALCON_SCONCAT_SV(&phql, " VALUES ", &joined_insert_sqls);
	zval_ptr_dtor(&joined_insert_sqls);

	RETVAL_ZVAL(&phql, 0, 0);
}

/**
 * Returns a PHQL statement built with the criteria
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, _generateUpdate) {

	zval dependency_injector = {}, model = {}, service_name = {}, meta_data = {}, model_instance = {}, conditions = {}, no_primary = {};
	zval primary_keys = {}, first_primary_key = {}, column_map = {}, attribute_field = {}, primary_key_condition = {}, exception_message = {};
	zval phql = {}, bind_params = {}, columns = {}, updated_columns = {}, *value, joined_columns = {};
	zval order = {}, limit = {}, offset = {};
	zend_string *str_key;
	ulong idx;
	zend_class_entry *ce0;

	PHALCON_CALL_METHOD(&dependency_injector, getThis(), "getdi", &PHALCON_GLOBAL(z_true));

	phalcon_read_property(&model, getThis(), SL("_model"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&model)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "At least one model is required to build the query");
		return;
	}

	ZVAL_STR(&service_name, IS(modelsMetadata));

	PHALCON_CALL_METHOD(&meta_data, &dependency_injector, "getshared", &service_name);
	PHALCON_VERIFY_INTERFACE(&meta_data, phalcon_mvc_model_metadatainterface_ce);
	ce0 = phalcon_fetch_class(&model, ZEND_FETCH_CLASS_DEFAULT);

	object_init_ex(&model_instance, ce0);
	if (phalcon_has_constructor(&model_instance)) {
		PHALCON_CALL_METHOD(NULL, &model_instance, "__construct", &PHALCON_GLOBAL(z_null), &dependency_injector);
	}
	zval_ptr_dtor(&dependency_injector);


	phalcon_read_property(&conditions, getThis(), SL("_conditions"), PH_COPY);
	if (phalcon_is_numeric(&conditions)) {
		ZVAL_TRUE(&no_primary);

		PHALCON_CALL_METHOD(&primary_keys, &meta_data, "getprimarykeyattributes", &model_instance);
		if (phalcon_fast_count_ev(&primary_keys)) {
			if (phalcon_array_isset_fetch_long(&first_primary_key, &primary_keys, 0, PH_READONLY)) {
				/**
				 * The PHQL contains the renamed columns if available
				 */
				PHALCON_CALL_METHOD(&column_map, &meta_data, "getcolumnmap", &model_instance);

				if (Z_TYPE(column_map) == IS_ARRAY) {
					if (!phalcon_array_isset_fetch(&attribute_field, &column_map, &first_primary_key, PH_READONLY)) {
						PHALCON_CONCAT_SVS(&exception_message, "Column '", &first_primary_key, "\" isn't part of the column map");
						PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
						zval_ptr_dtor(&column_map);
						zval_ptr_dtor(&primary_keys);
						return;
					}
					zval_ptr_dtor(&column_map);
				} else {
					ZVAL_COPY_VALUE(&attribute_field, &first_primary_key);
				}

				PHALCON_CONCAT_SVSVSV(&primary_key_condition, "[", &model, "].[", &attribute_field, "] = ", &conditions);
				ZVAL_COPY_VALUE(&conditions, &primary_key_condition);

				ZVAL_FALSE(&no_primary);
			}
		}
		zval_ptr_dtor(&primary_keys);

		/**
		 * A primary key is mandatory in these cases
		 */
		if (PHALCON_IS_TRUE(&no_primary)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Source related to this model does not have a primary key defined");
			return;
		}
	}
	zval_ptr_dtor(&meta_data);
	zval_ptr_dtor(&model_instance);

	PHALCON_CONCAT_SVS(&phql, "UPDATE [", &model, "] SET ");

	array_init(&bind_params);

	phalcon_read_property(&columns, getThis(), SL("_columns"), PH_NOISY|PH_READONLY);

	array_init(&updated_columns);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(columns), idx, str_key, value) {
		zval column = {}, updated_column = {}, bind_name = {};
		if (str_key) {
			ZVAL_STR(&column, str_key);
		} else {
			ZVAL_LONG(&column, idx);
		}

		if (Z_TYPE_P(value) == IS_OBJECT) {
			PHALCON_CONCAT_VSV(&updated_column, &column, "=", value);
		} else if (Z_TYPE_P(value) == IS_NULL) {
			PHALCON_CONCAT_VS(&updated_column, &column, " = null");
		} else {
			PHALCON_CONCAT_SV(&bind_name, "phu", &column);

			PHALCON_CONCAT_VSVS(&updated_column, &column, " = :", &bind_name, ":");
			phalcon_array_update(&bind_params, &bind_name, value, PH_COPY);
		}

		phalcon_array_append(&updated_columns, &updated_column, 0);
	} ZEND_HASH_FOREACH_END();

	PHALCON_CALL_SELF(NULL, "bind", &bind_params, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&bind_params);

	phalcon_fast_join_str(&joined_columns, SL(", "), &updated_columns);
	zval_ptr_dtor(&updated_columns);
	phalcon_concat_self(&phql, &joined_columns);
	zval_ptr_dtor(&joined_columns);

	if (Z_TYPE(conditions) == IS_STRING) {
		if (PHALCON_IS_NOT_EMPTY(&conditions)) {
			PHALCON_SCONCAT_SV(&phql, " WHERE ", &conditions);
		}
		zval_ptr_dtor(&conditions);
	}

	/**
	 * Process order clause
	 */
	phalcon_read_property(&order, getThis(), SL("_order"), PH_NOISY|PH_READONLY);
	if (PHALCON_IS_NOT_EMPTY(&order)) {
		phalcon_orm_phql_build_order(&phql, &order);
	}

	/**
	 * Process limit parameters
	 */
	phalcon_read_property(&limit, getThis(), SL("_limit"), PH_NOISY|PH_COPY);
	if (PHALCON_IS_NOT_EMPTY(&limit) && Z_TYPE(limit) != IS_ARRAY) {
		phalcon_read_property(&offset, getThis(), SL("_offset"), PH_READONLY);
		if (PHALCON_IS_NOT_EMPTY(&offset)) {
			PHALCON_SCONCAT_SV(&limit, " OFFSET ", &offset);
		}
	}

	if (PHALCON_IS_NOT_EMPTY(&limit)) {
		phalcon_orm_phql_build_limit(&phql, &limit);
	}
	zval_ptr_dtor(&limit);

	RETVAL_ZVAL(&phql, 0, 0);
}

/**
 * Returns a PHQL statement built with the criteria
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Criteria, _generateDelete) {

	zval dependency_injector = {}, model = {}, conditions = {}, service_name = {}, meta_data = {}, model_instance = {}, no_primary = {}, primary_keys = {}, first_primary_key = {};
	zval column_map = {}, attribute_field = {}, exception_message = {}, primary_key_condition = {}, phql = {}, order = {}, limit = {}, offset = {};
	zend_class_entry *ce0;

	PHALCON_CALL_METHOD(&dependency_injector, getThis(), "getdi", &PHALCON_GLOBAL(z_true));

	phalcon_read_property(&model, getThis(), SL("_model"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&model)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "At least one model is required to build the query");
		return;
	}

	phalcon_read_property(&conditions, getThis(), SL("_conditions"), PH_COPY);
	if (phalcon_is_numeric(&conditions)) {

		/**
		 * If the conditions is a single numeric field. We internally create a condition
		 * using the related primary key
		 */
		ZVAL_STR(&service_name, IS(modelsMetadata));

		/**
		 * Get the models metadata service to obtain the column names, column map and
		 * primary key
		 */
		PHALCON_CALL_METHOD(&meta_data, &dependency_injector, "getshared", &service_name);
		PHALCON_VERIFY_INTERFACE(&meta_data, phalcon_mvc_model_metadatainterface_ce);
		ce0 = phalcon_fetch_class(&model, ZEND_FETCH_CLASS_DEFAULT);

		object_init_ex(&model_instance, ce0);
		if (phalcon_has_constructor(&model_instance)) {
			PHALCON_CALL_METHOD(NULL, &model_instance, "__construct", &PHALCON_GLOBAL(z_null), &dependency_injector);
		}
		zval_ptr_dtor(&dependency_injector);

		ZVAL_TRUE(&no_primary);

		PHALCON_CALL_METHOD(&primary_keys, &meta_data, "getprimarykeyattributes", &model_instance);
		if (phalcon_fast_count_ev(&primary_keys)) {
			if (phalcon_array_isset_fetch_long(&first_primary_key, &primary_keys, 0, PH_READONLY)) {
				/**
				 * The PHQL contains the renamed columns if available
				 */
				PHALCON_CALL_METHOD(&column_map, &meta_data, "getcolumnmap", &model_instance);

				if (Z_TYPE(column_map) == IS_ARRAY) {
					if (!phalcon_array_isset_fetch(&attribute_field, &column_map, &first_primary_key, PH_READONLY)) {
						PHALCON_CONCAT_SVS(&exception_message, "Column '", &first_primary_key, "\" isn't part of the column map");
						PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
						return;
					}
				} else {
					ZVAL_COPY_VALUE(&attribute_field, &first_primary_key);
				}

				PHALCON_CONCAT_SVSVSV(&primary_key_condition, "[", &model, "].[", &attribute_field, "] = ", &conditions);
				ZVAL_COPY_VALUE(&conditions, &primary_key_condition);

				ZVAL_FALSE(&no_primary);
			}
		}
		zval_ptr_dtor(&primary_keys);
		zval_ptr_dtor(&model_instance);
		zval_ptr_dtor(&meta_data);

		/**
		 * A primary key is mandatory in these cases
		 */
		if (PHALCON_IS_TRUE(&no_primary)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Source related to this model does not have a primary key defined");
			return;
		}
	}

	PHALCON_CONCAT_SVS(&phql, " DELETE FROM [", &model, "]");

	/**
	 * Only append conditions if it's string
	 */
	if (Z_TYPE(conditions) == IS_STRING) {
		if (PHALCON_IS_NOT_EMPTY(&conditions)) {
			PHALCON_SCONCAT_SV(&phql, " WHERE ", &conditions);
		}
		zval_ptr_dtor(&conditions);
	}

	/**
	 * Process order clause
	 */
	phalcon_read_property(&order, getThis(), SL("_order"), PH_NOISY|PH_READONLY);
	if (PHALCON_IS_NOT_EMPTY(&order)) {
		phalcon_orm_phql_build_order(&phql, &order);
	}

	/**
	 * Process limit parameters
	 */
	phalcon_read_property(&limit, getThis(), SL("_limit"), PH_NOISY|PH_COPY);
	if (PHALCON_IS_NOT_EMPTY(&limit) && Z_TYPE(limit) != IS_ARRAY) {
		phalcon_read_property(&offset, getThis(), SL("_offset"), PH_READONLY);
		if (PHALCON_IS_NOT_EMPTY(&offset)) {
			PHALCON_SCONCAT_SV(&limit, " OFFSET ", &offset);
		}
	}
	if (PHALCON_IS_NOT_EMPTY(&limit)) {
		phalcon_orm_phql_build_limit(&phql, &limit);
	}
	zval_ptr_dtor(&limit);

	RETVAL_ZVAL(&phql, 0, 0);
}
