#include "compare.h"
#include "../carray.h"

int
INT_compare (int *pa, int *pb, CArray *CARRAY_UNUSED(ap))
{
    const int a = *pa;
    const int b = *pb;

    return a < b ? -1 : a == b ? 0 : 1;
}

#define LT(a,b) ((a) < (b) || ((b) != (b) && (a) ==(a)))

int
DOUBLE_compare(double *pa, double *pb)
{
    const double a = *pa;
    const double b = *pb;
    int ret;

    if (LT(a,b)) {
        ret = -1;
    }
    else if (LT(b,a)) {
        ret = 1;
    }
    else {
        ret = 0;
    }
    return ret;
}
