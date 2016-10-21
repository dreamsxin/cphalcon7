<?php 

namespace Phalcon\Http\Client {

	/**
	 * Phalcon\Http\Client\Header
	 */
	
	class Header implements \Countable {

		const BUILD_STATUS = 1;

		const BUILD_FIELDS = 2;

		protected static $_messages;

		protected $_fields;

		protected $_version;

		protected $_status_code;

		protected $_status_message;

		protected $_status;

		/**
		 * \Phalcon\Http\Client\Header constructor
		 */
		public function __construct(){ }


		public function set($name, $value){ }


		public function setStatusCode($value){ }


		public function setMultiple($values){ }


		public function addMultiple($values){ }


		public function get($name){ }


		public function getStatusCode(){ }


		public function getAll(){ }


		public function remove($name){ }


		public function parse($content){ }


		public function build($flags=null){ }


		public function count(){ }

	}
}
