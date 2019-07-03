<?php

namespace Phalcon\Async\Network;

list ($a, $b) = TcpSocket::pair();

\Phalcon\Async\Task::async(function () use ($a) {
    try {
        $a->write('Hello World :)');
    } finally {
        $a->close();
    }
});

var_dump($b->read());
