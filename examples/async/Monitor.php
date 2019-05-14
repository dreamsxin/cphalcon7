<?php

$monitor = new Phalcon\Async\Monitor('/tmp/');

$event = $monitor->awaitEvent();
var_dump($event);

