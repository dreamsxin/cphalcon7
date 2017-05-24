<?php 

namespace Phalcon\Mvc\User {

	/**
	 * Phalcon\Mvc\User\Component
	 *
	 * This class can be used to provide user components easy access to services
	 * in the application
	 */
	
	abstract class Component extends \Phalcon\User\Component implements \Phalcon\Di\InjectionAwareInterface, \Phalcon\Events\EventsAwareInterface {
	}
}
