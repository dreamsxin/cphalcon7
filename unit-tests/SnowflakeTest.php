<?php

/*
	+------------------------------------------------------------------------+
	| Phalcon Framework                                                      |
	+------------------------------------------------------------------------+
	| Copyright (c) 2011-2014 Phalcon Team (http://www.phalconphp.com)       |
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

class SnowflakeTest extends PHPUnit\Framework\TestCase
{
	public function test()
	{
		$this->markTestSkipped("Skipped");
		return;
		$snowflake = new Phalcon\Snowflake;
		$id = $snowflake->nextId();
		$desc = $snowflake->parse($id);

		$this->assertTrue(is_array($desc));
	}
}
