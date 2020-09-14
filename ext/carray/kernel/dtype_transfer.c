#include "carray.h"
#include "dtype_transfer.h"
#include "common/strided_loops.h"
#include "assign.h"
#include "shape.h"
#include "common/common.h"
#include "common/exceptions.h"
#include "php.h"
#include "descriptor.h"
#include "common/strided_loops.h"

/*************************** DTYPE CAST FUNCTIONS *************************/

/* Does a simple aligned cast */
typedef struct {
    CArrayAuxData base;
    CArray_VectorUnaryFunc *castfunc;
    CArray *aip, *aop;
} _strided_cast_data;

static int
get_nbo_cast_numeric_transfer_function(int aligned,
                            int src_stride, int dst_stride,
                            int src_type_num, int dst_type_num,
                            CArray_StridedUnaryOp **out_stransfer,
                            CArrayAuxData **out_transferdata)
{
    *out_stransfer = CArray_GetStridedNumericCastFn(aligned,
                                src_stride, dst_stride,
                                src_type_num, dst_type_num);
    *out_transferdata = NULL;
    if (*out_stransfer == NULL) {
        throw_typeerror_exception("unexpected error in GetStridedNumericCastFn");
        return CARRAY_FAIL;
    }

    return CARRAY_SUCCEED;
}

static int
get_nbo_cast_transfer_function(int aligned,
                            int src_stride, int dst_stride,
                            CArrayDescriptor *src_dtype, CArrayDescriptor *dst_dtype,
                            int move_references,
                            CArray_StridedUnaryOp **out_stransfer,
                            CArrayAuxData **out_transferdata,
                            int *out_needs_api,
                            int *out_needs_wrap)
{
    _strided_cast_data *data;
    CArray_VectorUnaryFunc *castfunc;
    CArrayDescriptor *tmp_dtype;
    int shape = 1, src_itemsize = src_dtype->elsize, dst_itemsize = dst_dtype->elsize;

    return get_nbo_cast_numeric_transfer_function(aligned,
                                    src_stride, dst_stride,
                                    src_dtype->type_num, dst_dtype->type_num,
                                    out_stransfer, out_transferdata);
}

static int
get_cast_transfer_function(int aligned,
                            int src_stride, int dst_stride,
                            CArrayDescriptor *src_dtype, CArrayDescriptor *dst_dtype,
                            int move_references,
                            CArray_StridedUnaryOp **out_stransfer,
                            CArrayAuxData **out_transferdata,
                            int *out_needs_api)
{
    CArray_StridedUnaryOp *caststransfer;
    CArrayAuxData *castdata, *todata = NULL, *fromdata = NULL;
    int needs_wrap = 0;
    int src_itemsize = src_dtype->elsize, dst_itemsize = dst_dtype->elsize;
    
    if (get_nbo_cast_transfer_function(aligned,
                            src_stride, dst_stride,
                            src_dtype, dst_dtype,
                            move_references,
                            &caststransfer,
                            &castdata,
                            out_needs_api,
                            &needs_wrap) != CARRAY_SUCCEED) {
        return CARRAY_FAIL;
    }

    if (!needs_wrap) {
        *out_stransfer = caststransfer;
        *out_transferdata = castdata;

        return CARRAY_SUCCEED;
    }
}


/*
 * Prepares shape and strides for a simple raw array iteration.
 * This sorts the strides into FORTRAN order, reverses any negative
 * strides, then coalesces axes where possible. The results are
 * filled in the output parameters.
 *
 * This is intended for simple, lightweight iteration over arrays
 * where no buffering of any kind is needed.
 *
 * The arrays shape, out_shape, strides, and out_strides must all
 * point to different data.
 *
 * Returns 0 on success, -1 on failure.
 */
int
CArray_PrepareOneRawArrayIter(int ndim, int *shape,
                              char *data, int *strides,
                              int *out_ndim, int *out_shape,
                              char **out_data, int *out_strides)
{
    ca_stride_sort_item * strideperm = emalloc(sizeof(ca_stride_sort_item) * ndim);
    int i, j;

    /* Special case 0 and 1 dimensions */
    if (ndim == 0) {
        *out_ndim = 1;
        *out_data = data;
        out_shape[0] = 1;
        out_strides[0] = 0;
        return 0;
    }
    else if (ndim == 1) {
        int stride_entry = strides[0], shape_entry = shape[0];
        *out_ndim = 1;
        out_shape[0] = shape[0];
        /* Always make a positive stride */
        if (stride_entry >= 0) {
            *out_data = data;
            out_strides[0] = stride_entry;
        }
        else {
            *out_data = data + stride_entry * (shape_entry - 1);
            out_strides[0] = -stride_entry;
        }
        efree(strideperm);
        return 0;
    }

    /* Sort the axes based on the destination strides */
    CArray_CreateSortedStridePerm(ndim, strides, strideperm);
    for (i = 0; i < ndim; ++i) {
        int iperm = strideperm[ndim - i - 1].perm;
        out_shape[i] = shape[iperm];
        out_strides[i] = strides[iperm];
    }

    /* Reverse any negative strides */
    for (i = 0; i < ndim; ++i) {
        int stride_entry = out_strides[i], shape_entry = out_shape[i];

        if (stride_entry < 0) {
            data += stride_entry * (shape_entry - 1);
            out_strides[i] = -stride_entry;
        }
        /* Detect 0-size arrays here */
        if (shape_entry == 0) {
            *out_ndim = 1;
            *out_data = data;
            out_shape[0] = 0;
            out_strides[0] = 0;
            return 0;
        }
    }

    /* Coalesce any dimensions where possible */
    i = 0;
    for (j = 1; j < ndim; ++j) {
        if (out_shape[i] == 1) {
            /* Drop axis i */
            out_shape[i] = out_shape[j];
            out_strides[i] = out_strides[j];
        }
        else if (out_shape[j] == 1) {
            /* Drop axis j */
        }
        else if (out_strides[i] * out_shape[i] == out_strides[j]) {
            /* Coalesce axes i and j */
            out_shape[i] *= out_shape[j];
        }
        else {
            /* Can't coalesce, go to next i */
            ++i;
            out_shape[i] = out_shape[j];
            out_strides[i] = out_strides[j];
        }
    }
    ndim = i+1;

#if 0
    /* DEBUG */
    {
        printf("raw iter ndim %d\n", ndim);
        printf("shape: ");
        for (i = 0; i < ndim; ++i) {
            printf("%d ", (int)out_shape[i]);
        }
        printf("\n");
        printf("strides: ");
        for (i = 0; i < ndim; ++i) {
            printf("%d ", (int)out_strides[i]);
        }
        printf("\n");
    }
#endif
    efree(strideperm);
    *out_data = data;
    *out_ndim = ndim;
    return 0;
}

/*************************** DEST SETZERO *******************************/

/* Sets dest to zero */
typedef struct {
    CArrayAuxData base;
    int dst_itemsize;
} _dst_memset_zero_data;

static void
_null_to_strided_memset_zero(char *dst,
                        int dst_stride,
                        char *CARRAY_UNUSED(src), int CARRAY_UNUSED(src_stride),
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *data)
{
    _dst_memset_zero_data *d = (_dst_memset_zero_data *)data;
    int dst_itemsize = d->dst_itemsize;

    while (N > 0) {
        memset(dst, 0, dst_itemsize);
        dst += dst_stride;
        --N;
    }
}

/* zero-padded data copy function */
static CArrayAuxData *_dst_memset_zero_data_clone(CArrayAuxData *data)
{
    _dst_memset_zero_data *newdata =
            (_dst_memset_zero_data *)emalloc(
                                    sizeof(_dst_memset_zero_data));
    if (newdata == NULL) {
        return NULL;
    }

    memcpy(newdata, data, sizeof(_dst_memset_zero_data));

    return (CArrayAuxData *)newdata;
}

static void
_null_to_contig_memset_zero(char *dst,
                        int dst_stride,
                        char *CARRAY_UNUSED(src), int CARRAY_UNUSED(src_stride),
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *data)
{
    _dst_memset_zero_data *d = (_dst_memset_zero_data *)data;
    int dst_itemsize = d->dst_itemsize;
    memset(dst, 0, N*dst_itemsize);
}

static void
_dec_src_ref_nop(char *CARRAY_UNUSED(dst),
                        int CARRAY_UNUSED(dst_stride),
                        char *CARRAY_UNUSED(src), int CARRAY_UNUSED(src_stride),
                        int CARRAY_UNUSED(N),
                        int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
    /* NOP */
}

/*
 * Returns a transfer function which zeros out the dest values.
 *
 * Returns CARRAY_SUCCEED or CARRAY_FAIL.
 */
static int
get_setdstzero_transfer_function(int aligned,
                            int dst_stride,
                            CArrayDescriptor *dst_dtype,
                            CArray_StridedUnaryOp **out_stransfer,
                            CArrayAuxData **out_transferdata,
                            int *out_needs_api);

/*
 * Returns a transfer function which DECREFs any references in src_type.
 *
 * Returns CARRAY_SUCCEED or CARRAY_FAIL.
 */
static int
get_decsrcref_transfer_function(int aligned,
                                int src_stride,
                                CArrayDescriptor *src_dtype,
                                CArray_StridedUnaryOp **out_stransfer,
                                CArrayAuxData **out_transferdata,
                                int *out_needs_api);


int 
CArray_CastRawArrays(int count, char *src, char *dst,
                     int src_stride, int dst_stride,
                     CArrayDescriptor *src_dtype, CArrayDescriptor *dst_dtype,
                     int move_references)
{
    CArray_StridedUnaryOp *stransfer = NULL;
    CArrayAuxData *transferdata = NULL;
    int aligned = 1, needs_api = 0;

    
    /* Make sure the copy is reasonable */
    if (dst_stride == 0 && count > 1) {
        throw_valueerror_exception("CArray CastRawArrays cannot do a reduction");
        return CARRAY_FAIL;
    }
    else if (count == 0) {
        return CARRAY_SUCCEED;
    }

    /* Check data alignment, both uint and true */
    aligned = raw_array_is_aligned(1, &count, dst, &dst_stride,
                                   carray_uint_alignment(dst_dtype->elsize)) &&
              raw_array_is_aligned(1, &count, dst, &dst_stride,
                                   dst_dtype->alignment) &&
              raw_array_is_aligned(1, &count, src, &src_stride,
                                   carray_uint_alignment(src_dtype->elsize)) &&
              raw_array_is_aligned(1, &count, src, &src_stride,
                                   src_dtype->alignment);

    /* Get the function to do the casting */
    if (CArray_GetDTypeTransferFunction(aligned,
                        src_stride, dst_stride,
                        src_dtype, dst_dtype,
                        move_references,
                        &stransfer, &transferdata,
                        &needs_api) != CARRAY_SUCCEED) {
        return CARRAY_FAIL;
    }           

    /* Cast */
    stransfer(dst, dst_stride, src, src_stride, count,
              src_dtype->elsize, transferdata);

    CARRAY_AUXDATA_FREE(transferdata);
    
    return (needs_api) ? CARRAY_FAIL : CARRAY_SUCCEED;
}


/********************* MAIN DTYPE TRANSFER FUNCTION ***********************/
int
CArray_GetDTypeTransferFunction(int aligned,
                            int src_stride, int dst_stride,
                            CArrayDescriptor *src_dtype, CArrayDescriptor *dst_dtype,
                            int move_references,
                            CArray_StridedUnaryOp **out_stransfer,
                            CArrayAuxData **out_transferdata,
                            int *out_needs_api)
{
    int src_itemsize, dst_itemsize;
    int src_type_num, dst_type_num;
    int is_builtin;
    
    /*
     * If one of the dtypes is NULL, we give back either a src decref
     * function or a dst setzero function
     */
    if (dst_dtype == NULL) {
        if (move_references) {
            return get_decsrcref_transfer_function(aligned,
                                src_dtype->elsize,
                                src_dtype,
                                out_stransfer, out_transferdata,
                                out_needs_api);
        }
        else {
            *out_stransfer = &_dec_src_ref_nop;
            *out_transferdata = NULL;
            return CARRAY_SUCCEED;
        }
    }
    else if (src_dtype == NULL) {
        return get_setdstzero_transfer_function(aligned,
                                dst_dtype->elsize,
                                dst_dtype,
                                out_stransfer, out_transferdata,
                                out_needs_api);
    }

    src_itemsize = src_dtype->elsize;
    dst_itemsize = dst_dtype->elsize;
    src_type_num = src_dtype->type_num;
    dst_type_num = dst_dtype->type_num;
    is_builtin = src_type_num < CARRAY_NTYPES && dst_type_num < CARRAY_NTYPES;
    //php_printf("%d", src_dtype->elsize);
    /*
     * If there are no references and the data types are equivalent and builtin,
     * return a simple copy
     */
    if (CArray_EquivTypes(src_dtype, dst_dtype) &&
            !CArrayDataType_REFCHK(src_dtype) && !CArrayDataType_REFCHK(dst_dtype) &&
            is_builtin) {
                
        /*
         * We can't pass through the aligned flag because it's not
         * appropriate. Consider a size-8 string, it will say it's
         * aligned because strings only need alignment 1, but the
         * copy function wants to know if it's alignment 8.
         *
         * TODO: Change align from a flag to a "best power of 2 alignment"
         *       which holds the strongest alignment value for all
         *       the data which will be used.
         */
        *out_stransfer = CArray_GetStridedCopyFn(0,
                                        src_stride, dst_stride,
                                        src_dtype->elsize);                           
        *out_transferdata = NULL;
        return CARRAY_SUCCEED;
    }

    return get_cast_transfer_function(aligned,
                    src_stride, dst_stride,
                    src_dtype, dst_dtype,
                    move_references,
                    out_stransfer, out_transferdata,
                    out_needs_api);
}



int
get_decsrcref_transfer_function(int aligned,
                                int src_stride,
                                CArrayDescriptor *src_dtype,
                                CArray_StridedUnaryOp **out_stransfer,
                                CArrayAuxData **out_transferdata,
                                int *out_needs_api)
{
     /* If there are no references, it's a nop */
    if (!CArrayDataType_REFCHK(src_dtype)) {
        *out_stransfer = &_dec_src_ref_nop;
        *out_transferdata = NULL;

        return CARRAY_SUCCEED;
    }
    throw_notimplemented_exception();
}                                                               

int
get_setdstzero_transfer_function(int aligned,
                                 int dst_stride,
                                 CArrayDescriptor *dst_dtype,
                                 CArray_StridedUnaryOp **out_stransfer,
                                 CArrayAuxData **out_transferdata,
                                 int *out_needs_api)
{
    _dst_memset_zero_data *data;
    /* If there are no references, just set the whole thing to zero */
    if (!CArrayDataType_REFCHK(dst_dtype)) {
        data = (_dst_memset_zero_data *)
                        emalloc(sizeof(_dst_memset_zero_data));
        if (data == NULL) {
            throw_memory_exception("Memory Error");
            return CARRAY_FAIL;
        }

        data->base.free = (CArrayAuxData_FreeFunc *)(&free);
        data->base.clone = &_dst_memset_zero_data_clone;
        data->dst_itemsize = dst_dtype->elsize;

        if (dst_stride == data->dst_itemsize) {
            *out_stransfer = &_null_to_contig_memset_zero;
        }
        else {
            *out_stransfer = &_null_to_strided_memset_zero;
        }
        *out_transferdata = (CArrayAuxData *)data;

        return CARRAY_SUCCEED;
    }
    throw_notimplemented_exception();
}
