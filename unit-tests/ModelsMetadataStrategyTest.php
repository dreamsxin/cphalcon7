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

class ModelsMetadataStrategyTest extends PHPUnit\Framework\TestCase
{

	protected $_expectedMeta = array(
		0 => array( // PHALCON_MVC_MODEL_METADATA_MODELS_ATTRIBUTES
			0 => 'id',
			1 => 'name',
			2 => 'type',
			3 => 'year',
		),
		1 => array( // PHALCON_MVC_MODEL_METADATA_MODELS_PRIMARY_KEY
			0 => 'id',
		),
		2 => array( // PHALCON_MVC_MODEL_METADATA_MODELS_NON_PRIMARY_KEY
			0 => 'name',
			1 => 'type',
			2 => 'year',
		),
		3 => array( // PHALCON_MVC_MODEL_METADATA_MODELS_NOT_NULL
			0 => 'id',
			1 => 'name',
			2 => 'type',
			3 => 'year',
		),
		4 => array( // PHALCON_MVC_MODEL_METADATA_MODELS_DATA_TYPES
			'id' => Phalcon\Db\Column::TYPE_INTEGER,
			'name' => Phalcon\Db\Column::TYPE_VARCHAR,
			'type' => Phalcon\Db\Column::TYPE_VARCHAR,
			'year' => Phalcon\Db\Column::TYPE_INTEGER,
		),
		5 => array( // PHALCON_MVC_MODEL_METADATA_MODELS_DATA_TYPES_NUMERIC
			'id' => true,
			'year' => true,
		),
		8 => 'id', // PHALCON_MVC_MODEL_METADATA_MODELS_IDENTITY_COLUMN
		9 => array( // PHALCON_MVC_MODEL_METADATA_MODELS_DATA_TYPES_BIND
			'id' => 1,
			'name' => 2,
			'type' => 2,
			'year' => 1,
		),
		10 => array(), // PHALCON_MVC_MODEL_METADATA_MODELS_AUTOMATIC_DEFAULT_INSERT
		11 => array(), // PHALCON_MVC_MODEL_METADATA_MODELS_AUTOMATIC_DEFAULT_UPDATE
		12 => array(), // PHALCON_MVC_MODEL_METADATA_MODELS_DATA_DEFAULT_VALUE
		13 => array( // PHALCON_MVC_MODEL_METADATA_MODELS_DATA_SZIE
			'id' => 32,
			'name' => 70,
			'type' => 32,
			'year' => 32,
		),
		14 => array( // PHALCON_MVC_MODEL_METADATA_MODELS_DATA_SCALE
			'id' => 0,
			'name' => 0,
			'type' => 0,
			'year' => 0,
		),
		15 => array( // PHALCON_MVC_MODEL_METADATA_MODELS_DATA_BYTE
			'id' => 4,
			'name' => 70,
			'type' => 32,
			'year' => 4,
		),
	);

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
		$className = str_replace('\\', '/', $className);
		if (file_exists('unit-tests/models/'.$className.'.php')) {
			require 'unit-tests/models/'.$className.'.php';
		}
	}

	protected function _getDI()
	{
		Phalcon\Di::reset();

		$di = new Phalcon\Di();

		$di['modelsManager'] = function() {
			return new Phalcon\Mvc\Model\Manager();
		};

		$di->set('modelsQuery', 'Phalcon\Mvc\Model\Query');
		$di->set('modelsQueryBuilder', 'Phalcon\Mvc\Model\Query\Builder');
		$di->set('modelsCriteria', 'Phalcon\\Mvc\\Model\\Criteria');

		$di['db'] = function() {
			require 'unit-tests/config.db.php';
			return new Phalcon\Db\Adapter\Pdo\Postgresql($configPostgresql);
		};

		$di['annotations'] = function() {
			return new Phalcon\Annotations\Adapter\Memory();
		};

		return $di;
	}

	public function testMetadataDatabaseIntrospection()
	{
		require 'unit-tests/config.db.php';
		if (empty($configPostgresql)) {
			$this->markTestSkipped('Test skipped');
			return;
		}

		$di = $this->_getDI();

		$di['modelsMetadata'] = function() {
			$metaData = new Phalcon\Mvc\Model\Metadata\Memory();
			$metaData->setStrategy(new Phalcon\Mvc\Model\MetaData\Strategy\Introspection());
			return $metaData;
		};

		$metaData = $di['modelsMetadata'];

		$robots = new Robots($di);

		$meta = $metaData->readMetaData($robots);
		$this->assertEquals($meta, $this->_expectedMeta);

		$meta = $metaData->readMetaData($robots);
		$this->assertEquals($meta, $this->_expectedMeta);
	}

	public function testMetadataAnnotations()
	{
		require 'unit-tests/config.db.php';
		if (empty($configPostgresql)) {
			$this->markTestSkipped('Test skipped');
			return;
		}

		$di = $this->_getDI();

		$di['modelsMetadata'] = function(){
			$metaData = new Phalcon\Mvc\Model\Metadata\Memory();
			$metaData->setStrategy(new Phalcon\Mvc\Model\MetaData\Strategy\Annotations());
			return $metaData;
		};

		$metaData = $di['modelsMetadata'];

		$robots = new Boutique\Robots();

		$meta = $metaData->readMetaData($robots);
		$this->assertEquals($meta, $this->_expectedMeta);

		// Issue 2954
		$robot = Boutique\Robotters::findFirst();
		$code = $robot->code;
		$serialized = serialize($robot);
		$unserialized = unserialize($serialized);
		$this->assertEquals($code, $unserialized->code);
	}

}
