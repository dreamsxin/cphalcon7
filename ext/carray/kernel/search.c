#include "search.h"
#include "carray.h"
#include "shape.h"
#include "alloc.h"
#include "ctors.h"
#include "buffer.h"

#define _LESS_THAN_OR_EQUAL(a,b) ((a) <= (b))

int
INT_argmax(int *ip, int n, int *max_ind, CArray *CARRAY_UNUSED(aip))
{
    int i;
    int mp = *ip;
    *max_ind = 0;

    if ((mp) != (mp)) {
        /* nan encountered; it's maximal */
        return 0;
    }

    for (i = 1; i < n; i++) {
        ip++;
        /*
         * Propagate nans, similarly as max() and min()
         */
        if (!_LESS_THAN_OR_EQUAL(*ip, mp)) {  /* negated, for correct nan handling */
            mp = *ip;
            *max_ind = i;
            if ((mp) != (mp)) {
                /* nan encountered, it's maximal */
                break;
            }
        }
    }
    return 0;
}

int
DOUBLE_argmax(double *ip, int n, int *max_ind, CArray *CARRAY_UNUSED(aip))
{
    int i;
    double mp = *ip;
    *max_ind = 0;

    if (isnan(mp)) {
        /* nan encountered; it's maximal */
        return 0;
    }

    for (i = 1; i < n; i++) {
        ip++;
        /*
         * Propagate nans, similarly as max() and min()
         */
        if (!_LESS_THAN_OR_EQUAL(*ip, mp)) {  /* negated, for correct nan handling */
            mp = *ip;
            *max_ind = i;
            if (isnan(mp)) {
                /* nan encountered, it's maximal */
                break;
            }
        }
    }
    return 0;
}

int
DOUBLE_argmin(double *ip, int n, int *min_ind, CArray *CARRAY_UNUSED(aip))
{
    int i;
    double mp = *ip;
    *min_ind = 0;

    if (isnan(mp)) {
        /* nan encountered; it's maximal */
        return 0;
    }

    for (i = 1; i < n; i++) {
        ip++;
        /*
         * Propagate nans, similarly as max() and min()
         */
        if (!_LESS_THAN_OR_EQUAL(mp, *ip)) {  /* negated, for correct nan handling */
            mp = *ip;
            *min_ind = i;
            if (isnan(mp)) {
                /* nan encountered, it's maximal */
                break;
            }
        }
    }
    return 0;
}

int
INT_argmin(int *ip, int n, int *min_ind, CArray *CARRAY_UNUSED(aip))
{
    int i;
    double mp = *ip;
    *min_ind = 0;

    if ((mp) != (mp)) {
        /* nan encountered; it's maximal */
        return 0;
    }

    for (i = 1; i < n; i++) {
        ip++;
        /*
         * Propagate nans, similarly as max() and min()
         */
        if (!_LESS_THAN_OR_EQUAL(mp, *ip)) {  /* negated, for correct nan handling */
            mp = *ip;
            *min_ind = i;
            if ((mp) != (mp)) {
                /* nan encountered, it's maximal */
                break;
            }
        }
    }
    return 0;
}

CArray *
CArray_Argmax(CArray * target, int * axis, MemoryPointer * out)
{
    CArray *ap = NULL, *rp = NULL;
    CArray_ArgFunc* arg_func;
    char *ip;
    int *rptr;
    int i, n, m;
    int elsize;

    if ((ap = (CArray *)CArray_CheckAxis(target, axis, 0)) == NULL) {
        return NULL;
    }

    /*
     * We need to permute the array so that axis is placed at the end.
     * And all other dimensions are shifted left.
     */
    if (*axis != CArray_NDIM(ap)-1) {
        CArray_Dims newaxes;
        int dims[CARRAY_MAXDIMS];
        int j;

        newaxes.ptr = dims;
        newaxes.len = CArray_NDIM(ap);
        for (j = 0; j < *axis; j++) {
            dims[j] = j;
        }
        for (j = *axis; j < CArray_NDIM(ap) - 1; j++) {
            dims[j] = j + 1;
        }
        dims[CArray_NDIM(ap) - 1] = *axis;
        CArray_DECREF(target);
        target = CArray_Transpose(ap, &newaxes, NULL);

        CArray_DECREF(ap);
        if (target == NULL) {
            return NULL;
        }
    }
    else {
        target = ap;
        CArray_DECREF(ap);
    }

    /* Will get native-byte order contiguous copy. */
    CArrayDescriptor * tmp_descr = CArray_DescrFromType(CArray_DESCR(target)->type_num);
    ap = CArray_ContiguousFromAny(target, tmp_descr, 1, 0);

    CArray_DECREF(target);
    if (ap == NULL) {
        return NULL;
    }
    arg_func = CArray_DESCR(ap)->f->argmax;
    if (arg_func == NULL) {
        throw_typeerror_exception("data type not ordered");
        goto fail;
    }
    elsize = CArray_DESCR(ap)->elsize;
    m = CArray_DIMS(ap)[CArray_NDIM(ap)-1];
    if (m == 0) {
        throw_valueerror_exception("attempt to get argmax of an empty sequence");
        goto fail;
    }

    rp = emalloc(sizeof(CArray));
    rp = CArray_New(rp, CArray_NDIM(ap)-1, CArray_DIMS(ap),TYPE_INTEGER_INT, NULL, NULL, 0, 0, ap);

    if (rp == NULL) {
        goto fail;
    }

    n = CArray_SIZE(ap)/m;
    rptr = (int *)CArray_DATA(rp);

    for (ip = CArray_DATA(ap), i = 0; i < n; i++, ip += elsize*m) {
        arg_func(ip, m, rptr, ap);
        rptr += 1;
    }

    if (out != NULL) {
        add_to_buffer(out, rp, sizeof(CArray));
    }

    if (*axis != CArray_NDIM(ap)-1) {
        CArray_DECREF(ap);
        CArrayDescriptor_DECREF(CArray_DESCR(target));
        CArray_Free(ap);
    } else {
        CArrayDescriptor_FREE(tmp_descr);
    }

    return rp;
fail:
    CArray_DECREF(ap);
    //CArray_DECREF(rp);
    return NULL;
}

CArray *
CArray_Argmin(CArray * target, int * axis, MemoryPointer * out)
{
    CArray *ap = NULL, *rp = NULL;
    CArray_ArgFunc* arg_func;
    char *ip;
    int *rptr;
    int i, n, m;
    int elsize;

    if ((ap = (CArray *)CArray_CheckAxis(target, axis, 0)) == NULL) {
        return NULL;
    }

    /*
     * We need to permute the array so that axis is placed at the end.
     * And all other dimensions are shifted left.
     */
    if (*axis != CArray_NDIM(ap)-1) {
        CArray_Dims newaxes;
        int dims[CARRAY_MAXDIMS];
        int j;

        newaxes.ptr = dims;
        newaxes.len = CArray_NDIM(ap);
        for (j = 0; j < *axis; j++) {
            dims[j] = j;
        }
        for (j = *axis; j < CArray_NDIM(ap) - 1; j++) {
            dims[j] = j + 1;
        }
        dims[CArray_NDIM(ap) - 1] = *axis;
        CArray_DECREF(target);
        target = CArray_Transpose(ap, &newaxes, NULL);

        CArray_DECREF(ap);
        if (target == NULL) {
            return NULL;
        }
    }
    else {
        target = ap;
        CArray_DECREF(ap);
    }

    /* Will get native-byte order contiguous copy. */
    CArrayDescriptor * tmp_descr = CArray_DescrFromType(CArray_DESCR(target)->type_num);
    ap = CArray_ContiguousFromAny(target, tmp_descr, 1, 0);

    CArray_DECREF(target);
    if (ap == NULL) {
        return NULL;
    }
    arg_func = CArray_DESCR(ap)->f->argmin;
    if (arg_func == NULL) {
        throw_typeerror_exception("data type not ordered");
        goto fail;
    }
    elsize = CArray_DESCR(ap)->elsize;
    m = CArray_DIMS(ap)[CArray_NDIM(ap)-1];
    if (m == 0) {
        throw_valueerror_exception("attempt to get argmax of an empty sequence");
        goto fail;
    }

    rp = emalloc(sizeof(CArray));
    rp = CArray_New(rp, CArray_NDIM(ap)-1, CArray_DIMS(ap),TYPE_INTEGER_INT, NULL, NULL, 0, 0, ap);

    if (rp == NULL) {
        goto fail;
    }

    n = CArray_SIZE(ap)/m;
    rptr = (int *)CArray_DATA(rp);

    for (ip = CArray_DATA(ap), i = 0; i < n; i++, ip += elsize*m) {
        arg_func(ip, m, rptr, ap);
        rptr += 1;
    }

    if (out != NULL) {
        add_to_buffer(out, rp, sizeof(CArray));
    }

    if (*axis != CArray_NDIM(ap)-1) {
        CArray_DECREF(ap);
        CArrayDescriptor_DECREF(CArray_DESCR(target));
        CArray_Free(ap);
    } else {
        CArrayDescriptor_FREE(tmp_descr);
    }

    return rp;
    fail:
    CArray_DECREF(ap);
    //CArray_DECREF(rp);
    return NULL;
}