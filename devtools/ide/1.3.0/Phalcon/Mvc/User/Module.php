<?php 

namespace Phalcon\Mvc\User {

	/**
	 * Phalcon\Mvc\User\Module
	 *
	 * This class can be used to provide user modules easy access to services
	 * in the application
	 */
	
	abstract class Module extends \Phalcon\User\Module implements \Phalcon\Di\InjectionAwareInterface, \Phalcon\Events\EventsAwareInterface {
	}
}
