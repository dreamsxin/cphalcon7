
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2015 Phalcon Team (http://www.phalconphp.com)       |
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


#ifndef PHALCON_KERNEL_MATH_H
#define PHALCON_KERNEL_MATH_H

#include "php_phalcon.h"

double phalcon_floor(zval *op1);
double phalcon_ceil(zval *op1);
void phalcon_round(zval *return_value, zval *op1, zval *op2, zval *op3);
void phalcon_pow(zval *return_value, zval *op1, zval *op2);
long phalcon_mt_rand(long min, long max);

#define phalcon_pow_function(result, op1, op2) pow_function(result, op1, op2)

#endif
