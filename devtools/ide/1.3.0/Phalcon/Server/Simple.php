<?php 

namespace Phalcon\Server {

	/**
	 * Phalcon\Server\Simple
	 *
	 *<code>
	 * class App extends Phalcon\Application {
	 *     public function handle($data = NULL) {
	 *         $data = trim($data);
	 *         if (empty($data)) {
	 *             return 'Please input'.PHP_EOL;
	 *         } else {
	 *             return '>>> '.$data.PHP_EOL;
	 *         }
	 *    }
	 * }
	 * $server = new Phalcon\Server\Simple();
	 * $server->start(new App);
	 *
	 *</code>
	 */
	
	class Simple {

		/**
		 * \Phalcon\Server\Simple constructor
		 *
		 * @param string $host
		 * @param int $port
		 * @throws \Phalcon\Server\Exception
		 */
		public function __construct($host=null, $port=null){ }


		/**
		 * Run the Server
		 *
		 */
		public function start(\Phalcon\Application $application=null){ }

	}
}
