#ifndef PHPSCI_EXT_ITERATORS_H
#define PHPSCI_EXT_ITERATORS_H

#include "carray.h"

/*** Global flags that may be passed to the iterator constructors ***/

/* Track an index representing C order */
#define CARRAY_ITER_C_INDEX                    0x00000001
/* Track an index representing Fortran order */
#define CARRAY_ITER_F_INDEX                    0x00000002
/* Track a multi-index */
#define CARRAY_ITER_MULTI_INDEX                0x00000004
/* User code external to the iterator does the 1-dimensional innermost loop */
#define CARRAY_ITER_EXTERNAL_LOOP              0x00000008
/* Convert all the operands to a common data type */
#define CARRAY_ITER_COMMON_DTYPE               0x00000010
/* Operands may hold references, requiring API access during iteration */
#define CARRAY_ITER_REFS_OK                    0x00000020
/* Zero-sized operands should be permitted, iteration checks IterSize for 0 */
#define CARRAY_ITER_ZEROSIZE_OK                0x00000040
/* Permits reductions (size-0 stride with dimension size > 1) */
#define CARRAY_ITER_REDUCE_OK                  0x00000080
/* Enables sub-range iteration */
#define CARRAY_ITER_RANGED                     0x00000100
/* Enables buffering */
#define CARRAY_ITER_BUFFERED                   0x00000200
/* When buffering is enabled, grows the inner loop if possible */
#define CARRAY_ITER_GROWINNER                  0x00000400
/* Delay allocation of buffers until first Reset* call */
#define CARRAY_ITER_DELAY_BUFALLOC             0x00000800
/* When CARRAY_KEEPORDER is specified, disable reversing negative-stride axes */
#define CARRAY_ITER_DONT_NEGATE_STRIDES        0x00001000
/*
 * If output operands overlap with other operands (based on heuristics that
 * has false positives but no false negatives), make temporary copies to
 * eliminate overlap.
 */
#define CARRAY_ITER_COPY_IF_OVERLAP            0x00002000

/*** Per-operand flags that may be passed to the iterator constructors ***/

/* The operand will be read from and written to */
#define CARRAY_ITER_READWRITE                  0x00010000
/* The operand will only be read from */
#define CARRAY_ITER_READONLY                   0x00020000
/* The operand will only be written to */
#define CARRAY_ITER_WRITEONLY                  0x00040000
/* The operand's data must be in native byte order */
#define CARRAY_ITER_NBO                        0x00080000
/* The operand's data must be aligned */
#define CARRAY_ITER_ALIGNED                    0x00100000
/* The operand's data must be contiguous (within the inner loop) */
#define CARRAY_ITER_CONTIG                     0x00200000
/* The operand may be copied to satisfy requirements */
#define CARRAY_ITER_COPY                       0x00400000
/* The operand may be copied with WRITEBACKIFCOPY to satisfy requirements */
#define CARRAY_ITER_UPDATEIFCOPY               0x00800000
/* Allocate the operand if it is NULL */
#define CARRAY_ITER_ALLOCATE                   0x01000000
/* If an operand is allocated, don't use any subtype */
#define CARRAY_ITER_NO_SUBTYPE                 0x02000000
/* This is a virtual array slot, operand is NULL but temporary data is there */
#define CARRAY_ITER_VIRTUAL                    0x04000000
/* Require that the dimension match the iterator dimensions exactly */
#define CARRAY_ITER_NO_BROADCAST               0x08000000
/* A mask is being used on this array, affects buffer -> array copy */
#define CARRAY_ITER_WRITEMASKED                0x10000000
/* This array is the mask for all WRITEMASKED operands */
#define CARRAY_ITER_ARRAYMASK                  0x20000000
/* Assume iterator order data access for COPY_IF_OVERLAP */
#define CARRAY_ITER_OVERLAP_ASSUME_ELEMENTWISE 0x40000000

#define CARRAY_ITER_GLOBAL_FLAGS               0x0000ffff
#define CARRAY_ITER_PER_OP_FLAGS               0xffff0000

typedef struct CArrayIterator
{
    int    index;             // Current 1-d index
    int    size;
    int  * coordinates;     // Coordinate vectors of index
    int  * dims_m1;          // Size of array minus 1 for each dimension
    int    ndims_m1;
    int  * factors;         // Factor for ND-index to 1D-index
    int  * strides;         // Array Strides
    int  * backstrides;     // Backstrides
    char * data_pointer;    // Data pointer to element defined by index
    int    contiguous;      // 1 = Contiguous, 0 = Non-contiguous
    int  ** bounds;
    int  ** limits;
    int  *  limits_sizes;
    CArray * array;         // Pointer to represented CArray
} CArrayIterator;

typedef int (CArrayIterator_IterNextFunc)(CArrayIterator *iter);

#define IT_IDATA(it) ((int *)((it)->data_pointer))
#define IT_DDATA(it) ((double *)((it)->data_pointer))
#define CArrayIterator_DATA(it) ((void *)((it)->data_pointer))
#define CArrayIterator_NOTDONE(it) ((it)->index < (it)->size)

CArrayIterator * CArray_NewIter(CArray * array);
static char* get_ptr(CArrayIterator * iter, uintptr_t * coordinates);
void CArrayIterator_Dump(CArrayIterator * iterator);
void CArrayIterator_GOTO(CArrayIterator * iterator, int * destination);
void CArrayIterator_NEXT(CArrayIterator * iterator);
void CArrayIterator_RESET(CArrayIterator * iterator);
void CArrayIterator_FREE(CArrayIterator * it);

CArrayIterator * CArray_BroadcastToShape(CArray * target, int * dims, int nd);
CArrayIterator * CArray_IterAllButAxis(CArray *obj, int *inaxis);
#endif //PHPSCI_EXT_ITERATORS_H