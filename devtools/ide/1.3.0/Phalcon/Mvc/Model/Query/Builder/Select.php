<?php 

namespace Phalcon\Mvc\Model\Query\Builder {

	/**
	 * Phalcon\Mvc\Model\Query\Builder\Select
	 *
	 *<code>
	 *$resultset = $this->modelsManager->createBuilder()
	 *   ->from('Robots')
	 *   ->join('RobotsParts')
	 *   ->limit(20)
	 *   ->orderBy('Robots.name')
	 *   ->getQuery()
	 *   ->execute();
	 *</code>
	 */
	
	class Select extends \Phalcon\Mvc\Model\Query\Builder\Join implements \Phalcon\Mvc\Model\Query\BuilderInterface, \Phalcon\Di\InjectionAwareInterface, \Phalcon\Events\EventsAwareInterface {

		protected $_type;

		protected $_columns;

		protected $_models;

		protected $_group;

		protected $_having;

		protected $_order;

		protected $_limit;

		protected $_offset;

		protected $_forUpdate;

		protected $_sharedLock;

		protected $_distinct;

		/**
		 * \Phalcon\Mvc\Model\Query\Builder\Select constructor
		 *
		 *<code>
		 * $params = array(
		 *    'models'     => array('Users'),
		 *    'columns'    => array('id', 'name', 'status'),
		 *    'conditions' => array(
		 *        array(
		 *            "created > :min: AND created < :max:",
		 *            array("min" => '2013-01-01',   'max' => '2014-01-01'),
		 *            array("min" => PDO::PARAM_STR, 'max' => PDO::PARAM_STR),
		 *        ),
		 *    ),
		 *    // or 'conditions' => "created > '2013-01-01' AND created < '2014-01-01'",
		 *    'group'      => array('id', 'name'),
		 *    'having'     => "name = 'Kamil'",
		 *    'order'      => array('name', 'id'),
		 *    'limit'      => 20,
		 *    'offset'     => 20,
		 *    // or 'limit' => array(20, 20),
		 *);
		 *$queryBuilder = new \Phalcon\Mvc\Model\Query\Builder\Select($params);
		 *</code>
		 *
		 * @param array $params
		 * @param \Phalcon\Di $dependencyInjector
		 */
		public function __construct($params=null){ }


		/**
		 * Sets SELECT DISTINCT / SELECT ALL flag
		 *
		 * @param bool|null distinct
		 * @return \Phalcon\Mvc\Model\Query\Builder\Select
		 */
		public function distinct($distinct){ }


		/**
		 * Returns SELECT DISTINCT / SELECT ALL flag
		 *
		 * @return boolean
		 */
		public function getDistinct(){ }


		/**
		 * Sets the columns to be queried
		 *
		 *<code>
		 *	$builder->columns(array('id', 'name'));
		 *</code>
		 *
		 * @param string|array $columns
		 * @return \Phalcon\Mvc\Model\Query\Builder\Select
		 */
		public function columns($columns){ }


		/**
		 * Return the columns to be queried
		 *
		 * @return string|array
		 */
		public function getColumns(){ }


		/**
		 * Sets the models who makes part of the query
		 *
		 *<code>
		 *	$builder->from('Robots');
		 *	$builder->from(array('Robots', 'RobotsParts'));
		 *</code>
		 *
		 * @param string|array $model
		 * @return \Phalcon\Mvc\Model\Query\Builder\Select
		 */
		public function from($model, $alias=null, $merge=null){ }


		/**
		 * Add a model to take part of the query
		 *
		 *<code>
		 *	$builder->addFrom('Robots', 'r');
		 *</code>
		 *
		 * @param string|array $model
		 * @param string $alias
		 * @return \Phalcon\Mvc\Model\Query\Builder\Select
		 */
		public function addFrom($model, $alias=null){ }


		/**
		 * Return the models who makes part of the query
		 *
		 * @return array
		 */
		public function getFrom(){ }


		/**
		 * Sets a ORDER BY condition clause
		 *
		 *<code>
		 *	$builder->orderBy('Robots.name');
		 *	$builder->orderBy(array('1', 'Robots.name'));
		 *</code>
		 *
		 * @param string|array $orderBy
		 * @return \Phalcon\Mvc\Model\Query\Builder\Select
		 */
		public function orderBy($orderBy){ }


		/**
		 * Returns the set ORDER BY clause
		 *
		 * @return string|array
		 */
		public function getOrderBy(){ }


		/**
		 * Sets a HAVING condition clause. You need to escape PHQL reserved words using [ and ] delimiters
		 *
		 *<code>
		 *	$builder->having('SUM(Robots.price) > 0');
		 *</code>
		 *
		 * @param string $having
		 * @return \Phalcon\Mvc\Model\Query\Builder\Select
		 */
		public function having($having){ }


		/**
		 * Return the current having clause
		 *
		 * @return string|array
		 */
		public function getHaving(){ }


		/**
		 * Sets a LIMIT clause, optionally a offset clause
		 *
		 *<code>
		 *	$builder->limit(100);
		 *	$builder->limit(100, 20);
		 *</code>
		 *
		 * @param int $limit
		 * @param int $offset
		 * @return \Phalcon\Mvc\Model\Query\Builder\Select
		 */
		public function limit($limit, $offset=null){ }


		/**
		 * Returns the current LIMIT clause
		 *
		 * @return string|array
		 */
		public function getLimit(){ }


		/**
		 * Sets an OFFSET clause
		 *
		 *<code>
		 *	$builder->offset(30);
		 *</code>
		 *
		 * @param int $limit
		 * @param int $offset
		 * @return \Phalcon\Mvc\Model\Query\Builder\Select
		 */
		public function offset($offset){ }


		/**
		 * Returns the current OFFSET clause
		 *
		 * @return string|array
		 */
		public function getOffset(){ }


		/**
		 * Sets a GROUP BY clause
		 *
		 *<code>
		 *	$builder->groupBy(array('Robots.name'));
		 *</code>
		 *
		 * @param string|array $group
		 * @return \Phalcon\Mvc\Model\Query\Builder\Select
		 */
		public function groupBy($group){ }


		/**
		 * Returns the GROUP BY clause
		 *
		 * @return string|array
		 */
		public function getGroupBy(){ }


		/**
		 * Returns a PHQL statement built based on the builder parameters
		 *
		 * @return string
		 */
		protected function _compile(){ }

	}
}
