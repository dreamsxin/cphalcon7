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

#include "paginator/adapter.h"
#include "paginator/adapterinterface.h"
#include "paginator/exception.h"
#include "di/injectable.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"

/**
 * Phalcon\Paginator\Adapter
 *
 * Base class for Phalcon\Paginator adapters
 */
zend_class_entry *phalcon_paginator_adapter_ce;

PHP_METHOD(Phalcon_Paginator_Adapter, setCurrentPage);
PHP_METHOD(Phalcon_Paginator_Adapter, getCurrentPage);
PHP_METHOD(Phalcon_Paginator_Adapter, setLimit);
PHP_METHOD(Phalcon_Paginator_Adapter, getLimit);

static const zend_function_entry phalcon_paginator_adapter_method_entry[] = {
	PHP_ME(Phalcon_Paginator_Adapter, setCurrentPage, arginfo_phalcon_paginator_adapterinterface_setcurrentpage, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Paginator_Adapter, getCurrentPage, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Paginator_Adapter, setLimit, arginfo_phalcon_paginator_adapterinterface_setlimit, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Paginator_Adapter, getLimit, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Paginator\Adapter initializer
 */
PHALCON_INIT_CLASS(Phalcon_Paginator_Adapter){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Paginator, Adapter, paginator_adapter, phalcon_di_injectable_ce, phalcon_paginator_adapter_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_paginator_adapter_ce, SL("_limitRows"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_paginator_adapter_ce, SL("_page"), 1, ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_paginator_adapter_ce, 1, phalcon_paginator_adapterinterface_ce);

	return SUCCESS;
}


/**
 * Set current page number
 *
 * @param int $page
 */
PHP_METHOD(Phalcon_Paginator_Adapter, setCurrentPage){

	zval *current_page;

	phalcon_fetch_params(0, 1, 0, &current_page);

	phalcon_update_property(getThis(), SL("_page"), current_page);
	RETURN_THIS();
}

/**
 * Get current page number
 *
 * @param int $page
 */
PHP_METHOD(Phalcon_Paginator_Adapter, getCurrentPage){

	RETURN_MEMBER(getThis(), "_page");
}

/**
 * Set current rows limit
 *
 * @param int $limit
 *
 * @return Phalcon\Paginator\Adapter
 */
PHP_METHOD(Phalcon_Paginator_Adapter, setLimit){

	zval *current_limit;

	phalcon_fetch_params(0, 1, 0, &current_limit);

	phalcon_update_property(getThis(), SL("_limitRows"), current_limit);
	RETURN_THIS();
}

/**
 * Get current rows limit
 *
 * @return int $limit
 */
PHP_METHOD(Phalcon_Paginator_Adapter, getLimit){

	RETURN_MEMBER(getThis(), "_limitRows");
}
