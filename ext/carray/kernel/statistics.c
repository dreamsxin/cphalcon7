#include "statistics.h"
#include "carray.h"
#include "convert_type.h"
#include "alloc.h"
#include "buffer.h"
#include "common/common.h"
#include "arraytypes.h"

/*
 * Implementation which is common between CArray_Correlate
 * and CArray_Correlate2.
 *
 * inverted is set to 1 if computed correlate(ap2, ap1), 0 otherwise
 */
static CArray*
_carray_correlate(CArray *ap1, CArray *ap2, int typenum,
                   int mode, int *inverted)
{
    CArray *ret;
    int length;
    int i, n1, n2, n, n_left, n_right;
    int is1, is2, os;
    char *ip1, *ip2, *op;
    CArray_DotFunc *dot;

    n1 = CArray_DIMS(ap1)[0];
    n2 = CArray_DIMS(ap2)[0];
    if (n1 < n2) {
        ret = ap1;
        ap1 = ap2;
        ap2 = ret;
        ret = NULL;
        i = n1;
        n1 = n2;
        n2 = i;
        *inverted = 1;
    } else {
        *inverted = 0;
    }

    length = n1;
    n = n2;
    switch(mode) {
        case 0:
            length = length - n + 1;
            n_left = n_right = 0;
            break;
        case 1:
            n_left = (int)(n/2);
            n_right = n - n_left - 1;
            break;
        case 2:
            n_right = n - 1;
            n_left = n - 1;
            length = length + n - 1;
            break;
        default:
            throw_valueerror_exception("mode must be 0, 1, or 2");
            return NULL;
    }

    /*
     * Need to choose an output array that can hold a sum
     * -- use priority to determine which subtype.
     */
    ret = new_array_for_sum(ap1, ap2, NULL, 1, &length, typenum, NULL);
    if (ret == NULL) {
        return NULL;
    }
    dot = CArray_DESCR(ret)->f->dotfunc;
    if (dot == NULL) {
        throw_valueerror_exception("function not available for this data type");
        goto clean_ret;
    }

    is1 = CArray_STRIDES(ap1)[0];
    is2 = CArray_STRIDES(ap2)[0];
    op = CArray_DATA(ret);
    os = CArray_DESCR(ret)->elsize;
    ip1 = CArray_DATA(ap1);
    ip2 = CArray_BYTES(ap2) + n_left*is2;
    n = n - n_left;
    for (i = 0; i < n_left; i++) {
        dot(ip1, is1, ip2, is2, op, n);
        n++;
        ip2 -= is2;
        op += os;
    }
    if (small_correlate(ip1, is1, n1 - n2 + 1, CArray_TYPE(ap1),
                        ip2, is2, n, CArray_TYPE(ap2),
                        op, os)) {
        ip1 += is1 * (n1 - n2 + 1);
        op += os * (n1 - n2 + 1);
    }
    else {
        for (i = 0; i < (n1 - n2 + 1); i++) {
            dot(ip1, is1, ip2, is2, op, n);
            ip1 += is1;
            op += os;
        }
    }
    for (i = 0; i < n_right; i++) {
        n--;
        dot(ip1, is1, ip2, is2, op, n);
        ip1 += is1;
        op += os;
    }

    return ret;

clean_ret:
    CArray_DECREF(ret);
    return NULL;
}


CArray *
CArray_Correlate(CArray * op1, CArray * op2, int mode, MemoryPointer * out)
{
    CArray *ap1, *ap2, *ret = NULL;
    int typenum;
    int unused;
    CArrayDescriptor *typec;

    typenum = CArray_ObjectType(op1, 0);
    typenum = CArray_ObjectType(op2, typenum);

    typec = CArray_DescrFromType(typenum);
    CArrayDescriptor_INCREF(typec);

    ap1 = CArray_FromAny(op1, typec, 1, 1, CARRAY_ARRAY_DEFAULT);
    if (ap1 == NULL) {
        CArrayDescriptor_DECREF(typec);
        return NULL;
    }
    ap2 = CArray_FromAny(op2, typec, 1, 1, CARRAY_ARRAY_DEFAULT);
    if (ap2 == NULL) {
        goto fail;
    }

    ret = _carray_correlate(ap1, ap2, typenum, mode, &unused);
    if (ret == NULL) {
        goto fail;
    }

    if (out != NULL) {
        add_to_buffer(out, ret, sizeof(CArray));
    }

    CArrayDescriptor_FREE(typec);
    return ret;

fail:
    CArray_DECREF(ap1);
    CArray_DECREF(ap2);
    CArray_DECREF(ret);
    return NULL;
}


CArray *
CArray_Correlate2(CArray * op1, CArray * op2, int mode, MemoryPointer * out)
{
    CArray *ap1, *ap2, *ret = NULL;
    int typenum;
    CArrayDescriptor *typec;
    int inverted;
    int st;

    typenum = CArray_ObjectType(op1, 0);
    typenum = CArray_ObjectType(op2, typenum);

    typec = CArray_DescrFromType(typenum);
    CArrayDescriptor_INCREF(typec);

    ap1 = CArray_FromAny(op1, typec, 1, 1, CARRAY_ARRAY_DEFAULT);
    if (ap1 == NULL) {
        CArrayDescriptor_DECREF(typec);
        CArrayDescriptor_FREE(typec);
        return NULL;
    }
    ap2 = CArray_FromAny(op2, typec, 1, 1, CARRAY_ARRAY_DEFAULT);
    if (ap2 == NULL) {
        goto clean_ap1;
    }

    ret = _carray_correlate(ap1, ap2, typenum, mode, &inverted);
    if (ret == NULL) {
        goto clean_ap2;
    }

    /*
     * If we inverted input orders, we need to reverse the output array (i.e.
     * ret = ret[::-1])
     */
    if (inverted) {
        //st = _carray_revert(ret);
        if (st) {
            goto clean_ret;
        }
    }

    if (out != NULL) {
        add_to_buffer(out, ret, sizeof(CArray));
    }

    CArrayDescriptor_DECREF(typec);

    if (ap1->data != op1->data) {
        CArray_Free(ap1);
    }

    if (ap2->data != op2->data) {
        CArray_Free(ap2);
    }

    return ret;
clean_ret:
    CArray_DECREF(ret);
clean_ap2:
    CArray_DECREF(ap2);
clean_ap1:
    CArray_DECREF(ap1);
    return NULL;
}