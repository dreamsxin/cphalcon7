
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
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#include "db/adapter/pdo/postgresql.h"
#include "db/adapter/pdo.h"
#include "db/adapterinterface.h"
#include "db/exception.h"
#include "db/column.h"
#include "db/rawvalue.h"
#include "db/reference.h"

#include <Zend/zend_smart_str.h>
#include <ext/pdo/php_pdo_driver.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/string.h"
#include "kernel/operators.h"
#include "kernel/exception.h"

/**
 * Phalcon\Db\Adapter\Pdo\Postgresql
 *
 * Specific functions for the Postgresql database system
 * <code>
 *
 * $config = array(
 *  "host" => "192.168.0.11",
 *  "dbname" => "blog",
 *  "username" => "postgres",
 *  "password" => ""
 * );
 *
 * $connection = new Phalcon\Db\Adapter\Pdo\Postgresql($config);
 *
 * </code>
 */
zend_class_entry *phalcon_db_adapter_pdo_postgresql_ce;

PHP_METHOD(Phalcon_Db_Adapter_Pdo_Postgresql, connect);
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Postgresql, describeColumns);
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Postgresql, useExplicitIdValue);
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Postgresql, getDefaultIdValue);
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Postgresql, supportSequences);
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Postgresql, escapeBytea);
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Postgresql, unescapeBytea);
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Postgresql, escapeArray);
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Postgresql, unescapeArray);

static const zend_function_entry phalcon_db_adapter_pdo_postgresql_method_entry[] = {
	PHP_ME(Phalcon_Db_Adapter_Pdo_Postgresql, connect, arginfo_phalcon_db_adapterinterface_connect, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Postgresql, describeColumns, arginfo_phalcon_db_adapterinterface_describecolumns, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Postgresql, useExplicitIdValue, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Postgresql, getDefaultIdValue, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Postgresql, supportSequences, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Postgresql, escapeBytea, arginfo_phalcon_db_adapterinterface_escapebytea, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Postgresql, unescapeBytea, arginfo_phalcon_db_adapterinterface_unescapebytea, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Postgresql, escapeArray, arginfo_phalcon_db_adapterinterface_escapearray, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Postgresql, unescapeArray, arginfo_phalcon_db_adapterinterface_unescapearray, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Db\Adapter\Pdo\Postgresql initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Adapter_Pdo_Postgresql){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Db\\Adapter\\Pdo, Postgresql, db_adapter_pdo_postgresql, phalcon_db_adapter_pdo_ce, phalcon_db_adapter_pdo_postgresql_method_entry, 0);

	zend_declare_property_string(phalcon_db_adapter_pdo_postgresql_ce, SL("_type"), "pgsql", ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_db_adapter_pdo_postgresql_ce, SL("_dialectType"), "postgresql", ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_db_adapter_pdo_postgresql_ce, 1, phalcon_db_adapterinterface_ce);

	return SUCCESS;
}

/**
 * This method is automatically called in Phalcon\Db\Adapter\Pdo constructor.
 * Call it when you need to restore a database connection.
 *
 * Support set search_path after connectted if schema is specified in config.
 *
 * @param array $descriptor
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Postgresql, connect){

	zval *desc = NULL, descriptor = {}, schema = {}, password = {}, sql = {};

	phalcon_fetch_params(0, 0, 1, &desc);

	if (!desc || !zend_is_true(desc)) {
		phalcon_read_property(&descriptor, getThis(), SL("_descriptor"), PH_CTOR);
	} else {
		ZVAL_DUP(&descriptor, desc);
	}

	if (Z_TYPE(descriptor) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Invalid CONNECT definition");
		return;
	}

	if (phalcon_array_isset_fetch_str(&schema, &descriptor, SL("schema"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_schema"), &schema);
		phalcon_array_unset_str(&descriptor, SL("schema"), 0);
	}

	if (phalcon_array_isset_fetch_str(&password, &descriptor, SL("password"), PH_READONLY)) {
		if (Z_TYPE(password) == IS_STRING && Z_STRLEN(password) == 0) {
			phalcon_array_update_str(&descriptor, SL("password"), &PHALCON_GLOBAL(z_null), 0);
		}
	}

	PHALCON_CALL_PARENT(NULL, phalcon_db_adapter_pdo_postgresql_ce, getThis(), "connect", &descriptor);
	zval_ptr_dtor(&descriptor);

	/**
	 * Execute the search path in the after connect
	 */
	if (Z_TYPE(schema) == IS_STRING) {
		PHALCON_CONCAT_SVS(&sql, "SET search_path TO '", &schema, "'");
		PHALCON_CALL_METHOD(NULL, getThis(), "execute", &sql);
		zval_ptr_dtor(&sql);
	}
}

/**
 * Returns an array of Phalcon\Db\Column objects describing a table
 *
 * <code>print_r($connection->describeColumns("posts")); ?></code>
 *
 * @param string $table
 * @param string $schema
 * @return Phalcon\Db\Column[]
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Postgresql, describeColumns){

	zval *table, *_schema = NULL, schema = {}, dialect = {}, sql = {}, fetch_num = {}, describe = {}, old_column = {}, *field;

	phalcon_fetch_params(0, 1, 1, &table, &_schema);

	if (!_schema || !zend_is_true(_schema)) {
		phalcon_read_property(&schema, getThis(), SL("_schema"), PH_NOISY|PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&schema, _schema);
	}

	array_init(return_value);

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(&sql, &dialect, "describecolumns", table, &schema);

	/**
	 * We're using FETCH_NUM to fetch the columns
	 */
	ZVAL_LONG(&fetch_num, PDO_FETCH_NUM);

	PHALCON_CALL_METHOD(&describe, getThis(), "fetchall", &sql, &fetch_num);
	zval_ptr_dtor(&sql);

	/**
	 * 0:name, 1:type, 2:size, 3:numeric size, 4:numeric scale, 5: null, 6: key, 7: extra, 8: position, 9: element type
	 */
	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(describe), field) {
		zval definition = {}, char_size = {}, numeric_size = {}, numeric_scale = {}, column_type = {}, element_type = {};
		zval attribute = {}, column_name = {}, column = {};

		array_init_size(&definition, 1);
		add_assoc_long_ex(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_STR);

		phalcon_array_fetch_long(&char_size, field, 2, PH_NOISY|PH_READONLY);
		if (Z_TYPE(char_size) != IS_NULL) {
			convert_to_long(&char_size);
		}

		phalcon_array_fetch_long(&numeric_size, field, 3, PH_NOISY|PH_READONLY);
		if (phalcon_is_numeric(&numeric_size)) {
			convert_to_long(&numeric_size);
		}

		phalcon_array_fetch_long(&numeric_scale, field, 4, PH_NOISY|PH_READONLY);
		if (phalcon_is_numeric(&numeric_scale)) {
			convert_to_long(&numeric_scale);
		}

		phalcon_array_fetch_long(&column_type, field, 1, PH_NOISY|PH_READONLY);

		/**
		 * Check the column type to get the correct Phalcon type
		 */
		while (1) {
			/**
			 * Tinyint(1) is boolean
			 */
			if (phalcon_memnstr_str(&column_type, SL("smallint"))) {
				if (phalcon_is_equal_long(&numeric_size, 1)) {
					phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_BOOLEAN, 0);
					phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_BOOL, 0);
				} else {
					phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_INTEGER, 0);
					phalcon_array_update_str_bool(&definition, SL("isNumeric"), 1, 0);
					phalcon_array_update_str_long(&definition, SL("size"), Z_LVAL(numeric_size), 0);
					phalcon_array_update_str_long(&definition, SL("bytes"), 2, 0);
					phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_INT, 0);
				}
				break;
			}

			/**
			 * Bigint
			 */
			if (phalcon_memnstr_str(&column_type, SL("bigint"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_BIGINTEGER, 0);
				phalcon_array_update_str_bool(&definition, SL("isNumeric"), 1, 0);
				phalcon_array_update_str_long(&definition, SL("size"), Z_LVAL(numeric_size), 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 8, 0);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_INT, 0);
				break;
			}

			/**
			 * Smallint/Bigint/Integers/Int are int
			 */
			if (phalcon_memnstr_str(&column_type, SL("int"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_INTEGER, 0);
				phalcon_array_update_str_bool(&definition, SL("isNumeric"), 1, 0);
				phalcon_array_update_str_long(&definition, SL("size"), Z_LVAL(numeric_size), 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 4, 0);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_INT, 0);
				break;
			}

			/**
			 * Numeric
			 */
			if (phalcon_memnstr_str(&column_type, SL("numeric"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DECIMAL, 0);
				phalcon_array_update_str_bool(&definition, SL("isNumeric"), 1, 0);
				if (Z_TYPE(numeric_size) == IS_LONG) {
					phalcon_array_update_str_long(&definition, SL("size"), Z_LVAL(numeric_size), 0);
					phalcon_array_update_str_long(&definition, SL("bytes"), Z_LVAL(numeric_size), 0);
				} else {
					phalcon_array_update_str_long(&definition, SL("size"), 30, 0);
					phalcon_array_update_str_long(&definition, SL("bytes"), 30, 0);
				}
				if (Z_TYPE(numeric_scale) == IS_LONG) {
					phalcon_array_update_str_long(&definition, SL("scale"), Z_LVAL(numeric_scale), 0);
				} else {
					phalcon_array_update_str_long(&definition, SL("scale"), 6, 0);
				}
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_DECIMAL, 0);
				break;
			}

			/**
			 * Double
			 */
			if (phalcon_memnstr_str(&column_type, SL("double"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DECIMAL, 0);
				phalcon_array_update_str_bool(&definition, SL("isNumeric"), 1, 0);
				if (Z_TYPE(numeric_size) == IS_LONG) {
					phalcon_array_update_str_long(&definition, SL("size"), Z_LVAL(numeric_size), 0);
					phalcon_array_update_str_long(&definition, SL("bytes"), Z_LVAL(numeric_size), 0);
				} else {
					phalcon_array_update_str_long(&definition, SL("size"), 53, 0);
					phalcon_array_update_str_long(&definition, SL("bytes"), 8, 0);
				}
				if (Z_TYPE(numeric_scale) == IS_LONG) {
					phalcon_array_update_str_long(&definition, SL("scale"), Z_LVAL(numeric_scale), 0);
				} else {
					phalcon_array_update_str_long(&definition, SL("scale"), 6, 0);
				}
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_DECIMAL, 0);
				break;
			}

			/**
			 * Float
			 */
			if (phalcon_memnstr_str(&column_type, SL("float"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DECIMAL, 0);
				phalcon_array_update_str_bool(&definition, SL("isNumeric"), 1, 0);

				phalcon_array_update_str_long(&definition, SL("size"), 30, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 10, 0);

				phalcon_array_update_str_long(&definition, SL("scale"), 2, 0);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_DECIMAL, 0);
				break;
			}

			/**
			 * Money
			 */
			if (phalcon_memnstr_str(&column_type, SL("money"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DECIMAL, 0);
				phalcon_array_update_str_bool(&definition, SL("isNumeric"), 1, 0);

				phalcon_array_update_str_long(&definition, SL("size"), 19, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 8, 0);

				phalcon_array_update_str_long(&definition, SL("scale"), 2, 0);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_DECIMAL, 0);
				break;
			}

			/**
			 * Varchar
			 */
			if (phalcon_memnstr_str(&column_type, SL("varying"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_VARCHAR, 0);
				phalcon_array_update_str(&definition, SL("size"), &char_size, 0);
				break;
			}

			/**
			 * Datetime
			 */
			if (phalcon_memnstr_str(&column_type, SL("datetime"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DATETIME, 0);
				break;
			}

			/**
			 * Timestamp
			 */
			if (phalcon_memnstr_str(&column_type, SL("timestamp"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DATETIME, 0);
				break;
			}

			/**
			 * Special type for datetime
			 */
			if (phalcon_memnstr_str(&column_type, SL("date"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DATE, 0);
				break;
			}

			/**
			 * Chars are chars
			 */
			if (phalcon_memnstr_str(&column_type, SL("char"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_CHAR, 0);
				phalcon_array_update_str(&definition, SL("size"), &char_size, 0);
				break;
			}

			/**
			 * Text are varchars
			 */
			if (phalcon_memnstr_str(&column_type, SL("text"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_TEXT, 0);
				phalcon_array_update_str(&definition, SL("size"), &char_size, 0);
				break;
			}

			/**
			 * Boolean
			 */
			if (phalcon_memnstr_str(&column_type, SL("bool"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_BOOLEAN, 0);
				phalcon_array_update_str_long(&definition, SL("size"), 0, 0);
				phalcon_array_update_str_long(&definition, SL("bindType"), 5, 0);
				break;
			}

			/**
			 * UUID
			 */
			if (phalcon_memnstr_str(&column_type, SL("uuid"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_CHAR, 0);
				phalcon_array_update_str_long(&definition, SL("size"), 36, 0);
				break;
			}

			/**
			 * JSON
			 */
			if (phalcon_memnstr_str(&column_type, SL("json"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_JSON, 0);
				break;
			}

			/**
			 * JSONB
			 */
			if (phalcon_memnstr_str(&column_type, SL("jsonb"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_JSONB, 0);
				break;
			}

			/**
			 * ARRAY
			 */
			if (phalcon_memnstr_str(&column_type, SL("ARRAY"))) {
				phalcon_array_fetch_long(&element_type, field, 9, PH_NOISY|PH_READONLY);
				if (phalcon_memnstr_str(&element_type, SL("char"))) {
					phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_ARRAY, 0);
				} else {
					phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_INT_ARRAY, 0);
				}
				break;
			}

			/**
			 * BYTEA
			 */
			if (phalcon_memnstr_str(&column_type, SL("bytea"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_BYTEA, 0);
				break;
			}

			/**
			 * By default is string
			 */
			phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_OTHER, 0);
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
		phalcon_array_fetch_long(&attribute, field, 6, PH_NOISY|PH_READONLY);
		if (PHALCON_IS_STRING(&attribute, "PRI")) {
			phalcon_array_update_str_bool(&definition, SL("primary"), 1, 0);
		}

		/**
		 * Check if the column allows null values
		 */
		phalcon_array_fetch_long(&attribute, field, 5, PH_NOISY|PH_READONLY);
		if (PHALCON_IS_STRING(&attribute, "NO")) {
			phalcon_array_update_str_bool(&definition, SL("notNull"), 1, 0);
		}

		/**
		 * Check if the column is auto increment
		 */
		phalcon_array_fetch_long(&attribute, field, 7, PH_NOISY|PH_READONLY);
		if (PHALCON_IS_STRING(&attribute, "auto_increment")) {
			phalcon_array_update_str_bool(&definition, SL("autoIncrement"), 1, 0);
		} else if (PHALCON_IS_NOT_EMPTY(&attribute)) {
			phalcon_array_update_str(&definition, SL("default"), &attribute, PH_COPY);
		}

		phalcon_array_fetch_long(&column_name, field, 0, PH_NOISY|PH_READONLY);

		/**
		 * Create a Phalcon\Db\Column to abstract the column
		 */
		object_init_ex(&column, phalcon_db_column_ce);
		PHALCON_CALL_METHOD(NULL, &column, "__construct", &column_name, &definition);

		phalcon_array_append(return_value, &column, 0);
		ZVAL_COPY_VALUE(&old_column, &column_name);
		zval_ptr_dtor(&definition);
	} ZEND_HASH_FOREACH_END();
	zval_ptr_dtor(&describe);
}

/**
 * Check whether the database system requires an explicit value for identity columns
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Postgresql, useExplicitIdValue){


	RETURN_TRUE;
}

/**
 * Return the default identity value to insert in an identity column
 *
 * @return Phalcon\Db\RawValue
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Postgresql, getDefaultIdValue){

	zval default_value = {};

	ZVAL_STRING(&default_value, "default");

	object_init_ex(return_value, phalcon_db_rawvalue_ce);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", &default_value);
}

/**
 * Check whether the database system requires a sequence to produce auto-numeric values
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Postgresql, supportSequences){


	RETURN_TRUE;
}

/**
 * Convert php bytea to database bytea
 *
 * @param string $value
 * @return string
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Postgresql, escapeBytea){

	zval *value;

	phalcon_fetch_params(0, 1, 0, &value);

	PHALCON_CALL_FUNCTION(return_value, "pg_escape_bytea", value);
}

/**
 * Convert database bytea to php bytea
 *
 * @param string $value
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Postgresql, unescapeBytea){

	zval *value;

	phalcon_fetch_params(0, 1, 0, &value);

	PHALCON_CALL_FUNCTION(return_value, "pg_unescape_bytea", value);
}

/**
 * Convert php array to database array
 *
 * @param array $value
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Postgresql, escapeArray){

	zval *value, *type = NULL, *constant, ret = {}, search = {}, replace = {};

	phalcon_fetch_params(1, 1, 1, &value, &type);

	if (Z_TYPE_P(value) == IS_NULL) {
		RETURN_NULL();
	}

	if ((constant = zend_get_constant_str(SL("JSON_UNESCAPED_UNICODE"))) != NULL) {
		RETURN_MM_ON_FAILURE(phalcon_json_encode(&ret, value, Z_LVAL_P(constant)));
	} else {
		RETURN_MM_ON_FAILURE(phalcon_json_encode(&ret, value, 0));
	}
	PHALCON_MM_ADD_ENTRY(&ret);

	array_init_size(&search, 2);
	PHALCON_MM_ADD_ENTRY(&search);
	phalcon_array_append_str(&search, SL("["), 0);
	phalcon_array_append_str(&search, SL("]"), 0);

	array_init_size(&replace, 2);
	PHALCON_MM_ADD_ENTRY(&replace);
	phalcon_array_append_str(&replace, SL("{"), 0);
	phalcon_array_append_str(&replace, SL("}"), 0);

	PHALCON_STR_REPLACE(return_value, &search, &replace, &ret);
	RETURN_MM();
}

void pg_text_array_parse(zval *return_value, zval* v) {
	if (Z_TYPE_P(v) != IS_STRING) {
		ZVAL_NULL(return_value);
		return;
	}
	smart_str str = {0};
	if (Z_STRLEN_P(v)) {
		long len = Z_STRLEN_P(v);
		char *c = Z_STRVAL_P(v);
		long i;
		int is_value = 0, has_open = 0, has_escaped = 0;
		for (i = 0; i < len; i++) {
			if (has_escaped) {
				smart_str_appendc(&str, c[i]);
				has_escaped = 0;
				continue;
			}
			if (c[i] == '{') {
				smart_str_appendc(&str, c[i]);
				smart_str_appendc(&str, '"');
				is_value = 1;
				continue;
			}
			if (c[i] == '\\') {
				if (!is_value) {
					smart_str_appendc(&str, '"');
					is_value = 1;
				}
				has_escaped = 1;
				smart_str_appendc(&str, c[i]);
				continue;
			}
			if (c[i] == ',' || c[i] == '}') {
				if (has_open) {
					smart_str_appendc(&str, c[i]);
					continue;
				}
				smart_str_appendc(&str, '"');
				smart_str_appendc(&str, c[i]);
				has_open = 0;
				is_value = 0;
				continue;
			}
			if (c[i] == '"') {
				if (has_open) {
					has_open = 0;
					is_value = 0;
				} else {
					has_open = 1;
					is_value = 1;
				}
				continue;
			}
			if (!is_value) {
				smart_str_appendc(&str, '"');
				is_value = 1;
			}
			smart_str_appendc(&str, c[i]);
		}
	}

	smart_str_0(&str);

	if (str.s) {
		RETURN_NEW_STR(str.s);
	} else {
		smart_str_free(&str);
		RETURN_NULL();
	}
}

/**
 * Convert database array to php array
 *
 * @param string $value
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Postgresql, unescapeArray){

	zval *value, *type = NULL, parse_value = {}, search = {}, replace = {}, ret = {};

	phalcon_fetch_params(1, 1, 1, &value, &type);

	if (Z_LVAL_P(type) != PHALCON_DB_COLUMN_TYPE_INT_ARRAY) {
		pg_text_array_parse(&parse_value, value);
		PHALCON_MM_ADD_ENTRY(&parse_value);
	} else {
		ZVAL_COPY_VALUE(&parse_value, value);
	}

	array_init_size(&search, 2);
	PHALCON_MM_ADD_ENTRY(&search);
	phalcon_array_append_str(&search, SL("{"), 0);
	phalcon_array_append_str(&search, SL("}"), 0);

	array_init_size(&replace, 2);
	PHALCON_MM_ADD_ENTRY(&replace);
	phalcon_array_append_str(&replace, SL("["), 0);
	phalcon_array_append_str(&replace, SL("]"), 0);

	PHALCON_STR_REPLACE(&ret, &search, &replace, &parse_value);
	PHALCON_MM_ADD_ENTRY(&ret);

	RETURN_MM_ON_FAILURE(phalcon_json_decode(return_value, &ret, 1));
	RETURN_MM();
}
