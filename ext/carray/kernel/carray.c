#include "carray.h"
#include "alloc.h"
#include "iterators.h"
#include "convert.h"
#include "buffer.h"
#include "flagsobject.h"
#include "php.h"
#include "common/exceptions.h"
#include "php_ini.h"
#include "zend_smart_str.h"
#include "zend_bitset.h"
#include "ext/spl/spl_array.h"
#include "zend_globals.h"
#include "zend_interfaces.h"
#include "php_ini.h"
#include "php_variables.h"
#include "php_globals.h"
#include "php_content_types.h"
#include "zend_multibyte.h"
#include "zend_smart_str.h"
#include "casting.h"
#include "getset.h"
#include "matlib.h"
#include "item_selection.h"
#include "shape.h"
#include "convert_type.h"
#include "search.h"
#include "common/sort.h"
#include "common/compare.h"
#include "clip.h"
#include "ctors.h"
#include "linalg.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wint-conversion"
#pragma clang diagnostic ignored "-Wimplicit-function-declaration"
/**
 * @param CHAR_TYPE
 */
int
CHAR_TYPE_INT(char CHAR_TYPE)
{
    if(CHAR_TYPE == TYPE_DOUBLE) {
        return TYPE_DOUBLE_INT;
    }
    if(CHAR_TYPE == TYPE_INTEGER) {
        return TYPE_INTEGER_INT;
    }
    if(CHAR_TYPE == TYPE_FLOAT) {
        return TYPE_FLOAT_INT;
    }
    throw_valueerror_exception("Unknown type");
}

/**
 * Print current CArray
 **/ 
void
CArray_ToString(CArray * carray)
{

}


/**
 * Create CArray from Double ZVAL
 * @return
 */
MemoryPointer
CArray_FromZval_Double(zval * php_obj, char * type)
{

}

/**
 * Create CArray from Long ZVAL
 * @return
 */
MemoryPointer
CArray_FromZval_Long(zval * php_obj, char * type)
{

}

void
_strided_byte_swap(void *p, int stride, int n, int size)
{
    char *a, *b, c = 0;
    int j, m;

    switch(size) {
        case 1: /* no byteswap necessary */
            break;
        case 4:
            for (a = (char*)p; n > 0; n--, a += stride - 1) {
                b = a + 3;
                c = *a; *a++ = *b; *b-- = c;
                c = *a; *a = *b; *b   = c;
            }
            break;
        case 8:
            for (a = (char*)p; n > 0; n--, a += stride - 3) {
                b = a + 7;
                c = *a; *a++ = *b; *b-- = c;
                c = *a; *a++ = *b; *b-- = c;
                c = *a; *a++ = *b; *b-- = c;
                c = *a; *a = *b; *b   = c;
            }
            break;
        case 2:
            for (a = (char*)p; n > 0; n--, a += stride) {
                b = a + 1;
                c = *a; *a = *b; *b = c;
            }
            break;
        default:
            m = size/2;
            for (a = (char *)p; n > 0; n--, a += stride - m) {
                b = a + (size - 1);
                for (j = 0; j < m; j++) {
                    c=*a; *a++ = *b; *b-- = c;
                }
            }
            break;
    }
}

static int
_copy_from_same_shape(CArray *dest, CArray *src,
                      strided_copy_func_t myfunc, int swap)
{
    int maxaxis = -1, elsize;
    int maxdim;
    CArrayIterator *dit, *sit;
    CArrayDescriptor* descr;

    dit = CArray_IterAllButAxis(dest, &maxaxis);
    sit = CArray_IterAllButAxis(src, &maxaxis);

    maxdim = dest->dimensions[maxaxis];

    if ((dit == NULL) || (sit == NULL)) {
        return -1;
    }

    elsize = CArray_ITEMSIZE(dest);
    descr = CArray_DESCR(dest);

    while(dit->index < dit->size) {
        /* strided copy of elsize bytes */
        myfunc(dit->data_pointer, dest->strides[maxaxis],
               sit->data_pointer, src->strides[maxaxis],
               maxdim, elsize, descr);
        if (swap) {
            _strided_byte_swap(dit->data_pointer,
                               dest->strides[maxaxis],
                               dest->dimensions[maxaxis],
                               elsize);
        }
        CArrayIterator_NEXT(dit);
        CArrayIterator_NEXT(sit);
    }

    CArrayIterator_FREE(sit);
    CArrayIterator_FREE(dit);
    return 0;
}

static int
_broadcast_copy(CArray *dest, CArray *src,
                strided_copy_func_t myfunc, int swap)
{
    
}

static void
_strided_byte_copy(char *dst, int outstrides, char *src, int instrides,
                   int N, int elsize, CArrayDescriptor* ignore)
{
    int i, j;
    char *tout = dst;
    char *tin = src;

#define _FAST_MOVE(_type_)                              \
    for(i=0; i<N; i++) {                                \
        ((_type_ *)tout)[0] = ((_type_ *)tin)[0];       \
        tin += instrides;                               \
        tout += outstrides;                             \
    }                                                   \
    return

    switch(elsize) {
        case 8:
            _FAST_MOVE(long);
        case 4:
            _FAST_MOVE(int);
        case 1:
            _FAST_MOVE(int8_t);
        case 2:
            _FAST_MOVE(int16_t);
        case 16:
            for (i = 0; i < N; i++) {
                ((long *)tout)[0] = ((long *)tin)[0];
                ((long *)tout)[1] = ((long *)tin)[1];
                tin += instrides;
                tout += outstrides;
            }
            return;
        default:
            for(i = 0; i < N; i++) {
                for(j=0; j<elsize; j++) {
                    *tout++ = *tin++;
                }
                tin = tin + instrides - elsize;
                tout = tout + outstrides - elsize;
            }
    }
#undef _FAST_MOVE

}

void
_unaligned_strided_byte_copy(char *dst, int outstrides, char *src,
                             int instrides, int N, int elsize,
                             CArrayDescriptor* ignore)
{
    int i;
    char *tout = dst;
    char *tin = src;

#define _COPY_N_SIZE(size)              \
        for(i=0; i<N; i++) {                    \
        memcpy(tout, tin, size);                \
        tin += instrides;                       \
        tout += outstrides;                     \
        }                                       \
        return;

        switch(elsize) {
            case 8:
                _COPY_N_SIZE(8);
            case 4:
                _COPY_N_SIZE(4);
            case 1:
                _COPY_N_SIZE(1);
            case 2:
                _COPY_N_SIZE(2);
            case 16:
                _COPY_N_SIZE(16);
            default:
                _COPY_N_SIZE(elsize);
        }
#undef _COPY_N_SIZE
}

static void
_strided_void_copy(char* dst, int outstrides, char* src, int instrides,
                   int N, int elsize, CArrayDescriptor* descr)
{
    int i;
    char* tmp = (char*)emalloc(elsize);

    for (i=0; i<N; i++) {
        memcpy(tmp, src, elsize);
        memcpy(dst, tmp, elsize);
        src += instrides;
        dst += outstrides;
    }
    efree(tmp);
}

static void
_unaligned_strided_byte_move(char *dst, int outstrides, char *src,
                             int instrides, int N, int elsize, 
                             CArrayDescriptor* ignore)
{
    int i;
    char *tout = dst;
    char *tin = src;

#define _MOVE_N_SIZE(size)             \
    for(i=0; i<N; i++) {               \
        memmove(tout, tin, size);      \
        tin += instrides;              \
        tout += outstrides;            \
    }                                  \
    return _MOVE_N_SIZE(elsize);
#undef _MOVE_N_SIZE
}

/*
 * Returns the copy func for the arrays.  The arrays must be the same type.
 * If src is NULL then it is assumed to be the same type and aligned.
 */
static strided_copy_func_t
strided_copy_func(CArray* dest, CArray* src, int usecopy)
{
    if (CArray_DESCR(dest)->refcount) {
        return _strided_void_copy;
    }
    else if (CArray_SAFEALIGNEDCOPY(dest) && (src == NULL || CArray_SAFEALIGNEDCOPY(src))) {
        return _strided_byte_copy;
    }
    else if (usecopy) {
        return _unaligned_strided_byte_copy;
    }
    else {
        return _unaligned_strided_byte_move;
    }
}

/*
 * This is the main array creation routine.
 *
 * Flags argument has multiple related meanings
 * depending on data and strides:
 *
 * If data is given, then flags is flags associated with data.
 * If strides is not given, then a contiguous strides array will be created
 * and the CARRAY_ARRAY_C_CONTIGUOUS bit will be set.  If the flags argument
 * has the CARRAY_ARRAY_F_CONTIGUOUS bit set, then a FORTRAN-style strides array will be
 * created (and of course the CARRAY_ARRAY_F_CONTIGUOUS flag bit will be set).
 *
 * If data is not given but created here, then flags will be NPY_ARRAY_DEFAULT
 * and a non-zero flags argument can be used to indicate a FORTRAN style
 * array is desired.
 *
 * Dimensions and itemsize must have been checked for validity.
 */
static void
_array_fill_strides(int *strides, int *dims, int nd, size_t itemsize,
                    int inflag, int *objflags)
{
    int i;

    /* Only make Fortran strides if not contiguous as well */
    if ((inflag & (CARRAY_ARRAY_F_CONTIGUOUS|CARRAY_ARRAY_C_CONTIGUOUS)) ==
                                            CARRAY_ARRAY_F_CONTIGUOUS) {
        for (i = 0; i < nd; i++) {
            strides[i] = itemsize;
            if (dims[i]) {
                itemsize *= dims[i];
            }

        }

        if ((nd > 1) && ((strides[0] != strides[nd-1]) || (dims[nd-1] > 1))) {
            *objflags = ((*objflags)|CARRAY_ARRAY_F_CONTIGUOUS) &
                                            ~CARRAY_ARRAY_C_CONTIGUOUS;
        }
        else {
            *objflags |= (CARRAY_ARRAY_F_CONTIGUOUS|CARRAY_ARRAY_C_CONTIGUOUS);
        }
    }
    else {
        for (i = nd - 1; i >= 0; i--) {
            strides[i] = itemsize;
            if (dims[i]) {
                itemsize *= dims[i];
            }
        }

        if ((nd > 1) && ((strides[0] != strides[nd-1]) || (dims[0] > 1))) {
            *objflags = ((*objflags)|CARRAY_ARRAY_C_CONTIGUOUS) &
                                            ~CARRAY_ARRAY_F_CONTIGUOUS;
        }
        else {
            *objflags |= (CARRAY_ARRAY_C_CONTIGUOUS|CARRAY_ARRAY_F_CONTIGUOUS);
        }
    }
    return;
}

/**
 * @param dims
 * @param ndims
 * @param type
 */
int *
CArray_Generate_Strides(int * dims, int ndims, char type)
{
    int i;
    int * strides;
    int * target_stride;
    target_stride = (int*)emalloc((ndims * sizeof(int)));

    for(i = 0; i < ndims; i++) {
        target_stride[i] = 0;
    }

    if(type == TYPE_INTEGER)
        target_stride[ndims-1] = sizeof(int);
    if(type == TYPE_DOUBLE)
        target_stride[ndims-1] = sizeof(double);

    for(i = ndims-2; i >= 0; i--) {
        target_stride[i] = dims[i+1] * target_stride[i+1];
    }
    
    return target_stride;
}

/**
 *
 * @param target_zval
 * @param ndims
 */
void
Hashtable_ndim(zval * target_zval, int * ndims)
{
    zval * element;
    int current_dim = *ndims;
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(target_zval), element) {
        ZVAL_DEREF(element);
        if (Z_TYPE_P(element) == IS_ARRAY) {
            *ndims = *ndims + 1;
            Hashtable_ndim(element, ndims);
            break;
        }
    } ZEND_HASH_FOREACH_END();
}

/**
 *
 * @param target_zval
 * @param dims
 * @param ndims
 * @param current_dim
 */
void
Hashtable_dimensions(zval * target_zval, int * dims, int ndims, int current_dim)
{
    zval * element;
    if (Z_TYPE_P(target_zval) == IS_ARRAY) {
        dims[current_dim] = (int)zend_hash_num_elements(Z_ARRVAL_P(target_zval));
    }

    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(target_zval), element) {
        ZVAL_DEREF(element);
        if (Z_TYPE_P(element) == IS_ARRAY) {
            Hashtable_dimensions(element, dims, ndims, (current_dim + 1));
            break;
        }
    } ZEND_HASH_FOREACH_END();
}

/**
 *
 * @param target_zval
 * @param dims
 * @param ndims
 * @param current_dim
 */
void
Hashtable_type(zval * target_zval, char * type)
{
    zval * element;

    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(target_zval), element) {
        ZVAL_DEREF(element);
        if (Z_TYPE_P(element) == IS_ARRAY) {
            Hashtable_type(element, type);
            break;
        }
        if (Z_TYPE_P(element) == IS_LONG && *type != 'd') {
            *type = 'i';
            break;
        }
        if (Z_TYPE_P(element) == IS_DOUBLE) {
            *type = 'd';
            break;
        }
        if (Z_TYPE_P(element) == IS_STRING) {
            *type = 'c';
            break;
        }
    } ZEND_HASH_FOREACH_END();
}

/**
 * Create CArray from HashTable ZVAL
 * @param php_array
 * @param type
 * @return
 */
void
CArray_FromZval_Hashtable(zval * php_array, char type, MemoryPointer * ptr)
{
    CArray * new_carray;
    new_carray = emalloc(sizeof(CArray));
    char auto_flag = 'a';
    int * dims, ndims = 1;
    int last_index = 0;
    Hashtable_ndim(php_array, &ndims);
    dims = (int*)emalloc(ndims * sizeof(int));
    Hashtable_dimensions(php_array, dims, ndims, 0);
    // If `a` (auto), find be
    // st element type
    if(type == auto_flag) {
        Hashtable_type(php_array, &type);
    }

    CArray_INIT(ptr, new_carray, dims, ndims, type);
    CArray_Hashtable_Data_Copy(new_carray, php_array, &last_index);
    efree(dims);
}

/**
 * Dump CArray
 */
void
CArray_Dump(CArray * ca)
{
    int i;
    php_printf("CArray.dims\t\t\t[");
    for(i = 0; i < ca->ndim; i ++) {
        php_printf(" %d", ca->dimensions[i]);
    }
    php_printf(" ]\n");
    php_printf("CArray.strides\t\t\t[");
    for(i = 0; i < ca->ndim; i ++) {
        php_printf(" %d", ca->strides[i]);
    }
    php_printf(" ]\n");
    php_printf("CArray.ndim\t\t\t%d\n", ca->ndim);
    php_printf("CArray.refcount\t\t\t%d\n", ca->refcount);
    php_printf("CArray.flags\t\t\t");
    if(CArray_CHKFLAGS(ca, CARRAY_ARRAY_C_CONTIGUOUS)) {
        php_printf("\n\t\t\t\tCARRAY_ARRAY_C_CONTIGUOUS ");
    }
    if(CArray_CHKFLAGS(ca, CARRAY_ARRAY_F_CONTIGUOUS)) {
        php_printf("\n\t\t\t\tCARRAY_ARRAY_F_CONTIGUOUS ");
    }
    if(CArray_CHKFLAGS(ca, CARRAY_ARRAY_ALIGNED)) {
        php_printf("\n\t\t\t\tCARRAY_ARRAY_ALIGNED ");
    }
    if(CArray_CHKFLAGS(ca, CARRAY_ARRAY_WRITEABLE)) {
        php_printf("\n\t\t\t\tCARRAY_ARRAY_WRITEABLE ");
    }
    if(CArray_CHKFLAGS(ca, CARRAY_ARRAY_WRITEBACKIFCOPY)) {
        php_printf("\n\t\t\t\tCARRAY_ARRAY_WRITEBACKIFCOPY ");
    }
    if(CArray_CHKFLAGS(ca, CARRAY_ARRAY_OWNDATA)) {
        php_printf("\n\t\t\t\tCARRAY_ARRAY_OWNDATA ");
    }
    if(CArray_CHKFLAGS(ca, CARRAY_ARRAY_UPDATE_ALL)) {
        php_printf("\n\t\t\t\tCARRAY_ARRAY_UPDATE_ALL ");
    }
    if(CArray_CHKFLAGS(ca, CARRAY_ARRAY_UPDATEIFCOPY)) {
        php_printf("\n\t\t\t\tCARRAY_ARRAY_UPDATEIFCOPY ");
    }
    php_printf("\n");
    php_printf("CArray.descriptor.refcount\t%d\n", ca->descriptor->refcount);
    php_printf("CArray.descriptor.elsize\t%d\n", ca->descriptor->elsize);
    php_printf("CArray.descriptor.alignment\t%d\n", ca->descriptor->alignment);
    php_printf("CArray.descriptor.numElements\t%d\n", ca->descriptor->numElements);
    php_printf("CArray.descriptor.type\t\t%c\n", ca->descriptor->type);
    php_printf("CArray.descriptor.type_num\t%d\n", ca->descriptor->type_num);
}

CArray *
CArray_NewScalar(char type, MemoryPointer *out) {
    int shape = 0;
    return CArray_Zeros(&shape, 0, type, NULL, out);    
}

/**
 * Multiply vector list by scalar
 *
 * @param list
 * @param scalar
 * @return
 */
int
CArray_MultiplyList(const int * list, unsigned int size)
{
    int i;
    int total = 0;
    if(size == 1) {
        return list[0];
    }
    for(i = size-1; i >= 0; i--) {
        if(i == size - 1) {
            total = list[i];
        } else {
            total = total * list[i];
        }
    }
    return total;
}

/**
 * @param target_carray
 */
void
CArray_Hashtable_Data_Copy(CArray * target_carray, zval * target_zval, int * first_index)
{
    zval * element;
    int * data_int;
    double * data_double;

    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(target_zval), element) {
        ZVAL_DEREF(element);
        if (Z_TYPE_P(element) == IS_ARRAY) {
            CArray_Hashtable_Data_Copy(target_carray, element, first_index);
        }
        if (Z_TYPE_P(element) == IS_LONG) {
            if (CArray_TYPE(target_carray) == TYPE_INTEGER_INT) {
                convert_to_long(element);
                data_int = (int *) CArray_DATA(target_carray);
                data_int[*first_index] = (int) zval_get_long(element);
                *first_index = *first_index + 1;
            }
            if (CArray_TYPE(target_carray) == TYPE_DOUBLE_INT) {
                convert_to_long(element);
                data_double = (double *) CArray_DATA(target_carray);
                data_double[*first_index] = (double) zval_get_long(element);
                *first_index = *first_index + 1;
            }
        }
        if (Z_TYPE_P(element) == IS_DOUBLE) {
            if (CArray_TYPE(target_carray) == TYPE_DOUBLE_INT) {
                convert_to_double(element);
                data_double = (double *) CArray_DATA(target_carray);
                data_double[*first_index] = (double) zval_get_double(element);
                *first_index = *first_index + 1;
            }
            if (CArray_TYPE(target_carray) == TYPE_INTEGER_INT) {
                convert_to_double(element);
                data_int = (int *) CArray_DATA(target_carray);
                data_int[*first_index] = (int) zval_get_double(element);
                *first_index = *first_index + 1;
            }
        }
        if (Z_TYPE_P(element) == IS_STRING) {
            
        }
    } ZEND_HASH_FOREACH_END();
}

/**
 * @param ptr
 * @param num_elements
 * @param dims
 * @param ndim
 */
void
CArray_INIT(MemoryPointer * ptr, CArray * output_ca, int * dims, int ndim, char type)
{
    CArrayDescriptor * output_ca_dscr;
    int * target_stride;
    int i, num_elements = 0;

    if(output_ca == NULL) {
        output_ca = (CArray *)emalloc(sizeof(CArray));
    }

    for(i = 0; i < ndim; i++) {
        if(i == 0) {
            num_elements = dims[i];
            continue;
        }
        num_elements = dims[i] * num_elements;
    }

    // If 0 dimensions, this is a scalar
    if(ndim == 0) {
        num_elements = 1;
    }

    output_ca_dscr = (CArrayDescriptor*)ecalloc(1, sizeof(struct CArrayDescriptor));
    output_ca_dscr->refcount = 0;
    // Build CArray Data Descriptor
    output_ca_dscr->type = type;
    output_ca_dscr->alignment = 0;

    if(ndim != 0) {
        target_stride = CArray_Generate_Strides(dims, ndim, type);
        output_ca_dscr->elsize = target_stride[ndim-1];
    }
    if(ndim == 0) {
        switch(type) {
            case TYPE_DOUBLE:
                output_ca_dscr->elsize = sizeof(double);
                break;
            case TYPE_INTEGER:
                output_ca_dscr->elsize = sizeof(int);
                break;
            default:
                output_ca_dscr->elsize = sizeof(double);        
        }
        
    }
    output_ca_dscr->type_num = CHAR_TYPE_INT(type);
    output_ca_dscr->numElements = num_elements;
    if(output_ca == NULL) {
        output_ca = (CArray *)emalloc(sizeof(CArray));
    }
    CArray_NewFromDescr_int(output_ca, output_ca_dscr, ndim, dims, target_stride, NULL, CARRAY_NEEDS_INIT, NULL, 1, 0);
    output_ca->flags &= ~CARRAY_ARRAY_F_CONTIGUOUS;
    add_to_buffer(ptr, output_ca, sizeof(output_ca));
    efree(target_stride);
}

/**
 * Create CArray from ZVAL
 * @return MemoryPointer
 */
void
CArray_FromZval(zval * php_obj, char type, MemoryPointer * ptr)
{
    if(Z_TYPE_P(php_obj) == IS_LONG) {
        throw_notimplemented_exception();
        return;
    }
    if(Z_TYPE_P(php_obj) == IS_ARRAY) {
        CArray_FromZval_Hashtable(php_obj, type, ptr);
        return;
    }
    if(Z_TYPE_P(php_obj) == IS_DOUBLE) {
        throw_notimplemented_exception();
        return;
    }
    throw_notimplemented_exception();
}

/**
 * @param target
 * @param base
 */
int
CArray_SetBaseCArray(CArray * target, CArray * base)
{
    CArray_INCREF(base);
    target->base = base;
    return 0;
}

static void
_select_carray_funcs(CArrayDescriptor *descr)
{
    int i;
    CArray_VectorUnaryFunc * castfunc;
    CArray_ArrFuncs * functions;
    if(descr->f == NULL) {
        functions = ecalloc(1, sizeof(CArray_ArrFuncs));
        descr->f = functions;
    }

    if(descr->type_num == TYPE_INTEGER_INT) {
        descr->f->copyswap = &INT_copyswap;
        descr->f->setitem  = &INT_setitem;
        descr->f->copyswapn = &INT_copyswapn;
        descr->f->fill = &INT_fill;
        descr->f->fasttake = &INT_fasttake;
        descr->f->argmax = &INT_argmax;
        descr->f->argmin = &INT_argmin;
        descr->f->sort[0] = &carray_quicksort;
        descr->f->sort[1] = &carray_heapsort;
        descr->f->sort[2] = &carray_mergesort;
        descr->f->compare = &INT_compare;
        descr->f->cancastto[0] = TYPE_DOUBLE_INT;
        descr->f->cancastto[1] = TYPE_INTEGER_INT;
        descr->f->fastclip = &INT_clip;
        descr->f->dotfunc = &INT_dot;
    }

    if(descr->type_num == TYPE_DOUBLE_INT) {
        descr->f->copyswap = &DOUBLE_copyswap;
        descr->f->copyswapn = &DOUBLE_copyswapn;
        descr->f->setitem  = &DOUBLE_setitem;
        descr->f->fill = &DOUBLE_fill;
        descr->f->fasttake = &DOUBLE_fasttake;
        descr->f->argmax = &DOUBLE_argmax;
        descr->f->argmin = &DOUBLE_argmin;
        descr->f->sort[0] = &carray_quicksort;
        descr->f->sort[1] = &carray_heapsort;
        descr->f->sort[2] = &carray_mergesort;
        descr->f->compare = &DOUBLE_compare;
        descr->f->cancastto[0] = TYPE_DOUBLE_INT;
        descr->f->cancastto[1] = TYPE_INTEGER_INT;
        descr->f->fastclip = &DOUBLE_clip;
        descr->f->dotfunc = &DOUBLE_dot;
    }

    /**
     * SELECT CASTING FUNCTIONS
     **/
    for(i = 0; i < CARRAY_NTYPES; i++) {
        switch(i) {
            case TYPE_DOUBLE_INT:
                if(descr->type_num == TYPE_INTEGER_INT) {
                    descr->f->cast[i] = (void (*)(CArray_VectorUnaryFunc))INT_TO_DOUBLE;
                }
                break;
            case TYPE_INTEGER_INT:
                if(descr->type_num == TYPE_DOUBLE_INT) {
                    descr->f->cast[i] = (void (*)(CArray_VectorUnaryFunc))DOUBLE_TO_INT;
                }
                if(descr->type_num == TYPE_INTEGER_INT) {
                    descr->f->cast[i] = (void (*)(CArray_VectorUnaryFunc))INT_TO_INT;
                }
                break;
        }
    }
}

/**
 * @return
 */ 
CArrayDescriptor *
CArray_DescrFromType(int typenum)
{
    CArrayDescriptor *ret;
    ret = (CArrayDescriptor *)ecalloc(1, sizeof(CArrayDescriptor));
    ret->type_num = typenum;
    ret->numElements = 0;

    if(typenum == 0) {
        ret->elsize = sizeof(int);
        ret->type = TYPE_DEFAULT;
        ret->type_num = TYPE_DEFAULT_INT;
    }
    if(typenum == TYPE_DOUBLE_INT) {
        ret->elsize = sizeof(double);
        ret->type   = TYPE_DOUBLE;
    }
    if(typenum == TYPE_BOOL_INT) {
        ret->elsize = sizeof(int);
        ret->type   = TYPE_BOOL;
    }
    if(typenum == TYPE_INTEGER_INT) {
        ret->elsize = sizeof(int);
        ret->type   = TYPE_INTEGER;
    }
    if(typenum == TYPE_FLOAT_INT) {
        ret->elsize = sizeof(float);
        ret->type   = TYPE_FLOAT;
    }
    if(typenum == TYPE_LONG_INT) {
        ret->elsize = sizeof(long);
        ret->type   = TYPE_LONG;
    }
    if (ret->f == NULL) {
        _select_carray_funcs(ret);
    }
    return ret;
}

/**
 * @return
 **/ 
CArray *
CArray_NewFromDescr( CArray *subtype, CArrayDescriptor *descr,
                     int nd, int *dims, int *strides, void *data,
                     int flags, CArray * base)
{
    return CArray_NewFromDescrAndBase(
            subtype, descr,
            nd, dims, strides, data,
            flags, base);
}

/**
 * @return
 **/ 
CArray *
CArray_New(CArray *subtype, int nd, int *dims, int type_num,
           int *strides, void *data, int itemsize, int flags, CArray * base)
{
    CArrayDescriptor *descr;
    CArray *new;

    descr = CArray_DescrFromType(type_num);
    if (descr == NULL) {
        return NULL;
    }

    new = CArray_NewFromDescr(subtype, descr, nd, dims, strides,
                              data, flags, base);
    return new;
}

/**
 * @return
 **/ 
CArray *
CArray_NewFromDescrAndBase(CArray * subtype, CArrayDescriptor * descr, int nd,
                           int * dims, int * strides, void * data, int flags,
                           CArray * base)
{
    return CArray_NewFromDescr_int(subtype, descr, nd, dims, strides, data, flags, base, 0, 0);
}



/**
 * @return
 */
CArray *
CArray_NewFromDescr_int(CArray * self, CArrayDescriptor *descr, int nd,
                        int *dims, int *strides, void *data,
                        int flags, CArray *base, int zeroed,
                        int allow_emptystring)
{
    int i, is_empty, num_elements = 0;
    uintptr_t nbytes;

    self->refcount = 0;
    nbytes = descr->elsize;
    is_empty = 0;

    for (i = 0; i < nd; i++) {
        uintptr_t dim = dims[i];

        if (dim == 0) {
            is_empty = 1;
            continue;
        }

        if (dim < 0) {
            throw_valueerror_exception("negative dimensions are not allowed");
            return NULL;
        }
    }
    
    self->ndim = nd;
    self->dimensions = NULL;
    self->data = NULL;

    if (data == NULL) {
        self->flags = CARRAY_ARRAY_DEFAULT;
        if (flags) {
            self->flags |= CARRAY_ARRAY_F_CONTIGUOUS;
            if (nd > 1) {
                self->flags &= ~CARRAY_ARRAY_C_CONTIGUOUS;
            }
            flags = CARRAY_ARRAY_F_CONTIGUOUS;
        } 
    } else {
        self->flags = (flags & ~CARRAY_ARRAY_WRITEBACKIFCOPY);
        self->flags &= ~CARRAY_ARRAY_UPDATEIFCOPY;
    }

    self->descriptor = descr;
    self->base =  NULL;
    self->refcount = 0;

    if (nd > 0) {
        self->dimensions = (int*)emalloc(nd * sizeof(int));
        self->strides = (int*)emalloc(nd * sizeof(int));
        
        if (self->dimensions == NULL) {
            throw_memory_exception("MemoryError");
            goto fail;
        }        
        memcpy(self->dimensions, dims, sizeof(int)*nd);
    
        for(i = 0; i < nd; i++) {
            if(i == 0) {
                num_elements = self->dimensions[i];
                continue;
            }
            num_elements = self->dimensions[i] * num_elements;
        }

        descr->numElements = num_elements;
        if (strides == NULL) {  
            _array_fill_strides(self->strides, dims, nd, descr->elsize, flags, &(self->flags));
        }
        else {
            memcpy(self->strides, strides, sizeof(int)*nd);
        }
    } else {
        descr->numElements = 1;
        num_elements = 1;
        self->dimensions = self->strides = NULL;
        self->flags |= CARRAY_ARRAY_F_CONTIGUOUS;
    }

    if (data == NULL) {
        if (is_empty) {
            nbytes = descr->elsize;
        }
        if (zeroed || CArrayDataType_FLAGCHK(descr, CARRAY_NEEDS_INIT)) {
            self->data = carray_data_alloc_zeros(num_elements, descr->elsize, CArray_TYPE(self));
        } else {
            self->data = carray_data_alloc(num_elements, descr->elsize);
        }
        if (self->data == NULL) {
            throw_memory_exception("MemoryError");
            goto fail;
        }
        self->flags |= CARRAY_ARRAY_OWNDATA;
    }
    else {
        self->flags &= ~CARRAY_ARRAY_OWNDATA;
        self->data = data;
    }
    
    CArray_UpdateFlags(self, CARRAY_ARRAY_UPDATE_ALL);

    if (base != NULL) {
        if(CArray_SetBaseCArray(self, base) < 0) {
            goto fail;
        }
    }
    
    if(self->descriptor->f == NULL) {
        _select_carray_funcs(self->descriptor);
    }
    
    return self;
fail:
    return NULL;
}

/**
 * check axis
 **/ 
CArray *
CArray_CheckAxis(CArray * arr, int * axis, int flags)
{
    CArray * temp1, * temp2;
    int n = CArray_NDIM(arr);

    if(axis == NULL) {
        return arr;
    }

    if (*axis == INT_MAX || n == 0) {
        if (n != 1) {
            temp1 = CArray_Ravel(arr, 0);
            if (temp1 == NULL) {
                *axis = 0;
                return NULL;
            }
            if (*axis == INT_MAX) {
                *axis = CArray_NDIM(temp1)-1;
            }
        } else {
            temp1 = arr;
            *axis = 0;
        }
        if (!flags && *axis == 0) {
            return temp1;
        }
    }
    else {
        temp1 = arr;
        CArray_INCREF(temp1);
    }

    if (flags) {
        temp2 = CArray_CheckFromAny(temp1, NULL, 0, 0, flags, NULL);
        //CArray_DECREF(temp1);
        if (temp2 == NULL) {
            return NULL;
        }
    } else {
        CArray_INCREF(temp1);
        temp2 = temp1;
    }

    CArray_DECREF(temp1);
    n = CArray_NDIM(temp2);
    if (check_and_adjust_axis(axis, n) < 0) {
        CArray_DECREF(temp2);
        return NULL;
    }
    return temp2;
}

int
CArray_ElementStrides(CArray *obj)
{
    CArray *arr;
    int itemsize;
    int i, ndim;
    int *strides;

    arr = obj;

    itemsize = CArray_ITEMSIZE(arr);
    ndim = CArray_NDIM(arr);
    strides = CArray_STRIDES(arr);

    for (i = 0; i < ndim; i++) {
        if ((strides[i] % itemsize) != 0) {
            return 0;
        }
    }
    return 1;
}

CArray *
CArray_CheckFromAny(CArray *op, CArrayDescriptor *descr, int min_depth,
                    int max_depth, int requires, CArray *context)
{
    CArray *obj;
    if (requires & CARRAY_ARRAY_NOTSWAPPED) {
        if (!descr &&
                CArray_ISBYTESWAPPED(op)) {
            descr = CArray_DescrNew(CArray_DESCR(op));
        }
        else if (descr && !CArray_ISNBO(descr->byteorder)) {
            CArray_DESCR_REPLACE(descr);
        }
        if (descr && descr->byteorder != CARRAY_IGNORE) {
            descr->byteorder = CARRAY_NATIVE;
        }
    }

    obj = CArray_FromCArray(op, descr, CArray_FLAGS(op));
    if (obj == NULL) {
        return NULL;
    }
    if ((requires & CARRAY_ARRAY_ELEMENTSTRIDES) &&
        !CArray_ElementStrides(obj)) {
        CArray *ret;
        ret = CArray_NewCopy(obj, CARRAY_ANYORDER);
        CArray_DECREF(obj);
        obj = ret;
    }
    return obj;
}

/**
 * @param array
 * @param index
 * @param current_dim
 */
static void
_print_recursive(CArray * array, CArrayIterator * iterator, int * index, int current_dim,
        int summarized, int x_summarized, int y_summarized, int notated, int has_digits,
        int max_digits)
{
    int offset;
    int i, index_jumps, j;
    index_jumps = ((int *)CArray_STRIDES(array))[current_dim] / CArray_ITEMSIZE(array);

    if(current_dim < array->ndim-1) {
        *index = 0;
        for (i = *index; i < array->dimensions[current_dim]; i++) {
            if (iterator->index >= CArray_DIMS(array)[CArray_NDIM(array)-1] && !current_dim) {

                // Break line for 2D stacks
                if (CArray_NDIM(array) == 3) {
                    php_printf("\n");
                }

                for (j = 0; j < current_dim + 1; j++) {
                    php_printf(" ");
                }
            } else if(current_dim &&
            (iterator->index % (CArray_DIMS(array)[CArray_NDIM(array)-1]*CArray_DIMS(array)[CArray_NDIM(array)-2]))) {
                for (j = 0; j < current_dim + 1; j++) {
                    php_printf(" ");
                }
            }
            php_printf("[");
            _print_recursive(array, iterator, index, current_dim + 1, summarized,
                    x_summarized, y_summarized, notated, has_digits, max_digits);
        }
    }
    if(current_dim >= array->ndim-1) {
        *index = *index + 1;
        char tmp_str_num[11];
        if(array->descriptor->type == TYPE_INTEGER) {
            int * value;
            if (notated) {
                for (i = 0; i < CArray_DIMS(array)[current_dim]; i++) {
                    value = (int *) CArrayIterator_DATA(iterator);
                    php_printf(" %.10e ", (double)*value);
                    CArrayIterator_NEXT(iterator);
                }
            } else {
                for (i = 0; i < CArray_DIMS(array)[current_dim]; i++) {
                    value = (int *) CArrayIterator_DATA(iterator);
                    snprintf(tmp_str_num, 11, "%d", *value);
                    offset = max_digits - strlen(tmp_str_num);
                    if (*value == INFINITY) {
                        offset = max_digits - 3;
                    }
                    for (j = 0; j < offset; j++) {
                        php_printf(" ");
                    }
                    php_printf(" %d ", *value);
                    CArrayIterator_NEXT(iterator);
                }
            }
        }
        if(array->descriptor->type == TYPE_DOUBLE) {
            char tmp_str_num[320];
            double * value;
            if (notated) {
                for (i = 0; i < CArray_DIMS(array)[current_dim]; i++) {
                    value = (double *) CArrayIterator_DATA(iterator);
                    php_printf(" %e ", (double)*value);
                    CArrayIterator_NEXT(iterator);
                }
            } else {
                if (has_digits) {
                    for (i = 0; i < CArray_DIMS(array)[current_dim]; i++) {
                        value = (double *) CArrayIterator_DATA(iterator);
                        snprintf(tmp_str_num, 320, "%.8f", *value);
                        offset = max_digits - strlen(tmp_str_num);
                        if (*value == INFINITY) {
                            offset = max_digits - 3;
                        }
                        for (j = 0; j < offset; j++) {
                            php_printf(" ");
                        }
                        php_printf(" %.8f ", *value);
                        CArrayIterator_NEXT(iterator);
                    }
                } else {
                    for (i = 0; i < CArray_DIMS(array)[current_dim]; i++) {
                        value = IT_DDATA(iterator);
                        snprintf(tmp_str_num, 320, "%.0f", *value);
                        offset = max_digits - strlen(tmp_str_num);
                        if (*value == INFINITY) {
                            offset = max_digits - 3;
                        }
                        for (j = 0; j < offset; j++) {
                            php_printf(" ");
                        }
                        php_printf(" %.0f. ", *value);
                        CArrayIterator_NEXT(iterator);
                    }
                }
            }
        }
    }
    if (iterator->index == (CArray_SIZE(array)) && current_dim < CArray_NDIM(array)) {
        php_printf("]");
        return;
    } else if (current_dim == CArray_NDIM(array) - 1) {
        if (!(iterator->index % (CArray_DIMS(array)[CArray_NDIM(array)-1]*CArray_DIMS(array)[CArray_NDIM(array)-2])) )
        {
            php_printf("]");
            return;
        } else {
            php_printf("]\n");
            return;
        }
    } else {
        php_printf("]\n");
        return;
    }
}


/**
 * @param array
 */
void
CArray_Print(CArray *array, int force_summary)
{
    int biggest_number_len = 0;
    int i;
    int has_digits = 0;
    int summarized = 0;
    int x_summarized = 0;
    int y_summarized = 0;
    int notated = 0;
    int start_index = 0;

    if(array->ndim == 0) {
        if(CArray_TYPE(array) == TYPE_DOUBLE_INT) {
            php_printf("%.17g", DDATA(array)[0]);
            return;
        }
        if(CArray_TYPE(array) == TYPE_INTEGER_INT) {
            php_printf("%d", IDATA(array)[0]);
            return;
        }
        if(CArray_TYPE(array) == TYPE_FLOAT_INT) {
            php_printf("%f", FDATA(array)[0]);
            return;
        }
    }

    if (!force_summary && CArray_SIZE(array) > 1200) {
        summarized = 1;
    } else if (force_summary) {
        summarized = 1;
    }

    if (CArray_DIMS(array)[CArray_NDIM(array)-1] > 6 && CArray_NDIM(array) > 1 && summarized) {
        x_summarized = 1;
    }

    if (CArray_NDIM(array) > 1 && CArray_DIMS(array)[CArray_NDIM(array) - 1] > 200 && summarized) {
        x_summarized = 1;
    }

    if (CArray_NDIM(array) >= 2 && summarized) {
        for (i = 0; i < CArray_NDIM(array)-1; i++) {
            if (CArray_DIMS(array)[i] > 12 && summarized) {
                y_summarized = 1;
            }
        }
    }
    php_printf("[");
    CArrayIterator * it = CArray_NewIter(array);

    if(CArray_TYPE(array) == TYPE_DOUBLE_INT) {
        char tmp_str_num[320];
        do {
            if ((int)IT_DDATA(it)[0] != IT_DDATA(it)[0]) {
                has_digits = 1;
            }
            if (IT_DDATA(it)[0] > 99999999) {
                notated = 1;
            }
            if (has_digits) {
                snprintf(tmp_str_num, 320, "%.8f", IT_DDATA(it)[0]);
            } else {
                snprintf(tmp_str_num, 320, "%.0f", IT_DDATA(it)[0]);
            }

            if (strlen(tmp_str_num) > biggest_number_len) {
                biggest_number_len = strlen(tmp_str_num);
            }
            if (notated && has_digits) {
                break;
            }
            CArrayIterator_NEXT(it);
        } while (CArrayIterator_NOTDONE(it));
    }
    if(CArray_TYPE(array) == TYPE_INTEGER_INT) {
        char tmp_str_num[11];
        do {
            snprintf(tmp_str_num, 11, "%d", IT_IDATA(it)[0]);
            if (strlen(tmp_str_num) > biggest_number_len) {
                biggest_number_len = strlen(tmp_str_num);
            }
            if (IT_IDATA(it)[0] > 99999999) {
                notated = 1;
                break;
            }
            CArrayIterator_NEXT(it);
        } while (CArrayIterator_NOTDONE(it));
    }

    CArrayIterator_RESET(it);
    _print_recursive(array, it, &start_index, 0, summarized, x_summarized, y_summarized, notated, has_digits, biggest_number_len);
    CArrayIterator_FREE(it);
    php_printf("\n");
}

/**
 * @param prototype
 * @param order
 * @param dtype
 * @param subok
 * @return
 */
CArray *
CArray_NewLikeArray(CArray *prototype, CARRAY_ORDER order, CArrayDescriptor *dtype, int subok)
{
    CArray *ret = emalloc(sizeof(CArray));
    int ndim = CArray_NDIM(prototype);

    /* If no override data type, use the one from the prototype */
    if (dtype == NULL) {
        dtype = CArray_DESCR(prototype);
        CArrayDescriptor_INCREF(dtype);
    }

    /* Handle ANYORDER and simple KEEPORDER cases */
    switch (order) {
        case CARRAY_ANYORDER:
            order = CArray_ISFORTRAN(prototype) ?
                    CARRAY_FORTRANORDER : CARRAY_CORDER;
            break;
        case CARRAY_KEEPORDER:
            if (CArray_IS_C_CONTIGUOUS(prototype) || ndim <= 1) {
                order = CARRAY_CORDER;
                break;
            }
            else if (CArray_IS_F_CONTIGUOUS(prototype)) {
                order = CARRAY_FORTRANORDER;
                break;
            }
            break;
        default:
            break;
    }

    if (order != CARRAY_KEEPORDER) {
        ret = CArray_NewFromDescr(ret,
                                   dtype,
                                   ndim,
                                   CArray_DIMS(prototype),
                                   NULL,
                                   NULL,
                                   order,
                                   subok ? prototype : NULL);
    } /* KEEPORDER needs some analysis of the strides */
    else {
        int strides[CARRAY_MAXDIMS], stride;
        int *shape = CArray_DIMS(prototype);
        ca_stride_sort_item strideperm[CARRAY_MAXDIMS];
        int idim;

        CArray_CreateSortedStridePerm(CArray_NDIM(prototype),
                                      CArray_STRIDES(prototype),
                                      strideperm);

        /* Build the new strides */
        stride = dtype->elsize;
        for (idim = ndim-1; idim >= 0; --idim) {
            int i_perm = strideperm[idim].perm;
            strides[i_perm] = stride;
            stride *= shape[i_perm];
        }

        /* Finally, allocate the array */
        ret = CArray_NewFromDescr(ret,
                                  dtype,
                                  ndim,
                                  shape,
                                  strides,
                                  NULL,
                                  0,
                                  subok ? prototype : NULL);
    }

    return ret;
}

/* 
 * Call this from contexts where an array might be written to, but we have no
 * way to tell. (E.g., when converting to a read-write buffer.)
 */
int
array_might_be_written(CArray *obj)
{
    if (CArray_FLAGS(obj) & CARRAY_ARRAY_WARN_ON_WRITE) {
        /* Only warn once per array */
        while (1) {
            CArray_CLEARFLAGS(obj, CARRAY_ARRAY_WARN_ON_WRITE);
            if (!CArray_BASE(obj)) {
                break;
            }
            obj = (CArray *)CArray_BASE(obj);
        }
    }
    return 0;
}

/**
 * This function does nothing if obj is writeable, and raises an exception
 * (and returns -1) if obj is not writeable. It may also do other
 * house-keeping, such as issuing warnings on arrays which are transitioning
 * to become views. Always call this function at some point before writing to
 * an array.
 **/ 
int
CArray_FailUnlessWriteable(CArray *obj, const char *name)
{
    if (!CArray_ISWRITEABLE(obj)) {
        char * msg = (char*)emalloc(strlen(name) + strlen(" is read-only"));
        sprintf(msg, "%s is read-only", name);
        throw_valueerror_exception(msg);
        return -1;
    }
    if (array_might_be_written(obj) < 0) {
        return -1;
    }
    return 0;
}

/**
 * Precondition: 'arr' is a copy of 'base' (though possibly with different
 * strides, ordering, etc.). This function sets the WRITEBACKIFCOPY flag and the
 * ->base pointer on 'arr'.
 * 
 * Steals a reference to 'base'.
 * 
 * Returns 0 on success, -1 on failure.
 **/ 
int
CArray_SetWritebackIfCopyBase(CArray *arr, CArray *base)
{
    if (base == NULL) {
        throw_valueerror_exception(
                  "Cannot WRITEBACKIFCOPY to NULL array");
        return -1;
    }
    if (CArray_BASE(arr) != NULL) {
        throw_valueerror_exception(
                  "Cannot set array with existing base to WRITEBACKIFCOPY");
        goto fail;
    }
    if (CArray_FailUnlessWriteable(base, "WRITEBACKIFCOPY base") < 0) {
        goto fail;
    }

    /*
     * Any writes to 'arr' will magically turn into writes to 'base', so we
     * should warn if necessary.
     */
    if (CArray_FLAGS(base) & CARRAY_ARRAY_WARN_ON_WRITE) {
        CArray_ENABLEFLAGS(arr, CARRAY_ARRAY_WARN_ON_WRITE);
    }

    /*
     * Unlike CArray_SetBaseObject, we do not compress the chain of base
     * references.
     */
    ((CArray *)arr)->base = (CArray *)base;
    CArray_ENABLEFLAGS(arr, CARRAY_ARRAY_WRITEBACKIFCOPY);
    CArray_CLEARFLAGS(base, CARRAY_ARRAY_WRITEABLE);

  fail:
    CArray_DECREF(base);
    return -1;
}

/**
 * @return
 */ 
int
CArray_CopyAsFlat(CArray *dst, CArray *src, CARRAY_ORDER order)
{
    //PyArray_StridedUnaryOp *stransfer = NULL;
    //NpyAuxData *transferdata = NULL;
    //NpyIter *dst_iter, *src_iter;

    //NpyIter_IterNextFunc *dst_iternext, *src_iternext;
    char **dst_dataptr, **src_dataptr;
    int dst_stride, src_stride;
    int *dst_countptr, *src_countptr;
    int baseflags;

    char *dst_data, *src_data;
    int dst_count, src_count, count;
    int src_itemsize;
    int dst_size, src_size;
    int needs_api;

    /*
     * If the shapes match and a particular order is forced
     * for both, use the more efficient CopyInto
     */
    if (order != CARRAY_ANYORDER && order != CARRAY_KEEPORDER &&
        CArray_NDIM(dst) == CArray_NDIM(src) &&
        CArray_CompareLists(CArray_DIMS(dst), CArray_DIMS(src),
        CArray_NDIM(dst))) {
            
            //return CArray_CopyInto(dst, src);
    }
}

/**
 * Copy an Array into another array -- memory must not overlap
 * Does not require src and dest to have "broadcastable" shapes
 * (only the same number of elements).
 */ 
int
CArray_CopyAnyInto(CArray *dst, CArray *src)
{
    return CArray_CopyAsFlat(dst, src, CARRAY_CORDER);
}

/**
 * If WRITEBACKIFCOPY and self has data, reset the base WRITEABLE flag,
 * copy the local data to base, release the local data, and set flags
 * appropriately. Return 0 if not relevant, 1 if success, < 0 on failure
 */ 
int
CArray_ResolveWritebackIfCopy(CArray * self)
{
    CArray *fa = (CArray *)self;
    if (fa && fa->base) {
        if ((fa->flags & CARRAY_ARRAY_UPDATEIFCOPY) || (fa->flags & CARRAY_ARRAY_WRITEBACKIFCOPY)) {
            /*
             * UPDATEIFCOPY or WRITEBACKIFCOPY means that fa->base's data
             * should be updated with the contents
             * of self.
             * fa->base->flags is not WRITEABLE to protect the relationship
             * unlock it.
             */
            int retval = 0;
            CArray_ENABLEFLAGS((fa->base),CARRAY_ARRAY_WRITEABLE);
            CArray_CLEARFLAGS(self, CARRAY_ARRAY_UPDATEIFCOPY);
            CArray_CLEARFLAGS(self, CARRAY_ARRAY_WRITEBACKIFCOPY);
            retval = CArray_CopyAnyInto(fa->base, self);
            CArray_DECREF(fa->base);
            fa->base = NULL;
            if (retval < 0) {
                /* this should never happen, how did the two copies of data
                 * get out of sync?
                 */
                return retval;
            }
            return 1;
        }
    }
    return 0;
}


/**
 * Convert MemoryPointer to CArray
 * @param ptr
 */
CArray *
CArray_FromMemoryPointer(MemoryPointer * ptr)
{
    return PHPSCI_MAIN_MEM_STACK.buffer[ptr->uuid];
}

CArrayDescriptor *
CArray_DescrNew(CArrayDescriptor * base)
{
    CArrayDescriptor * new;
    assert(base != NULL);

    new = (CArrayDescriptor *)emalloc(sizeof(CArrayDescriptor));
    if (new == NULL) {
        return NULL;
    }

    memcpy((char *)new, (char *)base, sizeof(CArrayDescriptor));

    return new;
}

int
CArray_EquivTypes(CArrayDescriptor * a, CArrayDescriptor * b)
{
    if(a->elsize == b->elsize) {
        return 1;
    }
    return 0;
}

int
CArray_EquivArrTypes(CArray * a, CArray * b)
{
    return CArray_EquivTypes(CArray_DESCR(a), CArray_DESCR(b));
}

/* If destination is not the right type, then src
   will be cast to destination -- this requires
   src and dest to have the same shape
*/
/* Requires arrays to have broadcastable shapes
   The arrays are assumed to have the same number of elements
   They can be different sizes and have different types however.
*/
static int
_array_copy_into(CArray *dest, CArray *src, int usecopy)
{
    strided_copy_func_t myfunc;
    int swap;
    int simple;
    int same;
    if (!CArray_EquivArrTypes(dest, src)) {
        return CArray_CastTo(dest, src);
    }

    if (!CArray_ISWRITEABLE(dest)) {
        throw_valueerror_exception("cannot write to array");
        return -1;
    }

    same = CArray_SAMESHAPE(dest, src);
    simple = same && ((CArray_ISCARRAY_RO(src) && CArray_ISCARRAY(dest)) ||
             (CArray_ISFARRAY_RO(src) && CArray_ISFARRAY(dest)));

    if (simple) {
        if (usecopy) {
            memcpy(dest->data, src->data, CArray_NBYTES(dest));
        }
        else {
            memmove(dest->data, src->data, CArray_NBYTES(dest));
        }
        return 0;
    }

    swap = CArray_ISNOTSWAPPED(dest) != CArray_ISNOTSWAPPED(src);

    /**if (src->nd == 0) {
        return _copy_from0d(dest, src, usecopy, swap);
    }**/

    myfunc = strided_copy_func(dest, src, usecopy);

    /*
     * Could combine these because _broadcasted_copy would work as well.
     * But, same-shape copying is so common we want to speed it up.
     */
    if (same) {
        return _copy_from_same_shape(dest, src, myfunc, swap);
    }
    else {
        return _broadcast_copy(dest, src, myfunc, swap);
    }
}

int
CArray_CopyInto(CArray * dest, CArray * src)
{
    return _array_copy_into(dest, src, 1);
}

CArray *
CArray_FromCArray(CArray * arr, CArrayDescriptor *newtype, int flags)
{
    CArray *ret = NULL;
    int copy = 0;
    int arrflags;
    CArrayDescriptor *oldtype;
    char *msg = "cannot copy back to a read-only array";
    int ensureArray = 0;
    CARRAY_CASTING casting = CARRAY_SAFE_CASTING;

    assert(NULL != arr);

    oldtype = CArray_DESCR(arr);
    if (newtype == NULL) {
        newtype = oldtype;
        CArrayDescriptor_INCREF(oldtype);
    } else if (CArrayDataType_ISUNSIZED(newtype)) {
        CArray_DESCR_REPLACE(newtype);
        if (newtype == NULL) {
            return NULL;
        }
        newtype->elsize = oldtype->elsize;
    }

    /* If the casting if forced, use the 'unsafe' casting rule */
    if (flags & CARRAY_ARRAY_FORCECAST) {
        casting = CARRAY_UNSAFE_CASTING;
    }

    if (!CArray_CanCastArrayTo(arr, newtype, casting)) {
        throw_typeerror_exception("Cannot cast array data");
    }

    /* Don't copy if sizes are compatible */
    if ((flags & CARRAY_ARRAY_ENSURECOPY) || CArray_EquivTypes(oldtype, newtype)) {
        arrflags = arr->flags;
        copy = (flags & CARRAY_ARRAY_ENSURECOPY) ||
        ((flags & CARRAY_ARRAY_C_CONTIGUOUS) && (!(arrflags & CARRAY_ARRAY_C_CONTIGUOUS)))
        || ((flags & CARRAY_ARRAY_ALIGNED) && (!(arrflags & CARRAY_ARRAY_ALIGNED)))
        || (arr->ndim > 1 &&
            ((flags & CARRAY_ARRAY_F_CONTIGUOUS) && (!(arrflags & CARRAY_ARRAY_F_CONTIGUOUS))))
        || ((flags & CARRAY_ARRAY_WRITEABLE) && (!(arrflags & CARRAY_ARRAY_WRITEABLE)));

        if (copy) {
            if ((flags & CARRAY_ARRAY_UPDATEIFCOPY) &&
                (!CArray_ISWRITEABLE(arr))) {
                CArrayDescriptor_DECREF(newtype);
                throw_valueerror_exception(msg);
                return NULL;
            }
        
            if ((flags & CARRAY_ARRAY_ENSUREARRAY)) {
                ensureArray = 1;
            }

            ret = CArray_Alloc(newtype, arr->ndim, arr->dimensions,
                                    flags & CARRAY_ARRAY_F_CONTIGUOUS,
                                    ensureArray ? NULL : arr);

            if (ret == NULL) {
                return NULL;
            }

            if (CArray_CopyInto(ret, arr) == -1) {
                return NULL;
            }
            /**if (flags & NPY_UPDATEIFCOPY)  {
                ret->flags |= NPY_UPDATEIFCOPY;
                ret->base_arr = arr;
                assert(NULL == ret->base_arr || NULL == ret->base_obj);
                NpyArray_FLAGS(ret->base_arr) &= ~NPY_WRITEABLE;
                Npy_INCREF(arr);
            }  **/   
        } else {

            CArrayDescriptor_DECREF(newtype);
            if (flags & CARRAY_ARRAY_ENSUREARRAY)  {
                CArrayDescriptor_INCREF(CArray_DESCR(arr));
                /**ret = CArray_NewView(CArray_DESCR(arr), arr->ndim, arr->dimensions, arr->strides,
                                    arr, 0, 1);**/
                if (ret == NULL) {
                    return NULL;
                } 
            } else {
                ret = arr;
                CArray_INCREF(arr);
            }
        }
    } else {
        if ((flags & CARRAY_ARRAY_UPDATEIFCOPY) &&
            (!CArray_ISWRITEABLE(arr))) {
            CArrayDescriptor_DECREF(newtype);
            throw_valueerror_exception(msg);
            return NULL;
        }
        if ((flags & CARRAY_ARRAY_ENSUREARRAY)) {
            ensureArray = 1;
        }

        ret = CArray_Alloc(newtype, arr->ndim, arr->dimensions,
                           flags, ensureArray ? NULL : arr);

        if (ret == NULL) {
            return NULL;
        }
        
        if (CArray_CastTo(ret, arr) < 0) {
            CArray_DECREF(ret);
            return NULL;
        }

        if (flags & CARRAY_ARRAY_UPDATEIFCOPY)  {
            ret->flags |= CARRAY_ARRAY_UPDATEIFCOPY;
            ret->base = arr;
            ret->base->flags &= ~CARRAY_ARRAY_WRITEABLE;
            CArray_INCREF(arr);
        } else {
            CArray_DECREF(arr);
            CArray_Free(arr);
        }

    }
    return ret;
}

CArray *
CArray_FromAnyUnwrap(CArray *op, CArrayDescriptor *newtype, int min_depth,
                     int max_depth, int flags, CArray *context)
{
    CArray *r = NULL;
    int seq = 0;

    r = CArray_FromCArray(op, newtype, flags);

    /* If we didn't succeed return NULL */
    if (r == NULL) {
        return NULL;
    }

    if (min_depth != 0 && (CArray_NDIM(r) < min_depth)) {
        throw_valueerror_exception("object of too small depth for desired array");
        CArray_DECREF(r);
        return NULL;
    }
    if (max_depth != 0 && (CArray_NDIM(r) > max_depth)) {
        throw_valueerror_exception("object too deep for desired array");
        CArray_DECREF(r);
        return NULL;
    }
    return r;
}

/**
 * ca::empty
 **/ 
CArray *
CArray_Empty(int nd, int *dims, CArrayDescriptor *type, int fortran, MemoryPointer * ptr)
{
    CArray *ret = ecalloc(1, sizeof(CArray));
    if (type == NULL) {
        type = CArray_DescrFromType(TYPE_DEFAULT_INT);
    }

    ret = (CArray *)CArray_NewFromDescr( ret, type, nd, dims,
                                         NULL, NULL, CARRAY_CORDER, NULL );
                                  
    if (ret == NULL) {
        return NULL;
    }

    if(ptr != NULL) {
        add_to_buffer(ptr, ret, sizeof(CArray));
    }
    
    return ret;
}

/**
 * ca::eye
 **/ 
CArray *
CArray_Eye(int n, int m, int k, char * dtype, MemoryPointer * out)
{
    int i, aux = 0;
    int dims[2];
    CArray * target;

    dims[0] = n;
    dims[1] = m;

    if(dtype != NULL) {
        target = CArray_Zeros(dims, 2, *dtype, NULL, out);
    } else {
        target = CArray_Zeros(dims, 2, TYPE_DOUBLE, NULL, out);
    }

    if (k >= m) {
        return target;
    }

    if (k >= 0) {
        i = k;
    } else {
        i = (-k) * m;
    }
    
    if(CArray_TYPE(target) == TYPE_DOUBLE_INT) {
        for(aux = i; aux < ((m*n)/(n/m)); aux+=m+1) {
            DDATA(target)[aux] = 1;
        }
    }
    if(CArray_TYPE(target) == TYPE_INTEGER_INT) {
         for(aux = i; aux < ((m*n)/(n/m)); aux+=m+1) {
            IDATA(target)[aux] = 1;
        }
    }

    return target;
}

CArray *
CArray_FromAny(CArray *op, CArrayDescriptor *newtype, int min_depth,
               int max_depth, int flags)
{
    /*
     * This is the main code to make a NumPy array from a Python
     * Object.  It is called from many different places.
     */
    CArray *arr = NULL, *ret = NULL;
    CArrayDescriptor *dtype = NULL;
    int ndim = 0;
    int dims[CARRAY_MAXDIMS];

    arr = op;

    /* If we got dimensions and dtype instead of an array */
    if (arr == NULL) {
        if (flags & CARRAY_ARRAY_UPDATEIFCOPY) {
            CArrayDescriptor_DECREF(newtype);
            throw_typeerror_exception("UPDATEIFCOPY used for non-array input.");
            return NULL;
        }
        else if (min_depth != 0 && ndim < min_depth) {
            CArrayDescriptor_DECREF(dtype);
            CArrayDescriptor_DECREF(newtype);
            throw_valueerror_exception("object of too small depth for desired array");
            ret = NULL;
        }
        else if (max_depth != 0 && ndim > max_depth) {
            CArrayDescriptor_DECREF(dtype);
            CArrayDescriptor_DECREF(newtype);
            throw_valueerror_exception("object too deep for desired array");
            ret = NULL;
        }
            //else if (ndim == 0) {
            //throw_notimplemented_exception();
            //ret = CArray_FromScalar(op, newtype);
            //CArrayDescriptor_DECREF(dtype);
            //}
        else {
            if (newtype == NULL) {
                newtype = dtype;
            }
            else {
                CArrayDescriptor_DECREF(dtype);
            }

            /* Create an array and copy the data */
            ret = CArray_NewFromDescr(ret, newtype, ndim, dims, NULL, NULL,
                                      flags&CARRAY_ARRAY_F_CONTIGUOUS, NULL);
            if (ret == NULL) {
                return NULL;
            }

            if (ndim > 0) {
                if (CArray_AssignFromSequence(ret, op) < 0) {
                    CArray_DECREF(ret);
                    ret = NULL;
                }
            }
            else {
                if (CArray_DESCR(ret)->f->setitem(op,CArray_DATA(ret), ret) < 0) {
                    CArray_DECREF(ret);
                    ret = NULL;
                }
            }
        }
    }
    else {
        if (min_depth != 0 && CArray_NDIM(arr) < min_depth) {
            throw_valueerror_exception("object of too small depth for desired array");
            CArray_DECREF(arr);
            ret = NULL;
        }
        else if (max_depth != 0 && CArray_NDIM(arr) > max_depth) {
            throw_valueerror_exception("object too deep for desired array");
            CArray_DECREF(arr);
            ret = NULL;
        }
        else {
            ret = CArray_FromArray(arr, newtype, flags);
            CArray_DECREF(arr);
        }
    }

    return ret;
}

#pragma clang diagnostic pop