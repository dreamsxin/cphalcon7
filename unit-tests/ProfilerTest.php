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

class Profiler extends Phalcon\Profiler
{

	private $_points = 0;

	public function beforeStartProfile($profile)
	{
		$this->_points++;
	}

	public function afterEndProfile($profile)
	{
		$this->_points--;
	}

	public function getPoints()
	{
		return $this->_points;
	}

}

class ProfilerListener
{
	protected $_profiler;

	public function __construct(){
		$this->_profiler = new Profiler();
	}

	public function startOne($event, $data)
	{
		$this->_profiler->startProfile($event->getType(), [$data]);
	}

	public function endOne($event, $data)
	{
		$this->_profiler->stopProfile();
	}

	public function getProfiler(){
		return $this->_profiler;
	}

}

class ProfilerTest extends PHPUnit\Framework\TestCase
{

	public function testNormal()
	{
		$eventsManager = new Phalcon\Events\Manager();

		$listener = new ProfilerListener();
		$profiler = $listener->getProfiler();

		$eventsManager->attach('test', $listener);

		$eventsManager->fire('test:startOne', $this, "extra data");

		$this->assertEquals($profiler->getPoints(), 1);

		$profile = $profiler->getLastProfile();
		$this->assertEquals(get_class($profile), 'Phalcon\Profiler\Item');

		$eventsManager->fire('test:endOne', $this);

		$this->assertEquals($profiler->getPoints(), 0);

		$profile = $profiler->getLastProfile();
		$this->assertEquals(get_class($profile), 'Phalcon\Profiler\Item');
		$this->assertEquals(gettype($profile->getTotalElapsedSeconds()), 'double');
		$this->assertEquals(gettype($profile->getTotalUsageMemory()), 'integer');
		$this->assertEquals($profile->getName(), 'startOne');

		$this->assertEquals(count($profiler->getProfiles()), 1);
		$this->assertEquals(gettype($profiler->getTotalElapsedSeconds()), "double");
		$this->assertEquals(gettype($profiler->getTotalUsageMemory()), "integer");
		$this->assertEquals($profiler->getPoints(), 0);

		// second time
		$eventsManager->fire('test:startOne', $this, "extra data");

		$this->assertEquals($profiler->getPoints(), 1);

		$profile = $profiler->getLastProfile();
		$this->assertEquals(get_class($profile), 'Phalcon\Profiler\Item');

		$eventsManager->fire('test:endOne', $this);

		$this->assertEquals($profiler->getPoints(), 0);

		$profile = $profiler->getLastProfile();
		$this->assertEquals(get_class($profile), 'Phalcon\Profiler\Item');

		$this->assertEquals(count($profiler->getProfiles()), 2);
		$this->assertEquals(gettype($profiler->getTotalElapsedSeconds()), "double");
		$this->assertEquals(gettype($profiler->getTotalUsageMemory()), "integer");
		$this->assertEquals($profiler->getPoints(), 0);

		$profiler->reset();

		$this->assertEquals(count($profiler->getProfiles()), 0);
		$this->assertEquals($profiler->getPoints(), 0);
	}

	public function testMulti()
	{
		$profiler = new Phalcon\Profiler();

		$profiler->startProfile('one');

		$profile = $profiler->getCurrentProfile();
		$this->assertEquals($profile->getName(), 'one');

		$profile = $profiler->getLastProfile();
		$this->assertEquals($profile->getName(), 'one');

		$profiler->startProfile('two');

		$profile = $profiler->getCurrentProfile();
		$this->assertEquals($profile->getName(), 'two');

		$profile = $profiler->getLastProfile();
		$this->assertEquals($profile->getName(), 'two');

		$profiler->stopProfile('two');

		$this->assertEquals(count($profiler->getProfiles()), 1);
		$this->assertEquals(gettype($profiler->getTotalElapsedSeconds()), "double");
		$this->assertEquals(gettype($profiler->getTotalUsageMemory()), "integer");

		$profile = $profiler->getCurrentProfile();
		$this->assertEquals($profile->getName(), 'one');

		$profile = $profiler->getLastProfile();
		$this->assertEquals($profile->getName(), 'two');

		$profiler->stopProfile('one');

		$this->assertEquals(count($profiler->getProfiles()), 2);
		$this->assertEquals(gettype($profiler->getTotalElapsedSeconds()), "double");
		$this->assertEquals(gettype($profiler->getTotalUsageMemory()), "integer");

		$profile = $profiler->getCurrentProfile();
		$this->assertNull($profile);

		$profile = $profiler->getLastProfile();
		$this->assertEquals($profile->getName(), 'two');

		// Test skip
		$profiler = new Phalcon\Profiler();

		$profiler->startProfile('one');
		$profiler->startProfile('two');
		$profiler->startProfile('two');

		$profiler->stopProfile('one');

		$profiles = $profiler->getProfiles();
		$this->assertEquals(count($profiles), 3);

		$this->assertEquals($profiles[0]->getName(), 'two');
		$this->assertEquals($profiles[1]->getName(), 'two');
		$this->assertEquals($profiles[0]->getTotalElapsedSeconds(), 0);
		$this->assertEquals($profiles[0]->getTotalUsageMemory(), 0);
		$this->assertNull($profiles[0]->getFinalTime());
		$this->assertNull($profiles[0]->getEndMemory());

		$this->assertEquals($profiles[2]->getName(), 'one');
		$this->assertTrue($profiles[2]->getTotalElapsedSeconds() > 0);
		$this->assertTrue($profiles[2]->getTotalUsageMemory()*1000 >= 0);
		$this->assertEquals(gettype($profiles[2]->getFinalTime()), 'double');
		$this->assertEquals(gettype($profiles[2]->getEndMemory()), 'integer');

		$profile = $profiler->getCurrentProfile();
		$this->assertNull($profile);

		$profile = $profiler->getLastProfile();
		$this->assertEquals($profile->getName(), 'two');

		// Test unique
		$profiler = new Phalcon\Profiler(TRUE);
			
		try {
			$profiler->startProfile('one');
			$profiler->startProfile('one');
			$this->assertTrue(false);
		}
		catch (Phalcon\Profiler\Exception $ex) {
			$this->assertTrue(true);
		}
		catch (Exception $e) {
			$this->assertTrue(false);
		}
	}

}
