<?php 

namespace Phalcon\Events {

	/**
	 * Phalcon\Events\Listener
	 *
	 * This class offers contextual information of a fired event in the EventsManager
	 */
	
	class Listener {

		protected $_listener;

		protected $_priority;

		protected $_event;

		/**
		 * \Phalcon\Events\Listener constructor
		 *
		 * @param callable $listener
		 * @param mixed $priority
		 */
		public function __construct($listener, $priority){ }


		public function setListener($listener){ }


		public function getListener(){ }


		public function setPriority($priority){ }


		public function getPriority(){ }


		public function setEvent($event){ }


		public function getEvent(){ }

	}
}
