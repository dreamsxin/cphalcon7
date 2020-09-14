/*
  Original File
  Copyright (c) NumPy (/numpy/core/src/common/mem_overlap.c)

  Edited for CArrays in 2018
  Henrique Borba
  henrique.borba.dev@gmail.com
*/  

#include "../carray.h"
#include "mem_overlap.h"
#include "ca_extint128.h"

static int
diophantine_sort_A(const void *xp, const void *yp)
{
    int64_t xa = ((diophantine_term_t*)xp)->a;
    int64_t ya = ((diophantine_term_t*)yp)->a;

    if (xa < ya) {
        return 1;
    }
    else if (ya < xa) {
        return -1;
    }
    else {
        return 0;
    }
}

/**
 * Depth-first bounded Euclid search
 */
static mem_overlap_t
diophantine_dfs(unsigned int n,
                unsigned int v,
                diophantine_term_t *E,
                diophantine_term_t *Ep,
                int64_t *Gamma, int64_t *Epsilon,
                int64_t b,
                size_t max_work,
                int require_ub_nontrivial,
                int64_t *x,
                size_t *count)
{
    int64_t a_gcd, gamma, epsilon, a1, u1, a2, u2, c, r, c1, c2, t, t_l, t_u, b2, x1, x2;
    extint128_t x10, x20, t_l1, t_l2, t_u1, t_u2;
    mem_overlap_t res;
    char overflow = 0;

    if (max_work >= 0 && *count >= max_work) {
        return MEM_OVERLAP_TOO_HARD;
    }

    /* Fetch precomputed values for the reduced problem */
    if (v == 1) {
        a1 = E[0].a;
        u1 = E[0].ub;
    }
    else {
        a1 = Ep[v-2].a;
        u1 = Ep[v-2].ub;
    }

    a2 = E[v].a;
    u2 = E[v].ub;

    a_gcd = Ep[v-1].a;
    gamma = Gamma[v-1];
    epsilon = Epsilon[v-1];

    /* Generate set of allowed solutions */
    c = b / a_gcd;
    r = b % a_gcd;
    if (r != 0) {
        ++*count;
        return MEM_OVERLAP_NO;
    }

    c1 = a2 / a_gcd;
    c2 = a1 / a_gcd;

    /*
      The set to enumerate is:
      x1 = gamma*c + c1*t
      x2 = epsilon*c - c2*t
      t integer
      0 <= x1 <= u1
      0 <= x2 <= u2
      and we have c, c1, c2 >= 0
     */

    x10 = mul_64_64(gamma, c);
    x20 = mul_64_64(epsilon, c);

    t_l1 = ceildiv_128_64(neg_128(x10), c1);
    t_l2 = ceildiv_128_64(sub_128(x20, to_128(u2), &overflow), c2);

    t_u1 = floordiv_128_64(sub_128(to_128(u1), x10, &overflow), c1);
    t_u2 = floordiv_128_64(x20, c2);

    if (overflow) {
        return MEM_OVERLAP_OVERFLOW;
    }

    if (gt_128(t_l2, t_l1)) {
        t_l1 = t_l2;
    }

    if (gt_128(t_u1, t_u2)) {
        t_u1 = t_u2;
    }

    if (gt_128(t_l1, t_u1)) {
        ++*count;
        return MEM_OVERLAP_NO;
    }

    t_l = to_64(t_l1, &overflow);
    t_u = to_64(t_u1, &overflow);

    x10 = add_128(x10, mul_64_64(c1, t_l), &overflow);
    x20 = sub_128(x20, mul_64_64(c2, t_l), &overflow);

    t_u = safe_sub(t_u, t_l, &overflow);
    t_l = 0;
    x1 = to_64(x10, &overflow);
    x2 = to_64(x20, &overflow);

    if (overflow) {
        return MEM_OVERLAP_OVERFLOW;
    }

    /* The bounds t_l, t_u ensure the x computed below do not overflow */

    if (v == 1) {
        /* Base case */
        if (t_u >= t_l) {
            x[0] = x1 + c1*t_l;
            x[1] = x2 - c2*t_l;
            if (require_ub_nontrivial) {
                unsigned int j;
                int is_ub_trivial;

                is_ub_trivial = 1;
                for (j = 0; j < n; ++j) {
                    if (x[j] != E[j].ub/2) {
                        is_ub_trivial = 0;
                        break;
                    }
                }

                if (is_ub_trivial) {
                    /* Ignore 'trivial' solution */
                    ++*count;
                    return MEM_OVERLAP_NO;
                }
            }
            return MEM_OVERLAP_YES;
        }
        ++*count;
        return MEM_OVERLAP_NO;
    }
    else {
        /* Recurse to all candidates */
        for (t = t_l; t <= t_u; ++t) {
            x[v] = x2 - c2*t;

            /* b2 = b - a2*x[v]; */
            b2 = safe_sub(b, safe_mul(a2, x[v], &overflow), &overflow);
            if (overflow) {
                return MEM_OVERLAP_OVERFLOW;
            }

            res = diophantine_dfs(n, v-1, E, Ep, Gamma, Epsilon,
                                  b2, max_work, require_ub_nontrivial,
                                  x, count);
            if (res != MEM_OVERLAP_NO) {
                return res;
            }
        }
        ++*count;
        return MEM_OVERLAP_NO;
    }
}


/**
 * Euclid's algorithm for GCD.
 *
 * Solves for gamma*a1 + epsilon*a2 == gcd(a1, a2)
 * providing |gamma| < |a2|/gcd, |epsilon| < |a1|/gcd.
 */
static void
euclid(int64_t a1, int64_t a2, int64_t *a_gcd, int64_t *gamma, int64_t *epsilon)
{
    int64_t gamma1, gamma2, epsilon1, epsilon2, r;

    assert(a1 > 0);
    assert(a2 > 0);

    gamma1 = 1;
    gamma2 = 0;
    epsilon1 = 0;
    epsilon2 = 1;

    /* The numbers remain bounded by |a1|, |a2| during
       the iteration, so no integer overflows */
    while (1) {
        if (a2 > 0) {
            r = a1/a2;
            a1 -= r*a2;
            gamma1 -= r*gamma2;
            epsilon1 -= r*epsilon2;
        }
        else {
            *a_gcd = a1;
            *gamma = gamma1;
            *epsilon = epsilon1;
            break;
        }

        if (a1 > 0) {
            r = a2/a1;
            a2 -= r*a1;
            gamma2 -= r*gamma1;
            epsilon2 -= r*epsilon1;
        }
        else {
            *a_gcd = a2;
            *gamma = gamma2;
            *epsilon = epsilon2;
            break;
        }
    }
}

/**
 * Precompute GCD and bounds transformations
 */
static int
diophantine_precompute(unsigned int n,
                       diophantine_term_t *E,
                       diophantine_term_t *Ep,
                       int64_t *Gamma, int64_t *Epsilon)
{
    int64_t a_gcd, gamma, epsilon, c1, c2;
    unsigned int j;
    char overflow = 0;

    assert(n >= 2);

    euclid(E[0].a, E[1].a, &a_gcd, &gamma, &epsilon);
    Ep[0].a = a_gcd;
    Gamma[0] = gamma;
    Epsilon[0] = epsilon;

    if (n > 2) {
        c1 = E[0].a / a_gcd;
        c2 = E[1].a / a_gcd;

        /* Ep[0].ub = E[0].ub * c1 + E[1].ub * c2; */
        Ep[0].ub = safe_add(safe_mul(E[0].ub, c1, &overflow),
                            safe_mul(E[1].ub, c2, &overflow), &overflow);
        if (overflow) {
            return 1;
        }
    }

    for (j = 2; j < n; ++j) {
        euclid(Ep[j-2].a, E[j].a, &a_gcd, &gamma, &epsilon);
        Ep[j-1].a = a_gcd;
        Gamma[j-1] = gamma;
        Epsilon[j-1] = epsilon;

        if (j < n - 1) {
            c1 = Ep[j-2].a / a_gcd;
            c2 = E[j].a / a_gcd;

            /* Ep[j-1].ub = c1 * Ep[j-2].ub + c2 * E[j].ub; */
            Ep[j-1].ub = safe_add(safe_mul(c1, Ep[j-2].ub, &overflow),
                                  safe_mul(c2, E[j].ub, &overflow), &overflow);

            if (overflow) {
                return 1;
            }
        }
    }

    return 0;
}

/**
 * Solve bounded Diophantine equation
 *
 * The problem considered is::
 *
 *     A[0] x[0] + A[1] x[1] + ... + A[n-1] x[n-1] == b
 *     0 <= x[i] <= U[i]
 *     A[i] > 0
 *
 * Solve via depth-first Euclid's algorithm, as explained in [1].
 *
 * If require_ub_nontrivial!=0, look for solutions to the problem
 * where b = A[0]*(U[0]/2) + ... + A[n]*(U[n-1]/2) but ignoring
 * the trivial solution x[i] = U[i]/2. All U[i] must be divisible by 2.
 * The value given for `b` is ignored in this case.
 */
mem_overlap_t
solve_diophantine(unsigned int n, diophantine_term_t *E, int64_t b,
                  size_t max_work, int require_ub_nontrivial, int64_t *x)
{
    mem_overlap_t res;
    unsigned int j;

    for (j = 0; j < n; ++j) {
        if (E[j].a <= 0) {
            return MEM_OVERLAP_ERROR;
        }
        else if (E[j].ub < 0) {
            return MEM_OVERLAP_NO;
        }
    }

    if (require_ub_nontrivial) {
        int64_t ub_sum = 0;
        char overflow = 0;
        for (j = 0; j < n; ++j) {
            if (E[j].ub % 2 != 0) {
                return MEM_OVERLAP_ERROR;
            }
            ub_sum = safe_add(ub_sum,
                              safe_mul(E[j].a, E[j].ub/2, &overflow),
                              &overflow);
        }
        if (overflow) {
            return MEM_OVERLAP_ERROR;
        }
        b = ub_sum;
    }

    if (b < 0) {
        return MEM_OVERLAP_NO;
    }

    if (n == 0) {
        if (require_ub_nontrivial) {
            /* Only trivial solution for 0-variable problem */
            return MEM_OVERLAP_NO;
        }
        if (b == 0) {
            return MEM_OVERLAP_YES;
        }
        return MEM_OVERLAP_NO;
    }
    else if (n == 1) {
        if (require_ub_nontrivial) {
            /* Only trivial solution for 1-variable problem */
            return MEM_OVERLAP_NO;
        }
        if (b % E[0].a == 0) {
            x[0] = b / E[0].a;
            if (x[0] >= 0 && x[0] <= E[0].ub) {
                return MEM_OVERLAP_YES;
            }
        }
        return MEM_OVERLAP_NO;
    }
    else {
        size_t count = 0;
        diophantine_term_t *Ep = NULL;
        int64_t *Epsilon = NULL, *Gamma = NULL;

        Ep = emalloc(n * sizeof(diophantine_term_t));
        Epsilon = emalloc(n * sizeof(int64_t));
        Gamma = emalloc(n * sizeof(int64_t));
        if (Ep == NULL || Epsilon == NULL || Gamma == NULL) {
            res = MEM_OVERLAP_ERROR;
        }
        else if (diophantine_precompute(n, E, Ep, Gamma, Epsilon)) {
            res = MEM_OVERLAP_OVERFLOW;
        }
        else {
            res = diophantine_dfs(n, n-1, E, Ep, Gamma, Epsilon, b, max_work,
                                  require_ub_nontrivial, x, &count);
        }
        efree(Ep);
        efree(Gamma);
        efree(Epsilon);
        return res;
    }
}


/**
 * Simplify Diophantine decision problem.
 * 
 * Combine identical coefficients, remove unnecessary variables, and trim
 * bounds.
 * 
 * The feasible/infeasible decision result is retained.
 * 
 * Returns: 0 (success), -1 (integer overflow).
 */ 
int
diophantine_simplify(unsigned int *n, diophantine_term_t *E, int64_t b)
{
    unsigned int i, j, m;
    char overflow = 0;

    /* Skip obviously infeasible cases */
    for (j = 0; j < *n; ++j) {
        if (E[j].ub < 0) {
            return 0;
        }
    }

    if (b < 0) {
        return 0;
    }

    /* Sort vs. coefficients */
    qsort(E, *n, sizeof(diophantine_term_t), diophantine_sort_A);

    /* Combine identical coefficients */
    m = *n;
    i = 0;
    for (j = 1; j < m; ++j) {
        if (E[i].a == E[j].a) {
            E[i].ub = safe_add(E[i].ub, E[j].ub, &overflow);
            --*n;
        }
        else {
            ++i;
            if (i != j) {
                E[i] = E[j];
            }
        }
    }

    /* Trim bounds and remove unnecessary variables */
    m = *n;
    i = 0;
    for (j = 0; j < m; ++j) {
        E[j].ub = MIN(E[j].ub, b / E[j].a);
        if (E[j].ub == 0) {
            /* If the problem is feasible at all, x[i]=0 */
            --*n;
        }
        else {
            if (i != j) {
                E[i] = E[j];
            }
            ++i;
        }
    }

    if (overflow) {
        return -1;
    }
    else {
        return 0;
    }
}

static int
strides_to_terms(CArray *arr, diophantine_term_t *terms,
                 unsigned int *nterms, int skip_empty)
{
    int i;

    for (i = 0; i < CArray_NDIM(arr); ++i) {
        if (skip_empty) {
            if (CArray_DIM(arr, i) <= 1 || CArray_STRIDE(arr, i) == 0) {
                continue;
            }
        }

        terms[*nterms].a = CArray_STRIDE(arr, i);

        if (terms[*nterms].a < 0) {
            terms[*nterms].a = -terms[*nterms].a;
        }

        if (terms[*nterms].a < 0) {
            /* integer overflow */
            return 1;
        }

        terms[*nterms].ub = CArray_DIM(arr, i) - 1;
        ++*nterms;
    }

    return 0;
}

/* Gets a half-open range [start, end) of offsets from the data pointer */
void
offset_bounds_from_strides(const int itemsize, const int nd,
                           const int *dims, const int *strides,
                           int *lower_offset, int *upper_offset)
{
    int max_axis_offset;
    int lower = 0;
    int upper = 0;
    int i;

    for (i = 0; i < nd; i++) {
        if (dims[i] == 0) {
            /* If the array size is zero, return an empty range */
            *lower_offset = 0;
            *upper_offset = 0;
            return;
        }
        /* Expand either upwards or downwards depending on stride */
        max_axis_offset = strides[i] * (dims[i] - 1);
        if (max_axis_offset > 0) {
            upper += max_axis_offset;
        }
        else {
            lower += max_axis_offset;
        }
    }
    /* Return a half-open range */
    upper += itemsize;
    *lower_offset = lower;
    *upper_offset = upper;
}

/* Gets a half-open range [start, end) which contains the array data */
static void
get_array_memory_extents(CArray *arr,
                         int *out_start, int *out_end,
                         int *num_bytes)
{
    int low, upper;
    int j;
    offset_bounds_from_strides(CArray_ITEMSIZE(arr), CArray_NDIM(arr),
                               CArray_DIMS(arr), CArray_STRIDES(arr),
                               &low, &upper);
    *out_start = *((int*)CArray_DATA(arr)) + (int)low;
    *out_end = *((int*)CArray_DATA(arr)) + (int)upper;

    *num_bytes = CArray_ITEMSIZE(arr);
    for (j = 0; j < CArray_NDIM(arr); ++j) {
        *num_bytes *= CArray_DIM(arr, j);
    }
}

/**
 * Determine whether two arrays share some memory.
 * 
 * Returns: 0 (no shared memory), 1 (shared memory), or < 0 (failed to solve).
 * 
 * Note that failures to solve can occur due to integer overflows, or effort
 * required solving the problem exceeding max_work.  The general problem is
 * NP-hard and worst case runtime is exponential in the number of dimensions.
 * max_work controls the amount of work done, either exact (max_work == -1), only
 * a simple memory extent check (max_work == 0), or set an upper bound
 * max_work > 0 for the number of solution candidates considered.
 **/ 
mem_overlap_t 
solve_may_share_memory(CArray *a, CArray *b, size_t max_work)
{
    int64_t rhs;
    diophantine_term_t terms[2*CARRAY_MAXDIMS + 2];
    int start1 = 0, end1 = 0, size1 = 0;
    int start2 = 0, end2 = 0, size2 = 0;
    int uintp_rhs;
    int64_t x[2*CARRAY_MAXDIMS + 2];
    unsigned int nterms;

    get_array_memory_extents(a, &start1, &end1, &size1);
    get_array_memory_extents(b, &start2, &end2, &size2);

    if (!(start1 < end2 && start2 < end1 && start1 < end1 && start2 < end2)) {
        /* Memory extents don't overlap */
        return MEM_OVERLAP_NO;
    }

    if (max_work == 0) {
        /* Too much work required, give up */
        return MEM_OVERLAP_TOO_HARD;
    }

    /* Convert problem to Diophantine equation form with positive coefficients.
       The bounds computed by offset_bounds_from_strides correspond to
       all-positive strides.

       start1 + sum(abs(stride1)*x1)
       == start2 + sum(abs(stride2)*x2)
       == end1 - 1 - sum(abs(stride1)*x1')
       == end2 - 1 - sum(abs(stride2)*x2')

       <=>

       sum(abs(stride1)*x1) + sum(abs(stride2)*x2')
       == end2 - 1 - start1

       OR

       sum(abs(stride1)*x1') + sum(abs(stride2)*x2)
       == end1 - 1 - start2

       We pick the problem with the smaller RHS (they are non-negative due to
       the extent check above.)
    */
   uintp_rhs = MIN(end2 - 1 - start1, end1 - 1 - start2);
    if (uintp_rhs > CARRAY_MAX_INT64) {
        /* Integer overflow */
        return MEM_OVERLAP_OVERFLOW;
    }
    rhs = (int64_t)uintp_rhs;

    nterms = 0;
    if (strides_to_terms(a, terms, &nterms, 1)) {
        return MEM_OVERLAP_OVERFLOW;
    }
    if (strides_to_terms(b, terms, &nterms, 1)) {
        return MEM_OVERLAP_OVERFLOW;
    }
    if (CArray_ITEMSIZE(a) > 1) {
        terms[nterms].a = 1;
        terms[nterms].ub = CArray_ITEMSIZE(a) - 1;
        ++nterms;
    }
    if (CArray_ITEMSIZE(b) > 1) {
        terms[nterms].a = 1;
        terms[nterms].ub = CArray_ITEMSIZE(b) - 1;
        ++nterms;
    }

    /* Simplify, if possible */
    if (diophantine_simplify(&nterms, terms, rhs)) {
        /* Integer overflow */
        return MEM_OVERLAP_OVERFLOW;
    }

    /* Solve */
    return solve_diophantine(nterms, terms, rhs, max_work, 0, x);
}