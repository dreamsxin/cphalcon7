
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
PHP_METHOD(Phalcon_Arr_Int, get);
PHP_METHOD(Phalcon_Arr_Int, add);
PHP_METHOD(Phalcon_Arr_Int, insert);
PHP_METHOD(Phalcon_Arr_Int, sort);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_int___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_int_set, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_int_get, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_arr_int_add, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_arr_int_method_entry[] = {
	PHP_ME(Phalcon_Arr_Int, __construct, arginfo_phalcon_arr_int___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Arr_Int, set, arginfo_phalcon_arr_int_set, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Arr_Int, get, arginfo_phalcon_arr_int_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Arr_Int, add, arginfo_phalcon_arr_int_add, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_object_handlers phalcon_arr_int_object_handlers;
zend_object* phalcon_arr_int_object_create_handler(zend_class_entry *ce)
{
	phalcon_arr_int_object *intern = ecalloc(1, sizeof(phalcon_arr_int_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_arr_int_object_handlers;

	return &intern->std;
}

void phalcon_arr_int_object_free_handler(zend_object *object)
{
	phalcon_arr_int_object *intern = phalcon_arr_int_object_from_obj(object);

	efree(intern->value);
	zend_object_std_dtor(object);
}

/**
 * Phalcon\Arr\Int initializer
 */
PHALCON_INIT_CLASS(Phalcon_Arr_Int){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Arr, Int, arr_int, phalcon_arr_int_method_entry, 0);

	return SUCCESS;
}

/**
 * Phalcon\Arr\Int constructor
 */
PHP_METHOD(Phalcon_Arr_Int, __construct){

	zval *size = NULL;
	phalcon_arr_int_object *intern;

	phalcon_fetch_params(0, 0, 1, &size);

	intern = phalcon_arr_int_object_from_obj(Z_OBJ_P(getThis()));

	if (size && Z_LVAL_P(size) > 0) {
		intern->len = Z_LVAL_P(size);
	} else {
		intern->len = 1;
	}
	intern->total = intern->len > 100 ? intern->len : 100;
	intern->value = ecalloc(intern->len, sizeof(zend_long));
}

/**
 *  Sets value
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Arr_Int, set){

	zval *index, *value;
	phalcon_arr_int_object *intern;

	phalcon_fetch_params(0, 2, 0, &index, &value);

	intern = phalcon_arr_int_object_from_obj(Z_OBJ_P(getThis()));

	if (Z_LVAL_P(index) < 0 || Z_LVAL_P(index) >= intern->len) {
		RETURN_FALSE;
	}
	intern->value[Z_LVAL_P(index)] = Z_LVAL_P(value);
	RETURN_TRUE;
}

/**
 *  Gets value
 *
 * @return int
 */
PHP_METHOD(Phalcon_Arr_Int, get){

	zval *index;
	phalcon_arr_int_object *intern;

	phalcon_fetch_params(0, 1, 0, &index);

	intern = phalcon_arr_int_object_from_obj(Z_OBJ_P(getThis()));

	if (Z_LVAL_P(index) < 0 || Z_LVAL_P(index) >= intern->len) {
		RETURN_FALSE;
	}
	RETURN_LONG(intern->value[Z_LVAL_P(index)]);
}

/**
 *  Add value
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Arr_Int, add){

	zval *value;
	phalcon_arr_int_object *intern;

	phalcon_fetch_params(0, 1, 0, &value);

	intern = phalcon_arr_int_object_from_obj(Z_OBJ_P(getThis()));

	if (intern->len >= intern->total) {
		erealloc(intern->value, (intern->total+100)*sizeof(zend_long));
		intern->total += 100;
	}
	intern->value[intern->len] = Z_LVAL_P(value);
	intern->len += 1;
	RETURN_TRUE;
}
