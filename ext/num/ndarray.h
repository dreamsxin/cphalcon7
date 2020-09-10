
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
  | Author:  Astraeux  <astraeux@gmail.com>                                |
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#ifndef PHALCON_NUM_NDARRAY_H
#define PHALCON_NUM_NDARRAY_H

#include "php_phalcon.h"

#define NUM_NDARRAY_PROPERT_DATA          "_data"
#define NUM_NDARRAY_PROPERT_SHAPE         "_shape"

int num_calc_shape(zval *data, zval *shape, zend_long dimension);

int num_ndarray_recursive(zval *data1, zval *data2, num_func_t num_func);
int num_ndarray_compare_recursive(zval *ret, zval *data, num_func_t num_func);
int num_ndarray_self_recursive(zval *data, num_func_t_one num_func);
int num_ndarray_arithmetic_recursive(zval *data1, zval *data2, num_func_t num_func);
zend_string *num_ndarray_to_string(zval *data, int level);


extern zend_class_entry *phalcon_num_ndarray_ce;

PHALCON_INIT_CLASS(Phalcon_Num_Ndarray);

#endif