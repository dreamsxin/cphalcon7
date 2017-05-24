<?php 

namespace Phalcon\Mvc\Model {

	/**
	 * Phalcon\Mvc\Model\CriteriaInterface initializer
	 */
	
	interface CriteriaInterface {

		/**
		 * Set a model on which the query will be executed
		 *
		 * @param string $modelName
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function setModelName($modelName);


		/**
		 * Returns an internal model name on which the criteria will be applied
		 *
		 * @return string
		 */
		public function getModelName();


		/**
		 * Adds the bind parameter to the criteria
		 *
		 * @param array $bindParams
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function bind($bindParams, $merge=null);


		/**
		 * Sets the bind types in the criteria
		 * This method replaces all previously set bound parameters
		 *
		 * @param array $bindTypes
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function bindTypes($bindTypes, $merge=null);


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
		public function columns($columns);


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
		public function join($model, $conditions=null, $alias=null, $type=null);


		/**
		 * Adds the conditions parameter to the criteria
		 *
		 * @param string $conditions
		 * @param array $bindParams
		 * @param array $bindTypes
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function where($conditions, $bindParams=null, $bindTypes=null);


		/**
		 * Appends a condition to the current conditions using an AND operator
		 *
		 * @param string $conditions
		 * @param array $bindParams
		 * @param array $bindTypes
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function andWhere($conditions, $bindParams=null, $bindTypes=null);


		/**
		 * Appends a condition to the current conditions using an OR operator
		 *
		 * @param string $conditions
		 * @param array $bindParams
		 * @param array $bindTypes
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function orWhere($conditions, $bindParams=null, $bindTypes=null);


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
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function betweenWhere($expr, $minimum, $maximum, $useOrWhere=null);


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
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function notBetweenWhere($expr, $minimum, $maximum, $useOrWhere=null);


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
		public function inWhere($expr, $values, $useOrWhere=null);


		/**
		 * Appends a NOT IN condition to the current conditions
		 *
		 *<code>
		 *	$criteria->notInWhere('id', [1, 2, 3]);
		 *</code>
		 *
		 * @param string $expr
		 * @param array $values
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function notInWhere($expr, $values, $useOrWhere=null);


		/**
		 * Returns the conditions parameter in the criteria
		 *
		 * @return string
		 */
		public function getWhere();


		/**
		 * Adds the conditions parameter to the criteria
		 *
		 * @param string $conditions
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function conditions($conditions);


		/**
		 * Returns the columns to be queried
		 *
		 * @return string|array
		 */
		public function getColumns();


		/**
		 * Returns the conditions parameter in the criteria
		 *
		 * @return string
		 */
		public function getConditions();


		/**
		 * Adds the order-by parameter to the criteria
		 *
		 * @param string $orderColumns
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function orderBy($orderColumns);


		/**
		 * Returns the order parameter in the criteria
		 *
		 * @return string
		 */
		public function getOrder();


		/**
		 * Sets the limit parameter to the criteria
		 *
		 * @param int $limit
		 * @param int $offset
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function limit($limit, $offset=null);


		/**
		 * Returns the limit parameter in the criteria
		 *
		 * @return int
		 */
		public function getLimit();


		public function setUniqueRow($uniqueRow);


		public function getUniqueRow();


		/**
		 * Sets the "for_update" parameter to the criteria
		 *
		 * @param boolean $forUpdate
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function forUpdate($forUpdate=null);


		/**
		 * Sets the "shared_lock" parameter to the criteria
		 *
		 * @param boolean $sharedLock
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function sharedLock($sharedLock=null);


		/**
		 * Returns all the parameters defined in the criteria
		 *
		 * @return array
		 */
		public function getParams();


		/**
		 * Builds a \Phalcon\Mvc\Model\Criteria based on an input array like $_POST
		 *
		 * @param \Phalcon\DiInterface $dependencyInjector
		 * @param string $modelName
		 * @param array $data
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public static function fromInput(\Phalcon\DiInterface $dependencyInjector, $modelName, $data);


		/**
		 * Executes a find using the parameters built with the criteria
		 *
		 * @return \Phalcon\Mvc\Model\ResultsetInterface
		 */
		public function execute();


		/**
		 * Set columns for an insert
		 *
		 *<code>
		 *	$criteria->insert(array('name', 'type'), array(array('phalcon', 1), array('zephir', 2)));
		 *</code>
		 *
		 * @param array $columns
		 * @param array $values
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function insert();


		/**
		 * Set columns for an update
		 *
		 *<code>
		 *	$criteria->update(array('name' => 'phalcon'));
		 *</code>
		 *
		 * @param array $columns
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function update();


		/**
		 * Set the table for a delete.
		 *
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public function delete();


		/**
		 * Returns a PHQL statement built with the criteria
		 *
		 * @return String
		 */
		public function getPhql();

	}
}
