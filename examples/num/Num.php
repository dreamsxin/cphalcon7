<?php

$ndarray = \Phalcon\Num::arange(0, 1);
echo $ndarray;

$ndarray = \Phalcon\Num::array([[1.0, 2, 3], [2, 3, 4]]);
echo $ndarray;

/* output: 
array([
  [1,2,3],
  [2,3,4]
])
*/

$ndarray = new \Phalcon\Num\Ndarray([[1.0, 2, 3], [2, 3, 4]]);
echo $ndarray;
/* output: 
array([
  [1,2,3],
  [2,3,4]
])
*/

var_dump($ndarray->getData()); // returns array([1.0, 2, 3], [2, 3, 4])

var_dump($ndarray->getShape()); // returns array(2, 3)

echo $ndarray->getNdim().PHP_EOL; // returns 2

echo $ndarray->getSize().PHP_EOL; // returns 6

echo \Phalcon\Num::amin($ndarray).PHP_EOL; // returns 1

echo \Phalcon\Num::amax($ndarray).PHP_EOL; // returns 4

$a = new \Phalcon\Num\Ndarray([[1.0, 2, 3], [2, 3, 4]]);
$b = new \Phalcon\Num\Ndarray([[3.2, 1.5, 1], [2.5, 4, 2]]);
echo $a->add($b);
/* output:
array([
  [4.2,3.5,4],
  [4.5,7,6]
])
*/

$a = new \Phalcon\Num\Ndarray([[1.0, 2, 3], [2, 3, 4]]);
$b = new \Phalcon\Num\Ndarray([[3.2, 1.5, 1], [2.5, 4, 2]]);
echo $a->sub($b);
/* output:
array([
  [-2.2,0.5,2],
  [-0.5,-1,2]
])
*/

$a = new \Phalcon\Num\Ndarray([[1.0, 2, 3], [2, 3, 4]]);
$b = new \Phalcon\Num\Ndarray([[3.2, 1.5, 1], [2.5, 4, 2]]);
echo $a->mult($b);
/* output:
array([
  [3.2,3,3],
  [5,12,8]
])
*/

$a = new \Phalcon\Num\Ndarray([[1.0, 2, 3], [2, 3, 4]]);
$b = new \Phalcon\Num\Ndarray([[3.2, 1.5, 1], [2.5, 4, 2]]);
echo $a->div($b);
/* output:
array([
  [0.3125,1.3333333333333,3],
  [0.8,0.75,2]
])
*/


$ndarray = \Phalcon\Num::array([[1.0, 2, 3], [2, 3, 4]]);
echo $ndarray->apply(function($val, $idx) {
	var_dump($val, $idx);
	return 0;
});