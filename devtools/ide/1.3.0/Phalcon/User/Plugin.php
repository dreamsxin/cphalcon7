<?php 

namespace Phalcon\User {

	/**
	 * Phalcon\User\Plugin
	 *
	 * This class can be used to provide user plugins an easy access to services
	 * in the application
	 */
	
	abstract class Plugin extends \Phalcon\Di\Injectable implements \Phalcon\Events\EventsAwareInterface, \Phalcon\Di\InjectionAwareInterface {
	}
}
