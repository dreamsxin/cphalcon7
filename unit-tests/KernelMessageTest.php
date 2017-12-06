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

class KernelMessageTest extends PHPUnit\Framework\TestCase
{
	public function testMessages()
	{
		Phalcon\Kernel::setBasePath("unit-tests/");
		$this->assertEquals(Phalcon\Kernel::message('validation', 'Alnum'), "字段 :field 只能包含字母和数字");
		Phalcon\Di::reset();
		$di = new \Phalcon\Di();
		$di->set('translate', function(){
			$t = new \Phalcon\Translate\Adapter\Php(array(
				'locale' => 'validation',
				'directory' => __DIR__ . DIRECTORY_SEPARATOR . 'locale'
			));
			return $t;
		});
		$this->assertEquals(Phalcon\Kernel::message('validation', 'Url'), "字段 :field 必须是 url");
	}

	public function testMessagesDir()
	{
		Phalcon\Kernel::setBasePath("unit-tests/");
		Phalcon\Kernel::setMessagesDir("locale/");
		$this->assertEquals(Phalcon\Kernel::message('validation', 'Field :field must be a url'), "字段 :field 必须是 url");
	}
}
