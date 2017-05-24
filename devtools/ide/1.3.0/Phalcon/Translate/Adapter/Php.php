<?php 

namespace Phalcon\Translate\Adapter {

	/**
	 * Phalcon\Translate\Adapter\Php
	 *
	 * Allows to define translation lists using PHP file
	 *
	 *<code>
	 *  $t = new \Phalcon\Translate\Adapter\Php(array(
	 *      'locale' => 'en_US.utf8',
	 *      'directory' => __DIR__ . DIRECTORY_SEPARATOR . 'locale'
	 * ));
	 *</code>
	 */
	
	class Php extends \Phalcon\Translate\Adapter implements \Phalcon\Translate\AdapterInterface, \ArrayAccess {

		protected $_translate;

		/**
		 * \Phalcon\Translate\Adapter\Php constructor
		 *
		 * @param array $options
		 */
		public function __construct($options){ }


		/**
		 * Returns the translation related to the given key
		 *
		 * @param string $index
		 * @param array $placeholders
		 * @return string
		 */
		public function query($index, $placeholders=null){ }


		/**
		 * Check whether is defined a translation key in the internal array
		 *
		 * @param string $index
		 * @return boolean
		 */
		public function exists($index){ }

	}
}
