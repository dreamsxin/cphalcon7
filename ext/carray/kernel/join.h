#ifndef CARRAY_JOIN_H
#define CARRAY_JOIN_H

#include "carray.h"

CArray *
CArray_Concatenate(CArray ** target, int narrays, int * axis, MemoryPointer * out);

#endif //CARRAY_JOIN_H
