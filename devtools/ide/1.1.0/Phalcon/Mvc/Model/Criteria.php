<?php 

namespace Phalcon\Mvc\Model {

	/**
	 * Phalcon\Mvc\Model\Criteria
	 *
	 * This class allows to build the array parameter required by Phalcon\Mvc\Model::find
	 * and Phalcon\Mvc\Model::findFirst using an object-oriented interface
	 *
	 *<code>
	 *$robots = Robots::query()
	 *    ->where("type = :type:")
	 *    ->andWhere("year < 2000")
	 *    ->bind(array("type" => "mechanical"))
	 *    ->order("name")
	 *    ->execute();
	 *</code>
	 */
	
	class Criteria extends \Phalcon\Di\Injectable implements \Phalcon\Events\EventsAwareInterface, \Phalcon\Di\InjectionAwareInterface, \Phalcon\Mvc\Model\CriteriaInterface {

		protected $_type;

		protected $_model;

		protected $_bindParams;

		protected $_bindTypes;

		protected $_columns;

		protected $_values;

		protected $_joins;

		protected $_conditions;

		protected $_order;

		protected $_limit;

		protected $_offset;

		protected $_forUpdate;

		protected $_sharedLock;

		protected $_hiddenParamNumber;

		protected $_cacheOptions;

		protected $_group;

		protected $_having;

		protected $_uniqueRow;

		/**
		 * Set a model on which the query will be executed
		 *
		 * @param string $modelName
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function setModelName($modelName){ }


		/**
		 * Returns an internal model name on which the criteria will be applied
		 *
		 * @return string
		 */
		public function getModelName(){ }


		/**
		 * Sets the bound parameters in the criteria
		 * This method replaces all previously set bound parameters
		 *
		 * @param string $bindParams
		 * @param boolean $merge
		 *
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function bind($bindParams, $merge=null){ }


		/**
		 * Sets the bind types in the criteria
		 * This method replaces all previously set bound parameters
		 *
		 * @param array $bindTypes
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function bindTypes($bindTypes, $merge=null){ }


		public function select($columns){ }


		/**
		 * Sets the columns to be queried
		 *
		 *<code>
		 *	$criteria->columns(array('id', 'name'));
		 *</code>
		 *
		 * @param string|array $columns
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function columns($columns){ }


		/**
		 * Returns the columns to be queried
		 *
		 * @return string|array
		 */
		public function getColumns(){ }


		/**
		 * Adds a join to the query
		 *
		 *<code>
		 *	$criteria->join('Robots');
		 *	$criteria->join('Robots', 'r.id = RobotsParts.robots_id');
		 *	$criteria->join('Robots', 'r.id = RobotsParts.robots_id', 'r');
		 *	$criteria->join('Robots', 'r.id = RobotsParts.robots_id', 'r', 'LEFT');
		 *</code>
		 *
		 * @param string $model
		 * @param string $conditions
		 * @param string $alias
		 * @param string $type
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function join($model, $conditions=null, $alias=null, $type=null){ }


		/**
		 * Adds a INNER join to the query
		 *
		 *<code>
		 *	$criteria->innerJoin('Robots');
		 *	$criteria->innerJoin('Robots', 'r.id = RobotsParts.robots_id');
		 *	$criteria->innerJoin('Robots', 'r.id = RobotsParts.robots_id', 'r');
		 *	$criteria->innerJoin('Robots', 'r.id = RobotsParts.robots_id', 'r', 'LEFT');
		 *</code>
		 *
		 * @param string $model
		 * @param string $conditions
		 * @param string $alias
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function innerJoin($model, $conditions=null, $alias=null){ }


		/**
		 * Adds a LEFT join to the query
		 *
		 *<code>
		 *	$criteria->leftJoin('Robots', 'r.id = RobotsParts.robots_id', 'r');
		 *</code>
		 *
		 * @param string $model
		 * @param string $conditions
		 * @param string $alias
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function leftJoin($model, $conditions=null, $alias=null){ }


		/**
		 * Adds a RIGHT join to the query
		 *
		 *<code>
		 *	$criteria->rightJoin('Robots', 'r.id = RobotsParts.robots_id', 'r');
		 *</code>
		 *
		 * @param string $model
		 * @param string $conditions
		 * @param string $alias
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function rightJoin($model, $conditions=null, $alias=null){ }


		/**
		 * Sets the conditions parameter in the criteria
		 *
		 * @param string $conditions
		 * @param array $bindParams
		 * @param array $bindTypes
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function where($conditions, $bindParams=null, $bindTypes=null){ }


		/**
		 * Appends a condition to the current conditions using an AND operator (deprecated)
		 *
		 * @deprecated 1.0.0
		 * @see \Phalcon\Mvc\Model\Criteria::andWhere()
		 * @param string $conditions
		 * @param array $bindParams
		 * @param array $bindTypes
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function addWhere($conditions, $bindParams=null, $bindTypes=null){ }


		/**
		 * Appends a condition to the current conditions using an AND operator
		 *
		 * @param string $conditions
		 * @param array $bindParams
		 * @param array $bindTypes
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function andWhere($conditions, $bindParams=null, $bindTypes=null){ }


		/**
		 * Appends a condition to the current conditions using an OR operator
		 *
		 * @param string $conditions
		 * @param array $bindParams
		 * @param array $bindTypes
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function orWhere($conditions, $bindParams=null, $bindTypes=null){ }


		/**
		 * Appends a BETWEEN condition to the current conditions
		 *
		 *<code>
		 *	$criteria->betweenWhere('price', 100.25, 200.50);
		 *</code>
		 *
		 * @param string $expr
		 * @param mixed $minimum
		 * @param mixed $maximum
		 * @param boolean $useOrWhere
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function betweenWhere($expr, $minimum, $maximum, $useOrWhere=null){ }


		/**
		 * Appends a NOT BETWEEN condition to the current conditions
		 *
		 *<code>
		 *	$criteria->notBetweenWhere('price', 100.25, 200.50);
		 *</code>
		 *
		 * @param string $expr
		 * @param mixed $minimum
		 * @param mixed $maximum
		 * @param boolean $useOrWhere
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function notBetweenWhere($expr, $minimum, $maximum, $useOrWhere=null){ }


		/**
		 * Appends an IN condition to the current conditions
		 *
		 *<code>
		 *	$criteria->inWhere('id', [1, 2, 3]);
		 *</code>
		 *
		 * @param string $expr
		 * @param array $values
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function inWhere($expr, $values, $useOrWhere=null){ }


		/**
		 * Appends a NOT IN condition to the current conditions
		 *
		 *<code>
		 *	$criteria->notInWhere('id', [1, 2, 3]);
		 *</code>
		 *
		 * @param string $expr
		 * @param array $values
		 * @param boolean $useOrWhere
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function notInWhere($expr, $values, $useOrWhere=null){ }


		/**
		 * Returns the conditions parameter in the criteria
		 *
		 * @return string
		 */
		public function getWhere(){ }


		/**
		 * Adds the conditions parameter to the criteria
		 *
		 * @param string $conditions
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function conditions($conditions){ }


		/**
		 * Returns the conditions parameter in the criteria
		 *
		 * @return string
		 */
		public function getConditions(){ }


		/**
		 * Adds the order-by parameter to the criteria (deprecated)
		 *
		 * @deprecated 1.2.1
		 * @see \Phalcon\Mvc\Model\Criteria::orderBy()
		 * @param string $orderColumns
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function order($orderColumns){ }


		/**
		 * Adds the order-by parameter to the criteria
		 *
		 * @param string $orderColumns
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function orderBy($orderColumns){ }


		/**
		 * Returns the order parameter in the criteria
		 *
		 * @return string
		 */
		public function getOrder(){ }


		/**
		 * Adds the limit parameter to the criteria
		 *
		 * @param int $limit
		 * @param int $offset
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function limit($limit, $offset=null){ }


		/**
		 * Returns the limit parameter in the criteria
		 *
		 * @return int
		 */
		public function getLimit(){ }


		/**
		 * Tells to the query if only the first row in the resultset must be returned
		 *
		 * @param boolean $uniqueRow
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function setUniqueRow($uniqueRow){ }


		/**
		 * Check if the query is programmed to get only the first row in the resultset
		 *
		 * @return boolean
		 */
		public function getUniqueRow(){ }


		/**
		 * Adds the "for_update" parameter to the criteria
		 *
		 * @param boolean $forUpdate
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function forUpdate($forUpdate=null){ }


		/**
		 * Adds the "shared_lock" parameter to the criteria
		 *
		 * @param boolean $sharedLock
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function sharedLock($sharedLock=null){ }


		/**
		 * Returns all the parameters defined in the criteria
		 *
		 * @return array
		 */
		public function getParams(){ }


		/**
		 * Builds a \Phalcon\Mvc\Model\Criteria based on an input array like $_POST
		 *
		 * @param \Phalcon\DiInterface $dependencyInjector
		 * @param string $modelName
		 * @param array $data
		 * @return \Phalcon\Mvc\Model\Criteria
		 */
		public static function fromInput(\Phalcon\DiInterface $dependencyInjector, $modelName, $data){ }


		/**
		 * Sets a GROUP BY clause
		 *
		 * @param string $group
		 * @return \Phalcon\Mvc\Model\Criteria
		 */
		public static function groupBy($group){ }


		/**
		 * Sets a HAVING condition clause. You need to escape PHQL reserved words using [ and ] delimiters
		 *
		 * @param string $having
		 * @return \Phalcon\Mvc\Model\Criteria
		 */
		public function having($having){ }


		/**
		 * Executes a find using the parameters built with the criteria
		 *
		 * @param boolean $useRawsql
		 * @return \Phalcon\Mvc\Model\ResultsetInterface
		 */
		public function execute($useRawsql){ }


		/**
		 * Sets the cache options in the criteria
		 * This method replaces all previously set cache options
		 *
		 * @param array $options
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function cache($options){ }


		/**
		 * Sets insert type of PHQL statement to be executed
		 *
		 * @param array $columns
		 * @param array $values
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function insert(){ }


		/**
		 * Sets update type of PHQL statement to be executed
		 *
		 * @param array $columns
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function update(){ }


		/**
		 * Sets update type of PHQL statement to be executed
		 *
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function delete(){ }


		/**
		 * Returns a PHQL statement built with the criteria
		 *
		 * @return string
		 */
		public function getPhql(){ }


		/**
		 * Returns a PHQL statement built with the criteria
		 *
		 * @return string
		 */
		public function _generateSelect(){ }


		/**
		 * Returns a PHQL statement built with the criteria
		 *
		 * @return string
		 */
		public function _generateInsert(){ }


		/**
		 * Returns a PHQL statement built with the criteria
		 *
		 * @return string
		 */
		public function _generateUpdate(){ }


		/**
		 * Returns a PHQL statement built with the criteria
		 *
		 * @return string
		 */
		public function _generateDelete(){ }

	}
}
