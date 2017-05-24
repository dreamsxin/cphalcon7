<?php 

namespace Phalcon\Annotations\Adapter {

	/**
	 * Phalcon\Annotations\Adapter\Cache
	 *
	 * Stores the parsed annotations in cache. This adapter is suitable for production
	 *
	 *<code>
	 * $annotations = new \Phalcon\Annotations\Adapter\Cache();
	 *</code>
	 */
	
	class Cache extends \Phalcon\Annotations\Adapter implements \Phalcon\Annotations\AdapterInterface {

		protected $_lifetime;

		protected $_cache;

		/**
		 * Constructor for \Phalcon\Session\Adapter\Cache
		 *
		 * @param array $options
		 */
		public function __construct($options){ }


		/**
		 * Reads parsed annotations from cache
		 *
		 * @param string $key
		 * @return \Phalcon\Annotations\Reflection
		 */
		public function read($key){ }


		/**
		 * Writes parsed annotations to cache
		 *
		 * @param string $key
		 * @param \Phalcon\Annotations\Reflection $data
		 */
		public function write($key, $data){ }

	}
}
