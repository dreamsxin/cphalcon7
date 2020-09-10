
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

#ifndef PHALCON_NUM_H
#define PHALCON_NUM_H

#include "php_phalcon.h"

typedef double (*num_func_t)(double val1, double val2);
typedef double (*num_func_t_one)(double val);

static zend_always_inline double num_operator_add(double val1, double val2) {
    return val1 + val2;
}

static zend_always_inline double num_operator_sub(double val1, double val2) {
    return val1 - val2;
}

static zend_always_inline double num_operator_mult(double val1, double val2) {
    return val1 * val2;
}

static zend_always_inline double num_operator_div(double val1, double val2) {
    return val1 / val2;
}

static zend_always_inline double num_max(double val1, double val2) {
    return val1 > val2 ? val1 : val2;
}

static zend_always_inline double num_min(double val1, double val2) {
    return val1 > val2 ? val2 : val1;
}

extern zend_class_entry *phalcon_num_ce;

PHALCON_INIT_CLASS(Phalcon_Num);

#endif	/* PHALCON_NUM_H */
