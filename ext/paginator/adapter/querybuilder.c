
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

#include "paginator/adapter/querybuilder.h"
#include "paginator/adapter.h"
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
PHP_METHOD(Phalcon_Paginator_Adapter_QueryBuilder, setQueryBuilder);
PHP_METHOD(Phalcon_Paginator_Adapter_QueryBuilder, getQueryBuilder);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_paginator_adapter_querybuilder___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, config)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_paginator_adapter_querybuilder_setquerybuilder, 0, 0, 1)
	ZEND_ARG_INFO(0, queryBuilder)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_paginator_adapter_querybuilder_method_entry[] = {
	PHP_ME(Phalcon_Paginator_Adapter_QueryBuilder, __construct, arginfo_phalcon_paginator_adapter_querybuilder___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Paginator_Adapter_QueryBuilder, getPaginate, arginfo_phalcon_paginator_adapterinterface_getpaginate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Paginator_Adapter_QueryBuilder, setQueryBuilder, arginfo_phalcon_paginator_adapter_querybuilder_setquerybuilder, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Paginator_Adapter_QueryBuilder, getQueryBuilder, arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Paginator\Adapter\QueryBuilder initializer
 */
PHALCON_INIT_CLASS(Phalcon_Paginator_Adapter_QueryBuilder){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Paginator\\Adapter, QueryBuilder, paginator_adapter_querybuilder, phalcon_paginator_adapter_ce, phalcon_paginator_adapter_querybuilder_method_entry, 0);

	zend_declare_property_null(phalcon_paginator_adapter_querybuilder_ce, SL("_builder"), ZEND_ACC_PROTECTED);

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

	zval event_name = {}, original_builder = {}, builder = {}, total_builder = {}, limit = {}, number_page = {}, number = {}, query = {}, items = {};
	zval total_query = {}, result = {}, row = {}, rowcount = {}, dependency_injector = {};
	zval service_name = {}, models_manager = {}, models = {}, model_name = {}, model = {}, connection = {}, bind_params = {}, bind_types = {};
	zval processed = {}, *value, processed_types = {};
	zval intermediate = {}, dialect = {}, sql = {}, tmp = {}, page = {};
	zend_string *str_key;
	ulong idx;
	ldiv_t tp;
	long int i_limit, i_number_page, i_number, i_before, i_rowcount;
	long int i_total_pages, i_next;

	ZVAL_STRING(&event_name, "query:beforeGetPaginate");
	PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
	zval_ptr_dtor(&event_name);

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
	zval_ptr_dtor(&query);

	/* Remove the 'ORDER BY' clause, PostgreSQL requires this */
	PHALCON_CALL_METHOD(NULL, &total_builder, "orderby", &PHALCON_GLOBAL(z_null));

	/* Obtain the PHQL for the total query */
	PHALCON_CALL_METHOD(&total_query, &total_builder, "getquery");
	zval_ptr_dtor(&total_builder);

	PHALCON_CALL_METHOD(&dependency_injector, &total_query, "getdi");
	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_paginator_exception_ce, "A dependency injection object is required to access internal services");
		return;
	}

	/* Get the connection through the model */
	ZVAL_STR(&service_name, IS(modelsManager));

	PHALCON_CALL_METHOD(&models_manager, &dependency_injector, "getshared", &service_name);
	zval_ptr_dtor(&dependency_injector);
	if (Z_TYPE(models_manager) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_paginator_exception_ce, "The injected service 'modelsManager' is not object");
		return;
	}

	PHALCON_VERIFY_INTERFACE(&models_manager, phalcon_mvc_model_managerinterface_ce);

	PHALCON_CALL_METHOD(&models, &builder, "getfrom");
	zval_ptr_dtor(&builder);

	if (Z_TYPE(models) == IS_ARRAY) {
		phalcon_array_get_current(&model_name, &models);
	} else {
		ZVAL_COPY(&model_name, &models);
	}
	zval_ptr_dtor(&models);

	PHALCON_CALL_METHOD(&model, &models_manager, "load", &model_name);
	zval_ptr_dtor(&models_manager);
	zval_ptr_dtor(&model_name);
	PHALCON_CALL_METHOD(&connection, &model, "getreadconnection");
	zval_ptr_dtor(&model);
	PHALCON_CALL_METHOD(&intermediate, &total_query, "parse");
	PHALCON_SEPARATE(&intermediate);

	PHALCON_CALL_METHOD(&tmp, &total_query, "getindex");
	if (Z_TYPE(tmp) > IS_NULL) {
		phalcon_array_update_str(&intermediate, SL("index"), &tmp, PH_COPY);
		zval_ptr_dtor(&tmp);
	}

	PHALCON_CALL_METHOD(&dialect, &connection, "getdialect");
	PHALCON_CALL_METHOD(&sql, &dialect, "select", &intermediate, &PHALCON_GLOBAL(z_true));
	zval_ptr_dtor(&dialect);
	zval_ptr_dtor(&intermediate);

	PHALCON_CALL_METHOD(&bind_params, &total_query, "getbindparams");
	PHALCON_CALL_METHOD(&bind_types, &total_query, "getbindtypes");
	zval_ptr_dtor(&total_query);

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

				PHALCON_STR_REPLACE(&sql_tmp, &string_wildcard, value, &sql);
				zval_ptr_dtor(&string_wildcard);
				zval_ptr_dtor(&sql);
				ZVAL_COPY_VALUE(&sql, &sql_tmp);

				phalcon_array_unset(&bind_types, &wildcard, 0);
			} else if (Z_TYPE(wildcard) == IS_LONG) {
				PHALCON_CONCAT_SV(&string_wildcard, ":", &wildcard);
				phalcon_array_update(&processed, &string_wildcard, value, PH_COPY);
				zval_ptr_dtor(&string_wildcard);
			} else {
				phalcon_array_update(&processed, &wildcard, value, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();

	} else {
		ZVAL_COPY(&processed, &bind_params);
	}
	zval_ptr_dtor(&bind_params);

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
		ZVAL_COPY(&processed_types, &bind_types);
	}
	zval_ptr_dtor(&bind_types);

	PHALCON_CALL_METHOD(&result, &connection, "query", &sql, &processed, &processed_types);
	zval_ptr_dtor(&connection);
	zval_ptr_dtor(&sql);
	zval_ptr_dtor(&processed);
	zval_ptr_dtor(&processed_types);
	PHALCON_CALL_METHOD(&row, &result, "fetch");
	zval_ptr_dtor(&result);

	phalcon_array_fetch_str(&rowcount, &row, SL("rowcount"), PH_NOISY|PH_READONLY);

	i_rowcount    = phalcon_get_intval(&rowcount);
	tp            = ldiv(i_rowcount, i_limit);
	i_total_pages = tp.quot + (tp.rem ? 1 : 0);
	i_next        = (i_number_page < i_total_pages) ? (i_number_page + 1) : i_total_pages;

	zval_ptr_dtor(&row);

	object_init(&page);
	phalcon_update_property(&page, SL("items"),			   &items);
	zval_ptr_dtor(&items);
	phalcon_update_property_long(&page, SL("before"),      i_before);
	phalcon_update_property_long(&page, SL("first"),       1);
	phalcon_update_property_long(&page, SL("next"),        i_next);
	phalcon_update_property_long(&page, SL("last"),        i_total_pages);
	phalcon_update_property_long(&page, SL("current"),     i_number_page);
	phalcon_update_property_long(&page, SL("total_pages"), i_total_pages);
	phalcon_update_property_long(&page, SL("total_items"), i_rowcount);

	ZVAL_STRING(&event_name, "query:afterGetPaginate");
	PHALCON_CALL_METHOD(return_value, getThis(), "fireeventdata", &event_name, &page);
	zval_ptr_dtor(&event_name);

	if (zend_is_true(return_value)) {
		zval_ptr_dtor(&page);
	} else {
		zval_ptr_dtor(return_value);
		RETURN_ZVAL(&page, 0, 0);
	}
}
