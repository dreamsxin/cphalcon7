#include "assign.h"
#include "carray.h"

int
raw_array_is_aligned(int ndim, int *shape, char *data, int *strides, int alignment)
{

    /*
     * The code below expects the following:
     *  * that alignment is a power of two, as required by the C standard.
     *  * that casting from pointer to uintp gives a sensible representation
     *    we can use bitwise operations on (perhaps *not* req. by C std,
     *    but assumed by glibc so it should be fine)
     *  * that casting stride from intp to uintp (to avoid dependence on the
     *    signed int representation) preserves remainder wrt alignment, so
     *    stride%a is the same as ((unsigned intp)stride)%a. Req. by C std.
     *
     *  The code checks whether the lowest log2(alignment) bits of `data`
     *  and all `strides` are 0, as this implies that
     *  (data + n*stride)%alignment == 0 for all integers n.
     */

    if (alignment > 1) {
        uintptr_t align_check = (uintptr_t)data;
        int i;

        for (i = 0; i < ndim; i++) {
            /* skip dim == 1 as it is not required to have stride 0 */
            if (shape[i] > 1) {
                /* if shape[i] == 1, the stride is never used */
                align_check |= (uintptr_t)strides[i];
            }
            else if (shape[i] == 0) {
                /* an array with zero elements is always aligned */
                return 1;
            }
        }

        return carray_is_aligned((void *)align_check, alignment);
    }
    else {
        return 1;
    }
}


int
IsAligned(CArray * array)
{
    if (CArray_DESCR(array) == NULL) {
        return 0;
    }

    return raw_array_is_aligned(CArray_NDIM(array), CArray_DIMS(array),
                                CArray_DATA(array), CArray_STRIDES(array),
                                CArray_DESCR(array)->alignment);
}