# Sorting Routines

---

## sort
```php
public static sort($x, int $axis = -1, $kind = 'quicksort') : CArray
```
> Return a sorted copy of an array.

##### Parameters

`CArray|Array` **$x** Input array.

`int` **$axis** (Optional) Axis along which to sort. The default is -1, 
which sorts along the last axis.

`string` **$kind** (Optional) Sorting Algorithm. Default is `quicksort`. Options:
*quicksort*, *mergesort*, *heapsort* and *stable*.

##### Returns

`CArray` Array of the same type and shape as `$x`.

##### Examples

**Example 1**
```php
echo CArray::sort([[1 ,4 ],[3 ,1]]);
```
```
[[ 1  4 ]
 [ 1  3 ]]
```

**Example 2**
```php
echo CArray::sort([[1 ,4 ],[3 ,1]], 0);
```
```
[[ 1  1 ]
 [ 3  4 ]]
```
