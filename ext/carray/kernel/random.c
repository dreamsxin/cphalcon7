#include "carray.h"
#include "random.h"
#include "matlib.h"

#ifndef RK_DEV_URANDOM
#define RK_DEV_URANDOM "/dev/urandom"
#endif

#ifndef RK_DEV_RANDOM
#define RK_DEV_RANDOM "/dev/random"
#endif

/* Magic Mersenne Twister constants */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL
#define UPPER_MASK 0x80000000UL
#define LOWER_MASK 0x7fffffffUL

/* Thomas Wang 32 bits integer hash function */
unsigned long
rk_hash(unsigned long key)
{
    key += ~(key << 15);
    key ^=  (key >> 10);
    key +=  (key << 3);
    key ^=  (key >> 6);
    key += ~(key << 11);
    key ^=  (key >> 16);
    return key;
}

rk_error
rk_devfill(void *buffer, size_t size, int strong)
{
#ifndef _WIN32
    FILE *rfile;
    int done;

    if (strong) {
        rfile = fopen(RK_DEV_RANDOM, "rb");
    }
    else {
        rfile = fopen(RK_DEV_URANDOM, "rb");
    }
    if (rfile == NULL) {
        return RK_ENODEV;
    }
    done = fread(buffer, size, 1, rfile);
    fclose(rfile);
    if (done) {
        return RK_NOERR;
    }
#else

#ifndef RK_NO_WINCRYPT
    HCRYPTPROV hCryptProv;
    BOOL done;

    if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL,
            CRYPT_VERIFYCONTEXT) || !hCryptProv) {
        return RK_ENODEV;
    }
    done = CryptGenRandom(hCryptProv, size, (unsigned char *)buffer);
    CryptReleaseContext(hCryptProv, 0);
    if (done) {
        return RK_NOERR;
    }
#endif

#endif
    return RK_ENODEV;
}

/*
 * Slightly optimised reference implementation of the Mersenne Twister
 * Note that regardless of the precision of long, only 32 bit random
 * integers are produced
 */
unsigned long
rk_random(rk_state *state)
{
    unsigned long y;

    if (state->pos == RK_STATE_LEN) {
        int i;

        for (i = 0; i < N - M; i++) {
            y = (state->key[i] & UPPER_MASK) | (state->key[i+1] & LOWER_MASK);
            state->key[i] = state->key[i+M] ^ (y>>1) ^ (-(y & 1) & MATRIX_A);
        }
        for (; i < N - 1; i++) {
            y = (state->key[i] & UPPER_MASK) | (state->key[i+1] & LOWER_MASK);
            state->key[i] = state->key[i+(M-N)] ^ (y>>1) ^ (-(y & 1) & MATRIX_A);
        }
        y = (state->key[N - 1] & UPPER_MASK) | (state->key[0] & LOWER_MASK);
        state->key[N - 1] = state->key[M - 1] ^ (y >> 1) ^ (-(y & 1) & MATRIX_A);

        state->pos = 0;
    }
    y = state->key[state->pos++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

double
rk_double(rk_state *state)
{
    /* shifts : 67108864 = 0x4000000, 9007199254740992 = 0x20000000000000 */
    long a = rk_random(state) >> 5, b = rk_random(state) >> 6;
    return (a * 67108864.0 + b) / 9007199254740992.0;
}

void
rk_seed(unsigned long seed, rk_state *state)
{
    int pos;
    seed &= 0xffffffffUL;

    /* Knuth's PRNG as used in the Mersenne Twister reference implementation */
    for (pos = 0; pos < RK_STATE_LEN; pos++) {
        state->key[pos] = seed;
        seed = (1812433253UL * (seed ^ (seed >> 30)) + pos + 1) & 0xffffffffUL;
    }
    state->pos = RK_STATE_LEN;
    state->gauss = 0;
    state->has_gauss = 0;
    state->has_binomial = 0;
}


rk_error
rk_randomseed(rk_state *state)
{
#ifndef _WIN32
    struct timeval tv;
#else
    struct _timeb  tv;
#endif
    int i;

    if (rk_devfill(state->key, sizeof(state->key), 0) == RK_NOERR) {
        /* ensures non-zero key */
        state->key[0] |= 0x80000000UL;
        state->pos = RK_STATE_LEN;
        state->gauss = 0;
        state->has_gauss = 0;
        state->has_binomial = 0;

        for (i = 0; i < 624; i++) {
            state->key[i] &= 0xffffffffUL;
        }
        return RK_NOERR;
    }

#ifndef _WIN32
    gettimeofday(&tv, NULL);
    rk_seed(rk_hash(getpid()) ^ rk_hash(tv.tv_sec) ^ rk_hash(tv.tv_usec)
            ^ rk_hash(clock()), state);
#else
    _FTIME(&tv);
    rk_seed(rk_hash(tv.time) ^ rk_hash(tv.millitm) ^ rk_hash(clock()), state);
#endif

    return RK_ENODEV;
}

CArray *
CArray_Rand(int * size, int nd, MemoryPointer * out)
{
    rk_state * state = emalloc(sizeof(rk_state));
    double * array_data;
    CArray * target;
    int length;
    int i;
    target = CArray_Zeros(size, nd, TYPE_DOUBLE, NULL, out);
    
    length = CArray_SIZE(target);
    array_data = (double *)CArray_DATA(target);

    rk_randomseed(state);    
    for(i = 0; i < length; i++) {
        array_data[i] = rk_double(state);
    }
    efree(state);
    return target;
}