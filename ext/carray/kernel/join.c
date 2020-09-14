#include "join.h"
#include "carray.h"
#include "alloc.h"
#include "stdio.h"
#include "shape.h"
#include "buffer.h"

static CArray *
_swap_and_concat(CArray **op, int axis, int n, MemoryPointer * out)
{
    CArray **newtup = NULL;
    CArray *otmp, *arr;
    int i;
    int axis2 = 0;

    newtup = emalloc(sizeof(CArray) * n);
    if (newtup==NULL) return NULL;
    for (i=0; i<n; i++) {
        otmp = op[i];
        arr = CArray_FromCArray(otmp, CArray_DESCR(otmp), CArray_FLAGS(otmp));
        CArray_DECREF(otmp);
        if (arr==NULL) goto fail;
        otmp = CArray_SwapAxes(arr, axis, 0, NULL);
        CArray_DECREF(arr);
        if (otmp == NULL) goto fail;
        newtup[i] = otmp;
    }
    otmp = CArray_Concatenate(newtup, n, &axis2, out);
    if (otmp == NULL) return NULL;
    arr = CArray_SwapAxes(otmp, axis, 0, NULL);
    CArray_DECREF(otmp);
    return arr;
fail:
    return NULL;
}

CArray *
CArray_Concatenate(CArray ** target, int narrays, int * axis, MemoryPointer * out)
{
    CArray *ret;
    CArray *otmp, **mps;
    int i, n, tmp, nd=0, new_dim;
    char *data;
    //PyTypeObject *subtype;
    double prior1, prior2;
    int numbytes;

    n = narrays;
    if (n == -1) {
        return NULL;
    }
    if (n == 0) {
        throw_valueerror_exception("concatenation of zero-length sequences is "
                                   "impossible");
        return NULL;
    }

    if ((*axis < 0) || ((0 < *axis) && (*axis < CARRAY_MAXDIMS))) {
        return _swap_and_concat(target, *axis, n, out);
    }

    mps = target;
    if (mps == NULL) return NULL;

    /* Make sure these arrays are legal to concatenate. */
    /* Must have same dimensions except d0 */

    prior1 = CArray_PRIORITY;

    ret = emalloc(sizeof(CArray));
    for(i=0; i<n; i++) {
        if (*axis >= CARRAY_MAXDIMS) {
            otmp = CArray_Ravel(mps[i],0);
            CArray_DECREF(mps[i]);
            mps[i] = otmp;
        }
        prior2 = 0.0;
        if (prior2 > prior1) {
            prior1 = prior2;
        }
    }

    new_dim = 0;
    for(i=0; i<n; i++) {
        if (mps[i] == NULL) goto fail;
        if (i == 0) nd = mps[i]->ndim;
        else {
            if (nd != mps[i]->ndim) {
                throw_valueerror_exception("arrays must have same "
                                           "number of dimensions");
                goto fail;
            }
            if (!CArray_CompareLists(mps[0]->dimensions+1,
                                      mps[i]->dimensions+1,
                                      nd-1)) {
                throw_valueerror_exception("array dimensions must "
                                           "agree except for d_0");
                goto fail;
            }
        }
        if (nd == 0) {
            throw_valueerror_exception("0-d arrays can't be concatenated");
            goto fail;
        }
        new_dim += mps[i]->dimensions[0];
    }

    tmp = mps[0]->dimensions[0];
    mps[0]->dimensions[0] = new_dim;
    CArrayDescriptor_INCREF(CArray_DESCR(mps[0]));

    ret = (CArray *)CArray_NewFromDescr(ret, CArray_DESCR(mps[0]), nd,
                                        CArray_DIMS(mps[0]),NULL, NULL, 0,
                                        NULL);
    mps[0]->dimensions[0] = tmp;

    if (ret == NULL) goto fail;

    data = ret->data;
    for(i=0; i<n; i++) {
        numbytes = CArray_NBYTES(mps[i]);
        memcpy(data, mps[i]->data, numbytes);
        data += numbytes;
    }

    CArray_INCREF(ret);
    for(i=0; i<n; i++) {
        CArray_DECREF(mps[i]);
    }

    if (out != NULL) {
        add_to_buffer(out, ret, sizeof(CArray));
    }

    return ret;

fail:
    CArray_DECREF(ret);
    for(i=0; i<n; i++) {
        CArray_DECREF(mps[i]);
    }
    return NULL;
}