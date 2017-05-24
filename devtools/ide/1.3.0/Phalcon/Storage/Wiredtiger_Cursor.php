<?php 

namespace Phalcon\Storage {

	/**
	 * Phalcon\Storage\Wiredtiger\Cursor
	 *
	 * It can be used to replace APC or local memstoraged.
	 */
	
	class Wiredtiger_Cursor implements \Iterator, \Traversable {

		protected $_uri;

		protected $_config;

		/**
		 * \Phalcon\Storage\Wiredtiger\Cursor constructor
		 *
		 * @param \Phalcon\Storage\Wiredtiger $db
		 * @param string $uri
		 * @param string $config
		 */
		public function __construct(\Phalcon\Storage\Wiredtiger $db, $uri, $config=null){ }


		/**
		 * Reconfigure the cursor to overwrite the record
		 *
		 * @param string $config
		 */
		public function reconfigure($config){ }


		/**
		 * Sets data
		 *
		 * @param mixed $key
		 * @param mixed $value
		 * @return boolean
		 */
		public function set($key, $value){ }


		/**
		 * Sets data
		 *
		 * @param array $data
		 * @return boolean
		 */
		public function sets($data){ }


		/**
		 * Gets data
		 *
		 * @param mixed $key
		 * @return mixed
		 */
		public function get($key){ }


		/**
		 * Gets multi value
		 *
		 * @param array $keys
		 * @return array
		 */
		public function gets($keys){ }


		/**
		 * Delete data
		 *
		 * @param mixed $key
		 * @return boolean
		 */
		public function delete($key){ }


		/**
		 * Gets current value
		 *
		 * @return mixed
		 */
		public function current(){ }


		/**
		 * Gets current key
		 *
		 * @return mixed
		 */
		public function key(){ }


		/**
		 * Moves cursor to next row
		 *
		 * @return boolean
		 */
		public function next(){ }


		/**
		 * Moves cursor to prev row
		 *
		 * @return boolean
		 */
		public function prev(){ }


		/**
		 * Rewinds cursor to it's beginning
		 *
		 * @return mixed
		 */
		public function rewind(){ }


		/**
		 * Rewinds cursor to it's last
		 *
		 * @return mixed
		 */
		public function last(){ }


		/**
		 * Check whether has rows to fetch
		 *
		 * @return mixed
		 */
		public function valid(){ }


		/**
		 * Close the cursor
		 *
		 */
		public function close(){ }

	}
}
