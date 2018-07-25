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

class XhprofTest extends PHPUnit\Framework\TestCase
{
	private function four() {
		return __FUNCTION__.PHP_EOL;
	}

	private function tree() {
		$this->four();
		return __FUNCTION__.PHP_EOL;
	}

	private function two() {
		$this->tree();
		return __FUNCTION__.PHP_EOL;
	}

	private function one() {
		$this->two();
		$this->two();
		return __FUNCTION__.PHP_EOL;
	}

	public function testNormal()
	{
		$this->markTestSkipped("Skipped");
		return;
		Phalcon\Xhprof::enable(Phalcon\Xhprof::FLAG_MEMORY | Phalcon\Xhprof::FLAG_CPU);

		$this->one();

		$data = Phalcon\Xhprof::disable();

		$calls = $data['XhprofTest::one==>XhprofTest::two']["calls"];
		$this->assertEquals($calls, 2);
	}
}
