
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

#include "db/column.h"
#include "db/columninterface.h"
#include "db/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/exception.h"
#include "kernel/operators.h"
#include "kernel/fcall.h"

#include "internal/arginfo.h"

/**
 * Phalcon\Db\Column
 *
 * Allows to define columns to be used on create or alter table operations
 *
 *<code>
 *	use Phalcon\Db\Column as Column;
 *
 * //column definition
 * $column = new Column("id", array(
 *   "type" => Column::TYPE_INTEGER,
 *   "size" => 10,
 *   "unsigned" => true,
 *   "notNull" => true,
 *   "autoIncrement" => true,
 *   "first" => true
 * ));
 *
 * //add column to existing table
 * $connection->addColumn("robots", null, $column);
 *</code>
 *
 */
zend_class_entry *phalcon_db_column_ce;

PHP_METHOD(Phalcon_Db_Column, __construct);
PHP_METHOD(Phalcon_Db_Column, getSchemaName);
PHP_METHOD(Phalcon_Db_Column, getName);
PHP_METHOD(Phalcon_Db_Column, getType);
PHP_METHOD(Phalcon_Db_Column, getTypeReference);
PHP_METHOD(Phalcon_Db_Column, getTypeValues);
PHP_METHOD(Phalcon_Db_Column, getSize);
PHP_METHOD(Phalcon_Db_Column, getBytes);
PHP_METHOD(Phalcon_Db_Column, getScale);
PHP_METHOD(Phalcon_Db_Column, isUnsigned);
PHP_METHOD(Phalcon_Db_Column, isNotNull);
PHP_METHOD(Phalcon_Db_Column, isPrimary);
PHP_METHOD(Phalcon_Db_Column, isAutoIncrement);
PHP_METHOD(Phalcon_Db_Column, isNumeric);
PHP_METHOD(Phalcon_Db_Column, isFirst);
PHP_METHOD(Phalcon_Db_Column, getAfterPosition);
PHP_METHOD(Phalcon_Db_Column, getBindType);
PHP_METHOD(Phalcon_Db_Column, getDefaultValue);
PHP_METHOD(Phalcon_Db_Column, __set_state);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_column___construct, 0, 0, 2)
	ZEND_ARG_INFO(0, columnName)
	ZEND_ARG_INFO(0, definition)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_db_column_method_entry[] = {
	PHP_ME(Phalcon_Db_Column, __construct, arginfo_phalcon_db_column___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Db_Column, getSchemaName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Column, getName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Column, getType, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Column, getTypeReference, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Column, getTypeValues, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Column, getSize, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Column, getBytes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Column, getScale, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Column, isUnsigned, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Column, isNotNull, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Column, isPrimary, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Column, isAutoIncrement, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Column, isNumeric, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Column, isFirst, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Column, getAfterPosition, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Column, getBindType, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Column, getDefaultValue, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Column, __set_state, arginfo___set_state, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_MALIAS(Phalcon_Db_Column, getDefault, getDefaultValue, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Db\Column initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Column){

	PHALCON_REGISTER_CLASS(Phalcon\\Db, Column, db_column, phalcon_db_column_method_entry, 0);

	zend_declare_property_null(phalcon_db_column_ce, SL("_columnName"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_column_ce, SL("_schemaName"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_column_ce, SL("_type"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_db_column_ce, SL("_typeReference"), -1, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_column_ce, SL("_typeValues"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_db_column_ce, SL("_isNumeric"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_column_ce, SL("_size"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_column_ce, SL("_bytes"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_column_ce, SL("_scale"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_db_column_ce, SL("_unsigned"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_db_column_ce, SL("_notNull"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_db_column_ce, SL("_primary"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_db_column_ce, SL("_autoIncrement"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_db_column_ce, SL("_first"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_column_ce, SL("_after"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_db_column_ce, SL("_bindType"), 2, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_column_ce, SL("_default"), ZEND_ACC_PROTECTED);

	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_INTEGER"), PHALCON_DB_COLUMN_TYPE_INTEGER);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_BIGINTEGER"), PHALCON_DB_COLUMN_TYPE_BIGINTEGER);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_DATE"), PHALCON_DB_COLUMN_TYPE_DATE);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_VARCHAR"), PHALCON_DB_COLUMN_TYPE_VARCHAR);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_DECIMAL"), PHALCON_DB_COLUMN_TYPE_DECIMAL);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_DATETIME"), PHALCON_DB_COLUMN_TYPE_DATETIME);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_CHAR"), PHALCON_DB_COLUMN_TYPE_CHAR);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_TEXT"), PHALCON_DB_COLUMN_TYPE_TEXT);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_FLOAT"), PHALCON_DB_COLUMN_TYPE_FLOAT);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_BOOLEAN"), PHALCON_DB_COLUMN_TYPE_BOOLEAN);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_DOUBLE"), PHALCON_DB_COLUMN_TYPE_DOUBLE);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_TINYBLOB"), PHALCON_DB_COLUMN_TYPE_TINYBLOB);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_BLOB"), PHALCON_DB_COLUMN_TYPE_BLOB);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_MEDIUMBLOB"), PHALCON_DB_COLUMN_TYPE_MEDIUMBLOB);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_LONGBLOB"), PHALCON_DB_COLUMN_TYPE_LONGBLOB);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_JSON"), PHALCON_DB_COLUMN_TYPE_JSON);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_JSONB"), PHALCON_DB_COLUMN_TYPE_JSONB);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_ARRAY"), PHALCON_DB_COLUMN_TYPE_ARRAY);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_INT_ARRAY"), PHALCON_DB_COLUMN_TYPE_INT_ARRAY);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_TIMESTAMP"), PHALCON_DB_COLUMN_TYPE_TIMESTAMP);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_BYTEA"), PHALCON_DB_COLUMN_TYPE_BYTEA);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_MONEY"), PHALCON_DB_COLUMN_TYPE_MONEY);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("TYPE_OTHER"), PHALCON_DB_COLUMN_TYPE_OTHER);

	zend_declare_class_constant_long(phalcon_db_column_ce, SL("BIND_PARAM_NULL"), PHALCON_DB_COLUMN_BIND_PARAM_NULL);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("BIND_PARAM_INT"), PHALCON_DB_COLUMN_BIND_PARAM_INT);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("BIND_PARAM_STR"), PHALCON_DB_COLUMN_BIND_PARAM_STR);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("BIND_PARAM_BOOL"), PHALCON_DB_COLUMN_BIND_PARAM_BOOL);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("BIND_PARAM_DECIMAL"), PHALCON_DB_COLUMN_BIND_PARAM_DECIMAL);
	zend_declare_class_constant_long(phalcon_db_column_ce, SL("BIND_SKIP"), PHALCON_DB_COLUMN_BIND_SKIP);

	zend_class_implements(phalcon_db_column_ce, 1, phalcon_db_columninterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Db\Column constructor
 *
 * @param string $columnName
 * @param array $definition
 */
PHP_METHOD(Phalcon_Db_Column, __construct){

	zval *column_name, *definition, type = {}, type_reference = {}, type_values = {}, not_null = {}, primary = {}, size = {}, bytes = {}, scale = {}, dunsigned = {}, is_numeric = {};
	zval auto_increment = {}, first = {}, after = {}, bind_type = {}, default_value = {};

	phalcon_fetch_params(0, 2, 0, &column_name, &definition);

	phalcon_update_property(getThis(), SL("_columnName"), column_name);

	/**
	 * Get the column type, one of the TYPE_* constants
	 */
	if (phalcon_array_isset_fetch_str(&type, definition, SL("type"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_type"), &type);
	} else {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Column type is required");
		return;
	}

	if (phalcon_array_isset_fetch_str(&type_reference, definition, SL("typeReference"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_typeReference"), &type_reference);
	}

	if (phalcon_array_isset_fetch_str(&type_values, definition, SL("typeValues"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_typeValues"), &type_values);
	}

	/**
	 * Check if the field is nullable
	 */
	if (phalcon_array_isset_fetch_str(&not_null, definition, SL("notNull"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_notNull"), &not_null);
	}

	/**
	 * Check if the field is primary key
	 */
	if (phalcon_array_isset_fetch_str(&primary, definition, SL("primary"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_primary"), &primary);
	}

	if (phalcon_array_isset_fetch_str(&size, definition, SL("size"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_size"), &size);
		phalcon_update_property(getThis(), SL("_bytes"), &size);
	}

	if (phalcon_array_isset_fetch_str(&bytes, definition, SL("bytes"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_bytes"), &bytes);
	}

	/**
	 * Check if the column has a decimal scale
	 */
	if (phalcon_array_isset_fetch_str(&scale, definition, SL("scale"), PH_READONLY)) {
		int i_type     = phalcon_get_intval(&type);
		int is_numeric = (i_type == PHALCON_DB_COLUMN_TYPE_DECIMAL || i_type == PHALCON_DB_COLUMN_TYPE_DOUBLE || i_type == PHALCON_DB_COLUMN_TYPE_FLOAT);

		if (is_numeric) {
			phalcon_update_property(getThis(), SL("_scale"), &scale);
		}
	}

	/**
	 * Check if the field is unsigned (only MySQL)
	 */
	if (phalcon_array_isset_fetch_str(&dunsigned, definition, SL("unsigned"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_unsigned"), &dunsigned);
	}

	/**
	 * Check if the field is numeric
	 */
	if (phalcon_array_isset_fetch_str(&is_numeric, definition, SL("isNumeric"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_isNumeric"), &is_numeric);
	}

	/**
	 * Check if the field is auto-increment/serial
	 */
	if (phalcon_array_isset_fetch_str(&auto_increment, definition, SL("autoIncrement"), PH_READONLY)) {
		if (PHALCON_LE_LONG(&type, PHALCON_DB_COLUMN_TYPE_BIGINTEGER)) {
			phalcon_update_property(getThis(), SL("_autoIncrement"), &auto_increment);
		} else {
			PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Column type cannot be auto-increment");
			return;
		}
	}

	/**
	 * Check if the field is placed at the first position of the table
	 */
	if (phalcon_array_isset_fetch_str(&first, definition, SL("first"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_first"), &first);
	}

	/**
	 * Name of the column which is placed before the current field
	 */
	if (phalcon_array_isset_fetch_str(&after, definition, SL("after"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_after"), &after);
	}

	/**
	 * The bind type to cast the field when passing it to PDO
	 */
	if (phalcon_array_isset_fetch_str(&bind_type, definition, SL("bindType"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_bindType"), &bind_type);
	}

	/**
	 * Default values
	 */
	if (phalcon_array_isset_fetch_str(&default_value, definition, SL("default"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_default"), &default_value);
	}
}

/**
 * Returns schema's table related to column
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Column, getSchemaName){


	RETURN_MEMBER(getThis(), "_schemaName");
}

/**
 * Returns column name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Column, getName){


	RETURN_MEMBER(getThis(), "_columnName");
}

/**
 * Returns column type
 *
 * @return int
 */
PHP_METHOD(Phalcon_Db_Column, getType){


	RETURN_MEMBER(getThis(), "_type");
}

/**
 * Returns column type reference
 *
 * @return int
 */
PHP_METHOD(Phalcon_Db_Column, getTypeReference){


	RETURN_MEMBER(getThis(), "_typeReference");
}

/**
 * Returns column type values
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Column, getTypeValues){


	RETURN_MEMBER(getThis(), "_typeValues");
}

/**
 * Returns column size
 *
 * @return int
 */
PHP_METHOD(Phalcon_Db_Column, getSize){


	RETURN_MEMBER(getThis(), "_size");
}

/**
 * Returns column bytes
 *
 * @return int
 */
PHP_METHOD(Phalcon_Db_Column, getBytes){


	RETURN_MEMBER(getThis(), "_bytes");
}

/**
 * Returns column scale
 *
 * @return int
 */
PHP_METHOD(Phalcon_Db_Column, getScale){


	RETURN_MEMBER(getThis(), "_scale");
}

/**
 * Returns true if number column is unsigned
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Column, isUnsigned){


	RETURN_MEMBER(getThis(), "_unsigned");
}

/**
 * Not null
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Column, isNotNull){


	RETURN_MEMBER(getThis(), "_notNull");
}

/**
 * Column is part of the primary key?
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Column, isPrimary){


	RETURN_MEMBER(getThis(), "_primary");
}

/**
 * Auto-Increment
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Column, isAutoIncrement){


	RETURN_MEMBER(getThis(), "_autoIncrement");
}

/**
 * Check whether column have an numeric type
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Column, isNumeric){


	RETURN_MEMBER(getThis(), "_isNumeric");
}

/**
 * Check whether column have first position in table
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Column, isFirst){


	RETURN_MEMBER(getThis(), "_first");
}

/**
 * Check whether field absolute to position in table
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Column, getAfterPosition){


	RETURN_MEMBER(getThis(), "_after");
}

/**
 * Returns the type of bind handling
 *
 * @return int
 */
PHP_METHOD(Phalcon_Db_Column, getBindType){


	RETURN_MEMBER(getThis(), "_bindType");
}

/**
 * Returns the field default values
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Column, getDefaultValue){


	RETURN_MEMBER(getThis(), "_default");
}

/**
 * Restores the internal state of a Phalcon\Db\Column object
 *
 * @param array $data
 * @return \Phalcon\Db\Column
 */
PHP_METHOD(Phalcon_Db_Column, __set_state){

	zval *data, definition = {}, column_name = {}, column_type = {}, not_null = {}, primary = {}, size = {}, bytes = {}, scale = {}, dunsigned = {}, after = {}, is_numeric = {}, first = {}, bind_type = {}, default_value = {};

	phalcon_fetch_params(0, 1, 0, &data);

	if (Z_TYPE_P(data) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Column state must be an array");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&column_name, data, SL("_columnName"), PH_READONLY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Column name is required");
		return;
	}

	array_init(&definition);

	if (phalcon_array_isset_fetch_str(&column_type, data, SL("_type"), PH_READONLY)) {
		phalcon_array_update_str(&definition, SL("type"), &column_type, PH_COPY);
	}

	if (phalcon_array_isset_fetch_str(&not_null, data, SL("_notNull"), PH_READONLY)) {
		phalcon_array_update_str(&definition, SL("notNull"), &not_null, PH_COPY);
	}

	if (phalcon_array_isset_fetch_str(&primary, data, SL("_primary"), PH_READONLY)) {
		phalcon_array_update_str(&definition, SL("primary"), &primary, PH_COPY);
	}

	if (phalcon_array_isset_fetch_str(&size, data, SL("_size"), PH_READONLY)) {
		phalcon_array_update_str(&definition, SL("size"), &size, PH_COPY);
		phalcon_array_update_str(&definition, SL("bytes"), &size, PH_COPY);
	}

	if (phalcon_array_isset_fetch_str(&bytes, data, SL("_bytes"), PH_READONLY)) {
		phalcon_array_update_str(&definition, SL("bytes"), &bytes, PH_COPY);
	}

	if (phalcon_array_isset_fetch_str(&scale, data, SL("_scale"), PH_READONLY)) {
		phalcon_array_update_str(&definition, SL("scale"), &scale, PH_COPY);
	}

	if (phalcon_array_isset_fetch_str(&dunsigned, data, SL("_unsigned"), PH_READONLY)) {
		phalcon_array_update_str(&definition, SL("unsigned"), &dunsigned, PH_COPY);
	}

	if (phalcon_array_isset_fetch_str(&after, data, SL("_after"), PH_READONLY)) {
		phalcon_array_update_str(&definition, SL("after"), &after, PH_COPY);
	}

	if (phalcon_array_isset_fetch_str(&is_numeric, data, SL("_isNumeric"), PH_READONLY)) {
		phalcon_array_update_str(&definition, SL("isNumeric"), &is_numeric, PH_COPY);
	}

	if (phalcon_array_isset_fetch_str(&first, data, SL("_first"), PH_READONLY)) {
		phalcon_array_update_str(&definition, SL("first"), &first, PH_COPY);
	}

	if (phalcon_array_isset_fetch_str(&bind_type, data, SL("_bindType"), PH_READONLY)) {
		phalcon_array_update_str(&definition, SL("bindType"), &bind_type, PH_COPY);
	}

	if (phalcon_array_isset_fetch_str(&default_value, data, SL("_default"), PH_READONLY)) {
		phalcon_array_update_str(&definition, SL("default"), &default_value, PH_COPY);
	}

	object_init_ex(return_value, phalcon_db_column_ce);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", &column_name, &definition);
}
