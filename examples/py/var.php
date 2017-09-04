<?php

$global = 'global';

$py = <<<EOT
import phalcon

var = phalcon.var('global')

unknown = ''
try:
    phalcon.var('unknown')
except NameError, e:
    unknown = str(e)
EOT;

Phalcon\Py::exec($py);
var_dump(Phalcon\Py::eval('var'));
var_dump(Phalcon\Py::eval('unknown'));