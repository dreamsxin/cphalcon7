<?php

// 重连数据库
Phalcon\Aop::addBefore('Phalcon\Db\Pdo::prepare()', function($obj){
	$db = $obj->getObject();
	try{
		$pdo = $db->getInternalHandler();
		$pdo->getAttribute(\PDO::ATTR_CONNECTION_STATUS);
	} catch (PDOException $e) {
		$db->connect();
	}
});
