<?php

register_shutdown_function(function () {
    echo "===> Shutdown function(s) execute here.\n";
});

$work = function (string $title): void {
    var_dump($title);
};

Phalcon\Async\Task::await(Phalcon\Async\Task::async(function () use ($work) {
    $defer = new Phalcon\Async\Deferred();

    Phalcon\Async\Task::await(Phalcon\Async\Task::async($work, 'A'));
    Phalcon\Async\Task::await(Phalcon\Async\Task::async($work, 'B'));

    Phalcon\Async\Task::async(function () {
        $defer = new Phalcon\Async\Deferred();

        Task::async(function () use ($defer) {
            (new Timer(1000))->awaitTimeout();

            $defer->resolve('H :)');
        });

        var_dump(Phalcon\Async\Task::await($defer->awaitable()));
    });

    Phalcon\Async\Task::async(function () use ($defer) {
        var_dump(Phalcon\Async\Task::await($defer->awaitable()));
    });

    $timer = new Phalcon\Async\Timer(500);

    Phalcon\Async\Task::async(function () use ($timer, $defer, $work) {
        $timer->awaitTimeout();

        $defer->resolve('F');

        Phalcon\Async\Task::async($work, 'G');
    });

    var_dump('ROOT TASK DONE');
}));

Phalcon\Async\Task::async($work, 'C');

Phalcon\Async\Task::async(function () use ($work) {
    (new Phalcon\Async\Timer(0))->awaitTimeout();

    Phalcon\Async\Task::async($work, 'E');
});

Phalcon\Async\Task::async(function ($v) {
    var_dump(Phalcon\Async\Task::await($v));
}, Phalcon\Async\Deferred::value('D'));

var_dump('=> END OF MAIN SCRIPT');
