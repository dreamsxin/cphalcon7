<?php 

namespace Phalcon\Cache {

	/**
	 * Phalcon\Cache\BackendInterface initializer
	 */
	
	interface BackendInterface {

		/**
		 * Starts a cache. The $keyname allows to identify the created fragment
		 *
		 * @param string $keyName
		 * @param long $lifetime
		 * @return mixed
		 */
		public function start($keyName, $lifetime=null);


		/**
		 * Stops the frontend without store any cached content
		 *
		 * @param boolean $stopBuffer
		 */
		public function stop($stopBuffer=null);


		/**
		 * Returns front-end instance adapter related to the back-end
		 *
		 * @return mixed
		 */
		public function getFrontend();


		/**
		 * Returns the backend options
		 *
		 * @return array
		 */
		public function getOptions();


		/**
		 * Checks whether the last cache is fresh or cached
		 *
		 * @return boolean
		 */
		public function isFresh();


		/**
		 * Checks whether the cache has starting buffering or not
		 *
		 * @return boolean
		 */
		public function isStarted();


		/**
		 * Returns a cached content
		 *
		 * @param string $keyName
		 * @return mixed
		 */
		public function get($keyName);


		/**
		 * Stores cached content into the file backend and stops the frontend
		 *
		 * @param string $keyName
		 * @param string $content
		 * @param long $lifetime
		 * @param boolean $stopBuffer
		 */
		public function save($keyName=null, $value=null, $lifetime=null, $stopBuffer=null);


		/**
		 * Deletes a value from the cache by its key
		 *
		 * @param string $keyName
		 * @return boolean
		 */
		public function delete($keyName);


		/**
		 * Query the existing cached keys
		 *
		 * @param string $prefix
		 * @return array
		 */
		public function queryKeys($prefix=null);


		/**
		 * Checks if cache exists and it hasn't expired
		 *
		 * @param string $keyName
		 * @return boolean
		 */
		public function exists($keyName);


		/**
		 * Immediately invalidates all existing items.
		 *
		 * @return boolean
		 */
		public function flush();

	}
}
