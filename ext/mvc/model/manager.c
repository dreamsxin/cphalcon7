
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

#include "mvc/model/manager.h"
#include "mvc/model/managerinterface.h"
#include "mvc/model/exception.h"
#include "mvc/model/query.h"
#include "mvc/model/query/builder.h"
#include "mvc/model/relation.h"
#include "diinterface.h"
#include "di/injectable.h"
#include "db/adapterinterface.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/string.h"
#include "kernel/file.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/hash.h"
#include "kernel/framework/orm.h"
#include "kernel/debug.h"

#include "interned-strings.h"

/**
 * Phalcon\Mvc\Model\Manager
 *
 * This components controls the initialization of models, keeping record of relations
 * between the different models of the application.
 *
 * A ModelsManager is injected to a model via a Dependency Injector/Services Container such as Phalcon\Di.
 *
 * <code>
 * $di = new Phalcon\Di();
 *
 * $di->set('modelsManager', function() {
 *      return new Phalcon\Mvc\Model\Manager();
 * });
 *
 * $robot = new Robots($di);
 * </code>
 */
zend_class_entry *phalcon_mvc_model_manager_ce;

PHP_METHOD(Phalcon_Mvc_Model_Manager, setCustomEventsManager);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getCustomEventsManager);
PHP_METHOD(Phalcon_Mvc_Model_Manager, initialize);
PHP_METHOD(Phalcon_Mvc_Model_Manager, isInitialized);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getLastInitialized);
PHP_METHOD(Phalcon_Mvc_Model_Manager, load);
PHP_METHOD(Phalcon_Mvc_Model_Manager, setModelSource);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getModelSource);
PHP_METHOD(Phalcon_Mvc_Model_Manager, setModelSchema);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getModelSchema);
PHP_METHOD(Phalcon_Mvc_Model_Manager, setConnectionService);
PHP_METHOD(Phalcon_Mvc_Model_Manager, setWriteConnectionService);
PHP_METHOD(Phalcon_Mvc_Model_Manager, setReadConnectionService);
PHP_METHOD(Phalcon_Mvc_Model_Manager, setDefaultConnectionService);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getDefaultConnectionService);
PHP_METHOD(Phalcon_Mvc_Model_Manager, setDefaultWriteConnectionService);
PHP_METHOD(Phalcon_Mvc_Model_Manager, setDefaultReadConnectionService);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getWriteConnection);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getReadConnection);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getReadConnectionService);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getWriteConnectionService);
PHP_METHOD(Phalcon_Mvc_Model_Manager, notifyEvent);
PHP_METHOD(Phalcon_Mvc_Model_Manager, missingMethod);
PHP_METHOD(Phalcon_Mvc_Model_Manager, addBehavior);
PHP_METHOD(Phalcon_Mvc_Model_Manager, useDynamicUpdate);
PHP_METHOD(Phalcon_Mvc_Model_Manager, isUsingDynamicUpdate);
PHP_METHOD(Phalcon_Mvc_Model_Manager, addHasOne);
PHP_METHOD(Phalcon_Mvc_Model_Manager, addBelongsTo);
PHP_METHOD(Phalcon_Mvc_Model_Manager, addHasMany);
PHP_METHOD(Phalcon_Mvc_Model_Manager, addHasManyToMany);
PHP_METHOD(Phalcon_Mvc_Model_Manager, existsBelongsTo);
PHP_METHOD(Phalcon_Mvc_Model_Manager, existsHasMany);
PHP_METHOD(Phalcon_Mvc_Model_Manager, existsHasOne);
PHP_METHOD(Phalcon_Mvc_Model_Manager, existsHasManyToMany);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getRelationByAlias);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getRelationRecords);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getReusableRecords);
PHP_METHOD(Phalcon_Mvc_Model_Manager, setReusableRecords);
PHP_METHOD(Phalcon_Mvc_Model_Manager, clearReusableObjects);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getBelongsToRecords);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getHasManyRecords);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getHasOneRecords);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getBelongsTo);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getHasMany);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getHasOne);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getHasManyToMany);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getHasOneAndHasMany);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getRelations);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getRelationsBetween);
PHP_METHOD(Phalcon_Mvc_Model_Manager, createQuery);
PHP_METHOD(Phalcon_Mvc_Model_Manager, executeQuery);
PHP_METHOD(Phalcon_Mvc_Model_Manager, createBuilder);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getLastQuery);
PHP_METHOD(Phalcon_Mvc_Model_Manager, registerNamespaceAlias);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getNamespaceAlias);
PHP_METHOD(Phalcon_Mvc_Model_Manager, getNamespaceAliases);
PHP_METHOD(Phalcon_Mvc_Model_Manager, __destruct);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_setcustomeventsmanager, 0, 0, 2)
	ZEND_ARG_INFO(0, model)
	ZEND_ARG_INFO(0, eventsManager)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_getcustomeventsmanager, 0, 0, 1)
	ZEND_ARG_INFO(0, model)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_setmodelsource, 0, 0, 2)
	ZEND_ARG_INFO(0, model)
	ZEND_ARG_INFO(0, source)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_getmodelsource, 0, 0, 1)
	ZEND_ARG_INFO(0, model)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_setmodelschema, 0, 0, 2)
	ZEND_ARG_INFO(0, model)
	ZEND_ARG_INFO(0, schema)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_getmodelschema, 0, 0, 1)
	ZEND_ARG_INFO(0, model)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_setconnectionservice, 0, 0, 2)
	ZEND_ARG_INFO(0, model)
	ZEND_ARG_INFO(0, connectionService)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_setwriteconnectionservice, 0, 0, 2)
	ZEND_ARG_INFO(0, model)
	ZEND_ARG_INFO(0, connectionService)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_setreadconnectionservice, 0, 0, 2)
	ZEND_ARG_INFO(0, model)
	ZEND_ARG_INFO(0, connectionService)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_setdefaultconnectionservice, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, connectionService, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_setdefaultwriteconnectionservice, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, connectionService, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_setdefaultreadconnectionservice, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, connectionService, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_getwriteconnection, 0, 0, 1)
	ZEND_ARG_INFO(0, model)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_getreadconnection, 0, 0, 1)
	ZEND_ARG_INFO(0, model)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_getreadconnectionservice, 0, 0, 1)
	ZEND_ARG_INFO(0, model)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_getwriteconnectionservice, 0, 0, 1)
	ZEND_ARG_INFO(0, model)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_usedynamicupdate, 0, 0, 2)
	ZEND_ARG_INFO(0, model)
	ZEND_ARG_INFO(0, dynamicUpdate)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_isusingdynamicupdate, 0, 0, 1)
	ZEND_ARG_INFO(0, model)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_addhasmanytomany, 0, 0, 7)
	ZEND_ARG_INFO(0, model)
	ZEND_ARG_INFO(0, fields)
	ZEND_ARG_INFO(0, intermediateModel)
	ZEND_ARG_INFO(0, intermediateFields)
	ZEND_ARG_INFO(0, intermediateReferencedFields)
	ZEND_ARG_INFO(0, referencedModel)
	ZEND_ARG_INFO(0, referencedFields)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_existshasmanytomany, 0, 0, 2)
	ZEND_ARG_INFO(0, modelName)
	ZEND_ARG_INFO(0, modelRelation)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_getrelationbyalias, 0, 0, 2)
	ZEND_ARG_INFO(0, modelName)
	ZEND_ARG_INFO(0, alias)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_getrelationrecords, 0, 0, 3)
	ZEND_ARG_INFO(0, relation)
	ZEND_ARG_INFO(0, method)
	ZEND_ARG_INFO(0, record)
	ZEND_ARG_INFO(0, parameters)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_getreusablerecords, 0, 0, 2)
	ZEND_ARG_INFO(0, modelName)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_setreusablerecords, 0, 0, 3)
	ZEND_ARG_INFO(0, modelName)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, records)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_gethasmanytomany, 0, 0, 1)
	ZEND_ARG_INFO(0, model)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_registernamespacealias, 0, 0, 2)
	ZEND_ARG_INFO(0, alias)
	ZEND_ARG_INFO(0, namespace)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_manager_getnamespacealias, 0, 0, 1)
	ZEND_ARG_INFO(0, alias)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_manager_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Manager, setCustomEventsManager, arginfo_phalcon_mvc_model_manager_setcustomeventsmanager, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getCustomEventsManager, arginfo_phalcon_mvc_model_manager_getcustomeventsmanager, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, initialize, arginfo_phalcon_mvc_model_managerinterface_initialize, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, isInitialized, arginfo_phalcon_mvc_model_managerinterface_isinitialized, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getLastInitialized, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, load, arginfo_phalcon_mvc_model_managerinterface_load, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, setModelSource, arginfo_phalcon_mvc_model_manager_setmodelsource, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getModelSource, arginfo_phalcon_mvc_model_manager_getmodelsource, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, setModelSchema, arginfo_phalcon_mvc_model_manager_setmodelschema, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getModelSchema, arginfo_phalcon_mvc_model_manager_getmodelschema, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, setConnectionService, arginfo_phalcon_mvc_model_manager_setconnectionservice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, setWriteConnectionService, arginfo_phalcon_mvc_model_manager_setwriteconnectionservice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, setReadConnectionService, arginfo_phalcon_mvc_model_manager_setreadconnectionservice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, setDefaultConnectionService, arginfo_phalcon_mvc_model_manager_setdefaultconnectionservice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getDefaultConnectionService, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, setDefaultWriteConnectionService, arginfo_phalcon_mvc_model_manager_setdefaultwriteconnectionservice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, setDefaultReadConnectionService, arginfo_phalcon_mvc_model_manager_setdefaultreadconnectionservice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getWriteConnection, arginfo_phalcon_mvc_model_manager_getwriteconnection, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getReadConnection, arginfo_phalcon_mvc_model_manager_getreadconnection, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getReadConnectionService, arginfo_phalcon_mvc_model_manager_getreadconnectionservice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getWriteConnectionService, arginfo_phalcon_mvc_model_manager_getwriteconnectionservice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, notifyEvent, arginfo_phalcon_mvc_model_managerinterface_notifyevent, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, missingMethod, arginfo_phalcon_mvc_model_managerinterface_missingmethod, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, addBehavior, arginfo_phalcon_mvc_model_managerinterface_addbehavior, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, useDynamicUpdate, arginfo_phalcon_mvc_model_manager_usedynamicupdate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, isUsingDynamicUpdate, arginfo_phalcon_mvc_model_manager_isusingdynamicupdate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, addHasOne, arginfo_phalcon_mvc_model_managerinterface_addhasone, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, addBelongsTo, arginfo_phalcon_mvc_model_managerinterface_addbelongsto, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, addHasMany, arginfo_phalcon_mvc_model_managerinterface_addhasmany, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, addHasManyToMany, arginfo_phalcon_mvc_model_manager_addhasmanytomany, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, existsBelongsTo, arginfo_phalcon_mvc_model_managerinterface_existsbelongsto, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, existsHasMany, arginfo_phalcon_mvc_model_managerinterface_existshasmany, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, existsHasOne, arginfo_phalcon_mvc_model_managerinterface_existshasone, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, existsHasManyToMany, arginfo_phalcon_mvc_model_manager_existshasmanytomany, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getRelationByAlias, arginfo_phalcon_mvc_model_manager_getrelationbyalias, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getRelationRecords, arginfo_phalcon_mvc_model_manager_getrelationrecords, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getReusableRecords, arginfo_phalcon_mvc_model_manager_getreusablerecords, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, setReusableRecords, arginfo_phalcon_mvc_model_manager_setreusablerecords, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, clearReusableObjects, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getBelongsToRecords, arginfo_phalcon_mvc_model_managerinterface_getbelongstorecords, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getHasManyRecords, arginfo_phalcon_mvc_model_managerinterface_gethasmanyrecords, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getHasOneRecords, arginfo_phalcon_mvc_model_managerinterface_gethasonerecords, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getBelongsTo, arginfo_phalcon_mvc_model_managerinterface_getbelongsto, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getHasMany, arginfo_phalcon_mvc_model_managerinterface_gethasmany, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getHasOne, arginfo_phalcon_mvc_model_managerinterface_gethasone, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getHasManyToMany, arginfo_phalcon_mvc_model_manager_gethasmanytomany, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getHasOneAndHasMany, arginfo_phalcon_mvc_model_managerinterface_gethasoneandhasmany, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getRelations, arginfo_phalcon_mvc_model_managerinterface_getrelations, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getRelationsBetween, arginfo_phalcon_mvc_model_managerinterface_getrelationsbetween, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, createQuery, arginfo_phalcon_mvc_model_managerinterface_createquery, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, executeQuery, arginfo_phalcon_mvc_model_managerinterface_executequery, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, createBuilder, arginfo_phalcon_mvc_model_managerinterface_createbuilder, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getLastQuery, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, registerNamespaceAlias, arginfo_phalcon_mvc_model_manager_registernamespacealias, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getNamespaceAlias, arginfo_phalcon_mvc_model_manager_getnamespacealias, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, getNamespaceAliases, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Manager, __destruct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_DTOR)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Manager initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Manager){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model, Manager, mvc_model_manager, phalcon_di_injectable_ce, phalcon_mvc_model_manager_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_customEventsManager"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_readConnectionServices"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_writeConnectionServices"), ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_mvc_model_manager_ce, SL("_defaultConnectionService"), "db", ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_defaultReadConnectionService"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_defaultWriteConnectionService"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_aliases"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_hasMany"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_hasManySingle"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_hasOne"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_hasOneSingle"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_belongsTo"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_belongsToSingle"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_hasManyToMany"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_hasManyToManySingle"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_initialized"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_sources"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_schemas"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_behaviors"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_lastInitialized"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_lastQuery"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_reusable"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_dynamicUpdate"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_manager_ce, SL("_namespaceAliases"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_mvc_model_manager_ce, 1, phalcon_mvc_model_managerinterface_ce);

	return SUCCESS;
}

/**
 * Sets a custom events manager for a specific model
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @param Phalcon\Events\ManagerInterface $eventsManager
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, setCustomEventsManager){

	zval *model, *events_manager, class_name = {};

	phalcon_fetch_params(0, 2, 0, &model, &events_manager);

	phalcon_get_class(&class_name, model, 1);
	phalcon_update_property_array(getThis(), SL("_customEventsManager"), &class_name, events_manager);
	zval_ptr_dtor(&class_name);
}

/**
 * Returns a custom events manager related to a model
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @return Phalcon\Events\ManagerInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getCustomEventsManager){

	zval *model, custom_events_manager = {}, class_name = {}, events_manager = {};

	phalcon_fetch_params(0, 1, 0, &model);

	phalcon_read_property(&custom_events_manager, getThis(), SL("_customEventsManager"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(custom_events_manager) == IS_ARRAY) {
		phalcon_get_class(&class_name, model, 1);
		if (phalcon_array_isset_fetch(&events_manager, &custom_events_manager, &class_name, PH_READONLY)) {
			zval_ptr_dtor(&class_name);
			RETURN_CTOR(&events_manager);
		}
		zval_ptr_dtor(&class_name);
	}

	PHALCON_CALL_PARENT(return_value, phalcon_mvc_model_manager_ce, getThis(), "geteventsmanager");
}

/**
 * Initializes a model in the model manager
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, initialize){

	zval *model, class_name = {}, initialized = {}, event_name = {};

	phalcon_fetch_params(0, 1, 0, &model);

	phalcon_get_class(&class_name, model, 1);

	phalcon_read_property(&initialized, getThis(), SL("_initialized"), PH_NOISY|PH_READONLY);

	/**
	 * Models are just initialized once per request
	 */
	if (phalcon_array_isset(&initialized, &class_name)) {
		zval_ptr_dtor(&class_name);
		RETURN_TRUE;
	}

	ZVAL_STRING(&event_name, "modelsManager:beforeInitialize");
	PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name, model);
	zval_ptr_dtor(&event_name);

	/**
	 * Update the model as initialized, this avoid cyclic initializations
	 */
	phalcon_update_property_array(getThis(), SL("_initialized"), &class_name, model);
	zval_ptr_dtor(&class_name);

	/**
	 * Call the 'initialize' method if it's implemented
	 */
	if (phalcon_method_exists_ex(model, SL("initialize")) == SUCCESS) {
		PHALCON_CALL_METHOD(NULL, model, "initialize");
	}

	/**
	 * Update the last initialized model, so it can be used in
	 * modelsManager:afterInitialize
	 */
	phalcon_update_property(getThis(), SL("_lastInitialized"), model);

	/**
	 * If an EventsManager is available we pass to it every initialized model
	 */
	ZVAL_STRING(&event_name, "modelsManager:afterInitialize");
	PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name, model);
	zval_ptr_dtor(&event_name);
	RETURN_TRUE;
}

/**
 * Check whether a model is already initialized
 *
 * @param string $modelName
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, isInitialized){

	zval *model_name, initialized = {}, lowercased = {};

	phalcon_fetch_params(0, 1, 0, &model_name);

	phalcon_read_property(&initialized, getThis(), SL("_initialized"), PH_NOISY|PH_READONLY);

	phalcon_fast_strtolower(&lowercased, model_name);

	RETVAL_BOOL(phalcon_array_isset(&initialized, &lowercased));
	zval_ptr_dtor(&lowercased);
}

/**
 * Get last initialized model
 *
 * @return Phalcon\Mvc\ModelInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getLastInitialized){


	RETURN_MEMBER(getThis(), "_lastInitialized");
}

/**
 * Loads a model throwing an exception if it doesn't exist
 *
 * @param  string $modelName
 * @param  boolean $newInstance
 * @return Phalcon\Mvc\ModelInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, load){

	zval *model_name, *new_instance = NULL, initialized = {}, lowercased = {}, dependency_injector = {};
	zend_class_entry *ce0;

	phalcon_fetch_params(0, 1, 1, &model_name, &new_instance);
	PHALCON_ENSURE_IS_STRING(model_name);

	if (!new_instance) {
		new_instance = &PHALCON_GLOBAL(z_false);
	}

	phalcon_read_property(&initialized, getThis(), SL("_initialized"), PH_NOISY|PH_READONLY);

	phalcon_fast_strtolower(&lowercased, model_name);

	/**
	 * Check if a model with the same is already loaded
	 */
	if (!zend_is_true(new_instance) && phalcon_array_isset_fetch(return_value, &initialized, &lowercased, PH_COPY)) {
		zval_ptr_dtor(&lowercased);
		return;
	}
	zval_ptr_dtor(&lowercased);

	/**
	 * Load it using an autoloader
	 */
	if ((ce0 = phalcon_class_exists(model_name, 1)) != NULL) {
		PHALCON_CALL_METHOD(&dependency_injector, getThis(), "getdi");

		PHALCON_OBJECT_INIT(return_value, ce0);
		if (phalcon_has_constructor(return_value)) {
			PHALCON_CALL_METHOD(NULL, return_value, "__construct", &PHALCON_GLOBAL(z_null), &dependency_injector, getThis());
		}
		zval_ptr_dtor(&dependency_injector);
		return;
	}

	/**
	 * The model doesn't exist throw an exception
	 */
	zend_throw_exception_ex(phalcon_mvc_model_exception_ce, 0, "Model '%s' could not be loaded", Z_STRVAL_P(model_name));
}

/**
 * Sets the mapped source for a model
 *
 * @param Phalcon\Mvc\Model|string $model
 * @param string $source
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, setModelSource){

	zval *model, *source, entity_name = {};

	phalcon_fetch_params(0, 2, 0, &model, &source);

	if (Z_TYPE_P(model) != IS_OBJECT && Z_TYPE_P(model) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Model is not an object");
		return;
	}
	if (Z_TYPE_P(source) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Source must be a string");
		return;
	}
	phalcon_get_class(&entity_name, model, 1);
	phalcon_update_property_array(getThis(), SL("_sources"), &entity_name, source);
	zval_ptr_dtor(&entity_name);
}

/**
 * Returns the mapped source for a model
 *
 * @param Phalcon\Mvc\Model|string $model
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getModelSource){

	zval *model, entity_name = {}, sources = {}, source = {}, class_name = {};

	phalcon_fetch_params(0, 1, 0, &model);

	if (Z_TYPE_P(model) != IS_OBJECT && Z_TYPE_P(model) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Model is not an object");
		return;
	}

	phalcon_get_class(&entity_name, model, 1);

	phalcon_read_property(&sources, getThis(), SL("_sources"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(sources) == IS_ARRAY) {
		if (phalcon_array_isset_fetch(&source, &sources, &entity_name, PH_READONLY)) {
			zval_ptr_dtor(&entity_name);
			RETURN_CTOR(&source);
		}
	}

	phalcon_get_class_ns(&class_name, model, 0);

	phalcon_uncamelize(return_value, &class_name);
	zval_ptr_dtor(&class_name);

	phalcon_update_property_array(getThis(), SL("_sources"), &entity_name, return_value);
	zval_ptr_dtor(&entity_name);
}

/**
 * Sets the mapped schema for a model
 *
 * @param Phalcon\Mvc\Model $model
 * @param string $schema
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, setModelSchema){

	zval *model, *schema, entity_name = {};

	phalcon_fetch_params(0, 2, 0, &model, &schema);

	if (Z_TYPE_P(model) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Model is not an object");
		return;
	}
	if (Z_TYPE_P(schema) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Schema must be a string");
		return;
	}

	phalcon_get_class(&entity_name, model, 1);
	phalcon_update_property_array(getThis(), SL("_schemas"), &entity_name, schema);
	zval_ptr_dtor(&entity_name);
}

/**
 * Returns the mapped schema for a model
 *
 * @param Phalcon\Mvc\Model $model
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getModelSchema){

	zval *model, entity_name = {}, schemas = {};

	phalcon_fetch_params(0, 1, 0, &model);

	if (Z_TYPE_P(model) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Model is not an object");
		return;
	}

	phalcon_get_class(&entity_name, model, 1);

	phalcon_read_property(&schemas, getThis(), SL("_schemas"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(schemas) == IS_ARRAY) {
		if (!phalcon_array_isset_fetch(return_value, &schemas, &entity_name, PH_COPY)) {
			ZVAL_NULL(return_value);
		}
	}
	zval_ptr_dtor(&entity_name);
}

/**
 * Sets both write and read connection service for a model
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @param string $connectionService
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, setConnectionService){

	zval *model, *connection_service, entity_name = {};

	phalcon_fetch_params(0, 2, 0, &model, &connection_service);

	if (Z_TYPE_P(connection_service) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The connection service must be a string");
		return;
	}

	phalcon_get_class(&entity_name, model, 1);
	phalcon_update_property_array(getThis(), SL("_readConnectionServices"), &entity_name, connection_service);
	phalcon_update_property_array(getThis(), SL("_writeConnectionServices"), &entity_name, connection_service);
	zval_ptr_dtor(&entity_name);
}

/**
 * Sets write connection service for a model
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @param string $connectionService
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, setWriteConnectionService){

	zval *model, *connection_service, entity_name = {};

	phalcon_fetch_params(0, 2, 0, &model, &connection_service);

	if (Z_TYPE_P(connection_service) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The connection service must be a string");
		return;
	}

	phalcon_get_class(&entity_name, model, 1);
	phalcon_update_property_array(getThis(), SL("_writeConnectionServices"), &entity_name, connection_service);
	zval_ptr_dtor(&entity_name);
}

/**
 * Sets read connection service for a model
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @param string $connectionService
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, setReadConnectionService){

	zval *model, *connection_service, entity_name = {};

	phalcon_fetch_params(0, 2, 0, &model, &connection_service);

	if (Z_TYPE_P(connection_service) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The connection service must be a string");
		return;
	}

	phalcon_get_class(&entity_name, model, 1);
	phalcon_update_property_array(getThis(), SL("_readConnectionServices"), &entity_name, connection_service);
	zval_ptr_dtor(&entity_name);
}

/**
 * Sets default connection service for a model
 *
 * @param string $connectionService
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, setDefaultConnectionService){

	zval *connection_service;

	phalcon_fetch_params(0, 1, 0, &connection_service);

	if (PHALCON_IS_NOT_EMPTY(connection_service)) {
		phalcon_update_property(getThis(), SL("_defaultConnectionService"), connection_service);
	}
}

/**
 * Gets default connection service for a model
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getDefaultConnectionService){

	RETURN_MEMBER(getThis(), "_defaultConnectionService");
}

/**
 * Sets default write connection service for a model
 *
 * @param string $connectionService
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, setDefaultWriteConnectionService){

	zval *connection_service;

	phalcon_fetch_params(0, 1, 0, &connection_service);

	phalcon_update_property(getThis(), SL("_defaultWriteConnectionService"), connection_service);
}

/**
 * Sets default read connection service for a model
 *
 * @param string $connectionService
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, setDefaultReadConnectionService){

	zval *connection_service;

	phalcon_fetch_params(0, 1, 0, &connection_service);

	phalcon_update_property(getThis(), SL("_defaultReadConnectionService"), connection_service);
}

/**
 * Returns the connection to write data related to a model
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @return Phalcon\Db\AdapterInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getWriteConnection){

	zval *model, service = {}, dependency_injector = {};

	phalcon_fetch_params(0, 1, 0, &model);

	PHALCON_CALL_METHOD(&dependency_injector, getThis(), "getdi");
	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
		zval_ptr_dtor(&service);
		return;
	}

	PHALCON_CALL_SELF(&service, "getwriteconnectionservice", model);

	/**
	 * Request the connection service from the DI
	 */
	PHALCON_CALL_METHOD(return_value, &dependency_injector, "getshared", &service);
	zval_ptr_dtor(&dependency_injector);
	zval_ptr_dtor(&service);
	if (Z_TYPE_P(return_value) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Invalid injected connection service");
		return;
	}

	PHALCON_VERIFY_INTERFACE(return_value, phalcon_db_adapterinterface_ce);
}

/**
 * Returns the connection to read data related to a model
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @return Phalcon\Db\AdapterInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getReadConnection){

	zval *model, service = {}, dependency_injector = {};

	phalcon_fetch_params(0, 1, 0, &model);

	PHALCON_CALL_SELF(&service, "getreadconnectionservice", model);

	PHALCON_CALL_METHOD(&dependency_injector, getThis(), "getdi");
	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
		return;
	}

	/**
	 * Request the connection service from the DI
	 */
	PHALCON_CALL_METHOD(return_value, &dependency_injector, "getshared", &service);
	zval_ptr_dtor(&dependency_injector);
	zval_ptr_dtor(&service);
	if (Z_TYPE_P(return_value) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Invalid injected connection service");
		return;
	}

	PHALCON_VERIFY_INTERFACE(return_value, phalcon_db_adapterinterface_ce);
}

/**
 * Returns the connection service name used to read data related to a model
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @param string
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getReadConnectionService){

	zval *model, connection_services = {}, entity_name = {}, connection = {};

	phalcon_fetch_params(0, 1, 0, &model);

	phalcon_read_property(&connection_services, getThis(), SL("_readConnectionServices"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(connection_services) == IS_ARRAY) {
		phalcon_get_class(&entity_name, model, 1);

		/**
		 * Check if there is a custom service connection name
		 */
		if (phalcon_array_isset_fetch(&connection, &connection_services, &entity_name, PH_READONLY)) {
			zval_ptr_dtor(&entity_name);
			RETURN_CTOR(&connection);
		}
		zval_ptr_dtor(&entity_name);
	}

	phalcon_read_property(&connection, getThis(), SL("_defaultReadConnectionService"), PH_NOISY|PH_READONLY);
	if (PHALCON_IS_NOT_EMPTY(&connection)) {
		RETURN_CTOR(&connection);
	}

	RETURN_MEMBER(getThis(), "_defaultConnectionService");
}

/**
 * Returns the connection service name used to write data related to a model
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @param string
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getWriteConnectionService){

	zval *model, connection_services = {}, entity_name = {}, connection = {};

	phalcon_fetch_params(0, 1, 0, &model);

	phalcon_read_property(&connection_services, getThis(), SL("_writeConnectionServices"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(connection_services) == IS_ARRAY) {
		phalcon_get_class(&entity_name, model, 1);

		/**
		 * Check if there is a custom service connection name
		 */
		if (phalcon_array_isset_fetch(&connection, &connection_services, &entity_name, PH_READONLY)) {
			zval_ptr_dtor(&entity_name);
			RETURN_CTOR(&connection);
		}
		zval_ptr_dtor(&entity_name);
	}

	phalcon_read_property(&connection, getThis(), SL("_defaultWriteConnectionService"), PH_NOISY|PH_READONLY);
	if (PHALCON_IS_NOT_EMPTY(&connection)) {
		RETURN_CTOR(&connection);
	}

	RETURN_MEMBER(getThis(), "_defaultConnectionService");
}

/**
 * Receives events generated in the models and dispatches them to a events-manager if available
 * Notify the behaviors that are listening in the model
 *
 * @param string $eventName
 * @param Phalcon\Mvc\ModelInterface $model
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, notifyEvent){

	zval *eventname, *model, entity_name = {}, behaviors = {}, models_behaviors = {};

	phalcon_fetch_params(0, 2, 0, &eventname, &model);

	phalcon_get_class(&entity_name, model, 1);

	/**
	 * Dispatch events to the global events manager
	 */
	phalcon_read_property(&behaviors, getThis(), SL("_behaviors"), PH_NOISY|PH_READONLY);
	ZVAL_TRUE(return_value);
	if (Z_TYPE(behaviors) == IS_ARRAY && phalcon_array_isset_fetch(&models_behaviors, &behaviors, &entity_name, PH_READONLY)) {
		zval *behavior;
		/**
		 * Notify all the events on the behavior
		 */
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(models_behaviors), behavior) {
			PHALCON_CALL_METHOD(return_value, behavior, "notify", eventname, model);
			if (PHALCON_IS_FALSE(return_value)) {
				break;
			}
		} ZEND_HASH_FOREACH_END();
	}

	zval_ptr_dtor(&entity_name);
}

/**
 * Dispatch a event to the listeners and behaviors
 * This method expects that the endpoint listeners/behaviors returns true
 * meaning that a least one is implemented
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @param string $eventName
 * @param array $data
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, missingMethod){

	zval *model, *eventname, *data, behaviors = {}, entity_name = {}, models_behaviors = {}, *behavior, result = {}, events_manager = {}, fire_event_name = {};

	phalcon_fetch_params(0, 3, 0, &model, &eventname, &data);

	/**
	 * Dispatch events to the global events manager
	 */
	phalcon_read_property(&behaviors, getThis(), SL("_behaviors"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(behaviors) == IS_ARRAY) {
		phalcon_get_class(&entity_name, model, 1);
		/**
		 * Notify all the events on the behavior
		 */
		if (phalcon_array_isset_fetch(&models_behaviors, &behaviors, &entity_name, PH_READONLY)) {
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(models_behaviors), behavior) {
				PHALCON_CALL_METHOD(&result, behavior, "missingmethod", model, eventname, data);
				if (Z_TYPE(result) != IS_NULL) {
					RETVAL_ZVAL(&result, 0, 0);
					zval_ptr_dtor(&entity_name);
					return;
				}
			} ZEND_HASH_FOREACH_END();
		}
		zval_ptr_dtor(&entity_name);
	}

	/**
	 * Dispatch events to the global events manager
	 */
	PHALCON_CALL_METHOD(&events_manager, getThis(), "geteventsmanager");
	if (Z_TYPE(events_manager) == IS_OBJECT) {
		PHALCON_CONCAT_SV(&fire_event_name, "model:", eventname);
		PHALCON_RETURN_CALL_METHOD(&events_manager, "fire", &fire_event_name, model, data);
		zval_ptr_dtor(&fire_event_name);
	} else {
		RETVAL_NULL();
	}
	zval_ptr_dtor(&events_manager);
}

/**
 * Binds a behavior to a model
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @param Phalcon\Mvc\Model\BehaviorInterface $behavior
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, addBehavior){

	zval *model, *behavior, entity_name = {}, behaviors = {}, models_behaviors = {};

	phalcon_fetch_params(0, 2, 0, &model, &behavior);

	if (Z_TYPE_P(behavior) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The behavior is invalid");
		return;
	}

	phalcon_get_class(&entity_name, model, 1);

	/**
	 * Get the current behaviors
	 */
	phalcon_read_property(&behaviors, getThis(), SL("_behaviors"), PH_NOISY|PH_READONLY);
	if (!phalcon_array_isset_fetch(&models_behaviors, &behaviors, &entity_name, PH_COPY)) {
		array_init(&models_behaviors);
	}

	/**
	 * Append the behavior to the list of behaviors
	 */
	phalcon_array_append(&models_behaviors, behavior, PH_COPY);

	/**
	 * Update the behaviors list
	 */
	phalcon_update_property_array(getThis(), SL("_behaviors"), &entity_name, &models_behaviors);
	zval_ptr_dtor(&entity_name);
	zval_ptr_dtor(&models_behaviors);
}

/**
 * Sets if a model must use dynamic update instead of the all-field update
 *
 * @param Phalcon\Mvc\Model $model
 * @param boolean $dynamicUpdate
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, useDynamicUpdate){

	zval *model, *dynamic_update, entity_name = {};

	phalcon_fetch_params(0, 2, 0, &model, &dynamic_update);

	phalcon_get_class(&entity_name, model, 1);
	phalcon_update_property_array(getThis(), SL("_dynamicUpdate"), &entity_name, dynamic_update);
	zval_ptr_dtor(&entity_name);
}

/**
 * Checks if a model is using dynamic update instead of all-field update
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, isUsingDynamicUpdate){

	zval *model, dynamic_update = {}, entity_name = {};

	phalcon_fetch_params(0, 1, 0, &model);

	phalcon_read_property(&dynamic_update, getThis(), SL("_dynamicUpdate"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(dynamic_update) == IS_ARRAY) {
		phalcon_get_class(&entity_name, model, 1);
		if (phalcon_array_isset_fetch(return_value, &dynamic_update, &entity_name, PH_COPY)) {
			zval_ptr_dtor(&entity_name);
			return;
		}
		zval_ptr_dtor(&entity_name);
	}

	RETURN_TRUE;
}

/**
 * Setup a 1-1 relation between two models
 *
 * @param   Phalcon\Mvc\Model $model
 * @param mixed $fields
 * @param string $referencedModel
 * @param mixed $referencedFields
 * @param array $options
 * @return  Phalcon\Mvc\Model\Relation
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, addHasOne){

	zval *model, *fields, *referenced_model, *referenced_fields, *options = NULL, entity_name = {}, referenced_entity = {};
	zval key_relation = {}, has_one = {}, relations = {}, number_fields = {}, number_referenced = {}, type = {}, relation = {}, alias = {};
	zval lower_alias = {}, key_alias = {}, has_one_single = {}, single_relations = {};

	phalcon_fetch_params(0, 4, 1, &model, &fields, &referenced_model, &referenced_fields, &options);

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	phalcon_get_class(&entity_name, model, 1);
	phalcon_fast_strtolower(&referenced_entity, referenced_model);

	PHALCON_CONCAT_VSV(&key_relation, &entity_name, "$", &referenced_entity);

	phalcon_read_property(&has_one, getThis(), SL("_hasOne"), PH_NOISY|PH_READONLY);
	if (!phalcon_array_isset_fetch(&relations, &has_one, &key_relation, PH_COPY)) {
		array_init(&relations);
	}

	/**
	 * Check if the number of fields are the same
	 */
	if (Z_TYPE_P(referenced_fields) == IS_ARRAY) {
		phalcon_fast_count(&number_fields, fields);
		phalcon_fast_count(&number_referenced, referenced_fields);
		if (!PHALCON_IS_EQUAL(&number_fields, &number_referenced)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Number of referenced fields are not the same");
			return;
		}
	}

	/**
	 * Type '1' is 'has one'
	 */
	ZVAL_LONG(&type, 1);

	/**
	 * Create a relationship instance
	 */
	object_init_ex(&relation, phalcon_mvc_model_relation_ce);
	PHALCON_CALL_METHOD(NULL, &relation, "__construct", &type, referenced_model, fields, referenced_fields, options);

	/**
	 * Check an alias for the relation
	 */
	if (phalcon_array_isset_fetch_str(&alias, options, SL("alias"), PH_READONLY)) {
		phalcon_fast_strtolower(&lower_alias, &alias);
	} else {
		ZVAL_COPY(&lower_alias, &referenced_entity);
	}
	zval_ptr_dtor(&referenced_entity);

	/**
	 * Append a new relationship
	 */
	phalcon_array_append(&relations, &relation, PH_COPY);

	/**
	 * Update the global alias
	 */
	PHALCON_CONCAT_VSV(&key_alias, &entity_name, "$", &lower_alias);
	zval_ptr_dtor(&lower_alias);
	phalcon_update_property_array(getThis(), SL("_aliases"), &key_alias, &relation);
	zval_ptr_dtor(&key_alias);

	/**
	 * Update the relations
	 */
	phalcon_update_property_array(getThis(), SL("_hasOne"), &key_relation, &relations);
	zval_ptr_dtor(&key_relation);
	zval_ptr_dtor(&relations);

	/**
	 * Get existing relations by model
	 */
	phalcon_read_property(&has_one_single, getThis(), SL("_hasOneSingle"), PH_NOISY|PH_READONLY);
	if (!phalcon_array_isset_fetch(&single_relations, &has_one_single, &entity_name, PH_COPY)) {
		array_init(&single_relations);
	}

	/**
	 * Append a new relationship
	 */
	phalcon_array_append(&single_relations, &relation, PH_COPY);

	/**
	 * Update relations by model
	 */
	phalcon_update_property_array(getThis(), SL("_hasOneSingle"), &entity_name, &single_relations);
	zval_ptr_dtor(&entity_name);
	zval_ptr_dtor(&single_relations);

	RETVAL_ZVAL(&relation, 0, 0);
}

/**
 * Setup a relation reverse many to one between two models
 *
 * @param   Phalcon\Mvc\Model $model
 * @param mixed $fields
 * @param string $referencedModel
 * @param mixed $referencedFields
 * @param array $options
 * @return  Phalcon\Mvc\Model\Relation
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, addBelongsTo){

	zval *model, *fields, *referenced_model, *referenced_fields, *options = NULL, entity_name = {}, referenced_entity = {}, key_relation = {}, belongs_to = {};
	zval relations = {}, number_fields = {}, number_referenced = {}, type = {}, relation = {}, alias = {}, lower_alias = {}, key_alias = {}, belongs_to_single = {}, single_relations = {};

	phalcon_fetch_params(0, 4, 1, &model, &fields, &referenced_model, &referenced_fields, &options);

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	phalcon_get_class(&entity_name, model, 1);
	phalcon_fast_strtolower(&referenced_entity, referenced_model);

	PHALCON_CONCAT_VSV(&key_relation, &entity_name, "$", &referenced_entity);

	phalcon_read_property(&belongs_to, getThis(), SL("_belongsTo"), PH_NOISY|PH_READONLY);
	if (!phalcon_array_isset_fetch(&relations, &belongs_to, &key_relation, PH_COPY)) {
		array_init(&relations);
	}

	/**
	 * Check if the number of fields are the same
	 */
	if (Z_TYPE_P(referenced_fields) == IS_ARRAY) {
		phalcon_fast_count(&number_fields, fields);
		phalcon_fast_count(&number_referenced, referenced_fields);
		if (!PHALCON_IS_EQUAL(&number_fields, &number_referenced)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Number of referenced fields are not the same");
			return;
		}
	}

	/**
	 * Type '0' is 'belongs to'
	 */
	ZVAL_LONG(&type, 0);

	/**
	 * Create a relationship instance
	 */
	object_init_ex(&relation, phalcon_mvc_model_relation_ce);
	PHALCON_CALL_METHOD(NULL, &relation, "__construct", &type, referenced_model, fields, referenced_fields, options);

	/**
	 * Check an alias for the relation
	 */
	if (phalcon_array_isset_fetch_str(&alias, options, SL("alias"), PH_READONLY)) {
		phalcon_fast_strtolower(&lower_alias, &alias);
	} else {
		ZVAL_COPY(&lower_alias, &referenced_entity);
	}
	zval_ptr_dtor(&referenced_entity);

	/**
	 * Append a new relationship
	 */
	phalcon_array_append(&relations, &relation, PH_COPY);

	/**
	 * Update the global alias
	 */
	PHALCON_CONCAT_VSV(&key_alias, &entity_name, "$", &lower_alias);
	zval_ptr_dtor(&lower_alias);
	phalcon_update_property_array(getThis(), SL("_aliases"), &key_alias, &relation);
	zval_ptr_dtor(&key_alias);

	/**
	 * Update the relations
	 */
	phalcon_update_property_array(getThis(), SL("_belongsTo"), &key_relation, &relations);
	zval_ptr_dtor(&key_relation);
	zval_ptr_dtor(&relations);

	/**
	 * Get existing relations by model
	 */
	phalcon_read_property(&belongs_to_single, getThis(), SL("_belongsToSingle"), PH_NOISY|PH_READONLY);
	if (!phalcon_array_isset_fetch(&single_relations, &belongs_to_single, &entity_name, PH_COPY)) {
		array_init(&single_relations);
	}

	/**
	 * Append a new relationship
	 */
	phalcon_array_append(&single_relations, &relation, PH_COPY);

	/**
	 * Update relations by model
	 */
	phalcon_update_property_array(getThis(), SL("_belongsToSingle"), &entity_name, &single_relations);
	zval_ptr_dtor(&entity_name);
	zval_ptr_dtor(&single_relations);


	RETVAL_ZVAL(&relation, 0, 0);
}

/**
 * Setup a relation 1-n between two models
 *
 * @param 	Phalcon\Mvc\ModelInterface $model
 * @param mixed $fields
 * @param string $referencedModel
 * @param mixed $referencedFields
 * @param array $options
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, addHasMany){

	zval *model, *fields, *referenced_model, *referenced_fields, *options = NULL, entity_name = {}, referenced_entity = {};
	zval key_relation = {}, has_many = {}, relations = {}, number_fields = {}, number_referenced = {}, type = {}, relation = {}, alias = {};
	zval lower_alias = {}, key_alias = {}, has_many_single = {}, single_relations = {};

	phalcon_fetch_params(0, 4, 1, &model, &fields, &referenced_model, &referenced_fields, &options);

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	phalcon_get_class(&entity_name, model, 1);
	phalcon_fast_strtolower(&referenced_entity, referenced_model);

	PHALCON_CONCAT_VSV(&key_relation, &entity_name, "$", &referenced_entity);

	phalcon_read_property(&has_many, getThis(), SL("_hasMany"), PH_NOISY|PH_READONLY);
	if (!phalcon_array_isset_fetch(&relations, &has_many, &key_relation, PH_COPY)) {
		array_init(&relations);
	}

	/**
	 * Check if the number of fields are the same
	 */
	if (Z_TYPE_P(referenced_fields) == IS_ARRAY) {
		phalcon_fast_count(&number_fields, fields);
		phalcon_fast_count(&number_referenced, referenced_fields);
		if (!PHALCON_IS_EQUAL(&number_fields, &number_referenced)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Number of referenced fields are not the same");
			return;
		}
	}

	/**
	 * Type '2' is 'has many'
	 */
	ZVAL_LONG(&type, 2);

	/**
	 * Create a relationship instance
	 */
	object_init_ex(&relation, phalcon_mvc_model_relation_ce);
	PHALCON_CALL_METHOD(NULL, &relation, "__construct", &type, referenced_model, fields, referenced_fields, options);

	/**
	 * Check an alias for the relation
	 */
	if (phalcon_array_isset_fetch_str(&alias, options, SL("alias"), PH_READONLY)) {
		phalcon_fast_strtolower(&lower_alias, &alias);
	} else {
		ZVAL_COPY(&lower_alias, &referenced_entity);
	}
	zval_ptr_dtor(&referenced_entity);

	/**
	 * Append a new relationship
	 */
	phalcon_array_append(&relations, &relation, PH_COPY);

	/**
	 * Update the global alias
	 */
	PHALCON_CONCAT_VSV(&key_alias, &entity_name, "$", &lower_alias);
	zval_ptr_dtor(&lower_alias);
	phalcon_update_property_array(getThis(), SL("_aliases"), &key_alias, &relation);
	zval_ptr_dtor(&key_alias);

	/**
	 * Update the relations
	 */
	phalcon_update_property_array(getThis(), SL("_hasMany"), &key_relation, &relations);
	zval_ptr_dtor(&key_relation);
	zval_ptr_dtor(&relations);

	/**
	 * Get existing relations by model
	 */
	phalcon_read_property(&has_many_single, getThis(), SL("_hasManySingle"), PH_NOISY|PH_READONLY);
	if (!phalcon_array_isset_fetch(&single_relations, &has_many_single, &entity_name, PH_COPY)) {
		array_init(&single_relations);
	}

	/**
	 * Append a new relationship
	 */
	phalcon_array_append(&single_relations, &relation, PH_COPY);

	/**
	 * Update relations by model
	 */
	phalcon_update_property_array(getThis(), SL("_hasManySingle"), &entity_name, &single_relations);
	zval_ptr_dtor(&entity_name);
	zval_ptr_dtor(&single_relations);

	RETVAL_ZVAL(&relation, 0, 0);
}

/**
 * Setups a relation n-m between two models
 *
 * @param string $fields
 * @param string $intermediateModel
 * @param string $intermediateFields
 * @param string $intermediateReferencedFields
 * @param string $referencedModel
 * @param string $referencedFields
 * @param   array $options
 * @return  Phalcon\Mvc\Model\Relation
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, addHasManyToMany){

	zval *model, *fields, *intermediate_model, *intermediate_fields, *intermediate_referenced_fields, *referenced_model, *referenced_fields, *options = NULL;
	zval entity_name = {}, referenced_entity = {}, key_relation = {}, has_many_to_many = {}, relations = {}, number_fields = {}, number_referenced = {}, type = {};
	zval relation = {}, alias = {}, lower_alias = {}, key_alias = {}, has_many_to_many_single = {}, single_relations = {};

	phalcon_fetch_params(0, 7, 1, &model, &fields, &intermediate_model, &intermediate_fields, &intermediate_referenced_fields, &referenced_model, &referenced_fields, &options);

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	phalcon_get_class(&entity_name, model, 1);

	phalcon_fast_strtolower(&referenced_entity, referenced_model);

	PHALCON_CONCAT_VSV(&key_relation, &entity_name, "$", &referenced_entity);

	phalcon_read_property(&has_many_to_many, getThis(), SL("_hasManyToMany"), PH_NOISY|PH_READONLY);
	if (!phalcon_array_isset_fetch(&relations, &has_many_to_many, &key_relation, PH_COPY)) {
		array_init(&relations);
	}

	/**
	 * Check if the number of fields are the same from the model to the intermediate
	 * model
	 */
	if (Z_TYPE_P(intermediate_fields) == IS_ARRAY) {
		phalcon_fast_count(&number_fields, fields);
		phalcon_fast_count(&number_referenced, intermediate_fields);
		if (!PHALCON_IS_EQUAL(&number_fields, &number_referenced)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Number of referenced fields are not the same");
			return;
		}
	}

	/**
	 * Check if the number of fields are the same from the intermediate model to the
	 * referenced model
	 */
	if (Z_TYPE_P(intermediate_referenced_fields) == IS_ARRAY) {
		phalcon_fast_count(&number_fields, fields);
		phalcon_fast_count(&number_referenced, intermediate_fields);
		if (!PHALCON_IS_EQUAL(&number_fields, &number_referenced)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Number of referenced fields are not the same");
			return;
		}
	}

	/**
	 * Type '4' is 'has many through'
	 */
	ZVAL_LONG(&type, 4);

	/**
	 * Create a relationship instance
	 */
	object_init_ex(&relation, phalcon_mvc_model_relation_ce);
	PHALCON_CALL_METHOD(NULL, &relation, "__construct", &type, referenced_model, fields, referenced_fields, options);

	/**
	 * Set extended intermediate relation data
	 */
	PHALCON_CALL_METHOD(NULL, &relation, "setintermediaterelation", intermediate_fields, intermediate_model, intermediate_referenced_fields);

	/**
	 * Check an alias for the relation
	 */
	if (phalcon_array_isset_fetch_str(&alias, options, SL("alias"), PH_READONLY)) {
		phalcon_fast_strtolower(&lower_alias, &alias);
	} else {
		ZVAL_COPY(&lower_alias, &referenced_entity);
	}

	/**
	 * Append a new relationship
	 */
	phalcon_array_append(&relations, &relation, PH_COPY);

	/**
	 * Update the global alias
	 */
	PHALCON_CONCAT_VSV(&key_alias, &entity_name, "$", &lower_alias);
	zval_ptr_dtor(&lower_alias);
	phalcon_update_property_array(getThis(), SL("_aliases"), &key_alias, &relation);
	zval_ptr_dtor(&key_alias);

	/**
	 * Update the relations
	 */
	phalcon_update_property_array(getThis(), SL("_hasManyToMany"), &key_relation, &relations);
	zval_ptr_dtor(&key_relation);
	zval_ptr_dtor(&relations);

	/**
	 * Get existing relations by model
	 */
	phalcon_read_property(&has_many_to_many_single, getThis(), SL("_hasManyToManySingle"), PH_NOISY|PH_READONLY);
	if (!phalcon_array_isset_fetch(&single_relations, &has_many_to_many_single, &entity_name, PH_COPY)) {
		array_init(&single_relations);
	}

	/**
	 * Append a new relationship
	 */
	phalcon_array_append(&single_relations, &relation, PH_COPY);

	/**
	 * Update relations by model
	 */
	phalcon_update_property_array(getThis(), SL("_hasManyToManySingle"), &entity_name, &single_relations);
	zval_ptr_dtor(&entity_name);
	zval_ptr_dtor(&single_relations);

	RETVAL_ZVAL(&relation, 0, 0);
}

/**
 * Checks whether a model has a belongsTo relation with another model
 *
 * @param 	string $modelName
 * @param 	string $modelRelation
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, existsBelongsTo){

	zval *model_name, *model_relation, initialized = {}, entity_name = {}, entity_relation = {}, key_relation = {}, belongs_to = {};

	phalcon_fetch_params(0, 2, 0, &model_name, &model_relation);

	phalcon_read_property(&initialized, getThis(), SL("_initialized"), PH_NOISY|PH_READONLY);

	phalcon_fast_strtolower(&entity_name, model_name);
	phalcon_fast_strtolower(&entity_relation, model_relation);

	/**
	 * Relationship unique key
	 */
	PHALCON_CONCAT_VSV(&key_relation, &entity_name, "$", &entity_relation);
	zval_ptr_dtor(&entity_relation);

	/**
	 * Initialize the model first
	 */
	if (!phalcon_array_isset(&initialized, &entity_name)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "load", model_name);
	}

	phalcon_read_property(&belongs_to, getThis(), SL("_belongsTo"), PH_NOISY|PH_READONLY);
	if (phalcon_array_isset(&belongs_to, &key_relation)) {
		RETVAL_TRUE;
	} else {
		RETVAL_FALSE;
	}
	zval_ptr_dtor(&key_relation);
}

/**
 * Checks whether a model has a hasMany relation with another model
 *
 * @param 	string $modelName
 * @param 	string $modelRelation
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, existsHasMany){

	zval *model_name, *model_relation, initialized = {}, entity_name = {}, entity_relation = {}, key_relation = {}, has_many = {};

	phalcon_fetch_params(0, 2, 0, &model_name, &model_relation);

	phalcon_read_property(&initialized, getThis(), SL("_initialized"), PH_NOISY|PH_READONLY);

	phalcon_fast_strtolower(&entity_name, model_name);
	phalcon_fast_strtolower(&entity_relation, model_relation);

	/**
	 * Relationship unique key
	 */
	PHALCON_CONCAT_VSV(&key_relation, &entity_name, "$", &entity_relation);
	zval_ptr_dtor(&entity_relation);

	/**
	 * Initialize the model first
	 */
	if (!phalcon_array_isset(&initialized, &entity_name)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "load", model_name);
	}

	phalcon_read_property(&has_many, getThis(), SL("_hasMany"), PH_NOISY|PH_READONLY);
	if (phalcon_array_isset(&has_many, &key_relation)) {
		RETVAL_TRUE;
	} else {
		RETVAL_FALSE;
	}
	zval_ptr_dtor(&key_relation);
}

/**
 * Checks whether a model has a hasOne relation with another model
 *
 * @param 	string $modelName
 * @param 	string $modelRelation
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, existsHasOne){

	zval *model_name, *model_relation, initialized = {}, entity_name = {}, entity_relation = {}, key_relation = {}, has_one = {};

	phalcon_fetch_params(0, 2, 0, &model_name, &model_relation);

	phalcon_read_property(&initialized, getThis(), SL("_initialized"), PH_NOISY|PH_READONLY);

	phalcon_fast_strtolower(&entity_name, model_name);
	phalcon_fast_strtolower(&entity_relation, model_relation);

	/**
	 * Relationship unique key
	 */
	PHALCON_CONCAT_VSV(&key_relation, &entity_name, "$", &entity_relation);
	zval_ptr_dtor(&entity_relation);

	/**
	 * Initialize the model first
	 */
	if (!phalcon_array_isset(&initialized, &entity_name)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "load", model_name);
	}

	phalcon_read_property(&has_one, getThis(), SL("_hasOne"), PH_NOISY|PH_READONLY);
	if (phalcon_array_isset(&has_one, &key_relation)) {
		RETVAL_TRUE;
	} else {
		RETVAL_FALSE;
	}
	zval_ptr_dtor(&key_relation);
}

/**
 * Checks whether a model has a hasManyToMany relation with another model
 *
 * @param 	string $modelName
 * @param 	string $modelRelation
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, existsHasManyToMany){

	zval *model_name, *model_relation, initialized = {}, entity_name = {}, entity_relation = {}, key_relation = {}, has_many_to_many = {};

	phalcon_fetch_params(0, 2, 0, &model_name, &model_relation);

	phalcon_read_property(&initialized, getThis(), SL("_initialized"), PH_NOISY|PH_READONLY);

	phalcon_fast_strtolower(&entity_name, model_name);
	phalcon_fast_strtolower(&entity_relation, model_relation);

	/**
	 * Relationship unique key
	 */
	PHALCON_CONCAT_VSV(&key_relation, &entity_name, "$", &entity_relation);
	zval_ptr_dtor(&entity_relation);

	/**
	 * Initialize the model first
	 */
	if (!phalcon_array_isset(&initialized, &entity_name)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "load", model_name);
	}

	phalcon_read_property(&has_many_to_many, getThis(), SL("_hasManyToMany"), PH_NOISY|PH_READONLY);
	if (phalcon_array_isset(&has_many_to_many, &key_relation)) {
		RETVAL_TRUE;
	} else {
		RETVAL_FALSE;
	}
	zval_ptr_dtor(&key_relation);
}

/**
 * Returns a relation by its alias
 *
 * @param string $modelName
 * @param string $alias
 * @return Phalcon\Mvc\Model\Relation
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getRelationByAlias){

	zval *model_name, *alias, aliases = {}, key_alias = {}, key_lower = {}, relation = {};

	phalcon_fetch_params(0, 2, 0, &model_name, &alias);

	phalcon_read_property(&aliases, getThis(), SL("_aliases"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(aliases) == IS_ARRAY) {
		PHALCON_CONCAT_VSV(&key_alias, model_name, "$", alias);
		phalcon_fast_strtolower(&key_lower, &key_alias);
		if (phalcon_array_isset_fetch(&relation, &aliases, &key_lower, PH_READONLY)) {
			zval_ptr_dtor(&key_alias);
			zval_ptr_dtor(&key_lower);
			RETURN_CTOR(&relation);
		}
		zval_ptr_dtor(&key_alias);
		zval_ptr_dtor(&key_lower);
	}

	RETURN_FALSE;
}

/**
 * Helper method to query records based on a relation definition
 *
 * @param Phalcon\Mvc\Model\Relation $relation
 * @param string $method
 * @param Phalcon\Mvc\ModelInterface $record
 * @param array $parameters
 * @return Phalcon\Mvc\Model\Resultset\Simple
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getRelationRecords){

	zval *relation, *method, *record, *p = NULL, parameters = {}, pre_conditions = {}, placeholders = {}, referenced_model = {}, is_through = {}, conditions = {};
	zval intermediate_model = {}, intermediate_fields = {}, fields = {}, join_conditions = {}, joined_join_conditions = {};
	zval joined_conditions = {}, builder = {}, query = {}, *field, dependency_injector = {}, find_params = {}, find_arguments = {}, arguments = {};
	zval type = {}, retrieve_method = {}, reusable = {}, unique_key = {}, records = {}, referenced_entity = {}, call_object = {};
	zend_string *str_key;
	ulong idx;
	int f_reusable;

	phalcon_fetch_params(1, 3, 1, &relation, &method, &record, &p);

	if (p) {
		PHALCON_MM_ZVAL_DUP(&parameters, p);
	}

	if (Z_TYPE(parameters) == IS_ARRAY) {
		/**
		 * Re-use conditions
		 */
		if (phalcon_array_isset_fetch_long(&pre_conditions, &parameters, 0, PH_COPY)) {
			PHALCON_MM_ADD_ENTRY(&pre_conditions);
			phalcon_array_unset_long(&parameters, 0, 0);
		} else {
			if (phalcon_array_isset_fetch_str(&pre_conditions, &parameters, SL("conditions"), PH_COPY)) {
				PHALCON_MM_ADD_ENTRY(&pre_conditions);
				phalcon_array_unset_str(&parameters, SL("conditions"), 0);
			}
		}

		/**
		 * Re-use bound parameters
		 */
		if (phalcon_array_isset_fetch_str(&placeholders, &parameters, SL("bind"), PH_CTOR)) {
			phalcon_array_unset_str(&parameters, SL("bind"), 0);
		} else {
			array_init(&placeholders);
		}
		PHALCON_MM_ADD_ENTRY(&placeholders);
	} else {
		if (Z_TYPE(parameters) == IS_STRING) {
			PHALCON_MM_ZVAL_COPY(&pre_conditions, &parameters);
		}

		array_init(&placeholders);
		PHALCON_MM_ADD_ENTRY(&placeholders);
	}

	/**
	 * Perform the query on the referenced model
	 */
	PHALCON_MM_CALL_METHOD(&referenced_model, relation, "getreferencedmodel");
	PHALCON_MM_ADD_ENTRY(&referenced_model);

	/**
	 * Check if the relation is direct or through an intermediate model
	 */
	PHALCON_MM_CALL_METHOD(&is_through, relation, "isthrough");
	if (zend_is_true(&is_through)) {
		array_init(&conditions);
		PHALCON_MM_ADD_ENTRY(&conditions);

		PHALCON_MM_CALL_METHOD(&intermediate_model, relation, "getintermediatemodel");
		PHALCON_MM_ADD_ENTRY(&intermediate_model);

		/**
		 * Appends conditions created from the fields defined in the relation
		 */
		PHALCON_MM_CALL_METHOD(&fields, relation, "getfields");
		PHALCON_MM_ADD_ENTRY(&fields);
		if (Z_TYPE(fields) != IS_ARRAY) {
			zval condition = {}, value = {};
			PHALCON_MM_CALL_METHOD(&intermediate_fields, relation, "getintermediatefields");
			PHALCON_MM_ADD_ENTRY(&intermediate_fields);
			PHALCON_MM_CALL_METHOD(&value, record, "readattribute", &fields);

			PHALCON_CONCAT_SVSVS(&condition, "[", &intermediate_model, "].[", &intermediate_fields, "] = ?0");
			phalcon_array_append(&conditions, &condition, 0);
			phalcon_array_append(&placeholders, &value, 0);
		} else {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Not supported");
			return;
		}

		array_init(&join_conditions);
		PHALCON_MM_ADD_ENTRY(&join_conditions);

		/**
		 * Create the join conditions
		 */
		PHALCON_MM_CALL_METHOD(&intermediate_fields, relation, "getintermediatereferencedfields");
		PHALCON_MM_ADD_ENTRY(&intermediate_fields);
		if (Z_TYPE(intermediate_fields) != IS_ARRAY) {
			zval referenced_fields = {}, condition = {};
			PHALCON_MM_CALL_METHOD(&referenced_fields, relation, "getreferencedfields");

			PHALCON_CONCAT_SVSV(&condition, "[", &intermediate_model, "].[", &intermediate_fields);
			PHALCON_SCONCAT_SVSVS(&condition, "] = [", &referenced_model, "].[", &referenced_fields, "]");
			phalcon_array_append(&join_conditions, &condition, 0);
			zval_ptr_dtor(&referenced_fields);
		} else {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Not supported");
			return;
		}

		/**
		 * We don't trust the user or the database so we use bound parameters
		 */
		phalcon_fast_join_str(&joined_join_conditions, SL(" AND "), &join_conditions);
		PHALCON_MM_ADD_ENTRY(&joined_join_conditions);

		/**
		 * Add extra conditions passed by the programmer
		 */
		if (PHALCON_IS_NOT_EMPTY(&pre_conditions)) {
			phalcon_array_append(&conditions, &pre_conditions, PH_COPY);
		}

		/**
		 * We don't trust the user or the database so we use bound parameters
		 */
		phalcon_fast_join_str(&joined_conditions, SL(" AND "), &conditions);
		PHALCON_MM_ADD_ENTRY(&joined_conditions);

		/**
		 * Create a query builder
		 */
		PHALCON_MM_CALL_METHOD(&builder, getThis(), "createbuilder", &parameters);
		PHALCON_MM_ADD_ENTRY(&builder);
		PHALCON_MM_CALL_METHOD(NULL, &builder, "from", &referenced_model);
		PHALCON_MM_CALL_METHOD(NULL, &builder, "innerjoin", &intermediate_model, &joined_join_conditions);
		PHALCON_MM_CALL_METHOD(NULL, &builder, "andwhere", &joined_conditions, &placeholders);

		/**
		 * Get the query
		 */
		PHALCON_MM_CALL_METHOD(&query, &builder, "getquery");
		PHALCON_MM_ADD_ENTRY(&query);

		/**
		 * Execute the query
		 */
		PHALCON_MM_RETURN_CALL_METHOD(&query, "execute");
		RETURN_MM();
	}

	if (PHALCON_IS_NOT_EMPTY(&pre_conditions)) {
		array_init_size(&conditions, 1);
		phalcon_array_append(&conditions, &pre_conditions, PH_COPY);
	} else {
		array_init(&conditions);
	}
	PHALCON_MM_ADD_ENTRY(&conditions);

	/**
	 * Appends conditions created from the fields defined in the relation
	 */
	PHALCON_MM_CALL_METHOD(&fields, relation, "getfields");
	PHALCON_MM_ADD_ENTRY(&fields);
	if (Z_TYPE(fields) != IS_ARRAY) {
		zval value = {}, referenced_field = {}, condition = {};
		PHALCON_MM_CALL_METHOD(&value, record, "readattribute", &fields);
		PHALCON_MM_ADD_ENTRY(&value);
		PHALCON_MM_CALL_METHOD(&referenced_field, relation, "getreferencedfields");

		PHALCON_CONCAT_SVS(&condition, "[", &referenced_field, "] = ?0");
		phalcon_array_append(&conditions, &condition, 0);
		phalcon_array_append(&placeholders, &value, PH_COPY);
		zval_ptr_dtor(&referenced_field);
	} else {
		zval referenced_fields = {};
		/**
		 * Compound relation
		 */
		PHALCON_MM_CALL_METHOD(&referenced_fields, relation, "getreferencedfields");
		PHALCON_MM_ADD_ENTRY(&referenced_fields);
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(fields), idx, str_key, field) {
			zval tmp = {}, value = {}, referenced_field = {}, condition = {};
			if (str_key) {
				ZVAL_STR(&tmp, str_key);
			} else {
				ZVAL_LONG(&tmp, idx);
			}
			PHALCON_MM_CALL_METHOD(&value, record, "readattribute", field);

			phalcon_array_fetch(&referenced_field, &referenced_fields, &tmp, PH_NOISY|PH_READONLY);

			PHALCON_CONCAT_SVSV(&condition, "[", &referenced_field, "] = ?", &tmp);
			phalcon_array_append(&conditions, &condition, 0);
			phalcon_array_append(&placeholders, &value, 0);
		} ZEND_HASH_FOREACH_END();
	}

	PHALCON_MM_CALL_METHOD(&dependency_injector, record, "getdi");
	PHALCON_MM_ADD_ENTRY(&dependency_injector);

	/**
	 * We don't trust the user or the database so we use bound parameters
	 */
	phalcon_fast_join_str(&joined_conditions, SL(" AND "), &conditions);
	PHALCON_MM_ADD_ENTRY(&joined_conditions);

	/**
	 * Create a valid params array to pass to the find/findFirst method
	 */
	array_init_size(&find_params, 3);
	PHALCON_MM_ADD_ENTRY(&find_params);
	phalcon_array_append(&find_params, &joined_conditions, PH_COPY);
	phalcon_array_update_str(&find_params, SL("bind"), &placeholders, PH_COPY);
	phalcon_array_update_str(&find_params, SL("di"), &dependency_injector, PH_COPY);
	if (Z_TYPE(parameters) == IS_ARRAY) {
		phalcon_fast_array_merge(&find_arguments, &find_params, &parameters);
		PHALCON_MM_ADD_ENTRY(&find_arguments);
	} else {
		ZVAL_COPY_VALUE(&find_arguments, &find_params);
	}

	array_init_size(&arguments, 1);
	PHALCON_MM_ADD_ENTRY(&arguments);
	phalcon_array_append(&arguments, &find_arguments, PH_COPY);

	/**
	 * Check the right method to get the data
	 */
	if (Z_TYPE_P(method) == IS_NULL) {
		PHALCON_MM_CALL_METHOD(&type, relation, "gettype");
		PHALCON_MM_ADD_ENTRY(&type);

		switch (phalcon_get_intval(&type)) {

			case 0:
				PHALCON_MM_ZVAL_STRING(&retrieve_method, "findFirst");
				break;

			case 1:
				PHALCON_MM_ZVAL_STRING(&retrieve_method, "findFirst");
				break;

			case 2:
				PHALCON_MM_ZVAL_STRING(&retrieve_method, "find");
				break;

		}
	} else {
		ZVAL_COPY_VALUE(&retrieve_method, method);
	}

	/**
	 * Find first results could be reusable
	 */
	PHALCON_MM_CALL_METHOD(&reusable, relation, "isreusable");
	PHALCON_MM_ADD_ENTRY(&reusable);
	if (zend_is_true(&reusable)) {
		f_reusable = 1;

		phalcon_unique_key(&unique_key, &referenced_model, &arguments);
		PHALCON_MM_ADD_ENTRY(&unique_key);

		PHALCON_MM_CALL_METHOD(&records, getThis(), "getreusablerecords", &referenced_model, &unique_key);
		PHALCON_MM_ADD_ENTRY(&records);
		if (Z_TYPE(records) == IS_ARRAY || Z_TYPE(records) == IS_OBJECT) {
			RETURN_MM_CTOR(&records);
		}
	} else {
		/* Use int variable in order not to confuse static code analysers */
		f_reusable = 0;
	}

	/**
	 * Load the referenced model
	 */
	PHALCON_MM_CALL_METHOD(&referenced_entity, getThis(), "load", &referenced_model);
	PHALCON_MM_ADD_ENTRY(&referenced_entity);

	/**
	 * Call the function in the model
	 */
	array_init_size(&call_object, 2);
	PHALCON_MM_ADD_ENTRY(&call_object);
	phalcon_array_append(&call_object, &referenced_entity, PH_COPY);
	phalcon_array_append(&call_object, &retrieve_method, PH_COPY);

	PHALCON_MM_CALL_USER_FUNC_ARRAY(&records, &call_object, &arguments);
	PHALCON_MM_ADD_ENTRY(&records);

	/**
	 * Store the result in the cache if it's reusable
	 */
	if (f_reusable) {
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "setreusablerecords", &referenced_model, &unique_key, &records);
	}

	RETURN_MM_CTOR(&records);
}

/**
 * Returns a reusable object from the internal list
 *
 * @param string $modelName
 * @param string $key
 * @return object
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getReusableRecords){

	zval *model_name, *key, reusable = {}, records = {};

	phalcon_fetch_params(0, 2, 0, &model_name, &key);

	phalcon_read_property(&reusable, getThis(), SL("_reusable"), PH_NOISY|PH_READONLY);
	if (phalcon_array_isset_fetch(&records, &reusable, key, PH_READONLY)) {
		RETURN_CTOR(&records);
	}

	RETURN_NULL();
}

/**
 * Stores a reusable record in the internal list
 *
 * @param string $modelName
 * @param string $key
 * @param mixed $records
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, setReusableRecords){

	zval *model_name, *key, *records;

	phalcon_fetch_params(0, 3, 0, &model_name, &key, &records);

	phalcon_update_property_array(getThis(), SL("_reusable"), key, records);

}

/**
 * Clears the internal reusable list
 *
 * @param
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, clearReusableObjects){


	phalcon_update_property_null(getThis(), SL("_reusable"));

}

/**
 * Gets belongsTo related records from a model
 *
 * @param string $method
 * @param string $modelName
 * @param string $modelRelation
 * @param Phalcon\Mvc\Model $record
 * @param array $parameters
 * @return Phalcon\Mvc\Model\ResultsetInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getBelongsToRecords){

	zval *method, *model_name, *model_relation, *record, *parameters = NULL, belongs_to = {}, entity_name = {};
	zval entity_relation = {}, key_relation = {}, relations = {}, relation = {};

	phalcon_fetch_params(0, 4, 1, &method, &model_name, &model_relation, &record, &parameters);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&belongs_to, getThis(), SL("_belongsTo"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(belongs_to) == IS_ARRAY) {
		phalcon_fast_strtolower(&entity_name, model_name);
		phalcon_fast_strtolower(&entity_relation, model_relation);

		/**
		 * Check if there is a relation between them
		 */
		PHALCON_CONCAT_VSV(&key_relation, &entity_name, "$", &entity_relation);
		zval_ptr_dtor(&entity_name);
		zval_ptr_dtor(&entity_relation);
		if (!phalcon_array_isset(&belongs_to, &key_relation)) {
			zval_ptr_dtor(&key_relation);
			RETURN_FALSE;
		}

		/**
		 * relations is an array with all the belongsTo relationships to that model
		 */
		phalcon_array_fetch(&relations, &belongs_to, &key_relation, PH_NOISY|PH_READONLY);
		zval_ptr_dtor(&key_relation);

		/**
		 * Get the first relation
		 */
		phalcon_array_fetch_long(&relation, &relations, 0, PH_NOISY|PH_READONLY);

		/**
		 * Perform the query
		 */
		PHALCON_RETURN_CALL_METHOD(getThis(), "getrelationrecords", &relation, method, record, parameters);
		return;
	}

	RETURN_FALSE;
}

/**
 * Gets hasMany related records from a model
 *
 * @param string $method
 * @param string $modelName
 * @param string $modelRelation
 * @param Phalcon\Mvc\Model $record
 * @param array $parameters
 * @return Phalcon\Mvc\Model\ResultsetInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getHasManyRecords){

	zval *method, *model_name, *model_relation, *record, *parameters = NULL, has_many = {}, entity_name = {}, entity_relation = {};
	zval key_relation = {}, relations = {}, relation = {};

	phalcon_fetch_params(0, 4, 1, &method, &model_name, &model_relation, &record, &parameters);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&has_many, getThis(), SL("_hasMany"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(has_many) == IS_ARRAY) {
		phalcon_fast_strtolower(&entity_name, model_name);
		phalcon_fast_strtolower(&entity_relation, model_relation);

		/**
		 * Check if there is a relation between them
		 */
		PHALCON_CONCAT_VSV(&key_relation, &entity_name, "$", &entity_relation);
		zval_ptr_dtor(&entity_name);
		zval_ptr_dtor(&key_relation);
		if (!phalcon_array_isset(&has_many, &key_relation)) {
			zval_ptr_dtor(&key_relation);
			RETURN_FALSE;
		}

		/**
		 * relations is an array with all the belongsTo relationships to that model
		 */
		phalcon_array_fetch(&relations, &has_many, &key_relation, PH_NOISY|PH_READONLY);
		zval_ptr_dtor(&key_relation);

		/**
		 * Get the first relation
		 */
		phalcon_array_fetch_long(&relation, &relations, 0, PH_NOISY|PH_READONLY);

		/**
		 * Perform the query
		 */
		PHALCON_RETURN_CALL_METHOD(getThis(), "getrelationrecords", &relation, method, record, parameters);
		return;
	}

	RETURN_FALSE;
}

/**
 * Gets belongsTo related records from a model
 *
 * @param string $method
 * @param string $modelName
 * @param string $modelRelation
 * @param Phalcon\Mvc\Model $record
 * @param array $parameters
 * @return Phalcon\Mvc\Model\ResultsetInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getHasOneRecords){

	zval *method, *model_name, *model_relation, *record, *parameters = NULL, has_one = {}, entity_name = {}, entity_relation = {};
	zval key_relation = {}, relations = {}, relation = {};

	phalcon_fetch_params(0, 4, 1, &method, &model_name, &model_relation, &record, &parameters);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&has_one, getThis(), SL("_hasOne"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(has_one) == IS_ARRAY) {
		phalcon_fast_strtolower(&entity_name, model_name);
		phalcon_fast_strtolower(&entity_relation, model_relation);

		/**
		 * Check if there is a relation between them
		 */
		PHALCON_CONCAT_VSV(&key_relation, &entity_name, "$", &entity_relation);
		zval_ptr_dtor(&entity_name);
		zval_ptr_dtor(&entity_relation);
		if (!phalcon_array_isset(&has_one, &key_relation)) {
			zval_ptr_dtor(&key_relation);
			RETURN_FALSE;
		}

		/**
		 * relations is an array with all the belongsTo relationships to that model
		 */
		phalcon_array_fetch(&relations, &has_one, &key_relation, PH_NOISY|PH_READONLY);
		zval_ptr_dtor(&key_relation);

		/**
		 * Get the first relation
		 */
		phalcon_array_fetch_long(&relation, &relations, 0, PH_NOISY|PH_READONLY);

		/**
		 * Perform the query
		 */
		PHALCON_RETURN_CALL_METHOD(getThis(), "getrelationrecords", &relation, method, record, parameters);
		return;
	}

	RETURN_FALSE;
}

/**
 * Gets all the belongsTo relations defined in a model
 *
 *<code>
 *	$relations = $modelsManager->getBelongsTo(new Robots());
 *</code>
 *
 * @param  Phalcon\Mvc\ModelInterface $model
 * @return Phalcon\Mvc\Model\RelationInterface[]
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getBelongsTo){

	zval *model, belongs_to_single = {}, lower_name = {}, relations = {};

	phalcon_fetch_params(0, 1, 0, &model);

	phalcon_read_property(&belongs_to_single, getThis(), SL("_belongsToSingle"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(belongs_to_single) == IS_ARRAY) {
		phalcon_get_class(&lower_name, model, 1);
		if (phalcon_array_isset_fetch(&relations, &belongs_to_single, &lower_name, PH_READONLY)) {
			zval_ptr_dtor(&lower_name);
			RETURN_CTOR(&relations);
		}
		zval_ptr_dtor(&lower_name);
	}

	RETURN_EMPTY_ARRAY();
}

/**
 * Gets hasMany relations defined on a model
 *
 * @param  Phalcon\Mvc\ModelInterface $model
 * @return Phalcon\Mvc\Model\RelationInterface[]
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getHasMany){

	zval *model, has_many_single = {}, lower_name = {}, relations = {};

	phalcon_fetch_params(0, 1, 0, &model);

	phalcon_read_property(&has_many_single, getThis(), SL("_hasManySingle"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(has_many_single) == IS_ARRAY) {
		phalcon_get_class(&lower_name, model, 1);
		if (phalcon_array_isset_fetch(&relations, &has_many_single, &lower_name, PH_READONLY)) {
			zval_ptr_dtor(&lower_name);
			RETURN_CTOR(&relations);
		}
		zval_ptr_dtor(&lower_name);
	}

	RETURN_EMPTY_ARRAY();
}

/**
 * Gets hasOne relations defined on a model
 *
 * @param  Phalcon\Mvc\ModelInterface $model
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getHasOne){

	zval *model, has_one_single = {}, lower_name = {}, relations = {};

	phalcon_fetch_params(0, 1, 0, &model);

	phalcon_read_property(&has_one_single, getThis(), SL("_hasOneSingle"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(has_one_single) == IS_ARRAY) {
		phalcon_get_class(&lower_name, model, 1);
		if (phalcon_array_isset_fetch(&relations, &has_one_single, &lower_name, PH_READONLY)) {
			zval_ptr_dtor(&lower_name);
			RETURN_CTOR(&relations);
		}
		zval_ptr_dtor(&lower_name);
	}

	RETURN_EMPTY_ARRAY();
}

/**
 * Gets hasManyToMany relations defined on a model
 *
 * @param  Phalcon\Mvc\ModelInterface $model
 * @return Phalcon\Mvc\Model\RelationInterface[]
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getHasManyToMany){

	zval *model, has_many_to_many_single = {}, lower_name = {}, relations = {};

	phalcon_fetch_params(0, 1, 0, &model);

	phalcon_read_property(&has_many_to_many_single, getThis(), SL("_hasManyToManySingle"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(has_many_to_many_single) == IS_ARRAY) {
		phalcon_get_class(&lower_name, model, 1);
		if (phalcon_array_isset_fetch(&relations, &has_many_to_many_single, &lower_name, PH_READONLY)) {
			zval_ptr_dtor(&lower_name);
			RETURN_CTOR(&relations);
		}
		zval_ptr_dtor(&lower_name);
	}

	RETURN_EMPTY_ARRAY();
}

/**
 * Gets hasOne relations defined on a model
 *
 * @param  Phalcon\Mvc\ModelInterface $model
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getHasOneAndHasMany){

	zval *model, has_one = {}, has_many = {};

	phalcon_fetch_params(0, 1, 0, &model);

	PHALCON_CALL_METHOD(&has_one, getThis(), "gethasone", model);
	PHALCON_CALL_METHOD(&has_many, getThis(), "gethasmany", model);
	phalcon_fast_array_merge(return_value, &has_one, &has_many);
	zval_ptr_dtor(&has_one);
	zval_ptr_dtor(&has_many);
}

/**
 * Query all the relationships defined on a model
 *
 * @param string $modelName
 * @return Phalcon\Mvc\Model\RelationInterface[]
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getRelations){

	zval *model_name, entity_name = {}, all_relations = {}, belongs_to, relations = {}, *relation = NULL, has_many = {}, has_one = {};

	phalcon_fetch_params(0, 1, 0, &model_name);

	phalcon_fast_strtolower(&entity_name, model_name);

	array_init(&all_relations);

	/**
	 * Get belongs-to relations
	 */
	phalcon_read_property(&belongs_to, getThis(), SL("_belongsToSingle"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(belongs_to) == IS_ARRAY && phalcon_array_isset_fetch(&relations, &belongs_to, &entity_name, PH_READONLY)) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(relations), relation) {
			phalcon_array_append(&all_relations, relation, PH_COPY);
		} ZEND_HASH_FOREACH_END();
	}

	/**
	 * Get has-many relations
	 */
	phalcon_read_property(&has_many, getThis(), SL("_hasManySingle"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(has_many) == IS_ARRAY && phalcon_array_isset_fetch(&relations, &has_many, &entity_name, PH_READONLY)) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(relations), relation) {
			phalcon_array_append(&all_relations, relation, PH_COPY);
		} ZEND_HASH_FOREACH_END();
	}

	/**
	 * Get has-one relations
	 */
	phalcon_read_property(&has_one, getThis(), SL("_hasOneSingle"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(has_one) == IS_ARRAY && phalcon_array_isset_fetch(&relations, &has_one, &entity_name, PH_READONLY)) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(relations), relation) {
			phalcon_array_append(&all_relations, relation, PH_COPY);
		} ZEND_HASH_FOREACH_END();
	}
	zval_ptr_dtor(&entity_name);

	RETVAL_ZVAL(&all_relations, 0, 0);
}

/**
 * Query the first relationship defined between two models
 *
 * @param string $first
 * @param string $second
 * @return Phalcon\Mvc\Model\RelationInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getRelationsBetween){

	zval *first, *second, first_name = {}, second_name = {}, key_relation = {}, belongs_to = {}, relations = {}, has_many = {}, has_one = {};

	phalcon_fetch_params(0, 2, 0, &first, &second);

	phalcon_fast_strtolower(&first_name, first);
	phalcon_fast_strtolower(&second_name, second);

	PHALCON_CONCAT_VSV(&key_relation, &first_name, "$", &second_name);
	zval_ptr_dtor(&first_name);
	zval_ptr_dtor(&second_name);

	/**
	 * Check if it's a belongs-to relationship
	 */
	phalcon_read_property(&belongs_to, getThis(), SL("_belongsTo"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(belongs_to) == IS_ARRAY && phalcon_array_isset_fetch(&relations, &belongs_to, &key_relation, PH_READONLY)) {
		zval_ptr_dtor(&key_relation);
		RETURN_CTOR(&relations);
	}

	/**
	 * Check if it's a has-many relationship
	 */
	phalcon_read_property(&has_many, getThis(), SL("_hasMany"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(has_many) == IS_ARRAY) {
		if (phalcon_array_isset_fetch(&relations, &has_many, &key_relation, PH_READONLY)) {
			zval_ptr_dtor(&key_relation);
			RETURN_CTOR(&relations);
		}
	}

	/**
	 * Check if it's a has-one relationship
	 */
	phalcon_read_property(&has_one, getThis(), SL("_hasOne"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(has_one) == IS_ARRAY) {
		if (phalcon_array_isset_fetch(&relations, &has_one, &key_relation, PH_READONLY)) {
			zval_ptr_dtor(&key_relation);
			RETURN_CTOR(&relations);
		}
	}
	zval_ptr_dtor(&key_relation);

	RETURN_FALSE;
}

/**
 * Creates a Phalcon\Mvc\Model\Query without execute it
 *
 * @param string $phql
 * @return Phalcon\Mvc\Model\QueryInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, createQuery){

	zval *phql, dependency_injector = {}, service_name = {}, has = {}, parameters = {}, query = {};

	phalcon_fetch_params(0, 1, 0, &phql);

	PHALCON_CALL_METHOD(&dependency_injector, getThis(), "getdi");
	PHALCON_VERIFY_INTERFACE_EX(&dependency_injector, phalcon_diinterface_ce, phalcon_mvc_model_exception_ce);

	/**
	 * Create a query
	 */
	ZVAL_STR(&service_name, IS(modelsQuery));

	PHALCON_CALL_METHOD(&has, &dependency_injector, "has", &service_name);
	if (zend_is_true(&has)) {
		array_init(&parameters);

		phalcon_array_append(&parameters, phql, PH_COPY);
		phalcon_array_append(&parameters, &dependency_injector, PH_COPY);

		PHALCON_CALL_METHOD(&query, &dependency_injector, "get", &service_name, &parameters);
		zval_ptr_dtor(&parameters);
	} else {
		object_init_ex(&query, phalcon_mvc_model_query_ce);
		PHALCON_CALL_METHOD(NULL, &query, "__construct", phql, &dependency_injector);
	}
	zval_ptr_dtor(&dependency_injector);

	phalcon_update_property(getThis(), SL("_lastQuery"), &query);

	RETVAL_ZVAL(&query, 0, 0);
}

/**
 * Creates a Phalcon\Mvc\Model\Query and execute it
 *
 * @param string $phql
 * @param array $placeholders
 * @return Phalcon\Mvc\Model\QueryInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, executeQuery){

	zval *phql, *placeholders = NULL, *types = NULL, query = {};

	phalcon_fetch_params(1, 1, 2, &phql, &placeholders, &types);

	if (!placeholders) {
		placeholders = &PHALCON_GLOBAL(z_null);
	}

	if (!types) {
		types = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_MM_CALL_METHOD(&query, getThis(), "createquery", phql);
	PHALCON_MM_ADD_ENTRY(&query);
	phalcon_update_property(getThis(), SL("_lastQuery"), &query);

	/**
	 * Execute the query
	 */
	PHALCON_MM_RETURN_CALL_METHOD(&query, "execute", placeholders, types);
	RETURN_MM();
}

/**
 * Creates a Phalcon\Mvc\Model\Query\Builder
 *
 * @param string $params
 * @return Phalcon\Mvc\Model\Query\BuilderInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, createBuilder){

	zval *params = NULL, *_type = NULL, type = {};

	phalcon_fetch_params(0, 0, 2, &params, &_type);

	if (!_type) {
		phalcon_get_class_constant(&type, phalcon_mvc_model_query_ce, SL("TYPE_SELECT"));
	} else {
		ZVAL_COPY(&type, _type);
	}

	if (params) {
		PHALCON_CALL_CE_STATIC(return_value, phalcon_mvc_model_query_builder_ce, "create", &type, params);
	} else {
		PHALCON_CALL_CE_STATIC(return_value, phalcon_mvc_model_query_builder_ce, "create", &type);
	}
	zval_ptr_dtor(&type);
}

/**
 * Returns the lastest query created or executed in the models manager
 *
 * @return Phalcon\Mvc\Model\QueryInterface
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getLastQuery){


	RETURN_MEMBER(getThis(), "_lastQuery");
}

/**
 * Registers shorter aliases for namespaces in PHQL statements
 *
 * @param string $alias
 * @param string $namespace
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, registerNamespaceAlias){

	zval *alias, *namespace;

	phalcon_fetch_params(0, 2, 0, &alias, &namespace);

	if (Z_TYPE_P(alias) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The namespace alias must be a string");
		return;
	}
	if (Z_TYPE_P(namespace) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The namespace must be a string");
		return;
	}

	phalcon_update_property_array(getThis(), SL("_namespaceAliases"), alias, namespace);

}

/**
 * Returns a real namespace from its alias
 *
 * @param string $alias
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getNamespaceAlias){

	zval *alias, namespace_aliases = {}, namespace = {}, exception_message = {};

	phalcon_fetch_params(0, 1, 0, &alias);

	phalcon_read_property(&namespace_aliases, getThis(), SL("_namespaceAliases"), PH_NOISY|PH_READONLY);
	if (phalcon_array_isset_fetch(&namespace, &namespace_aliases, alias, PH_READONLY)) {
		RETURN_CTOR(&namespace);
	}

	PHALCON_CONCAT_SVS(&exception_message, "Namespace alias '", alias, "' is not registered");
	PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
}

/**
 * Returns all the registered namespace aliases
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, getNamespaceAliases){

	RETURN_MEMBER(getThis(), "_namespaceAliases");
}

/**
 * Destroys the PHQL cache
 */
PHP_METHOD(Phalcon_Mvc_Model_Manager, __destruct){

	phalcon_orm_destroy_cache();
}
