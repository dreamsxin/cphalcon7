<?php

namespace Phalcon\Async\Network;

$socket = UdpSocket::multicast('239.255.255.250', 1900);

try {
    while (true) {
        print_r($datagram = $socket->receive());

        $datagram = $datagram->withData(implode("\r\n", [
            'HTTP/1.1 200 OK',
            'CACHE-CONTROL: max-age=1800',
            'DATE: ' . gmdate('D, d M Y H:i:s') . ' GMT',
            'EXT: ',
            'LOCATION: http://localhost:8080/root.xml',
            'OPT: "http://schemas.upnp.org/upnp/1/0/"; ns=01',
            'SERVER: PHP/' . PHP_VERSION,
            'ST: upnp:rootdevice',
            'USN: uuid:3a06277c-5888-4a7d-b66b-20d914d168bd::upnp:rootdevice'
        ]) . "\r\n\r\n");
        
        var_dump($socket->sendAsync($datagram));
    }
} finally {
    $socket->close();
}
