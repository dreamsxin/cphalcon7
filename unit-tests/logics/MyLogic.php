<?php

class MyLogic extends Phalcon\Mvc\User\Logic
{
	public $num = 0;
	public $param1;
	public $param2;

	public function start()
	{
		$this->num += 1;
	}

	public function finish()
	{
		$this->num += 1;
	}

	// 覆盖方法 call
	public static function call($action = NULL, $params = NULL)
	{
		$logic = new MyLogic($action, $params);
		return $logic;
	}

}