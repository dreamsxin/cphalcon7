
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

#include "db/dialect/postgresql.h"
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

/**
 * Phalcon\Db\Dialect\Postgresql
 *
 * Generates database specific SQL for the PostgreSQL RBDMS
 */
zend_class_entry *phalcon_db_dialect_postgresql_ce;

PHP_METHOD(Phalcon_Db_Dialect_Postgresql, getColumnDefinition);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, addColumn);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, modifyColumn);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, dropColumn);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, addIndex);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, dropIndex);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, addPrimaryKey);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, dropPrimaryKey);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, addForeignKey);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, dropForeignKey);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, _getTableOptions);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, createTable);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, dropTable);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, createView);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, dropView);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, tableExists);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, viewExists);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, describeColumns);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, listTables);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, listViews);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, describeIndexes);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, describeReferences);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, tableOptions);
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, getDefaultValue);

static const zend_function_entry phalcon_db_dialect_postgresql_method_entry[] = {
	PHP_ME(Phalcon_Db_Dialect_Postgresql, getColumnDefinition, arginfo_phalcon_db_dialectinterface_getcolumndefinition, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, addColumn, arginfo_phalcon_db_dialectinterface_addcolumn, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, modifyColumn, arginfo_phalcon_db_dialectinterface_modifycolumn, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, dropColumn, arginfo_phalcon_db_dialectinterface_dropcolumn, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, addIndex, arginfo_phalcon_db_dialectinterface_addindex, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, dropIndex, arginfo_phalcon_db_dialectinterface_dropindex, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, addPrimaryKey, arginfo_phalcon_db_dialectinterface_addprimarykey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, dropPrimaryKey, arginfo_phalcon_db_dialectinterface_dropprimarykey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, addForeignKey, arginfo_phalcon_db_dialectinterface_addforeignkey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, dropForeignKey, arginfo_phalcon_db_dialectinterface_dropforeignkey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, _getTableOptions, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, createTable, arginfo_phalcon_db_dialectinterface_createtable, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, dropTable, arginfo_phalcon_db_dialectinterface_droptable, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, createView, arginfo_phalcon_db_dialectinterface_createview, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, dropView, arginfo_phalcon_db_dialectinterface_dropview, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, tableExists, arginfo_phalcon_db_dialectinterface_tableexists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, viewExists, arginfo_phalcon_db_dialectinterface_viewexists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, describeColumns, arginfo_phalcon_db_dialectinterface_describecolumns, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, listTables, arginfo_phalcon_db_dialectinterface_listtables, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, listViews, arginfo_phalcon_db_dialectinterface_listtables, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, describeIndexes, arginfo_phalcon_db_dialectinterface_describeindexes, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, describeReferences, arginfo_phalcon_db_dialectinterface_describereferences, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, tableOptions, arginfo_phalcon_db_dialectinterface_tableoptions, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Postgresql, getDefaultValue, arginfo_phalcon_db_dialectinterface_getdefaultvalue, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Db\Dialect\Postgresql initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Dialect_Postgresql){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Db\\Dialect, Postgresql, db_dialect_postgresql, phalcon_db_dialect_ce, phalcon_db_dialect_postgresql_method_entry, 0);

	zend_declare_property_string(phalcon_db_dialect_postgresql_ce, SL("_escapeChar"), "\"", ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_db_dialect_postgresql_ce, 1, phalcon_db_dialectinterface_ce);

	return SUCCESS;
}

/**
 * Gets the column name in PostgreSQL
 *
 * @param Phalcon\Db\ColumnInterface $column
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, getColumnDefinition){

	zval *column, size = {}, column_type = {}, isautoincrement = {}, column_sql = {}, type_values = {}, *value, name = {};
	int c, i = 0;

	phalcon_fetch_params(0, 1, 0, &column);

	if (Z_TYPE_P(column) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Column definition must be an object compatible with Phalcon\\Db\\ColumnInterface");
		return;
	}

	PHALCON_CALL_METHOD(&size, column, "getsize");
	PHALCON_CALL_METHOD(&column_type, column, "gettype");
	PHALCON_CALL_METHOD(&isautoincrement, column, "isautoincrement");

	if (Z_TYPE(column_type) == IS_STRING) {
		ZVAL_DUP(&column_sql, &column_type);
		zval_ptr_dtor(&column_type);
		PHALCON_CALL_METHOD(&type_values, column, "gettypevalues");
		if (PHALCON_IS_NOT_EMPTY(&type_values)) {
			zval slash = {};
			ZVAL_STRING(&slash, "\"");
			if (Z_TYPE(type_values) == IS_ARRAY) {
				c = phalcon_fast_count_int(&type_values);
				phalcon_concat_self_str(&column_sql, SL("("));
				ZEND_HASH_FOREACH_VAL(Z_ARRVAL(type_values), value) {
					zval value_cslashes = {};
					i++;
					PHALCON_CALL_FUNCTION(&value_cslashes, "addcslashes", value, &slash);
					if (i < c) {
						PHALCON_SCONCAT_SVS(&column_sql, "\"", &value_cslashes, "\", ");
					} else {
						PHALCON_SCONCAT_SVS(&column_sql, "\"", &value_cslashes, "\"");
					}
					zval_ptr_dtor(&value_cslashes);
				} ZEND_HASH_FOREACH_END();
				phalcon_concat_self_str(&column_sql, SL(")"));
			} else {
				zval value_cslashes = {};
				PHALCON_CALL_FUNCTION(&value_cslashes, "addcslashes", &type_values, &slash);
				PHALCON_SCONCAT_SVS(&column_sql, "(\"", &value_cslashes, "\")");
				zval_ptr_dtor(&value_cslashes);
			}
			zval_ptr_dtor(&slash);
			RETURN_ZVAL(&column_sql, 0, 0);
		}

		PHALCON_CALL_METHOD(&column_type, column, "gettypereference");
		switch (phalcon_get_intval(&column_type)) {

			case PHALCON_DB_COLUMN_TYPE_INTEGER:
				break;

			case PHALCON_DB_COLUMN_TYPE_BIGINTEGER:
				break;

			case PHALCON_DB_COLUMN_TYPE_FLOAT:
				break;

			case PHALCON_DB_COLUMN_TYPE_DECIMAL:
			{
				zval scale = {};
				PHALCON_CALL_METHOD(&scale, column, "getscale");
				PHALCON_SCONCAT_SVSVS(&column_sql, "(", &size, ",", &scale, ")");
				zval_ptr_dtor(&scale);
				break;
			}

			case PHALCON_DB_COLUMN_TYPE_DATE:
				break;

			case PHALCON_DB_COLUMN_TYPE_VARCHAR:
				PHALCON_SCONCAT_SVS(&column_sql, "(", &size, ")");
				break;

			case PHALCON_DB_COLUMN_TYPE_DATETIME:
			case PHALCON_DB_COLUMN_TYPE_TIMESTAMP:
				break;

			case PHALCON_DB_COLUMN_TYPE_CHAR:
				PHALCON_SCONCAT_SVS(&column_sql, "(", &size, ")");
				break;

			case PHALCON_DB_COLUMN_TYPE_TEXT:
			case PHALCON_DB_COLUMN_TYPE_JSON:
			case PHALCON_DB_COLUMN_TYPE_JSONB:
				break;

			case PHALCON_DB_COLUMN_TYPE_BOOLEAN:
				PHALCON_SCONCAT_STR(&column_sql, "(1)");
				break;

			default:
				PHALCON_CALL_METHOD(&name, column, "getname");
				PHALCON_THROW_EXCEPTION_FORMAT(phalcon_db_exception_ce, "Unrecognized PostgreSQL data type at column %s", Z_STRVAL(name));
		}
		RETURN_ZVAL(&column_sql, 0, 0);
	}

	switch (phalcon_get_intval(&column_type)) {

		case PHALCON_DB_COLUMN_TYPE_INTEGER:
			if (zend_is_true(&isautoincrement)) {
				ZVAL_STRING(&column_sql, "SERIAL");
			} else {
				ZVAL_STRING(&column_sql, "INT");
			}
			break;

		case PHALCON_DB_COLUMN_TYPE_BIGINTEGER:
			if (zend_is_true(&isautoincrement)) {
					ZVAL_STRING(&column_sql, "BIGSERIAL");
			} else {
				ZVAL_STRING(&column_sql, "BIGINT");
			}
			break;

		case PHALCON_DB_COLUMN_TYPE_FLOAT:
			ZVAL_STRING(&column_sql, "FLOAT");
			break;

		case PHALCON_DB_COLUMN_TYPE_DECIMAL:
		{
			zval scale = {};
			PHALCON_CALL_METHOD(&scale, column, "getscale");
			PHALCON_CONCAT_SVSVS(&column_sql, "NUMERIC(", &size, ",", &scale, ")");
			zval_ptr_dtor(&scale);
			break;
		}

		case PHALCON_DB_COLUMN_TYPE_DATE:
			ZVAL_STRING(&column_sql, "DATE");
			break;

		case PHALCON_DB_COLUMN_TYPE_VARCHAR:
			PHALCON_CONCAT_SVS(&column_sql, "CHARACTER VARYING(", &size, ")");
			break;

		case PHALCON_DB_COLUMN_TYPE_DATETIME:
		case PHALCON_DB_COLUMN_TYPE_TIMESTAMP:
			ZVAL_STRING(&column_sql, "TIMESTAMP");
			break;

		case PHALCON_DB_COLUMN_TYPE_CHAR:
			PHALCON_CONCAT_SVS(&column_sql, "CHARACTER(", &size, ")");
			break;

		case PHALCON_DB_COLUMN_TYPE_TEXT:
			ZVAL_STRING(&column_sql, "TEXT");
			break;

		case PHALCON_DB_COLUMN_TYPE_JSON:
			ZVAL_STRING(&column_sql, "JSON");
			break;

		case PHALCON_DB_COLUMN_TYPE_JSONB:
			ZVAL_STRING(&column_sql, "JSONB");
			break;

		case PHALCON_DB_COLUMN_TYPE_BOOLEAN:
			ZVAL_STRING(&column_sql, "SMALLINT(1)");
			break;

		default:
			PHALCON_CALL_METHOD(&name, column, "getname");
			PHALCON_THROW_EXCEPTION_FORMAT(phalcon_db_exception_ce, "Unrecognized PostgreSQL data type at column %s", Z_STRVAL(name));
			zval_ptr_dtor(&name);
			return;
	}

	RETURN_ZVAL(&column_sql, 0, 0);
}

/**
 * Generates SQL to add a column to a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\ColumnInterface $column
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, addColumn){

	zval *table_name, *schema_name, *column, table = {}, sql = {}, name = {}, column_definition = {}, column_type = {}, default_value = {}, is_not_null = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &column);
	PHALCON_VERIFY_INTERFACE_EX(column, phalcon_db_columninterface_ce, phalcon_db_exception_ce);

	PHALCON_CALL_METHOD(&table, getThis(), "preparetable", table_name, schema_name);
	PHALCON_CONCAT_SVS(&sql, "ALTER TABLE ", &table, " ADD COLUMN ");

	PHALCON_CALL_METHOD(&name, column, "getname");
	PHALCON_CALL_METHOD(&column_definition, getThis(), "getcolumndefinition", column);

	PHALCON_SCONCAT_SVSV(&sql, "\"", &name, "\" ", &column_definition);
	zval_ptr_dtor(&name);
	zval_ptr_dtor(&column_definition);

	PHALCON_CALL_METHOD(&default_value, column, "getdefaultvalue");
	if (Z_TYPE(default_value) != IS_NULL) {
		PHALCON_CALL_METHOD(&column_type, column, "gettype");
		PHALCON_CALL_METHOD(&default_value, getThis(), "getdefaultvalue", &default_value, &column_type);
		PHALCON_SCONCAT_SV(&sql, " DEFAULT ", &default_value);
	}
	zval_ptr_dtor(&default_value);

	PHALCON_CALL_METHOD(&is_not_null, column, "isnotnull");
	if (zend_is_true(&is_not_null)) {
		phalcon_concat_self_str(&sql, SL(" NOT NULL"));
	}

	RETURN_ZVAL(&sql, 0, 0);
}

/**
 * Generates SQL to modify a column in a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\ColumnInterface $column
 * @param Phalcon\Db\ColumnInterface $currentColumn
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, modifyColumn){

	zval *table_name, *schema_name, *column, *current_column = NULL, table = {}, alter_table = {}, sql = {}, name = {}, current_name = {};
	zval column_definition = {}, column_type ={}, current_type = {}, is_not_null = {}, current_is_not_null = {}, default_value = {};
	zval current_default_value = {};

	phalcon_fetch_params(0, 3, 1, &table_name, &schema_name, &column, &current_column);
	PHALCON_VERIFY_INTERFACE_EX(column, phalcon_db_columninterface_ce, phalcon_db_exception_ce);

	PHALCON_CALL_METHOD(&table, getThis(), "preparetable", table_name, schema_name);
	PHALCON_CONCAT_SV(&alter_table, "ALTER TABLE ", &table);

	PHALCON_CALL_METHOD(&name, column, "getname");
	PHALCON_CALL_METHOD(&column_definition, getThis(), "getcolumndefinition", column);
	PHALCON_CALL_METHOD(&column_type, column, "gettype");

	if (!current_column) {
		PHALCON_SCONCAT_VSVSVS(&sql, &alter_table, " ALTER COLUMN \"", &name, "\" TYPE ", &column_definition, ";");

		PHALCON_CALL_METHOD(&is_not_null, column, "isnotnull");
		if (zend_is_true(&is_not_null)) {
			PHALCON_SCONCAT_VSVS(&sql, &alter_table, " ALTER COLUMN \"", &name, "\" SET NOT NULL;");
		} else {
			PHALCON_SCONCAT_VSVS(&sql, &alter_table, " ALTER COLUMN \"", &name, "\" DROP NOT NULL;");
		}

		PHALCON_CALL_METHOD(&default_value, column, "getdefaultvalue");
		if (Z_TYPE(default_value) == IS_NULL) {
			PHALCON_SCONCAT_VSVS(&sql, &alter_table, " ALTER COLUMN \"", &name, "\" DROP DEFAULT;");
		} else {
			PHALCON_CALL_METHOD(&default_value, getThis(), "getdefaultvalue", &default_value, &column_type);
			PHALCON_SCONCAT_VSVSV(&sql, &alter_table, " ALTER COLUMN \"", &name, "\" SET DEFAULT ", &default_value);
		}
		RETURN_ZVAL(&sql, 0, 0);
	}

	PHALCON_VERIFY_INTERFACE_EX(current_column, phalcon_db_columninterface_ce, phalcon_db_exception_ce);
	PHALCON_CALL_METHOD(&current_name, current_column, "getname");

	if (!PHALCON_IS_EQUAL(&name, &current_name)) {
		PHALCON_CONCAT_VSVSVS(&sql, &alter_table, " RENAME COLUMN \"", &current_name, "\" TO \"", &name, "\";");
	}

	PHALCON_CALL_METHOD(&current_type, current_column, "gettype");

	if (!PHALCON_IS_EQUAL(&column_type, &current_type)) {
		PHALCON_SCONCAT_VSVSVS(&sql, &alter_table, " ALTER COLUMN \"", &name, "\" TYPE ", &column_definition, ";");
	}

	PHALCON_CALL_METHOD(&is_not_null, column, "isnotnull");
	PHALCON_CALL_METHOD(&current_is_not_null, current_column, "isnotnull");
	if (!PHALCON_IS_EQUAL(&is_not_null, &current_is_not_null)) {
		if (zend_is_true(&is_not_null)) {
			PHALCON_SCONCAT_VSVS(&sql, &alter_table, " ALTER COLUMN \"", &name, "\" SET NOT NULL;");
		} else {
			PHALCON_SCONCAT_VSVS(&sql, &alter_table, " ALTER COLUMN \"", &name, "\" DROP NOT NULL;");
		}
	}

	PHALCON_CALL_METHOD(&default_value, column, "getdefaultvalue");
	PHALCON_CALL_METHOD(&current_default_value, current_column, "getdefaultvalue");
	if (!PHALCON_IS_EQUAL(&default_value, &current_default_value)) {
		if (Z_TYPE(default_value) == IS_NULL) {
			PHALCON_SCONCAT_VSVS(&sql, &alter_table, " ALTER COLUMN \"", &name, "\" DROP DEFAULT;");
		} else {
			PHALCON_CALL_METHOD(&column_type, column, "gettype");
			PHALCON_CALL_METHOD(&default_value, getThis(), "getdefaultvalue", &default_value, &column_type);
			PHALCON_SCONCAT_VSVSV(&sql, &alter_table, " ALTER COLUMN \"", &name, "\" SET DEFAULT ", &default_value);
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
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, dropColumn){

	zval *table_name, *schema_name, *column_name, table = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &column_name);

	PHALCON_CALL_METHOD(&table, getThis(), "preparetable", table_name, schema_name);
	PHALCON_CONCAT_SVSVS(return_value, "ALTER TABLE ", &table, " DROP COLUMN \"", column_name, "\"");
}

/**
 * Generates SQL to add an index to a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\Index $index
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, addIndex){

	zval *table_name, *schema_name, *index, name = {}, table = {}, type = {}, columns = {}, columnlist = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &index);

	PHALCON_CALL_METHOD(&name, index, "getname");
	if (phalcon_comparestr_str(&name, SL("PRIMARY"), &PHALCON_GLOBAL(z_false))) {
		PHALCON_RETURN_CALL_METHOD(getThis(), "addprimarykey", table_name, schema_name, index);
		return;
	}

	PHALCON_CALL_METHOD(&table, getThis(), "preparetable", table_name, schema_name);
	PHALCON_CALL_METHOD(&type, index, "gettype");
	PHALCON_CALL_METHOD(&columns, index, "getcolumns");
	PHALCON_CALL_METHOD(&columnlist, getThis(), "getcolumnlist", &columns);

	if (PHALCON_IS_NOT_EMPTY(&type)) {
		PHALCON_CONCAT_SVSVSVSVS(return_value, "CREATE ", &type, " INDEX \"", &name, "\" ON ", &table, " (", &columnlist, ")");
	} else {
		PHALCON_CONCAT_SVSVSVS(return_value, "CREATE INDEX \"", &name, "\" ON ", &table, " (", &columnlist, ")");
	}
}

/**
 * Generates SQL to delete an index from a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param string $indexName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, dropIndex){

	zval *table_name, *schema_name, *index_name;

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &index_name);

	PHALCON_CONCAT_SVS(return_value, "DROP INDEX \"", index_name, "\"");
}

/**
 * Generates SQL to add the primary key to a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\Index $index
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, addPrimaryKey){

	zval *table_name, *schema_name, *index, table = {}, columns = {}, columnlist = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &index);
	PHALCON_VERIFY_INTERFACE_EX(index, phalcon_db_indexinterface_ce, phalcon_db_exception_ce);

	PHALCON_CALL_METHOD(&table, getThis(), "preparetable", table_name, schema_name);
	PHALCON_CALL_METHOD(&columns, index, "getcolumns");
	PHALCON_CALL_METHOD(&columnlist, getThis(), "getcolumnlist", &columns);
	PHALCON_CONCAT_SVSVS(return_value, "ALTER TABLE ", &table, " ADD CONSTRAINT \"PRIMARY\" PRIMARY KEY (", &columnlist, ")");
}

/**
 * Generates SQL to delete primary key from a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, dropPrimaryKey){

	zval *table_name, *schema_name, table = {};

	phalcon_fetch_params(0, 2, 0, &table_name, &schema_name);

	PHALCON_CALL_METHOD(&table, getThis(), "preparetable", table_name, schema_name);

	PHALCON_CONCAT_SVS(return_value, "ALTER TABLE ", &table, " DROP CONSTRAINT \"PRIMARY\"");
}

/**
 * Generates SQL to add an index to a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\ReferenceInterface $reference
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, addForeignKey){

	zval *table_name, *schema_name, *reference, table = {}, name = {}, columns = {}, columnlist = {}, referenced_table = {}, referenced_columns = {};
	zval referenced_columnlist = {}, on_delete = {}, on_udpate = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &reference);
	PHALCON_VERIFY_INTERFACE_EX(reference, phalcon_db_referenceinterface_ce, phalcon_db_exception_ce);

	PHALCON_CALL_METHOD(&table, getThis(), "preparetable", table_name, schema_name);

	PHALCON_CALL_METHOD(&name, reference, "getname");
	PHALCON_CALL_METHOD(&columns, reference, "getcolumns");
	PHALCON_CALL_METHOD(&columnlist, getThis(), "getcolumnlist", &columns);
	PHALCON_CALL_METHOD(&referenced_table, reference, "getreferencedtable", &columns);
	PHALCON_CALL_METHOD(&referenced_columns, reference, "getreferencedcolumns");
	PHALCON_CALL_METHOD(&referenced_columnlist, getThis(), "getcolumnlist", &referenced_columns);
	PHALCON_CALL_METHOD(&on_delete, reference, "getondelete");
	PHALCON_CALL_METHOD(&on_udpate, reference, "getonupdate");

	PHALCON_CONCAT_SVSVSVS(return_value, "ALTER TABLE ", &table, " ADD CONSTRAINT \"", &name, "\" FOREIGN KEY (", &columnlist, ")");
	PHALCON_SCONCAT_SVSVS(return_value, " REFERENCES \"", &referenced_table, "\" (", &referenced_columnlist, ")");

	if (PHALCON_IS_NOT_EMPTY(&on_delete)) {
		PHALCON_SCONCAT_SV(return_value, " ON DELETE ", &on_delete);
	}

	if (PHALCON_IS_NOT_EMPTY(&on_udpate)) {
		PHALCON_SCONCAT_SV(return_value, " ON UPDATE ", &on_udpate);
	}
}

/**
 * Generates SQL to delete a foreign key from a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param string $referenceName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, dropForeignKey){

	zval *table_name, *schema_name, *reference_name, table = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &reference_name);

	PHALCON_CALL_METHOD(&table, getThis(), "preparetable", table_name, schema_name);

	PHALCON_CONCAT_SVSVS(return_value, "ALTER TABLE ", &table, " DROP CONSTRAINT \"", reference_name, "\"");
}

/**
 * Generates SQL to add the table creation options
 *
 * @param array $definition
 * @return array
 */
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, _getTableOptions){

	zval *definition;

	phalcon_fetch_params(0, 1, 0, &definition);

	RETURN_EMPTY_ARRAY();
}

/**
 * Generates SQL to create a table in PostgreSQL
 *
 * @param 	string $tableName
 * @param string $schemaName
 * @param array $definition
 * @return 	string
 */
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, createTable){

	zval *table_name, *schema_name, *definition, columns = {}, table = {}, options = {}, temporary = {}, sql = {}, create_lines = {}, slash = {};
	zval primary_columns = {}, *column, primary_column_list = {}, primary_sql = {}, indexes = {}, indexsql_aftercreate = {};
	zval *index, references, *reference, joined_lines = {};

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

	array_init(&primary_columns);
	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(&columns), column) {
		zval column_name = {}, column_definition = {}, column_line = {}, default_value = {}, column_type = {}, attribute = {};

		PHALCON_CALL_METHOD(&column_name, column, "getname");
		PHALCON_CALL_METHOD(&column_definition, getThis(), "getcolumndefinition", column);

		PHALCON_CONCAT_SVSV(&column_line, "\"", &column_name, "\" ", &column_definition);

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
		 * Mark the column as primary key
		 */
		PHALCON_CALL_METHOD(&attribute, column, "isprimary");
		if (zend_is_true(&attribute)) {
			phalcon_array_append(&primary_columns, &column_name, PH_COPY);
		}

		phalcon_array_append(&create_lines, &column_line, PH_COPY);
	} ZEND_HASH_FOREACH_END();

	if (phalcon_fast_count_ev(&primary_columns)) {
		PHALCON_CALL_METHOD(&primary_column_list, getThis(), "getcolumnlist", &primary_columns);
		PHALCON_CONCAT_SVS(&primary_sql, "PRIMARY KEY (", &primary_column_list, ")");
		phalcon_array_append(&create_lines, &primary_sql, PH_COPY);
	}

	/**
	 * Create related indexes
	 */
	ZVAL_NULL(&indexsql_aftercreate);
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
				PHALCON_CONCAT_SVS(&index_sql, "CONSTRAINT \"PRIMARY\" PRIMARY KEY (", &column_list, ")");
			} else if (PHALCON_IS_NOT_EMPTY(&index_type)) {
				PHALCON_CONCAT_SVSVSVS(&index_sql, "CONSTRAINT \"", &index_name, "\" ", &index_type, " (", &column_list, ")");
			} else {
				PHALCON_SCONCAT_SVSVSVS(&indexsql_aftercreate, "CREATE INDEX \"", &index_name, "\" ON ", &table, " (", &column_list, ");");
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

			PHALCON_CONCAT_SVSVS(&constaint_sql, "CONSTRAINT \"", &name, "\" FOREIGN KEY (", &column_list, ")");
			PHALCON_CONCAT_VSVSVS(&reference_sql, &constaint_sql, " REFERENCES \"", &referenced_table, "\" (", &referenced_column_list, ")");

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
	PHALCON_SCONCAT_SV(&sql, ";", &indexsql_aftercreate);

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
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, dropTable){

	zval *table_name, *schema_name, *if_exists = NULL, table = {}, sql = {};

	phalcon_fetch_params(0, 2, 1, &table_name, &schema_name, &if_exists);

	if (!if_exists) {
		if_exists = &PHALCON_GLOBAL(z_true);
	}

	PHALCON_CALL_METHOD(&table, getThis(), "preparetable", table_name, schema_name);

	if (zend_is_true(if_exists)) {
		PHALCON_CONCAT_SV(&sql, "DROP TABLE IF EXISTS ", &table);
	} else {
		PHALCON_CONCAT_SV(&sql, "DROP TABLE ", &table);
	}

	RETURN_CTOR(&sql);
}

/**
 * Generates SQL to create a view
 *
 * @param string $viewName
 * @param array $definition
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, createView){

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
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, dropView){

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
 * <code>echo $dialect->tableExists("posts", "blog")</code>
 * <code>echo $dialect->tableExists("posts")</code>
 *
 * @param string $tableName
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, tableExists){

	zval *table_name, *schema_name = NULL, sql = {};

	phalcon_fetch_params(0, 1, 1, &table_name, &schema_name);

	if (!schema_name) {
		schema_name = &PHALCON_GLOBAL(z_null);
	}

	if (zend_is_true(schema_name)) {
		PHALCON_CONCAT_SVSVS(&sql, "SELECT CASE WHEN COUNT(*) > 0 THEN 1 ELSE 0 END FROM information_schema.tables WHERE table_schema = '", schema_name, "' AND table_name='", table_name, "'");
	} else {
		PHALCON_CONCAT_SVS(&sql, "SELECT CASE WHEN COUNT(*) > 0 THEN 1 ELSE 0 END FROM information_schema.tables WHERE table_schema = 'public' AND table_name='", table_name, "'");
	}

	RETURN_CTOR(&sql);
}

/**
 * Generates SQL checking for the existence of a schema.view
 *
 * @param string $viewName
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, viewExists){

	zval *view_name, *schema_name = NULL;

	phalcon_fetch_params(0, 1, 1, &view_name, &schema_name);

	if (!schema_name) {
		schema_name = &PHALCON_GLOBAL(z_null);
	}

	if (zend_is_true(schema_name)) {
		PHALCON_CONCAT_SVSVS(return_value, "SELECT CASE WHEN COUNT(*) > 0 THEN 1 ELSE 0 END FROM pg_views WHERE viewname='", view_name, "' AND schemaname='", schema_name, "'");
	} else {
		PHALCON_CONCAT_SVS(return_value, "SELECT CASE WHEN COUNT(*) > 0 THEN 1 ELSE 0 END FROM pg_views WHERE viewname='", view_name, "'");
	}
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
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, describeColumns){

	zval *table, *schema = NULL, sql = {};

	phalcon_fetch_params(0, 1, 1, &table, &schema);

	if (!schema) {
		schema = &PHALCON_GLOBAL(z_null);
	}

	if (zend_is_true(schema)) {
		PHALCON_CONCAT_SVSVS(&sql, "SELECT DISTINCT c.column_name AS Field, c.data_type AS Type, c.character_maximum_length AS Size, c.numeric_precision AS NumericSize, c.numeric_scale AS NumericScale, c.is_nullable AS Null, CASE WHEN pkc.column_name NOTNULL THEN 'PRI' ELSE '' END AS Key, CASE WHEN c.data_type LIKE '%int%' AND c.column_default LIKE '%nextval%' THEN 'auto_increment' ELSE c.column_default END AS Extra, c.ordinal_position AS Position, e.data_type AS element_type FROM information_schema.columns c LEFT JOIN ( SELECT kcu.column_name, kcu.table_name, kcu.table_schema FROM information_schema.table_constraints tc INNER JOIN information_schema.key_column_usage kcu on (kcu.constraint_name = tc.constraint_name and kcu.table_name=tc.table_name and kcu.table_schema=tc.table_schema) WHERE tc.constraint_type='PRIMARY KEY') pkc ON (c.column_name=pkc.column_name AND c.table_schema = pkc.table_schema AND c.table_name=pkc.table_name) LEFT JOIN information_schema.element_types e ON ((c.table_catalog, c.table_schema, c.table_name, 'TABLE', c.dtd_identifier) = (e.object_catalog, e.object_schema, e.object_name, e.object_type, e.collection_type_identifier)) WHERE c.table_schema='", schema, "' AND c.table_name='", table, "' ORDER BY c.ordinal_position");
	} else {
		PHALCON_CONCAT_SVS(&sql, "SELECT DISTINCT c.column_name AS Field, c.data_type AS Type, c.character_maximum_length AS Size, c.numeric_precision AS NumericSize, c.numeric_scale AS NumericScale, c.is_nullable AS Null, CASE WHEN pkc.column_name NOTNULL THEN 'PRI' ELSE '' END AS Key, CASE WHEN c.data_type LIKE '%int%' AND c.column_default LIKE '%nextval%' THEN 'auto_increment' ELSE c.column_default END AS Extra, c.ordinal_position AS Position, e.data_type AS element_type FROM information_schema.columns c LEFT JOIN ( SELECT kcu.column_name, kcu.table_name, kcu.table_schema FROM information_schema.table_constraints tc INNER JOIN information_schema.key_column_usage kcu on (kcu.constraint_name = tc.constraint_name and kcu.table_name=tc.table_name and kcu.table_schema=tc.table_schema) WHERE tc.constraint_type='PRIMARY KEY') pkc ON (c.column_name=pkc.column_name AND c.table_schema = pkc.table_schema AND c.table_name=pkc.table_name) LEFT JOIN information_schema.element_types e ON ((c.table_catalog, c.table_schema, c.table_name, 'TABLE', c.dtd_identifier) = (e.object_catalog, e.object_schema, e.object_name, e.object_type, e.collection_type_identifier)) WHERE c.table_schema='public' AND c.table_name='", table, "' ORDER BY c.ordinal_position");
	}

	RETURN_ZVAL(&sql, 0, 0);
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
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, listTables){

	zval *schema_name = NULL, sql = {};

	phalcon_fetch_params(0, 0, 1, &schema_name);

	if (!schema_name) {
		schema_name = &PHALCON_GLOBAL(z_null);
	}

	if (zend_is_true(schema_name)) {
		PHALCON_CONCAT_SVS(&sql, "SELECT table_name FROM information_schema.tables WHERE table_schema = '", schema_name, "' ORDER BY table_name");
	} else {
		ZVAL_STRING(&sql, "SELECT table_name FROM information_schema.tables WHERE table_schema = 'public' ORDER BY table_name");
	}

	RETURN_ZVAL(&sql, 0, 0);
}

/**
 * Generates the SQL to list all views of a schema or user
 *
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, listViews){

	zval *schema_name = NULL, sql = {};

	phalcon_fetch_params(0, 0, 1, &schema_name);

	if (!schema_name) {
		schema_name = &PHALCON_GLOBAL(z_null);
	}

	if (zend_is_true(schema_name)) {
		PHALCON_CONCAT_SVS(&sql, "SELECT viewname AS view_name FROM pg_views WHERE schemaname = '", schema_name, "' ORDER BY view_name");
	} else {
		ZVAL_STRING(&sql, "SELECT viewname AS view_name FROM pg_views WHERE schemaname = 'public' ORDER BY view_name");
	}

	RETURN_ZVAL(&sql, 0, 0);
}

/**
 * Generates SQL to query indexes on a table
 *
 * @param string $table
 * @param string $schema
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, describeIndexes){

	zval *table, *schema = NULL, sql = {};

	phalcon_fetch_params(0, 1, 1, &table, &schema);

	if (!schema) {
		schema = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CONCAT_SVS(&sql, "SELECT 0 as c0, t.relname as table_name, i.relname as key_name, 3 as c3, a.attname as column_name FROM pg_class t, pg_class i, pg_index ix, pg_attribute a WHERE t.oid = ix.indrelid AND i.oid = ix.indexrelid AND a.attrelid = t.oid AND a.attnum = ANY(ix.indkey) AND t.relkind = 'r' AND t.relname = '", table, "' ORDER BY t.relname, i.relname;");
	RETURN_ZVAL(&sql, 0, 0);
}

/**
 * Generates SQL to query foreign keys on a table
 *
 * @param string $table
 * @param string $schema
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, describeReferences){

	zval *table, *schema = NULL, sql = {};

	phalcon_fetch_params(0, 1, 1, &table, &schema);

	if (!schema) {
		schema = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STRING(&sql, "SELECT tc.table_name as TABLE_NAME, kcu.column_name as COLUMN_NAME, tc.constraint_name as CONSTRAINT_NAME, tc.table_catalog as REFERENCED_TABLE_SCHEMA, ccu.table_name AS REFERENCED_TABLE_NAME, ccu.column_name AS REFERENCED_COLUMN_NAME FROM information_schema.table_constraints AS tc JOIN information_schema.key_column_usage AS kcu ON tc.constraint_name = kcu.constraint_name JOIN information_schema.constraint_column_usage AS ccu ON ccu.constraint_name = tc.constraint_name WHERE constraint_type = 'FOREIGN KEY' AND ");
	if (zend_is_true(schema)) {
		PHALCON_SCONCAT_SVSVS(&sql, "tc.table_schema = '", schema, "' AND tc.table_name='", table, "'");
	} else {
		PHALCON_SCONCAT_SVS(&sql, "tc.table_name='", table, "'");
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
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, tableOptions){

	zval *table, *schema = NULL;

	phalcon_fetch_params(0, 1, 1, &table, &schema);

	if (!schema) {
		schema = &PHALCON_GLOBAL(z_null);
	}

	RETURN_EMPTY_STRING();
}

/**
 * Return the default value
 *
 * @param string $defaultValue
 * @param string $columnDefinition
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Postgresql, getDefaultValue){

	zval *default_value, *column_type, slash = {}, value_cslashes = {};
	int type;

	phalcon_fetch_params(0, 2, 0, &default_value, &column_type);

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
