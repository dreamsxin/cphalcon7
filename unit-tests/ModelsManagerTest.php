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

class ModelsManagerTest extends PHPUnit\Framework\TestCase
{

	public function setUp()
	{
		spl_autoload_register(array($this, 'modelsAutoloader'));
	}

	public function tearDown()
	{
		spl_autoload_unregister(array($this, 'modelsAutoloader'));
	}

	public function modelsAutoloader($className)
	{
		$className = str_replace("\\", DIRECTORY_SEPARATOR, $className);
		$path = 'unit-tests/models/'.$className.'.php';
		if (file_exists($path)) {
			require $path;
		}
	}

	protected function _getDI($dbService)
	{

		Phalcon\Di::reset();

		$di = new Phalcon\Di();

		$di->set('modelsManager', function(){
			$manager = new Phalcon\Mvc\Model\Manager();
			$manager->registerNamespaceAlias('s', 'Some');
			return $manager;
		});

		$di->set('db', $dbService, true);

		return $di;
	}

	public function testModelsMysql()
	{
		require 'unit-tests/config.db.php';
		if (empty($configMysql)) {
			$this->markTestSkipped("Skipped");
			return;
		}

		$di = $this->_getDI(function(){
			require 'unit-tests/config.db.php';
			$db = new Phalcon\Db\Adapter\Pdo\Mysql($configMysql);
			return $db;
		});

		$this->_executeTestsNormal($di);
	}

	public function testModelsPostgresql()
	{
		require 'unit-tests/config.db.php';
		if (empty($configPostgresql)) {
			$this->markTestSkipped("Skipped");
			return;
		}

		$di = $this->_getDI(function(){
			require 'unit-tests/config.db.php';
			return new Phalcon\Db\Adapter\Pdo\Postgresql($configPostgresql);
		});

		$this->_executeTestsNormal($di);
	}

	public function testModelsSqlite()
	{
		require 'unit-tests/config.db.php';
		if (empty($configSqlite)) {
			$this->markTestSkipped("Skipped");
			return;
		}

		$di = $this->_getDI(function(){
			require 'unit-tests/config.db.php';
			return new Phalcon\Db\Adapter\Pdo\Sqlite($configSqlite);
		});

		$this->_executeTestsNormal($di);
	}

	protected function _executeTestsNormal($di)
	{
		$expected = array(
			'models' => array(
				's:Robots',
			),
			'tables' => array(
				'robots',
			),
			'columns' => array(
				array(
					'type' => 'object',
					'model' => 's:Robots',
					'column' => 'robots',
				),
			),
		);
		$query = new Phalcon\Mvc\Model\Query('SELECT * FROM [s:Robots]');
		$query->setDI($di);
		$this->assertEquals($query->parse(), $expected);
	}
}
