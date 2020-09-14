# Hyperbolic Routines

---

## sinh
```php
public static sinh($x) : CArray
```
> Hyperbolic sine, element-wise.

##### Parameters

`CArray|Array` **$x** Input array.

##### Returns

`CArray` The hyperbolic sine of each element of `$x`.

##### Examples

**Example 1**
```php
echo CArray::sinh(0);
```
```
0.000000
```

---

## cosh
```php
public static cosh($x) : CArray
```
> Compute hyperbolic cosine element-wise.

##### Parameters

`CArray|Array` **$x** Input array.

##### Returns

`CArray` The hyperbolic cosine of each element of `$x`.

##### Examples

**Example 1**
```php
echo CArray::cosh([1, -1, 0]);
```
```
[ 1.54308063  1.54308063  1.00000000 ]
```

---

## tanh
```php
public static tanh($x) : CArray
```
> Compute hyperbolic tangent element-wise.

##### Parameters

`CArray|Array` **$x** Input array.

##### Returns

`CArray` The hyperbolic tangent of each element of `$x`.

##### Examples

**Example 1**
```php
echo CArray::tanh([-1, 0, 1]);
```
```
[ -0.76159416   0.00000000   0.76159416 ]
```