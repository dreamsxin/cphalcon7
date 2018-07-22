<?php

class MyServices
{
	private $val = 0;
	public function doVal() {
		echo 'Myval='.$this->val.PHP_EOL;
		$this->val++;
		echo 'Myval='.$this->val.PHP_EOL;
	}
}

Phalcon\Aop::addBefore('read MyServices->val', function($obj){
	var_dump('before read');
	echo $obj->getPropertyName().'='.$obj->getPropertyValue().PHP_EOL;
});
Phalcon\Aop::addAfter('read MyServices->val', function($obj){
	var_dump('after read');
	echo $obj->getPropertyName().'='.$obj->getPropertyValue().PHP_EOL;
});
Phalcon\Aop::addBefore('write MyServices->val', function($obj){
	var_dump('before write');
	echo $obj->getPropertyName().'='.$obj->getPropertyValue().PHP_EOL;
	$obj->setAssignedValue(3);
});
Phalcon\Aop::addAfter('write MyServices->val', function($obj){
	var_dump('after write');
	echo $obj->getPropertyName().'='.$obj->getPropertyValue().PHP_EOL;
});
$services = new MyServices();
$services->doVal();
