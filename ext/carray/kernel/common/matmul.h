#ifndef CARRAY_MATMUL_H
#define CARRAY_MATMUL_H

#include "../carray.h"

void DOUBLE_matmul(char **args, int *dimensions, int *steps, void *CARRAY_UNUSED(func));
void INT_matmul(char **args, int *dimensions, int *steps, void *CARRAY_UNUSED(func));
#endif //CARRAY_MATMUL_H
