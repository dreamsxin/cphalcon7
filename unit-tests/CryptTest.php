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
	+------------------------------------------------------------------------+
*/

class CryptTest extends PHPUnit\Framework\TestCase
{

	/**
	 * @requires extension openssl
	 */
	public function testEncryption()
	{
		$tests = array(
			md5(uniqid()) => str_repeat('x', mt_rand(1, 255)),
			time().time() => str_shuffle('abcdefeghijklmnopqrst'),
			'le$ki12432543543543543' => null
		);

		$methods = array('aes-256-ecb', 'aes-256-cbc', 'aes-256-cfb');

		$encrypt = new Phalcon\Crypt();
		foreach ($methods as $method) {
			$encrypt->setMethod($method);

			foreach ($tests as $key => $test) {
				$encrypt->setKey(substr($key, 0, 16));
				$encryption = $encrypt->encrypt($test);
				$this->assertEquals(rtrim($encrypt->decrypt($encryption), "\0"), $test);
			}

			foreach ($tests as $key => $test) {
				$encryption = $encrypt->encrypt($test, substr($key, 0, 16));
				$this->assertEquals(rtrim($encrypt->decrypt($encryption, substr($key, 0, 16)), "\0"), $test);
			}
		}
	}

	/**
	 * @requires extension openssl
	 */
	public function testEncryptBase64()
	{
		$crypt = new \Phalcon\Crypt();

		$key = substr('phalcon notice 13123123', 0, 16);
		$text = 'https://github.com/phalcon/cphalcon/issues?state=open';

		$encrypted = $crypt->encryptBase64($text, $key);
		$actual = $crypt->decryptBase64($encrypted, $key);
		$this->assertEquals($actual, $text);

		$encrypted = $crypt->encryptBase64($text, $key, TRUE);
		$actual = $crypt->decryptBase64($encrypted, $key, TRUE);
		$this->assertEquals($actual, $text);
	}

	/**
	 * @requires extension openssl
	 * @medium
	 */
	public function testPadding()
	{
		$texts = array('');
		$key   = '0123456789ABCDEF0123456789ABCDEF';
		$methods = array('aes-256-ecb', 'aes-256-cbc', 'aes-256-cfb');
		$pads  = array(
			Phalcon\Crypt::PADDING_ANSI_X_923, Phalcon\Crypt::PADDING_PKCS7,
			Phalcon\Crypt::PADDING_ISO_10126, Phalcon\Crypt::PADDING_ISO_IEC_7816_4,
			Phalcon\Crypt::PADDING_ZERO, Phalcon\Crypt::PADDING_SPACE
		);

		for ($i=1; $i<128; ++$i) {
			$texts[] = str_repeat('A', $i);
		}

		$crypt = new Phalcon\Crypt();
		$crypt->setKey(substr($key, 0, 16));

		foreach ($pads as $padding) {
			$crypt->setPadding($padding);
			foreach ($methods as $method) {
				$crypt->setMethod($method);
				foreach ($texts as $text) {
					$encrypted = $crypt->encrypt($text);
					$actual    = $crypt->decrypt($encrypted);

					$this->assertEquals($actual, $text);
				}
			}
		}
	}
}
