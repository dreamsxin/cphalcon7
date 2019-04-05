<?php

class Test {
	public function job(Phalcon\Async\Deferred $defer = NULL, Phalcon\Async\ContextVar $var) {
		if ($defer) {
			$defer->resolve($num = $var->get());
		}
		return $var->get();
	}
}

$var = new Phalcon\Async\ContextVar();

$context = Phalcon\Async\Context::current();
$context1 = $context->with($var, 111);
$context2 = $context->with($var, 222);

$defer = new Phalcon\Async\Deferred();

$t = Phalcon\Async\Task::asyncWithContext($context1, 'Test::job', $defer, $var);

var_dump('----GO WAIT----');
var_dump(Phalcon\Async\Task::await($defer->awaitable()));
var_dump(Phalcon\Async\Task::await($t));
var_dump('OUTER DONE!');

$t = Phalcon\Async\Task::asyncWithContext($context2, 'Test::job', NULL, $var);

var_dump('----GO WAIT2----');
var_dump(Phalcon\Async\Task::await($t));
var_dump('OUTER DONE2!');


var_dump('----GO CANCELLED----');
$cancel = null;
$context = Phalcon\Async\Context::current()->withCancel($cancel);
Phalcon\Async\Task::asyncWithContext($context, function () {
    try {
        (new Phalcon\Async\Timer(200))->awaitTimeout();

        var_dump('TASK COMPLETED');
    } catch (\Throwable $e) {
		var_dump('TASK CANCELLED');
        //echo $e;
    }
});

$cancel(new \Error('This is taking too long...'));

$timer = new Phalcon\Async\Timer(200);
$timer->awaitTimeout();

var_dump('----GO CANCELLED2----');
$cancel2 = null;
$context = Phalcon\Async\Context::current()->withCancel($cancel2);
Phalcon\Async\Task::asyncWithContext($context, function (Phalcon\Async\Context $context) {
	$defer = new Phalcon\Async\Deferred(function (Phalcon\Async\Deferred $defer, \Throwable $e) {
		var_dump('Error1:'.$e->getMessage());
		$defer->resolve(777);
	});

	var_dump('AWAIT DEFERRED');
	var_dump($context->cancelled);
	// 等待取消异常
	var_dump(Phalcon\Async\Task::await($defer->awaitable()));
	var_dump('AWAIT DEFERRED2');
	var_dump($context->cancelled);
	
	try {
		$context->throwIfCancelled();
	} catch (\Throwable $e) {
		var_dump('Error2:'.$e->getMessage());
	}
}, $context);

$timer->awaitTimeout();

var_dump('=> CANCEL!');
$cancel2();

$timer->awaitTimeout();

var_dump('DONE');
