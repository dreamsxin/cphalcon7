<?php

$builder = Phalcon\Async\Process\Builder::shell(true);
$builder = $builder->withStdoutInherited();
$builder = $builder->withStderrInherited();

$process = $builder->start();
$stdin = $process->getStdin();

try {
    $stdin->write("ls -la\n");
} finally {
    $stdin->close();
    
    var_dump($process->join());
}
