#include "carray.h"
#include "matlib.h"
#include "scalar.h"
#include "alloc.h"
#include "convert.h"
#include "buffer.h"
#include "iterators.h"

CArray *
CArray_Zeros(int * shape, int nd, char type, char * order, MemoryPointer * rtn_ptr)
{
    int is_fortran = 0;
    CArrayDescriptor * new_descr;
    CArrayScalar * sc = emalloc(sizeof(CArrayScalar));
    CArray * rtn;
    int allocated = 0;

    if (order == NULL) {
        order = emalloc(sizeof(char));
        *order = 'C';
        allocated = 1;
    }

    if (*order == 'F') {
        is_fortran = 1;
    }

    new_descr = CArray_DescrFromType(CHAR_TYPE_INT(type));
    rtn = CArray_Empty(nd, shape, new_descr, is_fortran, rtn_ptr);

    sc->obval = emalloc(new_descr->elsize);
    sc->type  = CHAR_TYPE_INT(type);

    if(type == TYPE_DOUBLE){
        *((double *)sc->obval) = (double)0.00;
    }
    if(type == TYPE_INTEGER){
        *((int *)sc->obval) = (int)0;
    }
    if(type == TYPE_FLOAT){
        *((float *)sc->obval) = (float)0;
    }

    CArray_FillWithScalar(rtn, sc);

    efree(sc->obval);
    efree(sc);

    if(allocated) {
        efree(order);
    }
    return rtn;
}


CArray *
CArray_Ones(int * shape, int nd, char * type, char * order, MemoryPointer * rtn_ptr)
{
    int is_fortran = 0, order_allocated = 0;
    CArrayDescriptor * new_descr;
    CArrayScalar * sc = emalloc(sizeof(CArrayScalar));
    CArray * rtn;

    if (order == NULL) {
        order = emalloc(sizeof(char));
        *order = 'C';
        order_allocated = 1;
    }

    if (*order == 'F') {
        is_fortran = 1;
    }

    new_descr = CArray_DescrFromType(CHAR_TYPE_INT(*type));
    rtn = CArray_Empty(nd, shape, new_descr, is_fortran, rtn_ptr);

    sc->obval = emalloc(new_descr->elsize);
    sc->type  = CHAR_TYPE_INT(*type);

    if(*type == TYPE_DOUBLE){
        *((double *)sc->obval) = (double)1.00;
    }
    if(*type == TYPE_INTEGER){
        *((int *)sc->obval) = (int)1;
    }
    if(*type == TYPE_FLOAT){
        *((float *)sc->obval) = (float)1;
    }
    
    CArray_FillWithScalar(rtn, sc);

    efree(sc->obval);
    efree(sc);

    if (order_allocated) {
        efree(order);
    }
    return rtn;
}

CArray *
CArray_Flip(CArray *a, int * axis, MemoryPointer * out)
{
    CArrayIterator * it;
    CArray * rtn;

    it  = CArray_NewIter(a);
    rtn = CArray_NewLikeArray(a, CARRAY_KEEPORDER, CArray_DESCR(a), 0);
    CArrayDescriptor_INCREF(CArray_DESCR(a));

    if (axis == NULL) {
        switch(CArray_TYPE(a)) {
            case TYPE_DOUBLE_INT:
                do {
                    DDATA(rtn)[CArray_DESCR(a)->numElements - it->index - 1] = *IT_DDATA(it);
                    CArrayIterator_NEXT(it);
                } while(CArrayIterator_NOTDONE(it));
                break;
            case TYPE_INTEGER_INT:
                do {
                    IDATA(rtn)[CArray_DESCR(a)->numElements - it->index - 1] = *IT_IDATA(it);
                    CArrayIterator_NEXT(it);
                } while(CArrayIterator_NOTDONE(it));
                break;
        }
    }
    else {
        throw_notimplemented_exception();
        CArrayIterator_FREE(it);
        return NULL;
    }

    if (out != NULL) {
        add_to_buffer(out, rtn, sizeof(CArray));
    }

    CArrayIterator_FREE(it);
    return rtn;
}