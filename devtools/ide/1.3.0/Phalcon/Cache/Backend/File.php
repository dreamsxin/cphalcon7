<?php 

namespace Phalcon\Cache\Backend {

	/**
	 * Phalcon\Cache\Backend\File
	 *
	 * Allows to cache output fragments using a file backend
	 *
	 *<code>
	 *	//Cache the file for 2 days
	 *	$frontendOptions = array(
	 *		'lifetime' => 172800
	 *	);
	 *
	 *  //Create a output cache
	 *  $frontCache = \Phalcon\Cache\Frontend\Output($frontOptions);
	 *
	 *	//Set the cache directory
	 *	$backendOptions = array(
	 *		'cacheDir' => '../app/cache/'
	 *	);
	 *
	 *  //Create the File backend
	 *  $cache = new \Phalcon\Cache\Backend\File($frontCache, $backendOptions);
	 *
	 *	$content = $cache->start('my-cache');
	 *	if ($content === null) {
	 *  	echo '<h1>', time(), '</h1>';
	 *  	$cache->save();
	 *	} else {
	 *		echo $content;
	 *	}
	 *</code>
	 */
	
	class File extends \Phalcon\Cache\Backend implements \Phalcon\Cache\BackendInterface, \Phalcon\Di\InjectionAwareInterface, \Phalcon\Events\EventsAwareInterface {

		protected $_cacheDir;

		/**
		 * \Phalcon\Cache\Backend\File constructor
		 *
		 * @param \Phalcon\Cache\FrontendInterface $frontend
		 * @param array $options
		 */
		public function __construct($frontend, $options){ }


		/**
		 * Returns a cached content
		 *
		 * @param string $keyName
		 * @return mixed
		 */
		public function get($keyName){ }


		/**
		 * Stores cached content into the file backend and stops the frontend
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
		 * Checks if cache exists and it isn't expired
		 *
		 * @param string $keyName
		 * @param  long $lifetime
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

	}
}
