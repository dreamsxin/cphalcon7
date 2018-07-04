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
  |          Rack Lin <racklin@gmail.com>                                  |
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

class DateTest extends PHPUnit\Framework\TestCase
{
	public function setUp()
	{
		date_default_timezone_set('America/Chicago');
	}

	public function testValid()
	{
		$this->assertTrue(\Phalcon\Date::valid("2012-12-12"));
		$this->assertTrue(\Phalcon\Date::valid("2012-12-12 11:11:11", 'Y-m-d H:i:s'));
		$this->assertTrue(\Phalcon\Date::valid("2012-12-12 11:11:11"));
		$this->assertFalse(\Phalcon\Date::valid("2012-12-12 11:11:11", 'Y-m-d'));

		$date = \Phalcon\Date::add('2000-01-01', '1 days');
		$this->assertEquals($date, '2000-01-02');

		$date = \Phalcon\Date::add('2000-01-01', '-1 days');
		$this->assertEquals($date, '1999-12-31');
	}

}
