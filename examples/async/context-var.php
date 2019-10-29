<?php




namespace Phalcon\Async;

function job(Deferred $defer, ContextVar $var)
{
    var_dump('INNER DONE!');
	var_dump(Context::current()->getVar('request'));
    // $defer->resolve($num = Context::current()->getVar());
    $defer->resolve($num = $var->get());
    
    return $num;
}

$var = new ContextVar();
$req = new ContextVar('request');

$context = Context::current();

$context = $context->with($var, 777);
$context = $context->with($req, 'It\'s me');

$defer = new Deferred();

$t = Task::asyncWithContext($context, __NAMESPACE__ . '\\job', $defer, $var);

var_dump('GO WAIT');
var_dump(Task::await($defer->awaitable()));
var_dump(Task::await($t));
var_dump('OUTER DONE!');


$context->setVar('request', 'It\'s me2');
var_dump($context->getVar('request'));
