# Exponents and logarithms Routines

---

## exp
```php
public static exp($x) : CArray
```
> Calculate the exponential of all elements.

##### Parameters

`CArray|Array` **$x** Input array.

##### Returns

`CArray` Output array, element-wise exponential of `$x`.

---

## expm1
```php
public static expm1($x) : CArray
```
> Calculate `exp($x) - 1` for all elements in the array.

##### Parameters

`CArray|Array` **$x** Input array.

##### Returns

`CArray` Output array, element-wise `exp($x)-1` of `$x`.

---

## exp2
```php
public static exp2($x) : CArray
```
> Calculate `2**p` for all `p` (value) in the input array.

##### Parameters

`CArray|Array` **$x** Input array.

##### Returns

`CArray` Output array, element-wise `2**p` of `$x`.

---

## log
```php
public static log($x) : CArray
```
> Natural logarithm, element-wise.

##### Parameters

`CArray|Array` **$x** Input array.

##### Returns

`CArray` Output array, element-wise natural logarithm of `$x`.

---

## log10
```php
public static log10($x) : CArray
```
> Return the base 10 logarithm of `$x`, element-wise.

##### Parameters

`CArray|Array` **$x** Input array.

##### Returns

`CArray` Output array, element-wise base 10 logarithm of `$x`.

---

## log2
```php
public static log2($x) : CArray
```
> Base-2 logarithm of `$x`.

##### Parameters

`CArray|Array` **$x** Input array.

##### Returns

`CArray` Output array, element-wise base 2 logarithm of `$x`.

---

## log1p
```php
public static log1p($x) : CArray
```
> Calculates `log(1 + $x)` of `$x`

##### Parameters

`CArray|Array` **$x** Input array.

##### Returns

`CArray` Output array, element-wise `log(1 + $x)` of `$x`.
