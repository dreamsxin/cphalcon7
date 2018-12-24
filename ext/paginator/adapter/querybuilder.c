
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
	zend_declare_property_null(phalcon_paginator_adapter_querybuilder_ce, SL("_totalItems"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_paginator_adapter_querybuilder_ce, SL("_limitRows"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_paginator_adapter_querybuilder_ce, 1, phalcon_paginator_adapterinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Paginator\Adapter\QueryBuilder
 *
 * @param array $config
 */
PHP_METHOD(Phalcon_Paginator_Adapter_QueryBuilder, __construct){

	zval *config, builder = {}, limit = {}, page = {}, total_items = {};
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

	if (phalcon_array_isset_fetch_str(&total_items, config, SL("totalItems"), PH_READONLY) && Z_TYPE(total_items) == IS_LONG) {
		phalcon_update_property(getThis(), SL("_totalItems"), &total_items);
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

static inline void phalcon_query_sql_replace(zval *sql, zval *wildcard, zval *value)
{
	zval string_wildcard = {}, fixed_value = {}, sql_tmp = {};
	PHALCON_CONCAT_SVS(&string_wildcard, ":", wildcard, ",");
	if (!phalcon_memnstr(sql, &string_wildcard)) {
		zval_ptr_dtor(&string_wildcard);
		PHALCON_CONCAT_SVS(&string_wildcard, ":", wildcard, " ");
		if (!phalcon_memnstr(sql, &string_wildcard)) {
			zval_ptr_dtor(&string_wildcard);
			PHALCON_CONCAT_SVS(&string_wildcard, ":", wildcard, ")");

			if (!phalcon_memnstr(sql, &string_wildcard)) {
				zval_ptr_dtor(&string_wildcard);
				PHALCON_CONCAT_SV(&string_wildcard, ":", wildcard);
				ZVAL_COPY(&fixed_value, value);
			} else {
				PHALCON_CONCAT_VS(&fixed_value, value, ")");
			}
		} else {
			PHALCON_CONCAT_VS(&fixed_value, value, " ");
		}
	} else {
		PHALCON_CONCAT_VS(&fixed_value, value, ",");
	}
	phalcon_fast_str_replace(&sql_tmp, &string_wildcard, &fixed_value, sql);
	zval_ptr_dtor(&string_wildcard);
	zval_ptr_dtor(&fixed_value);
	ZVAL_COPY_VALUE(sql, &sql_tmp);
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

	PHALCON_MM_INIT();

	PHALCON_MM_ZVAL_STRING(&event_name, "pagination:beforeGetPaginate");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);

	phalcon_read_property(&original_builder, getThis(), SL("_builder"), PH_READONLY);

	/* Make a copy of the original builder to leave it as it is */
	if (phalcon_clone(&builder, &original_builder) == FAILURE) {
		RETURN_MM();
	}

	/* make a copy of the original builder to count the total of records */
	if (phalcon_clone(&total_builder, &builder) == FAILURE) {
		RETURN_MM();
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
		PHALCON_MM_CALL_METHOD(NULL, &builder, "limit", &limit);
	} else {
		ZVAL_LONG(&number, i_number);
		PHALCON_MM_CALL_METHOD(NULL, &builder, "limit", &limit, &number);
	}

	PHALCON_MM_CALL_METHOD(&query, &builder, "getquery");
	PHALCON_MM_ADD_ENTRY(&query);

	/* Execute the query an return the requested slice of data */
	PHALCON_MM_CALL_METHOD(&items, &query, "execute");
	PHALCON_MM_ADD_ENTRY(&items);

	phalcon_read_property(&rowcount, getThis(), SL("_totalItems"), PH_READONLY);
	if (Z_TYPE(rowcount) != IS_LONG) {
		/* Remove the 'ORDER BY' clause, PostgreSQL requires this */
		PHALCON_MM_CALL_METHOD(NULL, &total_builder, "orderby", &PHALCON_GLOBAL(z_null));

		/* Obtain the PHQL for the total query */
		PHALCON_MM_CALL_METHOD(&total_query, &total_builder, "getquery");
		PHALCON_MM_ADD_ENTRY(&total_query);

		PHALCON_MM_CALL_METHOD(&dependency_injector, &total_query, "getdi");
		PHALCON_MM_ADD_ENTRY(&dependency_injector);
		if (Z_TYPE(dependency_injector) != IS_OBJECT) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_paginator_exception_ce, "A dependency injection object is required to access internal services");
			return;
		}

		/* Get the connection through the model */
		ZVAL_STR(&service_name, IS(modelsManager));

		PHALCON_MM_CALL_METHOD(&models_manager, &dependency_injector, "getshared", &service_name);
		PHALCON_MM_ADD_ENTRY(&models_manager);
		if (Z_TYPE(models_manager) != IS_OBJECT) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_paginator_exception_ce, "The injected service 'modelsManager' is not object");
			return;
		}

		PHALCON_MM_VERIFY_INTERFACE(&models_manager, phalcon_mvc_model_managerinterface_ce);

		PHALCON_MM_CALL_METHOD(&models, &builder, "getfrom");
		PHALCON_MM_ADD_ENTRY(&models);

		if (Z_TYPE(models) == IS_ARRAY) {
			phalcon_array_get_current(&model_name, &models);
		} else {
			ZVAL_COPY(&model_name, &models);
		}
		PHALCON_MM_ADD_ENTRY(&model_name);

		PHALCON_MM_CALL_METHOD(&model, &models_manager, "load", &model_name);
		PHALCON_MM_ADD_ENTRY(&model);
		PHALCON_MM_CALL_METHOD(&connection, &model, "getreadconnection");
		PHALCON_MM_ADD_ENTRY(&connection);
		PHALCON_MM_CALL_METHOD(&intermediate, &total_query, "parse");
		PHALCON_MM_SEPARATE(&intermediate);

		PHALCON_MM_CALL_METHOD(&tmp, &total_query, "getindex");
		PHALCON_MM_ADD_ENTRY(&tmp);
		if (Z_TYPE(tmp) > IS_NULL) {
			phalcon_array_update_str(&intermediate, SL("index"), &tmp, PH_COPY);
		}

		PHALCON_MM_CALL_METHOD(&dialect, &connection, "getdialect");
		PHALCON_MM_ADD_ENTRY(&dialect);
		PHALCON_MM_CALL_METHOD(&sql, &dialect, "select", &intermediate, &PHALCON_GLOBAL(z_true));
		PHALCON_MM_ADD_ENTRY(&sql);

		PHALCON_MM_CALL_METHOD(&bind_params, &total_query, "getbindparams");
		PHALCON_MM_ADD_ENTRY(&bind_params);
		PHALCON_MM_CALL_METHOD(&bind_types, &total_query, "getbindtypes");
		PHALCON_MM_SEPARATE(&bind_types);

		/**
		 * Replace the placeholders
		 */
		if (Z_TYPE(bind_params) == IS_ARRAY) {
			array_init(&processed);
			PHALCON_MM_ADD_ENTRY(&processed);

			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(bind_params), idx, str_key, value) {
				zval wildcard = {};
				if (str_key) {
					ZVAL_STR(&wildcard, str_key);
				} else {
					ZVAL_LONG(&wildcard, idx);
				}

				if (Z_TYPE_P(value) == IS_OBJECT && instanceof_function(Z_OBJCE_P(value), phalcon_db_rawvalue_ce)) {
					zval tmp_value = {};
					PHALCON_MM_CALL_METHOD(&tmp_value, value, "getvalue");
					phalcon_query_sql_replace(&sql, &wildcard, &tmp_value);
					zval_ptr_dtor(&tmp_value);
					PHALCON_MM_ADD_ENTRY(&sql);
					phalcon_array_unset(&bind_types, &wildcard, 0);
				} else if (Z_TYPE_P(value) == IS_ARRAY) {
					zval *v, bind_keys = {}, joined_keys = {}, hidden_param = {};
					array_init(&bind_keys);
					ZVAL_LONG(&hidden_param, 0);
					ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(value), v) {
						zval k = {}, query_key = {};
						/**
						 * Key with auto bind-params
						 */
						PHALCON_CONCAT_SVV(&k, "phi", &wildcard, &hidden_param);

						PHALCON_CONCAT_SV(&query_key, ":", &k);
						phalcon_array_append(&bind_keys, &query_key, 0);
						phalcon_array_update(&processed, &k, v, PH_COPY);
						zval_ptr_dtor(&k);
						phalcon_increment(&hidden_param);
					} ZEND_HASH_FOREACH_END();

					phalcon_fast_join_str(&joined_keys, SL(", "), &bind_keys);
					zval_ptr_dtor(&bind_keys);
					phalcon_query_sql_replace(&sql, &wildcard, &joined_keys);
					PHALCON_MM_ADD_ENTRY(&sql);
					zval_ptr_dtor(&joined_keys);
					phalcon_array_unset(&bind_types, &wildcard, 0);
				} else if (Z_TYPE(wildcard) == IS_LONG) {
					zval string_wildcard = {};
					PHALCON_CONCAT_SV(&string_wildcard, ":", &wildcard);
					phalcon_array_update(&processed, &string_wildcard, value, PH_COPY);
					zval_ptr_dtor(&string_wildcard);
				} else {
					phalcon_array_update(&processed, &wildcard, value, PH_COPY);
				}
			} ZEND_HASH_FOREACH_END();
		} else {
			PHALCON_MM_ZVAL_DUP(&processed, &bind_params);
		}

		/**
		 * Replace the bind Types
		 */
		if (Z_TYPE(bind_types) == IS_ARRAY) {
			array_init(&processed_types);
			PHALCON_MM_ADD_ENTRY(&processed_types);
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(bind_types), idx, str_key, value) {
				zval tmp = {}, string_wildcard = {};
				if (str_key) {
					ZVAL_STR(&tmp, str_key);
				} else {
					ZVAL_LONG(&tmp, idx);
				}

				if (Z_TYPE(tmp) == IS_LONG) {
					PHALCON_CONCAT_SV(&string_wildcard, ":", &tmp);
					phalcon_array_update(&processed_types, &string_wildcard, value, PH_COPY);
					zval_ptr_dtor(&string_wildcard);
				} else {
					phalcon_array_update(&processed_types, &tmp, value, PH_COPY);
				}
			} ZEND_HASH_FOREACH_END();

		} else {
			PHALCON_MM_ZVAL_COPY(&processed_types, &bind_types);
		}

		PHALCON_MM_CALL_METHOD(&result, &connection, "query", &sql, &processed, &processed_types);
		PHALCON_MM_ADD_ENTRY(&result);

		PHALCON_MM_CALL_METHOD(&row, &result, "fetch");
		PHALCON_MM_ADD_ENTRY(&row);

		phalcon_array_fetch_str(&rowcount, &row, SL("rowcount"), PH_NOISY|PH_READONLY);
	}
	i_rowcount    = phalcon_get_intval(&rowcount);
	tp            = ldiv(i_rowcount, i_limit);
	i_total_pages = tp.quot + (tp.rem ? 1 : 0);
	i_next        = (i_number_page < i_total_pages) ? (i_number_page + 1) : i_total_pages;

	object_init(&page);
	phalcon_update_property(&page, SL("items"),			   &items);
	phalcon_update_property_long(&page, SL("before"),      i_before);
	phalcon_update_property_long(&page, SL("first"),       1);
	phalcon_update_property_long(&page, SL("next"),        i_next);
	phalcon_update_property_long(&page, SL("last"),        i_total_pages);
	phalcon_update_property_long(&page, SL("current"),     i_number_page);
	phalcon_update_property_long(&page, SL("total_pages"), i_total_pages);
	phalcon_update_property_long(&page, SL("totalPages"), i_total_pages);
	phalcon_update_property_long(&page, SL("total_items"), i_rowcount);
	phalcon_update_property_long(&page, SL("totalItems"), i_rowcount);
	PHALCON_MM_ADD_ENTRY(&page);

	PHALCON_MM_ZVAL_STRING(&event_name, "pagination:afterGetPaginate");
	PHALCON_MM_CALL_METHOD(return_value, getThis(), "fireevent", &event_name, &page);

	if (Z_TYPE_P(return_value) < IS_ARRAY) {
		zval_ptr_dtor(return_value);
		RETURN_MM_CTOR(&page);
	}
	RETURN_MM();
}
