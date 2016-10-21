<?php 

namespace Phalcon\Mvc\Model\MetaData {

	/**
	 * Phalcon\Mvc\Model\MetaData\Libmemcached
	 *
	 * Stores model meta-data in the Libmemcached cache. Data will erased if the web server is restarted
	 *
	 * By default meta-data is stored for 48 hours (172800 seconds)
	 *
	 * You can query the meta-data by printing libmemcached_get('$PMM$') or libmemcached_get('$PMM$my-app-id')
	 *
	 *<code>
	 *	$metaData = new Phalcon\Mvc\Model\Metadata\Libmemcached(array(
	 *     'servers' => array(
	 *         array('host' => 'localhost', 'port' => 11211, 'weight' => 1),
	 *     ),
	 *     'client' => array(
	 *         Memcached::OPT_HASH => Memcached::HASH_MD5,
	 *         Memcached::OPT_PREFIX_KEY => 'prefix.',
	 *     ),
	 *		'prefix' => 'my-app-id',
	 *		'lifetime' => 86400
	 *	));
	 *</code>
	 */
	
	class Libmemcached extends \Phalcon\Mvc\Model\MetaData implements \Phalcon\Mvc\Model\MetaDataInterface, \Phalcon\DI\InjectionAwareInterface, \Phalcon\Events\EventsAwareInterface {

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

		protected $_libmemcached;

		/**
		 * \Phalcon\Mvc\Model\MetaData\Libmemcached constructor
		 *
		 * @param array $options
		 */
		public function __construct($options=null){ }


		/**
		 * Reads metadata from Libmemcached
		 *
		 * @param  string $key
		 * @return array
		 */
		public function read($key){ }


		/**
		 *  Writes the metadata to Libmemcached
		 *
		 * @param string $key
		 * @param array $data
		 */
		public function write($key, $data){ }


		public function reset(){ }

	}
}
