#include "carray.h"
#include "assign_scalar.h"
#include "convert_datatype.h"
#include "assign.h"
#include "common/exceptions.h"
#include "common/common.h"
#include "alloc.h"
#include "dtype_transfer.h"

/*
 * Assigns the scalar value to every element of the destination raw array.
 *
 * Returns 0 on success, -1 on failure.
 */
 int
raw_array_assign_scalar(int ndim, int *shape,
        CArrayDescriptor *dst_dtype, char *dst_data, int *dst_strides,
        CArrayDescriptor *src_dtype, char *src_data)
{
    int idim;
    int * shape_it, * dst_strides_it;
    int * coord;

    CArray_StridedUnaryOp *stransfer = NULL;
    CArrayAuxData *transferdata = NULL;
    int aligned, needs_api = 0;
    int src_itemsize = src_dtype->elsize;

    shape_it = emalloc(sizeof(int) * ndim * 2);
    dst_strides_it = emalloc(sizeof(int) * ndim * 2);
    coord = emalloc(sizeof(int) * ndim * 2);

    /* Check both uint and true alignment */
    aligned = raw_array_is_aligned(ndim, shape, dst_data, dst_strides,
                                   carray_uint_alignment(dst_dtype->elsize)) &&
              raw_array_is_aligned(ndim, shape, dst_data, dst_strides,
                                   dst_dtype->alignment) &&
              carray_is_aligned(src_data, carray_uint_alignment(src_dtype->elsize) &&
              carray_is_aligned(src_data, src_dtype->alignment));

    /* Use raw iteration with no heap allocation */
    if (CArray_PrepareOneRawArrayIter(
                    ndim, shape,
                    dst_data, dst_strides,
                    &ndim, shape_it,
                    &dst_data, dst_strides_it) < 0) {
        return -1;
    }

    /* Get the function to do the casting */
    if (CArray_GetDTypeTransferFunction(aligned,
                        0, dst_strides_it[0],
                        src_dtype, dst_dtype,
                        0,
                        &stransfer, &transferdata,
                        &needs_api) != CARRAY_SUCCEED) {
        return -1;
    }

    if (!needs_api) {
        int nitems = 1, i;
        for (i = 0; i < ndim; i++) {
            nitems *= shape_it[i];
        }
    }

    CARRAY_RAW_ITER_START(idim, ndim, coord, shape_it) {
        /* Process the innermost dimension */
        stransfer(dst_data, dst_strides_it[0], src_data, 0,
                    shape_it[0], src_itemsize, transferdata);
    } CARRAY_RAW_ITER_ONE_NEXT(idim, ndim, coord,
                            shape_it, dst_data, dst_strides_it);


    CARRAY_AUXDATA_FREE(transferdata);

    efree(shape_it);
    efree(dst_strides_it);
    efree(coord);
    return (needs_api) ? -1 : 0;
}

/*
 * Assigns a scalar value specified by 'src_dtype' and 'src_data'
 * to elements of 'dst'.
 *
 * dst: The destination array.
 * src_dtype: The data type of the source scalar.
 * src_data: The memory element of the source scalar.
 * wheremask: If non-NULL, a boolean mask specifying where to copy.
 * casting: An exception is raised if the assignment violates this
 *          casting rule.
 *
 * This function is implemented in array_assign_scalar.c.
 *
 * Returns 0 on success, -1 on failure.
 */
int 
CArray_AssignRawScalar(CArray *dst, CArrayDescriptor *src_dtype, char *src_data, CArray *wheremask,
                       CARRAY_CASTING casting)
{
    int allocated_src_data = 0;
    long long scalarbuffer[4];

    if (CArray_FailUnlessWriteable(dst, "assignment destination") < 0) {
        return -1;
    }

    /* Check the casting rule */
    if (!can_cast_scalar_to(src_dtype, src_data,
                            CArray_DESCR(dst), casting)) {
        throw_typeerror_exception("Cannot cast scalar");
        return -1;
    }
    
     /*
     * Make a copy of the src data if it's a different dtype than 'dst'
     * or isn't aligned, and the destination we're copying to has
     * more than one element. To avoid having to manage object lifetimes,
     * we also skip this if 'dst' has an object dtype.
     */
    if ((!CArray_EquivTypes(CArray_DESCR(dst), src_dtype) ||
         !(carray_is_aligned(src_data, carray_uint_alignment(src_dtype->elsize)) &&
         carray_is_aligned(src_data, src_dtype->alignment))) &&
         CArray_SIZE(dst) > 1) {
            char *tmp_src_data;
            /*
            * Use a static buffer to store the aligned/cast version,
            * or allocate some memory if more space is needed.
            */
            if ((int)sizeof(scalarbuffer) >= CArray_DESCR(dst)->elsize) {
                tmp_src_data = (char *)&scalarbuffer[0];
            }
            else {
                tmp_src_data = emalloc(CArray_DESCR(dst)->elsize);
                if (tmp_src_data == NULL) {
                    throw_memory_exception("Memory Error");
                    goto fail;
                }
                allocated_src_data = 1;
            }
            if (CArrayDataType_FLAGCHK(CArray_DESCR(dst), CARRAY_NEEDS_INIT)) {
                memset(tmp_src_data, 0, CArray_DESCR(dst)->elsize);
            }
            
            if (CArray_CastRawArrays(1, src_data, tmp_src_data, 0, 0,
                                src_dtype, CArray_DESCR(dst), 0) != CARRAY_SUCCEED) {
                src_data = tmp_src_data;
                goto fail;
            }

            /* Replace src_data/src_dtype */
            src_data = tmp_src_data;
            src_dtype = CArray_DESCR(dst);
    }
    
    if (wheremask == NULL) {
        /* A straightforward value assignment */
        /* Do the assignment with raw array iteration */
        if (raw_array_assign_scalar(CArray_NDIM(dst), CArray_DIMS(dst),
                CArray_DESCR(dst), CArray_DATA(dst), CArray_STRIDES(dst),
                src_dtype, src_data) < 0) {
                    
            goto fail;
        }
    }
    else {
        throw_notimplemented_exception();
    }

    //if (allocated_src_data) {
    //    free(src_data);
    //}

    return 0;
fail:

    if (allocated_src_data) {
        free(src_data);
    }
    return -1;
}