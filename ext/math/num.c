
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

#include "math/num.h"
#include "math/ndarray.h"

#include "kernel/main.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/operators.h"

#include "internal/arginfo.h"

/**
 * Phalcon\Math\Num
 *
 */
zend_class_entry *phalcon_math_num_ce;

PHP_METHOD(Phalcon_Math_Num, array);
PHP_METHOD(Phalcon_Math_Num, resize);
PHP_METHOD(Phalcon_Math_Num, arange);
PHP_METHOD(Phalcon_Math_Num, amin);
PHP_METHOD(Phalcon_Math_Num, amax);
PHP_METHOD(Phalcon_Math_Num, unique);
PHP_METHOD(Phalcon_Math_Num, power);
PHP_METHOD(Phalcon_Math_Num, square);
PHP_METHOD(Phalcon_Math_Num, sqrt);
PHP_METHOD(Phalcon_Math_Num, exp);
PHP_METHOD(Phalcon_Math_Num, log);
PHP_METHOD(Phalcon_Math_Num, log10);
PHP_METHOD(Phalcon_Math_Num, sin);
PHP_METHOD(Phalcon_Math_Num, cos);
PHP_METHOD(Phalcon_Math_Num, tan);
PHP_METHOD(Phalcon_Math_Num, ceil);
PHP_METHOD(Phalcon_Math_Num, floor);
PHP_METHOD(Phalcon_Math_Num, fabs);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_array, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, data, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_resize, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, rows, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, cols, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_arange, 0, 0, 1)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, stop)
	ZEND_ARG_INFO(0, step)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_amin, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_TYPE_INFO(0, axis, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_amax, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_TYPE_INFO(0, axis, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_unique, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_TYPE_INFO(0, index, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_flib, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_TYPE_INFO(0, axis, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_power, 0, 0, 2)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, exponent)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_square, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_sqrt, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_exp, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_log, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_log10, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_sin, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_cos, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_tan, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_ceil, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_floor, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_fabs, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_math_num_method_entry[] = {
	PHP_ME(Phalcon_Math_Num, array, arginfo_phalcon_math_num_array, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num, resize, arginfo_phalcon_math_num_resize, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num, arange, arginfo_phalcon_math_num_arange, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num, amin, arginfo_phalcon_math_num_amin, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num, amax, arginfo_phalcon_math_num_amax, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num, unique, arginfo_phalcon_math_num_unique, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num, flip, arginfo_phalcon_math_num_flip, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num, power, arginfo_phalcon_math_num_power, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num, square, arginfo_phalcon_math_num_square, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num, sqrt, arginfo_phalcon_math_num_sqrt, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num, exp, arginfo_phalcon_math_num_exp, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num, log, arginfo_phalcon_math_num_log, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num, log10, arginfo_phalcon_math_num_log10, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num, sin, arginfo_phalcon_math_num_sin, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num, cos, arginfo_phalcon_math_num_cos, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num, tan, arginfo_phalcon_math_num_tan, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num, ceil, arginfo_phalcon_math_num_ceil, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num, floor, arginfo_phalcon_math_num_floor, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num, fabs, arginfo_phalcon_math_num_fabs, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};


/**
 * Phalcon\Math\Num initializer
 */
PHALCON_INIT_CLASS(Phalcon_Math_Num){

	PHALCON_REGISTER_CLASS(Phalcon\\Math, Num, math_num, phalcon_math_num_method_entry, 0);

	return SUCCESS;
}

/**
 * Creates a Phalcon\Math\Num\Array
 *
 * @param array $data
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num, array){

	zval *data;

	phalcon_fetch_params(0, 1, 0, &data);

}

/**
 * Return a new array with the specified shape
 *
 * @param int $type
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num, resize){


}

/**
 * Return evenly spaced values within a given interval
 *
 * @param int|float $start
 * @param int|float $end
 * @param int|float $step
 * @param int $type
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num, arange){


}

/**
 * Return the minimum of an array or minimum along an axis
 *
 * @param mixed $data
 * @param int $axis
 * @return Phalcon\Math\Num\Array|scalar
 */
PHP_METHOD(Phalcon_Math_Num, amin){

	PHALCON_CALL_CE_STATIC(&valid, phalcon_math_num_array_ce, "valid", data);
}

/**
 * Return the maximum of an array or maximum along an axis
 *
 * @param mixed $data
 * @param int $axis
 * @return Phalcon\Math\Num\Array|scalar
 */
PHP_METHOD(Phalcon_Math_Num, amax){

	PHALCON_CALL_CE_STATIC(&valid, phalcon_math_num_array_ce, "valid", data);
}

/**
 * Return the maximum of an array or maximum along an axis
 *
 * @param mixed $data
 * @param boolean $index
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num, unique){

	PHALCON_CALL_CE_STATIC(&valid, phalcon_math_num_array_ce, "valid", data);
}

/**
 * First array elements raised to powers from second array, element-wise
 *
 * @param mixed $data
 * @param mixed $exponent
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num, power){

	PHALCON_CALL_CE_STATIC(&valid, phalcon_math_num_array_ce, "valid", data);
	PHALCON_CALL_CE_STATIC(&valid, phalcon_math_num_array_ce, "valid", exponent);
}

/**
 * Return the element-wise square of the input
 *
 * @param mixed $data
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num, square){

	PHALCON_CALL_CE_STATIC(&valid, phalcon_math_num_array_ce, "valid", data);
}

/**
 * Return the positive square-root of an array, element-wise
 *
 * @param mixed $data
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num, sqrt){

	PHALCON_CALL_CE_STATIC(&valid, phalcon_math_num_array_ce, "valid", data);
}

/**
 * Calculate the exponential of all elements in the input array
 *
 * @param mixed $data
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num, exp){

	PHALCON_CALL_CE_STATIC(&valid, phalcon_math_num_array_ce, "valid", data);
}

/**
 * Natural logarithm, element-wise
 *
 * @param mixed $data
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num, log){

	PHALCON_CALL_CE_STATIC(&valid, phalcon_math_num_array_ce, "valid", data);
}

/**
 * Return the base 10 logarithm of the input array, element-wise
 *
 * @param mixed $data
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num, log10){

	PHALCON_CALL_CE_STATIC(&valid, phalcon_math_num_array_ce, "valid", data);
}

/**
 * Trigonometric sine, element-wise
 *
 * @param mixed $data
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num, sin){

	PHALCON_CALL_CE_STATIC(&valid, phalcon_math_num_array_ce, "valid", data);
}

/**
 * Cosine element-wise
 *
 * @param mixed $data
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num, cos){

	PHALCON_CALL_CE_STATIC(&valid, phalcon_math_num_array_ce, "valid", data);
}

/**
 * Compute tangent element-wise
 *
 * @param mixed $data
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num, tan){

	PHALCON_CALL_CE_STATIC(&valid, phalcon_math_num_array_ce, "valid", data);
}

/**
 * Return the ceiling of the input, element-wise
 *
 * @param mixed $data
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num, ceil){

	PHALCON_CALL_CE_STATIC(&valid, phalcon_math_num_array_ce, "valid", data);
}

/**
 * Return the floor of the input, element-wise
 *
 * @param mixed $data
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num, floor){

	PHALCON_CALL_CE_STATIC(&valid, phalcon_math_num_array_ce, "valid", data);
}

/**
 * Compute the absolute values element-wise
 *
 * @param mixed $data
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num, fabs){

	PHALCON_CALL_CE_STATIC(&valid, phalcon_math_num_array_ce, "valid", data);
}
