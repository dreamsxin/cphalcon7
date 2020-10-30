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

class BeautifulQRCode {

	static public function generate($data, $backgroundColor, $primaryColor, $secondaryColor, $scale = 2, $type = 'png') {

		$count = sqrt(strlen($data));
		$qr = [];
		for ($i=0; $i < $count; $i++) {
			$qr[$i] = unpack("C".$count, substr($data, $count*$i, $count));
		}

		$canvasSize = 500;
		$circleSize = ($canvasSize / ($count + 5));
		$imageX2 = imagecreatetruecolor($canvasSize * $scale, $canvasSize * $scale);
		$bg = self::hexColorAlloc($imageX2, $backgroundColor);
		$secondary = self::hexColorAlloc($imageX2, $secondaryColor);
		imagefill($imageX2, 0, 0, $bg);

		$col_ellipse = self::hexColorAlloc($imageX2, $primaryColor);

		$height = $with = $circleSize * $scale;

		for ($i = 0; $i < $count; $i++) {
			if ($i < 7) {
				for ($j = 7; $j < $count - 7; $j++) {
					if ($qr[$i][$j+1] & 1) {
						$cx = $circleSize * (3 + $j) * $scale;
						$cy = $circleSize * (3 + $i) * $scale;
						imagefilledellipse($imageX2, $cx, $cy, $with, $height, $col_ellipse);
					}
				}
			} elseif ($i >= 7 && $i < $count - 7) {
				for ($j = 0; $j < $count; $j++) {
					if ($qr[$i][$j+1] & 1) {
						$cx = $circleSize * (3 + $j) * $scale;
						$cy = $circleSize * (3 + $i) * $scale;
						imagefilledellipse($imageX2, $cx, $cy, $with, $height, $col_ellipse);
					}
				}
			} else {
				for ($j = 7; $j < $count; $j++) {
					if ($qr[$i][$j+1] & 1) {
						$cx = $circleSize * (3 + $j) * $scale;
						$cy = $circleSize * (3 + $i) * $scale;
						imagefilledellipse($imageX2, $cx, $cy, $with, $height, $col_ellipse);
					}
				}
			}
		}

		self::imagefillroundedrectangle($imageX2, $circleSize * 2.5 * $scale, $circleSize * 2.5 * $scale, $circleSize * 9.5 * $scale, $circleSize * 9.5 * $scale, 25, $col_ellipse);
		self::imagefillroundedrectangle($imageX2, $circleSize * 3.5 * $scale, $circleSize * 3.5 * $scale, $circleSize * 8.5 * $scale, $circleSize * 8.5 * $scale, 25, $bg);
		self::imagefillroundedrectangle($imageX2, $circleSize * 4.5 * $scale, $circleSize * 4.5 * $scale, $circleSize * 7.5 * $scale, $circleSize * 7.5 * $scale, 25, $secondary);

		self::imagefillroundedrectangle($imageX2, $circleSize * ($count - 4.5) * $scale, $circleSize * 2.5 * $scale, $circleSize * ($count + 2.5) * $scale, $circleSize * 9.5 * $scale, 25, $col_ellipse);
		self::imagefillroundedrectangle($imageX2, $circleSize * ($count - 3.5) * $scale, $circleSize * 3.5 * $scale, $circleSize * ($count + 1.5) * $scale, $circleSize * 8.5 * $scale, 25, $bg);
		self::imagefillroundedrectangle($imageX2, $circleSize * ($count - 2.5) * $scale, $circleSize * 4.5 * $scale, $circleSize * ($count + 0.5) * $scale, $circleSize * 7.5 * $scale, 25, $secondary);

		self::imagefillroundedrectangle($imageX2, $circleSize * 2.5 * $scale, $circleSize * ($count - 4.5) * $scale, $circleSize * 9.5 * $scale, $circleSize * ($count + 2.5) * $scale, 25, $col_ellipse);
		self::imagefillroundedrectangle($imageX2, $circleSize * 3.5 * $scale, $circleSize * ($count - 3.5) * $scale, $circleSize * 8.5 * $scale, $circleSize * ($count + 1.5) * $scale, 25, $bg);
		self::imagefillroundedrectangle($imageX2, $circleSize * 4.5 * $scale, $circleSize * ($count - 2.5) * $scale, $circleSize * 7.5 * $scale, $circleSize * ($count + 0.5) * $scale, 25, $secondary);

		$imageOut = imagecreatetruecolor($canvasSize, $canvasSize);
		imagecopyresampled($imageOut, $imageX2, 0, 0, 0, 0, $canvasSize, $canvasSize, $canvasSize * $scale, $canvasSize * $scale);

		ob_start();
		if ($type == 'png') {
			imagepng($imageOut);
		} else {
			imagejpeg($imageOut);
		}
		$imageData = ob_get_clean();

		imagedestroy($imageX2);
		imagedestroy($imageOut);

		return $imageData;
	}

	static public function imagefillroundedrectangle($im, $x, $y, $cx, $cy, $rad, $col) {

		// Draw the middle cross shape of the rectangle
		imagefilledrectangle($im, $x, $y + $rad, $cx, $cy - $rad, $col);
		imagefilledrectangle($im, $x + $rad, $y, $cx - $rad, $cy, $col);

		$dia = $rad * 2;

		// Fill in the rounded corners
		imagefilledellipse($im, $x + $rad, $y + $rad, $rad * 2, $dia, $col);
		imagefilledellipse($im, $x + $rad, $cy - $rad, $rad * 2, $dia, $col);
		imagefilledellipse($im, $cx - $rad, $cy - $rad, $rad * 2, $dia, $col);
		imagefilledellipse($im, $cx - $rad, $y + $rad, $rad * 2, $dia, $col);
	}

	static public function hexColorAlloc($im, $hex) {
		if (strlen($hex) == 1) {
			$hex = str_repeat($hex, 6);
		} elseif (strlen($hex) == 2) {
			$hex = str_repeat($hex, 3);
		} elseif (strlen($hex) == 3) {
			$hex = str_repeat($hex, 2);
		}
		$a = hexdec(substr($hex, 0, 2));
		$b = hexdec(substr($hex, 2, 2));
		$c = hexdec(substr($hex, 4, 2));

		return imagecolorallocate($im, $a, $b, $c);
	}
}

class ChartTest extends PHPUnit\Framework\TestCase
{
	public function testCaptcha()
	{
		if (!class_exists('Imagick')) {
			$this->markTestSkipped('Imagick extension is required');
			return;
		}
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

	public function testBeautifulQRCode()
	{
		if (!function_exists('gd_info')) {
			$this->markTestSkipped('gd extension is required');
			return;
		}

		if (!class_exists('Phalcon\Chart\QRcode')) {
			$this->markTestSkipped('Class `Phalcon\Chart\QRcode` is not exists');
			return false;
		}

		@unlink('unit-tests/assets/qr-beautiful.png');

		$str = 'Phalcon is web framework';

		try {
			$qr = new \Phalcon\Chart\QRcode();
			$ret = $qr->generate($str);

			if ($ret) {
				$data = BeautifulQRCode::generate($qr->getData(), '000000', 'FFCC00', 'B3FF00');
				$this->assertTrue(strlen($data) > 1);
				// file_put_contents('unit-tests/assets/qr-beautiful.png', $data);
			}
		} catch (Exception $e) {
		}
	}
}
