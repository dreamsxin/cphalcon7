<?php 

namespace Phalcon\User {

	/**
	 * Phalcon\User\Module
	 *
	 * This class can be used to provide user modules easy access to services
	 * in the application
	 */
	
	abstract class Module extends \Phalcon\Di\Injectable implements \Phalcon\Events\EventsAwareInterface, \Phalcon\Di\InjectionAwareInterface {
	}
}
