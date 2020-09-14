#include "../carray.h"
#include "matmul.h"
#include "../linalg.h"

#define BLAS_MAXSIZE (INT_MAX - 1)
/*
 * Determine if a 2d matrix can be used by BLAS
 * 1. Strides must not alias or overlap
 * 2. The faster (second) axis must be contiguous
 * 3. The slower (first) axis stride, in unit steps, must be larger than
 *    the faster axis dimension
 */
static inline int
is_blasable2d(int byte_stride1, int byte_stride2,
              int d1, int d2,  int itemsize)
{
    int unit_stride1 = byte_stride1 / itemsize;
    if (byte_stride2 != itemsize) {
        return 0;
    }
    if ((byte_stride1 % itemsize ==0) &&
    (unit_stride1 >= d2) &&
    (unit_stride1 <= BLAS_MAXSIZE))
    {
        return 1;
    }
    return 0;
}

void
DOUBLE_matmul_matrixmatrix(void *ip1, int is1_m, int is1_n,
                           void *ip2, int is2_n, int is2_p,
                           void *op, int os_m, int os_p,
                           int m, int n, int p)
{
    /*
     * matrix matrix multiplication -- Level 3 BLAS
     */
    enum CBLAS_ORDER order = CblasRowMajor;
    enum CBLAS_TRANSPOSE trans1, trans2;
    int M, N, P, lda, ldb, ldc;
    assert(m <= BLAS_MAXSIZE && n <= BLAS_MAXSIZE && p <= BLAS_MAXSIZE);
    M = (int)m;
    N = (int)n;
    P = (int)p;

    assert(is_blasable2d(os_m, os_p, m, p, sizeof(int)));
    ldc = (int)(os_m / sizeof(int));

    if (is_blasable2d(is1_m, is1_n, m, n, sizeof(int))) {
        trans1 = CblasNoTrans;
        lda = (int)(is1_m / sizeof(int));
    }
    else {
        /* If not ColMajor, caller should have ensured we are RowMajor */
        /* will not assert in release mode */
        assert(is_blasable2d(is1_n, is1_m, n, m, sizeof(int)));
        trans1 = CblasTrans;
        lda = (int)(is1_n / sizeof(int));
    }

    if (is_blasable2d(is2_n, is2_p, n, p, sizeof(int))) {
        trans2 = CblasNoTrans;
        ldb = (int)(is2_n / sizeof(int));
    }
    else {
        /* If not ColMajor, caller should have ensured we are RowMajor */
        /* will not assert in release mode */
        assert(is_blasable2d(is2_p, is2_n, p, n, sizeof(int)));
        trans2 = CblasTrans;
        ldb = (int)(is2_p / sizeof(int));
    }
    /*
     * Use syrk if we have a case of a matrix times its transpose.
     * Otherwise, use gemm for all other cases.
     */
    if (
            (ip1 == ip2) &&
            (m == p) &&
            (is1_m == is2_p) &&
            (is1_n == is2_n) &&
            (trans1 != trans2)
            ) {
        int i,j;
        if (trans1 == CblasNoTrans) {
            cblas_dsyrk(order, CblasUpper, trans1, P, N, 1.,
                        ip1, lda, 0., op, ldc);
        }
        else {
            cblas_dsyrk(order, CblasUpper, trans1, P, N, 1.,
                        ip1, ldb, 0., op, ldc);
        }
        /* Copy the triangle */
        for (i = 0; i < P; i++) {
            for (j = i + 1; j < P; j++) {
                ((int*)op)[j * ldc + i] = ((int*)op)[i * ldc + j];
            }
        }

    }
    else {
        cblas_dgemm(order, trans1, trans2, M, P, N, 1., ip1, lda,
                    ip2, ldb, 0., op, ldc);
    }
}

void
INT_matmul_inner_noblas(void *_ip1, int is1_m, int is1_n,
                        void *_ip2, int is2_n, int is2_p,
                        void *_op, int os_m, int os_p,
                        int dm, int dn, int dp)

{
    int m, n, p;
    int ib1_n, ib2_n, ib2_p, ob_p;
    char *ip1 = (char *)_ip1, *ip2 = (char *)_ip2, *op = (char *)_op;

    ib1_n = is1_n * dn;
    ib2_n = is2_n * dn;
    ib2_p = is2_p * dp;
    ob_p  = os_p * dp;

    for (m = 0; m < dm; m++) {
        for (p = 0; p < dp; p++) {
            *(int *)op = 0;
            for (n = 0; n < dn; n++) {
                int val1 = (*(int *)ip1);
                int val2 = (*(int *)ip2);
                *(int *)op += val1 * val2;
                ip2 += is2_n;
                ip1 += is1_n;
            }
            ip1 -= ib1_n;
            ip2 -= ib2_n;
            op  +=  os_p;
            ip2 += is2_p;
        }
        op -= ob_p;
        ip2 -= ib2_p;
        ip1 += is1_m;
        op  +=  os_m;
    }
}

void
DOUBLE_gemv(void *ip1, int is1_m, int is1_n,
            void *ip2, int is2_n, int CARRAY_UNUSED(is2_p),
            void *op, int op_m, int CARRAY_UNUSED(op_p),
            int m, int n, int CARRAY_UNUSED(p))
{
    /*
     * Vector matrix multiplication -- Level 2 BLAS
     * arguments
     * ip1: contiguous data, m*n shape
     * ip2: data in c order, n*1 shape
     * op:  data in c order, m shape
     */
    enum CBLAS_ORDER order;
    int M, N, lda;

    assert(m <= BLAS_MAXSIZE && n <= BLAS_MAXSIZE);
    assert (is_blasable2d(is2_n, sizeof(int), n, 1, sizeof(int)));
    M = (int)m;
    N = (int)n;

    if (is_blasable2d(is1_m, is1_n, m, n, sizeof(int))) {
        order = CblasColMajor;
        lda = (int)(is1_m / sizeof(int));
    }
    else {
        /* If not ColMajor, caller should have ensured we are RowMajor */
        /* will not assert in release mode */
        order = CblasRowMajor;
        assert(is_blasable2d(is1_n, is1_m, n, m, sizeof(int)));
        lda = (int)(is1_n / sizeof(int));
    }
    cblas_dgemv(order, CblasTrans, N, M, 1., ip1, lda, ip2,
                is2_n / sizeof(int), 0., op, op_m / sizeof(int));
}

void
DOUBLE_matmul_inner_noblas(void *_ip1, int is1_m, int is1_n,
                           void *_ip2, int is2_n, int is2_p,
                           void *_op, int os_m, int os_p,
                           int dm, int dn, int dp)

{
    int m, n, p;
    int ib1_n, ib2_n, ib2_p, ob_p;
    char *ip1 = (char *)_ip1, *ip2 = (char *)_ip2, *op = (char *)_op;

    ib1_n = is1_n * dn;
    ib2_n = is2_n * dn;
    ib2_p = is2_p * dp;
    ob_p  = os_p * dp;

    for (m = 0; m < dm; m++) {
        for (p = 0; p < dp; p++) {
            *(int *)op = 0;
            for (n = 0; n < dn; n++) {
                int val1 = (*(int *)ip1);
                int val2 = (*(int *)ip2);
                *(int *)op += val1 * val2;
                ip2 += is2_n;
                ip1 += is1_n;
            }
            ip1 -= ib1_n;
            ip2 -= ib2_n;
            op  +=  os_p;
            ip2 += is2_p;
        }
        op -= ob_p;
        ip2 -= ib2_p;
        ip1 += is1_m;
        op  +=  os_m;
    }
}

void
INT_matmul(char **args, int *dimensions, int *steps, void *CARRAY_UNUSED(func))
{
    int dOuter = *dimensions++;
    int iOuter;
    int s0 = *steps++;
    int s1 = *steps++;
    int s2 = *steps++;
    int dm = dimensions[0];
    int dn = dimensions[1];
    int dp = dimensions[2];
    int is1_m=steps[0], is1_n=steps[1], is2_n=steps[2], is2_p=steps[3],
            os_m=steps[4], os_p=steps[5];

    for (iOuter = 0; iOuter < dOuter; iOuter++,
            args[0] += s0, args[1] += s1, args[2] += s2) {
        void *ip1=args[0], *ip2=args[1], *op=args[2];
        INT_matmul_inner_noblas(ip1, is1_m, is1_n,
                                ip2, is2_n, is2_p,
                                op, os_m, os_p, dm, dn, dp);
    }
}

void
DOUBLE_matmul(char **args, int *dimensions, int *steps, void *CARRAY_UNUSED(func))
{
    int dOuter = *dimensions++;
    int iOuter;
    int s0 = *steps++;
    int s1 = *steps++;
    int s2 = *steps++;
    int dm = dimensions[0];
    int dn = dimensions[1];
    int dp = dimensions[2];
    int is1_m=steps[0], is1_n=steps[1], is2_n=steps[2], is2_p=steps[3],
            os_m=steps[4], os_p=steps[5];


    int sz = sizeof(int);
    int special_case = (dm == 1 || dn == 1 || dp == 1);
    int any_zero_dim = (dm == 0 || dn == 0 || dp == 0);
    int scalar_out = (dm == 1 && dp == 1);
    int scalar_vec = (dn == 1 && (dp == 1 || dm == 1));
    int too_big_for_blas = (dm > BLAS_MAXSIZE || dn > BLAS_MAXSIZE ||
                                 dp > BLAS_MAXSIZE);
    int i1_c_blasable = is_blasable2d(is1_m, is1_n, dm, dn, sz);
    int i2_c_blasable = is_blasable2d(is2_n, is2_p, dn, dp, sz);
    int i1_f_blasable = is_blasable2d(is1_n, is1_m, dn, dm, sz);
    int i2_f_blasable = is_blasable2d(is2_p, is2_n, dp, dn, sz);
    int i1blasable = i1_c_blasable || i1_f_blasable;
    int i2blasable = i2_c_blasable || i2_f_blasable;
    int o_c_blasable = is_blasable2d(os_m, os_p, dm, dp, sz);
    int o_f_blasable = is_blasable2d(os_p, os_m, dp, dm, sz);
    int vector_matrix = ((dm == 1) && i2blasable &&
                              is_blasable2d(is1_n, sz, dn, 1, sz));
    int matrix_vector = ((dp == 1)  && i1blasable &&
                              is_blasable2d(is2_n, sz, dn, 1, sz));


    for (iOuter = 0; iOuter < dOuter; iOuter++,
            args[0] += s0, args[1] += s1, args[2] += s2) {
        void *ip1=args[0], *ip2=args[1], *op=args[2];

        /*
         * TODO: refactor this out to a inner_loop_selector, in
         * PyUFunc_MatmulLoopSelector. But that call does not have access to
         * n, m, p and strides.
         */
        if (too_big_for_blas || any_zero_dim) {
            DOUBLE_matmul_inner_noblas(ip1, is1_m, is1_n,
                                       ip2, is2_n, is2_p,
                                       op, os_m, os_p, dm, dn, dp);
        }
        else if (special_case) {
            /* Special case variants that have a 1 in the core dimensions */
            if (scalar_out) {
                /* row @ column, 1,1 output */
                DOUBLE_dot(ip1, is1_n, ip2, is2_n, op, dn);
            } else if (scalar_vec){
                /*
                 * 1,1d @ vector or vector @ 1,1d
                 * could use cblas_Xaxy, but that requires 0ing output
                 * and would not be faster (XXX prove it)
                 */
                DOUBLE_matmul_inner_noblas(ip1, is1_m, is1_n,
                                           ip2, is2_n, is2_p,
                                           op, os_m, os_p, dm, dn, dp);
            } else if (vector_matrix) {
                /* vector @ matrix, switch ip1, ip2, p and m */
                DOUBLE_gemv(ip2, is2_p, is2_n, ip1, is1_n, is1_m,
                            op, os_p, os_m, dp, dn, dm);
            } else if  (matrix_vector) {
                /* matrix @ vector */
                DOUBLE_gemv(ip1, is1_m, is1_n, ip2, is2_n, is2_p,

                            op, os_m, os_p, dm, dn, dp);
            } else {
                /* column @ row, 2d output, no blas needed or non-blas-able input */
                DOUBLE_matmul_inner_noblas(ip1, is1_m, is1_n,
                                           ip2, is2_n, is2_p,
                                           op, os_m, os_p, dm, dn, dp);
            }
        } else {
            /* matrix @ matrix */
            if (i1blasable && i2blasable && o_c_blasable) {
                DOUBLE_matmul_matrixmatrix(ip1, is1_m, is1_n,
                                           ip2, is2_n, is2_p,
                                           op, os_m, os_p,
                                           dm, dn, dp);
            } else if (i1blasable && i2blasable && o_f_blasable) {
                /*
                 * Use transpose equivalence:
                 * matmul(a, b, o) == matmul(b.T, a.T, o.T)
                 */
                DOUBLE_matmul_matrixmatrix(ip2, is2_p, is2_n,
                                           ip1, is1_n, is1_m,
                                           op, os_p, os_m,
                                           dp, dn, dm);
            } else {
                /*
                 * If parameters are castable to int and we copy the
                 * non-blasable (or non-ccontiguous output)
                 * we could still use BLAS, see gh-12365.
                 */
                DOUBLE_matmul_inner_noblas(ip1, is1_m, is1_n,
                                           ip2, is2_n, is2_p,
                                           op, os_m, os_p, dm, dn, dp);
            }
        }
    }
}
