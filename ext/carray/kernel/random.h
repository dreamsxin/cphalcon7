#ifndef PHPSCI_EXT_RANDOM_H
#define PHPSCI_EXT_RANDOM_H

#include "carray.h"

#define RK_STATE_LEN 624

typedef struct rk_state_
{
    unsigned long key[RK_STATE_LEN];
    int pos;
    int has_gauss; /* !=0: gauss contains a gaussian deviate */
    double gauss;

    /* The rk_state structure has been extended to store the following
     * information for the binomial generator. If the input values of n or p
     * are different than nsave and psave, then the other parameters will be
     * recomputed. RTK 2005-09-02 */

    int has_binomial; /* !=0: following parameters initialized for
                              binomial */
    double psave;
    long nsave;
    double r;
    double q;
    double fm;
    long m;
    double p1;
    double xm;
    double xl;
    double xr;
    double c;
    double laml;
    double lamr;
    double p2;
    double p3;
    double p4;

}
rk_state;

typedef enum {
    RK_NOERR = 0, /* no error */
    RK_ENODEV = 1, /* no RK_DEV_RANDOM device */
    RK_ERR_MAX = 2
} rk_error;

unsigned long rk_random(rk_state *state);

/*
 * Returns a random double between 0.0 and 1.0, 1.0 excluded.
 */
double rk_double(rk_state *state);

void rk_seed(unsigned long seed, rk_state *state);
CArray * CArray_Rand(int * size, int nd, MemoryPointer * out);

#endif //PHPSCI_EXT_RANDOM_H
