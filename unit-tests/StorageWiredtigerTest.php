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

class StorageWiredtigerTest extends PHPUnit\Framework\TestCase
{
	public function testNormal()
	{
		if (!class_exists('Phalcon\Storage\Wiredtiger')) {
			$this->markTestSkipped('Class `Phalcon\Storage\Wiredtiger` is not exists');
			return false;
		}
		$db = new Phalcon\Storage\Wiredtiger('unit-tests/cache/wiredtiger');
		$this->assertTrue($db->create('table:phalcon_test'));
		$cursor = $db->open('table:phalcon_test');
		$this->assertTrue($cursor->set("key1", "value1"));
		$this->assertTrue($cursor->set("key2", "value2"));
		$this->assertEquals($cursor->get("key1"), "value1");
		$this->assertEquals($cursor->get("key2"), "value2");
		$this->assertEquals($cursor->gets(array("key1", "key2")), array("value1", "value2"));
		$this->assertTrue($cursor->delete("key1"));
		$this->assertEquals($cursor->get("key1"), NULL);
		foreach ($cursor as $key => $val) {
			$this->assertEquals($key, 'key2');
			$this->assertEquals($val, 'value2');
		}
	}

	public function testArray()
	{
		if (!class_exists('Phalcon\Storage\Wiredtiger')) {
			$this->markTestSkipped('Class `Phalcon\Storage\Wiredtiger` is not exists');
			return false;
		}
		$db = new Phalcon\Storage\Wiredtiger('unit-tests/cache/wiredtiger');
		$this->assertTrue($db->create('table:phalcon_array', 'key_format=iS,value_format=SS'));
		$cursor = $db->open('table:phalcon_array');
		$this->assertTrue($cursor->set(array(1, "key1"), array("val1", "val2")));
		$this->assertTrue($cursor->set(array(2, "key2"), array("val2", "val3")));
		$this->assertEquals($cursor->get(array(1, "key1")), array("val1", "val2"));
		$this->assertEquals($cursor->get(array(2, "key2")), array("val2", "val3"));
	}
}
