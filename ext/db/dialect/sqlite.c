
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
  |          Vladimir Kolesnikov <vladimir@extrememember.com>              |
  +------------------------------------------------------------------------+
*/

#include "db/dialect/sqlite.h"
#include "db/dialect.h"
#include "db/dialectinterface.h"
#include "db/column.h"
#include "db/columninterface.h"
#include "db/indexinterface.h"
#include "db/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/operators.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/array.h"
#include "kernel/string.h"

/**
 * Phalcon\Db\Dialect\Sqlite
 *
 * Generates database specific SQL for the SQLite RDBMS
 */
zend_class_entry *phalcon_db_dialect_sqlite_ce;

PHP_METHOD(Phalcon_Db_Dialect_Sqlite, getColumnDefinition);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, addColumn);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, modifyColumn);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, dropColumn);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, addIndex);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, dropIndex);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, addPrimaryKey);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, dropPrimaryKey);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, addForeignKey);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, dropForeignKey);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, _getTableOptions);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, createTable);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, dropTable);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, createView);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, dropView);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, tableExists);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, viewExists);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, describeColumns);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, listTables);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, listViews);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, describeIndexes);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, describeIndex);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, describeReferences);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, tableOptions);
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, getDefaultValue);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_dialect_sqlite_describeindex, 0, 0, 1)
	ZEND_ARG_INFO(0, indexName)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_db_dialect_sqlite_method_entry[] = {
	PHP_ME(Phalcon_Db_Dialect_Sqlite, getColumnDefinition, arginfo_phalcon_db_dialectinterface_getcolumndefinition, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, addColumn, arginfo_phalcon_db_dialectinterface_addcolumn, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, modifyColumn, arginfo_phalcon_db_dialectinterface_modifycolumn, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, dropColumn, arginfo_phalcon_db_dialectinterface_dropcolumn, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, addIndex, arginfo_phalcon_db_dialectinterface_addindex, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, dropIndex, arginfo_phalcon_db_dialectinterface_dropindex, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, addPrimaryKey, arginfo_phalcon_db_dialectinterface_addprimarykey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, dropPrimaryKey, arginfo_phalcon_db_dialectinterface_dropprimarykey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, addForeignKey, arginfo_phalcon_db_dialectinterface_addforeignkey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, dropForeignKey, arginfo_phalcon_db_dialectinterface_dropforeignkey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, _getTableOptions, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, createTable, arginfo_phalcon_db_dialectinterface_createtable, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, dropTable, arginfo_phalcon_db_dialectinterface_droptable, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, createView, arginfo_phalcon_db_dialectinterface_createview, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, dropView, arginfo_phalcon_db_dialectinterface_dropview, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, tableExists, arginfo_phalcon_db_dialectinterface_tableexists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, viewExists, arginfo_phalcon_db_dialectinterface_viewexists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, describeColumns, arginfo_phalcon_db_dialectinterface_describecolumns, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, listTables, arginfo_phalcon_db_dialectinterface_listtables, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, listViews, arginfo_phalcon_db_dialectinterface_listtables, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, describeIndexes, arginfo_phalcon_db_dialectinterface_describeindexes, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, describeIndex, arginfo_phalcon_db_dialect_sqlite_describeindex, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, describeReferences, arginfo_phalcon_db_dialectinterface_describereferences, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, tableOptions, arginfo_phalcon_db_dialectinterface_tableoptions, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Sqlite, getDefaultValue, arginfo_phalcon_db_dialectinterface_getdefaultvalue, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Db\Dialect\Sqlite initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Dialect_Sqlite){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Db\\Dialect, Sqlite, db_dialect_sqlite, phalcon_db_dialect_ce, phalcon_db_dialect_sqlite_method_entry, 0);

	zend_declare_property_string(phalcon_db_dialect_sqlite_ce, SL("_escapeChar"), "\"", ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_db_dialect_sqlite_ce, 1, phalcon_db_dialectinterface_ce);

	return SUCCESS;
}

/**
 * Gets the column name in Sqlite
 *
 * @param Phalcon\Db\ColumnInterface $column
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, getColumnDefinition){

	zval *column, size = {}, column_type = {}, column_sql = {}, type_values = {}, slash = {}, *value, value_cslashes = {}, is_unsigned = {}, scale = {}, name = {};
	int c, i = 0;

	phalcon_fetch_params(0, 1, 0, &column);

	if (Z_TYPE_P(column) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Column definition must be an instance of Phalcon\\Db\\Column");
		return;
	}

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
				break;

			case PHALCON_DB_COLUMN_TYPE_BIGINTEGER:
				PHALCON_CALL_METHOD(&is_unsigned, column, "isunsigned");
				if (zend_is_true(&is_unsigned)) {
					phalcon_concat_self_str(&column_sql, SL(" UNSIGNED"));
				}

				break;

			case PHALCON_DB_COLUMN_TYPE_DECIMAL:
				PHALCON_CALL_METHOD(&scale, column, "getscale");
				PHALCON_SCONCAT_SVSVS(&column_sql, "(", &size, ",", &scale, ")");
				break;

			case PHALCON_DB_COLUMN_TYPE_FLOAT:
				break;

			case PHALCON_DB_COLUMN_TYPE_DOUBLE:

				PHALCON_CALL_METHOD(&is_unsigned, column, "isunsigned");
				if (zend_is_true(&is_unsigned)) {
					phalcon_concat_self_str(&column_sql, SL(" UNSIGNED"));
				}

				break;

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
			case PHALCON_DB_COLUMN_TYPE_TINYBLOB:
			case PHALCON_DB_COLUMN_TYPE_BLOB:
			case PHALCON_DB_COLUMN_TYPE_MEDIUMBLOB:
			case PHALCON_DB_COLUMN_TYPE_LONGBLOB:
			case PHALCON_DB_COLUMN_TYPE_BOOLEAN:
				break;

			default:
				PHALCON_CALL_METHOD(&name, column, "getname");
				PHALCON_THROW_EXCEPTION_FORMAT(phalcon_db_exception_ce, "Unrecognized SQLite data type at column %s", Z_STRVAL(name));
		}
		RETURN_CTOR(&column_sql);
	}

	switch (phalcon_get_intval(&column_type)) {

		case PHALCON_DB_COLUMN_TYPE_INTEGER:
			ZVAL_STRING(&column_sql, "INTEGER");
			break;

		case PHALCON_DB_COLUMN_TYPE_BIGINTEGER:
			ZVAL_STRING(&column_sql, "BIGINT");

			PHALCON_CALL_METHOD(&is_unsigned, column, "isunsigned");
			if (zend_is_true(&is_unsigned)) {
				phalcon_concat_self_str(&column_sql, SL(" UNSIGNED"));
			}

			break;

		case PHALCON_DB_COLUMN_TYPE_DECIMAL:
			PHALCON_CALL_METHOD(&scale, column, "getscale");
			PHALCON_CONCAT_SVSVS(&column_sql, "NUMERIC(", &size, ",", &scale, ")");
			break;

		case PHALCON_DB_COLUMN_TYPE_FLOAT:
			ZVAL_STRING(&column_sql, "FLOAT");
			break;

		case PHALCON_DB_COLUMN_TYPE_DOUBLE:
			ZVAL_STRING(&column_sql, "DOUBLE");

			PHALCON_CALL_METHOD(&is_unsigned, column, "isunsigned");
			if (zend_is_true(&is_unsigned)) {
				phalcon_concat_self_str(&column_sql, SL(" UNSIGNED"));
			}

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
			PHALCON_CONCAT_SVS(&column_sql, "CHARACTER(", &size, ")");
			break;

		case PHALCON_DB_COLUMN_TYPE_VARCHAR:
			PHALCON_CONCAT_SVS(&column_sql, "VARCHAR(", &size, ")");
			break;

		case PHALCON_DB_COLUMN_TYPE_TEXT:
			ZVAL_STRING(&column_sql, "TEXT");
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

		case PHALCON_DB_COLUMN_TYPE_BOOLEAN:
			ZVAL_STRING(&column_sql, "TINYINT");
			break;

		default:
			PHALCON_CALL_METHOD(&name, column, "getname");
			PHALCON_THROW_EXCEPTION_FORMAT(phalcon_db_exception_ce, "Unrecognized SQLite data type at column %s", Z_STRVAL(name));
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
 */
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, addColumn){

	zval *table_name, *schema_name, *column, sql = {}, name = {}, column_definition = {}, column_type = {}, default_value = {}, is_not_null = {}, is_autoincrement = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &column);

	PHALCON_VERIFY_INTERFACE_EX(column, phalcon_db_columninterface_ce, phalcon_db_exception_ce);

	if (zend_is_true(schema_name)) {
		PHALCON_CONCAT_SVSVS(&sql, "ALTER TABLE \"", schema_name, "\".\"", table_name, "\" ADD COLUMN ");
	}
	else {
		PHALCON_CONCAT_SVS(&sql, "ALTER TABLE \"", table_name, "\" ADD COLUMN ");
	}

	PHALCON_CALL_METHOD(&name, column, "getname");
	PHALCON_CALL_METHOD(&column_definition, getThis(), "getcolumndefinition", column);
	PHALCON_SCONCAT_SVSV(&sql, "\"", &name, "\" ", &column_definition);

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
	 * See http://www.sqlite.org/syntaxdiagrams.html#column-constraint
	 */
	if (zend_is_true(&is_autoincrement)) {
		phalcon_concat_self_str(&sql, SL(" PRIMARY KEY AUTOINCREMENT"));
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
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, modifyColumn){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Altering a DB column is not supported by SQLite");
}

/**
 * Generates SQL to delete a column from a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param string $columnName
 * @return 	string
 */
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, dropColumn){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Dropping DB column is not supported by SQLite");
}

/**
 * Generates SQL to add an index to a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\IndexInterface $index
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, addIndex){

	zval *table_name, *schema_name, *index, name = {}, index_type = {}, sql = {}, columns = {}, quoted_column_list = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &index);
	PHALCON_VERIFY_INTERFACE_EX(index, phalcon_db_indexinterface_ce, phalcon_db_exception_ce);


	PHALCON_CALL_METHOD(&name, index, "getname");
	PHALCON_CALL_METHOD(&index_type, index, "gettype");

	if (zend_is_true(schema_name)) {
		if (Z_TYPE(index_type) == IS_STRING && Z_STRLEN(index_type) > 0) {
			PHALCON_CONCAT_SVSVSVSVS(&sql, "CREATE ", &index_type, " INDEX \"", schema_name, "\".\"", &name, "\" ON \"", table_name, "\" (");
		} else {
			PHALCON_CONCAT_SVSVSVS(&sql, "CREATE INDEX \"", schema_name, "\".\"", &name, "\" ON \"", table_name, "\" (");
		}
	} else if (Z_TYPE(index_type) == IS_STRING && Z_STRLEN(index_type) > 0) {
		PHALCON_CONCAT_SVSVSVS(&sql, "CREATE ", &index_type, " INDEX \"", &name, "\" ON \"", table_name, "\" (");
	} else {
		PHALCON_CONCAT_SVSVS(&sql, "CREATE INDEX \"", &name, "\" ON \"", table_name, "\" (");
	}

	PHALCON_CALL_METHOD(&columns, index, "getcolumns");
	PHALCON_CALL_METHOD(&quoted_column_list, getThis(), "getcolumnlist", &columns);

	PHALCON_SCONCAT_VS(&sql, &quoted_column_list, ")");
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
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, dropIndex){

	zval *table_name, *schema_name, *index_name, sql = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &index_name);

	if (zend_is_true(schema_name)) {
		PHALCON_CONCAT_SVSVS(&sql, "DROP INDEX \"", schema_name, "\".\"", index_name, "\"");
	}
	else {
		PHALCON_CONCAT_SVS(&sql, "DROP INDEX \"", index_name, "\"");
	}

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
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, addPrimaryKey){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Adding a primary key after table has been created is not supported by SQLite");
}

/**
 * Generates SQL to delete primary key from a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, dropPrimaryKey){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Removing a primary key after table has been created is not supported by SQLite");
}

/**
 * Generates SQL to add an index to a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\Reference $reference
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, addForeignKey){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Adding a foreign key constraint to an existing table is not supported by SQLite");
}

/**
 * Generates SQL to delete a foreign key from a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param string $referenceName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, dropForeignKey){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Dropping a foreign key constraint is not supported by SQLite");
}

/**
 * Generates SQL to add the table creation options
 *
 * @param array $definition
 * @return array
 */
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, _getTableOptions){

	zval *definition;

	phalcon_fetch_params(0, 1, 0, &definition);

	array_init(return_value);
}

/**
 * Generates SQL to create a table in Sqlite
 *
 * @param 	string $tableName
 * @param string $schemaName
 * @param array $definition
 * @return 	string
 */
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, createTable){

	zval *table_name, *schema_name, *definition, columns = {}, table = {}, options = {}, temporary = {}, sql = {}, create_lines = {}, slash = {};
	zval has_primary = {}, *column, indexes = {}, *index, references = {}, *reference, joined_lines = {};

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
	ZVAL_FALSE(&has_primary);
	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(&columns), column) {
		zval column_name = {}, column_definition = {}, column_type = {}, column_line = {}, default_value = {}, attribute = {};

		PHALCON_CALL_METHOD(&column_name, column, "getname");
		PHALCON_CALL_METHOD(&column_definition, getThis(), "getcolumndefinition", column);

		PHALCON_CONCAT_SVSV(&column_line, "`", &column_name, "` ", &column_definition);

		/**
		 * Mark the column as primary key
		 */
		PHALCON_CALL_METHOD(&attribute, column, "isprimary");
		if (zend_is_true(&attribute) && !zend_is_true(&has_primary)) {
			phalcon_concat_self_str(&column_line, SL(" PRIMARY KEY"));
			ZVAL_TRUE(&has_primary);
		}

		/**
		 * Add an AUTO_INCREMENT clause
		 */
		PHALCON_CALL_METHOD(&attribute, column, "isautoincrement");
		if (zend_is_true(&attribute)) {
			phalcon_concat_self_str(&column_line, SL(" AUTOINCREMENT"));
		}

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
			} else if (PHALCON_IS_NOT_EMPTY(&index_type) && phalcon_comparestr_str(&index_type, SL("UNIQUE"), &PHALCON_GLOBAL(z_false))) {
				PHALCON_CONCAT_SVS(&index_sql, "UNIQUE (", &column_list, ")");
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

	RETURN_CTOR(&sql);
}

/**
 * Generates SQL to drop a table
 *
 * @param  string $tableName
 * @param  string $schemaName
 * @param  boolean $ifExists
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, dropTable){

	zval *table_name, *schema_name, *if_exists = NULL, table = {};

	phalcon_fetch_params(0, 2, 1, &table_name, &schema_name, &if_exists);

	if (!if_exists) {
		if_exists = &PHALCON_GLOBAL(z_true);
	}

	PHALCON_CALL_METHOD(&table, getThis(), "preparetable", table_name, schema_name);

	if (zend_is_true(if_exists)) {
		PHALCON_CONCAT_SVS(return_value, "DROP TABLE IF EXISTS \"", &table, "\"");
	} else {
		PHALCON_CONCAT_SVS(return_value, "DROP TABLE \"", &table, "\"");
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
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, createView){

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
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, dropView){

	zval *view_name, *schema_name, *if_exists = NULL, view = {};

	phalcon_fetch_params(0, 2, 1, &view_name, &schema_name, &if_exists);

	if (!if_exists) {
		if_exists = &PHALCON_GLOBAL(z_true);
	}

	PHALCON_CALL_METHOD(&view, getThis(), "preparetable", view_name, schema_name);

	if (zend_is_true(if_exists)) {
		PHALCON_CONCAT_SV(return_value, "DROP VIEW IF EXISTS ", &view);
	} else {
		PHALCON_CONCAT_SVS(return_value, "DROP VIEW ", &view, "");
	}
	zval_ptr_dtor(&view);
}

/**
 * Generates SQL checking for the existence of a schema.table
 *
 * <code>echo $dialect->tableExists("posts", "blog")</code>
 * <code>echo $dialect->tableExists("posts")</code>
 *
 * @param string $tableName
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, tableExists){

	zval *table_name, *schema_name = NULL;

	phalcon_fetch_params(0, 1, 1, &table_name, &schema_name);

	PHALCON_CONCAT_SVS(return_value, "SELECT CASE WHEN COUNT(*) > 0 THEN 1 ELSE 0 END FROM sqlite_master WHERE type='table' AND tbl_name='", table_name, "'");
}

/**
 * Generates SQL checking for the existence of a schema.view
 *
 * @param string $viewName
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, viewExists){

	zval *view_name, *schema_name = NULL;

	phalcon_fetch_params(0, 1, 1, &view_name, &schema_name);

	PHALCON_CONCAT_SVS(return_value, "SELECT CASE WHEN COUNT(*) > 0 THEN 1 ELSE 0 END FROM sqlite_master WHERE type='view' AND tbl_name='", view_name, "'");
}

/**
 * Generates a SQL describing a table
 *
 * <code>print_r($dialect->describeColumns("posts") ?></code>
 *
 * @param string $table
 * @param string $schema
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, describeColumns){

	zval *table, *schema = NULL;

	phalcon_fetch_params(0, 1, 1, &table, &schema);

	PHALCON_CONCAT_SVS(return_value, "PRAGMA table_info('", table, "')");
}

/**
 * Generates SQL list all tables on database
 *
 * <code>print_r($dialect->listTables("blog")) ?></code>
 *
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, listTables){

	zval *schema_name = NULL;

	phalcon_fetch_params(0, 0, 1, &schema_name);

	RETURN_STRING("SELECT tbl_name FROM sqlite_master WHERE type = 'table' ORDER BY tbl_name");
}

/**
 * Generates the SQL to list all views of a schema or user
 *
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, listViews){

	zval *schema_name = NULL;

	phalcon_fetch_params(0, 0, 1, &schema_name);

	RETURN_STRING("SELECT tbl_name FROM sqlite_master WHERE type = 'view' ORDER BY tbl_name");
}

/**
 * Generates SQL to query indexes on a table
 *
 * @param string $table
 * @param string $schema
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, describeIndexes){

	zval *table, *schema = NULL;

	phalcon_fetch_params(0, 1, 1, &table, &schema);

	PHALCON_CONCAT_SVS(return_value, "PRAGMA index_list('", table, "')");
}

/**
 * Generates SQL to query indexes detail on a table
 *
 * @param string $indexName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, describeIndex){

	zval *index_name;

	phalcon_fetch_params(0, 1, 0, &index_name);

	PHALCON_CONCAT_SVS(return_value, "PRAGMA index_info('", index_name, "')");
}

/**
 * Generates SQL to query foreign keys on a table
 *
 * @param string $table
 * @param string $schema
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, describeReferences){

	zval *table, *schema = NULL;

	phalcon_fetch_params(0, 1, 1, &table, &schema);

	PHALCON_CONCAT_SVS(return_value, "PRAGMA foreign_key_list('", table, "')");
}

/**
 * Generates the SQL to describe the table creation options
 *
 * @param string $table
 * @param string $schema
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, tableOptions){

	zval *table, *schema = NULL;

	phalcon_fetch_params(0, 1, 1, &table, &schema);

	RETURN_EMPTY_STRING();
}

/**
 * Return the default value
 *
 * @param string $defaultValue
 * @param string $columnDefinition
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Sqlite, getDefaultValue){

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
