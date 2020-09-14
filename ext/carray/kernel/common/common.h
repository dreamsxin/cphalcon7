#ifndef PHPSCI_EXT_COMMON_H
#define PHPSCI_EXT_COMMON_H

#include "../carray.h"
#include "../assign.h"

#  define CARRAY_LONGLONG_SUFFIX(x)  (x##L)
#  define CARRAY_ULONGLONG_SUFFIX(x) (x##UL)

/*
 * numarray-style bit-width typedefs
 */
#define CARRAY_MAX_INT8 127
#define CARRAY_MIN_INT8 -128
#define CARRAY_MAX_UINT8 255
#define CARRAY_MAX_INT16 32767
#define CARRAY_MIN_INT16 -32768
#define CARRAY_MAX_UINT16 65535
#define CARRAY_MAX_INT32 2147483647
#define CARRAY_MIN_INT32 (-CARRAY_MAX_INT32 - 1)
#define CARRAY_MAX_UINT32 4294967295U
#define CARRAY_MAX_INT64 CARRAY_LONGLONG_SUFFIX(9223372036854775807)
#define CARRAY_MIN_INT64 (-CARRAY_MAX_INT64 - CARRAY_LONGLONG_SUFFIX(1))
#define CARRAY_MAX_UINT64 CARRAY_ULONGLONG_SUFFIX(18446744073709551615)
#define CARRAY_MAX_INT128 CARRAY_LONGLONG_SUFFIX(85070591730234615865843651857942052864)
#define CARRAY_MIN_INT128 (-CARRAY_MAX_INT128 - CARRAY_LONGLONG_SUFFIX(1))
#define CARRAY_MAX_UINT128 CARRAY_ULONGLONG_SUFFIX(170141183460469231731687303715884105728)
#define CARRAY_MAX_INT256 CARRAY_LONGLONG_SUFFIX(57896044618658097711785492504343953926634992332820282019728792003956564819967)
#define CARRAY_MIN_INT256 (-CARRAY_MAX_INT256 - CARRAY_LONGLONG_SUFFIX(1))
#define CARRAY_MAX_UINT256 CARRAY_ULONGLONG_SUFFIX(115792089237316195423570985008687907853269984665640564039457584007913129639935)
#define CARRAY_MIN_DATETIME CARRAY_MIN_INT64
#define CARRAY_MAX_DATETIME CARRAY_MAX_INT64
#define CARRAY_MIN_TIMEDELTA CARRAY_MIN_INT64
#define CARRAY_MAX_TIMEDELTA CARRAY_MAX_INT64
#define CARRAY_MAX_INT   INT_MAX
#define CARRAY_MAX_INTP  CARRAY_MAX_INT


#if CARRAY_MAX_INTP > INT_MAX
# define CARRAY_CBLAS_CHUNK  (INT_MAX / 2 + 1)
#else
# define CARRAY_CBLAS_CHUNK  CARRAY_MAX_INTP
#endif


/*
 * Convert CArray stride to BLAS stride. Returns 0 if conversion cannot be done
 * (BLAS won't handle negative or zero strides the way we want).
 */
static inline int
blas_stride(int stride, unsigned itemsize)
{
    if (stride > 0 && carray_is_aligned((void*)&stride, itemsize)) {
        stride /= itemsize;
        if (stride <= INT_MAX) {
            return stride;
        }
    }
    return 0;
}

/* used for some alignment checks */
#define _ALIGN(type) offsetof(struct {char c; type v;}, v)
#define _UINT_ALIGN(type) carray_uint_alignment(sizeof(type))

/* Get equivalent "uint" alignment given an itemsize, for use in copy code */
static inline int
carray_uint_alignment(int itemsize)
{
    int alignment = 0; /* return value of 0 means unaligned */

    switch(itemsize){
        case 1:
            return 1;
        case 2:
            alignment = _ALIGN(uint16_t);
            break;
        case 4:
            alignment = _ALIGN(uint32_t);
            break;
        case 8:
            alignment = _ALIGN(uint64_t);
            break;
        case 16:
            /*
             * 16 byte types are copied using 2 uint64 assignments.
             * See the strided copy function in lowlevel_strided_loops.c.
             */
            alignment = _ALIGN(uint64_t);
            break;
        default:
            break;
    }
    return alignment;
}

/*
 * Returns -1 and sets an exception if *index is an invalid index for
 * an array of size max_item, otherwise adjusts it in place to be
 * 0 <= *index < max_item, and returns 0.
 * 'axis' should be the array axis that is being indexed over, if known. If
 * unknown, use -1.
 * If _save is NULL it is assumed the GIL is taken
 * If _save is not NULL it is assumed the GIL is not taken and it
 * is acquired in the case of an error
 */
static inline int
check_and_adjust_index(int *index, int max_item, int axis)
{
    /* Check that index is valid, taking into account negative indices */
    if (CARRAY_UNLIKELY((*index < -max_item) || (*index >= max_item))) {
        /* Try to be as clear as possible about what went wrong. */
        if (axis >= 0) {
            throw_indexerror_exception("index is out of bounds for axis");
        } else {
            throw_indexerror_exception("index is out of bounds for size");
        }
        return -1;
    }
    /* adjust negative indices */
    if (*index < 0) {
        *index += max_item;
    }
    return 0;
}




CArray * new_array_for_sum(CArray *ap1, CArray *ap2, CArray* out,
                           int nd, int dimensions[], int typenum, CArray **result);
int _IsWriteable(CArray *ap);
#endif //PHPSCI_EXT_COMMON_H