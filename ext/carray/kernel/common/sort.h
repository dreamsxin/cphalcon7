#ifndef CARRAY_SORT_H
#define CARRAY_SORT_H

#include "stdio.h"
#include "string.h"

#define QS_STACK 100
#define SMALL_QUICKSORT 15
#define SMALL_MERGESORT 20
#define SMALL_STRING 16

#define CARRAY_ENOMEM 1
#define CARRAY_ECOMP 2

static inline int carray_get_msb(int unum)
{
    int depth_limit = 0;
    while (unum >>= 1)  {
        depth_limit++;
    }
    return depth_limit;
}

inline static void
GENERIC_SWAP(char *a, char *b, size_t len)
{
    while(len--) {
        const char t = *a;
        *a++ = *b;
        *b++ = t;
    }
}

inline static void
GENERIC_COPY(char *a, char *b, size_t len)
{
    memcpy(a, b, len);
}


/*
 *****************************************************************************
 **                             GENERIC SORT                                **
 *****************************************************************************
 */
int carray_quicksort(void *vec, int cnt, void *arr);
int carray_heapsort(void *vec, int cnt, void *arr);
int carray_mergesort(void *vec, int cnt, void *arr);
int carray_timsort(void *vec, int cnt, void *arr);
int carray_aquicksort(void *vec, int *ind, int cnt, void *arr);
int carray_aheapsort(void *vec, int *ind, int cnt, void *arr);
int carray_amergesort(void *vec, int *ind, int cnt, void *arr);
int carray_atimsort(void *vec, int *ind, int cnt, void *arr);

#endif //CARRAY_SORT_H
