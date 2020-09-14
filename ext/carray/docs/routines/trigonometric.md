# Trigonometric Routines

---

## sin
```php
public static sin($x) : CArray
```
> Trigonometric sine, element-wise.

##### Parameters

`CArray|Array` **$x** Input array.

##### Returns

`CArray` The sine of each element of `$x`.

##### Examples

**Example 1**
```php
$a = CArray::linspace(-pi(), pi(), 10);
echo CArray::sin($a);
```
```
[ -0.00000000  -1.00000000   0.00000000   1.00000000   0.00000000 ]
```

---

## tan
```php
public static tan($x) : CArray
```
> Compute tangent element-wise.
> Equivalent to CArray::sin(x)/CArray::cos(x).

##### Parameters

`CArray|Array` **$x** Input array.

##### Returns

`CArray` The tangent of each element of `$x`.

##### Examples

**Example 1**
```php
echo CArray::tan([-pi(), pi()/2, pi()]);
```
```
[ 1.224647e-16  1.633124e+16  -1.224647e-16 ]
```

---

## arcsin
```php
public static arcsin($x) : CArray
```
> Inverse sine, element-wise.

##### Parameters

`CArray|Array` **$x** Input array.

##### Returns

`CArray` The inverse sine of each element in `$x`.

##### Examples

**Example 1**
```php
echo CArray::arcsin(1);
```
```
1.570796
```

---

## arccos
```php
public static arccos($x) : CArray
```
> Trigonometric inverse cosine, element-wise.

##### Parameters

`CArray|Array` **$x** Input array.

##### Returns

`CArray` The inverse cosine of each element in `$x`.

##### Examples

**Example 1**
```php
echo CArray::arccos([1, -1]);
```
```
[ 0.00000000  3.14159265 ]
```

---

## arctan
```php
public static arctan($x) : CArray
```
> Trigonometric inverse tangent, element-wise.

##### Parameters

`CArray|Array` **$x** Input array.

##### Returns

`CArray` The inverse tangent of each element in `$x`.

##### Examples

**Example 1**
```php
echo CArray::arctan([0, 1]);
```
```
[ 0.00000000  0.78539816 ]
```