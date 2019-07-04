<?php

namespace Phalcon\Async\Process;

$builder = ProcessBuilder::shell(true);
$builder = $builder->withStdoutInherited();
$builder = $builder->withStderrInherited();

$process = $builder->start();
$stdin = $process->getStdin();

try {
    $stdin->write("exit 6\n");
} finally {
    $stdin->close();
    
    var_dump($process->join());
}
