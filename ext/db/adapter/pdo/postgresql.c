
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

#include "db/adapter/pdo/postgresql.h"
#include "db/adapter/pdo.h"
#include "db/adapterinterface.h"
#include "db/column.h"
#include "db/rawvalue.h"
#include "db/reference.h"

#include <ext/pdo/php_pdo_driver.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/string.h"
#include "kernel/operators.h"

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

static const zend_function_entry phalcon_db_adapter_pdo_postgresql_method_entry[] = {
	PHP_ME(Phalcon_Db_Adapter_Pdo_Postgresql, connect, arginfo_phalcon_db_adapterinterface_connect, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Postgresql, describeColumns, arginfo_phalcon_db_adapterinterface_describecolumns, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Postgresql, useExplicitIdValue, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Postgresql, getDefaultIdValue, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Postgresql, supportSequences, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Db\Adapter\Pdo\Postgresql initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Adapter_Pdo_Postgresql){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Db\\Adapter\\Pdo, Postgresql, db_adapter_pdo_postgresql, phalcon_db_adapter_pdo_ce, phalcon_db_adapter_pdo_postgresql_method_entry, 0);

	zend_declare_property_string(phalcon_db_adapter_pdo_postgresql_ce, SL("_type"), "pgsql", ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_db_adapter_pdo_postgresql_ce, SL("_dialectType"), "postgresql", ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_pdo_postgresql_ce, SL("_schema"), ZEND_ACC_PROTECTED);

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

	zval *descriptor = NULL, *schema = NULL, *sql, *password;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 0, 1, &descriptor);

	if (!descriptor) {
		PHALCON_INIT_VAR(descriptor);
	} else {
		PHALCON_SEPARATE_PARAM(descriptor);
	}

	if (!zend_is_true(descriptor)) {
		descriptor = phalcon_read_property(getThis(), SL("_descriptor"), PH_NOISY);
	}

	if (phalcon_array_isset_str(descriptor, SL("schema"))) {
		PHALCON_OBS_VAR(schema);
		phalcon_array_fetch_str(&schema, descriptor, SL("schema"), PH_NOISY);
		phalcon_array_unset_str(descriptor, SL("schema"), PH_COPY);

		phalcon_update_property_this(getThis(), SL("_schema"), schema);
	}
	else {
		PHALCON_INIT_VAR(schema);
	}

	if (phalcon_array_isset_str_fetch(&password, descriptor, SL("password"))) {
		/* There is a bug in pdo_pgsql driver when the password is empty,
		 * the driver tries to access invalid memory:
		 *
		 * if (dbh->password[0] != '\'' && dbh->password[strlen(dbh->password) - 1] != '\'')
		 *
		 * To avoid this we set the password to null
		 */
		if (Z_TYPE_P(password) == IS_STRING && Z_STRLEN_P(password) == 0) {
			phalcon_array_update_str(descriptor, SL("password"), &PHALCON_GLOBAL(z_null), PH_COPY | PH_COPY);
		}
	}


	PHALCON_CALL_PARENT(NULL, phalcon_db_adapter_pdo_postgresql_ce, getThis(), "connect", descriptor);

	/** 
	 * Execute the search path in the after connect
	 */
	if (Z_TYPE_P(schema) == IS_STRING) {
		PHALCON_INIT_VAR(sql);
		PHALCON_CONCAT_SVS(sql, "SET search_path TO '", schema, "'");
		PHALCON_CALL_METHOD(NULL, getThis(), "execute", sql);
	}

	PHALCON_MM_RESTORE();
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

	zval *table, *schema = NULL, *columns, *dialect, *sql = NULL, *fetch_num;
	zval *describe = NULL, *old_column = NULL, *field = NULL, *definition = NULL;
	zval *char_size = NULL, *numeric_size = NULL, *numeric_scale = NULL, *column_type = NULL;
	zval *attribute = NULL, *column_name = NULL, *column = NULL;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 1, &table, &schema);

	if (!schema || !zend_is_true(schema)) {
		schema = phalcon_read_property(getThis(), SL("_schema"), PH_NOISY);
	}

	PHALCON_INIT_VAR(columns);
	array_init(columns);

	dialect = phalcon_read_property(getThis(), SL("_dialect"), PH_NOISY);

	PHALCON_CALL_METHOD(&sql, dialect, "describecolumns", table, schema);

	/** 
	 * We're using FETCH_NUM to fetch the columns
	 */
	PHALCON_INIT_VAR(fetch_num);
	ZVAL_LONG(fetch_num, PDO_FETCH_NUM);

	PHALCON_CALL_METHOD(&describe, getThis(), "fetchall", sql, fetch_num);

	/** 
	 * 0:name, 1:type, 2:size, 3:numeric size, 4:numeric scale, 5: null, 6: key, 7: extra, 8: position, 9: element type
	 */
	PHALCON_INIT_VAR(old_column);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(describe), field) {
		PHALCON_INIT_NVAR(definition);
		array_init_size(definition, 1);
		add_assoc_long_ex(definition, SL("bindType"), 2);

		PHALCON_OBS_NVAR(char_size);
		phalcon_array_fetch_long(&char_size, field, 2, PH_NOISY);
		if (Z_TYPE_P(char_size) != IS_NULL) {
			convert_to_long(char_size);
		}

		PHALCON_OBS_NVAR(numeric_size);
		phalcon_array_fetch_long(&numeric_size, field, 3, PH_NOISY);
		if (phalcon_is_numeric(numeric_size)) {
			convert_to_long(numeric_size);
		}

		PHALCON_OBS_NVAR(numeric_scale); 
		phalcon_array_fetch_long(&numeric_scale, field, 4, PH_NOISY);
		if (phalcon_is_numeric(numeric_scale)) {
			convert_to_long(numeric_scale);
		}

		PHALCON_OBS_NVAR(column_type);
		phalcon_array_fetch_long(&column_type, field, 1, PH_NOISY);

		/** 
		 * Check the column type to get the correct Phalcon type
		 */
		while (1) {
			/**
			 * Tinyint(1) is boolean
			 */
			if (phalcon_memnstr_str(column_type, SL("smallint(1)"))) {
				phalcon_array_update_str_long(definition, SL("type"), PHALCON_DB_COLUMN_TYPE_BOOLEAN, 0);
				phalcon_array_update_str_long(definition, SL("bindType"), 5, 0);
				break;
			}

			/**
			 * Smallint/Bigint/Integers/Int are int
			 */
			if (phalcon_memnstr_str(column_type, SL("int"))) {
				phalcon_array_update_str_long(definition, SL("type"), PHALCON_DB_COLUMN_TYPE_INTEGER, 0);
				phalcon_array_update_str_bool(definition, SL("isNumeric"), 1, 0);
				phalcon_array_update_str(definition, SL("size"), numeric_size, PH_COPY);
				phalcon_array_update_str_long(definition, SL("bindType"), 1, 0);
				break;
			}

			/**
			 * Varchar
			 */
			if (phalcon_memnstr_str(column_type, SL("varying"))) {
				phalcon_array_update_str_long(definition, SL("type"), PHALCON_DB_COLUMN_TYPE_VARCHAR, 0);
				phalcon_array_update_str(definition, SL("size"), char_size, PH_COPY);
				break;
			}

			/**
			 * Special type for datetime
			 */
			if (phalcon_memnstr_str(column_type, SL("date"))) {
				phalcon_array_update_str_long(definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DATE, 0);
				phalcon_array_update_str_long(definition, SL("size"), 0, 0);
				break;
			}

			/**
			 * Numeric
			 */
			if (phalcon_memnstr_str(column_type, SL("numeric"))) {
				phalcon_array_update_str_long(definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DECIMAL, 0);
				phalcon_array_update_str_bool(definition, SL("isNumeric"), 1, 0);
				if (phalcon_is_numeric(numeric_size)) {
					phalcon_array_update_str(definition, SL("size"), numeric_size, PH_COPY);
					phalcon_array_update_str_long(definition, SL("bytes"), Z_LVAL_P(numeric_size) * 8, 0);
				} else {
					phalcon_array_update_str_long(definition, SL("size"), 30, 0);
					phalcon_array_update_str_long(definition, SL("bytes"), 80, 0);
				}
				if (phalcon_is_numeric(numeric_scale)) {
					phalcon_array_update_str(definition, SL("scale"), numeric_scale, PH_COPY);
				} else {
					phalcon_array_update_str_long(definition, SL("scale"), 6, 0);
				}
				phalcon_array_update_str_long(definition, SL("bindType"), 32, 0);
				break;
			}

			/**
			 * Chars are chars
			 */
			if (phalcon_memnstr_str(column_type, SL("char"))) {
				phalcon_array_update_str_long(definition, SL("type"), PHALCON_DB_COLUMN_TYPE_CHAR, 0);
				phalcon_array_update_str(definition, SL("size"), char_size, PH_COPY);
				break;
			}

			/**
			 * Date
			 */
			if (phalcon_memnstr_str(column_type, SL("timestamp"))) {
				phalcon_array_update_str_long(definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DATETIME, 0);
				phalcon_array_update_str_long(definition, SL("size"), 0, 0);
				break;
			}

			/**
			 * Text are varchars
			 */
			if (phalcon_memnstr_str(column_type, SL("text"))) {
				phalcon_array_update_str_long(definition, SL("type"), PHALCON_DB_COLUMN_TYPE_TEXT, 0);
				phalcon_array_update_str(definition, SL("size"), char_size, PH_COPY);
				break;
			}

			/**
			 * Float/Smallfloats/Decimals are float
			 */
			if (phalcon_memnstr_str(column_type, SL("float"))) {
				phalcon_array_update_str_long(definition, SL("type"), PHALCON_DB_COLUMN_TYPE_FLOAT, 0);
				phalcon_array_update_str_bool(definition, SL("isNumeric"), 1, 0);
				phalcon_array_update_str(definition, SL("size"), numeric_size, PH_COPY);
				phalcon_array_update_str_long(definition, SL("bindType"), 32, 0);
				break;
			}

			/**
			 * Boolean
			 */
			if (phalcon_memnstr_str(column_type, SL("bool"))) {
				phalcon_array_update_str_long(definition, SL("type"), PHALCON_DB_COLUMN_TYPE_BOOLEAN, 0);
				phalcon_array_update_str_long(definition, SL("size"), 0, 0);
				phalcon_array_update_str_long(definition, SL("bindType"), 5, 0);
				break;
			}

			/**
			 * UUID
			 */
			if (phalcon_memnstr_str(column_type, SL("uuid"))) {
				phalcon_array_update_str_long(definition, SL("type"), PHALCON_DB_COLUMN_TYPE_CHAR, 0);
				phalcon_array_update_str_long(definition, SL("size"), 36, 0);
				break;
			}

			/**
			 * JSON
			 */
			if (phalcon_memnstr_str(column_type, SL("json"))) {
				phalcon_array_update_str_long(definition, SL("type"), PHALCON_DB_COLUMN_TYPE_JSON, 0);
				phalcon_array_update_str(definition, SL("size"), char_size, PH_COPY);
				break;
			}

			/**
			 * ARRAY
			 */
			if (phalcon_memnstr_str(column_type, SL("ARRAY"))) {
				phalcon_array_update_str_long(definition, SL("type"), PHALCON_DB_COLUMN_TYPE_ARRAY, 0);
				phalcon_array_update_str(definition, SL("size"), char_size, PH_COPY);
				break;
			}

			/**
			 * By default is string
			 */
			phalcon_array_update_str_long(definition, SL("type"), PHALCON_DB_COLUMN_TYPE_OTHER, 0);
			break;
		}

		if (phalcon_memnstr_str(column_type, SL("unsigned"))) {
			phalcon_array_update_str_bool(definition, SL("unsigned"), 1, 0);
		}

		if (Z_TYPE_P(old_column) == IS_NULL) {
			phalcon_array_update_str_bool(definition, SL("first"), 1, 0);
		} else {
			phalcon_array_update_str(definition, SL("after"), old_column, PH_COPY);
		}

		/** 
		 * Check if the field is primary key
		 */
		PHALCON_OBS_NVAR(attribute);
		phalcon_array_fetch_long(&attribute, field, 6, PH_NOISY);
		if (PHALCON_IS_STRING(attribute, "PRI")) {
			phalcon_array_update_str_bool(definition, SL("primary"), 1, 0);
		}

		/** 
		 * Check if the column allows null values
		 */
		PHALCON_OBS_NVAR(attribute);
		phalcon_array_fetch_long(&attribute, field, 5, PH_NOISY);
		if (PHALCON_IS_STRING(attribute, "NO")) {
			phalcon_array_update_str_bool(definition, SL("notNull"), 1, 0);
		}

		/** 
		 * Check if the column is auto increment
		 */
		PHALCON_OBS_NVAR(attribute);
		phalcon_array_fetch_long(&attribute, field, 7, PH_NOISY);
		if (PHALCON_IS_STRING(attribute, "auto_increment")) {
			phalcon_array_update_str_bool(definition, SL("autoIncrement"), 1, 0);
		} else if (!PHALCON_IS_EMPTY(attribute)) {
			phalcon_array_update_str(definition, SL("default"), attribute, PH_COPY);
		}

		PHALCON_OBS_NVAR(column_name);
		phalcon_array_fetch_long(&column_name, field, 0, PH_NOISY);

		/** 
		 * Create a Phalcon\Db\Column to abstract the column
		 */
		PHALCON_INIT_NVAR(column);
		object_init_ex(column, phalcon_db_column_ce);
		PHALCON_CALL_METHOD(NULL, column, "__construct", column_name, definition);

		phalcon_array_append(columns, column, PH_COPY);
		PHALCON_CPY_WRT(old_column, column_name);
	} ZEND_HASH_FOREACH_END();

	RETURN_CTOR(columns);
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

	zval *null_value, *default_value;

	PHALCON_MM_GROW();

	PHALCON_INIT_VAR(null_value);
	ZVAL_STRING(null_value, "default");

	PHALCON_INIT_VAR(default_value);
	object_init_ex(default_value, phalcon_db_rawvalue_ce);
	PHALCON_CALL_METHOD(NULL, default_value, "__construct", null_value);

	RETURN_CTOR(default_value);
}

/**
 * Check whether the database system requires a sequence to produce auto-numeric values
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Postgresql, supportSequences){


	RETURN_TRUE;
}
