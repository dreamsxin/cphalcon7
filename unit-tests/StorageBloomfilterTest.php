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

class StorageBloomfilterTest extends PHPUnit\Framework\TestCase
{
	public function testNormal()
	{
		if (!class_exists('Phalcon\Storage\Bloomfilter')) {
			$this->markTestSkipped('Class `Phalcon\Storage\Bloomfilter` is not exists');
			return false;
		}
		$filter = new Phalcon\Storage\Bloomfilter('unit-tests/cache/bloomfilter.bin');
		$this->assertFalse($filter->check("Phalcon7"));
		$this->assertTrue($filter->add("Phalcon7"));
		$this->assertTrue($filter->check("Phalcon7"));
		$this->assertTrue($filter->save());
		$this->assertTrue($filter->reset());
		$this->assertFalse($filter->check("Phalcon7"));

		$filter = new Phalcon\Storage\Bloomfilter('unit-tests/cache/bloomfilter.bin');
		$this->assertTrue($filter->check("Phalcon7"));
	}
}
