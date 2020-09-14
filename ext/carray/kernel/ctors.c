#include "ctors.h"
#include "carray.h"
#include "alloc.h"
#include "convert.h"
#include "convert_type.h"
#include "php.h"
#include "iterators.h"

int
setArrayFromSequence(CArray *a, CArray *s, int dim, int offset)
{
    int i, slen;
    int res = -1;

    /* INCREF on entry DECREF on exit */
    CArray_INCREF(s);

    if (dim > a->ndim) {
        throw_valueerror_exception("setArrayFromSequence: sequence/array dimensions mismatch.");
        goto fail;
    }

    slen = CArray_SIZE(s);
    if (slen != a->dimensions[dim]) {
        throw_valueerror_exception("setArrayFromSequence: sequence/array shape mismatch.");
        goto fail;
    }

    for (i = 0; i < slen; i++) {
        CArray *o = CArray_Slice_Index(s, i, NULL);
        if ((a->ndim - dim) > 1) {
            res = setArrayFromSequence(a, o, dim+1, offset);
        }
        else {
            res = a->descriptor->f->setitem(o->data, (a->data + offset), a);
        }
        CArray_DECREF(o);
        CArray_DECREF(s);
        CArrayDescriptor_FREE(o->descriptor);
        CArray_Free(o);
        if (res < 0) {
            goto fail;
        }
        offset += a->strides[dim];
    }
    CArray_DECREF(s);
    return 0;

fail:
    CArray_DECREF(s);
    return res;
}

int
CArray_AssignFromSequence(CArray *self, CArray *v)
{
    if (CArray_NDIM(self) == 0) {
        throw_valueerror_exception("assignment to 0-d array");
        return -1;
    }
    return setArrayFromSequence(self, v, 0, 0);
}

CArray *
CArray_FromArray(CArray *arr, CArrayDescriptor *newtype, int flags)
{

    CArray *ret = NULL;
    int itemsize;
    int copy = 0;
    int arrflags;
    CArrayDescriptor *oldtype;
    CARRAY_CASTING casting = CARRAY_SAFE_CASTING;

    oldtype = CArray_DESCR(arr);
    if (newtype == NULL) {
        /*
         * Check if object is of array with Null newtype.
         * If so return it directly instead of checking for casting.
         */
        if (flags == 0) {
            CArray_INCREF(arr);
            return (CArray *)arr;
        }
        newtype = oldtype;
        CArrayDescriptor_INCREF(oldtype);
    }
    itemsize = newtype->elsize;
    if (itemsize == 0) {
        CArray_DESCR_REPLACE(newtype);
        if (newtype == NULL) {
            return NULL;
        }
        newtype->elsize = oldtype->elsize;
        itemsize = newtype->elsize;
    }

    /* If the casting if forced, use the 'unsafe' casting rule */
    if (flags & CARRAY_ARRAY_FORCECAST) {
        casting = CARRAY_UNSAFE_CASTING;
    }

    /* Raise an error if the casting rule isn't followed */
    if (!CArray_CanCastArrayTo(arr, newtype, casting)) {
        throw_valueerror_exception("Cannot cast array data according to the rule");
        CArrayDescriptor_DECREF(newtype);
        return NULL;
    }

    arrflags = CArray_FLAGS(arr);
    /* If a guaranteed copy was requested */
    copy = (flags & CARRAY_ARRAY_ENSURECOPY) ||
           /* If C contiguous was requested, and arr is not */
           ((flags & CARRAY_ARRAY_C_CONTIGUOUS) &&
            (!(arrflags & CARRAY_ARRAY_C_CONTIGUOUS))) ||
           /* If an aligned array was requested, and arr is not */
           ((flags & CARRAY_ARRAY_ALIGNED) &&
            (!(arrflags & CARRAY_ARRAY_ALIGNED))) ||
           /* If a Fortran contiguous array was requested, and arr is not */
           ((flags & CARRAY_ARRAY_F_CONTIGUOUS) &&
            (!(arrflags & CARRAY_ARRAY_F_CONTIGUOUS))) ||
           /* If a writeable array was requested, and arr is not */
           ((flags & CARRAY_ARRAY_WRITEABLE) &&
            (!(arrflags & CARRAY_ARRAY_WRITEABLE))) ||
           !CArray_EquivTypes(oldtype, newtype);

    if (copy) {
        CARRAY_ORDER order = CARRAY_KEEPORDER;
        int subok = 1;

        /* Set the order for the copy being made based on the flags */
        if (flags & CARRAY_ARRAY_F_CONTIGUOUS) {
            order = CARRAY_FORTRANORDER;
        }
        else if (flags & CARRAY_ARRAY_C_CONTIGUOUS) {
            order = CARRAY_CORDER;
        }

        if ((flags & CARRAY_ARRAY_ENSUREARRAY)) {
            subok = 0;
        }

        ret = CArray_NewLikeArray(arr, order, newtype, subok);

        if (ret == NULL) {
            return NULL;
        }

        if (CArray_CopyInto(ret, arr) < 0) {
            CArray_DECREF(ret);
            return NULL;
        }

        if (flags & CARRAY_ARRAY_UPDATEIFCOPY)  {
            ret->flags |= CARRAY_ARRAY_UPDATEIFCOPY;
            ret->base = arr;
            ret->flags &= ~CARRAY_ARRAY_WRITEABLE;
            CArray_INCREF(arr);
        }

    }
        /*
         * If no copy then take an appropriate view if necessary, or
         * just return a reference to ret itself.
         */
    else {
        CArray_INCREF(arr);
        ret = arr;
    }

    return ret;
}

static void
_carray_to_array_recursive(CArray * array, int * dimension, zval * current_dim_z, CArrayIterator * it)
{
    zval tmp;
    zval * current;
    int i, next_dim;
    if (*dimension == CArray_NDIM(array)) {
        return;
    }

    if (*dimension < (CArray_NDIM(array) - 1)) {
        *dimension = *dimension + 1;
        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(current_dim_z), current)
        {
            for (i = 0; i < CArray_DIMS(array)[*dimension]; i++) {
                 array_init_size(&tmp, CArray_DIMS(array)[*dimension]);
                 zend_hash_next_index_insert_new(Z_ARRVAL_P(current), &tmp);
                 _carray_to_array_recursive(array, dimension, &tmp, it);
            }
        }ZEND_HASH_FOREACH_END();
    } else {
        if (CArray_NDIM(array) == 2) {
            ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(current_dim_z), current) {
                for (i = 0; i < CArray_DIMS(array)[*dimension]; i++) {
                    if (CArray_TYPE(array) == TYPE_INTEGER_INT) {
                        ZVAL_LONG(&tmp, *((int *) it->data_pointer));
                    }
                    if (CArray_TYPE(array) == TYPE_DOUBLE_INT) {
                        ZVAL_DOUBLE(&tmp, *((double *) it->data_pointer));
                    }
                    zend_hash_next_index_insert_new(Z_ARRVAL_P(current), &tmp);
                    CArrayIterator_NEXT(it);
                }
            }ZEND_HASH_FOREACH_END();
        }
        if (CArray_NDIM(array) > 2) {
            for (i = 0; i < CArray_DIMS(array)[*dimension]; i++) {
                if (CArray_TYPE(array) == TYPE_INTEGER_INT) {
                    ZVAL_LONG(&tmp, *((int *) it->data_pointer));
                }
                if (CArray_TYPE(array) == TYPE_DOUBLE_INT) {
                    ZVAL_DOUBLE(&tmp, *((double *) it->data_pointer));
                }
                zend_hash_next_index_insert_new(Z_ARRVAL_P(current_dim_z), &tmp);
                CArrayIterator_NEXT(it);
            }
        }
    }
}

void
CArray_ToArray(CArray *a, zval * rtn)
{
    CArrayIterator * it;
    int dimension = 0;
    zval * current;
    zval tmp;
    int i, j;
    if (CArray_NDIM(a) == 0) {
        if (CArray_TYPE(a) == TYPE_DOUBLE_INT) {
            ZVAL_DOUBLE(rtn, DDATA(a)[0]);
            return;
        }
        if (CArray_TYPE(a) == TYPE_LONG_INT || CArray_TYPE(a) == TYPE_INTEGER_INT) {
            ZVAL_DOUBLE(rtn, IDATA(a)[0]);
            return;
        }
    }

    if (CArray_NDIM(a) > 1) {
        array_init_size(rtn, CArray_DIMS(a)[0]);
        for (i = 0; i < CArray_DIMS(a)[0]; i++) {
            array_init(&tmp);
            zend_hash_next_index_insert_new(Z_ARRVAL_P(rtn), &tmp);
        }
        dimension++;
        it = CArray_NewIter(a);
        _carray_to_array_recursive(a, &dimension, rtn, it);
        CArrayIterator_FREE(it);
        return;
    }

    if (CArray_NDIM(a) == 1) {
        it = CArray_NewIter(a);
        array_init_size(rtn, CArray_DIMS(a)[0]);
        for (i = 0; i < CArray_DIMS(a)[0]; i++) {
                    if (CArray_TYPE(a) == TYPE_INTEGER_INT) {
                        ZVAL_LONG(&tmp, *((int *) it->data_pointer));
                    }
                    if (CArray_TYPE(a) == TYPE_DOUBLE_INT) {
                        ZVAL_DOUBLE(&tmp, *((double *) it->data_pointer));
                    }
                    zend_hash_next_index_insert_new(Z_ARRVAL_P(rtn), &tmp);
                    CArrayIterator_NEXT(it);
        };
        CArrayIterator_FREE(it);
    }
}