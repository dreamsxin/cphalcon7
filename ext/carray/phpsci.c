
/*
  +----------------------------------------------------------------------+
  | PHPSci CArray                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 2018 PHPSci Team                                       |
  +----------------------------------------------------------------------+
  | Licensed under the Apache License, Version 2.0 (the "License");      |
  | you may not use this file except in compliance with the License.     |
  | You may obtain a copy of the License at                              |
  |                                                                      |
  |     http://www.apache.org/licenses/LICENSE-2.0                       |
  |                                                                      |
  | Unless required by applicable law or agreed to in writing, software  |
  | distributed under the License is distributed on an "AS \IS" BASIS,   |
  | WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or      |
  | implied.                                                             |
  | See the License for the specific language governing permissions and  |
  | limitations under the License.                                       |
  +----------------------------------------------------------------------+
  | Authors: Henrique Borba <henrique.borba.dev@gmail.com>               |
  +----------------------------------------------------------------------+
*/

#include "carray/phpsci.h"

#include <php.h>
#include <ext/standard/info.h>
#include <Zend/zend_interfaces.h>

#include "carray/kernel/carray.h"
#include "carray/kernel/iterators.h"
#include "carray/kernel/shape.h"
#include "carray/kernel/calculation.h"
#include "carray/kernel/convert.h"
#include "carray/kernel/common/exceptions.h"
#include "carray/kernel/linalg.h"
#include "carray/kernel/alloc.h"
#include "carray/kernel/number.h"
#include "carray/kernel/trigonometric.h"
#include "carray/kernel/common/exceptions.h"
#include "carray/kernel/item_selection.h"
#include "carray/kernel/scalar.h"
#include "carray/kernel/random.h"
#include "carray/kernel/range.h"
#include "carray/kernel/buffer.h"
#include "carray/kernel/getset.h"
#include "carray/kernel/matlib.h"
#include "carray/kernel/join.h"
#include "carray/kernel/ctors.h"
#include "carray/kernel/statistics.h"
#include "carray/kernel/search.h"
#include "carray/kernel/exp_logs.h"
#include "carray/kernel/clip.h"
#include "carray/kernel/storage.h"
#include "carray/kernel/round.h"
#include "carray/kernel/random/distributions.h"
#include "carray/kernel/interfaces/rubix.h"

#ifdef HAVE_CLBLAS
#include "carray/kernel/gpu.h"
#endif

typedef struct _zend_carray_cdata {
    zend_object std;
} end_carray_cdata;

static inline zend_object *carray_create_object(zend_class_entry *ce) /* {{{ */
{
    end_carray_cdata * intern = emalloc(sizeof(end_carray_cdata) + zend_object_properties_size(ce));

    zend_object_std_init(&intern->std, ce);
    object_properties_init(&intern->std, ce);

    intern->std.handlers = &carray_object_handlers;

    return &intern->std;
}

static inline zend_object *crubix_create_object(zend_class_entry *ce) /* {{{ */
{
    end_carray_cdata * intern = emalloc(sizeof(end_carray_cdata) + zend_object_properties_size(ce));

    zend_object_std_init(&intern->std, ce);
    object_properties_init(&intern->std, ce);

    intern->std.handlers = &carray_object_handlers;

    return &intern->std;
}

void
ZVAL_TO_MEMORYPOINTER(zval * obj, MemoryPointer * ptr, char * type)
{
    ptr->free = 0;
    if (Z_TYPE_P(obj) == IS_ARRAY) {
        if (type == NULL) {
            CArray_FromZval(obj, 'a', ptr);
        } else {
            CArray_FromZval(obj, *type, ptr);
        }
        ptr->free = 2;
        return;
    }
    if (Z_TYPE_P(obj) == IS_OBJECT) {
        zval rv;
        ptr->uuid = (int)zval_get_long(zend_read_property(carray_sc_entry, obj, "uuid", sizeof("uuid") - 1, 1, &rv));
        return;
    }
    if (Z_TYPE_P(obj) == IS_LONG) {
        int * dims = emalloc(sizeof(int));
        dims[0] = 1;
        CArray * self = emalloc(sizeof(CArray));
        CArrayDescriptor * descr;
        descr = CArray_DescrFromType(TYPE_INTEGER_INT);
        self = CArray_NewFromDescr(self, descr, 0, dims, NULL, NULL, 0, NULL);
        convert_to_long(obj);

        if (zval_get_long(obj) > INT_MAX) {
            throw_overflow_exception("CArrays only works with int32 and "
                                     "float64 values, LONG INT detected.");
        }

        IDATA(self)[0] = (int)zval_get_long(obj);
        add_to_buffer(ptr, self, sizeof(CArray));
        efree(dims);
        ptr->free = 1;
    }
    if (Z_TYPE_P(obj) == IS_DOUBLE) {
        int * dims = emalloc(sizeof(int));
        dims[0] = 1;
        CArray * self = emalloc(sizeof(CArray));
        CArrayDescriptor * descr;
        descr = CArray_DescrFromType(TYPE_DOUBLE_INT);
        self = CArray_NewFromDescr(self, descr, 0, dims, NULL, NULL, 0, NULL);
        convert_to_double(obj);
        DDATA(self)[0] = (double)zval_get_double(obj);
        add_to_buffer(ptr, self, sizeof(CArray));
        efree(dims);
        ptr->free = 1;
    }
}

void
FREE_FROM_MEMORYPOINTER(MemoryPointer * ptr)
{
    if(ptr->free == 1 || ptr->free == 2) {
        CArray_Alloc_FreeFromMemoryPointer(ptr);
    }
}

void *
FREE_TUPLE(int * tuple)
{
    if(tuple != NULL)
        efree(tuple);
}

int * ZVAL_TO_TUPLE(zval * obj, int * size)
{
    zval * element;
    *size = 0;
    int * data_int;
    data_int = (int *)emalloc(zend_hash_num_elements(Z_ARRVAL_P(obj)) * sizeof(int));

    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(obj), element) {
        convert_to_long(element);
        data_int[*size] = (int)zval_get_long(element);
        *size = *size + 1;
    } ZEND_HASH_FOREACH_END();
    return data_int;
}

zval * 
MEMORYPOINTER_TO_ZVAL(MemoryPointer * ptr)
{
    zval * a = emalloc(sizeof(zval));
    object_init_ex(a, carray_sc_entry);
    CArray * arr = CArray_FromMemoryPointer(ptr);
    zend_update_property_long(carray_sc_entry, a, "uuid", sizeof("uuid") - 1, ptr->uuid);
    zend_update_property_long(carray_sc_entry, a, "ndim", sizeof("ndim") - 1, arr->ndim);
    return a;
}

void
RETURN_MEMORYPOINTER(zval *return_value, MemoryPointer *ptr)
{
    zend_class_entry *scope = zend_get_executed_scope();
    if(!strcmp(scope->name->val, "CRubix")) {
        RETURN_RUBIX_MEMORYPOINTER(return_value, ptr);
        return;
    }

    object_init_ex(return_value, carray_sc_entry);
    CArray *arr = CArray_FromMemoryPointer(ptr);
    zend_update_property_long(carray_sc_entry, return_value, "uuid", sizeof("uuid") - 1, ptr->uuid);
    zend_update_property_long(carray_sc_entry, return_value, "ndim", sizeof("ndim") - 1, arr->ndim);
}

void
RETURN_RUBIX_MEMORYPOINTER(zval * return_value, MemoryPointer * ptr)
{
    object_init_ex(return_value, crubix_sc_entry);
    CArray *arr = CArray_FromMemoryPointer(ptr);
    zend_update_property_long(carray_sc_entry, return_value, "uuid", sizeof("uuid") - 1, ptr->uuid);
    zend_update_property_long(carray_sc_entry, return_value, "ndim", sizeof("ndim") - 1, arr->ndim);
}

static int
TYPESTR_TO_INT(char * str)
{
    if (!strcmp(str, TYPE_INT32_STRING) || !strcmp(str, TYPE_INT_STRING)) {
        return TYPE_INTEGER_INT;
    }
    if (!strcmp(str, TYPE_INT64_STRING) || !strcmp(str, TYPE_LONG_STRING)) {
        return TYPE_LONG_INT;
    }
    if (!strcmp(str, TYPE_FLOAT32_STRING) || !strcmp(str, TYPE_FLOAT_STRING)) {
        return TYPE_FLOAT_INT;
    }
    if (!strcmp(str, TYPE_FLOAT64_STRING) || !strcmp(str, TYPE_DOUBLE_STRING)) {
        return TYPE_DOUBLE_INT;
    }
}

PHP_METHOD(CArray, __construct)
{
    MemoryPointer ptr;
    zval * obj_zval;
    char * type;
    size_t type_name_len;
    char   type_parsed;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_ZVAL(obj_zval)
        Z_PARAM_OPTIONAL
        Z_PARAM_STRING(type, type_name_len)
    ZEND_PARSE_PARAMETERS_END();
    if(ZEND_NUM_ARGS() == 1) {
        type_parsed = 'a';
    }
    if(ZEND_NUM_ARGS() > 1) {
        type_parsed = type[0];
    }
    ZVAL_TO_MEMORYPOINTER(obj_zval, &ptr, &type_parsed);
    zval * obj = getThis();
    CArray * arr = CArray_FromMemoryPointer(&ptr);
    zend_update_property_long(carray_sc_entry, obj, "uuid", sizeof("uuid") - 1, (int)ptr.uuid);
    zend_update_property_long(carray_sc_entry, obj, "ndim", sizeof("ndim") - 1, (int)arr->ndim);
}

/**
 * GET & SETS
 **/ 
ZEND_BEGIN_ARG_INFO_EX(arginfo_array_set, 0, 0, 2)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()
PHP_METHOD(CArray, __set)
{
    size_t name_len;
    char * name;
    zval * value;
    MemoryPointer value_ptr, target_ptr;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STRING(name, name_len)
        Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END();
    if(!strcmp(name, "flat")) {
        CArray * value_arr, * self_arr;
        ZVAL_TO_MEMORYPOINTER(getThis(), &target_ptr, NULL);
        ZVAL_TO_MEMORYPOINTER(value, &value_ptr, NULL);
        value_arr = CArray_FromMemoryPointer(&value_ptr);
        self_arr = CArray_FromMemoryPointer(&target_ptr);
        array_flat_set(self_arr, value_arr);
        return;
    }
    if(!strcmp(name, "uuid")) {
        zend_update_property_long(carray_sc_entry, getThis(), "uuid", sizeof("uuid") - 1, zval_get_long(value));
        return;
    }
    if(!strcmp(name, "ndim")) {
        zend_update_property_long(carray_sc_entry, getThis(), "ndim", sizeof("ndim") - 1, zval_get_long(value));
        return;
    }
    throw_valueerror_exception("Unknown property.");    
}

PHP_METHOD(CArray, offsetExists)
{
    zval *index;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &index) == FAILURE) {
        return;
    }
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_offsetGet, 0, 0, 1)
    ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()
PHP_METHOD(CArray, offsetGet)
{
    CArray * _this_ca, * ret_ca = NULL;
    MemoryPointer ptr, target_ptr;
    zval *index;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &index) == FAILURE) {
        return;
    }

    zval * obj = getThis();
    ZVAL_TO_MEMORYPOINTER(obj, &ptr, NULL);
    _this_ca = CArray_FromMemoryPointer(&ptr);

    if (Z_TYPE_P(index) == IS_STRING) {
        convert_to_string(index);
        ret_ca = (CArray *) CArray_Slice_Str(_this_ca, zval_get_string(index)->val, &target_ptr);
    }

    if (Z_TYPE_P(index) == IS_LONG) {
        convert_to_long(index);
        if (zval_get_long(index) > CArray_DIMS(_this_ca)[0]) {
            throw_indexerror_exception("");
            return;
        }
        ret_ca = (CArray *) CArray_Slice_Index(_this_ca, (int)zval_get_long(index), &target_ptr);
    }

    if(ret_ca != NULL) {
        RETURN_MEMORYPOINTER(return_value, &target_ptr);
    }
}

ZEND_BEGIN_ARG_INFO(arginfo_offsetSet, 0)
                ZEND_ARG_INFO(0, index)
                ZEND_ARG_INFO(0, newval)
ZEND_END_ARG_INFO();
PHP_METHOD(CArray, offsetSet)
{
    CArray * target, * value;
    MemoryPointer target_ptr, value_ptr;
    long indexl;
    zval *index, *val;
    zval * obj = getThis();
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "zz", &index, &val) == FAILURE) {
        return;
    }
    convert_to_long(index);
    indexl = zval_get_double(index);
    ZVAL_TO_MEMORYPOINTER(val, &value_ptr, NULL);
    ZVAL_TO_MEMORYPOINTER(obj, &target_ptr, NULL);
    target = CArray_FromMemoryPointer(&target_ptr);
    value = CArray_FromMemoryPointer(&value_ptr);

    if ((int)indexl  >= CArray_DIMS(target)[0]) {
        throw_indexerror_exception("");
        return;
    }

    setArrayFromSequence(target, value, CArray_NDIM(value), ((int)indexl * CArray_STRIDES(target)[0]));
    FREE_FROM_MEMORYPOINTER(&target_ptr);
    FREE_FROM_MEMORYPOINTER(&value_ptr);
}
PHP_METHOD(CArray, offsetUnset)
{
    zval *index;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &index) == FAILURE) {
        return;
    }

}


PHP_METHOD(CArray, setShape)
{
    MemoryPointer ptr;
    CArray * carray, * newcarray;
    zval * new_shape_zval;
    int * new_shape;
    int ndim;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(new_shape_zval)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(getThis(), &ptr, NULL);
    carray = CArray_FromMemoryPointer(&ptr);
    new_shape = ZVAL_TO_TUPLE(new_shape_zval, &ndim);
    newcarray = CArray_Newshape(carray, new_shape, zend_hash_num_elements(Z_ARRVAL_P(new_shape_zval)), CARRAY_CORDER, &ptr);
    FREE_TUPLE(new_shape);

    if (newcarray == NULL) {
        return;
    }
    RETURN_MEMORYPOINTER(return_value, &ptr);
}
PHP_METHOD(CArray, shape)
{
    MemoryPointer ptr;
    CArray * target;
    zval * obj = getThis();
    zval tmp_zval;
    int i;

    ZVAL_TO_MEMORYPOINTER(obj, &ptr, NULL);
    target = CArray_FromMemoryPointer(&ptr);

    array_init_size(return_value, CArray_NDIM(target));
    for (i = 0; i < CArray_NDIM(target); i++) {
        ZVAL_LONG(&tmp_zval, CArray_DIMS(target)[i]);
        zend_hash_next_index_insert_new(Z_ARRVAL_P(return_value), &tmp_zval);
    }
}
PHP_METHOD(CArray, reshape)
{
    MemoryPointer ptr;
    CArray * carray, * newcarray;
    zval * new_shape_zval, * target;
    int * new_shape;
    int ndim;
    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target)
            Z_PARAM_ZVAL(new_shape_zval)
    ZEND_PARSE_PARAMETERS_END();

    if(ZEND_NUM_ARGS() == 1) {
        throw_valueerror_exception("Expected 2 arguments");
        return;
    }

    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    carray = CArray_FromMemoryPointer(&ptr);
    new_shape = ZVAL_TO_TUPLE(new_shape_zval, &ndim);
    newcarray = CArray_Newshape(carray, new_shape, zend_hash_num_elements(Z_ARRVAL_P(new_shape_zval)), CARRAY_CORDER, &ptr);
    FREE_TUPLE(new_shape);

    if (newcarray == NULL) {
        return;
    }
    RETURN_MEMORYPOINTER(return_value, &ptr);
}



PHP_METHOD(CArray, dump)
{
    MemoryPointer ptr;
    CArray * array;
    zval * obj = getThis();
    ZVAL_TO_MEMORYPOINTER(obj, &ptr, NULL);
    array = CArray_FromMemoryPointer(&ptr);
    CArray_Dump(array);
}

PHP_METHOD(CArray, print)
{
    zval * obj = getThis();
    CArray * array;
    MemoryPointer ptr;
    ZVAL_TO_MEMORYPOINTER(obj, &ptr, NULL);
    array = CArray_FromMemoryPointer(&ptr);
    CArray_Print(array, 0);
}

/**
 * DESTRUCTOR
 **/ 
PHP_METHOD(CArray, __destruct)
{
    MemoryPointer ptr;
    ZVAL_TO_MEMORYPOINTER(getThis(), &ptr, NULL);
    CArray_Alloc_FreeFromMemoryPointer(&ptr);
}

/**
 * CALCULATIONS
 **/ 
PHP_METHOD(CArray, sum)
{
    zval * target;
    long axis;
    int * axis_p;
    CArray * ret, * target_ca;
    MemoryPointer ptr, out_ptr;
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_ZVAL(target)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(axis)
    ZEND_PARSE_PARAMETERS_END();
    if(ZEND_NUM_ARGS() == 1) {
        axis_p = NULL;
    }
    if(ZEND_NUM_ARGS() > 1) {
        axis_p = (int*)emalloc(sizeof(int));
        *axis_p = axis;
    }
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Sum(target_ca, axis_p, target_ca->descriptor->type_num, &out_ptr);

    if(ZEND_NUM_ARGS() > 1) {
        efree(axis_p);
    }

    if (ret == NULL) {
        return;
    }

    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &out_ptr);
}

/**
 * TRIGONOMETRIC
 */
PHP_METHOD(CArray, sin)
{
    zval * target;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Sin(target_ca, &rtn_tr);

    if (ret == NULL) {
        return;
    }

    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}
PHP_METHOD(CArray, cos)
{
    zval * target;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Cos(target_ca, &rtn_tr);

    if (ret == NULL) {
        return;
    }

    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}
PHP_METHOD(CArray, tan)
{
    zval * target;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Tan(target_ca, &rtn_tr);

    if (ret == NULL) {
        return;
    }
    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}
PHP_METHOD(CArray, arcsin)
{
    zval * target;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Arcsin(target_ca, &rtn_tr);

    if (ret == NULL) {
        return;
    }
    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}
PHP_METHOD(CArray, arccos)
{
    zval * target;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Arccos(target_ca, &rtn_tr);

    if (ret == NULL) {
        return;
    }
    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}
PHP_METHOD(CArray, arctan)
{
    zval * target;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Arctan(target_ca, &rtn_tr);

    if (ret == NULL) {
        return;
    }
    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}
PHP_METHOD(CArray, sinh)
{
    zval * target;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Sinh(target_ca, &rtn_tr);

    if (ret == NULL) {
        return;
    }
    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}
PHP_METHOD(CArray, cosh)
{
    zval * target;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Cosh(target_ca, &rtn_tr);

    if (ret == NULL) {
        return;
    }
    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}
PHP_METHOD(CArray, tanh)
{
    zval * target;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Tanh(target_ca, &rtn_tr);

    if (ret == NULL) {
        return;
    }
    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}

/**
 * EXPONENTS AND LOGARITHMS
 */
PHP_METHOD(CArray, exp)
{
    zval * target;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Exp(target_ca, &rtn_tr);

    if (ret == NULL) {
        return;
    }
    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}
PHP_METHOD(CArray, expm1)
{
    zval * target;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Expm1(target_ca, &rtn_tr);

    if (ret == NULL) {
        return;
    }
    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}
PHP_METHOD(CArray, exp2)
{
    zval * target;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Exp2(target_ca, &rtn_tr);

    if (ret == NULL) {
        return;
    }
    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}
PHP_METHOD(CArray, log)
{
    zval * target;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Log(target_ca, &rtn_tr);

    if (ret == NULL) {
        return;
    }
    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}
PHP_METHOD(CArray, log10)
{
    zval * target;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Log10(target_ca, &rtn_tr);

    if (ret == NULL) {
        return;
    }
    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}
PHP_METHOD(CArray, log2)
{
    zval * target;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Log2(target_ca, &rtn_tr);

    if (ret == NULL) {
        return;
    }
    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}
PHP_METHOD(CArray, log1p)
{
    zval * target;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Log1p(target_ca, &rtn_tr);

    if (ret == NULL) {
        return;
    }
    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}



PHP_METHOD(CArray, transpose)
{
    zval * target;
    zval * axes = NULL;
    int size_axes;
    CArray * ret, * target_ca;
    MemoryPointer ptr;
    CArray_Dims permute;
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_ZVAL(target)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(axes)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    if(ZEND_NUM_ARGS() == 1) {
        ret = CArray_Transpose(target_ca, NULL, &ptr);
    }
    if(ZEND_NUM_ARGS() > 1) {
        permute.ptr = ZVAL_TO_TUPLE(axes, &size_axes);
        permute.len = size_axes;
        ret = CArray_Transpose(target_ca, &permute, &ptr);
        FREE_TUPLE(permute.ptr);
    }
    if (ret == NULL) {
        return;
    }
    RETURN_MEMORYPOINTER(return_value, &ptr);
}

/**
 * METHODS
 */  
PHP_METHOD(CArray, identity)
{
    MemoryPointer ptr;
    CArray * output;
    zend_long size;
    char * dtype = NULL;
    size_t type_len;
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_LONG(size)
        Z_PARAM_OPTIONAL
        Z_PARAM_STRING(dtype, type_len)
    ZEND_PARSE_PARAMETERS_END();

    output = CArray_Eye((int)size, (int)size, 0, dtype, &ptr);

    if (output == NULL) {
		return;
	}
    RETURN_MEMORYPOINTER(return_value, &ptr);
}
PHP_METHOD(CArray, eye)
{
    MemoryPointer ptr;
    CArray * output;
    zend_long n, m, k;
    char * dtype = NULL;
    size_t type_len;
    ZEND_PARSE_PARAMETERS_START(1, 4)
        Z_PARAM_LONG(n)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(m)
        Z_PARAM_LONG(k)
        Z_PARAM_STRING(dtype, type_len)
    ZEND_PARSE_PARAMETERS_END();

    if(ZEND_NUM_ARGS() == 1) {
        m = n;
        k = 0;
    }
    if(ZEND_NUM_ARGS() == 2) {
        k = 0;
    }

    output = CArray_Eye((int)n, (int)m, (int)k, dtype, &ptr);
    if (output == NULL) {
		return;
	}
    RETURN_MEMORYPOINTER(return_value, &ptr);
}

/**
 * SEARCH
 */
PHP_METHOD(CArray, argmax)
{
    zval * target;
    long axis;
    int * axis_p;
    CArray * ret, * target_ca;
    MemoryPointer ptr, out_ptr;
    ZEND_PARSE_PARAMETERS_START(1, 2)
            Z_PARAM_ZVAL(target)
            Z_PARAM_OPTIONAL
            Z_PARAM_LONG(axis)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);

    axis_p = (int*)emalloc(sizeof(int));

    if(ZEND_NUM_ARGS() == 1) {
        CArray_DECREF(target_ca);
        target_ca = CArray_Ravel(target_ca, CARRAY_KEEPORDER);
        CArrayDescriptor_DECREF(CArray_DESCR(target_ca));
        *axis_p = 0;
    }
    if(ZEND_NUM_ARGS() > 1) {
        *axis_p = axis;
    }

    ret = CArray_Argmax(target_ca, axis_p, &out_ptr);

    efree(axis_p);
    if (ret == NULL) {
        return;
    }

    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &out_ptr);
}
PHP_METHOD(CArray, argmin)
{
    zval * target;
    long axis;
    int * axis_p;
    CArray * ret, * target_ca;
    MemoryPointer ptr, out_ptr;
    ZEND_PARSE_PARAMETERS_START(1, 2)
            Z_PARAM_ZVAL(target)
            Z_PARAM_OPTIONAL
            Z_PARAM_LONG(axis)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);

    axis_p = (int*)emalloc(sizeof(int));

    if(ZEND_NUM_ARGS() == 1) {
        CArray_DECREF(target_ca);
        target_ca = CArray_Ravel(target_ca, CARRAY_KEEPORDER);
        CArrayDescriptor_DECREF(CArray_DESCR(target_ca));
        *axis_p = 0;
    }
    if(ZEND_NUM_ARGS() > 1) {
        *axis_p = axis;
    }

    ret = CArray_Argmin(target_ca, axis_p, &out_ptr);

    efree(axis_p);
    if (ret == NULL) {
        return;
    }

    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &out_ptr);
}

/**
 * SORTING
 */
PHP_METHOD(CArray, sort)
{
    zval * target;
    char * kind = NULL;
    long axis = 0;
    int * axis_p, decref = 0;
    zend_bool is_null = 0;
    size_t s_kind;
    CArray * ret, * target_ca = NULL, * tmp_ca = NULL;
    MemoryPointer ptr, out_ptr;
    CARRAY_SORTKIND sortkind;
    ZEND_PARSE_PARAMETERS_START(1, 3)
            Z_PARAM_ZVAL(target)
            Z_PARAM_OPTIONAL
            Z_PARAM_LONG_EX(axis, is_null, 0, 0)
            Z_PARAM_STRING(kind, s_kind)
    ZEND_PARSE_PARAMETERS_END();
    axis_p = (int*)emalloc(sizeof(int));

    if(ZEND_NUM_ARGS() == 1) {
        ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
        target_ca = CArray_FromMemoryPointer(&ptr);
        *axis_p = -1;
        sortkind = CARRAY_QUICKSORT;
    }
    if(ZEND_NUM_ARGS() == 2) {
        if (!is_null) {
            ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
            target_ca = CArray_FromMemoryPointer(&ptr);
            *axis_p = axis;
        } else {
            decref = 1;
            *axis_p = -1;
            ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
            tmp_ca = CArray_FromMemoryPointer(&ptr);
            target_ca = CArray_Ravel(tmp_ca, CARRAY_KEEPORDER);
        }
        sortkind = CARRAY_QUICKSORT;
    }
    if(ZEND_NUM_ARGS() > 2) {
        if (!is_null) {
            ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
            target_ca = CArray_FromMemoryPointer(&ptr);
            *axis_p = axis;
        } else {
            decref = 1;
            *axis_p = -1;
            ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
            tmp_ca = CArray_FromMemoryPointer(&ptr);
            target_ca = CArray_Ravel(tmp_ca, CARRAY_KEEPORDER);
        }
        if(strcmp(kind, "quicksort") == 0) {
            sortkind = CARRAY_QUICKSORT;
        }
        if(strcmp(kind, "mergesort") == 0) {
            sortkind = CARRAY_MERGESORT;
        }
        if(strcmp(kind, "heapsort") == 0) {
            CArray_INCREF(target_ca);
            sortkind = CARRAY_HEAPSORT;
        }
        if(strcmp(kind, "stable") == 0) {
            sortkind = CARRAY_MERGESORT;
        }
    }

    ret = CArray_Sort(target_ca, axis_p, sortkind, 0, &out_ptr);

    efree(axis_p);
    if (ret == NULL) {
        return;
    }
    CArrayDescriptor_INCREF(CArray_DESCR(ret));
    if(decref) {
        CArray_DECREF(target_ca);
    }
    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &out_ptr);
}


/**
 * LINEAR ALGEBRA 
 */ 
PHP_METHOD(CArray, matmul)
{
    MemoryPointer target1_ptr, target2_ptr, result_ptr;

    zval * target1, * target2;
    CArray * target_ca1, * target_ca2, * output_ca;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ZVAL(target1)
        Z_PARAM_ZVAL(target2)
    ZEND_PARSE_PARAMETERS_END();    
    ZVAL_TO_MEMORYPOINTER(target1, &target1_ptr, NULL);
    ZVAL_TO_MEMORYPOINTER(target2, &target2_ptr, NULL);
    
    target_ca1 = CArray_FromMemoryPointer(&target1_ptr);
    target_ca2 = CArray_FromMemoryPointer(&target2_ptr);
    output_ca = CArray_Matmul(target_ca1, target_ca2, NULL, &result_ptr);

    FREE_FROM_MEMORYPOINTER(&target1_ptr);
    FREE_FROM_MEMORYPOINTER(&target2_ptr);

    if (output_ca != NULL) {
        RETURN_MEMORYPOINTER(return_value, &result_ptr);
    }
}
PHP_METHOD(CArray, inv)
{
    MemoryPointer target, rtn_ptr;
    zval * target_z;
    CArray * target_ca, * rtn_ca = NULL;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(target_z)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target_z, &target, NULL);
    target_ca = CArray_FromMemoryPointer(&target);
    rtn_ca = CArray_Inv(target_ca, &rtn_ptr);

    FREE_FROM_MEMORYPOINTER(&target);
    if (rtn_ca != NULL) {
        RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
    }
}
PHP_METHOD(CArray, solve)
{
    MemoryPointer out, a_ptr, b_ptr;
    CArray *a_ca, *rtn_ca, *b_ca;
    zval *a, *b;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ZVAL(a)
        Z_PARAM_ZVAL(b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(a, &a_ptr, NULL);
    ZVAL_TO_MEMORYPOINTER(b, &b_ptr, NULL);
    a_ca = CArray_FromMemoryPointer(&a_ptr);
    b_ca = CArray_FromMemoryPointer(&b_ptr);

    rtn_ca = CArray_Solve(a_ca, b_ca, &out);

    if (rtn_ca == NULL) {
        return;
    }

    FREE_FROM_MEMORYPOINTER(&a_ptr);
    FREE_FROM_MEMORYPOINTER(&b_ptr);
    RETURN_MEMORYPOINTER(return_value, &out);
}
/**
 * @todo Implement more norm types
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CArray, norm)
{
    MemoryPointer target, rtn_ptr;
    zval * target_z;
    CArray * target_ca, * rtn_ca = NULL;
    size_t type_len;
    char * dtype;
    int norm;
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_ZVAL(target_z)
        Z_PARAM_OPTIONAL
        Z_PARAM_STRING(dtype, type_len)
    ZEND_PARSE_PARAMETERS_END();
    if(ZEND_NUM_ARGS() >= 1) {
        norm = 1;
    }
    ZVAL_TO_MEMORYPOINTER(target_z, &target, NULL);
    target_ca = CArray_FromMemoryPointer(&target);
    rtn_ca = CArray_Norm(target_ca, norm, &rtn_ptr);

    FREE_FROM_MEMORYPOINTER(&target);
    if (rtn_ca != NULL) {
        RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
    }
}
PHP_METHOD(CArray, det)
{
    MemoryPointer target, rtn_ptr;
    zval * target_z;
    CArray * target_ca, * rtn_ca = NULL;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target_z)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target_z, &target, NULL);
    target_ca = CArray_FromMemoryPointer(&target);
    rtn_ca = CArray_Det(target_ca, &rtn_ptr);

    FREE_FROM_MEMORYPOINTER(&target);
    if (rtn_ca != NULL) {
        RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
    }
}
PHP_METHOD(CArray, matrix_rank)
{
    zval * a;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(a)
    ZEND_PARSE_PARAMETERS_END();
}
PHP_METHOD(CArray, vdot)
{
    MemoryPointer a_ptr, b_ptr, rtn_ptr;
    zval * targeta_z, * targetb_z;
    CArray * a_ca, * b_ca, * rtn_ca = NULL;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ZVAL(targeta_z)
        Z_PARAM_ZVAL(targetb_z)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(targeta_z, &a_ptr, NULL);
    ZVAL_TO_MEMORYPOINTER(targetb_z, &b_ptr, NULL);

    a_ca = CArray_FromMemoryPointer(&a_ptr);
    b_ca = CArray_FromMemoryPointer(&b_ptr);

    rtn_ca = CArray_Vdot(a_ca, b_ca, &rtn_ptr);

    if (rtn_ca == NULL) {
        return;
    }

    FREE_FROM_MEMORYPOINTER(&a_ptr);
    FREE_FROM_MEMORYPOINTER(&b_ptr);

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}
PHP_METHOD(CArray, inner)
{
    MemoryPointer rtn_ptr, a_ptr, b_ptr;
    zval *a, *b;
    CArray *a_ca, *b_ca, *rtn_ca;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ZVAL(a)
        Z_PARAM_ZVAL(b)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(a, &a_ptr, NULL);
    ZVAL_TO_MEMORYPOINTER(b, &b_ptr, NULL);

    a_ca = CArray_FromMemoryPointer(&a_ptr);
    b_ca = CArray_FromMemoryPointer(&b_ptr);

    rtn_ca = CArray_InnerProduct(a_ca, b_ca, &rtn_ptr);

    FREE_FROM_MEMORYPOINTER(&a_ptr);
    FREE_FROM_MEMORYPOINTER(&b_ptr);
    if (rtn_ca == NULL) {
        return;
    }

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}
PHP_METHOD(CArray, outer)
{

}
PHP_METHOD(CArray, eig)
{
    zval * a;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(a)
    ZEND_PARSE_PARAMETERS_END();
}
PHP_METHOD(CArray, eigvals)
{
    zval * a;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(a)
    ZEND_PARSE_PARAMETERS_END();
}
PHP_METHOD(CArray, svd)
{
    int full_matrices_int = 1, compute_uv_int = 1, i = 0;
    MemoryPointer a_ptr, * out_ptr;
    zval * a;
    zval * tmp_zval;
    zend_bool  full_matrices = IS_FALSE, compute_uv = IS_FALSE;
    CArray ** rtn, * a_ca;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(a)
            Z_PARAM_OPTIONAL
            Z_PARAM_BOOL(full_matrices)
            Z_PARAM_BOOL(compute_uv)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(a, &a_ptr, NULL);
    a_ca = CArray_FromMemoryPointer(&a_ptr);
    if(ZEND_NUM_ARGS() == 2) {
        if (full_matrices == IS_FALSE) {
            full_matrices_int = 0;
        }
    }
    if(ZEND_NUM_ARGS() == 3) {
        if (full_matrices == IS_FALSE) {
            full_matrices_int = 0;
        }
        if (compute_uv == IS_FALSE) {
            compute_uv_int = 0;
        }
    }
    out_ptr = emalloc(3 * sizeof(MemoryPointer));
    rtn = CArray_Svd(a_ca, full_matrices_int, compute_uv_int, out_ptr);

    FREE_FROM_MEMORYPOINTER(&a_ptr);

    if (rtn == NULL) {
        efree(out_ptr);
        return;
    }

    array_init_size(return_value, 3);
    for (i = 0; i < 3;i ++) {
        tmp_zval = MEMORYPOINTER_TO_ZVAL(&(out_ptr[i]));
        zend_hash_next_index_insert_new(Z_ARRVAL_P(return_value), tmp_zval);
        efree(tmp_zval);
    }

    efree(out_ptr);
    efree(rtn);
}
PHP_METHOD(CArray, qr)
{
    zval * a;
    ZEND_PARSE_PARAMETERS_START(1, 1)
         Z_PARAM_ZVAL(a)
    ZEND_PARSE_PARAMETERS_END();


}
PHP_METHOD(CArray, cholesky)
{
    zval * a;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(a)
    ZEND_PARSE_PARAMETERS_END();

}







PHP_METHOD(CArray, zeros)
{   
    zval * zshape;
    char * dtype, order = 'C';
    int ndim;
    int * shape;
    MemoryPointer ptr;
    size_t type_len;
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_ZVAL(zshape)
        Z_PARAM_OPTIONAL
        Z_PARAM_STRING(dtype, type_len)
    ZEND_PARSE_PARAMETERS_END();
    if(ZEND_NUM_ARGS() == 1) {
        dtype = emalloc(sizeof(char));
        *dtype = 'd';
    }

    // @todo Validate input array (check for empty shape array)

    shape = ZVAL_TO_TUPLE(zshape, &ndim);
    CArray_Zeros(shape, ndim, *dtype, &order, &ptr);
    efree(shape);
    if(ZEND_NUM_ARGS() == 1) {
        efree(dtype);
    }
    RETURN_MEMORYPOINTER(return_value, &ptr);
}
PHP_METHOD(CArray, ones)
{   
    zval * zshape;
    char * dtype, order = 'C';
    int ndim;
    int * shape;
    MemoryPointer ptr;
    size_t type_len;
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_ZVAL(zshape)
        Z_PARAM_OPTIONAL
        Z_PARAM_STRING(dtype, type_len)
    ZEND_PARSE_PARAMETERS_END();
    if(ZEND_NUM_ARGS() == 1) {
        dtype = emalloc(sizeof(char));
        *dtype = 'd';
    }

    shape = ZVAL_TO_TUPLE(zshape, &ndim);
    CArray_Ones(shape, ndim, dtype, &order, &ptr);
    efree(shape);
    if(ZEND_NUM_ARGS() == 1) {
        efree(dtype);
    }
    RETURN_MEMORYPOINTER(return_value, &ptr);
}

/**
 * ARITHMETICS
 */
PHP_METHOD(CArray, add)
{
    MemoryPointer target1_ptr, target2_ptr, result_ptr;
    zval * target1, * target2;
    CArray * target_ca1, * target_ca2, * output_ca;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ZVAL(target1)
        Z_PARAM_ZVAL(target2)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target1, &target1_ptr, NULL);
    ZVAL_TO_MEMORYPOINTER(target2, &target2_ptr, NULL);
    target_ca1 = CArray_FromMemoryPointer(&target1_ptr);
    target_ca2 = CArray_FromMemoryPointer(&target2_ptr);
    output_ca = CArray_Add(target_ca1, target_ca2, &result_ptr);

    FREE_FROM_MEMORYPOINTER(&target1_ptr);
    FREE_FROM_MEMORYPOINTER(&target2_ptr);
    if (output_ca != NULL) {
        RETURN_MEMORYPOINTER(return_value, &result_ptr);
    }
}
PHP_METHOD(CArray, subtract)
{
    MemoryPointer target1_ptr, target2_ptr, result_ptr;
    zval * target1, * target2;
    CArray * target_ca1, * target_ca2, * output_ca;
    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target1)
            Z_PARAM_ZVAL(target2)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target1, &target1_ptr, NULL);
    ZVAL_TO_MEMORYPOINTER(target2, &target2_ptr, NULL);
    target_ca1 = CArray_FromMemoryPointer(&target1_ptr);
    target_ca2 = CArray_FromMemoryPointer(&target2_ptr);
    output_ca = CArray_Subtract(target_ca1, target_ca2, &result_ptr);


    FREE_FROM_MEMORYPOINTER(&target1_ptr);
    FREE_FROM_MEMORYPOINTER(&target2_ptr);
    if (output_ca != NULL) {
        RETURN_MEMORYPOINTER(return_value, &result_ptr);
    }
}
PHP_METHOD(CArray, multiply)
{
    MemoryPointer target1_ptr, target2_ptr, result_ptr;
    zval * target1, * target2;
    CArray * target_ca1, * target_ca2, * output_ca;
    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target1)
            Z_PARAM_ZVAL(target2)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target1, &target1_ptr, NULL);
    ZVAL_TO_MEMORYPOINTER(target2, &target2_ptr, NULL);
    target_ca1 = CArray_FromMemoryPointer(&target1_ptr);
    target_ca2 = CArray_FromMemoryPointer(&target2_ptr);
    output_ca = CArray_Multiply(target_ca1, target_ca2, &result_ptr);


    FREE_FROM_MEMORYPOINTER(&target1_ptr);
    FREE_FROM_MEMORYPOINTER(&target2_ptr);
    if (output_ca != NULL) {
        RETURN_MEMORYPOINTER(return_value, &result_ptr);
    }
}
PHP_METHOD(CArray, divide)
{
    MemoryPointer target1_ptr, target2_ptr, result_ptr;
    zval * target1, * target2;
    CArray * target_ca1, * target_ca2, * output_ca;
    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target1)
            Z_PARAM_ZVAL(target2)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target1, &target1_ptr, NULL);
    ZVAL_TO_MEMORYPOINTER(target2, &target2_ptr, NULL);

    target_ca1 = CArray_FromMemoryPointer(&target1_ptr);
    target_ca2 = CArray_FromMemoryPointer(&target2_ptr);
    output_ca = CArray_Divide(target_ca1, target_ca2, &result_ptr);


    FREE_FROM_MEMORYPOINTER(&target1_ptr);
    FREE_FROM_MEMORYPOINTER(&target2_ptr);
    if (output_ca != NULL) {
        RETURN_MEMORYPOINTER(return_value, &result_ptr);
    }
}
PHP_METHOD(CArray, power)
{
    MemoryPointer target1_ptr, target2_ptr, result_ptr;
    zval * target1, * target2;
    CArray * target_ca1, * target_ca2, * output_ca;
    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target1)
            Z_PARAM_ZVAL(target2)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target1, &target1_ptr, NULL);
    ZVAL_TO_MEMORYPOINTER(target2, &target2_ptr, NULL);
    target_ca1 = CArray_FromMemoryPointer(&target1_ptr);
    target_ca2 = CArray_FromMemoryPointer(&target2_ptr);
    output_ca = CArray_Power(target_ca1, target_ca2, &result_ptr);


    FREE_FROM_MEMORYPOINTER(&target1_ptr);
    FREE_FROM_MEMORYPOINTER(&target2_ptr);
    if (output_ca != NULL) {
        RETURN_MEMORYPOINTER(return_value, &result_ptr);
    }
}
PHP_METHOD(CArray, mod)
{
    MemoryPointer target1_ptr, target2_ptr, result_ptr;
    zval * target1, * target2;
    CArray * target_ca1, * target_ca2, * output_ca;
    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target1)
            Z_PARAM_ZVAL(target2)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target1, &target1_ptr, NULL);
    ZVAL_TO_MEMORYPOINTER(target2, &target2_ptr, NULL);
    target_ca1 = CArray_FromMemoryPointer(&target1_ptr);
    target_ca2 = CArray_FromMemoryPointer(&target2_ptr);
    output_ca = CArray_Mod(target_ca1, target_ca2, &result_ptr);


    FREE_FROM_MEMORYPOINTER(&target1_ptr);
    FREE_FROM_MEMORYPOINTER(&target2_ptr);
    if (output_ca != NULL) {
        RETURN_MEMORYPOINTER(return_value, &result_ptr);
    }
}
PHP_METHOD(CArray, fmod)
{
    MemoryPointer target1_ptr, target2_ptr, result_ptr;
    zval * target1, * target2;
    CArray * target_ca1, * target_ca2, * output_ca;
    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target1)
            Z_PARAM_ZVAL(target2)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target1, &target1_ptr, NULL);
    ZVAL_TO_MEMORYPOINTER(target2, &target2_ptr, NULL);
    target_ca1 = CArray_FromMemoryPointer(&target1_ptr);
    target_ca2 = CArray_FromMemoryPointer(&target2_ptr);
    output_ca = CArray_Mod(target_ca1, target_ca2, &result_ptr);


    FREE_FROM_MEMORYPOINTER(&target1_ptr);
    FREE_FROM_MEMORYPOINTER(&target2_ptr);
    if (output_ca != NULL) {
        RETURN_MEMORYPOINTER(return_value, &result_ptr);
    }
}
PHP_METHOD(CArray, remainder)
{
    MemoryPointer target1_ptr, target2_ptr, result_ptr;
    zval * target1, * target2;
    CArray * target_ca1, * target_ca2, * output_ca;
    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target1)
            Z_PARAM_ZVAL(target2)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target1, &target1_ptr, NULL);
    ZVAL_TO_MEMORYPOINTER(target2, &target2_ptr, NULL);
    target_ca1 = CArray_FromMemoryPointer(&target1_ptr);
    target_ca2 = CArray_FromMemoryPointer(&target2_ptr);
    output_ca = CArray_Mod(target_ca1, target_ca2, &result_ptr);


    FREE_FROM_MEMORYPOINTER(&target1_ptr);
    FREE_FROM_MEMORYPOINTER(&target2_ptr);
    if (output_ca != NULL) {
        RETURN_MEMORYPOINTER(return_value, &result_ptr);
    }
}
PHP_METHOD(CArray, prod)
{
    zval * target;
    long axis;
    int * axis_p;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_ptr;
    ZEND_PARSE_PARAMETERS_START(1, 2)
            Z_PARAM_ZVAL(target)
            Z_PARAM_OPTIONAL
            Z_PARAM_LONG(axis)
    ZEND_PARSE_PARAMETERS_END();
    if(ZEND_NUM_ARGS() == 1) {
        axis_p = NULL;
    }
    if(ZEND_NUM_ARGS() > 1) {
        axis_p = (int*)emalloc(sizeof(int));
        *axis_p = axis;
    }
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Prod(target_ca, axis_p, target_ca->descriptor->type_num, &rtn_ptr);

    if (ret == NULL) {
		return;
	}
    efree(axis_p);
    CArray_DECREF(target_ca);
    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

PHP_METHOD(CArray, cumprod)
{
    zval * target;
    long axis;
    int * axis_p;
    CArray * ret, * target_ca;
    MemoryPointer ptr;
    ZEND_PARSE_PARAMETERS_START(1, 2)
            Z_PARAM_ZVAL(target)
            Z_PARAM_OPTIONAL
            Z_PARAM_LONG(axis)
    ZEND_PARSE_PARAMETERS_END();
    if(ZEND_NUM_ARGS() == 1) {
        axis_p = NULL;
    }
    if(ZEND_NUM_ARGS() > 1) {
        axis_p = (int*)emalloc(sizeof(int));
        *axis_p = axis;
    }
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_CumProd(target_ca, axis_p, target_ca->descriptor->type_num, &ptr);
    if (ret == NULL) {
		return;
	}
    efree(axis_p);
    RETURN_MEMORYPOINTER(return_value, &ptr);
}
PHP_METHOD(CArray, cumsum)
{
    zval * target;
    long axis;
    int * axis_p;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_ptr;
    ZEND_PARSE_PARAMETERS_START(1, 2)
            Z_PARAM_ZVAL(target)
            Z_PARAM_OPTIONAL
            Z_PARAM_LONG(axis)
    ZEND_PARSE_PARAMETERS_END();
    if(ZEND_NUM_ARGS() == 1) {
        axis_p = NULL;
    }
    if(ZEND_NUM_ARGS() > 1) {
        axis_p = (int*)emalloc(sizeof(int));
        *axis_p = axis;
    }
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_CumSum(target_ca, axis_p, target_ca->descriptor->type_num, &rtn_ptr);
    efree(axis_p);

    if (ret == NULL) {
        return;
    }
    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}
PHP_METHOD(CArray, negative)
{
    MemoryPointer out, rtn_ptr;
    CArray * target_ca, * rtn_ca;
    zval * target;
    ZEND_PARSE_PARAMETERS_START(1, 1)
         Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &out, NULL);
    target_ca = CArray_FromMemoryPointer(&out);
    rtn_ca = CArray_Negative(target_ca, &rtn_ptr);
    if (rtn_ca == NULL) {
        return;
    }

    FREE_FROM_MEMORYPOINTER(&out);
    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}
PHP_METHOD(CArray, sqrt)
{
    MemoryPointer target_ptr, rtn_ptr;
    CArray * target_ca, * rtn_ca;
    zval * target;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &target_ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&target_ptr);
    rtn_ca = CArray_Sqrt(target_ca, &rtn_ptr);

    if (rtn_ca == NULL) {
        return;
    }

    FREE_FROM_MEMORYPOINTER(&target_ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}
PHP_METHOD(CArray, reciprocal)
{
    MemoryPointer target_ptr, rtn_ptr;
    CArray * target_ca, * rtn_ca;
    zval * target;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &target_ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&target_ptr);
    rtn_ca = CArray_Reciprocal(target_ca, &rtn_ptr);

    if (rtn_ca == NULL) {
        return;
    }

    FREE_FROM_MEMORYPOINTER(&target_ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * ROUNDING
 */
PHP_METHOD(CArray, ceil)
{
    MemoryPointer target_ptr, rtn_ptr;
    CArray * target_ca, * rtn_ca;
    zval * target;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &target_ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&target_ptr);
    rtn_ca = CArray_Ceil(target_ca, &rtn_ptr);

    if (rtn_ca == NULL) {
        return;
    }

    FREE_FROM_MEMORYPOINTER(&target_ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}
PHP_METHOD(CArray, floor)
{
    MemoryPointer target_ptr, rtn_ptr;
    CArray * target_ca, * rtn_ca;
    zval * target;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &target_ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&target_ptr);
    rtn_ca = CArray_Floor(target_ca, &rtn_ptr);

    if (rtn_ca == NULL) {
        return;
    }

    FREE_FROM_MEMORYPOINTER(&target_ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}
PHP_METHOD(CArray, around)
{
    MemoryPointer target_ptr, rtn_ptr;
    CArray * target_ca, * rtn_ca;
    zval * target;
    long decimals;
    ZEND_PARSE_PARAMETERS_START(1, 2)
            Z_PARAM_ZVAL(target)
            Z_PARAM_OPTIONAL
            Z_PARAM_LONG(decimals)
    ZEND_PARSE_PARAMETERS_END();
    if(ZEND_NUM_ARGS() == 1) {
        decimals = 0;
    }

    ZVAL_TO_MEMORYPOINTER(target, &target_ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&target_ptr);
    rtn_ca = CArray_Round(target_ca, (int)decimals, &rtn_ptr);

    if (rtn_ca == NULL) {
        return;
    }

    if (target_ptr.free == 1 || target_ptr.free == 2) {
        CArrayDescriptor_INCREF(CArray_DESCR(rtn_ca));
    }

    FREE_FROM_MEMORYPOINTER(&target_ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}


/**
 * STATISTICS
 */
PHP_METHOD(CArray, correlate)
{
    MemoryPointer out, a_ptr, v_ptr;
    CArray * a_ca, * rtn_ca, * v_ca;
    zval * a, * v;
    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(a)
            Z_PARAM_ZVAL(v)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(a, &a_ptr, NULL);
    ZVAL_TO_MEMORYPOINTER(v, &v_ptr, NULL);
    a_ca = CArray_FromMemoryPointer(&a_ptr);
    v_ca = CArray_FromMemoryPointer(&v_ptr);

    rtn_ca = CArray_Correlate2(a_ca, v_ca, 0, &out);

    if (rtn_ca == NULL) {
        return;
    }

    FREE_FROM_MEMORYPOINTER(&a_ptr);
    FREE_FROM_MEMORYPOINTER(&v_ptr);
    RETURN_MEMORYPOINTER(return_value, &out);
}


/**
 * INDEXING ROUTINES
 */
PHP_METHOD(CArray, diagonal)
{
    MemoryPointer a_ptr, rtn_ptr;
    CArray * target_array;
    zval * a;
    long axis1, axis2, offset;
    ZEND_PARSE_PARAMETERS_START(1, 4)
        Z_PARAM_ZVAL(a)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(offset)
        Z_PARAM_LONG(axis1)
        Z_PARAM_LONG(axis2)
    ZEND_PARSE_PARAMETERS_END();

    if(ZEND_NUM_ARGS() == 1) {
        offset = 0;
        axis1 = 0;
        axis2 = 1;
    }
    if(ZEND_NUM_ARGS() == 2) {
        axis1 = 0;
        axis2 = 1;
    }
    if(ZEND_NUM_ARGS() == 3) {
        axis2 = 1;
    }
    
    ZVAL_TO_MEMORYPOINTER(a, &a_ptr, NULL);
    target_array = CArray_FromMemoryPointer(&a_ptr);
    CArray * rtn_array = CArray_Diagonal(target_array, offset, axis1, axis2, &rtn_ptr);
    if(rtn_array == NULL) {
        return;
    }
    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}
PHP_METHOD(CArray, take)
{
    CArray * ca_a, * ca_indices, * out;
    MemoryPointer a_ptr, indices_ptr, out_ptr;
    zval * a, * indices;
    zend_long axis;
    ZEND_PARSE_PARAMETERS_START(2, 3)
        Z_PARAM_ZVAL(a)
        Z_PARAM_ZVAL(indices)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(axis)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(a, &a_ptr, NULL);
    ZVAL_TO_MEMORYPOINTER(indices, &indices_ptr, NULL);
    ca_a = CArray_FromMemoryPointer(&a_ptr);
    if(ZEND_NUM_ARGS() < 3) {
        axis = INT_MAX;
    }

    ca_indices = CArray_FromMemoryPointer(&indices_ptr);
    out = CArray_TakeFrom(ca_a, ca_indices, axis, &out_ptr, CARRAY_RAISE);

    if(out != NULL) {
        RETURN_MEMORYPOINTER(return_value, &out_ptr);
    }
}
PHP_METHOD(CArray, atleast_1d)
{
    zval * temp_zval;
    int i;
    CArray * target, * out_carray;
    MemoryPointer ptr, out;
    zval * dict;
    int dict_size;
    ZEND_PARSE_PARAMETERS_START(1, -1)
        Z_PARAM_VARIADIC('+', dict, dict_size)
    ZEND_PARSE_PARAMETERS_END();
    if (dict_size == 1) {
        ZVAL_TO_MEMORYPOINTER(&(dict[0]), &ptr, NULL);
        target = CArray_FromMemoryPointer(&ptr);
        out_carray = CArray_atleast1d(target, &out);
        CArrayDescriptor_INCREF(CArray_DESCR(out_carray));
        RETURN_MEMORYPOINTER(return_value, &out);
        FREE_FROM_MEMORYPOINTER(&ptr);
    } else {
        array_init_size(return_value, dict_size);
        for(i = 0; i < dict_size; i++) {
            ZVAL_TO_MEMORYPOINTER(&(dict[i]), &ptr, NULL);
            target = CArray_FromMemoryPointer(&ptr);
            out_carray = CArray_atleast1d(target, &out);
            temp_zval = MEMORYPOINTER_TO_ZVAL(&out);
            zend_hash_next_index_insert_new(Z_ARRVAL_P(return_value), temp_zval);
            if(ptr.free == 1) {
                efree(temp_zval);
                CArray_Alloc_FreeFromMemoryPointer(&ptr);
            }
            if(ptr.free == 2) {
                efree(temp_zval);
                CArrayDescriptor_INCREF(CArray_DESCR(target));
                CArray_Alloc_FreeFromMemoryPointer(&ptr);
            }
            if(!ptr.free) {
                CArrayDescriptor_DECREF(CArray_DESCR(target));
            }
        }
    }
}
PHP_METHOD(CArray, atleast_2d)
{
    zval * temp_zval;
    int i;
    CArray * target;
    MemoryPointer ptr, out;
    zval * dict;
    int dict_size;
    ZEND_PARSE_PARAMETERS_START(1, -1)
        Z_PARAM_VARIADIC('+', dict, dict_size)
    ZEND_PARSE_PARAMETERS_END();
    if (dict_size == 1) {
        ZVAL_TO_MEMORYPOINTER(&(dict[0]), &ptr, NULL);
        target = CArray_FromMemoryPointer(&ptr);
        CArray_atleast2d(target, &out);
        RETURN_MEMORYPOINTER(return_value, &out);
        if(ptr.free == 1) {
            CArray_Alloc_FreeFromMemoryPointer(&ptr);
        }
        if(ptr.free == 2) {
            CArrayDescriptor_DECREF(CArray_DESCR(target));
            CArray_Alloc_FreeFromMemoryPointer(&ptr);
        }
        if(!ptr.free) {
            CArrayDescriptor_DECREF(CArray_DESCR(target));
        }
    } else {
        array_init_size(return_value, dict_size);
        for(i = 0; i < dict_size; i++) {
            ZVAL_TO_MEMORYPOINTER(&(dict[i]), &ptr, NULL);
            target = CArray_FromMemoryPointer(&ptr);
            CArray_atleast2d(target, &out);
            temp_zval = MEMORYPOINTER_TO_ZVAL(&out);
            zend_hash_next_index_insert_new(Z_ARRVAL_P(return_value), temp_zval);
            if(ptr.free) {
                efree(temp_zval);
                CArray_Alloc_FreeFromMemoryPointer(&ptr);
            }
            if(!ptr.free) {
                efree(temp_zval);
                CArrayDescriptor_DECREF(CArray_DESCR(target));
            }
        }
    }
}
PHP_METHOD(CArray, atleast_3d)
{
    zval * temp_zval;
    int i;
    CArray * target, * out_carray;
    MemoryPointer ptr, out;
    zval * dict;
    int dict_size;
    ZEND_PARSE_PARAMETERS_START(1, -1)
        Z_PARAM_VARIADIC('+', dict, dict_size)
    ZEND_PARSE_PARAMETERS_END();
    if (dict_size == 1) {
        ZVAL_TO_MEMORYPOINTER(&(dict[0]), &ptr, NULL);
        target = CArray_FromMemoryPointer(&ptr);
        out_carray = CArray_atleast3d(target, &out);
        RETURN_MEMORYPOINTER(return_value, &out);
        if(ptr.free) {
            CArray_Alloc_FreeFromMemoryPointer(&ptr);
        }
        if(!ptr.free) {
            CArrayDescriptor_DECREF(CArray_DESCR(target));
        }
    } else {
        array_init_size(return_value, dict_size);
        for(i = 0; i < dict_size; i++) {
            ZVAL_TO_MEMORYPOINTER(&(dict[i]), &ptr, NULL);
            target = CArray_FromMemoryPointer(&ptr);
            out_carray = CArray_atleast3d(target, &out);
            temp_zval = MEMORYPOINTER_TO_ZVAL(&out);
            zend_hash_next_index_insert_new(Z_ARRVAL_P(return_value), temp_zval);
            if(ptr.free) {
                efree(temp_zval);
                CArray_Alloc_FreeFromMemoryPointer(&ptr);
            }
            if(!ptr.free) {
                efree(temp_zval);
                CArrayDescriptor_DECREF(CArray_DESCR(target));
            }
        }
    }
}
PHP_METHOD(CArray, squeeze)
{
    MemoryPointer a_ptr, out_ptr;
    CArray * target_array, * rtn_array;
    zval * a;
    zval * axis;
    int axis_i;
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_ZVAL(a)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL(axis)
    ZEND_PARSE_PARAMETERS_END();
    if(ZEND_NUM_ARGS() == 1 || Z_TYPE_P(axis) == IS_NULL) {
        axis_i = INT_MAX;
    }
    if(ZEND_NUM_ARGS() > 1 && Z_TYPE_P(axis) != IS_LONG && Z_TYPE_P(axis) != IS_NULL) {
        throw_valueerror_exception("axis must be either NULL or LONG");
        return;
    } else if (ZEND_NUM_ARGS() > 1) {
        convert_to_long(axis);
        axis_i = zval_get_long(axis);
    }

    ZVAL_TO_MEMORYPOINTER(a, &a_ptr, NULL);
    target_array = CArray_FromMemoryPointer(&a_ptr);

    if (ZEND_NUM_ARGS() > 1 && Z_TYPE_P(axis) != IS_NULL) {
        rtn_array = CArray_Squeeze(target_array, &axis_i, &out_ptr);
    } else {
        rtn_array = CArray_Squeeze(target_array, NULL, &out_ptr);
    }

    FREE_FROM_MEMORYPOINTER(&a_ptr);
    if(rtn_array != NULL) {
        RETURN_MEMORYPOINTER(return_value, &out_ptr);
    }
}
PHP_METHOD(CArray, expand_dims)
{
    zval * a;
    long axis;
    CArray * target, * rtn;
    MemoryPointer target_ptr, out;
    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(a)
            Z_PARAM_LONG(axis)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(a, &target_ptr, NULL);
    target = CArray_FromMemoryPointer(&target_ptr);

    rtn = CArray_ExpandDims(target, (int)axis, &out);

    if (rtn == NULL) {
        return;
    }

    FREE_FROM_MEMORYPOINTER(&target_ptr);
    RETURN_MEMORYPOINTER(return_value, &out);
}

/**
 * MANIPULATION ROUTINES
 */
PHP_METHOD(CArray, swapaxes)
{
    MemoryPointer a_ptr;
    CArray * target_array;
    zval * a;
    long axis1, axis2;
    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_ZVAL(a)
        Z_PARAM_LONG(axis1)
        Z_PARAM_LONG(axis2)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(a, &a_ptr, NULL);
    target_array = CArray_FromMemoryPointer(&a_ptr);
    CArray_SwapAxes(target_array, (int)axis1, (int)axis2, &a_ptr);
    RETURN_MEMORYPOINTER(return_value, &a_ptr);
}
PHP_METHOD(CArray, rollaxis)
{
    MemoryPointer a_ptr;
    CArray * target_array;
    zval * a;
    long axis, start;
    ZEND_PARSE_PARAMETERS_START(2, 3)
            Z_PARAM_ZVAL(a)
            Z_PARAM_LONG(axis)
            Z_PARAM_OPTIONAL
            Z_PARAM_LONG(start)
    ZEND_PARSE_PARAMETERS_END();
    if(ZEND_NUM_ARGS() == 2) {
        start = 0;
    }
    ZVAL_TO_MEMORYPOINTER(a, &a_ptr, NULL);
    target_array = CArray_FromMemoryPointer(&a_ptr);
    CArray_Rollaxis(target_array, (int)axis, (int)start, &a_ptr);
    RETURN_MEMORYPOINTER(return_value, &a_ptr);
}
PHP_METHOD(CArray, flip)
{
    MemoryPointer a_ptr, out_ptr;
    CArray * target_array, *rtn;
    zval * a, * axis;
    int axis_p, start;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(a)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(a, &a_ptr, NULL);
    target_array = CArray_FromMemoryPointer(&a_ptr);

    if(ZEND_NUM_ARGS() == 1) {
        rtn = CArray_Flip(target_array, NULL, &out_ptr);
    }

    FREE_FROM_MEMORYPOINTER(&a_ptr);
    RETURN_MEMORYPOINTER(return_value, &out_ptr);
}
PHP_METHOD(CArray, moveaxis)
{
    MemoryPointer a_ptr, src_ptr, dst_ptr, out_ptr;
    zval * a, * source, * destination;
    CArray * a_array, * src_array, * dst_array;
    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_ZVAL(a)
        Z_PARAM_ZVAL(source)
        Z_PARAM_ZVAL(destination)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(a, &a_ptr, NULL);
    ZVAL_TO_MEMORYPOINTER(source, &src_ptr, NULL);
    ZVAL_TO_MEMORYPOINTER(destination, &dst_ptr, NULL);

    a_array = CArray_FromMemoryPointer(&a_ptr);
    src_array = CArray_FromMemoryPointer(&src_ptr);
    dst_array = CArray_FromMemoryPointer(&dst_ptr);

    a_array = CArray_Moveaxis(a_array, src_array, dst_array, &out_ptr);

    FREE_FROM_MEMORYPOINTER(&src_ptr);
    FREE_FROM_MEMORYPOINTER(&dst_ptr);

    if (a_array != NULL) {
        RETURN_MEMORYPOINTER(return_value, &out_ptr);
    }
}
PHP_METHOD(CArray, concatenate)
{
    int i = 0;
    zval * array_of_zvals, * element;
    zval * axis;
    int * axis_p = NULL;
    MemoryPointer * ptrs, result_ptr;
    CArray ** arrays, * rtn_array;
    ZEND_PARSE_PARAMETERS_START(1, 2)
            Z_PARAM_ARRAY(array_of_zvals)
            Z_PARAM_OPTIONAL
            Z_PARAM_ZVAL(axis)
    ZEND_PARSE_PARAMETERS_END();

    if(ZEND_NUM_ARGS() == 2) {
        axis_p = emalloc(sizeof(int));
        if (Z_TYPE_P(axis) != IS_LONG && Z_TYPE_P(axis) != IS_NULL) {
            throw_axis_exception("axis must be an integer");
            return;
        }
        if (Z_TYPE_P(axis) == IS_LONG) {
            convert_to_long(axis);
            *axis_p = zval_get_long(axis);
        }
    } else {
        axis_p = emalloc(sizeof(int));
        *axis_p = 0;
    }

    int count_objects = zend_array_count(Z_ARRVAL_P(array_of_zvals));
    ptrs = emalloc(count_objects * sizeof(MemoryPointer));

    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(array_of_zvals), element) {
                ZVAL_TO_MEMORYPOINTER(element, ptrs + i, NULL);
                i++;
            } ZEND_HASH_FOREACH_END();

    arrays = emalloc(count_objects * sizeof(CArray *));
    for (i = 0; i < count_objects; i++) {
        arrays[i] = CArray_FromMemoryPointer(ptrs + i);
    }

    rtn_array = CArray_Concatenate(arrays, count_objects, axis_p, &result_ptr);

    for (i = 0; i < count_objects; i++) {
        FREE_FROM_MEMORYPOINTER(ptrs + i);
    }
    efree(ptrs);
    efree(arrays);
    if(ZEND_NUM_ARGS() == 2) {
        efree(axis_p);
    }

    if (rtn_array == NULL) {
        return;
    }
    RETURN_MEMORYPOINTER(return_value, &result_ptr);
}


/**
 * NUMERICAL RANGES
 */
PHP_METHOD(CArray, arange)
{
    MemoryPointer a_ptr;
    CArray * target_array;
    double start, stop, step_d;
    int typenum;
    zval * start_stop, * stop_start = NULL, * step = NULL;
    char * dtype = NULL;
    size_t type_len;
    ZEND_PARSE_PARAMETERS_START(1, 4)
        Z_PARAM_ZVAL(start_stop)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL(stop_start)
        Z_PARAM_ZVAL(step)
        Z_PARAM_STRING(dtype, type_len)
    ZEND_PARSE_PARAMETERS_END();
    
    if(ZEND_NUM_ARGS() == 1) {
        convert_to_double(start_stop);
        if (UNEXPECTED(EXPECTED(zval_get_double(start_stop) > 0x7fffffff))) {
            throw_valueerror_exception("Too many elements.");
            RETURN_FALSE;
        }
        start = (double)0.00;
        stop  = (double)zval_get_double(start_stop);
        typenum = TYPE_DEFAULT_INT;
        step_d = 1.00;
    }
    if(ZEND_NUM_ARGS() == 2) {
        convert_to_double(start_stop);
        convert_to_double(stop_start);
        if (UNEXPECTED(EXPECTED(zval_get_double(start_stop) > 0x7fffffff)) ||
            UNEXPECTED(EXPECTED(zval_get_double(stop_start) > 0x7fffffff))) {
            throw_valueerror_exception("Too many elements.");
            RETURN_FALSE;
        }
        start = (double)zval_get_double(start_stop);
        stop  = (double)zval_get_double(stop_start);
        typenum = TYPE_DEFAULT_INT;
        step_d = 1.00;
    }
    if(ZEND_NUM_ARGS() == 3) {
        convert_to_double(start_stop);
        convert_to_double(stop_start);
        convert_to_double(step);
        if (UNEXPECTED(EXPECTED(zval_get_double(start_stop) > 0x7fffffff)) ||
            UNEXPECTED(EXPECTED(zval_get_double(stop_start) > 0x7fffffff)) ||
            UNEXPECTED(EXPECTED(zval_get_double(step) > 0x7fffffff))) {
            throw_valueerror_exception("Too many elements.");
            RETURN_FALSE;
        }
        start = (double)zval_get_double(start_stop);
        stop  = (double)zval_get_double(stop_start);
        step_d  = (double)zval_get_double(step);
        typenum = TYPE_DEFAULT_INT;
    }
    if(ZEND_NUM_ARGS() == 4) {
        convert_to_double(start_stop);
        convert_to_double(stop_start);
        convert_to_double(step);
        if (UNEXPECTED(EXPECTED(zval_get_double(start_stop) > 0x7fffffff)) ||
            UNEXPECTED(EXPECTED(zval_get_double(stop_start) > 0x7fffffff)) ||
            UNEXPECTED(EXPECTED(zval_get_double(step) > 0x7fffffff))) {
            throw_valueerror_exception("Too many elements.");
            RETURN_FALSE;
        }
        start = (double)zval_get_double(start_stop);
        stop  = (double)zval_get_double(stop_start);
        step_d  = (double)zval_get_double(step);
        typenum = CHAR_TYPE_INT(dtype[0]);
    }
    target_array = CArray_Arange(start, stop, step_d, typenum , &a_ptr);
    RETURN_MEMORYPOINTER(return_value, &a_ptr);
}
PHP_METHOD(CArray, linspace)
{
    long num_samples;
    int num;
    zend_bool endpoint;
    size_t type_len;
    CArray * ret;
    zval * start, * stop;
    double start_d = 0, stop_d = 0;
    char * typestr = NULL;
    int type_num, axis;
    MemoryPointer out;
    zend_long axis_l;

    ZEND_PARSE_PARAMETERS_START(2, 7)
            Z_PARAM_ZVAL(start)
            Z_PARAM_ZVAL(stop)
            Z_PARAM_OPTIONAL
            Z_PARAM_LONG(num_samples)
            Z_PARAM_BOOL(endpoint)
            //Z_PARAM_LONG(axis_l)
            Z_PARAM_STRING(typestr, type_len)
    ZEND_PARSE_PARAMETERS_END();
    if(ZEND_NUM_ARGS() == 2) {
        convert_to_double(start);
        convert_to_double(stop);
        start_d = (double)zval_get_double(start);
        stop_d = (double)zval_get_double(stop);
        num = 50;
        type_num = TYPE_DOUBLE_INT;
        endpoint = 1;
        axis = 0;
    }
    if(ZEND_NUM_ARGS() == 3) {
        convert_to_double(start);
        convert_to_double(stop);
        start_d = (double)zval_get_double(start);
        stop_d = (double)zval_get_double(stop);
        num = (int)num_samples;
        type_num = TYPE_DOUBLE_INT;
        endpoint = 1;
        axis = 0;
    }
    if(ZEND_NUM_ARGS() == 4) {
        convert_to_double(start);
        convert_to_double(stop);
        start_d = (double)zval_get_double(start);
        stop_d = (double)zval_get_double(stop);
        num = (int)num_samples;
        type_num = TYPE_DOUBLE_INT;
        axis = 0;
    }
    if(ZEND_NUM_ARGS() == 5) {
        convert_to_double(start);
        convert_to_double(stop);
        start_d = (double)zval_get_double(start);
        stop_d = (double)zval_get_double(stop);
        num = (int)num_samples;
        type_num = TYPESTR_TO_INT(typestr);
        axis = (int)0;
    }

    ret = CArray_Linspace(start_d, stop_d, num, endpoint, 1, &axis, type_num, &out);

    RETURN_MEMORYPOINTER(return_value, &out);
}
PHP_METHOD(CArray, logspace)
{
    long num_samples;
    int num;
    zend_bool endpoint;
    size_t type_len;
    CArray * ret;
    zval * start, * stop;
    double start_d, stop_d, base;
    char * typestr;
    int type_num;
    MemoryPointer out;

    ZEND_PARSE_PARAMETERS_START(2, 7)
            Z_PARAM_ZVAL(start)
            Z_PARAM_ZVAL(stop)
            Z_PARAM_OPTIONAL
            Z_PARAM_LONG(num_samples)
            Z_PARAM_BOOL(endpoint)
            Z_PARAM_DOUBLE(base)
            Z_PARAM_STRING(typestr, type_len)
    ZEND_PARSE_PARAMETERS_END();

    if(ZEND_NUM_ARGS() == 2) {
        convert_to_double(start);
        convert_to_double(stop);
        start_d = (double)zval_get_double(start);
        stop_d = (double)zval_get_double(stop);
        num = 50;
        type_num = TYPE_DOUBLE_INT;
        endpoint = 1;
        base = 10.00;
    }
    if(ZEND_NUM_ARGS() == 3) {
        convert_to_double(start);
        convert_to_double(stop);
        start_d = (double)zval_get_double(start);
        stop_d = (double)zval_get_double(stop);
        num = (int)num_samples;
        type_num = TYPE_DOUBLE_INT;
        endpoint = 1;
        base = 10.00;
    }
    if(ZEND_NUM_ARGS() == 4) {
        convert_to_double(start);
        convert_to_double(stop);
        start_d = (double)zval_get_double(start);
        stop_d = (double)zval_get_double(stop);
        num = (int)num_samples;
        type_num = TYPE_DOUBLE_INT;
        base = 10.00;
    }
    if(ZEND_NUM_ARGS() == 5) {
        convert_to_double(start);
        convert_to_double(stop);
        start_d = (double)zval_get_double(start);
        stop_d = (double)zval_get_double(stop);
        num = (int)num_samples;
        type_num = TYPE_DOUBLE_INT;
    }
    if(ZEND_NUM_ARGS() == 6) {
        convert_to_double(start);
        convert_to_double(stop);
        start_d = (double)zval_get_double(start);
        stop_d = (double)zval_get_double(stop);
        num = (int)num_samples;
        type_num = TYPESTR_TO_INT(typestr);
    }

    ret = CArray_Logspace(start_d, stop_d, num, endpoint, base, type_num, &out);

    RETURN_MEMORYPOINTER(return_value, &out);
}

PHP_METHOD(CArray, geomspace)
{
    long num_samples;
    int num;
    zend_bool endpoint;
    size_t type_len;
    CArray * ret;
    zval * start, * stop;
    double start_d, stop_d;
    char * typestr;
    int type_num;
    MemoryPointer out;

    ZEND_PARSE_PARAMETERS_START(2, 7)
            Z_PARAM_ZVAL(start)
            Z_PARAM_ZVAL(stop)
            Z_PARAM_OPTIONAL
            Z_PARAM_LONG(num_samples)
            Z_PARAM_BOOL(endpoint)
            Z_PARAM_STRING(typestr, type_len)
    ZEND_PARSE_PARAMETERS_END();

    if(ZEND_NUM_ARGS() == 2) {
        convert_to_double(start);
        convert_to_double(stop);
        start_d = (double)zval_get_double(start);
        stop_d = (double)zval_get_double(stop);
        num = 50;
        type_num = TYPE_DOUBLE_INT;
        endpoint = 1;
    }
    if(ZEND_NUM_ARGS() == 3) {
        convert_to_double(start);
        convert_to_double(stop);
        start_d = (double)zval_get_double(start);
        stop_d = (double)zval_get_double(stop);
        num = (int)num_samples;
        type_num = TYPE_DOUBLE_INT;
        endpoint = 1;
    }
    if(ZEND_NUM_ARGS() == 4) {
        convert_to_double(start);
        convert_to_double(stop);
        start_d = (double)zval_get_double(start);
        stop_d = (double)zval_get_double(stop);
        num = (int)num_samples;
        type_num = TYPE_DOUBLE_INT;
    }
    if(ZEND_NUM_ARGS() == 5) {
        convert_to_double(start);
        convert_to_double(stop);
        start_d = (double)zval_get_double(start);
        stop_d = (double)zval_get_double(stop);
        num = (int)num_samples;
        type_num = TYPE_DOUBLE_INT;
    }
    if(ZEND_NUM_ARGS() == 6) {
        convert_to_double(start);
        convert_to_double(stop);
        start_d = (double)zval_get_double(start);
        stop_d = (double)zval_get_double(stop);
        num = (int)num_samples;
        type_num = TYPESTR_TO_INT(typestr);
    }

    ret = CArray_Geomspace(start_d, stop_d, num, endpoint, type_num, &out);

    RETURN_MEMORYPOINTER(return_value, &out);
}

/**
 * RANDOM
 **/ 
PHP_METHOD(CArray, rand)
{
    zval * size;
    int len, *dims;
    MemoryPointer out;
    ZEND_PARSE_PARAMETERS_START(1, 1)
       Z_PARAM_ARRAY(size)
    ZEND_PARSE_PARAMETERS_END();
    dims = ZVAL_TO_TUPLE(size, &len);
    CArray_Rand(dims, len, &out);
    RETURN_MEMORYPOINTER(return_value, &out);
    FREE_TUPLE(dims);
}
PHP_METHOD(CArray, poisson)
{
    zval * size;
    int len, *dims;
    double lambda;
    MemoryPointer out;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY(size)
        Z_PARAM_DOUBLE(lambda)
    ZEND_PARSE_PARAMETERS_END();
    dims = ZVAL_TO_TUPLE(size, &len);
    CArray_Poisson(dims, lambda, &out);
    RETURN_MEMORYPOINTER(return_value, &out);
    FREE_TUPLE(dims);
}


/**
 * MISC
 **/ 
PHP_METHOD(CArray, fill)
{
    zval * obj = getThis();
    zval * scalar_obj;
    CArrayScalar * scalar;
    MemoryPointer ptr;
    CArray * target_ca;
    ZVAL_TO_MEMORYPOINTER(obj, &ptr, NULL);
    ZEND_PARSE_PARAMETERS_START(1, 1)
       Z_PARAM_ZVAL(scalar_obj)
    ZEND_PARSE_PARAMETERS_END();
    if(Z_TYPE_P(scalar_obj) == IS_LONG) {
        convert_to_long(scalar_obj);
        scalar = CArrayScalar_NewInt((int)zval_get_long(scalar_obj));
    }
    if(Z_TYPE_P(scalar_obj) == IS_DOUBLE) {
        convert_to_double(scalar_obj);
        scalar = CArrayScalar_NewDouble(zval_get_double(scalar_obj));
    }
    target_ca = CArray_FromMemoryPointer(&ptr);
    CArray_FillWithScalar(target_ca, scalar);
    CArrayScalar_FREE(scalar);
}
PHP_METHOD(CArray, clip)
{
    MemoryPointer ptr_a, ptr_min, ptr_max, ptr_rtn;
    CArray * ca_a, * ca_min = NULL, * ca_max = NULL, * rtn = NULL;
    zval * a, * a_min, * a_max;
    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_ZVAL(a)
        Z_PARAM_ZVAL(a_min)
        Z_PARAM_ZVAL(a_max)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(a_min, &ptr_min, NULL);
    ZVAL_TO_MEMORYPOINTER(a_max, &ptr_max, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);

    if (Z_TYPE_P(a_min) != IS_NULL) {
        ca_min = CArray_FromMemoryPointer(&ptr_min);
    }
    if (Z_TYPE_P(a_max) != IS_NULL) {
        ca_max = CArray_FromMemoryPointer(&ptr_max);
    }

    rtn = CArray_Clip(ca_a, ca_min, ca_max, &ptr_rtn);

    FREE_FROM_MEMORYPOINTER(&ptr_a);

    if (Z_TYPE_P(a_min) != IS_NULL) {
        FREE_FROM_MEMORYPOINTER(&ptr_min);
    }

    if (Z_TYPE_P(a_max) != IS_NULL) {
        FREE_FROM_MEMORYPOINTER(&ptr_max);
    }

    if (rtn == NULL) {
        return;
    }

    RETURN_MEMORYPOINTER(return_value, &ptr_rtn);
}
PHP_METHOD(CArray, convolve)
{
    MemoryPointer out, a_ptr, v_ptr;
    CArray * a_ca, * rtn_ca, * v_ca, *v_ca_flipped;
    zval * a, * v;
    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(a)
            Z_PARAM_ZVAL(v)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(a, &a_ptr, NULL);
    ZVAL_TO_MEMORYPOINTER(v, &v_ptr, NULL);
    a_ca = CArray_FromMemoryPointer(&a_ptr);
    v_ca = CArray_FromMemoryPointer(&v_ptr);

    v_ca_flipped = CArray_Flip(v_ca, NULL, NULL);
    rtn_ca = CArray_Correlate(a_ca, v_ca_flipped, 2, &out);

    if (rtn_ca == NULL) {
        return;
    }

    CArray_Free(v_ca_flipped);
    CArrayDescriptor_DECREF(CArray_DESCR(v_ca));
    FREE_FROM_MEMORYPOINTER(&a_ptr);
    FREE_FROM_MEMORYPOINTER(&v_ptr);
    RETURN_MEMORYPOINTER(return_value, &out);
}


/**
 * LOGICAL FUNCTIONS
 */
PHP_METHOD(CArray, any)
{

}
PHP_METHOD(CArray, all)
{

}


PHP_METHOD(CArray, __toString)
{
    CArray * target_ca;
    MemoryPointer ptr;
    zval * obj = getThis();
    zend_string *str = zend_string_init(" ", 1, 0);

    ZVAL_TO_MEMORYPOINTER(obj, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    CArray_Print(target_ca, 0);

    ZVAL_STR(return_value, str);
}
PHP_METHOD(CArray, toArray)
{
    CArray * target_ca;
    MemoryPointer ptr;
    zval * obj = getThis();

    ZVAL_TO_MEMORYPOINTER(obj, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    CArray_ToArray(target_ca, return_value);
}
PHP_METHOD(CArray, map)
{
    int i;
    zval result;
    CArray * target_ca, * ret_ca;
    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fci_cache = empty_fcall_info_cache;
    zval * obj = getThis();
    zval * tmp;
    MemoryPointer ptr, target_ptr;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_FUNC_EX(fci, fci_cache, 1, 0)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(obj, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    array_init_size(return_value, CArray_DIMS(target_ca)[0]);

    zval *params = (zval *)safe_emalloc(CArray_DIMS(target_ca)[0], sizeof(zval), 0);
    for (i = 0; i < CArray_DIMS(target_ca)[0]; i++) {
        ret_ca = (CArray *) CArray_Slice_Index(target_ca, i, &target_ptr);

        if (ret_ca == NULL) {
            return;
        }

        tmp = MEMORYPOINTER_TO_ZVAL(&target_ptr);
        ZVAL_COPY(&params[i], tmp);
        fci.param_count = 1;
        fci.retval = &result;
        fci.params = params;
        zend_call_function(&fci, &fci_cache);

        zend_hash_next_index_insert_new(Z_ARRVAL_P(return_value), &result);

        zval_ptr_dtor(tmp);
        zval_ptr_dtor(&params[i]);
        efree(tmp);
    }
    efree(params);
}
PHP_METHOD(CArray, __invoke)
{
    throw_notimplemented_exception();
}

/**
 * STORAGE
 */
PHP_METHOD(CArray, save)
{
    MemoryPointer ptr;
    zval * obj = getThis();
    char * filename;
    size_t filename_len;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STRING(filename, filename_len)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(obj, &ptr, NULL);

    CArray *target = CArray_FromMemoryPointer(&ptr);
    if(!CArrayStorage_SaveBin(filename, target)){
        throw_memory_exception("An error occurred");
        return;
    }
}
PHP_METHOD(CArray, load)
{
    MemoryPointer rtn;
    char * filename;
    size_t filename_len;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_STRING(filename, filename_len)
    ZEND_PARSE_PARAMETERS_END();

    if(!CArrayStorage_LoadBin(filename, &rtn)){
        throw_memory_exception("An error occurred");
        return;
    }

    RETURN_MEMORYPOINTER(return_value, &rtn);
}

/**
 * RubixML Tensor Interface
 */
static zend_function_entry crubix_class_methods[] =
{
        PHP_ME(CRubix, identity, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, zeros, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, ones, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, diagonal, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, fill, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, shape, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(CRubix, m, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(CRubix, n, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(CRubix, size, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(CRubix, diagonalAsVector, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, symmetric, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, reshape, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, transpose, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, subMatrix, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, solve, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        PHP_ME(CRubix, log, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, log1p, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, sin, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, asin, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, atan, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, cos, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, acos, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, tan, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, sqrt, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, exp, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, expm1, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, reciprocal, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, abs, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        PHP_ME(CRubix, maximum, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, minimum, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, flatten, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        PHP_ME(CRubix, inverse, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, matmul, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, eig, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, dot, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        PHP_ME(CRubix, multiply, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, divide, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, add, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, subtract, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, pow, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, mod, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, negate, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, sum, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, product, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        PHP_ME(CRubix, floor, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, ceil, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        PHP_ME(CRubix, argmin, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, argmax, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, min, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, max, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        PHP_ME(CRubix, equalMatrix, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, notEqualMatrix, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        PHP_ME(CRubix, greaterMatrix, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, greaterEqualMatrix, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, greaterVector, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, greaterEqualVector, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, greaterScalar, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, greaterEqualScalar, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, greaterColumnVector, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, greaterEqualColumnVector, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        PHP_ME(CRubix, equalScalar, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, lessMatrix, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, lessEqualMatrix, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, lessVector, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, lessEqualVector, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, lessScalar, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, lessEqualScalar, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, lessColumnVector, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, lessEqualColumnVector, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, equalVector, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, notEqualVector, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, notEqualScalar, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        PHP_ME(CRubix, sign, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, round, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CRubix, offsetGet, arginfo_array_offsetGet, ZEND_ACC_PUBLIC)
};

/**
 * CLASS METHODS
 */
static zend_function_entry carray_class_methods[] =
{
        PHP_ME(CArray, __construct, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(CArray, __destruct, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(CArray, dump, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(CArray, print, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(CArray, __set, arginfo_array_set, ZEND_ACC_PUBLIC)
        PHP_ME(CArray, __invoke, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(CArray, __toString, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(CArray, toArray, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(CArray, map, NULL, ZEND_ACC_PUBLIC)

        // RANDOM
        PHP_ME(CArray, rand, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, poisson, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        // MISC
        PHP_ME(CArray, fill, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, clip, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, convolve, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        // INDEXING
        PHP_ME(CArray, diagonal, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, take, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, atleast_1d, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, atleast_2d, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, atleast_3d, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, squeeze, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, expand_dims, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        // INITIALIZERS
        PHP_ME(CArray, zeros, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, ones, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        // NUMERICAL RANGES
        PHP_ME(CArray, arange, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, linspace, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, logspace, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, geomspace, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        //ARRAY MANIPULATION
        PHP_ME(CArray, swapaxes, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, rollaxis, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, moveaxis, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, concatenate, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, flip, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        // SORTING
        PHP_ME(CArray, sort, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        // METHODS
        PHP_ME(CArray, identity, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, eye, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        // SEARCH
        PHP_ME(CArray, argmax, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, argmin, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        // SHAPE
        PHP_ME(CArray, transpose, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, reshape, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, setShape, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(CArray, shape, NULL, ZEND_ACC_PUBLIC)

        //ROUNDING
        PHP_ME(CArray, ceil, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, floor, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, around, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        // LINEAR ALGEBRA
        PHP_ME(CArray, matmul, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, solve, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, inv, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, vdot, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, inner, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, outer, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        // EIGNVALUES
        PHP_ME(CArray, eig, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, eigvals, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        // NORMS
        PHP_ME(CArray, norm, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, det, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, matrix_rank, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        // DECOMPOSITIONS
        PHP_ME(CArray, svd, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, cholesky, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, qr, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        // ARITHMETIC
        PHP_ME(CArray, add, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, subtract, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, multiply, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, divide, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, power, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, mod, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, fmod, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, remainder, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, negative, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, sqrt, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, reciprocal, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        // STATISTICS
        PHP_ME(CArray, correlate, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        // EXPONENTS AND LOGARITHMS
        PHP_ME(CArray, exp, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, expm1, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, exp2, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, log, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, log10, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, log2, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, log1p, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        // LOGICAL FUNCTIONS
        PHP_ME(CArray, any, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, all, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        // CALCULATION
        PHP_ME(CArray, sum, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, prod, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, cumprod, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, cumsum, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        // TRIGONOMETRIC
        PHP_ME(CArray, sin, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, cos, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, tan, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, arcsin, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, arccos, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, arctan, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, sinh, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, cosh, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
        PHP_ME(CArray, tanh, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        // STORAGE
        PHP_ME(CArray, save, NULL, ZEND_ACC_PUBLIC)
        PHP_ME(CArray, load, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

        // CARRAY ITERATOR
        PHP_ME(CArray, offsetUnset, arginfo_array_offsetGet, ZEND_ACC_PUBLIC)
        PHP_ME(CArray, offsetSet, arginfo_offsetSet, ZEND_ACC_PUBLIC)
        PHP_ME(CArray, offsetGet, arginfo_array_offsetGet, ZEND_ACC_PUBLIC)
        PHP_ME(CArray, offsetExists, arginfo_array_offsetGet, ZEND_ACC_PUBLIC)
        { NULL, NULL, NULL }
};
static zend_function_entry carray_iterator_class_methods[] =
{
        { NULL, NULL, NULL }
};
zend_function_entry carray_functions[] = {
        {NULL, NULL, NULL}
};

static int carray_do_operation_ex(zend_uchar opcode, zval *result, zval *op1, zval *op2) /* {{{ */
{
    MemoryPointer rtn_ptr, op1_ptr, op2_ptr;
    CArray * result_ca, * op1_ca, * op2_ca;

    ZVAL_TO_MEMORYPOINTER(op1, &op1_ptr, NULL);
    ZVAL_TO_MEMORYPOINTER(op2, &op2_ptr, NULL);

    op1_ca = CArray_FromMemoryPointer(&op1_ptr);
    op2_ca = CArray_FromMemoryPointer(&op2_ptr);

    switch (opcode) {
        case ZEND_ADD:
            result_ca = CArray_Add(op1_ca, op2_ca, &rtn_ptr);
            FREE_FROM_MEMORYPOINTER(&op1_ptr);
            FREE_FROM_MEMORYPOINTER(&op2_ptr);
            if (result_ca == NULL) {
                return FAILURE;
            }
            RETURN_MEMORYPOINTER(result, &rtn_ptr);
            return SUCCESS;
        case ZEND_SUB:
            result_ca = CArray_Subtract(op1_ca, op2_ca, &rtn_ptr);
            FREE_FROM_MEMORYPOINTER(&op1_ptr);
            FREE_FROM_MEMORYPOINTER(&op2_ptr);
            if (result_ca == NULL) {
                return FAILURE;
            }
            RETURN_MEMORYPOINTER(result, &rtn_ptr);
            return SUCCESS;
        case ZEND_MUL:
            result_ca = CArray_Multiply(op1_ca, op2_ca, &rtn_ptr);
            FREE_FROM_MEMORYPOINTER(&op1_ptr);
            FREE_FROM_MEMORYPOINTER(&op2_ptr);
            if (result_ca == NULL) {
                return FAILURE;
            }
            RETURN_MEMORYPOINTER(result, &rtn_ptr);
            return SUCCESS;
        case ZEND_POW:
            result_ca = CArray_Power(op1_ca, op2_ca, &rtn_ptr);
            FREE_FROM_MEMORYPOINTER(&op1_ptr);
            FREE_FROM_MEMORYPOINTER(&op2_ptr);
            if (result_ca == NULL) {
                return FAILURE;
            }
            RETURN_MEMORYPOINTER(result, &rtn_ptr);
            return SUCCESS;
        case ZEND_DIV:
            result_ca = CArray_Divide(op1_ca, op2_ca, &rtn_ptr);
            FREE_FROM_MEMORYPOINTER(&op1_ptr);
            FREE_FROM_MEMORYPOINTER(&op2_ptr);
            if (result_ca == NULL) {
                return FAILURE;
            }
            RETURN_MEMORYPOINTER(result, &rtn_ptr);
            return SUCCESS;
        case ZEND_MOD:
            result_ca = CArray_Mod(op1_ca, op2_ca, &rtn_ptr);
            FREE_FROM_MEMORYPOINTER(&op1_ptr);
            FREE_FROM_MEMORYPOINTER(&op2_ptr);
            if (result_ca == NULL) {
                return FAILURE;
            }
            RETURN_MEMORYPOINTER(result, &rtn_ptr);
            return SUCCESS;
        case ZEND_SL:
        case ZEND_SR:
        case ZEND_BW_OR:
        case ZEND_BW_AND:
        case ZEND_BW_XOR:
        case ZEND_BW_NOT:
        default:
            return FAILURE;
    }
}

static
int carray_compare(zval *object1, zval *object2 TSRMLS_DC) /* {{{ */
{
    CArray *a, *b;
    MemoryPointer ptr1, ptr2;
    ZVAL_TO_MEMORYPOINTER(object1, &ptr1, NULL);
    ZVAL_TO_MEMORYPOINTER(object2, &ptr2, NULL);

    a = CArray_FromMemoryPointer(&ptr1);
    b = CArray_FromMemoryPointer(&ptr2);

    if (CArray_DATA(a) == CArray_DATA(b)) {
        return SUCCESS;
    }
    return FAILURE;
}

static int
carray_count(zval *object, long *count TSRMLS_DC) {
    MemoryPointer ptr;
    CArray * target;
    ZVAL_TO_MEMORYPOINTER(object, &ptr, NULL);
    target = CArray_FromMemoryPointer(&ptr);
    if (CArray_NDIM(target) > 0) {
        *count = (long)CArray_DIMS(target)[0];
    } else {
        *count = (long)1;
    }
    return SUCCESS;
}

static
int carray_do_operation(zend_uchar opcode, zval *result, zval *op1, zval *op2) /* {{{ */
{
    zval op1_copy;
    int retval;

    if (result == op1) {
        ZVAL_COPY_VALUE(&op1_copy, op1);
        op1 = &op1_copy;
    }

    retval = carray_do_operation_ex(opcode, result, op1, op2);

    if (retval == SUCCESS && op1 == &op1_copy) {
        zval_ptr_dtor(op1);
    }

    return retval;
}

static int
carray_cast(zval *readobj, zval *retval, int type) {
    throw_valueerror_exception("Use astype() for casting");
    return FAILURE;
}

void carray_ce_register()
{
    zend_class_entry ce;
    zend_class_entry cerubix;

    // Initialize Classes

	INIT_NS_CLASS_ENTRY(ce, "Phalcon", "CArray", carray_class_methods);
	INIT_NS_CLASS_ENTRY(cerubix, "Phalcon", "CRubix", crubix_class_methods);

    // Register CArray Class
    carray_sc_entry = zend_register_internal_class(&ce);
    carray_sc_entry->create_object = carray_create_object;

    // Register PHP Object
    memcpy(&carray_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    carray_object_handlers.do_operation = carray_do_operation;
    carray_object_handlers.compare_objects = carray_compare;
    carray_object_handlers.count_elements = carray_count;

    // Register RubixML Interface
    crubix_sc_entry = zend_register_internal_class_ex(&cerubix, carray_sc_entry);

#ifdef HAVE_CLBLAS
    // If --with-opencl flag is set, initialize GPU context
    start_clblas_context();
#endif

    // Pretend CArray is a PHP Array for compatibility
    zend_class_implements(carray_sc_entry, 1, zend_ce_arrayaccess);
    zend_class_implements(crubix_sc_entry, 1, zend_ce_arrayaccess);

    // Register Exception Classes
    init_exception_objects();

    // Pray
    return SUCCESS;
}
