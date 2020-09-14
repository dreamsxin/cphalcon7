# CArray Internals C Reference

## Memory Stack

The `MemoryStack` struct is a global buffer containing a set of `CArray` pointers. The global buffer
can be accessed calling `PHPSCI_MAIN_MEM_STACK`. 

```C
struct MemoryStack {
    CArray * buffer;
    int size;
    int capacity;
    size_t bsize;
} MemoryStack;
```

> #### `CArray *` MemoryStack.buffer
> Dynamic allocated buffer containing all instances of **CArray** during runtime.

> #### `int` MemoryStack.size
> The number of CArray instances stored within the buffer.

> #### `int` MemoryStack.capacity
> The maximum capacity of the buffer. When **size** is equal **capacity** the buffer will be reallocated.

> #### `size_t` MemoryStack.bsize
> The current amount of bytes allocated within **buffer**.

## Memory Pointer

The PHP interpreter performs operations using the MemoryPointer structure. This object contains the
necessary information to relate the current PHP `zval` object with an instance of `CArray`.

```C
typedef struct MemoryPointer {
    int uuid;
} MemoryPointer;
```

> #### `int` MemoryPointer.uuid
> Contains the `uuid` of the related CArray instance within the buffer. Ex: If `uuid`is 0, the CArray instance allocated within `MemoryStack.buffer[0]` is used.

## CArray Structure

The `CArray` object contains all information required for our array. It keep track of the data buffer, and
other array properties like strides, dimensions and so on. All instances of `CArray` within PHP points
to this structure.

```C
struct CArray {
    int * strides;   
    int * dimensions;
    int ndim;        
    char * data;      
    CArray * base;
    int flags;
    CArrayDescriptor * descriptor;
    int refcount;
};
```

> #### `int *` CArray.strides
> An vector of integers containing the amount of bytes that must be skipped to get to the next element of that dimension.

> #### `int *` CArray.dimensions
> An vector of integers containing the amount of elements within each dimension. This could be seen as the shape of this array.

> #### `int` CArray.ndim
> Integer describing the number of dimensions.

> #### `char *` CArray.data
> The pointer to the first element of this array

> #### `CArray *` CArray.base
> Points to the CArray containing the original data. This is useful when one CArray shares the same data
with other (CArray Views).

> #### `int` CArray.flags
> Some properties are stored as flags. For example, if one CArray contains data from other CArray, them it won't have `CARRAY_ARRAY_OWNDATA` flag.

> #### `CArrayDescriptor *` CArray.descriptor
> Describes the memory layout and data properties of the CArray. One descriptor may be shared with multiples CArrays.

> #### `int` CArray.refcount
> CArray reference count. The reference count prevents PHP Gargage Collector from free shared data buffer across different CArray objects.


### CArrayDescriptor
This structure describes the CArray memory layout and data type. CArrayDescriptor can be used for
CArray initialization.

```C
typedef struct CArrayDescriptor {
    char type;          // b = boolean, d = double, i = signer integer, u = unsigned integer, f = floating point, c = char
    int flags;          // Data related flags
    int type_num;       // 0 = boolean, 1 = double, 2 = signed integer, 3 = unsigned integer, 4 = floating point, 5 = char
    int elsize;         // Datatype size
    int numElements;    // Number of elements
    int alignment;      // Alignment Information
    int refcount;
} CArrayDescriptor;
```

## Creating CArrays


#### `CArray *` CArray_NewFromDescr_int
```C
CArray * CArray_NewFromDescr_int(CArray * self, CArrayDescriptor *descr, int nd, int *dims, int *strides, void *data, int flags, CArray *base, int zeroed, int allow_emptystring);
```
#### `CArray *` CArray_NewLikeArray
```C
CArray * CArray_NewLikeArray(CArray *prototype, CARRAY_ORDER order, CArrayDescriptor *dtype, int subok);
```

## CArray Iterators