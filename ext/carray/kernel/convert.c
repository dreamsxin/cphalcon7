#include "convert.h"
#include "carray.h"
#include "alloc.h"
#include "buffer.h"
#include "common/exceptions.h"
#include "scalar.h"
#include "assign_scalar.h"
#include "zend_alloc.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-function-declaration"
/**
 * Slice CArray
 * 
 * @todo Handle Exceptions (invalid index, etc)
 **/
CArray *
CArray_Slice_Index(CArray * self, int index, MemoryPointer * out)
{
    CArray * ret = NULL;
    CArrayDescriptor * subarray_descr;
    int * new_dimensions, * new_strides;
    int new_num_elements = 0;
    int nd, i, flags;
    ret = (CArray *)ecalloc(1, sizeof(CArray));
   
    subarray_descr = (CArrayDescriptor *)ecalloc(1, sizeof(CArrayDescriptor));

    nd = CArray_NDIM(self) - 1;
    new_dimensions = (int*)emalloc(nd * sizeof(int));
    
    for(i = 1; i < CArray_NDIM(self); i++) {
        new_dimensions[i-1] = self->dimensions[i];
    }
    subarray_descr->elsize = CArray_DESCR(self)->elsize;
    subarray_descr->type = CArray_DESCR(self)->type;
    subarray_descr->type_num = CArray_DESCR(self)->type_num;
    subarray_descr->alignment = 0;

    new_strides  = (int*)emalloc(nd * sizeof(int));
    for(i = 1; i < CArray_NDIM(self); i++) {
        new_strides[i-1] = self->strides[i];
    }
    
    new_num_elements = self->dimensions[nd];
    
    for(i = nd-1; i > 0; i--) {
        new_num_elements = new_num_elements * CArray_DIMS(self)[i];
    }
    subarray_descr->numElements = new_num_elements;
    ret->descriptor = subarray_descr;
    
    flags = CArray_FLAGS(self);
   
    ret = (CArray *)CArray_NewFromDescr_int(
            ret, subarray_descr,
            nd, new_dimensions, new_strides,
            (CArray_DATA(self) + (index * self->strides[0])),
            flags, self,
            0, 1);

    if (out != NULL) {
        add_to_buffer(out, ret, sizeof(*ret));
    }

    efree(new_dimensions);
    efree(new_strides);
    return ret;        
}

/**
 * @param self
 * @param target
 * @return
 */
CArray *
CArray_View(CArray *self)
{
    CArray *ret = emalloc(sizeof(CArray));
    CArrayDescriptor *dtype;
    CArray *subtype;
    int flags;

    dtype = CArray_DESCR(self);

    flags = CArray_FLAGS(self);

    ret = (CArray *)CArray_NewFromDescr_int(
            ret, dtype,
            CArray_NDIM(self), CArray_DIMS(self), CArray_STRIDES(self),
            CArray_DATA(self),
            flags, self,
            0, 1);
            
    return ret;
}

/**
 * @param obj
 * @param order
 * @return
 */
CArray *
CArray_NewCopy(CArray *obj, CARRAY_ORDER order)
{
    CArray * ret;
    ret = (CArray *)CArray_NewLikeArray(obj, order, NULL, 1);

    return ret;
}

int
CArray_CanCastSafely(int fromtype, int totype)
{
    CArrayDescriptor *from, *to;
    int felsize, telsize;

    if (fromtype == totype) {
        return 1;
    }
    if (fromtype == TYPE_BOOL_INT) {
        return 1;
    }
    if (totype == TYPE_BOOL_INT) {
        return 0;
    }

    from = CArray_DescrFromType(fromtype);

    if (from->f->cancastto) {

        int *curtype = from->f->cancastto;
        while (*curtype != CARRAY_NTYPES) {
            if (*curtype++ == totype) {
                return 1;
            }
        }
    }

    return 0;
}

int
CArray_CanCastTo(CArrayDescriptor *from, CArrayDescriptor *to)
{
    int fromtype = from->type_num;
    int totype = to->type_num;
    int ret;

    ret = CArray_CanCastSafely(fromtype, totype);

    if (ret) {
        /* Check String and Unicode more closely */
        if (fromtype == TYPE_STRING_INT) {
            if (totype == TYPE_STRING_INT) {
                ret = (from->elsize <= to->elsize);
            }
        }
    }
    return ret;
}

CArray_VectorUnaryFunc *
CArray_GetCastFunc(CArrayDescriptor *descr, int type_num)
{
    CArray_VectorUnaryFunc *castfunc = NULL;

    castfunc = descr->f->cast[type_num];
    
    if (NULL == castfunc) {
        throw_valueerror_exception("No cast function available.");
        return NULL;
    }
    return castfunc;
}

int
CArray_CastTo(CArray *out, CArray *mp)
{
    int simple;
    int same;
    CArray_VectorUnaryFunc *castfunc = NULL;
    int mpsize = CArray_SIZE(mp);
    int iswap, oswap;

    if (mpsize == 0) {
        return 0;
    }
    if (!CArray_ISWRITEABLE(out)) {
        throw_valueerror_exception("output array is not writeable");
        return -1;
    }

    castfunc = CArray_GetCastFunc(CArray_DESCR(mp), CArray_DESCR(out)->type_num);
    if (castfunc == NULL) {
        return -1;
    }

    same = CArray_SAMESHAPE(out, mp);
    simple = same && ((CArray_ISCARRAY_RO(mp) && CArray_ISCARRAY(out)) ||
                      (CArray_ISFARRAY_RO(mp) && CArray_ISFARRAY(out)));

    if (simple) {
        castfunc(mp->data, out->data, mpsize, mp, out);
        return 0;
    }         
}

int
CArray_FillWithScalar(CArray * arr, CArrayScalar * sc)
{
    CArrayDescriptor * dtype = NULL;
    long long value_buffer[4];
    void * value = NULL;
    int retcode = 0;
    
    dtype = CArray_DescrFromScalar(sc);
    value = scalar_value(sc, dtype);
    if (value == NULL) {
        CArrayDescriptor_FREE(dtype);
        return -1;
    }

    if (value != NULL) {
        retcode = CArray_AssignRawScalar(arr, dtype, value, NULL, CARRAY_UNSAFE_CASTING);
        
        CArrayDescriptor_FREE(dtype);
        return retcode;
    }

    throw_notimplemented_exception();
}

static
void
remove_spaces(char* s) {
    const char* d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while (*s++ = *d++);
}

static int *
_dimensions_from_slice_str(CArray * self, char * index, int * ndim, int *stride_step, int * strides)
{
    int * dimensions, * ignored_dimensions;
    int sum_dimension, ignore_dim = 0;
    int current_dim = 0, num_args = 0;
    char * indice, * start_stop_step, *end_indice, *end_sss;
    int len;
    int indices_count = 1, i;
    int * start = NULL, * stop = NULL, * step = NULL;

    len = strlen(index);
    remove_spaces(index);

    for (i = 0; i < len; i++) {
        if (index[i] == ',') {
            indices_count++;
        }
    }

    dimensions = emalloc(sizeof(int) * CArray_NDIM(self));
    ignored_dimensions = ecalloc(CArray_NDIM(self), sizeof(int));

    if (indices_count > CArray_NDIM(self)) {
        throw_indexerror_exception("too many indices for array");
        return NULL;
    }

    *ndim = 0;

    indice = strtok_r(index, ",", &end_indice);
    while(indice != NULL) {
        sum_dimension = 0;
        if (strstr(indice, ":") == NULL) {
            ignore_dim = 1;
            ignored_dimensions[current_dim] = 1;
        } else {
            if (indice[0] == ':') {
                num_args = 1;
                if (strlen(indice) > 1) {
                    if (indice[1] == ':') {
                        num_args = 2;
                    }
                }
            }

            start_stop_step = strtok_r(indice, ":", &end_sss);
            while (start_stop_step != NULL) {

                num_args++;
                if (num_args == 1) {
                    start = emalloc(sizeof(int));
                    *start = atoi(start_stop_step);
                }
                if (num_args == 2) {
                    stop = emalloc(sizeof(int));
                    *stop = atoi(start_stop_step);
                }
                if (num_args == 3) {
                    step = emalloc(sizeof(int));
                    *step = atoi(start_stop_step);
                }
                start_stop_step = strtok_r(NULL, ":", &end_sss);
            }

        }

        if (num_args == 0) {
            start = emalloc(sizeof(int));
            *start = 0;
        }

        if (start != NULL) {
            sum_dimension = *start;
            *stride_step = *stride_step + (*start * CArray_STRIDES(self)[current_dim]);
            efree(start);
            start = NULL;
        } else {
            sum_dimension = 0;
        }

        if (stop != NULL) {
            sum_dimension = sum_dimension + *stop;
            efree(stop);
            stop = NULL;
        } else {
            sum_dimension = CArray_DIMS(self)[current_dim] - sum_dimension;
        }

        if (step != NULL) {
            sum_dimension = (int)ceil(sum_dimension / *step);
            efree(step);
            step = NULL;
        }

        if (sum_dimension < 0) {
            sum_dimension = 0;
        }

        if (!ignore_dim) {
            dimensions[*ndim] = sum_dimension;
            *ndim += 1;
        } else {
            ignore_dim = 0;
            *stride_step = *stride_step + (atoi(indice) * CArray_STRIDES(self)[current_dim]);
        }

        current_dim++;
        num_args = 0;
        indice = strtok_r(NULL, ",", &end_indice);
    }

    int current_index = 0;
    for (i = indices_count; i < CArray_NDIM(self); i++) {
        if(!ignored_dimensions[i]) {
            dimensions[*ndim] = CArray_DIMS(self)[i];
            *ndim += 1;
        }
    }


    current_index = 0;
    for (i = 0; i < CArray_NDIM(self); i++) {
        if(!ignored_dimensions[i]) {
            strides[current_index] = CArray_STRIDES(self)[i];
            current_index++;
        }
    }

    efree(ignored_dimensions);
    return dimensions;
}

CArray *
CArray_Slice_Str(CArray *self, char *index, MemoryPointer *out)
{
    CArray *rtn = emalloc(sizeof(CArray));
    int *strides = emalloc(sizeof(int) * CArray_NDIM(self));
    int i, ndim;
    int step = 0;
    int *dimensions = _dimensions_from_slice_str(self, index, &ndim, &step, strides);
    CArrayDescriptor *newDescr;

    if (dimensions == NULL) {
        return NULL;
    }

    if (strides == NULL) {
        return NULL;
    }

    newDescr = CArray_DescrNew(CArray_DESCR(self));
    rtn = CArray_NewFromDescr_int(rtn, newDescr, ndim, dimensions, strides, CArray_DATA(self),
                                  0, self, 0, 0);

    rtn->flags = (CARRAY_ARRAY_C_CONTIGUOUS | CARRAY_ARRAY_WRITEABLE | CARRAY_ARRAY_ALIGNED);

    rtn->data += step;

    if (out != NULL) {
        add_to_buffer(out, rtn, sizeof(CArray));
    }

    CArrayDescriptor_INCREF(CArray_DESCR(rtn));
    efree(strides);
    efree(dimensions);
    return rtn;
}

#pragma clang diagnostic pop