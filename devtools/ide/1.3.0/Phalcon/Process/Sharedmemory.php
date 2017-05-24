<?php 

namespace Phalcon\Process {

	/**
	 * Phalcon\Process\Sharedmemory
	 *
	 * This class defines sharedmemory entity and its description
	 *
	 */
	
	class Sharedmemory {

		protected $_name;

		protected $_shm;

		/**
		 * \Phalcon\Process\Sharedmemory constructor
		 */
		final public function __construct($name){ }


		/**
		 * Checks if open a shared memory file
		 */
		public function isOpen(){ }


		/**
		 * Gets the name
		 */
		public function getName(){ }


		/**
		 * Gets the size
		 */
		public function getSize(){ }


		/**
		 * Open a shared memory
		 */
		public function open(){ }


		/**
		 * Create a shared memory
		 */
		public function create($size){ }


		/**
		 * Lock the shared memory
		 */
		public function lock($blocking=null){ }


		/**
		 * Unock the shared memory
		 */
		public function unlock($force=null){ }


		/**
		 * Read the shared memory
		 */
		public function read(){ }


		/**
		 * Write the shared memory
		 */
		public function write($value){ }

	}
}
