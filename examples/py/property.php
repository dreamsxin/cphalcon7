<?php

Phalcon\Py::exec("
class Test:
    def __init__(self):
	    self.a = 1
");

$py = new Phalcon\Py\Object('__main__', 'Test');
var_dump(isset($py->a));
unset($py->a);
var_dump(isset($py->a));