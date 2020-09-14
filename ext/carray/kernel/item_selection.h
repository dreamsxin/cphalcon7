#ifndef PHPSCI_EXT_ITEM_SELECTION_H
#define PHPSCI_EXT_ITEM_SELECTION_H

#include "carray.h"

int
INT_fasttake(int *dest, int *src, int *indarray,
                    int nindarray, int n_outer,
                    int m_middle, int nelem,
                    CARRAY_CLIPMODE clipmode);
int
DOUBLE_fasttake(double *dest, double *src, int *indarray,
                    int nindarray, int n_outer,
                    int m_middle, int nelem,
                    CARRAY_CLIPMODE clipmode);

                    
CArray * CArray_Diagonal(CArray *self, int offset, int axis1, int axis2, MemoryPointer * rtn);
CArray * CArray_TakeFrom(CArray * target, CArray * indices0, int axis, MemoryPointer * out, CARRAY_CLIPMODE clipmode);
CArray * CArray_Sort(CArray * target, int * axis, CARRAY_SORTKIND which, int inplace, MemoryPointer * out);
#endif //PHPSCI_EXT_ITEM_SELECTION_H