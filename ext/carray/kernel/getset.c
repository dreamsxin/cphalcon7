#include "getset.h"
#include "iterators.h"
#include "carray.h"
#include "alloc.h"

int 
array_flat_set(CArray * self, CArray * val) 
{
    CArray * arr = NULL;
    int retval = -1;
    CArrayIterator   *self_it, *arr_it;
    CArrayDescriptor *typecode;
    int swap;
    CArray_CopySwapFunc *copyswap;

    typecode = emalloc(sizeof(CArrayDescriptor));
    memcpy(typecode, CArray_DESCR(self), sizeof(CArrayDescriptor));
    CArrayDescriptor_INCREF(typecode);

    CArrayDescriptor_INCREF(CArray_DESCR(val));
    arr = CArray_FromAnyUnwrap(val, typecode, 0, 0, CARRAY_ARRAY_FORCECAST, NULL);

    if(arr == NULL) {
        return -1;
    }

    arr_it = CArray_NewIter(arr);
    if (arr_it == NULL) {
        goto exit;
    }

    self_it = CArray_NewIter(self);
    if (self_it == NULL) {
        goto exit;
    }

    if (arr_it->size == 0) {
        retval = 0;
        goto exit;
    }

    swap = CArray_ISNOTSWAPPED(self) != CArray_ISNOTSWAPPED(arr);
    copyswap = CArray_DESCR(self)->f->copyswap;
    
    if (CArray_DESCR(self)->refcount) {
        while (self_it->index < self_it->size) {
            memmove(self_it->data_pointer, arr_it->data_pointer, CArray_SIZE(self));
            CArrayIterator_NEXT(self_it);
            CArrayIterator_NEXT(arr_it);
            if (arr_it->index == arr_it->size) {
                CArrayIterator_RESET(arr_it);
            }
        }
        retval = 0;
        goto exit;
    }

    while(self_it->index < self_it->size) {
        memmove(self_it->data_pointer, arr_it->data_pointer, CArray_ITEMSIZE(self));
        if (swap) {
            copyswap(self_it->data_pointer, NULL, swap, self);
        }
        CArrayIterator_NEXT(self_it);
        CArrayIterator_NEXT(arr_it);
    
        if (arr_it->index == arr_it->size) {
            CArrayIterator_RESET(arr_it);
        }
    }
    retval = 0;
    CArrayIterator_FREE(arr_it);
    CArrayIterator_FREE(self_it);
    efree(typecode);
    return retval;
exit:
    CArray_DECREF(arr);
    return retval;
}
