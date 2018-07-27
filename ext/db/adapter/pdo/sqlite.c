
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
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Sqlite, escapeBytea);
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Sqlite, unescapeBytea);
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Sqlite, escapeArray);
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Sqlite, unescapeArray);

static const zend_function_entry phalcon_db_adapter_pdo_sqlite_method_entry[] = {
	PHP_ME(Phalcon_Db_Adapter_Pdo_Sqlite, connect, arginfo_phalcon_db_adapterinterface_connect, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Sqlite, describeColumns, arginfo_phalcon_db_adapterinterface_describecolumns, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Sqlite, describeIndexes, arginfo_phalcon_db_adapterinterface_describeindexes, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Sqlite, describeReferences, arginfo_phalcon_db_adapterinterface_describereferences, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Sqlite, useExplicitIdValue, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Sqlite, escapeBytea, arginfo_phalcon_db_adapterinterface_escapebytea, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Sqlite, unescapeBytea, arginfo_phalcon_db_adapterinterface_unescapebytea, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Sqlite, escapeArray, arginfo_phalcon_db_adapterinterface_escapearray, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Sqlite, unescapeArray, arginfo_phalcon_db_adapterinterface_unescapearray, ZEND_ACC_PUBLIC)
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

	zval *desc = NULL, descriptor = {}, dbname = {};

	phalcon_fetch_params(0, 0, 1, &desc);

	if (!desc || !zend_is_true(desc)) {
		phalcon_read_property(&descriptor, getThis(), SL("_descriptor"), PH_CTOR);
	} else {
		ZVAL_DUP(&descriptor, desc);
	}

	if (!phalcon_array_isset_fetch_str(&dbname, &descriptor, SL("dbname"), PH_READONLY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "dbname must be specified");
		return;
	} else {
		phalcon_array_update_str(&descriptor, SL("dsn"), &dbname, PH_COPY);
	}

	PHALCON_CALL_PARENT(NULL, phalcon_db_adapter_pdo_sqlite_ce, getThis(), "connect", &descriptor);
	zval_ptr_dtor(&descriptor);
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

	zval *table, *schema = NULL, dialect = {}, size_pattern = {}, sql = {}, fetch_num = {}, describe = {}, old_column = {}, *field;

	phalcon_fetch_params(1, 1, 1, &table, &schema);

	if (!schema) {
		schema = &PHALCON_GLOBAL(z_null);
	}

	array_init(return_value);

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_MM_ZVAL_STRING(&size_pattern, "#\\(([0-9]++)(?:,\\s*([0-9]++))?\\)#");

	PHALCON_MM_CALL_METHOD(&sql, &dialect, "describecolumns", table, schema);
	PHALCON_MM_ADD_ENTRY(&sql);

	/**
	 * We're using FETCH_NUM to fetch the columns
	 */
	ZVAL_LONG(&fetch_num, PDO_FETCH_NUM);

	PHALCON_MM_CALL_METHOD(&describe, getThis(), "fetchall", &sql, &fetch_num);
	PHALCON_MM_ADD_ENTRY(&describe);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(describe), field) {
		zval definition = {}, column_type = {}, matches = {}, pos = {}, match_one = {}, match_two = {}, attribute = {}, column_name = {}, column = {};

		array_init(&definition);
		PHALCON_MM_ADD_ENTRY(&definition);
		add_assoc_long_ex(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_STR);

		phalcon_array_fetch_long(&column_type, field, 2, PH_NOISY|PH_READONLY);

		if (phalcon_memnstr_str(&column_type, SL("("))) {
			ZVAL_NULL(&matches);
			RETURN_MM_ON_FAILURE(phalcon_preg_match(&pos, &size_pattern, &column_type, &matches, 0, 0));
			PHALCON_MM_ADD_ENTRY(&matches);
			if (zend_is_true(&pos)) {
				if (phalcon_array_isset_fetch_long(&match_one, &matches, 1, PH_READONLY)) {
					phalcon_array_update_str_long(&definition, SL("size"), phalcon_get_intval(&match_one), 0);
					phalcon_array_update_str_long(&definition, SL("bytes"), phalcon_get_intval(&match_one), 0);
				}
				if (phalcon_array_isset_fetch_long(&match_two, &matches, 2, PH_READONLY)) {
					phalcon_array_update_str_long(&definition, SL("scale"), phalcon_get_intval(&match_two), 0);
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
				PHALCON_MM_ZVAL_STRING(&column_type, "boolean"); // Change column type to skip size check.
				break;
			}

			/**
			 * Smallint
			 */
			if (phalcon_memnstr_str(&column_type, SL("smallint"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_INTEGER, 0);
				phalcon_array_update_str(&definition, SL("isNumeric"), &PHALCON_GLOBAL(z_true), 0);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_INT, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 2, 0);

				phalcon_array_fetch_long(&attribute, field, 5, PH_NOISY|PH_READONLY);

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
				phalcon_array_update_str(&definition, SL("isNumeric"), &PHALCON_GLOBAL(z_true), 0);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_INT, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 3, 0);

				phalcon_array_fetch_long(&attribute, field, 5, PH_NOISY|PH_READONLY);

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
				phalcon_array_update_str(&definition, SL("isNumeric"), &PHALCON_GLOBAL(z_true), 0);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_INT, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 8, 0);

				phalcon_array_fetch_long(&attribute, field, 5, PH_NOISY|PH_READONLY);

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
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_INT, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 4, 0);

				phalcon_array_fetch_long(&attribute, field, 5, PH_NOISY|PH_READONLY);

				/**
				 * Check if the column is auto increment
				 */
				if (zend_is_true(&attribute)) {
					phalcon_array_update_str_bool(&definition, SL("autoIncrement"), 1, 0);
				}
				break;
			}

			/**
			 * Float/Smallfloats/Decimals are float
			 */
			if (phalcon_memnstr_str(&column_type, SL("float"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_FLOAT, 0);
				phalcon_array_update_str_bool(&definition, SL("isNumeric"), 1, 0);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_DECIMAL, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 4, 0);
				break;
			}

			/**
			 * Double are floats
			 */
			if (phalcon_memnstr_str(&column_type, SL("double"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DOUBLE, 0);
				phalcon_array_update_str(&definition, SL("isNumeric"), &PHALCON_GLOBAL(z_true), 0);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_DECIMAL, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 8, 0);
				break;
			}

			/**
			 * Decimals are floats
			 */
			if (phalcon_memnstr_str(&column_type, SL("decimal"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DECIMAL, 0);
				phalcon_array_update_str_bool(&definition, SL("isNumeric"), 1, 0);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_DECIMAL, 0);
				if (phalcon_is_numeric(&match_one)) {
					phalcon_array_update_str_long(&definition, SL("bytes"), phalcon_get_intval(&match_one) * 8, 0);
				} else {
					phalcon_array_update_str_long(&definition, SL("size"), 5, 0);
					phalcon_array_update_str_long(&definition, SL("bytes"), 5, 0);
				}
				if (phalcon_is_numeric(&match_two)) {
					phalcon_array_update_str_long(&definition, SL("scale"), phalcon_get_intval(&match_two), 0);
				} else {
					phalcon_array_update_str_long(&definition, SL("scale"), 2, 0);
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
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_TIMESTAMP, 0);
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

		if (Z_TYPE(old_column) <= IS_NULL) {
			phalcon_array_update_str_bool(&definition, SL("first"), 1, 0);
		} else {
			phalcon_array_update_str(&definition, SL("after"), &old_column, PH_COPY);
		}

		/**
		 * Check if the field is primary key
		 */
		phalcon_array_fetch_long(&attribute, field, 5, PH_NOISY|PH_READONLY);
		if (zend_is_true(&attribute)) {
			phalcon_array_update_str_bool(&definition, SL("primary"), 1, 0);
		}

		/**
		 * Check if the column allows null values
		 */
		phalcon_array_fetch_long(&attribute, field, 3, PH_NOISY|PH_READONLY);
		if (zend_is_true(&attribute)) {
			phalcon_array_update_str_bool(&definition, SL("notNull"), 1, 0);
		}

		phalcon_array_fetch_long(&column_name, field, 1, PH_NOISY|PH_READONLY);

		/**
		 * If the column set the default values, get it
		 */
		phalcon_array_fetch_long(&attribute, field, 4, PH_NOISY|PH_READONLY);
		if (!PHALCON_IS_EMPTY(&attribute)) {
			phalcon_array_update_str(&definition, SL("default"), &attribute, PH_COPY);
		}

		/**
		 * Every column is stored as a Phalcon\Db\Column
		 */
		object_init_ex(&column, phalcon_db_column_ce);
		PHALCON_MM_CALL_METHOD(NULL, &column, "__construct", &column_name, &definition);
		phalcon_array_append(return_value, &column, 0);
		ZVAL_COPY_VALUE(&old_column, &column_name);
	} ZEND_HASH_FOREACH_END();
	RETURN_MM();
}

/**
 * Lists table indexes
 *
 * @param string $table
 * @param string $schema
 * @return Phalcon\Db\Index[]
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Sqlite, describeIndexes){

	zval *table, *_schema = NULL, schema = {}, dialect = {}, fetch_num = {}, sql = {}, describe = {}, indexes = {}, *index, index_objects = {}, *index_columns;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(1, 1, 1, &table, &_schema);

	if (!_schema || !zend_is_true(_schema)) {
		phalcon_read_property(&schema, getThis(), SL("_schema"), PH_NOISY|PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&schema, _schema);
	}

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	/**
	 * We're using FETCH_NUM to fetch the columns
	 */
	ZVAL_LONG(&fetch_num, PDO_FETCH_NUM);

	PHALCON_MM_CALL_METHOD(&sql, &dialect, "describeindexes", table, &schema);
	PHALCON_MM_ADD_ENTRY(&sql);
	PHALCON_MM_CALL_METHOD(&describe, getThis(), "fetchall", &sql, &fetch_num);
	PHALCON_MM_ADD_ENTRY(&describe);

	/**
	 * Cryptic Guide: 0: position, 1: name
	 */
	array_init(&indexes);
	PHALCON_MM_ADD_ENTRY(&indexes);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(describe), index) {
		zval key_name = {}, sql_index_describe = {}, describe_index = {}, *index_column;

		phalcon_array_fetch_long(&key_name, index, 1, PH_NOISY|PH_READONLY);

		PHALCON_MM_CALL_METHOD(&sql_index_describe, &dialect, "describeindex", &key_name);
		PHALCON_MM_ADD_ENTRY(&sql_index_describe);
		PHALCON_MM_CALL_METHOD(&describe_index, getThis(), "fetchall", &sql_index_describe, &fetch_num);
		PHALCON_MM_ADD_ENTRY(&describe_index);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(describe_index), index_column) {
			zval column_name = {};
			phalcon_array_fetch_long(&column_name, index_column, 2, PH_NOISY|PH_READONLY);
			phalcon_array_append_multi_2(&indexes, &key_name, &column_name, PH_COPY);
		} ZEND_HASH_FOREACH_END();
	} ZEND_HASH_FOREACH_END();

	array_init(&index_objects);
	PHALCON_MM_ADD_ENTRY(&index_objects);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(indexes), idx, str_key, index_columns) {
		zval name = {}, index = {};
		if (str_key) {
			ZVAL_STR(&name, str_key);
		} else {
			ZVAL_LONG(&name, idx);
		}

		object_init_ex(&index, phalcon_db_index_ce);
		PHALCON_MM_CALL_METHOD(NULL, &index, "__construct", &name, index_columns);

		phalcon_array_update(&index_objects, &name, &index, 0);
	} ZEND_HASH_FOREACH_END();

	RETURN_MM_CTOR(&index_objects);
}

/**
 * Lists table references
 *
 * @param string $table
 * @param string $schema
 * @return Phalcon\Db\Reference[]
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Sqlite, describeReferences){

	zval *table, *schema = NULL, dialect = {}, sql, fetch_num, describe, *reference_describe;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(1, 1, 1, &table, &schema);

	if (!schema) {
		schema = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	/**
	 * Get the SQL to describe the references
	 */
	PHALCON_MM_CALL_METHOD(&sql, &dialect, "describereferences", table, schema);
	PHALCON_MM_ADD_ENTRY(&sql);

	/**
	 * We're using FETCH_NUM to fetch the columns
	 */
	ZVAL_LONG(&fetch_num, PDO_FETCH_NUM);

	/**
	 * Execute the SQL describing the references
	 */
	PHALCON_MM_CALL_METHOD(&describe, getThis(), "fetchall", &sql, &fetch_num);
	PHALCON_MM_ADD_ENTRY(&describe);

	/**
	 * Cryptic Guide: 2: table, 3: from, 4: to
	 */
	array_init(return_value);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(describe), idx, str_key, reference_describe) {
		zval number = {}, constraint_name = {}, referenced_table = {}, from = {}, to = {}, columns = {}, referenced_columns = {}, reference_array = {}, reference = {};
		if (str_key) {
			ZVAL_STR(&number, str_key);
		} else {
			ZVAL_LONG(&number, idx);
		}

		PHALCON_CONCAT_SV(&constraint_name, "foreign_key_", &number);
		PHALCON_MM_ADD_ENTRY(&constraint_name);
		phalcon_array_fetch_long(&referenced_table, reference_describe, 2, PH_NOISY|PH_READONLY);
		phalcon_array_fetch_long(&from, reference_describe, 3, PH_NOISY|PH_READONLY);
		phalcon_array_fetch_long(&to, reference_describe, 4, PH_NOISY|PH_READONLY);

		array_init_size(&columns, 1);
		phalcon_array_append(&columns, &from, PH_COPY);

		array_init_size(&referenced_columns, 1);
		phalcon_array_append(&referenced_columns, &to, PH_COPY);

		array_init_size(&reference_array, 4);
		add_assoc_null_ex(&reference_array, SL("referencedSchema"));
		phalcon_array_update_str(&reference_array, SL("referencedTable"), &referenced_table, PH_COPY);
		phalcon_array_update_str(&reference_array, SL("columns"), &columns, 0);
		phalcon_array_update_str(&reference_array, SL("referencedColumns"), &referenced_columns, 0);
		PHALCON_MM_ADD_ENTRY(&reference_array);

		/**
		 * Every route is abstracted as a Phalcon\Db\Reference instance
		 */
		object_init_ex(&reference, phalcon_db_reference_ce);
		PHALCON_CALL_METHOD(NULL, &reference, "__construct", &constraint_name, &reference_array);
		phalcon_array_update(return_value, &constraint_name, &reference, 0);
	} ZEND_HASH_FOREACH_END();

	RETURN_MM();
}

/**
 * Check whether the database system requires an explicit value for identity columns
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Sqlite, useExplicitIdValue){


	RETURN_TRUE;
}

/**
 * Convert php bytea to database bytea
 *
 * @param string $value
 * @return string
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Sqlite, escapeBytea){

	zval *value;

	phalcon_fetch_params(0, 1, 0, &value);

	RETURN_CTOR(value);
}

/**
 * Convert database bytea to php bytea
 *
 * @param string $value
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Sqlite, unescapeBytea){

	zval *value;

	phalcon_fetch_params(0, 1, 0, &value);

	RETURN_CTOR(value);
}

/**
 * Convert php array to database array
 *
 * @param array $value
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Sqlite, escapeArray){

	zval *value, *type = NULL;

	phalcon_fetch_params(0, 1, 1, &value, &type);

	RETURN_CTOR(value);
}

/**
 * Convert database array to php array
 *
 * @param string $value
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Sqlite, unescapeArray){

	zval *value, *type = NULL;

	phalcon_fetch_params(0, 1, 1, &value, &type);

	RETURN_CTOR(value);
}
