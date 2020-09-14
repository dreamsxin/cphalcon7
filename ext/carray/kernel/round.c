#include "carray.h"
#include "round.h"
#include "alloc.h"
#include "buffer.h"
#include "iterators.h"

CArray *
CArray_Floor(CArray *a, MemoryPointer *out)
{
    CArrayIterator * it1, * it2;
    CArray * rtn;

    rtn = CArray_NewLikeArray(a, CARRAY_KEEPORDER, CArray_DESCR(a), 0);

    it1 = CArray_NewIter(a);
    it2 = CArray_NewIter(rtn);

    switch(CArray_TYPE(rtn)) {
        case TYPE_DOUBLE_INT:
            do {
                *IT_DDATA(it2) = floor(*IT_DDATA(it1));
                CArrayIterator_NEXT(it1);
                CArrayIterator_NEXT(it2);
            } while (CArrayIterator_NOTDONE(it1));
            break;
        case TYPE_INTEGER_INT:
            do {
                *IT_IDATA(it2) = *IT_IDATA(it1);
                CArrayIterator_NEXT(it1);
                CArrayIterator_NEXT(it2);
            } while (CArrayIterator_NOTDONE(it1));
            break;
        default:
            throw_notimplemented_exception();
            goto fail;
    }

    if (out != NULL) {
        add_to_buffer(out, rtn, sizeof(CArray));
    }

    CArrayIterator_FREE(it1);
    CArrayIterator_FREE(it2);
    return rtn;
fail:
    CArrayIterator_FREE(it1);
    CArrayIterator_FREE(it2);
    return NULL;
}

CArray *
CArray_Ceil(CArray *a, MemoryPointer *out)
{
    CArrayIterator * it1, * it2;
    CArray * rtn;

    rtn = CArray_NewLikeArray(a, CARRAY_KEEPORDER, CArray_DESCR(a), 0);

    it1 = CArray_NewIter(a);
    it2 = CArray_NewIter(rtn);

    switch(CArray_TYPE(rtn)) {
        case TYPE_DOUBLE_INT:
            do {
                *IT_DDATA(it2) = ceil(*IT_DDATA(it1));
                CArrayIterator_NEXT(it1);
                CArrayIterator_NEXT(it2);
            } while (CArrayIterator_NOTDONE(it1));
            break;
        case TYPE_INTEGER_INT:
            do {
                *IT_IDATA(it2) = (int)ceil((double)*IT_IDATA(it1));
                CArrayIterator_NEXT(it1);
                CArrayIterator_NEXT(it2);
            } while (CArrayIterator_NOTDONE(it1));
            break;
        default:
            throw_notimplemented_exception();
            goto fail;
    }

    if (out != NULL) {
        add_to_buffer(out, rtn, sizeof(CArray));
    }

    CArrayIterator_FREE(it1);
    CArrayIterator_FREE(it2);
    return rtn;
fail:
    CArrayIterator_FREE(it1);
    CArrayIterator_FREE(it2);
    return NULL;
}

CArray *
CArray_Round(CArray *a, int decimals, MemoryPointer *out)
{
    CArrayIterator * it1, * it2;
    CArray * rtn;
    int multiplier = 1, negative_decimals = 0, i;

    rtn = CArray_NewLikeArray(a, CARRAY_KEEPORDER, CArray_DESCR(a), 0);

    it1 = CArray_NewIter(a);
    it2 = CArray_NewIter(rtn);

    if (decimals >= 0) {
        for (i = 0; i <= decimals; i++) {
            multiplier = multiplier * 10;
        }
    } else {
        negative_decimals = 1;
        for (i = 0; i > decimals; i--) {
            multiplier = multiplier * 10;
        }
    }

    switch(CArray_TYPE(rtn)) {
        case TYPE_DOUBLE_INT:
            if (negative_decimals) {
                do {
                    *IT_DDATA(it2) = ceil(*IT_DDATA(it1) / multiplier) * multiplier;
                    CArrayIterator_NEXT(it1);
                    CArrayIterator_NEXT(it2);
                } while (CArrayIterator_NOTDONE(it1));
            } else {
                do {
                    if ((*IT_DDATA(it1) - (int)*IT_DDATA(it1)) >= 0.5) {
                        *IT_DDATA(it2) = ceil(pow(10,decimals)* *IT_DDATA(it1))/pow(10,decimals);
                    } else {
                        *IT_DDATA(it2) = floor(pow(10,decimals)* *IT_DDATA(it1))/pow(10,decimals);
                    }

                    CArrayIterator_NEXT(it1);
                    CArrayIterator_NEXT(it2);
                } while (CArrayIterator_NOTDONE(it1));
            }
            break;
        case TYPE_INTEGER_INT:
            if (negative_decimals) {
                do {
                    *IT_IDATA(it2) = (int)(ceil(((double)(*IT_IDATA(it1))/multiplier)) * multiplier);
                    CArrayIterator_NEXT(it1);
                    CArrayIterator_NEXT(it2);
                } while (CArrayIterator_NOTDONE(it1));
            } else {
                do {
                    *IT_IDATA(it2) = *IT_IDATA(it1);
                    CArrayIterator_NEXT(it1);
                    CArrayIterator_NEXT(it2);
                } while (CArrayIterator_NOTDONE(it1));
            }
            break;
        default:
            throw_notimplemented_exception();
            goto fail;
    }

    if (out != NULL) {
        add_to_buffer(out, rtn, sizeof(CArray));
    }

    CArrayIterator_FREE(it1);
    CArrayIterator_FREE(it2);
    return rtn;
fail:
    CArrayIterator_FREE(it1);
    CArrayIterator_FREE(it2);
    return NULL;
}

