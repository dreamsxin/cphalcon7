
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

#include "db/index.h"
#include "db/indexinterface.h"
#include "db/exception.h"

#include "kernel/main.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"

#include "internal/arginfo.h"

/**
 * Phalcon\Db\Index
 *
 * Allows to define indexes to be used on tables. Indexes are a common way
 * to enhance database performance. An index allows the database server to find
 * and retrieve specific rows much faster than it could do without an index
 */
zend_class_entry *phalcon_db_index_ce;

PHP_METHOD(Phalcon_Db_Index, __construct);
PHP_METHOD(Phalcon_Db_Index, getName);
PHP_METHOD(Phalcon_Db_Index, getColumns);
PHP_METHOD(Phalcon_Db_Index, getType);
PHP_METHOD(Phalcon_Db_Index, __set_state);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_index___construct, 0, 0, 2)
	ZEND_ARG_INFO(0, indexName)
	ZEND_ARG_INFO(0, columns)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_db_index_method_entry[] = {
	PHP_ME(Phalcon_Db_Index, __construct, arginfo_phalcon_db_index___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Db_Index, getName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Index, getColumns, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Index, getType, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Index, __set_state, arginfo___set_state, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Db\Index initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Index){

	PHALCON_REGISTER_CLASS(Phalcon\\Db, Index, db_index, phalcon_db_index_method_entry, 0);

	zend_declare_property_null(phalcon_db_index_ce, SL("_indexName"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_index_ce, SL("_columns"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_index_ce, SL("_type"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_db_index_ce, 1, phalcon_db_indexinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Db\Index constructor
 *
 * @param string $indexName
 * @param array $columns
 * @param string|null $type
 */
PHP_METHOD(Phalcon_Db_Index, __construct){

	zval *index_name, *columns, *type = NULL;

	phalcon_fetch_params(0, 2, 1, &index_name, &columns, &type);

	if (!type) {
		type = &PHALCON_GLOBAL(z_null);
	}

	phalcon_update_property(getThis(), SL("_indexName"), index_name);
	phalcon_update_property(getThis(), SL("_columns"), columns);
	phalcon_update_property(getThis(), SL("_type"), type);
}

/**
 * Gets the index name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Index, getName){


	RETURN_MEMBER(getThis(), "_indexName");
}

/**
 * Gets the columns that comprends the index
 *
 * @return array
 */
PHP_METHOD(Phalcon_Db_Index, getColumns){


	RETURN_MEMBER(getThis(), "_columns");
}

/**
 * Gets the index type
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Index, getType){

	zval type = {};

	phalcon_read_property(&type, getThis(), SL("_type"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(type) == IS_STRING) {
		RETURN_CTOR(&type);
	} else {
		RETVAL_EMPTY_STRING();
	}
}

/**
 * Restore a Phalcon\Db\Index object from export
 *
 * @param array $data
 * @return Phalcon\Db\IndexInterface
 */
PHP_METHOD(Phalcon_Db_Index, __set_state){

	zval *data, index_name = {}, columns = {};

	phalcon_fetch_params(0, 1, 0, &data);

	if (!phalcon_array_isset_fetch_str(&index_name, data, SL("_indexName"), PH_READONLY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "_indexName parameter is required");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&columns, data, SL("_columns"), PH_READONLY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "_columns parameter is required");
		return;
	}

	/**
	 * Return a Phalcon\Db\Index as part of the returning state
	 */
	object_init_ex(return_value, phalcon_db_index_ce);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", &index_name, &columns);
}
