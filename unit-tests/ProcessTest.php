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

class ProcessTest extends PHPUnit\Framework\TestCase
{
	public function testNormal()
	{
		$this->markTestSkipped('Skip');
		return false;
		if (!class_exists('Phalcon\Process\Proc')) {
			$this->markTestSkipped('Class `Phalcon\Process\Proc` is not exists');
			return false;
		}
		$process = new Phalcon\Process\Proc('ping -c 1 localhost');
		$this->assertTrue($process->start());

		$ret = $process->read(Phalcon\Process\Proc::STDOUT);

		$this->assertTrue(!empty($ret));
    }

	public function testHandle()
	{
		$this->markTestSkipped('Skip');
		return false;
		if (!class_exists('Phalcon\Process\Proc')) {
			$this->markTestSkipped('Class `Phalcon\Process\Proc` is not exists');
			return false;
		}
		$process = new Phalcon\Process\Proc('ping -c 1 localhost');
		$this->assertTrue($process->start());
		$ret = $process->handle(function($pipe, $data) {
			// onread
		},function($pipe){
			// onsend
		},function($pipe, $error){
			// onerror
		},function(){
			// ontimeout
		});

		$this->assertTrue($ret);
	}
}
