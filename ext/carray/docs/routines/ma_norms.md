# Matrix Norm Routines

---

## norm
```php
public static norm($x) : CArray
```
> Matrix or vector Frobenius norm.

##### Parameters

`CArray|Array` **$x** Input array.

##### Returns

`CArray` Norm of the matrix or vector.

##### Examples

**Example 1**
```php
$a = CArray::arange(9) - 4;
$b = CArray::reshape($a, [3, 3]);
echo CArray::norm($b);
```
```
7.745967
```

---

## det
```php
public static det($a) : CArray
```
> Compute the determinant of an array.

##### Parameters

`CArray|Array` **$a** Input array.

##### Returns

`CArray` Determinant of `$a`.

##### Examples

**Example 1**
```php
$a = new CArray([[1, 2], [3, 4]]);
echo CArray::det($a);
```
```
-2.000000
```