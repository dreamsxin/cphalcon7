# Dimensions Routines

---

## atleast_1d
```php
public static atleast_1d($a ...) : CArray | Array
```
> Convert inputs to arrays with at least one dimension.
> Scalar inputs are converted to 1-dimensional arrays, whilst higher-dimensional inputs are preserved.

##### Parameters

`CArray|Array` **$a ...** One or more input arrays.

##### Returns

`CArray|Array` An CArray, or array of CArrays, each with NDIM >= 1. Copies are made only if necessary.

---

## atleast_2d

```php
public static atleast_2d($a ...) : CArray | Array
```
> Convert inputs to arrays with at least two dimensions.

##### Parameters

`CArray|Array` **$a ...** One or more input arrays.

##### Returns

`CArray|Array` An CArray, or array of CArrays, each with NDIM >= 2. Copies are made only if necessary.

---

## atleast_3d

```php
public static atleast_3d($a ...) : CArray | Array
```
> Convert inputs to arrays with at least three dimensions.

##### Parameters

`CArray|Array` **$a ...** One or more input arrays.

##### Returns

`CArray|Array` An CArray, or array of CArrays, each with NDIM >= 3. Copies are made only if necessary.

---

## squeeze

```php
public static squeeze($a, $axis = NULL) : CArray
```
> Remove single-dimensional entries from the shape of an array.
  
##### Parameters

`CArray|Array` **$a** Input data.

`int` **$axis** Selects a subset of the single-dimensional entries in the shape.

##### Returns

`CArray` The input array, but with all or a subset of the dimensions of length 1 removed. This is always a itself or a view into `$a`.

---

## expand_dims

```php
public static expand_dims($a, $axis = NULL) : CArray
```
> Expand the shape of an array.
  
> Insert a new axis that will appear at the `$axis` position in the expanded array shape.
  
##### Parameters

`CArray|Array` **$a** Input data.

`int` **$axis** Position in the expanded axes where the new axis is placed.

##### Returns

`CArray` **View** of `$a` with the number of dimensions increased by one.
