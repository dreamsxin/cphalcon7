<?php 

namespace Phalcon\Mvc {

	/**
	 * Phalcon\Mvc\ModelInterface initializer
	 */
	
	interface ModelInterface {

		/**
		 * Sets a transaction related to the Model instance
		 *
		 * @param \Phalcon\Mvc\Model\TransactionInterface $transaction
		 * @return \Phalcon\Mvc\ModelInterface
		 */
		public function setTransaction($transaction);


		/**
		 * Returns table name mapped in the model
		 *
		 * @return string
		 */
		public function getSource();


		/**
		 * Returns schema name where table mapped is located
		 *
		 * @return string
		 */
		public function getSchema();


		/**
		 * Returns the name of identity field (if one is present)
		 *
		 * @return string
		 */
		public function getIdentityField();


		/**
		 * Returns the column map if any
		 *
		 * @return array
		 */
		public function getColumnMap();


		/**
		 * Returns the reverse column map if any
		 *
		 * @return array
		 */
		public function getReverseColumnMap();


		/**
		 * Returns table attributes names (fields)
		 *
		 * @return array
		 */
		public function getAttributes();


		/**
		 * Returns an array of fields which are part of the primary key
		 *
		 * @return array
		 */
		public function getPrimaryKeyAttributes();


		/**
		 * Returns an arrau of fields which are not part of the primary key
		 *
		 * @return array
		 */
		public function getNonPrimaryKeyAttributes();


		/**
		 * Returns an array of not null attributes
		 *
		 * @return array
		 */
		public function getNotNullAttributes();


		/**
		 * Returns attributes which types are numerical
		 *
		 * @return array
		 */
		public function getDataTypesNumeric();


		/**
		 * Checks if the attribute is not null
		 *
		 * @return array
		 */
		public function isNotNull($attribute);


		/**
		 * Returns the columns data types
		 *
		 * @return array
		 */
		public function getDataTypes();


		/**
		 * Returns attribute data size
		 *
		 * @param string $attribute
		 * @return array
		 */
		public function getDataSize($attribute);


		/**
		 * Returns attribute data byte
		 *
		 * @param string $attribute
		 * @return array
		 */
		public function getDataByte($attribute);


		/**
		 * Returns attribute data scale
		 *
		 * @param string $attribute
		 * @return array
		 */
		public function getDataScale($attribute);


		/**
		 * Returns attributes and their bind data types
		 *
		 * @return array
		 */
		public function getBindTypes();


		/**
		 * Returns attributes and their default values
		 *
		 * @return array
		 */
		public function getDefaultValues();


		/**
		 * Returns attributes that must be ignored from the INSERT SQL generation
		 *
		 * @return array
		 */
		public function getAutomaticCreateAttributes();


		/**
		 * Returns attributes that must be ignored from the UPDATE SQL generation
		 *
		 * @return array
		 */
		public function getAutomaticUpdateAttributes();


		/**
		 * Check if a model has certain column
		 *
		 * @param string $column
		 * @return boolean
		 */
		public function hasRealAttribute($column);


		public function getRealAttribute($column);


		/**
		 * Check if a model has certain attribute
		 *
		 * @param string $attribute
		 * @return boolean
		 */
		public function hasAttribute($attribute);


		/**
		 * Gets a model certain attribute
		 *
		 * @param string $attribute
		 * @return string
		 */
		public function getAttribute($attribute);


		/**
		 * Sets both read/write connection services
		 *
		 * @param string $connectionService
		 */
		public function setConnectionService($connectionService);


		/**
		 * Sets the DependencyInjection connection service used to write data
		 *
		 * @param string $connectionService
		 */
		public function setWriteConnectionService($connectionService);


		/**
		 * Sets the DependencyInjection connection service used to read data
		 *
		 * @param string $connectionService
		 */
		public function setReadConnectionService($connectionService);


		/**
		 * Returns DependencyInjection connection service used to read data
		 *
		 * @return string
		 */
		public function getReadConnectionService();


		/**
		 * Returns DependencyInjection connection service used to write data
		 *
		 * @return string
		 */
		public function getWriteConnectionService();


		/**
		 * Gets internal database connection
		 *
		 * @return \Phalcon\Db\AdapterInterface
		 */
		public function getReadConnection();


		/**
		 * Gets internal database connection
		 *
		 * @return \Phalcon\Db\AdapterInterface
		 */
		public function getWriteConnection();


		/**
		 * Assigns values to a model from an array
		 *
		 * @param array $data
		 * @param array $columnMap
		 * @param array $whiteList
		 * @return \Phalcon\Mvc\Model
		 */
		public function assign($data, $columnMap=null, $whiteList=null);


		/**
		 * Assigns values to a model from an array returning a new model
		 *
		 * @param \Phalcon\Mvc\Model $base
		 * @param array $data
		 * @param array $columnMap
		 * @param int $dirtyState
		 * @param boolean $keepSnapshots
		 * @return \Phalcon\Mvc\Model $result
		 */
		public static function cloneResultMap($base, $data, $columnMap, $dirtyState, $keepSnapshots=null, $sourceModel=null);


		/**
		 * Assigns values to a model from an array returning a new model
		 *
		 * @param \Phalcon\Mvc\Model $base
		 * @param array $data
		 * @param int $dirtyState
		 * @return \Phalcon\Mvc\Model
		 */
		public static function cloneResult($base, $data, $dirtyState=null, $sourceModel=null);


		/**
		 * Returns an hydrated result based on the data and the column map
		 *
		 * @param \Phalcon\Mvc\Model $base
		 * @param array $data
		 * @param array $columnMap
		 * @param int $hydrationMode
		 */
		public static function cloneResultMapHydrate($data, $columnMap, $hydrationMode, $sourceModel=null);


		/**
		 * Allows to query a set of records that match the specified conditions
		 *
		 * @param 	array $parameters
		 * @return  \Phalcon\Mvc\Model\ResultsetInterface
		 */
		public static function find($parameters=null);


		/**
		 * Allows to query the first record that match the specified conditions
		 *
		 * @param array $parameters
		 * @return \Phalcon\Mvc\ModelInterface
		 */
		public static function findFirst($parameters=null, $autoCreate=null);


		/**
		 * Create a criteria for a especific model
		 *
		 * @param \Phalcon\DiInterface $dependencyInjector
		 * @return \Phalcon\Mvc\Model\CriteriaInterface
		 */
		public static function query($dependencyInjector=null);


		/**
		 * Allows to count how many records match the specified conditions
		 *
		 * @param array $parameters
		 * @return int
		 */
		public static function count($parameters=null);


		/**
		 * Allows to calculate a summatory on a column that match the specified conditions
		 *
		 * @param array $parameters
		 * @return double
		 */
		public static function sum($parameters=null);


		/**
		 * Allows to get the maximum value of a column that match the specified conditions
		 *
		 * @param array $parameters
		 * @return mixed
		 */
		public static function maximum($parameters=null);


		/**
		 * Allows to get the minimum value of a column that match the specified conditions
		 *
		 * @param array $parameters
		 * @return mixed
		 */
		public static function minimum($parameters=null);


		/**
		 * Allows to calculate the average value on a column matching the specified conditions
		 *
		 * @param array $parameters
		 * @return double
		 */
		public static function average($parameters=null);


		/**
		 * Appends a customized message on the validation process
		 *
		 * @param \Phalcon\Mvc\Model\MessageInterface $message
		 */
		public function appendMessage($message, $field=null, $type=null, $code=null);


		/**
		 * Check whether validation process has generated any messages
		 *
		 * @return boolean
		 */
		public function validationHasFailed();


		/**
		 * Returns all the validation messages
		 *
		 * @return \Phalcon\Mvc\Model\MessageInterface[]
		 */
		public function getMessages($filter=null);


		/**
		 * Inserts or updates a model instance. Returning true on success or false otherwise.
		 *
		 * @param  array $data
		 * @param  array $whiteList
		 * @return boolean
		 */
		public function save($data=null, $whiteList=null, $exists=null);


		/**
		 * Inserts a model instance. If the instance already exists in the persistance it will throw an exception
		 * Returning true on success or false otherwise.
		 *
		 * @param  array $data
		 * @param  array $whiteList
		 * @return boolean
		 */
		public function create($data=null, $whiteList=null, $existsCheck=null);


		/**
		 * Updates a model instance. If the instance doesn't exist in the persistance it will throw an exception
		 * Returning true on success or false otherwise.
		 *
		 * @param  array $data
		 * @param  array $whiteList
		 * @return boolean
		 */
		public function update($data=null, $whiteList=null, $existsCheck=null);


		/**
		 * Deletes a model instance. Returning true on success or false otherwise.
		 *
		 * @return boolean
		 */
		public function delete();


		/**
		 * Returns the type of the latest operation performed by the ORM
		 * Returns one of the OP_* class constants
		 *
		 * @return int
		 */
		public function getOperationMade();


		/**
		 * Refreshes the model attributes re-querying the record from the database
		 */
		public function refresh();


		/**
		 * Reads an attribute value by its name
		 *
		 * @param string $attribute
		 * @return mixed
		 */
		public function readAttribute($attribute);


		/**
		 * Writes an attribute value by its name
		 *
		 * @param string $attribute
		 * @param mixed $value
		 */
		public function writeAttribute($attribute, $value);


		/**
		 * Returns related records based on defined relations
		 *
		 * @param string $alias
		 * @param array $arguments
		 * @return \Phalcon\Mvc\Model\ResultsetInterface
		 */
		public function getRelated($alias, $arguments=null);


		/**
		 * Allows to delete a set of records that match the specified conditions
		 *
		 * @param 	array $parameters
		 * @return	boolean
		 */
		public static function remove($parameters);


		public function reset();

	}
}
