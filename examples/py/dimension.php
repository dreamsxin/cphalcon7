<?php

$py = Phalcon\Py::eval('[1, 2, 3]');
var_dump($py);
var_dump(isset($py[2]));
unset($py[2]);
var_dump(isset($py[2]));