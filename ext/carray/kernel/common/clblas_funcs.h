#ifndef PHPSCI_EXT_CLBLAS_FUNC_H
#define PHPSCI_EXT_CLBLAS_FUNC_H

#include "../carray.h"

CArray * clblas_matrixproduct(int typenum, CArray * ap1, CArray *ap2, CArray *out, MemoryPointer * ptr);

#endif //PHPSCI_EXT_CLBLAS_FUNC_H