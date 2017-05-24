<?php 

namespace Phalcon\Intrusive {

	/**
	 * Phalcon\Intrusive\Avltree
	 *
	 * This class defines avltree entity and its description
	 *
	 */
	
	class Avltree {

		protected $_avltree;

		/**
		 * \Phalcon\Intrusive\Avltree constructor
		 */
		final public function __construct(){ }


		/**
		 * Insert node value
		 *
		 * @param mixed value
		 * @return \Phalcon\Intrusive\Avltree\Node
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
		 * @return \Phalcon\Intrusive\Avltree\Node
		 */
		public function replace($oldValue, $newValue){ }


		/**
		 * Lookup node value
		 *
		 * @param mixed value
		 * @return \Phalcon\Intrusive\Avltree\Node
		 */
		public function find($value){ }


		/**
		 * Returns first node value
		 *
		 * @return \Phalcon\Intrusive\Avltree\Node
		 */
		public function first(){ }


		/**
		 * Returns last node value
		 *
		 * @return \Phalcon\Intrusive\Avltree\Node
		 */
		public function last(){ }


		/**
		 * Returns prev node value
		 *
		 * @param mixed $value
		 * @return \Phalcon\Intrusive\Avltree\Node
		 */
		public function prev($value){ }


		/**
		 * Returns next node value
		 *
		 * @param mixed $value
		 * @return \Phalcon\Intrusive\Avltree\Node
		 */
		public function next($value){ }

	}
}
