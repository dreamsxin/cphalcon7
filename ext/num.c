
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

#include "num.h"
#include "num/ndarray.h"
#include "queue/../exception.h"

#include "kernel/main.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/object.h"

zend_class_entry *phalcon_num_ce;

PHP_METHOD(Phalcon_Num, array);
PHP_METHOD(Phalcon_Num, arange);
PHP_METHOD(Phalcon_Num, amin);
PHP_METHOD(Phalcon_Num, amax);
PHP_METHOD(Phalcon_Num, power);
PHP_METHOD(Phalcon_Num, square);
PHP_METHOD(Phalcon_Num, sqrt);
PHP_METHOD(Phalcon_Num, exp);
PHP_METHOD(Phalcon_Num, log);
PHP_METHOD(Phalcon_Num, log10);
PHP_METHOD(Phalcon_Num, sin);
PHP_METHOD(Phalcon_Num, cos);
PHP_METHOD(Phalcon_Num, tan);
PHP_METHOD(Phalcon_Num, ceil);
PHP_METHOD(Phalcon_Num, floor);
PHP_METHOD(Phalcon_Num, fabs);


ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_num_array, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_num_arange, 0, 0, 2)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, stop)
	ZEND_ARG_INFO(0, step)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_num_amin, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, ndarray, Phalcon\\Num\\Ndarray, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_num_amax, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, ndarray, Phalcon\\Num\\Ndarray, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_num_power, 0, 0, 2)
	ZEND_ARG_OBJ_INFO(0, ndarray, Phalcon\\Num\\Ndarray, 0)
	ZEND_ARG_INFO(0, exponent)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_num_square, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, ndarray, Phalcon\\Num\\Ndarray, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_num_sqrt, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, ndarray, Phalcon\\Num\\Ndarray, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_num_method_entry[] = {
	PHP_ME(Phalcon_Num, array, arginfo_phalcon_num_array, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Num, arange, arginfo_phalcon_num_arange, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Num, amin, arginfo_phalcon_num_amin, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Num, amax, arginfo_phalcon_num_amax, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Num, power, arginfo_phalcon_num_power, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Num, square, arginfo_phalcon_num_square, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Num, sqrt, arginfo_phalcon_num_sqrt, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Num, exp, arginfo_phalcon_num_sqrt, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Num, log, arginfo_phalcon_num_sqrt, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Num, log10, arginfo_phalcon_num_sqrt, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Num, sin, arginfo_phalcon_num_sqrt, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Num, cos, arginfo_phalcon_num_sqrt, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Num, tan, arginfo_phalcon_num_sqrt, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Num, ceil, arginfo_phalcon_num_sqrt, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Num, floor, arginfo_phalcon_num_sqrt, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Num, fabs, arginfo_phalcon_num_sqrt, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};


/**
 * Phalcon\Num initializer
 */
PHALCON_INIT_CLASS(Phalcon_Num){

	PHALCON_REGISTER_CLASS(Phalcon, Num, num, phalcon_num_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	return SUCCESS;
}

/**
 * Creates a N-dimensional array (ndarray) object
 */
PHP_METHOD(Phalcon_Num, array)
{
    zval *data;
    if( zend_parse_parameters(ZEND_NUM_ARGS(), "z", &data) == FAILURE ) {
        RETURN_NULL();
    }
    object_init_ex(return_value, phalcon_num_ndarray_ce);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", data);
}

/**
 * Return evenly spaced values within a given interval.
 */
PHP_METHOD(Phalcon_Num, arange)
{
    zval *start, *stop, *_step = NULL, step = {}, data = {};

	phalcon_fetch_params(1, 2, 1, &start, &stop, &_step);

	if (Z_TYPE_P(start) != IS_LONG && Z_TYPE_P(start) != IS_DOUBLE) {
		php_error_docref(NULL, E_ERROR, "Start parameter must be an integer or float");
		RETURN_MM_NULL();
	}

    if (Z_TYPE_P(stop) == IS_LONG && Z_TYPE_P(stop) == IS_DOUBLE) {
		php_error_docref(NULL, E_ERROR, "Stop parameter must be an integer or float");
		RETURN_MM_NULL();
    }

    if (!_step || (Z_TYPE_P(_step) == IS_LONG && Z_TYPE_P(_step) == IS_DOUBLE)) {
        ZVAL_LONG(&step, 1);
    } else {
		ZVAL_COPY_VALUE(&step, _step);
    }
	
	PHALCON_MM_CALL_FUNCTION(&data, "range", start, stop, &step);
	PHALCON_MM_ADD_ENTRY(&data);
    object_init_ex(return_value, phalcon_num_ndarray_ce);
	PHALCON_MM_CALL_METHOD(NULL, return_value, "__construct", &data);

	RETURN_MM();
}

/**
 * Return the minimum of an array
 */
PHP_METHOD(Phalcon_Num, amin)
{
    zval *obj, data={}, ret={};
    ZVAL_NULL(&ret);
    if( zend_parse_parameters(ZEND_NUM_ARGS(), "z", &obj) == FAILURE ) {
        RETURN_NULL();
    }
	phalcon_read_property(&data, obj, SL(NUM_NDARRAY_PROPERT_DATA), PH_READONLY);
    num_ndarray_compare_recursive(&ret, &data, num_min);
    RETURN_NCTOR(&ret);
}

/**
 * Return the maximum of an array
 */
PHP_METHOD(Phalcon_Num, amax)
{
    zval *obj, data={}, ret={};
    ZVAL_NULL(&ret);
    if( zend_parse_parameters(ZEND_NUM_ARGS(), "z", &obj) == FAILURE ) {
        RETURN_NULL();
    }
	phalcon_read_property(&data, obj, SL(NUM_NDARRAY_PROPERT_DATA), PH_READONLY);
    num_ndarray_compare_recursive(&ret, &data, num_max);
    RETURN_NCTOR(&ret);
}

/**
 * First array elements raised to powers from second array, element-wise
 */
PHP_METHOD(Phalcon_Num, power)
{
    zval *obj, *_exponent, newdata = {}, exponent = {};
    if( zend_parse_parameters(ZEND_NUM_ARGS(), "zz", &obj, &_exponent) == FAILURE ) {
        RETURN_NULL();
    }
	phalcon_read_property(&newdata, obj, SL(NUM_NDARRAY_PROPERT_DATA), PH_CTOR);
	if (Z_TYPE_P(_exponent) == IS_OBJECT) {
		PHALCON_VERIFY_CLASS_EX(_exponent, phalcon_num_ndarray_ce, phalcon_exception_ce);
		phalcon_read_property(&exponent, _exponent, SL(NUM_NDARRAY_PROPERT_DATA), PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&exponent, _exponent);
	}
    num_ndarray_arithmetic_recursive(&newdata, &exponent, pow);

    RETURN_NCTOR(&newdata);
}

/**
 * Return the element-wise square of the input
 */
PHP_METHOD(Phalcon_Num, square)
{
    zval *obj, exponent = {}, newdata = {};
    if( zend_parse_parameters(ZEND_NUM_ARGS(), "z", &obj) == FAILURE ) {
        RETURN_NULL();
    }
    ZVAL_DOUBLE(&exponent, 2);

	phalcon_read_property(&newdata, obj, SL(NUM_NDARRAY_PROPERT_DATA), PH_CTOR);
    num_ndarray_arithmetic_recursive(&newdata, &exponent, pow);

    RETURN_NCTOR(&newdata);
}

/**
 * Return the positive square-root of an array, element-wise
 */
PHP_METHOD(Phalcon_Num, sqrt)
{
    zval *obj, newdata = {};
    if( zend_parse_parameters(ZEND_NUM_ARGS(), "z", &obj) == FAILURE ) {
        RETURN_NULL();
    }
	phalcon_read_property(&newdata, obj, SL(NUM_NDARRAY_PROPERT_DATA), PH_CTOR);
    num_ndarray_self_recursive(&newdata, sqrt);

    RETURN_NCTOR(&newdata);
}

/**
 * Calculate the exponential of all elements in the input array
 */
PHP_METHOD(Phalcon_Num, exp)
{
    zval *obj, newdata = {};
    if( zend_parse_parameters(ZEND_NUM_ARGS(), "z", &obj) == FAILURE ) {
        RETURN_NULL();
    }
	phalcon_read_property(&newdata, obj, SL(NUM_NDARRAY_PROPERT_DATA), PH_CTOR);
    num_ndarray_self_recursive(&newdata, exp);

    RETURN_NCTOR(&newdata);
}

/**
 * Natural logarithm, element-wise
 */
PHP_METHOD(Phalcon_Num, log)
{
    zval *obj, newdata = {};
    if( zend_parse_parameters(ZEND_NUM_ARGS(), "z", &obj) == FAILURE ) {
        RETURN_NULL();
    }
	phalcon_read_property(&newdata, obj, SL(NUM_NDARRAY_PROPERT_DATA), PH_CTOR);
    num_ndarray_self_recursive(&newdata, log);

    RETURN_NCTOR(&newdata);
}

/**
 * Return the base 10 logarithm of the input array, element-wise
 */
PHP_METHOD(Phalcon_Num, log10)
{
    zval *obj, newdata = {};
    if( zend_parse_parameters(ZEND_NUM_ARGS(), "z", &obj) == FAILURE ) {
        RETURN_NULL();
    }
	phalcon_read_property(&newdata, obj, SL(NUM_NDARRAY_PROPERT_DATA), PH_CTOR);
    num_ndarray_self_recursive(&newdata, log10);

    RETURN_NCTOR(&newdata);
}

/**
 * Trigonometric sine, element-wise
 */
PHP_METHOD(Phalcon_Num, sin)
{
    zval *obj, newdata = {};
    if( zend_parse_parameters(ZEND_NUM_ARGS(), "z", &obj) == FAILURE ) {
        RETURN_NULL();
    }
	phalcon_read_property(&newdata, obj, SL(NUM_NDARRAY_PROPERT_DATA), PH_CTOR);
    num_ndarray_self_recursive(&newdata, sin);

    RETURN_NCTOR(&newdata);
}

/**
 * Cosine element-wise
 */
PHP_METHOD(Phalcon_Num, cos)
{
    zval *obj, newdata = {};
    if( zend_parse_parameters(ZEND_NUM_ARGS(), "z", &obj) == FAILURE ) {
        RETURN_NULL();
    }
	phalcon_read_property(&newdata, obj, SL(NUM_NDARRAY_PROPERT_DATA), PH_CTOR);
    num_ndarray_self_recursive(&newdata, cos);

    RETURN_NCTOR(&newdata);
}

/**
 * Compute tangent element-wise
 */
PHP_METHOD(Phalcon_Num, tan)
{
    zval *obj, newdata = {};
    if( zend_parse_parameters(ZEND_NUM_ARGS(), "z", &obj) == FAILURE ) {
        RETURN_NULL();
    }
	phalcon_read_property(&newdata, obj, SL(NUM_NDARRAY_PROPERT_DATA), PH_CTOR);
    num_ndarray_self_recursive(&newdata, tan);

    RETURN_NCTOR(&newdata);
}

/**
 * Return the ceiling of the input, element-wise
 */
PHP_METHOD(Phalcon_Num, ceil)
{
    zval *obj, newdata = {};
    if( zend_parse_parameters(ZEND_NUM_ARGS(), "z", &obj) == FAILURE ) {
        RETURN_NULL();
    }
	phalcon_read_property(&newdata, obj, SL(NUM_NDARRAY_PROPERT_DATA), PH_CTOR);
    num_ndarray_self_recursive(&newdata, ceil);

    RETURN_NCTOR(&newdata);
}

/**
 * Return the floor of the input, element-wise
 */
PHP_METHOD(Phalcon_Num, floor)
{
    zval *obj, newdata = {};
    if( zend_parse_parameters(ZEND_NUM_ARGS(), "z", &obj) == FAILURE ) {
        RETURN_NULL();
    }
	phalcon_read_property(&newdata, obj, SL(NUM_NDARRAY_PROPERT_DATA), PH_CTOR);
    num_ndarray_self_recursive(&newdata, floor);

    RETURN_NCTOR(&newdata);
}

/**
 * Compute the absolute values element-wise
 */
PHP_METHOD(Phalcon_Num, fabs)
{
    zval *obj, newdata = {};
    if( zend_parse_parameters(ZEND_NUM_ARGS(), "z", &obj) == FAILURE ) {
        RETURN_NULL();
    }
	phalcon_read_property(&newdata, obj, SL(NUM_NDARRAY_PROPERT_DATA), PH_CTOR);

    RETURN_NCTOR(&newdata);
}
