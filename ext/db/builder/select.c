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

#include "db/builder/select.h"
#include "db/builder/join.h"
#include "db/builder/exception.h"
#include "db/adapterinterface.h"

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
#include "kernel/debug.h"

/**
 * Phalcon\Db\Builder\Select
 */
zend_class_entry *phalcon_db_builder_select_ce;

PHP_METHOD(Phalcon_Db_Builder_Select, __construct);
PHP_METHOD(Phalcon_Db_Builder_Select, distinct);
PHP_METHOD(Phalcon_Db_Builder_Select, columns);
PHP_METHOD(Phalcon_Db_Builder_Select, orderBy);
PHP_METHOD(Phalcon_Db_Builder_Select, having);
PHP_METHOD(Phalcon_Db_Builder_Select, limit);
PHP_METHOD(Phalcon_Db_Builder_Select, offset);
PHP_METHOD(Phalcon_Db_Builder_Select, groupBy);
PHP_METHOD(Phalcon_Db_Builder_Select, _execute);
PHP_METHOD(Phalcon_Db_Builder_Select, count);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_builder_select___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, tables)
	ZEND_ARG_TYPE_INFO(0, db, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_builder_select_distinct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, distinct, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_builder_select_columns, 0, 0, 1)
	ZEND_ARG_INFO(0, columns)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_builder_select_orderby, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, orderBy, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_builder_select_having, 0, 0, 1)
	ZEND_ARG_INFO(0, having)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_builder_select_limit, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, limit, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_builder_select_offset, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, offset, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_builder_select_groupby, 0, 0, 1)
	ZEND_ARG_INFO(0, group)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_db_builder_select_method_entry[] = {
	PHP_ME(Phalcon_Db_Builder_Select, __construct, arginfo_phalcon_db_builder_select___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Db_Builder_Select, distinct, arginfo_phalcon_db_builder_select_distinct, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Builder_Select, columns, arginfo_phalcon_db_builder_select_columns, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Builder_Select, orderBy, arginfo_phalcon_db_builder_select_orderby, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Builder_Select, having, arginfo_phalcon_db_builder_select_having, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Builder_Select, limit, arginfo_phalcon_db_builder_select_limit, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Builder_Select, offset, arginfo_phalcon_db_builder_select_offset, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Builder_Select, groupBy, arginfo_phalcon_db_builder_select_groupby, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Builder_Select, _execute, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Db_Builder_Select, count, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Db\Builder\Select initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Builder_Select){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Db\\Builder, Select, db_builder_select, phalcon_db_builder_join_ce, phalcon_db_builder_select_method_entry, 0);

	return SUCCESS;
}

/**
 * Phalcon\Db\Builder\Select constructor
 *
 * @param string|array $tables
 * @param string $db
 */
PHP_METHOD(Phalcon_Db_Builder_Select, __construct){

	zval *tables, *db = NULL, columns = {};

	phalcon_fetch_params(0, 1, 1, &tables, &db);

	if (Z_TYPE_P(tables) != IS_STRING && Z_TYPE_P(tables) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_builder_exception_ce, "The tables must be string or array");
		return;
	}

	phalcon_update_property_array_str(getThis(), SL("_definition"), SL("tables"), tables);

	if (db && PHALCON_IS_NOT_EMPTY(db)) {
		phalcon_update_property(getThis(), SL("_defaultConnectionService"), db);
	}

	ZVAL_STRINGL(&columns, "*", 1);
	phalcon_update_property_array_str(getThis(), SL("_definition"), SL("columns"), &columns);
	zval_ptr_dtor(&columns);
}

/**
 * Sets SELECT DISTINCT / SELECT ALL flag
 *
 * @param bool|null distinct
 * @return Phalcon\Db\Builder\Select
 */
PHP_METHOD(Phalcon_Db_Builder_Select, distinct){

	zval *distinct;

	phalcon_fetch_params(0, 1, 0, &distinct);

	phalcon_update_property_array_str(getThis(), SL("_definition"), SL("distinct"), distinct);
	RETURN_THIS();
}

/**
 * Sets the columns to be queried
 *
 * @param string|array $columns
 * @return Phalcon\Db\Builder\Select
 */
PHP_METHOD(Phalcon_Db_Builder_Select, columns){

	zval *columns;

	phalcon_fetch_params(0, 1, 0, &columns);

	phalcon_update_property_array_str(getThis(), SL("_definition"), SL("columns"), columns);
	RETURN_THIS();
}

/**
 * Sets a ORDER BY condition clause
 *
 * @param string|array $orderBy
 * @return Phalcon\Db\Builder\Select
 */
PHP_METHOD(Phalcon_Db_Builder_Select, orderBy){

	zval *order_by;

	phalcon_fetch_params(0, 1, 0, &order_by);

	phalcon_update_property_array_str(getThis(), SL("_definition"), SL("order"), order_by);
	RETURN_THIS();
}

/**
 * Sets a HAVING condition clause. You need to escape PHQL reserved words using [ and ] delimiters
 *
 *<code>
 *	$builder->having('SUM(Robots.price) > 0');
 *</code>
 *
 * @param string $having
 * @return Phalcon\Db\Builder\Select
 */
PHP_METHOD(Phalcon_Db_Builder_Select, having){

	zval *having;

	phalcon_fetch_params(0, 1, 0, &having);

	phalcon_update_property_array_str(getThis(), SL("_definition"), SL("having"), having);
	RETURN_THIS();
}

/**
 * Sets a LIMIT clause, optionally a offset clause
 *
 * @param int $limit
 * @param int $offset
 * @return Phalcon\Db\Builder\Select
 */
PHP_METHOD(Phalcon_Db_Builder_Select, limit){

	zval *limit, *offset = NULL;

	phalcon_fetch_params(0, 1, 1, &limit, &offset);

	phalcon_update_property_array_str(getThis(), SL("_definition"), SL("limit"), limit);

	if (offset) {
		phalcon_update_property_array_str(getThis(), SL("_definition"), SL("offset"), offset);
	}

	RETURN_THIS();
}

/**
 * Sets an OFFSET clause
 *
 * @param int $offset
 * @return Phalcon\Db\Builder\Select
 */
PHP_METHOD(Phalcon_Db_Builder_Select, offset){

	zval *offset;

	phalcon_fetch_params(0, 1, 0, &offset);

	phalcon_update_property_array_str(getThis(), SL("_definition"), SL("offset"), offset);
	RETURN_THIS();
}

/**
 * Sets a GROUP BY clause
 *
 * @param string|array $group
 * @return Phalcon\Db\Builder\Select
 */
PHP_METHOD(Phalcon_Db_Builder_Select, groupBy){

	zval *group;

	phalcon_fetch_params(0, 1, 0, &group);

	phalcon_update_property_array_str(getThis(), SL("_definition"), SL("group"), group);
	RETURN_THIS();
}

/**
 * Returns a PHQL statement built based on the builder parameters
 *
 * @return Phalcon\Db\ResultInterface
 */
PHP_METHOD(Phalcon_Db_Builder_Select, _execute){

	zval *count = NULL, definition = {}, conditions = {}, joins = {}, bind_params = {}, bind_types = {}, service = {}, dependency_injector = {}, connection = {}, dialect = {}, sql_select = {};

	phalcon_fetch_params(1, 0, 1, &count);

	if (!count) {
		count = &PHALCON_GLOBAL(z_false);
	}

	phalcon_read_property(&definition, getThis(), SL("_definition"), PH_SEPARATE);
	PHALCON_MM_ADD_ENTRY(&definition);

	phalcon_read_property(&conditions, getThis(), SL("_conditions"), PH_READONLY);
	if (PHALCON_IS_NOT_EMPTY(&conditions)) {
		phalcon_array_update_str(&definition, SL("where"), &conditions, PH_COPY);
	}

	phalcon_read_property(&joins, getThis(), SL("_joins"), PH_READONLY);
	if (PHALCON_IS_NOT_EMPTY(&joins)) {
		phalcon_array_update_str(&definition, SL("joins"), &joins, PH_COPY);
	}

	PHALCON_MM_CALL_SELF(&bind_params, "getbindparams");
	PHALCON_MM_ADD_ENTRY(&bind_params);

	PHALCON_MM_CALL_SELF(&bind_types, "getbindtypes");
	PHALCON_MM_ADD_ENTRY(&bind_types);

	phalcon_read_property(&service, getThis(), SL("_defaultConnectionService"), PH_READONLY);
	if (Z_TYPE(service) != IS_OBJECT) {
		if (PHALCON_IS_EMPTY(&service)) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_db_builder_exception_ce, "Invalid injected connection service");
			return;
		}

		PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi");
		PHALCON_MM_ADD_ENTRY(&dependency_injector);
		if (Z_TYPE(dependency_injector) != IS_OBJECT) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_db_builder_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
			return;
		}

		/**
		 * Request the connection service from the DI
		 */
		PHALCON_MM_CALL_METHOD(&connection, &dependency_injector, "getshared", &service);
		PHALCON_MM_ADD_ENTRY(&connection);
		if (Z_TYPE(connection) != IS_OBJECT) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_db_builder_exception_ce, "Invalid injected connection service");
			return;
		}
	} else {
		ZVAL_COPY_VALUE(&connection, &service);
		PHALCON_MM_VERIFY_INTERFACE(&connection, phalcon_db_adapterinterface_ce);
	}

	PHALCON_MM_CALL_METHOD(&dialect, &connection, "getdialect");
	PHALCON_MM_ADD_ENTRY(&dialect);

	PHALCON_MM_CALL_METHOD(&sql_select, &dialect, "select", &definition, count);
	PHALCON_MM_ADD_ENTRY(&sql_select);

	/**
	 * Execute the query
	 */
	PHALCON_MM_CALL_METHOD(return_value, &connection, "query", &sql_select, &bind_params, &bind_types);

	RETURN_MM();
}

/**
 * Returns a PHQL statement built based on the builder parameters
 *
 * @return Phalcon\Db\ResultInterface
 */
PHP_METHOD(Phalcon_Db_Builder_Select, count){

	PHALCON_CALL_METHOD(return_value, getThis(), "_execute", &PHALCON_GLOBAL(z_true));
}
