<?php 

namespace Phalcon\Server {

	/**
	 * Phalcon\Server\Http
	 *
	 *<code>
	 *
	 *	$server = new Phalcon\Server\Http('127.0.0.1', 8989);
	 *  $server->start($application);
	 *
	 *</code>
	 */
	
	class Http {

		/**
		 * \Phalcon\Http\Server constructor
		 *
		 * @param array $config
		 * @throws \Phalcon\Server\Exception
		 */
		public function __construct($config=null){ }


		/**
		 * Run the Server
		 *
		 */
		public function start(\Phalcon\Application $application){ }

	}
}
