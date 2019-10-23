<?php




namespace Phalcon\Async;

function job(Deferred $defer, ContextVar $var)
{
    var_dump('INNER DONE!');
	// $req = Context::current()->getVar('request');
    // $defer->resolve($num = Context::current()->getVar());
    $defer->resolve($num = $var->get());
    
    return $num;
}

$var = new ContextVar();
$req = new ContextVar('requset');

$context = Context::current();

$context = $context->with($req, 'It\'s me');
$context = $context->with($var, 777);

$defer = new Deferred();

$t = Task::asyncWithContext($context, __NAMESPACE__ . '\\job', $defer, $var);

var_dump('GO WAIT');
var_dump(Task::await($defer->awaitable()));
var_dump(Task::await($t));
var_dump('OUTER DONE!');
