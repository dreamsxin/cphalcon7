<?php

$builder = new Phalcon\Async\Process\Builder(PHP_BINARY);
$builder->configureStdout(Phalcon\Async\Process\Builder::STDIO_INHERIT, Phalcon\Async\Process\Builder::STDOUT);
$builder->configureStderr(Phalcon\Async\Process\Builder::STDIO_INHERIT, Phalcon\Async\Process\Builder::STDERR);

$builder->setEnv([
    'MY_TITLE' => 'TEST'
]);

$process = $builder->start(__DIR__ . '/process/p2.php');

Phalcon\Async\Task::async(function () use ($process) {
    (new Phalcon\Async\Timer(400))->awaitTimeout();
    var_dump('SIGNAL!');
    $process->signal(Phalcon\Async\SignalWatcher::SIGINT);
});

$code = $process->join();

echo "\nEXIT CODE: ", $code, "\n";

$builder->configureStdin(Phalcon\Async\Process\Builder::STDIO_PIPE);

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

$builder = new Phalcon\Async\Process\Builder('ls');
$builder->setDirectory(__DIR__);
$builder->configureStdout(Phalcon\Async\Process\Builder::STDIO_PIPE);
$builder->configureStderr(Phalcon\Async\Process\Builder::STDIO_INHERIT, Phalcon\Async\Process\Builder::STDERR);

$process = $builder->start(...\array_slice($_SERVER['argv'], 1));

// \Concurrent\Task::async($reader, $process->getStdout(), 256);

$reader($process->getStdout(), 256);

$code = $process->join();

echo "\nEXIT CODE: ", $code, "\n";
