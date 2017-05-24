<?php 

namespace Phalcon\Storage {

	/**
	 * Phalcon\Storage\Btree
	 *
	 * It can be used to replace APC or local memstoraged.
	 */
	
	class Btree {

		protected $_db;

		/**
		 * \Phalcon\Storage\Btree constructor
		 *
		 * @param string $db
		 */
		public function __construct($db){ }


		/**
		 * Stores storaged content
		 *
		 * @param string $key
		 * @param string $value
		 * @return string
		 */
		public function set($key, $value){ }


		/**
		 * Returns a storaged content
		 *
		 * @param string $key
		 * @return mixed
		 */
		public function get($key){ }


		/**
		 * Returns a storaged content
		 *
		 * @param string|array $keys
		 * @return boolean
		 */
		public function delete($key){ }

	}
}
