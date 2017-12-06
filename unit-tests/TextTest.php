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

class TextTest extends PHPUnit\Framework\TestCase
{

	public function testNormal()
	{
		$this->assertEquals(Phalcon\Text::camelize('coco_bongo'), 'CocoBongo');
		$this->assertEquals(Phalcon\Text::uncamelize('CocoBongo'), 'coco_bongo');

		$this->assertTrue(strlen(Phalcon\Text::random(Phalcon\Text::RANDOM_ALNUM)) == 8);
		$this->assertTrue(strlen(Phalcon\Text::random(Phalcon\Text::RANDOM_ALNUM, 10)) == 10);

		$this->assertTrue(Phalcon\Text::startsWith("Hello", "He"));
		$this->assertTrue(Phalcon\Text::endsWith("Hello", "llo"));

		$this->assertEquals(Phalcon\Text::lower('Abc'), 'abc');
		$this->assertEquals(Phalcon\Text::upper('Abc'), 'ABC');

		$this->assertEquals(Phalcon\Text::bytes(1025), '1.02 KB');
		$this->assertEquals(Phalcon\Text::bytes(1025, 'MB'), '0.00 MB');
		$this->assertEquals(Phalcon\Text::bytes(1025, NULL, NULL, FALSE), '1.00 KiB');

		$this->assertEquals(Phalcon\Text::reduceSlashes("foo//bar/baz"), 'foo/bar/baz');

		$this->assertEquals(Phalcon\Text::concat("/", "/tmp/", "/folder_1/", "/folder_2", "folder_3/"), '/tmp/folder_1/folder_2/folder_3/');

		$this->assertEquals(Phalcon\Text::underscore('look behind'), 'look_behind');

		$this->assertEquals(Phalcon\Text::humanize('start-a-horse'), 'start a horse');

		$str = 'phalcon7';

		$this->assertEquals(Phalcon\Text::increment($str), 'phalcon7_1');
		$this->assertEquals(Phalcon\Text::decrement($str), 'phalcon7_1');

		$this->assertEquals(Phalcon\Text::increment($str, 1), 'phalcon8');
		$this->assertEquals(Phalcon\Text::decrement($str, 1), 'phalcon6');

		$str = '012345678';

		$this->assertEquals(Phalcon\Text::increment($str), '012345678_1');
		$this->assertEquals(Phalcon\Text::decrement($str), '012345678_1');

		$this->assertEquals(Phalcon\Text::increment($str, 1), '012345679');
		$this->assertEquals(Phalcon\Text::decrement($str, 1), '012345677');
	}
}
