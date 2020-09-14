#ifndef PHPSCI_EXT_DTYPE_TRANSFER_H
#define PHPSCI_EXT_DTYPE_TRANSFER_H

#include "carray.h"
#include "common/common.h"
#include "common/strided_loops.h"

int CArray_CastRawArrays(int count, char *src, char *dst,
                         int src_stride, int dst_stride,
                         CArrayDescriptor *src_dtype, CArrayDescriptor *dst_dtype,
                         int move_references);

int CArray_GetDTypeTransferFunction(int aligned,
                         int src_stride, int dst_stride,
                         CArrayDescriptor *src_dtype, CArrayDescriptor *dst_dtype,
                         int move_references,
                         CArray_StridedUnaryOp **out_stransfer,
                         CArrayAuxData **out_transferdata,
                         int *out_needs_api);

int
CArray_PrepareOneRawArrayIter(int ndim, int *shape,
                              char *data, int *strides,
                              int *out_ndim, int *out_shape,
                              char **out_data, int *out_strides);
#endif //PHPSCI_EXT_DTYPE_TRANSFER_H
