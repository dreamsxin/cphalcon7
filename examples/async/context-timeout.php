<?php

namespace Phalcon\Async;

Task::asyncWithContext(Context::current()->withTimeout(200), function () {
    Task::asyncWithContext(Context::current()->shield(), function () {
        (new Timer(1000))->awaitTimeout();
        
        var_dump('SHIELDED DONE');
    });
    
    try {
        var_dump((new Timer(1000))->awaitTimeout());
    } catch (\Throwable $e) {
        echo $e;
    }
    
    var_dump('DONE!');
});
