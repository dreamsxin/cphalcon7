#ifndef PHPSCI_EXT_ASSIGN_SCALAR_H
#define PHPSCI_EXT_ASSIGN_SCALAR_H

#include "carray.h"

int CArray_AssignRawScalar(CArray *dst, CArrayDescriptor *src_dtype, char *src_data, CArray *wheremask,
                           CARRAY_CASTING casting);

#endif