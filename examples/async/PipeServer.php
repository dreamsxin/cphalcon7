<?php

error_reporting(-1);

$file = sprintf('%s/async-test.sock', sys_get_temp_dir());

if (file_exists($file)) {
unlink($file);
}

$server = Phalcon\Async\Network\Pipe\Server::listen($file);
$sock = $server->accept();

try {
    $timer = new Phalcon\Async\Timer(2);
    $len = 0;

    while (null !== ($chunk = $sock->read())) {
        $timer->awaitTimeout();
        $len += strlen($chunk);

        if ($chunk !== str_repeat('A', strlen($chunk))) {
            throw new \Error('Corrupted data received');
        }
    }

    var_dump($len);
} catch (\Throwable $e) {
    echo $e, "\n\n";
} finally {
    $sock->close();
}
