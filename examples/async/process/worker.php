<?php

var_dump(Phalcon\Async\Process\Process::isWorker());
var_dump($ipc = Phalcon\Async\Process\Process::connect());

var_dump($tcp = Phalcon\Async\Network\TcpSocket::import($ipc));

try {
    while (null !== ($chunk = $tcp->read())) {
        echo $chunk;
    }
} finally {
    $tcp->close();
}
