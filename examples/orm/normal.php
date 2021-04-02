<?php

$di = new \Phalcon\DI\FactoryDefault();

$di->set('db', function () {
	$configSqlite = array(
		'dbname' => '/tmp/phalcon_test.sqlite',
	);
	return new \Phalcon\Db\Adapter\Pdo\Sqlite($configSqlite);
}, TRUE);

// 导入 sql
$builder = Phalcon\Async\Process\ProcessBuilder::shell();
$builder = $builder->withStdoutInherited();
var_dump($builder->execute('sqlite3 /tmp/phalcon_test.sqlite < '.__DIR__.'/../../unit-tests/schemas/sqlite/phalcon_test.sql'));

// 定义 Robots 类
var_dump(Phalcon\Mvc\ORM::factory('robots'), new Robots);

$robot = Phalcon\Mvc\ORM::factory('robots')->findFirst();
var_dump($robot);

// 实例化
$robot = Phalcon\Mvc\ORM::factory('robots')->new();
var_dump($robot);