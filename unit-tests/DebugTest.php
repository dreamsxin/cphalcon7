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

class DebugTest extends PHPUnit\Framework\TestCase
{

	public function testNormal()
	{
		$a = 'a';
		$b = 'b';

		$dump = new \Phalcon\Debug\Dump;
		$this->assertEquals($dump->all($a, $b), "<pre style='background-color:#f3f3f3; font-size:11px; padding:10px; border:1px solid #ccc; text-align:left; color:#333'><b style='color:teal'>String</b> (<span style='color:teal'>1</span>) \"<span style='color:teal'>a</span>\"</pre><pre style='background-color:#f3f3f3; font-size:11px; padding:10px; border:1px solid #ccc; text-align:left; color:#333'><b style='color:teal'>String</b> (<span style='color:teal'>1</span>) \"<span style='color:teal'>b</span>\"</pre>");
	}
}
