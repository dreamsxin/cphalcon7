<?php

namespace Phalcon\Async;

var_dump('I AM A THREAD! :-)');
var_dump(Thread::isWorker());

$ipc = Thread::connect();
$ipc = Thread::connect();

try {
    var_dump($ipc->read());
    $ipc->write('world');
} finally {
    $ipc->close();
}
