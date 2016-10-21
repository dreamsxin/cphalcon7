<?php 

namespace Phalcon\Http\Client\Adapter {

	/**
	 * Phalcon\Http\Client\Adapter\Curl
	 */
	
	class Curl extends \Phalcon\Http\Client\Adapter implements \Phalcon\Http\Client\AdapterInterface {

		const VERSION = 0.0.1;

		const AUTH_TYPE_ANY = any;

		const AUTH_TYPE_BASIC = basic;

		const AUTH_TYPE_DIGEST = digest;

		protected $_curl;

		public function __construct($uri, $method){ }


		protected function sendInternal(){ }

	}
}
