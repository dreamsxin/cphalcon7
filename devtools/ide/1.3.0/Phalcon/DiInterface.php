<?php 

namespace Phalcon {

	/**
	 * Phalcon\DiInterface initializer
	 */
	
	interface DiInterface extends \ArrayAccess {

		/**
		 * Registers a service in the service container
		 *
		 * @param string $name
		 * @param mixed $definition
		 * @param boolean $shared
		 * @return \Phalcon\Di\ServiceInterface
		 */
		public function set($name, $definition, $shared=null);


		/**
		 * Removes a service from the service container
		 *
		 * @param string $name
		 */
		public function remove($name);


		/**
		 * Resolves the service based on its configuration
		 *
		 * @param string $name
		 * @param array $parameters
		 * @param boolean $noError
		 * @return object
		 */
		public function get($name, $parameters=null, $noError=null, $noerror=null);


		/**
		 * Resolves a shared service based on their configuration
		 *
		 * @param string $name
		 * @param array $parameters
		 * @param boolean $noError
		 * @return object
		 */
		public function getShared($name, $parameters=null, $noError=null);


		/**
		 * Sets a service using a raw \Phalcon\Di\Service definition
		 *
		 * @param string $name
		 * @param \Phalcon\Di\ServiceInterface $rawDefinition
		 * @return \Phalcon\Di\ServiceInterface
		 */
		public function setService($name, \Phalcon\Di\ServiceInterface $rawDefinition);


		/**
		 * Returns the corresponding \Phalcon\Di\Service instance for a service
		 *
		 * @param string $name
		 * @return \Phalcon\Di\ServiceInterface
		 */
		public function getService($name);


		/**
		 * Check whether the DI contains a service by a name
		 *
		 * @param string $name
		 * @return boolean
		 */
		public function has($name);


		/**
		 * Check whether the last service obtained via getShared produced a fresh instance or an existing one
		 *
		 * @return boolean
		 */
		public function wasFreshInstance();


		/**
		 * Return the services registered in the DI
		 *
		 * @return array
		 */
		public function getServices();


		/**
		 * Set the default dependency injection container to be obtained into static methods
		 *
		 * @param \Phalcon_DiInterface $dependencyInjector
		 */
		public static function setDefault(\Phalcon\DiInterface $dependencyInjector);


		/**
		 * Return the last DI created
		 *
		 * @return \Phalcon_DiInterface
		 */
		public static function getDefault();


		/**
		 * Resets the internal default DI
		 */
		public static function reset();

	}
}
