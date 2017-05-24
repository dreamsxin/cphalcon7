<?php 

namespace Phalcon\Websocket {

	/**
	 * Phalcon\Websocket\Server
	 *
	 *<code>
	 * $server = new Phalcon\Websocket\Server(8080);
	 * $server->on(Phalcon\Websocket\Server::ON_ACCEPT, function($server, $conn){
	 *     echo 'Accept'.PHP_EOL;
	 * });
	 * $server->on(Phalcon\Websocket\Server::ON_CLOSE, function($server){
	 *     echo 'Close'.PHP_EOL;
	 * });
	 * $server->on(Phalcon\Websocket\Server::ON_DATA, function($server, $conn, $data){
	 *     echo 'Data'.PHP_EOL;
	 * });
	 * $server->run();
	 *<／code>
	 */
	
	class Server {

		const ON_ACCEPT = 0;

		const ON_CLOSE = 1;

		const ON_DATA = 2;

		const ON_TICK = 3;

		/**
		 * \Phalcon\Websocket\Server constructor
		 *
		 * @param int $port
		 */
		public function __construct($port=null){ }


		/**
		 * Set external event loop
		 *
		 * @param \Phalcon\Websocket\Eventloop $eventloop
		 */
		public function setEventLoop(\Phalcon\Websocket\EventLoopInterface $eventloop){ }


		/**
		 * Service a socket (used with external event loop)
		 */
		public function serviceFd($fd, $events){ }


		/**
		 * Launch WebSocket server
		 */
		public function run(){ }


		/**
		 * Stop WebSocket server
		 */
		public function stop(){ }


		/**
		 * Register a callback for specified event
		 */
		public function on($event, $callback){ }


		/**
		 * Broadcast a message to all connected clients
		 */
		public function broadcast($text, $ignored=null){ }

	}
}
