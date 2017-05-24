<?php 

namespace Phalcon\Websocket {

	/**
	 * Phalcon\Websocket\Connection
	 *
	 */
	
	class Connection {

		/**
		 * Send data to the client
		 */
		public function send($text){ }


		/**
		 * Send data to the client as JSON string
		 */
		public function sendJson($payload){ }


		public function isConnected(){ }


		/**
		 * Get connection unique ID
		 */
		public function getUid(){ }


		/**
		 * Close connection to the client
		 */
		public function disconnect(){ }

	}
}
