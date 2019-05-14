<?php

namespace Phalcon\Async\Network;

$file = __DIR__ . '/localhost.';

$tls = new TlsServerEncryption();
$tls = $tls->withDefaultCertificate($file . 'crt', $file . 'key', 'localhost');

$server = TcpServer::listen('localhost', 10011, $tls);

$sock = $server->accept();

if (!empty($_SERVER['argv'][1])) {
    $sock->encrypt();
}

try {
    $timer = new \Phalcon\Async\Timer(2);
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
