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

class DateTimeTest extends PHPUnit\Framework\TestCase
{
	public function setUp()
	{
		date_default_timezone_set('PRC');
	}

	public function testStartEnd()
	{
        $dt = new Phalcon\Date\DateTime('2016-12-10 10:22:22');
        $this->assertInstanceOf('Phalcon\Date\DateTime', $dt->startOfDay());
		$this->assertEquals($dt->format('Y-m-d H:i:s'), '2016-12-10 00:00:00');
        $this->assertInstanceOf('Phalcon\Date\DateTime', $dt->endOfDay());
		$this->assertEquals($dt->format('Y-m-d H:i:s'), '2016-12-10 23:59:59');

        $dt = new Phalcon\Date\DateTime('2016-12-10 10:22:22');
        $this->assertInstanceOf('Phalcon\Date\DateTime', $dt->startOfMonth());
		$this->assertEquals($dt->format('Y-m-d H:i:s'), '2016-12-01 00:00:00');
        $this->assertInstanceOf('Phalcon\Date\DateTime', $dt->endOfMonth());
		$this->assertEquals($dt->format('Y-m-d H:i:s'), '2016-12-31 23:59:59');

        $dt = new Phalcon\Date\DateTime('2016-11-10 10:22:22');
        $this->assertInstanceOf('Phalcon\Date\DateTime', $dt->startOfQuarter());
		$this->assertEquals($dt->format('Y-m-d H:i:s'), '2016-10-01 00:00:00');
        $this->assertInstanceOf('Phalcon\Date\DateTime', $dt->endOfQuarter());
		$this->assertEquals($dt->format('Y-m-d H:i:s'), '2016-12-31 23:59:59');

        $dt = new Phalcon\Date\DateTime('2016-11-10 10:22:22');
        $this->assertInstanceOf('Phalcon\Date\DateTime', $dt->startOfYear());
		$this->assertEquals($dt->format('Y-m-d H:i:s'), '2016-01-01 00:00:00');
        $this->assertInstanceOf('Phalcon\Date\DateTime', $dt->endOfYear());
		$this->assertEquals($dt->format('Y-m-d H:i:s'), '2016-12-31 23:59:59');

        $dt = new Phalcon\Date\DateTime('2016-11-10 10:22:22');
        $this->assertInstanceOf('Phalcon\Date\DateTime', $dt->startOfDecade());
		$this->assertEquals($dt->format('Y-m-d H:i:s'), '2011-01-01 00:00:00');
        $this->assertInstanceOf('Phalcon\Date\DateTime', $dt->endOfDecade());
		$this->assertEquals($dt->format('Y-m-d H:i:s'), '2020-12-31 23:59:59');

        $dt = new Phalcon\Date\DateTime('2016-11-10 10:22:22');
        $this->assertInstanceOf('Phalcon\Date\DateTime', $dt->startOfCentury());
		$this->assertEquals($dt->format('Y-m-d H:i:s'), '2001-01-01 00:00:00');
        $this->assertInstanceOf('Phalcon\Date\DateTime', $dt->endOfCentury());
		$this->assertEquals($dt->format('Y-m-d H:i:s'), '2100-12-31 23:59:59');
	}

	public function testModify()
	{
		$dt = new Phalcon\Date\DateTime('2016-12-10 10:22:22');
		$dt->modifyYear(-1);
		$this->assertEquals($dt->format('Y-m-d H:i:s'), '2015-12-10 10:22:22');
		$dt->modifyQuarter(4);
		$this->assertEquals($dt->format('Y-m-d H:i:s'), '2016-12-10 10:22:22');
		$dt->modifyMonth(-1);
		$this->assertEquals($dt->format('Y-m-d H:i:s'), '2016-11-10 10:22:22');
		$dt->modifyDay(-1);
		$this->assertEquals($dt->format('Y-m-d H:i:s'), '2016-11-09 10:22:22');
		$dt->modifyHour(-1);
		$this->assertEquals($dt->format('Y-m-d H:i:s'), '2016-11-09 09:22:22');
		$dt->modifyMinute(-1);
		$this->assertEquals($dt->format('Y-m-d H:i:s'), '2016-11-09 09:21:22');
		$dt->modifySecond(-1);
		$this->assertEquals($dt->format('Y-m-d H:i:s'), '2016-11-09 09:21:21');
	}
}
