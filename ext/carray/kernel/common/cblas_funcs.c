/*
 * This module provides a BLAS optimized matrix multiply,
 * inner product and dot for numpy arrays
 * 
 * Original File
 * Copyright (c) NumPy (/numpy/core/src/common/cblasfuncs.c)
 * 
 * Modified for CArrays in 2018
 * 
 * Henrique Borba
 * henrique.borba.dev@gmail.com
 */
#include "config.h"
#include "../carray.h"
#include "../convert.h"
#include "cblas_funcs.h"
#include "../buffer.h"
#include "../alloc.h"
#include "common.h"

#ifdef HAVE_CBLAS
#include "cblas.h"

/*
 * Helper: dispatch to appropriate cblas_?gemm for typenum.
 */
static void
gemm(int typenum, CBLAS_ORDER order,
     CBLAS_TRANSPOSE transA, CBLAS_TRANSPOSE transB,
     int m, int n, int k,
     CArray *A, int lda, CArray *B, int ldb, CArray *R)
{
    int i ;
    const void *Adata = CArray_DATA(A), *Bdata = CArray_DATA(B);
    void *Rdata = CArray_DATA(R);
    int ldc = CArray_DIM(R, 1) > 1 ? CArray_DIM(R, 1) : 1;
   
    
    switch (typenum) {
        case TYPE_DOUBLE_INT:
            cblas_dgemm(order, transA, transB, m, n, k, 1.,
                        Adata, lda, Bdata, ldb, 0., Rdata, ldc);
            break;
        case TYPE_FLOAT_INT:
            cblas_sgemm(order, transA, transB, m, n, k, 1.f,
                        Adata, lda, Bdata, ldb, 0.f, Rdata, ldc);
            break;
    }    

}

/*
 * Helper: dispatch to appropriate cblas_?syrk for typenum.
 */
static void
syrk(int typenum, CBLAS_ORDER order, CBLAS_TRANSPOSE trans,
     int n, int k,
     CArray *A, int lda, CArray *R)
{
    const void *Adata = CArray_DATA(A);
    void *Rdata = CArray_DATA(R);
    int ldc = CArray_DIM(R, 1) > 1 ? CArray_DIM(R, 1) : 1;

    int i;
    int j;

    switch (typenum) {
        case TYPE_DOUBLE_INT:
            cblas_dsyrk(order, CblasUpper, trans, n, k, 1.,
                        Adata, lda, 0., Rdata, ldc);

            for (i = 0; i < n; i++) {
                for (j = i + 1; j < n; j++) {
                    *((double*)CArray_GETPTR2(R, j, i)) =
                            *((double*)CArray_GETPTR2(R, i, j));
                }
            }
            break;
        case TYPE_FLOAT_INT:
            cblas_ssyrk(order, CblasUpper, trans, n, k, 1.f,
                        Adata, lda, 0.f, Rdata, ldc);

            for (i = 0; i < n; i++) {
                for (j = i + 1; j < n; j++) {
                    *((float*)CArray_GETPTR2(R, j, i)) =
                            *((float*)CArray_GETPTR2(R, i, j));
                }
            }
            break;
    }
}

static MatrixShape
_select_matrix_shape(CArray *array)
{
    switch (CArray_NDIM(array)) {
        case 0:
            return _scalar;
        case 1:
            if (CArray_DIM(array, 0) > 1)
                return _column;
            return _scalar;
        case 2:
            if (CArray_DIM(array, 0) > 1) {
                if (CArray_DIM(array, 1) == 1)
                    return _column;
                else
                    return _matrix;
            }
            if (CArray_DIM(array, 1) == 1)
                return _scalar;
            return _row;
    }
    return _matrix;
}

/*
 * This also makes sure that the data segment is aligned with
 * an itemsize address as well by returning one if not true.
 */
static int
_bad_strides(CArray * ap)
{
    int itemsize = CArray_ITEMSIZE(ap);
    int i, N=CArray_NDIM(ap);
    int *strides = CArray_STRIDES(ap);

    if ((*((int *)(CArray_DATA(ap))) % itemsize) != 0) {
        return 1;
    }
    for (i = 0; i < N; i++) {
        if ((strides[i] < 0) || (strides[i] % itemsize) != 0) {
            return 1;
        }
    }

    return 0;
}

/*
 * Helper: dispatch to appropriate cblas_?gemv for typenum.
 */
static void
gemv(int typenum, CBLAS_ORDER order, CBLAS_TRANSPOSE trans,
     CArray *A, int lda, CArray *X, int incX,
     CArray *R)
{
    const void *Adata = CArray_DATA(A), *Xdata = CArray_DATA(X);
    void *Rdata = CArray_DATA(R);

    int m = CArray_DIM(A, 0), n = CArray_DIM(A, 1);

    switch (typenum) {
        case TYPE_DOUBLE_INT:
            cblas_dgemv(order, trans, m, n, 1., Adata, lda, Xdata, incX,
                        0., Rdata, 1);
            break;
        case TYPE_FLOAT_INT:
            cblas_sgemv(order, trans, m, n, 1.f, Adata, lda, Xdata, incX,
                        0.f, Rdata, 1);
            break;
    }
}

/**
 * Dot product of ap1 and ap2 using CBLAS.
 */ 
CArray * 
cblas_matrixproduct(int typenum, CArray * ap1, CArray *ap2, CArray *out, MemoryPointer * ptr)
{
    CArray *result = NULL, *out_buf = NULL;
    int j, lda, ldb;
    int l;
    int nd;
    int ap1stride = 0;
    int bad1 = 0, bad2 = 0;
    int dimensions[CARRAY_MAXDIMS];
    int numbytes;
    MatrixShape ap1shape, ap2shape;

    if (_bad_strides(ap1)) {
        CArray *op1 = CArray_NewCopy(ap1, CARRAY_ANYORDER);
        memcpy(CArray_DATA(op1), CArray_DATA(ap1), CArray_SIZE(ap1) * CArray_DESCR(ap1)->elsize);
        CArray_DECREF(ap1);
        ap1 = op1;
        bad1 = 1;
        if (ap1 == NULL) {
            goto fail;
        }
    }
    if (_bad_strides(ap2)) {
        CArray *op2 = CArray_NewCopy(ap2, CARRAY_ANYORDER);
        memcpy(CArray_DATA(op2), CArray_DATA(ap2), CArray_SIZE(ap2) * CArray_DESCR(ap2)->elsize);
        CArray_DECREF(ap2);
        ap2 = op2;
        bad2 = 1;
        if (ap2 == NULL) {
            goto fail;
        }
    }

    ap1shape = _select_matrix_shape(ap1);
    ap2shape = _select_matrix_shape(ap2);
    
    if (ap1shape == _scalar || ap2shape == _scalar) {
        CArray *oap1, *oap2;
        oap1 = ap1; oap2 = ap2;
        /* One of ap1 or ap2 is a scalar */
        if (ap1shape == _scalar) {
            /* Make ap2 the scalar */
            CArray *t = ap1;
            ap1 = ap2;
            ap2 = t;
            ap1shape = ap2shape;
            ap2shape = _scalar;
        }

        if (ap1shape == _row) {
            ap1stride = CArray_STRIDE(ap1, 1);
        }
        else if (CArray_NDIM(ap1) > 0) {
            ap1stride = CArray_STRIDE(ap1, 0);
        }

        if (CArray_NDIM(ap1) == 0 || CArray_NDIM(ap2) == 0) {
            int *thisdims;
            if (CArray_NDIM(ap1) == 0) {
                nd = CArray_NDIM(ap2);
                thisdims = CArray_DIMS(ap2);
            }
            else {
                nd = CArray_NDIM(ap1);
                thisdims = CArray_DIMS(ap1);
            }
            l = 1;
            for (j = 0; j < nd; j++) {
                dimensions[j] = thisdims[j];
                l *= dimensions[j];
            }
        }
        else {
            l = CArray_DIM(oap1, CArray_NDIM(oap1) - 1);

            if (CArray_DIM(oap2, 0) != l) {
                //dot_alignment_error(oap1, PyArray_NDIM(oap1) - 1, oap2, 0);
                goto fail;
            }
            nd = CArray_NDIM(ap1) + CArray_NDIM(ap2) - 2;
            /*
             * nd = 0 or 1 or 2. If nd == 0 do nothing ...
             */
            if (nd == 1) {
                /*
                 * Either PyArray_NDIM(ap1) is 1 dim or PyArray_NDIM(ap2) is
                 * 1 dim and the other is 2 dim
                 */
                dimensions[0] = (CArray_NDIM(oap1) == 2) ?
                                CArray_DIM(oap1, 0) : CArray_DIM(oap2, 1);
                l = dimensions[0];
                /*
                 * Fix it so that dot(shape=(N,1), shape=(1,))
                 * and dot(shape=(1,), shape=(1,N)) both return
                 * an (N,) array (but use the fast scalar code)
                 */
            }
            else if (nd == 2) {
                dimensions[0] = CArray_DIM(oap1, 0);
                dimensions[1] = CArray_DIM(oap2, 1);
                /*
                 * We need to make sure that dot(shape=(1,1), shape=(1,N))
                 * and dot(shape=(N,1),shape=(1,1)) uses
                 * scalar multiplication appropriately
                 */
                if (ap1shape == _row) {
                    l = dimensions[1];
                }
                else {
                    l = dimensions[0];
                }
            }

            /* Check if the summation dimension is 0-sized */
            if (CArray_DIM(oap1, CArray_NDIM(oap1) - 1) == 0) {
                l = 0;
            }
        }
    }
    else {
        /*
         * (PyArray_NDIM(ap1) <= 2 && PyArray_NDIM(ap2) <= 2)
         * Both ap1 and ap2 are vectors or matrices
         */
        l = CArray_DIM(ap1, CArray_NDIM(ap1) - 1);

        if (CArray_DIM(ap2, 0) != l) {
            throw_notimplemented_exception();
            goto fail;
        }
        nd = CArray_NDIM(ap1) + CArray_NDIM(ap2) - 2;

        if (nd == 1) {
            dimensions[0] = (CArray_NDIM(ap1) == 2) ?
                            CArray_DIM(ap1, 0) : CArray_DIM(ap2, 1);
        }
        else if (nd == 2) {
            dimensions[0] = CArray_DIM(ap1, 0);
            dimensions[1] = CArray_DIM(ap2, 1);
        }
    }

    out_buf = new_array_for_sum(ap1, ap2, out, nd, dimensions, typenum, NULL);

    if (out_buf == NULL) {
        goto fail;
    }

    numbytes = CArray_NBYTES(out_buf);
    memset(CArray_DATA(out_buf), 0, numbytes);
    
    //if (numbytes == 0 || l == 0) {
    //    CArray_DECREF(ap1);
    //    CArray_DECREF(ap2);
    //    CArray_DECREF(out_buf);
    //    return result;
    //}

    if (ap2shape == _scalar) {
        /*
         * Multiplication by a scalar -- Level 1 BLAS
         * if ap1shape is a matrix and we are not contiguous, then we can't
         * just blast through the entire array using a single striding factor
         */

        if (typenum == TYPE_DOUBLE_INT) {
            if (l == 1) {
                *((double *)CArray_DATA(out_buf)) = *((double *)CArray_DATA(ap2)) *
                                                     *((double *)CArray_DATA(ap1));
            }
            else if (ap1shape != _matrix) {
                throw_notimplemented_exception();
                return NULL;
                /**cblas_daxpy(l,
                            *((double *)PyArray_DATA(ap2)),
                            (double *)PyArray_DATA(ap1),
                            ap1stride/sizeof(double),
                            (double *)PyArray_DATA(out_buf), 1);*/
            }
            else {
                int maxind, oind, i, a1s, outs;
                char *ptr, *optr;
                double val;

                maxind = (CArray_DIM(ap1, 0) >= CArray_DIM(ap1, 1) ? 0 : 1);
                oind = 1 - maxind;
                ptr = CArray_DATA(ap1);
                optr = CArray_DATA(out_buf);
                l = CArray_DIM(ap1, maxind);
                val = *((double *)CArray_DATA(ap2));
                a1s = CArray_STRIDE(ap1, maxind) / sizeof(double);
                outs = CArray_STRIDE(out_buf, maxind) / sizeof(double);

                
                for (i = 0; i < CArray_DIM(ap1, oind); i++) {
                    //cblas_daxpy(l, val, (double *)ptr, a1s,
                                //(double *)optr, outs);
                    ptr += CArray_STRIDE(ap1, oind);
                    optr += CArray_STRIDE(out_buf, oind);
                    throw_notimplemented_exception();
                    return NULL;
                }
            }
        }
        else if (typenum == TYPE_FLOAT_INT) {
            if (l == 1) {
                *((float *)CArray_DATA(out_buf)) = *((float *)CArray_DATA(ap2)) *
                                                    *((float *)CArray_DATA(ap1));
            }
            else if (ap1shape != _matrix) {
                /**cblas_saxpy(l,
                            *((float *)PyArray_DATA(ap2)),
                            (float *)PyArray_DATA(ap1),
                            ap1stride/sizeof(float),
                            (float *)PyArray_DATA(out_buf), 1);**/
            }
            else {
                int maxind, oind, i, a1s, outs;
                char *ptr, *optr;
                float val;

                maxind = (CArray_DIM(ap1, 0) >= CArray_DIM(ap1, 1) ? 0 : 1);
                oind = 1 - maxind;
                ptr = CArray_DATA(ap1);
                optr = CArray_DATA(out_buf);
                l = CArray_DIM(ap1, maxind);
                val = *((float *)CArray_DATA(ap2));
                a1s = CArray_STRIDE(ap1, maxind) / sizeof(float);
                outs = CArray_STRIDE(out_buf, maxind) / sizeof(float);
                for (i = 0; i < CArray_DIM(ap1, oind); i++) {
                    /**cblas_saxpy(l, val, (float *)ptr, a1s,
                                (float *)optr, outs);
                    ptr += PyArray_STRIDE(ap1, oind);
                    optr += PyArray_STRIDE(out_buf, oind);**/
                    throw_notimplemented_exception();
                    return NULL;
                }
            }
        }

    }
    else if ((ap2shape == _column) && (ap1shape != _matrix)) {
        /* Dot product between two vectors -- Level 1 BLAS */
        CArray_DESCR(out_buf)->f->dotfunc(
                CArray_DATA(ap1), CArray_STRIDE(ap1, (ap1shape == _row)),
                CArray_DATA(ap2), CArray_STRIDE(ap2, 0),
                CArray_DATA(out_buf), l);
    }
    else if (ap1shape == _matrix && ap2shape != _matrix) {
        /* Matrix vector multiplication -- Level 2 BLAS */
        /* lda must be MAX(M,1) */
        CBLAS_ORDER Order;
        int ap2s;

        if (!CArray_ISONESEGMENT(ap1)) {
            CArray *new;
            new = CArray_Copy(ap1);
            CArray_DECREF(ap1);
            ap1 = (CArray *)new;
            if (new == NULL) {
                goto fail;
            }
        }

        if (CArray_ISCONTIGUOUS(ap1)) {
            Order = CblasRowMajor;
            lda = (CArray_DIM(ap1, 1) > 1 ? CArray_DIM(ap1, 1) : 1);
        }
        else {
            Order = CblasColMajor;
            lda = (CArray_DIM(ap1, 0) > 1 ? CArray_DIM(ap1, 0) : 1);
        }

        ap2s = CArray_STRIDE(ap2, 0) / CArray_ITEMSIZE(ap2);
        gemv(typenum, Order, CblasNoTrans, ap1, lda, ap2, ap2s, out_buf);
    }
    else if (ap1shape != _matrix && ap2shape == _matrix) {
        /* Vector matrix multiplication -- Level 2 BLAS */
        CBLAS_ORDER Order;
        int ap1s;

        if (!CArray_ISONESEGMENT(ap2)) {
            CArray *new;
            new = CArray_Copy(ap2);
            CArray_DECREF(ap2);
            ap2 = (CArray *)new;
            if (new == NULL) {
                goto fail;
            }
        }

        if (CArray_ISCONTIGUOUS(ap2)) {
            Order = CblasRowMajor;
            lda = (CArray_DIM(ap2, 1) > 1 ? CArray_DIM(ap2, 1) : 1);
        }
        else {
            Order = CblasColMajor;
            lda = (CArray_DIM(ap2, 0) > 1 ? CArray_DIM(ap2, 0) : 1);
        }
        if (ap1shape == _row) {
            ap1s = CArray_STRIDE(ap1, 1) / CArray_ITEMSIZE(ap1);
        }
        else {
            ap1s = CArray_STRIDE(ap1, 0) / CArray_ITEMSIZE(ap1);
        }
        gemv(typenum, Order, CblasTrans, ap2, lda, ap1, ap1s, out_buf);
        CArray_INCREF(out_buf);
    }
    else {
        /*
         * (PyArray_NDIM(ap1) == 2 && PyArray_NDIM(ap2) == 2)
         * Matrix matrix multiplication -- Level 3 BLAS
         *  L x M  multiplied by M x N
         */
        CBLAS_ORDER Order;
        CBLAS_TRANSPOSE Trans1, Trans2;
        int M, N, L;

        /* Optimization possible: */
        /*
         * We may be able to handle single-segment arrays here
         * using appropriate values of Order, Trans1, and Trans2.
         */
        if (!CArray_IS_C_CONTIGUOUS(ap2) && !CArray_IS_F_CONTIGUOUS(ap2)) {
            CArray *new = CArray_Copy(ap2);

            CArray_DECREF(ap2);
            ap2 = (CArray *)new;
            if (new == NULL) {
                goto fail;
            }
        }
        if (!CArray_IS_C_CONTIGUOUS(ap1) && !CArray_IS_F_CONTIGUOUS(ap1)) {
            CArray *new = CArray_Copy(ap1);

            CArray_DECREF(ap1);
            ap1 = (CArray *)new;
            if (new == NULL) {
                goto fail;
            }
        }

        Order = CblasRowMajor;
        Trans1 = CblasNoTrans;
        Trans2 = CblasNoTrans;
        L = CArray_DIM(ap1, 0);
        N = CArray_DIM(ap2, 1);
        M = CArray_DIM(ap2, 0);
        lda = (CArray_DIM(ap1, 1) > 1 ? CArray_DIM(ap1, 1) : 1);
        ldb = (CArray_DIM(ap2, 1) > 1 ? CArray_DIM(ap2, 1) : 1);

        /*
         * Avoid temporary copies for arrays in Fortran order
         */
        if (CArray_IS_F_CONTIGUOUS(ap1)) {
            Trans1 = CblasTrans;
            lda = (CArray_DIM(ap1, 0) > 1 ? CArray_DIM(ap1, 0) : 1);
        }
        if (CArray_IS_F_CONTIGUOUS(ap2)) {
            Trans2 = CblasTrans;
            ldb = (CArray_DIM(ap2, 0) > 1 ? CArray_DIM(ap2, 0) : 1);
        }

        /*
         * Use syrk if we have a case of a matrix times its transpose.
         * Otherwise, use gemm for all other cases.
         */
        if (
                (CArray_BYTES(ap1) == CArray_BYTES(ap2)) &&
                (CArray_DIM(ap1, 0) == CArray_DIM(ap2, 1)) &&
                (CArray_DIM(ap1, 1) == CArray_DIM(ap2, 0)) &&
                (CArray_STRIDE(ap1, 0) == CArray_STRIDE(ap2, 1)) &&
                (CArray_STRIDE(ap1, 1) == CArray_STRIDE(ap2, 0)) &&
                ((Trans1 == CblasTrans) ^ (Trans2 == CblasTrans)) &&
                ((Trans1 == CblasNoTrans) ^ (Trans2 == CblasNoTrans))
                ) {

            if (Trans1 == CblasNoTrans) {
                syrk(typenum, Order, Trans1, N, M, ap1, lda, out_buf);
            }
            else {
                syrk(typenum, Order, Trans1, N, M, ap2, ldb, out_buf);
            }
        }
        else {
            gemm(typenum, Order, Trans1, Trans2, L, N, M, ap1, lda, ap2, ldb,
                 out_buf);
        }
    }

    /* Trigger possible copyback into `result` */
    CArray_ResolveWritebackIfCopy(out_buf);

    if (ptr != NULL) {
        add_to_buffer(ptr, out_buf, sizeof(CArray));
    }

    if (bad1) {
        CArray_Free(ap1);
    }
    if (bad2) {
        CArray_Free(ap2);
    }

    return out_buf;
fail:
    //CArray_DECREF(ap1);
    //CArray_DECREF(ap2);
    //CArray_DECREF(out_buf);
    //CArray_DECREF(result);
    return NULL;
}
#endif