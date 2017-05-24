<?php 

namespace Phalcon\Events {

	/**
	 * Phalcon\Events\EventInterface initializer
	 */
	
	interface EventInterface {

		/**
		 * Sets event type
		 */
		public function setType($eventType);


		/**
		 * Gets event type
		 */
		public function getType();


		/**
		 * Sets event source
		 */
		public function setSource($source);


		/**
		 * Gets event source
		 */
		public function getSource();


		/**
		 * Sets event data
		 */
		public function setData($data);


		/**
		 * Gets event data
		 */
		public function getData();


		/**
		 * Sets event is cancelable
		 */
		public function setCancelable($cancelable);


		/**
		 * Check whether the event is cancelable
		 */
		public function isCancelable();


		/**
		 * Stops the event preventing propagation
		 */
		public function stop();


		/**
		 * Check whether the event is currently stopped
		 */
		public function isStopped();

	}
}
