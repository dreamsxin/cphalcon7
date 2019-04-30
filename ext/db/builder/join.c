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

#include "db/builder/join.h"
#include "db/builder/where.h"

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

/**
 * Phalcon\Db\Builder\Join
 */
zend_class_entry *phalcon_db_builder_join_ce;

PHP_METHOD(Phalcon_Db_Builder_Join, join);
PHP_METHOD(Phalcon_Db_Builder_Join, innerJoin);
PHP_METHOD(Phalcon_Db_Builder_Join, leftJoin);
PHP_METHOD(Phalcon_Db_Builder_Join, rightJoin);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_builder_join_join, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, table, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, conditions, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, type, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_builder_join_innerjoin, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, table, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, conditions, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_builder_join_leftjoin, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, table, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, conditions, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_builder_join_rightjoin, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, table, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, conditions, IS_STRING, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_db_builder_join_method_entry[] = {
	PHP_ME(Phalcon_Db_Builder_Join, join, arginfo_phalcon_db_builder_join_join, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Builder_Join, innerJoin, arginfo_phalcon_db_builder_join_innerjoin, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Builder_Join, leftJoin, arginfo_phalcon_db_builder_join_leftjoin, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Builder_Join, rightJoin, arginfo_phalcon_db_builder_join_rightjoin, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Db\Builder\Join initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Builder_Join){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Db\\Builder, Join, db_builder_join, phalcon_db_builder_where_ce, phalcon_db_builder_join_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_db_builder_join_ce, SL("_joins"), ZEND_ACC_PROTECTED);

	zend_declare_class_constant_stringl(phalcon_db_builder_join_ce, SL("LEFT"),  SL("LEFT"));
	zend_declare_class_constant_stringl(phalcon_db_builder_join_ce, SL("RIGHT"),  SL("RIGHT"));
	zend_declare_class_constant_stringl(phalcon_db_builder_join_ce, SL("INNER"),  SL("INNER"));

	return SUCCESS;
}

/**
 * Adds a join to the query
 *
 * @param string $table
 * @param string $conditions
 * @param string $type
 * @return Phalcon\Db\Builder\Join
 */
PHP_METHOD(Phalcon_Db_Builder_Join, join){

	zval *table, *conditions = NULL, *type = NULL, join = {};

	phalcon_fetch_params(0, 1, 2, &table, &conditions, &type);

	if (!conditions) {
		conditions = &PHALCON_GLOBAL(z_null);
	}

	array_init_size(&join, 3);
	phalcon_array_update_str(&join, SL("source"), table, PH_COPY);
	phalcon_array_update_str(&join, SL("conditions"), conditions, PH_COPY);
	

	if (!type || Z_TYPE_P(type) == IS_NULL) {
		zval tmp = {};

		ZVAL_STRING(&tmp, "INNER");
		phalcon_array_update_str(&join, SL("type"), &tmp, 0);
	} else {
		phalcon_array_update_str(&join, SL("type"), type, PH_COPY);
	}
	phalcon_update_property_array_append(getThis(), SL("_joins"), &join);
	zval_ptr_dtor(&join);
	RETURN_THIS();
}

/**
 * Adds a INNER join to the query
 *
 * @param string $table
 * @param string $conditions
 * @return Phalcon\Db\Builder\Join
 */
PHP_METHOD(Phalcon_Db_Builder_Join, innerJoin){

	zval *table, *conditions = NULL, type = {}, join = {};

	phalcon_fetch_params(0, 1, 1, &table, &conditions);

	if (!conditions) {
		conditions = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STRING(&type, "INNER");

	array_init_size(&join, 3);
	phalcon_array_update_str(&join, SL("source"), table, PH_COPY);
	phalcon_array_update_str(&join, SL("conditions"), conditions, PH_COPY);
	phalcon_array_update_str(&join, SL("type"), &type, 0);
	phalcon_update_property_array_append(getThis(), SL("_joins"), &join);
	zval_ptr_dtor(&join);
	RETURN_THIS();
}

/**
 * Adds a LEFT join to the query
 *
 * @param string $table
 * @param string $conditions
 * @return Phalcon\Db\Builder\Join
 */
PHP_METHOD(Phalcon_Db_Builder_Join, leftJoin){

	zval *table, *conditions = NULL, type = {}, join = {};

	phalcon_fetch_params(0, 1, 1, &table, &conditions);

	if (!conditions) {
		conditions = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STRING(&type, "LEFT");

	array_init_size(&join, 3);
	phalcon_array_update_str(&join, SL("source"), table, PH_COPY);
	phalcon_array_update_str(&join, SL("conditions"), conditions, PH_COPY);
	phalcon_array_update_str(&join, SL("type"), &type, 0);
	phalcon_update_property_array_append(getThis(), SL("_joins"), &join);
	zval_ptr_dtor(&join);
	RETURN_THIS();
}

/**
 * Adds a RIGHT join to the query
 *
 * @param string $table
 * @param string $conditions
 * @return Phalcon\Db\Builder\Join
 */
PHP_METHOD(Phalcon_Db_Builder_Join, rightJoin){

	zval *table, *conditions = NULL, type = {}, join = {};

	phalcon_fetch_params(0, 1, 1, &table, &conditions);

	if (!conditions) {
		conditions = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STRING(&type, "RIGHT");

	array_init_size(&join, 3);
	phalcon_array_update_str(&join, SL("source"), table, PH_COPY);
	phalcon_array_update_str(&join, SL("conditions"), conditions, PH_COPY);
	phalcon_array_update_str(&join, SL("type"), &type, 0);
	phalcon_update_property(getThis(), SL("_joins"), &join);
	zval_ptr_dtor(&join);
	RETURN_THIS();
}
