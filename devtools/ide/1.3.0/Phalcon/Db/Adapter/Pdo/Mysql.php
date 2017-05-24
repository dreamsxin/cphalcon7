<?php 

namespace Phalcon\Db\Adapter\Pdo {

	/**
	 * Phalcon\Db\Adapter\Pdo\Mysql
	 *
	 * Specific functions for the Mysql database system
	 *
	 *<code>
	 *
	 *	$config = array(
	 *		"host" => "192.168.0.11",
	 *		"dbname" => "blog",
	 *		"port" => 3306,
	 *		"username" => "sigma",
	 *		"password" => "secret"
	 *	);
	 *
	 *	$connection = new Phalcon\Db\Adapter\Pdo\Mysql($config);
	 *
	 *</code>
	 */
	
	class Mysql extends \Phalcon\Db\Adapter\Pdo implements \Phalcon\Events\EventsAwareInterface, \Phalcon\Di\InjectionAwareInterface, \Phalcon\Db\AdapterInterface {

		protected $_type;

		protected $_dialectType;

		/**
		 * Escapes a column/table/schema name
		 *
		 * @param string $identifier
		 * @return string
		 */
		public function escapeIdentifier($identifier){ }


		/**
		 * Returns an array of \Phalcon\Db\Column objects describing a table
		 *
		 * <code>
		 * print_r($connection->describeColumns("posts")); ?>
		 * </code>
		 *
		 * @param string $table
		 * @param string $schema
		 * @return \Phalcon\Db\Column[]
		 */
		public function describeColumns($table, $schema=null){ }


		/**
		 * Convert php bytea to database bytea
		 *
		 * @param string $value
		 * @return string
		 * @return string
		 */
		public function escapeBytea($value){ }


		/**
		 * Convert database bytea to php bytea
		 *
		 * @param string $value
		 * @return string
		 */
		public function unescapeBytea($value){ }


		/**
		 * Convert php array to database array
		 *
		 * @param array $value
		 * @return string
		 */
		public function escapeArray($value, $type=null){ }


		/**
		 * Convert database array to php array
		 *
		 * @param string $value
		 * @return array
		 */
		public function unescapeArray($value, $type=null){ }

	}
}
