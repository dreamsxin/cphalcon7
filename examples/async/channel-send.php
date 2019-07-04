<?php

// Pass any truthy arg via cli to enable blocking select behavior.

namespace Phalcon\Async;

error_reporting(-1);
ini_set('display_errors', (DIRECTORY_SEPARATOR == '\\') ? '0' : '1');

$a = new Channel();
$b = new Channel(2);

Task::async(function (iterable $it) {
    $timer = new Timer(300);
    $timer->awaitTimeout();

    foreach ($it as $val) {
        var_dump('RECV: ' . $val);

        $timer->awaitTimeout();
    }
}, $a->getIterator());

$group = new ChannelGroup([
    'A' => $a,
    'B' => $b
]);

if (empty($_SERVER['argv'][1])) {
    $timeout = null;
} else {
    $timeout = (int) $_SERVER['argv'][1];
}

for ($i = 0; $i < 5; $i++) {
    if (null !== ($t = $group->send($i, $timeout))) {
        var_dump('SEND TO: ' . $t);
    } else {
        var_dump('DISCARD');
    }
}

$a->close();
$b->close();

foreach ($b as $val) {
    var_dump('BUF: ' . $val);
}
