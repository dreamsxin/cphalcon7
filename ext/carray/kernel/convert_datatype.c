#include "carray.h"
#include "convert_datatype.h"
#include "common/exceptions.h"
#include "convert_type.h"

int
can_cast_scalar_to(CArrayDescriptor *scal_type, char *scal_data,
                   CArrayDescriptor *to, CARRAY_CASTING casting)
{
    int swap;
    int is_small_unsigned = 0, type_num;
    int ret;
    CArrayDescriptor *dtype;

    /* An aligned memory buffer large enough to hold any type */
    long long value[4];

    /*
     * If the two dtypes are actually references to the same object
     * or if casting type is forced unsafe then always OK.
     */
    if (scal_type == to || casting == CARRAY_UNSAFE_CASTING ) {
        return 1;
    }

    swap = !CArray_ISNBO(scal_type->byteorder);
    scal_type->f->copyswap(&value, scal_data, swap, NULL);

    type_num = min_scalar_type_num((char *)&value, scal_type->type_num,
                                    &is_small_unsigned);

    /*
     * If we've got a small unsigned scalar, and the 'to' type
     * is not unsigned, then make it signed to allow the value
     * to be cast more appropriately.
     */
    if (is_small_unsigned && !(CArrayTypeNum_ISUNSIGNED(to->type_num))) {
        type_num = type_num_unsigned_to_signed(type_num);
    }

    dtype = CArray_DescrFromType(type_num);
    if (dtype == NULL) {
        return 0;
    }

    ret = CArray_CanCastTypeTo(dtype, to, casting);
    return ret;
}           
                       