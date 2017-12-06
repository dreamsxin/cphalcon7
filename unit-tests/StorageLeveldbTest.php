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

class StorageLeveldbTest extends PHPUnit\Framework\TestCase
{
	public function testNormal()
	{
		if (!class_exists('Phalcon\Storage\Leveldb')) {
			$this->markTestSkipped('Class `Phalcon\Storage\Leveldb` is not exists');
			return false;
		}

		$db = new Phalcon\Storage\Leveldb('unit-tests/cache/leveldb');
		$this->assertTrue($db->put('key1', 'value1'));
		$this->assertTrue($db->put('key2', 'value2'));
		$this->assertTrue($db->put('key4', 'value4'));
		$this->assertEquals($db->get("key1"), "value1");
		$this->assertEquals($db->get("key2"), "value2");
		$this->assertTrue($db->delete("key1"));
		$this->assertFalse($db->get("key1"));
		
		$batch = new Phalcon\Storage\Leveldb\Writebatch();
		$this->assertTrue($batch->put("key3", "value3"));
		$this->assertTrue($batch->delete("key4"));
		$this->assertTrue($db->write($batch));

		$ret = [];
		foreach($db->iterator() as $key => $value) {
			$ret[$key] = $value;
		}
		$this->assertEquals($ret, ['key2' => 'value2', 'key3' => 'value3']);
	}
}
