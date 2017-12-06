<?php

/*
	+------------------------------------------------------------------------+
	| Phalcon Framework                                                      |
	+------------------------------------------------------------------------+
	| Copyright (c) 2011-2014 Phalcon Team (http://www.phalconphp.com)       |
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

class SyncTest extends PHPUnit\Framework\TestCase
{
	public function testMutex()
	{
		if (!class_exists('Phalcon\Sync\Mutex')) {
			$this->markTestSkipped('Class `Phalcon\Sync\Mutex is not exists');
			return false;
		}
		$mutex = new \Phalcon\Sync\Mutex();

		$this->assertTrue($mutex->lock());
		$this->assertTrue($mutex->unlock());

		$mutex2 = new \Phalcon\Sync\Mutex("UniqueName");

		$this->assertTrue($mutex2->lock(3000));
		$this->assertTrue($mutex2->unlock());
	}

	public function testSemaphore()
	{
		if (!class_exists('Phalcon\Sync\Semaphore')) {
			$this->markTestSkipped('Class `Phalcon\Sync\Semaphore` is not exists');
			return false;
		}
		$semaphore = new \Phalcon\Sync\Semaphore("LimitedResource_2_clients", 2);

		$this->assertTrue($semaphore->lock(3000));
		$this->assertTrue($semaphore->unlock());
	}

	public function testEvent()
	{
		$this->markTestSkipped("Test skipped");
		return;

		// In a web application:
		$event = new \Phalcon\Sync\Event("GetAppReport");

		$this->assertTrue($event->fire());

		// In a cron job:
		$event = new \Phalcon\Sync\Event("GetAppReport");
		$this->assertTrue($event->wait());
	}

	public function testReaderwriter()
	{
		if (!class_exists('Phalcon\Sync\Readerwriter')) {
			$this->markTestSkipped('Class `Phalcon\Sync\Readerwriter is not exists');
			return false;
		}

		$readwrite = new \Phalcon\Sync\Readerwriter("FileCacheLock");

		$this->assertTrue($readwrite->readlock());
		$this->assertTrue($readwrite->readunlock());

		$this->assertTrue($readwrite->writelock());
		$this->assertTrue($readwrite->writeunlock());
	}

	public function testSharedmemory()
	{
		if (!class_exists('Phalcon\Sync\Sharedmemory')) {
			$this->markTestSkipped('Class `Phalcon\Sync\Sharedmemory is not exists');
			return false;
		}

		$mem = new \Phalcon\Sync\Sharedmemory("AppReportName", 1024);

		$this->assertTrue($mem->first());
	}
}
