#ifndef CARRAY_EXP_LOGS_H
#define CARRAY_EXP_LOGS_H

#include "carray.h"

CArray * CArray_Exp(CArray * target, MemoryPointer * out);
CArray * CArray_Expm1(CArray * target, MemoryPointer * out);
CArray * CArray_Exp2(CArray * target, MemoryPointer * out);
CArray * CArray_Log(CArray * target, MemoryPointer * out);
CArray * CArray_Log10(CArray * target, MemoryPointer * out);
CArray * CArray_Log2(CArray * target, MemoryPointer * out);
CArray * CArray_Log1p(CArray * target, MemoryPointer * out);
#endif //CARRAY_EXP_LOGS_H
