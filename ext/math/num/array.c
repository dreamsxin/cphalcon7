
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

#include "math/num.array.h"

#include "kernel/main.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/operators.h"

#include "internal/arginfo.h"

/**
 * Phalcon\Math\Num\Array
 *
 */
zend_class_entry *phalcon_math_num_array_ce;

PHP_METHOD(Phalcon_Math_Num_Array, __construct);
PHP_METHOD(Phalcon_Math_Num_Array, __toString);
PHP_METHOD(Phalcon_Math_Num_Array, valid);
PHP_METHOD(Phalcon_Math_Num_Array, astype);
PHP_METHOD(Phalcon_Math_Num_Array, reshape);
PHP_METHOD(Phalcon_Math_Num_Array, getData);
PHP_METHOD(Phalcon_Math_Num_Array, getShape);
PHP_METHOD(Phalcon_Math_Num_Array, getNdim);
PHP_METHOD(Phalcon_Math_Num_Array, getSize);
PHP_METHOD(Phalcon_Math_Num_Array, add);
PHP_METHOD(Phalcon_Math_Num_Array, sub);
PHP_METHOD(Phalcon_Math_Num_Array, mult);
PHP_METHOD(Phalcon_Math_Num_Array, div);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_array___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, data, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_array_valid, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, data, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_array_astype, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_array_reshape, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, data, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_array_add, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, data, Phalcon\\Math\\NUM\\Array, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_array_sub, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, data, Phalcon\\Math\\NUM\\Array, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_array_mult, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, data, Phalcon\\Math\\NUM\\Array, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_math_num_array_div, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, data, Phalcon\\Math\\NUM\\Array, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_math_num_array_method_entry[] = {
	PHP_ME(Phalcon_Math_Num_Array, __construct, arginfo_phalcon_math_num_array___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Math_Num_Array, __toString, arginfo___tostring, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Math_Num_Array, valid, arginfo_phalcon_math_num_array_valid, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num_Array, astype, arginfo_phalcon_math_num_array_astype, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num_Array, reshape, arginfo_phalcon_math_num_array_reshape, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Math_Num_Array, getData, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Math_Num_Array, getNdim, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Math_Num_Array, getSize, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Math_Num_Array, add, arginfo_phalcon_math_num_array_add, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Math_Num_Array, sub, arginfo_phalcon_math_num_array_sub, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Math_Num_Array, mult, arginfo_phalcon_math_num_array_mult, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Math_Num_Array, div, arginfo_phalcon_math_num_array_div, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


/**
 * Phalcon\Math\Num\Array initializer
 */
PHALCON_INIT_CLASS(Phalcon_Math_Num_Array){

	PHALCON_REGISTER_CLASS(Phalcon\\Math\\Num, Array, math_num_array, phalcon_math_num_array_method_entry, 0);

	zend_declare_property_null(phalcon_math_num_array_ce, SL("_data"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_math_num_array_ce, SL("_shape"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

zval* phalcon_math_num_array_calc_shape(zval *data, zval *shape, zend_long dimension){
    zval *ret, *check_shape;
    zend_long count;
    zval *val;
    HashTable *ht;
    ht = Z_ARRVAL_P(data);
    if (Z_TYPE_P(data) == IS_ARRAY) {
        count = Z_ARR_P(data)->nNumOfElements;
        check_shape = zend_hash_index_find(Z_ARR_P(shape), dimension);
        if (check_shape != NULL && Z_LVAL_P(check_shape) != count) {
            php_error_docref(NULL, E_ERROR, "Dimensions did not match");
            return NULL;
        } else {
            add_index_long(shape, dimension, count);
        }
        ZEND_HASH_FOREACH_VAL(ht, val) {
            phalcon_math_num_array_calc_shape(val, shape, dimension + 1);
        } ZEND_HASH_FOREACH_END();
    }
}

void phalcon_math_num_array_to_string(smart_str *output, zval *data, zend_long level){
    zend_string *space;
    zval *first, *val;
    HashPosition pointer = 0;
    HashTable *ht;
    zend_ulong idx;
    if (Z_TYPE_P(data) != IS_ARRAY) {
		if (Z_TYPE_P(data) != IS_STRING) {
			zval v = {};
			ZVAL_DUP(&v, data);
			convert_to_string(&v);
			smart_str_appends(Z_STRVAL(v));
			zval_ptr_dtor(&v);
		} else {
			smart_str_appends(Z_STRVAL_P(data));
		}
        return;
    }
    space = level == 0 ? zend_string_init(ZEND_STRL(""), 0) : strpprintf(0, "%*c", level, ' ');
    ht = Z_ARRVAL_P(data);
    first = zend_hash_get_current_data_ex(ht, &pointer);
    if (Z_TYPE_P(first) == IS_ARRAY) {
        smart_str_sets(output, ZSTR_VAL(space));
        smart_str_appends(output, "[\n");
        ZEND_HASH_FOREACH_NUM_KEY_VAL(ht, idx, val) {
           phalcon_math_num_array_to_string(output, val, level + 2);
            if (idx + 1 < zend_array_count(ht)) {
                smart_str_appends(output, ",\n");
            } else {
                smart_str_appends(output, "\n");
                smart_str_appends(output, ZSTR_VAL(space));
                smart_str_appends(output, "]");
            }
        } ZEND_HASH_FOREACH_END();
    } else {
		zval tmp = {};
		zend_string *delim;
		delim = zend_string_init(ZEND_STRL(","), 0);

        smart_str_appends(output, ZSTR_VAL(space));
        smart_str_appends(output, "[");
        php_implode(delim, data, &tmp);
		zend_string_release(delim);

        smart_str_appends(output, Z_STRVAL(tmp));
		zval_ptr_dtor(&tmp);
        smart_str_appends(output, "]");
    }
    smart_str_0(output);
    zval_ptr_dtor(first);
	zend_string_release(space);
}

int phalcon_math_num_array_recursive(zval *data1, zval *data2, phalcon_math_num_func_two_t num_func){
    zval *val1, *val2;
    zend_ulong idx;
    HashTable *ht1, *ht2;
    if (Z_TYPE_P(data1) == IS_ARRAY) {
        ht1 = Z_ARR_P(data1);
        ht2 = Z_ARR_P(data2);
        ZEND_HASH_FOREACH_NUM_KEY_VAL(ht1, idx, val1) {
            val2 = zend_hash_index_find(ht2, idx);
            if (Z_TYPE_P(val1) == IS_ARRAY) {
                phalcon_math_num_array_recursive(val1, val2, num_func);
            } else {
                convert_to_double(val1);
                convert_to_double(val2);
                ZVAL_DOUBLE(val1, num_func(Z_DVAL_P(val1), Z_DVAL_P(val2)));
            }
        } ZEND_HASH_FOREACH_END();
    }
    return SUCCESS;
}

int phalcon_math_num_array_compare_recursive(zval *ret, zval *data, phalcon_math_num_func_two_t num_func){
    zval *val;
    if (Z_TYPE_P(data) == IS_ARRAY) {
        ZEND_HASH_FOREACH_VAL(Z_ARR_P(data), val) {
            if (Z_TYPE_P(val) == IS_ARRAY) {
                phalcon_math_num_array_compare_recursive(ret, val, num_func);
            } else {
                convert_to_double(val);
                if (Z_TYPE_P(ret) == IS_NULL) {
                    ZVAL_DOUBLE(ret, Z_DVAL_P(val));
                } else {
                    ZVAL_DOUBLE(ret, num_func(Z_DVAL_P(ret), Z_DVAL_P(val)));
                }
            }
        } ZEND_HASH_FOREACH_END();
    }
    return SUCCESS;
}

int phalcon_math_num_array_self_recursive(zval *data, num_func_t_one num_func){
    zval *val, tmp;
    zend_ulong idx;
    if (Z_TYPE_P(data) == IS_ARRAY) {
        ZEND_HASH_FOREACH_NUM_KEY_VAL(Z_ARR_P(data), idx, val) {
            if (Z_TYPE_P(val) == IS_ARRAY) {
                ZVAL_DUP(&tmp, val);
                add_index_zval(data, idx, &tmp);
                phalcon_math_num_array_self_recursive(&tmp, num_func);
            } else {
                convert_to_double(val);
                ZVAL_DOUBLE(val, num_func(Z_DVAL_P(val)));
            }
        } ZEND_HASH_FOREACH_END();
    }
    return SUCCESS;
}

int phalcon_math_num_array_arithmetic_recursive(zval *data1, zval *data2, phalcon_math_num_func_two_t num_func){
    zval *val1, *val2, tmp;
    zend_ulong idx;
    HashTable *ht1, *ht2;
    if (Z_TYPE_P(data1) == IS_ARRAY) {
        ht1 = Z_ARR_P(data1);
        if (Z_TYPE_P(data2) == IS_ARRAY) {
            ht2 = Z_ARR_P(data2);
            ZEND_HASH_FOREACH_NUM_KEY_VAL(ht1, idx, val1) {
                val2 = zend_hash_index_find(ht2, idx);
                if (Z_TYPE_P(val1) == IS_ARRAY) {
                    ZVAL_DUP(&tmp, val1);
                    add_index_zval(data1, idx, &tmp);
                    phalcon_math_num_array_arithmetic_recursive(&tmp, val2, num_func);
                } else {
                    convert_to_double(val1);
                    convert_to_double(val2);
                    ZVAL_DOUBLE(val1, num_func(Z_DVAL_P(val1), Z_DVAL_P(val2)));
                }
            } ZEND_HASH_FOREACH_END();
        } else {
            ZEND_HASH_FOREACH_NUM_KEY_VAL(ht1, idx, val1) {
                if (Z_TYPE_P(val1) == IS_ARRAY) {
                    ZVAL_DUP(&tmp, val1);
                    add_index_zval(data1, idx, &tmp);
                    phalcon_math_num_array_arithmetic_recursive(&tmp, data2, num_func);
                } else {
                    convert_to_double(val1);
                    convert_to_double(data2);
                    ZVAL_DOUBLE(val1, num_func(Z_DVAL_P(val1), Z_DVAL_P(data2)));
                }
            } ZEND_HASH_FOREACH_END();
        }
    }
    return SUCCESS;
}

/**
 * Phalcon\Math\Num\Array constructor
 *
 * @param array $data
 */
PHP_METHOD(Phalcon_Math_Num_Array, __construct){

	zval *data, valid = {}, shape = {};

	phalcon_fetch_params(0, 1, 0, &data);

	PHALCON_CALL_CE_STATIC(&valid, phalcon_math_num_array_ce, "valid", data);
	if (!zend_is_true(&valid)) {
		return;
	}
	array_init(&shape);
	phalcon_math_num_array_calc_shape(data, &shape, 0);

	phalcon_update_property(getThis(), SL("_data"), data);
	phalcon_update_property(getThis(), SL("_shape"), &shape);
	zval_ptr_dtor(&shape);
}

/**
 * Magic method __toString
 *
 * @return string
 */
PHP_METHOD(Phalcon_Math_Num_Array, __toString){

	zval data = {};
    zend_string *ret;
    smart_str output = {0};

	phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);

    smart_str_appends(&output, "array(");
    ret = phalcon_math_num_array_to_string(&output, &data, 0);
    smart_str_appends(&output, ")\n");
    smart_str_0(&output);
	RETURN_STRING(ZSTR_VAL(output.s));
}

/**
 * Check array item type
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Math_Num_Array, valid){

	RETURN_TRUE;
}

/**
 * Copy of the array, cast to a specified type
 *
 * @param int $type
 * @return array
 */
PHP_METHOD(Phalcon_Math_Num_Array, astype){


}

/**
 * Returns an array containing the same data with a new shape.
 *
 * @param int $rows
 * @param int $cols
 * @return array
 */
PHP_METHOD(Phalcon_Math_Num_Array, reshape){


}

/**
 * Returns the data
 *
 * @return array
 */
PHP_METHOD(Phalcon_Math_Num_Array, getData){


	RETURN_MEMBER(getThis(), "_data");
}

/**
 * Returns the shape
 *
 * @return array
 */
PHP_METHOD(Phalcon_Math_Num_Array, getShape){


	RETURN_MEMBER(getThis(), "_shape");
}

/**
 * Number of array dimensions
 *
 * @return int
 */
PHP_METHOD(Phalcon_Math_Num_Array, getNdim){

}

/**
 * Number of elements in the array
 *
 * @return int
 */
PHP_METHOD(Phalcon_Math_Num_Array, getSize){

}

/**
 * Elementwise addition
 *
 * @param Phalcon\Math\Num\Array
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num_Array, add){

}

/**
 * Elementwise substraction
 *
 * @param Phalcon\Math\Num\Array
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num_Array, sub){

}

/**
 * Elementwise Multiplication
 *
 * @param Phalcon\Math\Num\Array
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num_Array, mult){

}

/**
 * Elementwise division
 *
 * @param Phalcon\Math\Num\Array
 * @return Phalcon\Math\Num\Array
 */
PHP_METHOD(Phalcon_Math_Num_Array, div){

}

