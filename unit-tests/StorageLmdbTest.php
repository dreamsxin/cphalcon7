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

class NoneFrontend implements Phalcon\Storage\FrontendInterface {
	public function beforeStore($value) : string {
		return $value;
	}
	public function afterRetrieve($value) {
		return $value;
	}
}

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
		return;
		// cur
		$db = new Phalcon\Storage\Lmdb('unit-tests/cache/lmdbcur', NULL, NULL, NULL, NULL, Phalcon\Storage\Lmdb::CREATE);

		$db->begin();
		try {
			$db->put('key1', '1');
		} catch (\Exception $e) {
		} finally {
		}

		$cur = $db->cursor();

		foreach ($cur as $key => $v) {
			$this->assertEquals($cur->key(), 'key1');
			$this->assertEquals($cur->current(), '1');

			if ($key == 'key1') {
				$this->assertTrue($cur->put('key1', '0'));
				$this->assertEquals($cur->key(), 'key1');
				$this->assertEquals($cur->current(), '0');
			}
		}

		$db->commit();

		// dup
		$flags = \Phalcon\Storage\Lmdb::CREATE | \Phalcon\Storage\Lmdb::INTEGERKEY | \Phalcon\Storage\Lmdb::INTEGERDUP | \Phalcon\Storage\Lmdb::DUPSORT | \Phalcon\Storage\Lmdb::DUPFIXED;
		$db = new \Phalcon\Storage\Lmdb('unit-tests/cache/lmdbdup', NULL, NULL, NULL, NULL, $flags);

		$db->begin();
		try {
			$db->put('key1', '1', Phalcon\Storage\Lmdb::NODUPDATA);
			$db->put('key1', '2', Phalcon\Storage\Lmdb::NODUPDATA);
			$db->put('key1', '3', Phalcon\Storage\Lmdb::NODUPDATA);
			$db->put('key1', '4', Phalcon\Storage\Lmdb::NODUPDATA);
			$db->put('key2', '1', Phalcon\Storage\Lmdb::NODUPDATA);
			$db->put('key2', '2', Phalcon\Storage\Lmdb::NODUPDATA);
			$db->put('key2', '3', Phalcon\Storage\Lmdb::NODUPDATA);
			$db->put('key2', '4', Phalcon\Storage\Lmdb::NODUPDATA);
			$db->put('key3', '1');
			$db->put('key3', '2');
			$db->put('key3', '3');
			$db->put('key3', '4');
		} catch (\Exception $e) {
			//print_r($e);
		} finally {
			$db->commit();
		}

		$db->begin(\Phalcon\Storage\Lmdb::RDONLY);
		$cur = $db->cursor();

		$cur->valid();
		if ($cur->retrieve('key1')) {
			$this->assertEquals($cur->count(), 4);
			$this->assertEquals($cur->key(), 'key1');
			$this->assertEquals($cur->current(), '1');

			$this->assertTrue($cur->next());
			$this->assertEquals($cur->key(), 'key1');
			$this->assertEquals($cur->current(), '2');

			$this->assertTrue($cur->next(\Phalcon\Storage\Lmdb\Cursor::NEXT_DUP));
			$this->assertEquals($cur->key(), 'key1');
			$this->assertEquals($cur->current(), '3');

			$this->assertTrue($cur->next(\Phalcon\Storage\Lmdb\Cursor::NEXT_DUP));
			$this->assertEquals($cur->key(), 'key1');
			$this->assertEquals($cur->current(), '4');

			$this->assertTrue($cur->next(\Phalcon\Storage\Lmdb\Cursor::NEXT_NODUP));
			$this->assertEquals($cur->key(), 'key2');
			$this->assertEquals($cur->current(), '1');

			$this->assertTrue($cur->next());
			$this->assertEquals($cur->key(), 'key2');
			$this->assertEquals($cur->current(), '2');

			$this->assertTrue($cur->next(\Phalcon\Storage\Lmdb\Cursor::NEXT_NODUP));
			$this->assertEquals($cur->key(), 'key3');
			$this->assertEquals($cur->current(), '1');

			$this->assertTrue($cur->next());
			$this->assertEquals($cur->key(), 'key3');
			$this->assertEquals($cur->current(), '2');
		}

		$db->commit();

		// frontend
		$flags = \Phalcon\Storage\Lmdb::CREATE | \Phalcon\Storage\Lmdb::DUPSORT | \Phalcon\Storage\Lmdb::DUPFIXED;
		$db = new \Phalcon\Storage\Lmdb('unit-tests/cache/lmdbfrontend', NULL, NULL, NULL, NULL, $flags, new NoneFrontend);

		$db->begin();
		try {
			$db->put('key1', '1');
			$db->put('key1', '2');
			$db->put('key1', '3');
			$db->put('key1', '4');
			$db->put('key2', '1');
			$db->put('key2', '2');
			$db->put('key2', '3');
			$db->put('key2', '4');
			$db->put('key3', '1');
			$db->put('key3', '2');
			$db->put('key3', '3');
			$db->put('key3', '4');
		} catch (\Exception $e) {
			//print_r($e);
		} finally {
			$db->commit();
		}

		$db->begin(\Phalcon\Storage\Lmdb::RDONLY);
		$cur = $db->cursor();

		$cur->valid();
		if ($cur->retrieve('key1')) {
			$this->assertEquals($cur->count(), 4);
			$this->assertEquals($cur->key(), 'key1');
			$this->assertEquals($cur->current(), '1');
			$this->assertEquals($cur->current(), '1');

			$this->assertTrue($cur->next());
			$this->assertEquals($cur->key(), 'key1');
			$this->assertEquals($cur->current(), '2');

			$this->assertTrue($cur->next(\Phalcon\Storage\Lmdb\Cursor::NEXT_DUP));
			$this->assertEquals($cur->key(), 'key1');
			$this->assertEquals($cur->current(), '3');

			$this->assertTrue($cur->next(\Phalcon\Storage\Lmdb\Cursor::NEXT_DUP));
			$this->assertEquals($cur->key(), 'key1');
			$this->assertEquals($cur->current(), '4');

			$this->assertEquals($cur->get('key1', \Phalcon\Storage\Lmdb\Cursor::GET_MULTIPLE), '1234');

			$this->assertTrue($cur->next(\Phalcon\Storage\Lmdb\Cursor::NEXT_NODUP));
			$this->assertEquals($cur->key(), 'key2');
			$this->assertEquals($cur->current(), '1');

			$this->assertTrue($cur->next());
			$this->assertEquals($cur->key(), 'key2');
			$this->assertEquals($cur->current(), '2');

			$this->assertTrue($cur->next(\Phalcon\Storage\Lmdb\Cursor::NEXT_NODUP));
			$this->assertEquals($cur->key(), 'key3');
			$this->assertEquals($cur->current(), '1');

			$this->assertTrue($cur->next());
			$this->assertEquals($cur->key(), 'key3');
			$this->assertEquals($cur->current(), '2');
		}

		$db->commit();
	}
}
