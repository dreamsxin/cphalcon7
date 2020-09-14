#include "calculation.h"
#include "carray.h"
#include "iterators.h"
#include "buffer.h"
#include "common/exceptions.h"
#include "alloc.h"

/**
 * CArray Cumulative Sum
 **/ 
CArray *
CArray_CumSum(CArray * self, int * axis, int rtype, MemoryPointer * out_ptr)
{
    int i, j = 0;
    CArray * arr, * ret = NULL;
    CArrayDescriptor * descr;
    ret = (CArray *)emalloc(sizeof(CArray));
    arr = CArray_CheckAxis(self, axis, 0);
    int * ret_dims;

    if (check_and_adjust_axis_msg(axis, CArray_NDIM(self)) < 0) {
        efree(ret);
        return NULL;
    }

    switch (rtype) {
        case TYPE_INTEGER_INT:
            descr = CArray_DescrFromType(TYPE_INTEGER_INT);
            break;
        case TYPE_DOUBLE_INT:
            descr = CArray_DescrFromType(TYPE_DOUBLE_INT);
            break;
        default:
            throw_typeerror_exception("Invalid type for cumsum");
            return NULL;
    }

    if(axis == NULL) {
        CArrayIterator * it = CArray_NewIter(self);
        descr->numElements = CArray_DESCR(self)->numElements;
        ret_dims = emalloc(sizeof(int));
        ret_dims[0] = descr->numElements;
        ret = CArray_NewFromDescr_int(ret, descr, 1, ret_dims, NULL, NULL, 0, NULL, 1, 0);
        if(rtype == TYPE_INTEGER_INT) {
            int accumulator = 0;
            do {
                accumulator += *(IT_IDATA(it));
                IDATA(ret)[it->index] = accumulator;
                CArrayIterator_NEXT(it);
            } while(CArrayIterator_NOTDONE(it));
        }
        if(rtype == TYPE_DOUBLE_INT) {
            double accumulator = 0;
            do {
                accumulator += *(IT_DDATA(it));
                DDATA(ret)[it->index] = accumulator;
                CArrayIterator_NEXT(it);
            } while(CArrayIterator_NOTDONE(it));
        }
        efree(ret_dims);
        CArrayIterator_FREE(it);
    }
    else
    {
        ret = CArray_NewFromDescr_int(ret, descr, CArray_NDIM(self), CArray_DIMS(self), NULL, NULL, 0, NULL, 1, 0);
        descr->numElements = CArray_DESCR(self)->numElements;

        CArrayIterator * it = CArray_IterAllButAxis(self, axis);
        CArrayIterator * retit = CArray_IterAllButAxis(ret, axis);
        if(rtype == TYPE_INTEGER_INT) {
            int * accumulator = ecalloc(CArray_DIMS(self)[*axis], sizeof(int));
            i = 0;
            do {
                for(j = 0; j < self->dimensions[*axis]; j++) {
                    accumulator[i] += ((int*)CArrayIterator_DATA(it))[j * (self->strides[*axis]/self->descriptor->elsize)];
                    IT_IDATA(retit)[j * (ret->strides[*axis]/ret->descriptor->elsize)] = accumulator[i];
                }
                CArrayIterator_NEXT(it);
                CArrayIterator_NEXT(retit);
                i++;
            } while(CArrayIterator_NOTDONE(it));
            efree(accumulator);
        }
        if(rtype == TYPE_DOUBLE_INT) {
            double * accumulator = ecalloc(CArray_DIMS(self)[*axis], sizeof(double));
            i = 0;
            do {
                for(j = 0; j < self->dimensions[*axis]; j++) {
                    accumulator[i] += ((double*)CArrayIterator_DATA(it))[j * (self->strides[*axis]/self->descriptor->elsize)];
                    IT_DDATA(retit)[j * (ret->strides[*axis]/ret->descriptor->elsize)] = accumulator[i];
                }
                CArrayIterator_NEXT(it);
                CArrayIterator_NEXT(retit);
                i++;
            } while(CArrayIterator_NOTDONE(it));
            efree(accumulator);
        }
        CArray_DECREF(self);
        CArrayIterator_FREE(it);
        CArrayIterator_FREE(retit);
    }

    if (out_ptr != NULL) {
        add_to_buffer(out_ptr, ret, sizeof(CArray));
    }

    return ret;
}

/**
 * CArray Cumulative Prod
 **/ 
CArray *
CArray_CumProd(CArray * self, int * axis, int rtype, MemoryPointer * out_ptr)
{
    int i, j = 0;
    CArray * arr, * ret = NULL;
    CArrayDescriptor * descr;
    ret = (CArray *)emalloc(sizeof(CArray));
    arr = CArray_CheckAxis(self, axis, 0);
    int * ret_dims;

    if (check_and_adjust_axis_msg(axis, CArray_NDIM(self)) < 0) {
        efree(ret);
        return NULL;
    }

    switch (rtype) {
        case TYPE_INTEGER_INT:
            descr = CArray_DescrFromType(TYPE_INTEGER_INT);
            break;
        case TYPE_DOUBLE_INT:
            descr = CArray_DescrFromType(TYPE_DOUBLE_INT);
            break;
        default:
            throw_typeerror_exception("Invalid type for cumprod");
            return NULL;
    }

    if(axis == NULL) {
        CArrayIterator * it = CArray_NewIter(self);
        descr->numElements = CArray_DESCR(self)->numElements;
        ret_dims = emalloc(sizeof(int));
        ret_dims[0] = descr->numElements;
        ret = CArray_NewFromDescr_int(ret, descr, 1, ret_dims, NULL, NULL, 0, NULL, 1, 0);
        if(rtype == TYPE_INTEGER_INT) {
            int accumulator = 1;
            do {
                accumulator *= *(IT_IDATA(it));
                IDATA(ret)[it->index] = accumulator;
                CArrayIterator_NEXT(it);
            } while(CArrayIterator_NOTDONE(it));
        }
        if(rtype == TYPE_DOUBLE_INT) {
            double accumulator = 1;
            do {
                accumulator *= *(IT_DDATA(it));
                DDATA(ret)[it->index] = accumulator;
                CArrayIterator_NEXT(it);
            } while(CArrayIterator_NOTDONE(it));
        }
        efree(ret_dims);
        CArrayIterator_FREE(it);
    }
    else
    {
        ret = CArray_NewFromDescr_int(ret, descr, CArray_NDIM(self), CArray_DIMS(self), NULL, NULL, 0, NULL, 1, 0);
        descr->numElements = CArray_DESCR(self)->numElements;

        CArrayIterator * it = CArray_IterAllButAxis(self, axis);
        CArrayIterator * retit = CArray_IterAllButAxis(ret, axis);
        if(rtype == TYPE_INTEGER_INT) {
            int * accumulator = ecalloc(CArray_DIMS(self)[*axis], sizeof(int));
            for (i = 0; i < CArray_DIMS(self)[*axis]; i++) {
                accumulator[i] = 1;
            }
            i = 0;
            do {
                for(j = 0; j < self->dimensions[*axis]; j++) {
                    accumulator[i] *= ((int*)CArrayIterator_DATA(it))[j * (self->strides[*axis]/self->descriptor->elsize)];
                    IT_IDATA(retit)[j * (ret->strides[*axis]/ret->descriptor->elsize)] = accumulator[i];
                }
                CArrayIterator_NEXT(it);
                CArrayIterator_NEXT(retit);
                i++;
            } while(CArrayIterator_NOTDONE(it));
            efree(accumulator);
        }
        if(rtype == TYPE_DOUBLE_INT) {
            double * accumulator = ecalloc(CArray_DIMS(self)[*axis], sizeof(double));
            for (i = 0; i < CArray_DIMS(self)[*axis]; i++) {
                accumulator[i] = 1.0;
            }
            i = 0;
            do {
                for(j = 0; j < self->dimensions[*axis]; j++) {
                    accumulator[i] *= ((double*)CArrayIterator_DATA(it))[j * (self->strides[*axis]/self->descriptor->elsize)];
                    IT_DDATA(retit)[j * (ret->strides[*axis]/ret->descriptor->elsize)] = accumulator[i];
                }
                CArrayIterator_NEXT(it);
                CArrayIterator_NEXT(retit);
                i++;
            } while(CArrayIterator_NOTDONE(it));
            efree(accumulator);
        }
        CArray_DECREF(self);
        CArrayIterator_FREE(it);
        CArrayIterator_FREE(retit);
    }

    if (out_ptr != NULL) {
        add_to_buffer(out_ptr, ret, sizeof(CArray));
    }

    return ret;
}

/**
 * CArray Prod
 **/ 
CArray *
CArray_Prod(CArray * self, int * axis, int rtype, MemoryPointer * out_ptr)
{
    int i, j = 0, z;
    void * total;
    CArray * arr, * ret = NULL;
    CArrayDescriptor * descr;
    CArrayIterator * it = NULL;

    ret = (CArray *)emalloc(sizeof(CArray));
    descr = (CArrayDescriptor*)ecalloc(1, sizeof(CArrayDescriptor));
    arr = CArray_CheckAxis(self, axis, 0);
    
    if(axis != NULL) {
        if(*axis >= CArray_NDIM(self)) {
            throw_axis_exception("Invalid axis for current matrix shape.");
            return NULL;
        }
        int index_jumps = self->strides[*axis] / self->descriptor->elsize;
    }

    if (arr == NULL) {
        return NULL;
    }

    switch(rtype) {
        case TYPE_INTEGER_INT:
            total = (int *)emalloc(sizeof(int));
            *((int *)total) = 0;
            break;
        case TYPE_DOUBLE_INT:
            total = (double *)emalloc(sizeof(double));
            *((double *)total) = 0.00;
            break;
        default:
            total = (double *)emalloc(sizeof(double));
            *((double *)total) = 0.00;
    }
    
    descr->type_num = self->descriptor->type_num;
    descr->type = self->descriptor->type;
    descr->elsize = self->descriptor->elsize;

    if(axis == NULL) {
        descr->numElements = 1;
        ret = CArray_NewFromDescr_int(ret, descr, 0, NULL, NULL, NULL, 0, NULL, 1, 0);
        if(rtype == TYPE_INTEGER_INT) {
            *((int*)total) = IDATA(self)[0];
            for(i = 1; i < CArray_DESCR(self)->numElements; i++) {
                *((int*)total) *= IDATA(self)[i];
            }
            IDATA(ret)[0] = *((int *)total);
        }
        if(rtype == TYPE_DOUBLE_INT) {
            *((double*)total) = DDATA(self)[0];
            for(i = 0; i < CArray_DESCR(self)->numElements; i++) {
                *((double*)total) *= DDATA(self)[i];
            }
            DDATA(ret)[0] = *((double *)total);
        }
    }
    if(axis != NULL) {
        int * new_dimensions = (int*)emalloc((self->ndim - 1) * sizeof(int));    
        for(i = 0; i < self->ndim; i++) {
            if(i != *axis) {
                new_dimensions[j] = self->dimensions[i];
                j++;
            }         
        }      
        int num_elements = new_dimensions[0];
        int * strides = CArray_Generate_Strides(new_dimensions, self->ndim-1, self->descriptor->type);

        for(i = 1; i < self->ndim-1; i++) {
            num_elements *= new_dimensions[i];
        }
        descr->numElements = num_elements;
        
        ret->descriptor = descr;
        
        if(rtype == TYPE_INTEGER_INT) {
            ret = CArray_NewFromDescr_int(ret, descr, self->ndim-1, new_dimensions, strides, NULL, 0, NULL, 1, 0);   
            it = CArray_IterAllButAxis(self, axis);
            
            for(i = 0; i < num_elements; i++) {
                IDATA(ret)[i] = 1;
            }

            i = 0;
            do {
                for(j = 0; j < self->dimensions[*axis]; j++) {
                    IDATA(ret)[i] *= ((int*)CArrayIterator_DATA(it))[j * (self->strides[*axis]/self->descriptor->elsize)];
                }
                CArrayIterator_NEXT(it);
                i++;
            } while(CArrayIterator_NOTDONE(it));
        }
        if(rtype == TYPE_DOUBLE_INT) {
            ret = CArray_NewFromDescr_int(ret, descr, self->ndim-1, new_dimensions, strides, NULL, 0, NULL, 1, 0);   
            it = CArray_IterAllButAxis(self, axis);
            
            for(i = 0; i < num_elements; i++) {
                DDATA(ret)[i] = 1.0;
            }

            i = 0;
            do {
                for(j = 0; j < self->dimensions[*axis]; j++) {
                    DDATA(ret)[i] *= ((double*)CArrayIterator_DATA(it))[j * (self->strides[*axis]/self->descriptor->elsize)];
                }
                CArrayIterator_NEXT(it);
                i++;
            } while(CArrayIterator_NOTDONE(it));
        }
        efree(new_dimensions);
        efree(strides);
    }

    if (out_ptr != NULL) {
        add_to_buffer(out_ptr, ret, sizeof(*ret));
    }

    if (it != NULL) {
        CArrayIterator_FREE(it);
    }

    efree(total);
    return ret;
}

/**
 * CArray Sum
 **/ 
CArray *
CArray_Sum(CArray * self, int * axis, int rtype, MemoryPointer * out_ptr)
{
    int i, j = 0, z;
    void * total;
    CArray * arr, * ret = NULL;
    CArrayDescriptor * descr;
    ret = (CArray *)emalloc(sizeof(CArray));
    descr = (CArrayDescriptor*)ecalloc(1, sizeof(CArrayDescriptor));
    arr = CArray_CheckAxis(self, axis, 0);
    
    if(check_and_adjust_axis_msg(axis, CArray_NDIM(self)) < 0) {
        return NULL;
    }

    if (arr == NULL) {
        return NULL;
    }

    switch(rtype) {
        case TYPE_INTEGER_INT:
            total = (int *)emalloc(sizeof(int));
            *((int *)total) = 0;
            break;
        case TYPE_DOUBLE_INT:
            total = (double *)emalloc(sizeof(double));
            *((double *)total) = 0.00;
            break;
        default:
            total = (double *)emalloc(sizeof(double));
            *((double *)total) = 0.00;
    }
    
    descr->type_num = self->descriptor->type_num;
    descr->type = self->descriptor->type;
    descr->elsize = self->descriptor->elsize;

    if(axis == NULL) {
        descr->numElements = 1;
        ret = CArray_NewFromDescr_int(ret, descr, 0, NULL, NULL, NULL, 0, NULL, 1, 0);
        if(rtype == TYPE_INTEGER_INT) {
            for(i = 0; i < CArray_DESCR(self)->numElements; i++) {
                *((int*)total) += IDATA(self)[i];
            }
            IDATA(ret)[0] = *((int *)total);
        }
        if(rtype == TYPE_DOUBLE_INT) {
            for(i = 0; i < CArray_DESCR(self)->numElements; i++) {
                *((double*)total) += DDATA(self)[i];
            }
            DDATA(ret)[0] = *((double *)total);
        }
    }
    if(axis != NULL) {
        CArrayIterator * it;
        int * new_dimensions = (int*)emalloc((self->ndim - 1) * sizeof(int));    
        for(i = 0; i < self->ndim; i++) {
            if(i != *axis) {
                new_dimensions[j] = self->dimensions[i];
                j++;
            }         
        }      
        int num_elements = new_dimensions[0];
        int * strides = CArray_Generate_Strides(new_dimensions, self->ndim-1, self->descriptor->type);
        for(i = 1; i < self->ndim-1; i++) {
            num_elements *= new_dimensions[i];
        }
        descr->numElements = num_elements;
        //descr->alignment = 0;
        ret->descriptor = descr;
        
        if(rtype == TYPE_INTEGER_INT) {
            ret = CArray_NewFromDescr_int(ret, descr, self->ndim-1, new_dimensions, strides, NULL, 0, NULL, 1, 0);   
            it = CArray_IterAllButAxis(self, axis);
            i = 0;
            do {
                for(j = 0; j < self->dimensions[*axis]; j++) {
                    IDATA(ret)[i] += ((int*)CArrayIterator_DATA(it))[j * (self->strides[*axis]/self->descriptor->elsize)];
                }
                CArrayIterator_NEXT(it);
                i++;
            } while(CArrayIterator_NOTDONE(it));
        }
        if(rtype == TYPE_DOUBLE_INT) {
            ret = CArray_NewFromDescr_int(ret, descr, self->ndim-1, new_dimensions, strides, NULL, 0, NULL, 1, 0);   
            it = CArray_IterAllButAxis(self, axis);
            i = 0;
            do {
                for(j = 0; j < self->dimensions[*axis]; j++) {
                    DDATA(ret)[i] += ((double*)CArrayIterator_DATA(it))[j * (self->strides[*axis]/self->descriptor->elsize)];
                }
                CArrayIterator_NEXT(it);
                i++;
            } while(CArrayIterator_NOTDONE(it));
        }
        CArray_DECREF(self);
        CArrayIterator_FREE(it);
        efree(strides);
        efree(new_dimensions);
    }
    add_to_buffer(out_ptr, ret, sizeof(*ret));
    efree(total);
    return ret;
}


CArray *
CArray_Any(CArray * target, int * axis, MemoryPointer * out)
{

}