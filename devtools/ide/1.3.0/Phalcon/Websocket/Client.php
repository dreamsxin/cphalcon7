<?php 

namespace Phalcon\Websocket {

	/**
	 * Phalcon\Websocket\Client
	 *
	 *<code>
	 * $client = new Phalcon\Websocket\Client('127.0.0.1', 8080);
	 * $client->on(Phalcon\Websocket\Client::ON_ACCEPT, function($client, $conn){
	 *     echo 'Accept'.PHP_EOL;
	 * });
	 * $client->on(Phalcon\Websocket\Client::ON_CLOSE, function(){
	 *     echo 'Close'.PHP_EOL;
	 * });
	 * $client->on(Phalcon\Websocket\Client::ON_DATA, function($client, $conn){
	 *     echo 'Data'.PHP_EOL;
	 * });
	 * $client->connect();
	 *<／code>
	 */
	
	class Client {

		const ON_ACCEPT = 0;

		const ON_CLOSE = 1;

		const ON_DATA = 2;

		const ON_TICK = 3;

		protected $_host;

		protected $_port;

		protected $_path;

		/**
		 * \Phalcon\Websocket\Client constructor
		 *
		 * @param string $host
		 * @param int $port
		 */
		public function __construct($host, $port=null, $path=null){ }


		/**
		 * Register a callback for specified event
		 */
		public function on($event, $callback){ }


		/**
		 * Establish connection with remote server
		 */
		public function connect($accept=null, $close=null, $data=null, $tick=null){ }


		/**
		 * Send data to the client
		 */
		public function send($text){ }


		/**
		 * Send data to the client as JSON string
		 */
		public function sendJson($payload){ }


		/**
		 * Check is connection is established
		 */
		public function isConnected(){ }


		/**
		 * Close connection to the client
		 */
		public function disconnect(){ }

	}
}
