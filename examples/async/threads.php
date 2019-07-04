<?php

namespace Phalcon\Async;

var_dump(Thread::isAvailable());
var_dump(Thread::isWorker());

$thread = new Thread(__DIR__ . '/threads-bootstrap.php');
$ipc = $thread->getIpc();

try {
    $ipc->write('Hello!');
    var_dump($ipc->read());
} finally {
    $ipc->close();
}

$thread->join();
