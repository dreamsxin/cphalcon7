#ifndef PHPSCI_EXT_LINALG_H
#define PHPSCI_EXT_LINALG_H

#include "carray.h"

void FLOAT_dot(char *ip1, int is1, char *ip2, int is2, char *op, int n);
void INT_dot(char *ip1, int is1, char *ip2, int is2, char *op, int n);
void DOUBLE_dot(char *ip1, int is1, char *ip2, int is2, char *op, int n);

CArray * CArray_Matmul(CArray * ap1, CArray * ap2, CArray * out, MemoryPointer * ptr);
CArray * CArray_Inv(CArray * a, MemoryPointer * out);
CArray * CArray_Norm(CArray * a, int norm, MemoryPointer * out);
CArray * CArray_Det(CArray * a, MemoryPointer * out);
CArray * CArray_Vdot(CArray * target_a, CArray * target_b, MemoryPointer * out);
CArray ** CArray_Svd(CArray * a, int full_matrices, int compute_uv, MemoryPointer * out);
CArray * CArray_InnerProduct(CArray *a, CArray *b, MemoryPointer *out);
CArray * CArray_Solve(CArray *target_a, CArray *target_b, MemoryPointer * out);


void * linearize_DOUBLE_matrix(double *dst_in, double *src_in, CArray * a);
#endif