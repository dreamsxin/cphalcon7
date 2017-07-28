
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

#ifndef PHMATHON_MATH_NUM_ARRAY_H
#define PHMATHON_MATH_NUM_ARRAY_H

#include "php_phalcon.h"

typedef struct _phalcon_math_num_array_object {
	void* data;
	zend_object std;
} phalcon_math_num_array_object;

static inline phalcon_math_num_array_object *phalcon_math_num_array_object_from_obj(zend_object *obj) {
	return (phalcon_math_num_array_object*)((char*)(obj) - XtOffsetOf(phalcon_math_num_array_object, std));
}

static inline phalcon_math_num_array_object *phalcon_math_num_array_object_from_ctx(struct phalcon_math_num_array_context *ctx) {
	return (phalcon_math_num_array_object*)((char*)(ctx) - XtOffsetOf(phalcon_math_num_array_object, ctx));
}

zval* phalcon_math_num_array_calc_shape(zval* data, zval* shape, zend_long dimension);
zend_string *phalcon_math_num_array_to_string(zval* data, zend_long level);

int phalcon_math_num_array_recursive(zval* data1, zval* data2, phalcon_math_num_func_two_t num_func);
int phalcon_math_num_array_compare_recursive(zval* ret, zval* data, phalcon_math_num_func_two_t num_func);
int phalcon_math_num_array_self_recursive(zval* data, phalcon_math_num_func_one_t num_func);
int phalcon_math_num_array_arithmetic_recursive(zval* data1, zval* data2, phalcon_math_num_func_two_t num_func);

extern zend_class_entry *phalcon_math_num_array_ce;

PHMATHON_INIT_CLASS(Phalcon_Math_Num_Array);

#endif /* PHMATHON_MATH_NUM_ARRAY_H */
