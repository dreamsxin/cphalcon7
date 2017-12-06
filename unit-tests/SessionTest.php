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
	+------------------------------------------------------------------------+
*/

class SessionTest extends PHPUnit\Framework\TestCase
{
	public function setUp()
	{
		if (PHP_SESSION_ACTIVE == session_status()) {
			session_destroy();
		}
	}

	public function tearDown()
	{
		@session_destroy();
	}

	public function testSessionFiles()
	{
		$session = new Phalcon\Session\Adapter\Files();

		$this->assertTrue($session->start());
		$this->assertTrue($session->isStarted());

		$session->set('some', 'value');

		$this->assertEquals($session->get('some'), 'value');
		$this->assertTrue($session->has('some'));
		$this->assertEquals($session->get('undefined', 'my-default'), 'my-default');

		// Automatically deleted after reading
		$this->assertEquals($session->get('some', NULL, TRUE), 'value');
		$this->assertFalse($session->has('some'));
	}

	public function testSessionMemcached()
	{
		if (!extension_loaded('memcached')) {
			$this->markTestSkipped('Warning: memcached extension is not loaded');
			return false;
		}
		$session = new Phalcon\Session\Adapter\Memcached(array(
			'servers' => array(
				array(
					'host' => '127.0.0.1',
					'port' => '11211'
				)
			),
			'prefix' => 'memcached'
		));

		$this->assertTrue($session->start());
		$this->assertTrue($session->isStarted());

		$session->set('some', 'value');

		$this->assertEquals($session->get('some'), 'value');
		$this->assertTrue($session->has('some'));
		$this->assertEquals($session->get('undefined', 'my-default'), 'my-default');

		// Automatically deleted after reading
		$this->assertEquals($session->get('some', NULL, TRUE), 'value');
		$this->assertFalse($session->has('some'));
	}

}
