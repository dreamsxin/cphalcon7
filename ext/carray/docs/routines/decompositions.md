# Decomposition Routines

---

## svd
```php
public static svd($a) : CArray
```
> Singular Value Decomposition.

##### Parameters

`CArray|Array` **$a** 2-D Input array.

##### Returns

`Array` Array of CArrays containing the unitary arrays ([0] and [2]) and singular values ([1]).

##### Examples

**Example 1**
```php
$a = new CArray([[1, 4], [5, 6]]);
$b = CArray::svd($a);


print_r($b);
echo "\nUNITARY ARRAYS\n";
echo $b[0];
echo "\nSINGULAR VALUES\n";
echo $b[1];
echo "\nUNITARY ARRAYS\n";
echo $b[2];
```
```
Array
(
    [0] => CArray Object
        (
            [uuid] => 1
            [ndim] => 2
        )

    [1] => CArray Object
        (
            [uuid] => 2
            [ndim] => 1
        )

    [2] => CArray Object
        (
            [uuid] => 3
            [ndim] => 2
        )

)

UNITARY ARRAYS
[[ -0.44475472  -0.89565241 ]
 [ -0.89565241   0.44475472 ]]

SINGULAR VALUES
[ 8.68334898  1.61228116 ]

UNITARY ARRAYS
[[ -0.56694909  -0.82375283 ]
 [  0.82375283  -0.56694909 ]]
```