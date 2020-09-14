#include "config.h"
#include "../carray.h"
#include "../convert.h"
#include "../buffer.h"
#include "../alloc.h"
#include "common.h"
#include "exceptions.h"
#include "cblas.h"


#ifdef HAVE_CLBLAS

#include "clblas_funcs.h"
#include "clBLAS.h"
#include "../gpu.h"
#include "cblas_funcs.h"

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

static void
cldaxpy(int n_elements, int alpha, double *a, int incX, double *b, int incY) {
    cl_double alphad = alpha;
    size_t offsetX = 0, offsetY = 0;

    cl_int err;
    cl_platform_id platform = 0;
    cl_device_id device = 0;
    cl_context_properties props[3] = { CL_CONTEXT_PLATFORM, 0, 0 };
    cl_context ctx = 0;
    cl_command_queue queue = 0;
    cl_mem bufA, bufB;
    cl_event event = NULL;

    ctx = getCLContext();
    queue = getCLQueue();

    /* Setup OpenCL environment. */
    err = clGetPlatformIDs( 1, &platform, NULL );
    err = clGetDeviceIDs( platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL );

    props[1] = (cl_context_properties)platform;
    ctx = clCreateContext( props, 1, &device, NULL, NULL, &err );
    queue = clCreateCommandQueue( ctx, device, 0, &err );

    /* Setup clBLAS */
    err = clblasSetup( );

    /* Prepare OpenCL memory objects and place matrices inside them. */
    bufA = clCreateBuffer( ctx, CL_MEM_READ_ONLY, n_elements * sizeof(double),
                          NULL, &err );

    php_printf("%d", err);

    bufB = clCreateBuffer( ctx, CL_MEM_READ_ONLY, sizeof(double),
                          NULL, &err );



    err = clEnqueueWriteBuffer( queue, bufA, CL_TRUE, 0,
        n_elements * sizeof(double), a, 0, NULL, NULL );
    err = clEnqueueWriteBuffer( queue, bufB, CL_TRUE, 0,
        sizeof(double), b, 0, NULL, NULL );

    err = clblasDaxpy((size_t)n_elements, alphad, bufA, 0, incX, bufB, 0, incY, 1, &queue, 0, NULL, &event);

    return;
}

/*
 * Helper: dispatch to appropriate cblas_?gemm for typenum.
 */
static void
clgemm(int typenum, clblasOrder order,
     clblasTranspose transA, clblasTranspose transB,
     int m, int n, int k,
     CArray *A, int lda, CArray *B, int ldb, CArray *R)
{
    int i ;
    const void *Adata = CArray_DATA(A), *Bdata = CArray_DATA(B);
    void *Rdata = CArray_DATA(R);
    int ldc = CArray_DIM(R, 1) > 1 ? CArray_DIM(R, 1) : 1;

    cl_int err;
    cl_context ctx = 0;
    cl_command_queue queue = 0;
    cl_mem bufA, bufB, bufC;
    cl_event event = NULL;
    int ret = 0;

    ctx = getCLContext();
    queue = getCLQueue();

    /* Prepare OpenCL memory objects and place matrices inside them. */
    bufA = clCreateBuffer( ctx, CL_MEM_READ_ONLY, m * k * CArray_DESCR(A)->elsize,
                          NULL, &err );
    bufB = clCreateBuffer( ctx, CL_MEM_READ_ONLY, k * n * CArray_DESCR(B)->elsize,
                          NULL, &err );
    bufC = clCreateBuffer( ctx, CL_MEM_READ_WRITE, m * n * CArray_DESCR(R)->elsize,
                          NULL, &err );

    err = clEnqueueWriteBuffer( queue, bufA, CL_TRUE, 0,
        m * k * CArray_DESCR(A)->elsize, Adata, 0, NULL, NULL );
    err = clEnqueueWriteBuffer( queue, bufB, CL_TRUE, 0,
        k * n * CArray_DESCR(B)->elsize, Bdata, 0, NULL, NULL );
    err = clEnqueueWriteBuffer( queue, bufC, CL_TRUE, 0,
        m * n * CArray_DESCR(R)->elsize, Rdata, 0, NULL, NULL );


    cl_float alpha = 1;
    cl_float beta  = 0;


    switch (typenum) {
        case TYPE_DOUBLE_INT:
            /* Call clBLAS extended function. Perform gemm for the lower right sub-matrices */
            err = clblasDgemm( order, transA, transB,
                                m, n, k,
                                alpha, bufA, 0, lda,
                                bufB, 0, ldb, beta,
                                bufC, 0, ldc,
                                1, &queue, 0, NULL, &event );
            break;
        case TYPE_FLOAT_INT:
            break;
    }

    /* Wait for calculations to be finished. */
    err = clWaitForEvents( 1, &event );

    /* Fetch results of calculations from GPU memory. */
    err = clEnqueueReadBuffer( queue, bufC, CL_TRUE, 0,
                                m * n * CArray_DESCR(R)->elsize,
                                Rdata, 0, NULL, NULL );


    /* Release OpenCL memory objects. */
    clReleaseMemObject( bufC );
    clReleaseMemObject( bufB );
    clReleaseMemObject( bufA );

    /* Finalize work with clBLAS */
    clblasTeardown( );

    /* Release OpenCL working objects. */
    clReleaseCommandQueue( queue );
    clReleaseContext( ctx );

}

/**
 * @param typenum
 * @param ap1
 * @param ap2
 * @param out
 * @param ptr
 * @return
 */
CArray *
clblas_matrixproduct(int typenum, CArray * ap1, CArray *ap2, CArray *out, MemoryPointer * ptr)
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
         * (CArray_NDIM(ap1) <= 2 && CArray_NDIM(ap2) <= 2)
         * Both ap1 and ap2 are vectors or matrices
         */
        l = CArray_DIM(ap1, CArray_NDIM(ap1) - 1);

        if (CArray_DIM(ap2, 0) != l) {
            //dot_alignment_error(ap1, PyArray_NDIM(ap1) - 1, ap2, 0);
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
    if (numbytes == 0 || l == 0) {
        CArray_DECREF(ap1);
        CArray_DECREF(ap2);
        CArray_DECREF(out_buf);
        return result;
    }

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
                throw_not_implemented_exception();
                return NULL;
                //cldaxpy(ap1, ap2, out_buf);
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
                    cldaxpy(l, val, (double *)ptr, a1s, (double *)optr, outs);
                    ptr += CArray_STRIDE(ap1, oind);
                    optr += CArray_STRIDE(out_buf, oind);
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
        clblasOrder Order;
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
            Order = clblasRowMajor;
            lda = (CArray_DIM(ap1, 1) > 1 ? CArray_DIM(ap1, 1) : 1);
        }
        else {
            Order = clblasColumnMajor;
            lda = (CArray_DIM(ap1, 0) > 1 ? CArray_DIM(ap1, 0) : 1);
        }

        ap2s = CArray_STRIDE(ap2, 0) / CArray_ITEMSIZE(ap2);
        gemv(typenum, Order, clblasNoTrans, ap1, lda, ap2, ap2s, out_buf);
    }
    else if (ap1shape != _matrix && ap2shape == _matrix) {
        /* Vector matrix multiplication -- Level 2 BLAS */
        clblasOrder Order;
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
            Order = clblasRowMajor;
            lda = (CArray_DIM(ap2, 1) > 1 ? CArray_DIM(ap2, 1) : 1);
        }
        else {
            Order = clblasColumnMajor;
            lda = (CArray_DIM(ap2, 0) > 1 ? CArray_DIM(ap2, 0) : 1);
        }
        if (ap1shape == _row) {
            ap1s = CArray_STRIDE(ap1, 1) / CArray_ITEMSIZE(ap1);
        }
        else {
            ap1s = CArray_STRIDE(ap1, 0) / CArray_ITEMSIZE(ap1);
        }

        gemv(typenum, Order, clblasTrans, ap2, lda, ap1, ap1s, out_buf);
    }
    else {
        /*
         * (PyArray_NDIM(ap1) == 2 && PyArray_NDIM(ap2) == 2)
         * Matrix matrix multiplication -- Level 3 BLAS
         *  L x M  multiplied by M x N
         */
        clblasOrder Order;
        clblasTranspose Trans1, Trans2;
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

        Order = clblasRowMajor;
        Trans1 = clblasNoTrans;
        Trans2 = clblasNoTrans;
        L = CArray_DIM(ap1, 0);
        N = CArray_DIM(ap2, 1);
        M = CArray_DIM(ap2, 0);
        lda = (CArray_DIM(ap1, 1) > 1 ? CArray_DIM(ap1, 1) : 1);
        ldb = (CArray_DIM(ap2, 1) > 1 ? CArray_DIM(ap2, 1) : 1);

        /*
         * Avoid temporary copies for arrays in Fortran order
         */
        if (CArray_IS_F_CONTIGUOUS(ap1)) {
            Trans1 = clblasTrans;
            lda = (CArray_DIM(ap1, 0) > 1 ? CArray_DIM(ap1, 0) : 1);
        }
        if (CArray_IS_F_CONTIGUOUS(ap2)) {
            Trans2 = clblasTrans;
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
                ((Trans1 == clblasTrans) ^ (Trans2 == clblasTrans)) &&
                ((Trans1 == clblasNoTrans) ^ (Trans2 == clblasNoTrans))
                ) {

            if (Trans1 == clblasNoTrans) {
                //syrk(typenum, Order, Trans1, N, M, ap1, lda, out_buf);
            }
            else {
                //syrk(typenum, Order, Trans1, N, M, ap2, ldb, out_buf);
            }
        }
        else {
            clgemm(typenum, Order, Trans1, Trans2, L, N, M, ap1, lda, ap2, ldb,
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
