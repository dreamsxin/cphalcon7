# Matrix & Vector Products

---

## vdot
```php
public static vdot($a, $b) : CArray
```
> Return the dot product of two vectors.

> vdot does not perform a matrix product, but flattens input arguments to 1-D 
vectors first. Consequently, it should only be used for vectors.

##### Parameters

`CArray|Array` **$a** Input array.

`CArray|Array` **$b** Input array.

##### Returns

`CArray` Dot product of `$a` and `$b`.

##### Examples

**Example 1**
```php
$a = new CArray([[1, 4], [5, 6]]);
$b = new CArray([[4, 1], [2, 2]]);

echo CArray::vdot($a, $b);
```
```
30.0000
```

---

## matmul
```php
public static matmul($a, $b) : CArray
```
> Matrix product of two arrays.

##### Parameters

`CArray|Array` **$a** Input array.

`CArray|Array` **$b** Input array.

##### Returns

`CArray` The matrix product of the inputs. This is a scalar only when both `$x1`, `$x2` are 1-d vectors.

##### Examples

**Example 1**
```php
$a = new CArray([[1, 4], [5, 6]]);
$b = new CArray([[4, 1], [2, 2]]);

echo CArray::matmul($a, $b);
```
```
[[ 12   9 ]
 [ 32  17 ]]
```

---

## inner
```php
public static inner($a, $b) : CArray
```
> Inner product of two arrays.

##### Parameters

`CArray|Array` **$a** Input array.

`CArray|Array` **$b** Input array.

##### Returns

`CArray` Inner product of the input arrays.