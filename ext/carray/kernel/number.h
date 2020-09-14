#ifndef PHPSCI_EXT_NUMBER_H
#define PHPSCI_EXT_NUMBER_H

#include "carray.h"

CArray * CArray_Add(CArray *m1, CArray *m2, MemoryPointer * ptr);
CArray * CArray_Subtract(CArray *m1, CArray *m2, MemoryPointer * ptr);
CArray * CArray_Multiply(CArray *m1, CArray *m2, MemoryPointer * ptr);
CArray * CArray_Divide(CArray *m1, CArray *m2, MemoryPointer * ptr);
CArray * CArray_Power(CArray *m1, CArray *m2, MemoryPointer * ptr);
CArray * CArray_Mod(CArray *m1, CArray *m2, MemoryPointer * ptr);
CArray * CArray_Negative(CArray * a, MemoryPointer * out);
CArray * CArray_Sqrt(CArray *a, MemoryPointer *out);
CArray * CArray_Reciprocal(CArray *a, MemoryPointer *out);
CArray * CArray_Absolute(CArray *a, MemoryPointer *out);
#endif //PHPSCI_EXT_NUMBER_H
