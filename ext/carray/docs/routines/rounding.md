# Rounding Routines

---

## ceil
```php
public static ceil($x) : CArray
```
> The ceiling of the input, element-wise.

##### Parameters

`CArray|Array` **$x** Input array.

##### Returns

`CArray` The ceiling of each element in `$x`.

##### Examples

**Example 1**
```php
$a = new CArray([-1.7, -1.5, -0.2, 0.2, 1.5, 1.7, 2.0]);
echo CArray::ceil($a);
```
```
[ -1.  -1.   0.   1.   2.   2.   2. ]
```

---

## floor
```php
public static floor($x) : CArray
```
> The floor of the input, element-wise.

##### Parameters

`CArray|Array` **$x** Input array.

##### Returns

`CArray` The floor of each element in `$x`.

##### Examples

**Example 1**
```php
$a = new CArray([-1.7, -1.5, -0.2, 0.2, 1.5, 1.7, 2.0]);
echo CArray::floor($a);
```
```
[ -2.  -2.  -1.   0.   1.   1.   2. ]
```


---

## around
```php
public static around($x, int $decimals = 0) : CArray
```
> Evenly round to the given number of decimals.

##### Parameters

`CArray|Array` **$x** Input array.

`int` **$decimals** Number of decimal places to round to (default: 0).
If decimals is negative, it specifies the number of positions to the
 left of the decimal point.

##### Returns

`CArray` An array of the same type as `$a`, containing the rounded values.

##### Examples

**Example 1**
```php
$a = CArray::around([0.37, 1.64]);
echo $a;
```
```
[ 0.  2. ]
``` 

**Example 2**
```php
$a = CArray::around([0.37, 1.64], 2);
echo $a;
```
```
[ 0.37000000  1.64000000 ]
``` 