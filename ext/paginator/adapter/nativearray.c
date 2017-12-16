
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

#include "paginator/adapter/nativearray.h"
#include "paginator/adapter.h"
#include "paginator/adapterinterface.h"
#include "paginator/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"

/**
 * Phalcon\Paginator\Adapter\NativeArray
 *
 * Pagination using a PHP array as source of data
 *
 *<code>
 *	$paginator = new \Phalcon\Paginator\Adapter\Model(
 *		array(
 *			"data"  => array(
 *				array('id' => 1, 'name' => 'Artichoke'),
 *				array('id' => 2, 'name' => 'Carrots'),
 *				array('id' => 3, 'name' => 'Beet'),
 *				array('id' => 4, 'name' => 'Lettuce'),
 *				array('id' => 5, 'name' => '')
 *			),
 *			"limit" => 2,
 *			"page"  => $currentPage
 *		)
 *	);
 *</code>
 *
 */
zend_class_entry *phalcon_paginator_adapter_nativearray_ce;

PHP_METHOD(Phalcon_Paginator_Adapter_NativeArray, __construct);
PHP_METHOD(Phalcon_Paginator_Adapter_NativeArray, getPaginate);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_paginator_adapter_nativearray___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, config)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_paginator_adapter_nativearray_method_entry[] = {
	PHP_ME(Phalcon_Paginator_Adapter_NativeArray, __construct, arginfo_phalcon_paginator_adapter_nativearray___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Paginator_Adapter_NativeArray, getPaginate, arginfo_phalcon_paginator_adapterinterface_getpaginate, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Paginator\Adapter\NativeArray initializer
 */
PHALCON_INIT_CLASS(Phalcon_Paginator_Adapter_NativeArray){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Paginator\\Adapter, NativeArray, paginator_adapter_nativearray, phalcon_paginator_adapter_ce, phalcon_paginator_adapter_nativearray_method_entry, 0);

	zend_declare_property_null(phalcon_paginator_adapter_nativearray_ce, SL("_data"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_paginator_adapter_nativearray_ce, 1, phalcon_paginator_adapterinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Paginator\Adapter\NativeArray constructor
 *
 * @param array $config
 */
PHP_METHOD(Phalcon_Paginator_Adapter_NativeArray, __construct){

	zval *config, limit = {}, page = {}, data = {};

	phalcon_fetch_params(0, 1, 0, &config);

	if (phalcon_array_isset_fetch_str(&limit, config, SL("limit"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_limitRows"), &limit);
	}

	if (phalcon_array_isset_fetch_str(&page, config, SL("page"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_page"), &page);
	}

	if (!phalcon_array_isset_fetch_str(&data, config, SL("data"), PH_READONLY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_paginator_exception_ce, "Parameter 'data' is required");
		return;
	} else if (Z_TYPE(data) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_paginator_exception_ce, "'data' should be an array");
		return;
	}

	phalcon_update_property(getThis(), SL("_data"), &data);
}

/**
 * Returns a slice of the resultset to show in the pagination
 *
 * @return \stdClass
 */
PHP_METHOD(Phalcon_Paginator_Adapter_NativeArray, getPaginate){

	zval event_name = {}, items = {}, limit = {}, number_page = {}, start = {}, lim = {}, slice = {}, page = {};
	long int i_limit, i_number_page, i_number, i_before, i_rowcount;
	long int i_total_pages, i_next;
	ldiv_t tp;

	ZVAL_STRING(&event_name, "pagination:beforeGetPaginate");
	PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
	zval_ptr_dtor(&event_name);

	phalcon_read_property(&items, getThis(), SL("_data"), PH_NOISY|PH_READONLY);
	if (UNEXPECTED(Z_TYPE(items) != IS_ARRAY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_paginator_exception_ce, "Invalid data for paginator");
		return;
	}

	phalcon_read_property(&limit, getThis(), SL("_limitRows"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&number_page, getThis(), SL("_page"), PH_NOISY|PH_READONLY);
	i_limit       = phalcon_get_intval(&limit);
	i_number_page = phalcon_get_intval(&number_page);

	if (i_limit < 1) {
		/* This should never happen unless someone deliberately modified the properties of the object */
		i_limit = 10;
	}

	if (!i_number_page) {
		i_number_page = 1;
	}

	i_number      = (i_number_page - 1) * i_limit;
	i_before      = (i_number_page == 1) ? 1 : (i_number_page - 1);
	i_rowcount    = zend_hash_num_elements(Z_ARRVAL(items));
	tp            = ldiv(i_rowcount, i_limit);
	i_total_pages = tp.quot + (tp.rem ? 1 : 0);
	i_next        = (i_number_page < i_total_pages) ? (i_number_page + 1) : i_total_pages;

	ZVAL_LONG(&start, i_number);
	ZVAL_LONG(&lim, i_limit);

	PHALCON_CALL_FUNCTION(&slice, "array_slice", &items, &start, &lim);

	object_init(&page);
	phalcon_update_property(&page, SL("items"),            &slice);
	zval_ptr_dtor(&slice);
	phalcon_update_property_long(&page, SL("before"),      i_before);
	phalcon_update_property_long(&page, SL("first"),       1);
	phalcon_update_property_long(&page, SL("next"),        i_next);
	phalcon_update_property_long(&page, SL("last"),        i_total_pages);
	phalcon_update_property_long(&page, SL("current"),     i_number_page);
	phalcon_update_property_long(&page, SL("total_pages"), i_total_pages);
	phalcon_update_property_long(&page, SL("total_items"), i_rowcount);

	ZVAL_STRING(&event_name, "pagination:afterGetPaginate");
	PHALCON_CALL_METHOD(return_value, getThis(), "fireevent", &event_name, &page);
	zval_ptr_dtor(&event_name);

	if (Z_TYPE_P(return_value) >= IS_ARRAY) {
		zval_ptr_dtor(&page);
	} else {
		zval_ptr_dtor(return_value);
		RETURN_NCTOR(&page);
	}
}
