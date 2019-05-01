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

#include "db/builder/delete.h"
#include "db/builder/where.h"
#include "db/builder/exception.h"
#include "db/adapterinterface.h"
#include "di/injectable.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/string.h"

/**
 * Phalcon\Db\Builder\Delete
 */
zend_class_entry *phalcon_db_builder_delete_ce;

PHP_METHOD(Phalcon_Db_Builder_Delete, __construct);
PHP_METHOD(Phalcon_Db_Builder_Delete, _execute);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_builder_delete___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, table, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, db, IS_STRING, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_db_builder_delete_method_entry[] = {
	PHP_ME(Phalcon_Db_Builder_Delete, __construct, arginfo_phalcon_db_builder_delete___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Db_Builder_Delete, _execute, NULL, ZEND_ACC_PROTECTED)
	PHP_FE_END
};

/**
 * Phalcon\Db\Builder\Delete initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Builder_Delete){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Db\\Builder, Delete, db_builder_delete, phalcon_db_builder_where_ce, phalcon_db_builder_delete_method_entry, 0);

	return SUCCESS;
}

/**
 * Phalcon\Db\Builder\Delete constructor
 *
 * @param array $params
 * @param Phalcon\Di $dependencyInjector
 */
PHP_METHOD(Phalcon_Db_Builder_Delete, __construct){

	zval *tables, *db = NULL;

	phalcon_fetch_params(0, 1, 1, &tables, &db);

	if (Z_TYPE_P(tables) != IS_STRING && Z_TYPE_P(tables) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_builder_exception_ce, "The tables must be string or array");
		return;
	}

	phalcon_update_property_array_str(getThis(), SL("_definition"), SL("tables"), tables);

	if (db && PHALCON_IS_NOT_EMPTY(db)) {
		phalcon_update_property(getThis(), SL("_defaultConnectionService"), db);
	}
}

/**
 * Returns a PHQL statement built based on the builder parameters
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Builder_Delete, _execute){

	zval definition = {}, conditions = {}, bind_params = {}, bind_types = {}, service = {}, dependency_injector = {}, connection = {}, dialect = {}, sql_delete = {};

	PHALCON_MM_INIT();

	phalcon_read_property(&definition, getThis(), SL("_definition"), PH_SEPARATE);
	PHALCON_MM_ADD_ENTRY(&definition);

	phalcon_read_property(&conditions, getThis(), SL("_conditions"), PH_READONLY);
	if (PHALCON_IS_NOT_EMPTY(&conditions)) {
		phalcon_array_update_str(&definition, SL("where"), &conditions, PH_COPY);
	}

	PHALCON_MM_CALL_SELF(&bind_params, "getbindparams");
	PHALCON_MM_ADD_ENTRY(&bind_params);

	PHALCON_MM_CALL_SELF(&bind_types, "getbindtypes");
	PHALCON_MM_ADD_ENTRY(&bind_types);

	phalcon_read_property(&service, getThis(), SL("_defaultConnectionService"), PH_READONLY);
	
	if (Z_TYPE(service) != IS_OBJECT) {
		if (PHALCON_IS_EMPTY(&service)) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_db_builder_exception_ce, "Invalid injected connection service");
			return;
		}

		PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi");
		PHALCON_MM_ADD_ENTRY(&dependency_injector);
		if (Z_TYPE(dependency_injector) != IS_OBJECT) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_db_builder_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
			return;
		}

		/**
		 * Request the connection service from the DI
		 */
		PHALCON_MM_CALL_METHOD(&connection, &dependency_injector, "getshared", &service);
		PHALCON_MM_ADD_ENTRY(&connection);
		if (Z_TYPE(connection) != IS_OBJECT) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_db_builder_exception_ce, "Invalid injected connection service");
			return;
		}
	} else {
		ZVAL_COPY_VALUE(&connection, &service);
		PHALCON_MM_VERIFY_INTERFACE(&connection, phalcon_db_adapterinterface_ce);
	}

	PHALCON_MM_CALL_METHOD(&dialect, &connection, "getdialect");
	PHALCON_MM_ADD_ENTRY(&dialect);

	PHALCON_MM_CALL_METHOD(&sql_delete, &dialect, "delete", &definition);
	PHALCON_MM_ADD_ENTRY(&sql_delete);

	/**
	 * Execute the query
	 */
	PHALCON_MM_CALL_METHOD(return_value, &connection, "execute", &sql_delete, &bind_params, &bind_types);

	RETURN_MM();
}
