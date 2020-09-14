# Arithmetic Routines

---

## add
```php
public static add($x1, $x2) : CArray
```
> Add arguments element-wise.

##### Parameters

`CArray|Array` **$x1** **$x2** Input arrays.

##### Returns

`CArray` The sum of `$x1` and `$x2`, element-wise.

##### Examples

**Example 1**
```php
$x1 = CArray::arange(9.0);
$x1 = CArray::reshape($x1, [3, 3]);
$x2 = CArray::arange(3.0);

echo CArray::add($x1, $x2);
```
```
[[  0   2   4 ]
 [  3   5   7 ]
 [  6   8  10 ]]
```

**Example 2**
```php
$a = new CArray([[1, 2], [3, 4]]);
$b = new CArray([[5, 6], [7, 8]]);

$c = $a + $b; // Same as CArray::add

echo $c;
```
```
[[  6   8 ]
 [ 10  12 ]]
```

---

## subtract
```php
public static subtract($x1, $x2) : CArray
```
> Subtract arguments element-wise.

##### Parameters

`CArray|Array` **$x1** **$x2** Input arrays.

##### Returns

`CArray` The difference of `$x1` and `$x2`, element-wise. 

##### Examples

**Example 1**
```php
$x1 = CArray::arange(9.0);
$x1 = CArray::reshape($x1, [3, 3]);
$x2 = CArray::arange(3.0);
echo CArray::subtract($x1, $x2);
```
```
[[ 0  0  0 ]
 [ 3  3  3 ]
 [ 6  6  6 ]]
```

**Example 2**
```php
$x1 = CArray::arange(9.0);
$x1 = CArray::reshape($x1, [3, 3]);
$x2 = CArray::arange(3.0);
echo ($x1 - $x2);
```
```
[[ 0  0  0 ]
 [ 3  3  3 ]
 [ 6  6  6 ]]
```

---

## multiply
```php
public static multiply($x1, $x2) : CArray
```
> Multiply arguments element-wise.

##### Parameters

`CArray|Array` **$x1** **$x2** Input arrays.

##### Returns

`CArray` The multiplication of `$x1` and `$x2`, element-wise.

##### Examples

**Example 1**
```php
$x1 = CArray::arange(9.0);
$x1 = CArray::reshape($x1, [3, 3]);
$x2 = CArray::arange(3.0);
echo CArray::multiply($x1, $x2);
```
```
[[  0   1   4 ]
 [  0   4  10 ]
 [  0   7  16 ]]
```

**Example 2**
```php
$x1 = CArray::arange(9.0);
$x1 = CArray::reshape($x1, [3, 3]);
$x2 = CArray::arange(3.0);
echo ($x1 * $x2);
```
```
[[  0   1   4 ]
 [  0   4  10 ]
 [  0   7  16 ]]
```

---

## divide
```php
public static divide($x1, $x2) : CArray
```
> Returns a true division of the inputs, element-wise.

##### Parameters

`CArray|Array` **$x1** Dividend

`CArray|Array` **$x2** Divisor

##### Returns

`CArray` Return array.

##### Examples

**Example 1**
```php
echo CArray::divide([1, 2, 3, 4, 5], 4);
```
```
[ 0.25000000  0.50000000  0.75000000  1.00000000  1.25000000 ]
```

**Example 2**
```php
$a = new CArray([[1, 2], [3, 4]]);
echo ($a / 4);
```
```
[[ 0.25000000  0.50000000 ]
 [ 0.75000000  1.00000000 ]]
```

---

## power
```php
public static power($x1, $x2) : CArray
```
> `$x1` array elements raised to powers from `$x2` array, element-wise.

##### Parameters

`CArray|Array` **$x1** Bases

`CArray|Array` **$x2** Exponents

##### Returns

`CArray` The bases in `$x1` raised to the exponents in `$x2`.

##### Examples

**Example 1**
```php
$x1 = CArray::arange(6);
echo CArray::power($x1, 3);
```
```
[   0    1    8   27   64  125 ]
```

---

## mod
```php
public static mod($x1, $x2) : CArray
```
> Return element-wise remainder of division.

##### Parameters

`CArray|Array` **$x1** Dividend

`CArray|Array` **$x2** Divisor

##### Returns

`CArray` The element-wise remainder 

##### Examples

**Example 1**
```php
echo CArray::mod([4, 7], [2, 3]);
```
```
[ 0.  1. ]
```

---

## remainder
```php
public static remainder($x1, $x2) : CArray
```
> Return element-wise remainder of division.

##### Parameters

`CArray|Array` **$x1** Dividend

`CArray|Array` **$x2** Divisor

##### Returns

`CArray` The element-wise remainder 

##### Examples

**Example 1**
```php
echo CArray::remainder([4, 7], [2, 3]);
```
```
[ 0.  1. ]
```

---

## negative
```php
public static negative($x) : CArray
```
> Numerical negative, element-wise.

##### Parameters

`CArray|Array` **$x** Input array.

##### Returns

`CArray` Return array.

##### Examples

**Example 1**
```php
echo CArray::negative([1, -1]);
```
```
[ -1.   1. ]
```


---

## sqrt
```php
public static sqrt($x) : CArray
```

---

## reciprocal
```php
public static reciprocal($x) : CArray
```
> Return `1/$x` of the argument, element-wise.

##### Parameters

`CArray|Array` **$x** Input array.

##### Returns

`CArray` Return array.

##### Examples

**Example 1**
```php
echo CArray::reciprocal([1, 2., 3.33]);
```
```
[ 1.00000000  0.50000000  0.33333333 ]
```




