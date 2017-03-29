<?php

class Continue2Controller extends Phalcon\Mvc\Controller
{
	public function indexAction()
	{
		throw new Phalcon\ContinueException("This is an continue exception");
	}
}
