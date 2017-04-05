
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

#include "paginator/adapter/model.h"
#include "paginator/adapterinterface.h"
#include "paginator/exception.h"

#include <math.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"

/**
 * Phalcon\Paginator\Adapter\Model
 *
 * This adapter allows to paginate data using a Phalcon\Mvc\Model resultset as base
 */
zend_class_entry *phalcon_paginator_adapter_model_ce;

PHP_METHOD(Phalcon_Paginator_Adapter_Model, __construct);
PHP_METHOD(Phalcon_Paginator_Adapter_Model, setCurrentPage);
PHP_METHOD(Phalcon_Paginator_Adapter_Model, getPaginate);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_paginator_adapter_model___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, config)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_paginator_adapter_model_method_entry[] = {
	PHP_ME(Phalcon_Paginator_Adapter_Model, __construct, arginfo_phalcon_paginator_adapter_model___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Paginator_Adapter_Model, setCurrentPage, arginfo_phalcon_paginator_adapterinterface_setcurrentpage, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Paginator_Adapter_Model, getPaginate, arginfo_phalcon_paginator_adapterinterface_getpaginate, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Paginator\Adapter\Model initializer
 */
PHALCON_INIT_CLASS(Phalcon_Paginator_Adapter_Model){

	PHALCON_REGISTER_CLASS(Phalcon\\Paginator\\Adapter, Model, paginator_adapter_model, phalcon_paginator_adapter_model_method_entry, 0);

	zend_declare_property_null(phalcon_paginator_adapter_model_ce, SL("_limitRows"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_paginator_adapter_model_ce, SL("_config"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_paginator_adapter_model_ce, SL("_page"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_paginator_adapter_model_ce, 1, phalcon_paginator_adapterinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Paginator\Adapter\Model constructor
 *
 * @param array $config
 */
PHP_METHOD(Phalcon_Paginator_Adapter_Model, __construct)
{
	zval *config, limit = {}, page = {};

	phalcon_fetch_params(0, 1, 0, &config);

	phalcon_update_property(getThis(), SL("_config"), config);

	if (phalcon_array_isset_fetch_str(&limit, config, SL("limit"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_limitRows"), &limit);
	}

	if (phalcon_array_isset_fetch_str(&page, config, SL("page"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_page"), &page);
	}
}

/**
 * Set the current page number
 *
 * @param int $page
 */
PHP_METHOD(Phalcon_Paginator_Adapter_Model, setCurrentPage){

	zval *page;

	phalcon_fetch_params(0, 1, 0, &page);

	phalcon_update_property(getThis(), SL("_page"), page);
	RETURN_THIS();
}

/**
 * Returns a slice of the resultset to show in the pagination
 *
 * @return \stdClass
 */
PHP_METHOD(Phalcon_Paginator_Adapter_Model, getPaginate){

	zval show = {}, config = {}, items = {}, page_number = {}, rowcount = {}, page = {}, last_show_page = {}, start = {}, possible_pages = {}, total_pages = {};
	zval page_items = {}, maximum_pages = {}, next = {}, additional_page = {}, before = {}, remainder = {}, pages_total = {};
	long int i, i_show;

	phalcon_read_property(&show, getThis(), SL("_limitRows"), PH_READONLY);
	phalcon_read_property(&config, getThis(), SL("_config"), PH_READONLY);
	phalcon_read_property(&page_number, getThis(), SL("_page"), PH_READONLY);

	i_show = phalcon_get_intval(&show);

	phalcon_array_fetch_str(&items, &config, SL("data"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(page_number) == IS_NULL || PHALCON_LT(&show, &PHALCON_GLOBAL(z_zero))) {
		ZVAL_COPY_VALUE(&page_number, &PHALCON_GLOBAL(z_one));
	}

	phalcon_fast_count(&rowcount, &items);

	object_init(&page);

	phalcon_sub_function(&last_show_page, &page_number, &PHALCON_GLOBAL(z_one));

	mul_function(&start, &show, &last_show_page);
	phalcon_div_function(&possible_pages, &rowcount, &show);

	if (unlikely(Z_TYPE(possible_pages)) != IS_DOUBLE) {
		convert_to_double(&possible_pages);
	}

	ZVAL_LONG(&total_pages, (long int)ceil(Z_DVAL(possible_pages)));
	if (Z_TYPE(items) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_paginator_exception_ce, "Invalid data for paginator");
		return;
	}

	array_init(&page_items);
	if (PHALCON_GT(&rowcount, &PHALCON_GLOBAL(z_zero))) {
		/**
		 * Seek to the desired position
		 */
		if (PHALCON_LT(&start, &rowcount)) {
			PHALCON_CALL_METHOD(NULL, &items, "seek", &start);
		} else {
			PHALCON_CALL_METHOD(NULL, &items, "rewind");
			ZVAL_COPY_VALUE(&page_number, &PHALCON_GLOBAL(z_one));
			ZVAL_COPY_VALUE(&start, &PHALCON_GLOBAL(z_zero));
		}

		/**
		 * The record must be iterable
		 */
		for (i=1; ; ++i) {
			zval valid = {}, current = {};
			PHALCON_CALL_METHOD(&valid, &items, "valid");
			if (!PHALCON_IS_NOT_FALSE(&valid)) {
				break;
			}

			PHALCON_CALL_METHOD(&current, &items, "current");
			phalcon_array_append(&page_items, &current, PH_COPY);

			if (i >= i_show) {
				break;
			}
		}
	}

	phalcon_update_property(&page, SL("items"), &page_items);

	phalcon_add_function(&maximum_pages, &start, &show);
	if (PHALCON_LT(&maximum_pages, &rowcount)) {
		phalcon_add_function(&next, &page_number, &PHALCON_GLOBAL(z_one));
	} else if (PHALCON_IS_EQUAL(&maximum_pages, &rowcount)) {
			ZVAL_COPY_VALUE(&next, &rowcount);
	} else {
		phalcon_div_function(&possible_pages, &rowcount, &show);

		phalcon_add_function(&additional_page, &possible_pages, &PHALCON_GLOBAL(z_one));

		ZVAL_LONG(&next, phalcon_get_intval(&additional_page));
	}

	if (PHALCON_GT(&next, &total_pages)) {
		ZVAL_COPY_VALUE(&next, &total_pages);
	}

	phalcon_update_property(&page, SL("next"), &next);
	if (PHALCON_GT(&page_number, &PHALCON_GLOBAL(z_one))) {
		phalcon_sub_function(&before, &page_number, &PHALCON_GLOBAL(z_one));
	} else {
		ZVAL_COPY_VALUE(&before, &PHALCON_GLOBAL(z_one));
	}

	phalcon_update_property(&page, SL("first"), &PHALCON_GLOBAL(z_one));
	phalcon_update_property(&page, SL("before"), &before);
	phalcon_update_property(&page, SL("current"), &page_number);

	mod_function(&remainder, &rowcount, &show);

	phalcon_div_function(&possible_pages, &rowcount, &show);
	if (!PHALCON_IS_LONG(&remainder, 0)) {
		phalcon_add_function(&next, &possible_pages, &PHALCON_GLOBAL(z_one));

		ZVAL_LONG(&pages_total, phalcon_get_intval(&next));
	} else {
		ZVAL_COPY_VALUE(&pages_total, &possible_pages);
	}

	phalcon_update_property(&page, SL("last"), &pages_total);
	phalcon_update_property(&page, SL("total_pages"), &pages_total);
	phalcon_update_property(&page, SL("total_items"), &rowcount);

	RETURN_CTOR(&page);
}
