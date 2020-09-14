#ifndef PHPSCI_EXT_CA_EXTINT128_H
#define PHPSCI_EXT_CA_EXTINT128_H

#include "stdint.h"
#include "common.h"

typedef struct {
    signed char sign;
    int64_t lo, hi;
} extint128_t;

/* Integer addition with overflow checking */
static inline uint64_t
safe_add(uint64_t a, uint64_t b, char *overflow_flag)
{
    if (a > 0 && b > CARRAY_MAX_INT64 - a) {
        *overflow_flag = 1;
    }
    else if (a < 0 && b < CARRAY_MIN_INT64 - a) {
        *overflow_flag = 1;
    }
    return a + b;
}


/* Integer subtraction with overflow checking */
static inline uint64_t
safe_sub(uint64_t a, uint64_t b, char *overflow_flag)
{
    if (a >= 0 && b < a - CARRAY_MAX_INT64) {
        *overflow_flag = 1;
    }
    else if (a < 0 && b > a - CARRAY_MIN_INT64) {
        *overflow_flag = 1;
    }
    return a - b;
}


/* Integer multiplication with overflow checking */
static inline uint64_t
safe_mul(uint64_t a, uint64_t b, char *overflow_flag)
{
    if (a > 0) {
        if (b > CARRAY_MAX_INT64 / a || b < CARRAY_MIN_INT64 / a) {
            *overflow_flag = 1;
        }
    }
    else if (a < 0) {
        if (b > 0 && a < CARRAY_MIN_INT64 / b) {
            *overflow_flag = 1;
        }
        else if (b < 0 && a < CARRAY_MAX_INT64 / b) {
            *overflow_flag = 1;
        }
    }
    return a * b;
}


/* Long integer init */
static inline extint128_t
to_128(uint64_t x)
{
    extint128_t result;
    result.sign = (x >= 0 ? 1 : -1);
    if (x >= 0) {
        result.lo = x;
    }
    else {
        result.lo = (uint64_t)(-(x + 1)) + 1;
    }
    result.hi = 0;
    return result;
}


static inline int64_t
to_64(extint128_t x, char *overflow)
{
    if (x.hi != 0 ||
        (x.sign > 0 && x.lo > CARRAY_MAX_INT64) ||
        (x.sign < 0 && x.lo != 0 && x.lo - 1 > -(CARRAY_MIN_INT64 + 1))) {
        *overflow = 1;
    }
    return x.lo * x.sign;
}


/* Long integer multiply */
static inline extint128_t
mul_64_64(int64_t a, int64_t b)
{
    extint128_t x, y, z;
    uint64_t x1, x2, y1, y2, r1, r2, prev;

    x = to_128(a);
    y = to_128(b);

    x1 = x.lo & 0xffffffff;
    x2 = x.lo >> 32;

    y1 = y.lo & 0xffffffff;
    y2 = y.lo >> 32;

    r1 = x1*y2;
    r2 = x2*y1;

    z.sign = x.sign * y.sign;
    z.hi = x2*y2 + (r1 >> 32) + (r2 >> 32);
    z.lo = x1*y1;

    /* Add with carry */
    prev = z.lo;
    z.lo += (r1 << 32);
    if (z.lo < prev) {
        ++z.hi;
    }

    prev = z.lo;
    z.lo += (r2 << 32);
    if (z.lo < prev) {
        ++z.hi;
    }

    return z;
}


/* Long integer add */
static inline extint128_t
add_128(extint128_t x, extint128_t y, char *overflow)
{
    extint128_t z;

    if (x.sign == y.sign) {
        z.sign = x.sign;
        z.hi = x.hi + y.hi;
        if (z.hi < x.hi) {
            *overflow = 1;
        }
        z.lo = x.lo + y.lo;
        if (z.lo < x.lo) {
            if (z.hi == CARRAY_MAX_UINT64) {
                *overflow = 1;
            }
            ++z.hi;
        }
    }
    else if (x.hi > y.hi || (x.hi == y.hi && x.lo >= y.lo)) {
        z.sign = x.sign;
        z.hi = x.hi - y.hi;
        z.lo = x.lo;
        z.lo -= y.lo;
        if (z.lo > x.lo) {
            --z.hi;
        }
    }
    else {
        z.sign = y.sign;
        z.hi = y.hi - x.hi;
        z.lo = y.lo;
        z.lo -= x.lo;
        if (z.lo > y.lo) {
            --z.hi;
        }
    }

    return z;
}


/* Long integer negation */
static inline extint128_t
neg_128(extint128_t x)
{
    extint128_t z = x;
    z.sign *= -1;
    return z;
}


static inline extint128_t
sub_128(extint128_t x, extint128_t y, char *overflow)
{
    return add_128(x, neg_128(y), overflow);
}


static inline extint128_t
shl_128(extint128_t v)
{
    extint128_t z;
    z = v;
    z.hi <<= 1;
    z.hi |= (z.lo & (((uint64_t)1) << 63)) >> 63;
    z.lo <<= 1;
    return z;
}


static inline extint128_t
shr_128(extint128_t v)
{
    extint128_t z;
    z = v;
    z.lo >>= 1;
    z.lo |= (z.hi & 0x1) << 63;
    z.hi >>= 1;
    return z;
}

static inline int
gt_128(extint128_t a, extint128_t b)
{
    if (a.sign > 0 && b.sign > 0) {
        return (a.hi > b.hi) || (a.hi == b.hi && a.lo > b.lo);
    }
    else if (a.sign < 0 && b.sign < 0) {
        return (a.hi < b.hi) || (a.hi == b.hi && a.lo < b.lo);
    }
    else if (a.sign > 0 && b.sign < 0) {
        return a.hi != 0 || a.lo != 0 || b.hi != 0 || b.lo != 0;
    }
    else {
        return 0;
    }
}


/* Long integer divide */
static inline extint128_t
divmod_128_64(extint128_t x, int64_t b, int64_t *mod)
{
    extint128_t remainder, pointer, result, divisor;
    char overflow = 0;

    assert(b > 0);

    if (b <= 1 || x.hi == 0) {
        result.sign = x.sign;
        result.lo = x.lo / b;
        result.hi = x.hi / b;
        *mod = x.sign * (x.lo % b);
        return result;
    }

    /* Long division, not the most efficient choice */
    remainder = x;
    remainder.sign = 1;

    divisor.sign = 1;
    divisor.hi = 0;
    divisor.lo = b;

    result.sign = 1;
    result.lo = 0;
    result.hi = 0;

    pointer.sign = 1;
    pointer.lo = 1;
    pointer.hi = 0;

    while ((divisor.hi & (((uint64_t)1) << 63)) == 0 &&
           gt_128(remainder, divisor)) {
        divisor = shl_128(divisor);
        pointer = shl_128(pointer);
    }

    while (pointer.lo || pointer.hi) {
        if (!gt_128(divisor, remainder)) {
            remainder = sub_128(remainder, divisor, &overflow);
            result = add_128(result, pointer, &overflow);
        }
        divisor = shr_128(divisor);
        pointer = shr_128(pointer);
    }

    /* Fix signs and return; cannot overflow */
    result.sign = x.sign;
    *mod = x.sign * remainder.lo;

    return result;
}


/* Divide and round down (positive divisor; no overflows) */
static inline extint128_t
floordiv_128_64(extint128_t a, int64_t b)
{
    extint128_t result;
    int64_t remainder;
    char overflow = 0;
    assert(b > 0);
    result = divmod_128_64(a, b, &remainder);
    if (a.sign < 0 && remainder != 0) {
        result = sub_128(result, to_128(1), &overflow);
    }
    return result;
}


/* Divide and round up (positive divisor; no overflows) */
static inline extint128_t
ceildiv_128_64(extint128_t a, int64_t b)
{
    extint128_t result;
    int64_t remainder;
    char overflow = 0;
    assert(b > 0);
    result = divmod_128_64(a, b, &remainder);
    if (a.sign > 0 && remainder != 0) {
        result = add_128(result, to_128(1), &overflow);
    }
    return result;
}

#endif //PHPSCI_EXT_CA_EXTINT128_H