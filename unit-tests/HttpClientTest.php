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

class HttpClientTest extends PHPUnit\Framework\TestCase
{
	public function testHeader()
	{
		$header = new Phalcon\Http\Client\Header();
		$header->parse("HTTP/1.0 200 OK\r\nLocation:phalconphp.com");

		$this->assertEquals($header->get('Location'), 'phalconphp.com');
	}

	public function testUri()
	{
		$url = 'http://phalconphp.com/';

		$uri = new Phalcon\Http\Uri($url);
		$this->assertEquals($uri->build(), 'http://phalconphp.com/');

		$uri = $uri->resolve('index');
		$this->assertEquals($uri->build(), 'http://phalconphp.com/index');

		$uri = new Phalcon\Http\Uri('http://myleft.org/one/?x1=1&');
		$this->assertEquals($uri->extend('two?x2=2')->build(), 'http://myleft.org/one/two?x1=1&x2=2');
		$this->assertEquals($uri->extendQuery(['x3' => '3'])->build(), 'http://myleft.org/one/two?x1=1&x2=2&x3=3');
		$this->assertEquals($uri->extendPath('three')->build(), 'http://myleft.org/one/two/three?x1=1&x2=2&x3=3');
		$this->assertEquals($uri->extendPath('/four')->build(), 'http://myleft.org/four?x1=1&x2=2&x3=3');
	}

	public function testCurl()
	{
		$this->markTestSkipped("Test skipped");
		return;
		if (!extension_loaded('curl')) {
			$this->markTestSkipped('Warning: curl extension is not loaded');
			return false;
		}
	
		$client = new Phalcon\Http\Client\Adapter\Curl('http://baidu.com/');

		$response = $client->get();

		$this->assertEquals($response->getStatusCode(), 200);
	}

	public function testStream()
	{
		$this->markTestSkipped("Test skipped");
		return;
		$client = new Phalcon\Http\Client\Adapter\Stream('http://baidu.com/');

		$response = $client->get();

		$this->assertEquals($response->getStatusCode(), 200);
	}

	public function testFactory()
	{
		$this->markTestSkipped("Test skipped");
		return;
		/*
		$client = Phalcon\Http\Client::factory('http://localhost/');

		$response = $client->get();

		$this->assertEquals($response->getStatusCode(), 200);
		*/
	}

	public function testRest()
	{
		$this->markTestSkipped("Test skipped");
		return;
		/*
		$client = Phalcon\Http\Client::factory('http://localhost/');
		$client->setData(json_encode(array('query' => 'pahlcon')), 'application/json');
		$response = $client->get();

		$this->assertEquals($response->getStatusCode(), 200);
		*/
	}

	public function testUpload()
	{
		$this->markTestSkipped("Test skipped");
		return;
		/*
		$client = Phalcon\Http\Client::factory('http://localhost/');
		$client->setData(array(
				new CURLFile('phalcon.jpg')
		));
		// or
		$client->setFiles('phalcon.jpg');
		// or
		$client->setFiles(array('phalcon.jpg'));
		$response = $client->post();
		*/
	}
}
