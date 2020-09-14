/**
 * CArray Interface for RubixML
 */

#include <kernel/shape.h>
#include <kernel/alloc.h>
#include <kernel/iterators.h>
#include <kernel/linalg.h>
#include <kernel/exp_logs.h>
#include <kernel/number.h>
#include <kernel/trigonometric.h>
#include <kernel/convert.h>
#include <kernel/matlib.h>
#include <kernel/item_selection.h>
#include <kernel/buffer.h>
#include <kernel/search.h>
#include <kernel/round.h>
#include <kernel/calculation.h>
#include "rubix.h"
#include <kernel/carray.h>
#include <phpsci.h>
#include "php.h"
#include "php_ini.h"
#include <kernel/common/exceptions.h>
#include <kernel/scalar.h>
#include "lapacke.h"
#include "cblas.h"

/**
 * RubixML/Tensor/Matrix::identity
 */
PHP_METHOD(CRubix, identity)
{
    MemoryPointer ptr;
    zend_long n;
    CArray *output;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();

    if (n < 1) {
        throw_valueerror_exception(
                "Dimensionality must be greater than 0 on all axes.");
        return;
    }

    output = CArray_Eye((int)n, (int)n, 0, NULL, &ptr);

    if (output != NULL) {
        RETURN_MEMORYPOINTER(return_value, &ptr);
    }
}

/**
 * RubixML/Tensor/Matrix::zeros
 */
PHP_METHOD(CRubix, zeros)
{
    zend_long m, n;
    MemoryPointer ptr;
    char dtype = 'd', order = 'C';
    int ndim = 2;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(m)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();

    if (m < 1 || n < 1) {
        throw_valueerror_exception("Dimensionality must be greater than 0 on all axes.");
        return;
    }

    int *shape = emalloc(sizeof(int) * 2);
    shape[0] = (int)m;
    shape[1] = (int)n;

    CArray_Zeros(shape, ndim, dtype, &order, &ptr);

    RETURN_RUBIX_MEMORYPOINTER(return_value,&ptr);
    efree(shape);
}

/**
 * RubixML/Tensor/Matrix::ones
 */
PHP_METHOD(CRubix, ones)
{
    zend_long m, n;
    MemoryPointer ptr;
    char dtype = 'd', order = 'C';
    int ndim = 2;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(m)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();

    if (m < 1 || n < 1) {
        throw_valueerror_exception("Dimensionality must be greater than 0 on all axes.");
        return;
    }

    int *shape = emalloc(sizeof(int) * 2);
    shape[0] = (int)m;
    shape[1] = (int)n;

    CArray_Ones(shape, ndim, &dtype, &order, &ptr);

    RETURN_RUBIX_MEMORYPOINTER(return_value,&ptr);
    efree(shape);
}

/**
 * RubixML/Tensor/Matrix::diagonal
 */
PHP_METHOD(CRubix, diagonal)
{
    int i;
    char dtype = 'd', order = 'C';
    MemoryPointer a_ptr, rtn_ptr;
    CArray *target_array, *outarray;
    int len;
    zval *elements;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(elements)
    ZEND_PARSE_PARAMETERS_END();

    if (zend_hash_num_elements(Z_ARRVAL_P(elements)) < 1) {
        throw_valueerror_exception("Dimensionality must be greater than 0 on all axes.");
        return;
    }

    ZVAL_TO_MEMORYPOINTER(elements, &a_ptr, &dtype);
    target_array = CArray_FromMemoryPointer(&a_ptr);

    int *shape = emalloc(sizeof(int) * 2);
    shape[0] = zend_hash_num_elements(Z_ARRVAL_P(elements));
    shape[1] = shape[0];

    CArray_Zeros(shape, 2, dtype, &order, &rtn_ptr);
    outarray = CArray_FromMemoryPointer(&rtn_ptr);

    // CArray diagonal works differently, in this case we
    // implement Rubix algorithm below.
    for (i = 0; i < shape[0]; i++) {
        DDATA(outarray)[(i * shape[0]) + i] = DDATA(target_array)[i];
    }

    RETURN_RUBIX_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::fill
 */
PHP_METHOD(CRubix, fill)
{
    CArray *outarray;
    char dtype = 'd', order = 'C';
    MemoryPointer rtn_ptr;
    zend_long m, n;
    double value;
    CArrayScalar *scalar;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_DOUBLE(value)
        Z_PARAM_LONG(m)
        Z_PARAM_LONG(n)
    ZEND_PARSE_PARAMETERS_END();

    if (m < 1 || n < 1) {
        throw_valueerror_exception("Dimensionality must be greater than 0 on all axes.");
        return;
    }

    int *shape = emalloc(sizeof(int) * 2);
    shape[0] = (int)m;
    shape[1] = (int)n;

    CArray_Zeros(shape, 2, dtype, &order, &rtn_ptr);
    outarray = CArray_FromMemoryPointer(&rtn_ptr);

    scalar = CArrayScalar_NewDouble(value);
    CArray_FillWithScalar(outarray, scalar);

    CArrayScalar_FREE(scalar);
    RETURN_RUBIX_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::rand
 */
PHP_METHOD(CRubix, rand)
{

}

PHP_METHOD(CRubix, gaussian)
{

}

PHP_METHOD(CRubix, uniform)
{
    
}

/**
 * RubixML/Tensor/Matrix::minimum
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, minimum)
{
    int i = 0;
    MemoryPointer a_ptr, b_ptr, rtn_ptr;
    zval *a, *b;
    CArray *a_ca, *b_ca, *rtn;
    char dtype = 'd', order = 'C';
    CArrayIterator *itA, *itB;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(a)
            Z_PARAM_ZVAL(b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(a, &a_ptr, NULL);
    ZVAL_TO_MEMORYPOINTER(b, &b_ptr, NULL);

    a_ca = CArray_FromMemoryPointer(&a_ptr);
    b_ca = CArray_FromMemoryPointer(&b_ptr);

    rtn = CArray_Zeros(CArray_DIMS(a_ca), CArray_NDIM(a_ca),
                       dtype, &order, &rtn_ptr);

    itA = CArray_NewIter(a_ca);
    itB = CArray_NewIter(b_ca);

    do {
        if (*(IT_DDATA(itA)) < *(IT_DDATA(itB))) {
            DDATA(rtn)[i] = *(IT_DDATA(itA));
        } else {
            DDATA(rtn)[i] = *(IT_DDATA(itB));
        }

        CArrayIterator_NEXT(itA);
        CArrayIterator_NEXT(itB);
        i++;
    } while(CArrayIterator_NOTDONE(itA));

    CArrayIterator_FREE(itA);
    CArrayIterator_FREE(itB);

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::maximum
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, maximum)
{
    int i = 0;
    MemoryPointer a_ptr, b_ptr, rtn_ptr;
    zval *a, *b;
    CArray *a_ca, *b_ca, *rtn;
    char dtype = 'd', order = 'C';
    CArrayIterator *itA, *itB;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ZVAL(a)
        Z_PARAM_ZVAL(b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(a, &a_ptr, NULL);
    ZVAL_TO_MEMORYPOINTER(b, &b_ptr, NULL);

    a_ca = CArray_FromMemoryPointer(&a_ptr);
    b_ca = CArray_FromMemoryPointer(&b_ptr);

    rtn = CArray_Zeros(CArray_DIMS(a_ca), CArray_NDIM(a_ca),
            dtype, &order, &rtn_ptr);

    itA = CArray_NewIter(a_ca);
    itB = CArray_NewIter(b_ca);

    do {
        if (*(IT_DDATA(itA)) > *(IT_DDATA(itB))) {
            DDATA(rtn)[i] = *(IT_DDATA(itA));
        } else {
            DDATA(rtn)[i] = *(IT_DDATA(itB));
        }

        CArrayIterator_NEXT(itA);
        CArrayIterator_NEXT(itB);
        i++;
    } while(CArrayIterator_NOTDONE(itA));

    CArrayIterator_FREE(itA);
    CArrayIterator_FREE(itB);

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

PHP_METHOD(CRubix, stack)
{

}

PHP_METHOD(CRubix, implodeRow)
{

}

/**
 * RubixML/Tensor/Matrix::shape
 */
PHP_METHOD(CRubix, shape)
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

/**
 * RubixML/Tensor/Matrix::size
 */
PHP_METHOD(CRubix, size)
{
    MemoryPointer ptr;
    CArray * target;
    zval * obj = getThis();
    zval tmp_zval;
    int i;

    ZVAL_TO_MEMORYPOINTER(obj, &ptr, NULL);
    target = CArray_FromMemoryPointer(&ptr);

    RETURN_LONG(CArray_DESCR(target)->numElements);
}

/**
 * RubixML/Tensor/Matrix::m
 */
PHP_METHOD(CRubix, m)
{
    MemoryPointer ptr;
    CArray * target;
    zval * obj = getThis();
    zval tmp_zval;
    int i;

    ZVAL_TO_MEMORYPOINTER(obj, &ptr, NULL);
    target = CArray_FromMemoryPointer(&ptr);

    RETURN_LONG(CArray_DIMS(target)[0]);
}

/**
 * RubixML/Tensor/Matrix::n
 */
PHP_METHOD(CRubix, n)
{
    MemoryPointer ptr;
    CArray * target;
    zval * obj = getThis();
    zval tmp_zval;
    int i;

    ZVAL_TO_MEMORYPOINTER(obj, &ptr, NULL);
    target = CArray_FromMemoryPointer(&ptr);

    if (CArray_NDIM(target) == 1) {
        RETURN_LONG(CArray_DIMS(target)[0]);
        return;
    }

    RETURN_LONG(CArray_DIMS(target)[1]);
}

PHP_METHOD(CRubix, row)
{

}

PHP_METHOD(CRubix, column)
{

}

/**
 * RubixML/Tensor/Matrix::diagonalAsVector
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, diagonalAsVector)
{
    MemoryPointer a_ptr, rtn_ptr;
    CArray * target_array;
    zval * a;
    long axis1, axis2, offset;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(a)
    ZEND_PARSE_PARAMETERS_END();

    offset = 0;
    axis1 = 0;
    axis2 = 1;

    ZVAL_TO_MEMORYPOINTER(a, &a_ptr, NULL);
    target_array = CArray_FromMemoryPointer(&a_ptr);
    CArray * rtn_array = CArray_Diagonal(target_array, offset, axis1, axis2, &rtn_ptr);
    if(rtn_array == NULL) {
        return;
    }
    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::flatten
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, flatten)
{
    zval *a;
    CArray *a_ca, *rtn_ca;
    MemoryPointer ptr, rtn_ptr;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(a)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(a, &ptr, NULL);
    a_ca = CArray_FromMemoryPointer(&ptr);

    rtn_ca = CArray_Ravel(a_ca, CARRAY_CORDER);
    add_to_buffer(&rtn_ptr, rtn_ca, sizeof(CArray));

    CArrayDescriptor_INCREF(CArray_DESCR(rtn_ca));
    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::argmin
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, argmin)
{
    zval * target;
    int axis_p = 1;
    CArray * ret, * target_ca;
    MemoryPointer ptr, out_ptr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);

    ret = CArray_Argmin(target_ca, &axis_p, &out_ptr);

    if (ret == NULL) {
        return;
    }

    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &out_ptr);
}

/**
 * RubixML/Tensor/Matrix::argmax
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, argmax)
{
    zval * target;
    int axis_p = 1;
    CArray * ret, * target_ca;
    MemoryPointer ptr, out_ptr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);

    ret = CArray_Argmax(target_ca, &axis_p, &out_ptr);

    if (ret == NULL) {
        return;
    }

    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &out_ptr);
}

PHP_METHOD(CRubix, map)
{

}

/**
 * RubixML/Tensor/Matrix::inverse
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, inverse)
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

/**
 * RubixML/Tensor/Matrix::det
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, det)
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

PHP_METHOD(CRubix, trace)
{

}

PHP_METHOD(CRubix, rank)
{

}

PHP_METHOD(CRubix, fullRank)
{

}

/**
 * RubixML/Tensor/Matrix::symmetric
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, symmetric)
{
    int i;
    zval *obj;
    MemoryPointer ptr, trans_ptr;
    CArray *target, *transposed;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(obj)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(obj, &ptr, NULL);
    target = CArray_FromMemoryPointer(&ptr);

    if (CArray_NDIM(target) != 2) {
        throw_valueerror_exception("Matrix must have two dimensions");
        return;
    }

    transposed = CArray_Transpose(target, NULL, &trans_ptr);
    CArrayIterator * it = CArray_NewIter(transposed);

    if (CArray_DIMS(target)[0] != CArray_DIMS(target)[1]) {
        goto not_symmetric;
    }

    if (CArray_TYPE(target) == TYPE_DOUBLE_INT) {
        i = 0;
        do {
            if (DDATA(target)[i] != IT_DDATA(it)[0]) {
                goto not_symmetric;
            }
            i++;
            CArrayIterator_NEXT(it);
        } while(CArrayIterator_NOTDONE(it));
    }

    if (CArray_TYPE(target) == TYPE_INTEGER_INT) {
        i = 0;
        do {
            if ((double)IDATA(target)[i] != IT_IDATA(it)[0]) {
                goto not_symmetric;
            }
            i++;
            CArrayIterator_NEXT(it);
        } while(CArrayIterator_NOTDONE(it));
    }

    CArrayIterator_FREE(it);
    RETURN_LONG(1);
    return;

not_symmetric:
    CArray_DECREF(target);
    CArrayDescriptor_DECREF(CArray_DESCR(target));
    CArrayIterator_FREE(it);
    RETURN_LONG(0);
}

/**
 * RubixML/Tensor/Matrix::transpose
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, transpose)
{
    zval * target;
    zval * axes;
    int size_axes;
    CArray * ret, * target_ca;
    MemoryPointer ptr;
    CArray_Dims permute;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);

    ret = CArray_Transpose(target_ca, NULL, &ptr);

    RETURN_MEMORYPOINTER(return_value, &ptr);
}

/**
 * RubixML/Tensor/Matrix::matmul
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, matmul)
{
    MemoryPointer target1_ptr, target2_ptr, result_ptr;

    zval *target1, *target2;
    CArray *target_ca1, *target_ca2, *output_ca, *out;
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

/**
 * RubixML/Tensor/Vector::dot
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, dot)
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

PHP_METHOD(CRubix, convolve)
{

}

PHP_METHOD(CRubix, ref)
{

}

PHP_METHOD(CRubix, rref)
{

}

PHP_METHOD(CRubix, lu)
{

}

PHP_METHOD(CRubix, cholesky)
{

}

/**
 * RubixML/Tensor/Matrix::eig
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, eig)
{
    int status, i;
    double *data;
    CArray *target_ca;
    MemoryPointer ptr, ptr_wr, ptr_vr;
    zval *target, *tmp_zval;
    CArray *wr, *wi, *vl, *vr;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);

    if (!CArray_CHKFLAGS(target_ca, CARRAY_ARRAY_C_CONTIGUOUS)) {
        data = emalloc(sizeof(double) * CArray_SIZE(target_ca));
        linearize_DOUBLE_matrix(data, DDATA(target_ca), target_ca);
    } else {
        data = DDATA(target_ca);
    }

    int dims = CArray_DIMS(target_ca)[0];

    wr = CArray_Zeros(&dims, 1, 'd', NULL, &ptr_wr);
    wi = CArray_Zeros(&dims, 1, 'd', NULL, NULL);
    vl = CArray_Zeros(CArray_DIMS(target_ca), 2, 'd', NULL, NULL);
    vr = CArray_Zeros(CArray_DIMS(target_ca), 2, 'd', NULL, &ptr_vr);

    int info;
    double* work;
    int lwork = -1;

    status = LAPACKE_dgeev(
        LAPACK_ROW_MAJOR,
        'V',
        'V',
        CArray_DIMS(target_ca)[0],
        data,
        CArray_DIMS(target_ca)[0],
        DDATA(wr),
        DDATA(wi),
        DDATA(vl),
        CArray_DIMS(target_ca)[0],
        DDATA(vr),
        CArray_DIMS(target_ca)[0]
    );

    array_init_size(return_value, 2);
    
    tmp_zval = MEMORYPOINTER_TO_ZVAL(&ptr_wr);
    zend_hash_next_index_insert_new(Z_ARRVAL_P(return_value), tmp_zval);
    efree(tmp_zval);

    tmp_zval = MEMORYPOINTER_TO_ZVAL(&ptr_vr);
    zend_hash_next_index_insert_new(Z_ARRVAL_P(return_value), tmp_zval);
    efree(tmp_zval);

    CArray_Free(wi);
    CArray_Free(vl);
}

/**
 * RubixML/Tensor/Matrix::solve
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, solve)
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

PHP_METHOD(CRubix, l1Norm)
{

}

PHP_METHOD(CRubix, l2Norm)
{

}

PHP_METHOD(CRubix, infinityNorm)
{

}

PHP_METHOD(CRubix, maxNorm)
{

}

/**
 * RubixML/Tensor/Matrix::flatten
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, multiply)
{
    MemoryPointer target1_ptr, target2_ptr, result_ptr;
    zval * target1, * target2;
    CArray * target_ca1, * target_ca2, * output_ca, * out;
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

/**
 * RubixML/Tensor/Matrix::divide
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, divide)
{
    MemoryPointer target1_ptr, target2_ptr, result_ptr;
    zval * target1, * target2;
    CArray * target_ca1, * target_ca2, * output_ca, * out;
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

/**
 * RubixML/Tensor/Matrix::divide
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, add)
{
    MemoryPointer target1_ptr, target2_ptr, result_ptr;
    zval * target1, * target2;
    CArray * target_ca1, * target_ca2, * output_ca, * out;
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

/**
 * RubixML/Tensor/Matrix::subtract
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, subtract)
{
    MemoryPointer target1_ptr, target2_ptr, result_ptr;
    zval * target1, * target2;
    CArray * target_ca1, * target_ca2, * output_ca, * out;
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

/**
 * RubixML/Tensor/Matrix::pow
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, pow)
{
    MemoryPointer target1_ptr, target2_ptr, result_ptr;
    zval * target1, * target2;
    CArray * target_ca1, * target_ca2, * output_ca, * out;
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

/**
 * RubixML/Tensor/Matrix::mod
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, mod)
{
    MemoryPointer target1_ptr, target2_ptr, result_ptr;
    zval * target1, * target2;
    CArray * target_ca1, * target_ca2, * output_ca, * out;
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

/**
 * RubixML/Tensor/Matrix::reciprocal
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, reciprocal)
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
 * RubixML/Tensor/Matrix::abs
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, abs)
{
    MemoryPointer target_ptr, rtn_ptr;
    CArray * target_ca, * rtn_ca;
    zval * target;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &target_ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&target_ptr);
    rtn_ca = CArray_Absolute(target_ca, &rtn_ptr);

    if (rtn_ca == NULL) {
        return;
    }

    FREE_FROM_MEMORYPOINTER(&target_ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::sqrt
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, sqrt)
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

/**
 * RubixML/Tensor/Matrix::exp
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, exp)
{
    zval * target;
    long axis;
    int * axis_p;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Exp(target_ca, &rtn_tr);

    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}

/**
 * RubixML/Tensor/Matrix::expm1
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, expm1)
{
    zval * target;
    long axis;
    int * axis_p;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Expm1(target_ca, &rtn_tr);

    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}

/**
 * RubixML/Tensor/Matrix::log
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, log)
{
    zval * target;
    long axis;
    int * axis_p;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Log(target_ca, &rtn_tr);

    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}

/**
 * RubixML/Tensor/Matrix::log1p
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, log1p)
{
    zval * target;
    long axis;
    int * axis_p;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Log1p(target_ca, &rtn_tr);

    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}

/**
 * RubixML/Tensor/Matrix::sin
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, sin)
{
    zval * target;
    long axis;
    int * axis_p;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Sin(target_ca, &rtn_tr);

    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}

/**
 * RubixML/Tensor/Matrix::asin
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, asin)
{
    zval * target;
    long axis;
    int * axis_p;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Arcsin(target_ca, &rtn_tr);

    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}

/**
 * RubixML/Tensor/Matrix::cos
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, cos)
{
    zval * target;
    long axis;
    int * axis_p;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Cos(target_ca, &rtn_tr);

    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}

/**
 * RubixML/Tensor/Matrix::acos
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, acos)
{
    zval * target;
    long axis;
    int * axis_p;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Arccos(target_ca, &rtn_tr);

    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}

/**
 * RubixML/Tensor/Matrix::tan
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, tan)
{
    zval * target;
    long axis;
    int * axis_p;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Tan(target_ca, &rtn_tr);

    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}

/**
 * RubixML/Tensor/Matrix::atan
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, atan)
{
    zval * target;
    long axis;
    int * axis_p;
    CArray * ret, * target_ca;
    MemoryPointer ptr, rtn_tr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Arctan(target_ca, &rtn_tr);

    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_tr);
}

/**
 * RubixML/Tensor/Matrix::atan
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, reshape)
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

PHP_METHOD(CRubix, rad2deg)
{

}

PHP_METHOD(CRubix, deg2rad)
{

}

/**
 * RubixML/Tensor/Matrix::sum
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, sum)
{
    zval * target;
    int axis = 1;
    CArray * ret, * target_ca;
    MemoryPointer ptr, out_ptr;
    ZEND_PARSE_PARAMETERS_START(1, 2)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);

    if (CArray_NDIM(target_ca) == 2) {
        ret = CArray_Sum(target_ca, &axis, target_ca->descriptor->type_num, &out_ptr);
    } else {
        ret = CArray_Sum(target_ca, NULL, target_ca->descriptor->type_num, &out_ptr);
    }

    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &out_ptr);
}

/**
 * RubixML/Tensor/Matrix::product
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, product)
{
    zval *target;
    int axis = 1;
    CArray *ret, *target_ca;
    MemoryPointer ptr, rtn_ptr;
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);
    ret = CArray_Prod(target_ca, &axis, target_ca->descriptor->type_num, &rtn_ptr);

    CArray_DECREF(target_ca);
    FREE_FROM_MEMORYPOINTER(&ptr);
    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::min
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, min)
{
    int i, j, tmp_int;
    double tmp_double;
    char order = 'C';
    zval *target;
    MemoryPointer ptr, rtn_ptr;
    CArray *ret, *target_ca, *rtn_ca;
    CArrayIterator *it;
    int *newshape;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);

    if (CArray_NDIM(target_ca) == 1) {
        switch (CArray_TYPE(target_ca)) {
            case TYPE_INTEGER_INT:
                tmp_int = 0;
                for (i =0; i < CArray_DESCR(target_ca)->numElements; i++) {
                    if (tmp_int < IDATA(target_ca)[i] || i == 0) {
                        tmp_int = *(IDATA(target_ca));
                    }
                }
                RETURN_LONG((long) tmp_int);
                break;
            case TYPE_DOUBLE_INT:
                tmp_double = 0;
                for (i =0; i < CArray_DESCR(target_ca)->numElements; i++) {
                    if (tmp_double < DDATA(target_ca)[i] || i == 0) {
                        tmp_double = *(DDATA(target_ca));
                    }
                }
                RETURN_DOUBLE(tmp_double);
                break;
            default:
                return;
        }
    }

    switch (CArray_TYPE(target_ca)) {
        case TYPE_INTEGER_INT:
            i = 0, j = 0;
            newshape = emalloc(sizeof(int));
            newshape[0] = CArray_DIMS(target_ca)[1];

            rtn_ca = CArray_Zeros(newshape, 1, TYPE_INTEGER, &order, &rtn_ptr);
            it = CArray_NewIter(target_ca);
            do {
                if (tmp_int > IT_IDATA(it)[0] || i == 0) {
                    IDATA(rtn_ca)[j] = IT_IDATA(it)[0];
                    tmp_int = IDATA(rtn_ca)[j];
                }

                i++;
                if (i == CArray_DIMS(target_ca)[1]) {
                    tmp_int = 0;
                    i = 0;
                    j++;
                }
                CArrayIterator_NEXT(it);
            } while (CArrayIterator_NOTDONE(it));

        case TYPE_DOUBLE_INT:
            i = 0, j = 0;
            newshape = emalloc(sizeof(int));
            newshape[0] = CArray_DIMS(target_ca)[1];

            rtn_ca = CArray_Zeros(newshape, 1, TYPE_DOUBLE, &order, &rtn_ptr);
            it = CArray_NewIter(target_ca);
            do {
                if (tmp_double > IT_DDATA(it)[0] || i == 0) {
                    DDATA(rtn_ca)[j] = IT_DDATA(it)[0];
                    tmp_double = DDATA(rtn_ca)[j];
                }

                i++;
                if (i == CArray_DIMS(target_ca)[1]) {
                    tmp_int = 0;
                    i = 0;
                    j++;
                }
                CArrayIterator_NEXT(it);
            } while (CArrayIterator_NOTDONE(it));
    }

    add_to_buffer(&rtn_ptr, rtn_ca, sizeof(CArray));
    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::max
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, max)
{
    int i, j, tmp_int;
    double tmp_double;
    char order = 'C';
    zval *target;
    MemoryPointer ptr, rtn_ptr;
    CArray *ret, *target_ca, *rtn_ca;
    CArrayIterator *it;
    int *newshape;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target, &ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&ptr);

    if (CArray_NDIM(target_ca) == 1) {
        switch (CArray_TYPE(target_ca)) {
            case TYPE_INTEGER_INT:
                tmp_int = 0;
                for (i =0; i < CArray_DESCR(target_ca)->numElements; i++) {
                    if (tmp_int > IDATA(target_ca)[i] || i == 0) {
                        tmp_int = *(IDATA(target_ca));
                    }
                }
                RETURN_LONG((long) tmp_int);
                break;
            case TYPE_DOUBLE_INT:
                tmp_double = 0;
                for (i =0; i < CArray_DESCR(target_ca)->numElements; i++) {
                    if (tmp_double > DDATA(target_ca)[i] || i == 0) {
                        tmp_double = *(DDATA(target_ca));
                    }
                }
                RETURN_DOUBLE(tmp_double);
                break;
            default:
                return;
        }
    }

    switch (CArray_TYPE(target_ca)) {
        case TYPE_INTEGER_INT:
            i = 0, j = 0;
            newshape = emalloc(sizeof(int));
            newshape[0] = CArray_DIMS(target_ca)[1];

            rtn_ca = CArray_Zeros(newshape, 1, TYPE_INTEGER, &order, &rtn_ptr);
            it = CArray_NewIter(target_ca);
            do {
                if (tmp_int < IT_IDATA(it)[0] || i == 0) {
                    IDATA(rtn_ca)[j] = IT_IDATA(it)[0];
                    tmp_int = IDATA(rtn_ca)[j];
                }

                i++;
                if (i == CArray_DIMS(target_ca)[1]) {
                    tmp_int = 0;
                    i = 0;
                    j++;
                }
                CArrayIterator_NEXT(it);
            } while (CArrayIterator_NOTDONE(it));

        case TYPE_DOUBLE_INT:
            i = 0, j = 0;
            newshape = emalloc(sizeof(int));
            newshape[0] = CArray_DIMS(target_ca)[1];

            rtn_ca = CArray_Zeros(newshape, 1, TYPE_DOUBLE, &order, &rtn_ptr);
            it = CArray_NewIter(target_ca);
            do {
                if (tmp_double < IT_DDATA(it)[0] || i == 0) {
                    DDATA(rtn_ca)[j] = IT_DDATA(it)[0];
                    tmp_double = DDATA(rtn_ca)[j];
                }

                i++;
                if (i == CArray_DIMS(target_ca)[1]) {
                    tmp_int = 0;
                    i = 0;
                    j++;
                }
                CArrayIterator_NEXT(it);
            } while (CArrayIterator_NOTDONE(it));
    }

    add_to_buffer(&rtn_ptr, rtn_ca, sizeof(CArray));
    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

PHP_METHOD(CRubix, variance)
{

}

PHP_METHOD(CRubix, median)
{

}

PHP_METHOD(CRubix, quantile)
{

}

PHP_METHOD(CRubix, covariance)
{

}

/**
 * RubixML/Tensor/Matrix::round
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, round)
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
 * RubixML/Tensor/Matrix::floor
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, floor)
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

/**
 * RubixML/Tensor/Matrix::ceil
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, ceil)
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

PHP_METHOD(CRubix, clip)
{

}

PHP_METHOD(CRubix, clipLower)
{

}

PHP_METHOD(CRubix, clipUpper)
{

}

/**
 * RubixML/Tensor/Matrix::sign
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, sign)
{
    MemoryPointer target_ptr, rtn_ptr;
    CArray * target_ca, * rtn_ca;
    zval * target;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(target)
    ZEND_PARSE_PARAMETERS_END();
    ZVAL_TO_MEMORYPOINTER(target, &target_ptr, NULL);
    target_ca = CArray_FromMemoryPointer(&target_ptr);

    CArrayIterator *a_it = CArray_NewIter(target_ca);
    rtn_ca = CArray_Zeros(CArray_DIMS(target_ca), CArray_NDIM(target_ca), 'd', NULL, &rtn_ptr);

    if (CArray_TYPE(target_ca) == TYPE_INTEGER) {
        throw_typeerror_exception("Invalid type");
        return;
    }

    do {
        if (IT_DDATA(a_it)[0] > 0) {
            DDATA(rtn_ca)[a_it->index] = 1.0;
        } else if (IT_DDATA(a_it)[0] < 0) {
            DDATA(rtn_ca)[a_it->index] = -1.0;
        } else {
            DDATA(rtn_ca)[a_it->index] = 0.0;
        }

        CArrayIterator_NEXT(a_it);
    } while(CArrayIterator_NOTDONE(a_it));

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::negate
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, negate)
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

PHP_METHOD(CRubix, insert)
{

}

/**
 * RubixML/Tensor/Matrix::subMatrix
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, subMatrix)
{
    int i, init_jump, inner_jump, col_jump = 0, col = 0;
    double *data;
    MemoryPointer out, rtn_ptr;
    CArray * target_ca, * rtn_ca;
    long startRow, startColumn, endRow, endColumn;
    zval *target;

    ZEND_PARSE_PARAMETERS_START(5, 5)
        Z_PARAM_ZVAL(target)
        Z_PARAM_LONG(startRow)
        Z_PARAM_LONG(startColumn)
        Z_PARAM_LONG(endRow)
        Z_PARAM_LONG(endColumn)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target, &out, NULL);
    target_ca = CArray_FromMemoryPointer(&out);

    int *dims = emalloc(sizeof(int) * 2);

    dims[0] = endRow - startRow;
    dims[1] = endColumn - startColumn;

    rtn_ca = CArray_Zeros(dims, 2, 'd', NULL, &rtn_ptr);

    init_jump = ((CArray_STRIDES(target_ca)[0] * startRow) + (CArray_STRIDES(target_ca)[1] * startColumn)) / CArray_DESCR(target_ca)->elsize;
    inner_jump = CArray_STRIDES(target_ca)[1];

    data = DDATA(target_ca) + init_jump;

    for (i = 0; i < (dims[0] * dims[1]); i++) {
        if (col == (endColumn - startColumn)) {
            col = 0;
            data = (data + (CArray_STRIDES(target_ca)[0] / CArray_DESCR(target_ca)->elsize));
        }

        DDATA(rtn_ca)[i] = *(data + col);
        col++;
    }

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

PHP_METHOD(CRubix, augmentAbove)
{

}

PHP_METHOD(CRubix, augmentBelow)
{

}

PHP_METHOD(CRubix, augmentLeft)
{

}

PHP_METHOD(CRubix, augmentRight)
{

}

PHP_METHOD(CRubix, repeat)
{

}

/**
 * RubixML/Tensor/ColumnVector::equalMatrix
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, equalMatrix)
{
    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    CArrayIterator *it_a = CArray_NewIter(ca_a);
    CArrayIterator *it_b = CArray_NewIter(ca_b);

    if (CArray_DESCR(ca_a)->numElements >= CArray_DESCR(ca_b)->numElements) {
        rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);
        do {
            if (IT_DDATA(it_a)[0] == IT_DDATA(it_b)[0]) {
                DDATA(rtn_ca)[it_a->index] = 1;
            } else {
                DDATA(rtn_ca)[it_a->index] = 0;
            }

            CArrayIterator_NEXT(it_a);
            CArrayIterator_NEXT(it_b);

            if (!CArrayIterator_NOTDONE(it_b)) {
                CArrayIterator_RESET(it_b);
            }
        } while(CArrayIterator_NOTDONE(it_a));
    }

    if (CArray_DESCR(ca_b)->numElements > CArray_DESCR(ca_a)->numElements) {
        rtn_ca = CArray_Zeros(CArray_DIMS(ca_b), CArray_NDIM(ca_b), 'd', NULL, &rtn_ptr);
        do {
            if (IT_DDATA(it_a)[0] == IT_DDATA(it_b)[0]) {
                DDATA(rtn_ca)[it_b->index] = 1;
            } else {
                DDATA(rtn_ca)[it_b->index] = 0;
            }

            CArrayIterator_NEXT(it_a);
            CArrayIterator_NEXT(it_b);

            if (!CArrayIterator_NOTDONE(it_a)) {
                CArrayIterator_RESET(it_a);
            }
        } while(CArrayIterator_NOTDONE(it_b));
    }

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/ColumnVector::notEqualMatrix
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, notEqualMatrix)
{
    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    CArrayIterator *it_a = CArray_NewIter(ca_a);
    CArrayIterator *it_b = CArray_NewIter(ca_b);

    if (CArray_DESCR(ca_a)->numElements >= CArray_DESCR(ca_b)->numElements) {
        rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);
        do {
            if (IT_DDATA(it_a)[0] != IT_DDATA(it_b)[0]) {
                DDATA(rtn_ca)[it_a->index] = 1;
            } else {
                DDATA(rtn_ca)[it_a->index] = 0;
            }

            CArrayIterator_NEXT(it_a);
            CArrayIterator_NEXT(it_b);

            if (!CArrayIterator_NOTDONE(it_b)) {
                CArrayIterator_RESET(it_b);
            }
        } while(CArrayIterator_NOTDONE(it_a));
    }

    if (CArray_DESCR(ca_b)->numElements > CArray_DESCR(ca_a)->numElements) {
        rtn_ca = CArray_Zeros(CArray_DIMS(ca_b), CArray_NDIM(ca_b), 'd', NULL, &rtn_ptr);
        do {
            if (IT_DDATA(it_a)[0] != IT_DDATA(it_b)[0]) {
                DDATA(rtn_ca)[it_b->index] = 1;
            } else {
                DDATA(rtn_ca)[it_b->index] = 0;
            }

            CArrayIterator_NEXT(it_a);
            CArrayIterator_NEXT(it_b);

            if (!CArrayIterator_NOTDONE(it_a)) {
                CArrayIterator_RESET(it_a);
            }
        } while(CArrayIterator_NOTDONE(it_b));
    }

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::greaterMatrix
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, greaterMatrix)
{
    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    CArrayIterator *it_a = CArray_NewIter(ca_a);
    CArrayIterator *it_b = CArray_NewIter(ca_b);

    if (CArray_MultiplyList(CArray_DIMS(ca_a), CArray_NDIM(ca_a)) >= CArray_MultiplyList(CArray_DIMS(ca_b), CArray_NDIM(ca_b))) {
        rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);
        do {
            if (IT_DDATA(it_a)[0] > IT_DDATA(it_b)[0]) {
                DDATA(rtn_ca)[it_a->index] = 1;
            } else {
                DDATA(rtn_ca)[it_a->index] = 0;
            }

            CArrayIterator_NEXT(it_a);
            CArrayIterator_NEXT(it_b);

            if (!CArrayIterator_NOTDONE(it_b)) {
                CArrayIterator_RESET(it_b);
            }
        } while(CArrayIterator_NOTDONE(it_a));
    }

    if (CArray_MultiplyList(CArray_DIMS(ca_a), CArray_NDIM(ca_a)) < CArray_MultiplyList(CArray_DIMS(ca_b), CArray_NDIM(ca_b))) {
        rtn_ca = CArray_Zeros(CArray_DIMS(ca_b), CArray_NDIM(ca_b), 'd', NULL, &rtn_ptr);
        do {
            if (IT_DDATA(it_a)[0] > IT_DDATA(it_b)[0]) {
                DDATA(rtn_ca)[it_b->index] = 1;
            } else {
                DDATA(rtn_ca)[it_b->index] = 0;
            }

            CArrayIterator_NEXT(it_a);
            CArrayIterator_NEXT(it_b);

            if (!CArrayIterator_NOTDONE(it_a)) {
                CArrayIterator_RESET(it_a);
            }
        } while(CArrayIterator_NOTDONE(it_b));
    }

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::greaterEqualMatrix
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, greaterEqualMatrix)
{
    int i = 0;

    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    CArrayIterator *it_a = CArray_NewIter(ca_a);
    CArrayIterator *it_b = CArray_NewIter(ca_b);

    if (CArray_DESCR(ca_a)->numElements >= CArray_DESCR(ca_b)->numElements) {
        rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);
        do {
            if (IT_DDATA(it_a)[0] >= IT_DDATA(it_b)[0]) {
                DDATA(rtn_ca)[it_a->index] = 1;
            } else {
                DDATA(rtn_ca)[it_a->index] = 0;
            }

            CArrayIterator_NEXT(it_a);
            CArrayIterator_NEXT(it_b);

            if (!CArrayIterator_NOTDONE(it_b)) {
                CArrayIterator_RESET(it_b);
            }
        } while(CArrayIterator_NOTDONE(it_a));
    }

    if (CArray_DESCR(ca_b)->numElements > CArray_DESCR(ca_a)->numElements) {
        rtn_ca = CArray_Zeros(CArray_DIMS(ca_b), CArray_NDIM(ca_b), 'd', NULL, &rtn_ptr);
        do {
            if (IT_DDATA(it_a)[0] >= IT_DDATA(it_b)[0]) {
                DDATA(rtn_ca)[it_b->index] = 1;
            } else {
                DDATA(rtn_ca)[it_b->index] = 0;
            }

            i++;
            CArrayIterator_NEXT(it_b);
            
            if (i == CArray_DIMS(ca_b)[1]) {
                i = 0;
                CArrayIterator_NEXT(it_a);
            }

        } while(CArrayIterator_NOTDONE(it_b));
    }

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::lessMatrix
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, lessMatrix)
{
    int i = 0;

    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    CArrayIterator *it_a = CArray_NewIter(ca_a);
    CArrayIterator *it_b = CArray_NewIter(ca_b);

    if (CArray_DESCR(ca_a)->numElements >= CArray_DESCR(ca_b)->numElements) {
        rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);
        do {
            if (IT_DDATA(it_a)[0] < IT_DDATA(it_b)[0]) {
                DDATA(rtn_ca)[it_a->index] = 1;
            } else {
                DDATA(rtn_ca)[it_a->index] = 0;
            }

            CArrayIterator_NEXT(it_a);
            CArrayIterator_NEXT(it_b);

            if (!CArrayIterator_NOTDONE(it_b)) {
                CArrayIterator_RESET(it_b);
            }
        } while(CArrayIterator_NOTDONE(it_a));
    }

    if (CArray_DESCR(ca_b)->numElements > CArray_DESCR(ca_a)->numElements) {
        rtn_ca = CArray_Zeros(CArray_DIMS(ca_b), CArray_NDIM(ca_b), 'd', NULL, &rtn_ptr);
        do {
            if (IT_DDATA(it_a)[0] < IT_DDATA(it_b)[0]) {
                DDATA(rtn_ca)[it_b->index] = 1;
            } else {
                DDATA(rtn_ca)[it_b->index] = 0;
            }

            i++;
            CArrayIterator_NEXT(it_b);
            
            if (i == CArray_DIMS(ca_b)[1]) {
                i = 0;
                CArrayIterator_NEXT(it_a);
            }

        } while(CArrayIterator_NOTDONE(it_b));
    }

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::lessEqualMatrix
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, lessEqualMatrix)
{
    int i = 0;

    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    CArrayIterator *it_a = CArray_NewIter(ca_a);
    CArrayIterator *it_b = CArray_NewIter(ca_b);

    if (CArray_DESCR(ca_a)->numElements >= CArray_DESCR(ca_b)->numElements) {
        rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);
        do {
            if (IT_DDATA(it_a)[0] <= IT_DDATA(it_b)[0]) {
                DDATA(rtn_ca)[it_a->index] = 1;
            } else {
                DDATA(rtn_ca)[it_a->index] = 0;
            }

            CArrayIterator_NEXT(it_a);
            CArrayIterator_NEXT(it_b);

            if (!CArrayIterator_NOTDONE(it_b)) {
                CArrayIterator_RESET(it_b);
            }
        } while(CArrayIterator_NOTDONE(it_a));
    }

    if (CArray_DESCR(ca_b)->numElements > CArray_DESCR(ca_a)->numElements) {
        rtn_ca = CArray_Zeros(CArray_DIMS(ca_b), CArray_NDIM(ca_b), 'd', NULL, &rtn_ptr);
        do {
            if (IT_DDATA(it_a)[0] <= IT_DDATA(it_b)[0]) {
                DDATA(rtn_ca)[it_b->index] = 1;
            } else {
                DDATA(rtn_ca)[it_b->index] = 0;
            }

            i++;
            CArrayIterator_NEXT(it_b);
            
            if (i == CArray_DIMS(ca_b)[1]) {
                i = 0;
                CArrayIterator_NEXT(it_a);
            }

        } while(CArrayIterator_NOTDONE(it_b));
    }

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::equalVector
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, equalVector)
{
    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);

    CArrayIterator *it_a = CArray_NewIter(ca_a);
    CArrayIterator *it_b = CArray_NewIter(ca_b);

    do {
        if (IT_DDATA(it_a)[0] == IT_DDATA(it_b)[0]) {
            DDATA(rtn_ca)[it_a->index] = 1.0;
        } else {
            DDATA(rtn_ca)[it_a->index] = 0.0;
        }

        CArrayIterator_NEXT(it_a);
        CArrayIterator_NEXT(it_b);

        if (!CArrayIterator_NOTDONE(it_b)) {
            CArrayIterator_RESET(it_b);
        }
    } while(CArrayIterator_NOTDONE(it_a));

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::notEqualVector
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, notEqualVector)
{
    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);

    CArrayIterator *it_a = CArray_NewIter(ca_a);
    CArrayIterator *it_b = CArray_NewIter(ca_b);

    do {
        if (IT_DDATA(it_a)[0] != IT_DDATA(it_b)[0]) {
            DDATA(rtn_ca)[it_a->index] = 1.0;
        } else {
            DDATA(rtn_ca)[it_a->index] = 0.0;
        }

        CArrayIterator_NEXT(it_a);
        CArrayIterator_NEXT(it_b);

        if (!CArrayIterator_NOTDONE(it_b)) {
            CArrayIterator_RESET(it_b);
        }
    } while(CArrayIterator_NOTDONE(it_a));

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::greaterVector
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, greaterVector)
{
    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);

    CArrayIterator *it_a = CArray_NewIter(ca_a);
    CArrayIterator *it_b = CArray_NewIter(ca_b);

    do {
        if (IT_DDATA(it_a)[0] > IT_DDATA(it_b)[0]) {
            DDATA(rtn_ca)[it_a->index] = 1.0;
        } else {
            DDATA(rtn_ca)[it_a->index] = 0.0;
        }

        CArrayIterator_NEXT(it_a);
        CArrayIterator_NEXT(it_b);

        if (!CArrayIterator_NOTDONE(it_b)) {
            CArrayIterator_RESET(it_b);
        }
    } while(CArrayIterator_NOTDONE(it_a));

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::greaterEqualVector
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, greaterEqualVector)
{
    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);

    CArrayIterator *it_a = CArray_NewIter(ca_a);
    CArrayIterator *it_b = CArray_NewIter(ca_b);

    do {
        if (IT_DDATA(it_a)[0] >= IT_DDATA(it_b)[0]) {
            DDATA(rtn_ca)[it_a->index] = 1.0;
        } else {
            DDATA(rtn_ca)[it_a->index] = 0.0;
        }

        CArrayIterator_NEXT(it_a);
        CArrayIterator_NEXT(it_b);

        if (!CArrayIterator_NOTDONE(it_b)) {
            CArrayIterator_RESET(it_b);
        }
    } while(CArrayIterator_NOTDONE(it_a));

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::lessVector
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, lessVector)
{
    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);

    CArrayIterator *it_a = CArray_NewIter(ca_a);
    CArrayIterator *it_b = CArray_NewIter(ca_b);

    do {
        if (IT_DDATA(it_a)[0] < IT_DDATA(it_b)[0]) {
            DDATA(rtn_ca)[it_a->index] = 1.0;
        } else {
            DDATA(rtn_ca)[it_a->index] = 0.0;
        }

        CArrayIterator_NEXT(it_a);
        CArrayIterator_NEXT(it_b);

        if (!CArrayIterator_NOTDONE(it_b)) {
            CArrayIterator_RESET(it_b);
        }
    } while(CArrayIterator_NOTDONE(it_a));

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::lessEqualVector
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, lessEqualVector)
{
    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);

    CArrayIterator *it_a = CArray_NewIter(ca_a);
    CArrayIterator *it_b = CArray_NewIter(ca_b);

    do {
        if (IT_DDATA(it_a)[0] <= IT_DDATA(it_b)[0]) {
            DDATA(rtn_ca)[it_a->index] = 1.0;
        } else {
            DDATA(rtn_ca)[it_a->index] = 0.0;
        }

        CArrayIterator_NEXT(it_a);
        CArrayIterator_NEXT(it_b);

        if (!CArrayIterator_NOTDONE(it_b)) {
            CArrayIterator_RESET(it_b);
        }
    } while(CArrayIterator_NOTDONE(it_a));

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

PHP_METHOD(CRubix, equalColumnVector)
{

}

PHP_METHOD(CRubix, notEqualColumnVector)
{

}

/**
 * RubixML/Tensor/Matrix::greaterColumnVector
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, greaterColumnVector)
{
    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    if (CArray_NDIM(ca_a) != 2 || CArray_NDIM(ca_b) != 2)
    {
        throw_valueerror_exception("Shapes are not aligned");
        return;
    }

    if (CArray_DIM(ca_a, 1) != CArray_DIM(ca_b, 0)) {
        throw_valueerror_exception("Shapes are not aligned");
        return;
    }

    rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);

    CArrayIterator *it_a = CArray_NewIter(ca_a);
    CArrayIterator *it_b = CArray_NewIter(ca_b);

    do {
        if (IT_DDATA(it_a)[0] > IT_DDATA(it_b)[0]) {
            DDATA(rtn_ca)[it_a->index] = 1.0;
        } else {
            DDATA(rtn_ca)[it_a->index] = 0.0;
        }

        CArrayIterator_NEXT(it_a);


        if (!(it_a->index % CArray_DIM(ca_a, 1))) {
            CArrayIterator_NEXT(it_b);
        }
    } while(CArrayIterator_NOTDONE(it_a));

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::greaterColumnVector
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, greaterEqualColumnVector)
{
    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    if (CArray_NDIM(ca_a) != 2 || CArray_NDIM(ca_b) != 2)
    {
        throw_valueerror_exception("Shapes are not aligned");
        return;
    }

    if (CArray_DIM(ca_a, 1) != CArray_DIM(ca_b, 0)) {
        throw_valueerror_exception("Shapes are not aligned");
        return;
    }

    rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);

    CArrayIterator *it_a = CArray_NewIter(ca_a);
    CArrayIterator *it_b = CArray_NewIter(ca_b);

    do {
        if (IT_DDATA(it_a)[0] >= IT_DDATA(it_b)[0]) {
            DDATA(rtn_ca)[it_a->index] = 1.0;
        } else {
            DDATA(rtn_ca)[it_a->index] = 0.0;
        }

        CArrayIterator_NEXT(it_a);


        if (!(it_a->index % CArray_DIM(ca_a, 1))) {
            CArrayIterator_NEXT(it_b);
        }
    } while(CArrayIterator_NOTDONE(it_a));

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::lessColumnVector
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, lessColumnVector)
{
    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    if (CArray_NDIM(ca_a) != 2 || CArray_NDIM(ca_b) != 2)
    {
        throw_valueerror_exception("Shapes are not aligned");
        return;
    }

    if (CArray_DIM(ca_a, 1) != CArray_DIM(ca_b, 0)) {
        throw_valueerror_exception("Shapes are not aligned");
        return;
    }

    rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);

    CArrayIterator *it_a = CArray_NewIter(ca_a);
    CArrayIterator *it_b = CArray_NewIter(ca_b);

    do {
        if (IT_DDATA(it_a)[0] < IT_DDATA(it_b)[0]) {
            DDATA(rtn_ca)[it_a->index] = 1.0;
        } else {
            DDATA(rtn_ca)[it_a->index] = 0.0;
        }

        CArrayIterator_NEXT(it_a);


        if (!(it_a->index % CArray_DIM(ca_a, 1))) {
            CArrayIterator_NEXT(it_b);
        }
    } while(CArrayIterator_NOTDONE(it_a));

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::lessEqualColumnVector
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, lessEqualColumnVector)
{
    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    if (CArray_NDIM(ca_a) != 2 || CArray_NDIM(ca_b) != 2)
    {
        throw_valueerror_exception("Shapes are not aligned");
        return;
    }

    if (CArray_DIM(ca_a, 1) != CArray_DIM(ca_b, 0)) {
        throw_valueerror_exception("Shapes are not aligned");
        return;
    }

    rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);

    CArrayIterator *it_a = CArray_NewIter(ca_a);
    CArrayIterator *it_b = CArray_NewIter(ca_b);

    do {
        if (IT_DDATA(it_a)[0] <= IT_DDATA(it_b)[0]) {
            DDATA(rtn_ca)[it_a->index] = 1.0;
        } else {
            DDATA(rtn_ca)[it_a->index] = 0.0;
        }

        CArrayIterator_NEXT(it_a);


        if (!(it_a->index % CArray_DIM(ca_a, 1))) {
            CArrayIterator_NEXT(it_b);
        }
    } while(CArrayIterator_NOTDONE(it_a));

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::equalScalar
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, equalScalar)
{
    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);

    CArrayIterator *it_a = CArray_NewIter(ca_a);

    if (CArray_TYPE(ca_b) == TYPE_DOUBLE_INT) {
        do {
            if (IT_DDATA(it_a)[0] == DDATA(ca_b)[0]) {
                DDATA(rtn_ca)[it_a->index] = 1.0;
            } else {
                DDATA(rtn_ca)[it_a->index] = 0.0;
            }

            CArrayIterator_NEXT(it_a);
        } while (CArrayIterator_NOTDONE(it_a));
    }

    if (CArray_TYPE(ca_b) == TYPE_INTEGER_INT) {
        do {
            if (IT_DDATA(it_a)[0] == IDATA(ca_b)[0]) {
                DDATA(rtn_ca)[it_a->index] = 1.0;
            } else {
                DDATA(rtn_ca)[it_a->index] = 0.0;
            }

            CArrayIterator_NEXT(it_a);
        } while (CArrayIterator_NOTDONE(it_a));
    }

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::notEqualScalar
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, notEqualScalar)
{
    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    if (CArray_NDIM(ca_b) != 0)
    {
        throw_valueerror_exception("Shapes are not aligned");
        return;
    }

    rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);
    CArrayIterator *it_a = CArray_NewIter(ca_a);

    if (CArray_TYPE(ca_b) == TYPE_DOUBLE_INT) {
        do {
            if (IT_DDATA(it_a)[0] != DDATA(ca_b)[0]) {
                DDATA(rtn_ca)[it_a->index] = 1.0;
            } else {
                DDATA(rtn_ca)[it_a->index] = 0.0;
            }

            CArrayIterator_NEXT(it_a);
        } while (CArrayIterator_NOTDONE(it_a));
    }
    if (CArray_TYPE(ca_b) == TYPE_INTEGER_INT) {
        do {
            
            if (IT_DDATA(it_a)[0] != IDATA(ca_b)[0]) {
                DDATA(rtn_ca)[it_a->index] = 1.0;
            } else {
                DDATA(rtn_ca)[it_a->index] = 0.0;
            }

            CArrayIterator_NEXT(it_a);
        } while (CArrayIterator_NOTDONE(it_a));
    }

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::greaterScalar
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, greaterScalar)
{
    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);

    CArrayIterator *it_a = CArray_NewIter(ca_a);

    if (CArray_TYPE(ca_b) == TYPE_DOUBLE_INT) {
        do {
            if (IT_DDATA(it_a)[0] > DDATA(ca_b)[0]) {
                DDATA(rtn_ca)[it_a->index] = 1.0;
            } else {
                DDATA(rtn_ca)[it_a->index] = 0.0;
            }

            CArrayIterator_NEXT(it_a);
        } while (CArrayIterator_NOTDONE(it_a));
    }

    if (CArray_TYPE(ca_b) == TYPE_INTEGER_INT) {
        do {
            if (IT_DDATA(it_a)[0] > IDATA(ca_b)[0]) {
                DDATA(rtn_ca)[it_a->index] = 1.0;
            } else {
                DDATA(rtn_ca)[it_a->index] = 0.0;
            }

            CArrayIterator_NEXT(it_a);
        } while (CArrayIterator_NOTDONE(it_a));
    }

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::greaterEqualScalar
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, greaterEqualScalar)
{
    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);

    CArrayIterator *it_a = CArray_NewIter(ca_a);

    if (CArray_TYPE(ca_b) == TYPE_DOUBLE_INT) {
        do {
            if (IT_DDATA(it_a)[0] >= DDATA(ca_b)[0]) {
                DDATA(rtn_ca)[it_a->index] = 1.0;
            } else {
                DDATA(rtn_ca)[it_a->index] = 0.0;
            }

            CArrayIterator_NEXT(it_a);
        } while (CArrayIterator_NOTDONE(it_a));
    }

    if (CArray_TYPE(ca_b) == TYPE_INTEGER_INT) {
        do {
            if (IT_DDATA(it_a)[0] >= IDATA(ca_b)[0]) {
                DDATA(rtn_ca)[it_a->index] = 1.0;
            } else {
                DDATA(rtn_ca)[it_a->index] = 0.0;
            }

            CArrayIterator_NEXT(it_a);
        } while (CArrayIterator_NOTDONE(it_a));
    }

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::lessScalar
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, lessScalar)
{
    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);

    CArrayIterator *it_a = CArray_NewIter(ca_a);

    if (CArray_TYPE(ca_b) == TYPE_DOUBLE_INT) {
        do {
            if (IT_DDATA(it_a)[0] > DDATA(ca_b)[0]) {
                DDATA(rtn_ca)[it_a->index] = 1.0;
            } else {
                DDATA(rtn_ca)[it_a->index] = 0.0;
            }

            CArrayIterator_NEXT(it_a);
        } while (CArrayIterator_NOTDONE(it_a));
    }

    if (CArray_TYPE(ca_b) == TYPE_INTEGER_INT) {
        do {
            if (IT_DDATA(it_a)[0] < IDATA(ca_b)[0]) {
                DDATA(rtn_ca)[it_a->index] = 1.0;
            } else {
                DDATA(rtn_ca)[it_a->index] = 0.0;
            }

            CArrayIterator_NEXT(it_a);
        } while (CArrayIterator_NOTDONE(it_a));
    }

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

/**
 * RubixML/Tensor/Matrix::lessEqualScalar
 *
 * @param execute_data
 * @param return_value
 */
PHP_METHOD(CRubix, lessEqualScalar)
{
    zval *target_a, *target_b;
    MemoryPointer ptr_a, ptr_b, rtn_ptr;
    CArray *ca_a, *ca_b, *rtn_ca;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(target_a)
            Z_PARAM_ZVAL(target_b)
    ZEND_PARSE_PARAMETERS_END();

    ZVAL_TO_MEMORYPOINTER(target_a, &ptr_a, NULL);
    ZVAL_TO_MEMORYPOINTER(target_b, &ptr_b, NULL);

    ca_a = CArray_FromMemoryPointer(&ptr_a);
    ca_b = CArray_FromMemoryPointer(&ptr_b);

    rtn_ca = CArray_Zeros(CArray_DIMS(ca_a), CArray_NDIM(ca_a), 'd', NULL, &rtn_ptr);

    CArrayIterator *it_a = CArray_NewIter(ca_a);

    if (CArray_TYPE(ca_b) == TYPE_DOUBLE_INT) {
        do {
            if (IT_DDATA(it_a)[0] > DDATA(ca_b)[0]) {
                DDATA(rtn_ca)[it_a->index] = 1.0;
            } else {
                DDATA(rtn_ca)[it_a->index] = 0.0;
            }

            CArrayIterator_NEXT(it_a);
        } while (CArrayIterator_NOTDONE(it_a));
    }

    if (CArray_TYPE(ca_b) == TYPE_INTEGER_INT) {
        do {
            if (IT_DDATA(it_a)[0] <= IDATA(ca_b)[0]) {
                DDATA(rtn_ca)[it_a->index] = 1.0;
            } else {
                DDATA(rtn_ca)[it_a->index] = 0.0;
            }

            CArrayIterator_NEXT(it_a);
        } while (CArrayIterator_NOTDONE(it_a));
    }

    RETURN_MEMORYPOINTER(return_value, &rtn_ptr);
}

PHP_METHOD(CRubix, count)
{

}

PHP_METHOD(CRubix, offsetSet)
{

}

PHP_METHOD(CRubix, offsetExists)
{

}

PHP_METHOD(CRubix, offsetUnset)
{

}

PHP_METHOD(CRubix, getIterator)
{

}

ZEND_BEGIN_ARG_INFO_EX(arginfo_array_offsetGet, 0, 0, 1)
                ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()
PHP_METHOD(CRubix, offsetGet)
{
    CArray * _this_ca, * ret_ca;
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