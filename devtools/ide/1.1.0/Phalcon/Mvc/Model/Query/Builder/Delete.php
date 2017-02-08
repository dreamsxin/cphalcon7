<?php 

namespace Phalcon\Mvc\Model\Query\Builder {

	/**
	 * Phalcon\Mvc\Model\Query\Builder\Delete
	 *
	 *<code>
	 *$resultset = Phalcon\Mvc\Model\Query\Builder::createDeleteBuilder()
	 *   ->table('Robots')
	 *   ->where('name = "Peter"')
	 *   ->orderBy('Robots.id')
	 *   ->limit(20)
	 *   ->execute();
	 *</code>
	 */
	
	class Delete extends \Phalcon\Mvc\Model\Query\Builder\Where implements \Phalcon\Events\EventsAwareInterface, \Phalcon\Di\InjectionAwareInterface, \Phalcon\Mvc\Model\Query\BuilderInterface {

		protected $_type;

		protected $_table;

		protected $_columns;

		protected $_values;

		/**
		 * \Phalcon\Mvc\Model\Query\Builder\Delete constructor
		 *
		 * @param array $params
		 * @param \Phalcon\Di $dependencyInjector
		 */
		public function __construct($params=null){ }


		/**
		 * Sets the table to delete from
		 *
		 * @param string table
		 * @return \Phalcon\Mvc\Model\Query\Builder\Delete
		 */
		public function table($table){ }


		/**
		 * Gets the table to delete from
		 *
		 * @return bool
		 */
		public function getTable(){ }


		/**
		 * Returns a PHQL statement built based on the builder parameters
		 *
		 * @return string
		 */
		protected function _compile(){ }

	}
}
