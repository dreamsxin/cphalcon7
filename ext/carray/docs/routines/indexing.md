# Indexing Routines

---

## diagonal

```php
public static diagonal($a, int $offset = 0, int $axis1 = 0, int $axis2 = 0) : CArray
```
> Return specified diagonals.

> If a is 2-D, returns the diagonal of a with the given offset, i.e., the collection of elements of the 
form `$a[i][i+offset]`. If a has more than two dimensions, then the axes specified by axis1 and axis2 are used to 
determine the 2-D sub-array whose diagonal is returned. The shape of the resulting array can be determined by 
removing axis1 and axis2 and appending an index to the right equal to the size of the resulting diagonals.

##### Parameters

`CArray|Array` **$a** - Input array.

`CArray|Array` **$offset** (optional) - Offset of the diagonal from the main diagonal. Can be positive or negative. Defaults to 
main diagonal (0).

`CArray|Array` **$axis1** (optional) - Axis to be used as the first axis of the 2-D sub-arrays from which the diagonals should be taken. 
Defaults to first axis (0).

`CArray|Array` **$axis2** (optional) - Axis to be used as the second axis of the 2-D sub-arrays from which the diagonals should be 
taken. Defaults to second axis (1).

##### Returns

`CArray` Diagonals of `$a`.

##### Examples

**Example 1**
```php
$A = CArray::arange(4);
$A = CArray::reshape($A, [2, 2]);
echo CArray::diagonal($A);
```
````
[ 0  3 ]
````

