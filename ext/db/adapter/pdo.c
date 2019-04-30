
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

#include "db/adapter/pdo.h"
#include "db/adapter.h"
#include "db/adapterinterface.h"
#include "db/exception.h"
#include "db/result/pdo.h"
#include "db/column.h"
#include "debug.h"

#include <ext/pdo/php_pdo_driver.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/object.h"
#include "kernel/hash.h"
#include "kernel/concat.h"
#include "kernel/string.h"
#include "kernel/operators.h"
#include "kernel/debug.h"

#include "interned-strings.h"

/**
 * Phalcon\Db\Adapter\Pdo
 *
 * Phalcon\Db\Adapter\Pdo is the Phalcon\Db that internally uses PDO to connect to a database
 *
 *<code>
 *	$connection = new Phalcon\Db\Adapter\Pdo\Mysql(array(
 *		'host' => '192.168.0.11',
 *		'username' => 'sigma',
 *		'password' => 'secret',
 *		'dbname' => 'blog',
 *		'port' => '3306'
 *	));
 *</code>
 */
zend_class_entry *phalcon_db_adapter_pdo_ce;

PHP_METHOD(Phalcon_Db_Adapter_Pdo, __construct);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, connect);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, prepare);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, executePrepared);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, query);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, execute);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, affectedRows);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, close);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, escapeIdentifier);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, escapeValue);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, escapeString);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, convertBoundParams);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, lastInsertId);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, begin);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, rollback);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, commit);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getTransactionLevel);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, isUnderTransaction);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getInternalHandler);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getErrorInfo);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, descriptor)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_prepare, 0, 0, 1)
	ZEND_ARG_INFO(0, sqlStatement)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_executeprepared, 0, 0, 1)
	ZEND_ARG_INFO(0, statement)
	ZEND_ARG_TYPE_INFO(0, placeholders, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, dataTypes, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_begin, 0, 0, 0)
	ZEND_ARG_INFO(0, nesting)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_rollback, 0, 0, 0)
	ZEND_ARG_INFO(0, nesting)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_commit, 0, 0, 0)
	ZEND_ARG_INFO(0, nesting)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_db_adapter_pdo_method_entry[] = {
	PHP_ME(Phalcon_Db_Adapter_Pdo, __construct, arginfo_phalcon_db_adapter_pdo___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Db_Adapter_Pdo, connect, arginfo_phalcon_db_adapterinterface_connect, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, prepare, arginfo_phalcon_db_adapter_pdo_prepare, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, executePrepared, arginfo_phalcon_db_adapter_pdo_executeprepared, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, query, arginfo_phalcon_db_adapterinterface_query, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, execute, arginfo_phalcon_db_adapterinterface_execute, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, affectedRows, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, close, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, escapeIdentifier, arginfo_phalcon_db_adapterinterface_escapeidentifier, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, escapeValue, arginfo_phalcon_db_adapterinterface_escapevalue, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, escapeString, arginfo_phalcon_db_adapterinterface_escapestring, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, convertBoundParams, arginfo_phalcon_db_adapterinterface_convertboundparams, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, lastInsertId, arginfo_phalcon_db_adapterinterface_lastinsertid, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, begin, arginfo_phalcon_db_adapter_pdo_begin, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, rollback, arginfo_phalcon_db_adapter_pdo_rollback, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, commit, arginfo_phalcon_db_adapter_pdo_commit, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, getTransactionLevel, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, isUnderTransaction, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, getInternalHandler, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, getErrorInfo, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Db\Adapter\Pdo initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Adapter_Pdo){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Db\\Adapter, Pdo, db_adapter_pdo, phalcon_db_adapter_ce, phalcon_db_adapter_pdo_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_db_adapter_pdo_ce, SL("_pdo"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_pdo_ce, SL("_affectedRows"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_db_adapter_pdo_ce, SL("_transactionLevel"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_pdo_ce, SL("_schema"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Constructor for Phalcon\Db\Adapter\Pdo
 *
 * @param array $descriptor
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, __construct){

	zval *descriptor;

	phalcon_fetch_params(0, 1, 0, &descriptor);

	if (Z_TYPE_P(descriptor) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The descriptor must be an array");
		return;
	}

	PHALCON_CALL_METHOD(NULL, getThis(), "connect", descriptor);
	PHALCON_CALL_PARENT(NULL, phalcon_db_adapter_pdo_ce, getThis(), "__construct", descriptor);
}

/**
 * This method is automatically called in Phalcon\Db\Adapter\Pdo constructor.
 * Call it when you need to restore a database connection
 *
 *<code>
 * //Make a connection
 * $connection = new Phalcon\Db\Adapter\Pdo\Mysql(array(
 *  'host' => '192.168.0.11',
 *  'username' => 'sigma',
 *  'password' => 'secret',
 *  'dbname' => 'blog',
 * ));
 *
 * //Reconnect
 * $connection->connect();
 * </code>
 *
 * @param 	array $descriptor
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, connect)
{
	zval *desc = NULL, descriptor = {}, username = {}, password = {}, options = {}, dsn_parts = {}, *value, dsn_attributes = {}, pdo_type = {}, dsn = {}, persistent = {}, pdo = {};
	zend_class_entry *ce;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(1, 0, 1, &desc);

	if (!desc || Z_TYPE_P(desc) == IS_NULL) {
		phalcon_read_property(&descriptor, getThis(), SL("_descriptor"), PH_SEPARATE);
	} else {
		ZVAL_DUP(&descriptor, desc);
	}
	PHALCON_MM_ADD_ENTRY(&descriptor);

	/**
	 * Check for a username or use null as default
	 */
	if (phalcon_array_isset_fetch_str(&username, &descriptor, SL("username"), PH_COPY)) {
		phalcon_array_unset_str(&descriptor, SL("username"), 0);
	}

	/**
	 * Check for a password or use null as default
	 */
	if (phalcon_array_isset_fetch_str(&password, &descriptor, SL("password"), PH_COPY)) {
		phalcon_array_unset_str(&descriptor, SL("password"), 0);
	}

	/**
	 * Check if the developer has defined custom options or create one from scratch
	 */
	if (phalcon_array_isset_fetch_str(&options, &descriptor, SL("options"), PH_COPY)) {
		phalcon_array_unset_str(&descriptor, SL("options"), 0);
	} else {
		array_init(&options);
	}
	PHALCON_MM_ADD_ENTRY(&options);

	/**
	 * Remove the dialectClass from the descriptor if any
	 */
	if (phalcon_array_isset_str(&descriptor, SL("dialectClass"))) {
		phalcon_array_unset_str(&descriptor, SL("dialectClass"), 0);
	}

	/**
	 * Default options
	 */
	phalcon_array_update_long_long(&options, PDO_ATTR_ERRMODE, PDO_ERRMODE_EXCEPTION, PH_COPY);

	/**
	 * Check if the connection must be persistent
	 */
	if (phalcon_array_isset_fetch_str(&persistent, &descriptor, SL("persistent"), PH_READONLY)) {
		if (zend_is_true(&persistent)) {
			phalcon_array_update_long_bool(&options, PDO_ATTR_PERSISTENT, 1, 0);
		}
		phalcon_array_unset_str(&descriptor, SL("persistent"), 0);
	}

	/**
	 * Check if the user has defined a custom dsn
	 */
	if (!phalcon_array_isset_fetch_str(&dsn_attributes, &descriptor, SL("dsn"), PH_READONLY)) {
		array_init(&dsn_parts);

		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(descriptor), idx, str_key, value) {
			zval key = {}, dsn_attribute = {};
			if (str_key) {
				ZVAL_STR(&key, str_key);
			} else {
				ZVAL_LONG(&key, idx);
			}
			PHALCON_CONCAT_VSV(&dsn_attribute, &key, "=", value);
			phalcon_array_append(&dsn_parts, &dsn_attribute, PH_READONLY);
		} ZEND_HASH_FOREACH_END();

		phalcon_fast_join_str(&dsn_attributes, SL(";"), &dsn_parts);
		zval_ptr_dtor(&dsn_parts);
		PHALCON_MM_ADD_ENTRY(&dsn_attributes);
	}

	phalcon_read_property(&pdo_type, getThis(), SL("_type"), PH_NOISY|PH_READONLY);

	PHALCON_CONCAT_VSV(&dsn, &pdo_type, ":", &dsn_attributes);
	PHALCON_MM_ADD_ENTRY(&dsn);

	/**
	 * Create the connection using PDO
	 */
	ce = phalcon_fetch_str_class(SL("PDO"), ZEND_FETCH_CLASS_AUTO);

	PHALCON_OBJECT_INIT(&pdo, ce);
	PHALCON_MM_ADD_ENTRY(&pdo);
	PHALCON_MM_CALL_METHOD(NULL, &pdo, "__construct", &dsn, &username, &password, &options);

	phalcon_update_property(getThis(), SL("_pdo"), &pdo);
	RETURN_MM();
}

/**
 * Returns a PDO prepared statement to be executed with 'executePrepared'
 *
 *<code>
 * $statement = $connection->prepare('SELECT * FROM robots WHERE name = :name');
 * $pdoResult = $connection->executePrepared($statement, array('name' => 'Voltron'));
 *</code>
 *
 * @param string $sqlStatement
 * @return \PDOStatement
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, prepare){

	zval *sql_statement, pdo = {};

	phalcon_fetch_params(0, 1, 0, &sql_statement);

	phalcon_update_property(getThis(), SL("_sqlStatement"), sql_statement);

	phalcon_read_property(&pdo, getThis(), SL("_pdo"), PH_NOISY|PH_READONLY);
	PHALCON_RETURN_CALL_METHOD(&pdo, "prepare", sql_statement);
}

/**
 * Executes a prepared statement binding. This function uses integer indexes starting from zero
 *
 *<code>
 * $statement = $connection->prepare('SELECT * FROM robots WHERE name = :name');
 * $pdoResult = $connection->executePrepared($statement, array('name' => 'Voltron'));
 *</code>
 *
 * @param \PDOStatement $statement
 * @param array $placeholders
 * @param array $dataTypes
 * @return \PDOStatement
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, executePrepared){

	zval *statement, *_placeholders = NULL, *_data_types = NULL, placeholders = {}, data_types = {}, event_name = {}, status = {};
	zval *value = NULL, profiler = {}, sql_statement = {};
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(1, 1, 2, &statement, &_placeholders, &_data_types);

	if (!_placeholders || Z_TYPE_P(_placeholders) != IS_ARRAY) {
		ZVAL_NULL(&placeholders);
	} else {
		PHALCON_MM_ZVAL_DUP(&placeholders, _placeholders);
	}

	if (!_data_types || Z_TYPE_P(_data_types) != IS_ARRAY) {
		ZVAL_NULL(&data_types);
	} else {
		PHALCON_MM_ZVAL_DUP(&data_types, _data_types);
	}

	if (Z_TYPE(placeholders) == IS_ARRAY) {
		if (Z_TYPE(data_types) != IS_ARRAY) {
			array_init(&data_types);
			PHALCON_MM_ADD_ENTRY(&data_types);
		}
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(placeholders), idx, str_key, value) {
			zval wildcard = {}, type = {};
			if (str_key) {
				ZVAL_STR(&wildcard, str_key);
			} else {
				ZVAL_LONG(&wildcard, idx);
			}

			if (likely(phalcon_array_isset_fetch(&type, &data_types, &wildcard, PH_READONLY))) {
				/**
				 * The bind type is double so we try to get the double value
				 */
				if (phalcon_compare_strict_long(&type, PHALCON_DB_COLUMN_BIND_PARAM_DECIMAL)) {
					if (Z_TYPE_P(value) != IS_DOUBLE) {
						convert_to_double_ex(value);
					}
					phalcon_array_update_zval_long(&data_types, &wildcard, PHALCON_DB_COLUMN_BIND_PARAM_DECIMAL, 0);
				}
			} else {
				if (Z_TYPE_P(value) == IS_LONG) {
					phalcon_array_update_zval_long(&data_types, &wildcard, PHALCON_DB_COLUMN_BIND_PARAM_INT, 0);
				} else if (Z_TYPE_P(value) == IS_DOUBLE) {
					phalcon_array_update_zval_long(&data_types, &wildcard, PHALCON_DB_COLUMN_BIND_PARAM_DECIMAL, 0);
				} else if (PHALCON_IS_BOOL(value)) {
					phalcon_array_update_zval_long(&data_types, &wildcard, PHALCON_DB_COLUMN_BIND_PARAM_BOOL, 0);
				} else if (Z_TYPE_P(value) == IS_NULL) {
					phalcon_array_update_zval_long(&data_types, &wildcard, PHALCON_DB_COLUMN_BIND_PARAM_NULL, 0);
				} else {
					phalcon_array_update_zval_long(&data_types, &wildcard, PHALCON_DB_COLUMN_BIND_PARAM_STR, 0);
				}
			}
		} ZEND_HASH_FOREACH_END();
	}

	phalcon_update_property(getThis(), SL("_sqlVariables"), &placeholders);
	phalcon_update_property(getThis(), SL("_sqlBindTypes"), &data_types);

	PHALCON_MM_ZVAL_STRING(&event_name, "db:beforeExecutePrepared");
	PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name, statement);
	if (PHALCON_IS_FALSE(&status)) {
		RETURN_FALSE;
	}
	zval_ptr_dtor(&status);

	if (Z_TYPE(placeholders) == IS_ARRAY) {
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(placeholders), idx, str_key, value) {
			zval wildcard = {}, parameter = {}, type = {};
			if (str_key) {
				ZVAL_STR(&wildcard, str_key);
			} else {
				ZVAL_LONG(&wildcard, idx);
			}

			if (Z_TYPE(wildcard) == IS_LONG) {
				phalcon_add_function(&parameter, &wildcard, &PHALCON_GLOBAL(z_one));
			} else {
				ZVAL_COPY(&parameter, &wildcard);
			}
			PHALCON_MM_ADD_ENTRY(&parameter);
			
			if (likely(phalcon_array_isset_fetch(&type, &data_types, &wildcard, PH_READONLY))) {
				/**
				 * 1024 is ignore the bind type
				 */
				ZVAL_MAKE_REF(value);
				if (phalcon_compare_strict_long(&type, PHALCON_DB_COLUMN_BIND_SKIP)) {
					PHALCON_MM_CALL_METHOD(NULL, statement, "bindvalue", &parameter, value);
				} else {
					PHALCON_MM_CALL_METHOD(NULL, statement, "bindvalue", &parameter, value, &type);
				}
				ZVAL_UNREF(value);
			} else {
				ZVAL_MAKE_REF(value);
				PHALCON_MM_CALL_METHOD(NULL, statement, "bindvalue", &parameter, value);
				ZVAL_UNREF(value);
			}
		} ZEND_HASH_FOREACH_END();
	}

	phalcon_read_property(&profiler, getThis(), SL("_profiler"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(profiler) == IS_OBJECT) {
		zval profile_name = {}, profile_data = {};
		ZVAL_STR(&profile_name, IS(db));
		array_init(&profile_data);
		PHALCON_MM_ADD_ENTRY(&profile_data);

		phalcon_read_property(&sql_statement, getThis(), SL("_sqlStatement"), PH_NOISY|PH_READONLY);
		phalcon_array_update_str(&profile_data, SL("sqlStatement"), &sql_statement, PH_COPY);
		phalcon_array_update_str(&profile_data, SL("sqlVariables"), &placeholders, PH_COPY);
		phalcon_array_update_str(&profile_data, SL("sqlBindTypes"), &data_types, PH_COPY);

		PHALCON_MM_CALL_METHOD(NULL, &profiler, "startprofile", &profile_name, &profile_data);
		PHALCON_MM_CALL_METHOD(NULL, statement, "execute");
		PHALCON_MM_CALL_METHOD(NULL, &profiler, "stopprofile");
	} else {
		PHALCON_MM_CALL_METHOD(NULL, statement, "execute");
	}

	PHALCON_MM_ZVAL_STRING(&event_name, "db:afterExecutePrepared");
	PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name, statement);

	if (PHALCON_IS_FALSE(&status)) {
		RETURN_MM_FALSE;
	}

	RETURN_MM_CTOR(statement);
}

/**
 * Sends SQL statements to the database server returning the success state.
 * Use this method only when the SQL statement sent to the server is returning rows
 *
 *<code>
 *	//Querying data
 *	$resultset = $connection->query("SELECT * FROM robots WHERE type='mechanical'");
 *	$resultset = $connection->query("SELECT * FROM robots WHERE type=?", array("mechanical"));
 *</code>
 *
 * @param  string $sqlStatement
 * @param  array $bindParams
 * @param  array $bindTypes
 * @return Phalcon\Db\ResultInterface
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, query){

	zval *sql_statement, *bind_params = NULL, *bind_types = NULL, debug_message = {}, event_name = {}, status = {};
	zval statement = {}, new_statement = {};

	phalcon_fetch_params(1, 1, 2, &sql_statement, &bind_params, &bind_types);

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_CONCAT_SV(&debug_message, "SQL STATEMENT: ", sql_statement);
		PHALCON_DEBUG_LOG(&debug_message);
		zval_ptr_dtor(&debug_message);
		if (bind_params && PHALCON_IS_NOT_EMPTY(bind_params)) {
			ZVAL_STRING(&debug_message, "Bind Params: ");
			PHALCON_DEBUG_LOG(&debug_message);
			zval_ptr_dtor(&debug_message);
			PHALCON_DEBUG_LOG(bind_params);
		}
		if (bind_types && PHALCON_IS_NOT_EMPTY(bind_types)) {
			ZVAL_STRING(&debug_message, "Bind Types: ");
			PHALCON_DEBUG_LOG(&debug_message);
			zval_ptr_dtor(&debug_message);
			PHALCON_DEBUG_LOG(bind_types);
		}
	}

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	phalcon_update_property(getThis(), SL("_sqlStatement"), sql_statement);
	phalcon_update_property(getThis(), SL("_sqlVariables"), bind_params);
	phalcon_update_property(getThis(), SL("_sqlBindTypes"), bind_types);

	PHALCON_MM_ZVAL_STRING(&event_name, "db:beforeQuery");
	PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name, bind_params);
	if (PHALCON_IS_FALSE(&status)) {
		RETURN_FALSE;
	}
	zval_ptr_dtor(&status);

	PHALCON_MM_CALL_METHOD(&statement, getThis(), "prepare", sql_statement);
	PHALCON_MM_ADD_ENTRY(&statement);
	if (Z_TYPE(statement) == IS_OBJECT){
		PHALCON_MM_CALL_METHOD(&new_statement, getThis(), "executeprepared", &statement, bind_params, bind_types);
		PHALCON_MM_ADD_ENTRY(&new_statement);
		ZVAL_COPY_VALUE(&statement, &new_statement);
	}

	PHALCON_MM_ZVAL_STRING(&event_name, "db:afterQuery");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name, &new_statement);

	/**
	 * Execute the afterQuery event if a EventsManager is available
	 */
	if (likely(Z_TYPE(statement) == IS_OBJECT)) {
		object_init_ex(return_value, phalcon_db_result_pdo_ce);
		PHALCON_CALL_METHOD(NULL, return_value, "__construct", getThis(), &statement, sql_statement, bind_params, bind_types);
		RETURN_MM();
	}

	RETURN_MM_CTOR(&statement);
}

/**
 * Sends SQL statements to the database server returning the success state.
 * Use this method only when the SQL statement sent to the server doesn't return any row
 *
 *<code>
 *	//Inserting data
 *	$success = $connection->execute("INSERT INTO robots VALUES (1, 'Astro Boy')");
 *	$success = $connection->execute("INSERT INTO robots VALUES (?, ?)", array(1, 'Astro Boy'));
 *</code>
 *
 * @param  string $sqlStatement
 * @param  array $bindParams
 * @param  array $bindTypes
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, execute){

	zval *sql_statement, *bind_params = NULL, *bind_types = NULL, debug_message = {}, event_name = {}, status = {}, affected_rows = {};
	zval statement = {}, new_statement = {};

	phalcon_fetch_params(1, 1, 2, &sql_statement, &bind_params, &bind_types);

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_CONCAT_SV(&debug_message, "SQL STATEMENT: ", sql_statement);
		PHALCON_DEBUG_LOG(&debug_message);
		zval_ptr_dtor(&debug_message);
		if (bind_params && PHALCON_IS_NOT_EMPTY(bind_params)) {
			ZVAL_STRING(&debug_message, "Bind Params: ");
			PHALCON_DEBUG_LOG(&debug_message);
			zval_ptr_dtor(&debug_message);
			PHALCON_DEBUG_LOG(bind_params);
		}
		if (bind_types && PHALCON_IS_NOT_EMPTY(bind_types)) {
			ZVAL_STRING(&debug_message, "Bind Types: ");
			PHALCON_DEBUG_LOG(&debug_message);
			zval_ptr_dtor(&debug_message);
			PHALCON_DEBUG_LOG(bind_types);
		}
	}

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}


	phalcon_update_property(getThis(), SL("_sqlStatement"), sql_statement);
	phalcon_update_property(getThis(), SL("_sqlVariables"), bind_params);
	phalcon_update_property(getThis(), SL("_sqlBindTypes"), bind_types);

	PHALCON_MM_ZVAL_STRING(&event_name, "db:beforeExecute");
	PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name, bind_params);

	if (PHALCON_IS_FALSE(&status)) {
		RETURN_FALSE;
	}
	zval_ptr_dtor(&status);

	PHALCON_MM_CALL_METHOD(&statement, getThis(), "prepare", sql_statement);
	PHALCON_MM_ADD_ENTRY(&statement);
	if (Z_TYPE(statement) == IS_OBJECT) {
		PHALCON_MM_CALL_METHOD(&new_statement, getThis(), "executeprepared", &statement, bind_params, bind_types);
		PHALCON_MM_ADD_ENTRY(&new_statement);
		PHALCON_MM_CALL_METHOD(&affected_rows, &new_statement, "rowcount");
	} else {
		ZVAL_LONG(&affected_rows, 0);
	}

	/**
	 * Execute the afterQuery event if a EventsManager is available
	 */
	if (Z_TYPE(affected_rows) == IS_LONG) {
		phalcon_update_property(getThis(), SL("_affectedRows"), &affected_rows);
	}

	PHALCON_MM_ZVAL_STRING(&event_name, "db:afterExecute");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name, bind_params);

	RETURN_MM_TRUE;
}

/**
 * Returns the number of affected rows by the lastest INSERT/UPDATE/DELETE executed in the database system
 *
 *<code>
 *	$connection->execute("DELETE FROM robots");
 *	echo $connection->affectedRows(), ' were deleted';
 *</code>
 *
 * @return int
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, affectedRows){


	RETURN_MEMBER(getThis(), "_affectedRows");
}

/**
 * Closes the active connection returning success. Phalcon automatically closes and destroys
 * active connections when the request ends
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, close){

	zval pdo = {};

	phalcon_read_property(&pdo, getThis(), SL("_pdo"), PH_NOISY|PH_READONLY);
	if (likely(Z_TYPE(pdo) == IS_OBJECT)) {
		phalcon_update_property(getThis(), SL("_pdo"), &PHALCON_GLOBAL(z_null));
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Escapes a column/table/schema name
 *
 *<code>
 *	$escapedTable = $connection->escapeIdentifier('robots');
 *	$escapedTable = $connection->escapeIdentifier(array('store', 'robots'));
 *</code>
 *
 * @param string $identifier
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, escapeIdentifier){

	zval *identifier, domain = {}, name = {};

	phalcon_fetch_params(0, 1, 0, &identifier);

	if (Z_TYPE_P(identifier) == IS_ARRAY) {
		phalcon_array_fetch_long(&domain, identifier, 0, PH_NOISY|PH_READONLY);
		phalcon_array_fetch_long(&name, identifier, 1, PH_NOISY|PH_READONLY);
		PHALCON_CONCAT_SVSVS(return_value, "\"", &domain, "\".\"", &name, "\"");
		return;
	}

	PHALCON_CONCAT_SVS(return_value, "\"", identifier, "\"");
}

/**
 * Escapes a value to avoid SQL injections according to the active charset in the connection
 *
 *<code>
 *	$escapedValue = $connection->escapeValue('some dangerous value');
 *</code>
 *
 * @param mixed $value
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, escapeValue){

	zval *value;

	phalcon_fetch_params(0, 1, 0, &value);

	switch(Z_TYPE_P(value)) {
		case IS_TRUE:
			ZVAL_STRING(return_value, "TRUE");
			break;
		case IS_FALSE:
			ZVAL_STRING(return_value, "FALSE");
			break;
		case IS_LONG:
		case IS_DOUBLE:
			ZVAL_COPY(return_value, value);
			break;
		case IS_NULL:
			ZVAL_STRING(return_value, "NULL");
			break;
		case IS_ARRAY:
			PHALCON_CALL_METHOD(return_value, getThis(), "escapearray", value);
			break;
		default:
			PHALCON_CALL_METHOD(return_value, getThis(), "escapestring", value);
			break;
	}
}

/**
 * Escapes a value to avoid SQL injections according to the active charset in the connection
 *
 *<code>
 *	$escapedStr = $connection->escapeString('some dangerous value');
 *</code>
 *
 * @param string $str
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, escapeString){

	zval *str, pdo = {};

	phalcon_fetch_params(0, 1, 0, &str);

	phalcon_read_property(&pdo, getThis(), SL("_pdo"), PH_NOISY|PH_READONLY);
	PHALCON_RETURN_CALL_METHOD(&pdo, "quote", str);
}

/**
 * Converts bound parameters such as :name: or ?1 into PDO bind params ?
 *
 *<code>
 * print_r($connection->convertBoundParams('SELECT * FROM robots WHERE name = :name:', array('Bender')));
 *</code>
 *
 * @param string $sql
 * @param array $params
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, convertBoundParams){

	zval *sql, *params, query_params = {}, placeholders = {}, matches = {}, bind_pattern = {}, status = {}, *place_match = NULL, question = {}, bound_sql = {};

	phalcon_fetch_params(1, 2, 0, &sql, &params);

	array_init(&query_params);
	PHALCON_MM_ADD_ENTRY(&query_params);
	array_init(&placeholders);
	PHALCON_MM_ADD_ENTRY(&placeholders);

	PHALCON_MM_ZVAL_STRING(&bind_pattern, "/\\?([0-9]+)|:([a-zA-Z0-9_]+):/");

	RETURN_MM_ON_FAILURE(phalcon_preg_match(&status, &bind_pattern, sql, &matches, 2, 1));

	PHALCON_MM_ADD_ENTRY(&matches);
	PHALCON_MM_ADD_ENTRY(&status);

	if (zend_is_true(&status)) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(matches), place_match) {
			zval numeric_place = {}, value = {}, str_place = {};

			phalcon_array_fetch_long(&numeric_place, place_match, 1, PH_NOISY|PH_READONLY);
			if (!phalcon_array_isset_fetch(&value, params, &numeric_place, PH_READONLY)) {
				if (phalcon_array_isset_fetch_long(&str_place, place_match, 2, PH_READONLY)) {
					if (!phalcon_array_isset_fetch(&value, params, &str_place, 0)) {
						PHALCON_MM_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Matched parameter wasn't found in parameters list");
						return;
					}
				} else {
					PHALCON_MM_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Matched parameter wasn't found in parameters list");
					return;
				}
			}
			phalcon_array_append(&placeholders, &value, PH_COPY);
		} ZEND_HASH_FOREACH_END();

		PHALCON_MM_ZVAL_STRING(&question, "?");

		PHALCON_MM_CALL_FUNCTION(&bound_sql, "preg_replace", &bind_pattern, &question, sql);
		PHALCON_MM_ADD_ENTRY(&bound_sql);
	} else {
		ZVAL_COPY_VALUE(&bound_sql, sql);
	}

	/**
	 * Returns an array with the processed SQL and parameters
	 */
	array_init_size(return_value, 2);
	phalcon_array_update_str(return_value, SL("sql"), &bound_sql, PH_COPY);
	phalcon_array_update_str(return_value, SL("params"), &placeholders, PH_COPY);

	RETURN_MM();
}

/**
 * Returns the insert id for the auto_increment/serial column inserted in the lastest executed SQL statement
 *
 *<code>
 * //Inserting a new robot
 * $success = $connection->insert(
 *     "robots",
 *     array("Astro Boy", 1952),
 *     array("name", "year")
 * );
 *
 * //Getting the generated id
 * $id = $connection->lastInsertId();
 *</code>
 *
 * @param string $sequenceName
 * @return int
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, lastInsertId){

	zval *sequence_name = NULL, pdo;

	phalcon_fetch_params(0, 0, 1, &sequence_name);

	if (!sequence_name) {
		sequence_name = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&pdo, getThis(), SL("_pdo"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(pdo) != IS_OBJECT) {
		RETURN_FALSE;
	}

	PHALCON_RETURN_CALL_METHOD(&pdo, "lastinsertid", sequence_name);
}

/**
 * Starts a transaction in the connection
 *
 * @param boolean $nesting
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, begin){

	zval *nesting = NULL, pdo = {}, transaction_level = {}, event_name = {}, ntw_savepoint = {}, savepoint_name = {};
	zval debug_message = {};

	phalcon_fetch_params(0, 0, 1, &nesting);

	if (!nesting) {
		nesting = &PHALCON_GLOBAL(z_true);
	}

	phalcon_read_property(&pdo, getThis(), SL("_pdo"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(pdo) != IS_OBJECT) {
		RETURN_FALSE;
	}

	/**
	 * Increase the transaction nesting level
	 */
	phalcon_property_incr(getThis(), SL("_transactionLevel"));

	/**
	 * Check the transaction nesting level
	 */
	phalcon_read_property(&transaction_level, getThis(), SL("_transactionLevel"), PH_NOISY|PH_READONLY);

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_CONCAT_SV(&debug_message, "DB BEGIN TRANSACTION LEVEL: ", &transaction_level);
		PHALCON_DEBUG_LOG(&debug_message);
		zval_ptr_dtor(&debug_message);
	}

	if (PHALCON_IS_LONG(&transaction_level, 1)) {
		ZVAL_STRING(&event_name, "db:beforeBeginTransaction");
		PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
		zval_ptr_dtor(&event_name);

		PHALCON_CALL_METHOD(return_value, &pdo, "begintransaction");

		ZVAL_STRING(&event_name, "db:afterBeginTransaction");
		PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
		zval_ptr_dtor(&event_name);
		return;
	}

	if (zend_is_true(&transaction_level)) {
		if (zend_is_true(nesting)) {
			PHALCON_CALL_METHOD(&ntw_savepoint, getThis(), "isnestedtransactionswithsavepoints");
			if (zend_is_true(&ntw_savepoint)) {
				PHALCON_CALL_METHOD(&savepoint_name, getThis(), "getnestedtransactionsavepointname");

				/**
				 * Notify the events manager about the created savepoint
				 */
				ZVAL_STRING(&event_name, "db:beforeCreateSavepoint");
				PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name, &savepoint_name);
				zval_ptr_dtor(&event_name);

				PHALCON_CALL_METHOD(return_value, getThis(), "createsavepoint", &savepoint_name);

				ZVAL_STRING(&event_name, "db:afterCreateSavepoint");
				PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name, &savepoint_name);
				zval_ptr_dtor(&event_name);
				zval_ptr_dtor(&savepoint_name);
				return;
			}
		}
	}

	RETURN_FALSE;
}

/**
 * Rollbacks the active transaction in the connection
 *
 * @param boolean $nesting
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, rollback){

	zval *nesting = NULL, pdo = {}, transaction_level = {}, event_name = {}, ntw_savepoint = {}, savepoint_name = {};
	zval debug_message = {};

	phalcon_fetch_params(0, 0, 1, &nesting);

	if (!nesting) {
		nesting = &PHALCON_GLOBAL(z_true);
	}

	phalcon_read_property(&pdo, getThis(), SL("_pdo"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(pdo) != IS_OBJECT) {
		RETURN_FALSE;
	}

	/**
	 * Check the transaction nesting level
	 */
	phalcon_read_property(&transaction_level, getThis(), SL("_transactionLevel"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&transaction_level)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "There is no active transaction");
		return;
	}

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_CONCAT_SV(&debug_message, "DB ROLLBACK TRANSACTION LEVEL: ", &transaction_level);
		PHALCON_DEBUG_LOG(&debug_message);
		zval_ptr_dtor(&debug_message);
	}

	if (PHALCON_IS_LONG(&transaction_level, 1)) {
		/**
		 * Notify the events manager about the rollbacked transaction
		 */
		ZVAL_STRING(&event_name, "db:beforeRollbackTransaction");
		PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
		zval_ptr_dtor(&event_name);

		/**
		 * Reduce the transaction nesting level
		 */
		phalcon_property_decr(getThis(), SL("_transactionLevel"));
		PHALCON_RETURN_CALL_METHOD(&pdo, "rollback");

		ZVAL_STRING(&event_name, "db:afterRollbackTransaction");
		PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
		zval_ptr_dtor(&event_name);
		return;
	}

	if (zend_is_true(&transaction_level)) {
		if (zend_is_true(nesting)) {
			PHALCON_CALL_METHOD(&ntw_savepoint, getThis(), "isnestedtransactionswithsavepoints");
			if (zend_is_true(&ntw_savepoint)) {
				PHALCON_CALL_METHOD(&savepoint_name, getThis(), "getnestedtransactionsavepointname");

				/**
				 * Notify the events manager about the rollbacked savepoint
				 */
				ZVAL_STRING(&event_name, "db:beforeRollbackSavepoint");
				PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name, &savepoint_name);
				zval_ptr_dtor(&event_name);

				/**
				 * Reduce the transaction nesting level
				 */
				phalcon_property_decr(getThis(), SL("_transactionLevel"));
				PHALCON_CALL_METHOD(return_value, getThis(), "rollbacksavepoint", &savepoint_name);

				ZVAL_STRING(&event_name, "db:afterRollbackSavepoint");
				PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name, &savepoint_name);
				zval_ptr_dtor(&event_name);
				zval_ptr_dtor(&savepoint_name);
				return;
			}
		}
	}

	/**
	 * Reduce the transaction nesting level
	 */
	if (PHALCON_GT_LONG(&transaction_level, 0)) {
		phalcon_property_decr(getThis(), SL("_transactionLevel"));
	}

	RETURN_FALSE;
}

/**
 * Commits the active transaction in the connection
 *
 * @param boolean $nesting
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, commit){

	zval *nesting = NULL, pdo = {}, transaction_level = {}, event_name = {}, ntw_savepoint = {}, savepoint_name = {};
	zval debug_message = {};

	phalcon_fetch_params(0, 0, 1, &nesting);

	if (!nesting) {
		nesting = &PHALCON_GLOBAL(z_true);
	}

	phalcon_read_property(&pdo, getThis(), SL("_pdo"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(pdo) != IS_OBJECT) {
		RETURN_FALSE;
	}

	/**
	 * Check the transaction nesting level
	 */
	phalcon_read_property(&transaction_level, getThis(), SL("_transactionLevel"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&transaction_level)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "There is no active transaction");
		return;
	}

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_CONCAT_SV(&debug_message, "DB COMMIT TRANSACTION LEVEL: ", &transaction_level);
		PHALCON_DEBUG_LOG(&debug_message);
		zval_ptr_dtor(&debug_message);
	}

	if (PHALCON_IS_LONG(&transaction_level, 1)) {
		/**
		 * Notify the events manager about the commited transaction
		 */
		ZVAL_STRING(&event_name, "db:beforeCommitTransaction");
		PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
		zval_ptr_dtor(&event_name);

		/**
		 * Reduce the transaction nesting level
		 */
		phalcon_property_decr(getThis(), SL("_transactionLevel"));
		PHALCON_RETURN_CALL_METHOD(&pdo, "commit");

		ZVAL_STRING(&event_name, "db:afterCommitTransaction");
		PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
		zval_ptr_dtor(&event_name);

		return;
	}

	if (zend_is_true(&transaction_level)) {
		if (zend_is_true(nesting)) {

			/**
			 * Check if the current database system supports nested transactions
			 */
			PHALCON_CALL_METHOD(&ntw_savepoint, getThis(), "isnestedtransactionswithsavepoints");
			if (zend_is_true(&ntw_savepoint)) {
				PHALCON_CALL_METHOD(&savepoint_name, getThis(), "getnestedtransactionsavepointname");

				/**
				 * Notify the events manager about the commited savepoint
				 */
				ZVAL_STRING(&event_name, "db:beforeReleaseSavepoint");
				PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name, &savepoint_name);
				zval_ptr_dtor(&event_name);

				/**
				 * Reduce the transaction nesting level
				 */
				phalcon_property_decr(getThis(), SL("_transactionLevel"));
				PHALCON_CALL_METHOD(return_value, getThis(), "releasesavepoint", &savepoint_name);

				ZVAL_STRING(&event_name, "db:afterReleaseSavepoint");
				PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name, &savepoint_name);
				zval_ptr_dtor(&event_name);
				zval_ptr_dtor(&savepoint_name);
				return;
			}
		}
	}

	/**
	 * Reduce the transaction nesting level
	 */
	if (PHALCON_GT_LONG(&transaction_level, 0)) {
		phalcon_property_decr(getThis(), SL("_transactionLevel"));
	}

	RETURN_FALSE;
}

/**
 * Returns the current transaction nesting level
 *
 * @return int
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getTransactionLevel){


	RETURN_MEMBER(getThis(), "_transactionLevel");
}

/**
 * Checks whether the connection is under a transaction
 *
 *<code>
 *	$connection->begin();
 *	var_dump($connection->isUnderTransaction()); //true
 *</code>
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, isUnderTransaction){

	zval pdo = {};

	phalcon_read_property(&pdo, getThis(), SL("_pdo"), PH_NOISY|PH_READONLY);
	if (likely(Z_TYPE(pdo) == IS_OBJECT)) {
		PHALCON_RETURN_CALL_METHOD(&pdo, "intransaction");
		return;
	}

	RETURN_FALSE;
}

/**
 * Return internal PDO handler
 *
 * @return \PDO
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getInternalHandler){

	RETURN_MEMBER(getThis(), "_pdo");
}

/**
 * Return the error info, if any
 *
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getErrorInfo){

	zval pdo = {};

	phalcon_read_property(&pdo, getThis(), SL("_pdo"), PH_NOISY|PH_READONLY);
	PHALCON_RETURN_CALL_METHOD(&pdo, "errorinfo");
}
