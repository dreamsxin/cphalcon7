<?php

class MyLogic extends Phalcon\Mvc\User\Logic
{
	public $num = 0;

	// 覆盖方法 call
	public static function call($action = NULL, $params = NULL)
	{
		$logic = new MyLogic($action, $params);
		$logic->num = 1;
		return $logic;
	}

}