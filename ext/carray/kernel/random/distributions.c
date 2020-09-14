#include "../carray.h"
#include "distributions.h"
#include "../buffer.h"

/**
 * Poisson Random Distribution
 *
 * @param m
 * @param n
 * @param lambda
 * @return
 */
CArray*
CArray_Poisson(int *shape, double lambda, MemoryPointer *out)
{
    CArray *result;
    CArrayDescriptor *descr;
    double max = INT_MAX;
    time_t t;
    double l = exp(-lambda);
    double k;
    double p;
    int i;

    result = emalloc(sizeof(CArray));
    descr = CArray_DescrFromType(TYPE_DOUBLE_INT);
    result = CArray_NewFromDescr_int(result, descr, 2, shape, NULL, NULL,
                                     CARRAY_ARRAY_C_CONTIGUOUS, NULL, 1, 0);

    // Random Seed
    srand((unsigned) time(&t));

    for (i = 0; i < CArray_DESCR(result)->numElements; i++) {
        k = 0.0;
        p = 1.0;

        while (p > l) {
           k = k + 1.0;
           p *= rand() / max;
        }
        DDATA(result)[i] = (double) k - 1;
    }

    if (out != NULL) {
        add_to_buffer(out, result, sizeof(result));
    }

    return result;
}
