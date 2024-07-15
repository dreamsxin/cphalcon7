
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
#include "num/../exception.h"
#include "internal/arginfo.h"

#include <Zend/zend_smart_str.h>
#include <ext/standard/php_string.h>

#include "kernel/main.h"
#include "kernel/fcall.h"
#include "kernel/object.h"
#include "kernel/string.h"
#include "kernel/exception.h"

int num_calc_shape(zval *data, zval *shape, zend_long dimension){
    zval *check_shape;
    zend_long count;
    zval *val;
    HashTable *ht;
    ht = Z_ARRVAL_P(data);
    if (Z_TYPE_P(data) == IS_ARRAY) {
        count = Z_ARR_P(data)->nNumOfElements;
        check_shape = zend_hash_index_find(Z_ARR_P(shape), dimension);
        if (check_shape != NULL && Z_LVAL_P(check_shape) != count) {
            php_error_docref(NULL, E_ERROR, "Dimensions did not match");
            return FAILURE;
        } else {
            add_index_long(shape, dimension, count);
        }
        ZEND_HASH_FOREACH_VAL(ht, val) {
            if (num_calc_shape(val, shape, dimension + 1) != 0) {
				return FAILURE;
            }
        } ZEND_HASH_FOREACH_END();
    }
	return SUCCESS;
}

int num_ndarray_recursive(zval *data1, zval *data2, num_func_t num_func){
    zval *val1, *val2;
    zend_ulong idx;
    HashTable *ht1, *ht2;
    if (Z_TYPE_P(data1) == IS_ARRAY) {
        ht1 = Z_ARR_P(data1);
        ht2 = Z_ARR_P(data2);
        ZEND_HASH_FOREACH_NUM_KEY_VAL(ht1, idx, val1) {
            val2 = zend_hash_index_find(ht2, idx);
            if (Z_TYPE_P(val1) == IS_ARRAY) {
                num_ndarray_recursive(val1, val2, num_func);
            } else {
                convert_to_double(val1);
                convert_to_double(val2);
                ZVAL_DOUBLE(val1, num_func(Z_DVAL_P(val1), Z_DVAL_P(val2)));
            }
        } ZEND_HASH_FOREACH_END();
    }
    return SUCCESS;
}

int num_ndarray_compare_recursive(zval *ret, zval *data, num_func_t num_func){
    zval *val;
    if (Z_TYPE_P(data) == IS_ARRAY) {
        ZEND_HASH_FOREACH_VAL(Z_ARR_P(data), val) {
            if (Z_TYPE_P(val) == IS_ARRAY) {
                num_ndarray_compare_recursive(ret, val, num_func);
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

int num_ndarray_self_recursive(zval *data, num_func_t_one num_func){
    zval *val, tmp;
    zend_ulong idx;
    if (Z_TYPE_P(data) == IS_ARRAY) {
        ZEND_HASH_FOREACH_NUM_KEY_VAL(Z_ARR_P(data), idx, val) {
            if (Z_TYPE_P(val) == IS_ARRAY) {
                ZVAL_DUP(&tmp, val);
                add_index_zval(data, idx, &tmp);
                num_ndarray_self_recursive(&tmp, num_func);
            } else {
                convert_to_double(val);
                ZVAL_DOUBLE(val, num_func(Z_DVAL_P(val)));
            }
        } ZEND_HASH_FOREACH_END();
    }
    return SUCCESS;
}

int num_ndarray_arithmetic_recursive(zval *data1, zval *data2, num_func_t num_func){
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
                    num_ndarray_arithmetic_recursive(&tmp, val2, num_func);
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
                    num_ndarray_arithmetic_recursive(&tmp, data2, num_func);
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

zend_string *num_ndarray_to_string(zval *data, int level){
    zend_string *space, *ret;
    zval *first, *val, delim = {}, tmp;
    smart_str output = {0};
    HashPosition pointer = 0;
    HashTable *ht;
    zend_ulong idx;
    ZVAL_STRING(&delim, ",");

    if (Z_TYPE_P(data) != IS_ARRAY) {
        return zend_string_init(SL(Z_STRVAL_P(data)), 0);
    }
    space = level == 0 ? zend_string_init(SL(""), 0) : strpprintf(0, "%*c", level, ' ');
    ht = Z_ARRVAL_P(data);
    first = zend_hash_get_current_data_ex(ht, &pointer);
    if (Z_TYPE_P(first) == IS_ARRAY) {
        smart_str_sets(&output, ZSTR_VAL(space));
        smart_str_appends(&output, "[\n");
        ZEND_HASH_FOREACH_NUM_KEY_VAL(ht, idx, val) {
            ret = num_ndarray_to_string(val, level + 2);
            smart_str_appends(&output, ZSTR_VAL(ret));
			zend_string_release(ret);
            if (idx + 1 < zend_array_count(ht)) {
                smart_str_appends(&output, ",\n");
            } else {
                smart_str_appends(&output, "\n");
                smart_str_appends(&output, ZSTR_VAL(space));
                smart_str_appends(&output, "]");
            }
        } ZEND_HASH_FOREACH_END();
    } else {

        smart_str_appends(&output, ZSTR_VAL(space));
        smart_str_appends(&output, "[");
		phalcon_fast_join(&tmp, &delim, data);
        smart_str_appends(&output, Z_STRVAL(tmp));
		zval_ptr_dtor(&tmp);
        smart_str_appends(&output, "]");
    }

	zval_ptr_dtor(&delim);
	zend_string_release(space);
    smart_str_0(&output);
    return output.s;
}

zend_class_entry *phalcon_num_ndarray_ce;

PHP_METHOD(Phalcon_Num_Ndarray, __construct);
PHP_METHOD(Phalcon_Num_Ndarray, __toString);
PHP_METHOD(Phalcon_Num_Ndarray, getData);
PHP_METHOD(Phalcon_Num_Ndarray, getShape);
PHP_METHOD(Phalcon_Num_Ndarray, getNdim);
PHP_METHOD(Phalcon_Num_Ndarray, getSize);
PHP_METHOD(Phalcon_Num_Ndarray, add);
PHP_METHOD(Phalcon_Num_Ndarray, sub);
PHP_METHOD(Phalcon_Num_Ndarray, mult);
PHP_METHOD(Phalcon_Num_Ndarray, div);
PHP_METHOD(Phalcon_Num_Ndarray, apply);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_num_ndarray___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, data, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_num_ndarray_add, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, ndarray, Phalcon\\Num\\Ndarray, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_num_ndarray_apply, 0, 0, 1)
	ZEND_ARG_CALLABLE_INFO(0, callback, 0)
ZEND_END_ARG_INFO()

zend_function_entry phalcon_num_ndarray_method_entry[] = {
	PHP_ME(Phalcon_Num_Ndarray, __construct, arginfo_phalcon_num_ndarray___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Num_Ndarray, __toString, arginfo___tostring, ZEND_ACC_PUBLIC )
	PHP_ME(Phalcon_Num_Ndarray, getData, arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Num_Ndarray, getShape, arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Num_Ndarray, getNdim, arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Num_Ndarray, getSize, arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Num_Ndarray, add, arginfo_phalcon_num_ndarray_add, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Num_Ndarray, sub, arginfo_phalcon_num_ndarray_add, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Num_Ndarray, mult, arginfo_phalcon_num_ndarray_add, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Num_Ndarray, div, arginfo_phalcon_num_ndarray_add, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Num_Ndarray, apply, arginfo_phalcon_num_ndarray_apply, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


/**
 * Phalcon\Num initializer
 */
PHALCON_INIT_CLASS(Phalcon_Num_Ndarray){

	PHALCON_REGISTER_CLASS(Phalcon\\Num, Ndarray, num_ndarray, phalcon_num_ndarray_method_entry, 0);

    zend_declare_property_null(phalcon_num_ndarray_ce, SL(NUM_NDARRAY_PROPERT_DATA), ZEND_ACC_PUBLIC);
    zend_declare_property_null(phalcon_num_ndarray_ce, SL(NUM_NDARRAY_PROPERT_SHAPE), ZEND_ACC_PUBLIC);

	return SUCCESS;
}

/**
 * Phalcon\_Num\Ndarray
 *
 * @param array $data
 */
PHP_METHOD(Phalcon_Num_Ndarray, __construct)
{
    zval *data, shape = {};
    if( zend_parse_parameters(ZEND_NUM_ARGS(), "z", &data) == FAILURE ) {
        RETURN_NULL();
    }

    array_init(&shape);
    num_calc_shape(data, &shape, 0);

	phalcon_update_property(getThis(), SL(NUM_NDARRAY_PROPERT_DATA), data);
	phalcon_update_property(getThis(), SL(NUM_NDARRAY_PROPERT_SHAPE), &shape);
	zval_ptr_dtor(&shape);
}

PHP_METHOD(Phalcon_Num_Ndarray, __toString)
{
	zval data = {};
    zend_string *ret;
    smart_str output = {0};

	phalcon_read_property(&data, getThis(), SL(NUM_NDARRAY_PROPERT_DATA), PH_READONLY);

    ret = num_ndarray_to_string(&data, 0);
    smart_str_appends(&output, "array(");
    smart_str_appends(&output, ZSTR_VAL(ret));
    smart_str_appends(&output, ")\n");
    smart_str_0(&output);
	zend_string_release(ret);
    RETURN_STR(output.s);
}

/**
 * Data of the ndarray
 */
PHP_METHOD(Phalcon_Num_Ndarray, getData)
{

	RETURN_MEMBER(getThis(), NUM_NDARRAY_PROPERT_DATA);
}

/**
 * Shape of ndarray dimensions
 */
PHP_METHOD(Phalcon_Num_Ndarray, getShape)
{

	RETURN_MEMBER(getThis(), NUM_NDARRAY_PROPERT_SHAPE);
}

/**
 * Number of ndarray dimensions
 */
PHP_METHOD(Phalcon_Num_Ndarray, getNdim)
{
	zval shape = {};
	phalcon_read_property(&shape, getThis(), SL(NUM_NDARRAY_PROPERT_SHAPE), PH_READONLY);
    RETURN_LONG(zend_array_count(Z_ARRVAL(shape)));
}

/**
 * Number of elements in the ndarray
 */
PHP_METHOD(Phalcon_Num_Ndarray, getSize)
{
	zval shape = {}, *val;
	phalcon_read_property(&shape, getThis(), SL(NUM_NDARRAY_PROPERT_SHAPE), PH_READONLY);

    zend_ulong size = 1;
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL(shape), val) {
        size *= Z_LVAL_P(val);
    } ZEND_HASH_FOREACH_END();
    RETURN_LONG(size);
}

/**
 * Add an array to an other array
 */
PHP_METHOD(Phalcon_Num_Ndarray, add)
{
    zval *obj, shape1 = {}, shape2 = {}, compare, data1 = {}, data2 = {};

    if( zend_parse_parameters(ZEND_NUM_ARGS(), "z", &obj) == FAILURE ) {
        RETURN_NULL();
    }
	phalcon_read_property(&shape1, getThis(), SL(NUM_NDARRAY_PROPERT_SHAPE), PH_READONLY);
	phalcon_read_property(&shape2, obj, SL(NUM_NDARRAY_PROPERT_SHAPE), PH_READONLY);
    compare_function(&compare, &shape1, &shape2);
    if (Z_LVAL(compare) != 0) {
        php_error_docref(NULL, E_ERROR, "Operands could not be broadcast together with shapes");
        RETURN_NULL();
    }
	phalcon_read_property(&data1, getThis(), SL(NUM_NDARRAY_PROPERT_DATA), PH_READONLY);
	phalcon_read_property(&data2, obj, SL(NUM_NDARRAY_PROPERT_DATA), PH_READONLY);
    num_ndarray_recursive(&data1, &data2, num_operator_add);
    RETURN_THIS();
}

/**
 * Subtract an array from an other array
 */
PHP_METHOD(Phalcon_Num_Ndarray, sub)
{
    zval *obj, shape1 = {}, shape2 = {}, compare, data1 = {}, data2 = {};

    if( zend_parse_parameters(ZEND_NUM_ARGS(), "z", &obj) == FAILURE ) {
        RETURN_NULL();
    }
	phalcon_read_property(&shape1, getThis(), SL(NUM_NDARRAY_PROPERT_SHAPE), PH_READONLY);
	phalcon_read_property(&shape2, obj, SL(NUM_NDARRAY_PROPERT_SHAPE), PH_READONLY);
    compare_function(&compare, &shape1, &shape2);
    if (Z_LVAL(compare) != 0) {
        php_error_docref(NULL, E_ERROR, "Operands could not be broadcast together with shapes");
        RETURN_NULL();
    }
	phalcon_read_property(&data1, getThis(), SL(NUM_NDARRAY_PROPERT_DATA), PH_READONLY);
	phalcon_read_property(&data2, obj, SL(NUM_NDARRAY_PROPERT_DATA), PH_READONLY);
    num_ndarray_recursive(&data1, &data2, num_operator_sub);
    RETURN_THIS();
}

/**
 * Multiply an array by an other array
 */
PHP_METHOD(Phalcon_Num_Ndarray, mult)
{
    zval *obj, shape1 = {}, shape2 = {}, compare, data1 = {}, data2 = {};

    if( zend_parse_parameters(ZEND_NUM_ARGS(), "z", &obj) == FAILURE ) {
        RETURN_NULL();
    }
	phalcon_read_property(&shape1, getThis(), SL(NUM_NDARRAY_PROPERT_SHAPE), PH_READONLY);
	phalcon_read_property(&shape2, obj, SL(NUM_NDARRAY_PROPERT_SHAPE), PH_READONLY);
    compare_function(&compare, &shape1, &shape2);
    if (Z_LVAL(compare) != 0) {
        php_error_docref(NULL, E_ERROR, "Operands could not be broadcast together with shapes");
        RETURN_NULL();
    }
	phalcon_read_property(&data1, getThis(), SL(NUM_NDARRAY_PROPERT_DATA), PH_READONLY);
	phalcon_read_property(&data2, obj, SL(NUM_NDARRAY_PROPERT_DATA), PH_READONLY);
    num_ndarray_recursive(&data1, &data2, num_operator_mult);
    RETURN_THIS();
}

/**
 * An array divided by an other array
 */
PHP_METHOD(Phalcon_Num_Ndarray, div)
{
    zval *obj, shape1 = {}, shape2 = {}, compare, data1 = {}, data2 = {};

    if( zend_parse_parameters(ZEND_NUM_ARGS(), "z", &obj) == FAILURE ) {
        RETURN_NULL();
    }
	phalcon_read_property(&shape1, getThis(), SL(NUM_NDARRAY_PROPERT_SHAPE), PH_READONLY);
	phalcon_read_property(&shape2, obj, SL(NUM_NDARRAY_PROPERT_SHAPE), PH_READONLY);
    compare_function(&compare, &shape1, &shape2);
    if (Z_LVAL(compare) != 0) {
        php_error_docref(NULL, E_ERROR, "Operands could not be broadcast together with shapes");
        RETURN_NULL();
    }
	phalcon_read_property(&data1, getThis(), SL(NUM_NDARRAY_PROPERT_DATA), PH_READONLY);
	phalcon_read_property(&data2, obj, SL(NUM_NDARRAY_PROPERT_DATA), PH_READONLY);
    num_ndarray_recursive(&data1, &data2, num_operator_div);
    RETURN_THIS();
}



int num_ndarray_fci_recursive(zval *data, zend_fcall_info *fci, zend_fcall_info_cache *fci_cache){
    zval *val;
    zend_ulong idx;
    HashTable *ht1;
    if (Z_TYPE_P(data) == IS_ARRAY) {
        ht1 = Z_ARR_P(data);
        ZEND_HASH_FOREACH_NUM_KEY_VAL(ht1, idx, val) {
            if (Z_TYPE_P(val) == IS_ARRAY) {
                num_ndarray_fci_recursive(val, fci, fci_cache);
            } else {
				zval params[2], ret = {};

				ZVAL_COPY_VALUE(&params[0], val);
				ZVAL_LONG(&params[1], idx);

				fci->param_count = 2;
				fci->params = params;
				fci->retval = &ret;

				if (zend_call_function(fci, fci_cache) == FAILURE || Z_ISUNDEF(ret)) {
					zval_ptr_dtor(&ret);
					goto error;
				}
				switch (Z_TYPE(ret))
				{
					case IS_LONG: {
						ZVAL_LONG(val, Z_LVAL(ret));
						break;
					}
					case IS_DOUBLE: {
						ZVAL_DOUBLE(val, Z_DVAL(ret));
						break;
					}
					case IS_NULL: {
						zval_ptr_dtor(&ret);
						PHALCON_THROW_EXCEPTION_STR(phalcon_exception_ce, "Expected number, null returned");
						goto error;
					}       
					default: {
						zval_ptr_dtor(&ret);
						PHALCON_THROW_EXCEPTION_STR(phalcon_exception_ce, "Callable should return a number");
						goto error;
					}
				}
            }
        } ZEND_HASH_FOREACH_END();
    }
    return SUCCESS;
error:
    return FAILURE;
}

/**
 * 
 */
PHP_METHOD(Phalcon_Num_Ndarray, apply)
{
    zval data = {};
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;

	//parse prameters
	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_FUNC(fci, fci_cache)
	ZEND_PARSE_PARAMETERS_END_EX(
		PHALCON_THROW_EXCEPTION_STR(phalcon_exception_ce, "Expects a callback as a argument");
		return;
	);

	phalcon_read_property(&data, getThis(), SL(NUM_NDARRAY_PROPERT_DATA), PH_READONLY);

    num_ndarray_fci_recursive(&data, &fci, &fci_cache);
    RETURN_THIS();
}
