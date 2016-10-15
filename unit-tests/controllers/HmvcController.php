<?php

class HmvcController extends Phalcon\Mvc\Controller
{

	public function oneAction()
	{
		echo $this->dispatcher->getActionName().'-'.$this->app->request('/hmvc/two').'-'.$this->dispatcher->getActionName();
	}

	public function twoAction()
	{
		echo $this->dispatcher->getActionName();
	}
}