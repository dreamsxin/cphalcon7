#ifndef PHPSCI_EXT_FLAGSOBJECT_H
#define PHPSCI_EXT_FLAGSOBJECT_H

#include "carray.h"

/*
 * Enables the specified array flags.
 */
static void
CArray_ENABLEFLAGS(CArray * arr, int flags)
{
    (arr)->flags |= flags;
}

/*
 * Clears the specified array flags. Does no checking,
 * assumes you know what you're doing.
 */
static void
CArray_CLEARFLAGS(CArray *arr, int flags)
{
    (arr)->flags &= ~flags;
}

void CArray_UpdateFlags(CArray * array, int flagmask);

#endif //PHPSCI_EXT_FLAGSOBJECT_H