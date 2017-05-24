<?php 

namespace Phalcon\Intrusive {

	/**
	 * Phalcon\Intrusive\Rbtree
	 *
	 * This class defines rbtree entity and its description
	 *
	 */
	
	class Rbtree {

		protected $_rbtree;

		/**
		 * \Phalcon\Intrusive\Rbtree constructor
		 */
		final public function __construct(){ }


		/**
		 * Insert node value
		 *
		 * @param mixed value
		 * @return \Phalcon\Intrusive\Rbtree\Node
		 */
		public function insert($value){ }


		/**
		 * Remove node value
		 *
		 * @param mixed value
		 * @return boolean
		 */
		public function remove($value){ }


		/**
		 * Replace node value
		 *
		 * @param mixed oldValue
		 * @param mixed newValue
		 * @return \Phalcon\Intrusive\Rbtree\Node
		 */
		public function replace($oldValue, $newValue){ }


		/**
		 * Lookup node value
		 *
		 * @param mixed value
		 * @return \Phalcon\Intrusive\Rbtree\Node
		 */
		public function find($value){ }


		/**
		 * Returns first node value
		 *
		 * @return \Phalcon\Intrusive\Rbtree\Node
		 */
		public function first(){ }


		/**
		 * Returns last node value
		 *
		 * @return \Phalcon\Intrusive\Rbtree\Node
		 */
		public function last(){ }


		/**
		 * Returns prev node value
		 *
		 * @param mixed $value
		 * @return \Phalcon\Intrusive\Rbtree\Node
		 */
		public function prev($value){ }


		/**
		 * Returns next node value
		 *
		 * @param mixed $value
		 * @return \Phalcon\Intrusive\Rbtree\Node
		 */
		public function next($value){ }

	}
}
