#ifndef CARRAY_RANGE_H
#define CARRAY_RANGE_H

#include "carray.h"

CArray * CArray_Linspace(double start, double stop, int num, int endpoint, int retstep, int * axis, int type, MemoryPointer * out);
CArray * CArray_Arange(double start, double stop, double step, int type_num, MemoryPointer * ptr);
CArray * CArray_Logspace(double start, double stop, int num, int endpoint, double base, int typenum, MemoryPointer * out);
CArray * CArray_Geomspace(double start, double stop, int num, int endpoint, int typenum, MemoryPointer * out);

#endif //CARRAY_RANGE_H
