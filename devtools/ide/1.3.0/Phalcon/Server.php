<?php 

namespace Phalcon {

	/**
	 * Phalcon\Server
	 *
	 * It‘s an implementation of the socket server
	 *</code>
	 */
	
	abstract class Server {

		/**
		 * \Phalcon\Server constructor
		 *
		 * @param array $config
		 * @throws \Phalcon\Server\Exception
		 */
		public function __construct($config){ }


		/**
		 * Run the Server
		 *
		 */
		public function start(){ }


		/**
		 * Close the socket
		 *
		 * @param int $fd
		 * @return boolean
		 */
		public function close($fd){ }


		/**
		 * Emitted when the socket has connected
		 *
		 * @param int $fd
		 */
		abstract public function onConnect($fd);


		/**
		 * Emitted when the socket has received
		 *
		 * @param int $fd
		 * @param string $data
		 */
		abstract public function onReceive($fd, $data);


		/**
		 * Emitted when the socket has send
		 *
		 * @param int $fd
		 * @param string $data
		 */
		abstract public function onSend($fd);


		/**
		 * Emitted when the socket has closed
		 *
		 * @param int $fd
		 */
		abstract public function onClose($fd);

	}
}
