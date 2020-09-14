#ifndef PHPSCI_EXT_STRIDE_LOOPS_H
#define PHPSCI_EXT_STRIDE_LOOPS_H

#include "../carray.h"
#include "common.h"

/* byte swapping functions */
static inline uint16_t
carray_bswap2(uint16_t x)
{
    return ((x & 0xffu) << 8) | (x >> 8);
}

/*
 * treat as int16 and byteswap unaligned memory,
 * some cpus don't support unaligned access
 */
static inline void
carray_bswap2_unaligned(char * x)
{
    char a = x[0];
    x[0] = x[1];
    x[1] = a;
}

static inline uint32_t
carray_bswap4(uint32_t x)
{
#ifdef HAVE___BUILTIN_BSWAP32
    return __builtin_bswap32(x);
#else
    return ((x & 0xffu) << 24) | ((x & 0xff00u) << 8) |
           ((x & 0xff0000u) >> 8) | (x >> 24);
#endif
}

static inline void
carray_bswap4_unaligned(char * x)
{
    char a = x[0];
    x[0] = x[3];
    x[3] = a;
    a = x[1];
    x[1] = x[2];
    x[2] = a;
}

static inline uint64_t
carray_bswap8(uint64_t x)
{
#ifdef HAVE___BUILTIN_BSWAP64
    return __builtin_bswap64(x);
#else
    return ((x & 0xffULL) << 56) |
           ((x & 0xff00ULL) << 40) |
           ((x & 0xff0000ULL) << 24) |
           ((x & 0xff000000ULL) << 8) |
           ((x & 0xff00000000ULL) >> 8) |
           ((x & 0xff0000000000ULL) >> 24) |
           ((x & 0xff000000000000ULL) >> 40) |
           ( x >> 56);
#endif
}

static inline void
carray_bswap8_unaligned(char * x)
{
    char a = x[0]; x[0] = x[7]; x[7] = a;
    a = x[1]; x[1] = x[6]; x[6] = a;
    a = x[2]; x[2] = x[5]; x[5] = a;
    a = x[3]; x[3] = x[4]; x[4] = a;
}

/* Start raw iteration */
#define CARRAY_RAW_ITER_START(idim, ndim, coord, shape) \
        memset((coord), 0, (ndim) * sizeof(coord[0])); \
        do {


/* Increment to the next n-dimensional coordinate for one raw array */
#define CARRAY_RAW_ITER_ONE_NEXT(idim, ndim, coord, shape, data, strides) \
            for ((idim) = 1; (idim) < (ndim); ++(idim)) { \
                if (++(coord)[idim] == (shape)[idim]) { \
                    (coord)[idim] = 0; \
                    (data) -= ((shape)[idim] - 1) * (strides)[idim]; \
                } \
                else { \
                    (data) += (strides)[idim]; \
                    break; \
                } \
            } \
        } while ((idim) < (ndim))

/*
 * This function pointer is for unary operations that input an
 * arbitrarily strided one-dimensional array segment and output
 * an arbitrarily strided array segment of the same size.
 * It may be a fully general function, or a specialized function
 * when the strides or item size have particular known values.
 *
 * Examples of unary operations are a straight copy, a byte-swap,
 * and a casting operation,
 *
 * The 'transferdata' parameter is slightly special, following a
 * generic auxiliary data pattern defined in carray.h
 * Use CARRAY_AUXDATA_CLONE and CARRAY_AUXDATA_FREE to deal with this data.
 *
 */
typedef void (CArray_StridedUnaryOp)(char *dst, int dst_stride,
                                     char *src, int src_stride,
                                     int N, int src_itemsize,
                                     CArrayAuxData *transferdata);


/*
 * Gives back a function pointer to a specialized function for copying
 * strided memory.  Returns NULL if there is a problem with the inputs.
 *
 * aligned:
 *      Should be 1 if the src and dst pointers always point to
 *      locations at which a uint of equal size to dtype->elsize
 *      would be aligned, 0 otherwise.
 * src_stride:
 *      Should be the src stride if it will always be the same,
 *      MAX_INT otherwise.
 * dst_stride:
 *      Should be the dst stride if it will always be the same,
 *      MAX_INT otherwise.
 * itemsize:
 *      Should be the item size if it will always be the same, 0 otherwise.
 *
 */
CArray_StridedUnaryOp * CArray_GetStridedCopyFn(int aligned, int src_stride, int dst_stride, int itemsize);
CArray_StridedUnaryOp * CArray_GetStridedNumericCastFn(int aligned, int src_stride,
                             int dst_stride,
                             int src_type_num, int dst_type_num);
#endif //PHPSCI_EXT_STRIDE_LOOPS_H