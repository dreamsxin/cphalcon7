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

class IntrusiveAvltreeTest extends PHPUnit\Framework\TestCase
{
	public function testNormal()
	{
		if (!class_exists('Phalcon\Intrusive\Avltree')) {
			$this->markTestSkipped('Class `Phalcon\Intrusive\Avltree` is not exists');
			return false;
		}
		$avltree = new Phalcon\Intrusive\Avltree;
		$node2 = $avltree->insert(2);
		$node3 = $avltree->insert(3);
		$node4 = $avltree->insert(4);
		$node5 = $avltree->insert(5);
		$node6 = $avltree->insert(6);

		$node1 = new Phalcon\Intrusive\Avltree\Node(1);
		$avltree->insert($node1);

		$node = $avltree->find(1);
		$this->assertEquals($node->getValue(), 1);

		$node = $avltree->find(6);
		$this->assertEquals($node->getValue(), 6);

		$node = $avltree->first();
		$this->assertEquals($node->getValue(), 1);

		$node = $avltree->last();
		$this->assertEquals($node->getValue(), 6);

		$node = $avltree->find(4);
		$this->assertEquals($node->getValue(), 4);

		$this->assertEquals($node->prev()->getValue(), 3);
		$this->assertEquals($node->next()->getValue(), 5);

		$node = $avltree->prev(4);
		$this->assertEquals($node->getValue(), 3);

		$node = $avltree->next(4);
		$this->assertEquals($node->getValue(), 5);

		$node = $avltree->replace($node1, $node5);
		$this->assertEquals($node, $node5);

		$this->assertFalse($avltree->find(1));
	}
}
