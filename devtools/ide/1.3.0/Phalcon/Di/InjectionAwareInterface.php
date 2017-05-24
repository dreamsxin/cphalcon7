<?php 

namespace Phalcon\Di {

	/**
	 * Phalcon\Di\InjectionAwareInterface initializer
	 */
	
	interface InjectionAwareInterface {

		/**
		 * Sets the dependency injector
		 *
		 * @param \Phalcon\DiInterface $dependencyInjector
		 */
		public function setDI($dependencyInjector);


		/**
		 * Returns the internal dependency injector
		 *
		 * @return \Phalcon\DiInterface
		 */
		public function getDI($error=null, $notUseDefault=null);

	}
}
