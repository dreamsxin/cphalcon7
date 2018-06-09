
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

#include "db/dialect/mysql.h"
#include "db/dialect.h"
#include "db/dialectinterface.h"
#include "db/column.h"
#include "db/columninterface.h"
#include "db/indexinterface.h"
#include "db/referenceinterface.h"
#include "db/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"
#include "kernel/concat.h"
#include "kernel/array.h"
#include "kernel/string.h"
#include "kernel/file.h"

/**
 * Phalcon\Db\Dialect\Mysql
 *
 * Generates database specific SQL for the MySQL RBDMS
 */
zend_class_entry *phalcon_db_dialect_mysql_ce;

PHP_METHOD(Phalcon_Db_Dialect_Mysql, getColumnDefinition);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, addColumn);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, modifyColumn);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, dropColumn);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, addIndex);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, dropIndex);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, addPrimaryKey);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, dropPrimaryKey);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, addForeignKey);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, dropForeignKey);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, _getTableOptions);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, createTable);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, dropTable);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, createView);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, dropView);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, tableExists);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, viewExists);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, describeColumns);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, listTables);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, listViews);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, describeIndexes);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, describeReferences);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, tableOptions);
PHP_METHOD(Phalcon_Db_Dialect_Mysql, getDefaultValue);

static const zend_function_entry phalcon_db_dialect_mysql_method_entry[] = {
	PHP_ME(Phalcon_Db_Dialect_Mysql, getColumnDefinition, arginfo_phalcon_db_dialectinterface_getcolumndefinition, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, addColumn, arginfo_phalcon_db_dialectinterface_addcolumn, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, modifyColumn, arginfo_phalcon_db_dialectinterface_modifycolumn, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, dropColumn, arginfo_phalcon_db_dialectinterface_dropcolumn, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, addIndex, arginfo_phalcon_db_dialectinterface_addindex, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, dropIndex, arginfo_phalcon_db_dialectinterface_dropindex, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, addPrimaryKey, arginfo_phalcon_db_dialectinterface_addprimarykey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, dropPrimaryKey, arginfo_phalcon_db_dialectinterface_dropprimarykey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, addForeignKey, arginfo_phalcon_db_dialectinterface_addforeignkey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, dropForeignKey, arginfo_phalcon_db_dialectinterface_dropforeignkey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, _getTableOptions, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Db_Dialect_Mysql, createTable, arginfo_phalcon_db_dialectinterface_createtable, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, dropTable, arginfo_phalcon_db_dialectinterface_droptable, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, createView, arginfo_phalcon_db_dialectinterface_createview, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, dropView, arginfo_phalcon_db_dialectinterface_dropview, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, tableExists, arginfo_phalcon_db_dialectinterface_tableexists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, viewExists, arginfo_phalcon_db_dialectinterface_viewexists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, describeColumns, arginfo_phalcon_db_dialectinterface_describecolumns, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, listTables, arginfo_phalcon_db_dialectinterface_listtables, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, listViews, arginfo_phalcon_db_dialectinterface_listtables, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, describeIndexes, arginfo_phalcon_db_dialectinterface_describeindexes, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, describeReferences, arginfo_phalcon_db_dialectinterface_describereferences, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, tableOptions, arginfo_phalcon_db_dialectinterface_tableoptions, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mysql, getDefaultValue, arginfo_phalcon_db_dialectinterface_getdefaultvalue, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Db\Dialect\Mysql initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Dialect_Mysql){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Db\\Dialect, Mysql, db_dialect_mysql, phalcon_db_dialect_ce, phalcon_db_dialect_mysql_method_entry, 0);

	zend_declare_property_string(phalcon_db_dialect_mysql_ce, SL("_escapeChar"), "`", ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_db_dialect_mysql_ce, 1, phalcon_db_dialectinterface_ce);

	return SUCCESS;
}

/**
 * Gets the column name in MySQL
 *
 * @param Phalcon\Db\ColumnInterface $column
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, getColumnDefinition){

	zval *column, size = {}, column_type = {}, column_sql = {}, type_values = {}, slash = {}, *value, value_cslashes = {}, is_unsigned = {}, scale = {}, name = {};
	int c, i = 0;

	phalcon_fetch_params(0, 1, 0, &column);
	PHALCON_VERIFY_INTERFACE_EX(column, phalcon_db_columninterface_ce, phalcon_db_exception_ce);

	PHALCON_CALL_METHOD(&size, column, "getsize");
	PHALCON_CALL_METHOD(&column_type, column, "gettype");

	if (Z_TYPE(column_type) == IS_STRING) {
		PHALCON_ZVAL_DUP(&column_sql, &column_type);
		PHALCON_CALL_METHOD(&type_values, column, "gettypevalues");
		if (PHALCON_IS_NOT_EMPTY(&type_values)) {
			ZVAL_STRING(&slash, "\"");
			if (Z_TYPE(type_values) == IS_ARRAY) {
				c = phalcon_fast_count_int(&type_values);
				phalcon_concat_self_str(&column_sql, SL("("));
				ZEND_HASH_FOREACH_VAL(Z_ARRVAL(type_values), value) {
					i++;
					PHALCON_CALL_FUNCTION(&value_cslashes, "addcslashes", value, &slash);
					if (i < c) {
						PHALCON_SCONCAT_SVS(&column_sql, "\"", &value_cslashes, "\", ");
					} else {
						PHALCON_SCONCAT_SVS(&column_sql, "\"", &value_cslashes, "\"");
					}
				} ZEND_HASH_FOREACH_END();
				phalcon_concat_self_str(&column_sql, SL(")"));
			} else {
				PHALCON_CALL_FUNCTION(&value_cslashes, "addcslashes", &type_values, &slash);
				PHALCON_SCONCAT_SVS(&column_sql, "(\"", &value_cslashes, "\")");
			}
			RETURN_CTOR(&column_sql);
		}

		PHALCON_CALL_METHOD(&column_type, column, "gettypereference");
		switch (phalcon_get_intval(&column_type)) {
			case PHALCON_DB_COLUMN_TYPE_INTEGER:
				if (Z_LVAL(size) > 0) {
					PHALCON_SCONCAT_SVS(&column_sql, "(", &size, ")");
				}

				PHALCON_CALL_METHOD(&is_unsigned, column, "isunsigned");
				if (zend_is_true(&is_unsigned)) {
					phalcon_concat_self_str(&column_sql, SL(" UNSIGNED"));
				}

				break;

			case PHALCON_DB_COLUMN_TYPE_BIGINTEGER:
				if (Z_LVAL(size) > 0) {
					PHALCON_SCONCAT_SVS(&column_sql, "(", &size, ")");
				}

				PHALCON_CALL_METHOD(&is_unsigned, column, "isunsigned");
				if (zend_is_true(&is_unsigned)) {
					phalcon_concat_self_str(&column_sql, SL(" UNSIGNED"));
				}

				break;

			case PHALCON_DB_COLUMN_TYPE_DECIMAL:
				PHALCON_CALL_METHOD(&scale, column, "getscale");
				PHALCON_SCONCAT_SVSVS(&column_sql, "(", &size, ",", &scale, ")");

				PHALCON_CALL_METHOD(&is_unsigned, column, "isunsigned");
				if (zend_is_true(&is_unsigned)) {
					phalcon_concat_self_str(&column_sql, SL(" UNSIGNED"));
				}

				break;

			case PHALCON_DB_COLUMN_TYPE_FLOAT:
				PHALCON_CALL_METHOD(&scale, column, "getscale");
				if (zend_is_true(&size)) {
					PHALCON_SCONCAT_SV(&column_sql, "(", &size);
					if (zend_is_true(&scale)) {
						PHALCON_SCONCAT_SVS(&column_sql, ",", &scale, ")");
					} else {
						phalcon_concat_self_str(&column_sql, SL(")"));
					}
				}

				PHALCON_CALL_METHOD(&is_unsigned, column, "isunsigned");
				if (zend_is_true(&is_unsigned)) {
					phalcon_concat_self_str(&column_sql, SL(" UNSIGNED"));
				}

				break;

			case PHALCON_DB_COLUMN_TYPE_DOUBLE:
				PHALCON_CALL_METHOD(&scale, column, "getscale");
				if (zend_is_true(&size)) {
					PHALCON_SCONCAT_SV(&column_sql, "(", &size);
					if (zend_is_true(&scale)) {
						PHALCON_SCONCAT_SVS(&column_sql, ",", &scale, ")");
					} else {
						phalcon_concat_self_str(&column_sql, SL(")"));
					}
				}

				PHALCON_CALL_METHOD(&is_unsigned, column, "isunsigned");
				if (zend_is_true(&is_unsigned)) {
					phalcon_concat_self_str(&column_sql, SL(" UNSIGNED"));
				}

				break;

			case PHALCON_DB_COLUMN_TYPE_TINYBLOB:
			case PHALCON_DB_COLUMN_TYPE_BLOB:
			case PHALCON_DB_COLUMN_TYPE_MEDIUMBLOB:
			case PHALCON_DB_COLUMN_TYPE_LONGBLOB:
			case PHALCON_DB_COLUMN_TYPE_DATE:
			case PHALCON_DB_COLUMN_TYPE_DATETIME:
			case PHALCON_DB_COLUMN_TYPE_TIMESTAMP:
				break;

			case PHALCON_DB_COLUMN_TYPE_CHAR:
				PHALCON_SCONCAT_SVS(&column_sql, "(", &size, ")");
				break;

			case PHALCON_DB_COLUMN_TYPE_VARCHAR:
				PHALCON_SCONCAT_SVS(&column_sql, "(", &size, ")");
				break;

			case PHALCON_DB_COLUMN_TYPE_TEXT:
				break;

			case PHALCON_DB_COLUMN_TYPE_BOOLEAN:
				phalcon_concat_self_str(&column_sql, SL("(1)"));
				break;

			default:
				PHALCON_CALL_METHOD(&name, column, "getname");
				PHALCON_THROW_EXCEPTION_FORMAT(phalcon_db_exception_ce, "Unrecognized MySQL data type at column %s", Z_STRVAL(name));
		}

		RETURN_CTOR(&column_sql);
	}

	switch (phalcon_get_intval(&column_type)) {
		case PHALCON_DB_COLUMN_TYPE_INTEGER:
			if (Z_LVAL(size) > 0) {
				PHALCON_CONCAT_SVS(&column_sql, "INT(", &size, ")");
			}
			else {
				ZVAL_STRING(&column_sql, "INT");
			}

			PHALCON_CALL_METHOD(&is_unsigned, column, "isunsigned");
			if (zend_is_true(&is_unsigned)) {
				phalcon_concat_self_str(&column_sql, SL(" UNSIGNED"));
			}

			break;

		case PHALCON_DB_COLUMN_TYPE_BIGINTEGER:
			if (Z_LVAL(size) > 0) {
				PHALCON_CONCAT_SVS(&column_sql, "BIGINT(", &size, ")");
			}
			else {
				ZVAL_STRING(&column_sql, "BIGINT");
			}

			PHALCON_CALL_METHOD(&is_unsigned, column, "isunsigned");
			if (zend_is_true(&is_unsigned)) {
				phalcon_concat_self_str(&column_sql, SL(" UNSIGNED"));
			}

			break;

		case PHALCON_DB_COLUMN_TYPE_DECIMAL:
			PHALCON_CALL_METHOD(&scale, column, "getscale");
			PHALCON_CONCAT_SVSVS(&column_sql, "DECIMAL(", &size, ",", &scale, ")");

			PHALCON_CALL_METHOD(&is_unsigned, column, "isunsigned");
			if (zend_is_true(&is_unsigned)) {
				phalcon_concat_self_str(&column_sql, SL(" UNSIGNED"));
			}

			break;

		case PHALCON_DB_COLUMN_TYPE_FLOAT:
			ZVAL_STRING(&column_sql, "FLOAT");

			PHALCON_CALL_METHOD(&scale, column, "getscale");
			if (Z_TYPE(size) != IS_NULL) {
				PHALCON_SCONCAT_SV(&column_sql, "(", &size);
				if (Z_TYPE(scale) != IS_NULL) {
					PHALCON_SCONCAT_SVS(&column_sql, ",", &scale, ")");
				} else {
					phalcon_concat_self_str(&column_sql, SL(")"));
				}
			}

			PHALCON_CALL_METHOD(&is_unsigned, column, "isunsigned");
			if (zend_is_true(&is_unsigned)) {
				phalcon_concat_self_str(&column_sql, SL(" UNSIGNED"));
			}

			break;

		case PHALCON_DB_COLUMN_TYPE_DOUBLE:
			ZVAL_STRING(&column_sql, "DOUBLE");

			PHALCON_CALL_METHOD(&scale, column, "getscale");
			if (Z_TYPE(size) != IS_NULL) {
				PHALCON_SCONCAT_SV(&column_sql, "(", &size);
				if (Z_TYPE(scale) != IS_NULL) {
					PHALCON_SCONCAT_SVS(&column_sql, ",", &scale, ")");
				} else {
					phalcon_concat_self_str(&column_sql, SL(")"));
				}
			}

			PHALCON_CALL_METHOD(&is_unsigned, column, "isunsigned");
			if (zend_is_true(&is_unsigned)) {
				phalcon_concat_self_str(&column_sql, SL(" UNSIGNED"));
			}

			break;

		case PHALCON_DB_COLUMN_TYPE_TINYBLOB:
			ZVAL_STRING(&column_sql, "TINYBLOB");
			break;

		case PHALCON_DB_COLUMN_TYPE_BLOB:
			ZVAL_STRING(&column_sql, "BLOB");
			break;

		case PHALCON_DB_COLUMN_TYPE_MEDIUMBLOB:
			ZVAL_STRING(&column_sql, "MEDIUMBLOB");
			break;

		case PHALCON_DB_COLUMN_TYPE_LONGBLOB:
			ZVAL_STRING(&column_sql, "LONGBLOB");
			break;

		case PHALCON_DB_COLUMN_TYPE_DATE:
			ZVAL_STRING(&column_sql, "DATE");
			break;

		case PHALCON_DB_COLUMN_TYPE_DATETIME:
			ZVAL_STRING(&column_sql, "DATETIME");
			break;

		case PHALCON_DB_COLUMN_TYPE_TIMESTAMP:
			ZVAL_STRING(&column_sql, "TIMESTAMP");
			break;

		case PHALCON_DB_COLUMN_TYPE_CHAR:
			PHALCON_CONCAT_SVS(&column_sql, "CHAR(", &size, ")");
			break;

		case PHALCON_DB_COLUMN_TYPE_VARCHAR:
			PHALCON_CONCAT_SVS(&column_sql, "VARCHAR(", &size, ")");
			break;

		case PHALCON_DB_COLUMN_TYPE_TEXT:
			ZVAL_STRING(&column_sql, "TEXT");
			break;

		case PHALCON_DB_COLUMN_TYPE_BOOLEAN:
			ZVAL_STRING(&column_sql, "TINYINT(1)");
			break;

		default:
			PHALCON_CALL_METHOD(&name, column, "getname");
			PHALCON_THROW_EXCEPTION_FORMAT(phalcon_db_exception_ce, "Unrecognized MySQL data type at column %s", Z_STRVAL(name));
			return;

	}

	RETURN_CTOR(&column_sql);
}

/**
 * Generates SQL to add a column to a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\ColumnInterface $column
 * @return string
 * @see http://dev.mysql.com/doc/refman/5.5/en/example-auto-increment.html
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, addColumn){

	zval *table_name, *schema_name, *column, table = {}, sql = {}, name = {}, column_definition = {}, column_type = {}, default_value = {};
	zval is_not_null = {}, is_autoincrement = {}, is_first = {}, after_position = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &column);
	PHALCON_VERIFY_INTERFACE_EX(column, phalcon_db_columninterface_ce, phalcon_db_exception_ce);

	PHALCON_CALL_METHOD(&table, getThis(), "preparetable", table_name, schema_name);
	PHALCON_CONCAT_SVS(&sql, "ALTER TABLE ", &table, " ADD ");

	PHALCON_CALL_METHOD(&name, column, "getname");
	PHALCON_CALL_METHOD(&column_definition, getThis(), "getcolumndefinition", column);

	PHALCON_SCONCAT_SVSV(&sql, "`", &name, "` ", &column_definition);

	PHALCON_CALL_METHOD(&default_value, column, "getdefaultvalue");
	if (Z_TYPE(default_value) != IS_NULL) {
		PHALCON_CALL_METHOD(&column_type, column, "gettype");
		PHALCON_CALL_METHOD(&default_value, getThis(), "getdefaultvalue", &default_value, &column_type);
		PHALCON_SCONCAT_SV(&sql, " DEFAULT ", &default_value);
	}

	PHALCON_CALL_METHOD(&is_not_null, column, "isnotnull");
	if (zend_is_true(&is_not_null)) {
		phalcon_concat_self_str(&sql, SL(" NOT NULL"));
	}

	PHALCON_CALL_METHOD(&is_autoincrement, column, "isautoincrement");

	/*
	 * In MySQL the AUTO_INCREMENT column has to be a part of the PRIMARY KEY
	 * This is why we explicitly create PRIMARY KEY here, otherwise ALTER TABLE will fail.
	 */
	if (zend_is_true(&is_autoincrement)) {
		phalcon_concat_self_str(&sql, SL(" PRIMARY KEY AUTO_INCREMENT"));
	}

	PHALCON_CALL_METHOD(&is_first, column, "isfirst");
	if (zend_is_true(&is_first)) {
		phalcon_concat_self_str(&sql, SL(" FIRST"));
	} else {
		PHALCON_CALL_METHOD(&after_position, column, "getafterposition");
		if (zend_is_true(&after_position)) {
			PHALCON_SCONCAT_SVS(&sql, " AFTER `", &after_position, "`");
		}
	}

	RETURN_CTOR(&sql);
}

/**
 * Generates SQL to modify a column in a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\ColumnInterface $column
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, modifyColumn){

	zval *table_name, *schema_name, *column, sql = {}, name = {}, column_definition = {}, column_type = {}, is_not_null = {};
	zval default_value = {}, is_autoincrement = {}, is_first = {}, after_position = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &column);
	PHALCON_VERIFY_INTERFACE_EX(column, phalcon_db_columninterface_ce, phalcon_db_exception_ce);

	if (zend_is_true(schema_name)) {
		PHALCON_CONCAT_SVSVS(&sql, "ALTER TABLE `", schema_name, "`.`", table_name, "` MODIFY ");
	} else {
		PHALCON_CONCAT_SVS(&sql, "ALTER TABLE `", table_name, "` MODIFY ");
	}

	PHALCON_CALL_METHOD(&name, column, "getname");

	PHALCON_CALL_METHOD(&column_definition, getThis(), "getcolumndefinition", column);
	PHALCON_SCONCAT_SVSV(&sql, "`", &name, "` ", &column_definition);

	PHALCON_CALL_METHOD(&default_value, column, "getdefaultvalue");
	if (Z_TYPE(default_value) != IS_NULL) {
		PHALCON_CALL_METHOD(&column_type, column, "gettype");
		PHALCON_CALL_METHOD(&default_value, getThis(), "getdefaultvalue", &default_value, &column_type);
		PHALCON_SCONCAT_SV(&sql, " DEFAULT ", &default_value);
	}

	PHALCON_CALL_METHOD(&is_not_null, column, "isnotnull");
	if (zend_is_true(&is_not_null)) {
		phalcon_concat_self_str(&sql, SL(" NOT NULL"));
	}

	PHALCON_CALL_METHOD(&is_autoincrement, column, "isautoincrement");
	if (zend_is_true(&is_autoincrement)) {
		phalcon_concat_self_str(&sql, SL(" AUTO_INCREMENT"));
	}

	PHALCON_CALL_METHOD(&is_first, column, "isfirst");
	if (zend_is_true(&is_first)) {
		phalcon_concat_self_str(&sql, SL(" FIRST"));
	} else {
		PHALCON_CALL_METHOD(&after_position, column, "getafterposition");
		if (zend_is_true(&after_position)) {
			PHALCON_SCONCAT_SVS(&sql, " AFTER `", &after_position, "`");
		}
	}

	RETURN_CTOR(&sql);
}

/**
 * Generates SQL to delete a column from a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param string $columnName
 * @return 	string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, dropColumn){

	zval *table_name, *schema_name, *column_name, sql = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &column_name);

	if (zend_is_true(schema_name)) {
		PHALCON_CONCAT_SVSVS(&sql, "ALTER TABLE `", schema_name, "`.`", table_name, "` DROP COLUMN ");
	} else {
		PHALCON_CONCAT_SVS(&sql, "ALTER TABLE `", table_name, "` DROP COLUMN ");
	}
	PHALCON_SCONCAT_SVS(&sql, "`", column_name, "`");

	RETURN_CTOR(&sql);
}

/**
 * Generates SQL to add an index to a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\IndexInterface $index
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, addIndex){

	zval *table_name, *schema_name, *index, index_type = {}, sql = {}, columns = {}, quoted_column_list = {}, name = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &index);
	PHALCON_VERIFY_INTERFACE_EX(index, phalcon_db_indexinterface_ce, phalcon_db_exception_ce);

	PHALCON_CALL_METHOD(&index_type, index, "gettype");

	if (zend_is_true(schema_name)) {
		if (Z_TYPE(index_type) == IS_STRING && Z_STRLEN(index_type) > 0) {
			PHALCON_CONCAT_SVSVSVS(&sql, "ALTER TABLE `", schema_name, "`.`", table_name, "` ADD ", &index_type, " INDEX ");
		} else {
			PHALCON_CONCAT_SVSVS(&sql, "ALTER TABLE `", schema_name, "`.`", table_name, "` ADD INDEX ");
		}
	} else if (Z_TYPE(index_type) == IS_STRING && Z_STRLEN(index_type) > 0) {
		PHALCON_CONCAT_SVSVS(&sql, "ALTER TABLE `", table_name, "` ADD ", &index_type, " INDEX ");
	} else {
		PHALCON_CONCAT_SVS(&sql, "ALTER TABLE `", table_name, "` ADD INDEX ");
	}

	PHALCON_CALL_METHOD(&columns, index, "getcolumns");
	PHALCON_CALL_METHOD(&quoted_column_list, getThis(), "getcolumnlist", &columns);
	PHALCON_CALL_METHOD(&name, index, "getname");
	PHALCON_SCONCAT_SVSVS(&sql, "`", &name, "` (", &quoted_column_list, ")");

	RETURN_CTOR(&sql);
}

/**
 * Generates SQL to delete an index from a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param string $indexName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, dropIndex){

	zval *table_name, *schema_name, *index_name, sql;

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &index_name);

	if (zend_is_true(schema_name)) {
		PHALCON_CONCAT_SVSVS(&sql, "ALTER TABLE `", schema_name, "`.`", table_name, "` DROP INDEX ");
	} else {
		PHALCON_CONCAT_SVS(&sql, "ALTER TABLE `", table_name, "` DROP INDEX ");
	}

	PHALCON_SCONCAT_SVS(&sql, "`", index_name, "`");

	RETURN_CTOR(&sql);
}

/**
 * Generates SQL to add the primary key to a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\IndexInterface $index
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, addPrimaryKey){

	zval *table_name, *schema_name, *index, sql = {}, columns = {}, column_list = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &index);
	PHALCON_VERIFY_INTERFACE_EX(index, phalcon_db_indexinterface_ce, phalcon_db_exception_ce);

	if (zend_is_true(schema_name)) {
		PHALCON_CONCAT_SVSVS(&sql, "ALTER TABLE `", schema_name, "`.`", table_name, "` ADD PRIMARY KEY ");
	} else {
		PHALCON_CONCAT_SVS(&sql, "ALTER TABLE `", table_name, "` ADD PRIMARY KEY ");
	}

	PHALCON_CALL_METHOD(&columns, index, "getcolumns");
	PHALCON_CALL_METHOD(&column_list, getThis(), "getcolumnlist", &columns);
	PHALCON_SCONCAT_SVS(&sql, "(", &column_list, ")");

	RETURN_CTOR(&sql);
}

/**
 * Generates SQL to delete primary key from a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, dropPrimaryKey){

	zval *table_name, *schema_name, sql = {};

	phalcon_fetch_params(0, 2, 0, &table_name, &schema_name);

	if (zend_is_true(schema_name)) {
		PHALCON_CONCAT_SVSVS(&sql, "ALTER TABLE `", schema_name, "`.`", table_name, "` DROP PRIMARY KEY");
	} else {
		PHALCON_CONCAT_SVS(&sql, "ALTER TABLE `", table_name, "` DROP PRIMARY KEY");
	}

	RETURN_CTOR(&sql);
}

/**
 * Generates SQL to add an index to a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\ReferenceInterface $reference
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, addForeignKey){

	zval *table_name, *schema_name, *reference, sql = {}, columns = {}, quoted_column_list = {}, reference_name = {};
	zval referenced_schema = {}, referenced_columns = {}, quoted_columns = {}, referenced_table = {}, on_delete = {}, on_update = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &reference);

	PHALCON_VERIFY_INTERFACE_EX(reference, phalcon_db_referenceinterface_ce, phalcon_db_exception_ce);

	if (zend_is_true(schema_name)) {
		PHALCON_CONCAT_SVSVS(&sql, "ALTER TABLE `", schema_name, "`.`", table_name, "` ");
	} else {
		PHALCON_CONCAT_SVS(&sql, "ALTER TABLE `", table_name, "` ");
	}

    PHALCON_CALL_METHOD(&reference_name, reference, "getname");

	PHALCON_SCONCAT_SVS(&sql, "ADD CONSTRAINT `", &reference_name, "` FOREIGN KEY ");

	PHALCON_CALL_METHOD(&columns, reference, "getcolumns");

	PHALCON_CALL_METHOD(&quoted_column_list, getThis(), "getcolumnlist", &columns);

	PHALCON_SCONCAT_SVS(&sql, "(", &quoted_column_list, ") REFERENCES ");

	/**
	 * Add the schema
	 */
	PHALCON_CALL_METHOD(&referenced_schema, reference, "getreferencedschema");

	if (zend_is_true(&referenced_schema)) {
		PHALCON_SCONCAT_SVS(&sql, "`", &referenced_schema, "`.");
	}

	PHALCON_CALL_METHOD(&referenced_columns, reference, "getreferencedcolumns");
	PHALCON_CALL_METHOD(&quoted_columns, getThis(), "getcolumnlist", &referenced_columns);
	PHALCON_CALL_METHOD(&referenced_table, reference, "getreferencedtable");

	PHALCON_SCONCAT_SVSVS(&sql, "`", &referenced_table, "`(", &quoted_columns, ")");

	PHALCON_CALL_METHOD(&on_delete, reference, "getondelete");
	if (PHALCON_IS_NOT_EMPTY(&on_delete)) {
		PHALCON_SCONCAT_SV(&sql, " ON DELETE ", &on_delete);
	}

	PHALCON_CALL_METHOD(&on_update, reference, "getonupdate");
	if (PHALCON_IS_NOT_EMPTY(&on_update)) {
		PHALCON_SCONCAT_SV(&sql, " ON UPDATE ", &on_update);
	}

	RETURN_CTOR(&sql);
}

/**
 * Generates SQL to delete a foreign key from a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param string $referenceName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, dropForeignKey){

	zval *table_name, *schema_name, *reference_name, sql;

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &reference_name);

	if (zend_is_true(schema_name)) {
		PHALCON_CONCAT_SVSVS(&sql, "ALTER TABLE `", schema_name, "`.`", table_name, "` DROP FOREIGN KEY ");
	} else {
		PHALCON_CONCAT_SVS(&sql, "ALTER TABLE `", table_name, "` DROP FOREIGN KEY ");
	}

	PHALCON_SCONCAT_SVS(&sql, "`", reference_name, "`");

	RETURN_CTOR(&sql);
}

/**
 * Generates SQL to add the table creation options
 *
 * @param array $definition
 * @return array
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, _getTableOptions){

	zval *definition, options = {}, table_options = {}, engine = {}, sql_engine = {}, auto_increment = {}, sql_autoincrement = {};
	zval table_collation = {}, collation_parts = {}, first_part = {}, sql_charset = {}, sql_collate = {}, sql_table_options = {};

	phalcon_fetch_params(0, 1, 0, &definition);

	if (phalcon_array_isset_fetch_str(&options, definition, SL("options"), PH_READONLY)) {
		array_init(&table_options);

		/**
		 * Check if there is an ENGINE option
		 */
		if (phalcon_array_isset_fetch_str(&engine, &options, SL("ENGINE"), PH_READONLY)) {
			if (zend_is_true(&engine)) {
				PHALCON_CONCAT_SV(&sql_engine, "ENGINE=", &engine);
				phalcon_array_append(&table_options, &sql_engine, PH_COPY);
			}
		}

		/**
		 * Check if there is an AUTO_INCREMENT option
		 */
		if (phalcon_array_isset_fetch_str(&auto_increment, &options, SL("AUTO_INCREMENT"), PH_READONLY)) {
			if (zend_is_true(&auto_increment)) {
				PHALCON_CONCAT_SV(&sql_autoincrement, "AUTO_INCREMENT=", &auto_increment);
				phalcon_array_append(&table_options, &sql_autoincrement, PH_COPY);
			}
		}

		/**
		 * Check if there is a TABLE_COLLATION option
		 */
		if (phalcon_array_isset_fetch_str(&table_collation, &options, SL("TABLE_COLLATION"), PH_READONLY)) {
			if (zend_is_true(&table_collation)) {
				phalcon_fast_explode_str(&collation_parts, SL("_"), &table_collation);
				phalcon_array_fetch_long(&first_part, &collation_parts, 0, PH_NOISY|PH_READONLY);

				PHALCON_CONCAT_SV(&sql_charset, "DEFAULT CHARSET=", &first_part);
				phalcon_array_append(&table_options, &sql_charset, PH_COPY);

				PHALCON_CONCAT_SV(&sql_collate, "COLLATE=", &table_collation);
				phalcon_array_append(&table_options, &sql_collate, PH_COPY);
			}
		}

		if (phalcon_fast_count_ev(&table_options)) {
			phalcon_fast_join_str(&sql_table_options, SL(" "), &table_options);
			RETURN_CTOR(&sql_table_options);
		}
	}
	RETURN_NULL();
}

/**
 * Generates SQL to create a table in MySQL
 *
 * @param 	string $tableName
 * @param string $schemaName
 * @param array $definition
 * @return 	string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, createTable){

	zval *table_name, *schema_name, *definition, columns = {}, table = {}, options = {}, temporary = {}, sql = {}, create_lines = {}, slash = {};
	zval *column, indexes = {}, *index, references = {}, *reference, joined_lines = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &definition);

	if (!phalcon_array_isset_fetch_str(&columns, definition, SL("columns"), PH_READONLY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The index 'columns' is required in the definition array");
		return;
	}

	PHALCON_CALL_METHOD(&table, getThis(), "preparetable", table_name, schema_name);

	if (phalcon_array_isset_fetch_str(&options, definition, SL("options"), PH_READONLY)) {
		if (!phalcon_array_isset_fetch_str(&temporary, &options, SL("temporary"), PH_READONLY)) {
			ZVAL_FALSE(&temporary);
		}
	} else {
		ZVAL_FALSE(&temporary);
	}

	/**
	 * Create a temporary o normal table
	 */
	if (zend_is_true(&temporary)) {
		PHALCON_CONCAT_SVS(&sql, "CREATE TEMPORARY TABLE ", &table, " (\n\t");
	} else {
		PHALCON_CONCAT_SVS(&sql, "CREATE TABLE ", &table, " (\n\t");
	}

	array_init(&create_lines);

	ZVAL_STRING(&slash, "\"");

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(&columns), column) {
		zval column_name = {}, column_definition = {}, column_type = {}, column_line = {}, default_value = {}, attribute = {};

		PHALCON_CALL_METHOD(&column_name, column, "getname");
		PHALCON_CALL_METHOD(&column_definition, getThis(), "getcolumndefinition", column);

		PHALCON_CONCAT_SVSV(&column_line, "`", &column_name, "` ", &column_definition);

		PHALCON_CALL_METHOD(&default_value, column, "getdefaultvalue");
		if (Z_TYPE(default_value) != IS_NULL) {
			PHALCON_CALL_METHOD(&column_type, column, "gettype");
			PHALCON_CALL_METHOD(&default_value, getThis(), "getdefaultvalue", &default_value, &column_type);
			PHALCON_SCONCAT_SV(&column_line, " DEFAULT ", &default_value);
		}

		/**
		 * Add a NOT NULL clause
		 */
		PHALCON_CALL_METHOD(&attribute, column, "isnotnull");
		if (zend_is_true(&attribute)) {
			phalcon_concat_self_str(&column_line, SL(" NOT NULL"));
		}

		/**
		 * Add an AUTO_INCREMENT clause
		 */
		PHALCON_CALL_METHOD(&attribute, column, "isautoincrement");
		if (zend_is_true(&attribute)) {
			phalcon_concat_self_str(&column_line, SL(" AUTO_INCREMENT"));
		}

		/**
		 * Mark the column as primary key
		 */
		PHALCON_CALL_METHOD(&attribute, column, "isprimary");
		if (zend_is_true(&attribute)) {
			phalcon_concat_self_str(&column_line, SL(" PRIMARY KEY"));
		}

		phalcon_array_append(&create_lines, &column_line, PH_COPY);
	} ZEND_HASH_FOREACH_END();

	/**
	 * Create related indexes
	 */
	if (phalcon_array_isset_fetch_str(&indexes, definition, SL("indexes"), PH_READONLY)) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(&indexes), index) {
			zval index_name = {}, columns = {}, column_list = {}, index_type = {}, index_sql = {};

			PHALCON_CALL_METHOD(&index_name, index, "getname");
			PHALCON_CALL_METHOD(&columns, index, "getcolumns");
			PHALCON_CALL_METHOD(&column_list, getThis(), "getcolumnlist", &columns);
			PHALCON_CALL_METHOD(&index_type, index, "gettype");

			/**
			 * If the index name is primary we add a primary key
			 */
			if (PHALCON_IS_STRING(&index_name, "PRIMARY")) {
				PHALCON_CONCAT_SVS(&index_sql, "PRIMARY KEY (", &column_list, ")");
			} else if (PHALCON_IS_NOT_EMPTY(&index_type)) {
				PHALCON_CONCAT_VSVSVS(&index_sql, &index_type, " KEY `", &index_name, "` (", &column_list, ")");
			} else {
				PHALCON_CONCAT_SVSVS(&index_sql, "KEY `", &index_name, "` (", &column_list, ")");
			}
			phalcon_array_append(&create_lines, &index_sql, PH_COPY);
		} ZEND_HASH_FOREACH_END();
	}

	/**
	 * Create related references
	 */
	if (phalcon_array_isset_fetch_str(&references, definition, SL("references"), PH_READONLY)) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(&references), reference) {
			zval name = {}, columns = {}, column_list = {}, referenced_table = {}, referenced_columns = {}, referenced_column_list = {}, constaint_sql = {}, reference_sql = {}, on_delete = {}, on_update = {};

			PHALCON_CALL_METHOD(&name, reference, "getname");
			PHALCON_CALL_METHOD(&columns, reference, "getcolumns");
			PHALCON_CALL_METHOD(&column_list, getThis(), "getcolumnlist", &columns);
			PHALCON_CALL_METHOD(&referenced_table, reference, "getreferencedtable");
			PHALCON_CALL_METHOD(&referenced_columns, reference, "getreferencedcolumns");
			PHALCON_CALL_METHOD(&referenced_column_list, getThis(), "getcolumnlist", &referenced_columns);

			PHALCON_CONCAT_SVSVS(&constaint_sql, "CONSTRAINT `", &name, "` FOREIGN KEY (", &column_list, ")");
			PHALCON_CONCAT_VSVSVS(&reference_sql, &constaint_sql, " REFERENCES `", &referenced_table, "`(", &referenced_column_list, ")");

			PHALCON_CALL_METHOD(&on_delete, reference, "getondelete");
			if (PHALCON_IS_NOT_EMPTY(&on_delete)) {
				PHALCON_SCONCAT_SV(&reference_sql, " ON DELETE ", &on_delete);
			}

			PHALCON_CALL_METHOD(&on_update, reference, "getonupdate");
			if (PHALCON_IS_NOT_EMPTY(&on_update)) {
				PHALCON_SCONCAT_SV(&reference_sql, " ON UPDATE ", &on_update);
			}

			phalcon_array_append(&create_lines, &reference_sql, PH_COPY);
		} ZEND_HASH_FOREACH_END();
	}

	phalcon_fast_join_str(&joined_lines, SL(",\n\t"), &create_lines);

	PHALCON_SCONCAT_VS(&sql, &joined_lines, "\n)");

	if (phalcon_array_isset_str(definition, SL("options"))) {
		PHALCON_CALL_METHOD(&options, getThis(), "_gettableoptions", definition);
		PHALCON_SCONCAT_SV(&sql, " ", &options);
	}

	RETURN_CTOR(&sql);
}

/**
 * Generates SQL to drop a table
 *
 * @param  string $tableName
 * @param  string $schemaName
 * @param  boolean $ifExists
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, dropTable){

	zval *table_name, *schema_name, *if_exists = NULL, table = {};

	phalcon_fetch_params(0, 2, 1, &table_name, &schema_name, &if_exists);

	if (!if_exists) {
		if_exists = &PHALCON_GLOBAL(z_true);
	}

	PHALCON_CALL_METHOD(&table, getThis(), "preparetable", table_name, schema_name);

	if (zend_is_true(if_exists)) {
		PHALCON_CONCAT_SV(return_value, "DROP TABLE IF EXISTS ", &table);
	} else {
		PHALCON_CONCAT_SV(return_value, "DROP TABLE ", &table);
	}
}

/**
 * Generates SQL to create a view
 *
 * @param string $viewName
 * @param array $definition
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, createView){

	zval *view_name, *definition, *schema_name, view_sql = {}, view = {};

	phalcon_fetch_params(0, 3, 0, &view_name, &definition, &schema_name);

	if (!phalcon_array_isset_fetch_str(&view_sql, definition, SL("sql"), PH_READONLY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The index 'sql' is required in the definition array");
		return;
	}

	PHALCON_CALL_METHOD(&view, getThis(), "preparetable", view_name, schema_name);

	PHALCON_CONCAT_SVSV(return_value, "CREATE VIEW ", &view, " AS ", &view_sql);

	zval_ptr_dtor(&view);
}

/**
 * Generates SQL to drop a view
 *
 * @param string $viewName
 * @param string $schemaName
 * @param boolean $ifExists
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, dropView){

	zval *view_name, *schema_name, *if_exists = NULL, view = {};

	phalcon_fetch_params(0, 2, 1, &view_name, &schema_name, &if_exists);

	if (!if_exists) {
		if_exists = &PHALCON_GLOBAL(z_true);
	}

	PHALCON_CALL_METHOD(&view, getThis(), "preparetable", view_name, schema_name);

	if (zend_is_true(if_exists)) {
		PHALCON_CONCAT_SV(return_value, "DROP VIEW IF EXISTS ", &view);
	} else {
		PHALCON_CONCAT_SV(return_value, "DROP VIEW ", &view);
	}
	zval_ptr_dtor(&view);
}

/**
 * Generates SQL checking for the existence of a schema.table
 *
 * <code>
 * echo $dialect->tableExists("posts", "blog");
 * echo $dialect->tableExists("posts");
 * </code>
 *
 * @param string $tableName
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, tableExists){

	zval *table_name, *schema_name = NULL;

	phalcon_fetch_params(0, 1, 1, &table_name, &schema_name);

	if (schema_name && zend_is_true(schema_name)) {
		PHALCON_CONCAT_SVSVS(return_value, "SELECT IF(COUNT(*)>0, 1 , 0) FROM `INFORMATION_SCHEMA`.`TABLES` WHERE `TABLE_NAME`= '", table_name, "' AND `TABLE_SCHEMA`='", schema_name, "'");
	}
	else {
		PHALCON_CONCAT_SVS(return_value, "SELECT IF(COUNT(*)>0, 1 , 0) FROM `INFORMATION_SCHEMA`.`TABLES` WHERE `TABLE_NAME`='", table_name, "'");
	}
}

/**
 * Generates SQL checking for the existence of a schema.view
 *
 * @param string $viewName
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, viewExists){

	zval *view_name, *schema_name = NULL;

	phalcon_fetch_params(0, 1, 1, &view_name, &schema_name);

	if (schema_name && zend_is_true(schema_name)) {
		PHALCON_CONCAT_SVSVS(return_value, "SELECT IF(COUNT(*)>0, 1 , 0) FROM `INFORMATION_SCHEMA`.`VIEWS` WHERE `TABLE_NAME`= '", view_name, "' AND `TABLE_SCHEMA`='", schema_name, "'");
	}
	else {
		PHALCON_CONCAT_SVS(return_value, "SELECT IF(COUNT(*)>0, 1 , 0) FROM `INFORMATION_SCHEMA`.`VIEWS` WHERE `TABLE_NAME`='", view_name, "'");
	}
}

/**
 * Generates SQL describing a table
 *
 *<code>
 *	print_r($dialect->describeColumns("posts")) ?>
 *</code>
 *
 * @param string $table
 * @param string $schema
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, describeColumns){

	zval *table_name, *schema_name = NULL, table = {};

	phalcon_fetch_params(0, 1, 1, &table_name, &schema_name);

	if (!schema_name) {
		PHALCON_CALL_METHOD(&table, getThis(), "preparetable", table_name);
	} else {
		PHALCON_CALL_METHOD(&table, getThis(), "preparetable", table_name, schema_name);
	}

	PHALCON_CONCAT_SV(return_value, "DESCRIBE ", &table);
	zval_ptr_dtor(&table);

}

/**
 * Generates SQL list all tables on database
 *
 *<code>
 *	print_r($dialect->listTables("blog")) ?>
 *</code>
 *
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, listTables){

	zval *schema_name = NULL;

	phalcon_fetch_params(0, 0, 1, &schema_name);

	if (schema_name && zend_is_true(schema_name)) {
		PHALCON_CONCAT_SVS(return_value, "SHOW TABLES FROM `", schema_name, "`");
	}
	else {
		RETURN_STRING("SHOW TABLES");
	}
}

/**
 * Generates the SQL to list all views of a schema or user
 *
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, listViews){

	zval *schema_name = NULL;

	phalcon_fetch_params(0, 0, 1, &schema_name);

	if (schema_name && zend_is_true(schema_name)) {
		PHALCON_CONCAT_SVS(return_value, "SELECT `TABLE_NAME` AS view_name FROM `INFORMATION_SCHEMA`.`VIEWS` WHERE `TABLE_SCHEMA` = '", schema_name, "' ORDER BY view_name");
	}
	else {
		RETURN_STRING("SELECT `TABLE_NAME` AS view_name FROM `INFORMATION_SCHEMA`.`VIEWS` ORDER BY view_name");
	}
}

/**
 * Generates SQL to query indexes on a table
 *
 * @param string $table
 * @param string $schema
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, describeIndexes){

	zval *table, *schema = NULL;

	phalcon_fetch_params(0, 1, 1, &table, &schema);

	if (schema && zend_is_true(schema)) {
		PHALCON_CONCAT_SVSVS(return_value, "SHOW INDEXES FROM `", schema, "`.`", table, "`");
	}
	else {
		PHALCON_CONCAT_SVS(return_value, "SHOW INDEXES FROM `", table, "`");
	}
}

/**
 * Generates SQL to query foreign keys on a table
 *
 * @param string $table
 * @param string $schema
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, describeReferences){

	zval *table, *schema = NULL, sql = {};

	phalcon_fetch_params(0, 1, 1, &table, &schema);

	ZVAL_STRING(&sql, "SELECT TABLE_NAME,COLUMN_NAME,CONSTRAINT_NAME,REFERENCED_TABLE_SCHEMA,REFERENCED_TABLE_NAME,REFERENCED_COLUMN_NAME FROM INFORMATION_SCHEMA.KEY_COLUMN_USAGE WHERE REFERENCED_TABLE_NAME IS NOT NULL AND ");
	if (schema && zend_is_true(schema)) {
		PHALCON_SCONCAT_SVSVS(&sql, "CONSTRAINT_SCHEMA = \"", schema, "\" AND TABLE_NAME = \"", table, "\"");
	} else {
		PHALCON_SCONCAT_SVS(&sql, "TABLE_NAME = \"", table, "\"");
	}

	RETURN_ZVAL(&sql, 0, 0);
}

/**
 * Generates the SQL to describe the table creation options
 *
 * @param string $table
 * @param string $schema
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, tableOptions){

	zval *table, *schema = NULL, sql = {};

	phalcon_fetch_params(0, 1, 1, &table, &schema);

	ZVAL_STRING(&sql, "SELECT TABLES.TABLE_TYPE AS table_type,TABLES.AUTO_INCREMENT AS auto_increment,TABLES.ENGINE AS engine,TABLES.TABLE_COLLATION AS table_collation FROM INFORMATION_SCHEMA.TABLES WHERE ");
	if (schema && zend_is_true(schema)) {
		PHALCON_SCONCAT_SVSVS(&sql, "TABLES.TABLE_SCHEMA = \"", schema, "\" AND TABLES.TABLE_NAME = \"", table, "\"");
	} else {
		PHALCON_SCONCAT_SVS(&sql, "TABLES.TABLE_NAME = \"", table, "\"");
	}

	RETURN_ZVAL(&sql, 0, 0);
}

/**
 * Return the default value
 *
 * @param string $defaultValue
 * @param string $columnDefinition
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mysql, getDefaultValue){

	zval *default_value, *column_type, slash = {}, value_cslashes = {};
	int type;

	phalcon_fetch_params(0, 2, 0, &default_value, &column_type);

	type = Z_LVAL_P(column_type);

	if (Z_TYPE_P(column_type) == IS_LONG) {
		type = Z_LVAL_P(column_type);

		if (type == PHALCON_DB_COLUMN_TYPE_BOOLEAN) {
			if (zend_is_true(default_value)) {
				ZVAL_STRING(return_value, "true");
			} else {
				ZVAL_STRING(return_value, "false");
			}
			return;
		} else if (phalcon_comparestr_str(default_value, SL("CURRENT_TIMESTAMP"), &PHALCON_GLOBAL(z_false))) {
			ZVAL_STRING(return_value, "CURRENT_TIMESTAMP");
			return;
		}  else if (
			type == PHALCON_DB_COLUMN_TYPE_INTEGER
			|| type == PHALCON_DB_COLUMN_TYPE_BIGINTEGER
			|| type == PHALCON_DB_COLUMN_TYPE_FLOAT
			|| type == PHALCON_DB_COLUMN_TYPE_DOUBLE
			|| type == PHALCON_DB_COLUMN_TYPE_DECIMAL
			|| type == PHALCON_DB_COLUMN_TYPE_MONEY
		) {
			RETURN_CTOR(default_value);
		}
	}
	ZVAL_STRING(&slash, "\"");
	PHALCON_CALL_FUNCTION(&value_cslashes, "addcslashes", default_value, &slash);
	zval_ptr_dtor(&slash);
	PHALCON_CONCAT_SVS(return_value, "\"", &value_cslashes, "\"");
	zval_ptr_dtor(&value_cslashes);
}
