<?php 

namespace Phalcon\Config {

	/**
	 * Phalcon\Config\AdapterInterface initializer
	 */
	
	interface AdapterInterface {

		/**
		 * Sets base path
		 *
		 * @param string $basePath
		 * @return \Phalcon\Config\Adapter
		 */
		public static function setBasePath($basePath);


		/**
		 * Gets base path
		 *
		 * @return string
		 */
		public static function getBasePath();

	}
}
