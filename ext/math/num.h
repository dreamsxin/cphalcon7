
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

#ifndef PHMATHON_MATH_NUM_H
#define PHMATHON_MATH_NUM_H

#include "php_phalcon.h"

#define max(a,b) ( ((a)>(b)) ? (a):(b) )
#define min(a,b) ( ((a)>(b)) ? (b):(a) )

typedef double (*phalcon_math_num_func_two_t)(double val1, double val2);
typedef double (*phalcon_math_num_func_one_t)(double val);

static zend_always_inline double phalcon_math_num_operator_add(double val1, double val2) {
    return val1 + val2;
}

static zend_always_inline double phalcon_math_num_operator_sub(double val1, double val2) {
    return val1 - val2;
}

static zend_always_inline double phalcon_math_num_operator_mult(double val1, double val2) {
    return val1 * val2;
}

static zend_always_inline double phalcon_math_num_operator_div(double val1, double val2) {
    return val1 / val2;
}

static zend_always_inline double phalcon_math_num_max(double val1, double val2) {
    return val1 > val2 ? val1 : val2;
}

static zend_always_inline double phalcon_math_num_min(double val1, double val2) {
    return val1 > val2 ? val2 : val1;
}

extern zend_class_entry *phalcon_math_num_ce;

PHMATHON_INIT_CLASS(Phalcon_Math_Num);

#endif /* PHMATHON_MATH_NUM_H */
