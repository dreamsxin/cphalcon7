<?php 

namespace Phalcon\Config\Adapter {

	/**
	 * Phalcon\Config\Adapter\Ini
	 *
	 * Reads ini files and converts them to Phalcon\Config objects.
	 *
	 * Given the next configuration file:
	 *
	 *<code>
	 *[database]
	 *adapter = Mysql
	 *host = localhost
	 *username = scott
	 *password = cheetah
	 *dbname = test_db
	 *
	 *[phalcon]
	 *controllersDir = "../app/controllers/"
	 *modelsDir = "../app/models/"
	 *viewsDir = "../app/views/"
	 *</code>
	 *
	 * You can read it as follows:
	 *
	 *<code>
	 *	$config = new Phalcon\Config\Adapter\Ini("path/config.ini");
	 *	echo $config->phalcon->controllersDir;
	 *	echo $config->database->username;
	 *</code>
	 *
	 */
	
	class Ini extends \Phalcon\Config\Adapter implements \Phalcon\Config\AdapterInterface, \ArrayAccess, \Countable {

		/**
		 * \Phalcon\Config\Adapter constructor
		 *
		 * @param string $filePath
		 * @param string $absolutePath
		 * @param string $scannerMode
		 */
		public function __construct($filePath=null, $absolutePath=null, $scannerMode=null){ }


		/**
		 * Load config file
		 *
		 * @param string $filePath
		 */
		public function read($filePath, $absolutePath=null, $scannerMode=null){ }

	}
}
