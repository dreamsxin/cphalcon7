<?php 

namespace Phalcon\Mvc\Model\Query {

	/**
	 * Phalcon\Mvc\Model\Query\Builder
	 *
	 * Helps to create PHQL queries using an OO interface
	 *
	 *<code>
	 *$resultset = Phalcon\Mvc\Model\Query\Builder::create(Phalcon\Mvc\Model\Query::TYPE_SELECT)
	 *   ->from('Robots')
	 *   ->join('RobotsParts')
	 *   ->limit(20)
	 *   ->orderBy('Robots.name')
	 *   ->getQuery()
	 *   ->execute();
	 *</code>
	 */
	
	abstract class Builder extends \Phalcon\Di\Injectable implements \Phalcon\Events\EventsAwareInterface, \Phalcon\Di\InjectionAwareInterface, \Phalcon\Mvc\Model\Query\BuilderInterface {

		protected $_type;

		protected $_bindParams;

		protected $_bindTypes;

		protected $_mergeBindParams;

		protected $_mergeBindTypes;

		protected $_phql;

		protected $_hiddenParamNumber;

		/**
		 * Create a new Query Builder of the given type.
		 *
		 *<code>
		 *	Phalcon\Mvc\Model\Query\Builder::create(Phalcon\Mvc\Model\Query::TYPE_SELECT);
		 *</code>
		 *
		 * @return \Phalcon\Mvc\Model\Query\Builder
		 */
		public static function create($type){ }


		/**
		 * Create a new Query Builder for Select
		 *
		 *
		 * @return \Phalcon\Mvc\Model\Query\Builder\Select
		 */
		public static function createSelectBuilder($params=null, \Phalcon\DiInterface $dependencyInjector=null){ }


		/**
		 * Create a new Query Builder for Insert
		 *
		 *
		 * @return \Phalcon\Mvc\Model\Query\Builder\Insert
		 */
		public static function createInsertBuilder($params=null, \Phalcon\DiInterface $dependencyInjector=null){ }


		/**
		 * Create a new Query Builder for Update
		 *
		 *
		 * @return \Phalcon\Mvc\Model\Query\Builder\Update
		 */
		public static function createUpdateBuilder($params=null, \Phalcon\DiInterface $dependencyInjector=null){ }


		/**
		 * Create a new Query Builder for Delete
		 *
		 *
		 * @return \Phalcon\Mvc\Model\Query\Builder\Delete
		 */
		public static function createDeleteBuilder($params=null, \Phalcon\DiInterface $dependencyInjector=null){ }


		/**
		 * Gets the type of PHQL queries
		 *
		 *
		 * @return int
		 */
		public function getType(){ }


		/**
		 * Sets the bind parameters
		 *
		 * @param array $bindParams
		 * @return \Phalcon\Mvc\Model\Query\Builder
		 */
		public function setBindParams($bindparams, $merge=null){ }


		/**
		 * Gets the bind parameters
		 *
		 * @return \Phalcon\Mvc\Model\Query\Builder
		 */
		public function getBindParams(){ }


		/**
		 * Gets the merge bind parameters
		 *
		 * @return \Phalcon\Mvc\Model\Query\Builder
		 */
		public function getMergeBindParams(){ }


		/**
		 * Sets the bind types
		 *
		 * @param array $bindTypes
		 * @return \Phalcon\Mvc\Model\Query\Builder
		 */
		public function setBindTypes($bindtypes, $merge=null){ }


		/**
		 * Gets the bind types
		 *
		 * @return \Phalcon\Mvc\Model\Query\Builder
		 */
		public function getBindTypes(){ }


		/**
		 * Gets the merge bind types
		 *
		 * @return \Phalcon\Mvc\Model\Query\Builder
		 */
		public function getMergeBindTypes(){ }


		/**
		 * Compile the PHQL query
		 *
		 * @return \Phalcon\Mvc\Model\Query\Builder
		 */
		public function compile(){ }


		/**
		 * Returns a PHQL statement built based on the builder parameters
		 *
		 * @return string
		 */
		public function getPhql(){ }


		/**
		 * Returns the query built
		 *
		 * @return \Phalcon\Mvc\Model\Query
		 */
		public function getQuery(){ }

	}
}
