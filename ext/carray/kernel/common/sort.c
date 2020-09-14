#include "sort.h"
#include "../carray.h"

int
carray_heapsort(void *start, int num, void *varr)
{
    CArray *arr = varr;
    int elsize = CArray_ITEMSIZE(arr);
    CArray_CompareFunc *cmp = CArray_DESCR(arr)->f->compare;
    char *tmp = emalloc(elsize);
    char *a = (char *)start - elsize;
    int i, j, l;

    if (tmp == NULL) {
        return -CARRAY_ENOMEM;
    }

    for (l = num >> 1; l > 0; --l) {
        GENERIC_COPY(tmp, a + l*elsize, elsize);
        for (i = l, j = l << 1; j <= num;) {
            if (j < num && cmp(a + j*elsize, a + (j+1)*elsize, arr) < 0) {
                ++j;
            }
            if (cmp(tmp, a + j*elsize, arr) < 0) {
                GENERIC_COPY(a + i*elsize, a + j*elsize, elsize);
                i = j;
                j += j;
            }
            else {
                break;
            }
        }
        GENERIC_COPY(a + i*elsize, tmp, elsize);
    }

    for (; num > 1;) {
        GENERIC_COPY(tmp, a + num*elsize, elsize);
        GENERIC_COPY(a + num*elsize, a + elsize, elsize);
        num -= 1;
        for (i = 1, j = 2; j <= num;) {
            if (j < num && cmp(a + j*elsize, a + (j+1)*elsize, arr) < 0) {
                ++j;
            }
            if (cmp(tmp, a + j*elsize, arr) < 0) {
                GENERIC_COPY(a + i*elsize, a + j*elsize, elsize);
                i = j;
                j += j;
            }
            else {
                break;
            }
        }
        GENERIC_COPY(a + i*elsize, tmp, elsize);
    }

    efree(tmp);
    return 0;
}

int
carray_quicksort(void *start, int num, void *varr)
{
    CArray *arr = varr;
    int elsize = CArray_ITEMSIZE(arr);
    CArray_CompareFunc *cmp = CArray_DESCR(arr)->f->compare;
    char *vp;
    char *pl = start;
    char *pr = pl + (num - 1)*elsize;
    char *stack[QS_STACK];
    char **sptr = stack;
    char *pm, *pi, *pj, *pk;
    int depth[QS_STACK];
    int * psdepth = depth;
    int cdepth = carray_get_msb(num) * 2;

    /* Items that have zero size don't make sense to sort */
    if (elsize == 0) {
        return 0;
    }

    vp = emalloc(elsize);
    if (vp == NULL) {
        return -CARRAY_ENOMEM;
    }

    for (;;) {

        if (CARRAY_UNLIKELY(cdepth < 0)) {
            carray_heapsort(pl, (pr - pl) / elsize + 1, varr);
            goto stack_pop;
        }
        while(pr - pl > SMALL_QUICKSORT*elsize) {

            /* quicksort partition */
            pm = pl + (((pr - pl) / elsize) >> 1) * elsize;
            if (cmp(pm, pl, arr) < 0) {
                GENERIC_SWAP(pm, pl, elsize);
            }
            if (cmp(pr, pm, arr) < 0) {
                GENERIC_SWAP(pr, pm, elsize);
            }
            if (cmp(pm, pl, arr) < 0) {
                GENERIC_SWAP(pm, pl, elsize);
            }
            GENERIC_COPY(vp, pm, elsize);
            pi = pl;
            pj = pr - elsize;
            GENERIC_SWAP(pm, pj, elsize);
            /*
             * Generic comparisons may be buggy, so don't rely on the sentinels
             * to keep the pointers from going out of bounds.
             */
            for (;;) {
                do {
                    pi += elsize;
                } while (cmp(pi, vp, arr) < 0 && pi < pj);
                do {
                    pj -= elsize;
                } while (cmp(vp, pj, arr) < 0 && pi < pj);
                if (pi >= pj) {
                    break;
                }
                GENERIC_SWAP(pi, pj, elsize);
            }
            pk = pr - elsize;
            GENERIC_SWAP(pi, pk, elsize);
            /* push largest partition on stack */
            if (pi - pl < pr - pi) {
                *sptr++ = pi + elsize;
                *sptr++ = pr;
                pr = pi - elsize;
            }
            else {
                *sptr++ = pl;
                *sptr++ = pi - elsize;
                pl = pi + elsize;
            }
            *psdepth++ = --cdepth;
        }

        /* insertion sort */
        for (pi = pl + elsize; pi <= pr; pi += elsize) {
            GENERIC_COPY(vp, pi, elsize);
            pj = pi;
            pk = pi - elsize;
            while (pj > pl && cmp(vp, pk, arr) < 0) {
                GENERIC_COPY(pj, pk, elsize);
                pj -= elsize;
                pk -= elsize;
            }
            GENERIC_COPY(pj, vp, elsize);
        }
stack_pop:
        if (sptr == stack) {
            break;
        }
        pr = *(--sptr);
        pl = *(--sptr);
        cdepth = *(--psdepth);
    }

    efree(vp);
    return 0;
}

static void
carray_mergesort0(char *pl, char *pr, char *pw, char *vp, int elsize,
               CArray_CompareFunc *cmp, CArray *arr)
{
    char *pi, *pj, *pk, *pm;

    if (pr - pl > SMALL_MERGESORT*elsize) {
        /* merge sort */
        pm = pl + (((pr - pl)/elsize) >> 1)*elsize;
        carray_mergesort0(pl, pm, pw, vp, elsize, cmp, arr);
        carray_mergesort0(pm, pr, pw, vp, elsize, cmp, arr);
        GENERIC_COPY(pw, pl, pm - pl);
        pi = pw + (pm - pl);
        pj = pw;
        pk = pl;
        while (pj < pi && pm < pr) {
            if (cmp(pm, pj, arr) < 0) {
                GENERIC_COPY(pk, pm, elsize);
                pm += elsize;
                pk += elsize;
            }
            else {
                GENERIC_COPY(pk, pj, elsize);
                pj += elsize;
                pk += elsize;
            }
        }
        GENERIC_COPY(pk, pj, pi - pj);
    }
    else {
        /* insertion sort */
        for (pi = pl + elsize; pi < pr; pi += elsize) {
            GENERIC_COPY(vp, pi, elsize);
            pj = pi;
            pk = pi - elsize;
            while (pj > pl && cmp(vp, pk, arr) < 0) {
                GENERIC_COPY(pj, pk, elsize);
                pj -= elsize;
                pk -= elsize;
            }
            GENERIC_COPY(pj, vp, elsize);
        }
    }
}

int
carray_mergesort(void *start, int num, void *varr)
{
    CArray *arr = varr;
    int elsize = CArray_ITEMSIZE(arr);
    CArray_CompareFunc *cmp = CArray_DESCR(arr)->f->compare;
    char *pl = start;
    char *pr = pl + num*elsize;
    char *pw;
    char *vp;
    int err = -CARRAY_ENOMEM;

    /* Items that have zero size don't make sense to sort */
    if (elsize == 0) {
        return 0;
    }

    pw = emalloc((num >> 1) *elsize);
    vp = emalloc(elsize);

    if (pw != NULL && vp != NULL) {
        carray_mergesort0(pl, pr, pw, vp, elsize, cmp, arr);
        err = 0;
    }

    efree(vp);
    efree(pw);

    return err;
}
