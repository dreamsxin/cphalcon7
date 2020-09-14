#ifndef PHPSCI_EXT_MATLIB_H
#define PHPSCI_EXT_MATLIB_H

#include "carray.h"

CArray * CArray_Zeros(int * shape, int nd, char type, char * order, MemoryPointer * rtn_ptr);
CArray * CArray_Ones(int * shape, int nd, char * type, char * order, MemoryPointer * rtn_ptr);
CArray * CArray_Flip(CArray *a, int * axis, MemoryPointer * out);
#endif //PHPSCI_EXT_MATLIB_H
