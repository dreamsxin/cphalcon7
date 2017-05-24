<?php 

namespace Phalcon\Cache\Backend {

	/**
	 * Phalcon\Cache\Backend\Memcached
	 *
	 * Allows to cache output fragments, PHP data or raw data to a memcached backend
	 *
	 * This adapter uses the special memcached key "_PHCM" to store all the keys internally used by the adapter
	 *
	 *<code>
	 *
	 * // Cache data for 2 days
	 * $frontCache = new Phalcon\Cache\Frontend\Data(array(
	 *    "lifetime" => 172800
	 * ));
	 *
	 * //Create the Cache setting memcached connection options
	 * $cache = new Phalcon\Cache\Backend\Memcached($frontCache, array(
	 *     'servers' => array(
	 *         array('host' => 'localhost',
	 *               'port' => 11211,
	 *               'weight' => 1),
	 *     ),
	 *     'client' => array(
	 *         Memcached::OPT_HASH => Memcached::HASH_MD5,
	 *         Memcached::OPT_PREFIX_KEY => 'prefix.',
	 *     )
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
	
	class Memcached extends \Phalcon\Cache\Backend implements \Phalcon\Cache\BackendInterface, \Phalcon\Di\InjectionAwareInterface, \Phalcon\Events\EventsAwareInterface {

		protected $_memcache;

		/**
		 * \Phalcon\Cache\Backend\Memcached constructor
		 *
		 * @param \Phalcon\Cache\FrontendInterface $frontend
		 * @param array $options
		 */
		public function __construct($frontend, $options=null){ }


		/**
		 * Create internal connection to memcached
		 */
		protected function _connect(){ }


		/**
		 * Returns a cached content
		 *
		 * @param string $keyName
		 * @param  long $lifetime
		 * @return  mixed
		 */
		public function get($keyName){ }


		/**
		 * Stores cached content into the Memcached backend and stops the frontend
		 *
		 * @param string $keyName
		 * @param string $content
		 * @param long $lifetime
		 * @param boolean $stopBuffer
		 */
		public function save($keyName=null, $value=null, $lifetime=null, $stopBuffer=null){ }


		/**
		 * Deletes a value from the cache by its key
		 *
		 * @param string $keyName
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
		 * @param string $keyName
		 * @return boolean
		 */
		public function exists($keyName){ }


		/**
		 * Increment of a given key, by number $value
		 *
		 * @param string $keyName
		 * @param long $value
		 * @return mixed
		 */
		public function increment($keyName, $value=null){ }


		/**
		 * Decrement of a given key, by number $value
		 *
		 * @param string $keyName
		 * @param long $value
		 * @return mixed
		 */
		public function decrement($keyName, $value=null){ }


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
