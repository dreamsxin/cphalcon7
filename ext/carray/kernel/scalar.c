#include "carray.h"
#include "scalar.h"
#include "php.h"
#include "alloc.h"
#include "buffer.h"

CArrayScalar *
CArrayScalar_NewLong(long sc)
{
    CArrayScalar * ret = NULL;
    ret = emalloc(sizeof(CArrayScalar));
    ret->obval = emalloc(sizeof(long));
    ret->type = TYPE_LONG_INT;
    *((long*)ret->obval) = sc;
    return ret;
}

CArrayScalar *
CArrayScalar_NewFloat(float sc)
{
    CArrayScalar * ret = NULL;
    ret = emalloc(sizeof(CArrayScalar));
    ret->obval = emalloc(sizeof(float));
    ret->type = TYPE_FLOAT_INT;
    *((float*)ret->obval) = sc;
    return ret;
}

CArrayScalar *
CArrayScalar_NewDouble(double sc)
{
    CArrayScalar * ret = NULL;
    ret = emalloc(sizeof(CArrayScalar));
    ret->obval = emalloc(sizeof(double));
    ret->type = TYPE_DOUBLE_INT;
    *((double*)ret->obval) = sc;
    return ret;
}

CArrayScalar *
CArrayScalar_NewInt(int sc)
{
    CArrayScalar * ret = NULL;
    ret = emalloc(sizeof(CArrayScalar));
    ret->obval = emalloc(sizeof(int));
    ret->type = TYPE_INTEGER_INT;
    *((int*)ret->obval) = sc;
    return ret;
}

void
CArrayScalar_FREE(CArrayScalar * sc)
{
    efree(sc->obval);
    efree(sc);
}

CArrayScalar *
CArrayScalar_FromZval(PHPObject * obj, int is_double, int is_long)
{
    if(Z_TYPE_P(obj) == IS_DOUBLE) {
        /**
         * PHP assumes FLOAT and DOUBLE as same type, user
         * must identify wich one will be used.
         **/ 
        if(!is_double) {
            return CArrayScalar_NewFloat((float)Z_DVAL_P(obj));
        }
        return CArrayScalar_NewDouble(Z_DVAL_P(obj));
    }
    if(Z_TYPE_P(obj) == IS_LONG) {
        /**
         * PHP assumes LONG and INTEGER as same type, user
         * must identify wich one will be used.
         */ 
        if(!is_long) {
           return CArrayScalar_NewInt((int)Z_LVAL_P(obj));
        }
        return CArrayScalar_NewLong((long)Z_LVAL_P(obj));
    }
}

void *
scalar_value(CArrayScalar *scalar, CArrayDescriptor *descr)
{
    int type_num;
    int align;
    int memloc;

    if (descr == NULL) {
        descr = CArray_DescrFromScalar(scalar);
        type_num = descr->type_num;
        CArrayDescriptor_DECREF(descr);
    }
    else {
        type_num = descr->type_num;
    }
    
    switch(type_num)
    {
        case TYPE_DOUBLE_INT:
            return (double *) scalar->obval;
            break;
        case TYPE_INTEGER_INT:
            return (int *) scalar->obval;
            break;    
        case TYPE_FLOAT_INT:
            return (float *) scalar->obval;
            break;    
    }

    /**switch (type_num) {
        #define CASE(ut,lt) case TYPE_##ut: return &(((CArray##lt##ScalarObject *)scalar)->obval)
        CASE(INTEGER, Integer);
        CASE(LONG, Long);
        CASE(FLOAT, Float);
        CASE(DOUBLE, Double);
    }**/
}

CArrayDescriptor *
CArray_DescrFromScalar(CArrayScalar *sc)
{
    return CArray_DescrFromType(sc->type);
}