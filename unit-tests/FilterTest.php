<?php

/*
	+------------------------------------------------------------------------+
	| Phalcon Framework                                                      |
	+------------------------------------------------------------------------+
	| Copyright (c) 2011-2014 Phalcon Team (http://www.phalconphp.com)       |
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
	+------------------------------------------------------------------------+
*/

class FilterTest extends PHPUnit\Framework\TestCase
{
	public function testNormal()
	{
		$filter = new Phalcon\Filter;

		$ret = $filter->sanitize('1.1111', 'int!');
		$this->assertTrue(is_int($ret));

		$ret = $filter->sanitize('1.1111', 'float!');
		$this->assertTrue(is_float($ret));

		$ret = $filter->sanitize('-1.1111', 'abs');
		$this->assertTrue($ret === 1.1111);

		$ret = $filter->sanitize('D', ['in' => ['A', 'B', 'C']]);
		$this->assertTrue($ret === NULL);

		$ret = $filter->sanitize('A', ['in' => ['A', 'B', 'C']]);
		$this->assertTrue($ret === 'A');

		$data = array(
			'one' => ' one ',
			'two' => ['one' => ' one '],
			'three' => ['two' => ['one' => ' one ']]
		);

		$data2 = array(
			'one' => 'one',
			'two' => ['one' => 'one'],
			'three' => ['two' => ['one' => 'one']]
		);

		$ret = $filter->sanitize($data, 'trim', NULL, NULL, 3);
		$this->assertEquals($ret, $data2);
	}
}
