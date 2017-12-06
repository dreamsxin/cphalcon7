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

class StorageBtreeTest extends PHPUnit\Framework\TestCase
{
	public function testNormal()
	{
		if (!class_exists('Phalcon\Storage\Btree')) {
			$this->markTestSkipped('Class `Phalcon\Storage\Btree` is not exists');
			return false;
		}
		$btree = new Phalcon\Storage\Btree('unit-tests/cache/tree.db');
		$this->assertTrue($btree->set("key1", "value1"));
		$this->assertEquals($btree->get("key1"), "value1");
		$this->assertTrue($btree->delete("key1"));
		$this->assertEquals($btree->get("key1"), "");
	}
}
