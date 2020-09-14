#include "carray.h"
#include "alloc.h"
#include "buffer.h"
#include "common/exceptions.h"
#include "common/common.h"
#include "common/sort.h"
#include "iterators.h"
#include "common/partition.h"

/**
 * FAST TAKE
 */ 
int
INT_fasttake(int *dest, int *src, int *indarray,
                    int nindarray, int n_outer,
                    int m_middle, int nelem,
                    CARRAY_CLIPMODE clipmode)
{
    int i, j, k, tmp;
    switch(clipmode) {
    case CARRAY_RAISE:
        for (i = 0; i < n_outer; i++) {
            for (j = 0; j < m_middle; j++) {
                tmp = indarray[j];
                /*
                 * We don't know what axis we're operating on,
                 * so don't report it in case of an error.
                 */
                if (check_and_adjust_index(&tmp, nindarray, -1) < 0) {
                    return 1;
                }
                if (CARRAY_UNLIKELY(nelem == 1)) {
                    *dest++ = *(src + tmp);
                }
                else {
                    for (k = 0; k < nelem; k++) {
                        *dest++ = *(src + tmp*nelem + k);
                    }
                }
            }
            src += nelem*nindarray;
        }
        break;
    case CARRAY_WRAP:
        for (i = 0; i < n_outer; i++) {
            for (j = 0; j < m_middle; j++) {
                tmp = indarray[j];
                if (tmp < 0) {
                    while (tmp < 0) {
                        tmp += nindarray;
                    }
                }
                else if (tmp >= nindarray) {
                    while (tmp >= nindarray) {
                        tmp -= nindarray;
                    }
                }
                if (CARRAY_UNLIKELY(nelem == 1)) {
                    *dest++ = *(src+tmp);
                }
                else {
                    for (k = 0; k < nelem; k++) {
                        *dest++ = *(src+tmp*nelem+k);
                    }
                }
            }
            src += nelem*nindarray;
        }
        break;
    case CARRAY_CLIP:
        for (i = 0; i < n_outer; i++) {
            for (j = 0; j < m_middle; j++) {
                tmp = indarray[j];
                if (tmp < 0) {
                    tmp = 0;
                }
                else if (tmp >= nindarray) {
                    tmp = nindarray - 1;
                }
                if (CARRAY_UNLIKELY(nelem == 1)) {
                    *dest++ = *(src + tmp);
                }
                else {
                    for (k = 0; k < nelem; k++) {
                        *dest++ = *(src + tmp*nelem + k);
                    }
                }
            }
            src += nelem*nindarray;
        }
        break;
    }
    return 0;
}
int
DOUBLE_fasttake(double *dest, double *src, int *indarray,
                    int nindarray, int n_outer,
                    int m_middle, int nelem,
                    CARRAY_CLIPMODE clipmode)
{
    int i, j, k, tmp;

    switch(clipmode) {
    case CARRAY_RAISE:
        for (i = 0; i < n_outer; i++) {
            for (j = 0; j < m_middle; j++) {
                tmp = indarray[j];
                /*
                 * We don't know what axis we're operating on,
                 * so don't report it in case of an error.
                 */
                if (check_and_adjust_index(&tmp, nindarray, -1) < 0) {
                    return 1;
                }
                if (CARRAY_LIKELY(nelem == 1)) {
                    *dest++ = *(src + tmp);
                }
                else {
                    for (k = 0; k < nelem; k++) {
                        *dest++ = *(src + tmp*nelem + k);
                    }
                }
            }
            src += nelem*nindarray;
        }
        break;
    case CARRAY_WRAP:
        for (i = 0; i < n_outer; i++) {
            for (j = 0; j < m_middle; j++) {
                tmp = indarray[j];
                if (tmp < 0) {
                    while (tmp < 0) {
                        tmp += nindarray;
                    }
                }
                else if (tmp >= nindarray) {
                    while (tmp >= nindarray) {
                        tmp -= nindarray;
                    }
                }
                if (CARRAY_LIKELY(nelem == 1)) {
                    *dest++ = *(src+tmp);
                }
                else {
                    for (k = 0; k < nelem; k++) {
                        *dest++ = *(src+tmp*nelem+k);
                    }
                }
            }
            src += nelem*nindarray;
        }
        break;
    case CARRAY_CLIP:
        for (i = 0; i < n_outer; i++) {
            for (j = 0; j < m_middle; j++) {
                tmp = indarray[j];
                if (tmp < 0) {
                    tmp = 0;
                }
                else if (tmp >= nindarray) {
                    tmp = nindarray - 1;
                }
                if (CARRAY_LIKELY(nelem == 1)) {
                    *dest++ = *(src + tmp);
                }
                else {
                    for (k = 0; k < nelem; k++) {
                        *dest++ = *(src + tmp*nelem + k);
                    }
                }
            }
            src += nelem*nindarray;
        }
        break;
    }
    return 0;
}


/*
 * Diagonal
 */
CArray * 
CArray_Diagonal(CArray *self, int offset, int axis1, int axis2, MemoryPointer * rtn_ptr)
{
    int i, idim, ndim = CArray_NDIM(self);
    int *strides;
    int stride1, stride2, offset_stride;
    int *shape, dim1, dim2;

    char *data;
    int diag_size;
    CArrayDescriptor *dtype;
    CArray *ret = emalloc(sizeof(CArray));
    int ret_shape[CARRAY_MAXDIMS], ret_strides[CARRAY_MAXDIMS];

    if (ndim < 2) {
        throw_valueerror_exception("diag requires an array of at least two dimensions");
    }

    if (check_and_adjust_axis_msg(&axis1, ndim) < 0) {
        return NULL;
    }
    if (check_and_adjust_axis_msg(&axis2, ndim) < 0) {
        return NULL;
    }

    if (axis1 == axis2) {
        throw_valueerror_exception("axis1 and axis2 cannot be the same");
    }

    /* Get the shape and strides of the two axes */
    shape = CArray_DIMS(self);
    dim1 = shape[axis1];
    dim2 = shape[axis2];
    strides = CArray_STRIDES(self);
    stride1 = strides[axis1];
    stride2 = strides[axis2];

    /* Compute the data pointers and diag_size for the view */
    data = CArray_DATA(self);
    if (offset >= 0) {
        offset_stride = stride2;
        dim2 -= offset;
    }
    else {
        offset = -offset;
        offset_stride = stride1;
        dim1 -= offset;
    }
    diag_size = dim2 < dim1 ? dim2 : dim1;
    if (diag_size < 0) {
        diag_size = 0;
    }
    else {
        data += offset * offset_stride;
    }

    /* Build the new shape and strides for the main data */
    i = 0;
    for (idim = 0; idim < ndim; ++idim) {
        if (idim != axis1 && idim != axis2) {
            ret_shape[i] = shape[idim];
            ret_strides[i] = strides[idim];
            ++i;
        }
    }
    ret_shape[ndim-2] = diag_size;
    ret_strides[ndim-2] = stride1 + stride2;

    /* Create the diagonal view */
    dtype = CArray_DESCR(self);
    ret = CArray_NewFromDescrAndBase(
            ret, dtype, ndim-1, ret_shape, ret_strides, data,
            CArray_FLAGS(self), self);

    ret->flags &= ~CARRAY_ARRAY_WRITEABLE;
    ret->flags |= CARRAY_ARRAY_C_CONTIGUOUS | CARRAY_ARRAY_F_CONTIGUOUS;

    if (ret == NULL) {
        return NULL;
    }

    if(rtn_ptr != NULL) {
        
        add_to_buffer(rtn_ptr, ret, sizeof(CArray));
    }

    return ret;
}

/**
 * TakeFrom
 **/ 
CArray *
CArray_TakeFrom(CArray * target, CArray * indices0, int axis, 
                MemoryPointer * out_ptr, CARRAY_CLIPMODE clipmode)
{
    CArrayDescriptor * indices_type;
    CArray * out = NULL;
    CArrayDescriptor *dtype;
    CArray_FastTakeFunc *func;
    CArray *obj = NULL, *self, *indices;
    int nd, i, j, n, m, k, max_item, tmp, chunk, itemsize, nelem;
    int shape[CARRAY_MAXDIMS];
    char *src, *dest, *tmp_src;
    int err;
    int needs_refcounting;
    int auto_free = 0;

    indices = NULL;
    
    if (axis == INT_MAX) {
        auto_free = 1;
    }

    self = (CArray *)CArray_CheckAxis(target, &axis, CARRAY_ARRAY_CARRAY_RO);
    
    if (self == NULL) {
        return NULL;
    }
    
    indices_type = CArray_DescrFromType(TYPE_INTEGER_INT);
    indices = CArray_FromCArray(indices0, indices_type, 0);

    if (indices == NULL) {
        goto fail;
    }
    

    n = m = chunk = 1;
    nd = CArray_NDIM(self) + CArray_NDIM(indices) - 1;
    
    for (i = 0; i < nd; i++) {
        if (i < axis) {
            shape[i] = CArray_DIMS(self)[i];
            n *= shape[i];
        }
        else {
            if (i < axis+CArray_NDIM(indices)) {
                shape[i] = CArray_DIMS(indices)[i-axis];
                m *= shape[i];
            }
            else {
                shape[i] = CArray_DIMS(self)[i-CArray_NDIM(indices)+1];
                chunk *= shape[i];
            }
        }
    }
    
    
    if (out == NULL) {
        if (obj == NULL) {
            obj = emalloc(sizeof(CArray));
        }
        dtype = CArray_DESCR(self);
        CArrayDescriptor_INCREF(dtype);
        obj = (CArray *)CArray_NewFromDescr(obj, dtype, nd, shape,
                                            NULL, NULL, 0, self);
        
        if (obj == NULL) {
            goto fail;
        }
    }
    else {
        int flags = CARRAY_ARRAY_CARRAY | CARRAY_ARRAY_WRITEBACKIFCOPY;

        if ((CArray_NDIM(out) != nd) ||
            !CArray_CompareLists(CArray_DIMS(out), shape, nd)) {
            throw_valueerror_exception("output array does not match result of ndarray.take");
            goto fail;
        }

        if (clipmode == CARRAY_RAISE) {
            /*
             * we need to make sure and get a copy
             * so the input array is not changed
             * before the error is called
             */
            flags |= CARRAY_ARRAY_ENSURECOPY;
        }
        dtype = CArray_DESCR(self);
        CArrayDescriptor_INCREF(dtype);
        obj = (CArray *)CArray_FromCArray(out, dtype, flags);
        if (obj == NULL) {
            goto fail;
        }
    }

    max_item = CArray_DIMS(self)[axis];
    nelem = chunk;
    itemsize = CArray_ITEMSIZE(obj);
    chunk = chunk * itemsize;
    src = CArray_DATA(self);
    dest = CArray_DATA(obj);
    needs_refcounting = CArrayDataType_REFCHK(CArray_DESCR(self));


    if ((max_item == 0) && (CArray_SIZE(obj) != 0)) {
        throw_indexerror_exception("cannot do a non-empty take from an empty axes.");
        goto fail;
    }

    func = CArray_DESCR(self)->f->fasttake;
    if (func == NULL) {
        goto fail;
    }
    else {
        /* no gil release, need it for error reporting */
        err = func(dest, src, (int *)(CArray_DATA(indices)),
                    max_item, n, m, nelem, clipmode);
        if (err) {
            goto fail;
        }
    }

    CArray_DECREF(indices);
    if (out != NULL && out != obj) {
        CArray_INCREF(out);
        CArray_ResolveWritebackIfCopy(obj);
        CArray_DECREF(obj);
        obj = out;
    }
    if(out_ptr != NULL) {
        add_to_buffer(out_ptr, obj, sizeof(CArray));
    }
    CArrayDescriptor_FREE(indices_type);
    
    CArray_DECREF(target);
    if(auto_free) {    
        CArrayDescriptor_DECREF(CArray_DESCR(self));
        CArray_Free(self);
    } else {
        CArray_DECREF(target);
    }
    return obj;
fail:

    return NULL;
}

/*
 * These algorithms use special sorting.  They are not called unless the
 * underlying sort function for the type is available.  Note that axis is
 * already valid. The sort functions require 1-d contiguous and well-behaved
 * data.  Therefore, a copy will be made of the data if needed before handing
 * it to the sorting routine.  An iterator is constructed and adjusted to walk
 * over all but the desired sorting axis.
 */
static int
_new_sortlike(CArray *op, int axis, CArray_SortFunc *sort,
              CArray_PartitionFunc *part, int *kth, int nkth)
{
    int N = CArray_DIM(op, axis);
    int elsize = (int)CArray_ITEMSIZE(op);
    int astride = CArray_STRIDE(op, axis);
    int swap = CArray_ISBYTESWAPPED(op);
    int needcopy = !CArray_ISALIGNED(op) || swap || astride != elsize;
    int hasrefs = CArrayDataType_REFCHK(CArray_DESCR(op));

    CArray_CopySwapNFunc *copyswapn = CArray_DESCR(op)->f->copyswapn;
    char *buffer = NULL;
    CArrayIterator *it;
    int size;

    int ret = 0;

    /* Check if there is any sorting to do */
    if (N <= 1 || CArray_SIZE(op) == 0) {
        return 0;
    }

    it = (CArrayIterator *)CArray_IterAllButAxis(op, &axis);
    if (it == NULL) {
        return -1;
    }
    size = it->size;

    if (needcopy) {
        buffer = emalloc(N * elsize);
        if (buffer == NULL) {
            ret = -1;
            goto fail;
        }
    }

    while (size--) {
        char *bufptr = it->data_pointer;

        if (needcopy) {
            if (hasrefs) {
                /*
                 * For dtype's with objects, copyswapn Py_XINCREF's src
                 * and Py_XDECREF's dst. This would crash if called on
                 * an uninitialized buffer, or leak a reference to each
                 * object if initialized.
                 *
                 * So, first do the copy with no refcounting...
                 */
                _unaligned_strided_byte_copy(buffer, elsize, it->data_pointer, astride, N, elsize, CArray_DESCR(op));
                /* ...then swap in-place if needed */
                if (swap) {
                    copyswapn(buffer, elsize, NULL, 0, N, swap, op);
                }
            }
            else {
                copyswapn(buffer, elsize, it->data_pointer, astride, N, swap, op);
            }
            bufptr = buffer;
        }
        /*
         * TODO: If the input array is byte-swapped but contiguous and
         * aligned, it could be swapped (and later unswapped) in-place
         * rather than after copying to the buffer. Care would have to
         * be taken to ensure that, if there is an error in the call to
         * sort or part, the unswapping is still done before returning.
         */

        if (part == NULL) {
            ret = sort(bufptr, N, op);
            if (hasrefs) {
                ret = -1;
            }
            if (ret < 0) {
                goto fail;
            }
        }
        else {
            int pivots[CARRAY_MAX_PIVOT_STACK];
            int npiv = 0;
            int i;
            for (i = 0; i < nkth; ++i) {
                ret = part(bufptr, N, kth[i], pivots, &npiv, op);
                if (hasrefs) {
                    ret = -1;
                }
                if (ret < 0) {
                    goto fail;
                }
            }
        }

        if (needcopy) {
            if (hasrefs) {
                if (swap) {
                    copyswapn(buffer, elsize, NULL, 0, N, swap, op);
                }
                _unaligned_strided_byte_copy(it->data_pointer, astride,
                                             buffer, elsize, N, elsize, CArray_DESCR(op));
            }
            else {
                copyswapn(it->data_pointer, astride, buffer, elsize, N, swap, op);
            }
        }

        CArrayIterator_NEXT(it);
    }

fail:
    efree(buffer);
    if (ret < 0) {
        /* Out of memory during sorting or buffer creation */
        throw_memory_exception("Out of memory");
    }
    CArrayIterator_FREE(it);
    return ret;
}


CArray *
CArray_Sort(CArray * target, int * axis, CARRAY_SORTKIND which, int inplace, MemoryPointer * out)
{
    CArray_SortFunc * sort;
    CArray * op;
    int result;

    if (!inplace) {
        op = CArray_NewLikeArray(target, CARRAY_KEEPORDER, CArray_DESCR(target), 0);
        CArray_CopyInto(op, target);
    } else {
        op = target;
    }

    int n = CArray_NDIM(op);

    if (check_and_adjust_axis(axis, n) < 0) {
        return NULL;
    }

    if (CArray_FailUnlessWriteable(op, "sort array") < 0) {
        return NULL;
    }

    if (which < 0 || which >= CARRAY_NSORTS) {
        throw_valueerror_exception("not a valid sort kind");
        return NULL;
    }

    sort = CArray_DESCR(op)->f->sort[which];
    if (sort == NULL) {
        if (CArray_DESCR(op)->f->compare) {
            switch (which) {
                default:
                case CARRAY_QUICKSORT:
                    sort = carray_quicksort;
                    break;
                case CARRAY_HEAPSORT:
                    sort = carray_heapsort;
                    break;
                case CARRAY_MERGESORT:
                    sort = carray_mergesort;
                    break;
            }
        }
        else {
            throw_typeerror_exception("type does not have compare function");
            return NULL;
        }
    }

    result = _new_sortlike(op, *axis, sort, NULL, NULL, 0);

    if (out != NULL) {
        add_to_buffer(out, op, sizeof(CArray));
    }

    if (result < 0) {
        return NULL;
    }

    return op;
}