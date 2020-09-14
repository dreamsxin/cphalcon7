#include "clip.h"
#include "carray.h"
#include "calculation.h"
#include "alloc.h"
#include "ctors.h"
#include "buffer.h"

#define _CARRAY_DOUBLE_MIN(a, b) (isnan(a) ? (a) : CArray_MIN(a, b))
#define _CARRAY_DOUBLE_MAX(a, b) (isnan(a) ? (a) : CArray_MAX(a, b))
#define _CARRAY_DCLIP(x, min, max) \
        _CARRAY_DOUBLE_MIN(_CARRAY_DOUBLE_MAX((x), (min)), (max))


#define _CARRAY_INT_MIN(a, b) CArray_MIN(a, b)
#define _CARRAY_INT_MAX(a, b) CArray_MAX(a, b)
#define _CARRAY_ICLIP(x, min, max) \
        _CARRAY_INT_MIN(_CARRAY_INT_MAX((x), (min)), (max))


void
INT_clip(int *in, int ni, int *min, int *max, int *out)
{
    int i;
    int max_val = 0, min_val = 0;

    if (max != NULL) {
        max_val = *max;
    }
    if (min != NULL) {
        min_val = *min;
    }

    if (max == NULL) {
        for (i = 0; i < ni; i++) {
            if (in[i] < min_val) {
                out[i] = min_val;
            }
            else {
                out[i] = in[i];
            }
        }
    }
    else if (min == NULL) {
        for (i = 0; i < ni; i++) {
            if (in[i] > max_val) {
                out[i] = max_val;
            }
            else {
                out[i] = in[i];
            }
        }
    }
    else {
        /*
         * Visual Studio 2015 loop vectorizer handles NaN in an unexpected
         * manner, see: https://github.com/numpy/numpy/issues/7601
         */
        #if (_MSC_VER == 1900)
        #pragma loop( no_vector )
        #endif
        for (i = 0; i < ni; i++) {
            if (in[i] < min_val) {
                out[i]   = min_val;
            }
            else if (in[i] > max_val) {
                out[i]   = max_val;
            }
            else {
                out[i] = in[i];
            }
        }
    }
}

void
DOUBLE_clip(double *in, int ni, double *min, double *max, double *out)
{
    int i;
    double max_val = 0, min_val = 0;

    if (max != NULL) {
        max_val = *max;
    }
    if (min != NULL) {
        min_val = *min;
    }

    if (max == NULL) {
        for (i = 0; i < ni; i++) {
            if (in[i] < min_val) {
                out[i] = min_val;
            }
            else {
                out[i] = in[i];
            }
        }
    }
    else if (min == NULL) {
        for (i = 0; i < ni; i++) {
            if (in[i] > max_val) {
                out[i] = max_val;
            }
            else {
                out[i] = in[i];
            }
        }
    }
    else {
        /*
         * Visual Studio 2015 loop vectorizer handles NaN in an unexpected
         * manner, see: https://github.com/numpy/numpy/issues/7601
         */
#if (_MSC_VER == 1900)
#pragma loop( no_vector )
#endif
        for (i = 0; i < ni; i++) {
            if (in[i] < min_val) {
                out[i]   = min_val;
            }
            else if (in[i] > max_val) {
                out[i]   = max_val;
            }
            else {
                out[i] = in[i];
            }
        }
    }
}

CArray *
CArray_Clip(CArray * self, CArray * min, CArray * max, MemoryPointer * out_ptr)
{
    CArray_FastClipFunc *func;
    int outgood = 0, ingood = 0;
    CArray *maxa = NULL;
    CArray *mina = NULL;
    CArray *newout = NULL, *out = NULL, *newin = NULL;
    CArrayDescriptor *indescr = NULL, *newdescr = NULL;
    char *max_data, *min_data;
    CArray *zero;

    if ((max == NULL) && (min == NULL)) {
        throw_valueerror_exception("array_clip: must set either max or min");
        return NULL;
    }

    func = CArray_DESCR(self)->f->fastclip;

    /* First we need to figure out the correct type */
    if (min != NULL) {
        indescr = CArray_DESCR(min);
        if (indescr == NULL) {
            goto fail;
        }
    }
    if (max != NULL) {
        newdescr = CArray_DESCR(max);
        indescr = NULL;
        if (newdescr == NULL) {
            goto fail;
        }
    }
    else {
        /* Steal the reference */
        newdescr = indescr;
        indescr = NULL;
    }

    /*
     * Use the scalar descriptor only if it is of a bigger
     * KIND than the input array (and then find the
     * type that matches both).
     */
    if (newdescr->type_num > CArray_DESCR(self)->type_num, NULL) {
        indescr = CArray_DescrFromType(newdescr->type_num);

        if (indescr == NULL) {
            goto fail;
        }
        func = indescr->f->fastclip;
    }
    else {
        indescr = CArray_DESCR(self);
        CArrayDescriptor_INCREF(indescr);
    }
    newdescr = NULL;


    if (CArray_ISONESEGMENT(self) &&
        CArray_CHKFLAGS(self, CARRAY_ARRAY_ALIGNED) &&
        CArray_ISNOTSWAPPED(self) &&
        (CArray_DESCR(self) == indescr)) {
        ingood = 1;
    }

    if (!ingood) {
        int flags;

        if (CArray_ISFORTRAN(self)) {
            flags = CARRAY_ARRAY_FARRAY;
        }
        else {
            flags = CARRAY_ARRAY_CARRAY;
        }

        newin = CArray_FromArray(self, indescr, flags);

        if (newin == NULL) {
            goto fail;
        }
    }
    else {
        newin = self;
        CArray_INCREF(newin);
    }


    /*
     * If we have already made a copy of the data, then use
     * that as the output array
     */
    if (out == NULL && !ingood) {
        out = newin;
    }

    if (out == NULL) {

        out = emalloc(sizeof(CArray));
        out = CArray_NewFromDescr(out,
                                  indescr, CArray_NDIM(self),
                                  CArray_DIMS(self),
                                  NULL, NULL,
                                  CArray_ISFORTRAN(self),
                                  NULL);

        if (out == NULL) {
            goto fail;
        }

        outgood = 1;
    }

    if (max != NULL) {
        maxa = CArray_FromAny(max, indescr, 0, 0, CARRAY_ARRAY_DEFAULT);
        if (maxa == NULL) {
            goto fail;
        }
    }

    if (min != NULL) {
        mina = CArray_FromAny(min, indescr, 0, 0, CARRAY_ARRAY_DEFAULT);
        if (mina == NULL) {
            goto fail;
        }
    }

    /* Now we can call the fast-clip function */
    min_data = max_data = NULL;
    if (mina != NULL) {
        min_data = CArray_DATA(mina);
    }
    if (maxa != NULL) {
        max_data = CArray_DATA(maxa);
    }

    func(CArray_DATA(newin), CArray_SIZE(newin), min_data, max_data, CArray_DATA(out));

    if (out_ptr != NULL) {
        add_to_buffer(out_ptr, out, sizeof(CArray));
    }

    CArray_DECREF(self);

    return out;
fail:
    return NULL;
}