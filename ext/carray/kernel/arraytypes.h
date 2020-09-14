#ifndef CARRAY_ARRAYTYPES_H
#define CARRAY_ARRAYTYPES_H

#include "carray.h"


int
small_correlate(const char * d_, int dstride,
                int nd, int dtype,
                const char * k_, int kstride,
                int nk, int ktype,
                char * out_, int ostride);
#endif //CARRAY_ARRAYTYPES_H
