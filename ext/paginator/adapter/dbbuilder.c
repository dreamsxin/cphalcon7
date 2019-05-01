
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

#include "paginator/adapter/dbbuilder.h"
#include "paginator/adapter.h"
#include "paginator/adapterinterface.h"
#include "paginator/exception.h"
#include "db/builderinterface.h"
#include "db.h"

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
 * Phalcon\Paginator\Adapter\DbBuilder
 *
 * Pagination using a PHQL query builder as source of data
 *
 *<code>
 *  $builder = Phalcon\Db\Builder::select('robots')
 *                   ->columns('id, name')
 *                   ->orderBy('name');
 *
 *  $paginator = new Phalcon\Paginator\Adapter\DbBuilder(array(
 *      "builder" => $builder,
 *      "limit"=> 20,
 *      "page" => 1
 *  ));
 *</code>
 */
zend_class_entry *phalcon_paginator_adapter_dbbuilder_ce;

PHP_METHOD(Phalcon_Paginator_Adapter_DbBuilder, __construct);
PHP_METHOD(Phalcon_Paginator_Adapter_DbBuilder, getPaginate);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_paginator_adapter_dbbuilder___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, config)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_paginator_adapter_dbbuilder_method_entry[] = {
	PHP_ME(Phalcon_Paginator_Adapter_DbBuilder, __construct, arginfo_phalcon_paginator_adapter_dbbuilder___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Paginator_Adapter_DbBuilder, getPaginate, arginfo_phalcon_paginator_adapterinterface_getpaginate, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Paginator\Adapter\DbBuilder initializer
 */
PHALCON_INIT_CLASS(Phalcon_Paginator_Adapter_DbBuilder){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Paginator\\Adapter, DbBuilder, paginator_adapter_dbbuilder, phalcon_paginator_adapter_ce, phalcon_paginator_adapter_dbbuilder_method_entry, 0);

	zend_declare_property_null(phalcon_paginator_adapter_dbbuilder_ce, SL("_builder"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_paginator_adapter_dbbuilder_ce, SL("_totalItems"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_paginator_adapter_dbbuilder_ce, SL("_limitRows"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_paginator_adapter_dbbuilder_ce, 1, phalcon_paginator_adapterinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Paginator\Adapter\DbBuilder
 *
 * @param array $config
 */
PHP_METHOD(Phalcon_Paginator_Adapter_DbBuilder, __construct){

	zval *config, builder = {}, limit = {}, page = {}, total_items = {};
	long int i_limit;

	phalcon_fetch_params(0, 1, 0, &config);

	if (!phalcon_array_isset_fetch_str(&builder, config, SL("builder"), PH_READONLY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_paginator_exception_ce, "Parameter 'builder' is required");
		return;
	}

	PHALCON_VERIFY_INTERFACE_EX(&builder, phalcon_db_builderinterface_ce, phalcon_paginator_exception_ce);

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
 * Returns a slice of the resultset to show in the pagination
 *
 * @return \stdClass
 */
PHP_METHOD(Phalcon_Paginator_Adapter_DbBuilder, getPaginate){

	zval event_name = {}, builder = {}, limit = {}, number_page = {}, number = {}, fetch_mode = {}, items = {};
	zval row = {}, rowcount = {}, result = {}, page = {};
	ldiv_t tp;
	long int i_limit, i_number_page, i_number, i_before, i_rowcount;
	long int i_total_pages, i_next;

	PHALCON_MM_INIT();

	PHALCON_MM_ZVAL_STRING(&event_name, "pagination:beforeGetPaginate");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);

	phalcon_read_property(&builder, getThis(), SL("_builder"), PH_READONLY);

	phalcon_read_property(&limit, getThis(), SL("_limitRows"), PH_READONLY);
	phalcon_read_property(&number_page, getThis(), SL("_page"), PH_READONLY);

	phalcon_read_property(&rowcount, getThis(), SL("_totalItems"), PH_READONLY);
	if (Z_TYPE(rowcount) != IS_LONG) {
		PHALCON_MM_CALL_METHOD(&result, &builder, "count");
		PHALCON_MM_ADD_ENTRY(&result);

		PHALCON_MM_CALL_METHOD(&row, &result, "fetch");
		PHALCON_MM_ADD_ENTRY(&row);

		phalcon_array_fetch_str(&rowcount, &row, SL("rowcount"), PH_NOISY|PH_READONLY);
	}

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

	PHALCON_MM_CALL_METHOD(&result, &builder, "execute");
	PHALCON_MM_ADD_ENTRY(&result);

	phalcon_get_class_constant(&fetch_mode, phalcon_db_ce, SL("FETCH_OBJ"));
	PHALCON_MM_ADD_ENTRY(&fetch_mode);
	PHALCON_MM_CALL_METHOD(&items, &result, "fetchall", &fetch_mode);
	PHALCON_MM_ADD_ENTRY(&items);

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
