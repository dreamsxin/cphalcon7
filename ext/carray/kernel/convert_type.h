#ifndef PHPSCI_EXT_CONVERT_TYPE_H
#define PHPSCI_EXT_CONVERT_TYPE_H

#include "carray.h"

/* Converts a type number from unsigned to signed */
static int
type_num_unsigned_to_signed(int type_num)
{
    switch (type_num) {
        default:
            return type_num;
    }
}

/*
 * The is_small_unsigned output flag indicates whether it's an unsigned integer,
 * and would fit in a signed integer of the same bit size.
 */
static
int min_scalar_type_num(char *valueptr, int type_num,
                               int *is_small_unsigned)
{
    switch (type_num) {
        case TYPE_INTEGER_INT: {
            break;
        }
        case TYPE_DOUBLE_INT: {
            double value = *(double *)valueptr;
            if (value > -3.4e38 && value < 3.4e38) {
                return TYPE_FLOAT_INT;
            }
            break;
        }
    }
    return type_num;
}

int CArray_ObjectType(CArray * op, int minimum_type);
int CArray_CanCastTypeTo(CArrayDescriptor *from, CArrayDescriptor *to, CARRAY_CASTING casting);
int CArray_CanCastArrayTo(CArray *arr, CArrayDescriptor *to, CARRAY_CASTING casting);
#endif //PHPSCI_EXT_CONVERT_TYPE_H
