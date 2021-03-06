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
  |          Piotr Gasiorowski <p.gasiorowski@vipserv.org>                                  |
  +------------------------------------------------------------------------+
*/

use Phalcon\Di;
use Phalcon\Mvc\View\Exception;
use Phalcon\Mvc\View\Simple as View;
use Phalcon\Cache\Frontend\Output as FrontendCache;
use Phalcon\Cache\Backend\File as BackendCache;
use Phalcon\Cache\BackendInterface;

class ViewSimpleTest extends PHPUnit\Framework\TestCase
{
	private $level;

	public function setUp(): void
	{
		$this->level = ob_get_level();
		foreach (new DirectoryIterator('unit-tests/cache/') as $item) {
			$item->isDir() or unlink($item->getPathname());
		}
	}

	// Setup viewCache service and DI
	private function _getDI()
	{
		$di = new Di;
		$frontendCache = new FrontendCache(array('lifetime' => 2));
		$backendCache = new BackendCache($frontendCache, array('cacheDir' => 'unit-tests/cache/'));
		$di->set('viewCache', $backendCache);
		return $di;
	}

	public function tearDown(): void
	{
		foreach (new DirectoryIterator('unit-tests/cache/') as $item)
		{
			$item->isDir() or unlink($item->getPathname());
		}

		while (ob_get_level() > $this->level) {
			ob_end_flush();
		}
	}

	public function testSettersAndGetters()
	{
		$view = new View();

		$view->foo = 'bar';
		$this->assertEquals('bar', $view->foo);

		$this->assertEquals($view, $view->setVar('foo1', 'bar1'));
		$this->assertEquals('bar1', $view->getVar('foo1'));

		$expectedVars = array('foo2' => 'bar2', 'foo3' => 'bar3');
		$this->assertEquals($view, $view->setVars($expectedVars));
		$this->assertEquals('bar2', $view->foo2);
		$this->assertEquals('bar3', $view->foo3);
		$this->assertEquals($view, $view->setVars($expectedVars, false));

		$this->assertEquals($view, $view->setParamToView('foo4', 'bar4'));

		$expectedParamsToView = array('foo2' => 'bar2', 'foo3' => 'bar3', 'foo4' => 'bar4');
		$this->assertEquals($expectedParamsToView, $view->getParamsToView());

		$this->assertEquals($view, $view->setContent('<h1>hello</h1>'));
		$this->assertEquals('<h1>hello</h1>', $view->getContent());

		$view->setViewsDir('unit-tests/views/');
		$this->assertEquals('unit-tests/views/', $view->getViewsDir());

		$expectedCacheOptions = array("lifetime" => 86400, "key" => "simple-cache");
		$this->assertEmpty($view->getCacheOptions());
		$this->assertEquals($view, $view->setCacheOptions($expectedCacheOptions));
		$this->assertEquals($expectedCacheOptions, $view->getCacheOptions());
	}

	public function testMissingView()
	{
		try {
			$view = new View;
			$view->setViewsDir('unit-tests/views/');
			$view->render('test1/index');
			$this->assertTrue(false);
		} catch (\Phalcon\Mvc\View\Exception $e) {
			$this->assertTrue(true);
		}
	}

	public function testRenderStandard()
	{
		$view = new View;
		$view->setViewsDir(array('unit-tests/views/test2/', 'unit-tests/views/test3/'));

		$this->assertEquals('here', $view->render('index'));
		$this->assertEquals('there', $view->render('index2'));
	}

	public function testRenderWithVariables()
	{
		$view = new View;
		$view->setViewsDir('unit-tests/views/');

		$this->assertEquals('here', $view->render('test3/other'));

		$view->setParamToView('a_cool_var', 'le-this');
		$this->assertEquals('<p>le-this</p>', $view->render('test3/another'));
	}

	public function testRenderWithRegisteredEngine()
	{
		$view = new View;
		$view->setDI(new Di);
		$view->setViewsDir('unit-tests/views/');
		$view->setParamToView('name', 'FooBar');
		$view->registerEngines(array('.mhtml' => 'Phalcon\\Mvc\\View\\Engine\\Php'));

		$this->assertEquals('Hello FooBar', $view->render('test4/index'));
	}

	public function testRenderWithPartials()
	{
		$view = new View;
		$view->setViewsDir('unit-tests/views/');
		$expectedParams = array('cool_var' => 'FooBar');

		ob_start();
		$view->partial('partials/_partial1', $expectedParams);
		ob_clean();
		$this->assertEquals('Hey, this is a partial, also FooBar', $view->getContent());

		$view->setVars($expectedParams);
		$this->assertEquals('Hey, this is a partial, also FooBar', $view->render('test5/index'));
		$this->assertEquals('Hey, this is a partial, also FooBar<br />Hey, this is a second partial, also FooBar', $view->render('test9/index'));
	}

	public function testRenderWithCache()
	{
		// Create cache at first run
		$view = new View;
		$view->setViewsDir('unit-tests/views/');

		$this->assertFalse($view->getCache()); // No cache before DI is set
		$view->setDI($this->_getDI());
		$this->assertEquals($view, $view->cache(array('key' => 'view_simple_cache')));

		$cache = $view->getCache();
		$this->assertInstanceOf('Phalcon\Cache\BackendInterface', $cache);
		if (($cache instanceof BackendInterface) == false)
			$this->fail('Expected BackendInterface');

		$timeNow = time();
		$view->setParamToView('a_cool_var', $timeNow);
		$this->assertEquals("<p>$timeNow</p>", $view->render('test3/coolVar'));
		unset($view, $cache);

		// Re-use the cached contents
		$view = new View;
		$view->setViewsDir('unit-tests/views/');
		$view->setDI($this->_getDI());
		$view->cache(array('key' => 'view_simple_cache'));
		$this->assertEquals("<p>$timeNow</p>", $view->render('test3/coolVar'));
		sleep(3);
		// Cache should expire
		$this->assertEquals("<p></p>", @$view->render('test3/coolVar'));
	}

	public function testMultiDir()
	{
		$view = new View;
		$view->setViewsDir(array('unit-tests/views/test2/', 'unit-tests/views/test3/'));

		$this->assertEquals('there', $view->render('index2'));
		$this->assertEquals('there', $view->getContent());
	}

	public function testSection()
	{
		$view = new View;
		$view->setViewsDir('unit-tests/views/section/');

		$this->assertEquals('<span>one</span><span>two</span><span>three</span>', $view->render('index'));
	}
}
