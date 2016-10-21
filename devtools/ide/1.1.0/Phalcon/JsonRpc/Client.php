<?php 

namespace Phalcon\JsonRpc {

	/**
	 * Phalcon\JsonRpc\Client
	 *
	 *<code>
	 *	$httpclient = new Phalcon\Http\Client\Adapter\Stream('http://rpc.edu.local');
	 *	$rpc = new Phalcon\JsonRpc\Client($httpclient);
	 *	$rpc->call('auth/sigup', array('username' => 'phalcon', 'password' => 'Hello:)'));
	 *</code>
	 *
	 */
	
	class Client {

		protected $_httpclient;

		protected $_id;

		public function __construct($httpclient){ }


		/**
		 * Rpc call
		 *
		 * @param string $method
		 * @param string $data
		 * @return \Phalcon\JsonRpc\Response
		 */
		public function call($method, $params=null){ }

	}
}
