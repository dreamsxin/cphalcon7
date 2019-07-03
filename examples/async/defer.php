<?php

namespace Phalcon\Async;

$defer = new Deferred();

Task::async(function () use ($defer) {
    sleep(1);

    $defer->resolve('Hello!');
});

var_dump(Task::await(Deferred::transform($defer->awaitable(), function (?\Throwable $e, ?string $v = null) {
    if ($e) {
        throw $e;
    }

    return strtoupper($v);
})));
