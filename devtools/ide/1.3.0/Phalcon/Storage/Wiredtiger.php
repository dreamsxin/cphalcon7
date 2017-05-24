<?php 

namespace Phalcon\Storage {

	/**
	 * Phalcon\Storage\Wiredtiger
	 *
	 * It can be used to replace APC or local memstoraged.
	 */
	
	class Wiredtiger {

		protected $_home;

		protected $_config;

		/**
		 * \Phalcon\Storage\Wiredtiger constructor
		 *
		 * @param string $home
		 * @param string $config
		 */
		public function __construct($home, $config=null){ }


		/**
		 * Create table
		 *
		 * @param string $uri
		 * @param string $config
		 */
		public function create($uri, $config=null){ }


		/**
		 * Open table
		 *
		 * @param string $uri
		 * @param string $config
		 */
		public function open($uri, $config=null){ }


		/**
		 * Alter table
		 *
		 * @param string $uri
		 * @param string $config
		 */
		public function alter($uri, $config=null){ }


		/**
		 * Drop table
		 *
		 * @param string $uri
		 * @param string $config
		 */
		public function drop($uri, $config=null){ }


		/**
		 * Open a transaction
		 *
		 * @param string $config
		 */
		public function begin($config=null){ }


		/**
		 * Commit a transaction
		 *
		 * @param string $config
		 */
		public function commit($config=null){ }


		/**
		 * Rollback a transaction
		 *
		 * @param string $config
		 */
		public function rollback($config=null){ }


		/**
		 * Transaction sync
		 *
		 * @param string $config
		 */
		public function sync($config=null){ }

	}
}
