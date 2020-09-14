#ifndef PHPSCI_EXT_CARRAY_H
#define PHPSCI_EXT_CARRAY_H

#include "php.h"
#include "common/exceptions.h"

#define CArray_PRIORITY 0.0

typedef struct CArray CArray;

static const int CARRAY_ARRAY_WARN_ON_WRITE = (1 << 31);

#define TYPE_INT_STRING     "int"
#define TYPE_INT32_STRING   "int32"
#define TYPE_INT64_STRING   "int64"
#define TYPE_LONG_STRING    "long"
#define TYPE_FLOAT_STRING   "float"
#define TYPE_FLOAT32_STRING "float32"
#define TYPE_FLOAT64_STRING "float64"
#define TYPE_DOUBLE_STRING  "double"

#define CARRAY_NTYPES     7
#define CARRAY_MAXDIMS   100
#define TYPE_INTEGER     'i'
#define TYPE_DOUBLE      'd'
#define TYPE_FLOAT       'f'
#define TYPE_BOOL        'b'
#define TYPE_STRING      's'
#define TYPE_VOID        'v'
#define TYPE_LONG        'l'
#define TYPE_INTEGER_INT  0
#define TYPE_DOUBLE_INT   3
#define TYPE_FLOAT_INT    2
#define TYPE_BOOL_INT     5
#define TYPE_STRING_INT   4
#define TYPE_VOID_INT     6
#define TYPE_LONG_INT     1
#define TYPE_NOTYPE_INT   -1
#define TYPE_DEFAULT_INT  0
#define TYPE_DEFAULT      'd'

/* Macros to use for freeing and cloning auxiliary data */
#define CARRAY_AUXDATA_FREE(auxdata) \
    do { \
        if ((auxdata) != NULL) { \
            (auxdata)->free(auxdata); \
        } \
    } while(0)


typedef enum {
        CARRAY_CLIP=0,
        CARRAY_WRAP=1,
        CARRAY_RAISE=2
} CARRAY_CLIPMODE;

/* For specifying array memory layout or iteration order */
typedef enum {
    /* Fortran order if inputs are all Fortran, C otherwise */
    CARRAY_ANYORDER=-1,
    /* C order */
    CARRAY_CORDER=0,
    /* Fortran order */
    CARRAY_FORTRANORDER=1,
    /* An order as close to the inputs as possible */
    CARRAY_KEEPORDER=2
} CARRAY_ORDER;

typedef enum {
    CARRAY_QUICKSORT=0,
    CARRAY_HEAPSORT=1,
    CARRAY_MERGESORT=2
} CARRAY_SORTKIND;
#define CARRAY_NSORTS (CARRAY_MERGESORT + 1)

/*
 * Means c-style contiguous (last index varies the fastest). The data
 * elements right after each other.
 *
 * This flag may be requested in constructor functions.
 */
#define CARRAY_ARRAY_C_CONTIGUOUS    0x0001

/*
 * Set if array is a contiguous Fortran array: the first index varies
 * the fastest in memory (strides array is reverse of C-contiguous
 * array)
 *
 * This flag may be requested in constructor functions.
 */
#define CARRAY_ARRAY_F_CONTIGUOUS    0x0002

/*
 * Note: all 0-d arrays are C_CONTIGUOUS and F_CONTIGUOUS. If a
 * 1-d array is C_CONTIGUOUS it is also F_CONTIGUOUS. Arrays with
 * more then one dimension can be C_CONTIGUOUS and F_CONTIGUOUS
 * at the same time if they have either zero or one element.
 * If NPY_RELAXED_STRIDES_CHECKING is set, a higher dimensional
 * array is always C_CONTIGUOUS and F_CONTIGUOUS if it has zero elements
 * and the array is contiguous if carray.squeeze() is contiguous.
 * I.e. dimensions for which `carray.shape[dimension] == 1` are
 * ignored.
 */
#define CARRAY_ARRAY_OWNDATA         0x0004

/*
 * Array data is aligned on the appropriate memory address for the type
 * stored according to how the compiler would align things (e.g., an
 * array of integers (4 bytes each) starts on a memory address that's
 * a multiple of 4)
 *
 * This flag may be requested in constructor functions.
 */
#define CARRAY_ARRAY_ALIGNED         0x0100

/*
 * Array data is writeable
 *
 * This flag may be requested in constructor functions.
 */
#define CARRAY_ARRAY_WRITEABLE       0x0400
#define CARRAY_ARRAY_WRITEBACKIFCOPY 0x2000
#define CARRAY_ARRAY_BEHAVED        (CARRAY_ARRAY_ALIGNED | CARRAY_ARRAY_WRITEABLE)
#define CARRAY_ARRAY_DEFAULT        (CARRAY_ARRAY_CARRAY)
#define CARRAY_ARRAY_CARRAY         (CARRAY_ARRAY_C_CONTIGUOUS | CARRAY_ARRAY_BEHAVED)
#define CARRAY_ARRAY_CARRAY_RO      (CARRAY_ARRAY_C_CONTIGUOUS | CARRAY_ARRAY_ALIGNED)
#define CARRAY_ARRAY_UPDATE_ALL     (CARRAY_ARRAY_C_CONTIGUOUS | CARRAY_ARRAY_F_CONTIGUOUS | CARRAY_ARRAY_ALIGNED)
#define CARRAY_ARRAY_UPDATEIFCOPY    0x1000
#define CARRAY_ARRAY_FORCECAST       0x0010
#define CARRAY_ARRAY_ENSURECOPY      0x0020
#define CARRAY_ARRAY_ENSUREARRAY     0x0040
#define CARRAY_ARRAY_FARRAY          (CARRAY_ARRAY_F_CONTIGUOUS | CARRAY_ARRAY_BEHAVED)
#define CARRAY_ARRAY_FARRAY_RO       (CARRAY_ARRAY_F_CONTIGUOUS | CARRAY_ARRAY_ALIGNED)
#define CARRAY_ARRAY_NOTSWAPPED      0x0200

/*
 * Make sure that the strides are in units of the element size Needed
 * for some operations with record-arrays.
 *
 * This flag may be requested in constructor functions.
 */
#define CARRAY_ARRAY_ELEMENTSTRIDES  0x0080

/* The item must be reference counted when it is inserted or extracted. */
#define CARRAY_ITEM_REFCOUNT   0x01
/* Same as needing REFCOUNT */
#define CARRAY_ITEM_HASOBJECT  0x01
/* The item is a POINTER  */
#define CARRAY_ITEM_IS_POINTER 0x04
/* memory needs to be initialized for this data-type */
#define CARRAY_NEEDS_INIT      0x08
/* Use f.getitem when extracting elements of this data-type */
#define CARRAY_USE_GETITEM     0x20
/* Use f.setitem when setting creating 0-d array from this data-type.*/
#define CARRAY_USE_SETITEM     0x40
/* A sticky flag specifically for structured arrays */
#define CARRAY_ALIGNED_STRUCT  0x80

#define CArray_ISONESEGMENT(m) (CArray_NDIM(m) == 0 || \
                             CArray_CHKFLAGS(m, CARRAY_ARRAY_C_CONTIGUOUS) || \
                             CArray_CHKFLAGS(m, CARRAY_ARRAY_F_CONTIGUOUS))
#define CArrayDataType_FLAGCHK(dtype, flag) (((dtype)->flags & (flag)) == (flag))
#define CArray_ISFORTRAN(m) (CArray_CHKFLAGS(m, CARRAY_ARRAY_F_CONTIGUOUS) && \
                             (!CArray_CHKFLAGS(m, CARRAY_ARRAY_C_CONTIGUOUS)))
#define CArray_IS_F_CONTIGUOUS(m) CArray_CHKFLAGS(m, CARRAY_ARRAY_F_CONTIGUOUS)
#define CArray_IS_C_CONTIGUOUS(m) CArray_CHKFLAGS(m, CARRAY_ARRAY_C_CONTIGUOUS)
#define CArray_Copy(obj) CArray_NewCopy(obj, CARRAY_CORDER)


#define CArray_GETPTR2(obj, i, j) ((void *)(CArray_BYTES(obj) + \
                                            (i)*CArray_STRIDES(obj)[0] + \
                                            (j)*CArray_STRIDES(obj)[1]))

/**
 * Array Functions
 */
typedef int (CArray_FillFunc)(void *, int, struct CArray *);
typedef void * (CArray_GetItemFunc) (void *, struct CArray *);
typedef int (CArray_SetItemFunc)(void *, void *, struct CArray *);
typedef void (CArray_CopySwapNFunc)(void *, int, void *, int,
                                    int, int, struct CArray *);
typedef void (CArray_CopySwapFunc)(void *, void *, int, void *);
typedef void (CArray_VectorUnaryFunc)(void *, void *, int, void *,
                                        void *);
typedef int  (CArray_FastTakeFunc)(void *dest, void *src, int *indarray,
                                       int nindarray, int n_outer,
                                       int m_middle, int nelem,
                                       CARRAY_CLIPMODE clipmode);
typedef int (CArray_ArgFunc)(void*, int, int*, void *);
typedef int (CArray_SortFunc)(void *, int, void *);
typedef int (CArray_CompareFunc)(const void *, const void *, void *);
typedef int (CArray_PartitionFunc)(void *, int, int,
                                   int *, int *,
                                    void *);
typedef void (CArray_FastClipFunc)(void *in, int n_in, void *min,
                                    void *max, void *out);
typedef void (CArray_DotFunc)(char *, int, char *, int, char *, int);

typedef struct CArray_ArrFuncs {
    CArray_FastClipFunc *fastclip;

    /* The next four functions *cannot* be NULL */
    CArray_FastTakeFunc *fasttake;

    /*
     * Functions to get and set items with standard Python types
     * -- not array scalars
     */
    CArray_GetItemFunc *getitem;
    CArray_SetItemFunc *setitem;

    /*
     * Copy and/or swap data.  Memory areas may not overlap
     * Use memmove first if they might
     */
    CArray_CopySwapNFunc *copyswapn;
    CArray_CopySwapFunc *copyswap;

    CArray_ArgFunc *argmax;
    CArray_ArgFunc *argmin;

    /*
     * Function to compare items
     * Can be NULL
     */
    CArray_CompareFunc *compare;

    /*
    * Sorting functions
    * Can be NULL
    */
    CArray_SortFunc * sort[CARRAY_NSORTS];

    /*
     * Array of CArray_CastFuncsItem given cast functions to
     * user defined types. The array it terminated with CArray_NOTYPE.
     * Can be NULL.
     */
    struct CArray_CastFuncsItem* castfuncs;

    /*
     * Functions to cast to all other standard types
     * Can have some NULL entries
     */
    CArray_VectorUnaryFunc *cast[CARRAY_NTYPES];

    /*
     * Used for arange.
     * Can be NULL.
     */
    CArray_FillFunc *fill;

    CArray_DotFunc *dotfunc;

    int cancastto[CARRAY_NTYPES];
} CArray_ArrFuncs;


#define CARRAY_FAIL    0
#define CARRAY_SUCCEED 1

/************************************************************
 * CArray Auxiliary Data for inner loops, sort functions, etc.
 ************************************************************/
typedef struct CArrayAuxData_tag CArrayAuxData;

/* Function pointers for freeing or cloning auxiliary data */
typedef void (CArrayAuxData_FreeFunc) (CArrayAuxData *);
typedef CArrayAuxData *(CArrayAuxData_CloneFunc) (CArrayAuxData *);

struct CArrayAuxData_tag {
    CArrayAuxData_FreeFunc *free;
    CArrayAuxData_CloneFunc *clone;
    /* To allow for a bit of expansion without breaking the ABI */
    void *reserved[2];
};


/**
 * Casting
 **/ 
/* For specifying allowed casting in operations which support it */
typedef enum {
        /* Only allow identical types */
        CARRAY_NO_CASTING=0,
        /* Allow identical and byte swapped types */
        CARRAY_EQUIV_CASTING=1,
        /* Only allow safe casts */
        CARRAY_SAFE_CASTING=2,
        /* Allow safe casts or casts within the same kind */
        CARRAY_SAME_KIND_CASTING=3,
        /* Allow any casts */
        CARRAY_UNSAFE_CASTING=4
} CARRAY_CASTING;

/**
 * CArray Descriptor
 */
typedef struct CArrayDescriptor {
    char type;          // b = boolean, d = double, i = signer integer, u = unsigned integer, f = floating point, c = char
    int flags;          // Data related flags
    int type_num;       // 0 = boolean, 1 = double, 2 = signed integer, 3 = unsigned integer, 4 = floating point, 5 = char
    int elsize;         // Datatype size
    int numElements;    // Number of elements
    char byteorder;
    int alignment;      // Alignment Information
    int refcount;
    CArray_ArrFuncs *f;
} CArrayDescriptor;

/**
 * Stride Sorting
 */ 
typedef struct {
    int perm, stride;
} ca_stride_sort_item;

/**
 * CArray
 */
struct CArray {
    int uuid;           // Buffer UUID
    int * strides;      // Strides vector
    int * dimensions;   // Dimensions size vector (Shape)
    int ndim;           // Number of Dimensions
    char * data;        // Data Buffer
    CArray * base;      // Used when sharing memory from other CArray (slices, etc)
    int flags;          // Describes CArray memory approach (Memory related flags)
    CArrayDescriptor * descriptor;    // CArray data descriptor
    int refcount;
};

typedef void (*strided_copy_func_t)(char *, int, char *, int, int, int, CArrayDescriptor*);

/**
 * CArray Dims
 **/ 
typedef struct CArray_Dims {
    int * ptr;
    int len;
} CArray_Dims;

/**
 * Memory Pointer
 */
typedef struct MemoryPointer {
    int uuid;
    int free;
} MemoryPointer;

/**
 * Flags Object
 */ 
typedef struct CArrayFlags
{
    CArray * array;
    int      flags;
} CArrayFlags;

#define CARRAY_LIKELY(x) (x)
#define CARRAY_UNLIKELY(x) (x)

/**
 * CArray Data Macros
 **/ 
#define CHDATA(p) ((char *) CArray_DATA((CArray *)p))
#define SHDATA(p) ((short int *) CArray_DATA((CArray *)p))
#define DDATA(p)  ((double *) CArray_DATA((CArray *)p))
#define FDATA(p)  ((float *) CArray_DATA((CArray *)p))
#define CDATA(p)  ((f2c_complex *) CArray_DATA((CArray *)p))
#define ZDATA(p)  ((f2c_doublecomplex *) CArray_DATA((CArray *)p))
#define IDATA(p)  ((int *) CArray_DATA((CArray *)p))

/**
 * CArrays Func Macros
 **/
#define CArray_BYTES(a) (a->data)
#define CArray_DATA(a) ((void *)((a)->data))
#define CArray_ITEMSIZE(a) ((int)((a)->descriptor->elsize))
#define CArray_DIMS(a) ((int *)((a)->dimensions))
#define CArray_STRIDES(a) ((int *)((a)->strides))
#define CArray_DESCR(a) ((a)->descriptor)
#define CArray_SIZE(m) CArray_MultiplyList(CArray_DIMS(m), CArray_NDIM(m))
#define CArray_NBYTES(m) (CArray_ITEMSIZE(m) * CArray_SIZE(m))
#define CArray_DESCR_REPLACE(descr)                             \
    do {                                                          \
        CArrayDescriptor *_new_;                                    \
        _new_ = CArray_DescrNew(descr);                         \
        CArrayDescriptor_DECREF(descr);                                      \
        descr = _new_;                                            \
    } while(0)
#define CArray_ISCARRAY(m) CArray_FLAGSWAP(m, CARRAY_ARRAY_CARRAY)
#define CArray_ISCARRAY_RO(m) CArray_FLAGSWAP(m, CARRAY_ARRAY_CARRAY_RO)   
#define CArray_ISFARRAY(m) CArray_FLAGSWAP(m, CARRAY_ARRAY_FARRAY)
#define CArray_ISFARRAY_RO(m) CArray_FLAGSWAP(m, CARRAY_ARRAY_FARRAY_RO) 
#define CArray_ISNOTSWAPPED(m) CArray_ISNBO(CArray_DESCR(m)->byteorder)
#define CArray_FLAGSWAP(m, flags) (CArray_CHKFLAGS(m, flags) && CArray_ISNOTSWAPPED(m))
#define CArray_ISBYTESWAPPED(m) (!CArray_ISNOTSWAPPED(m))
#define CArrayDataType_ISUNSIZED(dtype) ((dtype)->elsize == 0)
#define CArrayTypeNum_ISUNSIGNED(type) (0)

#define CARRAY_BYTE_ORDER __BYTE_ORDER
#define CARRAY_LITTLE_ENDIAN __LITTLE_ENDIAN
#define CARRAY_BIG_ENDIAN __BIG_ENDIAN

#define CARRAY_LITTLE '<'
#define CARRAY_BIG '>'
#define CARRAY_NATIVE '='
#define CARRAY_SWAP 's'
#define CARRAY_IGNORE '|'

#if CARRAY_BYTE_ORDER == CARRAY_BIG_ENDIAN
#define CARRAY_NATBYTE CARRAY_BIG
#define CARRAY_OPPBYTE CARRAY_LITTLE
#else
#define CARRAY_NATBYTE CARRAY_LITTLE
#define CARRAY_OPPBYTE CARRAY_BIG
#endif
#define CArray_ISNBO(arg) ((arg) != CARRAY_OPPBYTE)


static inline int
CArray_TYPE(const CArray *arr)
{
    return arr->descriptor->type_num;
}

static inline char
CArray_TYPE_CHAR(const CArray *arr)
{
    return arr->descriptor->type;
}

static inline int
CArray_FLAGS(const CArray *arr)
{
    return arr->flags;
}

static inline CArray * 
CArray_BASE(const CArray *arr)
{
    return arr->base;
}

static inline int
CArray_STRIDE(const CArray *arr, int index)
{
    return ((arr)->strides[index]);
}

static inline int
CArray_DIM(const CArray *arr, int index)
{
    return ((arr)->dimensions[index]);
}

static inline int
CArray_CHKFLAGS(const CArray *arr, int flags) {
    return (CArray_FLAGS(arr) & flags) == flags;
}

static inline int
CArray_NDIM(const CArray *arr) {
    return arr->ndim;
}

static inline int
CArray_CompareLists(int *l1, int *l2, int n)
{
    int i;

    for (i = 0; i < n; i++) {
        if (l1[i] != l2[i]) {
            return 0;
        }
    }
    return 1;
}

static inline int
check_and_adjust_axis_msg(int *axis, int ndim)
{
    if (axis == NULL) {
        return 0;
    }

    /* Check that index is valid, taking into account negative indices */
    if (CARRAY_UNLIKELY((*axis < -ndim) || (*axis >= ndim))) {
        throw_axis_exception("Axis is out of bounds for array dimension");
        return -1;
    }
    
    /* adjust negative indices */
    if (*axis < 0) {
        *axis += ndim;
    }
    return 0;
}

static inline int
CArray_SAMESHAPE(const CArray * a, const CArray * b)
{
    return CArray_CompareLists(CArray_DIMS(a), CArray_DIMS(b), CArray_NDIM(a));
}

static inline int
check_and_adjust_axis(int *axis, int ndim)
{
    return check_and_adjust_axis_msg(axis, ndim);
}

/* Auxiliary routine: printing a matrix */
static inline void
print_matrix( char* desc, int m, int n, double* a, int lda ) {
    int i, j;
    printf( "\n %s\n", desc );
    for( i = 0; i < m; i++ ) {
        for( j = 0; j < n; j++ ) printf( " %6.2f", a[i+j*lda] );
        printf( "\n" );
    }
}

/*
 * Like ceil(value), but check for overflow.
 *
 * Return 0 on success, -1 on failure
 */
static int _safe_ceil_to_int(double value, int* ret)
{
    double ivalue;

    ivalue = ceil(value);
    if (ivalue < INT_MIN || ivalue > INT_MAX) {
        return -1;
    }

    *ret = (int)ivalue;
    return 0;
}

#define CArrayDataType_REFCHK(dtype) \
        CArrayDataType_FLAGCHK(dtype, CARRAY_ITEM_REFCOUNT)

#define PHPObject zval
#define CArray_ISBEHAVED(m) CArray_FLAGSWAP(m, CARRAY_ARRAY_BEHAVED)
#define CArrayTypeNum_ISFLEXIBLE(type) (((type) >=TYPE_STRING) &&        \
                                     ((type) <=TYPE_VOID))
#define CArray_ISCONTIGUOUS(m) CArray_CHKFLAGS(m, CARRAY_ARRAY_C_CONTIGUOUS)
#define CArray_ISWRITEABLE(m) CArray_CHKFLAGS(m, CARRAY_ARRAY_WRITEABLE)
#define CArray_ISALIGNED(m) CArray_CHKFLAGS(m, CARRAY_ARRAY_ALIGNED)
#define CArray_ISVARIABLE(obj) CArrayTypeNum_ISFLEXIBLE(CArray_TYPE(obj))
#define CArray_SAFEALIGNEDCOPY(obj) (CArray_ISALIGNED(obj) &&      \
                                       !CArray_ISVARIABLE(obj))

#define CArray_CheckExact(op) 1

#ifndef __COMP_CARRAY_UNUSED
        #if defined(__GNUC__)
                #define __COMP_CARRAY_UNUSED __attribute__ ((__unused__))
        # elif defined(__ICC)
                #define __COMP_CARRAY_UNUSED __attribute__ ((__unused__))
        # elif defined(__clang__)
                #define __COMP_CARRAY_UNUSED __attribute__ ((unused))
        #else
                #define __COMP_CARRAY_UNUSED
        #endif
#endif
#define CARRAY_UNUSED(x) (__CARRAY_UNUSED_TAGGED ## x) __COMP_CARRAY_UNUSED

void _unaligned_strided_byte_copy(char *dst, int outstrides, char *src,
                                  int instrides, int N, int elsize,
                                  CArrayDescriptor* ignore);
void _strided_byte_swap(void *p, int stride, int n, int size);


int CHAR_TYPE_INT(char CHAR_TYPE);
int CArray_MultiplyList(const int * list, unsigned int size);
void CArray_INIT(MemoryPointer * ptr, CArray * output_ca, int * dims, int ndim, char type);

CArray * CArray_NewFromDescr_int(CArray * self, CArrayDescriptor *descr, int nd,
                                 int *dims, int *strides, void *data,
                                 int flags, CArray *base, int zeroed,
                                 int allow_emptystring);

CArray * CArray_NewLikeArray(CArray *prototype, CARRAY_ORDER order, CArrayDescriptor *dtype, int subok);
CArray * CArray_CheckAxis(CArray * arr, int * axis, int flags);
void CArray_Hashtable_Data_Copy(CArray * target_carray, zval * target_zval, int * first_index);
void CArray_FromZval(zval * php_obj, char type, MemoryPointer * ptr);
void CArray_Dump(CArray * ca);
int * CArray_Generate_Strides(int * dims, int ndims, char type);
void CArray_Print(CArray *array, int force_summary);

CArray * CArray_FromMemoryPointer(MemoryPointer * ptr);
CArray * CArray_FromCArray(CArray * arr, CArrayDescriptor *newtype, int flags);

CArray * CArray_FromAnyUnwrap(CArray *op, CArrayDescriptor *newtype, int min_depth, 
                              int max_depth, int flags, CArray *context);

CArray * CArray_NewFromDescrAndBase(CArray * subtype, CArrayDescriptor * descr, int nd,
                                    int * dims, int * strides, void * data, int flags,
                                    CArray * base);

CArray * CArray_New(CArray *subtype, int nd, int *dims, int type_num,
           int *strides, void *data, int itemsize, int flags, CArray * base);

CArray * CArray_NewFromDescr( CArray *subtype, CArrayDescriptor *descr,
                     int nd, int *dims, int *strides, void *data,
                     int flags, CArray * base);             

CArrayDescriptor * CArray_DescrNew(CArrayDescriptor * base);
int CArray_SetWritebackIfCopyBase(CArray *arr, CArray *base);
int CArray_FailUnlessWriteable(CArray *obj, const char *name);
int array_might_be_written(CArray *obj);
CArrayDescriptor * CArray_DescrFromType(int typenum);
int CArray_ResolveWritebackIfCopy(CArray * self);
int CArray_CompareLists(int *l1, int *l2, int n);
int CArray_EquivTypes(CArrayDescriptor * a, CArrayDescriptor * b);
int CArray_EquivArrTypes(CArray * a, CArray * b);
int CArray_CopyInto(CArray * dest, CArray * src);
int CArray_ElementStrides(CArray *obj);

/**
 * Methods
 **/
CArray * CArray_NewScalar(char type, MemoryPointer *out);
CArray * CArray_Empty(int nd, int *dims, CArrayDescriptor *type, int fortran, MemoryPointer * ptr);
CArray * CArray_Eye(int n, int m, int k, char * dtype, MemoryPointer * out);
CArray * CArray_CheckFromAny(CArray *op, CArrayDescriptor *descr, int min_depth,
                    int max_depth, int requires, CArray *context);
CArray * CArray_FromAny(CArray *op, CArrayDescriptor *newtype, int min_depth, int max_depth, int flags);
CArray * CArray_FromArray(CArray *arr, CArrayDescriptor *newtype, int flags);

#define CArray_ContiguousFromAny(op, type, min_depth, max_depth) \
        CArray_FromAny(op, type, min_depth, \
                              max_depth, CARRAY_ARRAY_DEFAULT)
#endif //PHPSCI_EXT_CARRAY_H
