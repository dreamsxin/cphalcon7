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

class ModelsEventsTest extends PHPUnit\Framework\TestCase
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
		if (file_exists('unit-tests/models/'.$className.'.php')) {
			require 'unit-tests/models/'.$className.'.php';
		}
	}

	protected function _prepareDI(&$trace, $sqlite = FALSE)
	{
		Phalcon\Di::reset();

		$eventsManager = new Phalcon\Events\Manager();

		$eventsManager->attach('model', function($event, $model) use (&$trace) {
			if (!isset($trace[$event->getType()][get_class($model)])) {
				$trace[$event->getType()][get_class($model)] = 1;
			} else {
				$trace[$event->getType()][get_class($model)]++;
			}
			switch($event->getType()) {
				case 'beforeCreate':
				case 'beforeDelete':
				case 'notSaved':
					return false;
				default:
					return;
			}
		});

		$di = new Phalcon\Di();

		$di->set('modelsManager', function() use ($eventsManager) {

			$modelsManager = new Phalcon\Mvc\Model\Manager();

			$modelsManager->setEventsManager($eventsManager);

			return $modelsManager;
		}, true);

		$di->set('modelsMetadata', function(){
			return new Phalcon\Mvc\Model\Metadata\Memory();
		}, true);

		$di->set('modelsQuery', 'Phalcon\Mvc\Model\Query');
		$di->set('modelsQueryBuilder', 'Phalcon\Mvc\Model\Query\Builder');
		$di->set('modelsCriteria', 'Phalcon\\Mvc\\Model\\Criteria');

		$di->set('db', function() use (&$sqlite) {
			require 'unit-tests/config.db.php';
			if (!$sqlite) {
				return new Phalcon\Db\Adapter\Pdo\Mysql($configMysql);
			} else {
				return new Phalcon\Db\Adapter\Pdo\Sqlite($configSqlite);
			}
		}, true);
	}

	public function testValidatorsMysql()
	{
		$this->_testEventsCreate();
		$this->_testEventsDelete();
	}

	public function testValidatorsSqlite()
	{
		$this->_testEventsCreate(true);
		$this->_testEventsDelete(true);
	}

	public function _testEventsCreate($sqlite = FALSE)
	{
		require 'unit-tests/config.db.php';
		if ($sqlite) {
			if (empty($configSqlite)) {
				$this->markTestSkipped("Skipped");
				return;
			}
		} else if (empty($configMysql)) {
			$this->markTestSkipped('Test skipped');
			return;
		}

		$trace = array();

		$this->_prepareDI($trace, $sqlite);

		$robot = new GossipRobots();

		$robot->name = 'Test';
		$robot->year = 2000;
		$robot->type = 'Some Type';

		$robot->trace = &$trace;

		$robot->save();

		$this->assertEquals($trace, array(
			'beforeOperation' => array(
				'GossipRobots' => 1,
			),
			'beforeValidation' => array(
				'GossipRobots' => 2,
			),
			'beforeValidationOnCreate' => array(
				'GossipRobots' => 1,
			),
			'validation' => array(
				'GossipRobots' => 2,
			),
			'afterValidationOnCreate' => array(
				'GossipRobots' => 1,
			),
			'afterValidation' => array(
				'GossipRobots' => 2,
			),
			'beforeSave' => array(
				'GossipRobots' => 2,
			),
			'beforeCreate' => array(
				'GossipRobots' => 1,
			)
		));

	}


	public function _testEventsDelete()
	{
		require 'unit-tests/config.db.php';
		if (empty($configMysql)) {
			$this->markTestSkipped('Test skipped');
			return;
		}

		$trace = array();

		$this->_prepareDI($trace);

		$robot = GossipRobots::findFirst();

		$robot->trace = &$trace;

		$robot->delete();

		$this->assertEquals($trace, array(
			'beforeQuery' => array(
				'GossipRobots' => 1,
			),
			'afterQuery' => array(
				'GossipRobots' => 1,
			),
			'beforeOperation' => array(
				'GossipRobots' => 1,
			),
			'beforeDelete' => array(
				'GossipRobots' => 1,
			)
		));

	}

}
