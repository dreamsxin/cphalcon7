#if defined( __i386__ ) || defined(i386) || defined(_M_IX86)
    /*
     * __i386__ is defined by gcc and Intel compiler on Linux,
     * _M_IX86 by VS compiler,
     * i386 by Sun compilers on opensolaris at least
     */
    #define CARRAY_CPU_X86
#elif defined(__x86_64__) || defined(__amd64__) || defined(__x86_64) || defined(_M_AMD64)
    /*
     * both __x86_64__ and __amd64__ are defined by gcc
     * __x86_64 defined by sun compiler on opensolaris at least
     * _M_AMD64 defined by MS compiler
     */
    #define CARRAY_CPU_AMD64
#endif

#if (defined(CARRAY_CPU_X86) || defined(CARRAY_CPU_AMD64))
    #define CARRAY_CPU_HAVE_UNALIGNED_ACCESS 1
#else
    #define CARRAY_CPU_HAVE_UNALIGNED_ACCESS 0
#endif