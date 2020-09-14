# Search Routines

---

## argmax
```php
public static argmax($x, $axis = NULL) : CArray
```
> Returns the indices of the maximum values along an axis.

##### Parameters

`CArray|Array` **$x** Input array.

`CArray|Array` **$axis** (Optional) The index is into the flattened array, otherwise along the specified axis.

##### Returns

`CArray|Array` **$x** Array of indices into the array. It has the same shape as `$x` with the dimension along `$axis` removed.

##### Examples

**Example 1**
```php
$a = CArray::arange(6);
$a = CArray::reshape($a, [2, 3]) + 10;
echo CArray::argmax($a);
```
```
5
```

**Example 2**
```php
$a = CArray::arange(6);
$a = CArray::reshape($a, [2, 3]) + 10;
echo CArray::argmax($a, 0);
```
```
[ 1  1  1 ]
```

---

## argmin
```php
public static argmin($x, $axis = NULL) : CArray
```
> Returns the indices of the minimum values along an axis.

##### Parameters

`CArray|Array` **$x** Input array.

`CArray|Array` **$axis** (Optional) The index is into the flattened array, otherwise along the specified axis.

##### Returns

`CArray|Array` **$x** Array of indices into the array. It has the same shape as `$x` with the dimension along `$axis` removed.

##### Examples

**Example 1**
```php
$a = CArray::arange(6);
$a = CArray::reshape($a, [2, 3]) + 10;
echo CArray::argmin($a);
```
```
0
```

**Example 2**
```php
$a = CArray::arange(6);
$a = CArray::reshape($a, [2, 3]) + 10;
echo CArray::argmin($a, 0);
```
```
[ 0  0  0 ]
```