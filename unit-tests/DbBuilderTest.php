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
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

use Phalcon\Db\Builder as Builder;
use Phalcon\Db\Builder\Select as SelectBuilder;

class DbBuilderTest extends PHPUnit\Framework\TestCase
{
	protected function _getDI()
	{

		Phalcon\Di::reset();

		$di = new Phalcon\Di();

		return $di;
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
	}

	public function _testSelectBuilder($di)
	{
		$data = [
			'sql' => 'SELECT * FROM "robots"',
			'variables' => NULL,
			'types' => NULL
		];
		$builder = new SelectBuilder('robots');
		$ret = $builder->execute(true);
		$this->assertEquals($ret, $data);
	}

	public function _testUpdateBuilder($di)
	{
		$data = [
			'sql' => 'UPDATE "robots" SET "name" = :phu_name WHERE id = 1',
			'variables' => ['phu_name' => 'test'],
			'types' => NULL
		];
		$ret = Builder::update('robots')
			->set(['name' => 'test'])
			->where('id = 1')
			->execute(true);
		$this->assertEquals($ret, $data);
	}

	public function _testInsertBuilder($di)
	{
		$data = [
			'sql' => 'INSERT INTO "robots" ("name") VALUES (:phu_name)',
			'variables' => ['phu_name' => 'test'],
			'types' => NULL
		];
		$ret = Builder::insert('robots')->values(['name' => 'test'])->execute(true);
		$this->assertEquals($ret, $data);
	}

	public function _testDeleteBuilder($di)
	{
		$data = [
			'sql' => 'DELETE FROM "robots" WHERE id = 1',
			'variables' => NULL,
			'types' => NULL
		];
		$ret = Builder::delete('robots')->where('id = 1')->execute(true);
		$this->assertEquals($ret, $data);
	}
}
