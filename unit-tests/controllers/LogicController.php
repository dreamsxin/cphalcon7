<?php

class LogicController extends Phalcon\Mvc\Controller
{

	public function indexAction(\MyLogic $logic, $param2, $param1)
	{
		$logic->param1 = $param1;
		$logic->param2 = $param2;
		return $logic;
	}

}