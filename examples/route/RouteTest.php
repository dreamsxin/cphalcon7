<?php

//Phalcon\Debug::enable();
$router = new Phalcon\Mvc\Router(FALSE, TRUE); // Use TreeRoute
$router->add('/{controller}/{action}/{int:.*}', ['controller' => 'controller', 'action' => 'action']);
$router->handle('/hello/world/1');
var_dump($router->getControllerName(), $router->getActionName(), $router->getParams());


$router = new Phalcon\Mvc\Router(FALSE);
$router->add('/:controller/:action/:params', ['controller' => 1, 'action' => 2, 'params' => 3]);
$router->handle('/hello/world/1');
var_dump($router->getControllerName(), $router->getActionName(), $router->getParams());

Phalcon\Debug::disable();