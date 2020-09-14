#ifndef PHPSCI_EXT_CONVERT_DATATYPE_H
#define PHPSCI_EXT_CONVERT_DATATYPE_H

#include "carray.h"

int can_cast_scalar_to(CArrayDescriptor *scal_type, char *scal_data,
                       CArrayDescriptor *to, CARRAY_CASTING casting);

#endif //PHPSCI_EXT_CONVERT_DATATYPE_H
