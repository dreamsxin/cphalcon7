<?php 

namespace Phalcon\Mvc\User\Logic {

	/**
	 * Phalcon\Mvc\User\Logic
	 *
	 * This class can be used to provide user business logic an easy access to services in the application
	 */
	
	abstract class Model extends \Phalcon\Mvc\User\Logic implements \Phalcon\Events\EventsAwareInterface, \Phalcon\Di\InjectionAwareInterface {

		abstract public function get($arguments=null);


		abstract public function getAll($arguments=null);


		abstract public function save($arguments=null);


		abstract public function create($arguments=null);


		abstract public function delete($arguments=null);


		abstract public function deleteAll($arguments=null);


		abstract public function update($arguments=null);


		abstract public function updateAll($arguments=null);


		abstract public function count($arguments=null);

	}
}
