<?php 

namespace Phalcon\Cache {

	/**
	 * Phalcon\Cache\Yac
	 *
	 * It can be used to replace APC or local memcached.
	 */
	
	class Yac {

		protected $_prefix;

		/**
		 * \Phalcon\Cache\Yac constructor
		 *
		 * @param string $prefix
		 */
		public function __construct($prefix=null){ }


		/**
		 * Stores cached content
		 *
		 * @param string|array $keys
		 * @param mixed $value
		 * @param long $lifetime
		 * @return \Phalcon\Cache\Yac
		 */
		public function set($keys, $value=null, $lifetime=null){ }


		/**
		 * Returns a cached content
		 *
		 * @param string|array $keys
		 * @return mixed
		 */
		public function get($keys){ }


		/**
		 * Returns a cached content
		 *
		 * @param string|array $keys
		 * @param long $lifetime
		 * @return boolean
		 */
		public function delete($keys, $lifetime=null){ }


		public function flush(){ }


		public function dump($limit=null){ }


		/**
		 * Stores cached content
		 *
		 * @param string $key
		 * @param mixed $value
		 * @param long $lifetime
		 */
		public function __set($key, $value){ }


		/**
		 * Returns a cached content
		 *
		 * @param string $key
		 * @return mixed
		 */
		public function __get($key){ }

	}
}
