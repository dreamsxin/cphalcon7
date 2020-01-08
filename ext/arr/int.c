
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

#include "arr/int.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/concat.h"
#include "kernel/object.h"
#include "kernel/exception.h"

/**
 * Phalcon\Arr\Int
 *
 *<code>
 *
 *	$ints = new Phalcon\Arr\Int;
 *	$int->set(0, 1);
 *
 *</code>
 */
zend_class_entry *phalcon_arr_int_ce;

PHP_METHOD(Phalcon_Arr_Int, __construct);
PHP_METHOD(Phalcon_Arr_Int, set);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_int___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_int_set, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_arr_int_method_entry[] = {
	PHP_ME(Phalcon_Arr_Int, __construct, arginfo_phalcon_arr_int___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Arr_Int, set, arginfo_phalcon_arr_int_set, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Arr\Int initializer
 */
PHALCON_INIT_CLASS(Phalcon_Arr_Int){

	PHALCON_REGISTER_CLASS(Phalcon\\Arr, Int, arr_int, phalcon_arr_int_method_entry, 0);

	return SUCCESS;
}

/**
 * Phalcon\Arr\Int constructor
 */
PHP_METHOD(Phalcon_Arr_Int, __construct){

}

/**
 *  Sets value
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Arr_Int, set){

}
