<?php

/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2012 Phalcon Team (http://www.phalconphp.com)       |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled     |
  | with this package in the file docs/LICENSE.txt.                        |
  |                                                                        |
  | If you did not receive a copy of the license and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@phalconphp.com so we can send you a copy immediately.       |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@phalconphp.com>                      |
  |          Eduar Carvajal <eduar@phalconphp.com>                         |
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

class ArrTest extends PHPUnit_Framework_TestCase
{

	public function testNormal()
	{
		$data = array(
			'username' => 'Dreamszhu',
			'password' => '123456',
			'address' => 'ZheJiang',
			'foo' => array('bar' => 'Hello World'),
			'theme' => array(
				'one' => array('color' => 'green'),
				'two' => array('color' => 'red', 'size' => 11)
			)
		);

		$this->assertTrue(\Phalcon\Arr::is_assoc($data));
		$this->assertTrue(\Phalcon\Arr::is_array($data));
		
		// Get the value of $data['foo']['bar']
		$value = \Phalcon\Arr::path($data, 'foo.bar');
		$this->assertEquals($value, 'Hello World');

		$colors = \Phalcon\Arr::path($data, 'theme.*.color');
		$this->assertEquals($colors, array('green', 'red'));

		// Using an array of keys
		$colors = \Phalcon\Arr::path($data, array('theme', '*', 'color'));
		$this->assertEquals($colors, array('green', 'red'));

		// Set the values of "color" in theme
		\Phalcon\Arr::set_path($data, 'theme.*.color', 'blue');
		$colors = \Phalcon\Arr::path($data, array('theme', '*', 'color'));
		$this->assertEquals($colors, array('blue', 'blue'));

		// Append the values of "color" in theme
		\Phalcon\Arr::set_path($data, 'theme.*.color', 'red', NULL, true);
		$colors = \Phalcon\Arr::path($data, array('theme', '*', 'color'));
		$this->assertEquals($colors, array(array('blue', 'red'), array('blue', 'red')));

		$values = \Phalcon\Arr::range(5, 20);
		$this->assertEquals($values, array(5 => 5, 10 => 10, 15 => 15, 20 => 20));

		// Get the value "username", if it exists
		$username = \Phalcon\Arr::get($data, 'username');
		$this->assertEquals($username, 'Dreamszhu');

		$sex = \Phalcon\Arr::get($data, 'sex', 'No');
		$this->assertEquals($sex, 'No');

		// Use callback get value
		$sex = \Phalcon\Arr::get($data, function($array, $defalut_value){
			return isset($array['sex']) ? 'Yes' : 'No';
		});
		$this->assertEquals($sex, 'No');

		$info = \Phalcon\Arr::get($data, array('username', 'address'));
		$this->assertEquals($info, array('username' => 'Dreamszhu', 'address' => 'ZheJiang'));

		$sex = \Phalcon\Arr::choice($data, 'sex', 'one', 'two');
		$this->assertEquals($sex, 'two');

		// Get the values "username", "password"
		$auth = \Phalcon\Arr::extract($data, array('username', 'password'));
		$this->assertEquals($auth, array('username' => 'Dreamszhu', 'password' => '123456'));

		$data = array(
			array('id' => 1, 'name' => 'Google'),
			array('id' => 2, 'name' => 'Baidu')
		);

		// Get all of the "id" values from a result
		$ids = \Phalcon\Arr::pluck($data, 'id');
		$this->assertEquals($ids, array(1, 2));

		// Merge
		$john = array('name' => 'john', 'children' => array('fred', 'paul', 'sally', 'jane'));
		$mary = array('name' => 'mary', 'children' => array('jane'));

		$this->assertEquals(\Phalcon\Arr::merge($john, $mary), array('name' => 'mary', 'children' => array('fred', 'paul', 'sally', 'jane')));
	$this->assertEquals(\Phalcon\Arr::merge($john, $mary, array('sex' => 1)), array('name' => 'mary', 'children' => array('fred', 'paul', 'sally', 'jane'), 'sex' => 1));

		// Overwrite
		$a1 = array('name' => 'john', 'mood' => 'happy', 'food' => 'bacon');
		$a2 = array('name' => 'jack', 'food' => 'tacos', 'drink' => 'beer');

		$this->assertEquals(\Phalcon\Arr::overwrite($a1, $a2), array('name' => 'jack', 'mood' => 'happy', 'food' => 'tacos'));
		$this->assertEquals(\Phalcon\Arr::overwrite($a1, $a2, array('name' => 'Phalcon7')), array('name' => 'Phalcon7', 'mood' => 'happy', 'food' => 'tacos'));
	}
}
