<?php

error_reporting(-1);
ini_set('display_errors', (DIRECTORY_SEPARATOR == '\\') ? '0' : '1');

$async = !empty($_SERVER['argv'][1]);

function sslpair(): array
{
    $file = __DIR__. '/Cert/server.';

    $tls = new Phalcon\Async\Network\TlsServerEncryption();
    $tls = $tls->withDefaultCertificate($file . 'crt', $file . 'key', '123456');

    $server = Phalcon\Async\Network\TcpServer::listen('localhost', 0, $tls);

    $tasks = [
         Phalcon\Async\Task::async(function () use ($server) {
            try {
                $socket = $server->accept();
            } finally {
                $server->close();
            }

            $socket->encrypt();

            return $socket;
        }),
         Phalcon\Async\Task::async(function () use ($server) {
            $tls = new Phalcon\Async\Network\TlsClientEncryption();
            $tls = $tls->withAllowSelfSigned(true);
            $tls = $tls->withVerifyDepth(5);

            $socket = Phalcon\Async\Network\TcpSocket::connect($server->getAddress(), $server->getPort(), $tls);

            try {
                $socket->encrypt();
            } catch (\Throwable $e) {
                $socket->close();

                throw $e;
            }

            return $socket;
        })
    ];

    $result = array_fill(0, 2, null);

    return Phalcon\Async\Task::await(Phalcon\Async\Deferred::combine($tasks, function (Phalcon\Async\Deferred $defer, bool $last, $k, ?\Throwable $e, $v = null) use (& $result) {
        if ($e) {
            $defer->fail($e);
        } else {
            $result[$k] = $v;

            if ($last) {
                $defer->resolve($result);
            }
        }
    }));
}

list ($a, $b) = sslpair();

$chunkSize = 7000;
$count = 100000;

Phalcon\Async\Task::async(function () use ($a, $async, $chunkSize, $count) {
    try {
        $chunk = str_repeat('A', $chunkSize);

        for ($i = 0; $i < $count; $i++) {
            if ($async) {
                Task::async([
                    $a,
                    'write'
                ], $chunk);
            } else {
                $a->write($chunk);
            }
        }
        
        $a->flush();
    } catch (\Throwable $e) {
        echo $e, "\n\n";
    } finally {
        $a->close();
    }
});

$len = 0;
$time = microtime(true);

try {
    while (null !== ($chunk = $b->read())) {
        $len += strlen($chunk);

        if (!preg_match("'^A+$'", $chunk)) {
            throw new \Error('Corrupted data received');
        }
    }
} finally {
    $b->close();
}

if (($chunkSize * $count) != $len) {
    throw new \RuntimeException('Unexpected message size');
}

printf("Time taken: %.2f seconds\n", microtime(true) - $time);
printf("Data transferred: %.2f MB\n", $len / 1024 / 1024);
printf("Memory usage: %.2f MB\n\n", memory_get_peak_usage() / 1024 / 1024);
