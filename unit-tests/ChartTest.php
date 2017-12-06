<?php

/*
	+------------------------------------------------------------------------+
	| Phalcon Framework                                                      |
	+------------------------------------------------------------------------+
	| Copyright (c) 2011-2013 Phalcon Team (http://www.phalconphp.com)       |
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
	|          ZhuZongXin <dreamsxin@qq.com>                         |
	+------------------------------------------------------------------------+
*/

class ChartTest extends PHPUnit\Framework\TestCase
{
	public function testCaptcha()
	{
		if (!class_exists('Phalcon\Chart\Captcha')) {
			$this->markTestSkipped('Class `Phalcon\Chart\Captcha` is not exists');
			return false;
		}
		@unlink('unit-tests/assets/captcha.png');
		$captcha = new \Phalcon\Chart\Captcha('Hello', NULL, 24, 80, 34);
		$ret = $captcha->render('unit-tests/assets/captcha.png', NULL, NULL, NULL, 'red');
		$this->assertTrue(!empty($ret));
		$this->assertTrue(file_exists('unit-tests/assets/captcha.png'));
		@unlink('unit-tests/assets/captcha.png');
		$ret = $captcha->save('unit-tests/assets/captcha.png');
		$this->assertTrue($ret);
		$this->assertTrue(file_exists('unit-tests/assets/captcha.png'));
	}

	public function testTinyCaptcha()
	{
		@unlink('unit-tests/assets/captcha-tiny.gif');
		$tiny = new \Phalcon\Chart\Captcha\Tiny;
		$ret = $tiny->render('unit-tests/assets/captcha-tiny.gif');
		$this->assertTrue(!empty($tiny->getValue()));
		$this->assertTrue(!empty($ret));
		$this->assertTrue(file_exists('unit-tests/assets/captcha-tiny.gif'));

		@unlink('unit-tests/assets/captcha-tiny.gif');
		$tiny = new \Phalcon\Chart\Captcha\Tiny;
		$ret = $tiny->render();
		$this->assertFalse(file_exists('unit-tests/assets/captcha-tiny.gif'));
		$this->assertTrue($tiny->save('unit-tests/assets/captcha-tiny.gif'));
		$this->assertTrue(file_exists('unit-tests/assets/captcha-tiny.gif'));
	}

	public function testQRencode()
	{
		if (!class_exists('Phalcon\Chart\QRcode')) {
			$this->markTestSkipped('Class `Phalcon\Chart\QRcode` is not exists');
			return false;
		}

		@unlink('unit-tests/assets/qr.png');

		$str = 'Phalcon is web framework';

		try {
			$qr = new \Phalcon\Chart\QRcode();
			$ret = $qr->generate($str);
			$this->assertTrue($ret);

			if ($ret) {
				$data = $qr->render();
				$this->assertTrue(strlen($ret) > 0);

				$ret = $qr->save('unit-tests/assets/qr.png');
				$this->assertTrue($ret);

				$ret = $qr->scan('unit-tests/assets/qr.png');
				$this->assertEquals($ret, $str);

				$data = $qr->render(NULL, NULL, 'FFCC00', '000000');
				$this->assertTrue(strlen($ret) > 0);

				$ret = $qr->save('unit-tests/assets/qr.png', NULL, NULL, 'FFCC00', '000000');
				$this->assertTrue($ret);
			}
		} catch (Exception $e) {
		}
	}
}
