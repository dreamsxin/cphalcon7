<?php

class LogicController extends Phalcon\Mvc\Controller
{

	public function indexAction(\MyLogic $logic)
	{
		return $logic;
	}

}