<?php 

namespace Phalcon\Mvc\Model\MetaData {

	/**
	 * Phalcon\Mvc\Model\MetaData\Cache
	 *
	 * Stores model meta-data in the Cache cache. Data will erased if the web server is restarted
	 *
	 * By default meta-data is stored for 48 hours (172800 seconds)
	 *
	 * You can query the meta-data by printing cache_get('$PMM$') or cache_get('$PMM$my-app-id')
	 *
	 *<code>
	 *	$metaData = new Phalcon\Mvc\Model\Metadata\Cache(array(
	 *		'service' => 'cache', // Service Name
	 *		'lifetime' => 86400, // Service Name
	 *	));
	 *</code>
	 */
	
	class Cache extends \Phalcon\Mvc\Model\MetaData implements \Phalcon\Mvc\Model\MetaDataInterface, \Phalcon\Di\InjectionAwareInterface, \Phalcon\Events\EventsAwareInterface {

		const MODELS_ATTRIBUTES = 0;

		const MODELS_PRIMARY_KEY = 1;

		const MODELS_NON_PRIMARY_KEY = 2;

		const MODELS_NOT_NULL = 3;

		const MODELS_DATA_TYPES = 4;

		const MODELS_DATA_TYPES_NUMERIC = 5;

		const MODELS_DATE_AT = 6;

		const MODELS_DATE_IN = 7;

		const MODELS_IDENTITY_COLUMN = 8;

		const MODELS_DATA_TYPES_BIND = 9;

		const MODELS_AUTOMATIC_DEFAULT_INSERT = 10;

		const MODELS_AUTOMATIC_DEFAULT_UPDATE = 11;

		const MODELS_DATA_DEFAULT_VALUE = 12;

		const MODELS_DATA_SZIE = 13;

		const MODELS_DATA_SCALE = 14;

		const MODELS_DATA_BYTE = 15;

		const MODELS_COLUMN_MAP = 0;

		const MODELS_REVERSE_COLUMN_MAP = 1;

		protected $_lifetime;

		protected $_cache;

		/**
		 * \Phalcon\Mvc\Model\MetaData\Cache constructor
		 *
		 * @param array $options
		 */
		public function __construct($options=null){ }


		protected function _getCache(){ }


		/**
		 * Reads metadata from Cache
		 *
		 * @param  string $key
		 * @return array
		 */
		public function read($key){ }


		/**
		 *  Writes the metadata to Cache
		 *
		 * @param string $key
		 * @param array $data
		 */
		public function write($key, $data){ }


		public function reset(){ }

	}
}
