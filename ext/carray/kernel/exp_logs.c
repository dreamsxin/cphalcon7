#include "exp_logs.h"
#include "carray.h"
#include "trigonometric.h"
#include "alloc.h"
#include "buffer.h"

CArray *
CArray_Exp(CArray * target, MemoryPointer * out)
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

    CArray_ElementWise_CFunc(target, result, &exp);

    if(out != NULL) {
        add_to_buffer(out, result, sizeof(CArray *));
    }
}

CArray *
CArray_Expm1(CArray * target, MemoryPointer * out)
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

    CArray_ElementWise_CFunc(target, result, &expm1);

    if(out != NULL) {
        add_to_buffer(out, result, sizeof(CArray *));
    }
}

CArray *
CArray_Exp2(CArray * target, MemoryPointer * out)
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

    CArray_ElementWise_CFunc(target, result, &exp2);

    if(out != NULL) {
        add_to_buffer(out, result, sizeof(CArray *));
    }
}

CArray *
CArray_Log(CArray * target, MemoryPointer * out)
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

    CArray_ElementWise_CFunc(target, result, &log);

    if(out != NULL) {
        add_to_buffer(out, result, sizeof(CArray *));
    }
}

CArray *
CArray_Log10(CArray * target, MemoryPointer * out)
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

    CArray_ElementWise_CFunc(target, result, &log10);

    if(out != NULL) {
        add_to_buffer(out, result, sizeof(CArray *));
    }
}

CArray *
CArray_Log2(CArray * target, MemoryPointer * out)
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

    CArray_ElementWise_CFunc(target, result, &log2);

    if(out != NULL) {
        add_to_buffer(out, result, sizeof(CArray *));
    }
}

CArray *
CArray_Log1p(CArray * target, MemoryPointer * out)
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

    CArray_ElementWise_CFunc(target, result, &log1p);

    if(out != NULL) {
        add_to_buffer(out, result, sizeof(CArray *));
    }
}