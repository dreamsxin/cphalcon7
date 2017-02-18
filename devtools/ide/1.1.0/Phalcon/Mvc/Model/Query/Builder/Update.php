<?php 

namespace Phalcon\Mvc\Model\Query\Builder {

	/**
	 * Phalcon\Mvc\Model\Query\Builder\Update
	 *
	 *<code>
	 *$resultset = Phalcon\Mvc\Model\Query\Builder::createUpdateBuilder()
	 *   ->table('Robots')
	 *   ->set(array('name' => 'Google'))
	 *   ->getQuery()
	 *   ->execute();
	 *</code>
	 */
	
	class Update extends \Phalcon\Mvc\Model\Query\Builder\Where implements \Phalcon\Events\EventsAwareInterface, \Phalcon\Di\InjectionAwareInterface, \Phalcon\Mvc\Model\Query\BuilderInterface {

		protected $_type;

		protected $_table;

		protected $_set;

		/**
		 * \Phalcon\Mvc\Model\Query\Builder\Update constructor
		 *
		 * @param array $params
		 * @param \Phalcon\Di $dependencyInjector
		 */
		public function __construct($params=null){ }


		/**
		 * Sets the table to update
		 *
		 * @param string table
		 * @return \Phalcon\Mvc\Model\Query\Builder\Update
		 */
		public function table($table){ }


		/**
		 * Gets the table to update
		 *
		 * @return string
		 */
		public function getTable(){ }


		/**
		 * Sets the values to update with an associative array
		 *
		 *<code>
		 *	$builder->set(array('id' => 1, 'name' => 'Google'));
		 *</code>
		 *
		 * @param string|array $set
		 * @return \Phalcon\Mvc\Model\Query\Builder\Update
		 */
		public function set($set){ }


		/**
		 * Return the values to update with an associative array
		 *
		 * @return string|array
		 */
		public function getSet(){ }


		/**
		 * Returns a PHQL statement built based on the builder parameters
		 *
		 * @return string
		 */
		protected function _compile(){ }

	}
}
