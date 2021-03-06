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

class ModelsCriteriaTest extends PHPUnit\Framework\TestCase
{

	public function setUp(): void
	{
		spl_autoload_register(array($this, 'modelsAutoloader'));
	}

	public function tearDown(): void
	{
		spl_autoload_unregister(array($this, 'modelsAutoloader'));
	}

	public function modelsAutoloader($className)
	{
		if (file_exists('unit-tests/models/'.$className.'.php')) {
			require 'unit-tests/models/'.$className.'.php';
		}
	}

	protected function _getDI()
	{

		Phalcon\Di::reset();

		$di = new Phalcon\Di();

		$di->set('modelsManager', function(){
			return new Phalcon\Mvc\Model\Manager();
		});

		$di->set('modelsMetadata', function(){
			return new Phalcon\Mvc\Model\Metadata\Memory();
		});

		$di->set('modelsQuery', 'Phalcon\Mvc\Model\Query');
		$di->set('modelsQueryBuilder', 'Phalcon\Mvc\Model\Query\Builder');
		$di->set('modelsCriteria', 'Phalcon\\Mvc\\Model\\Criteria');

		return $di;
	}

	public function testModelsMysql()
	{
		require 'unit-tests/config.db.php';
		if (empty($configMysql)) {
			$this->markTestSkipped("Skipped");
			return;
		}

		$di = $this->_getDI();

		$di->set('db', function(){
			require 'unit-tests/config.db.php';
			return new Phalcon\Db\Adapter\Pdo\Mysql($configMysql);
		}, true);

		$this->_executeTestsNormal($di);
		$this->_executeTestsRenamed($di);
		$this->_executeTestsFromInput($di);
		$this->_executeTestIssues2131($di);
		$this->_executeJoinTests($di, "mysql");
		$this->_executeTestOther($di);
	}

	public function testModelsPostgresql()
	{
		require 'unit-tests/config.db.php';
		if (empty($configPostgresql)) {
			$this->markTestSkipped("Skipped");
			return;
		}

		$di = $this->_getDI();

		$di->set('db', function(){
			require 'unit-tests/config.db.php';
			return new Phalcon\Db\Adapter\Pdo\Postgresql($configPostgresql);
		}, true);

		$this->_executeTestsNormal($di);
		$this->_executeTestsRenamed($di);
		$this->_executeTestsFromInput($di);
		$this->_executeTestIssues2131($di);
		$this->_executeJoinTests($di, "postgresql");
		$this->_executeTestOther($di);
	}

	public function testModelsSQLite()
	{
		require 'unit-tests/config.db.php';
		if (empty($configSqlite)) {
			$this->markTestSkipped("Skipped");
			return;
		}

		$di = $this->_getDI();

		$di->set('db', function(){
			require 'unit-tests/config.db.php';
			return new Phalcon\Db\Adapter\Pdo\SQLite($configSqlite);
		}, true);

		$this->_executeTestsNormal($di);
		$this->_executeTestsRenamed($di);
		$this->_executeTestsFromInput($di);
		$this->_executeTestIssues2131($di);
		$this->_executeJoinTests($di, "sqlite");
		$this->_executeTestOther($di);
	}

	protected function _executeTestsNormal($di)
	{
		//Where
		$personas = Personas::query()->where("estado='I'")->execute();
		$people = People::find("estado='I'");
		$this->assertEquals(count($personas), count($people));

		$personas = Personas::query()->conditions("estado='I'")->execute();
		$people = People::find("estado='I'");
		$this->assertEquals(count($personas), count($people));

		$personas = Personas::query()
			->where("estado='A'")
			->orderBy("nombres")
			->execute();
		$people = People::find(array(
			"estado='A'",
			"order" => "nombres"
		));
		$this->assertEquals(count($personas), count($people));

		$somePersona = $personas->getFirst();
		$somePeople = $people->getFirst();
		$this->assertEquals($somePersona->cedula, $somePeople->cedula);

		//Where + Order + limit
		$personas = Personas::query()
			->where("estado='A'")
			->orderBy("nombres")
			->limit(100)
			->execute();
		$people = People::find(array(
			"estado='A'",
			"order" => "nombres",
			"limit" => 100
		));
		$this->assertEquals(count($personas), count($people));

		$somePersona = $personas->getFirst();
		$somePeople = $people->getFirst();
		$this->assertEquals($somePersona->cedula, $somePeople->cedula);

		//Where with bind params + order + Limit
		$personas = Personas::query()
			->where("estado=?1")
			->bind(array(1 => "A"))
			->orderBy("nombres")
			->limit(100)
			->execute();

		$people = People::find(array(
			"estado=?1",
			"bind" => array(1 => "A"),
			"order" => "nombres",
			"limit" => 100
		));
		$this->assertEquals(count($personas), count($people));

		$somePersona = $personas->getFirst();
		$somePeople = $people->getFirst();
		$this->assertEquals($somePersona->cedula, $somePeople->cedula);

		//Where with bind params + order + limit + Offset
		$personas = Personas::query()
			->where("estado=?1")
			->bind(array(1 => "A"))
			->orderBy("nombres")
			->limit(100, 10)
			->execute();

		$people = People::find(array(
			"estado=?1",
			"bind" => array(1 => "A"),
			"order" => "nombres",
			"limit" => array('number' => 100, 'offset' => 10)
		));
		$this->assertEquals(count($personas), count($people));

		$somePersona = $personas->getFirst();
		$somePeople = $people->getFirst();
		$this->assertEquals($somePersona->cedula, $somePeople->cedula);

		//Where with named bind params + order + limit 
		$personas = Personas::query()
			->where("estado=:estado:")
			->bind(array("estado" => "A"))
			->orderBy("nombres")
			->limit(100)
			->execute();

		$people = People::find(array(
			"estado=:estado:",
			"bind" => array("estado" => "A"),
			"order" => "nombres",
			"limit" => 100
		));
		$this->assertEquals(count($personas), count($people));

		$somePersona = $personas->getFirst();
		$somePeople = $people->getFirst();
		$this->assertEquals($somePersona->cedula, $somePeople->cedula);
	}

	protected function _executeJoinTests($di, $dbtype)
	{
		//Left join with Simple resultset
		$robotparts = RobotsParts::query()
			->columns("Robots.id, Robots.name, RobotsParts.id robotpart_id")
			->leftJoin("Robots", "Robots.id = RobotsParts.robots_id")
			->execute();
		$this->assertTrue(is_object($robotparts));
		$this->assertInstanceOf('Phalcon\Mvc\Model\Resultset\Simple', $robotparts);
		$this->assertNotNull($robotparts->getFirst()->id);
		$this->assertNotNull($robotparts->getFirst()->name);
		$this->assertNotNull($robotparts->getFirst()->robotpart_id);

		//Two left joins with Simple resultset
		$robotparts = RobotsParts::query()
			->columns("RobotsParts.id, r.id robot_id, p.id part_id")
			->leftJoin("Robots", "r.id = RobotsParts.robots_id", "r")
			->leftJoin("Parts", "p.id = RobotsParts.parts_id", "p")
			->execute();
		$this->assertTrue(is_object($robotparts));
		$this->assertInstanceOf('Phalcon\Mvc\Model\Resultset\Simple', $robotparts);
		$this->assertNotNull($robotparts->getFirst()->id);
		$this->assertNotNull($robotparts->getFirst()->robot_id);
		$this->assertNotNull($robotparts->getFirst()->part_id);

		//Right join not supported in sqlite
		if ($dbtype != "sqlite")
		{
			//Right join with Simple resultset
			$robotparts = RobotsParts::query()
				->columns("Robots.id, Robots.name, RobotsParts.id robotpart_id")
				->rightJoin("Robots", "Robots.id = RobotsParts.robots_id")
				->execute();
			$this->assertTrue(is_object($robotparts));
			$this->assertInstanceOf('Phalcon\Mvc\Model\Resultset\Simple', $robotparts);
			$this->assertNotNull($robotparts->getFirst()->id);
			$this->assertNotNull($robotparts->getFirst()->name);
			$this->assertNotNull($robotparts->getFirst()->robotpart_id);

			//Two right joins with Simple resultset
			$robotparts = RobotsParts::query()
				->columns("RobotsParts.id, r.id robot_id, p.id part_id")
				->rightJoin("Robots", "r.id = RobotsParts.robots_id", "r")
				->rightJoin("Parts", "p.id = RobotsParts.parts_id", "p")
				->execute();
			$this->assertTrue(is_object($robotparts));
			$this->assertInstanceOf('Phalcon\Mvc\Model\Resultset\Simple', $robotparts);
			$this->assertNotNull($robotparts->getFirst()->id);
			$this->assertNotNull($robotparts->getFirst()->robot_id);
			$this->assertNotNull($robotparts->getFirst()->part_id);
		}
	}

	protected function _executeTestsRenamed($di)
	{

		$personers = Personers::query()
			->where("status='I'")
			->execute();
		$this->assertTrue(is_object($personers));
		$this->assertEquals(get_class($personers), 'Phalcon\Mvc\Model\Resultset\Simple');

		$personers = Personers::query()
			->conditions("status='I'")
			->execute();
		$this->assertTrue(is_object($personers));
		$this->assertEquals(get_class($personers), 'Phalcon\Mvc\Model\Resultset\Simple');

		$personers = Personers::query()
			->where("status='A'")
			->orderBy("navnes")
			->execute();
		$this->assertTrue(is_object($personers));
		$this->assertEquals(get_class($personers), 'Phalcon\Mvc\Model\Resultset\Simple');

		$somePersoner = $personers->getFirst();
		$this->assertTrue(is_object($somePersoner));
		$this->assertEquals(get_class($somePersoner), 'Personers');

		$personers  = Personers::query()
			->where("status='A'")
			->orderBy("navnes")
			->limit(100)
			->execute();
		$this->assertTrue(is_object($personers));
		$this->assertEquals(get_class($personers), 'Phalcon\Mvc\Model\Resultset\Simple');

		$somePersoner = $personers->getFirst();
		$this->assertTrue(is_object($somePersoner));
		$this->assertEquals(get_class($somePersoner), 'Personers');

		$personers = Personers::query()
			->where("status=?1")
			->bind(array(1 => "A"))
			->orderBy("navnes")
			->limit(100)
			->execute();
		$this->assertTrue(is_object($personers));
		$this->assertEquals(get_class($personers), 'Phalcon\Mvc\Model\Resultset\Simple');

		$somePersoner = $personers->getFirst();
		$this->assertTrue(is_object($somePersoner));
		$this->assertEquals(get_class($somePersoner), 'Personers');

		$personers = Personers::query()
			->where("status=:status:")
			->bind(array("status" => "A"))
			->orderBy("navnes")
			->limit(100)->execute();
		$this->assertTrue(is_object($personers));
		$this->assertEquals(get_class($personers), 'Phalcon\Mvc\Model\Resultset\Simple');

		$somePersoner = $personers->getFirst();
		$this->assertTrue(is_object($somePersoner));
		$this->assertEquals(get_class($somePersoner), 'Personers');
	}

	protected function _executeTestsFromInput($di)
	{

		$data = array();
		$criteria = \Phalcon\Mvc\Model\Criteria::fromInput($di, "Robots", $data);
		$params = $criteria->getParams();
		$this->assertTrue(empty($params));
		$this->assertEquals($criteria->getModelName(), "Robots");

		$data = array('id' => 1);
		$criteria = \Phalcon\Mvc\Model\Criteria::fromInput($di, "Robots", $data);
		$this->assertEquals($criteria->getParams(), array(
			'conditions' => 'id=:id:',
			'bind' => array(
				'id' => 1,
			),
		));

		$data = array('name' => 'ol');
		$criteria = \Phalcon\Mvc\Model\Criteria::fromInput($di, "Robots", $data);
		$this->assertEquals($criteria->getParams(), array(
			'conditions' => 'name LIKE :name:',
			'bind' => array(
				'name' => '%ol%',
			),
		));

		$data = array('id' => 1, 'name' => 'ol');
		$criteria = \Phalcon\Mvc\Model\Criteria::fromInput($di, "Robots", $data);
		$this->assertEquals($criteria->getParams(), array(
			'conditions' => 'id=:id: AND name LIKE :name:',
			'bind' => array(
				'id' => 1,
				'name' => '%ol%',
			)
		));

		$data = array('id' => 1, 'name' => 'ol', 'other' => true);
		$criteria = \Phalcon\Mvc\Model\Criteria::fromInput($di, "Robots", $data);
		$this->assertEquals($criteria->getParams(), array(
			'conditions' => 'id=:id: AND name LIKE :name:',
			'bind' => array(
				'id' => 1,
				'name' => '%ol%',
			)
		));
	}
	
	public function _executeTestIssues2131($di)
	{
		$di->set('modelsCache', function(){
			$frontCache = new Phalcon\Cache\Frontend\Data();
			$modelsCache = new Phalcon\Cache\Backend\File($frontCache, array(
				'cacheDir' => 'unit-tests/cache/'
			));

			$modelsCache->delete("cache-2131");
			return $modelsCache;
		}, true);

		$personas = Personas::query()->where("estado='I'")->cache(array("key" => "cache-2131"))->execute();
		$this->assertTrue($personas->isFresh());

		$personas = Personas::query()->where("estado='I'")->cache(array("key" => "cache-2131"))->execute();
		$this->assertFalse($personas->isFresh());
	}
	
	public function _executeTestOther($di)
	{
		$personas = Personas::query()->where("estado='X'")->execute();
		$this->assertTrue(count($personas) == Personas::query()->where("estado='X'")->count());
	}

}
