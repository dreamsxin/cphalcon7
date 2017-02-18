
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2014 Phalcon Team (http://www.phalconphp.com)       |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled     |
  | with this package in the file docs/LICENSE.txt.                        |
  |                                                                        |
  | If you did not receive a copy of the license and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@phalconphp.com so we can send you a copy immediately.       |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@phalconphp.com>                      |
  |          Eduar Carvajal <eduar@phalconphp.com>                         |
  +------------------------------------------------------------------------+
*/

#include "mvc/modelinterface.h"
#include "kernel/main.h"

zend_class_entry *phalcon_mvc_modelinterface_ce;

static const zend_function_entry phalcon_mvc_modelinterface_method_entry[] = {
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, setTransaction, arginfo_phalcon_mvc_modelinterface_settransaction)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getSource, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getSchema, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getIdentityField, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getColumnMap, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getReverseColumnMap, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getAttributes, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getPrimaryKeyAttributes, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getNonPrimaryKeyAttributes, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getNotNullAttributes, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getDataTypesNumeric, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, isNotNull, arginfo_phalcon_mvc_modelinterface_isnotnull)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getDataTypes, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getDataSize, arginfo_phalcon_mvc_modelinterface_getdatasize)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getDataByte, arginfo_phalcon_mvc_modelinterface_getdatabyte)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getDataScale, arginfo_phalcon_mvc_modelinterface_getdatascale)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getBindTypes, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getDefaultValues, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getAutomaticCreateAttributes, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getAutomaticUpdateAttributes, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, hasRealAttribute, arginfo_phalcon_mvc_modelinterface_hasrealattribute)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getRealAttribute, arginfo_phalcon_mvc_modelinterface_getrealattribute)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, hasAttribute, arginfo_phalcon_mvc_modelinterface_hasattribute)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getAttribute, arginfo_phalcon_mvc_modelinterface_getattribute)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, setConnectionService, arginfo_phalcon_mvc_modelinterface_setconnectionservice)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, setWriteConnectionService, arginfo_phalcon_mvc_modelinterface_setwriteconnectionservice)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, setReadConnectionService, arginfo_phalcon_mvc_modelinterface_setreadconnectionservice)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getReadConnectionService, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getWriteConnectionService, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getReadConnection, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getWriteConnection, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, assign, arginfo_phalcon_mvc_modelinterface_assign)
	ZEND_FENTRY(cloneResultMap, NULL, arginfo_phalcon_mvc_modelinterface_cloneresultmap, ZEND_ACC_STATIC|ZEND_ACC_ABSTRACT|ZEND_ACC_PUBLIC)
	ZEND_FENTRY(cloneResult, NULL, arginfo_phalcon_mvc_modelinterface_cloneresult, ZEND_ACC_STATIC|ZEND_ACC_ABSTRACT|ZEND_ACC_PUBLIC)
	ZEND_FENTRY(cloneResultMapHydrate, NULL, arginfo_phalcon_mvc_modelinterface_cloneresultmaphydrate, ZEND_ACC_STATIC|ZEND_ACC_ABSTRACT|ZEND_ACC_PUBLIC)
	ZEND_FENTRY(find, NULL, arginfo_phalcon_mvc_modelinterface_find, ZEND_ACC_STATIC|ZEND_ACC_ABSTRACT|ZEND_ACC_PUBLIC)
	ZEND_FENTRY(findFirst, NULL, arginfo_phalcon_mvc_modelinterface_findfirst, ZEND_ACC_STATIC|ZEND_ACC_ABSTRACT|ZEND_ACC_PUBLIC)
	ZEND_FENTRY(query, NULL, arginfo_phalcon_mvc_modelinterface_query, ZEND_ACC_STATIC|ZEND_ACC_ABSTRACT|ZEND_ACC_PUBLIC)
	ZEND_FENTRY(count, NULL, arginfo_phalcon_mvc_modelinterface_count, ZEND_ACC_STATIC|ZEND_ACC_ABSTRACT|ZEND_ACC_PUBLIC)
	ZEND_FENTRY(sum, NULL, arginfo_phalcon_mvc_modelinterface_sum, ZEND_ACC_STATIC|ZEND_ACC_ABSTRACT|ZEND_ACC_PUBLIC)
	ZEND_FENTRY(maximum, NULL, arginfo_phalcon_mvc_modelinterface_maximum, ZEND_ACC_STATIC|ZEND_ACC_ABSTRACT|ZEND_ACC_PUBLIC)
	ZEND_FENTRY(minimum, NULL, arginfo_phalcon_mvc_modelinterface_minimum, ZEND_ACC_STATIC|ZEND_ACC_ABSTRACT|ZEND_ACC_PUBLIC)
	ZEND_FENTRY(average, NULL, arginfo_phalcon_mvc_modelinterface_average, ZEND_ACC_STATIC|ZEND_ACC_ABSTRACT|ZEND_ACC_PUBLIC)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, appendMessage, arginfo_phalcon_mvc_modelinterface_appendmessage)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, validationHasFailed, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getMessages, arginfo_phalcon_mvc_modelinterface_getmessages)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, save, arginfo_phalcon_mvc_modelinterface_save)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, create, arginfo_phalcon_mvc_modelinterface_create)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, update, arginfo_phalcon_mvc_modelinterface_update)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, delete, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getOperationMade, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, refresh, NULL)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, readAttribute, arginfo_phalcon_mvc_modelinterface_readattribute)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, writeAttribute, arginfo_phalcon_mvc_modelinterface_writeattribute)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, getRelated, arginfo_phalcon_mvc_modelinterface_getrelated)
	ZEND_FENTRY(remove, NULL, arginfo_phalcon_mvc_modelinterface_remove, ZEND_ACC_STATIC|ZEND_ACC_ABSTRACT|ZEND_ACC_PUBLIC)
	PHP_ABSTRACT_ME(Phalcon_Mvc_ModelInterface, reset, NULL)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\ModelInterface initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_ModelInterface){

	PHALCON_REGISTER_INTERFACE(Phalcon\\Mvc, ModelInterface, mvc_modelinterface, phalcon_mvc_modelinterface_method_entry);

	return SUCCESS;
}

/**
 * Sets a transaction related to the Model instance
 *
 * @param Phalcon\Mvc\Model\TransactionInterface $transaction
 * @return Phalcon\Mvc\ModelInterface
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, setTransaction);

/**
 * Returns table name mapped in the model
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getSource);

/**
 * Returns schema name where table mapped is located
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getSchema);

/**
 * Returns the name of identity field (if one is present)
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getIdentityField)

/**
 * Returns the column map if any
 *
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getColumnMap)

/**
 * Returns the reverse column map if any
 *
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getReverseColumnMap)

/**
 * Returns table attributes names (fields)
 *
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getAttributes)

/**
 * Returns an array of fields which are part of the primary key
 *
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getPrimaryKeyAttributes)

/**
 * Returns an arrau of fields which are not part of the primary key
 *
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getNonPrimaryKeyAttributes)

/**
 * Returns an array of not null attributes
 *
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getNotNullAttributes)

/**
 * Returns attributes which types are numerical
 *
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getDataTypesNumeric)

/**
 * Checks if the attribute is not null
 *
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, isNotNull)

/**
 * Returns the columns data types
 *
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getDataTypes)

/**
 * Returns attribute data size
 *
 * @param string $attribute
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getDataSize)

/**
 * Returns attribute data byte
 *
 * @param string $attribute
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getDataByte)

/**
 * Returns attribute data scale
 *
 * @param string $attribute
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getDataScale)

/**
 * Returns attributes and their bind data types
 *
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getBindTypes)

/**
 * Returns attributes and their default values
 *
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getDefaultValues)

/**
 * Returns attributes that must be ignored from the INSERT SQL generation
 *
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getAutomaticCreateAttributes)

/**
 * Returns attributes that must be ignored from the UPDATE SQL generation
 *
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getAutomaticUpdateAttributes)

/**
 * Check if a model has certain column
 *
 * @param string $column
 * @return boolean
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, hasRealAttribute)

/**
 * Gets a model certain column
 *
 * @param string $column
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getColumn)

/**
 * Check if a model has certain attribute
 *
 * @param string $attribute
 * @return boolean
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, hasAttribute)

/**
 * Gets a model certain attribute
 *
 * @param string $attribute
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getAttribute)

/**
 * Sets both read/write connection services
 *
 * @param string $connectionService
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, setConnectionService);

/**
 * Sets the DependencyInjection connection service used to write data
 *
 * @param string $connectionService
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, setWriteConnectionService);

/**
 * Sets the DependencyInjection connection service used to read data
 *
 * @param string $connectionService
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, setReadConnectionService);

/**
 * Returns DependencyInjection connection service used to read data
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getReadConnectionService);

/**
 * Returns DependencyInjection connection service used to write data
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getWriteConnectionService);

/**
 * Gets internal database connection
 *
 * @return Phalcon\Db\AdapterInterface
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getReadConnection);

/**
 * Gets internal database connection
 *
 * @return Phalcon\Db\AdapterInterface
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getWriteConnection);

/**
 * Assigns values to a model from an array
 *
 * @param array $data
 * @param array $columnMap
 * @param array $whiteList
 * @return Phalcon\Mvc\Model
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, assign);

/**
 * Assigns values to a model from an array returning a new model
 *
 * @param Phalcon\Mvc\Model $base
 * @param array $data
 * @param array $columnMap
 * @param int $dirtyState
 * @param boolean $keepSnapshots
 * @return Phalcon\Mvc\Model $result
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, cloneResultMap);

/**
 * Assigns values to a model from an array returning a new model
 *
 * @param Phalcon\Mvc\Model $base
 * @param array $data
 * @param int $dirtyState
 * @return Phalcon\Mvc\Model
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, cloneResult);

/**
 * Returns an hydrated result based on the data and the column map
 *
 * @param Phalcon\Mvc\Model $base
 * @param array $data
 * @param array $columnMap
 * @param int $hydrationMode
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, cloneResultMapHydrate);

/**
 * Allows to query a set of records that match the specified conditions
 *
 * @param 	array $parameters
 * @return  Phalcon\Mvc\Model\ResultsetInterface
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, find);

/**
 * Allows to query the first record that match the specified conditions
 *
 * @param array $parameters
 * @return Phalcon\Mvc\ModelInterface
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, findFirst);

/**
 * Create a criteria for a especific model
 *
 * @param Phalcon\DiInterface $dependencyInjector
 * @return Phalcon\Mvc\Model\CriteriaInterface
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, query);

/**
 * Allows to count how many records match the specified conditions
 *
 * @param array $parameters
 * @return int
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, count);

/**
 * Allows to calculate a summatory on a column that match the specified conditions
 *
 * @param array $parameters
 * @return double
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, sum);

/**
 * Allows to get the maximum value of a column that match the specified conditions
 *
 * @param array $parameters
 * @return mixed
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, maximum);

/**
 * Allows to get the minimum value of a column that match the specified conditions
 *
 * @param array $parameters
 * @return mixed
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, minimum);

/**
 * Allows to calculate the average value on a column matching the specified conditions
 *
 * @param array $parameters
 * @return double
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, average);

/**
 * Fires an event, implicitly calls behaviors and listeners in the events manager are notified
 *
 * @param string $eventName
 * @return boolean
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, fireEvent);

/**
 * Fires an event, implicitly calls behaviors and listeners in the events manager are notified
 * This method stops if one of the callbacks/listeners returns boolean false
 *
 * @param string $eventName
 * @return boolean
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, fireEventCancel);

/**
 * Appends a customized message on the validation process
 *
 * @param Phalcon\Mvc\Model\MessageInterface $message
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, appendMessage);

/**
 * Check whether validation process has generated any messages
 *
 * @return boolean
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, validationHasFailed);

/**
 * Returns all the validation messages
 *
 * @return Phalcon\Mvc\Model\MessageInterface[]
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getMessages);

/**
 * Inserts or updates a model instance. Returning true on success or false otherwise.
 *
 * @param  array $data
 * @param  array $whiteList
 * @return boolean
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, save);

/**
 * Inserts a model instance. If the instance already exists in the persistance it will throw an exception
 * Returning true on success or false otherwise.
 *
 * @param  array $data
 * @param  array $whiteList
 * @return boolean
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, create);

/**
 * Updates a model instance. If the instance doesn't exist in the persistance it will throw an exception
 * Returning true on success or false otherwise.
 *
 * @param  array $data
 * @param  array $whiteList
 * @return boolean
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, update);

/**
 * Deletes a model instance. Returning true on success or false otherwise.
 *
 * @return boolean
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, delete);

/**
 * Returns the type of the latest operation performed by the ORM
 * Returns one of the OP_* class constants
 *
 * @return int
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getOperationMade);

/**
 * Refreshes the model attributes re-querying the record from the database
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, refresh);

/**
 * Reads an attribute value by its name
 *
 * @param string $attribute
 * @return mixed
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, readAttribute);

/**
 * Writes an attribute value by its name
 *
 * @param string $attribute
 * @param mixed $value
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, writeAttribute);

/**
 * Returns related records based on defined relations
 *
 * @param string $alias
 * @param array $arguments
 * @return Phalcon\Mvc\Model\ResultsetInterface
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, getRelated);

/**
 * Allows to delete a set of records that match the specified conditions
 *
 * @param 	array $parameters
 * @return	boolean
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, remove);

/*
 * Reset a model instance data
 */
PHALCON_DOC_METHOD(Phalcon_Mvc_ModelInterface, reset);
