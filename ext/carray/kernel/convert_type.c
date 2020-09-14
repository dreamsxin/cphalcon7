#include "convert_type.h"
#include "alloc.h"
#include "carray.h"
#include "convert.h"
#include "convert_datatype.h"

/**
 * @param op
 * @param minimum_type
 * @return
 */
int
CArray_ObjectType(CArray * op, int minimum_type)
{
    CArrayDescriptor *dtype = NULL;
    int ret;

    if (minimum_type >= 0) {
        if (CArray_TYPE(op) <= minimum_type) {
            dtype = CArray_DescrFromType(minimum_type);
        }
        if (CArray_TYPE(op) > minimum_type) {
            dtype = CArray_DescrFromType(CArray_TYPE(op));
        }

        if (dtype == NULL) {
            return TYPE_NOTYPE_INT;
        }
    }

    if (dtype == NULL) {
        ret = TYPE_DEFAULT_INT;
    }
    else {
        ret = dtype->type_num;
    }

    if (dtype != NULL) {
        CArrayDescriptor_FREE(dtype);
    }
    return ret;
}

/*
 * Returns true if data of type 'from' may be cast to data of type
 * 'to' according to the rule 'casting'.
 */
int
CArray_CanCastTypeTo(CArrayDescriptor *from, CArrayDescriptor *to,
                     CARRAY_CASTING casting)
{
    /* Fast path for unsafe casts or basic types */
    if (casting == CARRAY_UNSAFE_CASTING ||
            (CARRAY_LIKELY(from->type_num == to->type_num) &&
             CARRAY_LIKELY(from->byteorder == to->byteorder))) {
        return 1;
    }
    /* Equivalent types can be cast with any value of 'casting'  */
    else if (CArray_EquivTypes(from, to)) {
        switch (from->type_num) {
            default:
                switch (casting) {
                    case CARRAY_NO_CASTING:
                        return CArray_EquivTypes(from, to);
                    case CARRAY_EQUIV_CASTING:
                        return (from->elsize == to->elsize);
                    case CARRAY_SAFE_CASTING:
                        return (from->elsize <= to->elsize);
                    default:
                        return 1;
                }
                break;
        }
    }
    /* If safe or same-kind casts are allowed */
    else if (casting == CARRAY_SAFE_CASTING || casting == CARRAY_SAME_KIND_CASTING) {
        if (CArray_CanCastTo(from, to)) {
            return 1;
        }
        else if(casting == CARRAY_SAME_KIND_CASTING) {
            throw_notimplemented_exception();
        }
        else {
            return 0;
        }
    }
    /* NPY_NO_CASTING or NPY_EQUIV_CASTING was specified */
    else {
        return 0;
    }
    
}



/*
 * Returns 1 if the array object may be cast to the given data type using
 * the casting rule, 0 otherwise.  This differs from CArray_CanCastTo in
 * that it handles scalar arrays (0 dimensions) specially, by checking
 * their value.
 */
int
CArray_CanCastArrayTo(CArray *arr, CArrayDescriptor *to,
                      CARRAY_CASTING casting)
{
    CArrayDescriptor *from = CArray_DESCR(arr);

    /* If it's a scalar, check the value */
    if (CArray_NDIM(arr) == 0) {
        return can_cast_scalar_to(from, CArray_DATA(arr), to, casting);
    }

    /* Otherwise, use the standard rules */
    return CArray_CanCastTypeTo(from, to, casting);
}