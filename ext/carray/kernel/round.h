#ifndef CARRAY_ROUND_H
#define CARRAY_ROUND_H

#include "carray.h"

CArray * CArray_Floor(CArray *a, MemoryPointer *out);
CArray * CArray_Ceil(CArray *a, MemoryPointer *out);
CArray * CArray_Round(CArray *a, int decimals, MemoryPointer *out);

#endif //CARRAY_ROUND_H
