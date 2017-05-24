<?php 

namespace Phalcon\Intrusive\Avltree {

	/**
	 * Phalcon\Intrusive\Avltree\Node
	 *
	 * This class defines avltree node entity
	 *
	 */
	
	class Node {

		protected $_value;

		/**
		 * \Phalcon\Intrusive\Avltree\Node constructor
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
		 * @return \Phalcon\Intrusive\Avltree\Node
		 */
		public function prev(){ }


		/**
		 * Gets next node
		 *
		 * @return \Phalcon\Intrusive\Avltree\Node
		 */
		public function next(){ }

	}
}
