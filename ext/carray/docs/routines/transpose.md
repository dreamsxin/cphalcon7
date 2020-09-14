# Transpose Routines

---

## moveaxis

```php
public static moveaxis($a, $source, $destination) : CArray
```
> Move axes of an array to new positions.
> Other axes remain in their original order.

##### Parameters

`CArray|Array` **$a** The array whose axes should be reordered.

`CArray|Array|Scalar` **$source** Original positions of the axes to move. These must be unique.

`CArray|Array|Scalar` **$destination** Destination positions for each of the original axes. These must also be unique.


##### Returns

`CArray` Array with moved axes. This array is a view of the input array.

##### Examples

**Example 1**
```php
echo CArray::moveaxis([[1, 2], [3, 4]], 0, 1);
```
```php
[[ 1.0000000000e+0  3.0000000000e+0 ]
 [ 2.0000000000e+0  4.0000000000e+0 ]]
```

**Example 2**
```php
<?php
echo "Before\n";

$x = CArray::zeros([1, 2, 3]);
echo $x;

echo "\nAfter\n";

$z = CArray::moveaxis($x, 0, -1);
echo $z;
```
```php
Before
[[[ 0.  0.  0. ]
  [ 0.  0.  0. ]]]

After
[[[ 0. ]
  [ 0. ]
  [ 0. ]]
 [[ 0. ]
  [ 0. ]
  [ 0. ]]]
```

-------

## rollaxis
```php
public static rollaxis($a, int $axis, int $start = 0) : CArray
```
> Roll the specified axis backwards, until it lies in a given position.

##### Parameters

`CArray|Array` **$a** - Target array.

`int` **$axis** - The axis to roll backwards. The positions of the other axes do not change relative to one another.

`int` **$start** (optional) - The axis is rolled until it lies before this position. The default, 0, results in a “complete” roll.

##### Returns

`CArray` A view of `$a` with rolled axis.


----


## swapaxes
```php
public static swapaxes($a, $axis1, $axis2) : CArray
```
> Interchange two axes of an array.

##### Parameters

`CArray|Array` **$a** - Target array.

`int` **$axis1** - First axis.

`int` **$axis2** - Second axis.

##### Returns

`CArray` A view of `$a` with interchanged axis.

---

## transpose
```php
public static transpose($a, $axes = NULL) : CArray
```
> Permute the dimensions of an array.

##### Parameters

`CArray|Array` **$a** - Target array.

`CArray|Array|int` **$axes** (optional) - By default, reverse the dimensions, otherwise permute the axes according to 
the values given.

##### Returns

`CArray` `$a` with its axes permuted. A view is returned whenever possible.