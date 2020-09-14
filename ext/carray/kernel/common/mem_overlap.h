/*
  Original File
  Copyright (c) NumPy (/numpy/core/src/common/mem_overlap.c)

  Edited for CArrays in 2018
  Henrique Borba
  henrique.borba.dev@gmail.com

  Solving memory overlap integer programs and bounded Diophantine equations with
  positive coefficients.
  Asking whether two strided arrays `a` and `b` overlap is equivalent to
  asking whether there is a solution to the following problem::
      sum(stride_a[i] * x_a[i] for i in range(ndim_a))
      -
      sum(stride_b[i] * x_b[i] for i in range(ndim_b))
      ==
      base_b - base_a

      0 <= x_a[i] < shape_a[i]

      0 <= x_b[i] < shape_b[i]

  for some integer x_a, x_b.  Itemsize needs to be considered as an additional
  dimension with stride 1 and size itemsize.
  Negative strides can be changed to positive (and vice versa) by changing
  variables x[i] -> shape[i] - 1 - x[i], and zero strides can be dropped, so
  that the problem can be recast into a bounded Diophantine equation with
  positive coefficients::

     sum(a[i] * x[i] for i in range(n)) == b

     a[i] > 0

     0 <= x[i] <= ub[i]

  This problem is NP-hard --- runtime of algorithms grows exponentially with
  increasing ndim.
  *Algorithm description*
  A straightforward algorithm that excludes infeasible solutions using GCD-based
  pruning is outlined in Ref. [1]. It is implemented below. A number of other
  algorithms exist in the literature; however, this one seems to have
  performance satisfactory for the present purpose.
  The idea is that an equation::

      a_1 x_1 + a_2 x_2 + ... + a_n x_n = b
  
      0 <= x_i <= ub_i, i = 1...n
  
  implies::

      a_2' x_2' + a_3 x_3 + ... + a_n x_n = b
  
      0 <= x_i <= ub_i, i = 2...n
  
      0 <= x_1' <= c_1 ub_1 + c_2 ub_2
  
  with a_2' = gcd(a_1, a_2) and x_2' = c_1 x_1 + c_2 x_2 with c_1 = (a_1/a_1'),
  and c_2 = (a_2/a_1').  This procedure can be repeated to obtain::
    
      a_{n-1}' x_{n-1}' + a_n x_n = b
    
      0 <= x_{n-1}' <= ub_{n-1}'
    
      0 <= x_n <= ub_n
  
  Now, one can enumerate all candidate solutions for x_n.  For each, one can use
  the previous-level equation to enumerate potential solutions for x_{n-1}, with
  transformed right-hand side b -> b - a_n x_n.  And so forth, until after n-1
  nested for loops we either arrive at a candidate solution for x_1 (in which
  case we have found one solution to the problem), or find that the equations do
  not allow any solutions either for x_1 or one of the intermediate x_i (in
  which case we have proved there is no solution for the upper-level candidates
  chosen). If no solution is found for any candidate x_n, we have proved the
  problem is infeasible --- which for the memory overlap problem means there is
  no overlap.
*/

#ifndef PHPSCI_EXT_MEM_OVERLAP_H
#define PHPSCI_EXT_MEM_OVERLAP_H

#include "../carray.h"

/* Bounds check only */
#define CARRAY_MAY_SHARE_BOUNDS 0

/* Exact solution */
#define CARRAY_MAY_SHARE_EXACT -1


typedef enum {
    MEM_OVERLAP_NO = 0,        /* no solution exists */
    MEM_OVERLAP_YES = 1,       /* solution found */
    MEM_OVERLAP_TOO_HARD = -1, /* max_work exceeded */
    MEM_OVERLAP_OVERFLOW = -2, /* algorithm failed due to integer overflow */
    MEM_OVERLAP_ERROR = -3     /* invalid input */
} mem_overlap_t;


typedef struct {
    int64_t a;
    int64_t ub;
} diophantine_term_t;

mem_overlap_t solve_may_share_memory(CArray *a, CArray *b, size_t max_work);

#endif //PHPSCI_EXT_MEM_OVERLAP_H