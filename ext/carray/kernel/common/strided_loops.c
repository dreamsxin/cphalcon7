#include "../carray.h"
#include "strided_loops.h"
#include "../include/cpu.h"

#define _CARRAY_NOP1(x) (x)
#define _CARRAY_NOP2(x) (x)
#define _CARRAY_NOP4(x) (x)
#define _CARRAY_NOP8(x) (x)

#define _CARRAY_SWAP2(x) carray_bswap2(x)

#define _CARRAY_SWAP4(x) carray_bswap4(x)

#define _CARRAY_SWAP_PAIR4(x) (((((uint32_t)x)&0xffu) << 8) | \
                       ((((uint32_t)x)&0xff00u) >> 8) | \
                       ((((uint32_t)x)&0xff0000u) << 8) | \
                       ((((uint32_t)x)&0xff000000u) >> 8))

#define _CARRAY_SWAP8(x) carray_bswap8(x)

#define _CARRAY_SWAP_PAIR8(x) (((((uint64_t)x)&0xffULL) << 24) | \
                       ((((uint64_t)x)&0xff00ULL) << 8) | \
                       ((((uint64_t)x)&0xff0000ULL) >> 8) | \
                       ((((uint64_t)x)&0xff000000ULL) >> 24) | \
                       ((((uint64_t)x)&0xff00000000ULL) << 24) | \
                       ((((uint64_t)x)&0xff0000000000ULL) << 8) | \
                       ((((uint64_t)x)&0xff000000000000ULL) >> 8) | \
                       ((((uint64_t)x)&0xff00000000000000ULL) >> 24))

#define _CARRAY_SWAP_INPLACE2(x) carray_bswap2_unaligned(x)

#define _CARRAY_SWAP_INPLACE4(x) carray_bswap4_unaligned(x)

#define _CARRAY_SWAP_INPLACE8(x) carray_bswap8_unaligned(x)

#define _CARRAY_SWAP_INPLACE16(x) { \
        char a = (x)[0]; (x)[0] = (x)[15]; (x)[15] = a; \
        a = (x)[1]; (x)[1] = (x)[14]; (x)[14] = a; \
        a = (x)[2]; (x)[2] = (x)[13]; (x)[13] = a; \
        a = (x)[3]; (x)[3] = (x)[12]; (x)[12] = a; \
        a = (x)[4]; (x)[4] = (x)[11]; (x)[11] = a; \
        a = (x)[5]; (x)[5] = (x)[10]; (x)[10] = a; \
        a = (x)[6]; (x)[6] = (x)[9]; (x)[9] = a; \
        a = (x)[7]; (x)[7] = (x)[8]; (x)[8] = a; \
        }

/*
 * x86 platform works with unaligned access but the compiler is allowed to
 * assume all data is aligned to its size by the C standard. This means it can
 * vectorize instructions peeling only by the size of the type, if the data is
 * not aligned to this size one ends up with data not correctly aligned for SSE
 * instructions (16 byte).
 * So this flag can only be enabled if autovectorization is disabled.
 */
#if CARRAY_CPU_HAVE_UNALIGNED_ACCESS
    #  define CARRAY_USE_UNALIGNED_ACCESS 0
#else
    #  define CARRAY_USE_UNALIGNED_ACCESS 0
#endif

/*
 * specialized copy and swap for source stride 0,
 * interestingly unrolling here is like above is only marginally profitable for
 * small types and detrimental for >= 8byte moves on x86
 * but it profits from vectorization enabled with -O3
 */
static void
_aligned_strided_to_contig_size1_srcstride0(char *dst,
                        int dst_stride,
                        char *src, int CARRAY_UNUSED(src_stride),
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 1 != 16
#  if !(1 == 1 && 1)
    uint8_t temp;
#  endif
#else
    uint64_t temp0, temp1;
#endif
    if (N == 0) {
        return;
    }
#if 1 && 1 != 16
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint8_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint8_t))));
#endif
#if 1 == 1 && 1
    memset(dst, *src, N);
#else

#  if 1 != 16
    temp = _CARRAY_NOP1(*((uint8_t *)src));
#  else
#    if 0 == 0
        temp0 = (*((uint64_t *)src));
        temp1 = (*((uint64_t *)src + 1));
#    elif 0 == 1
        temp0 = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        temp1 = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        temp0 = _CARRAY_SWAP8(*((uint64_t *)src));
        temp1 = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

    while (N > 0) {
#  if 1 != 16
        *((uint64_t *)dst) = temp;
#  else
        *((uint64_t *)dst) = temp0;
        *((uint64_t *)dst + 1) = temp1;
#  endif
#  if 1
        dst += 1;
#  else
        dst += dst_stride;
#  endif
        --N;
    }
#endif/* @elsize == 1 && 1 -- else */
}



/*
 * specialized copy and swap for source stride 0,
 * interestingly unrolling here is like above is only marginally profitable for
 * small types and detrimental for >= 8byte moves on x86
 * but it profits from vectorization enabled with -O3
 */
static void
_aligned_strided_to_contig_size2_srcstride0(char *dst,
                        int dst_stride,
                        char *src, int CARRAY_UNUSED(src_stride),
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 2 != 16
#  if !(2 == 1 && 1)
    uint16_t temp;
#  endif
#else
    uint64_t temp0, temp1;
#endif
    if (N == 0) {
        return;
    }
#if 1 && 2 != 16
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint16_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint16_t))));
#endif
#if 2 == 1 && 1
    memset(dst, *src, N);
#else

#  if 2 != 16
    temp = _CARRAY_NOP2(*((uint16_t *)src));
#  else
#    if 0 == 0
        temp0 = (*((uint64_t *)src));
        temp1 = (*((uint64_t *)src + 1));
#    elif 0 == 1
        temp0 = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        temp1 = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        temp0 = _CARRAY_SWAP8(*((uint64_t *)src));
        temp1 = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

    while (N > 0) {
#  if 2 != 16
        *((uint16_t *)dst) = temp;
#  else
        *((uint64_t *)dst) = temp0;
        *((uint64_t *)dst + 1) = temp1;
#  endif
#  if 1
        dst += 2;
#  else
        dst += dst_stride;
#  endif
        --N;
    }
#endif/* @elsize == 1 && 1 -- else */
}


/*
 * specialized copy and swap for source stride 0,
 * interestingly unrolling here is like above is only marginally profitable for
 * small types and detrimental for >= 8byte moves on x86
 * but it profits from vectorization enabled with -O3
 */
static void
_aligned_strided_to_contig_size4_srcstride0(char *dst,
                        int dst_stride,
                        char *src, int CARRAY_UNUSED(src_stride),
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 4 != 16
#  if !(4 == 1 && 1)
    uint32_t temp;
#  endif
#else
    uint64_t temp0, temp1;
#endif
    if (N == 0) {
        return;
    }
#if 1 && 4 != 16
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint32_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint32_t))));
#endif
#if 4 == 1 && 1
    memset(dst, *src, N);
#else

#  if 4 != 16
    temp = _CARRAY_NOP4(*((uint32_t *)src));
#  else
#    if 0 == 0
        temp0 = (*((uint64_t *)src));
        temp1 = (*((uint64_t *)src + 1));
#    elif 0 == 1
        temp0 = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        temp1 = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        temp0 = _CARRAY_SWAP8(*((uint64_t *)src));
        temp1 = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

    while (N > 0) {
#  if 4 != 16
        *((uint32_t *)dst) = temp;
#  else
        *((uint64_t *)dst) = temp0;
        *((uint64_t *)dst + 1) = temp1;
#  endif
#  if 1
        dst += 4;
#  else
        dst += dst_stride;
#  endif
        --N;
    }
#endif/* @elsize == 1 && 1 -- else */
}



/*
 * specialized copy and swap for source stride 0,
 * interestingly unrolling here is like above is only marginally profitable for
 * small types and detrimental for >= 8byte moves on x86
 * but it profits from vectorization enabled with -O3
 */
static void
_aligned_strided_to_contig_size8_srcstride0(char *dst,
                        int dst_stride,
                        char *src, int CARRAY_UNUSED(src_stride),
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 8 != 16
#  if !(8 == 1 && 1)
    uint64_t temp;
#  endif
#else
    uint64_t temp0, temp1;
#endif
    if (N == 0) {
        return;
    }
#if 1 && 8 != 16
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint64_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint64_t))));
#endif
#if 8 == 1 && 1
    memset(dst, *src, N);
#else

#  if 8 != 16
    temp = _CARRAY_NOP8(*((uint64_t *)src));
#  else
#    if 0 == 0
        temp0 = (*((uint64_t *)src));
        temp1 = (*((uint64_t *)src + 1));
#    elif 0 == 1
        temp0 = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        temp1 = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        temp0 = _CARRAY_SWAP8(*((uint64_t *)src));
        temp1 = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

    while (N > 0) {
#  if 8 != 16
        *((uint64_t *)dst) = temp;
#  else
        *((uint64_t *)dst) = temp0;
        *((uint64_t *)dst + 1) = temp1;
#  endif
#  if 1
        dst += 8;
#  else
        dst += dst_stride;
#  endif
        --N;
    }
#endif/* @elsize == 1 && 1 -- else */
}


/*
 * specialized copy and swap for source stride 0,
 * interestingly unrolling here is like above is only marginally profitable for
 * small types and detrimental for >= 8byte moves on x86
 * but it profits from vectorization enabled with -O3
 */
static void
_aligned_strided_to_contig_size16_srcstride0(char *dst,
                        int dst_stride,
                        char *src, int CARRAY_UNUSED(src_stride),
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 16 != 16
#  if !(16 == 1 && 1)
    uint64_t temp;
#  endif
#else
    uint64_t temp0, temp1;
#endif
    if (N == 0) {
        return;
    }
#if 1 && 16 != 16
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint64_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint64_t))));
#endif
#if 16 == 1 && 1
    memset(dst, *src, N);
#else

#  if 16 != 16
    temp = _CARRAY_NOP16(*((uint64_t *)src));
#  else
#    if 0 == 0
        temp0 = (*((uint64_t *)src));
        temp1 = (*((uint64_t *)src + 1));
#    elif 0 == 1
        temp0 = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        temp1 = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        temp0 = _CARRAY_SWAP8(*((uint64_t *)src));
        temp1 = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

    while (N > 0) {
#  if 16 != 16
        *((uint64_t *)dst) = temp;
#  else
        *((uint64_t *)dst) = temp0;
        *((uint64_t *)dst + 1) = temp1;
#  endif
#  if 1
        dst += 16;
#  else
        dst += dst_stride;
#  endif
        --N;
    }
#endif/* @elsize == 1 && 1 -- else */
}


/*
 * unrolling gains about 20-50% if the copy can be done in one mov instruction
 * if not it can decrease performance
 * tested to improve performance on intel xeon 5x/7x, core2duo, amd phenom x4
 */
static void
_aligned_strided_to_contig_size1(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 1
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint8_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint8_t))));
#endif
    /*printf("fn _aligned_strided_to_contig_size1\n");*/
    while (N > 0) {
#if 1

        /* aligned copy and swap */
#  if 1 != 16
        (*((uint8_t *)dst)) = _CARRAY_NOP1(*((uint8_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 1);
#  if 0 == 1
        _CARRAY_NOP1(dst);
#  elif 0 == 2
        _CARRAY_NOP0(dst);
        _CARRAY_NOP0(dst + 0);
#  endif

#endif

#if 1
        dst += 1;
#else
        dst += dst_stride;
#endif

#if 0
        src += 1;
#else
        src += src_stride;
#endif

        --N;
    }
}




/*
 * unrolling gains about 20-50% if the copy can be done in one mov instruction
 * if not it can decrease performance
 * tested to improve performance on intel xeon 5x/7x, core2duo, amd phenom x4
 */
static void
_aligned_strided_to_contig_size2(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 1
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint16_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint16_t))));
#endif
    /*printf("fn _aligned_strided_to_contig_size2\n");*/
    while (N > 0) {
#if 1

        /* aligned copy and swap */
#  if 2 != 16
        (*((uint16_t *)dst)) = _CARRAY_NOP2(*((uint16_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 2);
#  if 0 == 1
        _CARRAY_NOP2(dst);
#  elif 0 == 2
        _CARRAY_NOP1(dst);
        _CARRAY_NOP1(dst + 1);
#  endif

#endif

#if 1
        dst += 2;
#else
        dst += dst_stride;
#endif

#if 0
        src += 2;
#else
        src += src_stride;
#endif

        --N;
    }
}




/*
 * unrolling gains about 20-50% if the copy can be done in one mov instruction
 * if not it can decrease performance
 * tested to improve performance on intel xeon 5x/7x, core2duo, amd phenom x4
 */
static void
_aligned_strided_to_contig_size4(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 1
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint32_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint32_t))));
#endif
    /*printf("fn _aligned_strided_to_contig_size4\n");*/
    while (N > 0) {
#if 1

        /* aligned copy and swap */
#  if 4 != 16
        (*((uint32_t *)dst)) = _CARRAY_NOP4(*((uint32_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else
        memmove(dst, src, 4);
#  if 0 == 1
        _CARRAY_NOP4(dst);
#  elif 0 == 2
        _CARRAY_NOP2(dst);
        _CARRAY_NOP2(dst + 2);
#  endif

#endif

#if 1
        dst += 4;
#else
        dst += dst_stride;
#endif

#if 0
        src += 4;
#else
        src += src_stride;
#endif

        --N;
    }
}


/*
 * unrolling gains about 20-50% if the copy can be done in one mov instruction
 * if not it can decrease performance
 * tested to improve performance on intel xeon 5x/7x, core2duo, amd phenom x4
 */
static void
_aligned_strided_to_contig_size8(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 1
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint64_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint64_t))));
#endif
    /*printf("fn _aligned_strided_to_contig_size8\n");*/
    while (N > 0) {
#if 1

        /* aligned copy and swap */
#  if 8 != 16
        (*((uint64_t *)dst)) = _CARRAY_NOP8(*((uint64_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 8);
#  if 0 == 1
        _CARRAY_NOP8(dst);
#  elif 0 == 2
        _CARRAY_NOP4(dst);
        _CARRAY_NOP4(dst + 4);
#  endif

#endif

#if 1
        dst += 8;
#else
        dst += dst_stride;
#endif

#if 0
        src += 8;
#else
        src += src_stride;
#endif

        --N;
    }
}


/*
 * unrolling gains about 20-50% if the copy can be done in one mov instruction
 * if not it can decrease performance
 * tested to improve performance on intel xeon 5x/7x, core2duo, amd phenom x4
 */
static void
_aligned_strided_to_contig_size16(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 1
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint64_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint64_t))));
#endif
    /*printf("fn _aligned_strided_to_contig_size16\n");*/
    while (N > 0) {
#if 1

        /* aligned copy and swap */
#  if 16 != 16
        (*((uint64_t *)dst)) = _CARRAY_NOP16(*((uint64_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 16);
#  if 0 == 1
        _CARRAY_NOP16(dst);
#  elif 0 == 2
        _CARRAY_NOP8(dst);
        _CARRAY_NOP8(dst + 8);
#  endif

#endif

#if 1
        dst += 16;
#else
        dst += dst_stride;
#endif

#if 0
        src += 16;
#else
        src += src_stride;
#endif

        --N;
    }
}


static void
_aligned_strided_to_strided_size1(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 1
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint8_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint8_t))));
#endif
    /*printf("fn _aligned_strided_to_strided_size1\n");*/
    while (N > 0) {
#if 1

        /* aligned copy and swap */
#  if 1 != 16
        (*((uint8_t *)dst)) = _CARRAY_NOP1(*((uint8_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 1);
#  if 0 == 1
        _CARRAY_NOP1(dst);
#  elif 0 == 2
        _CARRAY_NOP0(dst);
        _CARRAY_NOP0(dst + 0);
#  endif

#endif

#if 0
        dst += 1;
#else
        dst += dst_stride;
#endif

#if 0
        src += 1;
#else
        src += src_stride;
#endif

        --N;
    }
}

/*
 * unrolling gains about 20-50% if the copy can be done in one mov instruction
 * if not it can decrease performance
 * tested to improve performance on intel xeon 5x/7x, core2duo, amd phenom x4
 */
static void
_aligned_strided_to_strided_size4(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 1
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint32_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint32_t))));
#endif
    /*printf("fn _aligned_strided_to_strided_size4\n");*/
    while (N > 0) {
#if 1

        /* aligned copy and swap */
#  if 4 != 16
        (*((uint32_t *)dst)) = _CARRAY_NOP4(*((uint32_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 4);
#  if 0 == 1
        _CARRAY_NOP4(dst);
#  elif 0 == 2
        _CARRAY_NOP2(dst);
        _CARRAY_NOP2(dst + 2);
#  endif

#endif

#if 0
        dst += 4;
#else
        dst += dst_stride;
#endif

#if 0
        src += 4;
#else
        src += src_stride;
#endif

        --N;
    }
}


/*
 * unrolling gains about 20-50% if the copy can be done in one mov instruction
 * if not it can decrease performance
 * tested to improve performance on intel xeon 5x/7x, core2duo, amd phenom x4
 */
static void
_aligned_strided_to_strided_size8(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 1
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint64_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint64_t))));
#endif
    /*printf("fn _aligned_strided_to_strided_size8\n");*/
    while (N > 0) {
#if 1

        /* aligned copy and swap */
#  if 8 != 16
        (*((uint64_t *)dst)) = _CARRAY_NOP8(*((uint64_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 8);
#  if 0 == 1
        _CARRAY_NOP8(dst);
#  elif 0 == 2
        _CARRAY_NOP4(dst);
        _CARRAY_NOP4(dst + 4);
#  endif

#endif

#if 0
        dst += 8;
#else
        dst += dst_stride;
#endif

#if 0
        src += 8;
#else
        src += src_stride;
#endif

        --N;
    }
}

static void
_aligned_strided_to_strided_size16(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 1
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint64_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint64_t))));
#endif
    /*printf("fn _aligned_strided_to_strided_size16\n");*/
    while (N > 0) {
#if 1

        /* aligned copy and swap */
#  if 16 != 16
        (*((uint64_t *)dst)) = _CARRAY_NOP16(*((uint64_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 16);
#  if 0 == 1
        _CARRAY_NOP16(dst);
#  elif 0 == 2
        _CARRAY_NOP8(dst);
        _CARRAY_NOP8(dst + 8);
#  endif

#endif

#if 0
        dst += 16;
#else
        dst += dst_stride;
#endif

#if 0
        src += 16;
#else
        src += src_stride;
#endif

        --N;
    }
}


/*
 * specialized copy and swap for source stride 0,
 * interestingly unrolling here is like above is only marginally profitable for
 * small types and detrimental for >= 8byte moves on x86
 * but it profits from vectorization enabled with -O3
 */
static void
_aligned_strided_to_strided_size1_srcstride0(char *dst,
                        int dst_stride,
                        char *src, int CARRAY_UNUSED(src_stride),
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 1 != 16
#  if !(1 == 1 && 0)
    uint8_t temp;
#  endif
#else
    uint64_t temp0, temp1;
#endif
    if (N == 0) {
        return;
    }
#if 1 && 1 != 16
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst,carray_uint_alignment(sizeof(uint8_t))));
    assert(N == 0 || carray_is_aligned(src,carray_uint_alignment(sizeof(uint8_t))));
#endif
#if 1 == 1 && 0
    memset(dst, *src, N);
#else

#  if 1 != 16
    temp = _CARRAY_NOP1(*((uint8_t *)src));
#  else
#    if 0 == 0
        temp0 = (*((uint64_t *)src));
        temp1 = (*((uint64_t *)src + 1));
#    elif 0 == 1
        temp0 = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        temp1 = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        temp0 = _CARRAY_SWAP8(*((uint64_t *)src));
        temp1 = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

    while (N > 0) {
#  if 1 != 16
        *((uint8_t *)dst) = temp;
#  else
        *((uint64_t *)dst) = temp0;
        *((uint64_t *)dst + 1) = temp1;
#  endif
#  if 0
        dst += 1;
#  else
        dst += dst_stride;
#  endif
        --N;
    }
#endif/* @elsize == 1 && 0 -- else */
}


/*
 * specialized copy and swap for source stride 0,
 * interestingly unrolling here is like above is only marginally profitable for
 * small types and detrimental for >= 8byte moves on x86
 * but it profits from vectorization enabled with -O3
 */
static void
_aligned_strided_to_strided_size2_srcstride0(char *dst,
                        int dst_stride,
                        char *src, int CARRAY_UNUSED(src_stride),
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 2 != 16
#  if !(2 == 1 && 0)
    uint16_t temp;
#  endif
#else
    uint64_t temp0, temp1;
#endif
    if (N == 0) {
        return;
    }
#if 1 && 2 != 16
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint16_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint16_t))));
#endif
#if 2 == 1 && 0
    memset(dst, *src, N);
#else

#  if 2 != 16
    temp = _CARRAY_NOP2(*((uint16_t *)src));
#  else
#    if 0 == 0
        temp0 = (*((uint64_t *)src));
        temp1 = (*((uint64_t *)src + 1));
#    elif 0 == 1
        temp0 = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        temp1 = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        temp0 = _CARRAY_SWAP8(*((uint64_t *)src));
        temp1 = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

    while (N > 0) {
#  if 2 != 16
        *((uint16_t *)dst) = temp;
#  else
        *((uint64_t *)dst) = temp0;
        *((uint64_t *)dst + 1) = temp1;
#  endif
#  if 0
        dst += 2;
#  else
        dst += dst_stride;
#  endif
        --N;
    }
#endif/* @elsize == 1 && 0 -- else */
}



/*
 * specialized copy and swap for source stride 0,
 * interestingly unrolling here is like above is only marginally profitable for
 * small types and detrimental for >= 8byte moves on x86
 * but it profits from vectorization enabled with -O3
 */
static void
_aligned_strided_to_strided_size4_srcstride0(char *dst,
                        int dst_stride,
                        char *src, int CARRAY_UNUSED(src_stride),
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 4 != 16
#  if !(4 == 1 && 0)
    uint32_t temp;
#  endif
#else
    uint64_t temp0, temp1;
#endif
    if (N == 0) {
        return;
    }
#if 1 && 4 != 16
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint32_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint32_t))));
#endif
#if 4 == 1 && 0
    memset(dst, *src, N);
#else

#  if 4 != 16
    temp = _CARRAY_NOP4(*((uint32_t *)src));
#  else
#    if 0 == 0
        temp0 = (*((uint64_t *)src));
        temp1 = (*((uint64_t *)src + 1));
#    elif 0 == 1
        temp0 = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        temp1 = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        temp0 = _CARRAY_SWAP8(*((uint64_t *)src));
        temp1 = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

    while (N > 0) {
#  if 4 != 16
        *((uint32_t *)dst) = temp;
#  else
        *((uint64_t *)dst) = temp0;
        *((uint64_t *)dst + 1) = temp1;
#  endif
#  if 0
        dst += 4;
#  else
        dst += dst_stride;
#  endif
        --N;
    }
#endif/* @elsize == 1 && 0 -- else */
}


/*
 * specialized copy and swap for source stride 0,
 * interestingly unrolling here is like above is only marginally profitable for
 * small types and detrimental for >= 8byte moves on x86
 * but it profits from vectorization enabled with -O3
 */
#if (0 == 0) && 1
static void
_aligned_strided_to_strided_size8_srcstride0(char *dst,
                        int dst_stride,
                        char *src, int CARRAY_UNUSED(src_stride),
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 8 != 16
#  if !(8 == 1 && 0)
    uint64_t temp;
#  endif
#else
    uint64_t temp0, temp1;
#endif
    if (N == 0) {
        return;
    }
#if 1 && 8 != 16
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint64_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint64_t))));
#endif
#if 8 == 1 && 0
    memset(dst, *src, N);
#else

#  if 8 != 16
    temp = _CARRAY_NOP8(*((uint64_t *)src));
#  else
#    if 0 == 0
        temp0 = (*((uint64_t *)src));
        temp1 = (*((uint64_t *)src + 1));
#    elif 0 == 1
        temp0 = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        temp1 = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        temp0 = _CARRAY_SWAP8(*((uint64_t *)src));
        temp1 = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

    while (N > 0) {
#  if 8 != 16
        *((uint64_t *)dst) = temp;
#  else
        *((uint64_t *)dst) = temp0;
        *((uint64_t *)dst + 1) = temp1;
#  endif
#  if 0
        dst += 8;
#  else
        dst += dst_stride;
#  endif
        --N;
    }
#endif/* @elsize == 1 && 0 -- else */
}


/*
 * specialized copy and swap for source stride 0,
 * interestingly unrolling here is like above is only marginally profitable for
 * small types and detrimental for >= 8byte moves on x86
 * but it profits from vectorization enabled with -O3
 */
#if (0 == 0) && 1
static void
_aligned_strided_to_strided_size16_srcstride0(char *dst,
                        int dst_stride,
                        char *src, int CARRAY_UNUSED(src_stride),
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 16 != 16
#  if !(16 == 1 && 0)
    uint64_t temp;
#  endif
#else
    uint64_t temp0, temp1;
#endif
    if (N == 0) {
        return;
    }
#if 1 && 16 != 16
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint64_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint64_t))));
#endif
#if 16 == 1 && 0
    memset(dst, *src, N);
#else

#  if 16 != 16
    temp = _CARRAY_NOP16(*((uint64_t *)src));
#  else
#    if 0 == 0
        temp0 = (*((uint64_t *)src));
        temp1 = (*((uint64_t *)src + 1));
#    elif 0 == 1
        temp0 = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        temp1 = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        temp0 = _CARRAY_SWAP8(*((uint64_t *)src));
        temp1 = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

    while (N > 0) {
#  if 16 != 16
        *((uint64_t *)dst) = temp;
#  else
        *((uint64_t *)dst) = temp0;
        *((uint64_t *)dst + 1) = temp1;
#  endif
#  if 0
        dst += 16;
#  else
        dst += dst_stride;
#  endif
        --N;
    }
#endif/* @elsize == 1 && 0 -- else */
}





/*
 * unrolling gains about 20-50% if the copy can be done in one mov instruction
 * if not it can decrease performance
 * tested to improve performance on intel xeon 5x/7x, core2duo, amd phenom x4
 */
static void
_strided_to_contig_size2(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 0
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint16_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint16_t))));
#endif
    /*printf("fn _strided_to_contig_size2\n");*/
    while (N > 0) {
#if 0

        /* aligned copy and swap */
#  if 2 != 16
        (*((uint16_t *)dst)) = _CARRAY_NOP2(*((uint16_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 2);
#  if 0 == 1
        _CARRAY_NOP2(dst);
#  elif 0 == 2
        _CARRAY_NOP1(dst);
        _CARRAY_NOP1(dst + 1);
#  endif

#endif

#if 1
        dst += 2;
#else
        dst += dst_stride;
#endif

#if 0
        src += 2;
#else
        src += src_stride;
#endif

        --N;
    }
}



static void
_strided_to_contig_size8(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 0
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint64_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint64_t))));
#endif
    /*printf("fn _strided_to_contig_size8\n");*/
    while (N > 0) {
#if 0

        /* aligned copy and swap */
#  if 8 != 16
        (*((uint64_t *)dst)) = _CARRAY_NOP8(*((uint64_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 8);
#  if 0 == 1
        _CARRAY_NOP8(dst);
#  elif 0 == 2
        _CARRAY_NOP4(dst);
        _CARRAY_NOP4(dst + 4);
#  endif

#endif

#if 1
        dst += 8;
#else
        dst += dst_stride;
#endif

#if 0
        src += 8;
#else
        src += src_stride;
#endif

        --N;
    }
}




static void
_strided_to_contig_size16(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 0
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint64_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint64_t))));
#endif
    /*printf("fn _strided_to_contig_size16\n");*/
    while (N > 0) {
#if 0

        /* aligned copy and swap */
#  if 16 != 16
        (*((uint64_t *)dst)) = _CARRAY_NOP16(*((uint64_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 16);
#  if 0 == 1
        _CARRAY_NOP16(dst);
#  elif 0 == 2
        _CARRAY_NOP8(dst);
        _CARRAY_NOP8(dst + 8);
#  endif

#endif

#if 1
        dst += 16;
#else
        dst += dst_stride;
#endif

#if 0
        src += 16;
#else
        src += src_stride;
#endif

        --N;
    }
}



/*
 * unrolling gains about 20-50% if the copy can be done in one mov instruction
 * if not it can decrease performance
 * tested to improve performance on intel xeon 5x/7x, core2duo, amd phenom x4
 */
static void
_contig_to_strided_size2(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 0
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint16_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint16_t))));
#endif
    /*printf("fn _contig_to_strided_size2\n");*/
    while (N > 0) {
#if 0

        /* aligned copy and swap */
#  if 2 != 16
        (*((uint16_t *)dst)) = _CARRAY_NOP2(*((uint16_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 2);
#  if 0 == 1
        _CARRAY_NOP2(dst);
#  elif 0 == 2
        _CARRAY_NOP1(dst);
        _CARRAY_NOP1(dst + 1);
#  endif

#endif

#if 0
        dst += 2;
#else
        dst += dst_stride;
#endif

#if 1
        src += 2;
#else
        src += src_stride;
#endif

        --N;
    }
}



/*
 * unrolling gains about 20-50% if the copy can be done in one mov instruction
 * if not it can decrease performance
 * tested to improve performance on intel xeon 5x/7x, core2duo, amd phenom x4
 */
static void
_contig_to_strided_size4(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 0
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint32_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint32_t))));
#endif
    /*printf("fn _contig_to_strided_size4\n");*/
    while (N > 0) {
#if 0

        /* aligned copy and swap */
#  if 4 != 16
        (*((uint32_t *)dst)) = _CARRAY_NOP4(*((uint32_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 4);
#  if 0 == 1
        _CARRAY_NOP4(dst);
#  elif 0 == 2
        _CARRAY_NOP2(dst);
        _CARRAY_NOP2(dst + 2);
#  endif

#endif

#if 0
        dst += 4;
#else
        dst += dst_stride;
#endif

#if 1
        src += 4;
#else
        src += src_stride;
#endif

        --N;
    }
}



/*
 * unrolling gains about 20-50% if the copy can be done in one mov instruction
 * if not it can decrease performance
 * tested to improve performance on intel xeon 5x/7x, core2duo, amd phenom x4
 */
static void
_contig_to_strided_size8(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 0
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint64_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint64_t))));
#endif
    /*printf("fn _contig_to_strided_size8\n");*/
    while (N > 0) {
#if 0

        /* aligned copy and swap */
#  if 8 != 16
        (*((uint64_t *)dst)) = _CARRAY_NOP8(*((uint64_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 8);
#  if 0 == 1
        _CARRAY_NOP8(dst);
#  elif 0 == 2
        _CARRAY_NOP4(dst);
        _CARRAY_NOP4(dst + 4);
#  endif

#endif

#if 0
        dst += 8;
#else
        dst += dst_stride;
#endif

#if 1
        src += 8;
#else
        src += src_stride;
#endif

        --N;
    }
}



/*
 * unrolling gains about 20-50% if the copy can be done in one mov instruction
 * if not it can decrease performance
 * tested to improve performance on intel xeon 5x/7x, core2duo, amd phenom x4
 */
static void
_contig_to_strided_size16(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 0
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint64_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint64_t))));
#endif
    /*printf("fn _contig_to_strided_size16\n");*/
    while (N > 0) {
#if 0

        /* aligned copy and swap */
#  if 16 != 16
        (*((uint64_t *)dst)) = _CARRAY_NOP16(*((uint64_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 16);
#  if 0 == 1
        _CARRAY_NOP16(dst);
#  elif 0 == 2
        _CARRAY_NOP8(dst);
        _CARRAY_NOP8(dst + 8);
#  endif

#endif

#if 0
        dst += 16;
#else
        dst += dst_stride;
#endif

#if 1
        src += 16;
#else
        src += src_stride;
#endif

        --N;
    }
}



/*
 * unrolling gains about 20-50% if the copy can be done in one mov instruction
 * if not it can decrease performance
 * tested to improve performance on intel xeon 5x/7x, core2duo, amd phenom x4
 */
static void
_strided_to_strided_size2(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 0
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint16_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint16_t))));
#endif
    /*printf("fn _strided_to_strided_size2\n");*/
    while (N > 0) {
#if 0

        /* aligned copy and swap */
#  if 2 != 16
        (*((uint16_t *)dst)) = _CARRAY_NOP2(*((uint16_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 2);
#  if 0 == 1
        _CARRAY_NOP2(dst);
#  elif 0 == 2
        _CARRAY_NOP1(dst);
        _CARRAY_NOP1(dst + 1);
#  endif

#endif

#if 0
        dst += 2;
#else
        dst += dst_stride;
#endif

#if 0
        src += 2;
#else
        src += src_stride;
#endif

        --N;
    }
}



/*
 * unrolling gains about 20-50% if the copy can be done in one mov instruction
 * if not it can decrease performance
 * tested to improve performance on intel xeon 5x/7x, core2duo, amd phenom x4
 */
static void
_strided_to_strided_size4(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 0
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint32_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint32_t))));
#endif
    /*printf("fn _strided_to_strided_size4\n");*/
    while (N > 0) {
#if 0

        /* aligned copy and swap */
#  if 4 != 16
        (*((uint32_t *)dst)) = _CARRAY_NOP4(*((uint32_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 4);
#  if 0 == 1
        _CARRAY_NOP4(dst);
#  elif 0 == 2
        _CARRAY_NOP2(dst);
        _CARRAY_NOP2(dst + 2);
#  endif

#endif

#if 0
        dst += 4;
#else
        dst += dst_stride;
#endif

#if 0
        src += 4;
#else
        src += src_stride;
#endif

        --N;
    }
}
#endif


static void
_strided_to_strided_size8(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 0
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint64_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint64_t))));
#endif
    /*printf("fn _strided_to_strided_size8\n");*/
    while (N > 0) {
#if 0

        /* aligned copy and swap */
#  if 8 != 16
        (*((uint64_t *)dst)) = _CARRAY_NOP8(*((uint64_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else
        /* unaligned copy and swap */
        memmove(dst, src, 8);
#  if 0 == 1
        _CARRAY_NOP8(dst);
#  elif 0 == 2
        _CARRAY_NOP4(dst);
        _CARRAY_NOP4(dst + 4);
#  endif

#endif

#if 0
        dst += 8;
#else
        dst += dst_stride;
#endif

#if 0
        src += 8;
#else
        src += src_stride;
#endif

        --N;
    }
}
#endif

/*
 * unrolling gains about 20-50% if the copy can be done in one mov instruction
 * if not it can decrease performance
 * tested to improve performance on intel xeon 5x/7x, core2duo, amd phenom x4
 */
static void
_strided_to_strided_size16(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 0
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint64_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint64_t))));
#endif
    /*printf("fn _strided_to_strided_size16\n");*/
    while (N > 0) {
#if 0

        /* aligned copy and swap */
#  if 16 != 16
        (*((uint64_t *)dst)) = _CARRAY_NOP16(*((uint64_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 16);
#  if 0 == 1
        _CARRAY_NOP16(dst);
#  elif 0 == 2
        _CARRAY_NOP8(dst);
        _CARRAY_NOP8(dst + 8);
#  endif

#endif

#if 0
        dst += 16;
#else
        dst += dst_stride;
#endif

#if 0
        src += 16;
#else
        src += src_stride;
#endif

        --N;
    }
}


/*
 * unrolling gains about 20-50% if the copy can be done in one mov instruction
 * if not it can decrease performance
 * tested to improve performance on intel xeon 5x/7x, core2duo, amd phenom x4
 */
static void
_aligned_contig_to_strided_size1(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 1
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint8_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint8_t))));
#endif
    /*printf("fn _aligned_contig_to_strided_size1\n");*/
    while (N > 0) {
#if 1

        /* aligned copy and swap */
#  if 1 != 16
        (*((uint8_t *)dst)) = _CARRAY_NOP1(*((uint8_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 1);
#  if 0 == 1
        _CARRAY_NOP1(dst);
#  elif 0 == 2
        _CARRAY_NOP0(dst);
        _CARRAY_NOP0(dst + 0);
#  endif

#endif

#if 0
        dst += 1;
#else
        dst += dst_stride;
#endif

#if 1
        src += 1;
#else
        src += src_stride;
#endif

        --N;
    }
}


/*
 * unrolling gains about 20-50% if the copy can be done in one mov instruction
 * if not it can decrease performance
 * tested to improve performance on intel xeon 5x/7x, core2duo, amd phenom x4
 */
static void
_aligned_contig_to_strided_size2(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 1
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint16_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint16_t))));
#endif
    /*printf("fn _aligned_contig_to_strided_size2\n");*/
    while (N > 0) {
#if 1

        /* aligned copy and swap */
#  if 2 != 16
        (*((uint16_t *)dst)) = _CARRAY_NOP2(*((uint16_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 2);
#  if 0 == 1
        _CARRAY_NOP2(dst);
#  elif 0 == 2
        _CARRAY_NOP1(dst);
        _CARRAY_NOP1(dst + 1);
#  endif

#endif

#if 0
        dst += 2;
#else
        dst += dst_stride;
#endif

#if 1
        src += 2;
#else
        src += src_stride;
#endif

        --N;
    }
}


static void
_aligned_contig_to_strided_size4(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 1
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint32_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint32_t))));
#endif
    /*printf("fn _aligned_contig_to_strided_size4\n");*/
    while (N > 0) {
#if 1

        /* aligned copy and swap */
#  if 4 != 16
        (*((uint32_t *)dst)) = _CARRAY_NOP4(*((uint32_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 4);
#  if 0 == 1
        _CARRAY_NOP4(dst);
#  elif 0 == 2
        _CARRAY_NOP2(dst);
        _CARRAY_NOP2(dst + 2);
#  endif

#endif

#if 0
        dst += 4;
#else
        dst += dst_stride;
#endif

#if 1
        src += 4;
#else
        src += src_stride;
#endif

        --N;
    }
}


static void
_aligned_strided_to_strided_size2(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 1
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint16_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint16_t))));
#endif
    /*printf("fn _aligned_strided_to_strided_size2\n");*/
    while (N > 0) {
#if 1

        /* aligned copy and swap */
#  if 2 != 16
        (*((uint16_t *)dst)) = _CARRAY_NOP2(*((uint16_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 2);
#  if 0 == 1
        _CARRAY_NOP2(dst);
#  elif 0 == 2
        _CARRAY_NOP1(dst);
        _CARRAY_NOP1(dst + 1);
#  endif

#endif

#if 0
        dst += 2;
#else
        dst += dst_stride;
#endif

#if 0
        src += 2;
#else
        src += src_stride;
#endif

        --N;
    }
}

static void
_aligned_contig_to_strided_size16(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 1
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint64_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint64_t))));
#endif
    /*printf("fn _aligned_contig_to_strided_size16\n");*/
    while (N > 0) {
#if 1

        /* aligned copy and swap */
#  if 16 != 16
        (*((uint64_t *)dst)) = _CARRAY_NOP16(*((uint64_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 16);
#  if 0 == 1
        _CARRAY_NOP16(dst);
#  elif 0 == 2
        _CARRAY_NOP8(dst);
        _CARRAY_NOP8(dst + 8);
#  endif

#endif

#if 0
        dst += 16;
#else
        dst += dst_stride;
#endif

#if 1
        src += 16;
#else
        src += src_stride;
#endif

        --N;
    }
}

static void
_aligned_contig_to_strided_size8(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 1
    /* sanity check */
    assert(N == 0 || carray_is_aligned(dst, carray_uint_alignment(sizeof(uint64_t))));
    assert(N == 0 || carray_is_aligned(src, carray_uint_alignment(sizeof(uint64_t))));
#endif
    /*printf("fn _aligned_contig_to_strided_size8\n");*/
    while (N > 0) {
#if 1

        /* aligned copy and swap */
#  if 8 != 16
        (*((uint64_t *)dst)) = _CARRAY_NOP8(*((uint64_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 8);
#  if 0 == 1
        _CARRAY_NOP8(dst);
#  elif 0 == 2
        _CARRAY_NOP4(dst);
        _CARRAY_NOP4(dst + 4);
#  endif

#endif

#if 0
        dst += 8;
#else
        dst += dst_stride;
#endif

#if 1
        src += 8;
#else
        src += src_stride;
#endif

        --N;
    }
}

static void
_strided_to_contig_size4(char *dst,
                        int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
#if 0
    /* sanity check */
    assert(N == 0 || npy_is_aligned(dst, carray_uint_alignment(sizeof(uint32_t))));
    assert(N == 0 || npy_is_aligned(src, carray_uint_alignment(sizeof(uint32_t))));
#endif
    /*printf("fn _strided_to_contig_size4\n");*/
    while (N > 0) {
#if 0

        /* aligned copy and swap */
#  if 4 != 16
        (*((uint32_t *)dst)) = _CARRAY_NOP4(*((uint32_t *)src));
#  else
#    if 0 == 0
        (*((uint64_t *)dst)) = (*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = (*((uint64_t *)src + 1));
#    elif 0 == 1
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src));
#    elif 0 == 2
        (*((uint64_t *)dst)) = _CARRAY_SWAP8(*((uint64_t *)src));
        (*((uint64_t *)dst + 1)) = _CARRAY_SWAP8(*((uint64_t *)src + 1));
#    endif
#  endif

#else

        /* unaligned copy and swap */
        memmove(dst, src, 4);
#  if 0 == 1
        _CARRAY_NOP4(dst);
#  elif 0 == 2
        _CARRAY_NOP2(dst);
        _CARRAY_NOP2(dst + 2);
#  endif

#endif

#if 1
        dst += 4;
#else
        dst += dst_stride;
#endif

#if 0
        src += 4;
#else
        src += src_stride;
#endif

        --N;
    }
}


/**
 * DOUBLE TO INT
 **/ 
#define _CONVERT_FN(x) ((int)x)

static void
_cast_double_to_int(
                        char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
    double src_value;
    int dst_value;
    
    while (N--) {
        memmove(&src_value, src, sizeof(src_value));
        dst_value = _CONVERT_FN(src_value);
        *(int *)dst = _CONVERT_FN(*(double *)src);
        memmove(dst, &dst_value, sizeof(dst_value));
        dst += dst_stride;
        src += src_stride;
    }
}

static void
_aligned_cast_double_to_int(
                        char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
    double src_value;
    int dst_value;

    assert(N == 0 || carray_is_aligned(src, _ALIGN(double)));
    assert(N == 0 || carray_is_aligned(dst, _ALIGN(int)));

    while (N--) {
        memmove(&src_value, src, sizeof(src_value));
        dst_value = _CONVERT_FN(src_value);
        *(int *)dst = _CONVERT_FN(*(double *)src);
        memmove(dst, &dst_value, sizeof(dst_value));
        dst += dst_stride;
        src += src_stride;
    }
}
#undef _CONVERT_FN


/**
 * INT TO DOUBLE
 **/ 
#define _CONVERT_FN(x) ((double)x)
static void
_cast_int_to_double(
                        char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
    int src_value;
    double dst_value;
    
    while (N--) {
        memmove(&src_value, src, sizeof(src_value));
        dst_value = _CONVERT_FN(src_value);
        *(double *)dst = _CONVERT_FN(*(int *)src);
        memmove(dst, &dst_value, sizeof(dst_value));
        dst += dst_stride;
        src += src_stride;
    }
}
static void
_aligned_cast_int_to_double(
                        char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int CARRAY_UNUSED(src_itemsize),
                        CArrayAuxData *CARRAY_UNUSED(data))
{
    int src_value;
    double dst_value;

    assert(N == 0 || carray_is_aligned(src, _ALIGN(int)));
    assert(N == 0 || carray_is_aligned(dst, _ALIGN(double)));

    while (N--) {
        memmove(&src_value, src, sizeof(src_value));
        dst_value = _CONVERT_FN(src_value);
        *(double *)dst = _CONVERT_FN(*(int *)src);
        memmove(dst, &dst_value, sizeof(dst_value));
        dst += dst_stride;
        src += src_stride;
    }
}
#undef _CONVERT_FN






















static void
_strided_to_strided(char *dst, int dst_stride,
                        char *src, int src_stride,
                        int N, int src_itemsize,
                        CArrayAuxData *CARRAY_UNUSED(data))
{
    while (N > 0) {
        memmove(dst, src, src_itemsize);
        dst += dst_stride;
        src += src_stride;
        --N;
    }
}

static void
_contig_to_contig(char *dst, int CARRAY_UNUSED(dst_stride),
                        char *src, int CARRAY_UNUSED(src_stride),
                        int N, int src_itemsize,
                        CArrayAuxData *CARRAY_UNUSED(data))
{
        
    memmove(dst, src, src_itemsize*N);
}


CArray_StridedUnaryOp * 
CArray_GetStridedNumericCastFn(int aligned, int src_stride,
                             int dst_stride,
                             int src_type_num, int dst_type_num)
{
        switch (src_type_num) {
                case TYPE_INTEGER_INT:
                        switch (dst_type_num) {
                                case TYPE_DOUBLE_INT:
                                #  if CARRAY_USE_UNALIGNED_ACCESS
                                       if (src_stride == sizeof(int) && dst_stride == sizeof(double)) {
                                           //return &_aligned_contig_cast_int_to_double;
                                       }
                                       else {
                                           //return &_aligned_cast_int_to_double;
                                       }    
                                #  else
                                        if (src_stride == sizeof(double) &&
                                                        dst_stride == sizeof(int)) {
                                                //return aligned ?
                                                //        &_aligned_contig_cast_double_to_int :
                                                //        &_contig_cast_double_to_int;
                                        }
                                        else { 
                                                return aligned ? &_aligned_cast_int_to_double :
                                                                &_cast_int_to_double;
                                        }   
                                #endif            
                        }      
                case TYPE_DOUBLE_INT:
                        switch (dst_type_num) {
                                case TYPE_INTEGER_INT:
                                #  if CARRAY_USE_UNALIGNED_ACCESS
                                       if (src_stride == sizeof(int) && dst_stride == sizeof(double)) {
                                           //return &_aligned_contig_cast_int_to_double;
                                       }
                                       else {
                                           //return &_aligned_cast_int_to_double;
                                       }    
                                #  else
                                        if (src_stride == sizeof(double) &&
                                                        dst_stride == sizeof(int)) {
                                                //return aligned ?
                                                //        &_aligned_contig_cast_double_to_int :
                                                //        &_contig_cast_double_to_int;
                                        }
                                        else { 
                                                return aligned ? &_aligned_cast_double_to_int :
                                                                &_cast_double_to_int;
                                        }   
                                #endif            
                        }      
        }
}

CArray_StridedUnaryOp * 
CArray_GetStridedCopyFn(int aligned, int src_stride, int dst_stride, int itemsize)
{
/*
 * Skip the "unaligned" versions on CPUs which support unaligned
 * memory accesses.
 */
    #if !CARRAY_USE_UNALIGNED_ACCESS
        if (aligned) {
    #endif
    /*!CARRAY_USE_UNALIGNED_ACCESS*/
    /* contiguous dst */
        /* contiguous dst */
        if (itemsize != 0 && dst_stride == itemsize) {
            /* constant src */
            if (src_stride == 0) {
                switch (itemsize) {
                    case 1:
                    
                        return
                          &_aligned_strided_to_contig_size1_srcstride0;
                    case 2:
                        return
                          &_aligned_strided_to_contig_size2_srcstride0;
                    case 4:
                        return
                          &_aligned_strided_to_contig_size4_srcstride0;
                    case 8:
                        return
                          &_aligned_strided_to_contig_size8_srcstride0;
                    case 16:
                        return
                          &_aligned_strided_to_contig_size16_srcstride0;   
                }
            }
            /* contiguous src */
            else if (src_stride == itemsize) {
                return &_contig_to_contig;
            }
            /* general src */
            else {
                switch (itemsize) {
                    case 1:
                        return &_aligned_strided_to_contig_size1;
                    case 2:
                        return &_aligned_strided_to_contig_size2;
                    case 4:
                        return &_aligned_strided_to_contig_size4;
                    case 8:
                        return &_aligned_strided_to_contig_size8;
                    case 16:
                        return &_aligned_strided_to_contig_size16;
                }
            }

            return &_strided_to_strided;
        }
        /* general dst */
        else {
            /* constant src */
            if (src_stride == 0) {
                switch (itemsize) {
                    case 1:
                        return &_aligned_strided_to_strided_size1_srcstride0;
                    case 2:
                        return &_aligned_strided_to_strided_size2_srcstride0;
                    case 4:
                        return &_aligned_strided_to_strided_size4_srcstride0;
                    case 8:
                        return &_aligned_strided_to_strided_size8_srcstride0;
                    case 16:
                        return &_aligned_strided_to_strided_size16_srcstride0;                
                }
            }
            /* contiguous src */
            else if (src_stride == itemsize) {
                switch (itemsize) {
                    case 1:
                        return &_aligned_contig_to_strided_size1;
                    case 2:
                        return &_aligned_contig_to_strided_size2;
                    case 4:
                        return &_aligned_contig_to_strided_size4;
                    case 8:
                        return &_aligned_contig_to_strided_size8;
                    case 16:
                        return &_aligned_contig_to_strided_size16;
                }
                return &_strided_to_strided;
            }
            else {
                switch (itemsize) {
                    case 1:
                        return &_aligned_strided_to_strided_size1;
                    case 2:
                        return &_aligned_strided_to_strided_size2;
                    case 4:
                        return &_aligned_strided_to_strided_size4;
                    case 8:
                        return &_aligned_strided_to_strided_size8;
                    case 16:
                        return &_aligned_strided_to_strided_size16;
                    /**end repeat**/
                }
            }
        }
    #if !CARRAY_USE_UNALIGNED_ACCESS
    }
    else {
        /* contiguous dst */
        if (itemsize != 0 && dst_stride == itemsize) {
            /* contiguous src */
            if (itemsize != 0 && src_stride == itemsize) {
                return &_contig_to_contig;
            }
            /* general src */
            else {
                switch (itemsize) {
                    case 1:
                        return &_aligned_strided_to_contig_size1;
                    case 2:
                        return &_strided_to_contig_size2;
                    case 4:
                        return &_strided_to_contig_size4;
                    case 8:
                        return &_strided_to_contig_size8;
                    case 16:
                        return &_strided_to_contig_size16;
                }
            }
            return &_strided_to_strided;
        }
        /* general dst */
        else {
            /* contiguous src */
            if (itemsize != 0 && src_stride == itemsize) {
                switch (itemsize) {
                    case 1:
                        return &_aligned_contig_to_strided_size1;
                    case 2:
                        return &_contig_to_strided_size2;
                    case 4:
                        return &_contig_to_strided_size4;
                    case 8:
                        return &_contig_to_strided_size8;
                    case 16:
                        return &_contig_to_strided_size16;
                }

                return &_strided_to_strided;
            }
            /* general src */
            else {
                switch (itemsize) {
                    case 1:
                        return &_aligned_strided_to_strided_size1;
                    case 2:
                        return &_strided_to_strided_size2;
                    case 4:
                        return &_strided_to_strided_size4;
                    case 8:
                        return &_strided_to_strided_size8;
                    case 16:
                        return &_strided_to_strided_size16;
                }
            }
        }
    }
    #endif/*!CARRAY_USE_UNALIGNED_ACCESS*/
    return &_strided_to_strided;
}