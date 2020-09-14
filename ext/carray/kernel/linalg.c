#include "config.h"
#include "iterators.h"
#include "carray.h"
#include "common/exceptions.h"
#include "linalg.h"
#include "common/common.h"
#include "convert_type.h"
#include "alloc.h"
#include "buffer.h"
#include "matlib.h"
#include "convert.h"
#include "shape.h"
#include "common/matmul.h"

#ifdef HAVE_CBLAS
    #include "cblas.h"
    #include "common/cblas_funcs.h"
#endif

#ifdef HAVE_CLBLAS
    #include "clBLAS.h"
    #include "common/clblas_funcs.h"
#endif


#include "lapacke.h"

static float s_one;
static float s_zero;
static float s_minus_one;
static float s_ninf;
static float s_nan;
static double d_one;
static double d_zero;
static double d_minus_one;
static double d_ninf;
static double d_nan;

typedef struct linearize_data_struct
{
    int rows;
    int columns;
    int row_strides;
    int column_strides;
    int output_lead_dim;
} LINEARIZE_DATA_t;

static inline void
init_linearize_data_ex(LINEARIZE_DATA_t *lin_data,
        int rows,
        int columns,
        int row_strides,
        int column_strides,
        int output_lead_dim)
{
    lin_data->rows = rows;
    lin_data->columns = columns;
    lin_data->row_strides = row_strides;
    lin_data->column_strides = column_strides;
    lin_data->output_lead_dim = output_lead_dim;
}

static inline void
init_linearize_data(LINEARIZE_DATA_t *lin_data, int rows, int columns,
        int row_strides, int column_strides)
{
    init_linearize_data_ex(lin_data, rows, columns, row_strides, column_strides, columns);
}

void *
linearize_DOUBLE_matrix(double *dst_in,
                        double *src_in,
                        CArray * a)
{
    double *src = (double *) src_in;
    double *dst = (double *) dst_in;

    if (dst) {
        int i, j;
        double* rv = dst;
        int columns = (int)CArray_DIMS(a)[1];
        int column_strides = CArray_STRIDES(a)[1]/sizeof(double);
        int one = 1;
        for (i = 0; i < CArray_DIMS(a)[0]; i++) {
            if (column_strides > 0) {
#ifdef HAVE_BLAS
                cblas_dcopy(columns,
                             (double*)src, column_strides,
                             (double*)dst, one);
#endif
            }
            else if (column_strides < 0) {
#ifdef HAVE_BLAS
                cblas_dcopy(columns,
                             (double*)((double*)src + (columns-1)*column_strides),
                             column_strides,
                             (double*)dst, one);
#endif
            }
            else {
                /*
                 * Zero stride has undefined behavior in some BLAS
                 * implementations (e.g. OSX Accelerate), so do it
                 * manually
                 */
                for (j = 0; j < columns; ++j) {
                    memcpy((double*)dst + j, (double*)src, sizeof(double));
                }
            }

            src += CArray_STRIDES(a)[0]/sizeof(double);
            dst += CArray_DIMS(a)[1];
        }
        return rv;
    } else {
        return src;
    }
}

/**
 * DOT
 */
void
FLOAT_dot(char *ip1, int is1, char *ip2, int is2, char *op, int n)
{

}

void
DOUBLE_dot(char *ip1, int is1, char *ip2, int is2, char *op, int n)
{
#ifdef HAVE_CBLAS
    int is1b = blas_stride(is1, sizeof(double));
    int is2b = blas_stride(is2, sizeof(double));
    if (is1b && is2b)
    {
        double sum = 0.;
        while (n > 0) {
            int chunk = n < CARRAY_CBLAS_CHUNK ? n : CARRAY_CBLAS_CHUNK;

            sum += cblas_ddot(chunk,
                    (double *) ip1, is1b,
                    (double *) ip2, is2b);
            /* use char strides here */
            ip1 += chunk * is1;
            ip2 += chunk * is2;
            n -= chunk;
        }
        *((double *)op) = (double)sum;
    }
    else {
#endif
        double sum = (double)0;
        int i;
        for (i = 0; i < n; i++, ip1 += is1, ip2 += is2) {
            const double ip1r = *((double *)ip1);
            const double ip2r = *((double *)ip2);
            sum += ip1r * ip2r;
        }
        *((double *)op) = sum;
#ifdef HAVE_CBLAS
    }
#endif
}

void
INT_dot(char *ip1, int is1, char *ip2, int is2, char *op, int n)
{
    long tmp = (long)0;
    int i;

    for (i = 0; i < n; i++, ip1 += is1, ip2 += is2) {
        tmp += (long)(*((int *)ip1)) *
               (long)(*((int *)ip2));
    }
    *((int *)op) = (int) tmp;
}

/**
 * CArray Matmul
 **/ 
CArray * 
CArray_Matmul(CArray * ap1, CArray * ap2, CArray * out, MemoryPointer * ptr)
{
    CArray * result = NULL, * target1, * target2;
    int nd1, nd2, nd, typenum;
    int i, j, l, matchDim, is1, is2, axis, os;
    int * dimensions = NULL;
    CArray_DotFunc *dot;
    CArrayIterator * it1, * it2;
    char * op;

    /**if (CArray_NDIM(ap1) == 0 || CArray_NDIM(ap2) == 0) {
        throw_valueerror_exception("Scalar operands are not allowed, use '*' instead");
        return NULL;
    }**/
    typenum = CArray_ObjectType(ap1, 0);
    typenum = CArray_ObjectType(ap2, typenum);

    nd1 = CArray_NDIM(ap1);
    nd2 = CArray_NDIM(ap2);

#ifdef HAVE_BLAS
#ifndef HAVE_CLBLAS
    if (nd1 <= 2 && nd2 <= 2 && (TYPE_DOUBLE_INT == typenum || TYPE_FLOAT_INT == typenum)) {
        return cblas_matrixproduct(typenum, ap1, ap2, out, ptr);
    }
#endif
#endif

#ifdef HAVE_CLBLAS
    if (nd1 <= 2 && nd2 <= 2 && (TYPE_DOUBLE_INT == typenum || TYPE_FLOAT_INT == typenum)) {
        return clblas_matrixproduct(typenum, ap1, ap2, out, ptr);
    }
#endif
    
    if(typenum == TYPE_INTEGER_INT) {
        dot = &INT_dot;
    }
    if(typenum == TYPE_FLOAT_INT) {
        dot = &FLOAT_dot;
    }
    if(typenum == TYPE_DOUBLE_INT) {
        dot = &DOUBLE_dot;
    }

    l = CArray_DIMS(ap1)[CArray_NDIM(ap1) - 1];
    if (CArray_NDIM(ap2) > 1) {
        matchDim = CArray_NDIM(ap2) - 2;
    }
    else {
        matchDim = 0;
    }

    if (CArray_DIMS(ap2)[matchDim] != l) {
        throw_valueerror_exception("Shapes are not aligned.");
        goto fail;
    }
    nd = CArray_NDIM(ap1) + CArray_NDIM(ap2) - 2;

    if (nd > CARRAY_MAXDIMS) {
        throw_valueerror_exception("dot: too many dimensions in result");
        goto fail;
    }
    
    j = 0;
    dimensions = emalloc((CArray_NDIM(ap1)) * sizeof(int));
    for (i = 0; i < CArray_NDIM(ap1) - 1; i++) {
        dimensions[j++] = CArray_DIMS(ap1)[i];
    }
    for (i = 0; i < CArray_NDIM(ap2) - 2; i++) {
        dimensions[j++] = CArray_DIMS(ap2)[i];
    }
    if(CArray_NDIM(ap2) > 1) {
        dimensions[j++] = CArray_DIMS(ap2)[CArray_NDIM(ap2)-1];
    }

    is1 = CArray_STRIDES(ap1)[CArray_NDIM(ap1)-1];
    is2 = CArray_STRIDES(ap2)[matchDim];
    /* Choose which subtype to return */
    result = new_array_for_sum(ap1, ap2, out, nd, dimensions, typenum, &result);

    if (result == NULL) {
        goto fail;
    }

    /* Ensure that multiarray.dot(<Nx0>,<0xM>) -> zeros((N,M)) */
    if (CArray_SIZE(ap1) == 0 && CArray_SIZE(ap2) == 0) {
        memset(CArray_DATA(result), 0, CArray_NBYTES(result));
    }

    op = CArray_DATA(result);
    os = CArray_DESCR(result)->elsize;
    axis = CArray_NDIM(ap1)-1;
    it1 = CArray_IterAllButAxis(ap1, &axis);
    if (it1 == NULL) {
        goto fail;
    }

    it2 = CArray_IterAllButAxis(ap2, &matchDim);
    if (it2 == NULL) {
        goto fail;
    }

    while (it1->index < it1->size) {
        while (it2->index < it2->size) {
            dot(it1->data_pointer, is1, it2->data_pointer, is2, op, l);
            op += os;
            CArrayIterator_NEXT(it2);
        }
        CArrayIterator_NEXT(it1);
        CArrayIterator_RESET(it2);
    }
    CArrayIterator_FREE(it1);
    CArrayIterator_FREE(it2);

    efree(dimensions);

    if (ptr != NULL) {
        add_to_buffer(ptr, result, sizeof(CArray));
    }

    return result;
fail:
    if(dimensions != NULL) {
        efree(dimensions);
    }
    return NULL;
}

/**
 * Compute matrix inverse
 **/ 
CArray *
CArray_Inv(CArray * a, MemoryPointer * out) {
    int status, casted = 0;
    int * ipiv = emalloc(sizeof(int) * CArray_DIMS(a)[0]);
    CArray * identity = CArray_Eye(CArray_DIMS(a)[0], CArray_DIMS(a)[0], 0, NULL, out);
    CArray * target;
    int order;
    double * data = emalloc(sizeof(double) * CArray_SIZE(a));

    if (CArray_NDIM(a) != 2) {
        throw_valueerror_exception("Matrix must have 2 dimensions");
        return NULL;
    }

    if (CArray_DESCR(a)->type_num != TYPE_DOUBLE_INT) {
        CArrayDescriptor *descr = CArray_DescrFromType(TYPE_DOUBLE_INT);
        if (CArray_CHKFLAGS(a, CARRAY_ARRAY_F_CONTIGUOUS)) {
            target = CArray_NewLikeArray(a, CARRAY_FORTRANORDER, descr, 0);
        }
        if (CArray_CHKFLAGS(a, CARRAY_ARRAY_C_CONTIGUOUS)) {
            target = CArray_NewLikeArray(a, CARRAY_CORDER, descr, 0);
        }
        if(CArray_CastTo(target, a) < 0) {
            return NULL;
        }
        casted = 1;
    } else {
        target = a;
    }

    if (!CArray_CHKFLAGS(a, CARRAY_ARRAY_C_CONTIGUOUS)) {
        linearize_DOUBLE_matrix(data, DDATA(target), target);
    } else {
        memcpy(data, DDATA(target), sizeof(double) * CArray_SIZE(target));
    }

    status = LAPACKE_dgesv(LAPACK_ROW_MAJOR,
            CArray_DIMS(target)[0],
            CArray_DIMS(target)[0],
            data,
            CArray_DIMS(target)[0],
            ipiv,
            DDATA(identity),
            CArray_DIMS(target)[0]);

    if (casted) {
        CArray_Free(target);
    }
    efree(data);
    efree(ipiv);
    return identity;
}

/**
 *
 * @param a
 * @param norm 0 =  largest absolute value, 1 = Frobenius norm, 2 = infinity norm, 3 = 1-norm
 * @param out
 * @return
 */
CArray *
CArray_Norm(CArray * a, int norm, MemoryPointer * out)
{
    double result;
    char norm_c;
    CArray * target, * rtn;
    CArrayDescriptor * rtn_descr;
    int casted = 0;
    double * data;

    switch(norm) {
        case 0:
            norm_c = 'M';
            break;
        case 1:
            norm_c = 'F';
            break;
        case 2:
            norm_c = 'I';
            break;
        case 3:
            norm_c = '1';
            break;
        default:
            throw_valueerror_exception("Can't find a NORM algorithm with the provided name.");
            goto fail;
    }

    if (CArray_NDIM(a) != 2) {
        throw_valueerror_exception("Matrix must have 2 dimensions");
        goto fail;
    }

    if (CArray_DESCR(a)->type_num != TYPE_DOUBLE_INT) {
        CArrayDescriptor *descr = CArray_DescrFromType(TYPE_DOUBLE_INT);
        if (CArray_CHKFLAGS(a, CARRAY_ARRAY_F_CONTIGUOUS)) {
            target = CArray_NewLikeArray(a, CARRAY_FORTRANORDER, descr, 0);
        }
        if (CArray_CHKFLAGS(a, CARRAY_ARRAY_C_CONTIGUOUS)) {
            target = CArray_NewLikeArray(a, CARRAY_CORDER, descr, 0);
        }
        if(CArray_CastTo(target, a) < 0) {
            goto fail;
        }
        casted = 1;
    } else {
        target = a;
    }

    if (!CArray_CHKFLAGS(a, CARRAY_ARRAY_C_CONTIGUOUS)) {
        data = emalloc(sizeof(double) * CArray_SIZE(target));
        linearize_DOUBLE_matrix(data, DDATA(target), target);
    } else {
        data = DDATA(target);
    }


    rtn = emalloc(sizeof(CArray));
    rtn_descr = CArray_DescrFromType(TYPE_DOUBLE_INT);
    rtn = CArray_NewFromDescr_int(rtn, rtn_descr, 0, NULL, NULL, NULL, 0, NULL, 0, 0);
    DDATA(rtn)[0] = LAPACKE_dlange(LAPACK_ROW_MAJOR,
            norm_c,
            CArray_DIMS(target)[0],
            CArray_DIMS(target)[1],
            data,
            CArray_DIMS(target)[0]);

    if (casted) {
        CArray_Free(target);
    }

    if(out != NULL) {
        add_to_buffer(out, rtn, sizeof(CArray));
    }
    return rtn;
fail:
    return NULL;
}

CArray *
CArray_Det(CArray * a, MemoryPointer * out)
{
    double result;
    int * ipiv = emalloc(sizeof(int) * CArray_DIMS(a)[0]);
    lapack_int status;
    double sign;
    int i;
    CArray * target, * rtn;
    CArrayDescriptor * rtn_descr;
    int casted = 0;
    double * data;

    if (CArray_NDIM(a) != 2) {
        throw_valueerror_exception("Expected matrix with 2 dimensions");
        goto fail;
    }

    if (CArray_DIMS(a)[0] != CArray_DIMS(a)[1]) {
        throw_valueerror_exception("Expected square matrix");
        goto fail;
    }
    if (CArray_DESCR(a)->type_num != TYPE_DOUBLE_INT) {
        CArrayDescriptor *descr = CArray_DescrFromType(TYPE_DOUBLE_INT);
        if (CArray_CHKFLAGS(a, CARRAY_ARRAY_F_CONTIGUOUS)) {
            target = CArray_NewLikeArray(a, CARRAY_FORTRANORDER, descr, 0);
        }
        if (CArray_CHKFLAGS(a, CARRAY_ARRAY_C_CONTIGUOUS)) {
            target = CArray_NewLikeArray(a, CARRAY_CORDER, descr, 0);
        }
        if(CArray_CastTo(target, a) < 0) {
            goto fail;
        }
        casted = 1;
    } else {
        target = a;
    }

    if (!CArray_CHKFLAGS(a, CARRAY_ARRAY_C_CONTIGUOUS)) {
        data = emalloc(sizeof(double) * CArray_SIZE(target));
        linearize_DOUBLE_matrix(data, DDATA(target), target);
    } else {
        data = DDATA(target);
    }


    status = LAPACKE_dgetrf(
            LAPACK_ROW_MAJOR,
            CArray_DIMS(a)[0],
            CArray_DIMS(a)[0],
            data,
            CArray_DIMS(a)[0],
            ipiv
            );

    int change_sign = 0;

    for (i = 0; i < CArray_DIMS(a)[0]; i++)
    {
        change_sign += (ipiv[i] != (i+1));
    }

    sign = (change_sign % 2)? -1.0 : 1.0;

    double acc_sign = sign;
    double acc_logdet = 0.0;
    double * src = data;

    for (i = 0; i < CArray_DIMS(a)[0]; i++) {
        double abs_element = *src;
        if (abs_element < 0.0) {
            acc_sign = -acc_sign;
            abs_element = -abs_element;
        }
        acc_logdet += log(abs_element);
        src += CArray_DIMS(a)[0]+1;
    }

    sign = acc_sign;

    rtn = emalloc(sizeof(CArray));
    rtn_descr = CArray_DescrFromType(TYPE_DOUBLE_INT);
    rtn = CArray_NewFromDescr_int(rtn, rtn_descr, 0, NULL, NULL, NULL, 0, NULL, 0, 0);

    DDATA(rtn)[0] = sign * exp(acc_logdet);

    if (casted) {
        CArray_Free(target);
    }

    if(out != NULL) {
        add_to_buffer(out, rtn, sizeof(CArray));
    }

    if (!CArray_CHKFLAGS(a, CARRAY_ARRAY_C_CONTIGUOUS)) {
        efree(data);
    }

    efree(ipiv);
    return rtn;
fail:
    return NULL;
}


/**
 * VDOT
 */
void
DOUBLE_vdot(char *ip1, int is1, char *ip2, int is2,
             char *op, int n)
{
    int is1b = blas_stride(is1, sizeof(double));
    int is2b = blas_stride(is2, sizeof(double));

    if (is1b && is2b) {
        double sum[2] = {0., 0.};  /* double for stability */

        while (n > 0) {
            int chunk = n < CARRAY_CBLAS_CHUNK ? n : CARRAY_CBLAS_CHUNK;
            double tmp[2];
#ifdef HAVE_CBLAS
            cblas_zdotc_sub((int)n, ip1, is1b, ip2, is2b, tmp);
#endif
            sum[0] += (double)tmp[0];
            sum[1] += (double)tmp[1];
            /* use char strides here */
            ip1 += chunk * is1;
            ip2 += chunk * is2;
            n -= chunk;
        }
        ((double *)op)[0] = (double)sum[0];
        ((double *)op)[1] = (double)sum[1];
    }
}

CArray *
CArray_Vdot(CArray * target_a, CArray * target_b, MemoryPointer * out)
{
    CArray * a = NULL, * b = NULL;
    int casted_a = 0, casted_b = 0;
    int typenum;
    char *ip1, *ip2, *op;
    int n, stride1, stride2;
    CArray *op1, *op2;
    int newdimptr[1] = {-1};
    CArray_Dims newdims = {newdimptr, 1};
    CArray *ap1 = NULL, *ap2  = NULL, *ret = NULL;
    CArrayDescriptor *type;
    CArray_DotFunc *vdot;

    if (CArray_DESCR(target_a)->type_num != TYPE_DOUBLE_INT) {
        CArrayDescriptor *descr = CArray_DescrFromType(TYPE_DOUBLE_INT);
        if (CArray_CHKFLAGS(target_a, CARRAY_ARRAY_F_CONTIGUOUS)) {
            a = CArray_NewLikeArray(target_a, CARRAY_FORTRANORDER, descr, 0);
        }
        if (CArray_CHKFLAGS(target_a, CARRAY_ARRAY_C_CONTIGUOUS)) {
            a = CArray_NewLikeArray(target_a, CARRAY_CORDER, descr, 0);
        }
        if(CArray_CastTo(a, target_a) < 0) {
            goto fail;
        }
        casted_a = 1;
    } else {
        a = target_a;
    }

    if (CArray_DESCR(target_b)->type_num != TYPE_DOUBLE_INT) {
        CArrayDescriptor *descr = CArray_DescrFromType(TYPE_DOUBLE_INT);
        if (CArray_CHKFLAGS(target_b, CARRAY_ARRAY_F_CONTIGUOUS)) {
            b = CArray_NewLikeArray(target_b, CARRAY_FORTRANORDER, descr, 0);
        }
        if (CArray_CHKFLAGS(target_b, CARRAY_ARRAY_C_CONTIGUOUS)) {
            b = CArray_NewLikeArray(target_b, CARRAY_CORDER, descr, 0);
        }
        if(CArray_CastTo(b, target_b) < 0) {
            goto fail;
        }
        casted_b = 1;
    } else {
        b = target_b;
    }

    op1 = a;
    op2 = b;

    /*
     * Conjugating dot product using the BLAS for vectors.
     * Flattens both op1 and op2 before dotting.
     */
    typenum = CArray_ObjectType(op1, 0);
    typenum = CArray_ObjectType(op2, typenum);

    type = CArray_DescrFromType(typenum);

    ap1 = op1;
    if (ap1 == NULL) {
        CArrayDescriptor_FREE(type);
        goto fail;
    }

    op1 = CArray_Newshape(ap1, newdims.ptr, newdims.len, CARRAY_CORDER, NULL);

    if (op1 == NULL) {
        CArrayDescriptor_FREE(type);
        goto fail;
    }

    ap1 = op1;

    ap2 = op2;
    if (ap2 == NULL) {
        goto fail;
    }

    op2 = CArray_Newshape(ap2, newdims.ptr, newdims.len, CARRAY_CORDER, NULL);

    if (op2 == NULL) {
        goto fail;
    }

    ap2 = op2;

    if (CArray_DIM(ap2, 0) != CArray_DIM(ap1, 0)) {
        throw_valueerror_exception("vectors have different lengths");
        goto fail;
    }

    /* array scalar output */
    ret = new_array_for_sum(ap1, ap2, NULL, 0, (int *)NULL, typenum, NULL);

    if (ret == NULL) {
        goto fail;
    }


    n = CArray_DIM(ap1, 0);
    stride1 = CArray_STRIDE(ap1, 0);
    stride2 = CArray_STRIDE(ap2, 0);
    ip1 = CArray_DATA(ap1);
    ip2 = CArray_DATA(ap2);
    op = CArray_DATA(ret);

    switch (typenum) {
        case TYPE_DOUBLE_INT:
            vdot = (CArray_DotFunc *)DOUBLE_vdot;
            break;

        default:
            throw_valueerror_exception("function not available for this data type");
            goto fail;
    }

    vdot(ip1, stride1, ip2, stride2, op, n);

    if (casted_a) {
        CArray_Free(a);
    }

    if (casted_b) {
        CArray_Free(b);
    }

    if (out != NULL) {
        add_to_buffer(out, ret, sizeof(CArray));
    }


    CArrayDescriptor_FREE(type);
    CArrayDescriptor_DECREF(CArray_DESCR(ap1));
    CArrayDescriptor_DECREF(CArray_DESCR(ap2));
    CArray_Free(ap1);
    CArray_Free(ap2);


    return ret;
fail:
    //Py_XDECREF(ap1);
    //Py_XDECREF(ap2);
    //Py_XDECREF(ret);
    return NULL;
}


CArray **
CArray_Svd(CArray * a, int full_matrices, int compute_uv, MemoryPointer * out)
{
    int m, n, casted = 0;
    int lda, ldu, ldvt, info, lwork;
    int * iwork;
    double * s, * u, * vt, * data = NULL;
    CArray * u_ca, * s_ca, *vh_ca, ** rtn, * target;

    if (CArray_NDIM(a) != 2) {
        throw_valueerror_exception("Expected 2D array");
        return NULL;
    }

    if (CArray_DESCR(a)->type_num != TYPE_DOUBLE_INT) {
        CArrayDescriptor *descr = CArray_DescrFromType(TYPE_DOUBLE_INT);

        if (CArray_CHKFLAGS(a, CARRAY_ARRAY_F_CONTIGUOUS)) {
            target = CArray_NewLikeArray(a, CARRAY_FORTRANORDER, descr, 0);
        }
        if (CArray_CHKFLAGS(a, CARRAY_ARRAY_C_CONTIGUOUS)) {
            target = CArray_NewLikeArray(a, CARRAY_CORDER, descr, 0);
        }
        if(CArray_CastTo(target, a) < 0) {
            return NULL;
        }
        casted = 1;
    } else {
        target = a;
    }

    data = emalloc(sizeof(double) * CArray_SIZE(a));
    if (!CArray_CHKFLAGS(a, CARRAY_ARRAY_C_CONTIGUOUS)) {
        linearize_DOUBLE_matrix(data, DDATA(target), target);
    } else {
        memcpy(data, DDATA(target), sizeof(double) * CArray_SIZE(target));
    }

    m = CArray_DIMS(target)[0];
    n = CArray_DIMS(target)[1];

    lda = m;
    ldu = m;
    ldvt = n;
    lwork = -1;

    s = emalloc(sizeof(double) * n);
    u = emalloc(sizeof(double) * (ldu * m));
    vt = emalloc(sizeof(double) * (ldvt * n));

    info = LAPACKE_dgesdd(LAPACK_ROW_MAJOR, 'S', m, n, data, lda, s, u, ldu, vt, ldvt);

    if( info > 0 ) {
        throw_valueerror_exception( "The algorithm computing SVD failed to converge." );
        return NULL;
    }

    u_ca = emalloc(sizeof(CArray));
    u_ca = CArray_NewFromDescr_int(u_ca, CArray_DESCR(target), CArray_NDIM(target), CArray_DIMS(target), CArray_STRIDES(target),
                                   NULL, 0, NULL, 1, 0);
    efree(u_ca->data);
    u_ca->data = (char *)u;
    CArrayDescriptor_INCREF(CArray_DESCR(target));

    s_ca = emalloc(sizeof(CArray));
    s_ca = CArray_NewFromDescr_int(s_ca, CArray_DESCR(target), 1, &n, NULL,
                                   NULL, 0, NULL, 1, 0);
    efree(s_ca->data);
    s_ca->data = (char *)s;
    CArrayDescriptor_INCREF(CArray_DESCR(target));

    vh_ca = emalloc(sizeof(CArray));
    vh_ca = CArray_NewFromDescr_int(vh_ca, CArray_DESCR(target), CArray_NDIM(target), CArray_DIMS(target), CArray_STRIDES(target),
                                    NULL, 0, NULL, 1, 0);
    efree(vh_ca->data);
    vh_ca->data = (char *)vt;
    CArrayDescriptor_INCREF(CArray_DESCR(target));

    if (out != NULL) {
        add_to_buffer(&(out[0]), u_ca, sizeof(CArray));
        add_to_buffer(&(out[1]), s_ca, sizeof(CArray));
        add_to_buffer(&(out[2]), vh_ca, sizeof(CArray));
    }

    rtn = emalloc(sizeof(CArray *) * 3);
    rtn[0] = u_ca;
    rtn[1] = s_ca;
    rtn[2] = vh_ca;

    if (casted) {
        CArrayDescriptor_DECREF(CArray_DESCR(target));
        CArray_Free(target);
    }

    if (data != NULL) {
        efree(data);
    }
    return rtn;
}

CArray *
CArray_InnerProduct(CArray *op1, CArray *op2, MemoryPointer *out)
{
    CArray *ap1 = NULL;
    CArray *ap2 = NULL;
    int typenum;
    CArrayDescriptor *typec = NULL;
    CArray* ap2t = NULL;
    int dims[CARRAY_MAXDIMS];
    CArray_Dims newaxes = {dims, 0};
    int i;
    CArray* ret = NULL;

    typenum = CArray_ObjectType(op1, 0);
    typenum = CArray_ObjectType(op2, typenum);
    typec = CArray_DescrFromType(typenum);
    if (typec == NULL) {
        throw_typeerror_exception("Cannot find a common data type.");
        goto fail;
    }

    CArrayDescriptor_INCREF(typec);
    ap1 = CArray_FromAny(op1, typec, 0, 0, CARRAY_ARRAY_ALIGNED);
    if (ap1 == NULL) {
        CArrayDescriptor_DECREF(typec);
        goto fail;
    }
    ap2 = (CArray *)CArray_FromAny(op2, typec, 0, 0, CARRAY_ARRAY_ALIGNED);
    if (ap2 == NULL) {
        goto fail;
    }

    newaxes.len = CArray_NDIM(ap2);
    if ((CArray_NDIM(ap1) >= 1) && (newaxes.len >= 2)) {
        for (i = 0; i < newaxes.len - 2; i++) {
            dims[i] = (int)i;
        }
        dims[newaxes.len - 2] = newaxes.len - 1;
        dims[newaxes.len - 1] = newaxes.len - 2;

        ap2t = CArray_Transpose(ap2, &newaxes, NULL);
        if (ap2t == NULL) {
            goto fail;
        }
    }
    else {
        ap2t = ap2;
        CArray_INCREF(ap2);
    }

    ret = CArray_Matmul(ap1, ap2t, NULL, out);

    if (ret == NULL) {
        goto fail;
    }

    CArray_DECREF(op2);
    CArrayDescriptor_DECREF(CArray_DESCR(op2));

    CArrayDescriptor_DECREF(typec);
    CArrayDescriptor_FREE(typec);
    return ret;
fail:
    throw_valueerror_exception("");
    return NULL;
}

static CArray *
DOUBLE_solve(CArray *a, CArray *b, MemoryPointer *rtn_ptr)
{
    CArray *out;
    int i;
    int * ipiv = emalloc(sizeof(int) * CArray_DIMS(a)[0]);
    int status;
    double *dataA = emalloc(sizeof(double) * CArray_SIZE(a));
    double *dataB = emalloc(sizeof(double) * CArray_SIZE(b));

    if (!CArray_CHKFLAGS(a, CARRAY_ARRAY_C_CONTIGUOUS)) {
        linearize_DOUBLE_matrix(dataA, DDATA(a), a);
    } else {
        memcpy(dataA, DDATA(a), sizeof(double) * CArray_SIZE(a));
    }

    if (!CArray_CHKFLAGS(b, CARRAY_ARRAY_C_CONTIGUOUS)) {
        linearize_DOUBLE_matrix(dataB, DDATA(b), b);
    } else {
        memcpy(dataB, DDATA(b), sizeof(double) * CArray_SIZE(b));
    }

    int n = CArray_DIMS(a)[0];
    int nrhs = 1;

    status = LAPACKE_dgesv(LAPACK_ROW_MAJOR,
                           n,
                           nrhs,
                           dataA,
                           CArray_DIMS(a)[0],
                           ipiv,
                           dataB,
                           1);

    if (status > 0) {
        throw_valueerror_exception("The diagonal element of the triangular factor of A is zero, so that A is singular");
    }

    int *dims = emalloc(sizeof(int));
    *dims = CArray_DIMS(a)[0];
    out = CArray_Zeros(dims, 1, TYPE_DOUBLE, NULL, rtn_ptr);
    efree(dims);

    for (i = 0; i < CArray_DIMS(a)[0]; i++) {
        DDATA(out)[i] = dataB[i];
    }    

    return out;
}

CArray *
CArray_Solve(CArray *target_a, CArray *target_b, MemoryPointer * out)
{
    CArray *rtn;
    int casted_a = 0, casted_b = 0;
    CArray *a = NULL, *b = NULL;

    if (CArray_DESCR(target_a)->type_num != TYPE_DOUBLE_INT) {
        CArrayDescriptor *descr = CArray_DescrFromType(TYPE_DOUBLE_INT);
        if (CArray_CHKFLAGS(target_a, CARRAY_ARRAY_F_CONTIGUOUS)) {
            a = CArray_NewLikeArray(target_a, CARRAY_FORTRANORDER, descr, 0);
        }
        if (CArray_CHKFLAGS(target_a, CARRAY_ARRAY_C_CONTIGUOUS)) {
            a = CArray_NewLikeArray(target_a, CARRAY_CORDER, descr, 0);
        }
        if(CArray_CastTo(a, target_a) < 0) {
            goto fail;
        }
        casted_a = 1;
    } else {
        a = target_a;
    }

    if (CArray_DESCR(target_b)->type_num != TYPE_DOUBLE_INT) {
        CArrayDescriptor *descr = CArray_DescrFromType(TYPE_DOUBLE_INT);
        if (CArray_CHKFLAGS(target_b, CARRAY_ARRAY_F_CONTIGUOUS)) {
            b = CArray_NewLikeArray(target_b, CARRAY_FORTRANORDER, descr, 0);
        }
        if (CArray_CHKFLAGS(target_b, CARRAY_ARRAY_C_CONTIGUOUS)) {
            b = CArray_NewLikeArray(target_b, CARRAY_CORDER, descr, 0);
        }
        if(CArray_CastTo(b, target_b) < 0) {
            goto fail;
        }
        casted_b = 1;
    } else {
        b = target_b;
    }

    if (CArray_TYPE(a) == TYPE_DOUBLE_INT && CArray_TYPE(b) == TYPE_DOUBLE_INT) {
       rtn = DOUBLE_solve(a, b, out);
    }

    return rtn;
fail:
    return NULL;
}