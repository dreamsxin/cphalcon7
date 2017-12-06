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

class IntrusiveRbtreeTest extends PHPUnit\Framework\TestCase
{
	public function testNormal()
	{
		if (!class_exists('Phalcon\Intrusive\Rbtree')) {
			$this->markTestSkipped('Class `Phalcon\Intrusive\Rbtree` is not exists');
			return false;
		}
		$rbtree = new Phalcon\Intrusive\Rbtree;
		$node2 = $rbtree->insert(2);
		$node3 = $rbtree->insert(3);
		$node4 = $rbtree->insert(4);
		$node5 = $rbtree->insert(5);
		$node6 = $rbtree->insert(6);

		$node1 = new Phalcon\Intrusive\Rbtree\Node(1);
		$rbtree->insert($node1);

		$node = $rbtree->find(1);
		$this->assertEquals($node->getValue(), 1);

		$node = $rbtree->find(6);
		$this->assertEquals($node->getValue(), 6);

		$node = $rbtree->first();
		$this->assertEquals($node->getValue(), 1);

		$node = $rbtree->last();
		$this->assertEquals($node->getValue(), 6);

		$node = $rbtree->find(4);
		$this->assertEquals($node->getValue(), 4);

		$this->assertEquals($node->prev()->getValue(), 3);
		$this->assertEquals($node->next()->getValue(), 5);

		$node = $rbtree->prev(4);
		$this->assertEquals($node->getValue(), 3);

		$node = $rbtree->next(4);
		$this->assertEquals($node->getValue(), 5);

		$node = $rbtree->replace($node1, $node5);
		$this->assertEquals($node, $node5);

		$this->assertFalse($rbtree->find(1));
	}
}
