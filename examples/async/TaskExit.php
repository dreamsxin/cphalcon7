<?php

error_reporting(-1);
ini_set('display_errors', '1');

$obj = new class() {
    public function __destruct() {
        fwrite(STDERR, "=> DTOR!\n");
    }
};

register_shutdown_function(function () {
    fwrite(STDERR, "=> SHUTDOWN FUNC!\n");
});

$t = Phalcon\Async\Task::async(function () {
    var_dump('BEFORE EXIT');
    exit(7);
    var_dump('AFTER EXIT');
});


//(new Phalcon\Async\Timer(20))->awaitTimeout();
    
var_dump('BEFORE AWAIT');
Phalcon\Async\Task::await($t);
var_dump('AFTER AWAIT');
