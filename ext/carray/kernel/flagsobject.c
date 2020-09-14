#include "flagsobject.h"
#include "carray.h"
#include "assign.h"
#include "common/common.h"

/**
 * Check whether the given array is stored contiguously
 **/ 
static void
_UpdateContiguousFlags(CArray * array)
{
    int sd;
    int dim;
    int i;
    int is_c_contig = 1;

    sd = CArray_ITEMSIZE(array);
    for (i = CArray_NDIM(array) - 1; i >= 0; --i) {
        dim = CArray_DIMS(array)[i];

        if (CArray_STRIDES(array)[i] != sd) {
            is_c_contig = 0;
            break;
         }
        /* contiguous, if it got this far */
        if (dim == 0) {
            break;
        }
        sd *= dim;
    }
    if (is_c_contig) {
        CArray_ENABLEFLAGS(array, CARRAY_ARRAY_C_CONTIGUOUS);
    }
    else {
        CArray_CLEARFLAGS(array, CARRAY_ARRAY_C_CONTIGUOUS);
    }
}

/**
 * Update CArray flags
 **/ 
void
CArray_UpdateFlags(CArray * array, int flagmask)
{
    /* Always update both, as its not trivial to guess one from the other */
    if (flagmask & (CARRAY_ARRAY_F_CONTIGUOUS | CARRAY_ARRAY_C_CONTIGUOUS)) {
        _UpdateContiguousFlags(array);
    }

    if (flagmask & CARRAY_ARRAY_ALIGNED) {
        if (IsAligned(array)) {
            CArray_ENABLEFLAGS(array, CARRAY_ARRAY_ALIGNED);
        }
        else {
            CArray_CLEARFLAGS(array, CARRAY_ARRAY_ALIGNED);
        }
    }

    if (flagmask & CARRAY_ARRAY_WRITEABLE) {
        if (_IsWriteable(array)) {
            CArray_ENABLEFLAGS(array, CARRAY_ARRAY_WRITEABLE);
        }
        else {
            CArray_CLEARFLAGS(array, CARRAY_ARRAY_WRITEABLE);
        }
    }
    return;
}

