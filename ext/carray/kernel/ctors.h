#ifndef CARRAY_CTORS_H
#define CARRAY_CTORS_H

#include "carray.h"

int setArrayFromSequence(CArray *a, CArray *s, int dim, int offset);
CArray * CArray_FromArray(CArray *arr, CArrayDescriptor *newtype, int flags);
void CArray_ToArray(CArray *a, zval * rtn);
#endif //CARRAY_CTORS_H
