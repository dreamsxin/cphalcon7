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

#include "mvc/model/query/builder/join.h"
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
 * Phalcon\Mvc\Model\Query\Builder\Join
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
zend_class_entry *phalcon_mvc_model_query_builder_join_ce;

PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Join, join);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Join, innerJoin);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Join, leftJoin);
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Join, rightJoin);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_join_join, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, model, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, conditions, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, alias, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_join_innerjoin, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, model, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, conditions, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, alias, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_join_leftjoin, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, model, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, conditions, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, alias, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_query_builder_join_rightjoin, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, model, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, conditions, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, alias, IS_STRING, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_query_builder_join_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Join, join, arginfo_phalcon_mvc_model_query_builder_join_join, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Join, innerJoin, arginfo_phalcon_mvc_model_query_builder_join_innerjoin, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Join, leftJoin, arginfo_phalcon_mvc_model_query_builder_join_leftjoin, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Query_Builder_Join, rightJoin, arginfo_phalcon_mvc_model_query_builder_join_rightjoin, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Query\Builder\Join initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Query_Builder_Join){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\Query\\Builder, Join, mvc_model_query_builder_join, phalcon_mvc_model_query_builder_where_ce, phalcon_mvc_model_query_builder_join_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_mvc_model_query_builder_join_ce, SL("_joins"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_mvc_model_query_builder_join_ce, 1, phalcon_mvc_model_query_builderinterface_ce);

	return SUCCESS;
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
 * @return Phalcon\Mvc\Model\Query\Builder\Join
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Join, join){

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
	zval_ptr_dtor(&join);
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
 * @return Phalcon\Mvc\Model\Query\Builder\Join
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Join, innerJoin){

	zval *model, *conditions = NULL, *alias = NULL, type = {}, join = {};

	phalcon_fetch_params(0, 1, 2, &model, &conditions, &alias);

	if (!conditions) {
		conditions = &PHALCON_GLOBAL(z_null);
	}

	if (!alias) {
		alias = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STRING(&type, "INNER");

	array_init_size(&join, 4);
	phalcon_array_append(&join, model, PH_COPY);
	phalcon_array_append(&join, conditions, PH_COPY);
	phalcon_array_append(&join, alias, PH_COPY);
	phalcon_array_append(&join, &type, 0);
	phalcon_update_property_array_append(getThis(), SL("_joins"), &join);
	zval_ptr_dtor(&join);
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
 * @return Phalcon\Mvc\Model\Query\Builder\Join
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Join, leftJoin){

	zval *model, *conditions = NULL, *alias = NULL, type = {}, join = {};

	phalcon_fetch_params(0, 1, 2, &model, &conditions, &alias);

	if (!conditions) {
		conditions = &PHALCON_GLOBAL(z_null);
	}

	if (!alias) {
		alias = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STRING(&type, "LEFT");

	array_init_size(&join, 4);
	phalcon_array_append(&join, model, PH_COPY);
	phalcon_array_append(&join, conditions, PH_COPY);
	phalcon_array_append(&join, alias, PH_COPY);
	phalcon_array_append(&join, &type, 0);
	phalcon_update_property_array_append(getThis(), SL("_joins"), &join);
	zval_ptr_dtor(&join);
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
 * @return Phalcon\Mvc\Model\Query\Builder\Join
 */
PHP_METHOD(Phalcon_Mvc_Model_Query_Builder_Join, rightJoin){

	zval *model, *conditions = NULL, *alias = NULL, type = {}, join = {};

	phalcon_fetch_params(0, 1, 2, &model, &conditions, &alias);

	if (!conditions) {
		conditions = &PHALCON_GLOBAL(z_null);
	}

	if (!alias) {
		alias = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STRING(&type, "RIGHT");

	array_init_size(&join, 4);
	phalcon_array_append(&join, model, PH_COPY);
	phalcon_array_append(&join, conditions, PH_COPY);
	phalcon_array_append(&join, alias, PH_COPY);
	phalcon_array_append(&join, &type, 0);
	phalcon_update_property(getThis(), SL("_joins"), &join);
	zval_ptr_dtor(&join);
	RETURN_THIS();
}
