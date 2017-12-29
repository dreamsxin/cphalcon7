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

class CacheTest extends PHPUnit\Framework\TestCase
{

	public function setUp()
	{
		date_default_timezone_set('UTC');
		if (!file_exists('unit-tests/cache/')) {
			mkdir("unit-tests/cache/", 0766);
		} else {
			chmod("unit-tests/cache/", 0766);
		}

		$iterator = new DirectoryIterator('unit-tests/cache/');
		foreach ($iterator as $item) {
			if (!$item->isDir()) {
				unlink($item->getPathname());
			}
		}
	}

	public function testOutputFileCache()
	{

		$time = date('H:i:s');

		$frontCache = new Phalcon\Cache\Frontend\Output(array(
			'lifetime' => 2
		));

		$cache = new Phalcon\Cache\Backend\File($frontCache, array(
			'cacheDir' => 'unit-tests/cache/',
			'prefix' => 'unit'
		));

		$this->assertFalse($cache->isStarted());

		ob_start();

		//First time cache
		$content = $cache->start('testoutput');
		$this->assertTrue($cache->isStarted());

		if ($content !== null) {
			$this->assertTrue(false);
		}

		echo $time;
		$cache->save();

		$obContent = ob_get_contents();
		ob_end_clean();

		$this->assertEquals($time, $obContent);
		$this->assertTrue(file_exists('unit-tests/cache/unittestoutput'));

		//Same cache
		$content = $cache->start('testoutput');
		$this->assertTrue($cache->isStarted());

		if ($content === null) {
			$this->assertTrue(false);
		}

		$this->assertEquals($time, $obContent);

		//Refresh cache
		sleep(3);

		$time2 = date('H:i:s');

		ob_start();

		$content = $cache->start('testoutput');
		$this->assertTrue($cache->isStarted());

		if ($content !== null) {
			$this->assertTrue(false);
		}

		echo $time2;
		$cache->save();

		$obContent2 = ob_get_contents();
		ob_end_clean();

		$this->assertNotEquals($time, $obContent2);
		$this->assertEquals($time2, $obContent2);

		//Check keys
		$keys = $cache->queryKeys();
		$this->assertEquals($keys, array(
			0 => 'unittestoutput',
		));

		// $cache->exists('testoutput') is not always true because Travis CI could be slow sometimes
		//Exists?
		if ($cache->exists('testoutput')) {
			$this->assertTrue($cache->exists('testoutput'));

			//Delete cache
			$this->assertTrue($cache->delete('testoutput'));
			$keys = $cache->queryKeys();
			$this->assertEquals($keys, array());
		}

	}

	public function testDataFileCache()
	{

		$frontCache = new Phalcon\Cache\Frontend\Data(array('lifetime' => 10));

		$cache = new Phalcon\Cache\Backend\File($frontCache, array(
			'cacheDir' => 'unit-tests/cache/',
		));

		$this->assertFalse($cache->isStarted());

		//Save
		$cache->save('test-data', "nothing interesting");

		$this->assertTrue(file_exists('unit-tests/cache/test-data'));

		//Get
		$cachedContent = $cache->get('test-data');
		$this->assertEquals($cachedContent, "nothing interesting");

		//Save
		$cache->save('test-data', "sure, nothing interesting");

		//Get
		$cachedContent = $cache->get('test-data');
		$this->assertEquals($cachedContent, "sure, nothing interesting");

		//Exists
		$this->assertTrue($cache->exists('test-data'));

		//Delete
		$this->assertTrue($cache->delete('test-data'));

	}

	public function testDataFileCacheIncrement()
	{
		$frontCache = new Phalcon\Cache\Frontend\Data();

		$cache = new Phalcon\Cache\Backend\File($frontCache, array(
			'cacheDir' => 'unit-tests/cache/'
		));
		$cache->delete('foo');
		$cache->save('foo', "1");
		$this->assertEquals(2, $cache->increment('foo'));

		$this->assertEquals($cache->get('foo'), 2);

		$this->assertEquals($cache->increment('foo', 5), 7);
	}

	public function testDataFileCacheDecrement()
	{
		$frontCache = new Phalcon\Cache\Frontend\Data();

		$cache = new Phalcon\Cache\Backend\File($frontCache, array(
			'cacheDir' => 'unit-tests/cache/'
		));
		$cache->delete('foo');
		$cache->save('foo', "100");
		$this->assertEquals(99, $cache->decrement('foo'));

		$this->assertEquals(95, $cache->decrement('foo', 4));
	}

	public function testMemoryCache()
	{
		$frontCache = new Phalcon\Cache\Frontend\None(array('lifetime' => 10));

		$cache = new Phalcon\Cache\Backend\Memory($frontCache, array('prefix' => 'unit'));

		$this->assertFalse($cache->isStarted());

		//Save
		$cache->save('test-data', "nothing interesting");

		//Get
		$cachedContent = $cache->get('test-data');
		$this->assertEquals($cachedContent, "nothing interesting");

		//Save
		$cache->save('test-data', "sure, nothing interesting");

		//Get
		$cachedContent = $cache->get('test-data');
		$this->assertEquals($cachedContent, "sure, nothing interesting");

		//Exists
		$this->assertTrue($cache->exists('test-data'));

		//Delete
		$this->assertTrue($cache->delete('test-data'));

		$string = str_repeat('a', 5000000);
		$r1 = memory_get_usage();
		$cache->save('test-data', $string);
		$r2 = memory_get_usage();
		$s1 = $cache->get('test-data');
		$r3 = memory_get_usage();
		$s2 = $cache->get('test-data');
		$r4 = memory_get_usage();
		$s3 = $cache->get('test-data');
		$r5 = memory_get_usage();
		//echo $r1, ' ', $r2, ' ', $r3, ' ', $r4, ' ', $r5, "\n";
		$this->assertEquals($s1, $s2);
		$this->assertEquals($s1, $s3);
		$this->assertEquals(strlen($s1), 5000000);
		$this->assertEquals($s1, $string);
		$this->assertTrue($cache->delete('test-data'));

		unset($s1, $s2, $s3);
		gc_collect_cycles();
		$r1 = memory_get_usage();
		$s1 = $frontCache->afterRetrieve($string);
		$r2 = memory_get_usage();
		$s2 = $frontCache->afterRetrieve($string);
		$r3 = memory_get_usage();
		$s3 = $frontCache->afterRetrieve($string);
		$r4 = memory_get_usage();
		$this->assertEquals($s1, $s2);
		$this->assertEquals($s1, $s3);
		$this->assertEquals($s1, $string);
		//echo $r1, ' ', $r2, ' ', $r3, ' ', $r4, "\n";
	}

	public function testMemoryCacheIncrAndDecr()
	{
		$frontCache = new Phalcon\Cache\Frontend\Output(array(
			'lifetime' => 2
		));

		$cache = new Phalcon\Cache\Backend\Memory($frontCache, array('prefix' => 'unit'));
		$cache->delete('foo');

		$cache->save('foo', 20);

		$this->assertEquals('21', $cache->increment('foo'));
		$this->assertEquals('24', $cache->increment('foo', 3));

		$this->assertEquals('23', $cache->decrement('foo'));
		$this->assertEquals('3', $cache->decrement('foo', 20));

		$this->assertEquals(3, $cache->get('foo'));
	}

	private function _prepareIgbinary()
	{

		if (!extension_loaded('igbinary')) {
			$this->markTestSkipped('Warning: igbinary extension is not loaded');
			return false;
		}

		return true;
	}

	public function testIgbinaryFileCache()
	{
		if (!$this->_prepareIgbinary()) {
			return false;
		}

		$frontCache = new Phalcon\Cache\Frontend\Igbinary(array('lifetime' => 600));

		$cache = new Phalcon\Cache\Backend\File($frontCache, array(
			'cacheDir' => 'unit-tests/cache/',
 			'prefix' => 'unit'
		));

		$this->assertFalse($cache->isStarted());

		//Save
		$cache->save('test-data', "nothing interesting");

		$this->assertTrue(file_exists('unit-tests/cache/test-data'));

		//Get
		$cachedContent = $cache->get('test-data');
		$this->assertEquals($cachedContent, "nothing interesting");

		//Save
		$cache->save('test-data', "sure, nothing interesting");

		//Get
		$cachedContent = $cache->get('test-data');
		$this->assertEquals($cachedContent, "sure, nothing interesting");

		//More complex save/get
		$data = array(
			'null'   => null,
			'array'  => array(1, 2, 3, 4 => 5),
			'string',
			123.45,
			6,
			true,
			false,
			null,
			0,
			""
		);

		$serialized = igbinary_serialize($data);
		$this->assertEquals($data, igbinary_unserialize($serialized));

		$cache->save('test-data', $data);
		$cachedContent = $cache->get('test-data');

		$this->assertEquals($cachedContent, $data);

		//Exists
		$this->assertTrue($cache->exists('test-data'));

		//Delete
		$this->assertTrue($cache->delete('test-data'));

	}

	protected function _prepareApc()
	{

		if (!function_exists('apc_fetch')) {
			$this->markTestSkipped('apc extension is not loaded');
			return false;
		}

		if (ini_get('apc.enable_cli') != 1) {
			$this->markTestSkipped('apc.enable_cli must be set to 1');
			return false;
		}

		return true;
	}

	public function testApcIncrement()
	{
		$ready = $this->_prepareApc();
		if (!$ready) {
			return false;
		}

		$frontCache = new Phalcon\Cache\Frontend\Data(array('lifetime' => 20));
		$cache = new Phalcon\Cache\Backend\Apc($frontCache);
		$cache->delete('increment');

		$cache->save('increment', 1);
		$this->assertEquals(2, $cache->increment('increment'));
		$this->assertEquals(4, $cache->increment('increment', 2));
		$this->assertEquals(14, $cache->increment('increment', 10));
	}

	public function testApcDecrement()
	{
		$ready = $this->_prepareApc();
		if (!$ready) {
			return false;
		}

		$frontCache = new Phalcon\Cache\Frontend\Data(array('lifetime' => 20));
		$cache = new Phalcon\Cache\Backend\Apc($frontCache);
		$cache->delete('decrement');

		$cache->save('decrement', 100);
		$this->assertEquals(99, $cache->decrement('decrement'));
		$this->assertEquals(97, $cache->decrement('decrement', 2));
		$this->assertEquals(87, $cache->decrement('decrement', 10));
	}

	public function testOutputApcCache()
	{
		$ready = $this->_prepareApc();
		if (!$ready) {
			return false;
		}

		apc_delete('_PHCAtest-output');

		$time = date('H:i:s');

		$frontCache = new Phalcon\Cache\Frontend\Output(array(
			'lifetime' => 2
		));

		$cache = new Phalcon\Cache\Backend\Apc($frontCache);

		ob_start();

		//First time cache
		$content = $cache->start('test-output');
		if ($content !== null) {
			$this->assertTrue(false);
		}

		echo $time;

		$cache->save();

		$obContent = ob_get_contents();
		ob_end_clean();

		$this->assertEquals($time, $obContent);
		$this->assertEquals($time, apc_fetch('_PHCAtest-output'));

		//Expect same cache
		$content = $cache->start('test-output');
		if ($content === null) {
			$this->assertTrue(false);
		}

		$this->assertEquals($content, $obContent);
		$this->assertEquals($content, apc_fetch('_PHCAtest-output'));

		//Query keys
		$keys = $cache->queryKeys();
		$this->assertEquals($keys, array(
			0 => 'test-output',
			1 => 'decrement',
			2 => 'increment'
		));

		//Delete entry from cache
		$this->assertTrue($cache->delete('test-output'));

		$keys = $cache->queryKeys();
		$this->assertEquals($keys, array(
			1 => 'decrement',
			2 => 'increment'
		));
	}

	public function testDataApcCache()
	{
		$ready = $this->_prepareApc();
		if (!$ready) {
			return false;
		}

		apc_delete('_PHCAtest-data');

		$frontCache = new Phalcon\Cache\Frontend\Data();

		$cache = new Phalcon\Cache\Backend\Apc($frontCache);

		$data = array(1, 2, 3, 4, 5);

		$cache->save('test-data', $data);

		$cachedContent = $cache->get('test-data');
		$this->assertEquals($cachedContent, $data);

		$cache->save('test-data', "sure, nothing interesting");

		$cachedContent = $cache->get('test-data');
		$this->assertEquals($cachedContent, "sure, nothing interesting");

		$this->assertTrue($cache->delete('test-data'));

		$cache->save('a', 1);
		$cache->save('long-key', 'long-val');
		$cache->save('bcd', 3);
		$keys = $cache->queryKeys();
		sort($keys);
		$this->assertEquals($keys, array('a', 'bcd', 'decrement', 'increment', 'long-key'));
		$this->assertEquals($cache->queryKeys('long'), array('long-key'));

		$this->assertTrue($cache->delete('a'));
		$keys = $cache->queryKeys();
		sort($keys);
		$this->assertEquals($keys, array('bcd', 'decrement', 'increment', 'long-key'));
		$this->assertTrue($cache->delete('long-key'));
		$this->assertTrue($cache->delete('bcd'));
		$keys = $cache->queryKeys();
		sort($keys);
		$this->assertEquals($keys, array('decrement', 'increment'));
	}

	protected function _prepareMongo()
	{
		if (!class_exists('Phalcon\Cache\Backend\Mongo')) {
			$this->markTestSkipped('Class `Phalcon\Cache\Backend\Mongo` is not exists');
			return false;
		}

		return true;
	}

	public function testOutputMongoCache()
	{
		$ready = $this->_prepareMongo();
		if (!$ready) {
			return false;
		}

		$time = date('H:i:s');

		$frontCache = new Phalcon\Cache\Frontend\Output(array(
			'lifetime' => 2
		));

		$cache = new Phalcon\Cache\Backend\Mongo($frontCache, array(
			'db' => 'phalcon_test',
			'collection' => 'caches'
		));
		$this->assertTrue($cache->flush());
		ob_start();

		//First time cache
		$content = $cache->start('test-output');
		$this->assertTrue($content === null);

		echo $time;

		$cache->save();

		$obContent = ob_get_contents();
		ob_end_clean();

		$this->assertEquals($time, $obContent);

		$data = $cache->get('test-output');
		$this->assertEquals($time, $data);

		//Expect same cache
		$content = $cache->start('test-output');
		$this->assertFalse($content === null);

		$data = $cache->get('test-output');
		$this->assertEquals($time, $data);

		//Query keys
		$keys = $cache->queryKeys();
		$this->assertEquals($keys, array(
			0 => 'test-output',
		));

		//Exists
		$this->assertTrue($cache->exists('test-output'));

		//Delete entry from cache
		$this->assertTrue($cache->delete('test-output'));
		$keys = $cache->queryKeys();
		$this->assertEquals($keys, array());
	}

	public function testDataMongoCache()
	{
		$ready = $this->_prepareMongo();
		if (!$ready) {
			return false;
		}

		// Travis can be slow, especially when Valgrind is used
		$frontCache = new Phalcon\Cache\Frontend\Data(array('lifetime' => 900));

		$cache = new Phalcon\Cache\Backend\Mongo($frontCache, array(
			'db' => 'phalcon_test',
			'collection' => 'caches'
		));
		$this->assertTrue($cache->flush());

		$data = array(1, 2, 3, 4, 5);

		$cache->save('test-data', $data);

		$cachedContent = $cache->get('test-data');
		$this->assertEquals($cachedContent, $data);

		$cache->save('test-data', "sure, nothing interesting");

		$cachedContent = $cache->get('test-data');
		$this->assertEquals($cachedContent, "sure, nothing interesting");

		//Exists
		$this->assertTrue($cache->exists('test-data'));

		$this->assertTrue($cache->delete('test-data'));
	}

	public function testMongoIncrement()
	{
		$ready = $this->_prepareMongo();
		if (!$ready) {
			return false;
		}

		$frontCache = new Phalcon\Cache\Frontend\Data(array('lifetime' => 200));

		$cache = new Phalcon\Cache\Backend\Mongo($frontCache, array(
			'db' => 'phalcon_test',
			'collection' => 'caches'
		));
		$cache->delete('foo');
		$cache->save('foo', 1);
		$this->assertEquals(1, $cache->get('foo'));

		$this->assertEquals(2, $cache->increment('foo'));
		$this->assertEquals(4, $cache->increment('foo', 2));
		$this->assertEquals(4, $cache->get('foo'));

		$this->assertEquals(14, $cache->increment('foo', 10));
	}

	public function testMongoDecrement()
	{
		$ready = $this->_prepareMongo();
		if (!$ready) {
			return false;
		}

		$frontCache = new Phalcon\Cache\Frontend\Data(array('lifetime' => 200));

		$cache = new Phalcon\Cache\Backend\Mongo($frontCache, array(
			'db' => 'phalcon_test',
			'collection' => 'caches'
		));
		$cache->delete('foo');
		$cache->save('foo', 100);

		$this->assertEquals(99, $cache->decrement('foo'));
		$this->assertEquals(89, $cache->decrement('foo', 10));
		$this->assertEquals(89, $cache->get('foo'));
		$this->assertEquals(1, $cache->decrement('foo', 88));
	}

	private function _prepareMemcached()
	{

		if (!extension_loaded('memcached')) {
			$this->markTestSkipped('Warning: memcached extension is not loaded');
			return false;
		}

		$memcache = new Memcached();
		$this->assertFalse(!$memcache->addServers(array(array('127.0.0.1', 11211, 1))));
		$memcache->flush();
		sleep(1);

		return $memcache;
	}

	public function testOutputMemcachedCache()
	{

		$memcache = $this->_prepareMemcached();
		if (!$memcache) {
			return false;
		}

		$memcache->delete('test-output');

		$time = date('H:i:s');

		$frontCache = new Phalcon\Cache\Frontend\Output(array(
			'lifetime' => 2
		));

		$cache = new Phalcon\Cache\Backend\Memcached($frontCache);

		ob_start();

		//First time cache
		$content = $cache->start('test-output');
		if ($content !== null) {
			$this->assertTrue(false);
		}

		echo $time;

		$cache->save();

		$obContent = ob_get_contents();
		ob_end_clean();

		$this->assertEquals($time, $obContent);
		$this->assertEquals($time, $memcache->get('test-output'));

		//Expect same cache
		$content = $cache->start('test-output');
		if ($content === null) {
			$this->assertTrue(false);
		}

		$this->assertEquals($time, $obContent);

		//Refresh cache
		sleep(3);

		$time2 = date('H:i:s');

		ob_start();

		$content = $cache->start('test-output');
		if($content!==null){
			$this->assertTrue(false);
		}
		echo $time2;
		$cache->save();

		$obContent2 = ob_get_contents();
		ob_end_clean();

		$this->assertNotEquals($time, $obContent2);
		$this->assertEquals($time2, $obContent2);
		$this->assertEquals($time2, $memcache->get('test-output'));

		//Check if exists
		$this->assertTrue($cache->exists('test-output'));

		//Delete entry from cache
		$this->assertTrue($cache->delete('test-output'));
	}

	public function testMemcachedIncrement() {
		$memcache = $this->_prepareMemcached();
		if (!$memcache) {
			return false;
		}

		// Travis can be slow, especially when Valgrind is used
		$frontCache = new Phalcon\Cache\Frontend\Data(array('lifetime' => 900));

		$cache = new Phalcon\Cache\Backend\Memcached($frontCache, array(
			'servers' => array(
				array(
					'host' => '127.0.0.1',
					'port' => '11211',
					'weight' => '1'),
			),
			'prefix' => 'unit'
		));

		$cache->delete('foo');
		$cache->save('foo', 1);
		$this->assertEquals(2, $cache->increment('foo'));
		$this->assertEquals(4, $cache->increment('foo', 2));
		$cache->increment('foo', 10);
		$this->assertEquals(14, $cache->get('foo'));
		$cache->delete('foo');
	}

	public function testMemcachedDecrement() {
		$memcache = $this->_prepareMemcached();
		if (!$memcache) {
			return false;
		}

		// Travis can be slow, especially when Valgrind is used
		$frontCache = new Phalcon\Cache\Frontend\Data(array('lifetime' => 900));

		$cache = new Phalcon\Cache\Backend\Memcached($frontCache, array(
			'servers' => array(
				array(
					'host' => '127.0.0.1',
					'port' => '11211',
					'weight' => '1'),
			),
			'prefix' => 'unit'
		));
		$cache->delete('foo');
		$cache->save('foo', 100);
		$this->assertEquals(99, $cache->decrement('foo'));
		$this->assertEquals(97, $cache->decrement('foo', 2));
		$cache->decrement('foo', 10);
		$this->assertEquals(87, $cache->get('foo'));
		$cache->delete('foo');
	}

	public function testDataMemcachedCache()
	{

		$memcache = $this->_prepareMemcached();
		if (!$memcache) {
			return false;
		}

		$memcache->delete('test-data');

		// Travis can be slow, especially when Valgrind is used
		$frontCache = new Phalcon\Cache\Frontend\Data(array('lifetime' => 900));

		$cache = new Phalcon\Cache\Backend\Memcached($frontCache, array(
			'servers' => array(
				array(
					'host' => '127.0.0.1',
					'port' => '11211',
					'weight' => '1'
				),
			),
			'prefix' => 'unit'
		));

		$keys = $cache->queryKeys();
		foreach ($keys as $key) {
			$cache->delete($key);
		}

		$data = array(1, 2, 3, 4, 5);

		$cache->save('test-data', $data);

		$cachedContent = $cache->get('test-data');
		$this->assertEquals($cachedContent, $data);

		$cache->save('test-data', "sure, nothing interesting");

		$cachedContent = $cache->get('test-data');
		$this->assertEquals($cachedContent, "sure, nothing interesting");

		$actual = $cache->queryKeys();
		$this->assertEquals($actual, array(0 => 'unittest-data'));

		//Check if exists
		$this->assertTrue($cache->exists('test-data'));

		//Delete
		$this->assertTrue($cache->delete('test-data'));
		$keys = $cache->queryKeys();
		$this->assertEquals($keys, array());
	}

	public function testDataMemcachedCacheOption()
	{
		$memcache = $this->_prepareMemcached();
		if (!$memcache) {
			return false;
		}

		$memcache->delete('test-data');

		// Travis can be slow, especially when Valgrind is used
		$frontCache = new Phalcon\Cache\Frontend\Data(array('lifetime' => 900));

		//Memcached OPT_PREFIX_KEY: prefix.
		$cache = new Phalcon\Cache\Backend\Memcached($frontCache, array(
			'servers' => array(
				array(
					'host' => '127.0.0.1',
					'port' => '11211',
					'weight' => '1'),
			),
			'client' => array(
				Memcached::OPT_PREFIX_KEY => 'prefix.',
			),
		));

		$data = array(1, 2, 3, 4, 5);

		$cache->save('test-data', $data);

		$cachedContent = $cache->get('test-data');
		$this->assertEquals($cachedContent, $data);

		$cachedContent = $memcache->get('test-data');
		//$this->assertNotEquals($cachedContent, $data);
		$this->assertFalse($cachedContent);

		$cachedContent = $memcache->get('prefix.test-data');
		$cachedUnserialize = unserialize($cachedContent);
		$this->assertEquals($cachedUnserialize, $data);

        //Memcached Option None
		$cache2 = new Phalcon\Cache\Backend\Memcached($frontCache, array(
			'servers' => array(
				array(
					'host' => '127.0.0.1',
					'port' => '11211',
					'weight' => '1'),
			),
			'client' => array(),
		));

		$cachedContent = $cache2->get('test-data');
		$this->assertNotEquals($cachedContent, $data);

		$cache2->save('test-data', "sure, nothing interesting");

		$cachedContent = $cache2->get('test-data');
		$this->assertEquals($cachedContent, "sure, nothing interesting");

		$cachedContent = $memcache->get('test-data');
		$cachedUnserialize = unserialize($cachedContent);
		$this->assertEquals($cachedUnserialize, "sure, nothing interesting");

		$cachedContent = $memcache->get('prefix.test-data');
		$cachedUnserialize = unserialize($cachedContent);
		$this->assertNotEquals($cachedUnserialize, "sure, nothing interesting");
		$this->assertEquals($cachedUnserialize, $data);

		//Delete
		$this->assertTrue($cache->delete('test-data'));
		$this->assertTrue($cache2->delete('test-data'));
	}

	protected function _prepareRedis()
	{

		if (!extension_loaded('redis')) {
			$this->markTestSkipped('Warning: redis extension is not loaded');
			return false;
		}

		$redis = new Redis();
		$redis->connect('127.0.0.1', 6379);
		//$redis->flushDB();
		return $redis;
	}

	public function testRedisIncrement()
	{
		$redis = $this->_prepareRedis();
		if (!$redis) {
			return false;
		}

		$frontCache = new Phalcon\Cache\Frontend\Data(array('lifetime' => 20));
		$cache = new Phalcon\Cache\Backend\Redis($frontCache, array(
			'host' => 'localhost',
			'port' => 6379,
			'prefix' => 'unit',
			// 'auth' => 'foobared',
			// 'persistent' => false
		));
		$cache->delete('increment');

		$cache->save('increment', 1);
		$this->assertEquals(2, $cache->increment('increment'));
		$this->assertEquals(4, $cache->increment('increment', 2));
		$this->assertEquals(14, $cache->increment('increment', 10));
	}

	public function testRedisDecrement()
	{
		$ready = $this->_prepareRedis();
		if (!$ready) {
			return false;
		}

		$frontCache = new Phalcon\Cache\Frontend\Data(array('lifetime' => 20));
		$cache = new Phalcon\Cache\Backend\Redis($frontCache, array(
			'host' => 'localhost',
			'port' => 6379,
			'prefix' => 'unit'
		));
		$cache->delete('decrement');

		$cache->save('decrement', 100);
		$this->assertEquals(99, $cache->decrement('decrement'));
		$this->assertEquals(97, $cache->decrement('decrement', 2));
		$this->assertEquals(87, $cache->decrement('decrement', 10));
	}

	public function testOutputRedisCache()
	{

		$redis = $this->_prepareRedis();
		if (!$redis) {
			return false;
		}

		$redis->delete('_PHCRtest-output');

		$time = date('H:i:s');

		$frontCache = new Phalcon\Cache\Frontend\Output(array('lifetime' => 2));
		$cache = new Phalcon\Cache\Backend\Redis($frontCache, array(
			'host' => 'localhost',
			'port' => 6379,
			'prefix' => 'unit'
		));

		ob_start();

		//First time cache
		$content = $cache->start('test-output');
		if ($content !== null) {
			$this->assertTrue(false);
		}

		echo $time;

		$cache->save();

		$obContent = ob_get_contents();
		ob_end_clean();

		$this->assertEquals($time, $obContent);
		$this->assertEquals($time, $redis->get('_PHCRtest-output'));

		//Expect same cache
		$content = $cache->start('test-output');
		if ($content === null) {
			$this->assertTrue(false);
		}

		$this->assertEquals($content, $obContent);
		$this->assertEquals($content, $redis->get('_PHCRtest-output'));

		//Query keys
		$keys = $cache->queryKeys();
		sort($keys);
		$this->assertEquals($keys, array(
			0 => 'decrement',
			1 => 'increment',
			2 => 'test-output'
		));

		//Delete entry from cache
		$this->assertTrue($cache->delete('test-output'));
		$keys = $cache->queryKeys();
		sort($keys);
		$this->assertEquals($keys, array(
			0 => 'decrement',
			1 => 'increment',
		));
	}

	public function testDataRedisCache()
	{
		$redis = $this->_prepareRedis();
		if (!$redis) {
			return false;
		}

		$redis->delete('test-data');

		$frontCache = new Phalcon\Cache\Frontend\Data();
		$cache = new Phalcon\Cache\Backend\Redis($frontCache, array(
			'host' => 'localhost',
			'port' => 6379,
			'prefix' => 'unit'
		));

		$data = array(1, 2, 3, 4, 5);

		$cache->save('test-data', $data);

		$cachedContent = $cache->get('test-data');
		$this->assertEquals($cachedContent, $data);

		$cache->save('test-data', "sure, nothing interesting");

		$cachedContent = $cache->get('test-data');
		$this->assertEquals($cachedContent, "sure, nothing interesting");

		$this->assertTrue($cache->delete('test-data'));

		$cache->save('a', 1);
		$cache->save('long-key', 'long-val');
		$cache->save('bcd', 3);
		$keys = $cache->queryKeys();
		sort($keys);
		$this->assertEquals($keys, array('a', 'bcd', 'decrement', 'increment', 'long-key'));
		$this->assertEquals($cache->queryKeys('long'), array('long-key'));

		$this->assertTrue($cache->delete('a'));
		$this->assertTrue($cache->delete('long-key'));
		$this->assertTrue($cache->delete('bcd'));
		$keys = $cache->queryKeys();
		sort($keys);
		$this->assertEquals($keys, array('decrement', 'increment'));
	}

	public function testCacheFileFlush()
	{
		$frontCache = new Phalcon\Cache\Frontend\Data(array('lifetime' => 10));

		// File
		$cache = new Phalcon\Cache\Backend\File($frontCache, array(
			'cacheDir' => 'unit-tests/cache/',
			'prefix' => 'unit'
		));

		$this->assertTrue($cache->flush());

		$cache->save('data', "1");
		$cache->save('data2', "2");
		$keys = $cache->queryKeys();
		sort($keys);
		$this->assertEquals($keys, array('unitdata', 'unitdata2'));

		$this->assertTrue($cache->flush());

		$this->assertEquals($cache->queryKeys(), array());

		$this->assertFalse(file_exists('unit-tests/cache/data'));
		$this->assertFalse(file_exists('unit-tests/cache/data2'));
	}

	public function testCacheMemoryFlush()
	{
		$frontCache = new Phalcon\Cache\Frontend\Data(array('lifetime' => 10));

		// Memory
		$cache = new Phalcon\Cache\Backend\Memory($frontCache);

		$this->assertTrue($cache->flush());

		$cache->save('data', "1");
		$cache->save('data2', "2");

		$this->assertEquals($cache->queryKeys(), array('data', 'data2'));

		$this->assertTrue($cache->flush());

		$this->assertEquals($cache->queryKeys(), array());

		$this->assertFalse($cache->exists('data'));
		$this->assertFalse($cache->exists('data2'));
	}

	public function testCacheApcFlush()
	{
		$frontCache = new Phalcon\Cache\Frontend\Data(array('lifetime' => 10));

		// Apc
		$ready = $this->_prepareApc();
		if (!$ready) {
			return false;
		}

		$cache = new Phalcon\Cache\Backend\Apc($frontCache);

		$this->assertTrue($cache->flush());

		$cache->save('data', "1");
		$cache->save('data2', "2");

		$this->assertEquals($cache->queryKeys(), array('data', 'data2'));

		$this->assertTrue($cache->flush());

		$this->assertEquals($cache->queryKeys(), array());

		$this->assertFalse($cache->exists('data'));
		$this->assertFalse($cache->exists('data2'));
	}

	public function testCacheMongoFlush()
	{
		// Mongo
		$ready = $this->_prepareMongo();
		if (!$ready) {
			return false;
		}

		$frontCache = new Phalcon\Cache\Frontend\Data();
		$cache = new Phalcon\Cache\Backend\Mongo($frontCache, array(
			'db' => 'phalcon_test',
			'collection' => 'caches'
		));

		$this->assertTrue($cache->flush());

		$cache->save('data', "1");
		$cache->save('data2', "2");

		$this->assertEquals($cache->queryKeys(), array('data', 'data2'));

		$this->assertTrue($cache->flush());

		$this->assertEquals($cache->queryKeys(), array());

		$this->assertFalse($cache->exists('data'));
		$this->assertFalse($cache->exists('data2'));
	}

	public function testCacheMemcachedFlush()
	{
		if (!extension_loaded('memcached')) {
			$this->markTestSkipped('Warning: memcached extension is not loaded');
			return false;
		}

		$frontCache = new Phalcon\Cache\Frontend\Data();

        //Memcached OPT_PREFIX_KEY: prefix.
		$cache = new Phalcon\Cache\Backend\Memcached($frontCache, array(
			'servers' => array(
				array(
					'host' => '127.0.0.1',
					'port' => '11211',
					'weight' => '1'),
			),
			'client' => array(
				Memcached::OPT_PREFIX_KEY => 'prefix.',
			)
		));

		$this->assertTrue($cache->flush());

		$cache->save('data', "1");
		$cache->save('data2', "2");

		$this->assertEquals($cache->queryKeys(), array('data', 'data2'));

		$this->assertTrue($cache->flush());

		$this->assertEquals($cache->queryKeys(), array());

		$this->assertFalse($cache->exists('data'));
		$this->assertFalse($cache->exists('data2'));
	}

	public function testCacheRedisFlush()
	{
		$redis = $this->_prepareRedis();
		if (!$redis) {
			return false;
		}

		$frontCache = new Phalcon\Cache\Frontend\Data();
		$cache = new Phalcon\Cache\Backend\Redis($frontCache, array(
			'host' => 'localhost',
			'port' => 6379
		));

		$this->assertTrue($cache->flush());

		$cache->save('data', "1");
		$cache->save('data2', "2");

		$this->assertEquals($cache->queryKeys(), array('data', 'data2'));

		$this->assertTrue($cache->flush());

		$this->assertEquals($cache->queryKeys(), array());

		$this->assertFalse($cache->exists('data'));
		$this->assertFalse($cache->exists('data2'));
	}

	public function testYacIncrement()
	{
		if (!class_exists('Phalcon\Cache\Yac')) {
			$this->markTestSkipped('Class `Phalcon\Cache\Yac` is not exists');
			return false;
		}
		if (!ini_get('phalcon.cache.enable_yac_cli')) {
			$this->markTestSkipped('Warning: phalcon.cache.enable_yac_cli is not enbale');
			return false;
		}

		$frontCache = new Phalcon\Cache\Frontend\Data(array('lifetime' => 20));
		$cache = new Phalcon\Cache\Backend\Yac($frontCache, array(
			'prefix' => 'unit'
		));

		$cache->delete('increment');

		$cache->save('increment', 1);
		$this->assertEquals(2, $cache->increment('increment'));
		$this->assertEquals(4, $cache->increment('increment', 2));
		$this->assertEquals(14, $cache->increment('increment', 10));
	}

	public function testYacDecrement()
	{
		if (!class_exists('Phalcon\Cache\Yac')) {
			$this->markTestSkipped('Class `Phalcon\Cache\Yac` is not exists');
			return false;
		}
		if (!ini_get('phalcon.cache.enable_yac_cli')) {
			$this->markTestSkipped('Warning: phalcon.cache.enable_yac_cli is not enbale');
			return false;
		}

		$frontCache = new Phalcon\Cache\Frontend\Data(array('lifetime' => 20));
		$cache = new Phalcon\Cache\Backend\Yac($frontCache, array(
			'prefix' => 'unit'
		));

		$cache->delete('decrement');

		$cache->save('decrement', 100);
		$this->assertEquals(99, $cache->decrement('decrement'));
		$this->assertEquals(97, $cache->decrement('decrement', 2));
		$this->assertEquals(87, $cache->decrement('decrement', 10));
	}

	public function testDataYacCache()
	{
		if (!class_exists('Phalcon\Cache\Yac')) {
			$this->markTestSkipped('Class `Phalcon\Cache\Yac` is not exists');
			return false;
		}
		if (!ini_get('phalcon.cache.enable_yac_cli')) {
			$this->markTestSkipped('Warning: phalcon.cache.enable_yac_cli is not enbale');
			return false;
		}

		$frontCache = new Phalcon\Cache\Frontend\Data();
		$cache = new Phalcon\Cache\Backend\Yac($frontCache, array(
			'prefix' => 'unit'
		));

		$data = array(1, 2, 3, 4, 5);

		$cache->save('test-data', $data);

		$cachedContent = $cache->get('test-data');
		$this->assertEquals($cachedContent, $data);

		$cache->save('test-data', "sure, nothing interesting");

		$cachedContent = $cache->get('test-data');
		$this->assertEquals($cachedContent, "sure, nothing interesting");

		$this->assertTrue($cache->delete('test-data'));

		$cache->save('a', 1);
		$cache->save('long-key', 'long-val');
		$cache->save('bcd', 3);
		$keys = $cache->queryKeys();
		sort($keys);
		$this->assertEquals($keys, array('a', 'bcd', 'decrement', 'increment', 'long-key'));
		$this->assertEquals($cache->queryKeys('long'), array('long-key'));

		$this->assertTrue($cache->delete('a'));
		$this->assertTrue($cache->delete('long-key'));
		$this->assertTrue($cache->delete('bcd'));
		$keys = $cache->queryKeys();
		sort($keys);
		$this->assertEquals($keys, array('decrement', 'increment'));
	}

	public function testCacheYacFlush()
	{
		if (!class_exists('Phalcon\Cache\Yac')) {
			$this->markTestSkipped('Class `Phalcon\Cache\Yac` is not exists');
			return false;
		}
		if (!ini_get('phalcon.cache.enable_yac_cli')) {
			$this->markTestSkipped('Warning: phalcon.cache.enable_yac_cli is not enbale');
			return false;
		}

		$frontCache = new Phalcon\Cache\Frontend\Data();
		$cache = new Phalcon\Cache\Backend\Yac($frontCache, array(
			'prefix' => 'unit'
		));

		$this->assertTrue($cache->flush());

		$cache->save('data', "1");
		$cache->save('data2', "2");

		$this->assertEquals($cache->queryKeys(), array('data', 'data2'));

		$this->assertTrue($cache->flush());

		$this->assertEquals($cache->queryKeys(), array());

		$this->assertFalse($cache->exists('data'));
		$this->assertFalse($cache->exists('data2'));
	}

	public function testCacheYac()
	{
		if (!class_exists('Phalcon\Cache\Yac')) {
			$this->markTestSkipped('Class `Phalcon\Cache\Yac` is not exists');
			return false;
		}
		if (!ini_get('phalcon.cache.enable_yac_cli')) {
			$this->markTestSkipped('Warning: phalcon.cache.enable_yac_cli is not enbale');
			return false;
		}

		$cache = new Phalcon\Cache\Yac();

		$key = "foo";
		$value = "dummy";

		$this->assertTrue($cache->set($key, $value));
		$this->assertEquals($cache->get($key), $value);

		$value = NULL;
		$this->assertTrue($cache->set($key, $value));
		$this->assertEquals($cache->get($key), $value);

		$value = TRUE;
		$this->assertTrue($cache->set($key, $value));
		$this->assertEquals($cache->get($key), $value);

		$value = FALSE;
		$this->assertTrue($cache->set($key, $value));
		$this->assertEquals($cache->get($key), $value);

		$value = range(1, 5);
		$this->assertTrue($cache->set($key, $value));
		$this->assertEquals($cache->get($key), $value);

		$value = 9234324;
		$this->assertTrue($cache->set($key, $value));
		$this->assertEquals($cache->get($key), $value);

		$value = 9234324.123456;
		$this->assertTrue($cache->set($key, $value));
		$this->assertEquals($cache->get($key), $value);

		$value = new StdClass();
		$cache->set($key, $value);
		$this->assertEquals($cache->get($key), $value);

		try {
			$value = fopen("php://input", "r");
			$cache->set($key, $value);
		} catch (Exception $e) {
			$this->assertEquals("Phalcon\Cache\Yac::set(): Type 'IS_RESOURCE' cannot be stored", $e->getMessage());
		}


		$value = range(1, 5);
		$this->assertTrue($cache->set($key, $value));
		$this->assertEquals($cache->get($key), $value);

		$this->assertTrue($cache->delete($key));
		$this->assertEquals($cache->get($key), NULL);
	}

	public function testWiredTiger()
	{
		if (!class_exists('Phalcon\Cache\Backend\Wiredtiger')) {
			$this->markTestSkipped('Class `Phalcon\Cache\Backend\Wiredtiger` is not exists');
			return false;
		}

		$frontCache = new Phalcon\Cache\Frontend\Data(array('lifetime' => 20));
		$cache = new Phalcon\Cache\Backend\Wiredtiger($frontCache, array(
			'home' => __DIR__.'/cache/wiredtiger',
			'table' => 'cache',
		));

		$cache->delete('increment');
		$cache->save('increment', 1);
		$this->assertEquals(2, $cache->increment('increment'));
		$this->assertEquals(4, $cache->increment('increment', 2));
		$this->assertEquals(14, $cache->increment('increment', 10));

		$cache->delete('decrement');
		$cache->save('decrement', 100);
		$this->assertEquals(99, $cache->decrement('decrement'));
		$this->assertEquals(97, $cache->decrement('decrement', 2));
		$this->assertEquals(87, $cache->decrement('decrement', 10));

		$data = array(1, 2, 3, 4, 5);

		$cache->save('test-data', $data);

		$cachedContent = $cache->get('test-data');
		$this->assertEquals($cachedContent, $data);

		$cache->save('test-data', "sure, nothing interesting");

		$cachedContent = $cache->get('test-data');
		$this->assertEquals($cachedContent, "sure, nothing interesting");

		$this->assertTrue($cache->delete('test-data'));

		$cache->save('a', 1);
		$cache->save('long-key', 'long-val');
		$cache->save('bcd', 3);
		$keys = $cache->queryKeys();
		sort($keys);
		$this->assertEquals($keys, array('a', 'bcd', 'decrement', 'increment', 'long-key'));
		$this->assertEquals($cache->queryKeys('long'), array('long-key'));

		$this->assertTrue($cache->delete('a'));
		$this->assertTrue($cache->delete('long-key'));
		$this->assertTrue($cache->delete('bcd'));
		$keys = $cache->queryKeys();
		sort($keys);
		$this->assertEquals($keys, array('decrement', 'increment'));

		$this->assertTrue($cache->flush());

		$cache->save('data', "1");
		$cache->save('data2', "2");

		$this->assertEquals($cache->queryKeys(), array('data', 'data2'));

		$this->assertTrue($cache->flush());

		$this->assertEquals($cache->queryKeys(), array());

		$this->assertFalse($cache->exists('data'));
		$this->assertFalse($cache->exists('data2'));
	}
}
