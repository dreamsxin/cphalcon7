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
  | If you did not receive a copy of the licnse and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@phalconphp.com so we can send you a copy immediately.       |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@phalconphp.com>                      |
  |          Eduar Carvajal <eduar@phalconphp.com>                         |
  +------------------------------------------------------------------------+
*/

use Phalcon\Mvc\Model\Query\Builder as Builder;
use Phalcon\Mvc\Model\Query\Builder\Select as SelectBuilder;

class ModelsQuerySelectBuilderTest extends PHPUnit\Framework\TestCase
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

	protected function _getDI()
	{

		Phalcon\Di::reset();

		$di = new Phalcon\Di();

		$di->set('modelsManager', function(){
			return new Phalcon\Mvc\Model\Manager();
		});

		$di->set('modelsMetadata', function(){
			return new Phalcon\Mvc\Model\Metadata\Memory();
		}, true);

		return $di;
	}

	public function testExecuteMysql()
	{
		require 'unit-tests/config.db.php';
		if (empty($configMysql)) {
			$this->markTestSkipped("Skipped");
			return;
		}

		$di = $this->_getDI();

		$di->set('db', function() {
			require 'unit-tests/config.db.php';
			return new Phalcon\Db\Adapter\Pdo\Mysql($configMysql);
		}, true);

		$this->_testSelectBuilder($di);
		$this->_testUpdateBuilder($di);
		$this->_testInsertBuilder($di);
		$this->_testDeleteBuilder($di);
		$this->_testIssue701($di);
		$this->_testIssue1115($di);
		$this->_testSelectDistinctAll($di);
		$this->_testConstructor($di);
		$this->_testConstructorLimit($di);
		$this->_testConstructorConditions($di);
		$this->_testGroup($di);
	}

	public function testExecuteSqlite()
	{
		require 'unit-tests/config.db.php';
		if (empty($configSqlite)) {
			$this->markTestSkipped("Skipped");
			return;
		}

		$di = $this->_getDI();

		$di->set('db', function() {
			require 'unit-tests/config.db.php';
			return new Phalcon\Db\Adapter\Pdo\Sqlite($configSqlite);
		}, true);

		$this->_testSelectBuilder($di);
		$this->_testUpdateBuilder($di);
		$this->_testInsertBuilder($di);
		$this->_testDeleteBuilder($di);
		$this->_testIssue701($di);
		$this->_testIssue1115($di);
		$this->_testSelectDistinctAll($di);
		$this->_testConstructor($di);
		$this->_testConstructorLimit($di);
		$this->_testConstructorConditions($di);
		$this->_testGroup($di);
	}

	public function _testSelectBuilder($di)
	{
		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots')
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].* FROM [Robots]');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from(array('Robots', 'RobotsParts'))
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].*, [RobotsParts].* FROM [Robots], [RobotsParts]');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->columns('*')
						->from('Robots')
						->getPhql();
		$this->assertEquals($phql, 'SELECT * FROM [Robots]');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->columns(array('id', 'name'))
						->from('Robots')
						->getPhql();
		$this->assertEquals($phql, 'SELECT id, name FROM [Robots]');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->columns('id')
						->from('Robots')
						->getPhql();
		$this->assertEquals($phql, 'SELECT id FROM [Robots]');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots')
						->where('Robots.name = "Voltron"')
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].* FROM [Robots] WHERE Robots.name = "Voltron"');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots')
						->where('Robots.name = "Voltron"')
						->andWhere('Robots.id > 100')
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].* FROM [Robots] WHERE (Robots.name = "Voltron") AND (Robots.id > 100)');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots')
						->where('Robots.name = "Voltron"')
						->andWhere('Robots.id > 100', NULL, NULL, FALSE)
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].* FROM [Robots] WHERE Robots.name = "Voltron" AND Robots.id > 100');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots')
						->where('Robots.name = "Voltron"')
						->orWhere('Robots.id > 100')
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].* FROM [Robots] WHERE (Robots.name = "Voltron") OR (Robots.id > 100)');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots')
						->where('Robots.name = "Voltron"')
						->orWhere('Robots.id > 100', NULL, NULL, FALSE)
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].* FROM [Robots] WHERE Robots.name = "Voltron" OR Robots.id > 100');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots')
						->where(100)
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].* FROM [Robots] WHERE [Robots].[id] = 100');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots')
						->groupBy('Robots.name')
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].* FROM [Robots] GROUP BY Robots.name');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots')
						->groupBy(array('Robots.name', 'Robots.id'))
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].* FROM [Robots] GROUP BY Robots.name, Robots.id');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->columns(array('Robots.name', 'SUM(Robots.price)'))
						->from('Robots')
						->groupBy('Robots.name')
						->getPhql();
		$this->assertEquals($phql, 'SELECT Robots.name, SUM(Robots.price) FROM [Robots] GROUP BY Robots.name');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->columns(array('Robots.name', 'SUM(Robots.price)'))
						->from('Robots')
						->groupBy('Robots.name')
						->having('SUM(Robots.price) > 1000')
						->getPhql();
		$this->assertEquals($phql, 'SELECT Robots.name, SUM(Robots.price) FROM [Robots] GROUP BY Robots.name HAVING SUM(Robots.price) > 1000');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots')
						->join('RobotsParts')
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].* FROM [Robots] JOIN [RobotsParts]');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots')
						->join('RobotsParts', null, 'p')
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].* FROM [Robots] JOIN [RobotsParts] AS [p]');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots')
						->join('RobotsParts', 'Robots.id = RobotsParts.robots_id', 'p')
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].* FROM [Robots] JOIN [RobotsParts] AS [p] ON Robots.id = RobotsParts.robots_id');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots')
						->join('RobotsParts', 'Robots.id = RobotsParts.robots_id', 'p')
						->join('Parts', 'Parts.id = RobotsParts.parts_id', 't')
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].* FROM [Robots] JOIN [RobotsParts] AS [p] ON Robots.id = RobotsParts.robots_id JOIN [Parts] AS [t] ON Parts.id = RobotsParts.parts_id');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots')
						->leftJoin('RobotsParts', 'Robots.id = RobotsParts.robots_id')
						->leftJoin('Parts', 'Parts.id = RobotsParts.parts_id')
						->where('Robots.id > 0')
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].* FROM [Robots] LEFT JOIN [RobotsParts] ON Robots.id = RobotsParts.robots_id LEFT JOIN [Parts] ON Parts.id = RobotsParts.parts_id WHERE Robots.id > 0');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots', 'r')
						->getPhql();
		$this->assertEquals($phql, 'SELECT [r].* FROM [Robots] AS [r]');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->addFrom('Robots', 'r')
						->getPhql();
		$this->assertEquals($phql, 'SELECT [r].* FROM [Robots] AS [r]');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots')
						->addFrom('Parts', 'p')
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].*, [p].* FROM [Robots], [Parts] AS [p]');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots')
						->addFrom(array('p' => 'Parts'))
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].*, [p].* FROM [Robots], [Parts] AS [p]');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots', 'r')
						->addFrom(array('p' => 'Parts'))
						->getPhql();
		$this->assertEquals($phql, 'SELECT [r].*, [p].* FROM [Robots] AS [r], [Parts] AS [p]');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from(array('r' => 'Robots'))
						->addFrom('Parts', 'p')
						->getPhql();
		$this->assertEquals($phql, 'SELECT [r].*, [p].* FROM [Robots] AS [r], [Parts] AS [p]');


		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from(array('r' => 'Robots', 'p' => 'Parts'))
						->getPhql();
		$this->assertEquals($phql, 'SELECT [r].*, [p].* FROM [Robots] AS [r], [Parts] AS [p]');


		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots')
						->orderBy('Robots.name')
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].* FROM [Robots] ORDER BY Robots.name');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots')
						->orderBy(array(1, 'Robots.name'))
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].* FROM [Robots] ORDER BY 1, Robots.name');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots')
						->limit(10)
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].* FROM [Robots] LIMIT 10');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots')
						->limit(10, 5)
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].* FROM [Robots] LIMIT 10 OFFSET 5');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->from('Robots')
						->notInWhere('id', [1, 2, 3])
						->limit(10, 5)
						->getPhql();
		$this->assertEquals($phql, 'SELECT [Robots].* FROM [Robots] WHERE id NOT IN (:phi0:, :phi1:, :phi2:) LIMIT 10 OFFSET 5');
		$this->assertEquals($builder->getMergeBindParams(), ['phi0' => 1, 'phi1' => 2, 'phi2' => 3]);
	}

	public function _testUpdateBuilder($di)
	{
		$builder = Builder::CreateUpdateBuilder();
		$phql = $builder->table('Robots')
						->set(array('name' => 'Google', 'price' => 1.0))
						->getPhql();
		$this->assertEquals($phql, 'UPDATE [Robots] SET [Robots].name = :phu_name:, [Robots].price = :phu_price:');
	}

	public function _testInsertBuilder($di)
	{
		$builder = Builder::CreateInsertBuilder();
		$phql = $builder->table('Robots')->columns(array('name', 'price'))
						->values(array(
							array('Google', 1.0)
						))
						->getPhql();
		$this->assertEquals($phql, 'INSERT INTO [Robots] ([name], [price]) VALUES ( :phi_0_0:,  :phi_0_1:)');
	}

	public function _testDeleteBuilder($di)
	{
		$builder = Builder::CreateDeleteBuilder();
		$phql = $builder->table('Robots')->where('name = "Google"')
						->getPhql();
		$this->assertEquals($phql, 'DELETE FROM [Robots] WHERE name = "Google"');
	}

	public function _testIssue701($di)
	{
		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
			->from('Robots')
			->leftJoin('RobotsParts', 'Robots.id = RobotsParts.robots_id')
			->leftJoin('Parts', 'Parts.id = RobotsParts.parts_id')
			->where('Robots.id > :1: AND Robots.id < :2:', array(1 => 0, 2 => 1000))
		;

		$params = $phql->getQuery()->getBindParams();
		$this->assertEquals($params[1], 0);
		$this->assertEquals($params[2], 1000);

		$phql->andWhere('Robots.name = :name:', array('name' => 'Voltron'));

		$params = $phql->getQuery()->getBindParams();
		$this->assertEquals($params[1], 0);
		$this->assertEquals($params[2], 1000);
		$this->assertEquals($params['name'], 'Voltron');
	}

	public function _testIssue1115($di)
	{
		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
			->columns(array('Robots.name'))
			->from('Robots')
			->having('Robots.price > 1000')
			->getPhql();
		$this->assertEquals($phql, 'SELECT Robots.name FROM [Robots] HAVING Robots.price > 1000');
	}

	public function _testSelectDistinctAll($di)
	{
		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
			->distinct(true)
			->columns(array('Robots.name'))
			->from('Robots')
			->getPhql();
		$this->assertEquals($phql, 'SELECT DISTINCT Robots.name FROM [Robots]');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
			->distinct(false)
			->columns(array('Robots.name'))
			->from('Robots')
			->getPhql();
		$this->assertEquals($phql, 'SELECT ALL Robots.name FROM [Robots]');

		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
			->distinct(true)
			->distinct(null)
			->columns(array('Robots.name'))
			->from('Robots')
			->getPhql();
		$this->assertEquals($phql, 'SELECT Robots.name FROM [Robots]');
	}

	/**
	 * Test checks passing query params and dependency injector into
	 * constructor
	 */
	public function _testConstructor($di)
	{
		$params = array(
			'models'     => 'Robots',
			'columns'    => array('id', 'name', 'status'),
			'conditions' => "a > 5",
			'group'      => array('type', 'source'),
			'having'     => "b < 5",
			'order'      => array('name', 'created'),
			'limit'      => 10,
			'offset'     => 15,
		);

		$builder = new SelectBuilder($params, $di);

		$expectedPhql = "SELECT id, name, status FROM [Robots] "
			. "WHERE a > 5 GROUP BY [type], [source] "
			. "HAVING b < 5 ORDER BY [name], [created] "
			. "LIMIT 10 OFFSET 15";

		$this->assertEquals($expectedPhql, $builder->getPhql());
		$this->assertEquals($di, $builder->getDI());
	}

	/**
	 * Test checks passing 'limit'/'offset' query param into constructor.
	 * limit key can take:
	 * - signle numeric value
	 * - array of 2 values (limit, offset)
	 */
	public function _testConstructorLimit($di)
	{
		// separate limit and offset
		$params = array(
			'models' => 'Robots',
			'limit'  => 10,
			'offset' => 15,
		);

		$builderLimitAndOffset = new SelectBuilder($params);

		// separate limit with offset

		$params = array(
			'models' => 'Robots',
			'limit'  => array(10, 15),
		);

		$builderLimitWithOffset = new SelectBuilder($params);

		$expectedPhql = "SELECT [Robots].* FROM [Robots] "
			. "LIMIT 10 OFFSET 15";

		$this->assertEquals($expectedPhql, $builderLimitAndOffset->getPhql());
		$this->assertEquals($expectedPhql, $builderLimitWithOffset->getPhql());
	}

	/**
	 * Test checks passing 'condition' query param into constructor.
	 * Conditions can now be passed as an string(as before) and
	 * as an array of 3 elements:
	 * - condition string for example "age > :age: AND created > :created:"
	 * - bind params for example array('age' => 18, 'created' => '2013-09-01')
	 * - bind types for example array('age' => PDO::PARAM_INT, 'created' => PDO::PARAM_STR)
	 *
	 * First two params are REQUIRED, bind types are optional.
	 */
	public function _testConstructorConditions($di)
	{
		// ------------- test for setters(classic) way ----------------

		$standardSelectBuilder = new SelectBuilder();
		$standardSelectBuilder->from('Robots')
			->where(
				"year > :min: AND year < :max:",
				array("min" => '2013-01-01',   'max' => '2100-01-01'),
				array("min" => PDO::PARAM_STR, 'max' => PDO::PARAM_STR)
			);

		$standardResult = $standardSelectBuilder->getQuery()->execute();

		// --------------- test for single condition ------------------
		$params = array(
			'models'     => 'Robots',
			'conditions' => array(
				array(
					"year > :min: AND year < :max:",
					array("min" => '2013-01-01',   'max' => '2100-01-01'),
					array("min" => PDO::PARAM_STR, 'max' => PDO::PARAM_STR),
				),
			),
		);

		$builderWithSingleCondition = new SelectBuilder($params);
		$singleConditionResult      = $builderWithSingleCondition->getQuery()->execute();

		// ------------- test for multiple conditions ----------------

		$params = array(
			'models'     => 'Robots',
			'conditions' => array(
				array(
					"year > :min:",
					array("min" => '2000-01-01'),
					array("min" => PDO::PARAM_STR),
				),
				array(
					"year < :max:",
					array('max' => '2100-01-01'),
					array("max" => PDO::PARAM_STR),
				),
			),
		);

		// conditions are merged!
		$builderMultipleConditions = new SelectBuilder($params);
		$multipleConditionResult   = $builderMultipleConditions->getQuery()->execute();

		$expectedPhql = "SELECT [Robots].* FROM [Robots] "
			. "WHERE year > :min: AND year < :max:";

		/* ------------ ASSERTING --------- */

		$this->assertEquals($expectedPhql, $standardSelectBuilder->getPhql());
		$this->assertInstanceOf("Phalcon\Mvc\Model\Resultset\Simple", $standardResult);

		$this->assertEquals($expectedPhql, $builderWithSingleCondition->getPhql());
		$this->assertInstanceOf("Phalcon\Mvc\Model\Resultset\Simple", $singleConditionResult);

		$this->assertEquals($expectedPhql, $builderMultipleConditions->getPhql());
		$this->assertInstanceOf("Phalcon\Mvc\Model\Resultset\Simple", $multipleConditionResult);
    }

	public function _testGroup($di)
	{
		$builder = new SelectBuilder();
		$phql = $builder->setDi($di)
						->columns(array('name', 'SUM(price)'))
						->from('Robots')
						->groupBy('id, name')
						->getPhql();
		$this->assertEquals($phql, 'SELECT name, SUM(price) FROM [Robots] GROUP BY [id], [name]');
	}
}
