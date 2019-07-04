<?php

namespace Phalcon\Async;

$result = TaskScheduler::run(function () {
    $t = Task::async(function (int $delay): int {
        (new Timer($delay))->awaitTimeout();
        
        return max(123, Task::await(Deferred::value()));
    }, 100);

    printf("LAUNCHED TASK: [%s]\nin %s:%u\n\n", $t->status, $t->file, $t->line);
    
    (new Timer(10))->awaitTimeout();

    print_r($t);
    print_r($t->getTrace());

    try {
        Task::await(Deferred::error(new \Error('Fail!')));
    } catch (\Throwable $e) {
        var_dump($e->getMessage());
    }

    var_dump(2 * Task::await($t));

    return 777;
});

$timer = new Timer(500);
$timer->awaitTimeout();
var_dump($result);
