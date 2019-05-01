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
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#include "db/adapter.h"
#include "db/adapterinterface.h"
#include "db/dialectinterface.h"
#include "db/exception.h"
#include "db/index.h"
#include "db/rawvalue.h"
#include "db/reference.h"
#include "di/injectable.h"
#include "db/builder/select.h"
#include "db/builder/update.h"
#include "db/builder/insert.h"
#include "db/builder/delete.h"

#include <ext/pdo/php_pdo_driver.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/operators.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/concat.h"
#include "kernel/fcall.h"
#include "kernel/exception.h"
#include "kernel/file.h"
#include "kernel/hash.h"
#include "kernel/string.h"

/**
 * Phalcon\Db\Adapter
 *
 * Base class for Phalcon\Db adapters
 */
zend_class_entry *phalcon_db_adapter_ce;

PHP_METHOD(Phalcon_Db_Adapter, __construct);
PHP_METHOD(Phalcon_Db_Adapter, setProfiler);
PHP_METHOD(Phalcon_Db_Adapter, getProfiler);
PHP_METHOD(Phalcon_Db_Adapter, setDialect);
PHP_METHOD(Phalcon_Db_Adapter, getDialect);
PHP_METHOD(Phalcon_Db_Adapter, fetchOne);
PHP_METHOD(Phalcon_Db_Adapter, fetchAll);
PHP_METHOD(Phalcon_Db_Adapter, insert);
PHP_METHOD(Phalcon_Db_Adapter, insertAsDict);
PHP_METHOD(Phalcon_Db_Adapter, update);
PHP_METHOD(Phalcon_Db_Adapter, delete);
PHP_METHOD(Phalcon_Db_Adapter, getColumnList);
PHP_METHOD(Phalcon_Db_Adapter, limit);
PHP_METHOD(Phalcon_Db_Adapter, tableExists);
PHP_METHOD(Phalcon_Db_Adapter, viewExists);
PHP_METHOD(Phalcon_Db_Adapter, forUpdate);
PHP_METHOD(Phalcon_Db_Adapter, sharedLock);
PHP_METHOD(Phalcon_Db_Adapter, createTable);
PHP_METHOD(Phalcon_Db_Adapter, dropTable);
PHP_METHOD(Phalcon_Db_Adapter, createView);
PHP_METHOD(Phalcon_Db_Adapter, dropView);
PHP_METHOD(Phalcon_Db_Adapter, addColumn);
PHP_METHOD(Phalcon_Db_Adapter, modifyColumn);
PHP_METHOD(Phalcon_Db_Adapter, dropColumn);
PHP_METHOD(Phalcon_Db_Adapter, addIndex);
PHP_METHOD(Phalcon_Db_Adapter, dropIndex);
PHP_METHOD(Phalcon_Db_Adapter, addPrimaryKey);
PHP_METHOD(Phalcon_Db_Adapter, dropPrimaryKey);
PHP_METHOD(Phalcon_Db_Adapter, addForeignKey);
PHP_METHOD(Phalcon_Db_Adapter, dropForeignKey);
PHP_METHOD(Phalcon_Db_Adapter, getColumnDefinition);
PHP_METHOD(Phalcon_Db_Adapter, listTables);
PHP_METHOD(Phalcon_Db_Adapter, listViews);
PHP_METHOD(Phalcon_Db_Adapter, describeIndexes);
PHP_METHOD(Phalcon_Db_Adapter, describeReferences);
PHP_METHOD(Phalcon_Db_Adapter, tableOptions);
PHP_METHOD(Phalcon_Db_Adapter, createSavepoint);
PHP_METHOD(Phalcon_Db_Adapter, releaseSavepoint);
PHP_METHOD(Phalcon_Db_Adapter, rollbackSavepoint);
PHP_METHOD(Phalcon_Db_Adapter, setNestedTransactionsWithSavepoints);
PHP_METHOD(Phalcon_Db_Adapter, isNestedTransactionsWithSavepoints);
PHP_METHOD(Phalcon_Db_Adapter, getNestedTransactionSavepointName);
PHP_METHOD(Phalcon_Db_Adapter, getDefaultIdValue);
PHP_METHOD(Phalcon_Db_Adapter, supportSequences);
PHP_METHOD(Phalcon_Db_Adapter, useExplicitIdValue);
PHP_METHOD(Phalcon_Db_Adapter, getDescriptor);
PHP_METHOD(Phalcon_Db_Adapter, getConnectionId);
PHP_METHOD(Phalcon_Db_Adapter, getSQLStatement);
PHP_METHOD(Phalcon_Db_Adapter, getExpectSQLStatement);
PHP_METHOD(Phalcon_Db_Adapter, getSQLVariables);
PHP_METHOD(Phalcon_Db_Adapter, getSQLBindTypes);
PHP_METHOD(Phalcon_Db_Adapter, getType);
PHP_METHOD(Phalcon_Db_Adapter, getDialectType);
PHP_METHOD(Phalcon_Db_Adapter, createSelectBuilder);
PHP_METHOD(Phalcon_Db_Adapter, createUpdateBuilder);
PHP_METHOD(Phalcon_Db_Adapter, createInsertBuilder);
PHP_METHOD(Phalcon_Db_Adapter, createDeleteBuilder);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_setprofiler, 0, 0, 1)
	ZEND_ARG_INFO(0, profiler)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_setdialect, 0, 0, 1)
	ZEND_ARG_INFO(0, dialect)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_insertasdict, 0, 0, 2)
	ZEND_ARG_INFO(0, table)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, dataTypes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_createselectbuilder, 0, 0, 1)
	ZEND_ARG_INFO(0, tables)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_createupdatebuilder, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, table, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_createinsertbuilder, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, table, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_createdeletebuilder, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, table, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_db_adapter_method_entry[] = {
	PHP_ME(Phalcon_Db_Adapter, __construct, NULL, ZEND_ACC_PROTECTED|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Db_Adapter, setProfiler, arginfo_phalcon_db_adapter_setprofiler, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, getProfiler, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, setDialect, arginfo_phalcon_db_adapter_setdialect, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, getDialect, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, fetchOne, arginfo_phalcon_db_adapterinterface_fetchone, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, fetchAll, arginfo_phalcon_db_adapterinterface_fetchall, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, insert, arginfo_phalcon_db_adapterinterface_insert, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, insertAsDict, arginfo_phalcon_db_adapter_insertasdict, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, update, arginfo_phalcon_db_adapterinterface_update, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, delete, arginfo_phalcon_db_adapterinterface_delete, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, getColumnList, arginfo_phalcon_db_adapterinterface_getcolumnlist, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, limit, arginfo_phalcon_db_adapterinterface_limit, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, tableExists, arginfo_phalcon_db_adapterinterface_tableexists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, viewExists, arginfo_phalcon_db_adapterinterface_viewexists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, forUpdate, arginfo_phalcon_db_adapterinterface_forupdate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, sharedLock, arginfo_phalcon_db_adapterinterface_sharedlock, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, createTable, arginfo_phalcon_db_adapterinterface_createtable, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, dropTable, arginfo_phalcon_db_adapterinterface_droptable, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, createView, arginfo_phalcon_db_adapterinterface_createview, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, dropView, arginfo_phalcon_db_adapterinterface_dropview, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, addColumn, arginfo_phalcon_db_adapterinterface_addcolumn, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, modifyColumn, arginfo_phalcon_db_adapterinterface_modifycolumn, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, dropColumn, arginfo_phalcon_db_adapterinterface_dropcolumn, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, addIndex, arginfo_phalcon_db_adapterinterface_addindex, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, dropIndex, arginfo_phalcon_db_adapterinterface_dropindex, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, addPrimaryKey, arginfo_phalcon_db_adapterinterface_addprimarykey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, dropPrimaryKey, arginfo_phalcon_db_adapterinterface_dropprimarykey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, addForeignKey, arginfo_phalcon_db_adapterinterface_addforeignkey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, dropForeignKey, arginfo_phalcon_db_adapterinterface_dropforeignkey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, getColumnDefinition, arginfo_phalcon_db_adapterinterface_getcolumndefinition, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, listTables, arginfo_phalcon_db_adapterinterface_listtables, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, listViews, arginfo_phalcon_db_adapterinterface_listviews, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, describeIndexes, arginfo_phalcon_db_adapterinterface_describeindexes, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, describeReferences, arginfo_phalcon_db_adapterinterface_describereferences, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, tableOptions, arginfo_phalcon_db_adapterinterface_tableoptions, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, createSavepoint, arginfo_phalcon_db_adapterinterface_createsavepoint, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, releaseSavepoint, arginfo_phalcon_db_adapterinterface_releasesavepoint, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, rollbackSavepoint, arginfo_phalcon_db_adapterinterface_rollbacksavepoint, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, setNestedTransactionsWithSavepoints, arginfo_phalcon_db_adapterinterface_setnestedtransactionswithsavepoints, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, isNestedTransactionsWithSavepoints, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, getNestedTransactionSavepointName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, getDefaultIdValue, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, supportSequences, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, useExplicitIdValue, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, getDescriptor, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, getConnectionId, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, getSQLStatement, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, getExpectSQLStatement, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, getSQLVariables, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, getSQLBindTypes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, getType, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, getDialectType, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, createSelectBuilder, arginfo_phalcon_db_adapter_createselectbuilder, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, createUpdateBuilder, arginfo_phalcon_db_adapter_createupdatebuilder, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, createInsertBuilder, arginfo_phalcon_db_adapter_createinsertbuilder, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, createDeleteBuilder, arginfo_phalcon_db_adapter_createdeletebuilder, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Db\Adapter initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Adapter){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Db, Adapter, db_adapter, phalcon_di_injectable_ce, phalcon_db_adapter_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);


	zend_declare_property_null(phalcon_db_adapter_ce, SL("_profiler"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_ce, SL("_descriptor"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_ce, SL("_dialectType"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_ce, SL("_type"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_ce, SL("_dialect"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_ce, SL("_connectionId"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_ce, SL("_sqlStatement"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_ce, SL("_sqlVariables"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_ce, SL("_sqlBindTypes"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_db_adapter_ce, SL("_transactionLevel"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_db_adapter_ce, SL("_transactionsWithSavepoints"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_db_adapter_ce, SL("_connectionConsecutive"), 0, ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);

	zend_class_implements(phalcon_db_adapter_ce, 1, phalcon_db_adapterinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Db\Adapter constructor
 *
 * @param array $descriptor
 */
PHP_METHOD(Phalcon_Db_Adapter, __construct){

	zval *descriptor, connection_consecutive = {}, next_consecutive = {}, dialect_type = {}, dialect_class = {}, dialect_object = {};
	zend_class_entry *ce0;

	phalcon_fetch_params(1, 1, 0, &descriptor);

	/**
	 * Every new connection created obtain a consecutive number from the static
	 * property self::$_connectionConsecutive
	 */
	phalcon_read_static_property_ce(&connection_consecutive, phalcon_db_adapter_ce, SL("_connectionConsecutive"), PH_READONLY);

	phalcon_add_function(&next_consecutive, &connection_consecutive, &PHALCON_GLOBAL(z_one));
	phalcon_update_static_property_ce(phalcon_db_adapter_ce, SL("_connectionConsecutive"), &next_consecutive);

	phalcon_update_property(getThis(), SL("_connectionId"), &connection_consecutive);
	/**
	 * Dialect class can override the default dialectwo
	 */
	if (!phalcon_array_isset_fetch_str(&dialect_class, descriptor, SL("dialectClass"), PH_READONLY)) {
		phalcon_read_property(&dialect_type, getThis(), SL("_dialectType"), PH_NOISY|PH_READONLY);

		PHALCON_CONCAT_SV(&dialect_class, "phalcon\\db\\dialect\\", &dialect_type);
		PHALCON_MM_ADD_ENTRY(&dialect_class);
	}

	/**
	 * Create the instance only if the dialect is a string
	 */
	if (likely(Z_TYPE(dialect_class) == IS_STRING)) {
		ce0 = phalcon_fetch_class(&dialect_class, ZEND_FETCH_CLASS_DEFAULT);
		PHALCON_OBJECT_INIT(&dialect_object, ce0);
		PHALCON_MM_ADD_ENTRY(&dialect_object);
		if (phalcon_has_constructor(&dialect_object)) {
			PHALCON_MM_CALL_METHOD(NULL, &dialect_object, "__construct");
		}
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "setdialect", &dialect_object);
	} else if (Z_TYPE(dialect_class) == IS_OBJECT) {
		PHALCON_MM_CALL_METHOD(NULL, getThis(),  "setdialect", &dialect_class);
	}

	phalcon_update_property(getThis(), SL("_descriptor"), descriptor);
	RETURN_MM();
}

/**
 * Sets the profiler
 *
 * @param Phalcon\Db\Profiler $profiler
 */
PHP_METHOD(Phalcon_Db_Adapter, setProfiler){

	zval *profiler;

	phalcon_fetch_params(0, 1, 0, &profiler);

	phalcon_update_property(getThis(), SL("_profiler"), profiler);

}

/**
 * Returns the profiler
 *
 * @return Phalcon\Db\Profiler
 */
PHP_METHOD(Phalcon_Db_Adapter, getProfiler){


	RETURN_MEMBER(getThis(), "_profiler");
}

/**
 * Sets the dialect used to produce the SQL
 *
 * @param Phalcon\Db\DialectInterface
 */
PHP_METHOD(Phalcon_Db_Adapter, setDialect){

	zval *dialect;

	phalcon_fetch_params(0, 1, 0, &dialect);

	PHALCON_VERIFY_INTERFACE_EX(dialect, phalcon_db_dialectinterface_ce, phalcon_db_exception_ce);
	phalcon_update_property(getThis(), SL("_dialect"), dialect);
}

/**
 * Returns internal dialect instance
 *
 * @return Phalcon\Db\DialectInterface
 */
PHP_METHOD(Phalcon_Db_Adapter, getDialect){


	RETURN_MEMBER(getThis(), "_dialect");
}

/**
 * Returns the first row in a SQL query result
 *
 *<code>
 *	//Getting first robot
 *	$robot = $connection->fetchOne("SELECT * FROM robots");
 *	print_r($robot);
 *
 *	//Getting first robot with associative indexes only
 *	$robot = $connection->fetchOne("SELECT * FROM robots", Phalcon\Db::FETCH_ASSOC);
 *	print_r($robot);
 *</code>
 *
 * @param string $sqlQuery
 * @param int $fetchMode
 * @param array $bindParams
 * @param array $bindTypes
 * @param mixed $fetchArgument
 * @param array $ctorArgs
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter, fetchOne){

	zval *sql_query, *_fetch_mode = NULL, *bind_params = NULL, *bind_types = NULL, *fetch_argument = NULL, *ctor_args = NULL, fetch_mode = {}, result = {};

	phalcon_fetch_params(0, 1, 5, &sql_query, &_fetch_mode, &bind_params, &bind_types, &fetch_argument, &ctor_args);

	if (!_fetch_mode) {
		ZVAL_LONG(&fetch_mode, PDO_FETCH_BOTH);
	} else {
		ZVAL_COPY_VALUE(&fetch_mode, _fetch_mode);
	}

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	if (!fetch_argument) {
		fetch_argument = &PHALCON_GLOBAL(z_null);
	}

	if (!ctor_args) {
		ctor_args = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_METHOD(&result, getThis(), "query", sql_query, bind_params, bind_types);
	if (Z_TYPE(result) == IS_OBJECT) {
		if (Z_TYPE(fetch_mode) != IS_NULL) {
			if (Z_TYPE_P(fetch_argument) != IS_NULL) {
				if (Z_TYPE_P(ctor_args) != IS_NULL) {
					PHALCON_CALL_METHOD(return_value, &result, "fetch", &fetch_mode, fetch_argument, ctor_args);
				} else {
					PHALCON_CALL_METHOD(return_value, &result, "fetch", &fetch_mode, fetch_argument);
				}
			} else {
				PHALCON_CALL_METHOD(NULL, &result, "setfetchmode", &fetch_mode);
				PHALCON_CALL_METHOD(return_value, &result, "fetch");
			}
		} else {
			PHALCON_CALL_METHOD(return_value, &result, "fetch");
		}
		zval_ptr_dtor(&result);
	} else {
		zval_ptr_dtor(&result);
		RETURN_EMPTY_ARRAY();
	}
}

/**
 * Dumps the complete result of a query into an array
 *
 *<code>
 *	//Getting all robots with associative indexes only
 *	$robots = $connection->fetchAll("SELECT * FROM robots", Phalcon\Db::FETCH_ASSOC);
 *	foreach ($robots as $robot) {
 *		print_r($robot);
 *	}
 *
 *  //Getting all robots that contains word "robot" withing the name
 *  $robots = $connection->fetchAll("SELECT * FROM robots WHERE name LIKE :name",
 *		Phalcon\Db::FETCH_ASSOC,
 *		array('name' => '%robot%')
 *  );
 *	foreach($robots as $robot){
 *		print_r($robot);
 *	}
 *</code>
 *
 * @param string $sqlQuery
 * @param int $fetchMode
 * @param array $bindParams
 * @param array $bindTypes
 * @param mixed $fetchArgument
 * @param array $ctorArgs
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter, fetchAll){

	zval *sql_query, *_fetch_mode = NULL, *bind_params = NULL, *bind_types = NULL, *fetch_argument = NULL, *ctor_args = NULL, fetch_mode = {}, result = {};

	phalcon_fetch_params(0, 1, 5, &sql_query, &_fetch_mode, &bind_params, &bind_types, &fetch_argument, &ctor_args);

	if (!_fetch_mode) {
		ZVAL_LONG(&fetch_mode, PDO_FETCH_BOTH);
	} else {
		ZVAL_COPY_VALUE(&fetch_mode, _fetch_mode);
	}

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	if (!fetch_argument) {
		fetch_argument = &PHALCON_GLOBAL(z_null);
	}

	if (!ctor_args) {
		ctor_args = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_METHOD(&result, getThis(), "query", sql_query, bind_params, bind_types);
	if (likely(Z_TYPE(result) == IS_OBJECT)) {
		if (Z_TYPE(fetch_mode) != IS_NULL) {
			if (Z_TYPE_P(fetch_argument) != IS_NULL) {
				if (Z_TYPE_P(ctor_args) != IS_NULL) {
					PHALCON_CALL_METHOD(return_value, &result, "fetchall", &fetch_mode, fetch_argument, ctor_args);
				} else {
					PHALCON_CALL_METHOD(return_value, &result, "fetchall", &fetch_mode, fetch_argument);
				}
			} else {
				PHALCON_CALL_METHOD(return_value, &result, "fetchall", &fetch_mode);
			}
		} else {
			PHALCON_CALL_METHOD(return_value, &result, "fetchall");
		}
		zval_ptr_dtor(&result);
	}
}

/**
 * Inserts data into a table using custom RBDM SQL syntax
 *
 * <code>
 * //Inserting a new robot
 * $success = $connection->insert(
 *     "robots",
 *     array("Astro Boy", 1952),
 *     array("name", "year")
 * );
 *
 * //Next SQL sentence is sent to the database system
 * INSERT INTO `robots` (`name`, `year`) VALUES ("Astro boy", 1952);
 * </code>
 *
 * @param 	string $table
 * @param 	array $values
 * @param 	array $fields
 * @param 	array $dataTypes
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, insert){

	zval *table, *values, *fields = NULL, *data_types = NULL, exception_message = {}, placeholders = {}, insert_values = {}, bind_data_types = {}, *value;
	zval escaped_table = {}, joined_values = {}, escaped_fields = {}, *field, insert_sql = {};
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 2, 2, &table, &values, &fields, &data_types);

	if (!fields) {
		fields = &PHALCON_GLOBAL(z_null);
	}

	if (!data_types) {
		data_types = &PHALCON_GLOBAL(z_null);
	}

	if (unlikely(Z_TYPE_P(values) != IS_ARRAY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The second parameter for insert isn't an Array");
		return;
	}

	/**
	 * A valid array with more than one element is required
	 */
	if (!phalcon_fast_count_ev(values)) {
		PHALCON_CONCAT_SVS(&exception_message, "Unable to insert into ", table, " without data");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_db_exception_ce, &exception_message);
		return;
	}

	array_init(&placeholders);
	array_init(&insert_values);

	if (Z_TYPE_P(data_types) == IS_ARRAY) {
		array_init(&bind_data_types);
	} else {
		ZVAL_COPY(&bind_data_types, data_types);
	}

	/**
	 * Objects are casted using __toString, null values are converted to string 'null',
	 * everything else is passed as '?'
	 */
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(values), idx, str_key, value) {
		zval position = {}, str_value = {}, bind_type = {};
		if (str_key) {
			ZVAL_STR(&position, str_key);
		} else {
			ZVAL_LONG(&position, idx);
		}
		if (Z_TYPE_P(value) == IS_OBJECT) {
			phalcon_strval(&str_value, value);
			phalcon_array_append(&placeholders, &str_value, 0);
		} else {
			if (Z_TYPE_P(value) == IS_NULL) {
				phalcon_array_append_str(&placeholders, SL("null"), 0);
			} else {
				phalcon_array_append_str(&placeholders, SL("?"), 0);
				phalcon_array_append(&insert_values, value, PH_COPY);
				if (Z_TYPE_P(data_types) == IS_ARRAY) {
					if (!phalcon_array_isset_fetch(&bind_type, data_types, &position, PH_READONLY)) {
						PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Incomplete number of bind types");
						return;
					}

					phalcon_array_append(&bind_data_types, &bind_type, PH_COPY);
				}
			}
		}
	} ZEND_HASH_FOREACH_END();

	if (PHALCON_GLOBAL(db).escape_identifiers) {
		PHALCON_CALL_METHOD(&escaped_table, getThis(), "escapeidentifier", table);
	} else {
		ZVAL_COPY(&escaped_table, table);
	}

	/**
	 * Build the final SQL INSERT statement
	 */
	phalcon_fast_join_str(&joined_values, SL(", "), &placeholders);
	zval_ptr_dtor(&placeholders);
	if (Z_TYPE_P(fields) == IS_ARRAY) {
		zval joined_fields = {};
		if (PHALCON_GLOBAL(db).escape_identifiers) {
			array_init(&escaped_fields);

			ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(fields), field) {
				zval escaped_field = {};
				PHALCON_CALL_METHOD(&escaped_field, getThis(), "escapeidentifier", field);
				phalcon_array_append(&escaped_fields, &escaped_field, 0);
			} ZEND_HASH_FOREACH_END();

		} else {
			ZVAL_COPY(&escaped_fields, fields);
		}

		phalcon_fast_join_str(&joined_fields, SL(", "), &escaped_fields);
		zval_ptr_dtor(&escaped_fields);

		PHALCON_CONCAT_SVSVSVS(&insert_sql, "INSERT INTO ", &escaped_table, " (", &joined_fields, ") VALUES (", &joined_values, ")");
		zval_ptr_dtor(&joined_fields);
	} else {
		PHALCON_CONCAT_SVSVS(&insert_sql, "INSERT INTO ", &escaped_table, " VALUES (", &joined_values, ")");
	}
	zval_ptr_dtor(&escaped_table);
	zval_ptr_dtor(&joined_values);

	/**
	 * Perform the execution via execute
	 */
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &insert_sql, &insert_values, &bind_data_types);
	zval_ptr_dtor(&insert_values);
	zval_ptr_dtor(&bind_data_types);
	zval_ptr_dtor(&insert_sql);
}

/**
 * Inserts data into a table using custom RBDM SQL syntax
 * Another, more convenient syntax
 *
 * <code>
 * //Inserting a new robot
 * $success = $connection->insertAsDict(
 *	 "robots",
 *	 array(
 *		  "name" => "Astro Boy",
 *		  "year" => 1952
 *	  )
 * );
 *
 * //Next SQL sentence is sent to the database system
 * INSERT INTO `robots` (`name`, `year`) VALUES ("Astro boy", 1952);
 * </code>
 *
 * @param string table
 * @param array data
 * @param array dataTypes
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, insertAsDict){

	zval *table, *data, *data_types = NULL, fields = {}, values = {}, *value;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 2, 1, &table, &data, &data_types);

	if (!data_types) {
		data_types = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(data) != IS_ARRAY || PHALCON_IS_EMPTY(data)) {
		RETURN_FALSE;
	}

	array_init(&fields);
	array_init(&values);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(data), idx, str_key, value) {
		zval field = {};
		if (str_key) {
			ZVAL_STR(&field, str_key);
		} else {
			ZVAL_LONG(&field, idx);
		}
		phalcon_array_append(&fields, &field, PH_COPY);
		phalcon_array_append(&values, value, PH_COPY);
	} ZEND_HASH_FOREACH_END();

	PHALCON_RETURN_CALL_METHOD(getThis(), "insert", table, &values, &fields, data_types);
	zval_ptr_dtor(&values);
	zval_ptr_dtor(&fields);
}

/**
 * Updates data on a table using custom RBDM SQL syntax
 *
 * <code>
 * //Updating existing robot
 * $success = $connection->update(
 *     "robots",
 *     array("name"),
 *     array("New Astro Boy"),
 *     "id = 101"
 * );
 *
 * //Next SQL sentence is sent to the database system
 * UPDATE `robots` SET `name` = "Astro boy" WHERE id = 101
 * </code>
 *
 * @param 	string $table
 * @param 	array $fields
 * @param 	array $values
 * @param 	string $whereCondition
 * @param 	array $dataTypes
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, update){

	zval *table, *fields, *_values = NULL, *where_condition = NULL, *data_types = NULL, values = {}, placeholders = {}, update_values = {}, bind_data_types = {}, *value;
	zval escaped_table = {}, set_clause = {}, update_sql = {}, conditions = {}, where_bind = {}, where_types = {};
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 2, 3, &table, &fields, &_values, &where_condition, &data_types);

	if (!where_condition) {
		where_condition = &PHALCON_GLOBAL(z_null);
	}

	if (!data_types) {
		data_types = &PHALCON_GLOBAL(z_null);
	}

	if (!_values || Z_TYPE_P(_values) != IS_ARRAY) {
		ZVAL_COPY_VALUE(&values, fields);
	} else {
		ZVAL_COPY_VALUE(&values, _values);
	}

	array_init(&placeholders);
	array_init(&update_values);

	if (Z_TYPE_P(data_types) == IS_ARRAY) {
		array_init(&bind_data_types);
	} else {
		ZVAL_COPY(&bind_data_types, data_types);
	}

	/**
	 * Objects are casted using __toString, null values are converted to string 'null',
	 * everything else is passed as '?'
	 */
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(values), idx, str_key, value) {
		zval position = {}, field = {}, escaped_field = {}, set_clause_part = {}, bind_type = {};

		if (!_values || Z_TYPE_P(_values) != IS_ARRAY) {
			if (!str_key) {
				PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The fields is valid");
				zval_ptr_dtor(&bind_data_types);
				zval_ptr_dtor(&update_values);
				return;
			}
		}

		if (str_key) {
			ZVAL_STR(&position, str_key);
			ZVAL_STR(&field, str_key);
			if (_values && Z_TYPE_P(_values) == IS_ARRAY) {
				if (!phalcon_array_isset(fields, &position)) {
					PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The key of values in the update is not the same as fields");
					zval_ptr_dtor(&bind_data_types);
					zval_ptr_dtor(&update_values);
					return;
				}
			}
		} else {
			ZVAL_LONG(&position, idx);
			if (!phalcon_array_isset_fetch(&field, fields, &position, PH_READONLY)) {
				PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The number of values in the update is not the same as fields");
				zval_ptr_dtor(&bind_data_types);
				zval_ptr_dtor(&update_values);
				return;
			}
		}

		if (PHALCON_GLOBAL(db).escape_identifiers) {
			PHALCON_CALL_METHOD(&escaped_field, getThis(), "escapeidentifier", &field);
		} else {
			ZVAL_COPY(&escaped_field, &field);
		}

		if (Z_TYPE_P(value) == IS_OBJECT) {
			PHALCON_CONCAT_VSV(&set_clause_part, &escaped_field, " = ", value);
			phalcon_array_append(&placeholders, &set_clause_part, 0);
		} else {
			if (Z_TYPE_P(value) == IS_NULL) {
				PHALCON_CONCAT_VS(&set_clause_part, &escaped_field, " = null");
			} else {
				PHALCON_CONCAT_VS(&set_clause_part, &escaped_field, " = ?");
				phalcon_array_append(&update_values, value, PH_COPY);
				if (Z_TYPE_P(data_types) == IS_ARRAY) {
					if (!phalcon_array_isset_fetch(&bind_type, data_types, &position, PH_READONLY)) {
						PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Incomplete number of bind types");
						zval_ptr_dtor(&bind_data_types);
						zval_ptr_dtor(&escaped_field);
						zval_ptr_dtor(&update_values);
						zval_ptr_dtor(&set_clause_part);
						return;
					}

					phalcon_array_append(&bind_data_types, &bind_type, PH_COPY);
				}
			}
			phalcon_array_append(&placeholders, &set_clause_part, 0);
		}
		zval_ptr_dtor(&escaped_field);
	} ZEND_HASH_FOREACH_END();

	if (PHALCON_GLOBAL(db).escape_identifiers) {
		PHALCON_CALL_METHOD(&escaped_table, getThis(), "escapeidentifier", table);
	} else {
		ZVAL_COPY(&escaped_table, table);
	}

	phalcon_fast_join_str(&set_clause, SL(", "), &placeholders);
	zval_ptr_dtor(&placeholders);
	if (Z_TYPE_P(where_condition) != IS_NULL) {
		PHALCON_CONCAT_SVSVS(&update_sql, "UPDATE ", &escaped_table, " SET ", &set_clause, " WHERE ");

		/**
		 * String conditions are simply appended to the SQL
		 */
		if (Z_TYPE_P(where_condition) == IS_STRING) {
			phalcon_concat_self(&update_sql, where_condition);
		} else {
			/**
			 * Array conditions may have bound params and bound types
			 */
			if (unlikely(Z_TYPE_P(where_condition) != IS_ARRAY)) {
				PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Invalid WHERE clause conditions");
				return;
			}

			/**
			 * If an index 'conditions' is present it contains string where conditions that are
			 * appended to the UPDATE sql
			 */
			if (phalcon_array_isset_fetch_str(&conditions, where_condition, SL("conditions"), PH_READONLY)) {
				phalcon_concat_self(&update_sql, &conditions);
			}

			/**
			 * Bound parameters are arbitrary values that are passed by separate
			 */
			if (phalcon_array_isset_fetch_str(&where_bind, where_condition, SL("bind"), PH_READONLY)) {
				phalcon_merge_append(&update_values, &where_bind);
			}

			/**
			 * Bind types is how the bound parameters must be casted before be sent to the
			 * database system
			 */
			if (phalcon_array_isset_fetch_str(&where_types, where_condition, SL("bindTypes"), PH_READONLY)) {
				phalcon_merge_append(&bind_data_types, &where_types);
			}
		}
	} else {
		PHALCON_CONCAT_SVSV(&update_sql, "UPDATE ", &escaped_table, " SET ", &set_clause);
	}
	zval_ptr_dtor(&escaped_table);
	zval_ptr_dtor(&set_clause);

	/**
	 * Perform the update via execute
	 */
	PHALCON_CALL_METHOD(return_value, getThis(), "execute", &update_sql, &update_values, &bind_data_types);
	zval_ptr_dtor(&bind_data_types);
	zval_ptr_dtor(&update_values);
	zval_ptr_dtor(&update_sql);
}

/**
 * Deletes data from a table using custom RBDM SQL syntax
 *
 * <code>
 * //Deleting existing robot
 * $success = $connection->delete(
 *     "robots",
 *     "id = 101"
 * );
 *
 * //Next SQL sentence is generated
 * DELETE FROM `robots` WHERE `id` = 101
 * </code>
 *
 * @param  string $table
 * @param  string $whereCondition
 * @param  array $placeholders
 * @param  array $dataTypes
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, delete){

	zval *table, *where_condition = NULL, *placeholders = NULL, *data_types = NULL, escaped_table = {}, sql = {};

	phalcon_fetch_params(0, 1, 3, &table, &where_condition, &placeholders, &data_types);

	if (!where_condition) {
		where_condition = &PHALCON_GLOBAL(z_null);
	}

	if (!placeholders) {
		placeholders = &PHALCON_GLOBAL(z_null);
	}

	if (!data_types) {
		data_types = &PHALCON_GLOBAL(z_null);
	}

	if (PHALCON_GLOBAL(db).escape_identifiers) {
		PHALCON_CALL_METHOD(&escaped_table, getThis(), "escapeidentifier", table);
	} else {
		ZVAL_COPY(&escaped_table, table);
	}

	if (PHALCON_IS_NOT_EMPTY(where_condition)) {
		PHALCON_CONCAT_SVSV(&sql, "DELETE FROM ", &escaped_table, " WHERE ", where_condition);
	} else {
		PHALCON_CONCAT_SV(&sql, "DELETE FROM ", &escaped_table);
	}
	zval_ptr_dtor(&escaped_table);

	/**
	 * Perform the update via execute
	 */
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql, placeholders, data_types);
	zval_ptr_dtor(&sql);
}

/**
 * Gets a list of columns
 *
 * @param array $columnList
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter, getColumnList){

	zval *column_list, dialect = {};

	phalcon_fetch_params(0, 1, 0, &column_list);

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);
	PHALCON_RETURN_CALL_METHOD(&dialect, "getcolumnlist", column_list);
}

/**
 * Appends a LIMIT clause to $sqlQuery argument
 *
 * <code>
 * 	echo $connection->limit("SELECT * FROM robots", 5);
 * </code>
 *
 * @param  	string $sqlQuery
 * @param 	int $number
 * @return 	string
 */
PHP_METHOD(Phalcon_Db_Adapter, limit){

	zval *sql_query, *number, dialect = {};

	phalcon_fetch_params(0, 2, 0, &sql_query, &number);

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);
	PHALCON_RETURN_CALL_METHOD(&dialect, "limit", sql_query, number);
}

/**
 * Generates SQL checking for the existence of a schema.table
 *
 * <code>
 * 	var_dump($connection->tableExists("blog", "posts"));
 * </code>
 *
 * @param string $tableName
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter, tableExists){

	zval *table_name, *schema_name = NULL, dialect = {}, sql = {}, fetch_num = {}, num = {};

	phalcon_fetch_params(1, 1, 1, &table_name, &schema_name);

	if (!schema_name) {
		schema_name = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_MM_CALL_METHOD(&sql, &dialect, "tableexists", table_name, schema_name);
	PHALCON_MM_ADD_ENTRY(&sql);

	ZVAL_LONG(&fetch_num, PDO_FETCH_NUM);

	PHALCON_MM_CALL_METHOD(&num, getThis(), "fetchone", &sql, &fetch_num);
	PHALCON_MM_ADD_ENTRY(&num);

	phalcon_array_fetch_long(return_value, &num, 0, PH_COPY);
	RETURN_MM();
}

/**
 * Generates SQL checking for the existence of a schema.view
 *
 *<code>
 * var_dump($connection->viewExists("active_users", "posts"));
 *</code>
 *
 * @param string $viewName
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter, viewExists){

	zval *view_name, *schema_name = NULL, dialect = {}, sql = {}, fetch_num = {}, num = {};

	phalcon_fetch_params(0, 1, 1, &view_name, &schema_name);

	if (!schema_name) {
		schema_name = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(&sql, &dialect, "viewexists", view_name, schema_name);

	ZVAL_LONG(&fetch_num, PDO_FETCH_NUM);

	PHALCON_CALL_METHOD(&num, getThis(), "fetchone", &sql, &fetch_num);
	zval_ptr_dtor(&sql);

	phalcon_array_fetch_long(return_value, &num, 0, PH_NOISY|PH_COPY);
	zval_ptr_dtor(&num);
}

/**
 * Returns a SQL modified with a FOR UPDATE clause
 *
 * @param string $sqlQuery
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter, forUpdate){

	zval *sql_query, dialect = {};

	phalcon_fetch_params(0, 1, 0, &sql_query);

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);
	PHALCON_RETURN_CALL_METHOD(&dialect, "forupdate", sql_query);
}

/**
 * Returns a SQL modified with a LOCK IN SHARE MODE clause
 *
 * @param string $sqlQuery
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter, sharedLock){

	zval *sql_query, dialect = {};

	phalcon_fetch_params(0, 1, 0, &sql_query);

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);
	PHALCON_RETURN_CALL_METHOD(&dialect, "sharedlock", sql_query);
}

/**
 * Creates a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param array $definition
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, createTable){

	zval *table_name, *schema_name, *definition, exception_message, columns, dialect = {}, sql = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &definition);

	if (Z_TYPE_P(definition) != IS_ARRAY) {
		PHALCON_CONCAT_SVS(&exception_message, "Invalid definition to create the table '", table_name, "'");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_db_exception_ce, &exception_message);
		return;
	}

	if (!phalcon_array_isset_fetch_str(&columns, definition, SL("columns"), PH_READONLY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The table must contain at least one column");
		return;
	}

	if (!phalcon_fast_count_ev(&columns)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The table must contain at least one column");
		return;
	}

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(&sql, &dialect, "createtable", table_name, schema_name, definition);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Drops a table from a schema/database
 *
 * @param string $tableName
 * @param   string $schemaName
 * @param boolean $ifExists
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, dropTable){

	zval *table_name, *schema_name = NULL, *if_exists = NULL, dialect = {}, sql = {};

	phalcon_fetch_params(0, 1, 2, &table_name, &schema_name, &if_exists);

	if (!schema_name) {
		schema_name = &PHALCON_GLOBAL(z_null);
	}

	if (!if_exists) {
		if_exists = &PHALCON_GLOBAL(z_true);
	}

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(&sql, &dialect, "droptable", table_name, schema_name, if_exists);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Creates a view
 *
 * @param string $tableName
 * @param array $definition
 * @param string $schemaName
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, createView){

	zval *view_name, *definition, *schema_name = NULL, exception_message = {}, dialect = {}, sql = {};

	phalcon_fetch_params(0, 2, 1, &view_name, &definition, &schema_name);

	if (!schema_name) {
		schema_name = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(definition) != IS_ARRAY) {
		PHALCON_CONCAT_SVS(&exception_message, "Invalid definition to create the view '", view_name, "'");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_db_exception_ce, &exception_message);
		return;
	}

	if (!phalcon_array_isset_str(definition, SL("sql"))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The table must contain at least one column");
		return;
	}

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(&sql, &dialect, "createview", view_name, definition, schema_name);
	PHALCON_CALL_METHOD(return_value, getThis(), "execute", &sql);
	zval_ptr_dtor(&sql);
}

/**
 * Drops a view
 *
 * @param string $viewName
 * @param   string $schemaName
 * @param boolean $ifExists
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, dropView){

	zval *view_name, *schema_name = NULL, *if_exists = NULL, dialect = {}, sql = {};

	phalcon_fetch_params(0, 1, 2, &view_name, &schema_name, &if_exists);

	if (!schema_name) {
		schema_name = &PHALCON_GLOBAL(z_null);
	}

	if (!if_exists) {
		if_exists = &PHALCON_GLOBAL(z_true);
	}

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(&sql, &dialect, "dropview", view_name, schema_name, if_exists);
	PHALCON_CALL_METHOD(return_value, getThis(), "execute", &sql);
	zval_ptr_dtor(&sql);
}

/**
 * Adds a column to a table
 *
 * @param string $tableName
 * @param 	string $schemaName
 * @param Phalcon\Db\ColumnInterface $column
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, addColumn){

	zval *table_name, *schema_name, *column, dialect = {}, sql = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &column);

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(&sql, &dialect, "addcolumn", table_name, schema_name, column);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Modifies a table column based on a definition
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\ColumnInterface $column
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, modifyColumn){

	zval *table_name, *schema_name, *column, dialect = {}, sql = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &column);

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(&sql, &dialect, "modifycolumn", table_name, schema_name, column);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Drops a column from a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param string $columnName
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, dropColumn){

	zval *table_name, *schema_name, *column_name, dialect = {}, sql = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &column_name);

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(&sql, &dialect, "dropcolumn", table_name, schema_name, column_name);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Adds an index to a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\IndexInterface $index
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, addIndex){

	zval *table_name, *schema_name, *index, dialect = {}, sql = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &index);

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(&sql, &dialect, "addindex", table_name, schema_name, index);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Drop an index from a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param string $indexName
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, dropIndex){

	zval *table_name, *schema_name, *index_name, dialect = {}, sql = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &index_name);

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(&sql, &dialect, "dropindex", table_name, schema_name, index_name);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Adds a primary key to a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\IndexInterface $index
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, addPrimaryKey){

	zval *table_name, *schema_name, *index, dialect = {}, sql = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &index);

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(&sql, &dialect, "addprimarykey", table_name, schema_name, index);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Drops a table's primary key
 *
 * @param string $tableName
 * @param string $schemaName
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, dropPrimaryKey){

	zval *table_name, *schema_name, dialect = {}, sql = {};

	phalcon_fetch_params(0, 2, 0, &table_name, &schema_name);

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(&sql, &dialect, "dropprimarykey", table_name, schema_name);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Adds a foreign key to a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\ReferenceInterface $reference
 * @return boolean true
 */
PHP_METHOD(Phalcon_Db_Adapter, addForeignKey){

	zval *table_name, *schema_name, *reference, dialect = {}, sql = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &reference);

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(&sql, &dialect, "addforeignkey", table_name, schema_name, reference);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Drops a foreign key from a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param string $referenceName
 * @return boolean true
 */
PHP_METHOD(Phalcon_Db_Adapter, dropForeignKey){

	zval *table_name, *schema_name, *reference_name, dialect = {}, sql = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &reference_name);

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(&sql, &dialect, "dropforeignkey", table_name, schema_name, reference_name);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Returns the SQL column definition from a column
 *
 * @param Phalcon\Db\ColumnInterface $column
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter, getColumnDefinition){

	zval *column, dialect = {};

	phalcon_fetch_params(0, 1, 0, &column);

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);
	PHALCON_RETURN_CALL_METHOD(&dialect, "getcolumndefinition", column);
}

/**
 * List all tables on a database
 *
 *<code>
 * 	print_r($connection->listTables("blog"));
 *</code>
 *
 * @param string $schemaName
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter, listTables){

	zval *schema_name = NULL, dialect = {}, sql = {}, fetch_num = {}, tables = {}, *table;

	phalcon_fetch_params(1, 0, 1, &schema_name);

	if (!schema_name) {
		schema_name = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	/**
	 * Get the SQL to list the tables
	 */
	PHALCON_MM_CALL_METHOD(&sql, &dialect, "listtables", schema_name);
	PHALCON_MM_ADD_ENTRY(&sql);

	/**
	 * Use fetch Num
	 */
	ZVAL_LONG(&fetch_num, PDO_FETCH_NUM);

	/**
	 * Execute the SQL returning the tables
	 */
	PHALCON_MM_CALL_METHOD(&tables, getThis(), "fetchall", &sql, &fetch_num);
	PHALCON_MM_ADD_ENTRY(&tables);

	if (Z_TYPE(tables) == IS_ARRAY) {
		array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL(tables)));
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(tables), table) {
			zval table_name = {};
			if (phalcon_array_isset_fetch_long(&table_name, table, 0, PH_READONLY)) {
				phalcon_array_append(return_value, &table_name, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();
	}
	RETURN_MM();
}

/**
 * List all views on a database
 *
 *<code>
 *	print_r($connection->listViews("blog")); ?>
 *</code>
 *
 * @param string $schemaName
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter, listViews){

	zval *schema_name = NULL, dialect = {}, sql = {}, fetch_num = {}, tables = {}, *table;

	phalcon_fetch_params(0, 0, 1, &schema_name);

	if (!schema_name) {
		schema_name = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	/**
	 * Get the SQL to list the tables
	 */
	PHALCON_CALL_METHOD(&sql, &dialect, "listviews", schema_name);

	/**
	 * Use fetch Num
	 */
	ZVAL_LONG(&fetch_num, PDO_FETCH_NUM);

	/**
	 * Execute the SQL returning the tables
	 */
	PHALCON_CALL_METHOD(&tables, getThis(), "fetchall", &sql, &fetch_num);
	zval_ptr_dtor(&sql);

	if (Z_TYPE(tables) == IS_ARRAY) {
		array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL(tables)));

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(tables), table) {
			zval table_name = {};
			if (phalcon_array_isset_fetch_long(&table_name, table, 0, PH_READONLY)) {
				phalcon_array_append(return_value, &table_name, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();
	}
	zval_ptr_dtor(&tables);
}

/**
 * Lists table indexes
 *
 *<code>
 *	print_r($connection->describeIndexes('robots_parts'));
 *</code>
 *
 * @param string $table
 * @param string $schema
 * @return Phalcon\Db\Index[]
 */
PHP_METHOD(Phalcon_Db_Adapter, describeIndexes){

	zval *table, *schema = NULL, dialect = {}, fetch_num = {}, sql = {}, describe = {}, *index, indexes = {}, *index_columns;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(1, 1, 1, &table, &schema);

	if (!schema) {
		schema = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	/**
	 * We're using FETCH_NUM to fetch the columns
	 */
	ZVAL_LONG(&fetch_num, PDO_FETCH_NUM);

	/**
	 * Get the SQL required to describe indexes from the Dialect
	 */
	PHALCON_MM_CALL_METHOD(&sql, &dialect, "describeindexes", table, schema);
	PHALCON_MM_ADD_ENTRY(&sql);

	/**
	 * Cryptic Guide: 2: table, 3: from, 4: to
	 */
	PHALCON_MM_CALL_METHOD(&describe, getThis(), "fetchall", &sql, &fetch_num);
	PHALCON_MM_ADD_ENTRY(&describe);

	array_init(&indexes);
	PHALCON_MM_ADD_ENTRY(&indexes);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(describe), index) {
		zval key_name = {}, column_name = {};
		phalcon_array_fetch_long(&key_name, index, 2, PH_NOISY|PH_READONLY);
		phalcon_array_fetch_long(&column_name, index, 4, PH_NOISY|PH_READONLY);
		phalcon_array_append_multi_2(&indexes, &key_name, &column_name, PH_COPY);
	} ZEND_HASH_FOREACH_END();

	array_init(return_value);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(indexes), idx, str_key, index_columns) {
		zval name = {}, index = {};
		if (str_key) {
			ZVAL_STR(&name, str_key);
		} else {
			ZVAL_LONG(&name, idx);
		}

		/**
		 * Every index is abstracted using a Phalcon\Db\Index instance
		 */
		object_init_ex(&index, phalcon_db_index_ce);
		PHALCON_MM_CALL_METHOD(NULL, &index, "__construct", &name, index_columns);

		phalcon_array_update(return_value, &name, &index, 0);
	} ZEND_HASH_FOREACH_END();
	RETURN_MM();
}

/**
 * Lists table references
 *
 *<code>
 * print_r($connection->describeReferences('robots_parts'));
 *</code>
 *
 * @param string $table
 * @param string $schema
 * @return Phalcon\Db\Reference[]
 */
PHP_METHOD(Phalcon_Db_Adapter, describeReferences){

	zval *table, *schema = NULL, dialect = {}, fetch_num = {}, sql = {}, empty_arr = {}, references = {}, describe = {}, *reference, *array_reference;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(1, 1, 1, &table, &schema);

	if (!schema) {
		schema = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	/**
	 * We're using FETCH_NUM to fetch the columns
	 */
	ZVAL_LONG(&fetch_num, PDO_FETCH_NUM);

	/**
	 * Get the SQL required to describe the references from the Dialect
	 */
	PHALCON_MM_CALL_METHOD(&sql, &dialect, "describereferences", table, schema);
	PHALCON_MM_ADD_ENTRY(&sql);

	array_init(&empty_arr);
	PHALCON_MM_ADD_ENTRY(&empty_arr);
	array_init(&references);
	PHALCON_MM_ADD_ENTRY(&references);

	/**
	 * Execute the SQL returning the
	 */
	PHALCON_MM_CALL_METHOD(&describe, getThis(), "fetchall", &sql, &fetch_num);
	PHALCON_MM_ADD_ENTRY(&describe);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(describe), reference) {
		zval constraint_name = {}, referenced_schema = {}, referenced_table = {}, reference_array = {}, column_name = {}, referenced_columns = {};
		phalcon_array_fetch_long(&constraint_name, reference, 2, PH_NOISY|PH_READONLY);
		if (!phalcon_array_isset(&references, &constraint_name)) {
			phalcon_array_fetch_long(&referenced_schema, reference, 3, PH_NOISY|PH_READONLY);
			phalcon_array_fetch_long(&referenced_table, reference, 4, PH_NOISY|PH_READONLY);

			array_init_size(&reference_array, 4);
			phalcon_array_update_str(&reference_array, SL("referencedSchema"), &referenced_schema, PH_COPY);
			phalcon_array_update_str(&reference_array, SL("referencedTable"), &referenced_table, PH_COPY);
			phalcon_array_update_str(&reference_array, SL("columns"), &empty_arr, PH_COPY);
			phalcon_array_update_str(&reference_array, SL("referencedColumns"), &empty_arr, PH_COPY);

			phalcon_array_update(&references, &constraint_name, &reference_array, 0);
		}

		phalcon_array_fetch_long(&column_name, reference, 1, PH_NOISY|PH_READONLY);
		phalcon_array_update_zval_str_append_multi_3(&references, &constraint_name, SL("columns"), &column_name, PH_COPY);

		phalcon_array_fetch_long(&referenced_columns, reference, 5, PH_NOISY|PH_READONLY);
		phalcon_array_update_zval_str_append_multi_3(&references, &constraint_name, SL("referencedColumns"), &referenced_columns, PH_COPY);
	} ZEND_HASH_FOREACH_END();

	array_init(return_value);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(references), idx, str_key, array_reference) {
		zval name = {}, referenced_schema = {}, referenced_table = {}, columns = {}, referenced_columns = {}, definition = {}, reference = {};
		if (str_key) {
			ZVAL_STR(&name, str_key);
		} else {
			ZVAL_LONG(&name, idx);
		}

		phalcon_array_fetch_str(&referenced_schema, array_reference, SL("referencedSchema"), PH_NOISY|PH_READONLY);
		phalcon_array_fetch_str(&referenced_table, array_reference, SL("referencedTable"), PH_NOISY|PH_READONLY);
		phalcon_array_fetch_str(&columns, array_reference, SL("columns"), PH_NOISY|PH_READONLY);
		phalcon_array_fetch_str(&referenced_columns, array_reference, SL("referencedColumns"), PH_NOISY|PH_READONLY);

		array_init_size(&definition, 4);
		phalcon_array_update_str(&definition, SL("referencedSchema"), &referenced_schema, PH_COPY);
		phalcon_array_update_str(&definition, SL("referencedTable"), &referenced_table, PH_COPY);
		phalcon_array_update_str(&definition, SL("columns"), &columns, PH_COPY );
		phalcon_array_update_str(&definition, SL("referencedColumns"), &referenced_columns, PH_COPY);
		PHALCON_MM_ADD_ENTRY(&definition);

		object_init_ex(&reference, phalcon_db_reference_ce);
		PHALCON_CALL_METHOD(NULL, &reference, "__construct", &name, &definition);

		phalcon_array_update(return_value, &name, &reference, 0);
	} ZEND_HASH_FOREACH_END();
	RETURN_MM();
}

/**
 * Gets creation options from a table
 *
 *<code>
 * print_r($connection->tableOptions('robots'));
 *</code>
 *
 * @param string $tableName
 * @param string $schemaName
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter, tableOptions){

	zval *table_name, *schema_name = NULL, dialect = {}, sql = {}, fetch_assoc = {}, describe = {}, first = {};

	phalcon_fetch_params(1, 1, 1, &table_name, &schema_name);

	if (!schema_name) {
		schema_name = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_MM_CALL_METHOD(&sql, &dialect, "tableoptions", table_name, schema_name);
	PHALCON_MM_ADD_ENTRY(&sql);
	if (zend_is_true(&sql)) {
		ZVAL_LONG(&fetch_assoc, PDO_FETCH_ASSOC);

		PHALCON_MM_CALL_METHOD(&describe, getThis(), "fetchall", &sql, &fetch_assoc);
		PHALCON_MM_ADD_ENTRY(&describe);

		phalcon_array_fetch_long(&first, &describe, 0, PH_NOISY|PH_READONLY);
		RETURN_MM_CTOR(&first);
	}

	RETURN_MM_EMPTY_ARRAY();
}

/**
 * Creates a new savepoint
 *
 * @param string $name
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, createSavepoint){

	zval *name, dialect = {}, supports_sp = {}, sql = {};

	phalcon_fetch_params(0, 1, 0, &name);

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(&supports_sp, &dialect, "supportssavepoints");
	if (!zend_is_true(&supports_sp)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Savepoints are not supported by this database adapter.");
		return;
	}

	PHALCON_CALL_METHOD(&sql, &dialect, "createsavepoint", name);
	PHALCON_CALL_METHOD(return_value, getThis(), "execute", &sql);
	zval_ptr_dtor(&sql);
}

/**
 * Releases given savepoint
 *
 * @param string $name
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, releaseSavepoint){

	zval *name, dialect = {}, supports_sp = {}, supports_rsp = {}, sql = {};

	phalcon_fetch_params(0, 1, 0, &name);

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(&supports_sp, &dialect, "supportssavepoints");
	if (!zend_is_true(&supports_sp)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Savepoints are not supported by this database adapter");
		return;
	}

	PHALCON_CALL_METHOD(&supports_rsp, &dialect, "supportsreleasesavepoints");
	if (!zend_is_true(&supports_rsp)) {
		RETURN_FALSE;
	}

	PHALCON_CALL_METHOD(&sql, &dialect, "releasesavepoint", name);
	PHALCON_CALL_METHOD(return_value, getThis(), "execute", &sql);
	zval_ptr_dtor(&sql);
}

/**
 * Rollbacks given savepoint
 *
 * @param string $name
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, rollbackSavepoint){

	zval *name, dialect = {}, supports_sp = {}, sql = {};

	phalcon_fetch_params(0, 1, 0, &name);

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(&supports_sp, &dialect, "supportssavepoints");
	if (!zend_is_true(&supports_sp)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Savepoints are not supported by this database adapter");
		return;
	}

	PHALCON_CALL_METHOD(&sql, &dialect, "rollbacksavepoint", name);
	PHALCON_CALL_METHOD(return_value, getThis(), "execute", &sql);
	zval_ptr_dtor(&sql);
}

/**
 * Set if nested transactions should use savepoints
 *
 * @param boolean $nestedTransactionsWithSavepoints
 * @return Phalcon\Db\AdapterInterface
 */
PHP_METHOD(Phalcon_Db_Adapter, setNestedTransactionsWithSavepoints){

	zval *nested_transactions_with_savepoints, transaction_level = {}, dialect = {}, supports_sp = {};

	phalcon_fetch_params(0, 1, 0, &nested_transactions_with_savepoints);

	phalcon_read_property(&transaction_level, getThis(), SL("_transactionLevel"), PH_NOISY|PH_READONLY);
	if (PHALCON_GT_LONG(&transaction_level, 0)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Nested transaction with savepoints behavior cannot be changed while a transaction is open");
		return;
	}

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(&supports_sp, &dialect, "supportssavepoints");
	if (!zend_is_true(&supports_sp)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Savepoints are not supported by this database adapter");
		return;
	}

	phalcon_update_property(getThis(), SL("_transactionsWithSavepoints"), nested_transactions_with_savepoints);

	RETURN_THIS();
}

/**
 * Returns if nested transactions should use savepoints
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, isNestedTransactionsWithSavepoints){


	RETURN_MEMBER(getThis(), "_transactionsWithSavepoints");
}

/**
 * Returns the savepoint name to use for nested transactions
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter, getNestedTransactionSavepointName){

	zval transaction_level = {};

	phalcon_read_property(&transaction_level, getThis(), SL("_transactionLevel"), PH_NOISY|PH_READONLY);
	PHALCON_CONCAT_SV(return_value, "PHALCON_SAVEPOINT_", &transaction_level);
}

/**
 * Returns the default identity value to be inserted in an identity column
 *
 *<code>
 * //Inserting a new robot with a valid default value for the column 'id'
 * $success = $connection->insert(
 *     "robots",
 *     array($connection->getDefaultIdValue(), "Astro Boy", 1952),
 *     array("id", "name", "year")
 * );
 *</code>
 *
 * @return Phalcon\Db\RawValue
 */
PHP_METHOD(Phalcon_Db_Adapter, getDefaultIdValue){

	zval default_value = {};

	ZVAL_STRING(&default_value, "null");
	object_init_ex(return_value, phalcon_db_rawvalue_ce);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", &default_value);
	zval_ptr_dtor(&default_value);
}

/**
 * Check whether the database system requires a sequence to produce auto-numeric values
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, supportSequences){


	RETURN_FALSE;
}

/**
 * Check whether the database system requires an explicit value for identity columns
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, useExplicitIdValue){


	RETURN_FALSE;
}

/**
 * Return descriptor used to connect to the active database
 *
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter, getDescriptor){


	RETURN_MEMBER(getThis(), "_descriptor");
}

/**
 * Gets the active connection unique identifier
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter, getConnectionId){


	RETURN_MEMBER(getThis(), "_connectionId");
}

/**
 * Active SQL statement in the object
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter, getSQLStatement){


	RETURN_MEMBER(getThis(), "_sqlStatement");
}

/**
 * Active SQL statement in the object with replace bound paramters
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter, getExpectSQLStatement){

	zval sql_statement = {}, sql_variables = {}, *value, sql = {};
	zend_string *str_key;

	PHALCON_MM_INIT();
	phalcon_read_property(&sql_statement, getThis(), SL("_sqlStatement"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&sql_variables, getThis(), SL("_sqlVariables"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(sql_variables) != IS_ARRAY) {
		RETURN_MM_CTOR(&sql_statement);
	}

	PHALCON_MM_ZVAL_COPY(&sql, &sql_statement);

	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL(sql_variables), str_key, value) {
		zval pattern = {}, escaped_value = {}, replaced_str = {};

		if (str_key) {
			zval wildcard = {}, tmp = {};
			ZVAL_STR(&wildcard, str_key);
			phalcon_addslashes(&tmp, &wildcard);
			PHALCON_CONCAT_SVS(&pattern, "#", &tmp , "#");
			zval_ptr_dtor(&tmp);
			PHALCON_MM_ADD_ENTRY(&pattern);
		} else {
			PHALCON_MM_ZVAL_STRING(&pattern, "#\?#");
		}

		PHALCON_MM_CALL_METHOD(&escaped_value, getThis(), "escapevalue", value);
		PHALCON_MM_ADD_ENTRY(&escaped_value);
		phalcon_fast_preg_replace(&replaced_str, &pattern, &escaped_value, &sql);
		PHALCON_MM_ADD_ENTRY(&replaced_str);
		ZVAL_COPY_VALUE(&sql, &replaced_str);
	} ZEND_HASH_FOREACH_END();
	RETURN_MM_CTOR(&sql);
}

/**
 * Active SQL statement in the object
 *
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter, getSQLVariables){


	RETURN_MEMBER(getThis(), "_sqlVariables");
}

/**
 * Active SQL statement in the object
 *
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter, getSQLBindTypes){


	RETURN_MEMBER(getThis(), "_sqlBindTypes");
}

/**
 * Returns type of database system the adapter is used for
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter, getType){


	RETURN_MEMBER(getThis(), "_type");
}

/**
 * Returns the name of the dialect used
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter, getDialectType){


	RETURN_MEMBER(getThis(), "_dialectType");
}

/**
 * Create a select builder
 *
 * @return Phalcon\Db\Builder\Select
 */
PHP_METHOD(Phalcon_Db_Adapter, createSelectBuilder){
	zval *table;

	phalcon_fetch_params(0, 1, 0, &table);

	object_init_ex(return_value, phalcon_db_builder_select_ce);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", table, getThis());
}

/**
 * Create a update builder
 *
 * @return Phalcon\Db\Builder\Update
 */
PHP_METHOD(Phalcon_Db_Adapter, createUpdateBuilder){
	zval *table;

	phalcon_fetch_params(0, 1, 0, &table);

	object_init_ex(return_value, phalcon_db_builder_update_ce);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", table, getThis());
}

/**
 * Create a insert builder
 *
 * @return Phalcon\Db\Builder\Insert
 */
PHP_METHOD(Phalcon_Db_Adapter, createInsertBuilder){
	zval *table;

	phalcon_fetch_params(0, 1, 0, &table);

	object_init_ex(return_value, phalcon_db_builder_insert_ce);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", table, getThis());
}

/**
 * Create a delete builder
 *
 * @return Phalcon\Db\Builder\Delete
 */
PHP_METHOD(Phalcon_Db_Adapter, createDeleteBuilder){
	zval *table;

	phalcon_fetch_params(0, 1, 0, &table);

	object_init_ex(return_value, phalcon_db_builder_delete_ce);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", table, getThis());
}
