<?php

var_dump(Phalcon\Async\Process::isForked());
var_dump($ipc = Phalcon\Async\Process::forked());

var_dump($tcp = Phalcon\Async\Network\TcpSocket::import($ipc));

try {
    while (null !== ($chunk = $tcp->read())) {
        echo $chunk;
    }
} finally {
    $tcp->close();
}
