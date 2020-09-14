# Ones and Zeros Routines

---

## identity

```php
public static identity($n) : CArray
```
> Return the identity array.

> The identity array is a square array with ones on the main diagonal.

##### Parameters

`CArray|Array` **$n** Number of rows (and columns) in `$n x $n` output.

##### Returns

`CArray` `$n x $n` CArray with its main diagonal set to one, and all other elements 0.
         
##### Examples

**Example 1**
```php
echo CArray::identity(4);
```
```
[[ 1.  0.  0.  0. ]
 [ 0.  1.  0.  0. ]
 [ 0.  0.  1.  0. ]
 [ 0.  0.  0.  1. ]]
```

---

## eye

```php
public static eye($n, $m = NULL, $k = 0) : CArray
```

> Return a 2-D array with ones on the diagonal and zeros elsewhere.

##### Parameters

`CArray|Array` **$n** Number of rows in the output.

`CArray|Array` **$m** Number of columns in the output. If NULL, defaults to `$n`.

`CArray|Array` **$k** Index of the diagonal: 0 (the default) refers to the main diagonal, a positive value refers to 
an upper diagonal, and a negative value to a lower diagonal.

##### Returns

`CArray` An array where all elements are equal to zero, except for the `$k`-th diagonal, whose values are equal to one.

##### Examples

**Example 1**
```php
echo CArray::eye(3, 3, 1);
```
```
[[ 0.  1.  0. ]
 [ 0.  0.  1. ]
 [ 0.  0.  0. ]]
```         
         
         
---

## ones
```php
public static ones(array $shape) : CArray
```
> Return a new CArray of given shape, filled with ones.

##### Parameters

`Array` **$shape** Shape of the new array, e.g., `[2, 3]` or `[2]`.

##### Returns

`CArray` CArray of ones with the given shape.


---

## zeros
```php
public static zeros(array $shape) : CArray
```

> Return a new CArray of given shape, filled with zeros.

##### Parameters

`Array` **$shape** Shape of the new array, e.g., `[2, 3]` or `[2]`.

##### Returns

`CArray` CArray of zeros with the given shape.