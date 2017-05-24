<?php 

namespace Phalcon\Mvc\User {

	/**
	 * Phalcon\Mvc\User\Logic
	 *
	 * This class can be used to provide user business logic an easy access to services in the application
	 */
	
	abstract class Logic extends \Phalcon\User\Logic implements \Phalcon\Di\InjectionAwareInterface, \Phalcon\Events\EventsAwareInterface {
	}
}
