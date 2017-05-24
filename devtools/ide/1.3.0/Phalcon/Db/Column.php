<?php 

namespace Phalcon\Db {

	/**
	 * Phalcon\Db\Column
	 *
	 * Allows to define columns to be used on create or alter table operations
	 *
	 *<code>
	 *	use Phalcon\Db\Column as Column;
	 *
	 * //column definition
	 * $column = new Column("id", array(
	 *   "type" => Column::TYPE_INTEGER,
	 *   "size" => 10,
	 *   "unsigned" => true,
	 *   "notNull" => true,
	 *   "autoIncrement" => true,
	 *   "first" => true
	 * ));
	 *
	 * //add column to existing table
	 * $connection->addColumn("robots", null, $column);
	 *</code>
	 *
	 */
	
	class Column implements \Phalcon\Db\ColumnInterface {

		const TYPE_INTEGER = 0;

		const TYPE_BIGINTEGER = 1;

		const TYPE_DATE = 4;

		const TYPE_VARCHAR = 8;

		const TYPE_DECIMAL = 3;

		const TYPE_DATETIME = 5;

		const TYPE_CHAR = 7;

		const TYPE_TEXT = 9;

		const TYPE_FLOAT = 2;

		const TYPE_BOOLEAN = 10;

		const TYPE_DOUBLE = 11;

		const TYPE_TINYBLOB = 12;

		const TYPE_BLOB = 15;

		const TYPE_MEDIUMBLOB = 13;

		const TYPE_LONGBLOB = 14;

		const TYPE_JSON = 16;

		const TYPE_JSONB = 17;

		const TYPE_ARRAY = 18;

		const TYPE_INT_ARRAY = 21;

		const TYPE_TIMESTAMP = 6;

		const TYPE_BYTEA = 19;

		const TYPE_MONEY = 20;

		const TYPE_OTHER = 100;

		const BIND_PARAM_NULL = 0;

		const BIND_PARAM_INT = 1;

		const BIND_PARAM_STR = 2;

		const BIND_PARAM_BOOL = 5;

		const BIND_PARAM_DECIMAL = 32;

		const BIND_SKIP = 1024;

		protected $_columnName;

		protected $_schemaName;

		protected $_type;

		protected $_typeReference;

		protected $_typeValues;

		protected $_isNumeric;

		protected $_size;

		protected $_bytes;

		protected $_scale;

		protected $_unsigned;

		protected $_notNull;

		protected $_primary;

		protected $_autoIncrement;

		protected $_first;

		protected $_after;

		protected $_bindType;

		protected $_default;

		/**
		 * \Phalcon\Db\Column constructor
		 *
		 * @param string $columnName
		 * @param array $definition
		 */
		public function __construct($columnName, $definition){ }


		/**
		 * Returns schema's table related to column
		 *
		 * @return string
		 */
		public function getSchemaName(){ }


		/**
		 * Returns column name
		 *
		 * @return string
		 */
		public function getName(){ }


		/**
		 * Returns column type
		 *
		 * @return int
		 */
		public function getType(){ }


		/**
		 * Returns column type reference
		 *
		 * @return int
		 */
		public function getTypeReference(){ }


		/**
		 * Returns column type values
		 *
		 * @return string
		 */
		public function getTypeValues(){ }


		/**
		 * Returns column size
		 *
		 * @return int
		 */
		public function getSize(){ }


		/**
		 * Returns column bytes
		 *
		 * @return int
		 */
		public function getBytes(){ }


		/**
		 * Returns column scale
		 *
		 * @return int
		 */
		public function getScale(){ }


		/**
		 * Returns true if number column is unsigned
		 *
		 * @return boolean
		 */
		public function isUnsigned(){ }


		/**
		 * Not null
		 *
		 * @return boolean
		 */
		public function isNotNull(){ }


		/**
		 * Column is part of the primary key?
		 *
		 * @return boolean
		 */
		public function isPrimary(){ }


		/**
		 * Auto-Increment
		 *
		 * @return boolean
		 */
		public function isAutoIncrement(){ }


		/**
		 * Check whether column have an numeric type
		 *
		 * @return boolean
		 */
		public function isNumeric(){ }


		/**
		 * Check whether column have first position in table
		 *
		 * @return boolean
		 */
		public function isFirst(){ }


		/**
		 * Check whether field absolute to position in table
		 *
		 * @return string
		 */
		public function getAfterPosition(){ }


		/**
		 * Returns the type of bind handling
		 *
		 * @return int
		 */
		public function getBindType(){ }


		/**
		 * Returns the field default values
		 *
		 * @return string
		 */
		public function getDefaultValue(){ }


		/**
		 * Restores the internal state of a \Phalcon\Db\Column object
		 *
		 * @param array $data
		 * @return \Phalcon\Db\Column
		 */
		public static function __set_state($properties=null){ }


		public function getDefault(){ }

	}
}
