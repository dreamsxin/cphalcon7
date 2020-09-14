#ifndef PHPSCI_EXT_SHAPE_H
#define PHPSCI_EXT_SHAPE_H

#include "carray.h"

CArray * normalize_axis_tuple(CArray * axis, int ndim, int allow_duplicate);

CArray * CArray_Newshape(CArray * self, int *newdims, int new_ndim, CARRAY_ORDER order, MemoryPointer * ptr);
CArray * CArray_Transpose(CArray * target, CArray_Dims * permute, MemoryPointer * ptr);
CArray * CArray_SwapAxes(CArray * ap, int a1, int a2, MemoryPointer * out);
CArray * CArray_Rollaxis(CArray * arr, int axis, int start, MemoryPointer * out);
CArray * CArray_Moveaxis(CArray * target, CArray * source, CArray * destination, MemoryPointer * out);
void     CArray_CreateSortedStridePerm(int ndim, int *strides, ca_stride_sort_item *out_strideperm);
CArray * CArray_Ravel(CArray *arr, CARRAY_ORDER order);
CArray * CArray_atleast1d(CArray * self, MemoryPointer * out);
CArray * CArray_atleast2d(CArray * self, MemoryPointer * out);
CArray * CArray_atleast3d(CArray * self, MemoryPointer * out);
CArray * CArray_Squeeze(CArray * self, int * axis, MemoryPointer * out);
CArray * CArray_SqueezeSelected(CArray * self, int *axis_flags, int n_axis);
void CArray_RemoveAxesInPlace(CArray *arr, int *flags);
CArray * CArray_ExpandDims(CArray * target, int axis, MemoryPointer * out);
#endif //PHPSCI_EXT_SHAPE_H
