<?php 

namespace Phalcon\Config {

	/**
	 * Phalcon\Config\Adapter
	 *
	 * Base class for Phalcon\Config adapters
	 */
	
	abstract class Adapter extends \Phalcon\Config implements \Countable, \ArrayAccess, \Phalcon\Config\AdapterInterface {

		protected static $_basePath;

		protected static $_instances;

		/**
		 * \Phalcon\Config\Adapter constructor
		 *
		 * @param string $filePath
		 * @param string $absolutePath
		 */
		public function __construct($filePath=null, $absolutePath=null){ }


		/**
		 * \Phalcon\Config\Adapter factory
		 *
		 * @param string $filePath
		 * @param string $absolutePath
		 */
		public static function factory($filePath=null, $absolutePath=null){ }


		/**
		 * Sets base path
		 *
		 * @param string $basePath
		 * @return \Phalcon\Config\Adapter
		 */
		public static function setBasePath($basePath){ }


		/**
		 * Gets base path
		 *
		 * @return string
		 */
		public static function getBasePath(){ }


		/**
		 * Load a configuration
		 *
		 * @param string $filePath
		 * @param string $absolutePath
		 */
		public function load($filePath, $absolutePath=null){ }


		abstract public function read($filePath, $absolutePath=null);

	}
}
