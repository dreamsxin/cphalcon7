
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
  |          Kenji Minamoto <kenji.minamoto@gmail.com>                     |
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#include "mvc/collection.h"
#include "mvc/collectioninterface.h"
#include "mvc/collection/document.h"
#include "mvc/collection/resultset.h"
#include "mvc/collection/exception.h"
#include "mvc/collection/message.h"
#include "mvc/collection/managerinterface.h"
#include "di.h"
#include "diinterface.h"
#include "di/injectable.h"
#include "events/eventsawareinterface.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/string.h"
#include "kernel/hash.h"
#include "kernel/operators.h"
#include "kernel/file.h"
#include "kernel/concat.h"
#include "kernel/variables.h"

#include "internal/arginfo.h"

/**
 * Phalcon\Mvc\Collection
 *
 * This component implements a high level abstraction for NoSQL databases which
 * works with documents
 */
zend_class_entry *phalcon_mvc_collection_ce;

PHP_METHOD(Phalcon_Mvc_Collection, __construct);
PHP_METHOD(Phalcon_Mvc_Collection, setId);
PHP_METHOD(Phalcon_Mvc_Collection, getId);
PHP_METHOD(Phalcon_Mvc_Collection, getIdString);
PHP_METHOD(Phalcon_Mvc_Collection, setEventsManager);
PHP_METHOD(Phalcon_Mvc_Collection, getEventsManager);
PHP_METHOD(Phalcon_Mvc_Collection, setColumnMap);
PHP_METHOD(Phalcon_Mvc_Collection, getColumnMap);
PHP_METHOD(Phalcon_Mvc_Collection, getColumnName);
PHP_METHOD(Phalcon_Mvc_Collection, getCollectionManager);
PHP_METHOD(Phalcon_Mvc_Collection, getReservedAttributes);
PHP_METHOD(Phalcon_Mvc_Collection, useImplicitObjectIds);
PHP_METHOD(Phalcon_Mvc_Collection, setStrictMode);
PHP_METHOD(Phalcon_Mvc_Collection, setSource);
PHP_METHOD(Phalcon_Mvc_Collection, getSource);
PHP_METHOD(Phalcon_Mvc_Collection, setConnectionService);
PHP_METHOD(Phalcon_Mvc_Collection, getConnectionService);
PHP_METHOD(Phalcon_Mvc_Collection, getConnection);
PHP_METHOD(Phalcon_Mvc_Collection, assign);
PHP_METHOD(Phalcon_Mvc_Collection, readAttribute);
PHP_METHOD(Phalcon_Mvc_Collection, writeAttribute);
PHP_METHOD(Phalcon_Mvc_Collection, cloneResult);
PHP_METHOD(Phalcon_Mvc_Collection, _getResultset);
PHP_METHOD(Phalcon_Mvc_Collection, _getGroupResultset);
PHP_METHOD(Phalcon_Mvc_Collection, _preSave);
PHP_METHOD(Phalcon_Mvc_Collection, _postSave);
PHP_METHOD(Phalcon_Mvc_Collection, validate);
PHP_METHOD(Phalcon_Mvc_Collection, validationHasFailed);
PHP_METHOD(Phalcon_Mvc_Collection, fireEvent);
PHP_METHOD(Phalcon_Mvc_Collection, fireEventCancel);
PHP_METHOD(Phalcon_Mvc_Collection, _cancelOperation);
PHP_METHOD(Phalcon_Mvc_Collection, _exists);
PHP_METHOD(Phalcon_Mvc_Collection, getMessages);
PHP_METHOD(Phalcon_Mvc_Collection, appendMessage);
PHP_METHOD(Phalcon_Mvc_Collection, save);
PHP_METHOD(Phalcon_Mvc_Collection, findById);
PHP_METHOD(Phalcon_Mvc_Collection, findFirst);
PHP_METHOD(Phalcon_Mvc_Collection, find);
PHP_METHOD(Phalcon_Mvc_Collection, count);
PHP_METHOD(Phalcon_Mvc_Collection, aggregate);
PHP_METHOD(Phalcon_Mvc_Collection, summatory);
PHP_METHOD(Phalcon_Mvc_Collection, create);
PHP_METHOD(Phalcon_Mvc_Collection, update);
PHP_METHOD(Phalcon_Mvc_Collection, delete);
PHP_METHOD(Phalcon_Mvc_Collection, getOperationMade);
PHP_METHOD(Phalcon_Mvc_Collection, toArray);
PHP_METHOD(Phalcon_Mvc_Collection, serialize);
PHP_METHOD(Phalcon_Mvc_Collection, unserialize);
PHP_METHOD(Phalcon_Mvc_Collection, execute);
PHP_METHOD(Phalcon_Mvc_Collection, incr);
PHP_METHOD(Phalcon_Mvc_Collection, refresh);
PHP_METHOD(Phalcon_Mvc_Collection, drop);
PHP_METHOD(Phalcon_Mvc_Collection, parse);
PHP_METHOD(Phalcon_Mvc_Collection, __set);
PHP_METHOD(Phalcon_Mvc_Collection, __get);
PHP_METHOD(Phalcon_Mvc_Collection, __callStatic);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_collection___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, dependencyInjector)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_collection_setcolumnmap, 0, 0, 1)
	ZEND_ARG_INFO(0, columnMap)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_collection_getcolumnname, 0, 0, 1)
	ZEND_ARG_INFO(0, column)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_collection_setstrictmode, 0, 0, 1)
	ZEND_ARG_INFO(0, strictMode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_collection_setsource, 0, 0, 1)
	ZEND_ARG_INFO(0, source)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_collection_aggregate, 0, 0, 1)
	ZEND_ARG_INFO(0, parameters)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_collection_summatory, 0, 0, 1)
	ZEND_ARG_INFO(0, field)
	ZEND_ARG_INFO(0, condition)
	ZEND_ARG_INFO(0, finalize)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_collection_toarray, 0, 0, 0)
	ZEND_ARG_INFO(0, columns)
	ZEND_ARG_INFO(0, renameColumns)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_collection_execute, 0, 0, 1)
	ZEND_ARG_INFO(0, code)
	ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_collection_incr, 0, 0, 1)
	ZEND_ARG_INFO(0, field)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_collection_parse, 0, 0, 1)
	ZEND_ARG_INFO(0, conditions)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_collection___set, 0, 0, 2)
	ZEND_ARG_INFO(0, property)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_collection___get, 0, 0, 1)
	ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_collection___callstatic, 0, 0, 1)
	ZEND_ARG_INFO(0, method)
	ZEND_ARG_INFO(0, arguments)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_collection_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Collection, __construct, arginfo_phalcon_mvc_collection___construct, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Collection, setId, arginfo_phalcon_mvc_collectioninterface_setid, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, getId, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, getIdString, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, setEventsManager, arginfo_phalcon_events_eventsawareinterface_seteventsmanager, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, getEventsManager, arginfo_phalcon_events_eventsawareinterface_geteventsmanager, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, setColumnMap, arginfo_phalcon_mvc_collection_setcolumnmap, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, getColumnMap, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, getColumnName, arginfo_phalcon_mvc_collection_getcolumnname, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, getCollectionManager, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, getReservedAttributes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, useImplicitObjectIds, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Collection, setStrictMode, arginfo_phalcon_mvc_collection_setstrictmode, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Collection, setSource, arginfo_phalcon_mvc_collection_setsource, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Collection, getSource, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, setConnectionService, arginfo_phalcon_mvc_collectioninterface_setconnectionservice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, getConnectionService, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, getConnection, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, assign, arginfo_phalcon_mvc_collectioninterface_assign, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, readAttribute, arginfo_phalcon_mvc_collectioninterface_readattribute, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, writeAttribute, arginfo_phalcon_mvc_collectioninterface_writeattribute, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, cloneResult, arginfo_phalcon_mvc_collectioninterface_cloneresult, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Collection, _getResultset, NULL, ZEND_ACC_PROTECTED|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Collection, _getGroupResultset, NULL, ZEND_ACC_PROTECTED|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Collection, _preSave, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Collection, _postSave, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Collection, validate, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Collection, validationHasFailed, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, fireEvent, arginfo_phalcon_di_injectable_fireevent, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, fireEventCancel, arginfo_phalcon_di_injectable_fireeventcancel, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, _cancelOperation, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Collection, _exists, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Collection, getMessages, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, appendMessage, arginfo_phalcon_mvc_collectioninterface_appendmessage, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, save, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, findById, arginfo_phalcon_mvc_collectioninterface_findbyid, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Collection, findFirst, arginfo_phalcon_mvc_collectioninterface_findfirst, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Collection, find, arginfo_phalcon_mvc_collectioninterface_find, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Collection, count, arginfo_phalcon_mvc_collectioninterface_count, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Collection, aggregate, arginfo_phalcon_mvc_collection_aggregate, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Collection, summatory, arginfo_phalcon_mvc_collection_summatory, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Collection, create, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, update, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, delete, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, getOperationMade, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, toArray, arginfo_phalcon_mvc_collection_toarray, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, serialize, arginfo_serializable_serialize, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, unserialize, arginfo_serializable_unserialize, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, execute, arginfo_phalcon_mvc_collection_execute, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Collection, incr, arginfo_phalcon_mvc_collection_incr, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, refresh, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, parse, arginfo_phalcon_mvc_collection_parse, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, drop, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Collection, __set, arginfo_phalcon_mvc_collection___set, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, __get, arginfo_phalcon_mvc_collection___get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Collection, __callStatic, arginfo_phalcon_mvc_collection___callstatic, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Collection initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Collection){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc, Collection, mvc_collection, phalcon_di_injectable_ce, phalcon_mvc_collection_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_collection_ce, SL("_collectionManager"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_mvc_collection_ce, SL("_operationMade"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_collection_ce, SL("_connection"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_collection_ce, SL("_errorMessages"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_collection_ce, SL("_reserved"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_bool(phalcon_mvc_collection_ce, SL("_disableEvents"), 0, ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);

	zend_declare_class_constant_long(phalcon_mvc_collection_ce, SL("OP_NONE"), 0);
	zend_declare_class_constant_long(phalcon_mvc_collection_ce, SL("OP_CREATE"), 1);
	zend_declare_class_constant_long(phalcon_mvc_collection_ce, SL("OP_UPDATE"), 2);
	zend_declare_class_constant_long(phalcon_mvc_collection_ce, SL("OP_DELETE"), 3);

	zend_class_implements(phalcon_mvc_collection_ce, 2, phalcon_mvc_collectioninterface_ce, zend_ce_serializable);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Collection constructor
 *
 * @param Phalcon\DiInterface $dependencyInjector
 * @param Phalcon\Mvc\Collection\ManagerInterface $collectionManager
 */
PHP_METHOD(Phalcon_Mvc_Collection, __construct){

	zval *dependency_injector = NULL, *collection_manager = NULL;
	zval di, service_name, mm;

	phalcon_fetch_params(0, 0, 2, &dependency_injector, &collection_manager);

	/**
	 * We use a default DI if the user doesn't define one
	 */
	if (!dependency_injector || Z_TYPE_P(dependency_injector) != IS_OBJECT) {
		PHALCON_CALL_CE_STATIC(&di, phalcon_di_ce, "getdefault");
	} else {
		ZVAL_COPY(&di, dependency_injector);
	}

	PHALCON_VERIFY_INTERFACE_EX(&di, phalcon_diinterface_ce, phalcon_mvc_collection_exception_ce, 1);

	phalcon_update_property_this(getThis(), SL("_dependencyInjector"), &di);

	/**
	 * Inject the manager service from the DI
	 */
	if (!collection_manager || Z_TYPE_P(collection_manager) != IS_OBJECT) {
		ZVAL_STRING(&service_name, "collectionManager");

		PHALCON_CALL_METHOD(&mm, &di, "getshared", &service_name);
		if (Z_TYPE(mm) != IS_OBJECT) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_collection_exception_ce, "The injected service 'collectionManager' is not valid");
			return;
		}
	} else {
		ZVAL_COPY(&mm, collection_manager);
	}

	PHALCON_VERIFY_INTERFACE_EX(&mm, phalcon_mvc_collection_managerinterface_ce, phalcon_mvc_collection_exception_ce, 1);

	/**
	 * Update the collection-manager
	 */
	phalcon_update_property_this(getThis(), SL("_collectionManager"), &mm);

	/**
	 * The manager always initializes the object
	 */
	PHALCON_CALL_METHOD(NULL, &mm, "initialize", getThis());

	/**
	 * This allows the developer to execute initialization stuff every time an instance
	 * is created
	 */
	if (phalcon_method_exists_ex(getThis(), SL("onconstruct")) == SUCCESS) {
		PHALCON_CALL_METHOD(NULL, getThis(), "onconstruct");
	}
}

/**
 * Sets a value for the _id property, creates a MongoId object if needed
 *
 * @param mixed $id
 */
PHP_METHOD(Phalcon_Mvc_Collection, setId){

	zval *id, id_name, column_map, *collection_manager, use_implicit_ids, mongo_id;
	zend_class_entry *ce0;

	phalcon_fetch_params(0, 1, 0, &id);

	ZVAL_STRING(&id_name, "_id");

	PHALCON_CALL_SELF(&column_map, "getcolumnmap");

	if (Z_TYPE(column_map) == IS_ARRAY) {
		if (phalcon_array_isset_str(&column_map, SL("_id"))) {
			phalcon_array_fetch_str(&id_name, &column_map, SL("_id"), PH_NOISY);
		}
	}

	if (Z_TYPE_P(id) != IS_OBJECT) {
		collection_manager = phalcon_read_property(getThis(), SL("_collectionManager"), PH_NOISY);

		/**
		 * Check if the collection use implicit ids
		 */
		PHALCON_CALL_METHOD(&use_implicit_ids, collection_manager, "isusingimplicitobjectids", getThis());
		if (zend_is_true(&use_implicit_ids)) {
			object_init_ex(&mongo_id, ce0);
			PHALCON_CALL_METHOD(NULL, &mongo_id, "__construct", id);
		} else {
			ZVAL_COPY(&mongo_id, id);
		}
	} else {
		ZVAL_COPY(&mongo_id, id);
	}

	phalcon_update_property_zval_zval(getThis(), &id_name, &mongo_id);
}

/**
 * Returns the value of the _id property
 *
 * @return \MongoId
 */
PHP_METHOD(Phalcon_Mvc_Collection, getId){

	zval id_name, column_map, id, mongo_id, *collection_manager, use_implicit_ids;
	zend_class_entry *ce0;

	PHALCON_MM_GROW();

	ZVAL_STRING(&id_name, "_id");

	PHALCON_CALL_SELF(&column_map, "getcolumnmap");

	if (Z_TYPE(column_map) == IS_ARRAY) { 
		if (phalcon_array_isset_str(&column_map, SL("_id"))) {
			phalcon_array_fetch_str(&id_name, &column_map, SL("_id"), PH_NOISY);
		}
	}

	if (phalcon_property_isset_fetch_zval(&id, getThis(), &id_name)) {
		if (Z_TYPE(id) != IS_OBJECT) {
			collection_manager = phalcon_read_property(getThis(), SL("_collectionManager"), PH_NOISY);

			/**
			 * Check if the collection use implicit ids
			 */
			PHALCON_CALL_METHOD(&use_implicit_ids, collection_manager, "isusingimplicitobjectids", getThis());
			if (zend_is_true(&use_implicit_ids)) {
				ce0 = zend_fetch_class(SSL("MongoId"), ZEND_FETCH_CLASS_AUTO);
				object_init_ex(&mongo_id, ce0);
				if (phalcon_has_constructor(&mongo_id)) {
					PHALCON_CALL_METHOD(NULL, &mongo_id, "__construct", &id);
				}
				phalcon_update_property_zval_zval(getThis(), &id_name, &mongo_id);
			} else {
				ZVAL_COPY(&mongo_id, &id);
			}
		} else {
			ZVAL_COPY(&mongo_id, &id);
		}
	}

	RETURN_CTOR(&mongo_id);
}

PHP_METHOD(Phalcon_Mvc_Collection, getIdString){

	zval id;

	PHALCON_CALL_SELF(&id, "getid");

	if (Z_TYPE(id) == IS_OBJECT) {
		PHALCON_RETURN_CALL_METHOD(&id, "__tostring");
	}

	RETURN_CTORW(&id);
}

/**
 * Sets a custom events manager
 *
 * @param Phalcon\Events\ManagerInterface $eventsManager
 */
PHP_METHOD(Phalcon_Mvc_Collection, setEventsManager){

	zval *events_manager, *collection_manager;

	phalcon_fetch_params(0, 1, 0, &events_manager);

	collection_manager = phalcon_read_property(getThis(), SL("_collectionManager"), PH_NOISY);
	PHALCON_CALL_METHODW(NULL, collection_manager, "setcustomeventsmanager", getThis(), events_manager);
}

/**
 * Returns the custom events manager
 *
 * @return Phalcon\Events\ManagerInterface
 */
PHP_METHOD(Phalcon_Mvc_Collection, getEventsManager){

	zval *collection_manager;

	collection_manager = phalcon_read_property(getThis(), SL("_collectionManager"), PH_NOISY);
	PHALCON_RETURN_CALL_METHODW(collection_manager, "getcustomeventsmanager", getThis());
}

/**
 * Returns the collection manager related to the entity instance
 *
 * @return Phalcon\Mvc\Collection\ManagerInterface
 */
PHP_METHOD(Phalcon_Mvc_Collection, getCollectionManager){


	RETURN_MEMBER(getThis(), "_collectionManager");
}

/**
 * Sets the column map
 *
 * @param array $columnMap
 * @return Phalcon\Mvc\Collection
 */
PHP_METHOD(Phalcon_Mvc_Collection, setColumnMap){

	zval *column_map, *collection_manager;

	phalcon_fetch_params(0, 1, 0, &column_map);

	collection_manager = phalcon_read_property(getThis(), SL("_collectionManager"), PH_NOISY);
	PHALCON_CALL_METHODW(NULL, collection_manager, "setcolumnmap", getThis(), column_map);

	RETURN_THISW();
}

/**
 * Returns the column map
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Collection, getColumnMap){

	zval *collection_manager;

	collection_manager = phalcon_read_property(getThis(), SL("_collectionManager"), PH_NOISY);
	PHALCON_RETURN_CALL_METHODW(collection_manager, "getcolumnmap", getThis());
}

/**
 * Returns the column name
 *
 * @param string $column
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Collection, getColumnName){

	zval *column, column_map, name;

	phalcon_fetch_params(0, 1, 0, &column);
	
	PHALCON_CALL_SELFW(&column_map, "getColumnMap");

	if (Z_TYPE(column_map) == IS_ARRAY) {
		if (!phalcon_array_isset_fetch(&name, &column_map, column)) {
			ZVAL_COPY(&name, column);
		}
	} else {
		ZVAL_COPY(&name, column);
	}

	RETURN_CTORW(&name);
}

/**
 * Returns an array with reserved properties that cannot be part of the insert/update
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Collection, getReservedAttributes){

	zval *reserved;

	reserved = phalcon_read_static_property_ce(phalcon_mvc_collection_ce, SL("_reserved"));
	if (Z_TYPE_P(reserved) == IS_NULL) {
		zval *dummy = &PHALCON_GLOBAL(z_true);

		array_init_size(return_value, 5);
		Z_TRY_ADDREF_P(dummy); add_assoc_zval_ex(return_value, SL("_connection"), dummy);
		Z_TRY_ADDREF_P(dummy); add_assoc_zval_ex(return_value, SL("_dependencyInjector"), dummy);
		Z_TRY_ADDREF_P(dummy); add_assoc_zval_ex(return_value, SL("_operationMade"), dummy);
		Z_TRY_ADDREF_P(dummy); add_assoc_zval_ex(return_value, SL("_errorMessages"), dummy);

		phalcon_update_static_property_ce(phalcon_mvc_collection_ce, SL("_reserved"), return_value);
		return;
	}

	RETURN_ZVAL(reserved, 1, 0);
}

/**
 * Sets if a collection must use implicit objects ids
 *
 * @param boolean $useImplicitObjectIds
 */
PHP_METHOD(Phalcon_Mvc_Collection, useImplicitObjectIds){

	zval *use_implicit_object_ids, *collection_manager;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 0, &use_implicit_object_ids);

	collection_manager = phalcon_read_property(getThis(), SL("_collectionManager"), PH_NOISY);
	PHALCON_CALL_METHOD(NULL, collection_manager, "useimplicitobjectids", getThis(), use_implicit_object_ids);

	PHALCON_MM_RESTORE();
}

/**
 * Sets strict mode enbale or disable
 *
 * @param boolean $strictMode
 */
PHP_METHOD(Phalcon_Mvc_Collection, setStrictMode){

	zval *strict_mode, *collection_manager;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 0, &strict_mode);

	collection_manager = phalcon_read_property(getThis(), SL("_collectionManager"), PH_NOISY);
	PHALCON_CALL_METHOD(NULL, collection_manager, "setstrictmode", getThis(), strict_mode);

	PHALCON_MM_RESTORE();
}

/**
 * Sets collection name which collection should be mapped
 *
 * @param string $source
 * @return Phalcon\Mvc\Collection
 */
PHP_METHOD(Phalcon_Mvc_Collection, setSource){

	zval *source, *collection_manager;

	phalcon_fetch_params(0, 1, 0, &source);

	collection_manager = phalcon_read_property(getThis(), SL("_collectionManager"), PH_NOISY);
	PHALCON_CALL_METHODW(NULL, collection_manager, "setsource", getThis(), source);

	RETURN_THISW();
}

/**
 * Returns collection name mapped in the collection
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Collection, getSource){

	zval *collection_manager;

	collection_manager = phalcon_read_property(getThis(), SL("_collectionManager"), PH_NOISY);

	PHALCON_RETURN_CALL_METHODW(collection_manager, "getsource", getThis());
}

/**
 * Sets the DependencyInjection connection service name
 *
 * @param string $connectionService
 * @return Phalcon\Mvc\Collection
 */
PHP_METHOD(Phalcon_Mvc_Collection, setConnectionService){

	zval *connection_service, *collection_manager;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 0, &connection_service);

	collection_manager = phalcon_read_property(getThis(), SL("_collectionManager"), PH_NOISY);
	PHALCON_CALL_METHOD(NULL, collection_manager, "setconnectionservice", getThis(), connection_service);
	RETURN_THIS();
}

/**
 * Returns DependencyInjection connection service
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Collection, getConnectionService){

	zval *collection_manager;

	collection_manager = phalcon_read_property(getThis(), SL("_collectionManager"), PH_NOISY);
	PHALCON_RETURN_CALL_METHODW(collection_manager, "getconnectionservice", getThis());
}

/**
 * Retrieves a database connection
 *
 * @return \MongoDb
 */
PHP_METHOD(Phalcon_Mvc_Collection, getConnection){

	zval connection, *collection_manager;

	phalcon_return_property(&connection, getThis(), SL("_connection"));
	if (Z_TYPE(connection) != IS_OBJECT) {
		collection_manager = phalcon_read_property(getThis(), SL("_collectionManager"), PH_NOISY);

		PHALCON_CALL_METHODW(&connection, collection_manager, "getconnection", getThis());
		phalcon_update_property_this(getThis(), SL("_connection"), &connection);
	}

	RETURN_CTORW(&connection);
}

/**
 * Assigns values to a collection from an array
 *
 *<code>
 *$robot->assign(array(
 *  'type' => 'mechanical',
 *  'name' => 'Astro Boy',
 *  'year' => 1952
 *));
 *</code>
 *
 * @param array $data
 * @param array $whiteList
 * @return Phalcon\Mvc\Collection
 */
PHP_METHOD(Phalcon_Mvc_Collection, assign){

	zval *data, *white_list = NULL, column_map, *value;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 1, 1, &data, &white_list);
	
	if (!white_list) {
		white_list = &PHALCON_GLOBAL(z_null);
	}
	
	if (Z_TYPE_P(data) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_collection_exception_ce, "Data to dump in the object must be an Array");
		return;
	}

	PHALCON_CALL_SELFW(&column_map, "getcolumnmap");

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(data), idx, str_key, value) {
		zval key, attribute, exception_message;

		if (str_key) {
			ZVAL_STR(&key, str_key);
		} else {
			ZVAL_LONG(&key, idx);
		}
		/** 
		 * Only string keys in the data are valid
		 */
		if (Z_TYPE_P(white_list) == IS_ARRAY) {
			if (!phalcon_fast_in_array(&key, white_list)) {
				continue;
			}
		}

		if (Z_TYPE(column_map) == IS_ARRAY) {
			/** 
			 * Every field must be part of the column map
			 */
			if (phalcon_array_isset_fetch(&attribute, &column_map, &key)) {
				phalcon_update_property_zval_zval(getThis(), &attribute, value);
			} else {
				PHALCON_CONCAT_SVS(&exception_message, "Column \"", &key, "\" doesn't make part of the column map");
				PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_collection_exception_ce, &exception_message);
				return;
			}
		} else {
			phalcon_update_property_zval_zval(getThis(), &key, value);
		}
	} ZEND_HASH_FOREACH_END();
}

/**
 * Reads an attribute value by its name
 *
 *<code>
 *	echo $robot->readAttribute('name');
 *</code>
 *
 * @param string $attribute
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Collection, readAttribute){

	zval *attribute, *attribute_value;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 0, &attribute);

	if (phalcon_isset_property_zval(getThis(), attribute)) {
		attribute_value = phalcon_read_property_zval(getThis(), attribute, PH_NOISY);
		RETURN_CTOR(attribute_value);
	}
	RETURN_MM_NULL();
}

/**
 * Writes an attribute value by its name
 *
 *<code>
 *	$robot->writeAttribute('name', 'Rosey');
 *</code>
 *
 * @param string $attribute
 * @param mixed $value
 */
PHP_METHOD(Phalcon_Mvc_Collection, writeAttribute){

	zval *attribute, *value;

	phalcon_fetch_params(0, 2, 0, &attribute, &value);

	phalcon_update_property_zval_zval(getThis(), attribute, value);

}

/**
 * Returns a cloned collection
 *
 * @param Phalcon\Mvc\Collection $collection
 * @param array $document
 * @return Phalcon\Mvc\Collection
 */
PHP_METHOD(Phalcon_Mvc_Collection, cloneResult){

	zval *collection, *document, cloned_collection, column_map, *value;
	zend_string *str_key;
	ulong idx;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 2, 0, &collection, &document);

	if (Z_TYPE_P(collection) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_collection_exception_ce, "Invalid collection");
		return;
	}
	if (Z_TYPE_P(document) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_collection_exception_ce, "Invalid document");
		return;
	}

	if (phalcon_clone(&cloned_collection, collection) == FAILURE) {
		RETURN_MM();
	}

	PHALCON_CALL_METHOD(&column_map, collection, "getcolumnmap");

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(document), idx, str_key, value) {
		zval tmp, attribute_field;
		if (str_key) {
			ZVAL_STR(&tmp, str_key);
		} else {
			ZVAL_LONG(&tmp, idx);
		}

		if (Z_TYPE(column_map) == IS_ARRAY) { 
			if (phalcon_array_isset(&column_map, &tmp)) {
				phalcon_array_fetch(&attribute_field, &column_map, &tmp, PH_NOISY);
			} else {
				ZVAL_COPY(&attribute_field, &tmp);
			}
		} else {
			ZVAL_COPY(&attribute_field, &tmp);
		}

		PHALCON_CALL_METHOD(NULL, &cloned_collection, "writeattribute", &attribute_field, value);
	} ZEND_HASH_FOREACH_END();

	if (phalcon_method_exists_ex(&cloned_collection, SL("afterfetch")) == SUCCESS) {
		PHALCON_CALL_METHOD(NULL, &cloned_collection, "afterfetch");
	}

	RETURN_CTOR(&cloned_collection);
}

/**
 * Returns a collection resultset
 *
 * @param array $params
 * @param Phalcon\Mvc\Collection $collection
 * @param \MongoDb $connection
 * @param boolean $unique
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Collection, _getResultset){

	zval *params, *collection, *connection, *unique;
	zval source, mongo_collection, conditions, new_conditions;
	zval fields, documents_cursor, limit, sort, order, skip;
	zval class_name, base, document, exception_message;
	zend_class_entry *ce0;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 4, 0, &params, &collection, &connection, &unique);

	PHALCON_CALL_METHOD(&source, collection, "getsource");
	if (PHALCON_IS_EMPTY(&source)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_collection_exception_ce, "Method getSource() returns empty string");
		return;
	}

	PHALCON_CALL_METHOD(&mongo_collection, connection, "selectcollection", &source);
	if (Z_TYPE(mongo_collection) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_collection_exception_ce, "Couldn't select mongo collection");
		return;
	}

	/**
	 * Convert the string to an array
	 */
	if (!phalcon_array_isset_fetch_str(&conditions, params, SL("conditions"))) {
		if (!phalcon_array_isset_fetch_long(&conditions, params, 0)) {
			array_init(&conditions);
		}
	}

	if (Z_TYPE(conditions) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_collection_exception_ce, "Find parameters must be an array");
		return;
	}

	PHALCON_CALL_METHOD(&new_conditions, collection, "parse", &conditions);

	/**
	 * Perform the find
	 */
	if (phalcon_array_isset_fetch_str(&fields, params, SL("fields"))) {
		PHALCON_CALL_METHOD(&documents_cursor, &mongo_collection, "find", &new_conditions, &fields);
	} else {
		PHALCON_CALL_METHOD(&documents_cursor, &mongo_collection, "find", &new_conditions);
	}

	/**
	 * Check if a 'limit' clause was defined
	 */
	if (phalcon_array_isset_fetch_str(&limit, params, SL("limit"))) {
		PHALCON_CALL_METHOD(NULL, &documents_cursor, "limit", &limit);
	}

	/**
	 * Check if a 'sort' clause was defined
	 */
	if (phalcon_array_isset_fetch_str(&sort, params, SL("sort"))) {
		PHALCON_CALL_METHOD(NULL, &documents_cursor, "sort", &sort);
	} else if (phalcon_array_isset_fetch_str(&order, params, SL("order"))) {
		PHALCON_CALL_METHOD(NULL, &documents_cursor, "sort", &order);
	}

	/**
	 * Check if a 'skip' clause was defined
	 */
	if (phalcon_array_isset_fetch_str(&skip, params, SL("skip"))) {
		PHALCON_CALL_METHOD(NULL, &documents_cursor, "skip", &skip);
	}

	/**
	 * If a group of specific fields are requested we use a
	 * Phalcon\Mvc\Collection\Document instead
	 */
	if (phalcon_array_isset_str(params, SL("fields"))) {
		if (phalcon_array_isset_fetch_str(&class_name, params, SL("class"))) {			
			ce0 = phalcon_fetch_class(&class_name TSRMLS_CC);
			object_init_ex(&base, ce0);
			if (!instanceof_function_ex(Z_OBJCE(base), phalcon_mvc_collection_document_ce, 1 TSRMLS_CC) && !instanceof_function_ex(Z_OBJCE(base), phalcon_mvc_collection_document_ce, 1 TSRMLS_CC)) {
				PHALCON_CONCAT_SVS(&exception_message, "Object of class '", &class_name, "' must be an implementation of Phalcon\\Mvc\\CollectionInterface or an instance of Phalcon\\Mvc\\Collection\\Document");
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_collection_exception_ce, &exception_message);
				return;
			}
		} else {
			object_init_ex(&base, phalcon_mvc_collection_document_ce);
		}
	} else {
		ZVAL_COPY(&base, collection);
	}

	if (PHALCON_IS_TRUE(unique)) {

		/**
		 * Requesting a single result
		 */
		PHALCON_CALL_METHOD(NULL, &documents_cursor, "rewind");
		PHALCON_CALL_METHOD(&document, &documents_cursor, "current");
		if (Z_TYPE(document) == IS_ARRAY) {
			/**
			 * Assign the values to the base object
			 */
			PHALCON_RETURN_CALL_SELF("cloneresult", &base, &document);
			RETURN_MM();
		}

		RETURN_MM_FALSE;
	}

	object_init_ex(return_value, phalcon_mvc_collection_resultset_ce);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", collection, &documents_cursor);

	RETURN_MM();
}

/**
 * Perform a count over a resultset
 *
 * @param array $params
 * @param Phalcon\Mvc\Collection $collection
 * @param \MongoDb $connection
 * @return int
 */
PHP_METHOD(Phalcon_Mvc_Collection, _getGroupResultset){

	zval *params, *collection, *connection, source, mongo_collection, conditions, new_conditions, simple, documents_cursor, limit, sort, skip;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 3, 0, &params, &collection, &connection);

	PHALCON_CALL_METHOD(&source, collection, "getsource");
	if (PHALCON_IS_EMPTY(&source)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_collection_exception_ce, "Method getSource() returns empty string");
		return;
	}

	PHALCON_CALL_METHOD(&mongo_collection, connection, "selectcollection", &source);

	/**
	 * Convert the string to an array
	 */
	if (!phalcon_array_isset_fetch_long(&conditions, params, 0)) {
		if (!phalcon_array_isset_fetch_str(&conditions, params, SL("conditions"))) {
			array_init(&conditions);
		}
	}

	PHALCON_CALL_METHOD(&new_conditions, collection, "parse", &conditions);

	ZVAL_TRUE(&simple);
	if (phalcon_array_isset_str(params, SL("limit"))) {
		ZVAL_FALSE(&simple);
	} else {
		if (phalcon_array_isset_str(params, SL("sort"))) {
			ZVAL_FALSE(&simple);
		} else {
			if (phalcon_array_isset_str(params, SL("skip"))) {
				ZVAL_FALSE(&simple);
			}
		}
	}

	if (PHALCON_IS_FALSE(&simple)) {
		/**
		 * Perform the find
		 */
		PHALCON_CALL_METHOD(&documents_cursor, &mongo_collection, "find", &new_conditions);

		/**
		 * Check if a 'limit' clause was defined
		 */
		if (phalcon_array_isset_fetch_str(&limit, params, SL("limit"))) {
			PHALCON_CALL_METHOD(NULL, &documents_cursor, "limit", &limit);
		}

		/**
		 * Check if a 'sort' clause was defined
		 */
		if (phalcon_array_isset_fetch_str(&sort, params, SL("sort"))) {
			PHALCON_CALL_METHOD(NULL, &documents_cursor, "sort", &sort);
		}

		/**
		 * Check if a 'skip' clause was defined
		 */
		if (phalcon_array_isset_fetch_str(&skip, params, SL("skip"))) {
			PHALCON_CALL_METHOD(NULL, &documents_cursor, "skip", &skip);
		}

		/**
		 * Only 'count' is supported
		 */
		phalcon_fast_count(return_value, &documents_cursor);
		RETURN_MM();
	}

	PHALCON_RETURN_CALL_METHOD(&mongo_collection, "count", &conditions);
	RETURN_MM();
}

/**
 * Executes internal hooks before save a document
 *
 * @param Phalcon\DiInterface $dependencyInjector
 * @param boolean $disableEvents
 * @param boolean $exists
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Collection, _preSave){

	zval *dependency_injector, *disable_events, *exists, event_name, status;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 3, 0, &dependency_injector, &disable_events, &exists);

	/**
	 * Run Validation Callbacks Before
	 */
	if (!zend_is_true(disable_events)) {
		ZVAL_STRING(&event_name, "beforeValidation");

		PHALCON_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_MM_FALSE;
		}

		if (!zend_is_true(exists)) {
			ZVAL_STRING(&event_name, "beforeValidationOnCreate");
		} else {
			ZVAL_STRING(&event_name, "beforeValidationOnUpdate");
		}

		PHALCON_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_MM_FALSE;
		}
	}

	/**
	 * Run validation
	 */
	ZVAL_STRING(&event_name, "validation");
	PHALCON_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
	if (PHALCON_IS_FALSE(&status)) {
		if (!zend_is_true(disable_events)) {
			ZVAL_STRING(&event_name, "onValidationFails");
			PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
		}
		RETURN_MM_FALSE;
	}

	if (!zend_is_true(disable_events)) {

		/**
		 * Run Validation Callbacks After
		 */
		if (!zend_is_true(exists)) {
			ZVAL_STRING(&event_name, "afterValidationOnCreate");
		} else {
			ZVAL_STRING(&event_name, "afterValidationOnUpdate");
		}

		PHALCON_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_MM_FALSE;
		}

		ZVAL_STRING(&event_name, "afterValidation");

		PHALCON_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_MM_FALSE;
		}

		/**
		 * Run Before Callbacks
		 */
		ZVAL_STRING(&event_name, "beforeSave");

		PHALCON_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_MM_FALSE;
		}

		if (zend_is_true(exists)) {
			ZVAL_STRING(&event_name, "beforeUpdate");
		} else {
			ZVAL_STRING(&event_name, "beforeCreate");
		}

		PHALCON_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_MM_FALSE;
		}
	}

	RETURN_MM_TRUE;
}

/**
 * Executes internal events after save a document
 *
 * @param boolean $disableEvents
 * @param boolean $success
 * @param boolean $exists
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Collection, _postSave){

	zval *disable_events, *success, *exists, event_name;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 3, 0, &disable_events, &success, &exists);

	if (PHALCON_IS_TRUE(success)) {
		if (!zend_is_true(disable_events)) {
			if (PHALCON_IS_TRUE(exists)) {
				ZVAL_STRING(&event_name, "afterUpdate");
			} else {
				ZVAL_STRING(&event_name, "afterCreate");
			}
			PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);

			ZVAL_STRING(&event_name, "afterSave");
			PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
		}

		RETURN_CTOR(success);
	}
	if (!zend_is_true(disable_events)) {
		ZVAL_STRING(&event_name, "notSave");
		PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
	}

	PHALCON_CALL_METHOD(NULL, getThis(), "_canceloperation", disable_events);
	RETURN_MM_FALSE;
}

/**
 * Executes validators on every validation call
 *
 *<code>
 *use Phalcon\Mvc\Collection\Validator\ExclusionIn as ExclusionIn;
 *
 *class Subscriptors extends Phalcon\Mvc\Collection
 *{
 *
 *	public function validation()
 *	{
 *		$this->validate(new ExclusionIn(array(
 *			'field' => 'status',
 *			'domain' => array('A', 'I')
 *		)));
 *		if ($this->validationHasFailed() == true) {
 *			return false;
 *		}
 *	}
 *
 *}
 *</code>
 *
 * @param object $validator
 */
PHP_METHOD(Phalcon_Mvc_Collection, validate){

	zval *validator, status, messages, *message;

	phalcon_fetch_params(0, 1, 0, &validator);

	if (Z_TYPE_P(validator) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_collection_exception_ce, "Validator must be an Object");
		return;
	}

	PHALCON_CALL_METHOD(&status, validator, "validate", getThis());
	if (PHALCON_IS_FALSE(&status)) {
		PHALCON_CALL_METHOD(&messages, validator, "getmessages");

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(messages), message) {
			phalcon_update_property_array_append(getThis(), SL("_errorMessages"), message);
		} ZEND_HASH_FOREACH_END();

	}
}

/**
 * Check whether validation process has generated any messages
 *
 *<code>
 *use Phalcon\Mvc\Collection\Validator\ExclusionIn as ExclusionIn;
 *
 *class Subscriptors extends Phalcon\Mvc\Collection
 *{
 *
 *	public function validation()
 *	{
 *		$this->validate(new ExclusionIn(array(
 *			'field' => 'status',
 *			'domain' => array('A', 'I')
 *		)));
 *		if ($this->validationHasFailed() == true) {
 *			return false;
 *		}
 *	}
 *
 *}
 *</code>
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Collection, validationHasFailed){

	zval *error_messages;

	PHALCON_MM_GROW();

	error_messages = phalcon_read_property(getThis(), SL("_errorMessages"), PH_NOISY);
	if (Z_TYPE_P(error_messages) == IS_ARRAY) {
		if (phalcon_fast_count_ev(error_messages)) {
			RETURN_MM_TRUE;
		}
	}

	RETURN_MM_FALSE;
}

/**
 * Fires an internal event
 *
 * @param string $eventName
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Collection, fireEvent){

	zval *eventname, *collection_manager;

	zval *lower;
	char *tmp;

	phalcon_fetch_params(0, 1, 0, &eventname);
	PHALCON_ENSURE_IS_STRING(eventname);

	PHALCON_MM_GROW();

	PHALCON_INIT_VAR(lower);
	tmp = zend_str_tolower_dup(Z_STRVAL_P(eventname), Z_STRLEN_P(eventname));
	ZVAL_STRINGL(lower, tmp, Z_STRLEN_P(eventname));

	/**
	 * Check if there is a method with the same name of the event
	 */
	if (phalcon_method_exists(getThis(), lower) == SUCCESS) {
		PHALCON_CALL_METHOD(NULL, getThis(), Z_STRVAL_P(lower));
	}

	/**
	 * Send a notification to the events manager
	 */
	collection_manager = phalcon_read_property(getThis(), SL("_collectionManager"), PH_NOISY);
	PHALCON_RETURN_CALL_METHOD(collection_manager, "notifyevent", eventname, getThis());
	RETURN_MM();
}

/**
 * Fires an internal event that cancels the operation
 *
 * @param string $eventName
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Collection, fireEventCancel){

	zval *eventname, lower, status, *collection_manager;

	phalcon_fetch_params(0, 1, 0, &eventname);

	phalcon_fast_strtolower(&lower, eventname);

	/**
	 * Check if there is a method with the same name of the event
	 */
	if (phalcon_method_exists(getThis(), &lower) == SUCCESS) {
		PHALCON_CALL_METHODW(&status, getThis(), Z_STRVAL(lower));
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}
	}

	/**
	 * Send a notification to the events manager
	 */
	collection_manager = phalcon_read_property(getThis(), SL("_collectionManager"), PH_NOISY);

	PHALCON_CALL_METHODW(&status, collection_manager, "notifyevent", eventname, getThis());
	if (PHALCON_IS_FALSE(&status)) {
		RETURN_MM_FALSE;
	}

	RETURN_TRUE;
}

/**
 * Cancel the current operation
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Collection, _cancelOperation){

	zval *disable_events, *operation_made, event_name;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 0, &disable_events);

	if (!zend_is_true(disable_events)) {
		operation_made = phalcon_read_property(getThis(), SL("_operationMade"), PH_NOISY);
		if (PHALCON_IS_LONG(operation_made, 3)) {
			ZVAL_STRING(&event_name, "notDeleted");
		} else {
			ZVAL_STRING(&event_name, "notSaved");
		}

		PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
	}
	RETURN_MM_FALSE;
}

/**
 * Checks if the document exists in the collection
 *
 * @param \MongoCollection $collection
 */
PHP_METHOD(Phalcon_Mvc_Collection, _exists){

	zval *collection, mongo_id, parameters, document_count;

	phalcon_fetch_params(0, 1, 0, &collection);

	PHALCON_CALL_SELF(&mongo_id, "getid");

	if (zend_is_true(&mongo_id)) {
		array_init_size(&parameters, 1);
		phalcon_array_update_str(&parameters, SL("_id"), &mongo_id, PH_COPY);

		/**
		 * Perform the count using the function provided by the driver
		 */
		PHALCON_CALL_METHODW(&document_count, collection, "count", &parameters);

		is_smaller_function(return_value, &PHALCON_GLOBAL(z_zero), &document_count);
		return;
	}

	RETURN_FALSE;
}

/**
 * Returns all the validation messages
 *
 * <code>
 *$robot = new Robots();
 *$robot->type = 'mechanical';
 *$robot->name = 'Astro Boy';
 *$robot->year = 1952;
 *if ($robot->save() == false) {
 *	echo "Umh, We can't store robots right now ";
 *	foreach ($robot->getMessages() as $message) {
 *		echo $message;
 *	}
 *} else {
 *	echo "Great, a new robot was saved successfully!";
 *}
 * </code>
 *
 * @return Phalcon\Mvc\Collection\MessageInterface[]
 */
PHP_METHOD(Phalcon_Mvc_Collection, getMessages){


	RETURN_MEMBER(getThis(), "_errorMessages");
}

/**
 * Appends a customized message on the validation process
 *
 *<code>
 *	use \Phalcon\Mvc\Collection\Message as Message;
 *
 *	class Robots extends Phalcon\Mvc\Collection
 *	{
 *
 *		public function beforeSave()
 *		{
 *			if ($this->name == 'Peter') {
 *				$message = new Message("Sorry, but a robot cannot be named Peter");
 *				$this->appendMessage($message);
 *			}
 *		}
 *	}
 *</code>
 *
 * @param Phalcon\Mvc\Collection\MessageInterface|string $message
 */
PHP_METHOD(Phalcon_Mvc_Collection, appendMessage){

	zval *message, *field = NULL, *type = NULL, *code = NULL, *collection_message;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 3, &message, &field, &type, &code);

	if (!field) {
		field = &PHALCON_GLOBAL(z_null);
	}

	if (!type) {
		type = &PHALCON_GLOBAL(z_null);
	}

	if (!code) {
		code = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(message) != IS_OBJECT) {

		PHALCON_INIT_VAR(collection_message);
		object_init_ex(collection_message, phalcon_mvc_collection_message_ce);
		PHALCON_CALL_METHOD(NULL, collection_message, "__construct", message, field, type, code);

		phalcon_update_property_array_append(getThis(), SL("_errorMessages"), collection_message);
	} else {
		phalcon_update_property_array_append(getThis(), SL("_errorMessages"), message);
	}

	PHALCON_MM_RESTORE();
}

/**
 * Creates/Updates a collection based on the values in the attributes
 *
 * @param array $data
 * @param array $whiteList
 * @param boolean $mode true is insert or false is update
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Collection, save){

	zval *arr = NULL, *white_list = NULL, *m = NULL, mode, *dependency_injector, column_map, attributes, reserved, *new_value;
	zval source, connection, mongo_collection, exists, empty_array, *disable_events;
	zval type, message, collection_message, messages, status, data, attribute_field;
	zval *value = NULL, success, options, ok, id, func;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 0, 3, &arr, &white_list, &m);

	if (!arr) {
		arr = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(arr) != IS_NULL) {
		if (Z_TYPE_P(arr) != IS_ARRAY) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_collection_exception_ce, "Data passed to save() must be an array");
			return;
		}
	}

	if (!white_list) {
		white_list = &PHALCON_GLOBAL(z_null);
	}

	if (!m) {
		ZVAL_NULL(&mode);
	} else {
		ZVAL_COPY_VALUE(&mode, m);
	}

	dependency_injector = phalcon_read_property(getThis(), SL("_dependencyInjector"), PH_NOISY);
	if (Z_TYPE_P(dependency_injector) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_collection_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
		return;
	}

	PHALCON_CALL_SELF(&column_map, "getcolumnmap", getThis());
	if (Z_TYPE(column_map) != IS_ARRAY) {
		PHALCON_CALL_FUNCTION(&attributes, "get_object_vars", getThis());
	} else {
		ZVAL_COPY(&attributes, &column_map);
	}

	PHALCON_CALL_METHOD(&reserved, getThis(), "getreservedattributes");

	if ( Z_TYPE_P(arr) == IS_ARRAY) {
		/**
		 * We only assign values to the public properties
		 */
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(arr), idx, str_key, new_value) {
			zval tmp, attribute_field, possible_setter;
			if (str_key) {
				ZVAL_STR(&tmp, str_key);
			} else {
				ZVAL_LONG(&tmp, idx);
			}

			if (phalcon_array_isset(&reserved, &tmp)) {
				continue;
			}

			if (Z_TYPE_P(white_list) == IS_ARRAY && !phalcon_fast_in_array(&tmp, white_list)) {
				continue;
			}

			if (Z_TYPE(column_map) == IS_ARRAY) {
				if (phalcon_array_isset_fetch(&attribute_field, &column_map, &tmp)) {
					PHALCON_CONCAT_SV(&possible_setter, "set", &attribute_field);
					zend_str_tolower(Z_STRVAL(possible_setter), Z_STRLEN(possible_setter));
					if (phalcon_method_exists(getThis(), &possible_setter) == SUCCESS) {
						PHALCON_CALL_METHOD(NULL, getThis(), Z_STRVAL(possible_setter), new_value);
					} else {
						phalcon_update_property_zval_zval(getThis(), &attribute_field, new_value);
					}
				} else if (phalcon_fast_in_array(&tmp, &column_map)) {
					PHALCON_CONCAT_SV(&possible_setter, "set", &tmp);
					zend_str_tolower(Z_STRVAL(possible_setter), Z_STRLEN(possible_setter));
					if (phalcon_method_exists(getThis(), &possible_setter) == SUCCESS) {
						PHALCON_CALL_METHOD(NULL, getThis(), Z_STRVAL(possible_setter), new_value);
					} else {
						phalcon_update_property_zval_zval(getThis(), &tmp, new_value);
					}
				}
			} else if (Z_TYPE(attributes) == IS_ARRAY && phalcon_array_isset(&tmp, &attributes)) {
				PHALCON_CONCAT_SV(&possible_setter, "set", &tmp);
				zend_str_tolower(Z_STRVAL(possible_setter), Z_STRLEN(possible_setter));
				if (phalcon_method_exists(getThis(), &possible_setter) == SUCCESS) {
					PHALCON_CALL_METHOD(NULL, getThis(), Z_STRVAL(possible_setter), new_value);
				} else {
					phalcon_update_property_zval_zval(getThis(), &tmp, new_value);
				}
			}
		} ZEND_HASH_FOREACH_END();
	}

	PHALCON_CALL_METHOD(&source, getThis(), "getsource");

	if (PHALCON_IS_EMPTY(&source)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_collection_exception_ce, "Method getSource() returns empty string");
		return;
	}

	PHALCON_CALL_METHOD(&connection, getThis(), "getconnection");

	/**
	 * Choose a collection according to the collection name
	 */
	PHALCON_CALL_METHOD(&mongo_collection, &connection, "selectcollection", &source);

	/**
	 * Check the dirty state of the current operation to update the current operation
	 */
	if (Z_TYPE(mode) == IS_NULL) {
		PHALCON_CALL_METHOD(&exists, getThis(), "_exists", &mongo_collection);

		ZVAL_BOOL(&mode, (PHALCON_IS_FALSE(&exists) ? 1 : 0));
		phalcon_update_property_long(getThis(), SL("_operationMade"), (PHALCON_IS_FALSE(&exists) ? 1 : 2));
	} else {		
		PHALCON_CALL_METHOD(&exists, getThis(), "_exists", &mongo_collection);
		if (PHALCON_IS_FALSE(&mode)) {
			if (!zend_is_true(&exists)) {
				ZVAL_STRING(&type, "InvalidUpdateAttempt");
				ZVAL_STRING(&message, "Document cannot be updated because it does not exist");

				object_init_ex(&collection_message, phalcon_mvc_collection_message_ce);
				PHALCON_CALL_METHOD(NULL, &collection_message, "__construct", &message, &PHALCON_GLOBAL(z_null), &type);

				array_init_size(&messages, 1);
				phalcon_array_append(&messages, &collection_message, PH_COPY);
				phalcon_update_property_this(getThis(), SL("_errorMessages"), &messages);
				RETURN_MM_FALSE;
			}
		} else {
			if (zend_is_true(&exists)) {
				ZVAL_STRING(&type, "InvalidCreateAttempt");
				ZVAL_STRING(&message, "Document cannot be created because it already exist");

				object_init_ex(&collection_message, phalcon_mvc_collection_message_ce);
				PHALCON_CALL_METHOD(NULL, &collection_message, "__construct", &message, &PHALCON_GLOBAL(z_null), &type);

				array_init_size(&messages, 1);
				phalcon_array_append(&messages, &collection_message, PH_COPY);
				phalcon_update_property_this(getThis(), SL("_errorMessages"), &messages);
				RETURN_MM_FALSE;
			}
		}

		phalcon_update_property_long(getThis(), SL("_operationMade"), (!PHALCON_IS_FALSE(&mode) ? 1 : 2));
	}

	array_init(&empty_array);

	/**
	 * The messages added to the validator are reset here
	 */
	phalcon_update_property_this(getThis(), SL("_errorMessages"), &empty_array);

	disable_events = phalcon_read_static_property_ce(phalcon_mvc_collection_ce, SL("_disableEvents"));

	/**
	 * Execute the preSave hook
	 */
	PHALCON_CALL_METHOD(&status, getThis(), "_presave", dependency_injector, disable_events, &exists);
	if (PHALCON_IS_FALSE(&status)) {
		RETURN_MM_FALSE;
	}

	array_init(&data);

	if (Z_TYPE(attributes) == IS_ARRAY) {
		ZEND_HASH_FOREACH_KEY(Z_ARRVAL(attributes), idx, str_key) {
			zval tmp;
			if (str_key) {
				ZVAL_STR(&tmp, str_key);
			} else {
				ZVAL_LONG(&tmp, idx);
			}

			if (phalcon_array_isset(&reserved, &tmp)) {
				continue;
			}

			if (Z_TYPE(column_map) == IS_ARRAY) { 
				if (!phalcon_array_isset_fetch(&attribute_field, &column_map, &tmp)) {
					ZVAL_COPY(&attribute_field, &tmp);
				}
			} else {
				ZVAL_COPY(&attribute_field, &tmp);
			}

			if (phalcon_isset_property_zval(getThis(), &attribute_field)) {
				value = phalcon_read_property_zval(getThis(), &attribute_field, PH_NOISY);

				phalcon_array_update_zval(&data, &tmp, value, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();
	}

	if (PHALCON_IS_FALSE(&mode)){
		ZVAL_STRING(&attribute_field, "_id");

		PHALCON_CALL_SELF(&id, "getid");
		phalcon_array_update_zval(&data, &attribute_field, &id, PH_COPY);
		ZVAL_STRING(&func, "save");
	} else {
		ZVAL_STRING(&func, "insert");
	}

	/**
	 * We always use safe stores to get the success state
	 */
	array_init_size(&options, 1);

	phalcon_array_update_str_long(&options, SL("w"), 1, 0);

	/**
	 * Save the document
	 */
	PHALCON_CALL_ZVAL_METHOD(NULL, &mongo_collection, &func, &data, &options);

	ZVAL_FALSE(&success);

	if (phalcon_array_isset_fetch_str(&ok, &status, SL("ok"))) {
		if (zend_is_true(&ok)) {
			ZVAL_TRUE(&success);
			if (PHALCON_IS_FALSE(&exists) && phalcon_array_isset_fetch_str(&id, &data, SL("_id"))) {
				ZVAL_STRING(&attribute_field, "_id");

				if (Z_TYPE(column_map) == IS_ARRAY) { 
					if (phalcon_array_isset_str(&column_map, SL("_id"))) {
						phalcon_array_fetch_str(&attribute_field, &column_map, SL("_id"), PH_NOISY);
					}
				}

				phalcon_update_property_zval_zval(getThis(), &attribute_field, &id);
			}
		}
	}

	/**
	 * Call the postSave hooks
	 */
	PHALCON_RETURN_CALL_METHOD(getThis(), "_postsave", disable_events, &success, &exists);
	RETURN_MM();
}

/**
 * Find a document by its id (_id)
 *
 * @param string|\MongoId $id
 * @return Phalcon\Mvc\Collection
 */
PHP_METHOD(Phalcon_Mvc_Collection, findById){

	zval *id, class_name, collection, collection_manager, use_implicit_ids, mongo_id, conditions, parameters;
	zend_class_entry *ce0, *ce1;

	phalcon_fetch_params(0, 1, 0, &id);

	if (Z_TYPE_P(id) != IS_OBJECT) {
		phalcon_get_called_class(&class_name );
		ce0 = phalcon_fetch_class(&class_name);

		object_init_ex(&collection, ce0);
		if (phalcon_has_constructor(&collection)) {
			PHALCON_CALL_METHODW(NULL, &collection, "__construct");
		}

		PHALCON_CALL_METHODW(&collection_manager, &collection, "getcollectionmanager");

		/**
		 * Check if the collection use implicit ids
		 */
		PHALCON_CALL_METHODW(&use_implicit_ids, &collection_manager, "isusingimplicitobjectids", &collection);
		if (zend_is_true(&use_implicit_ids)) {
			ce1 = zend_fetch_class(SSL("MongoId"), ZEND_FETCH_CLASS_AUTO);
			object_init_ex(&mongo_id, ce1);
			PHALCON_CALL_METHODW(NULL, &mongo_id, "__construct", id);
		} else {
			ZVAL_COPY(&mongo_id, id);
		}
	} else {
		ZVAL_COPY(&mongo_id, id);
	}

	array_init_size(&conditions, 1);
	phalcon_array_update_str(&conditions, SL("_id"), &mongo_id, PH_COPY);

	array_init_size(&parameters, 1);
	phalcon_array_append(&parameters, &conditions, PH_COPY);

	PHALCON_RETURN_CALL_SELFW("findfirst", &parameters);
}

/**
 * Allows to query the first record that match the specified conditions
 *
 * <code>
 *
 * //What's the first robot in the robots table?
 * $robot = Robots::findFirst();
 * echo "The robot name is ", $robot->name, "\n";
 *
 * //What's the first mechanical robot in robots table?
 * $robot = Robots::findFirst(array(
 *     array("type" => "mechanical")
 * ));
 * echo "The first mechanical robot name is ", $robot->name, "\n";
 *
 * //Get first virtual robot ordered by name
 * $robot = Robots::findFirst(array(
 *     array("type" => "mechanical"),
 *     "order" => array("name" => 1)
 * ));
 * echo "The first virtual robot name is ", $robot->name, "\n";
 *
 * </code>
 *
 * @param array $parameters
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Collection, findFirst){

	zval *parameters = NULL, class_name, collection, collection_manager;
	zval use_implicit_ids, mongo_id, conditions, params, connection;
	zend_class_entry *ce0, *ce1;

	phalcon_fetch_params(0, 0, 1, &parameters);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	phalcon_get_called_class(&class_name );
	ce0 = phalcon_fetch_class(&class_name);

	object_init_ex(&collection, ce0);
	if (phalcon_has_constructor(&collection)) {
		PHALCON_CALL_METHODW(NULL, &collection, "__construct");
	}

	if (Z_TYPE_P(parameters) != IS_NULL && Z_TYPE_P(parameters) != IS_ARRAY) {
		if (Z_TYPE_P(parameters) != IS_OBJECT) {
			PHALCON_CALL_METHODW(&collection_manager, &collection, "getcollectionmanager");

			/**
			 * Check if the collection use implicit ids
			 */
			PHALCON_CALL_METHODW(&use_implicit_ids, &collection_manager, "isusingimplicitobjectids", &collection);
			if (zend_is_true(&use_implicit_ids)) {
				ce1 = zend_fetch_class(SSL("MongoId"), ZEND_FETCH_CLASS_AUTO);

				object_init_ex(&mongo_id, ce1);
				PHALCON_CALL_METHODW(NULL, &mongo_id, "__construct", parameters);
			} else {
				ZVAL_COPY(&mongo_id, parameters);
			}
		} else {
			ZVAL_COPY(&mongo_id, parameters);
		}

		array_init_size(&conditions, 1);
		phalcon_array_update_str(&conditions, SL("_id"), &mongo_id, PH_COPY);

		array_init_size(&params, 1);
		phalcon_array_append(&params, &conditions, PH_COPY);
	} else {
		ZVAL_COPY(&params, parameters);
	}

	PHALCON_CALL_METHODW(&connection, &collection, "getconnection");

	PHALCON_RETURN_CALL_SELFW("_getresultset", &params, &collection, &connection, &PHALCON_GLOBAL(z_true));
}

/**
 * Allows to query a set of records that match the specified conditions
 *
 * <code>
 *
 * //How many robots are there?
 * $robots = Robots::find();
 * echo "There are ", count($robots), "\n";
 *
 * //How many mechanical robots are there?
 * $robots = Robots::find(array(
 *     array("type" => "mechanical")
 * ));
 * echo "There are ", count($robots), "\n";
 *
 * //Get and print virtual robots ordered by name
 * $robots = Robots::findFirst(array(
 *     array("type" => "virtual"),
 *     "order" => array("name" => 1)
 * ));
 * foreach ($robots as $robot) {
 *	   echo $robot->name, "\n";
 * }
 *
 * //Get first 100 virtual robots ordered by name
 * $robots = Robots::find(array(
 *     array("type" => "virtual"),
 *     "order" => array("name" => 1),
 *     "limit" => 100
 * ));
 * foreach ($robots as $robot) {
 *	   echo $robot->name, "\n";
 * }
 * </code>
 *
 * @param 	array $parameters
 * @return  array
 */
PHP_METHOD(Phalcon_Mvc_Collection, find){

	zval *parameters = NULL, class_name, collection, connection;
	zend_class_entry *ce0;

	phalcon_fetch_params(0, 0, 1, &parameters);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(parameters) != IS_NULL) {
		if (Z_TYPE_P(parameters) != IS_ARRAY) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_collection_exception_ce, "Invalid parameters for find");
			return;
		}
	}

	phalcon_get_called_class(&class_name);
	ce0 = phalcon_fetch_class(&class_name);

	object_init_ex(&collection, ce0);
	if (phalcon_has_constructor(&collection)) {
		PHALCON_CALL_METHODW(NULL, &collection, "__construct");
	}

	PHALCON_CALL_METHODW(&connection, &collection, "getconnection");

	PHALCON_RETURN_CALL_SELFW("_getresultset", parameters, &collection, &connection, &PHALCON_GLOBAL(z_false));
}

/**
 * Perform a count over a collection
 *
 *<code>
 * echo 'There are ', Robots::count(), ' robots';
 *</code>
 *
 * @param array $parameters
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Collection, count){

	zval *parameters = NULL, class_name, collection, connection;
	zend_class_entry *ce0;

	phalcon_fetch_params(0, 0, 1, &parameters);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	} else if (Z_TYPE_P(parameters) != IS_NULL && Z_TYPE_P(parameters) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_collection_exception_ce, "Invalid parameters for count");
		return;
	}

	phalcon_get_called_class(&class_name);
	ce0 = phalcon_fetch_class(&class_name);

	object_init_ex(&collection, ce0);
	if (phalcon_has_constructor(&collection)) {
		PHALCON_CALL_METHODW(NULL, &collection, "__construct");
	}

	PHALCON_CALL_METHODW(&connection, &collection, "getconnection");
	PHALCON_RETURN_CALL_SELF("_getgroupresultset", parameters, &collection, &connection);
}

/**
 * Perform an aggregation using the Mongo aggregation framework
 *
 * @param array $parameters
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Collection, aggregate){

	zval *parameters, class_name, collection, connection, source, mongo_collection;
	zend_class_entry *ce0;

	phalcon_fetch_params(0, 1, 0, &parameters);

	if (Z_TYPE_P(parameters) != IS_NULL) {
		if (Z_TYPE_P(parameters) != IS_ARRAY) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_collection_exception_ce, "Invalid parameters for aggregate");
			return;
		}
	}

	phalcon_get_called_class(&class_name);
	ce0 = phalcon_fetch_class(&class_name);

	object_init_ex(&collection, ce0);
	if (phalcon_has_constructor(&collection)) {
		PHALCON_CALL_METHODW(NULL, &collection, "__construct");
	}

	PHALCON_CALL_METHODW(&connection, &collection, "getconnection");
	PHALCON_CALL_METHODW(&source, &collection, "getsource");

	if (PHALCON_IS_EMPTY(&source)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_collection_exception_ce, "Method getSource() returns empty string");
		return;
	}

	PHALCON_CALL_METHODW(&mongo_collection, &connection, "selectcollection", &source);
	PHALCON_RETURN_CALL_METHODW(&mongo_collection, "aggregate", parameters);
}

/**
 * Allows to perform a summatory group for a column in the collection
 *
 * @param array $fields
 * @param array $condition
 * @param string $finalize
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Collection, summatory){

	zval *fields, *condition = NULL, *finalize = NULL, class_name, collection, connection, source, mongo_collection;
	zval keys, options, initial, reduce, group, retval, first_retval, summatory;
	zend_class_entry *ce0;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 2, &fields, &condition, &finalize);

	if (!condition) {
		condition = &PHALCON_GLOBAL(z_null);
	}

	if (!finalize) {
		finalize = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(fields) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_collection_exception_ce, "Invalid fields for group");
		return;
	}

	phalcon_get_called_class(&class_name);
	ce0 = phalcon_fetch_class(&class_name);

	object_init_ex(&collection, ce0);
	if (phalcon_has_constructor(&collection)) {
		PHALCON_CALL_METHOD(NULL, &collection, "__construct");
	}

	PHALCON_CALL_METHOD(&connection, &collection, "getconnection");

	PHALCON_CALL_METHOD(&source, &collection, "getsource");
	if (PHALCON_IS_EMPTY(&source)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_collection_exception_ce, "Method getSource() returns empty string");
		return;
	}

	PHALCON_CALL_METHOD(&mongo_collection, &connection, "selectcollection", &source);

	array_init(&keys);
	array_init(&options);

	if (!PHALCON_IS_EMPTY(condition)) {
		phalcon_array_update_str(&options, SL("condition"), condition, PH_COPY);
	}

	if (!PHALCON_IS_EMPTY(finalize)) {
		phalcon_array_update_str(&options, SL("finalize"), finalize, PH_COPY);
	}

	/**
	 * Uses a javascript hash to group the results
	 */
	array_init_size(&initial, 1);
	phalcon_array_update_str(&initial, SL("summatory"), fields, PH_COPY);

	/**
	 * Uses a javascript hash to group the results, however this is slow with larger
	 * datasets
	 */
	ZVAL_STRING(&reduce, "function (curr, result) { for (var key in result.summatory) {if (typeof curr[key] !== \"undefined\") { if (typeof curr[key] === \"string\") {result.summatory[key] += curr[key].trim().length > 0 ? parseFloat(curr[key].trim()) : 0;} else {result.summatory[key] += curr[key];} } }}");

	PHALCON_CALL_METHOD(&group, &mongo_collection, "group", &keys, &initial, &reduce, &options);
	if (phalcon_array_isset_str(&group, SL("retval"))) {
		phalcon_array_fetch_str(&retval, &group, SL("retval"), PH_NOISY);
		if (phalcon_array_isset_fetch_long(&first_retval, &retval, 0)) {
			if (phalcon_array_isset_fetch_str(&summatory, &first_retval, SL("summatory"))) {
				RETURN_CTOR(&summatory);
			}

			RETURN_CTOR(&first_retval);
		}

		RETURN_CTOR(&retval);
	}

	PHALCON_MM_RESTORE();
}

/**
 * Creates a document based on the values in the attributes
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Collection, create){

	zval *data = NULL, *white_list = NULL;

	phalcon_fetch_params(0, 0, 2, &data, &white_list);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!white_list) {
		white_list = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_RETURN_CALL_METHODW(getThis(), "save", data, white_list, &PHALCON_GLOBAL(z_true));
}

/**
 * Updates a document based on the values in the attributes
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Collection, update){

	zval *data = NULL, *white_list = NULL;

	phalcon_fetch_params(0, 0, 2, &data, &white_list);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!white_list) {
		white_list = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_RETURN_CALL_METHODW(getThis(), "save", data, white_list, &PHALCON_GLOBAL(z_false));
}

/**
 * Deletes a collection instance. Returning true on success or false otherwise.
 *
 * <code>
 *
 *	$robot = Robots::findFirst();
 *	$robot->delete();
 *
 *	foreach (Robots::find() as $robot) {
 *		$robot->delete();
 *	}
 * </code>
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Collection, delete){

	zval mongo_id, *disable_events, event_name, status, connection, source, mongo_collection, id_condition, success, options, ok;

	PHALCON_CALL_SELF(&mongo_id, "getid");

	if (!zend_is_true(&mongo_id)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_collection_exception_ce, "The document cannot be deleted because it doesn't exist");
		return;
	}

	disable_events = phalcon_read_static_property_ce(phalcon_mvc_collection_ce, SL("_disableEvents"));
	if (!zend_is_true(disable_events)) {
		ZVAL_STRING(&event_name, "beforeDelete");

		PHALCON_CALL_METHODW(&status, getThis(), "fireeventcancel", &event_name);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}
	}

	PHALCON_CALL_METHODW(&connection, getThis(), "getconnection");

	PHALCON_CALL_METHODW(&source, getThis(), "getsource");
	if (PHALCON_IS_EMPTY(&source)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_collection_exception_ce, "Method getSource() returns empty string");
		return;
	}

	/**
	 * Get the \MongoCollection
	 */
	PHALCON_CALL_METHODW(&mongo_collection, &connection, "selectcollection", &source);

	array_init_size(&id_condition, 1);
	phalcon_array_update_str(&id_condition, SL("_id"), &mongo_id, PH_COPY);

	ZVAL_FALSE(&success);

	array_init_size(&options, 1);
	phalcon_array_update_str_long(&options, SL("w"), 1, 0);

	/**
	 * Remove the instance
	 */
	PHALCON_CALL_METHODW(&status, &mongo_collection, "remove", &id_condition, &options);
	if (Z_TYPE(status) != IS_ARRAY) {
		RETURN_FALSE;
	}

	/**
	 * Check the operation status
	 */
	if (phalcon_array_isset_fetch_str(&ok, &status, SL("ok"))) {
		if (zend_is_true(&ok)) {
			ZVAL_TRUE(&success);
			if (!zend_is_true(disable_events)) {
				ZVAL_STRING(&event_name, "afterDelete");
				PHALCON_CALL_METHODW(NULL, getThis(), "fireevent", &event_name);
			}
		}
	}

	RETURN_CTORW(&success);
}

/**
 * Returns the type of the latest operation performed by the ODM
 * Returns one of the OP_* class constants
 *
 * @return int
 */
PHP_METHOD(Phalcon_Mvc_Collection, getOperationMade){


	RETURN_MEMBER(getThis(), "_operationMade");
}

/**
 * Returns the instance as an array representation
 *
 *<code>
 * print_r($robot->toArray());
 *</code>
 *
 * @param array $columns
 * @param bool $renameColumns
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Collection, toArray){

	zval *columns = NULL, *rename_columns = NULL, *allow_empty = NULL, data, reserved, attributes, column_map;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 0, 3, &columns, &rename_columns, &allow_empty);

	if (!rename_columns) {
		rename_columns = &PHALCON_GLOBAL(z_true);
	}

	if (!allow_empty) {
		allow_empty = &PHALCON_GLOBAL(z_false);
	}

	array_init(&data);

	PHALCON_CALL_METHODW(&reserved, getThis(), "getreservedattributes");

	/**
	 * Get an array with the values of the object
	 */
	PHALCON_CALL_SELFW(&column_map, "getcolumnmap", getThis());
	if (Z_TYPE(column_map) != IS_ARRAY) { 
		PHALCON_CALL_FUNCTIONW(&attributes, "get_object_vars", getThis());
	} else {
		ZVAL_COPY(&attributes, &column_map);
	}

	/**
	 * We only assign values to the public properties
	 */
	ZEND_HASH_FOREACH_KEY(Z_ARRVAL(attributes), idx, str_key) {
		zval tmp, attribute_field, *field_value;
		if (str_key) {
			ZVAL_STR(&tmp, str_key);
		} else {
			ZVAL_LONG(&tmp, idx);
		}

		if (phalcon_array_isset(&reserved, &tmp)) {
			continue;
		}

		if (zend_is_true(rename_columns) && Z_TYPE(column_map) == IS_ARRAY) { 
			if (!phalcon_array_isset_fetch(&attribute_field, &column_map, &tmp)) {
				ZVAL_COPY(&attribute_field, &tmp);
			}
		} else {
			ZVAL_COPY(&attribute_field, &tmp);
		}

		if (phalcon_isset_property_zval(getThis(), &attribute_field)) {
			field_value = phalcon_read_property_zval(getThis(), &attribute_field, PH_NOISY);

			if (zend_is_true(allow_empty)) {
				phalcon_array_update_zval(&data, &attribute_field, field_value, PH_COPY);
			} else if (Z_TYPE_P(field_value) != IS_NULL) {
				phalcon_array_update_zval(&data, &attribute_field, field_value, PH_COPY);
			}
		}
	} ZEND_HASH_FOREACH_END();

	RETURN_CTORW(&data);
}

/**
 * Serializes the object ignoring connections or protected properties
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Collection, serialize){

	zval data;

	PHALCON_CALL_METHODW(&data, getThis(), "toarray");

	/**
	 * Use the standard serialize function to serialize the array data
	 */
	phalcon_serialize(return_value, &data);
}

/**
 * Unserializes the object from a serialized string
 *
 * @param string $data
 */
PHP_METHOD(Phalcon_Mvc_Collection, unserialize){

	zval *data, attributes, dependency_injector, service, manager, *value;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 1, 0, &data);

	if (Z_TYPE_P(data) == IS_STRING) {
		phalcon_unserialize(&attributes, data);
		if (Z_TYPE(attributes) == IS_ARRAY) {
			/**
			 * Obtain the default DI
			 */
			PHALCON_CALL_CE_STATICW(&dependency_injector, phalcon_di_ce, "getdefault");

			if (Z_TYPE(dependency_injector) != IS_OBJECT) {
				PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_collection_exception_ce, "A dependency injector container is required to obtain the services related to the ODM");
				return;
			}

			/**
			 * Update the dependency injector
			 */
			phalcon_update_property_this(getThis(), SL("_dependencyInjector"), &dependency_injector);

			/**
			 * Gets the default collectionManager service
			 */
			ZVAL_STRING(&service, "collectionManager");

			PHALCON_CALL_METHODW(&manager, &dependency_injector, "getshared", &service);
			if (Z_TYPE(manager) != IS_OBJECT) {
				PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_collection_exception_ce, "The injected service 'collectionManager' is not valid");
				return;
			}

			PHALCON_VERIFY_INTERFACEW(&manager, phalcon_mvc_collection_managerinterface_ce);

			/**
			 * Update the collection manager
			 */
			phalcon_update_property_this(getThis(), SL("_collectionManager"), &manager);

			/**
			 * Update the objects attributes
			 */
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(attributes), idx, str_key, value) {
				zval tmp;
				if (str_key) {
					ZVAL_STR(&tmp, str_key);
				} else {
					ZVAL_LONG(&tmp, idx);
				}
				phalcon_update_property_zval_zval(getThis(), &tmp, value);
			} ZEND_HASH_FOREACH_END();

			RETURN_MM_NULL();
		}
	}
	PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_collection_exception_ce, "Invalid serialization data");
}

/**
 * Runs JavaScript code on the database server.
 *
 * <code>
 *
 * $ret = Robots::execute("function() { return 'Hello, world!';}");
 * echo $ret['retval'], "\n";
 *
 * </code>
 *
 * @param mixed $code
 * @param array $args
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Collection, execute){

	zval *code, *args = NULL, class_name, collection, connection;
	zend_class_entry *ce0;

	phalcon_fetch_params(0, 1, 1, &code, &args);

	if (args && Z_TYPE_P(args) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_collection_exception_ce, "Invalid args for execute");
		return;
	}

	phalcon_get_called_class(&class_name );
	ce0 = phalcon_fetch_class(&class_name);

	object_init_ex(&collection, ce0);
	if (phalcon_has_constructor(&collection)) {
		PHALCON_CALL_METHODW(NULL, &collection, "__construct");
	}

	PHALCON_CALL_METHODW(&connection, &collection, "getconnection");

	if (args) {
		PHALCON_RETURN_CALL_METHODW(&connection, "execute", code, args);
	} else {
		PHALCON_RETURN_CALL_METHODW(&connection, "execute", code);
	}
}

PHP_METHOD(Phalcon_Mvc_Collection, incr){

	zval *field, *v = NULL, value, mongo_id, source, connection, mongo_collection; 
	zval criteria, new_object, key, options, status;

	phalcon_fetch_params(0, 1, 1, &field, &v);

	if (!phalcon_isset_property_zval(getThis(), field)) {
		RETURN_FALSE;
	}

	if (!v) {
		ZVAL_LONG(&value, 1);
	} else {
		ZVAL_COPY(&value, v);
	}

	PHALCON_CALL_SELF(&mongo_id, "getid");

	if (!zend_is_true(&mongo_id)) {
		RETURN_FALSE;
	}

	PHALCON_CALL_METHODW(&source, getThis(), "getsource");
	PHALCON_CALL_METHODW(&connection, getThis(), "getconnection");
	PHALCON_CALL_METHODW(&mongo_collection, connection, "selectcollection", source);

	if (Z_TYPE(mongo_collection) == IS_OBJECT) {
		array_init_size(&criteria, 1);

		phalcon_array_update_str(&criteria, SL("_id"), &mongo_id, PH_COPY);

		ZVAL_STRING(&key, "$inc");

		array_init_size(&new_object, 1);

		phalcon_array_update_multi_2(&new_object, &key, field, v, PH_COPY);

		array_init_size(&options, 1);

		phalcon_array_update_str_long(&options, SL("w"), 0, 0);
		PHALCON_CALL_METHODW(&status, mongo_collection, "update", &criteria, &new_object, &options);

		if (zend_is_true(&status)) {
			PHALCON_CALL_SELFW(NULL, "refresh");
			RETURN_TRUE;
		}
	}

	RETURN_FALSE;
}

PHP_METHOD(Phalcon_Mvc_Collection, refresh){

	zval mongo_id, source, connection, mongo_collection, criteria, row, *value;
	zend_string *str_key;
	ulong idx;

	PHALCON_CALL_SELFW(&mongo_id, "getid");

	if (!zend_is_true(mongo_id)) {
		RETURN_FALSE;
	}

	PHALCON_CALL_METHODW(&source, getThis(), "getsource");
	PHALCON_CALL_METHODW(&connection, getThis(), "getconnection");
	PHALCON_CALL_METHODW(&mongo_collection, &connection, "selectcollection", &source);

	if (Z_TYPE_P(mongo_collection) == IS_OBJECT) {
		array_init_size(&criteria, 1);

		phalcon_array_update_str(&criteria, SL("_id"), mongo_id, PH_COPY);

		PHALCON_CALL_METHODW(&row, &mongo_collection, "findone", &criteria);

		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(row), idx, str_key, value) {
			zval tmp;
			if (str_key) {
				ZVAL_STR(&tmp, str_key);
			} else {
				ZVAL_LONG(&tmp, idx);
			}
			phalcon_update_property_zval_zval(getThis(), &tmp, value);
		} ZEND_HASH_FOREACH_END();

		RETURN_TRUE;
	}

	RETURN_FALSE;
}

PHP_METHOD(Phalcon_Mvc_Collection, drop){

	zval class_name, collection, source, connection, mongo_collection, status, ok;
	zend_class_entry *ce0;

	phalcon_get_called_class(&class_name );
	ce0 = phalcon_fetch_class(&class_name);

	object_init_ex(&collection, ce0);
	if (phalcon_has_constructor(&collection)) {
		PHALCON_CALL_METHODW(NULL, &collection, "__construct");
	}

	ZVAL_FALSE(&ok);

	PHALCON_CALL_METHODW(&source, &collection, "getsource");
	PHALCON_CALL_METHODW(&connection, &collection, "getconnection");
	PHALCON_CALL_METHODW(&mongo_collection, &connection, "selectcollection", &source);

	if (Z_TYPE_P(mongo_collection) == IS_OBJECT) {
		PHALCON_CALL_METHODW(&status, mongo_collection, "drop");

		if (phalcon_array_isset_str(&status, SL("ok"))) {
			phalcon_array_fetch_str(&ok, &status, SL("ok"), PH_NOISY);
		}
	}

	if (zend_is_true(&ok)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Parses the conditions
 *
 * @param array $conditions
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Collection, parse){

	zval *conditions, column_map, collection_manager, use_implicit_ids, *value;
	zend_string *str_key;
	ulong idx;
	zend_class_entry *ce0;

	phalcon_fetch_params(0, 1, 1, &conditions);

	if (Z_TYPE_P(conditions) != IS_ARRAY) {
		RETURN_CTOR(conditions);
	}

	PHALCON_SEPARATE_PARAM(conditions);

	PHALCON_CALL_SELFW(&column_map, "getcolumnmap");
	PHALCON_CALL_SELFW(&collection_manager, "getcollectionmanager");
	PHALCON_CALL_METHODW(&use_implicit_ids, &collection_manager, "isusingimplicitobjectids", getThis());

	ce0 = zend_fetch_class(SSL("MongoId"), ZEND_FETCH_CLASS_AUTO);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(conditions), idx, str_key, value) {
		zval tmp, value1, value2, column, mongo_id;
		if (str_key) {
			ZVAL_STR(&tmp, str_key);
		} else {
			ZVAL_LONG(&tmp, idx);
		}
		if (Z_TYPE_P(value) == IS_ARRAY) {
			PHALCON_CALL_SELF(&value1, "parse", value);
			phalcon_array_update_zval(conditions, &tmp, &value1, PH_COPY);
		}

		phalcon_array_fetch(&value2, conditions, &tmp, PH_NOISY);

		if (phalcon_array_isset_fetch(&column, &column_map, &tmp)) {
			if (!PHALCON_IS_EQUAL(&column, &tmp)) {
				phalcon_array_unset(conditions, &tmp, 0);
				phalcon_array_update_zval(conditions, &column, &value2, PH_COPY);
			}
		} else {
			ZVAL_COPY_VALUE(&column, &tmp);
		}

		if (PHALCON_IS_STRING(&column, "_id")) {
			if (Z_TYPE(value2) != IS_OBJECT && Z_TYPE(value2) != IS_ARRAY) {
				if (zend_is_true(&use_implicit_ids)) {
					object_init_ex(&mongo_id, ce0);
					if (phalcon_has_constructor(&mongo_id)) {
						PHALCON_CALL_METHOD(NULL, &mongo_id, "__construct", &value2);
					}
					phalcon_array_update_zval(conditions, &column, &mongo_id, PH_COPY);
				}
			}
		}
	} ZEND_HASH_FOREACH_END();

	RETURN_CTORW(&conditions);
}


/**
 * Magic method to assign values to the the collection
 *
 * @param string $property
 * @param mixed $value
 */
PHP_METHOD(Phalcon_Mvc_Collection, __set){

	zval *property, *value, possible_setter, class_name, method_exists;

	phalcon_fetch_params(0, 2, 0, &property, &value);

	if (Z_TYPE_P(property) == IS_STRING) {
		PHALCON_CONCAT_SV(&possible_setter, "set", property);
		phalcon_strtolower_inplace(&possible_setter);

		ZVAL_STRING(&class_name, "Phalcon\\Mvc\\Collection");

		PHALCON_CALL_FUNCTIONW(&method_exists, "method_exists", &class_name, &possible_setter);
		if (!zend_is_true(&method_exists)) {
			if (phalcon_method_exists(getThis(), &possible_setter) == SUCCESS) {
				PHALCON_CALL_METHODW(NULL, getThis(), Z_STRVAL_P(possible_setter), value);
				RETURN_CTORW(value);
			}
		}

		if (phalcon_isset_property_zval(getThis(), property)) {
			phalcon_get_class(&class_name, getThis(), 0);

			if (PHALCON_PROPERTY_IS_PRIVATE_ZVAL(getThis(), property)) {
				zend_error(E_ERROR, "Cannot access private property %s::%s", Z_STRVAL(class_name), Z_STRVAL_P(property));
				return;
			}

			if (PHALCON_PROPERTY_IS_PROTECTED_ZVAL(getThis(), property)) {
				zend_error(E_ERROR, "Cannot access protected property %s::%s", Z_STRVAL(class_name), Z_STRVAL_P(property));
				return;
			}
		}
	}

	phalcon_update_property_zval_zval(getThis(), property, value);

	RETURN_CTORW(value);
}

/**
 * Magic method to get related records using the relation alias as a property
 *
 * @param string $property
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Collection, __get){

	zval *property, possible_getter, class_name, method_exists;

	phalcon_fetch_params(0, 1, 0, &property);

	if (Z_TYPE_P(property) == IS_STRING) {
		PHALCON_CONCAT_SV(&possible_getter, "get", property);
		phalcon_strtolower_inplace(&possible_getter);

		ZVAL_STRING(&class_name, "Phalcon\\Mvc\\Collection");

		PHALCON_CALL_FUNCTIONW(&method_exists, "method_exists", &class_name, &possible_getter);
		if (!zend_is_true(&method_exists)) {
			if (phalcon_method_exists(getThis(), &possible_getter) == SUCCESS) {
				PHALCON_CALL_METHODW(&return_value, getThis(), &possible_getter);
				return;
			}
		}

		if (phalcon_isset_property_zval(getThis(), property)) {
			RETURN_NULL();
		}
	}

	RETURN_NULL();
}

/**
 * Handles method calls when a static method is not implemented
 *
 * @param string $method
 * @param array $arguments
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Collection, __callStatic){

	zval *method, *arguments = NULL, *extra_method = NULL;
	zval *class_name, exception_message;
	zval *collection, *extra_method_first = NULL, *field = NULL, *value, *conditions, *params;
	zend_class_entry *ce0;
	const char *type = NULL;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 1, &method, &arguments);
	
	if (!arguments) {
		arguments = &PHALCON_GLOBAL(z_null);
	}
	
	PHALCON_INIT_VAR(extra_method);
	
	/** 
	 * Check if the method starts with 'findFirst'
	 */
	if (phalcon_start_with_str(method, SL("findFirstBy"))) {
		type = "findfirst";
		phalcon_substr(extra_method, method, 11, 0);
	}
	
	/** 
	 * Check if the method starts with 'find'
	 */
	if (Z_TYPE_P(extra_method) == IS_NULL) {
		if (phalcon_start_with_str(method, SL("findBy"))) {
			type = "find";
			phalcon_substr(extra_method, method, 6, 0);
		}
	}
	
	/** 
	 * Check if the method starts with 'count'
	 */
	if (Z_TYPE_P(extra_method) == IS_NULL) {
		if (phalcon_start_with_str(method, SL("countBy"))) {
			type = "count";
			phalcon_substr(extra_method, method, 7, 0);
		}
	}

	PHALCON_INIT_VAR(class_name);
	phalcon_get_called_class(class_name );

	if (!type) {
		PHALCON_CONCAT_SVSVS(&exception_message, "The static method \"", method, "\" doesn't exist on collection \"", class_name, "\"");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_collection_exception_ce, &exception_message);
		return;
	}

	if (!phalcon_array_isset_long(arguments, 0)) {
		PHALCON_CONCAT_SVS(&exception_message, "The static method \"", method, "\" requires one argument");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_collection_exception_ce, &exception_message);
		return;
	}

	ce0 = phalcon_fetch_class(class_name);

	PHALCON_INIT_VAR(collection);
	object_init_ex(collection, ce0);
	if (phalcon_has_constructor(collection)) {
		PHALCON_CALL_METHOD(NULL, collection, "__construct");
	}

	if (!phalcon_isset_property_zval(collection, extra_method)) {
		PHALCON_INIT_NVAR(extra_method_first);
		phalcon_lcfirst(extra_method_first, extra_method);
		if (phalcon_isset_property_zval(collection, extra_method_first)) {
			PHALCON_CPY_WRT(field, extra_method_first);
		} else {
			PHALCON_INIT_NVAR(field);
			phalcon_uncamelize(field, extra_method);
			if (!phalcon_isset_property_zval(collection, field)) {
				PHALCON_CONCAT_SVS(&exception_message, "Cannot resolve attribute \"", extra_method, "' in the collection");
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_collection_exception_ce, &exception_message);
				return;
			}
		}
	} else {
		PHALCON_CPY_WRT(field, extra_method);
	}

	PHALCON_OBS_VAR(value);
	phalcon_array_fetch_long(&value, arguments, 0, PH_NOISY);	

	PHALCON_INIT_VAR(conditions);
	array_init_size(conditions, 1);
	phalcon_array_update_zval(conditions, field, value, PH_COPY);

	PHALCON_INIT_VAR(params);
	array_init_size(params, 1);
	phalcon_array_append(params, conditions, PH_COPY);

	/** 
	 * Execute the query
	 */
	PHALCON_RETURN_CALL_CE_STATIC(ce0, type, params);
	RETURN_MM();
}
