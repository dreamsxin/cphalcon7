#ifndef CARRAY_CLIP_H
#define CARRAY_CLIP_H

#include "carray.h"

CArray * CArray_Clip(CArray * self, CArray * min, CArray * max, MemoryPointer * out);
void DOUBLE_clip(double *in, int ni, double *min, double *max, double *out);
void INT_clip(int *in, int ni, int *min, int *max, int *out);

#endif //CARRAY_CLIP_H
