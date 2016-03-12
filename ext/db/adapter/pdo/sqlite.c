
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
  |          Rack Lin <racklin@gmail.com>                                  |
  +------------------------------------------------------------------------+
*/

#include "db/adapter/pdo/sqlite.h"
#include "db/adapter/pdo.h"
#include "db/adapterinterface.h"
#include "db/column.h"
#include "db/exception.h"
#include "db/index.h"
#include "db/reference.h"

#include <ext/pdo/php_pdo_driver.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/operators.h"
#include "kernel/concat.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/hash.h"

/**
 * Phalcon\Db\Adapter\Pdo\Sqlite
 *
 * Specific functions for the Sqlite database system
 * <code>
 *
 * $config = array(
 *  "dbname" => "/tmp/test.sqlite"
 * );
 *
 * $connection = new Phalcon\Db\Adapter\Pdo\Sqlite($config);
 *
 * </code>
 */
zend_class_entry *phalcon_db_adapter_pdo_sqlite_ce;

PHP_METHOD(Phalcon_Db_Adapter_Pdo_Sqlite, connect);
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Sqlite, describeColumns);
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Sqlite, describeIndexes);
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Sqlite, describeReferences);
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Sqlite, useExplicitIdValue);

static const zend_function_entry phalcon_db_adapter_pdo_sqlite_method_entry[] = {
	PHP_ME(Phalcon_Db_Adapter_Pdo_Sqlite, connect, arginfo_phalcon_db_adapterinterface_connect, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Sqlite, describeColumns, arginfo_phalcon_db_adapterinterface_describecolumns, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Sqlite, describeIndexes, arginfo_phalcon_db_adapterinterface_describeindexes, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Sqlite, describeReferences, arginfo_phalcon_db_adapterinterface_describereferences, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Sqlite, useExplicitIdValue, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Db\Adapter\Pdo\Sqlite initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Adapter_Pdo_Sqlite){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Db\\Adapter\\Pdo, Sqlite, db_adapter_pdo_sqlite, phalcon_db_adapter_pdo_ce, phalcon_db_adapter_pdo_sqlite_method_entry, 0);

	zend_declare_property_string(phalcon_db_adapter_pdo_sqlite_ce, SL("_type"), "sqlite", ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_db_adapter_pdo_sqlite_ce, SL("_dialectType"), "sqlite", ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_db_adapter_pdo_sqlite_ce, 1, phalcon_db_adapterinterface_ce);

	return SUCCESS;
}

/**
 * This method is automatically called in Phalcon\Db\Adapter\Pdo constructor.
 * Call it when you need to restore a database connection.
 *
 * @param array $descriptor
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Sqlite, connect){

	zval *descriptor = NULL, dbname = {};

	phalcon_fetch_params(0, 0, 1, &descriptor);

	if (!descriptor || !zend_is_true(descriptor)) {
		descriptor = phalcon_read_property(getThis(), SL("_descriptor"), PH_NOISY);
	}

	if (!phalcon_array_isset_str(descriptor, SL("dbname"))) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_db_exception_ce, "dbname must be specified");
		return;
	} else {
		phalcon_array_fetch_str(&dbname, descriptor, SL("dbname"), PH_NOISY);
		phalcon_array_update_str(descriptor, SL("dsn"), &dbname, PH_COPY);
	}

	PHALCON_CALL_PARENTW(NULL, phalcon_db_adapter_pdo_sqlite_ce, getThis(), "connect", descriptor);
}

/**
 * Returns an array of Phalcon\Db\Column objects describing a table
 *
 * <code>
 * print_r($connection->describeColumns("posts")); ?>
 * </code>
 *
 * @param string $table
 * @param string $schema
 * @return Phalcon\Db\Column[]
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Sqlite, describeColumns){

	zval *table, *schema = NULL, columns, *dialect, size_pattern = {}, sql = {}, fetch_num = {}, describe = {}, old_column = {}, *field;

	phalcon_fetch_params(0, 1, 1, &table, &schema);

	if (!schema) {
		schema = &PHALCON_GLOBAL(z_null);
	}

	array_init(&columns);

	dialect = phalcon_read_property(getThis(), SL("_dialect"), PH_NOISY);

	ZVAL_STRING(&size_pattern, "#\\(([0-9]++)(?:,\\s*([0-9]++))?\\)#");

	PHALCON_CALL_METHODW(&sql, dialect, "describecolumns", table, schema);

	/** 
	 * We're using FETCH_NUM to fetch the columns
	 */
	ZVAL_LONG(&fetch_num, PDO_FETCH_NUM);

	PHALCON_CALL_METHODW(&describe, getThis(), "fetchall", &sql, &fetch_num);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(describe), field) {
		zval definition = {}, column_type = {}, matches = {}, pos = {}, match_one = {}, match_two = {}, attribute = {}, column_name = {}, column = {};

		array_init_size(&definition, 1);
		add_assoc_long_ex(&definition, SL("bindType"), 2);

		phalcon_array_fetch_long(&column_type, field, 2, PH_NOISY);

		if (phalcon_memnstr_str(&column_type, SL("("))) {
			RETURN_ON_FAILURE(phalcon_preg_match(&pos, &size_pattern, &column_type, &matches));
			if (zend_is_true(&pos)) {
				if (phalcon_array_isset_fetch_long(&match_one, &matches, 1)) {
					convert_to_long(&match_one);
					phalcon_array_update_str(&definition, SL("size"), &match_one, PH_COPY);
					phalcon_array_update_str(&definition, SL("bytes"), &match_one, PH_COPY);
				}
				if (phalcon_array_isset_fetch_long(&match_two, &matches, 2)) {
					convert_to_long(&match_two);
					phalcon_array_update_str(&definition, SL("scale"), &match_two, PH_COPY);
				}
			}
		}

		/** 
		 * Check the column type to get the correct Phalcon type
		 */
		while (1) {
			/**
			 * Tinyint(1) is boolean
			 */
			if (phalcon_memnstr_str(&column_type, SL("tinyint(1)"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_BOOLEAN, 0);
				phalcon_array_update_str_long(&definition, SL("bindType"), 5, 0);
				ZVAL_STRING(&column_type, "boolean"); // Change column type to skip size check.
				break;
			}

			/**
			 * Smallint
			 */
			if (phalcon_memnstr_str(&column_type, SL("smallint"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_INTEGER, 0);
				phalcon_array_update_str(&definition, SL("isNumeric"), &PHALCON_GLOBAL(z_true), PH_COPY);
				phalcon_array_update_str_long(&definition, SL("bindType"), 1, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 16, 0);

				phalcon_array_fetch_long(&attribute, field, 5, PH_NOISY);

				/**
				 * Check if the column is auto increment
				 */
				if (zend_is_true(&attribute)) {
					phalcon_array_update_str_bool(&definition, SL("autoIncrement"), 1, 0);
				}
				break;
			}

			/**
			 * Mediumint
			 */
			if (phalcon_memnstr_str(&column_type, SL("mediumint"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_INTEGER, 0);
				phalcon_array_update_str(&definition, SL("isNumeric"), &PHALCON_GLOBAL(z_true), PH_COPY);
				phalcon_array_update_str_long(&definition, SL("bindType"), 1, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 24, 0);

				phalcon_array_fetch_long(&attribute, field, 5, PH_NOISY);

				/**
				 * Check if the column is auto increment
				 */
				if (zend_is_true(&attribute)) {
					phalcon_array_update_str_bool(&definition, SL("autoIncrement"), 1, 0);
				}
				break;
			}

			/**
			 * BIGINT
			 */
			if (phalcon_memnstr_str(&column_type, SL("bigint"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_INTEGER, 0);
				phalcon_array_update_str(&definition, SL("isNumeric"), &PHALCON_GLOBAL(z_true), PH_COPY);
				phalcon_array_update_str_long(&definition, SL("bindType"), 1, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 64, 0);

				phalcon_array_fetch_long(&attribute, field, 5, PH_NOISY);

				/**
				 * Check if the column is auto increment
				 */
				if (zend_is_true(&attribute)) {
					phalcon_array_update_str_bool(&definition, SL("autoIncrement"), 1, 0);
				}
				break;
			}

			/**
			 * Integers/Int are int
			 */
			phalcon_fast_stripos_str(&pos, &column_type, SL("int"));
			if (PHALCON_IS_NOT_FALSE(&pos)) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_INTEGER, 0);
				phalcon_array_update_str_bool(&definition, SL("isNumeric"), 1, 0);
				phalcon_array_update_str_long(&definition, SL("bindType"), 1, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 32, 0);

				phalcon_array_fetch_long(&attribute, field, 5, PH_NOISY);

				/**
				 * Check if the column is auto increment
				 */
				if (zend_is_true(&attribute)) {
					phalcon_array_update_str_bool(&definition, SL("autoIncrement"), 1, 0);
				}
				break;
			}

			/**
			 * Varchar are varchars
			 */
			if (phalcon_memnstr_str(&column_type, SL("varchar"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_VARCHAR, 0);
				break;
			}

			/**
			 * Date/Datetime are varchars
			 */
			if (phalcon_memnstr_str(&column_type, SL("date"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DATE, 0);
				break;
			}

			/**
			 * Timestamp as date
			 */
			if (phalcon_memnstr_str(&column_type, SL("timestamp"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DATE, 0);
				break;
			}

			/**
			 * Decimals are floats
			 */
			if (phalcon_memnstr_str(&column_type, SL("decimal"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DECIMAL, 0);
				phalcon_array_update_str_bool(&definition, SL("isNumeric"), 1, 0);
				phalcon_array_update_str_long(&definition, SL("bindType"), 32, 0);
				if (phalcon_is_numeric(&match_one)) {
					phalcon_array_update_str_long(&definition, SL("bytes"), Z_LVAL(match_one) * 8, 0);
				} else {
					phalcon_array_update_str_long(&definition, SL("size"), 5, 0);
					phalcon_array_update_str_long(&definition, SL("bytes"), 40, 0);
				}
				if (phalcon_is_numeric(&match_two)) {
					phalcon_array_update_str(&definition, SL("scale"), &match_two, PH_COPY);
				} else {
					phalcon_array_update_str_long(&definition, SL("scale"), 2, 0);
				}
				break;
			}

			/**
			 * Chars are chars
			 */
			if (phalcon_memnstr_str(&column_type, SL("char"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_CHAR, 0);
				break;
			}

			/**
			 * Special type for datetime
			 */
			if (phalcon_memnstr_str(&column_type, SL("datetime"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DATETIME, 0);
				break;
			}

			/**
			 * Text are varchars
			 */
			if (phalcon_memnstr_str(&column_type, SL("text"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_TEXT, 0);
				break;
			}

			/**
			 * Float/Smallfloats/Decimals are float
			 */
			if (phalcon_memnstr_str(&column_type, SL("float"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_FLOAT, 0);
				phalcon_array_update_str_bool(&definition, SL("isNumeric"), 1, 0);
				phalcon_array_update_str_long(&definition, SL("bindType"), 32, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 32, 0);
				break;
			}

			/**
			 * Double are floats
			 */
			if (phalcon_memnstr_str(&column_type, SL("double"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DOUBLE, 0);
				phalcon_array_update_str(&definition, SL("isNumeric"), &PHALCON_GLOBAL(z_true), PH_COPY);
				phalcon_array_update_str_long(&definition, SL("bindType"), 32, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 64, 0);
				break;
			}

			/**
			 * Enum are treated as char
			 */
			if (phalcon_memnstr_str(&column_type, SL("enum"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_CHAR, 0);
				break;
			}

			/**
			 * By default is string
			 */
			phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_VARCHAR, 0);
			break;
		}

		if (phalcon_memnstr_str(&column_type, SL("unsigned"))) {
			phalcon_array_update_str_bool(&definition, SL("unsigned"), 1, 0);
		}

		if (!zend_is_true(&old_column)) {
			phalcon_array_update_str_bool(&definition, SL("first"), 1, 0);
		} else {
			phalcon_array_update_str(&definition, SL("after"), &old_column, PH_COPY);
		}

		/** 
		 * Check if the field is primary key
		 */
		phalcon_array_fetch_long(&attribute, field, 5, PH_NOISY);
		if (zend_is_true(&attribute)) {
			phalcon_array_update_str_bool(&definition, SL("primary"), 1, 0);
		}

		/** 
		 * Check if the column allows null values
		 */
		phalcon_array_fetch_long(&attribute, field, 3, PH_NOISY);
		if (zend_is_true(&attribute)) {
			phalcon_array_update_str_bool(&definition, SL("notNull"), 1, 0);
		}

		phalcon_array_fetch_long(&column_name, field, 1, PH_NOISY);

		/** 
		 * If the column set the default values, get it
		 */
		phalcon_array_fetch_long(&attribute, field, 4, PH_NOISY);
		if (!PHALCON_IS_EMPTY(&attribute)) {
			phalcon_array_update_str(&definition, SL("default"), &attribute, PH_COPY);
		}

		/** 
		 * Every column is stored as a Phalcon\Db\Column
		 */
		object_init_ex(&column, phalcon_db_column_ce);
		PHALCON_CALL_METHODW(NULL, &column, "__construct", &column_name, &definition);

		phalcon_array_append(&columns, &column, PH_COPY);
		PHALCON_CPY_WRT_CTOR(&old_column, &column_name);

	} ZEND_HASH_FOREACH_END();

	RETURN_CTORW(&columns);
}

/**
 * Lists table indexes
 *
 * @param string $table
 * @param string $schema
 * @return Phalcon\Db\Index[]
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Sqlite, describeIndexes){

	zval *table, *schema = NULL, *dialect, fetch_num = {}, sql = {}, describe = {}, indexes = {}, *index, index_objects = {}, *index_columns;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 1, 1, &table, &schema);

	if (!schema) {
		schema = &PHALCON_GLOBAL(z_null);
	}

	dialect = phalcon_read_property(getThis(), SL("_dialect"), PH_NOISY);

	/** 
	 * We're using FETCH_NUM to fetch the columns
	 */
	ZVAL_LONG(&fetch_num, PDO_FETCH_NUM);

	PHALCON_CALL_METHODW(&sql, dialect, "describeindexes", table, schema);
	PHALCON_CALL_METHODW(&describe, getThis(), "fetchall", &sql, &fetch_num);

	/** 
	 * Cryptic Guide: 0: position, 1: name
	 */
	array_init(&indexes);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(describe), index) {
		zval key_name = {}, sql_index_describe = {}, describe_index = {}, *index_column;

		phalcon_array_fetch_long(&key_name, index, 1, PH_NOISY);

		PHALCON_CALL_METHODW(&sql_index_describe, dialect, "describeindex", &key_name);
		PHALCON_CALL_METHODW(&describe_index, getThis(), "fetchall", &sql_index_describe, &fetch_num);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(describe_index), index_column) {
			zval column_name = {};
			phalcon_array_fetch_long(&column_name, index_column, 2, PH_NOISY);
			phalcon_array_append_multi_2(&indexes, &key_name, &column_name, PH_COPY);
		} ZEND_HASH_FOREACH_END();
	} ZEND_HASH_FOREACH_END();

	array_init(&index_objects);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(indexes), idx, str_key, index_columns) {
		zval name = {}, index = {};
		if (str_key) {
			ZVAL_STR(&name, str_key);
		} else {
			ZVAL_LONG(&name, idx);
		}

		object_init_ex(&index, phalcon_db_index_ce);
		PHALCON_CALL_METHODW(NULL, &index, "__construct", &name, index_columns);

		phalcon_array_update_zval(&index_objects, &name, &index, PH_COPY);
	} ZEND_HASH_FOREACH_END();

	RETURN_CTORW(&index_objects);
}

/**
 * Lists table references
 *
 * @param string $table
 * @param string $schema
 * @return Phalcon\Db\Reference[]
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Sqlite, describeReferences){

	zval *table, *schema = NULL, *dialect, sql, fetch_num, describe, reference_objects, *reference_describe;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 1, 1, &table, &schema);

	if (!schema) {
		schema = &PHALCON_GLOBAL(z_null);
	}

	dialect = phalcon_read_property(getThis(), SL("_dialect"), PH_NOISY);

	/** 
	 * Get the SQL to describe the references
	 */
	PHALCON_CALL_METHODW(&sql, dialect, "describereferences", table, schema);

	/** 
	 * We're using FETCH_NUM to fetch the columns
	 */
	ZVAL_LONG(&fetch_num, PDO_FETCH_NUM);

	/** 
	 * Execute the SQL describing the references
	 */
	PHALCON_CALL_METHODW(&describe, getThis(), "fetchall", &sql, &fetch_num);

	/** 
	 * Cryptic Guide: 2: table, 3: from, 4: to
	 */
	array_init(&reference_objects);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(describe), idx, str_key, reference_describe) {
		zval number = {}, constraint_name = {}, referenced_table = {}, from = {}, to = {}, columns = {}, referenced_columns = {}, reference_array = {}, reference = {};
		if (str_key) {
			ZVAL_STR(&number, str_key);
		} else {
			ZVAL_LONG(&number, idx);
		}

		PHALCON_CONCAT_SV(&constraint_name, "foreign_key_", &number);
		phalcon_array_fetch_long(&referenced_table, reference_describe, 2, PH_NOISY);
		phalcon_array_fetch_long(&from, reference_describe, 3, PH_NOISY);
		phalcon_array_fetch_long(&to, reference_describe, 4, PH_NOISY);

		array_init_size(&columns, 1);
		phalcon_array_append(&columns, &from, PH_COPY);

		array_init_size(&referenced_columns, 1);
		phalcon_array_append(&referenced_columns, &to, PH_COPY);

		array_init_size(&reference_array, 4);
		add_assoc_null_ex(&reference_array, SL("referencedSchema"));
		phalcon_array_update_str(&reference_array, SL("referencedTable"), &referenced_table, PH_COPY);
		phalcon_array_update_str(&reference_array, SL("columns"), &columns, PH_COPY);
		phalcon_array_update_str(&reference_array, SL("referencedColumns"), &referenced_columns, PH_COPY);

		/** 
		 * Every route is abstracted as a Phalcon\Db\Reference instance
		 */
		object_init_ex(&reference, phalcon_db_reference_ce);
		PHALCON_CALL_METHODW(NULL, &reference, "__construct", &constraint_name, &reference_array);
		phalcon_array_update_zval(&reference_objects, &constraint_name, &reference, PH_COPY);
	} ZEND_HASH_FOREACH_END();

	RETURN_CTORW(&reference_objects);
}

/**
 * Check whether the database system requires an explicit value for identity columns
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Sqlite, useExplicitIdValue){


	RETURN_TRUE;
}

