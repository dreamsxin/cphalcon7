# Rearranging Routines

---

## reshape

```php
public static reshape($a, $newshape) : CArray
```
> Gives a new shape to an array without changing its data.

##### Parameters

`CArray|Array` **$a** Input array.

`CArray|Array` **$newshape** The new shape should be compatible with the original shape. If an integer, then the result 
will be a 1-D array of that length.

##### Returns

`CArray` This will be a new view of `$a` if possible; otherwise, it will be a copy.

##### Examples

**Example 1**
```php
$A = CArray::arange(8);
echo CArray::reshape($A, [2, 4]);
```
````
[[ 0  1  2  3 ]
 [ 4  5  6  7 ]]
````

---

## flip

```php
public static flip($a) : CArray
```
> Reverse the order of elements in an array

##### Parameters

`CArray|Array` **$a** Input array.

##### Returns

`CArray` A view of `$a` with the entries of axis reversed. 

##### Examples

**Example 1**
```php
$A = CArray::arange(8);
echo CArray::flip($A);
```
```
[ 7  6  5  4  3  2  1  0 ]
```