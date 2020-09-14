#ifndef CARRAY_STORAGE_H
#define CARRAY_STORAGE_H

#include "carray.h"

int CArrayStorage_SaveBin(char * filename, CArray *target);
int CArrayStorage_LoadBin(char * filename, MemoryPointer * out);
#endif //CARRAY_STORAGE_H
