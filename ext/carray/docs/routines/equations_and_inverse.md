# Equations & Inverse Routines

---

## inv
```php
public static inv($a) : CArray
```
> Compute the (multiplicative) inverse of a matrix.

##### Parameters

`CArray|Array` **$a** Input array.

##### Returns

`CArray` (Multiplicative) inverse of the matrix `$a`.

##### Examples

**Example 1**
```php
$a = new CArray([[1., 2.], [3., 4.]]);
echo CArray::inv($a);
```
```
[[ -2.00000000   1.00000000 ]
 [  1.50000000  -0.50000000 ]]
```
