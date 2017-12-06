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

class StorageDatrieTest extends PHPUnit\Framework\TestCase
{
	public function testNormal()
	{
		if (!class_exists('Phalcon\Storage\Datrie')) {
			$this->markTestSkipped('Class `Phalcon\Storage\Datrie` is not exists');
			return false;
		}
		@unlink('unit-tests/cache/datrie.db');
		$datrie = new Phalcon\Storage\Datrie('unit-tests/cache/datrie.db');

		$this->assertTrue($datrie->add("hello", 1));
		$this->assertTrue($datrie->add("word", 2));
		$this->assertEquals($datrie->get("hello"), 1);
		$this->assertEquals($datrie->get("word"), 2);
		$this->assertTrue($datrie->save());
		$this->assertTrue(file_exists('unit-tests/cache/datrie.db'));
		$str = 'hello word!';
		$ret = $datrie->search($str, true);
		$this->assertEquals($ret, array(array(0, 5), array(6, 4)));
		$this->assertTrue($datrie->delete("hello"));
		$ret = $datrie->search($str, true);
		$this->assertEquals($ret, array(array(6, 4)));
	}
}
