#ifndef PHPSCI_EXT_CALCULATION_H
#define PHPSCI_EXT_CALCULATION_H

#include "carray.h"

#define CArray_MAX(a,b) (((a)>(b))?(a):(b))
#define CArray_MIN(a,b) (((a)<(b))?(a):(b))


CArray * CArray_Sum(CArray * self, int * axis, int rtype, MemoryPointer * out_ptr);
CArray * CArray_Prod(CArray * self, int * axis, int rtype, MemoryPointer * out_ptr);
CArray * CArray_CumProd(CArray * self, int * axis, int rtype, MemoryPointer * out_ptr);
CArray * CArray_CumSum(CArray * self, int * axis, int rtype, MemoryPointer * out_ptr);
CArray * CArray_Any(CArray * target, int * axis, MemoryPointer * out);
#endif 