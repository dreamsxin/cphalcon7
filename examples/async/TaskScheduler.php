<?php

$result = Phalcon\Async\TaskScheduler::run(function () {
    $t = Phalcon\Async\Task::async(function (): int {
        return max(123, Phalcon\Async\Task::await(Phalcon\Async\Deferred::value()));
    });

    printf("LAUNCHED TASK: [%s]\nin %s:%u\n\n", $t->status, $t->file, $t->line);
    
    print_r($t);
    
    try {
        Phalcon\Async\Task::await(Phalcon\Async\Deferred::error(new \Error('Fail!')));
    } catch (\Throwable $e) {
        var_dump($e->getMessage());
    }
    
    var_dump(2 * Phalcon\Async\Task::await($t));
    
    return 777;
}, function (array $tasks) {
    print_r($tasks);
});

$timer = new Phalcon\Async\Timer(500);
$timer->awaitTimeout();
var_dump($result);
