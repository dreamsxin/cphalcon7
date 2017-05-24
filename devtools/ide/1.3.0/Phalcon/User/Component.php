<?php 

namespace Phalcon\User {

	/**
	 * Phalcon\User\Component
	 *
	 * This class can be used to provide user components easy access to services
	 * in the application
	 */
	
	abstract class Component extends \Phalcon\Di\Injectable implements \Phalcon\Events\EventsAwareInterface, \Phalcon\Di\InjectionAwareInterface {
	}
}
