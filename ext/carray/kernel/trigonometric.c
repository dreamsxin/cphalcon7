#include "trigonometric.h"
#include "carray.h"
#include "buffer.h"

void
CArray_ElementWise_CFunc_Double(CArray * target, CArray * result, CArray_CFunc_ElementWise * func)
{
    int i;
    for(i = 0; i < CArray_DESCR(target)->numElements; i++) {
        DDATA(result)[i] = func(DDATA(target)[i]);
    }
}

void
CArray_ElementWise_CFunc_Int(CArray * target, CArray * result, CArray_CFunc_ElementWise * func)
{
    int i;
    for(i = 0; i < CArray_DESCR(target)->numElements; i++) {
        DDATA(result)[i] = func((double)IDATA(target)[i]);
    }
}

void
CArray_ElementWise_CFunc(CArray * target, CArray * result, CArray_CFunc_ElementWise * func)
{
    if(CArray_TYPE(target) == TYPE_INTEGER_INT) {
        CArray_ElementWise_CFunc_Int(target, result, func);
        return;
    }
    if(CArray_TYPE(target) == TYPE_DOUBLE_INT) {
        CArray_ElementWise_CFunc_Double(target, result, func);
        return;
    }
}

CArray *
CArray_Sin(CArray * target, MemoryPointer * out)
{
    CArray * result;
    CArrayDescriptor * descr;
    int * new_strides;
    result = emalloc(sizeof(CArray));

    descr = CArray_DescrFromType(TYPE_DOUBLE_INT);

    result = CArray_NewFromDescr_int(result, descr, CArray_NDIM(target),
            CArray_DIMS(target), NULL, NULL,
            CArray_FLAGS(target), NULL, 1, 0);
    result->flags = ~CARRAY_ARRAY_F_CONTIGUOUS;

    CArray_ElementWise_CFunc(target, result, &sin);

    if(out != NULL) {
        add_to_buffer(out, result, sizeof(CArray *));
    }
}

CArray *
CArray_Cos(CArray * target, MemoryPointer * out)
{
    CArray * result;
    CArrayDescriptor * descr;
    int * new_strides;
    result = emalloc(sizeof(CArray));

    descr = CArray_DescrFromType(TYPE_DOUBLE_INT);

    result = CArray_NewFromDescr_int(result, descr, CArray_NDIM(target),
                                     CArray_DIMS(target), NULL, NULL,
                                     CArray_FLAGS(target), NULL, 1, 0);
    result->flags = ~CARRAY_ARRAY_F_CONTIGUOUS;

    CArray_ElementWise_CFunc(target, result, &cos);

    if(out != NULL) {
        add_to_buffer(out, result, sizeof(CArray *));
    }
}

CArray *
CArray_Tan(CArray * target, MemoryPointer * out)
{
    CArray * result;
    CArrayDescriptor * descr;
    int * new_strides;
    result = emalloc(sizeof(CArray));

    descr = CArray_DescrFromType(TYPE_DOUBLE_INT);

    result = CArray_NewFromDescr_int(result, descr, CArray_NDIM(target),
                                     CArray_DIMS(target), NULL, NULL,
                                     CArray_FLAGS(target), NULL, 1, 0);
    result->flags = ~CARRAY_ARRAY_F_CONTIGUOUS;

    CArray_ElementWise_CFunc(target, result, &tan);

    if(out != NULL) {
        add_to_buffer(out, result, sizeof(CArray *));
    }
}

CArray *
CArray_Arcsin(CArray * target, MemoryPointer * out)
{
    CArray * result;
    CArrayDescriptor * descr;
    int * new_strides;
    result = emalloc(sizeof(CArray));

    descr = CArray_DescrFromType(TYPE_DOUBLE_INT);

    result = CArray_NewFromDescr_int(result, descr, CArray_NDIM(target),
                                     CArray_DIMS(target), NULL, NULL,
                                     CArray_FLAGS(target), NULL, 1, 0);
    result->flags = ~CARRAY_ARRAY_F_CONTIGUOUS;

    CArray_ElementWise_CFunc(target, result, &asin);

    if(out != NULL) {
        add_to_buffer(out, result, sizeof(CArray *));
    }
}

CArray *
CArray_Arccos(CArray * target, MemoryPointer * out)
{
    CArray * result;
    CArrayDescriptor * descr;
    int * new_strides;
    result = emalloc(sizeof(CArray));

    descr = CArray_DescrFromType(TYPE_DOUBLE_INT);

    result = CArray_NewFromDescr_int(result, descr, CArray_NDIM(target),
                                     CArray_DIMS(target), NULL, NULL,
                                     CArray_FLAGS(target), NULL, 1, 0);
    result->flags = ~CARRAY_ARRAY_F_CONTIGUOUS;

    CArray_ElementWise_CFunc(target, result, &acos);

    if(out != NULL) {
        add_to_buffer(out, result, sizeof(CArray *));
    }
}

CArray *
CArray_Arctan(CArray * target, MemoryPointer * out)
{
    CArray * result;
    CArrayDescriptor * descr;
    int * new_strides;
    result = emalloc(sizeof(CArray));

    descr = CArray_DescrFromType(TYPE_DOUBLE_INT);

    result = CArray_NewFromDescr_int(result, descr, CArray_NDIM(target),
                                     CArray_DIMS(target), NULL, NULL,
                                     CArray_FLAGS(target), NULL, 1, 0);
    result->flags = ~CARRAY_ARRAY_F_CONTIGUOUS;

    CArray_ElementWise_CFunc(target, result, &atan);

    if(out != NULL) {
        add_to_buffer(out, result, sizeof(CArray *));
    }
}

CArray *
CArray_Sinh(CArray * target, MemoryPointer * out)
{
    CArray * result;
    CArrayDescriptor * descr;
    int * new_strides;
    result = emalloc(sizeof(CArray));

    descr = CArray_DescrFromType(TYPE_DOUBLE_INT);

    result = CArray_NewFromDescr_int(result, descr, CArray_NDIM(target),
                                     CArray_DIMS(target), NULL, NULL,
                                     CArray_FLAGS(target), NULL, 1, 0);
    result->flags = ~CARRAY_ARRAY_F_CONTIGUOUS;

    CArray_ElementWise_CFunc(target, result, &sinh);

    if(out != NULL) {
        add_to_buffer(out, result, sizeof(CArray *));
    }
}

CArray *
CArray_Cosh(CArray * target, MemoryPointer * out)
{
    CArray * result;
    CArrayDescriptor * descr;
    int * new_strides;
    result = emalloc(sizeof(CArray));

    descr = CArray_DescrFromType(TYPE_DOUBLE_INT);

    result = CArray_NewFromDescr_int(result, descr, CArray_NDIM(target),
                                     CArray_DIMS(target), NULL, NULL,
                                     CArray_FLAGS(target), NULL, 1, 0);
    result->flags = ~CARRAY_ARRAY_F_CONTIGUOUS;

    CArray_ElementWise_CFunc(target, result, &cosh);

    if(out != NULL) {
        add_to_buffer(out, result, sizeof(CArray *));
    }
}

CArray *
CArray_Tanh(CArray * target, MemoryPointer * out)
{
    CArray * result;
    CArrayDescriptor * descr;
    int * new_strides;
    result = emalloc(sizeof(CArray));

    descr = CArray_DescrFromType(TYPE_DOUBLE_INT);

    result = CArray_NewFromDescr_int(result, descr, CArray_NDIM(target),
                                     CArray_DIMS(target), NULL, NULL,
                                     CArray_FLAGS(target), NULL, 1, 0);
    result->flags = ~CARRAY_ARRAY_F_CONTIGUOUS;

    CArray_ElementWise_CFunc(target, result, &tanh);

    if(out != NULL) {
        add_to_buffer(out, result, sizeof(CArray *));
    }
}

CArray *
CArray_Arcsinh(CArray * target, MemoryPointer * out)
{
    CArray * result;
    CArrayDescriptor * descr;
    int * new_strides;
    result = emalloc(sizeof(CArray));

    descr = CArray_DescrFromType(TYPE_DOUBLE_INT);

    result = CArray_NewFromDescr_int(result, descr, CArray_NDIM(target),
                                     CArray_DIMS(target), NULL, NULL,
                                     CArray_FLAGS(target), NULL, 1, 0);
    result->flags = ~CARRAY_ARRAY_F_CONTIGUOUS;

    CArray_ElementWise_CFunc(target, result, &asinh);

    if(out != NULL) {
        add_to_buffer(out, result, sizeof(CArray *));
    }
}

CArray *
CArray_Arccosh(CArray * target, MemoryPointer * out)
{
    CArray * result;
    CArrayDescriptor * descr;
    int * new_strides;
    result = emalloc(sizeof(CArray));

    descr = CArray_DescrFromType(TYPE_DOUBLE_INT);

    result = CArray_NewFromDescr_int(result, descr, CArray_NDIM(target),
                                     CArray_DIMS(target), NULL, NULL,
                                     CArray_FLAGS(target), NULL, 1, 0);
    result->flags = ~CARRAY_ARRAY_F_CONTIGUOUS;

    CArray_ElementWise_CFunc(target, result, &acosh);

    if(out != NULL) {
        add_to_buffer(out, result, sizeof(CArray *));
    }
}

CArray *
CArray_Arctanh(CArray * target, MemoryPointer * out)
{
    CArray * result;
    CArrayDescriptor * descr;
    int * new_strides;
    result = emalloc(sizeof(CArray));

    descr = CArray_DescrFromType(TYPE_DOUBLE_INT);

    result = CArray_NewFromDescr_int(result, descr, CArray_NDIM(target),
                                     CArray_DIMS(target), NULL, NULL,
                                     CArray_FLAGS(target), NULL, 1, 0);
    result->flags = ~CARRAY_ARRAY_F_CONTIGUOUS;

    CArray_ElementWise_CFunc(target, result, &atanh);

    if(out != NULL) {
        add_to_buffer(out, result, sizeof(CArray *));
    }
}

