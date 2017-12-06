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

class AclTest extends PHPUnit\Framework\TestCase
{

	public function setUp()
	{
		spl_autoload_register(array($this, 'awaresAutoloader'));
	}

	public function tearDown()
	{
		spl_autoload_unregister(array($this, 'awaresAutoloader'));
	}

	public function awaresAutoloader($className)
	{
		if (file_exists('unit-tests/acl/'.$className.'.php')) {
			require 'unit-tests/acl/'.$className.'.php';
		}
	}

	public function testMemory()
	{
		$acl = new \Phalcon\Acl\Adapter\Memory();
		$acl->setDefaultAction(Phalcon\Acl::DENY);

		$roles = array(
			'Admin' => new \Phalcon\Acl\Role('Admin'),
			'Users' => new \Phalcon\Acl\Role('Users'),
			'Guests' => new \Phalcon\Acl\Role('Guests')
		);

		$resources = array(
			'welcome' => array('index', 'about'),
			'account' => array('index'),
		);

		foreach ($roles as $role => $object) {
			$acl->addRole($object);
		}

		foreach ($resources as $resource => $actions) {
			$acl->addResource(new \Phalcon\Acl\Resource($resource), $actions);
		}

		$this->assertFalse($acl->isAllowed('Admin', 'welcome', 'index'));
		$this->assertFalse($acl->isAllowed('Admin', 'welcome', 'about'));

		$acl->allow('Admin', 'welcome', '*');

		$this->assertTrue($acl->isAllowed('Admin', 'welcome', 'index'));
		$this->assertTrue($acl->isAllowed('Admin', 'welcome', 'about'));

		$this->assertFalse($acl->isAllowed('Admin', 'account', 'index'));
		$this->assertFalse($acl->isAllowed('Admin', 'account', 'about'));

		$acl->allow('Admin', '*', '*');

		$this->assertTrue($acl->isAllowed('Admin', 'welcome', 'index'));
		$this->assertTrue($acl->isAllowed('Admin', 'welcome', 'about'));

		$this->assertTrue($acl->isAllowed('Admin', 'account', 'index'));
		$this->assertTrue($acl->isAllowed('Admin', 'account', 'about'));

		$acl->deny('Admin', '*', '*');

		foreach ($roles as $role => $object) {
			$this->assertFalse($acl->isAllowed($role, 'welcome', 'about'));
		}

		$acl->allow("*", "welcome", "index");

		foreach ($roles as $role => $object) {
			$this->assertTrue($acl->isAllowed($role, 'welcome', 'index'));
		}

		$acl->deny("*", "welcome", "index");

		foreach ($roles as $role => $object) {
			$this->assertFalse($acl->isAllowed($role, 'welcome', 'index'));
		}

		$acl->allow('Admin', '*', 'index');

		foreach ($resources as $resource => $actions) {
			$this->assertTrue($acl->isAllowed('Admin', $resource, 'index'));
		}
	}

	public function testAclAware()
	{
		$acl = new Phalcon\Acl\Adapter\Memory;
		$acl->setDefaultAction(Phalcon\Acl::DENY);
		$acl->addRole('Guests');
		$acl->addRole('Members', 'Guests');
		$acl->addRole('Admins', 'Members');
		$acl->addResource('Post', array('update'));

		$guest = new \TestRole(1, 'Guests');
		$member = new \TestRole(2, 'Members');
		$anotherMember = new \TestRole(3, 'Members');
		$admin = new \TestRole(4, 'Admins');

		$resource = new \TestResource(2, 'Post');

		$acl->deny('Guests', 'Post', 'update', function(\TestRole $user, \TestResource $resource) {
			return $user->getId() != $resource->getUserId();
		});
		$acl->allow('Members', 'Post', 'update', function(\TestRole $user, \TestResource $resource) {
			return $user->getId() == $resource->getUserId();
		});
		$acl->allow('Admins', 'Post', 'update');

		$this->assertFalse($acl->isAllowed($guest, $resource, 'update'));
		$this->assertTrue($acl->isAllowed($member, $resource, 'update'));
		$this->assertFalse($acl->isAllowed($anotherMember, $resource, 'update'));
		$this->assertTrue($acl->isAllowed($admin, $resource, 'update'));
	}

	public function testIssues1513()
	{
		try {
			$acl = new \Phalcon\Acl\Adapter\Memory();
			$acl->setDefaultAction(Phalcon\Acl::DENY);
			$acl->addRole(new \Phalcon\Acl\Role('11'));
			$acl->addResource(new \Phalcon\Acl\Resource('11'), array('index'));
			$this->assertTrue(TRUE);
		} catch (Exception $e) {
			$this->assertTrue(FALSE);
		}
	}
}
