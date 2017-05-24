<?php 

namespace Phalcon\Mvc\Model\Query\Builder {

	/**
	 * Phalcon\Mvc\Model\Query\Builder\Insert
	 *
	 *<code>
	 *$resultset = Phalcon\Mvc\Model\Query\Builder::createInsertBuilder()
	 *   ->table('Robots')
	 *   ->columns(array('name'))
	 *   ->values(array(array('name' => 'Google'), array('name' => 'Baidu')))
	 *   ->getQuery()
	 *   ->execute();
	 *</code>
	 */
	
	class Insert extends \Phalcon\Mvc\Model\Query\Builder implements \Phalcon\Mvc\Model\Query\BuilderInterface, \Phalcon\Di\InjectionAwareInterface, \Phalcon\Events\EventsAwareInterface {

		protected $_type;

		protected $_table;

		protected $_columns;

		protected $_values;

		/**
		 * \Phalcon\Mvc\Model\Query\Builder\Insert constructor
		 *
		 * @param array $params
		 * @param \Phalcon\Di $dependencyInjector
		 */
		public function __construct($params=null){ }


		/**
		 * Sets the table to insert into
		 *
		 * @param string table
		 * @return \Phalcon\Mvc\Model\Query\Builder\Insert
		 */
		public function table($table){ }


		/**
		 * Gets the table to insert into
		 *
		 * @return boolean
		 */
		public function getTable(){ }


		/**
		 * Set the columns that will be inserted
		 *
		 * @param array $columns
		 * @return \Phalcon\Mvc\Model\Query\Builder\Insert
		 */
		public function columns($columns){ }


		/**
		 * Gets the columns that will be inserted
		 *
		 * @return string|array
		 */
		public function getColumns(){ }


		/**
		 * Sets the values to insert
		 *
		 * @param array $values
		 * @return \Phalcon\Mvc\Model\Query\Builder\Insert
		 */
		public function values($values){ }


		/**
		 * Gets the values to insert
		 *
		 * @return array
		 */
		public function getValues(){ }


		/**
		 * Returns a PHQL statement built based on the builder parameters
		 *
		 * @return string
		 */
		protected function _compile(){ }

	}
}
