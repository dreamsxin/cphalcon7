#ifndef PHPSCI_EXT_CONVERT_H
#define PHPSCI_EXT_CONVERT_H

#include "carray.h"
#include "scalar.h"

CArray * CArray_Slice_Index(CArray * self, int index, MemoryPointer * out);
CArray * CArray_Slice_Str(CArray * self, char * index, MemoryPointer * out);

CArray * CArray_View(CArray *self);
CArray * CArray_NewCopy(CArray *obj, CARRAY_ORDER order);
int CArray_CanCastTo(CArrayDescriptor *from, CArrayDescriptor *to);
int CArray_CanCastSafely(int fromtype, int totype);
int CArray_CastTo(CArray *out, CArray *mp);
int CArray_FillWithScalar(CArray * arr, CArrayScalar * sc);
#endif //PHPSCI_EXT_CONVERT_H
