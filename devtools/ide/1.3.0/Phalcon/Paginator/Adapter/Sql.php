<?php 

namespace Phalcon\Paginator\Adapter {

	/**
	 * Phalcon\Paginator\Adapter\Sql
	 *
	 * Pagination using a SQL as source of data
	 *
	 * <code>
	 * $sql = "SELECT * FROM robots WHERE type = :type LIMIT :limit OFFSET :offset";
	 * $sql2 = "SELECT COUNT(*) rowcount FROM robots WHERE type = :type FROM robots";
	 *
	 * $bind = ['type' => 'google'];
	 *
	 * $paginator = new \Phalcon\Paginator\Adapter\Sql(array(
	 *                 "db" => $this->db,
	 *                 "sql" => $sql,
	 *                 "total_sql" => $sql2,
	 *                 "bind" => $bind,
	 *                 "limit" => 20,
	 *                 "page" => $page
	 * ));
	 * </code>
	 */
	
	class Sql extends \Phalcon\Di\Injectable implements \Phalcon\Events\EventsAwareInterface, \Phalcon\Di\InjectionAwareInterface, \Phalcon\Paginator\AdapterInterface {

		protected $_db;

		protected $_sql;

		protected $_total_sql;

		protected $_bind;

		protected $_limitRows;

		protected $_page;

		protected $_fetchMode;

		/**
		 * \Phalcon\Paginator\Adapter\Sql
		 *
		 * @param array $config
		 */
		public function __construct($config){ }


		/**
		 * Returns a slice of the resultset to show in the pagination
		 *
		 * @return stdClass
		 */
		public function getPaginate(){ }


		/**
		 * Set current rows limit
		 *
		 * @param int $limit
		 *
		 * @return \Phalcon\Paginator\Adapter\Sql $this Fluent interface
		 */
		public function setLimit($limit){ }


		/**
		 * Get current rows limit
		 *
		 * @return int $limit
		 */
		public function getLimit(){ }


		/**
		 * Set current page number
		 *
		 * @param int $page
		 */
		public function setCurrentPage($page){ }


		/**
		 * Get current page number
		 *
		 * @param int $page
		 */
		public function getCurrentPage(){ }


		/**
		 * Set query builder object
		 *
		 * @param \Phalcon\Db\AdapterInterface $db
		 *
		 * @return \Phalcon\Paginator\Adapter\Sql $this Fluent interface
		 */
		public function setDb($db){ }


		/**
		 * Get query builder object
		 *
		 * @return \Phalcon\Db\AdapterInterface $db
		 */
		public function getDb(){ }

	}
}
