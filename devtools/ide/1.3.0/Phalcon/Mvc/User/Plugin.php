<?php 

namespace Phalcon\Mvc\User {

	/**
	 * Phalcon\Mvc\User\Plugin
	 *
	 * This class can be used to provide user plugins an easy access to services
	 * in the application
	 */
	
	class Plugin extends \Phalcon\User\Plugin implements \Phalcon\Di\InjectionAwareInterface, \Phalcon\Events\EventsAwareInterface {
	}
}
