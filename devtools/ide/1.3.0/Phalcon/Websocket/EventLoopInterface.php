<?php 

namespace Phalcon\Websocket {

	interface EventLoopInterface {

		public function add($fd, $flags);


		public function delete($fd);


		public function setMode($fd, $flags);


		public function lock();


		public function unlock();

	}
}
