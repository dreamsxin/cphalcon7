# Sums, products and differences Routines

---

## prod
```php
public static prod($x, int $axis = NULL) : CArray
```
> Return the product of array elements over a given axis.

##### Parameters

`CArray|Array` **$x** Input array.

`int` **$axis** (optional) Axis or axes along which a product is performed. 

##### Returns

`CArray` An array shaped as `$x` but with the specified axis removed.
. If `$axis` is NULL an 0-d CArray is returned. 

##### Examples

**Example 1**
```php
echo CArray::prod([[1.,2.],[3.,4.]]);
```
```
24.000000
```

**Example 2**
```php
echo CArray::prod([[1.,2.],[3.,4.]], 0);
```
```
[ 3.  8. ]
```




---

## sum
```php
public static sum($x, int $axis = NULL) : CArray
```
> Sum of array elements over a given axis.

##### Parameters

`CArray|Array` **$x** Input array.

`int` **$axis** (optional) Axis or axes along which a sum is performed. 

##### Returns

`CArray` An array shaped as `$x` but with the specified axis removed.
. If `$axis` is NULL an 0-d CArray is returned. 

##### Examples

**Example 1**
```php
echo CArray::sum([[1.,2.],[3.,4.]]);
```
```
10.000000
```

**Example 2**
```php
echo CArray::sum([[1.,2.],[3.,4.]]);
```
```
[ 4.  6. ]
```


---

## cumprod
```php
public static cumprod($x, int $axis = 0) : CArray
```
> Return the cumulative product of elements along a given axis.

##### Parameters

`CArray|Array` **$x** Input array.

`int` **$axis** (optional) Axis along which the cumulative product is computed.

##### Returns

`CArray` An array shaped as `$x`. If `$axis` is NULL an 0-d CArray is returned. 

##### Examples

**Example 1**
```php
$a = new CArray([1, 2, 3]);
echo CArray::cumprod($a);
```
```
[ 1  2  6 ]
```

**Example 2**
```php
$a = new CArray([[1, 2, 3], [4, 5, 6]]);
echo CArray::cumprod($a);
```
```
[   1    2    6   24  120  720 ]
```

---

## cumsum
```php
public static cumsum($x) : CArray
```
> Return the cumulative sum of elements along a given axis.

##### Parameters

`CArray|Array` **$x** Input array.

`int` **$axis** (optional) Axis along which the cumulative sum is computed.

##### Returns

`CArray` An array shaped as `$x`. If `$axis` is NULL an 0-d CArray is returned. 

##### Examples

**Example 1**
```php
$a = new CArray([[1, 2, 3], [4, 5, 6]]);
echo CArray::cumsum($a);
```
```
[  1   3   6  10  15  21 ]
```

**Example 2**
```php
echo CArray::cumsum([1, 2, 3]);
```
```
[ 1  3  6 ]
```