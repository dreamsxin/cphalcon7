#ifndef CARRAY_STATISTICS_H
#define CARRAY_STATISTICS_H

#include "carray.h"

CArray * CArray_Correlate2(CArray * op1, CArray * op2, int mode, MemoryPointer * out);
CArray * CArray_Correlate(CArray * op1, CArray * op2, int mode, MemoryPointer * out);

#endif //CARRAY_STATISTICS_H
