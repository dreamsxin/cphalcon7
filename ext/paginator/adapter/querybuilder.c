
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

#include "paginator/adapter/querybuilder.h"
#include "paginator/adapterinterface.h"
#include "paginator/exception.h"
#include "mvc/model/query/builderinterface.h"
#include "mvc/model/managerinterface.h"
#include "db/rawvalue.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/exception.h"
#include "kernel/operators.h"
#include "kernel/fcall.h"
#include "kernel/hash.h"
#include "kernel/concat.h"
#include "kernel/string.h"

#include "internal/arginfo.h"
#include "interned-strings.h"

/**
 * Phalcon\Paginator\Adapter\QueryBuilder
 *
 * Pagination using a PHQL query builder as source of data
 *
 *<code>
 *  $builder = $this->modelsManager->createBuilder()
 *                   ->columns('id, name')
 *                   ->from('Robots')
 *                   ->orderBy('name');
 *
 *  $paginator = new Phalcon\Paginator\Adapter\QueryBuilder(array(
 *      "builder" => $builder,
 *      "limit"=> 20,
 *      "page" => 1
 *  ));
 *</code>
 */
zend_class_entry *phalcon_paginator_adapter_querybuilder_ce;

PHP_METHOD(Phalcon_Paginator_Adapter_QueryBuilder, __construct);
PHP_METHOD(Phalcon_Paginator_Adapter_QueryBuilder, getPaginate);
PHP_METHOD(Phalcon_Paginator_Adapter_QueryBuilder, setCurrentPage);
PHP_METHOD(Phalcon_Paginator_Adapter_QueryBuilder, getCurrentPage);
PHP_METHOD(Phalcon_Paginator_Adapter_QueryBuilder, setLimit);
PHP_METHOD(Phalcon_Paginator_Adapter_QueryBuilder, getLimit);
PHP_METHOD(Phalcon_Paginator_Adapter_QueryBuilder, setQueryBuilder);
PHP_METHOD(Phalcon_Paginator_Adapter_QueryBuilder, getQueryBuilder);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_paginator_adapter_querybuilder___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, config)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_paginator_adapter_querybuilder_setlimit, 0, 0, 1)
	ZEND_ARG_INFO(0, limit)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_paginator_adapter_querybuilder_setquerybuilder, 0, 0, 1)
	ZEND_ARG_INFO(0, queryBuilder)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_paginator_adapter_querybuilder_method_entry[] = {
	PHP_ME(Phalcon_Paginator_Adapter_QueryBuilder, __construct, arginfo_phalcon_paginator_adapter_querybuilder___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Paginator_Adapter_QueryBuilder, getPaginate, arginfo_phalcon_paginator_adapterinterface_getpaginate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Paginator_Adapter_QueryBuilder, setLimit, arginfo_phalcon_paginator_adapter_querybuilder_setlimit, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Paginator_Adapter_QueryBuilder, getLimit, arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Paginator_Adapter_QueryBuilder, setCurrentPage, arginfo_phalcon_paginator_adapterinterface_setcurrentpage, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Paginator_Adapter_QueryBuilder, getCurrentPage, arginfo_phalcon_paginator_adapterinterface_getcurrentpage, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Paginator_Adapter_QueryBuilder, setQueryBuilder, arginfo_phalcon_paginator_adapter_querybuilder_setquerybuilder, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Paginator_Adapter_QueryBuilder, getQueryBuilder, arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Paginator\Adapter\QueryBuilder initializer
 */
PHALCON_INIT_CLASS(Phalcon_Paginator_Adapter_QueryBuilder){

	PHALCON_REGISTER_CLASS(Phalcon\\Paginator\\Adapter, QueryBuilder, paginator_adapter_querybuilder, phalcon_paginator_adapter_querybuilder_method_entry, 0);

	zend_declare_property_null(phalcon_paginator_adapter_querybuilder_ce, SL("_builder"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_paginator_adapter_querybuilder_ce, SL("_limitRows"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_paginator_adapter_querybuilder_ce, SL("_page"), 1, ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_paginator_adapter_querybuilder_ce, 1, phalcon_paginator_adapterinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Paginator\Adapter\QueryBuilder
 *
 * @param array $config
 */
PHP_METHOD(Phalcon_Paginator_Adapter_QueryBuilder, __construct){

	zval *config, builder = {}, limit = {}, page = {};
	long int i_limit;

	phalcon_fetch_params(0, 1, 0, &config);

	if (!phalcon_array_isset_fetch_str(&builder, config, SL("builder"), PH_READONLY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_paginator_exception_ce, "Parameter 'builder' is required");
		return;
	}

	PHALCON_VERIFY_INTERFACE_EX(&builder, phalcon_mvc_model_query_builderinterface_ce, phalcon_paginator_exception_ce);

	phalcon_update_property(getThis(), SL("_builder"), &builder);

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
}

/**
 * Set current page number
 *
 * @param int $page
 */
PHP_METHOD(Phalcon_Paginator_Adapter_QueryBuilder, setCurrentPage){

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
PHP_METHOD(Phalcon_Paginator_Adapter_QueryBuilder, getCurrentPage){

	RETURN_MEMBER(getThis(), "_page");
}

/**
 * Set current rows limit
 *
 * @param int $limit
 *
 * @return Phalcon\Paginator\Adapter\QueryBuilder $this Fluent interface
 */
PHP_METHOD(Phalcon_Paginator_Adapter_QueryBuilder, setLimit){

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
PHP_METHOD(Phalcon_Paginator_Adapter_QueryBuilder, getLimit){

	RETURN_MEMBER(getThis(), "_limitRows");
}

/**
 * Set query builder object
 *
 * @param Phalcon\Mvc\Model\Query\BuilderInterface $builder
 *
 * @return Phalcon\Paginator\Adapter\QueryBuilder $this Fluent interface
 */
PHP_METHOD(Phalcon_Paginator_Adapter_QueryBuilder, setQueryBuilder){

	zval *query_builder;

	phalcon_fetch_params(0, 1, 0, &query_builder);
	PHALCON_VERIFY_INTERFACE_EX(query_builder, phalcon_mvc_model_query_builderinterface_ce, phalcon_paginator_exception_ce);

	phalcon_update_property(getThis(), SL("_builder"), query_builder);

	RETURN_THIS();
}

/**
 * Get query builder object
 *
 * @return Phalcon\Mvc\Model\Query\BuilderInterface $builder
 */
PHP_METHOD(Phalcon_Paginator_Adapter_QueryBuilder, getQueryBuilder){

	RETURN_MEMBER(getThis(), "_builder");
}

/**
 * Returns a slice of the resultset to show in the pagination
 *
 * @return \stdClass
 */
PHP_METHOD(Phalcon_Paginator_Adapter_QueryBuilder, getPaginate){

	zval original_builder = {}, builder = {}, total_builder = {}, limit = {}, number_page = {}, number = {}, query = {}, items = {}, total_query = {}, result = {}, row = {}, rowcount = {}, dependency_injector = {};
	zval service_name = {}, models_manager = {}, models = {}, model_name = {}, model = {}, connection = {}, bind_params = {}, bind_types = {}, processed = {}, *value, processed_types = {};
	zval intermediate = {}, columns = {}, *column, dialect = {}, sql_select = {}, sql = {};
	zend_string *str_key;
	ulong idx;
	ldiv_t tp;
	long int i_limit, i_number_page, i_number, i_before, i_rowcount;
	long int i_total_pages, i_next;

	phalcon_read_property(&original_builder, getThis(), SL("_builder"), PH_READONLY);

	/* Make a copy of the original builder to leave it as it is */
	if (phalcon_clone(&builder, &original_builder) == FAILURE) {
		return;
	}

	/* make a copy of the original builder to count the total of records */
	if (phalcon_clone(&total_builder, &builder) == FAILURE) {
		return;
	}

	phalcon_read_property(&limit, getThis(), SL("_limitRows"), PH_READONLY);
	phalcon_read_property(&number_page, getThis(), SL("_page"), PH_READONLY);
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

	/* Set the limit clause avoiding negative offsets */
	if (i_number < i_limit) {
		PHALCON_CALL_METHOD(NULL, &builder, "limit", &limit);
	} else {
		ZVAL_LONG(&number, i_number);
		PHALCON_CALL_METHOD(NULL, &builder, "limit", &limit, &number);
	}

	PHALCON_CALL_METHOD(&query, &builder, "getquery");

	/* Execute the query an return the requested slice of data */
	PHALCON_CALL_METHOD(&items, &query, "execute");

	/* Remove the 'ORDER BY' clause, PostgreSQL requires this */
	PHALCON_CALL_METHOD(NULL, &total_builder, "orderby", &PHALCON_GLOBAL(z_null));

	/* Obtain the PHQL for the total query */
	PHALCON_CALL_METHOD(&total_query, &total_builder, "getquery");

	PHALCON_CALL_METHOD(&dependency_injector, &total_query, "getdi");
	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_paginator_exception_ce, "A dependency injection object is required to access internal services");
		return;
	}

	/* Get the connection through the model */
	ZVAL_STR(&service_name, IS(modelsManager));

	PHALCON_CALL_METHOD(&models_manager, &dependency_injector, "getshared", &service_name);
	if (Z_TYPE(models_manager) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_paginator_exception_ce, "The injected service 'modelsManager' is not object");
		return;
	}

	PHALCON_VERIFY_INTERFACE(&models_manager, phalcon_mvc_model_managerinterface_ce);

	PHALCON_CALL_METHOD(&models, &builder, "getfrom");

	if (Z_TYPE(models) == IS_ARRAY) {
		phalcon_array_get_current(&model_name, &models);
	} else {
		ZVAL_COPY_VALUE(&model_name, &models);
	}

	PHALCON_CALL_METHOD(&model, &models_manager, "load", &model_name);
	PHALCON_CALL_METHOD(&connection, &model, "getreadconnection");
	PHALCON_CALL_METHOD(&intermediate, &total_query, "parse");

	phalcon_array_fetch_string(&columns, &intermediate, IS(columns), PH_READONLY);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(columns), column) {
		zval type = {}, select_columns = {};

		phalcon_array_fetch_string(&type, column, IS(type), PH_NOISY|PH_READONLY);

		/**
		 * Complete objects are treated in a different way
		 */
		if (PHALCON_IS_STRING(&type, "object")) {
			ZVAL_STRING(&select_columns, "*");

			phalcon_array_update_string(&intermediate, IS(columns), &select_columns, PH_COPY);
			break;
		}
	} ZEND_HASH_FOREACH_END();

	PHALCON_CALL_METHOD(&dialect, &connection, "getdialect");
	PHALCON_CALL_METHOD(&sql_select, &dialect, "select", &intermediate);

	PHALCON_CALL_METHOD(&bind_params, &total_query, "getbindparams");
	PHALCON_CALL_METHOD(&bind_types, &total_query, "getbindtypes");

	PHALCON_CONCAT_SVS(&sql, "SELECT COUNT(*) \"rowcount\" FROM (", &sql_select, ") AS T");

	/**
	 * Replace the placeholders
	 */
	if (Z_TYPE(bind_params) == IS_ARRAY) {
		PHALCON_SEPARATE(&bind_types);
		array_init(&processed);

		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(bind_params), idx, str_key, value) {
			zval wildcard = {}, string_wildcard = {}, sql_tmp = {};
			if (str_key) {
				ZVAL_STR(&wildcard, str_key);
			} else {
				ZVAL_LONG(&wildcard, idx);
			}

			if (Z_TYPE_P(value) == IS_OBJECT && instanceof_function(Z_OBJCE_P(value), phalcon_db_rawvalue_ce)) {
				PHALCON_CONCAT_SV(&string_wildcard, ":", &wildcard);

				ZVAL_COPY_VALUE(&sql_tmp, &sql);
				PHALCON_STR_REPLACE(&sql, &string_wildcard, value, &sql_tmp);

				phalcon_array_unset(&bind_types, &wildcard, 0);
			} else if (Z_TYPE(wildcard) == IS_LONG) {
				PHALCON_CONCAT_SV(&string_wildcard, ":", &wildcard);
				phalcon_array_update(&processed, &string_wildcard, value, PH_COPY);
			} else {
				phalcon_array_update(&processed, &wildcard, value, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();

	} else {
		ZVAL_COPY_VALUE(&processed, &bind_params);
	}

	/**
	 * Replace the bind Types
	 */
	if (Z_TYPE(bind_types) == IS_ARRAY) {
		array_init(&processed_types);

		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(bind_types), idx, str_key, value) {
			zval wildcard = {}, string_wildcard = {};
			if (str_key) {
				ZVAL_STR(&wildcard, str_key);
			} else {
				ZVAL_LONG(&wildcard, idx);
			}

			if (Z_TYPE(wildcard) == IS_LONG) {
				PHALCON_CONCAT_SV(&string_wildcard, ":", &wildcard);
				phalcon_array_update(&processed_types, &string_wildcard, value, PH_COPY);
			} else {
				phalcon_array_update(&processed_types, &wildcard, value, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();

	} else {
		ZVAL_COPY_VALUE(&processed_types, &bind_types);
	}

	PHALCON_CALL_METHOD(&result, &connection, "query", &sql, &processed, &processed_types);
	PHALCON_CALL_METHOD(&row, &result, "fetch");

	phalcon_array_fetch_str(&rowcount, &row, SL("rowcount"), PH_NOISY|PH_READONLY);

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
