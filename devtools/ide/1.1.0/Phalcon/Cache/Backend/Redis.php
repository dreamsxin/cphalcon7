<?php 

namespace Phalcon\Cache\Backend {

	/**
	 * Phalcon\Cache\Backend\Redis
	 *
	 * Allows to cache output fragments, PHP data or raw data to a redis backend
	 *
	 * This adapter uses the special redisd key "_PHCR" to store all the keys internally used by the adapter
	 *
	 *<code>
	 *
	 * // Cache data for 2 days
	 * $frontCache = new Phalcon\Cache\Frontend\Data(array(
	 *    "lifetime" => 172800
	 * ));
	 *
	 * //Create the Cache setting redisd connection options
	 * $cache = new Phalcon\Cache\Backend\Redis($frontCache, array(
	 *		'host' => 'localhost',
	 *		'port' => 6379,
	 *		'auth' => 'foobared',
	 *  	'persistent' => false
	 * ));
	 *
	 * //Cache arbitrary data
	 * $cache->save('my-data', array(1, 2, 3, 4, 5));
	 *
	 * //Get data
	 * $data = $cache->get('my-data');
	 *
	 *</code>
	 */
	
	class Redis extends \Phalcon\Cache\Backend implements \Phalcon\Cache\BackendInterface, \Phalcon\Di\InjectionAwareInterface, \Phalcon\Events\EventsAwareInterface {

		protected $_redis;

		protected $_auth;

		/**
		 * \Phalcon\Cache\Backend\Redis constructor
		 *
		 * @param \Phalcon\Cache\FrontendInterface $frontend
		 * @param array $options
		 */
		public function __construct($frontend, $options=null){ }


		/**
		 * Create internal connection to redisd
		 */
		protected function _connect(){ }


		/**
		 * Returns a cached content
		 *
		 * @param int|string $keyName
		 * @param   long $lifetime
		 * @return  mixed
		 */
		public function get($keyName, $lifetime=null){ }


		/**
		 * Stores cached content into the Redisd backend and stops the frontend
		 *
		 * @param int|string $keyName
		 * @param string $content
		 * @param long $lifetime
		 * @param boolean $stopBuffer
		 */
		public function save($keyName=null, $content=null, $lifetime=null, $stopBuffer=null){ }


		/**
		 * Deletes a value from the cache by its key
		 *
		 * @param int|string $keyName
		 * @return boolean
		 */
		public function delete($keyName){ }


		/**
		 * Query the existing cached keys
		 *
		 * @param string $prefix
		 * @return array
		 */
		public function queryKeys($prefix=null){ }


		/**
		 * Checks if cache exists and it hasn't expired
		 *
		 * @param  string $keyName
		 * @param  long $lifetime
		 * @return boolean
		 */
		public function exists($keyName=null, $lifetime=null){ }


		/**
		 * Atomic increment of a given key, by number $value
		 * 
		 * @param  string $keyName
		 * @param  long $value
		 * @return mixed
		 */
		public function increment($key_name=null, $value=null){ }


		/**
		 * Atomic decrement of a given key, by number $value
		 * 
		 * @param  string $keyName
		 * @param  long $value
		 * @return mixed
		 */
		public function decrement($key_name=null, $value=null){ }


		/**
		 * Immediately invalidates all existing items.
		 * 
		 * @return boolean
		 */
		public function flush(){ }


		public function getTrackingKey(){ }


		public function setTrackingKey($key){ }

	}
}
