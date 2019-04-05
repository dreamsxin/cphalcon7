<?php

call_user_func(function () {
    $timeout = Phalcon\Async\Timer::timeout(500);

    Phalcon\Async\Task::await($timeout);
});

call_user_func(function () {
    Phalcon\Async\Timer::timeout(500);
});
