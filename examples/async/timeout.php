<?php

namespace Phalcon\Async;

call_user_func(function () {
    $timeout = Timer::timeout(500);

    Task::await($timeout);
});

call_user_func(function () {
    Timer::timeout(500);
});
