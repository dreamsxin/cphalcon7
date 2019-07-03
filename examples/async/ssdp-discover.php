<?php

namespace Phalcon\Async\Network;

$socket = UdpSocket::bind();

try {
    $socket->send(new UdpDatagram(implode("\r\n", [
        'M-SEARCH * HTTP/1.1',
        'HOST: 239.255.255.250:1900',
        'MAN: "ssdp:discover"',
        'SERVER: PHP/' . PHP_VERSION,
        'MX: 2',
        'ST: ssdp:all'
    ]) . "\r\n\r\n", '239.255.255.250', 1900));

    print_r($socket->receive());
} finally {
    $socket->close();
}
