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

class MatrixTest extends PHPUnit\Framework\TestCase
{

	public function testNormal()
	{
		$matrix1 = array(
			array(1, 2.9, 3),
			array(4, 5, 6),
		);
		$matrix2 = array(
			array(2, 1.5, 4),
			array(3, 'qq', 1),
		);
		$matrix3 = array(
			array(1, 2),
			array(3, 4),
			array(5, 6),
		);
		$matrix4 = array(
			array(1, 2),
			array(3, 4),
			array(null, 5, 6, 7),
		);

		$this->assertTrue(Phalcon\Matrix::valid($matrix1));
		$this->assertFalse(Phalcon\Matrix::valid($matrix4));

		$this->assertEquals(Phalcon\Matrix::transpose($matrix1), [[1, 4], [2.9, 5], [3, 6]]);
		$this->assertEquals(Phalcon\Matrix::add($matrix1, $matrix3), [[15.9, 18.9],[24, 27]]);
		$this->assertEquals(Phalcon\Matrix::add($matrix1, $matrix3, NULL, Phalcon\Matrix::TYPE_LONG), [[15, 18],[24, 27]]);

		$this->assertEquals(Phalcon\Matrix::add($matrix1, $matrix2, TRUE), [[3.0,4.4,7.0],[7.0,5.0,7.0]]);
		$this->assertEquals(Phalcon\Matrix::add($matrix1, $matrix2, TRUE, Phalcon\Matrix::TYPE_LONG), [[3,3,7],[7,5,7]]);
		$this->assertEquals(Phalcon\Matrix::add($matrix1, 1), [[2,3,4],[5,6,7]]);
		$this->assertEquals(Phalcon\Matrix::add($matrix1, 1, FALSE, Phalcon\Matrix::TYPE_LONG), [[2,3,4],[5,6,7]]);

		$this->assertEquals(Phalcon\Matrix::mul($matrix1, $matrix3, NULL, Phalcon\Matrix::TYPE_LONG), [[22,28],[49,64]]);
		$this->assertEquals(Phalcon\Matrix::mul($matrix1, 2, NULL, Phalcon\Matrix::TYPE_LONG), [[2, 4, 6],[8,10,12]]);
	}
}
