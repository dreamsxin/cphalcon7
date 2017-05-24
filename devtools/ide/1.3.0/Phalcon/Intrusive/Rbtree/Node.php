<?php 

namespace Phalcon\Intrusive\Rbtree {

	/**
	 * Phalcon\Intrusive\Rbtree\Node
	 *
	 * This class defines rbtree node entity
	 *
	 */
	
	class Node {

		protected $_value;

		/**
		 * \Phalcon\Intrusive\Rbtree\Node constructor
		 *
		 * @param mixed $value
		 */
		final public function __construct($value){ }


		/**
		 * Sets the value
		 *
		 * @param mixed $value
		 */
		public function setValue($value){ }


		/**
		 * Gets the value
		 *
		 * @return mixed
		 */
		public function getValue(){ }


		/**
		 * Gets prev node
		 *
		 * @return \Phalcon\Intrusive\Rbtree\Node
		 */
		public function prev(){ }


		/**
		 * Gets next node
		 *
		 * @return \Phalcon\Intrusive\Rbtree\Node
		 */
		public function next(){ }

	}
}
