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

#include "db/adapter/pdo/mysql.h"
#include "db/adapter/pdo.h"
#include "db/adapterinterface.h"
#include "db/column.h"

#include <ext/pdo/php_pdo_driver.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/concat.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/operators.h"

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
zend_class_entry *phalcon_db_adapter_pdo_mysql_ce;

PHP_METHOD(Phalcon_Db_Adapter_Pdo_Mysql, escapeIdentifier);
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Mysql, describeColumns);
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Mysql, escapeBytea);
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Mysql, unescapeBytea);
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Mysql, escapeArray);
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Mysql, unescapeArray);

static const zend_function_entry phalcon_db_adapter_pdo_mysql_method_entry[] = {
	PHP_ME(Phalcon_Db_Adapter_Pdo_Mysql, escapeIdentifier, arginfo_phalcon_db_adapterinterface_escapeidentifier, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Mysql, describeColumns, arginfo_phalcon_db_adapterinterface_describecolumns, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Mysql, escapeBytea, arginfo_phalcon_db_adapterinterface_escapebytea, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Mysql, unescapeBytea, arginfo_phalcon_db_adapterinterface_unescapebytea, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Mysql, escapeArray, arginfo_phalcon_db_adapterinterface_escapearray, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo_Mysql, unescapeArray, arginfo_phalcon_db_adapterinterface_unescapearray, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Db\Adapter\Pdo\Mysql initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Adapter_Pdo_Mysql){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Db\\Adapter\\Pdo, Mysql, db_adapter_pdo_mysql, phalcon_db_adapter_pdo_ce, phalcon_db_adapter_pdo_mysql_method_entry, 0);

	zend_declare_property_string(phalcon_db_adapter_pdo_mysql_ce, SL("_type"), "mysql", ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_db_adapter_pdo_mysql_ce, SL("_dialectType"), "mysql", ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_db_adapter_pdo_mysql_ce, 1, phalcon_db_adapterinterface_ce);

	return SUCCESS;
}

/**
 * Escapes a column/table/schema name
 *
 * @param string $identifier
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Mysql, escapeIdentifier){

	zval *identifier, domain = {}, name = {};

	phalcon_fetch_params(0, 1, 0, &identifier);

	if (Z_TYPE_P(identifier) == IS_ARRAY) {
		phalcon_array_fetch_long(&domain, identifier, 0, PH_NOISY|PH_READONLY);
		phalcon_array_fetch_long(&name, identifier, 1, PH_NOISY|PH_READONLY);
		if (PHALCON_GLOBAL(db).escape_identifiers) {
			PHALCON_CONCAT_SVSVS(return_value, "`", &domain, "`.`", &name, "`");
			return;
		}

		PHALCON_CONCAT_VSV(return_value, &domain, ".", &name);

		return;
	}
	if (PHALCON_GLOBAL(db).escape_identifiers) {
		PHALCON_CONCAT_SVS(return_value, "`", identifier, "`");
		return;
	}

	RETURN_CTOR(identifier);
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
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Mysql, describeColumns){

	zval *table, *_schema = NULL, schema = {}, dialect = {}, sql = {}, fetch_num = {}, describe = {}, size_pattern = {}, *field, old_column = {};

	phalcon_fetch_params(1, 1, 1, &table, &_schema);

	if (!_schema || !zend_is_true(_schema)) {
		phalcon_read_property(&schema, getThis(), SL("_schema"), PH_NOISY|PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&schema, _schema);
	}

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY|PH_READONLY);

	/**
	 * Get the SQL to describe a table
	 */
	PHALCON_MM_CALL_METHOD(&sql, &dialect, "describecolumns", table, &schema);
	PHALCON_MM_ADD_ENTRY(&sql);

	/**
	 * We're using FETCH_NUM to fetch the columns
	 */
	ZVAL_LONG(&fetch_num, PDO_FETCH_NUM);

	/**
	 * Get the describe
	 */
	PHALCON_MM_CALL_METHOD(&describe, getThis(), "fetchall", &sql, &fetch_num);
	PHALCON_MM_ADD_ENTRY(&describe);

	array_init(return_value);

	PHALCON_MM_ZVAL_STRING(&size_pattern, "#\\(([0-9]++)(?:,\\s*([0-9]++))?\\)#");

	/**
	 * Field Indexes: 0:name, 1:type, 2:not null, 3:key, 4:default, 5:extra
	 */
	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(describe), field) {
		zval definition = {}, column_type = {}, pos = {}, matches = {}, match_one = {}, match_two = {}, attribute = {}, column_name = {}, column = {};

		/**
		 * By default the bind types is two
		 */
		array_init(&definition);
		PHALCON_MM_ADD_ENTRY(&definition);
		add_assoc_long_ex(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_STR);

		/**
		 * By checking every column type we convert it to a Phalcon\Db\Column
		 */
		phalcon_array_fetch_long(&column_type, field, 1, PH_NOISY|PH_READONLY);

		/**
		 * If the column type has a parentheses we try to get the column size from it
		 */
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
			 * Point are varchars
			 */
			if (phalcon_memnstr_str(&column_type, SL("point"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_VARCHAR, 0);
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
			 * Smallint
			 */
			if (phalcon_memnstr_str(&column_type, SL("smallint"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_INTEGER, 0);
				phalcon_array_update_str(&definition, SL("isNumeric"), &PHALCON_GLOBAL(z_true), PH_COPY);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_INT, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 2, 0);
				break;
			}

			/**
			 * Mediumint
			 */
			if (phalcon_memnstr_str(&column_type, SL("mediumint"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_INTEGER, 0);
				phalcon_array_update_str(&definition, SL("isNumeric"), &PHALCON_GLOBAL(z_true), PH_COPY);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_INT, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 3, 0);
				break;
			}

			/**
			 * BIGINT
			 */
			if (phalcon_memnstr_str(&column_type, SL("bigint"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_BIGINTEGER, 0);
				phalcon_array_update_str(&definition, SL("isNumeric"), &PHALCON_GLOBAL(z_true), PH_COPY);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_INT, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 64, 0);
				break;
			}

			/**
			 * Integers/Int are int
			 */
			if (phalcon_memnstr_str(&column_type, SL("int"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_INTEGER, 0);
				phalcon_array_update_str(&definition, SL("isNumeric"), &PHALCON_GLOBAL(z_true), PH_COPY);
				phalcon_array_update_str_long(&definition, SL("bindType"), 1, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 4, 0);
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
			 * Special type for datetime
			 */
			if (phalcon_memnstr_str(&column_type, SL("datetime"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DATETIME, 0);
				break;
			}

			/**
			 * Decimals are floats
			 */
			if (phalcon_memnstr_str(&column_type, SL("decimal"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DECIMAL, 0);
				phalcon_array_update_str(&definition, SL("isNumeric"), &PHALCON_GLOBAL(z_true), PH_COPY);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_DECIMAL, 0);
				if (phalcon_is_numeric(&match_one)) {
					if (phalcon_is_numeric(&match_two) && PHALCON_GT(&match_two, &match_one)) {
						phalcon_array_update_str_long(&definition, SL("bytes"), (phalcon_get_intval(&match_two) + 2), 0);
					} else {
						phalcon_array_update_str_long(&definition, SL("bytes"), phalcon_get_intval(&match_one), 0);
					}
				} else {
					phalcon_array_update_str_long(&definition, SL("size"), 30, 0);
					phalcon_array_update_str_long(&definition, SL("bytes"), 10, 0);
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
				phalcon_array_update_str(&definition, SL("isNumeric"), &PHALCON_GLOBAL(z_true), PH_COPY);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_DECIMAL, 0);
				if (phalcon_is_numeric(&match_one) && PHALCON_GE_LONG(&match_one, 25)) {
					phalcon_array_update_str_long(&definition, SL("bytes"), 8, 0);
				} else {
					phalcon_array_update_str_long(&definition, SL("bytes"), 4, 0);
				}
				break;
			}

			/**
			 * Double are floats
			 */
			if (phalcon_memnstr_str(&column_type, SL("double"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DOUBLE, 0);
				phalcon_array_update_str(&definition, SL("isNumeric"), &PHALCON_GLOBAL(z_true), PH_COPY);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_DECIMAL, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 8, 0);
				break;
			}

			/**
			 * Boolean
			 */
			if (phalcon_memnstr_str(&column_type, SL("bit"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_BOOLEAN, 0);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_BOOL, 0);
				break;
			}

			/**
			 * TINYBLOB
			 */
			if (phalcon_memnstr_str(&column_type, SL("tinyblob"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_TINYBLOB, 0);
				break;
			}

			/**
			 * MEDIUMBLOB
			 */
			if (phalcon_memnstr_str(&column_type, SL("mediumblob"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_MEDIUMBLOB, 0);
				break;
			}

			/**
			 * LONGBLOB
			 */
			if (phalcon_memnstr_str(&column_type, SL("longblob"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_LONGBLOB, 0);
				break;
			}

			/**
			 * BLOB
			 */
			if (phalcon_memnstr_str(&column_type, SL("blob"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_BLOB, 0);
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
			 * By default is string
			 */
			phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_VARCHAR, 0);
			break;
		}

		/**
		 * Check if the column is unsigned, only MySQL support this
		 */
		if (phalcon_memnstr_str(&column_type, SL("unsigned"))) {
			phalcon_array_update_str(&definition, SL("unsigned"), &PHALCON_GLOBAL(z_true), PH_COPY);
		}

		/**
		 * Positions
		 */
		if (Z_TYPE(old_column) <= IS_NULL) {
			phalcon_array_update_str(&definition, SL("first"), &PHALCON_GLOBAL(z_true), PH_COPY);
		} else {
			phalcon_array_update_str(&definition, SL("after"), &old_column, PH_COPY);
		}

		/**
		 * Check if the field is primary key
		 */
		phalcon_array_fetch_long(&attribute, field, 3, PH_NOISY|PH_READONLY);
		if (PHALCON_IS_STRING(&attribute, "PRI")) {
			phalcon_array_update_str(&definition, SL("primary"), &PHALCON_GLOBAL(z_true), PH_COPY);
		}

		/**
		 * Check if the column allows null values
		 */
		phalcon_array_fetch_long(&attribute, field, 2, PH_NOISY|PH_READONLY);
		if (PHALCON_IS_STRING(&attribute, "NO")) {
			phalcon_array_update_str(&definition, SL("notNull"), &PHALCON_GLOBAL(z_true), PH_COPY);
		}

		/**
		 * Check if the column is auto increment
		 */
		phalcon_array_fetch_long(&attribute, field, 5, PH_NOISY|PH_READONLY);
		if (PHALCON_IS_STRING(&attribute, "auto_increment")) {
			phalcon_array_update_str(&definition, SL("autoIncrement"), &PHALCON_GLOBAL(z_true), PH_COPY);
		}

		phalcon_array_fetch_long(&column_name, field, 0, PH_NOISY|PH_READONLY);

		/**
		 * If the column set the default values, get it
		 */
		phalcon_array_fetch_long(&attribute, field, 4, PH_NOISY|PH_READONLY);
		if (Z_TYPE(attribute) == IS_STRING) {
			phalcon_array_update_str(&definition, SL("default"), &attribute, PH_COPY);
		}

		/**
		 * Every route is stored as a Phalcon\Db\Column
		 */
		object_init_ex(&column, phalcon_db_column_ce);
		PHALCON_MM_CALL_METHOD(NULL, &column, "__construct", &column_name, &definition);

		phalcon_array_append(return_value, &column, 0);

		ZVAL_COPY_VALUE(&old_column, &column_name);
	} ZEND_HASH_FOREACH_END();
	RETURN_MM();
}

/**
 * Convert php bytea to database bytea
 *
 * @param string $value
 * @return string
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Mysql, escapeBytea){

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
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Mysql, unescapeBytea){

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
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Mysql, escapeArray){

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
PHP_METHOD(Phalcon_Db_Adapter_Pdo_Mysql, unescapeArray){

	zval *value, *type = NULL;

	phalcon_fetch_params(0, 1, 1, &value, &type);

	RETURN_CTOR(value);
}
