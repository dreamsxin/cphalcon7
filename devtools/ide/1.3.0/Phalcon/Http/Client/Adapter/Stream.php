<?php 

namespace Phalcon\Http\Client\Adapter {

	/**
	 * Phalcon\Http\Client\Adapter\Stream
	 */
	
	class Stream extends \Phalcon\Http\Client\Adapter implements \Phalcon\Http\Client\AdapterInterface {

		const VERSION = 0.0.1;

		const AUTH_TYPE_ANY = any;

		const AUTH_TYPE_BASIC = basic;

		const AUTH_TYPE_DIGEST = digest;

		protected $_stream;

		public function __construct($uri, $method){ }


		protected function buildBody(){ }


		public function errorHandler($errno, $errstr, $errfile, $errline, $data){ }


		protected function sendInternal(){ }

	}
}
