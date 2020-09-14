#include "carray.h"
#include "arraytypes.h"


/*
 *****************************************************************************
 **                       small correlate                                   **
 *****************************************************************************
 */

/*
 * Compute correlation of data with with small kernels
 * Calling a BLAS dot product for the inner loop of the correlation is overkill
 * for small kernels. It is faster to compute it directly.
 * Intended to be used by _carray_correlate so no input verifications is done
 * especially it does not handle the boundaries, they should be handled by the
 * caller.
 * Returns 0 if kernel is considered too large or types are not supported, then
 * the regular array dot should be used to process the data.
 *
 * d_, dstride, nd, dtype: data pointer, its stride in bytes, number of
 *                         elements and type of data
 * k_, kstride, nk, ktype: kernel pointer, its stride in bytes, number of
 *                         elements and type of data
 * out_, ostride: output data pointer and its stride in bytes
 */
int
small_correlate(const char * d_, int dstride,
                int nd, int dtype,
                const char * k_, int kstride,
                int nk, int ktype,
                char * out_, int ostride)
{
    /* only handle small kernels and uniform types */
    if (nk > 11 || dtype != ktype) {
        return 0;
    }

    switch (dtype) {
        case TYPE_FLOAT_INT:
        {
            int i;
            const float * d = (float*)d_;
            const float * k = (float*)k_;
            float * out = (float*)out_;
            dstride /= sizeof(float);
            kstride /= sizeof(float);
            ostride /= sizeof(float);
            /* unroll inner loop to optimize register usage of the kernel*/
            switch (nk) {
                case 1:
                {
                    /* load kernel */
                    const float k1 = k[(1 - 1) * kstride];
                    for (i = 0; i < nd; i++) {
                        float s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        out[i * ostride] = s;
                    }
                    return 1;
                }
                case 2:
                {
                    /* load kernel */
                    const float k1 = k[(1 - 1) * kstride];
                    /* load kernel */
                    const float k2 = k[(2 - 1) * kstride];

                    for (i = 0; i < nd; i++) {
                        float s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        s += d[(i + 2 - 1) * dstride] * k2;
                        out[i * ostride] = s;
                    }
                    return 1;
                }
                case 3:
                {
                    const float k1 = k[(1 - 1) * kstride];
                    const float k2 = k[(2 - 1) * kstride];
                    const float k3 = k[(3 - 1) * kstride];
                    for (i = 0; i < nd; i++) {
                        float s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        s += d[(i + 2 - 1) * dstride] * k2;
                        s += d[(i + 3 - 1) * dstride] * k3;
                        out[i * ostride] = s;
                    }
                    return 1;
                }
                case 4:
                {
                    const float k1 = k[(1 - 1) * kstride];
                    const float k2 = k[(2 - 1) * kstride];
                    const float k3 = k[(3 - 1) * kstride];
                    const float k4 = k[(4 - 1) * kstride];

                    for (i = 0; i < nd; i++) {
                        float s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        s += d[(i + 2 - 1) * dstride] * k2;
                        s += d[(i + 3 - 1) * dstride] * k3;
                        s += d[(i + 4 - 1) * dstride] * k4;
                        out[i * ostride] = s;
                    }
                    return 1;
                }
                case 5:
                {
                    const float k1 = k[(1 - 1) * kstride];
                    const float k2 = k[(2 - 1) * kstride];
                    const float k3 = k[(3 - 1) * kstride];
                    const float k4 = k[(4 - 1) * kstride];
                    const float k5 = k[(5 - 1) * kstride];

                    for (i = 0; i < nd; i++) {
                        float s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        s += d[(i + 2 - 1) * dstride] * k2;
                        s += d[(i + 3 - 1) * dstride] * k3;
                        s += d[(i + 4 - 1) * dstride] * k4;
                        s += d[(i + 5 - 1) * dstride] * k5;

                        out[i * ostride] = s;
                    }
                    return 1;
                }
                case 6:
                {
                    const float k1 = k[(1 - 1) * kstride];
                    const float k2 = k[(2 - 1) * kstride];
                    const float k3 = k[(3 - 1) * kstride];
                    const float k4 = k[(4 - 1) * kstride];
                    const float k5 = k[(5 - 1) * kstride];
                    const float k6 = k[(6 - 1) * kstride];

                    for (i = 0; i < nd; i++) {
                        float s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        s += d[(i + 2 - 1) * dstride] * k2;
                        s += d[(i + 3 - 1) * dstride] * k3;
                        s += d[(i + 4 - 1) * dstride] * k4;
                        s += d[(i + 5 - 1) * dstride] * k5;
                        s += d[(i + 6 - 1) * dstride] * k6;
                        out[i * ostride] = s;
                    }
                    return 1;
                }
                case 7:
                {
                    const float k1 = k[(1 - 1) * kstride];
                    const float k2 = k[(2 - 1) * kstride];
                    const float k3 = k[(3 - 1) * kstride];
                    const float k4 = k[(4 - 1) * kstride];
                    const float k5 = k[(5 - 1) * kstride];
                    const float k6 = k[(6 - 1) * kstride];
                    const float k7 = k[(7 - 1) * kstride];

                    for (i = 0; i < nd; i++) {
                        float s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        s += d[(i + 2 - 1) * dstride] * k2;
                        s += d[(i + 3 - 1) * dstride] * k3;
                        s += d[(i + 4 - 1) * dstride] * k4;
                        s += d[(i + 5 - 1) * dstride] * k5;
                        s += d[(i + 6 - 1) * dstride] * k6;
                        s += d[(i + 7 - 1) * dstride] * k7;
                        out[i * ostride] = s;
                    }
                    return 1;
                }
                case 8:
                {
                    const float k1 = k[(1 - 1) * kstride];
                    const float k2 = k[(2 - 1) * kstride];
                    const float k3 = k[(3 - 1) * kstride];
                    const float k4 = k[(4 - 1) * kstride];
                    const float k5 = k[(5 - 1) * kstride];
                    const float k6 = k[(6 - 1) * kstride];
                    const float k7 = k[(7 - 1) * kstride];
                    const float k8 = k[(8 - 1) * kstride];
                    for (i = 0; i < nd; i++) {
                        float s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        s += d[(i + 2 - 1) * dstride] * k2;
                        s += d[(i + 3 - 1) * dstride] * k3;
                        s += d[(i + 4 - 1) * dstride] * k4;
                        s += d[(i + 5 - 1) * dstride] * k5;
                        s += d[(i + 6 - 1) * dstride] * k6;
                        s += d[(i + 7 - 1) * dstride] * k7;
                        s += d[(i + 8 - 1) * dstride] * k8;
                        out[i * ostride] = s;
                    }
                    return 1;
                }
                case 9:
                {
                    const float k1 = k[(1 - 1) * kstride];
                    const float k2 = k[(2 - 1) * kstride];
                    const float k3 = k[(3 - 1) * kstride];
                    const float k4 = k[(4 - 1) * kstride];
                    const float k5 = k[(5 - 1) * kstride];
                    const float k6 = k[(6 - 1) * kstride];
                    const float k7 = k[(7 - 1) * kstride];
                    const float k8 = k[(8 - 1) * kstride];
                    const float k9 = k[(9 - 1) * kstride];
                    for (i = 0; i < nd; i++) {
                        float s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        s += d[(i + 2 - 1) * dstride] * k2;
                        s += d[(i + 3 - 1) * dstride] * k3;
                        s += d[(i + 4 - 1) * dstride] * k4;
                        s += d[(i + 5 - 1) * dstride] * k5;
                        s += d[(i + 6 - 1) * dstride] * k6;
                        s += d[(i + 7 - 1) * dstride] * k7;
                        s += d[(i + 8 - 1) * dstride] * k8;
                        s += d[(i + 9 - 1) * dstride] * k9;

                        out[i * ostride] = s;
                    }
                    return 1;
                }
                case 10:
                {
                    const float k1 = k[(1 - 1) * kstride];
                    const float k2 = k[(2 - 1) * kstride];
                    const float k3 = k[(3 - 1) * kstride];
                    const float k4 = k[(4 - 1) * kstride];
                    const float k5 = k[(5 - 1) * kstride];
                    const float k6 = k[(6 - 1) * kstride];
                    const float k7 = k[(7 - 1) * kstride];
                    const float k8 = k[(8 - 1) * kstride];
                    const float k9 = k[(9 - 1) * kstride];
                    const float k10 = k[(10 - 1) * kstride];
                    for (i = 0; i < nd; i++) {
                        float s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        s += d[(i + 2 - 1) * dstride] * k2;
                        s += d[(i + 3 - 1) * dstride] * k3;
                        s += d[(i + 4 - 1) * dstride] * k4;
                        s += d[(i + 5 - 1) * dstride] * k5;
                        s += d[(i + 6 - 1) * dstride] * k6;
                        s += d[(i + 7 - 1) * dstride] * k7;
                        s += d[(i + 8 - 1) * dstride] * k8;
                        s += d[(i + 9 - 1) * dstride] * k9;
                        s += d[(i + 10 - 1) * dstride] * k10;
                        out[i * ostride] = s;
                    }
                    return 1;
                }
                case 11:
                {
                    const float k1 = k[(1 - 1) * kstride];
                    const float k2 = k[(2 - 1) * kstride];
                    const float k3 = k[(3 - 1) * kstride];
                    const float k4 = k[(4 - 1) * kstride];
                    const float k5 = k[(5 - 1) * kstride];
                    const float k6 = k[(6 - 1) * kstride];
                    const float k7 = k[(7 - 1) * kstride];
                    const float k8 = k[(8 - 1) * kstride];
                    const float k9 = k[(9 - 1) * kstride];
                    const float k10 = k[(10 - 1) * kstride];
                    const float k11 = k[(11 - 1) * kstride];
                    for (i = 0; i < nd; i++) {
                        float s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        s += d[(i + 2 - 1) * dstride] * k2;
                        s += d[(i + 3 - 1) * dstride] * k3;
                        s += d[(i + 4 - 1) * dstride] * k4;
                        s += d[(i + 5 - 1) * dstride] * k5;
                        s += d[(i + 6 - 1) * dstride] * k6;
                        s += d[(i + 7 - 1) * dstride] * k7;
                        s += d[(i + 8 - 1) * dstride] * k8;
                        s += d[(i + 9 - 1) * dstride] * k9;
                        s += d[(i + 10 - 1) * dstride] * k10;
                        s += d[(i + 11 - 1) * dstride] * k11;
                        out[i * ostride] = s;
                    }
                    return 1;
                }

                default:
                    return 0;
            }
        }

        case TYPE_DOUBLE_INT:
        {
            int i;
            const double * d = (double*)d_;
            const double * k = (double*)k_;
            double * out = (double*)out_;
            dstride /= sizeof(double);
            kstride /= sizeof(double);
            ostride /= sizeof(double);
            /* unroll inner loop to optimize register usage of the kernel*/
            switch (nk) {
                case 1:
                {
                    /* load kernel */
                    const double k1 = k[(1 - 1) * kstride];


                    for (i = 0; i < nd; i++) {
                        double s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        out[i * ostride] = s;
                    }
                    return 1;
                }
                case 2:
                {
                    /* load kernel */
                    const double k1 = k[(1 - 1) * kstride];
                    /* load kernel */
                    const double k2 = k[(2 - 1) * kstride];


                    for (i = 0; i < nd; i++) {
                        double s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        s += d[(i + 2 - 1) * dstride] * k2;


                        out[i * ostride] = s;
                    }
                    return 1;
                }
                case 3:
                {
                    /* load kernel */
                    const double k1 = k[(1 - 1) * kstride];
                    const double k2 = k[(2 - 1) * kstride];
                    /* load kernel */
                    const double k3 = k[(3 - 1) * kstride];


                    for (i = 0; i < nd; i++) {
                        double s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        s += d[(i + 2 - 1) * dstride] * k2;
                        s += d[(i + 3 - 1) * dstride] * k3;


                        out[i * ostride] = s;
                    }
                    return 1;
                }
                case 4:
                {
                    /* load kernel */
                    const double k1 = k[(1 - 1) * kstride];
                    /* load kernel */
                    const double k2 = k[(2 - 1) * kstride];
                    /* load kernel */
                    const double k3 = k[(3 - 1) * kstride];
                    /* load kernel */
                    const double k4 = k[(4 - 1) * kstride];

                    for (i = 0; i < nd; i++) {
                        double s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        s += d[(i + 2 - 1) * dstride] * k2;
                        s += d[(i + 3 - 1) * dstride] * k3;
                        s += d[(i + 4 - 1) * dstride] * k4;

                        out[i * ostride] = s;
                    }
                    return 1;
                }
                case 5:
                {
                    /* load kernel */
                    const double k1 = k[(1 - 1) * kstride];
                    /* load kernel */
                    const double k2 = k[(2 - 1) * kstride];
                    /* load kernel */
                    const double k3 = k[(3 - 1) * kstride];
                    /* load kernel */
                    const double k4 = k[(4 - 1) * kstride];
                    /* load kernel */
                    const double k5 = k[(5 - 1) * kstride];

                    for (i = 0; i < nd; i++) {
                        double s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        s += d[(i + 2 - 1) * dstride] * k2;
                        s += d[(i + 3 - 1) * dstride] * k3;
                        s += d[(i + 4 - 1) * dstride] * k4;
                        s += d[(i + 5 - 1) * dstride] * k5;

                        out[i * ostride] = s;
                    }
                    return 1;
                }
                case 6:
                {
                    /* load kernel */
                    const double k1 = k[(1 - 1) * kstride];
                    const double k2 = k[(2 - 1) * kstride];
                    const double k3 = k[(3 - 1) * kstride];
                    const double k4 = k[(4 - 1) * kstride];
                    const double k5 = k[(5 - 1) * kstride];
                    const double k6 = k[(6 - 1) * kstride];

                    for (i = 0; i < nd; i++) {
                        double s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        s += d[(i + 2 - 1) * dstride] * k2;
                        s += d[(i + 3 - 1) * dstride] * k3;
                        s += d[(i + 4 - 1) * dstride] * k4;
                        s += d[(i + 5 - 1) * dstride] * k5;
                        s += d[(i + 6 - 1) * dstride] * k6;

                        out[i * ostride] = s;
                    }
                    return 1;
                }
                case 7:
                {
                    /* load kernel */
                    const double k1 = k[(1 - 1) * kstride];
                    /* load kernel */
                    const double k2 = k[(2 - 1) * kstride];
                    /* load kernel */
                    const double k3 = k[(3 - 1) * kstride];
                    /* load kernel */
                    const double k4 = k[(4 - 1) * kstride];
                    /* load kernel */
                    const double k5 = k[(5 - 1) * kstride];
                    /* load kernel */
                    const double k6 = k[(6 - 1) * kstride];
                    /* load kernel */
                    const double k7 = k[(7 - 1) * kstride];

                    for (i = 0; i < nd; i++) {
                        double s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        s += d[(i + 2 - 1) * dstride] * k2;
                        s += d[(i + 3 - 1) * dstride] * k3;
                        s += d[(i + 4 - 1) * dstride] * k4;
                        s += d[(i + 5 - 1) * dstride] * k5;
                        s += d[(i + 6 - 1) * dstride] * k6;
                        s += d[(i + 7 - 1) * dstride] * k7;

                        out[i * ostride] = s;
                    }
                    return 1;
                }
                case 8:
                {
                    /* load kernel */
                    const double k1 = k[(1 - 1) * kstride];
                    /* load kernel */
                    const double k2 = k[(2 - 1) * kstride];
                    /* load kernel */
                    const double k3 = k[(3 - 1) * kstride];
                    /* load kernel */
                    const double k4 = k[(4 - 1) * kstride];
                    /* load kernel */
                    const double k5 = k[(5 - 1) * kstride];
                    /* load kernel */
                    const double k6 = k[(6 - 1) * kstride];
                    /* load kernel */
                    const double k7 = k[(7 - 1) * kstride];
                    const double k8 = k[(8 - 1) * kstride];


                    for (i = 0; i < nd; i++) {
                        double s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        s += d[(i + 2 - 1) * dstride] * k2;
                        s += d[(i + 3 - 1) * dstride] * k3;
                        s += d[(i + 4 - 1) * dstride] * k4;
                        s += d[(i + 5 - 1) * dstride] * k5;
                        s += d[(i + 6 - 1) * dstride] * k6;
                        s += d[(i + 7 - 1) * dstride] * k7;
                        s += d[(i + 8 - 1) * dstride] * k8;


                        out[i * ostride] = s;
                    }
                    return 1;
                }
                case 9:
                {
                    /* load kernel */
                    const double k1 = k[(1 - 1) * kstride];
                    /* load kernel */
                    const double k2 = k[(2 - 1) * kstride];
                    /* load kernel */
                    const double k3 = k[(3 - 1) * kstride];
                    /* load kernel */
                    const double k4 = k[(4 - 1) * kstride];
                    /* load kernel */
                    const double k5 = k[(5 - 1) * kstride];
                    /* load kernel */
                    const double k6 = k[(6 - 1) * kstride];
                    /* load kernel */
                    const double k7 = k[(7 - 1) * kstride];
                    /* load kernel */
                    const double k8 = k[(8 - 1) * kstride];
                    /* load kernel */
                    const double k9 = k[(9 - 1) * kstride];

                    for (i = 0; i < nd; i++) {
                        double s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        s += d[(i + 2 - 1) * dstride] * k2;
                        s += d[(i + 3 - 1) * dstride] * k3;
                        s += d[(i + 4 - 1) * dstride] * k4;
                        s += d[(i + 5 - 1) * dstride] * k5;
                        s += d[(i + 6 - 1) * dstride] * k6;
                        s += d[(i + 7 - 1) * dstride] * k7;
                        s += d[(i + 8 - 1) * dstride] * k8;
                        s += d[(i + 9 - 1) * dstride] * k9;
                        out[i * ostride] = s;
                    }
                    return 1;
                }
                case 10:
                {
                    /* load kernel */
                    const double k1 = k[(1 - 1) * kstride];
                    /* load kernel */
                    const double k2 = k[(2 - 1) * kstride];
                    /* load kernel */
                    const double k3 = k[(3 - 1) * kstride];
                    /* load kernel */
                    const double k4 = k[(4 - 1) * kstride];
                    /* load kernel */
                    const double k5 = k[(5 - 1) * kstride];
                    /* load kernel */
                    const double k6 = k[(6 - 1) * kstride];
                    /* load kernel */
                    const double k7 = k[(7 - 1) * kstride];
                    /* load kernel */
                    const double k8 = k[(8 - 1) * kstride];
                    /* load kernel */
                    const double k9 = k[(9 - 1) * kstride];
                    /* load kernel */
                    const double k10 = k[(10 - 1) * kstride];


                    for (i = 0; i < nd; i++) {
                        double s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        s += d[(i + 2 - 1) * dstride] * k2;
                        s += d[(i + 3 - 1) * dstride] * k3;
                        s += d[(i + 4 - 1) * dstride] * k4;
                        s += d[(i + 5 - 1) * dstride] * k5;
                        s += d[(i + 6 - 1) * dstride] * k6;
                        s += d[(i + 7 - 1) * dstride] * k7;
                        s += d[(i + 8 - 1) * dstride] * k8;
                        s += d[(i + 9 - 1) * dstride] * k9;
                        s += d[(i + 10 - 1) * dstride] * k10;
                        out[i * ostride] = s;
                    }
                    return 1;
                }

                case 11:
                {
                    /* load kernel */
                    const double k1 = k[(1 - 1) * kstride];
                    /* load kernel */
                    const double k2 = k[(2 - 1) * kstride];
                    /* load kernel */
                    const double k3 = k[(3 - 1) * kstride];
                    /* load kernel */
                    const double k4 = k[(4 - 1) * kstride];
                    /* load kernel */
                    const double k5 = k[(5 - 1) * kstride];
                    /* load kernel */
                    const double k6 = k[(6 - 1) * kstride];
                    /* load kernel */
                    const double k7 = k[(7 - 1) * kstride];
                    /* load kernel */
                    const double k8 = k[(8 - 1) * kstride];
                    /* load kernel */
                    const double k9 = k[(9 - 1) * kstride];
                    /* load kernel */
                    const double k10 = k[(10 - 1) * kstride];
                    /* load kernel */
                    const double k11 = k[(11 - 1) * kstride];
                    for (i = 0; i < nd; i++) {
                        double s = 0;
                        s += d[(i + 1 - 1) * dstride] * k1;
                        s += d[(i + 2 - 1) * dstride] * k2;
                        s += d[(i + 3 - 1) * dstride] * k3;
                        s += d[(i + 4 - 1) * dstride] * k4;
                        s += d[(i + 5 - 1) * dstride] * k5;
                        s += d[(i + 6 - 1) * dstride] * k6;
                        s += d[(i + 7 - 1) * dstride] * k7;
                        s += d[(i + 8 - 1) * dstride] * k8;
                        s += d[(i + 9 - 1) * dstride] * k9;
                        s += d[(i + 10 - 1) * dstride] * k10;
                        s += d[(i + 11 - 1) * dstride] * k11;
                        out[i * ostride] = s;
                    }
                    return 1;
                }

                default:
                    return 0;
            }
        }

        default:
            return 0;
    }
}
