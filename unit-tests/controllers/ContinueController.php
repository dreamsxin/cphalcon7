<?php

class ContinueController extends Phalcon\Mvc\Controller
{

	public function afterExecuteroute()
	{
		$this->dispatcher->setReturnedValue('ok');
	}

	public function indexAction()
	{
		$this->dispatcher->setReturnedValue('fail');
		throw new Phalcon\ContinueException("This is an continue exception");
	}
}
