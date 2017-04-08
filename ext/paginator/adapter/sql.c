
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

#include "paginator/adapter/sql.h"
#include "paginator/adapterinterface.h"
#include "paginator/exception.h"
#include "db/adapterinterface.h"
#include "di/injectable.h"

#include <ext/pdo/php_pdo_driver.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/exception.h"
#include "kernel/operators.h"
#include "kernel/fcall.h"

#include "internal/arginfo.h"

/**
 * Phalcon\Paginator\Adapter\Sql
 *
 * Pagination using a SQL as source of data
 *
 * <code>
 * $sql = "SELECT * FROM robots WHERE type = :type LIMIT :limit OFFSET :offset";
 * $sql2 = "SELECT COUNT(*) rowcount FROM robots WHERE type = :type FROM robots";
 *
 * $bind = ['type' => 'google'];
 *
 * $paginator = new \Phalcon\Paginator\Adapter\Sql(array(
 *                 "db" => $this->db,
 *                 "sql" => $sql,
 *                 "total_sql" => $sql2,
 *                 "bind" => $bind,
 *                 "limit" => 20,
 *                 "page" => $page
 * ));
 * </code>
 */
zend_class_entry *phalcon_paginator_adapter_sql_ce;

PHP_METHOD(Phalcon_Paginator_Adapter_Sql, __construct);
PHP_METHOD(Phalcon_Paginator_Adapter_Sql, getPaginate);
PHP_METHOD(Phalcon_Paginator_Adapter_Sql, setCurrentPage);
PHP_METHOD(Phalcon_Paginator_Adapter_Sql, getCurrentPage);
PHP_METHOD(Phalcon_Paginator_Adapter_Sql, setLimit);
PHP_METHOD(Phalcon_Paginator_Adapter_Sql, getLimit);
PHP_METHOD(Phalcon_Paginator_Adapter_Sql, setDb);
PHP_METHOD(Phalcon_Paginator_Adapter_Sql, getDb);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_paginator_adapter_sql___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, config)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_paginator_adapter_sql_setlimit, 0, 0, 1)
	ZEND_ARG_INFO(0, limit)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_paginator_adapter_sql_setdb, 0, 0, 1)
	ZEND_ARG_INFO(0, db)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_paginator_adapter_sql_method_entry[] = {
	PHP_ME(Phalcon_Paginator_Adapter_Sql, __construct, arginfo_phalcon_paginator_adapter_sql___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Paginator_Adapter_Sql, getPaginate, arginfo_phalcon_paginator_adapterinterface_getpaginate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Paginator_Adapter_Sql, setLimit, arginfo_phalcon_paginator_adapter_sql_setlimit, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Paginator_Adapter_Sql, getLimit, arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Paginator_Adapter_Sql, setCurrentPage, arginfo_phalcon_paginator_adapterinterface_setcurrentpage, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Paginator_Adapter_Sql, getCurrentPage, arginfo_phalcon_paginator_adapterinterface_getcurrentpage, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Paginator_Adapter_Sql, setDb, arginfo_phalcon_paginator_adapter_sql_setdb, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Paginator_Adapter_Sql, getDb, arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Paginator\Adapter\Sql initializer
 */
PHALCON_INIT_CLASS(Phalcon_Paginator_Adapter_Sql){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Paginator\\Adapter, Sql, paginator_adapter_sql, phalcon_di_injectable_ce, phalcon_paginator_adapter_sql_method_entry, 0);

	zend_declare_property_null(phalcon_paginator_adapter_sql_ce, SL("_db"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_paginator_adapter_sql_ce, SL("_sql"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_paginator_adapter_sql_ce, SL("_total_sql"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_paginator_adapter_sql_ce, SL("_bind"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_paginator_adapter_sql_ce, SL("_limitRows"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_paginator_adapter_sql_ce, SL("_page"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_paginator_adapter_sql_ce, SL("_fetchMode"), PDO_FETCH_OBJ, ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_paginator_adapter_sql_ce, 1, phalcon_paginator_adapterinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Paginator\Adapter\Sql
 *
 * @param array $config
 */
PHP_METHOD(Phalcon_Paginator_Adapter_Sql, __construct){

	zval *config, dbname = {}, db = {}, sql = {}, total_sql = {}, bind = {}, limit = {}, page = {}, fetch_mode = {};
	long int i_limit;

	phalcon_fetch_params(0, 1, 0, &config);

	if (!phalcon_array_isset_fetch_str(&sql, config, SL("sql"), PH_READONLY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_paginator_exception_ce, "Parameter 'sql' is required");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&total_sql, config, SL("total_sql"), PH_READONLY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_paginator_exception_ce, "Parameter 'total_sql' is required");
		return;
	}

	if (phalcon_array_isset_fetch_str(&bind, config, SL("bind"), PH_READONLY)) {
		if (Z_TYPE(bind) != IS_ARRAY) {
			phalcon_update_property_empty_array(getThis(), SL("_bind"));
		} else {
			phalcon_update_property(getThis(), SL("_bind"), &bind);
		}
	} else {
		phalcon_update_property_empty_array(getThis(), SL("_bind"));
	}


	if (!phalcon_array_isset_fetch_str(&dbname, config, SL("db"), PH_READONLY)) {
		ZVAL_STRING(&dbname, "db");
	}

	if (Z_TYPE(dbname) != IS_OBJECT) {
		PHALCON_CALL_METHOD(&db, getThis(), "getresolveservice", &dbname);
	} else {
		ZVAL_COPY_VALUE(&db, &dbname);
	}

	PHALCON_VERIFY_INTERFACE_EX(&db, phalcon_db_adapterinterface_ce, phalcon_paginator_exception_ce);
	phalcon_update_property(getThis(), SL("_db"), &db);

	phalcon_update_property(getThis(), SL("_sql"), &sql);
	phalcon_update_property(getThis(), SL("_total_sql"), &total_sql);

	if (!phalcon_array_isset_fetch_str(&limit, config, SL("limit"), PH_READONLY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_paginator_exception_ce, "Parameter 'limit' is required");
		return;
	}

	i_limit = phalcon_get_intval(&limit);
	if (i_limit < 1) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_paginator_exception_ce, "'limit' should be positive");
		return;
	}

	phalcon_update_property(getThis(), SL("_limitRows"), &limit);

	if (phalcon_array_isset_fetch_str(&page, config, SL("page"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_page"), &page);
	}

	if (phalcon_array_isset_fetch_str(&fetch_mode, config, SL("fetchMode"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_fetchMode"), &fetch_mode);
	}
}

/**
 * Set current page number
 *
 * @param int $page
 */
PHP_METHOD(Phalcon_Paginator_Adapter_Sql, setCurrentPage){

	zval *current_page;

	phalcon_fetch_params(0, 1, 0, &current_page);
	PHALCON_ENSURE_IS_LONG(current_page);

	phalcon_update_property(getThis(), SL("_page"), current_page);
	RETURN_THIS();
}

/**
 * Get current page number
 *
 * @param int $page
 */
PHP_METHOD(Phalcon_Paginator_Adapter_Sql, getCurrentPage){

	RETURN_MEMBER(getThis(), "_page");
}

/**
 * Set current rows limit
 *
 * @param int $limit
 *
 * @return Phalcon\Paginator\Adapter\Sql $this Fluent interface
 */
PHP_METHOD(Phalcon_Paginator_Adapter_Sql, setLimit){

	zval *current_limit;

	phalcon_fetch_params(0, 1, 0, &current_limit);
	PHALCON_ENSURE_IS_LONG(current_limit);

	phalcon_update_property(getThis(), SL("_limitRows"), current_limit);
	RETURN_THIS();
}

/**
 * Get current rows limit
 *
 * @return int $limit
 */
PHP_METHOD(Phalcon_Paginator_Adapter_Sql, getLimit){

	RETURN_MEMBER(getThis(), "_limitRows");
}

/**
 * Set query builder object
 *
 * @param Phalcon\Db\AdapterInterface $db
 *
 * @return Phalcon\Paginator\Adapter\Sql $this Fluent interface
 */
PHP_METHOD(Phalcon_Paginator_Adapter_Sql, setDb){

	zval *dbname, db = {};

	phalcon_fetch_params(0, 1, 0, &dbname);
	if (Z_TYPE_P(dbname) != IS_OBJECT) {
		PHALCON_CALL_METHOD(&db, getThis(), "getresolveservice", dbname);
	} else {
		ZVAL_COPY_VALUE(&db, dbname);
	}
	PHALCON_VERIFY_INTERFACE_EX(&db, phalcon_db_adapterinterface_ce, phalcon_paginator_exception_ce);

	phalcon_update_property(getThis(), SL("_db"), &db);

	RETURN_THIS();
}

/**
 * Get query builder object
 *
 * @return Phalcon\Db\AdapterInterface $db
 */
PHP_METHOD(Phalcon_Paginator_Adapter_Sql, getDb){

	RETURN_MEMBER(getThis(), "_db");
}

/**
 * Returns a slice of the resultset to show in the pagination
 *
 * @return stdClass
 */
PHP_METHOD(Phalcon_Paginator_Adapter_Sql, getPaginate){

	zval db = {}, sql = {}, total_sql = {}, bind = {}, limit = {}, number_page = {}, number = {}, fetch_mode = {}, items = {}, row = {}, rowcount = {};
	long int i_limit, i_number_page, i_number, i_before, i_rowcount;
	long int i_total_pages, i_next;
	ldiv_t tp;

	phalcon_read_property(&db, getThis(), SL("_db"), PH_READONLY);
	phalcon_read_property(&sql, getThis(), SL("_sql"), PH_READONLY);
	phalcon_read_property(&total_sql, getThis(), SL("_total_sql"), PH_READONLY);
	phalcon_read_property(&bind, getThis(), SL("_bind"), PH_SEPARATE);
	phalcon_read_property(&limit, getThis(), SL("_limitRows"), PH_READONLY);
	phalcon_read_property(&number_page, getThis(), SL("_page"), PH_READONLY);
	phalcon_read_property(&fetch_mode, getThis(), SL("_fetchMode"), PH_READONLY);

	i_limit       = phalcon_get_intval(&limit);
	i_number_page = phalcon_get_intval(&number_page);

	if (i_limit < 1) {
		/* This should never happen unless someone deliberately modified the properties of the object */
		i_limit = 10;
	}

	if (!i_number_page) {
		i_number_page = 1;
	}

	i_number = (i_number_page - 1) * i_limit;
	i_before = (i_number_page == 1) ? 1 : (i_number_page - 1);

	PHALCON_CALL_METHOD(&row, &db, "fetchone", &total_sql, &fetch_mode, &bind);

	phalcon_read_property(&rowcount, &row, SL("rowcount"), PH_READONLY);

	/* Set the limit clause avoiding negative offsets */
	if (i_number < i_limit) {
		phalcon_array_update_str(&bind, SL("limit"), &limit, PH_COPY);
		phalcon_array_update_str_long(&bind, SL("offset"), 0, 0);
	} else {
		ZVAL_LONG(&number, i_number);
		phalcon_array_update_str(&bind, SL("limit"), &limit, PH_COPY);
		phalcon_array_update_str(&bind, SL("offset"), &number, PH_COPY);
	}

	PHALCON_CALL_METHOD(&items, &db, "fetchall", &sql, &fetch_mode, &bind);

	i_rowcount    = phalcon_get_intval(&rowcount);
	tp            = ldiv(i_rowcount, i_limit);
	i_total_pages = tp.quot + (tp.rem ? 1 : 0);
	i_next        = (i_number_page < i_total_pages) ? (i_number_page + 1) : i_total_pages;

	object_init(return_value);
	phalcon_update_property(return_value, SL("items"),       &items);
	phalcon_update_property_long(return_value, SL("before"),      i_before);
	phalcon_update_property_long(return_value, SL("first"),       1);
	phalcon_update_property_long(return_value, SL("next"),        i_next);
	phalcon_update_property_long(return_value, SL("last"),        i_total_pages);
	phalcon_update_property_long(return_value, SL("current"),     i_number_page);
	phalcon_update_property_long(return_value, SL("total_pages"), i_total_pages);
	phalcon_update_property_long(return_value, SL("total_items"), i_rowcount);
}
