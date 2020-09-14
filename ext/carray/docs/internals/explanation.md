# CArray Internals Explained

## Memory Layout

CArrays are contiguous C arrays direct accessed and stored by the PHP interpreter.

```PHP
$array_1d = new CArray([2, 3, 4, 5, 6, 7]);
$array_2d = new CArray([[2, 3], [4, 5], [6, 7]]);
$array_3d = new CArray([[[2, 3], [4, 5], [6, 7]]]);
```

In the example above, we allocated 3 `CArrays` with the following shapes `[6]`, `[3, 2]` and `[1, 3, 2]`. As all CArrays are contiguous allocated, the C memory layout for all the CArrays above is the same:

```C
int * buffer = {2, 3, 4, 5, 6, 7}
```

The extension only knows where to look for your values because it has the dimensions and strides information for each CArray above. This is specially fast if we want to change our CArray shapes
without loop (iterate) over the values.

## Data Access

CArrays are internaly accessed using strides. The strides gives the amount of bytes to jump within
our buffer to find next value for the Nth-dimension.

```PHP
$array = new CArray([[[2, 3], [4, 5], [6, 7]], [[2, 3], [4, 5], [6, 7]]]);
```

The shape for this array is `[2, 3, 2]` and the type is `int`. Considering `int` as a 4 bytes type
we have the following generated strides vector: `[24, 8 ,4]`. So for each value of the first dimension we jump 24 bytes, the second 2D matrix within this 3D tensor starts at index 6 `(24 bytes / 4 bytes)`.

```C
int * buffer = {2, 3, 4, 5, 6, 7, -> 2, 3, 4, 5, 6, 7}
```

We jump two more indices `8 bytes / 4 bytes` so we get to the second row of the second internal matrice `[1, 1, 0]`

```C
int * buffer = {2, 3, 4, 5, 6, 7, 2, 3, -> 4, 5, 6, 7}
```

