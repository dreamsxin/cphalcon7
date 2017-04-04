
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

#include "db/reference.h"
#include "db/referenceinterface.h"
#include "db/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"

#include "internal/arginfo.h"

/**
 * Phalcon\Db\Reference
 *
 * Allows to define reference constraints on tables
 *
 *<code>
 *	$reference = new Phalcon\Db\Reference("field_fk", array(
 *		'referencedSchema' => "invoicing",
 *		'referencedTable' => "products",
 *		'columns' => array("product_type", "product_code"),
 *		'referencedColumns' => array("type", "code")
 *	));
 *</code>
 */
zend_class_entry *phalcon_db_reference_ce;

PHP_METHOD(Phalcon_Db_Reference, __construct);
PHP_METHOD(Phalcon_Db_Reference, getName);
PHP_METHOD(Phalcon_Db_Reference, getSchemaName);
PHP_METHOD(Phalcon_Db_Reference, getReferencedSchema);
PHP_METHOD(Phalcon_Db_Reference, getColumns);
PHP_METHOD(Phalcon_Db_Reference, getReferencedTable);
PHP_METHOD(Phalcon_Db_Reference, getReferencedColumns);
PHP_METHOD(Phalcon_Db_Reference, getOnDelete);
PHP_METHOD(Phalcon_Db_Reference, getOnUpdate);
PHP_METHOD(Phalcon_Db_Reference, __set_state);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_reference___construct, 0, 0, 2)
	ZEND_ARG_INFO(0, referenceName)
	ZEND_ARG_INFO(0, definition)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_db_reference_method_entry[] = {
	PHP_ME(Phalcon_Db_Reference, __construct, arginfo_phalcon_db_reference___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Db_Reference, getName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Reference, getSchemaName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Reference, getReferencedSchema, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Reference, getColumns, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Reference, getReferencedTable, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Reference, getReferencedColumns, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Reference, getOnDelete, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Reference, getOnUpdate, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Reference, __set_state, arginfo___set_state, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Db\Reference initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Reference){

	PHALCON_REGISTER_CLASS(Phalcon\\Db, Reference, db_reference, phalcon_db_reference_method_entry, 0);

	zend_declare_property_null(phalcon_db_reference_ce, SL("_schemaName"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_reference_ce, SL("_referencedSchema"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_reference_ce, SL("_referenceName"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_reference_ce, SL("_referencedTable"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_reference_ce, SL("_columns"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_reference_ce, SL("_referencedColumns"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_reference_ce, SL("_onDelete"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_reference_ce, SL("_onUpdate"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_db_reference_ce, 1, phalcon_db_referenceinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Db\Reference constructor
 *
 * @param string $referenceName
 * @param array $definition
 */
PHP_METHOD(Phalcon_Db_Reference, __construct){

	zval *reference_name, *definition, referenced_table = {}, columns = {}, referenced_columns = {}, schema = {}, referenced_schema = {}, on_delete = {}, on_update = {};
	long int number_columns, number_referenced_columns;

	phalcon_fetch_params(0, 2, 0, &reference_name, &definition);

	phalcon_update_property(getThis(), SL("_referenceName"), reference_name);

	if (phalcon_array_isset_fetch_str(&referenced_table, definition, SL("referencedTable"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_referencedTable"), &referenced_table);
	} else {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Referenced table is required");
		return;
	}

	if (phalcon_array_isset_fetch_str(&columns, definition, SL("columns"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_columns"), &columns);
	} else {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Foreign key columns are required");
		return;
	}

	if (phalcon_array_isset_fetch_str(&referenced_columns, definition, SL("referencedColumns"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_referencedColumns"), &referenced_columns);
	} else {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Referenced columns of the foreign key are required");
		return;
	}

	if (phalcon_array_isset_fetch_str(&schema, definition, SL("schema"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_schemaName"), &schema);
	}

	if (phalcon_array_isset_fetch_str(&referenced_schema, definition, SL("referencedSchema"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_referencedSchema"), &referenced_schema);
	}

	number_columns = phalcon_fast_count_int(&columns);
	number_referenced_columns = phalcon_fast_count_int(&referenced_columns);

	if (number_columns != number_referenced_columns) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Number of columns is not equal to the number of referenced columns");
		return;
	}

	if (phalcon_array_isset_fetch_str(&on_delete, definition, SL("onDelete"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_onDelete"), &on_delete);
	}

	if (phalcon_array_isset_fetch_str(&on_update, definition, SL("onUpdate"), PH_READONLY)) {
		phalcon_update_property(getThis(), SL("_onUpdate"), &on_update);
	}
}

/**
 * Gets the index name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Reference, getName){


	RETURN_MEMBER(getThis(), "_referenceName");
}

/**
 * Gets the schema where referenced table is
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Reference, getSchemaName){


	RETURN_MEMBER(getThis(), "_schemaName");
}

/**
 * Gets the schema where referenced table is
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Reference, getReferencedSchema){


	RETURN_MEMBER(getThis(), "_referencedSchema");
}

/**
 * Gets local columns which reference is based
 *
 * @return array
 */
PHP_METHOD(Phalcon_Db_Reference, getColumns){


	RETURN_MEMBER(getThis(), "_columns");
}

/**
 * Gets the referenced table
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Reference, getReferencedTable){


	RETURN_MEMBER(getThis(), "_referencedTable");
}

/**
 * Gets referenced columns
 *
 * @return array
 */
PHP_METHOD(Phalcon_Db_Reference, getReferencedColumns){


	RETURN_MEMBER(getThis(), "_referencedColumns");
}

/**
 * Gets the referenced on delete
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Reference, getOnDelete){


	RETURN_MEMBER(getThis(), "_onDelete");
}

/**
 * Gets the referenced on update
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Reference, getOnUpdate){


	RETURN_MEMBER(getThis(), "_onUpdate");
}

/**
 * Restore a Phalcon\Db\Reference object from export
 *
 * @param array $data
 * @return Phalcon\Db\Reference
 */
PHP_METHOD(Phalcon_Db_Reference, __set_state){

	zval *data, constraint_name = {}, referenced_schema = {}, referenced_table = {}, columns = {}, referenced_columns = {}, definition = {};

	phalcon_fetch_params(0, 1, 0, &data);

	if (!phalcon_array_isset_fetch_str(&constraint_name, data, SL("_referenceName"), PH_READONLY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "_referenceName parameter is required");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&referenced_schema, data, SL("_referencedSchema"), PH_READONLY)) {
		ZVAL_NULL(&referenced_schema);
	}

	if (!phalcon_array_isset_fetch_str(&referenced_table, data, SL("_referencedTable"), PH_READONLY)) {
		ZVAL_NULL(&referenced_table);
	}

	if (!phalcon_array_isset_fetch_str(&columns, data, SL("_columns"), PH_READONLY)) {
		ZVAL_NULL(&columns);
	}

	if (!phalcon_array_isset_fetch_str(&referenced_columns, data, SL("_referencedColumns"), PH_READONLY)) {
		ZVAL_NULL(&referenced_columns);
	}

	array_init_size(&definition, 4);
	phalcon_array_update_str(&definition, SL("referencedSchema"),  &referenced_schema, PH_COPY);
	phalcon_array_update_str(&definition, SL("referencedTable"),   &referenced_table, PH_COPY);
	phalcon_array_update_str(&definition, SL("columns"),           &columns, PH_COPY);
	phalcon_array_update_str(&definition, SL("referencedColumns"), &referenced_columns, PH_COPY);

	object_init_ex(return_value, phalcon_db_reference_ce);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", &constraint_name, &definition);
}
