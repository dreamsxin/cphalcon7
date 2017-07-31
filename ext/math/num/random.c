
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

#include "math/num/random.h"

#include "kernel/main.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/operators.h"

#include "internal/arginfo.h"

/**
 * Phalcon\Math\Num\Random
 *
 */
zend_class_entry *phalcon_math_num_random_ce;

PHP_METHOD(Phalcon_Math_Num_Random, rand);
PHP_METHOD(Phalcon_Math_Num_Random, randn);
PHP_METHOD(Phalcon_Math_Num_Random, randint);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_random_rand, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, d0, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, d1, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_random_randn, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, d0, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, d1, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_random_randn, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, low, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, hight, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_math_num_random_method_entry[] = {
	PHP_ME(Phalcon_Math_Num_Random, rand, arginfo_phalcon_math_num_random_rand, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num_Random, randn, arginfo_phalcon_math_num_random_randn, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num_Random, randint, arginfo_phalcon_math_num_random_randint, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};


/**
 * Phalcon\Math\Num\Random initializer
 */
PHALCON_INIT_CLASS(Phalcon_Math_Num_Random){

	PHALCON_REGISTER_CLASS(Phalcon\\Math\\Num, Random, math_num_random, phalcon_math_num_random_method_entry, 0);

	return SUCCESS;
}

/**
 * Random values in a given shape
 *
 * @param int $d0
 * @param int $d1
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num_Random, rand){


}

/**
 * Return a sample (or samples) from the “standard normal” distribution
 *
 * @param int $d0
 * @param int $d1
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num_Random, randn){


}

/**
 * Return random integers from low (inclusive) to high (exclusive)
 *
 * @param int $low
 * @param int $hight
 * @param int $size
 * @param int $type
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num_Random, randint){


}
