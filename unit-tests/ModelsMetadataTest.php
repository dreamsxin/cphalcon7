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

class ModelsMetadataTest extends PHPUnit\Framework\TestCase
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

	protected function _getDI()
	{

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

	public function testMetadataMysql()
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

		$this->_executeTests($di);

	}

	public function testMetadataPostgresql()
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

		$this->_executeTests($di);
	}

	public function testMetadataSqlite()
	{
		require 'unit-tests/config.db.php';
		if (empty($configSqlite)) {
			$this->markTestSkipped("Skipped");
			return;
		}

		$di = $this->_getDI();

		$di->set('db', function(){
			require 'unit-tests/config.db.php';
			return new Phalcon\Db\Adapter\Pdo\Sqlite($configSqlite);
		}, true);

		$this->_executeTests($di);
	}

	protected function _executeTests($di)
	{

		$metaData = $di->getShared('modelsMetadata');

		$personas = new Personas(NULL, $di);

		$pAttributes = array(
			0 => 'cedula',
			1 => 'tipo_documento_id',
			2 => 'nombres',
			3 => 'telefono',
			4 => 'direccion',
			5 => 'email',
			6 => 'fecha_nacimiento',
			7 => 'ciudad_id',
			8 => 'creado_at',
			9 => 'cupo',
			10 => 'estado',
		);

		$attributes = $metaData->getAttributes($personas);
		$this->assertEquals($attributes, $pAttributes);

		$ppkAttributes = array(
			0 => 'cedula'
		);

		$pkAttributes = $metaData->getPrimaryKeyAttributes($personas);
		$this->assertEquals($ppkAttributes, $pkAttributes);

		$pnpkAttributes = array(
			0 => 'tipo_documento_id',
			1 => 'nombres',
			2 => 'telefono',
			3 => 'direccion',
			4 => 'email',
			5 => 'fecha_nacimiento',
			6 => 'ciudad_id',
			7 => 'creado_at',
			8 => 'cupo',
			9 => 'estado',
		);

		$npkAttributes = $metaData->getNonPrimaryKeyAttributes($personas);
		$this->assertEquals($pnpkAttributes, $npkAttributes);

		$pnnAttributes = array(
			0 => 'cedula',
			1 => 'tipo_documento_id',
			2 => 'nombres',
			3 => 'cupo',
			4 => 'estado'
		);

		$nnAttributes = $metaData->getNotNullAttributes($personas);
		$this->assertEquals($nnAttributes, $pnnAttributes);

		$dataTypes = array(
			'cedula' => Phalcon\Db\Column::TYPE_CHAR,
			'tipo_documento_id' => Phalcon\Db\Column::TYPE_INTEGER,
			'nombres' => Phalcon\Db\Column::TYPE_VARCHAR,
			'telefono' => Phalcon\Db\Column::TYPE_VARCHAR,
			'direccion' => Phalcon\Db\Column::TYPE_VARCHAR,
			'email' => Phalcon\Db\Column::TYPE_VARCHAR,
			'fecha_nacimiento' => Phalcon\Db\Column::TYPE_DATE,
			'ciudad_id' => Phalcon\Db\Column::TYPE_INTEGER,
			'creado_at' => Phalcon\Db\Column::TYPE_DATE,
			'cupo' => Phalcon\Db\Column::TYPE_DECIMAL,
			'estado' => Phalcon\Db\Column::TYPE_CHAR,
		);

		$dtAttributes = $metaData->getDataTypes($personas);
		$this->assertEquals($dtAttributes, $dataTypes);

		$pndAttributes = array(
			'tipo_documento_id' => true,
			'ciudad_id' => true,
			'cupo' => true,
		);
		$ndAttributes = $metaData->getDataTypesNumeric($personas);
		$this->assertEquals($ndAttributes, $pndAttributes);

		$bindTypes = array(
			'cedula' => Phalcon\Db\Column::BIND_PARAM_STR,
			'tipo_documento_id' => Phalcon\Db\Column::BIND_PARAM_INT,
			'nombres' => Phalcon\Db\Column::BIND_PARAM_STR,
			'telefono' => Phalcon\Db\Column::BIND_PARAM_STR,
			'direccion' => Phalcon\Db\Column::BIND_PARAM_STR,
			'email' => Phalcon\Db\Column::BIND_PARAM_STR,
			'fecha_nacimiento' => Phalcon\Db\Column::BIND_PARAM_STR,
			'ciudad_id' => Phalcon\Db\Column::BIND_PARAM_INT,
			'creado_at' => Phalcon\Db\Column::BIND_PARAM_STR,
			'cupo' => Phalcon\Db\Column::BIND_PARAM_DECIMAL,
			'estado' => Phalcon\Db\Column::BIND_PARAM_STR,
		);

		$btAttributes = $metaData->getBindTypes($personas);
		$this->assertEquals($btAttributes, $bindTypes);

		$robots = new Robots(NULL, $di);

		//Robots
		$pAttributes = array(
			0 => 'id',
			1 => 'name',
			2 => 'type',
			3 => 'year'
		);

		$attributes = $metaData->getAttributes($robots);
		$this->assertEquals($attributes, $pAttributes);

		$ppkAttributes = array(
			0 => 'id'
		);

		$pkAttributes = $metaData->getPrimaryKeyAttributes($robots);
		$this->assertEquals($ppkAttributes, $pkAttributes);

		$pnpkAttributes = array(
			0 => 'name',
			1 => 'type',
			2 => 'year'
		);

		$npkAttributes = $metaData->getNonPrimaryKeyAttributes($robots);
		$this->assertEquals($pnpkAttributes, $npkAttributes);

		$this->assertEquals($metaData->getIdentityField($robots), 'id');

	}

}
