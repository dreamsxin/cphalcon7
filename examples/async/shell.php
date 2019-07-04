<?php

namespace Phalcon\Async\Process;

$builder = ProcessBuilder::shell();
$builder = $builder->withStdoutInherited();

var_dump('WAIT A SECOND...');

if (DIRECTORY_SEPARATOR == '\\') {
    var_dump($builder->execute('ping -n 2 localhost > NUL && echo DONE'));
} else {
    var_dump($builder->execute('sleep 1 && echo DONE'));
}
