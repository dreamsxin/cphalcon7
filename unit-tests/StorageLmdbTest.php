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

class StorageLmdbTest extends PHPUnit\Framework\TestCase
{
	public function testNormal()
	{
		if (!class_exists('Phalcon\Storage\Lmdb')) {
			$this->markTestSkipped('Class `Phalcon\Storage\Lmdb` is not exists');
			return false;
		}
		$db = new Phalcon\Storage\Lmdb('unit-tests/cache/lmdb');
		$db->begin();
		$this->assertTrue($db->put('key1', 'value1'));
		$this->assertTrue($db->put('key2', 'value2'));
		$this->assertEquals($db->get("key1"), "value1");
		$this->assertEquals($db->get("key2"), "value2");
		$this->assertEquals($db->getAll(), array('key1' => 'value1', 'key2' => 'value2'));

		$ret = [];
		foreach($db->cursor() as $key => $value) {
			$ret[$key] = $value;
		}
		$this->assertEquals($ret, ['key1' => 'value1', 'key2' => 'value2']);
		$db->commit();
	}
}
