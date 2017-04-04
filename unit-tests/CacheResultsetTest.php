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

class CacheResultsetTest extends PHPUnit_Framework_TestCase
{

	protected $di;

	public function __construct()
	{
		spl_autoload_register(array($this, 'modelsAutoloader'));
	}

	public function __destruct()
	{
		spl_autoload_unregister(array($this, 'modelsAutoloader'));
	}

	public function modelsAutoloader($className)
	{
		if (file_exists('unit-tests/models/'.$className.'.php')) {
			require 'unit-tests/models/'.$className.'.php';
		}
	}

	protected function _getCache($adapter='File')
	{
		if (!file_exists('unit-tests/cache/')) {
			mkdir("unit-tests/cache/", 0766);
		} else {
			chmod("unit-tests/cache/", 0766);
		}

		@unlink('unit-tests/cache/test-resultset');

		Phalcon\Di::reset();

		$di = new Phalcon\Di();

		$di->set('modelsManager', function(){
			return new Phalcon\Mvc\Model\Manager();
		}, true);

		$di->set('modelsMetadata', function(){
			return new Phalcon\Mvc\Model\Metadata\Memory();
		}, true);

		$di->set('modelsQuery', 'Phalcon\Mvc\Model\Query');
		$di->set('modelsQueryBuilder', 'Phalcon\Mvc\Model\Query\Builder');
		$di->set('modelsCriteria', 'Phalcon\\Mvc\\Model\\Criteria');

		$frontCache = new Phalcon\Cache\Frontend\Data(array(
			'lifetime' => 3600
		));

		switch ($adapter) {
			case 'File':
				$cache = new Phalcon\Cache\Backend\File($frontCache, array(
					'cacheDir' => 'unit-tests/cache/'
				));
				break;
			case 'Memcached':
				$cache = new Phalcon\Cache\Backend\Memcached($frontCache, array(
					"servers" => array(
						array(
							"host" => "localhost",
							"port" => "11211",
							"weight" => "1",
						)
					)
				));
				break;
			default:
				throw new Exception("Unknown cache adapter");
		}

		$di->set('modelsCache', $cache);

		$this->_di = $di;

		return $cache;
	}

	public function testMysqlCacheResultsetNormal()
	{
		require 'unit-tests/config.db.php';
		if (empty($configMysql)) {
			$this->markTestSkipped('Test skipped');
			return;
		}

		$cache = $this->_getCache();

		$this->_di->set('db', function() use ($configMysql) {
			return new Phalcon\Db\Adapter\Pdo\Mysql($configMysql);
		}, true);

		$cache->save('test-resultset', Robots::find(array('order' => 'id')));

		$this->assertTrue(file_exists('unit-tests/cache/test-resultset'));

		$robots = $cache->get('test-resultset');

		$this->assertEquals(get_class($robots), 'Phalcon\Mvc\Model\Resultset\Simple');
		$this->assertEquals(count($robots), 3);
		$this->assertEquals($robots->count(), 3);

	}

	public function testPostgresqlCacheResultsetNormal()
	{
		require 'unit-tests/config.db.php';
		if (empty($configPostgresql)) {
			$this->markTestSkipped('Test skipped');
			return;
		}

		$cache = $this->_getCache();

		$this->_di->set('db', function() use ($configPostgresql) {
			return new Phalcon\Db\Adapter\Pdo\Postgresql($configPostgresql);
		}, true);

		$cache->save('test-resultset', Robots::find(array('order' => 'id')));

		$this->assertTrue(file_exists('unit-tests/cache/test-resultset'));

		$robots = $cache->get('test-resultset');

		$this->assertEquals(get_class($robots), 'Phalcon\Mvc\Model\Resultset\Simple');
		$this->assertEquals(count($robots), 3);
		$this->assertEquals($robots->count(), 3);

	}

	public function testSqliteCacheResultsetNormal()
	{
		require 'unit-tests/config.db.php';
		if (empty($configSqlite)) {
			$this->markTestSkipped('Test skipped');
			return;
		}

		$cache = $this->_getCache();

		$this->_di->set('db', function() use ($configSqlite) {
			return new Phalcon\Db\Adapter\Pdo\Sqlite($configSqlite);
		}, true);

		$cache->save('test-resultset', Robots::find(array('order' => 'id')));

		$this->assertTrue(file_exists('unit-tests/cache/test-resultset'));

		$robots = $cache->get('test-resultset');

		$this->assertEquals(get_class($robots), 'Phalcon\Mvc\Model\Resultset\Simple');
		$this->assertEquals(count($robots), 3);
		$this->assertEquals($robots->count(), 3);

	}

}
