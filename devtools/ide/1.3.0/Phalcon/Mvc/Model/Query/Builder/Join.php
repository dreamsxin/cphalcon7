<?php 

namespace Phalcon\Mvc\Model\Query\Builder {

	/**
	 * Phalcon\Mvc\Model\Query\Builder\Join
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
	
	abstract class Join extends \Phalcon\Mvc\Model\Query\Builder\Where implements \Phalcon\Events\EventsAwareInterface, \Phalcon\Di\InjectionAwareInterface, \Phalcon\Mvc\Model\Query\BuilderInterface {

		protected $_joins;

		/**
		 * Adds a join to the query
		 *
		 *<code>
		 *	$builder->join('Robots');
		 *	$builder->join('Robots', 'r.id = RobotsParts.robots_id');
		 *	$builder->join('Robots', 'r.id = RobotsParts.robots_id', 'r');
		 *	$builder->join('Robots', 'r.id = RobotsParts.robots_id', 'r', 'LEFT');
		 *</code>
		 *
		 * @param string $model
		 * @param string $conditions
		 * @param string $alias
		 * @param string $type
		 * @return \Phalcon\Mvc\Model\Query\Builder\Join
		 */
		public function join($model, $conditions=null, $alias=null){ }


		/**
		 * Adds a INNER join to the query
		 *
		 *<code>
		 *	$builder->innerJoin('Robots');
		 *	$builder->innerJoin('Robots', 'r.id = RobotsParts.robots_id');
		 *	$builder->innerJoin('Robots', 'r.id = RobotsParts.robots_id', 'r');
		 *</code>
		 *
		 * @param string $model
		 * @param string $conditions
		 * @param string $alias
		 * @return \Phalcon\Mvc\Model\Query\Builder\Join
		 */
		public function innerJoin($model, $conditions=null, $alias=null){ }


		/**
		 * Adds a LEFT join to the query
		 *
		 *<code>
		 *	$builder->leftJoin('Robots', 'r.id = RobotsParts.robots_id', 'r');
		 *</code>
		 *
		 * @param string $model
		 * @param string $conditions
		 * @param string $alias
		 * @return \Phalcon\Mvc\Model\Query\Builder\Join
		 */
		public function leftJoin($model, $conditions=null, $alias=null){ }


		/**
		 * Adds a RIGHT join to the query
		 *
		 *<code>
		 *	$builder->rightJoin('Robots', 'r.id = RobotsParts.robots_id', 'r');
		 *</code>
		 *
		 * @param string $model
		 * @param string $conditions
		 * @param string $alias
		 * @return \Phalcon\Mvc\Model\Query\Builder\Join
		 */
		public function rightJoin($model, $conditions=null, $alias=null){ }

	}
}
