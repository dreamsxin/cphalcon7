<?php

echo "Python in PHP (" . Phalcon\Py::version() . ")\n";

$stdin = fopen('php://stdin', 'r');

while (!feof($stdin)) {
    echo ">>> ";
    $line = rtrim(fgets($stdin));
    Phalcon\Py::exec($line);
}
