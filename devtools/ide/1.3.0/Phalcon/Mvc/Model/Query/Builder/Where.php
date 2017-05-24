<?php 

namespace Phalcon\Mvc\Model\Query\Builder {

	/**
	 * Phalcon\Mvc\Model\Query\Builder
	 *
	 * Helps to create PHQL queries for WHERE statements
	 */
	
	abstract class Where extends \Phalcon\Mvc\Model\Query\Builder implements \Phalcon\Mvc\Model\Query\BuilderInterface, \Phalcon\Di\InjectionAwareInterface, \Phalcon\Events\EventsAwareInterface {

		protected $_conditions;

		/**
		 * Gets the type of PHQL queries
		 *
		 *
		 * @return int
		 */
		public function setConditions($conditions, $bindParams=null, $bindTypes=null, $bindParams=null, $type=null){ }


		/**
		 * Returns the conditions, If the conditions is a single numeric field. We internally create a condition
		 * using the related primary key
		 *
		 *<code>
		 *	$builder->getConditions();
		 *</code>
		 *
		 * @return string
		 */
		public function getConditions(){ }


		/**
		 * Sets the query conditions
		 *
		 *<code>
		 *	$builder->where('name = "Peter"');
		 *	$builder->where('name = :name: AND id > :id:', array('name' => 'Peter', 'id' => 100));
		 *</code>
		 *
		 * @param string|array $conditions
		 * @param array $bindParams
		 * @param array $bindTypes
		 * @return \Phalcon\Mvc\Model\Query\Builder
		 */
		public function where($conditions, $bindParams=null, $bindTypes=null){ }


		/**
		 * Appends a condition to the current conditions using a AND operator
		 *
		 *<code>
		 *	$builder->andWhere('name = "Peter"');
		 *	$builder->andWhere('name = :name: AND id > :id:', array('name' => 'Peter', 'id' => 100));
		 *</code>
		 *
		 * @param string|array $conditions
		 * @param array $bindParams
		 * @param array $bindTypes
		 * @return \Phalcon\Mvc\Model\Query\Builder
		 */
		public function andWhere($conditions, $bindParams=null, $bindTypes=null){ }


		/**
		 * Appends a condition to the current conditions using a OR operator
		 *
		 *<code>
		 *	$builder->orWhere('name = "Peter"');
		 *	$builder->orWhere('name = :name: AND id > :id:', array('name' => 'Peter', 'id' => 100));
		 *</code>
		 *
		 * @param string|array $conditions
		 * @param array $bindParams
		 * @param array $bindTypes
		 * @return \Phalcon\Mvc\Model\Query\Builder
		 */
		public function orWhere($conditions, $bindParams=null, $bindTypes=null){ }


		/**
		 * Appends a BETWEEN condition to the current conditions
		 *
		 *<code>
		 *	$builder->betweenWhere('price', 100.25, 200.50);
		 *</code>
		 *
		 * @param string $expr
		 * @param mixed $minimum
		 * @param mixed $maximum
		 * @param boolean $useOrWhere
		 * @return \Phalcon\Mvc\Model\Query\Builder
		 */
		public function betweenWhere($expr, $minimum, $maximum, $useOrWhere=null){ }


		/**
		 * Appends a NOT BETWEEN condition to the current conditions
		 *
		 *<code>
		 *	$builder->notBetweenWhere('price', 100.25, 200.50);
		 *</code>
		 *
		 * @param string $expr
		 * @param mixed $minimum
		 * @param mixed $maximum
		 * @param boolean $useOrWhere
		 * @return \Phalcon\Mvc\Model\Query\Builder
		 */
		public function notBetweenWhere($expr, $minimum, $maximum, $useOrWhere=null){ }


		/**
		 * Appends an IN condition to the current conditions
		 *
		 *<code>
		 *	$builder->inWhere('id', [1, 2, 3]);
		 *</code>
		 *
		 * @param string $expr
		 * @param array $values
		 * @param boolean $useOrWhere
		 * @return \Phalcon\Mvc\Model\Query\Builder
		 */
		public function inWhere($expr, $values, $useOrWhere=null){ }


		/**
		 * Appends a NOT IN condition to the current conditions
		 *
		 *<code>
		 *	$builder->notInWhere('id', [1, 2, 3]);
		 *</code>
		 *
		 * @param string $expr
		 * @param array $values
		 * @param boolean $useOrWhere
		 * @return \Phalcon\Mvc\Model\Query\Builder
		 */
		public function notInWhere($expr, $values, $useOrWhere=null){ }


		/**
		 * Return the conditions for the query
		 *
		 * @return string|array
		 */
		public function getWhere(){ }

	}
}
