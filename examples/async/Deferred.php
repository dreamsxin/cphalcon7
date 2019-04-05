<?php


class Test {
	public function job(Phalcon\Async\Deferred $defer) {
		$defer->resolve('A');
	}
}

$defer = new Phalcon\Async\Deferred();

$t = Phalcon\Async\Task::async('Test::job', $defer);
var_dump(Phalcon\Async\Task::await($defer->awaitable()));

Phalcon\Async\Task::async(function ($v) {
    var_dump(Phalcon\Async\Task::await($v));
}, Phalcon\Async\Deferred::value('D'));

var_dump('=> END OF MAIN SCRIPT');
