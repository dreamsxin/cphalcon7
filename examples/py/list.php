<?php

$list = Phalcon\Py::list([5,1,2,3,4]);
var_dump($list);
var_dump(Phalcon\Py::callMethod($list, 'sort'));
var_dump($list);