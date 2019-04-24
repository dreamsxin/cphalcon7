<?php

$builder = new Phalcon\Async\Process\Builder(PHP_BINARY);
$builder->withStdoutInherited();
$builder->withStderrInherited();

$builder = $builder->withEnv([
    'MY_TITLE' => 'TEST'
], true);

$process = $builder->start(__DIR__ . '/process/p2.php');

Phalcon\Async\Task::async(function () use ($process) {
    (new Phalcon\Async\Timer(400))->awaitTimeout();
    var_dump('SIGNAL!');
    $process->signal(Phalcon\Async\SignalWatcher::SIGINT);
});

$code = $process->join();

echo "\nEXIT CODE: ", $code, "\n";

$builder = $builder->withStdinPipe();
$process = $builder->start(__DIR__ . '/process/p1.php');

$stdin = $process->getStdin();

try {
    $stdin->write('Hello');
    
    (new Phalcon\Async\Timer(100))->awaitTimeout();
    
    $stdin->write(' World ');
    $stdin->write(':)');
} finally {
    $stdin->close();
}

$code = $process->join();

echo "\nEXIT CODE: ", $code, "\n";

$reader = function (Phalcon\Async\Process\ReadablePipe $pipe, int $len) {
    try {
        while (null !== ($chunk = $pipe->read($len))) {
            var_dump($chunk);
        }
    } finally {
        $pipe->close();
    }
};

$builder = Phalcon\Async\Process\Builder::shell();
$builder = $builder->withCwd(__DIR__);
$builder = $builder->withStdoutPipe();
$builder = $builder->withStderrInherited();

$process = $builder->start((\DIRECTORY_SEPARATOR == '\\') ? 'dir' : 'ls');

Phalcon\Async\Task::async($reader, $process->getStdout(), 256);

$code = $process->join();

echo "\nEXIT CODE: ", $code, "\n";
