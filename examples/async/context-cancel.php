<?php

namespace Phalcon\Async;

$cancel = null;
$context = Context::current()->withCancel($cancel);

Task::asyncWithContext($context, function () {
    var_dump('START TASK');

    try {
        (new Timer(1000))->awaitTimeout();

        var_dump('TASK COMPLETED');
    } catch (\Throwable $e) {
        echo $e;
    }
});

(new Timer(200))->awaitTimeout();

$cancel(new \Error('This is taking too long...'));

echo "\n";
