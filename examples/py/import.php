<?php

function test() {
	echo 'test';
}
$phalcon = Phalcon\Py::import('phalcon');
Phalcon\Py::callFunction($phalcon, 'call', 'test');