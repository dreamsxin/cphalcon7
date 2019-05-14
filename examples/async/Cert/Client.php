<?php

namespace Phalcon\Async\Network;

$tls = new TlsClientEncryption();
$tls = $tls->withAllowSelfSigned(true);

$sock = TcpSocket::connect('localhost', 10011, $tls);

if (!empty($_SERVER['argv'][1])) {
    $sock->encrypt();
}

try {
    $chunk = str_repeat('A', 7000);

    for ($i = 0; $i < 1000; $i++) {
        $sock->write($chunk);
    }
} finally {
    $sock->close();
}
