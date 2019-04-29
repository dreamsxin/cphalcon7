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

#include "mvc/model.h"
#include "mvc/modelinterface.h"
#include "mvc/model/criteria.h"
#include "mvc/model/exception.h"
#include "mvc/model/managerinterface.h"
#include "mvc/model/manager.h"
#include "mvc/model/metadatainterface.h"
#include "mvc/model/metadata/memory.h"
#include "mvc/model/query/builder.h"
#include "mvc/model/query.h"
#include "mvc/model/resultinterface.h"
#include "mvc/model/resultsetinterface.h"
#include "mvc/model/validationfailed.h"
#include "mvc/model/criteria.h"
#include "di.h"
#include "diinterface.h"
#include "di/injectable.h"
#include "events/eventsawareinterface.h"
#include "db/column.h"
#include "db/rawvalue.h"
#include "db/adapterinterface.h"
#include "filterinterface.h"
#include "validationinterface.h"
#include "validation/message/group.h"
#include "validation/message.h"
#include "validation.h"
#include "debug.h"

#include <Zend/zend_closures.h>
#include <ext/pdo/php_pdo_driver.h>

#ifdef PHALCON_USE_PHP_JSON
#include <ext/json/php_json.h>
#endif

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/hash.h"
#include "kernel/array.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/file.h"
#include "kernel/variables.h"
#include "kernel/debug.h"

#include "interned-strings.h"

/**
 * Phalcon\Mvc\Model
 *
 * <p>Phalcon\Mvc\Model connects business objects and database tables to create
 * a persistable domain model where logic and data are presented in one wrapping.
 * It‘s an implementation of the object-relational mapping (ORM).</p>
 *
 * <p>A model represents the information (data) of the application and the rules to manipulate that data.
 * Models are primarily used for managing the rules of interaction with a corresponding database table.
 * In most cases, each table in your database will correspond to one model in your application.
 * The bulk of your application’s business logic will be concentrated in the models.</p>
 *
 * <p>Phalcon\Mvc\Model is the first ORM written in C-language for PHP, giving to developers high performance
 * when interacting with databases while is also easy to use.</p>
 *
 * <code>
 *
 * $robot = new Robots();
 * $robot->type = 'mechanical';
 * $robot->name = 'Astro Boy';
 * $robot->year = 1952;
 * if ($robot->save() == false) {
 *  echo "Umh, We can store robots: ";
 *  foreach ($robot->getMessages() as $message) {
 *    echo $message;
 *  }
 * } else {
 *  echo "Great, a new robot was saved successfully!";
 * }
 * </code>
 *
 */
zend_class_entry *phalcon_mvc_model_ce;

PHP_METHOD(Phalcon_Mvc_Model, __construct);
PHP_METHOD(Phalcon_Mvc_Model, setEventsManager);
PHP_METHOD(Phalcon_Mvc_Model, getEventsManager);
PHP_METHOD(Phalcon_Mvc_Model, getModelsMetaData);
PHP_METHOD(Phalcon_Mvc_Model, getModelsManager);
PHP_METHOD(Phalcon_Mvc_Model, setTransaction);
PHP_METHOD(Phalcon_Mvc_Model, getTransaction);
PHP_METHOD(Phalcon_Mvc_Model, setSource);
PHP_METHOD(Phalcon_Mvc_Model, getSource);
PHP_METHOD(Phalcon_Mvc_Model, setSchema);
PHP_METHOD(Phalcon_Mvc_Model, getSchema);
PHP_METHOD(Phalcon_Mvc_Model, getIdentityField);
PHP_METHOD(Phalcon_Mvc_Model, getColumnMap);
PHP_METHOD(Phalcon_Mvc_Model, getReverseColumnMap);
PHP_METHOD(Phalcon_Mvc_Model, getAttributes);
PHP_METHOD(Phalcon_Mvc_Model, getPrimaryKeyAttributes);
PHP_METHOD(Phalcon_Mvc_Model, getNonPrimaryKeyAttributes);
PHP_METHOD(Phalcon_Mvc_Model, getNotNullAttributes);
PHP_METHOD(Phalcon_Mvc_Model, getDataTypesNumeric);
PHP_METHOD(Phalcon_Mvc_Model, isNotNull);
PHP_METHOD(Phalcon_Mvc_Model, getDataTypes);
PHP_METHOD(Phalcon_Mvc_Model, getDataSize);
PHP_METHOD(Phalcon_Mvc_Model, getDataByte);
PHP_METHOD(Phalcon_Mvc_Model, getDataScale);
PHP_METHOD(Phalcon_Mvc_Model, getBindTypes);
PHP_METHOD(Phalcon_Mvc_Model, getDefaultValues);
PHP_METHOD(Phalcon_Mvc_Model, getAutomaticCreateAttributes);
PHP_METHOD(Phalcon_Mvc_Model, getAutomaticUpdateAttributes);
PHP_METHOD(Phalcon_Mvc_Model, hasRealAttribute);
PHP_METHOD(Phalcon_Mvc_Model, getRealAttribute);
PHP_METHOD(Phalcon_Mvc_Model, hasAttribute);
PHP_METHOD(Phalcon_Mvc_Model, getAttribute);
PHP_METHOD(Phalcon_Mvc_Model, setFieldValue);
PHP_METHOD(Phalcon_Mvc_Model, getFieldValue);
PHP_METHOD(Phalcon_Mvc_Model, setConnectionService);
PHP_METHOD(Phalcon_Mvc_Model, setReadConnectionService);
PHP_METHOD(Phalcon_Mvc_Model, setWriteConnectionService);
PHP_METHOD(Phalcon_Mvc_Model, getReadConnectionService);
PHP_METHOD(Phalcon_Mvc_Model, getWriteConnectionService);
PHP_METHOD(Phalcon_Mvc_Model, setDirtyState);
PHP_METHOD(Phalcon_Mvc_Model, getDirtyState);
PHP_METHOD(Phalcon_Mvc_Model, getReadConnection);
PHP_METHOD(Phalcon_Mvc_Model, getWriteConnection);
PHP_METHOD(Phalcon_Mvc_Model, assign);
PHP_METHOD(Phalcon_Mvc_Model, cloneResultMap);
PHP_METHOD(Phalcon_Mvc_Model, cloneResultMapHydrate);
PHP_METHOD(Phalcon_Mvc_Model, cloneResult);
PHP_METHOD(Phalcon_Mvc_Model, find);
PHP_METHOD(Phalcon_Mvc_Model, findFirst);
PHP_METHOD(Phalcon_Mvc_Model, query);
PHP_METHOD(Phalcon_Mvc_Model, build);
PHP_METHOD(Phalcon_Mvc_Model, getUniqueKey);
PHP_METHOD(Phalcon_Mvc_Model, getUniqueParams);
PHP_METHOD(Phalcon_Mvc_Model, getUniqueTypes);
PHP_METHOD(Phalcon_Mvc_Model, _reBuild);
PHP_METHOD(Phalcon_Mvc_Model, exists);
PHP_METHOD(Phalcon_Mvc_Model, _groupResult);
PHP_METHOD(Phalcon_Mvc_Model, group);
PHP_METHOD(Phalcon_Mvc_Model, count);
PHP_METHOD(Phalcon_Mvc_Model, sum);
PHP_METHOD(Phalcon_Mvc_Model, maximum);
PHP_METHOD(Phalcon_Mvc_Model, minimum);
PHP_METHOD(Phalcon_Mvc_Model, average);
PHP_METHOD(Phalcon_Mvc_Model, fireEvent);
PHP_METHOD(Phalcon_Mvc_Model, fireEventCancel);
PHP_METHOD(Phalcon_Mvc_Model, _cancelOperation);
PHP_METHOD(Phalcon_Mvc_Model, appendMessage);
PHP_METHOD(Phalcon_Mvc_Model, validate);
PHP_METHOD(Phalcon_Mvc_Model, validationHasFailed);
PHP_METHOD(Phalcon_Mvc_Model, getMessages);
PHP_METHOD(Phalcon_Mvc_Model, _checkForeignKeysRestrict);
PHP_METHOD(Phalcon_Mvc_Model, _checkForeignKeysReverseRestrict);
PHP_METHOD(Phalcon_Mvc_Model, _checkForeignKeysReverseCascade);
PHP_METHOD(Phalcon_Mvc_Model, _preSave);
PHP_METHOD(Phalcon_Mvc_Model, _postSave);
PHP_METHOD(Phalcon_Mvc_Model, _doLowInsert);
PHP_METHOD(Phalcon_Mvc_Model, _doLowUpdate);
PHP_METHOD(Phalcon_Mvc_Model, _preSaveRelatedRecords);
PHP_METHOD(Phalcon_Mvc_Model, _postSaveRelatedRecords);
PHP_METHOD(Phalcon_Mvc_Model, save);
PHP_METHOD(Phalcon_Mvc_Model, create);
PHP_METHOD(Phalcon_Mvc_Model, update);
PHP_METHOD(Phalcon_Mvc_Model, delete);
PHP_METHOD(Phalcon_Mvc_Model, getOperationMade);
PHP_METHOD(Phalcon_Mvc_Model, refresh);
PHP_METHOD(Phalcon_Mvc_Model, skipOperation);
PHP_METHOD(Phalcon_Mvc_Model, readAttribute);
PHP_METHOD(Phalcon_Mvc_Model, writeAttribute);
PHP_METHOD(Phalcon_Mvc_Model, skipAttributes);
PHP_METHOD(Phalcon_Mvc_Model, skipAttributesOnCreate);
PHP_METHOD(Phalcon_Mvc_Model, getSkipAttributesOnCreate);
PHP_METHOD(Phalcon_Mvc_Model, skipAttributesOnUpdate);
PHP_METHOD(Phalcon_Mvc_Model, getSkipAttributesOnUpdate);
PHP_METHOD(Phalcon_Mvc_Model, hasOne);
PHP_METHOD(Phalcon_Mvc_Model, belongsTo);
PHP_METHOD(Phalcon_Mvc_Model, hasMany);
PHP_METHOD(Phalcon_Mvc_Model, hasManyToMany);
PHP_METHOD(Phalcon_Mvc_Model, addBehavior);
PHP_METHOD(Phalcon_Mvc_Model, setSnapshotData);
PHP_METHOD(Phalcon_Mvc_Model, hasSnapshotData);
PHP_METHOD(Phalcon_Mvc_Model, getSnapshotData);
PHP_METHOD(Phalcon_Mvc_Model, hasChanged);
PHP_METHOD(Phalcon_Mvc_Model, getChangedFields);
PHP_METHOD(Phalcon_Mvc_Model, useDynamicUpdate);
PHP_METHOD(Phalcon_Mvc_Model, getRelated);
PHP_METHOD(Phalcon_Mvc_Model, _getRelatedRecords);
PHP_METHOD(Phalcon_Mvc_Model, __call);
PHP_METHOD(Phalcon_Mvc_Model, __callStatic);
PHP_METHOD(Phalcon_Mvc_Model, __set);
PHP_METHOD(Phalcon_Mvc_Model, __get);
PHP_METHOD(Phalcon_Mvc_Model, __isset);
PHP_METHOD(Phalcon_Mvc_Model, serialize);
PHP_METHOD(Phalcon_Mvc_Model, unserialize);
PHP_METHOD(Phalcon_Mvc_Model, dump);
PHP_METHOD(Phalcon_Mvc_Model, getFieldValues);
PHP_METHOD(Phalcon_Mvc_Model, toArray);
PHP_METHOD(Phalcon_Mvc_Model, setup);
PHP_METHOD(Phalcon_Mvc_Model, remove);
PHP_METHOD(Phalcon_Mvc_Model, reset);
PHP_METHOD(Phalcon_Mvc_Model, filter);
PHP_METHOD(Phalcon_Mvc_Model, isRecord);
PHP_METHOD(Phalcon_Mvc_Model, isNewRecord);
PHP_METHOD(Phalcon_Mvc_Model, isDeletedRecord);
PHP_METHOD(Phalcon_Mvc_Model, jsonSerialize);
PHP_METHOD(Phalcon_Mvc_Model, __debugInfo);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, dependencyInjector)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_setsource, 0, 0, 1)
	ZEND_ARG_INFO(0, source)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_setschema, 0, 0, 1)
	ZEND_ARG_INFO(0, schema)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_setdirtystate, 0, 0, 1)
	ZEND_ARG_INFO(0, dirtyState)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_group, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, params, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_validate, 0, 0, 1)
	ZEND_ARG_INFO(0, validation)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_skipoperation, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, skip, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_skipattributes, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, attributes, IS_ARRAY, 0)
	ZEND_ARG_INFO(0, replace)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_skipattributesoncreate, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, attributes, IS_ARRAY, 0)
	ZEND_ARG_INFO(0, replace)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_skipattributesonupdate, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, attributes, IS_ARRAY, 0)
	ZEND_ARG_INFO(0, replace)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_hasone, 0, 0, 3)
	ZEND_ARG_INFO(0, fields)
	ZEND_ARG_INFO(0, referenceModel)
	ZEND_ARG_INFO(0, referencedFields)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_belongsto, 0, 0, 3)
	ZEND_ARG_INFO(0, fields)
	ZEND_ARG_INFO(0, referenceModel)
	ZEND_ARG_INFO(0, referencedFields)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_hasmany, 0, 0, 3)
	ZEND_ARG_INFO(0, fields)
	ZEND_ARG_INFO(0, referenceModel)
	ZEND_ARG_INFO(0, referencedFields)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_hasmanytomany, 0, 0, 6)
	ZEND_ARG_INFO(0, fields)
	ZEND_ARG_INFO(0, intermediateModel)
	ZEND_ARG_INFO(0, intermediateFields)
	ZEND_ARG_INFO(0, intermediateReferencedFields)
	ZEND_ARG_INFO(0, referenceModel)
	ZEND_ARG_INFO(0, referencedFields)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_addbehavior, 0, 0, 1)
	ZEND_ARG_INFO(0, behavior)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_setsnapshotdata, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, columnMap)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_haschanged, 0, 0, 0)
	ZEND_ARG_INFO(0, fieldName)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_usedynamicupdate, 0, 0, 1)
	ZEND_ARG_INFO(0, dynamicUpdate)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model___call, 0, 0, 1)
	ZEND_ARG_INFO(0, method)
	ZEND_ARG_INFO(0, arguments)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model___callstatic, 0, 0, 1)
	ZEND_ARG_INFO(0, method)
	ZEND_ARG_INFO(0, arguments)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model___set, 0, 0, 2)
	ZEND_ARG_INFO(0, property)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model___get, 0, 0, 1)
	ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model___isset, 0, 0, 1)
	ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_unserialize, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_setup, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_toarray, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, columns, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, mustColumn, _IS_BOOL, 1)
	ZEND_ARG_TYPE_INFO(0, negate, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_filter, 0, 0, 2)
	ZEND_ARG_INFO(0, field)
	ZEND_ARG_INFO(0, filters)
	ZEND_ARG_INFO(0, defaultValue)
	ZEND_ARG_INFO(0, notAllowEmpty)
	ZEND_ARG_TYPE_INFO(0, recursive, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model, __construct, arginfo_phalcon_mvc_model___construct, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Model, setEventsManager, arginfo_phalcon_events_eventsawareinterface_seteventsmanager, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getEventsManager, arginfo_phalcon_events_eventsawareinterface_geteventsmanager, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getModelsMetaData, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getModelsManager, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, setTransaction, arginfo_phalcon_mvc_modelinterface_settransaction, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getTransaction, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, setSource, arginfo_phalcon_mvc_model_setsource, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getSource, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, setSchema, arginfo_phalcon_mvc_model_setschema, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getSchema, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getIdentityField, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getColumnMap, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getReverseColumnMap, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getAttributes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getPrimaryKeyAttributes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getNonPrimaryKeyAttributes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getNotNullAttributes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getDataTypesNumeric, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, isNotNull, arginfo_phalcon_mvc_modelinterface_isnotnull, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getDataTypes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getDataSize, arginfo_phalcon_mvc_modelinterface_getdatasize, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getDataByte, arginfo_phalcon_mvc_modelinterface_getdatabyte, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getDataScale, arginfo_phalcon_mvc_modelinterface_getdatascale, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getBindTypes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getDefaultValues, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getAutomaticCreateAttributes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getAutomaticUpdateAttributes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, hasRealAttribute, arginfo_phalcon_mvc_modelinterface_hasrealattribute, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getRealAttribute, arginfo_phalcon_mvc_modelinterface_getrealattribute, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, hasAttribute, arginfo_phalcon_mvc_modelinterface_hasattribute, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getAttribute, arginfo_phalcon_mvc_modelinterface_getattribute, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, setFieldValue, arginfo_phalcon_mvc_modelinterface_setfieldvalue, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getFieldValue, arginfo_phalcon_mvc_modelinterface_getfieldvalue, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, setConnectionService, arginfo_phalcon_mvc_modelinterface_setconnectionservice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, setReadConnectionService, arginfo_phalcon_mvc_modelinterface_setreadconnectionservice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, setWriteConnectionService, arginfo_phalcon_mvc_modelinterface_setwriteconnectionservice, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getReadConnectionService, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getWriteConnectionService, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, setDirtyState, arginfo_phalcon_mvc_model_setdirtystate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getDirtyState, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getReadConnection, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getWriteConnection, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, assign, arginfo_phalcon_mvc_modelinterface_assign, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, cloneResultMap, arginfo_phalcon_mvc_modelinterface_cloneresultmap, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model, cloneResultMapHydrate, arginfo_phalcon_mvc_modelinterface_cloneresultmaphydrate, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model, cloneResult, arginfo_phalcon_mvc_modelinterface_cloneresult, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model, find, arginfo_phalcon_mvc_modelinterface_find, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model, findFirst, arginfo_phalcon_mvc_modelinterface_findfirst, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model, query, arginfo_phalcon_mvc_modelinterface_query, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model, build, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getUniqueKey, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getUniqueParams, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getUniqueTypes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, exists, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, _reBuild, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, _groupResult, NULL, ZEND_ACC_PROTECTED|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model, group, arginfo_phalcon_mvc_model_group, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model, count, arginfo_phalcon_mvc_modelinterface_count, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model, sum, arginfo_phalcon_mvc_modelinterface_sum, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model, maximum, arginfo_phalcon_mvc_modelinterface_maximum, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model, minimum, arginfo_phalcon_mvc_modelinterface_minimum, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model, average, arginfo_phalcon_mvc_modelinterface_average, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model, fireEvent, arginfo_phalcon_di_injectable_fireevent, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, fireEventCancel, arginfo_phalcon_di_injectable_fireeventcancel, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, _cancelOperation, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, appendMessage, arginfo_phalcon_mvc_modelinterface_appendmessage, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, validate, arginfo_phalcon_mvc_model_validate, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, validationHasFailed, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getMessages, arginfo_phalcon_mvc_modelinterface_getmessages, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, _checkForeignKeysRestrict, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, _checkForeignKeysReverseRestrict, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, _checkForeignKeysReverseCascade, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, _preSave, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, _postSave, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, _doLowInsert, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, _doLowUpdate, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, _preSaveRelatedRecords, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, _postSaveRelatedRecords, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, save, arginfo_phalcon_mvc_modelinterface_save, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, create, arginfo_phalcon_mvc_modelinterface_create, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, update, arginfo_phalcon_mvc_modelinterface_update, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, delete, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getOperationMade, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, refresh, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, skipOperation, arginfo_phalcon_mvc_model_skipoperation, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, readAttribute, arginfo_phalcon_mvc_modelinterface_readattribute, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, writeAttribute, arginfo_phalcon_mvc_modelinterface_writeattribute, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, skipAttributes, arginfo_phalcon_mvc_model_skipattributes, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, skipAttributesOnCreate, arginfo_phalcon_mvc_model_skipattributesoncreate, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, getSkipAttributesOnCreate, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, skipAttributesOnUpdate, arginfo_phalcon_mvc_model_skipattributesonupdate, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, getSkipAttributesOnUpdate, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, hasOne, arginfo_phalcon_mvc_model_hasone, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, belongsTo, arginfo_phalcon_mvc_model_belongsto, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, hasMany, arginfo_phalcon_mvc_model_hasmany, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, hasManyToMany, arginfo_phalcon_mvc_model_hasmanytomany, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, addBehavior, arginfo_phalcon_mvc_model_addbehavior, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, setSnapshotData, arginfo_phalcon_mvc_model_setsnapshotdata, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, hasSnapshotData, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getSnapshotData, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, hasChanged, arginfo_phalcon_mvc_model_haschanged, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getChangedFields, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, useDynamicUpdate, arginfo_phalcon_mvc_model_usedynamicupdate, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, getRelated, arginfo_phalcon_mvc_modelinterface_getrelated, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, _getRelatedRecords, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, __call, arginfo_phalcon_mvc_model___call, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, __callStatic, arginfo_phalcon_mvc_model___callstatic, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model, __set, arginfo_phalcon_mvc_model___set, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, __get, arginfo_phalcon_mvc_model___get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, __isset, arginfo_phalcon_mvc_model___isset, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, serialize, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, unserialize, arginfo_phalcon_mvc_model_unserialize, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, dump, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getFieldValues, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, toArray, arginfo_phalcon_mvc_model_toarray, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, setup, arginfo_phalcon_mvc_model_setup, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model, remove, arginfo_phalcon_mvc_modelinterface_remove, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model, reset, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, filter, arginfo_phalcon_mvc_model_filter, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, isRecord, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, isNewRecord, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, isDeletedRecord, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, __debugInfo, NULL, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Mvc_Model, setDbService, setConnectionService, arginfo_phalcon_mvc_modelinterface_setconnectionservice, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Mvc_Model, getRealAttributes, getAttributes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, jsonSerialize, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc, Model, mvc_model, phalcon_di_injectable_ce, phalcon_mvc_model_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_mvc_model_ce, SL("_errorMessages"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_mvc_model_ce, SL("_operationMade"), PHALCON_MODEL_OP_NONE, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_mvc_model_ce, SL("_dirtyState"), PHALCON_MODEL_DIRTY_STATE_TRANSIENT, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_transaction"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_uniqueKey"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_uniqueParams"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_uniqueTypes"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_skipped"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_related"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_snapshot"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_mvc_model_ce, SL("_seenRawvalues"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_filter"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_relatedResult"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_columnMap"), ZEND_ACC_PROTECTED);

	zend_declare_class_constant_long(phalcon_mvc_model_ce, SL("OP_NONE"), PHALCON_MODEL_OP_NONE);
	zend_declare_class_constant_long(phalcon_mvc_model_ce, SL("OP_CREATE"), PHALCON_MODEL_OP_CREATE);
	zend_declare_class_constant_long(phalcon_mvc_model_ce, SL("OP_UPDATE"), PHALCON_MODEL_OP_UPDATE);
	zend_declare_class_constant_long(phalcon_mvc_model_ce, SL("OP_DELETE"), PHALCON_MODEL_OP_DELETE);
	zend_declare_class_constant_long(phalcon_mvc_model_ce, SL("DIRTY_STATE_PERSISTENT"), PHALCON_MODEL_DIRTY_STATE_PERSISTEN);
	zend_declare_class_constant_long(phalcon_mvc_model_ce, SL("DIRTY_STATE_TRANSIENT"), PHALCON_MODEL_DIRTY_STATE_TRANSIENT);
	zend_declare_class_constant_long(phalcon_mvc_model_ce, SL("DIRTY_STATE_DETACHED"), PHALCON_MODEL_DIRTY_STATE_DETACHED);

	zend_class_implements(phalcon_mvc_model_ce, 3, phalcon_mvc_modelinterface_ce, phalcon_mvc_model_resultinterface_ce, zend_ce_serializable);

#ifdef PHALCON_USE_PHP_JSON
	zend_class_implements(phalcon_mvc_model_ce, 1, php_json_serializable_ce);
#endif
	return SUCCESS;
}

/**
 * <code>
 * 	private function getMessagesFromModel($model, $target)
 * 	{
 * 		$messages = $model->getMessages();
 * 		foreach ($messages as $message) {
 * 			if (is_object($message)) {
 * 				$message->setModel($target);
 * 			}
 *
 * 			$this->appendMessage($message);
 * 		}
 * 	}
 * </code>
 */
static int phalcon_mvc_model_get_messages_from_model(zval *this_ptr, zval *model, zval *target)
{
	zval messages, *message;

	if (
		phalcon_call_method(&messages, model, "getmessages", 0, NULL) == FAILURE
		|| Z_TYPE(messages) != IS_ARRAY
	) {
		return FAILURE;
	}

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(messages), message) {
		zval *params[] = { message };
		if (FAILURE == phalcon_call_method(NULL, this_ptr, "appendmessage", 1, params)) {
			break;
		}
	} ZEND_HASH_FOREACH_END();

	return likely(!EG(exception)) ? SUCCESS : FAILURE;
}

/**
 * Phalcon\Mvc\Model constructor
 *
 * @param Phalcon\DiInterface $dependencyInjector
 * @param Phalcon\Mvc\Model\ManagerInterface $modelsManager
 */
PHP_METHOD(Phalcon_Mvc_Model, __construct){

	zval *data = NULL, *dependency_injector = NULL, models_manager = {};

	phalcon_fetch_params(0, 0, 2, &data, &dependency_injector);

	if (dependency_injector && Z_TYPE_P(dependency_injector) != IS_NULL) {
		PHALCON_CALL_METHOD(NULL, getThis(), "setdi", dependency_injector);
	}

	PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");

	/**
	 * The manager always initializes the object
	 */
	PHALCON_CALL_METHOD(NULL, &models_manager, "initialize", getThis());
	zval_ptr_dtor(&models_manager);

	/**
	 * This allows the developer to execute initialization stuff every time an instance
	 * is created
	 */
	if (phalcon_method_exists_ex(getThis(), SL("onconstruct")) == SUCCESS) {
		PHALCON_CALL_METHOD(NULL, getThis(), "onconstruct");
	}

	if (data && Z_TYPE_P(data) == IS_ARRAY) {
		PHALCON_CALL_METHOD(NULL, getThis(), "assign", data);
	}
}

/**
 * Sets a custom events manager
 *
 * @param Phalcon\Events\ManagerInterface $eventsManager
 */
PHP_METHOD(Phalcon_Mvc_Model, setEventsManager){

	zval *events_manager, models_manager = {};

	phalcon_fetch_params(0, 1, 0, &events_manager);

	PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_CALL_METHOD(NULL, &models_manager, "setcustomeventsmanager", getThis(), events_manager);
	zval_ptr_dtor(&models_manager);
}

/**
 * Returns the custom events manager
 *
 * @return Phalcon\Events\ManagerInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, getEventsManager){

	zval models_manager = {};

	PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_CALL_METHOD(return_value, &models_manager, "getcustomeventsmanager", getThis());
	zval_ptr_dtor(&models_manager);
	if (Z_TYPE_P(return_value) == IS_NULL) {
		PHALCON_CALL_PARENT(return_value, phalcon_mvc_model_ce, getThis(), "geteventsmanager");
	}
}

/**
 * Returns the models meta-data service related to the entity instance
 *
 * @return Phalcon\Mvc\Model\MetaDataInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, getModelsMetaData){

	zval service_name = {};

	ZVAL_STR(&service_name, IS(modelsMetadata));

	PHALCON_CALL_METHOD(return_value, getThis(), "getresolveservice", &service_name, &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_true));
	if (Z_TYPE_P(return_value) != IS_OBJECT) {
		zval_ptr_dtor(return_value);
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The injected service 'modelsMetadata' is not object (1)");
		return;
	}
	PHALCON_VERIFY_INTERFACE(return_value, phalcon_mvc_model_metadatainterface_ce);
}

/**
 * Returns the models manager related to the entity instance
 *
 * @return Phalcon\Mvc\Model\ManagerInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, getModelsManager){

	zval service_name = {};

	ZVAL_STR(&service_name, IS(modelsManager));
	PHALCON_CALL_METHOD(return_value, getThis(), "getresolveservice", &service_name, &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_true));

	if (Z_TYPE_P(return_value) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The injected service 'modelsManager' is not object (1)");
		return;
	}
	PHALCON_VERIFY_INTERFACE(return_value, phalcon_mvc_model_managerinterface_ce);
}

/**
 * Sets a transaction related to the Model instance
 *
 *<code>
 *use Phalcon\Mvc\Model\Transaction\Manager as TxManager;
 *use Phalcon\Mvc\Model\Transaction\Failed as TxFailed;
 *
 *try {
 *
 *  $txManager = new TxManager();
 *
 *  $transaction = $txManager->get();
 *
 *  $robot = new Robots();
 *  $robot->setTransaction($transaction);
 *  $robot->name = 'WALL·E';
 *  $robot->created_at = date('Y-m-d');
 *  if ($robot->save() == false) {
 *    $transaction->rollback("Can't save robot");
 *  }
 *
 *  $robotPart = new RobotParts();
 *  $robotPart->setTransaction($transaction);
 *  $robotPart->type = 'head';
 *  if ($robotPart->save() == false) {
 *    $transaction->rollback("Robot part cannot be saved");
 *  }
 *
 *  $transaction->commit();
 *
 *} catch (TxFailed $e) {
 *  echo 'Failed, reason: ', $e->getMessage();
 *}
 *
 *</code>
 *
 * @param Phalcon\Mvc\Model\TransactionInterface $transaction
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, setTransaction){

	zval *transaction;

	phalcon_fetch_params(0, 1, 0, &transaction);

	if (Z_TYPE_P(transaction) == IS_OBJECT) {
		phalcon_update_property(getThis(), SL("_transaction"), transaction);
		RETURN_THIS();
	}
	PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Transaction should be an object");
	return;
}

/**
 * Returns a transaction related in the Model instance
 *
 * @return Phalcon\Mvc\Model\TransactionInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, getTransaction){

	RETURN_MEMBER(getThis(), "_transaction");
}

/**
 * Sets table name which model should be mapped
 *
 * @param string $source
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, setSource){

	zval *source, models_manager;

	phalcon_fetch_params(0, 1, 0, &source);

	PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_CALL_METHOD(NULL, &models_manager, "setmodelsource", getThis(), source);
	zval_ptr_dtor(&models_manager);
	RETURN_THIS();
}

/**
 * Returns table name mapped in the model
 *
 * @param Phalcon\Mvc\Model\Query $query
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model, getSource){

	zval *query = NULL, models_manager = {};

	phalcon_fetch_params(1, 0, 1, &query);

	if (!query) {
		query = &PHALCON_GLOBAL(z_null);
	}

	if (phalcon_method_exists_ex(getThis(), SL("selectsource")) == SUCCESS) {
		PHALCON_MM_CALL_METHOD(return_value, getThis(), "selectsource", query);
		if (Z_TYPE_P(return_value) != IS_STRING) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "'selectSource' didn't returned a valid source");
		}
		return;
	}

	PHALCON_MM_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_MM_ADD_ENTRY(&models_manager);
	PHALCON_MM_CALL_METHOD(return_value, &models_manager, "getmodelsource", getThis());
	RETURN_MM();
}

/**
 * Sets schema name where table mapped is located
 *
 * @param string $schema
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, setSchema){

	zval *schema, models_manager;

	phalcon_fetch_params(0, 1, 0, &schema);

	PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_CALL_METHOD(NULL, &models_manager, "setmodelschema", getThis(), schema);
	zval_ptr_dtor(&models_manager);
	RETURN_THIS();
}

/**
 * Returns schema name where table mapped is located
 *
 * @param Phalcon\Mvc\Model\Query $query
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model, getSchema){

	zval *query = NULL, models_manager = {};

	phalcon_fetch_params(1, 0, 1, &query);

	if (!query) {
		query = &PHALCON_GLOBAL(z_null);
	}

	if (phalcon_method_exists_ex(getThis(), SL("selectschema")) == SUCCESS) {
		PHALCON_MM_CALL_METHOD(return_value, getThis(), "selectschema", query);
		if (Z_TYPE_P(return_value) != IS_STRING) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "'selectSchema' didn't returned a valid schema");
		}
		RETURN_MM();
	}

	PHALCON_MM_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_MM_ADD_ENTRY(&models_manager);
	PHALCON_MM_CALL_METHOD(return_value, &models_manager, "getmodelschema", getThis());
	RETURN_MM();
}

/**
 * Returns the name of identity field (if one is present)
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model, getIdentityField){

	zval meta_data = {};
	PHALCON_MM_INIT();
	PHALCON_MM_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_MM_ADD_ENTRY(&meta_data);
	PHALCON_MM_CALL_METHOD(return_value, &meta_data, "getidentityfield", getThis());
	RETURN_MM();
}

/**
 * Returns the column map if any
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getColumnMap){

	zval meta_data = {};

	/**
	 * Check if column renaming is globally activated
	 */
	phalcon_read_property(return_value, getThis(), SL("_columnMap"), PH_COPY);

	if (!zend_is_true(return_value)) {
		PHALCON_MM_INIT();
		PHALCON_MM_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
		PHALCON_MM_ADD_ENTRY(&meta_data);
		PHALCON_MM_CALL_METHOD(return_value, &meta_data, "getcolumnmap", getThis());

		phalcon_update_property(getThis(), SL("_columnMap"), return_value);
		RETURN_MM();
	}
}

/**
 * Returns the reverse column map if any
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getReverseColumnMap){

	zval meta_data = {};

	PHALCON_MM_INIT();
	PHALCON_MM_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_MM_ADD_ENTRY(&meta_data);
	PHALCON_MM_CALL_METHOD(return_value, &meta_data, "getreversecolumnmap", getThis());
	RETURN_MM();
}

/**
 * Returns table attributes names (fields)
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getAttributes){

	zval meta_data = {};

	PHALCON_MM_INIT();
	PHALCON_MM_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_MM_ADD_ENTRY(&meta_data);
	PHALCON_MM_CALL_METHOD(return_value, &meta_data, "getattributes", getThis());
	RETURN_MM();
}

/**
 * Returns an array of fields which are part of the primary key
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getPrimaryKeyAttributes){

	zval meta_data = {};

	PHALCON_MM_INIT();
	PHALCON_MM_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_MM_ADD_ENTRY(&meta_data);
	PHALCON_MM_CALL_METHOD(return_value, &meta_data, "getprimarykeyattributes", getThis());
	RETURN_MM();
}

/**
 * Returns an arrau of fields which are not part of the primary key
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getNonPrimaryKeyAttributes){

	zval meta_data = {};

	PHALCON_MM_INIT();
	PHALCON_MM_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_MM_ADD_ENTRY(&meta_data);
	PHALCON_MM_CALL_METHOD(return_value, &meta_data, "getnonprimarykeyattributes", getThis());
	RETURN_MM();
}

/**
 * Returns an array of not null attributes
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getNotNullAttributes){

	zval meta_data = {};

	PHALCON_MM_INIT();
	PHALCON_MM_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_MM_ADD_ENTRY(&meta_data);
	PHALCON_MM_CALL_METHOD(return_value, &meta_data, "getnotnullattributes", getThis());
	RETURN_MM();
}

/**
 * Returns attributes which types are numerical
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getDataTypesNumeric){

	zval meta_data = {};

	PHALCON_MM_INIT();
	PHALCON_MM_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_MM_ADD_ENTRY(&meta_data);
	PHALCON_MM_CALL_METHOD(return_value, &meta_data, "getdatatypesnumeric", getThis());
	RETURN_MM();
}

/**
 * Checks if the attribute is not null
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, isNotNull){

	zval meta_data = {}, *attribute;

	phalcon_fetch_params(1, 1, 0, &attribute);

	PHALCON_MM_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_MM_ADD_ENTRY(&meta_data);
	PHALCON_MM_CALL_METHOD(return_value, &meta_data, "isNotNull", getThis(), attribute);
	RETURN_MM();
}

/**
 * Returns the columns data types
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getDataTypes){

	zval meta_data = {};

	PHALCON_MM_INIT();
	PHALCON_MM_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_MM_ADD_ENTRY(&meta_data);
	PHALCON_MM_CALL_METHOD(return_value, &meta_data, "getdatatypes", getThis());
	RETURN_MM();
}

/**
 * Returns attribute data size
 *
 * @param string $attribute
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getDataSize){

	zval meta_data = {}, *attribute;

	phalcon_fetch_params(1, 1, 0, &attribute);

	PHALCON_MM_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_MM_ADD_ENTRY(&meta_data);
	PHALCON_MM_CALL_METHOD(return_value, &meta_data, "getdatasize", getThis(), attribute);
	RETURN_MM();
}

/**
 * Returns attribute data byte
 *
 * @param string $attribute
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getDataByte){

	zval meta_data = {}, *attribute;

	phalcon_fetch_params(1, 1, 0, &attribute);

	PHALCON_MM_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_MM_ADD_ENTRY(&meta_data);
	PHALCON_MM_CALL_METHOD(return_value, &meta_data, "getdatabyte", getThis(), attribute);
	RETURN_MM();
}

/**
 * Returns attribute data scale
 *
 * @param string $attribute
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getDataScale){

	zval meta_data = {}, *attribute;

	phalcon_fetch_params(1, 1, 0, &attribute);

	PHALCON_MM_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_MM_ADD_ENTRY(&meta_data);
	PHALCON_MM_CALL_METHOD(return_value, &meta_data, "getdatascale", getThis(), attribute);
	RETURN_MM();
}

/**
 * Returns attributes and their bind data types
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getBindTypes){

	zval meta_data = {};

	PHALCON_MM_INIT();
	PHALCON_MM_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_MM_ADD_ENTRY(&meta_data);
	PHALCON_MM_CALL_METHOD(return_value, &meta_data, "getbindtypes", getThis());
	RETURN_MM();
}

/**
 * Returns attributes and their default values
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getDefaultValues){

	zval meta_data = {};

	PHALCON_MM_INIT();
	PHALCON_MM_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_MM_ADD_ENTRY(&meta_data);
	PHALCON_MM_CALL_METHOD(return_value, &meta_data, "getdefaultvalues", getThis());
	RETURN_MM();
}

/**
 * Returns attributes that must be ignored from the INSERT SQL generation
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getAutomaticCreateAttributes){

	zval meta_data = {};

	PHALCON_MM_INIT();
	PHALCON_MM_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_MM_ADD_ENTRY(&meta_data);
	PHALCON_MM_CALL_METHOD(return_value, &meta_data, "getautomaticcreateattributes", getThis());
	RETURN_MM();
}

/**
 * Returns attributes that must be ignored from the UPDATE SQL generation
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getAutomaticUpdateAttributes){

	zval meta_data = {};

	PHALCON_MM_INIT();
	PHALCON_MM_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_MM_ADD_ENTRY(&meta_data);
	PHALCON_MM_CALL_METHOD(return_value, &meta_data, "getautomaticupdateattributes", getThis());
	RETURN_MM();
}

/**
 * Check if a model has certain column
 *
 * @param string $column
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, hasRealAttribute){

	zval *column, meta_data = {};

	phalcon_fetch_params(1, 1, 0, &column);

	PHALCON_MM_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_MM_ADD_ENTRY(&meta_data);
	PHALCON_MM_CALL_METHOD(return_value, &meta_data, "hasrealattribute", getThis(), column);
	RETURN_MM();
}

/**
 * Gets a model certain column
 *
 * @param string $column
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model, getRealAttribute){

	zval *column, meta_data = {};

	phalcon_fetch_params(1, 1, 0, &column);

	PHALCON_MM_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_MM_ADD_ENTRY(&meta_data);
	PHALCON_MM_CALL_METHOD(return_value, &meta_data, "getrealattribute", getThis(), column);
	RETURN_MM();
}

/**
 * Check if a model has certain attribute
 *
 * @param string $attribute
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, hasAttribute){

	zval *attribute, meta_data = {};

	phalcon_fetch_params(1, 1, 0, &attribute);
	
	PHALCON_MM_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_MM_ADD_ENTRY(&meta_data);
	PHALCON_MM_CALL_METHOD(return_value, &meta_data, "hasattribute", getThis(), attribute);
	RETURN_MM();
}

/**
 * Gets a model certain attribute
 *
 * @param string $attribute
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model, getAttribute){

	zval *attribute, meta_data = {};

	phalcon_fetch_params(1, 1, 0, &attribute);

	PHALCON_MM_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_MM_ADD_ENTRY(&meta_data);
	PHALCON_MM_CALL_METHOD(return_value, &meta_data, "getAttribute", getThis(), attribute);
	RETURN_MM();
}

/**
 * Sets a model field value
 *
 * @param string $attribute
 * @param mixed
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, setFieldValue){

	zval *attribute, *value, possible_setter = {};

	phalcon_fetch_params(0, 2, 0, &attribute, &value);

	if (PHALCON_GLOBAL(orm).enable_property_method) {
		PHALCON_CONCAT_SV(&possible_setter, "__set", attribute);

		if (phalcon_method_exists_ce(phalcon_mvc_model_ce, &possible_setter) != SUCCESS) {
			if (phalcon_method_exists(getThis(), &possible_setter) == SUCCESS) {
				PHALCON_CALL_ZVAL_METHOD(NULL, getThis(), &possible_setter, value);
				zval_ptr_dtor(&possible_setter);
				return;
			}
		}
		zval_ptr_dtor(&possible_setter);
	}

	phalcon_update_property_zval_zval(getThis(), attribute, value);

	RETURN_MM_THIS();
}

/**
 * Gets a model field value
 *
 * @param string $attribute
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Model, getFieldValue){

	zval *attribute, possible_getter = {};

	phalcon_fetch_params(0, 1, 0, &attribute);

	if (PHALCON_GLOBAL(orm).enable_property_method) {
		PHALCON_CONCAT_SV(&possible_getter, "__get", attribute);

		if (phalcon_method_exists_ce(phalcon_mvc_model_ce, &possible_getter) != SUCCESS) {
			if (phalcon_method_exists(getThis(), &possible_getter) == SUCCESS) {
				PHALCON_CALL_ZVAL_METHOD(return_value, getThis(), &possible_getter);
				zval_ptr_dtor(&possible_getter);
				return;
			}
		}
		zval_ptr_dtor(&possible_getter);
	}

	if (phalcon_isset_property_zval(getThis(), attribute)) {
		phalcon_read_property_zval(return_value, getThis(), attribute, PH_READONLY);
	}

	RETURN_MM();
}

/**
 * Sets the DependencyInjection connection service name
 *
 * @param string $connectionService
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, setConnectionService){

	zval *connection_service, models_manager = {};

	phalcon_fetch_params(0, 1, 0, &connection_service);

	PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_CALL_METHOD(NULL, &models_manager, "setconnectionservice", getThis(), connection_service);
	zval_ptr_dtor(&models_manager);

	RETURN_THIS();
}

/**
 * Sets the DependencyInjection connection service name used to read data
 *
 * @param string $connectionService
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, setReadConnectionService){

	zval *connection_service, models_manager = {};

	phalcon_fetch_params(0, 1, 0, &connection_service);

	PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_CALL_METHOD(NULL, &models_manager, "setreadconnectionservice", getThis(), connection_service);
	zval_ptr_dtor(&models_manager);

	RETURN_THIS();
}

/**
 * Sets the DependencyInjection connection service name used to write data
 *
 * @param string $connectionService
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, setWriteConnectionService){

	zval *connection_service, models_manager = {};

	phalcon_fetch_params(0, 1, 0, &connection_service);

	PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_CALL_METHOD(NULL, &models_manager, "setwriteconnectionservice", getThis(), connection_service);
	zval_ptr_dtor(&models_manager);

	RETURN_THIS();
}

/**
 * Returns the DependencyInjection connection service name used to read data related the model
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model, getReadConnectionService){

	zval models_manager = {};

	PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_CALL_METHOD(return_value, &models_manager, "getreadconnectionservice", getThis());
	zval_ptr_dtor(&models_manager);
}

/**
 * Returns the DependencyInjection connection service name used to write data related to the model
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model, getWriteConnectionService){

	zval models_manager = {};

	PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_CALL_METHOD(return_value, &models_manager, "getwriteconnectionservice", getThis());
	zval_ptr_dtor(&models_manager);
}

/**
 * Sets the dirty state of the object using one of the DIRTY_STATE_* constants
 *
 * @param int $dirtyState
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, setDirtyState){

	zval *dirty_state;

	phalcon_fetch_params(0, 1, 0, &dirty_state);

	phalcon_update_property(getThis(), SL("_dirtyState"), dirty_state);

	RETURN_THIS();
}

/**
 * Returns one of the DIRTY_STATE_* constants telling if the record exists in the database or not
 *
 * @return int
 */
PHP_METHOD(Phalcon_Mvc_Model, getDirtyState){


	RETURN_MEMBER(getThis(), "_dirtyState");
}

/**
 * Gets the connection used to read data for the model
 *
 * @param Phalcon\Mvc\Model\Query $query
 * @param array $intermediate
 * @param array $bindParams
 * @param array $bindTypes
 * @return Phalcon\Db\AdapterInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, getReadConnection){

	zval *query = NULL, *intermediate = NULL, *bind_params = NULL, *bind_types = NULL, transaction = {}, models_manager = {};

	phalcon_fetch_params(1, 0, 4, &query, &intermediate, &bind_params, &bind_types);

	if (!query) {
		query = &PHALCON_GLOBAL(z_null);
	}

	if (!intermediate) {
		intermediate = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&transaction, getThis(), SL("_transaction"), PH_READONLY);
	if (Z_TYPE(transaction) == IS_OBJECT) {
		if (instanceof_function_ex(Z_OBJCE(transaction), phalcon_db_adapterinterface_ce, 1)) {
			RETURN_MM_CTOR(&transaction);
		}

		PHALCON_MM_CALL_METHOD(return_value, &transaction, "getconnection");
		PHALCON_MM_VERIFY_INTERFACE(return_value, phalcon_db_adapterinterface_ce);
		RETURN_MM();
	}

	if (phalcon_method_exists_ex(getThis(), SL("selectreadconnection")) == SUCCESS) {
		PHALCON_MM_CALL_METHOD(return_value, getThis(), "selectreadconnection", query, intermediate, bind_params, bind_types);
		if (Z_TYPE_P(return_value) != IS_OBJECT) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "'selectReadConnection' didn't returned a valid connection");
		}
		PHALCON_MM_VERIFY_INTERFACE(return_value, phalcon_db_adapterinterface_ce);
		RETURN_MM();
	}

	PHALCON_MM_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_MM_ADD_ENTRY(&models_manager);
	PHALCON_MM_CALL_METHOD(return_value, &models_manager, "getreadconnection", getThis());
	PHALCON_MM_VERIFY_INTERFACE(return_value, phalcon_db_adapterinterface_ce);
	RETURN_MM();
}

/**
 * Gets the connection used to write data to the model
 *
 * @param Phalcon\Mvc\Model\Query $query
 * @param array $intermediate
 * @param array $bindParams
 * @param array $bindTypes
 * @return Phalcon\Db\AdapterInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, getWriteConnection){

	zval *query = NULL, *intermediate = NULL, *bind_params = NULL, *bind_types = NULL, transaction = {}, models_manager = {};

	phalcon_fetch_params(0, 0, 4, &query, &intermediate, &bind_params, &bind_types);

	if (!query) {
		query = &PHALCON_GLOBAL(z_null);
	}

	if (!intermediate) {
		intermediate = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&transaction, getThis(), SL("_transaction"), PH_READONLY);

	if (Z_TYPE(transaction) == IS_OBJECT) {
		if (instanceof_function_ex(Z_OBJCE(transaction), phalcon_db_adapterinterface_ce, 1)) {
			RETURN_CTOR(&transaction);
		}

		PHALCON_CALL_METHOD(return_value, &transaction, "getconnection");
		PHALCON_MM_VERIFY_INTERFACE(return_value, phalcon_db_adapterinterface_ce);
		return;
	}

	if (phalcon_method_exists_ex(getThis(), SL("selectwriteconnection")) == SUCCESS) {
		PHALCON_CALL_METHOD(return_value, getThis(), "selectwriteconnection", query, intermediate, bind_params, bind_types);
		if (Z_TYPE_P(return_value) != IS_OBJECT) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "'selectWriteConnection' didn't returned a valid connection");
		}
		PHALCON_MM_VERIFY_INTERFACE(return_value, phalcon_db_adapterinterface_ce);
		return;
	}

	PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_CALL_METHOD(return_value, &models_manager, "getwriteconnection", getThis());
	zval_ptr_dtor(&models_manager);
	PHALCON_MM_VERIFY_INTERFACE(return_value, phalcon_db_adapterinterface_ce);
}

/**
 * Assigns values to a model from an array
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
 * @param array $columnMap
 * @param array $whiteList
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, assign){

	zval *data, *column_map = NULL, *white_list = NULL, *negate = NULL, *value, exception_message = {};
	zend_string *str_key;

	phalcon_fetch_params(1, 1, 3, &data, &column_map, &white_list, &negate);

	if (!column_map) {
		column_map = &PHALCON_GLOBAL(z_null);
	}

	if (!white_list) {
		white_list = &PHALCON_GLOBAL(z_null);
	}

	if (!negate) {
		negate = &PHALCON_GLOBAL(z_false);
	}

	if (Z_TYPE_P(column_map) == IS_ARRAY) {
		ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(data), str_key, value) {
			zval key = {}, attribute = {}, possible_setter = {};
			if (str_key) {
				ZVAL_STR(&key, str_key);

				if (Z_TYPE_P(white_list) == IS_ARRAY) {
					if (likely(!zend_is_true(negate))) {
						if (!phalcon_fast_in_array(&key, white_list)) {
							continue;
						}
					} else {
						if (phalcon_fast_in_array(&key, white_list)) {
							continue;
						}
					}
				}

				/**
				 * Every field must be part of the column map
				 */
				if (!phalcon_array_isset_fetch(&attribute, column_map, &key, PH_READONLY)) {
					if (phalcon_fast_in_array(&key, column_map)) {
						ZVAL_COPY_VALUE(&attribute, &key);
					} else {
						if (PHALCON_GLOBAL(orm).enable_strict) {
							PHALCON_CONCAT_SVS(&exception_message, "Column \"", &key, "\" doesn't make part of the column map");
							PHALCON_MM_ADD_ENTRY(&exception_message);
							PHALCON_MM_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
							return;
						} else {
							continue;
						}
					}
				}

				/**
				 * If the white-list is an array check if the attribute is on that list
				 */
				if (likely(PHALCON_GLOBAL(orm).enable_property_method)) {
					PHALCON_CONCAT_SV(&possible_setter, "__set", &attribute);
					PHALCON_MM_ADD_ENTRY(&possible_setter);
					zend_str_tolower(Z_STRVAL(possible_setter), Z_STRLEN(possible_setter));
					if (phalcon_method_exists(getThis(), &possible_setter) == SUCCESS) {
						PHALCON_MM_CALL_ZVAL_METHOD(NULL, getThis(), &possible_setter, value);
					} else {
						phalcon_update_property_zval_zval(getThis(), &attribute, value);
					}
				} else {
					phalcon_update_property_zval_zval(getThis(), &attribute, value);
				}
			}
		} ZEND_HASH_FOREACH_END();
	} else {
		ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(data), str_key, value) {
			zval key = {}, possible_setter = {};
			if (str_key) {
				ZVAL_STR(&key, str_key);
				/**
				 * If the white-list is an array check if the attribute is on that list
				 */
				if (Z_TYPE_P(white_list) == IS_ARRAY) {
					if (likely(!zend_is_true(negate))) {
						if (!phalcon_fast_in_array(&key, white_list)) {
							continue;
						}
					} else {
						if (phalcon_fast_in_array(&key, white_list)) {
							continue;
						}
					}
				}
				if (likely(PHALCON_GLOBAL(orm).enable_property_method)) {
					PHALCON_CONCAT_SV(&possible_setter, "__set", &key);
					PHALCON_MM_ADD_ENTRY(&possible_setter);
					zend_str_tolower(Z_STRVAL(possible_setter), Z_STRLEN(possible_setter));
					if (phalcon_method_exists(getThis(), &possible_setter) == SUCCESS) {
						PHALCON_MM_CALL_ZVAL_METHOD(NULL, getThis(), &possible_setter, value);
					} else {
						phalcon_update_property_zval_zval(getThis(), &key, value);
					}
				} else {
					phalcon_update_property_zval_zval(getThis(), &key, value);
				}
			}
		} ZEND_HASH_FOREACH_END();
	}
	RETURN_MM_THIS();
}

/**
 * Assigns values to a model from an array returning a new model.
 *
 *<code>
 *$robot = \Phalcon\Mvc\Model::cloneResultMap(new Robots(), array(
 *  'type' => 'mechanical',
 *  'name' => 'Astro Boy',
 *  'year' => 1952
 *));
 *</code>
 *
 * @param Phalcon\Mvc\Model $base
 * @param array $data
 * @param array $columnMap
 * @param int $dirtyState
 * @param boolean $keepSnapshots
 * @param Phalcon\Mvc\Model $sourceModel
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, cloneResultMap){

	zval *base, *data, *column_map, *dirty_state = NULL, *source_model = NULL;
	zval data_types = {}, connection = {}, *value, exception_message = {};
	zval snapshot_data = {};
	zend_string *str_key;

	phalcon_fetch_params(1, 3, 3, &base, &data, &column_map, &dirty_state, &source_model);

	if (!dirty_state) {
		dirty_state = &PHALCON_GLOBAL(z_zero);
	}

	if (Z_TYPE_P(base) != IS_OBJECT) {
		PHALCON_MM_ZVAL_STRING(&exception_message, "The base must be object");
		PHALCON_MM_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
		return;
	}

	if (phalcon_clone(return_value, base) == FAILURE) {
		RETURN_MM();
	}

	if (source_model && Z_TYPE_P(source_model) == IS_OBJECT) {
		PHALCON_MM_CALL_METHOD(&data_types, source_model, "getdatatypes");
		PHALCON_MM_ADD_ENTRY(&data_types);
		PHALCON_MM_CALL_METHOD(&connection, source_model, "getreadconnection");
		PHALCON_MM_ADD_ENTRY(&connection);
	}

	/**
	 * Change the dirty state to persistent
	 */
	PHALCON_MM_CALL_METHOD(NULL, return_value, "setdirtystate", dirty_state);

	array_init(&snapshot_data);
	PHALCON_MM_ADD_ENTRY(&snapshot_data);

	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(data), str_key, value) {
		zval key = {}, field_type = {}, convert_value = {}, attribute = {};

		if (str_key) {
			ZVAL_STR(&key, str_key);
			if (PHALCON_GLOBAL(orm).enable_auto_convert && zend_is_true(&data_types)) {
				if (phalcon_array_isset_fetch(&field_type, &data_types, &key, PH_READONLY) && Z_TYPE(field_type) == IS_LONG) {
					switch(Z_LVAL(field_type)) {
						case PHALCON_DB_COLUMN_TYPE_JSON:
							RETURN_MM_ON_FAILURE(phalcon_json_decode(&convert_value, value, 1));
							break;
						case PHALCON_DB_COLUMN_TYPE_BYTEA:
							PHALCON_MM_CALL_METHOD(&convert_value, &connection, "unescapebytea", value);
							break;
						case PHALCON_DB_COLUMN_TYPE_ARRAY:
						case PHALCON_DB_COLUMN_TYPE_INT_ARRAY:
							PHALCON_MM_CALL_METHOD(&convert_value, &connection, "unescapearray", value, &field_type);
							break;
						default:
							ZVAL_COPY(&convert_value, value);
					}
				} else {
					ZVAL_COPY(&convert_value, value);
				}
			} else {
				ZVAL_COPY(&convert_value, value);
			}
			PHALCON_MM_ADD_ENTRY(&convert_value);

			/**
			 * Only string keys in the data are valid
			 */
			if (Z_TYPE_P(column_map) == IS_ARRAY) {
				/**
				 * Every field must be part of the column map
				 */
				if (phalcon_array_isset_fetch(&attribute, column_map, &key, PH_NOISY|PH_READONLY)) {
					phalcon_update_property_zval_zval(return_value, &attribute, &convert_value);
				} else {
					PHALCON_CONCAT_SVS(&exception_message, "Column \"", &key, "\" doesn't make part of the column map");
					PHALCON_MM_ADD_ENTRY(&exception_message);
					PHALCON_MM_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}
			} else {
				phalcon_update_property_zval_zval(return_value, &key, &convert_value);
			}
			phalcon_array_update(&snapshot_data, &key, &convert_value, PH_COPY);
		}
	} ZEND_HASH_FOREACH_END();

	if (instanceof_function(Z_OBJCE_P(return_value), phalcon_mvc_model_ce)) {
		PHALCON_MM_CALL_METHOD(NULL, return_value, "setsnapshotdata", &snapshot_data, column_map);
		PHALCON_MM_CALL_METHOD(NULL, return_value, "build");
	}

	/**
	 * Call afterFetch, this allows the developer to execute actions after a record is
	 * fetched from the database
	 */
	if (phalcon_method_exists_ex(return_value, SL("afterfetch")) == SUCCESS) {
		PHALCON_MM_CALL_METHOD(NULL, return_value, "afterfetch");
	}
	RETURN_MM();
}

/**
 * Returns an hydrated result based on the data and the column map
 *
 * @param array $data
 * @param array $columnMap
 * @param int $hydrationMode
 * @param Phalcon\Mvc\Model $sourceModel
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Model, cloneResultMapHydrate){

	zval *data, *column_map, *hydration_mode, *source_model = NULL;
	zval data_types = {}, connection = {}, *value, exception_message = {};
	zend_string *str_key;

	phalcon_fetch_params(1, 3, 1, &data, &column_map, &hydration_mode, &source_model);

	/**
	 * If there is no column map and the hydration mode is arrays return the data as it
	 * is
	 */
	if (Z_TYPE_P(column_map) != IS_ARRAY) {
		if (PHALCON_IS_LONG(hydration_mode, 1)) {
			RETURN_MM_CTOR(data);
		}
	}

	/**
	 * Create the destination object according to the hydration mode
	 */
	if (PHALCON_IS_LONG(hydration_mode, 1)) {
		array_init(return_value);
	} else {
		object_init(return_value);
	}

	if (source_model && Z_TYPE_P(source_model) == IS_OBJECT) {
		PHALCON_MM_CALL_METHOD(&data_types, source_model, "getdatatypes");
		PHALCON_MM_ADD_ENTRY(&data_types);
		PHALCON_MM_CALL_METHOD(&connection, source_model, "getreadconnection");
		PHALCON_MM_ADD_ENTRY(&connection);
	}

	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(data), str_key, value) {
		zval key = {}, field_type = {}, convert_value = {}, attribute = {};
		if (str_key) {
			ZVAL_STR(&key, str_key);

			if (PHALCON_GLOBAL(orm).enable_auto_convert && Z_TYPE(data_types) == IS_ARRAY) {
				if (phalcon_array_isset_fetch(&field_type, &data_types, &key, PH_READONLY) && Z_TYPE(field_type) == IS_LONG) {
					switch(Z_LVAL(field_type)) {
						case PHALCON_DB_COLUMN_TYPE_JSON:
							RETURN_MM_ON_FAILURE(phalcon_json_decode(&convert_value, value, 1));
							break;
						case PHALCON_DB_COLUMN_TYPE_BYTEA:
							PHALCON_MM_CALL_METHOD(&convert_value, &connection, "unescapebytea", value);
							break;
						case PHALCON_DB_COLUMN_TYPE_ARRAY:
						case PHALCON_DB_COLUMN_TYPE_INT_ARRAY:
							PHALCON_MM_CALL_METHOD(&convert_value, &connection, "unescapearray", value, &field_type);
							break;
						default:
							ZVAL_COPY(&convert_value, value);
					}
				} else {
					ZVAL_COPY(&convert_value, value);
				}
			} else {
				ZVAL_COPY(&convert_value, value);
			}
			PHALCON_MM_ADD_ENTRY(&convert_value);

			if (Z_TYPE_P(column_map) == IS_ARRAY) {
				/**
				 * Every field must be part of the column map
				 */
				if (!phalcon_array_isset_fetch(&attribute, column_map, &key, PH_READONLY)) {
					PHALCON_CONCAT_SVS(&exception_message, "Column \"", &key, "\" doesn't make part of the column map");
					PHALCON_MM_ADD_ENTRY(&exception_message);
					PHALCON_MM_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}

				if (PHALCON_IS_LONG(hydration_mode, 1)) {
					phalcon_array_update(return_value, &attribute, &convert_value, PH_COPY);
				} else {
					phalcon_update_property_zval_zval(return_value, &attribute, &convert_value);
				}
			} else {
				if (PHALCON_IS_LONG(hydration_mode, 1)) {
					phalcon_array_update(return_value, &key, &convert_value, PH_COPY);
				} else {
					phalcon_update_property_zval_zval(return_value, &key, &convert_value);
				}
			}
		}
	} ZEND_HASH_FOREACH_END();
 
	RETURN_MM();
}

/**
 * Assigns values to a model from an array returning a new model
 *
 *<code>
 *$robot = Phalcon\Mvc\Model::cloneResult(new Robots(), array(
 *  'type' => 'mechanical',
 *  'name' => 'Astro Boy',
 *  'year' => 1952
 *));
 *</code>
 *
 * @param Phalcon\Mvc\Model $base
 * @param array $data
 * @param int $dirtyState
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, cloneResult){

	zval *base, *data, *dirty_state = NULL, *value = NULL, exception_message = {};
	zend_string *str_key;

	phalcon_fetch_params(0, 2, 1, &base, &data, &dirty_state);

	if (!dirty_state) {
		dirty_state = &PHALCON_GLOBAL(z_zero);
	}

	if (Z_TYPE_P(base) != IS_OBJECT) {
		ZVAL_STRING(&exception_message, "The base must be object");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
		zval_ptr_dtor(&exception_message);
		return;
	}

	/**
	 * Clone the base record
	 */
	if (phalcon_clone(return_value, base) == FAILURE) {
		return;
	}

	/**
	 * Mark the object as persistent
	 */
	PHALCON_CALL_METHOD(NULL, return_value, "setdirtystate", dirty_state);

	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(data), str_key, value) {
		if (!str_key) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Invalid key in array data provided to dumpResult()");
			return;
		}
		phalcon_update_property_string_zval(return_value, str_key, value);
	} ZEND_HASH_FOREACH_END();

	/**
	 * Call afterFetch, this allows the developer to execute actions after a record is
	 * fetched from the database
	 */
	if (phalcon_method_exists_ex(return_value, SL("afterfetch")) == SUCCESS) {
		PHALCON_CALL_METHOD(NULL, return_value, "afterfetch");
	}
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
 * $robots = Robots::find("type='mechanical'");
 * echo "There are ", count($robots), "\n";
 *
 * //Get and print virtual robots ordered by name
 * $robots = Robots::find(array("type='virtual'", "order" => "name"));
 * foreach ($robots as $robot) {
 *	   echo $robot->name, "\n";
 * }
 *
 * $robots = Robots::find("type=:type:", array("type" => "virtual"), array("order" => "name"));
 * foreach ($robots as $robot) {
 *	   echo $robot->name, "\n";
 * }
 *
 * //Get first 100 virtual robots ordered by name
 * $robots = Robots::find(array("type='virtual'", "order" => "name", "limit" => 100));
 * foreach ($robots as $robot) {
 *	   echo $robot->name, "\n";
 * }
 * </code>
 *
 * @param string|array $conditions
 * @param array $bindParams
 * @param array $options
 * @return  Phalcon\Mvc\Model\ResultsetInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, find){

	zval *parameters = NULL, *bind_params = NULL, *options = NULL, params = {},dependency_injector = {}, model_name = {}, service_name = {};
	zval  manager = {}, model = {}, builder = {}, event_name = {}, query = {}, hydration = {};

	phalcon_fetch_params(1, 0, 3, &parameters, &bind_params, &options);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	if (bind_params) {
		if (options && Z_TYPE_P(options) == IS_ARRAY) {
			ZVAL_DUP(&params, options);
		} else {
			array_init(&params);
		}
		if (Z_TYPE_P(parameters) != IS_NULL) {
			phalcon_array_update_str(&params, SL("conditions"), parameters, PH_COPY);
		}
		if (Z_TYPE_P(bind_params) == IS_ARRAY) {
			phalcon_array_update_str(&params, SL("bind"), bind_params, PH_COPY);
		}
		PHALCON_MM_ADD_ENTRY(&params);
	} else {
		if (Z_TYPE_P(parameters) != IS_NULL && Z_TYPE_P(parameters) != IS_ARRAY) {
			array_init(&params);
			phalcon_array_append(&params, parameters, PH_COPY);
		} else {
			ZVAL_DUP(&params, parameters);
		}
		PHALCON_MM_ADD_ENTRY(&params);
	}

	PHALCON_MM_CALL_CE_STATIC(&dependency_injector, phalcon_di_ce, "getdefault");
	PHALCON_MM_ADD_ENTRY(&dependency_injector);

	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
		return;
	}

	phalcon_get_called_class(&model_name);
	PHALCON_MM_ADD_ENTRY(&model_name);

	ZVAL_STR(&service_name, IS(modelsManager));

	PHALCON_MM_CALL_METHOD(&manager, &dependency_injector, "getshared", &service_name);
	PHALCON_MM_ADD_ENTRY(&manager);

	PHALCON_MM_CALL_METHOD(&model, &manager, "load", &model_name);
	PHALCON_MM_ADD_ENTRY(&model);

	PHALCON_MM_CALL_METHOD(&builder, &manager, "createbuilder", &params);
	PHALCON_MM_ADD_ENTRY(&builder);

	PHALCON_MM_CALL_METHOD(NULL, &builder, "from", &model_name);

	PHALCON_MM_ZVAL_STRING(&event_name, "beforeQuery");
	PHALCON_MM_CALL_METHOD(return_value, &model, "fireevent", &event_name, &builder);
	if (Z_TYPE_P(return_value) >= IS_STRING) {
		RETURN_MM();
	}

	PHALCON_MM_CALL_METHOD(&query, &builder, "getquery");
	PHALCON_MM_ADD_ENTRY(&query);

	/**
	 * Execute the query passing the bind-params and casting-types
	 */
	PHALCON_MM_CALL_METHOD(return_value, &query, "execute");

	/**
	 * Define an hydration mode
	 */
	if (Z_TYPE_P(return_value) == IS_OBJECT) {
		zval resultset = {};
		if (phalcon_array_isset_fetch_str(&hydration, &params, SL("hydration"), PH_READONLY)) {
			PHALCON_MM_CALL_METHOD(NULL, return_value, "sethydratemode", &hydration);
		}

		PHALCON_MM_ZVAL_STRING(&event_name, "afterQuery");
		PHALCON_MM_CALL_METHOD(&resultset, &model, "fireevent", &event_name, return_value);
		PHALCON_MM_ADD_ENTRY(&resultset);
		if (Z_TYPE(resultset) >= IS_ARRAY) {
			zval_ptr_dtor(return_value);
			RETURN_MM_NCTOR(&resultset);
		}
	}
	RETURN_MM();
}

/**
 * Allows to query the first record that match the specified conditions
 *
 * <code>
 *
 * //What's the first robot in robots table?
 * $robot = Robots::findFirst();
 * echo "The robot name is ", $robot->name;
 *
 * //What's the first mechanical robot in robots table?
 * $robot = Robots::findFirst("type='mechanical'");
 * echo "The first mechanical robot name is ", $robot->name;
 *
 * //Get first virtual robot ordered by name
 * $robot = Robots::findFirst(array("type='virtual'", "order" => "name"));
 * echo "The first virtual robot name is ", $robot->name;
 *
 * </code>
 *
 * @param string|array $conditions
 * @param array $bindParams
 * @param array $options
 * @param bool $autoCreate
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, findFirst){

	zval *parameters = NULL, *bind_params = NULL, *options = NULL,  *auto_create = NULL, dependency_injector = {}, model_name = {}, service_name = {}, has = {};
	zval manager = {}, model = {}, identityfield = {}, id_condition = {}, params = {}, builder = {}, query = {}, event_name = {}, hydration = {}, resultset = {};

	phalcon_fetch_params(1, 0, 4, &parameters, &bind_params, &options, &auto_create);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	if (!auto_create) {
		auto_create = &PHALCON_GLOBAL(z_false);
	}

	PHALCON_MM_CALL_CE_STATIC(&dependency_injector, phalcon_di_ce, "getdefault");
	PHALCON_MM_ADD_ENTRY(&dependency_injector);
	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
		return;
	}

	phalcon_get_called_class(&model_name);
	PHALCON_MM_ADD_ENTRY(&model_name);

	ZVAL_STR(&service_name, IS(modelsManager));

	PHALCON_CALL_METHOD(&has, &dependency_injector, "has", &service_name);
	if (zend_is_true(&has)) {
		PHALCON_MM_CALL_METHOD(&manager, &dependency_injector, "getshared", &service_name);
	} else {
		object_init_ex(&manager, phalcon_mvc_model_manager_ce);
	}
	PHALCON_MM_ADD_ENTRY(&manager);

	PHALCON_MM_CALL_METHOD(&model, &manager, "load", &model_name, &PHALCON_GLOBAL(z_true));
	PHALCON_MM_ADD_ENTRY(&model);

	if (bind_params) {
		if (options && Z_TYPE_P(options) == IS_ARRAY) {
			ZVAL_DUP(&params, options);
		} else {
			array_init(&params);
		}
		
		if (phalcon_is_numeric(parameters)) {
			zval condition = {};
			PHALCON_MM_CALL_METHOD(&identityfield, &model, "getidentityfield");
			PHALCON_MM_CALL_METHOD(&id_condition, &model, "getattribute", &identityfield);
			zval_ptr_dtor(&identityfield);

			PHALCON_SCONCAT_VSVS(&condition, &id_condition, " = '", parameters, "'");
			zval_ptr_dtor(&id_condition);
			phalcon_array_update_str(&params, SL("conditions"), &condition, 0);
		} else if (Z_TYPE_P(parameters) != IS_NULL) {
			phalcon_array_update_str(&params, SL("conditions"), parameters, PH_COPY);
		}
		if (Z_TYPE_P(bind_params) == IS_ARRAY) {
			phalcon_array_update_str(&params, SL("bind"), bind_params, PH_COPY);
		}
		PHALCON_MM_ADD_ENTRY(&params);
	} else {
		if (Z_TYPE_P(parameters) != IS_NULL && Z_TYPE_P(parameters) != IS_ARRAY) {
			array_init(&params);
			if (phalcon_is_numeric(parameters)) {
				zval condition = {};
				PHALCON_MM_CALL_METHOD(&identityfield, &model, "getidentityfield");
				PHALCON_MM_CALL_METHOD(&id_condition, &model, "getattribute", &identityfield);
				zval_ptr_dtor(&identityfield);

				PHALCON_SCONCAT_VSVS(&condition, &id_condition, " = '", parameters, "'");
				zval_ptr_dtor(&id_condition);
				phalcon_array_update_str(&params, SL("conditions"), &condition, 0);
			} else {
				phalcon_array_append(&params, parameters, PH_COPY);
			}
		} else {
			ZVAL_DUP(&params, parameters);
		}
		PHALCON_MM_ADD_ENTRY(&params);
	}

	if (!phalcon_array_isset_str(&params, SL("limit"))) {
		phalcon_array_unset_str(&params, SL("limit"), 0);
	}

	PHALCON_MM_CALL_METHOD(&builder, &manager, "createbuilder", &params);
	PHALCON_MM_ADD_ENTRY(&builder);

	PHALCON_MM_CALL_METHOD(NULL, &builder, "from", &model_name);
	PHALCON_MM_CALL_METHOD(NULL, &builder, "limit", &PHALCON_GLOBAL(z_one));

	PHALCON_MM_ZVAL_STRING(&event_name, "beforeQuery");
	PHALCON_MM_CALL_METHOD(return_value, &model, "fireevent", &event_name, &builder);
	if (Z_TYPE_P(return_value) >= IS_STRING) {
		RETURN_MM();
	}

	PHALCON_MM_CALL_METHOD(&query, &builder, "getquery");
	PHALCON_MM_ADD_ENTRY(&query);

	/**
	 * Return only the first row
	 */
	PHALCON_MM_CALL_METHOD(NULL, &query, "setuniquerow", &PHALCON_GLOBAL(z_true));

	/**
	 * Execute the query passing the bind-params and casting-types
	 */
	PHALCON_MM_CALL_METHOD(&resultset, &query, "execute");
	PHALCON_MM_ADD_ENTRY(&resultset);
	if (!zend_is_true(&resultset)) {
		if (zend_is_true(auto_create)) {
			PHALCON_MM_ZVAL_COPY(&resultset, &model);
		}
	} else {
		/**
		 * Define an hydration mode
		 */
		if (phalcon_array_isset_fetch_str(&hydration, &params, SL("hydration"), PH_READONLY)) {
			PHALCON_MM_CALL_METHOD(NULL, &resultset, "sethydratemode", &hydration);
		}	
	}
	zval_ptr_dtor(return_value);
	PHALCON_MM_ZVAL_STRING(&event_name, "afterQuery");
	PHALCON_MM_CALL_METHOD(return_value, &model, "fireevent", &event_name, &resultset);
	if (Z_TYPE_P(return_value) >= IS_ARRAY) {
		RETURN_MM();
	} else {
		zval_ptr_dtor(return_value);
		RETURN_MM_CTOR(&resultset);
	}
}

/**
 * Create a criteria for a specific model
 *
 * @param Phalcon\DiInterface $dependencyInjector
 * @return Phalcon\Mvc\Model\Criteria
 */
PHP_METHOD(Phalcon_Mvc_Model, query){

	zval *di = NULL, dependency_injector = {}, model_name = {}, service_name = {}, has = {};

	phalcon_fetch_params(1, 0, 1, &di);

	if (!di || Z_TYPE_P(di) != IS_OBJECT) {
		/**
		 * Use the global dependency injector if there is no one defined
		 */
		PHALCON_MM_CALL_CE_STATIC(&dependency_injector, phalcon_di_ce, "getdefault");
		PHALCON_MM_ADD_ENTRY(&dependency_injector);
		if (Z_TYPE(dependency_injector) != IS_OBJECT) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
			return;
		}
	} else {
		ZVAL_COPY_VALUE(&dependency_injector, di);
	}

	phalcon_get_called_class(&model_name);
	PHALCON_MM_ADD_ENTRY(&model_name);
	ZVAL_STR(&service_name, IS(modelsCriteria));

	PHALCON_MM_CALL_METHOD(&has, &dependency_injector, "has", &service_name);
	if (zend_is_true(&has)) {
		PHALCON_MM_CALL_METHOD(return_value, &dependency_injector, "get", &service_name);
	} else {
		object_init_ex(return_value, phalcon_mvc_model_criteria_ce);
	}

	PHALCON_MM_CALL_METHOD(NULL, return_value, "setdi", &dependency_injector);
	PHALCON_MM_CALL_METHOD(NULL, return_value, "setmodelname", &model_name);
	RETURN_MM();
}

/**
 * Builds a unique primary key condition
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, build){

	PHALCON_CALL_METHOD(return_value, getThis(), "_rebuild");
}

/**
 * Gets a unique key
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model, getUniqueKey){

	RETURN_MEMBER(getThis(), "_uniqueKey");
}

/**
 * Gets a unique params
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getUniqueParams){

	RETURN_MEMBER(getThis(), "_uniqueParams");
}

/**
 * Gets a unique params
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getUniqueTypes){

	RETURN_MEMBER(getThis(), "_uniqueTypes");
}

/**
 * Builds a unique primary key condition
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _reBuild){

	zval unique_params = {}, primary_keys = {}, bind_data_types = {}, number_primary = {}, column_map = {};
	zval snapshot = {}, unique_types = {}, *field = NULL, where_pk = {}, exception_message = {}, join_where = {};
	int not_empty_num = 0;

	/**
	 * Builds a unique primary key condition
	 */
	PHALCON_CALL_METHOD(&primary_keys, getThis(), "getprimarykeyattributes");
	PHALCON_CALL_METHOD(&bind_data_types, getThis(), "getbindtypes");

	phalcon_fast_count(&number_primary, &primary_keys);
	if (!zend_is_true(&number_primary)) {
		ZVAL_STRING(&exception_message, "No primary key defined");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
		return;
	}

	PHALCON_CALL_SELF(&column_map, "getcolumnmap");

	phalcon_read_property(&snapshot, getThis(), SL("_snapshot"), PH_NOISY|PH_READONLY);

	array_init(&where_pk);
	array_init(&unique_params);
	array_init(&unique_types);

	/**
	 * We need to create a primary key based on the current data
	 */
	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(primary_keys), field) {
		zval key = {}, attribute_field = {}, value = {}, pk_condition = {}, type = {};
		if (Z_TYPE(column_map) == IS_ARRAY) {
			if (!phalcon_array_isset_fetch(&attribute_field, &column_map, field, PH_READONLY)) {
				PHALCON_CONCAT_SVS(&exception_message, "Column '", field, "' isn't part of the column map");
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
				return;
			}
		} else {
			ZVAL_COPY_VALUE(&attribute_field, field);
		}

		if (unlikely(!PHALCON_GLOBAL(orm).allow_update_primary)
			|| Z_TYPE(snapshot) != IS_ARRAY
			|| !phalcon_array_isset_fetch(&value, &snapshot, &attribute_field, PH_READONLY)
		) {
			/**
			 * If the primary key attribute is set append it to the conditions
			 */
			if (phalcon_isset_property_zval(getThis(), &attribute_field)) {
				phalcon_read_property_zval(&value, getThis(), &attribute_field, PH_NOISY|PH_READONLY);
			}
		}


		if (Z_TYPE(value) <= IS_NULL) {
			PHALCON_CONCAT_VS(&pk_condition, &attribute_field, " IS NULL");
		} else {
			not_empty_num += 1;
			PHALCON_CONCAT_SV(&key, "pha_", &attribute_field);
			PHALCON_CONCAT_VSVS(&pk_condition, &attribute_field, "= :", &key, ":");
			phalcon_array_update(&unique_params, &key, &value, PH_COPY);

			if (phalcon_array_isset_fetch(&type, &bind_data_types, field, PH_READONLY)) {
				phalcon_array_update(&unique_types, &key, &type, PH_COPY);
			}
			zval_ptr_dtor(&key);
		}

		phalcon_array_append(&where_pk, &pk_condition, 0);
	} ZEND_HASH_FOREACH_END();
	zval_ptr_dtor(&column_map);
	zval_ptr_dtor(&primary_keys);
	zval_ptr_dtor(&bind_data_types);

	if (not_empty_num <= 0) {
		phalcon_update_property_null(getThis(), SL("_uniqueKey"));
		phalcon_update_property_null(getThis(), SL("_uniqueParams"));
		phalcon_update_property_null(getThis(), SL("_uniqueTypes"));

		RETVAL_FALSE;
	} else {
		phalcon_fast_join_str(&join_where, SL(" AND "), &where_pk);

		/**
		 * The unique key is composed of 3 parts _uniqueKey, uniqueParams, uniqueTypes
		 */
		phalcon_update_property(getThis(), SL("_uniqueKey"), &join_where);
		phalcon_update_property(getThis(), SL("_uniqueParams"), &unique_params);
		phalcon_update_property(getThis(), SL("_uniqueTypes"), &unique_types);

		RETVAL_TRUE;
	}
	zval_ptr_dtor(&where_pk);
	zval_ptr_dtor(&join_where);
	zval_ptr_dtor(&unique_params);
	zval_ptr_dtor(&unique_types);
}

/**
 * Checks if the current record already exists or not
 *
 * @param Phalcon\Mvc\Model\MetadataInterface $metaData
 * @param Phalcon\Db\AdapterInterface $connection
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, exists){

	zval *force = NULL, *conection = NULL, build = {}, dirty_state = {}, unique_key = {}, unique_params = {}, unique_types = {};
	zval model_name = {}, phql = {}, models_manager = {}, query = {}, model = {}, snapshot = {};

	phalcon_fetch_params(1, 0, 2, &force, &conection);

	if (!force) {
		force = &PHALCON_GLOBAL(z_false);
	}

	if (!conection) {
		conection = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_MM_CALL_METHOD(&build, getThis(), "_rebuild");
	PHALCON_MM_ADD_ENTRY(&build);

	if (!zend_is_true(&build)) {
		RETURN_MM_FALSE;
	}

	PHALCON_MM_CALL_METHOD(&dirty_state, getThis(), "getdirtystate");

	if (!zend_is_true(force) && PHALCON_IS_LONG(&dirty_state, PHALCON_MODEL_DIRTY_STATE_PERSISTEN)) {
		RETURN_MM_TRUE;
	}

	PHALCON_MM_CALL_METHOD(&unique_key, getThis(), "getuniquekey");
	PHALCON_MM_ADD_ENTRY(&unique_key);
	PHALCON_MM_CALL_METHOD(&unique_params, getThis(), "getuniqueparams");
	PHALCON_MM_ADD_ENTRY(&unique_params);
	PHALCON_MM_CALL_METHOD(&unique_types, getThis(), "getuniquetypes");
	PHALCON_MM_ADD_ENTRY(&unique_types);

	phalcon_get_called_class(&model_name);
	PHALCON_MM_ADD_ENTRY(&model_name);

	/**
	 * Here we use a single COUNT(*) without PHQL to make the execution faster
	 */
	PHALCON_CONCAT_SVSVS(&phql, "SELECT * FROM ", &model_name, " WHERE ", &unique_key, " LIMIT 1");
	PHALCON_MM_ADD_ENTRY(&phql);

	PHALCON_MM_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_MM_ADD_ENTRY(&models_manager);
	PHALCON_MM_CALL_METHOD(&query, &models_manager, "createquery", &phql);
	PHALCON_MM_ADD_ENTRY(&query);

	PHALCON_MM_CALL_METHOD(NULL, &query, "setconnection", conection);
	PHALCON_MM_CALL_METHOD(NULL, &query, "setuniquerow", &PHALCON_GLOBAL(z_true));
	PHALCON_MM_CALL_METHOD(NULL, &query, "setbindparams", &unique_params);
	PHALCON_MM_CALL_METHOD(NULL, &query, "setbindtypes", &unique_types);

	PHALCON_MM_CALL_METHOD(&model, &query, "execute");
	PHALCON_MM_ADD_ENTRY(&model);

	if (Z_TYPE(model) == IS_OBJECT) {
		zval changed = {}, *value = NULL;
		zend_string *str_key = NULL;
		phalcon_update_property_long(getThis(), SL("_dirtyState"), PHALCON_MODEL_DIRTY_STATE_PERSISTEN);
		PHALCON_MM_CALL_METHOD(&snapshot, &model, "getsnapshotdata");
		PHALCON_MM_ADD_ENTRY(&snapshot);

		if (!zend_is_true(force)) {
			PHALCON_MM_CALL_METHOD(&changed, getThis(), "getchangedfields");
			PHALCON_MM_ADD_ENTRY(&changed);
			if (phalcon_fast_count_ev(&changed)) {
				ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL(snapshot), str_key, value) {
					zval key = {};
					if (!str_key) {
						continue;
					}
					ZVAL_STR(&key, str_key);
					if (phalcon_fast_in_array(&key, &changed)) {
						continue;
					}

					phalcon_update_property_zval_zval(getThis(), &key, value);
				} ZEND_HASH_FOREACH_END();
			}
		} else if (zend_is_true(force)) {
			ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL(snapshot), str_key, value) {
				zval key = {};
				if (!str_key) {
					continue;
				}
				ZVAL_STR(&key, str_key);

				phalcon_update_property_zval_zval(getThis(), &key, value);
			} ZEND_HASH_FOREACH_END();
		
		}

		PHALCON_MM_CALL_METHOD(NULL, getThis(), "setsnapshotdata", &snapshot);
		RETVAL_TRUE;
	} else {
		phalcon_update_property_long(getThis(), SL("_dirtyState"), PHALCON_MODEL_DIRTY_STATE_TRANSIENT);
		RETVAL_FALSE;
	}
	RETURN_MM();
}

/**
 * Generate a PHQL SELECT statement for an aggregate
 *
 * @param string $function
 * @param string $alias
 * @param array $parameters
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Model, _groupResult){

	zval *function, *alias, *parameters, params = {}, group_column = {}, distinct_column = {}, columns = {}, group_columns = {};
	zval event_name = {}, model_name = {}, dependency_injector = {}, service_name = {}, manager = {}, model = {}, builder = {}, query = {};
	zval resultset = {}, first_row = {};

	phalcon_fetch_params(1, 3, 0, &function, &alias, &parameters);

	if (Z_TYPE_P(parameters) != IS_ARRAY && Z_TYPE_P(parameters) != IS_NULL) {
		array_init_size(&params, 1);
		phalcon_array_append(&params, parameters, PH_COPY);
	} else {
		if (Z_TYPE_P(parameters) == IS_ARRAY) {
			ZVAL_DUP(&params, parameters);
		} else {
			array_init(&params);
		}
	}
	PHALCON_MM_ADD_ENTRY(&params);

	phalcon_get_called_class(&model_name);
	PHALCON_MM_ADD_ENTRY(&model_name);

	PHALCON_MM_CALL_CE_STATIC(&dependency_injector, phalcon_di_ce, "getdefault");
	PHALCON_MM_ADD_ENTRY(&dependency_injector);

	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
		return;
	}

	ZVAL_STR(&service_name, IS(modelsManager));

	PHALCON_MM_CALL_METHOD(&manager, &dependency_injector, "getshared", &service_name);
	PHALCON_MM_ADD_ENTRY(&manager);
	PHALCON_MM_CALL_METHOD(&model, &manager, "load", &model_name);
	PHALCON_MM_ADD_ENTRY(&model);

	if (!phalcon_array_isset_fetch_str(&group_column, &params, SL("column"), PH_READONLY)) {
		PHALCON_MM_ZVAL_STRING(&group_column, "*");
	}

	/**
	 * Builds the columns to query according to the received parameters
	 */
	if (phalcon_array_isset_fetch_str(&distinct_column, &params, SL("distinct"), PH_READONLY)) {
		PHALCON_CONCAT_VSVSV(&columns, function, "(DISTINCT ", &distinct_column, ") AS ", alias);
	} else {
		if (phalcon_array_isset_fetch_str(&group_columns, &params, SL("group"), PH_READONLY)) {
			PHALCON_CONCAT_VSVSVSV(&columns, &group_columns, ", ", function, "(", &group_column, ") AS ", alias);
		} else {
			PHALCON_CONCAT_VSVSV(&columns, function, "(", &group_column, ") AS ", alias);
		}
	}
	PHALCON_MM_ADD_ENTRY(&columns);

	PHALCON_MM_CALL_METHOD(&builder, &manager, "createbuilder", &params);
	PHALCON_MM_ADD_ENTRY(&builder);

	PHALCON_MM_CALL_METHOD(NULL, &builder, "columns", &columns);
	PHALCON_MM_CALL_METHOD(NULL, &builder, "from", &model_name);

	PHALCON_MM_ZVAL_STRING(&event_name, "beforeQuery");
	PHALCON_MM_CALL_METHOD(return_value, &model, "fireevent", &event_name, &builder);
	if (Z_TYPE_P(return_value) >= IS_STRING) {
		RETURN_MM();
	}

	PHALCON_MM_CALL_METHOD(&query, &builder, "getquery");
	PHALCON_MM_ADD_ENTRY(&query);

	/**
	 * Execute the query
	 */
	PHALCON_MM_CALL_METHOD(&resultset, &query, "execute");
	PHALCON_MM_ADD_ENTRY(&resultset);

	/**
	 * Return the full resultset if the query is grouped
	 */
	if (phalcon_array_isset_str(&params, SL("group"))) {
		PHALCON_MM_ZVAL_STRING(&event_name, "afterQuery");
		PHALCON_MM_CALL_METHOD(return_value, &model, "fireevent", &event_name, &resultset);
		if (Z_TYPE_P(return_value) >= IS_ARRAY) {
			RETURN_MM();
		}

		RETURN_MM_CTOR(&resultset);
	}

	/**
	 * Return only the value in the first result
	 */
	PHALCON_MM_CALL_METHOD(&first_row, &resultset, "getfirst");
	PHALCON_MM_ADD_ENTRY(&first_row);

	phalcon_read_property_zval(&resultset, &first_row, alias, PH_COPY);
	PHALCON_MM_ADD_ENTRY(&resultset);

	PHALCON_MM_ZVAL_STRING(&event_name, "afterQuery");
	PHALCON_MM_CALL_METHOD(return_value, &model, "fireevent", &event_name, &resultset);
	if (Z_TYPE_P(return_value) >= IS_ARRAY) {
		RETURN_MM();
	}
	RETURN_MM_CTOR(&resultset);
}

/**
 * Generate a PHQL SELECT statement for an aggregate
 *
 *<code>
 *
 *	Robots::group(['aggregators' => array(array('column' => 'id', 'aggregator' => 'sum'), 'sumatory' => array('column' => 'price', 'aggregator' => 'sum'))]);
 *
 *</code>
 *
 * @param array $parameters
 * @return Phalcon\Mvc\Model\ResultsetInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, group){

	zval *params, dependency_injector = {}, columns = {}, aggregators = {}, service_name = {};
	zval manager = {}, model_name = {}, model = {}, builder = {}, query = {};
	zend_string *item_key;

	phalcon_fetch_params(0, 1, 0, &params);

	PHALCON_CALL_CE_STATIC(&dependency_injector, phalcon_di_ce, "getdefault");

	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
		return;
	}

	if (phalcon_array_isset_fetch_str(&aggregators, params, SL("aggregators"), PH_READONLY)) {
		zval *aggregator, joined_columns = {}, group_columns = {};
		if (Z_TYPE(aggregators) != IS_ARRAY) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The aggregators must be array");
			return;
		}

		array_init(&columns);
		ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL(aggregators), item_key, aggregator) {
			zval function = {}, column = {}, alias = {}, distinct = {}, group_column = {};
			if (Z_TYPE_P(aggregator) != IS_ARRAY) {
				continue;
			}
			if (!phalcon_array_isset_fetch_str(&function, aggregator, SL("aggregator"), PH_READONLY)) {
				continue;
			}

			if (!phalcon_array_isset_fetch_str(&column, aggregator, SL("column"), PH_READONLY)) {
				continue;
			}

			if (!item_key) {
				ZVAL_COPY_VALUE(&alias, &function);
			} else {
				ZVAL_STR(&alias, item_key);
			}

			/**
			 * Builds the columns to query according to the received parameters
			 */
			if (phalcon_array_isset_fetch_str(&distinct, aggregator, SL("distinct"), PH_READONLY) && zend_is_true(&distinct)) {
				PHALCON_CONCAT_VSVSV(&group_column, &function, "(DISTINCT ", &column, ") AS ", &alias);
			} else {
				PHALCON_CONCAT_VSVSV(&group_column, &function, "(", &column, ") AS ", &alias);
			}

			phalcon_array_append(&columns, &group_column, 0);
		} ZEND_HASH_FOREACH_END();

		phalcon_fast_join_str(&joined_columns, SL(", "), &columns);
		zval_ptr_dtor(&columns);

		if (phalcon_array_isset_fetch_str(&group_columns, params, SL("group"), PH_READONLY)) {
			PHALCON_CONCAT_VSV(&columns, &group_columns, ", ", &joined_columns);
		} else {
			ZVAL_COPY(&columns, &joined_columns);
		}
		zval_ptr_dtor(&joined_columns);
	} else {
		zval distinct_column = {}, group_columns = {};
		if (phalcon_array_isset_fetch_str(&distinct_column, params, SL("distinct"), PH_READONLY)) {
			PHALCON_CONCAT_SV(&columns, "DISTINCT ", &distinct_column);
		} else if (phalcon_array_isset_fetch_str(&group_columns, params, SL("group"), PH_READONLY)) {
			ZVAL_COPY(&columns, &group_columns);
		} else {
			ZVAL_STRING(&columns, "*");
		}
	}

	ZVAL_STR(&service_name, IS(modelsManager));

	PHALCON_CALL_METHOD(&manager, &dependency_injector, "getshared", &service_name);
	zval_ptr_dtor(&dependency_injector);

	phalcon_get_called_class(&model_name);
	PHALCON_CALL_METHOD(&model, &manager, "load", &model_name);
	PHALCON_CALL_METHOD(&builder, &manager, "createbuilder", params);
	zval_ptr_dtor(&manager);

	PHALCON_CALL_METHOD(NULL, &builder, "columns", &columns);
	zval_ptr_dtor(&columns);

	PHALCON_CALL_METHOD(NULL, &builder, "from", &model_name);
	zval_ptr_dtor(&model_name);

	if (phalcon_method_exists_ex(&model, SL("beforequery")) == SUCCESS) {
		PHALCON_CALL_METHOD(NULL, &model, "beforequery", &builder);
	}
	zval_ptr_dtor(&model);

	PHALCON_CALL_METHOD(&query, &builder, "getquery");
	zval_ptr_dtor(&builder);

	/**
	 * Execute the query
	 */
	PHALCON_CALL_METHOD(return_value, &query, "execute");
	zval_ptr_dtor(&query);
}

/**
 * Allows to count how many records match the specified conditions
 *
 * <code>
 *
 * //How many robots are there?
 * $number = Robots::count();
 * echo "There are ", $number, "\n";
 *
 * //How many mechanical robots are there?
 * $number = Robots::count("type='mechanical'");
 * echo "There are ", $number, " mechanical robots\n";
 *
 * </code>
 *
 * @param array $parameters
 * @return int
 */
PHP_METHOD(Phalcon_Mvc_Model, count){

	zval *parameters = NULL, function = {}, alias = {};

	phalcon_fetch_params(1, 0, 1, &parameters);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_MM_ZVAL_STRING(&function, "COUNT");
	PHALCON_MM_ZVAL_STRING(&alias, "rowcount");

	PHALCON_MM_CALL_SELF(return_value, "_groupresult", &function, &alias, parameters);
	RETURN_MM();
}

/**
 * Allows to calculate a summatory on a column that match the specified conditions
 *
 * <code>
 *
 * //How much are all robots?
 * $sum = Robots::sum(array('column' => 'price'));
 * echo "The total price of robots is ", $sum, "\n";
 *
 * //How much are mechanical robots?
 * $sum = Robots::sum(array("type='mechanical'", 'column' => 'price'));
 * echo "The total price of mechanical robots is  ", $sum, "\n";
 *
 * </code>
 *
 * @param array $parameters
 * @return double
 */
PHP_METHOD(Phalcon_Mvc_Model, sum){

	zval *parameters = NULL, function = {}, alias = {};

	phalcon_fetch_params(0, 0, 1, &parameters);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STRING(&function, "SUM");
	ZVAL_STRING(&alias, "sumatory");

	PHALCON_CALL_SELF(return_value, "_groupresult", &function, &alias, parameters);
	zval_ptr_dtor(&function);
	zval_ptr_dtor(&alias);
}

/**
 * Allows to get the maximum value of a column that match the specified conditions
 *
 * <code>
 *
 * //What is the maximum robot id?
 * $id = Robots::maximum(array('column' => 'id'));
 * echo "The maximum robot id is: ", $id, "\n";
 *
 * //What is the maximum id of mechanical robots?
 * $sum = Robots::maximum(array("type='mechanical'", 'column' => 'id'));
 * echo "The maximum robot id of mechanical robots is ", $id, "\n";
 *
 * </code>
 *
 * @param array $parameters
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Model, maximum){

	zval *parameters = NULL, function = {}, alias = {};

	phalcon_fetch_params(0, 0, 1, &parameters);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STRING(&function, "MAX");
	ZVAL_STRING(&alias, "maximum");

	PHALCON_CALL_SELF(return_value, "_groupresult", &function, &alias, parameters);
	zval_ptr_dtor(&function);
	zval_ptr_dtor(&alias);
}

/**
 * Allows to get the minimum value of a column that match the specified conditions
 *
 * <code>
 *
 * //What is the minimum robot id?
 * $id = Robots::minimum(array('column' => 'id'));
 * echo "The minimum robot id is: ", $id;
 *
 * //What is the minimum id of mechanical robots?
 * $sum = Robots::minimum(array("type='mechanical'", 'column' => 'id'));
 * echo "The minimum robot id of mechanical robots is ", $id;
 *
 * </code>
 *
 * @param array $parameters
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Model, minimum){

	zval *parameters = NULL, function = {}, alias = {};

	phalcon_fetch_params(0, 0, 1, &parameters);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STRING(&function, "MIN");
	ZVAL_STRING(&alias, "minimum");

	PHALCON_CALL_SELF(return_value, "_groupresult", &function, &alias, parameters);
	zval_ptr_dtor(&function);
	zval_ptr_dtor(&alias);
}

/**
 * Allows to calculate the average value on a column matching the specified conditions
 *
 * <code>
 *
 * //What's the average price of robots?
 * $average = Robots::average(array('column' => 'price'));
 * echo "The average price is ", $average, "\n";
 *
 * //What's the average price of mechanical robots?
 * $average = Robots::average(array("type='mechanical'", 'column' => 'price'));
 * echo "The average price of mechanical robots is ", $average, "\n";
 *
 * </code>
 *
 * @param array $parameters
 * @return double
 */
PHP_METHOD(Phalcon_Mvc_Model, average){

	zval *parameters = NULL, function = {}, alias = {};

	phalcon_fetch_params(0, 0, 1, &parameters);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STRING(&function, "AVG");
	ZVAL_STRING(&alias, "average");

	PHALCON_CALL_SELF(return_value, "_groupresult", &function, &alias, parameters);
	zval_ptr_dtor(&function);
	zval_ptr_dtor(&alias);
}

/**
 * Fires an event, implicitly calls behaviors and listeners in the events manager are notified
 *
 * @param string $eventName
 * @param mixed $data
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Model, fireEvent){

	zval *eventname, *data = NULL, *cancelable = NULL, models_manager = {};

	phalcon_fetch_params(0, 1, 2, &eventname, &data, &cancelable);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!cancelable) {
		cancelable = &PHALCON_GLOBAL(z_null);
	}

	if (likely(PHALCON_GLOBAL(orm).events)) {
		PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");

		/**
		 * Send a notification to the events manager
		 */
		PHALCON_CALL_METHOD(return_value, &models_manager, "notifyevent", eventname, getThis());
		zval_ptr_dtor(&models_manager);

		if (!PHALCON_IS_FALSE(return_value)) {
			zval fire_event_name = {};
			PHALCON_CONCAT_SV(&fire_event_name, "model:", eventname);
			PHALCON_CALL_PARENT(return_value, phalcon_mvc_model_ce, getThis(), "fireevent", &fire_event_name, data, cancelable);
			zval_ptr_dtor(&fire_event_name);
		}
	} else {
		RETURN_TRUE;
	}
}

/**
 * Fires an event, implicitly calls behaviors and listeners in the events manager are notified
 * This method stops if one of the callbacks/listeners returns boolean false
 *
 * @param string $eventName
 * @param mixed $data
 * @param boolean $cancelAble
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, fireEventCancel){

	zval *eventname, *data = NULL, *cancelable = NULL, models_manager = {};

	phalcon_fetch_params(0, 1, 2, &eventname, &data, &cancelable);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!cancelable) {
		cancelable = &PHALCON_GLOBAL(z_null);
	}

	if (likely(PHALCON_GLOBAL(orm).events)) {
		PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");

		/**
		 * Send a notification to the events manager
		 */
		PHALCON_CALL_METHOD(return_value, &models_manager, "notifyevent", eventname, getThis());
		zval_ptr_dtor(&models_manager);

		if (!PHALCON_IS_FALSE(return_value)) {
			zval fire_event_name = {};
			PHALCON_CONCAT_SV(&fire_event_name, "model:", eventname);
			PHALCON_CALL_PARENT(return_value, phalcon_mvc_model_ce, getThis(), "fireeventcancel", &fire_event_name, data, cancelable);
			zval_ptr_dtor(&fire_event_name);
		}
	} else {
		RETURN_TRUE;
	}
}

/**
 * Cancel the current operation
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _cancelOperation){

	zval operation_made = {}, event_name = {};

	phalcon_read_property(&operation_made, getThis(), SL("_operationMade"), PH_NOISY|PH_READONLY);

	if (PHALCON_IS_LONG(&operation_made, 3)) {
		ZVAL_STRING(&event_name, "notDeleted");
	} else {
		ZVAL_STRING(&event_name, "notSaved");
	}

	PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
	zval_ptr_dtor(&event_name);
}

/**
 * Appends a customized message on the validation process
 *
 * <code>
 * use \Phalcon\Validation\Message as Message;
 *
 * class Robots extends Phalcon\Mvc\Model
 * {
 *
 *   public function beforeSave()
 *   {
 *     if ($this->name == 'Peter') {
 *        $message = new Message("Sorry, but a robot cannot be named Peter");
 *        $this->appendMessage($message);
 *     }
 *   }
 * }
 * </code>
 *
 * @param Phalcon\Validation\MessageInterface $message
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, appendMessage){

	zval *message, *field = NULL, *t = NULL, *code = NULL, type = {}, custom_message = {}, exception_message = {}, model_message = {};
	zval message_message = {}, message_field = {}, message_type = {}, message_code = {};

	phalcon_fetch_params(1, 1, 3, &message, &field, &t, &code);

	if (!field) {
		field = &PHALCON_GLOBAL(z_null);
	}

	if (t && Z_TYPE_P(t) != IS_NULL) {
		PHALCON_MM_ZVAL_COPY(&type, t);
	} else {
		PHALCON_MM_ZVAL_STRING(&type, zend_zval_type_name(message));
	}

	if (!code) {
		code = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(message) != IS_OBJECT) {
		if (PHALCON_IS_EMPTY(field) || PHALCON_IS_EMPTY(&type)) {
			PHALCON_CONCAT_SVSVS(&exception_message, "Invalid message format '", &type, "', message: '", message, "'");
			PHALCON_MM_ADD_ENTRY(&exception_message);
			PHALCON_MM_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
			return;
		}

		if (phalcon_method_exists_ex(getThis(), SL("messages")) == SUCCESS) {
			PHALCON_MM_CALL_METHOD(&custom_message, getThis(), "messages", message, field, &type, code);
		} else {
			ZVAL_COPY(&custom_message, message);
		}
		PHALCON_MM_ADD_ENTRY(&custom_message);

		object_init_ex(&model_message, phalcon_validation_message_ce);
		PHALCON_CALL_METHOD(NULL, &model_message, "__construct", &custom_message, field, &type, code);
		PHALCON_MM_ADD_ENTRY(&model_message);
		phalcon_update_property_array_append(getThis(), SL("_errorMessages"), &model_message);
	} else {
		if (phalcon_method_exists_ex(getThis(), SL("messages")) == SUCCESS) {
			PHALCON_MM_CALL_METHOD(&message_message, message, "getmessage");
			PHALCON_MM_ADD_ENTRY(&message_message);
			PHALCON_MM_CALL_METHOD(&message_field, message, "getfield");
			PHALCON_MM_ADD_ENTRY(&message_field);
			PHALCON_MM_CALL_METHOD(&message_type, message, "gettype");
			PHALCON_MM_ADD_ENTRY(&message_type);
			PHALCON_MM_CALL_METHOD(&message_code, message, "getcode");
			PHALCON_MM_ADD_ENTRY(&message_code);
			PHALCON_MM_CALL_METHOD(&custom_message, getThis(), "messages", &message_message, &message_field, &message_type, &message_code);
			PHALCON_MM_ADD_ENTRY(&custom_message);

			PHALCON_MM_CALL_METHOD(NULL, message, "setmessage", &custom_message);
		}

		phalcon_update_property_array_append(getThis(), SL("_errorMessages"), message);
	}

	RETURN_MM_THIS();
}

/**
 * Executes validators on every validation call
 *
 *<code>
 *use Phalcon\Mvc\Model\Validator\ExclusionIn as ExclusionIn;
 *
 *class Subscriptors extends Phalcon\Mvc\Model
 *{
 *
 *	public function validation()
 *  {
 * 		$validation = new Phalcon\Validation();
 * 		$validation->add('status', new ExclusionIn(array(
 *			'domain' => array('A', 'I')
 *		)));
 * 		return $this->validate($validation);
 *	}
 *
 *}
 *</code>
 *
 * @param array|Phalcon\ValidationInterface $validation
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, validate){

	zval *validation, field = {}, handler = {}, value = {}, arguments = {}, status = {};
	zval prepared = {}, type = {}, code = {}, messages = {};

	phalcon_fetch_params(1, 1, 0, &validation);

	if (Z_TYPE_P(validation) == IS_ARRAY) {
		if (!phalcon_array_isset_fetch_str(&field, validation, SL("field"), PH_READONLY)) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Invalid field");
			return;
		}

		if (!phalcon_array_isset_fetch_str(&handler, validation, SL("validator"), PH_READONLY)) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Invalid validator");
			return;
		}

		if (!phalcon_is_callable(&handler)) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Validator must be an callable");
			return;
		}

		PHALCON_MM_CALL_METHOD(&value, getThis(), "readattribute", &field);
		PHALCON_MM_ADD_ENTRY(&value);

		if (Z_TYPE(handler) == IS_OBJECT && instanceof_function(Z_OBJCE(handler), zend_ce_closure)) {
			PHALCON_MM_CALL_METHOD(&status, &handler, "call", getThis(), &value);
		} else {
			array_init_size(&arguments, 1);
			phalcon_array_append(&arguments, &value, PH_COPY);
			PHALCON_MM_ADD_ENTRY(&arguments);

			PHALCON_MM_CALL_USER_FUNC_ARRAY(&status, &handler, &arguments);
		}

		if (PHALCON_IS_FALSE(&status)) {
			zval label = {}, message_str = {}, pairs = {};
			if (phalcon_method_exists_ex(getThis(), SL("getlabel")) == SUCCESS) {
				PHALCON_MM_CALL_METHOD(&label, getThis(), "getlabel", &field);
			} else {
				ZVAL_COPY(&label, &field);
			}
			PHALCON_MM_ADD_ENTRY(&label);

			if (!phalcon_array_isset_fetch_str(&type, validation, SL("type"), PH_READONLY)) {
				PHALCON_MM_ZVAL_STRING(&type, "Validator");
			}

			if (!phalcon_array_isset_fetch_str(&code, validation, SL("code"), PH_READONLY)) {
				ZVAL_COPY_VALUE(&code, &PHALCON_GLOBAL(z_zero));
			}

			if (phalcon_array_isset_fetch_str(&message_str, validation, SL("message"), PH_READONLY)) {
				array_init_size(&pairs, 1);
				phalcon_array_update_str(&pairs, SL(":field"), &label, PH_COPY);
				PHALCON_MM_ADD_ENTRY(&pairs);

				PHALCON_MM_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);
				PHALCON_MM_ADD_ENTRY(&prepared);
			} else {
				PHALCON_MM_CALL_CE_STATIC(&message_str, phalcon_validation_ce, "getmessage", &type);
				PHALCON_MM_ADD_ENTRY(&message_str);
				if (PHALCON_IS_NOT_EMPTY(&message_str)) {
					array_init_size(&pairs, 1);
					phalcon_array_update_str(&pairs, SL(":field"), &label, PH_COPY);
					PHALCON_MM_ADD_ENTRY(&pairs);

					PHALCON_MM_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);
					PHALCON_MM_ADD_ENTRY(&prepared);
				} else {
					PHALCON_CONCAT_SVS(&prepared, "Invalid '", &label, "' format");
					PHALCON_MM_ADD_ENTRY(&prepared);
				}
			}

			PHALCON_MM_CALL_METHOD(NULL, getThis(), "appendmessage", &prepared, &field, &type, &code);
		}

		RETURN_MM_NCTOR(&status);
	}

	/**
	 * Valid validation are objects
	 */
	PHALCON_MM_VERIFY_INTERFACE_EX(validation, phalcon_validationinterface_ce, phalcon_mvc_model_exception_ce);

	/**
	 * Call the validation, if it returns false we append the messages to the current
	 * object
	 */
	PHALCON_MM_CALL_METHOD(&messages, validation, "validate", &PHALCON_GLOBAL(z_null), getThis());
	PHALCON_MM_ADD_ENTRY(&messages);

	if (Z_TYPE(messages) == IS_OBJECT) {
		PHALCON_MM_VERIFY_CLASS_EX(&messages, phalcon_validation_message_group_ce, phalcon_mvc_model_exception_ce);
		PHALCON_MM_CALL_METHOD(NULL, &messages, "rewind");

		while (1) {
			zval valid = {}, current = {};

			PHALCON_MM_CALL_METHOD(&valid, &messages, "valid");
			if (PHALCON_IS_FALSE(&valid)) {
				break;
			}
			zval_ptr_dtor(&valid);

			PHALCON_MM_CALL_METHOD(&current, &messages, "current");
			PHALCON_MM_ADD_ENTRY(&current);
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "appendmessage", &current);
			PHALCON_MM_CALL_METHOD(NULL, &messages, "next");
		}

		if (phalcon_fast_count_int(&messages)) {
			RETVAL_FALSE;
		} else {
			RETVAL_TRUE;
		}
		RETURN_MM();
	}

	RETURN_MM_CTOR(&messages);
}

/**
 * Check whether validation process has generated any messages
 *
 *<code>
 *use Phalcon\Mvc\Model\Validator\ExclusionIn as ExclusionIn;
 *
 *class Subscriptors extends Phalcon\Mvc\Model
 *{
 *
 *	public function validation()
 *  {
 * 		$this->validate(new ExclusionIn(array(
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
PHP_METHOD(Phalcon_Mvc_Model, validationHasFailed){

	zval error_messages = {};

	phalcon_read_property(&error_messages, getThis(), SL("_errorMessages"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(error_messages) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL(error_messages))) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Returns all the validation messages
 *
 *<code>
 *	$robot = new Robots();
 *	$robot->type = 'mechanical';
 *	$robot->name = 'Astro Boy';
 *	$robot->year = 1952;
 *	if ($robot->save() == false) {
 *  	echo "Umh, We can't store robots right now ";
 *  	foreach ($robot->getMessages() as $message) {
 *			echo $message;
 *		}
 *	} else {
 *  	echo "Great, a new robot was saved successfully!";
 *	}
 * </code>
 *
 * @return Phalcon\Mvc\Model\MessageInterface[]
 */
PHP_METHOD(Phalcon_Mvc_Model, getMessages){

	zval *filter = NULL, messages = {}, *value, field = {};

	phalcon_fetch_params(0, 0, 1, &filter);

	if (!filter || (Z_TYPE_P(filter) != IS_STRING && Z_TYPE_P(filter) != IS_ARRAY)) {
		RETURN_MEMBER(getThis(), "_errorMessages");
	}

	phalcon_read_property(&messages, getThis(), SL("_errorMessages"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(messages) == IS_ARRAY) {
		array_init(return_value);

		if (Z_TYPE_P(filter) == IS_ARRAY) {
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(messages), value) {
				PHALCON_CALL_METHOD(&field, value, "getfield");

				if (phalcon_fast_in_array(&field, filter)) {
					phalcon_array_append(return_value, value, PH_COPY);
				}
			} ZEND_HASH_FOREACH_END();
		} else {
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(messages), value) {
				PHALCON_CALL_METHOD(&field, value, "getfield");

				if (PHALCON_IS_EQUAL(&field, filter)) {
					phalcon_array_append(return_value, value, PH_COPY);
				}
			} ZEND_HASH_FOREACH_END();
		}
	}
}

/**
 * Reads "belongs to" relations and check the virtual foreign keys when inserting or updating records
 * to verify that inserted/updated values are present in the related entity
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _checkForeignKeysRestrict){

	zval models_manager = {}, belongs_to = {}, error = {}, *relation, event_name;

	/**
	 * Get the models manager
	 */
	PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");

	/**
	 * We check if some of the belongsTo relations act as virtual foreign key
	 */
	PHALCON_CALL_METHOD(&belongs_to, &models_manager, "getbelongsto", getThis());

	if (phalcon_fast_count_ev(&belongs_to)) {
		ZVAL_FALSE(&error);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(belongs_to), relation) {
			zval foreign_key = {}, action = {}, relation_class = {}, referenced_model = {}, conditions = {}, bind_params = {}, fields = {}, referenced_fields = {}, condition = {}, *field;
			zval value = {}, extra_conditions = {}, join_conditions = {}, parameters = {}, rowcount = {}, user_message = {}, joined_fields = {}, type = {}, label = {}, pairs = {}, prepared = {};
			zend_string *str_key;
			ulong idx;

			PHALCON_CALL_METHOD(&foreign_key, relation, "getforeignkey");
			if (PHALCON_IS_NOT_FALSE(&foreign_key)) {
				/**
				 * Try to find a different action in the foreign key's options
				 */
				if (Z_TYPE(foreign_key) != IS_ARRAY || !phalcon_array_isset_fetch_str(&action, &foreign_key, SL("action"), PH_READONLY)) {
					/**
					 * By default action is restrict
					 */
					ZVAL_LONG(&action, 1);
				}

				/**
				 * Check only if the operation is restrict
				 */
				if (PHALCON_IS_LONG(&action, 1)) {
					PHALCON_CALL_METHOD(&relation_class, relation, "getreferencedmodel");

					/**
					 * Load the referenced model if needed
					 */
					PHALCON_CALL_METHOD(&referenced_model, &models_manager, "load", &relation_class);
					zval_ptr_dtor(&relation_class);

					/**
					 * Since relations can have multiple columns or a single one, we need to build a
					 * condition for each of these cases
					 */
					array_init(&conditions);
					array_init(&bind_params);

					PHALCON_CALL_METHOD(&fields, relation, "getfields");
					PHALCON_CALL_METHOD(&referenced_fields, relation, "getreferencedfields");

					if (Z_TYPE(fields) == IS_ARRAY) {
						/**
						 * Create a compound condition
						 */
						ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(fields), idx, str_key, field) {
							zval position = {}, referenced_field = {};
							if (str_key) {
								ZVAL_STR(&position, str_key);
							} else {
								ZVAL_LONG(&position, idx);
							}

							phalcon_read_property_zval(&value, getThis(), field, PH_NOISY|PH_READONLY);

							phalcon_array_fetch(&referenced_field, &referenced_fields, &position, PH_NOISY|PH_READONLY);

							PHALCON_CONCAT_SVSV(&condition, "[", &referenced_field, "] = ?", &position);
							phalcon_array_append(&conditions, &condition, PH_COPY);
							phalcon_array_append(&bind_params, &value, PH_COPY);
							zval_ptr_dtor(&condition);

						} ZEND_HASH_FOREACH_END();

					} else {
						/**
						 * Create a simple condition
						 */
						phalcon_read_property_zval(&value, getThis(), &fields, PH_NOISY|PH_READONLY);

						PHALCON_CONCAT_SVS(&condition, "[", &referenced_fields, "] = ?0");
						phalcon_array_append(&conditions, &condition, PH_COPY);
						phalcon_array_append(&bind_params, &value, PH_COPY);
						zval_ptr_dtor(&condition);
					}

					/**
					 * Check if the virtual foreign key has extra conditions
					 */
					if (phalcon_array_isset_fetch_str(&extra_conditions, &foreign_key, SL("conditions"), PH_READONLY)) {
						phalcon_array_append(&conditions, &extra_conditions, PH_COPY);
					}

					/**
					 * We don't trust the actual values in the object and pass the values using bound
					 * parameters
					 */
					phalcon_fast_join_str(&join_conditions, SL(" AND "), &conditions);
					zval_ptr_dtor(&conditions);

					array_init_size(&parameters, 2);
					phalcon_array_append(&parameters, &join_conditions, PH_COPY);
					phalcon_array_update_str(&parameters, SL("bind"), &bind_params, PH_COPY);

					/**
					 * Let's make the checking
					 */
					PHALCON_CALL_METHOD(&rowcount, &referenced_model, "count", &parameters);
					zval_ptr_dtor(&parameters);
					if (!zend_is_true(&rowcount)) {
						/**
						 * Get the user message or produce a new one
						 */
						ZVAL_STRING(&type, "ConstraintViolation");
						if (!phalcon_array_isset_fetch_str(&prepared, &foreign_key, SL("message"), PH_COPY)) {
							/**
							 * Create a message
							 */
							PHALCON_CALL_CE_STATIC(&user_message, phalcon_validation_ce, "getmessage", &type);
							if (phalcon_method_exists_ex(&referenced_model, SL("getlabel")) == SUCCESS) {
								PHALCON_CALL_METHOD(&joined_fields, &referenced_model, "getlabel", &fields);
								if (Z_TYPE(joined_fields) == IS_ARRAY) {
									phalcon_fast_join_str(&label, SL(", "), &joined_fields);
								} else {
									ZVAL_COPY(&label, &joined_fields);
								}
								zval_ptr_dtor(&joined_fields);
							} else {
								if (Z_TYPE(fields) == IS_ARRAY) {
									phalcon_fast_join_str(&label, SL(", "), &fields);
								} else {
									ZVAL_COPY(&label, &fields);
								}
							}
							array_init_size(&pairs, 1);
							phalcon_array_update_str(&pairs, SL(":field"), &label, PH_COPY);
							zval_ptr_dtor(&label);
							PHALCON_CALL_FUNCTION(&prepared, "strtr", &user_message, &pairs);
							zval_ptr_dtor(&user_message);
							zval_ptr_dtor(&pairs);
						}

						PHALCON_CALL_METHOD(NULL, getThis(), "appendmessage", &prepared, &fields, &type);
						zval_ptr_dtor(&prepared);
						zval_ptr_dtor(&type);

						ZVAL_TRUE(&error);
						break;
					}
					zval_ptr_dtor(&fields);
					zval_ptr_dtor(&referenced_fields);
					zval_ptr_dtor(&referenced_model);
				}
			}
			zval_ptr_dtor(&foreign_key);
		} ZEND_HASH_FOREACH_END();

		/**
		 * Call 'onValidationFails' if the validation fails
		 */
		if (PHALCON_IS_TRUE(&error)) {
			ZVAL_STRING(&event_name, "onValidationFails");
			PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
			PHALCON_CALL_METHOD(NULL, getThis(), "_canceloperation");
			zval_ptr_dtor(&event_name);
			RETURN_FALSE;
		}
	}
	zval_ptr_dtor(&models_manager);
	zval_ptr_dtor(&belongs_to);

	RETURN_TRUE;
}

/**
 * Reads both "hasMany" and "hasOne" relations and checks the virtual foreign keys (restrict) when deleting records
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _checkForeignKeysReverseRestrict){

	zval models_manager = {}, relations = {}, error = {}, *relation, event_name;

	RETVAL_TRUE;

	PHALCON_MM_INIT();

	/**
	 * Get the models manager
	 */
	PHALCON_MM_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_MM_ADD_ENTRY(&models_manager);

	/**
	 * We check if some of the hasOne/hasMany relations is a foreign key
	 */
	PHALCON_MM_CALL_METHOD(&relations, &models_manager, "gethasoneandhasmany", getThis());
	PHALCON_MM_ADD_ENTRY(&relations);
	if (phalcon_fast_count_ev(&relations)) {
		ZVAL_FALSE(&error);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(relations), relation) {
			zval foreign_key = {}, action = {}, relation_class = {}, referenced_model = {}, fields = {}, referenced_fields = {}, conditions = {}, bind_params = {}, *field, value = {};
			zval condition = {}, extra_conditions = {}, join_conditions = {}, parameters = {}, rowcount = {}, user_message = {}, type = {};
			zend_string *str_key;
			ulong idx;

			/**
			 * Check if the relation has a virtual foreign key
			 */
			PHALCON_MM_CALL_METHOD(&foreign_key, relation, "getforeignkey");
			PHALCON_MM_ADD_ENTRY(&foreign_key);
			if (PHALCON_IS_NOT_FALSE(&foreign_key)) {
				/**
				 * By default action is restrict
				 */
				ZVAL_LONG(&action, 1);

				/**
				 * Try to find a different action in the foreign key's options
				 */
				if (Z_TYPE(foreign_key) == IS_ARRAY && phalcon_array_isset_str(&foreign_key, SL("action"))) {
					phalcon_array_fetch_str(&action, &foreign_key, SL("action"), PH_NOISY|PH_READONLY);
				}

				/**
				 * Check only if the operation is restrict
				 */
				if (PHALCON_IS_LONG(&action, 1)) {
					PHALCON_MM_CALL_METHOD(&relation_class, relation, "getreferencedmodel");
					PHALCON_MM_ADD_ENTRY(&relation_class);

					/**
					 * Load a plain instance from the models manager
					 */
					PHALCON_MM_CALL_METHOD(&referenced_model, &models_manager, "load", &relation_class);
					PHALCON_MM_ADD_ENTRY(&referenced_model);
					PHALCON_MM_CALL_METHOD(&fields, relation, "getfields");
					PHALCON_MM_ADD_ENTRY(&fields);
					PHALCON_MM_CALL_METHOD(&referenced_fields, relation, "getreferencedfields");
					PHALCON_MM_ADD_ENTRY(&referenced_fields);

					/**
					 * Create the checking conditions. A relation can has many fields or a single one
					 */
					array_init(&conditions);
					array_init(&bind_params);

					if (Z_TYPE(fields) == IS_ARRAY) {

						ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(fields), idx, str_key, field) {
							zval tmp = {}, referenced_field = {};
							if (str_key) {
								ZVAL_STR(&tmp, str_key);
							} else {
								ZVAL_LONG(&tmp, idx);
							}

							phalcon_read_property_zval(&value, getThis(), field, PH_NOISY|PH_READONLY);

							phalcon_array_fetch(&referenced_field, &referenced_fields, &tmp, PH_NOISY|PH_READONLY);

							PHALCON_CONCAT_SVSV(&condition, "[", &referenced_field, "] = ?", &tmp);
							phalcon_array_append(&conditions, &condition, 0);
							phalcon_array_append(&bind_params, &value, PH_COPY);
						} ZEND_HASH_FOREACH_END();

					} else {
						phalcon_read_property_zval(&value, getThis(), &fields, PH_NOISY|PH_READONLY);

						PHALCON_CONCAT_SVS(&condition, "[", &referenced_fields, "] = ?0");
						phalcon_array_append(&conditions, &condition, 0);
						phalcon_array_append(&bind_params, &value, PH_COPY);
					}

					/**
					 * Check if the virtual foreign key has extra conditions
					 */
					if (phalcon_array_isset_fetch_str(&extra_conditions, &foreign_key, SL("conditions"), PH_READONLY)) {
						phalcon_array_append(&conditions, &extra_conditions, PH_COPY);
					}

					/**
					 * We don't trust the actual values in the object and then we're passing the values
					 * using bound parameters
					 */
					phalcon_fast_join_str(&join_conditions, SL(" AND "), &conditions);
					zval_ptr_dtor(&conditions);

					array_init_size(&parameters, 2);
					phalcon_array_append(&parameters, &join_conditions, 0);
					phalcon_array_update_str(&parameters, SL("bind"), &bind_params, 0);
					PHALCON_MM_ADD_ENTRY(&parameters);

					/**
					 * Let's make the checking
					 */
					PHALCON_MM_CALL_METHOD(&rowcount, &referenced_model, "count", &parameters);
					PHALCON_MM_ADD_ENTRY(&rowcount);
					if (zend_is_true(&rowcount)) {

						/**
						 * Create a new message
						 */
						if (!phalcon_array_isset_fetch_str(&user_message, &foreign_key, SL("message"), PH_COPY)) {
							PHALCON_CONCAT_SV(&user_message, "Record is referenced by model ", &relation_class);
						}
						PHALCON_MM_ADD_ENTRY(&user_message);

						/**
						 * Create a message
						 */
						PHALCON_MM_ZVAL_STRING(&type, "ConstraintViolation");
						PHALCON_MM_CALL_METHOD(NULL, getThis(), "appendmessage", &user_message, &fields, &type);

						ZVAL_BOOL(&error, 1);
						break;
					}
				}
			}
		} ZEND_HASH_FOREACH_END();

		/**
		 * Call validation fails event
		 */
		if (PHALCON_IS_TRUE(&error)) {
			PHALCON_MM_ZVAL_STRING(&event_name, "onValidationFails");
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "_canceloperation");

			RETVAL_FALSE;
		}
	}
	RETURN_MM();
}

/**
 * Reads both "hasMany" and "hasOne" relations and checks the virtual foreign keys (cascade) when deleting records
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _checkForeignKeysReverseCascade){

	zval models_manager = {}, relations = {}, *relation;

	PHALCON_MM_INIT();

	/**
	 * Get the models manager
	 */
	PHALCON_MM_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_MM_ADD_ENTRY(&models_manager);

	/**
	 * We check if some of the hasOne/hasMany relations is a foreign key
	 */
	PHALCON_MM_CALL_METHOD(&relations, &models_manager, "gethasoneandhasmany", getThis());
	PHALCON_MM_ADD_ENTRY(&relations);
	if (phalcon_fast_count_ev(&relations)) {

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(relations), relation) {
			zval foreign_key = {}, action = {}, relation_class = {}, referenced_model = {}, fields = {}, referenced_fields = {}, conditions = {}, bind_params = {}, *field;
			zval value = {}, condition = {}, extra_conditions = {}, join_conditions = {}, parameters = {}, resulset = {}, status = {};
			zend_string *str_key;
			ulong idx;

			/**
			 * Check if the relation has a virtual foreign key
			 */
			PHALCON_MM_CALL_METHOD(&foreign_key, relation, "getforeignkey");
			PHALCON_MM_ADD_ENTRY(&foreign_key);
			if (PHALCON_IS_NOT_FALSE(&foreign_key)) {
				/**
				 * By default action is restrict
				 */
				ZVAL_LONG(&action, 0);

				/**
				 * Try to find a different action in the foreign key's options
				 */
				if (Z_TYPE(foreign_key) == IS_ARRAY && phalcon_array_isset_str(&foreign_key, SL("action"))) {
					phalcon_array_fetch_str(&action, &foreign_key, SL("action"), PH_NOISY|PH_READONLY);
				}

				/**
				 * Check only if the operation is restrict
				 */
				if (PHALCON_IS_LONG(&action, 2)) {
					PHALCON_MM_CALL_METHOD(&relation_class, relation, "getreferencedmodel");
					PHALCON_MM_ADD_ENTRY(&relation_class);

					/**
					 * Load a plain instance from the models manager
					 */
					PHALCON_MM_CALL_METHOD(&referenced_model, &models_manager, "load", &relation_class);
					PHALCON_MM_ADD_ENTRY(&referenced_model);
					PHALCON_MM_CALL_METHOD(&fields, relation, "getfields");
					PHALCON_MM_ADD_ENTRY(&fields);
					PHALCON_MM_CALL_METHOD(&referenced_fields, relation, "getreferencedfields");
					PHALCON_MM_ADD_ENTRY(&referenced_fields);

					/**
					 * Create the checking conditions. A relation can has many fields or a single one
					 */
					array_init(&conditions);
					array_init(&bind_params);
					if (Z_TYPE(fields) == IS_ARRAY) {
						ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(fields), idx, str_key, field) {
							zval tmp = {}, referenced_field = {};
							if (str_key) {
								ZVAL_STR(&tmp, str_key);
							} else {
								ZVAL_LONG(&tmp, idx);
							}

							phalcon_read_property_zval(&value, getThis(), field, PH_NOISY|PH_READONLY);
							phalcon_array_fetch(&referenced_field, &referenced_fields, &tmp, PH_NOISY|PH_READONLY);

							PHALCON_CONCAT_SVSV(&condition, "[", &referenced_field, "] = ?", &tmp);
							phalcon_array_append(&conditions, &condition, 0);
							phalcon_array_append(&bind_params, &value, PH_COPY);
						} ZEND_HASH_FOREACH_END();

					} else {
						phalcon_read_property_zval(&value, getThis(), &fields, PH_NOISY|PH_READONLY);

						PHALCON_CONCAT_SVS(&condition, "[", &referenced_fields, "] = ?0");
						phalcon_array_append(&conditions, &condition, 0);
						phalcon_array_append(&bind_params, &value, PH_COPY);
					}

					/**
					 * Check if the virtual foreign key has extra conditions
					 */
					if (phalcon_array_isset_fetch_str(&extra_conditions, &foreign_key, SL("conditions"), PH_READONLY)) {
						phalcon_array_append(&conditions, &extra_conditions, PH_COPY);
					}

					/**
					 * We don't trust the actual values in the object and then we're passing the values
					 * using bound parameters
					 */
					phalcon_fast_join_str(&join_conditions, SL(" AND "), &conditions);
					zval_ptr_dtor(&conditions);

					array_init_size(&parameters, 2);
					phalcon_array_append(&parameters, &join_conditions, 0);
					phalcon_array_update_str(&parameters, SL("bind"), &bind_params, 0);
					PHALCON_MM_ADD_ENTRY(&parameters);

					/**
					 * Let's make the checking
					 */
					PHALCON_MM_CALL_METHOD(&resulset, &referenced_model, "find", &parameters);
					PHALCON_MM_ADD_ENTRY(&resulset);

					/**
					 * Delete the resultset
					 */
					PHALCON_MM_CALL_METHOD(&status, &resulset, "delete");

					/**
					 * Stop the operation
					 */
					if (PHALCON_IS_FALSE(&status)) {
						RETURN_MM_FALSE;
					}
				}
			}
		} ZEND_HASH_FOREACH_END();

	}

	RETURN_MM_TRUE;
}

/**
 * Executes internal hooks before save a record
 *
 * @param Phalcon\Mvc\Model\MetadataInterface $metaData
 * @param boolean $exists
 * @param string $identityField
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _preSave){

	zval *exists, *identity_field, event_name = {}, status = {}, attributes = {}, data_type_numeric = {}, data_types = {}, column_map = {};
	zval automatic_attributes = {}, default_values = {}, *error, *field, skipped = {}, exception_message = {};
	int method_exists = 0;

	phalcon_fetch_params(1, 2, 0, &exists, &identity_field);

	if (phalcon_method_exists_ex(getThis(), SL("filters")) == SUCCESS) {
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "filters");
	}

	/**
	 * Run Validation Callbacks Before
	 */
	if (likely(PHALCON_GLOBAL(orm).events)) {
		/**
		 * Call the beforeValidation
		 */
	 	PHALCON_MM_ZVAL_STRING(&event_name, "beforeValidation");
		PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_MM_FALSE;
		}
		zval_ptr_dtor(&status);

		if (!zend_is_true(exists)) {
			PHALCON_MM_ZVAL_STRING(&event_name, "beforeValidationOnCreate");
		} else {
			PHALCON_MM_ZVAL_STRING(&event_name, "beforeValidationOnUpdate");
		}

		/**
		 * Call the specific beforeValidation event for the current action
		 */
		PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_MM_FALSE;
		}
		zval_ptr_dtor(&status);
	}

	/**
	 * Check for Virtual foreign keys
	 */
	if (PHALCON_GLOBAL(orm).virtual_foreign_keys) {
		PHALCON_MM_CALL_METHOD(&status, getThis(), "_checkforeignkeysrestrict");
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_MM_FALSE;
		}
		zval_ptr_dtor(&status);
	}

	PHALCON_MM_CALL_METHOD(&attributes, getThis(), "getattributes");
	PHALCON_MM_ADD_ENTRY(&attributes);
	PHALCON_MM_CALL_METHOD(&data_type_numeric, getThis(), "getdatatypesnumeric");
	PHALCON_MM_ADD_ENTRY(&data_type_numeric);
	PHALCON_MM_CALL_METHOD(&data_types, getThis(), "getdatatypes");
	PHALCON_MM_ADD_ENTRY(&data_types);
	PHALCON_MM_CALL_METHOD(&column_map, getThis(), "getcolumnmap");
	PHALCON_MM_ADD_ENTRY(&column_map);

	/**
	 * Get fields that must be omitted from the SQL generation
	 */
	if (zend_is_true(exists)) {
		PHALCON_MM_CALL_METHOD(&automatic_attributes, getThis(), "getautomaticupdateattributes");
		PHALCON_MM_ADD_ENTRY(&automatic_attributes);
		array_init(&default_values);
	} else {
		PHALCON_MM_CALL_METHOD(&automatic_attributes, getThis(), "getautomaticupdateattributes");
		PHALCON_MM_ADD_ENTRY(&automatic_attributes);
		PHALCON_MM_CALL_METHOD(&default_values, getThis(), "getdefaultvalues");
	}
	PHALCON_MM_ADD_ENTRY(&default_values);

	error = &PHALCON_GLOBAL(z_false);

	/**
	 * Call the main validation event
	 */
 	PHALCON_MM_ZVAL_STRING(&event_name, "validation");
	PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
	if (PHALCON_IS_FALSE(&status)) {
		PHALCON_MM_ZVAL_STRING(&event_name, "onValidationFails");
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);

		RETURN_MM_FALSE;
	}
	zval_ptr_dtor(&status);

	if (phalcon_method_exists_ex(getThis(), SL("getlabel")) == SUCCESS) {
		method_exists = 1;
	}

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(attributes), field) {
		zval attribute_field = {}, value = {}, field_type = {}, is_not_null = {}, message = {}, type = {}, field_size = {};
		zval length = {}, label = {}, pairs = {}, prepared = {};
		/**
		 * We don't check fields that must be omitted
		 */
		if (!phalcon_array_isset(&automatic_attributes, field)) {

			if (Z_TYPE(column_map) == IS_ARRAY) {
				if (!phalcon_array_isset_fetch(&attribute_field, &column_map, field, PH_READONLY)) {
					PHALCON_CONCAT_SVS(&exception_message, "Column '", field, "' isn't part of the column map");
					PHALCON_MM_ADD_ENTRY(&exception_message);
					PHALCON_MM_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}
			} else {
				ZVAL_COPY_VALUE(&attribute_field, field);
			}

			if (phalcon_isset_property_zval(getThis(), &attribute_field)) {
				/**
				 * Read the attribute from the this_ptr using the real or renamed name
				 */
				phalcon_read_property_zval(&value, getThis(), &attribute_field, PH_READONLY);
			} else {
				ZVAL_NULL(&value);
			}

			phalcon_array_fetch(&field_type, &data_types, field, PH_NOISY|PH_READONLY);

			/**
			 * Field is null when: 1) is not set, 2) is numeric but its value is not numeric,
			 * 3) is null or 4) is empty string
			 */
			if (Z_TYPE(value) <= IS_NULL) {
				if (!PHALCON_GLOBAL(orm).not_null_validations) {
					continue;
				}

				if (!zend_is_true(exists) && PHALCON_IS_EQUAL(field, identity_field)) {
					continue;
				}

				PHALCON_MM_CALL_METHOD(&is_not_null, getThis(), "isnotnull", field);
				if (zend_is_true(&is_not_null) && !phalcon_array_isset(&default_values, field)) {
					PHALCON_MM_ZVAL_STRING(&type, "PresenceOf");
					PHALCON_MM_CALL_CE_STATIC(&message, phalcon_validation_ce, "getmessage", &type);
					PHALCON_MM_ADD_ENTRY(&message);
					if (method_exists) {
						PHALCON_MM_CALL_METHOD(&label, getThis(), "getlabel", &attribute_field);
						PHALCON_MM_ADD_ENTRY(&label);
					} else {
						PHALCON_MM_ZVAL_COPY(&label, &attribute_field);
					}
					array_init_size(&pairs, 1);
					phalcon_array_update_str(&pairs, SL(":field"), &label, PH_COPY);
					PHALCON_MM_ADD_ENTRY(&pairs);

					PHALCON_MM_CALL_FUNCTION(&prepared, "strtr", &message, &pairs);
					PHALCON_MM_ADD_ENTRY(&prepared);

					PHALCON_MM_CALL_METHOD(NULL, getThis(), "appendmessage", &prepared, &attribute_field, &type);
					error = &PHALCON_GLOBAL(z_true);
				}
			} else if (Z_TYPE(value) != IS_OBJECT || !instanceof_function(Z_OBJCE(value), phalcon_db_rawvalue_ce)) {
				if (phalcon_array_isset(&data_type_numeric, field)) {
					if (!phalcon_is_numeric_ex(&value)) {
						PHALCON_MM_ZVAL_STRING(&type, "Numericality");
						PHALCON_MM_CALL_CE_STATIC(&message, phalcon_validation_ce, "getmessage", &type);
						if (method_exists) {
							PHALCON_MM_CALL_METHOD(&label, getThis(), "getlabel", &attribute_field);
							PHALCON_MM_ADD_ENTRY(&label);
						} else {
							PHALCON_MM_ZVAL_COPY(&label, &attribute_field);
						}
						array_init_size(&pairs, 1);
						phalcon_array_update_str(&pairs, SL(":field"), &label, PH_COPY);
						PHALCON_MM_ADD_ENTRY(&pairs);
						PHALCON_MM_CALL_FUNCTION(&prepared, "strtr", &message, &pairs);
						PHALCON_MM_ADD_ENTRY(&prepared);

						PHALCON_MM_CALL_METHOD(NULL, getThis(), "appendmessage", &prepared, &attribute_field, &type);

						error = &PHALCON_GLOBAL(z_true);
					} else if (
						(phalcon_is_equal_long(&field_type, PHALCON_DB_COLUMN_TYPE_INTEGER) || phalcon_is_equal_long(&field_type, PHALCON_DB_COLUMN_TYPE_BIGINTEGER))
						&&
						!phalcon_is_long_ex(&value)
					) {
						PHALCON_MM_ZVAL_STRING(&type, "Numericality");
						PHALCON_MM_CALL_CE_STATIC(&message, phalcon_validation_ce, "getmessage", &type);
						PHALCON_MM_ADD_ENTRY(&message);
						if (method_exists) {
							PHALCON_MM_CALL_METHOD(&label, getThis(), "getlabel", &attribute_field);
						} else {
							ZVAL_COPY(&label, &attribute_field);
						}
						PHALCON_MM_ADD_ENTRY(&label);
						array_init_size(&pairs, 1);
						phalcon_array_update_str(&pairs, SL(":field"), &label, PH_COPY);
						PHALCON_MM_ADD_ENTRY(&pairs);
						PHALCON_MM_CALL_FUNCTION(&prepared, "strtr", &message, &pairs);
						PHALCON_MM_ADD_ENTRY(&prepared);

						PHALCON_MM_CALL_METHOD(NULL, getThis(), "appendmessage", &prepared, &attribute_field, &type);
						error = &PHALCON_GLOBAL(z_true);
					}
				} else if (phalcon_is_equal_long(&field_type, PHALCON_DB_COLUMN_TYPE_VARCHAR)
					|| phalcon_is_equal_long(&field_type, PHALCON_DB_COLUMN_TYPE_CHAR)) {
					if (!PHALCON_GLOBAL(orm).length_validations) {
						continue;
					}

					PHALCON_MM_CALL_METHOD(&field_size, getThis(), "getdatasize", field);
					if (Z_TYPE(field_size) != IS_NULL) {
						if (PHALCON_GLOBAL(orm).use_mb_strlen) {
							phalcon_strlen(&length, &value);
						} else {
							phalcon_fast_strlen(&length, &value);
						}

						if (phalcon_greater(&length, &field_size)) {
							PHALCON_MM_ZVAL_STRING(&type, "TooLong");
							PHALCON_MM_CALL_CE_STATIC(&message, phalcon_validation_ce, "getmessage", &type);
							PHALCON_MM_ADD_ENTRY(&message);
							if (method_exists) {
								PHALCON_MM_CALL_METHOD(&label, getThis(), "getlabel", &attribute_field);
							} else {
								ZVAL_COPY(&label, &attribute_field);
							}
							PHALCON_MM_ADD_ENTRY(&label);
							array_init_size(&pairs, 2);
							phalcon_array_update_str(&pairs, SL(":field"), &label, PH_COPY);
							phalcon_array_update_str(&pairs, SL(":max"), &field_size, PH_COPY);
							PHALCON_MM_ADD_ENTRY(&pairs);
							PHALCON_MM_CALL_FUNCTION(&prepared, "strtr", &message, &pairs);
							PHALCON_MM_ADD_ENTRY(&prepared);

							PHALCON_MM_CALL_METHOD(NULL, getThis(), "appendmessage", &prepared, &attribute_field, &type);

							error = &PHALCON_GLOBAL(z_true);
						}
					}
				}
			}
		}
	} ZEND_HASH_FOREACH_END();

	if (PHALCON_IS_TRUE(error)) {
		PHALCON_MM_ZVAL_STRING(&event_name, "onValidationFails");
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "_canceloperation");

		RETURN_MM_FALSE;
	}

	/**
	 * Run Validation
	 */
	if (likely(PHALCON_GLOBAL(orm).events)) {
		if (!zend_is_true(exists)) {
			PHALCON_MM_ZVAL_STRING(&event_name, "afterValidationOnCreate");
		} else {
			PHALCON_MM_ZVAL_STRING(&event_name, "afterValidationOnUpdate");
		}

		/**
		 * Run Validation Callbacks After
		 */
		PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_MM_FALSE;
		}
		zval_ptr_dtor(&status);

		PHALCON_MM_ZVAL_STRING(&event_name, "afterValidation");
		PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_MM_FALSE;
		}
		zval_ptr_dtor(&status);

		/**
		 * Run Before Callbacks
		 */
 		PHALCON_MM_ZVAL_STRING(&event_name, "beforeSave");
		PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}
		zval_ptr_dtor(&status);

		if (zend_is_true(exists)) {
			PHALCON_MM_ZVAL_STRING(&event_name, "beforeUpdate");
		} else {
			PHALCON_MM_ZVAL_STRING(&event_name, "beforeCreate");
		}

		phalcon_update_property_bool(getThis(), SL("_skipped"), 0);

		/**
		 * The operation can be skipped here
		 */
		PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_MM_FALSE;
		}
		zval_ptr_dtor(&status);

		/**
		 * Always return true if the operation is skipped
		 */
		phalcon_read_property(&skipped, getThis(), SL("_skipped"), PH_NOISY|PH_READONLY);
		if (PHALCON_IS_TRUE(&skipped)) {
			RETURN_MM_TRUE;
		}
	}

	RETURN_MM_TRUE;
}

/**
 * Executes internal events after save a record
 *
 * @param boolean $success
 * @param boolean $exists
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _postSave){

	zval *success, *exists, event_name = {};

	phalcon_fetch_params(0, 2, 0, &success, &exists);

	if (likely(PHALCON_GLOBAL(orm).events)) {
		if (zend_is_true(success)) {
			if (zend_is_true(exists)) {
				ZVAL_STRING(&event_name, "afterUpdate");
			} else {
				ZVAL_STRING(&event_name, "afterCreate");
			}
			PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
			zval_ptr_dtor(&event_name);

			ZVAL_STRING(&event_name, "afterSave");
			PHALCON_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
			zval_ptr_dtor(&event_name);

			RETURN_TRUE;
		}

		PHALCON_CALL_METHOD(NULL, getThis(), "_canceloperation");
		zval_ptr_dtor(&event_name);
		RETURN_FALSE;
	}

	if (zend_is_true(success)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Sends a pre-build INSERT SQL statement to the relational database system
 *
 * @param Phalcon\Mvc\Model\MetadataInterface $metaData
 * @param Phalcon\Db\AdapterInterface $connection
 * @param string $table
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _doLowInsert){

	zval *connection, *identity_field, read_connection = {}, bind_skip = {}, fields = {}, bind_params = {}, bind_types = {}, exception_message = {};
	zval attributes = {}, bind_data_types = {}, automatic_attributes = {}, not_null_attributes = {}, default_values = {}, data_types = {}, column_map = {};
	zval *field, default_value = {}, use_explicit_identity = {}, column_name = {}, column_value = {}, column_type = {}, phql = {}, model_name = {};
	zval phql_join_fields = {}, phql_join_values = {}, models_manager = {}, query = {}, status = {};
	int identity_field_is_not_false, ignore_last_lnsertid = 0; /* scan-build insists on using flags */

	phalcon_fetch_params(1, 2, 0, &connection, &identity_field);

	ZVAL_LONG(&bind_skip, 1024);

	array_init(&fields);
	PHALCON_MM_ADD_ENTRY(&fields);
	array_init(&bind_params);
	PHALCON_MM_ADD_ENTRY(&bind_params);
	array_init(&bind_types);
	PHALCON_MM_ADD_ENTRY(&bind_types);

	PHALCON_MM_CALL_METHOD(&attributes, getThis(), "getattributes");
	PHALCON_MM_ADD_ENTRY(&attributes);
	PHALCON_MM_CALL_METHOD(&bind_data_types, getThis(), "getbindtypes");
	PHALCON_MM_ADD_ENTRY(&bind_data_types);
	PHALCON_MM_CALL_METHOD(&automatic_attributes, getThis(), "getautomaticcreateattributes");
	PHALCON_MM_ADD_ENTRY(&automatic_attributes);
	PHALCON_MM_CALL_METHOD(&not_null_attributes, getThis(), "getnotnullattributes");
	PHALCON_MM_ADD_ENTRY(&not_null_attributes);
	PHALCON_MM_CALL_METHOD(&default_values, getThis(), "getdefaultvalues");
	PHALCON_MM_ADD_ENTRY(&default_values);
	PHALCON_MM_CALL_METHOD(&data_types, getThis(), "getdatatypes");
	PHALCON_MM_ADD_ENTRY(&data_types);
	PHALCON_MM_CALL_SELF(&column_map, "getcolumnmap");
	PHALCON_MM_ADD_ENTRY(&column_map);

	if (Z_TYPE_P(connection) != IS_OBJECT) {
		/**
		 * Create/Get the current database connection
		 */
		PHALCON_MM_CALL_METHOD(&read_connection, getThis(), "getreadconnection");
		PHALCON_MM_ADD_ENTRY(&read_connection);
	} else {
		ZVAL_COPY_VALUE(&read_connection, connection);
	}

	/**
	 * All fields in the model makes part or the INSERT
	 */
	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(attributes), field) {
		zval attribute_field = {}, value = {}, field_bind_type = {}, field_type = {}, convert_value = {};
		if (!phalcon_array_isset(&automatic_attributes, field)) {
			/**
			 * Check if the model has a column map
			 */
			if (Z_TYPE(column_map) == IS_ARRAY) {
				if (!phalcon_array_isset_fetch(&attribute_field, &column_map, field, PH_READONLY)) {
					PHALCON_CONCAT_SVS(&exception_message, "Column '", field, "' isn't part of the column map");
					PHALCON_MM_ADD_ENTRY(&exception_message);
					PHALCON_MM_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}
			} else {
				ZVAL_COPY_VALUE(&attribute_field, field);
			}

			/**
			 * Check every attribute in the model except identity field
			 */
			if (!PHALCON_IS_EQUAL(field, identity_field)) {
				/**
				 * Every column must have a bind data type defined
				 */
				if (!phalcon_array_isset_fetch(&field_bind_type, &bind_data_types, field, PH_READONLY)) {
					PHALCON_CONCAT_SVS(&exception_message, "Column '", field, "' has not defined a bind data type");
					PHALCON_MM_ADD_ENTRY(&exception_message);
					PHALCON_MM_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}

				/**
				 * This isset checks that the property be defined in the model
				 */
				if (phalcon_isset_property_zval(getThis(), &attribute_field)) {
					phalcon_read_property_zval(&value, getThis(), &attribute_field, PH_READONLY);
				}

				if (Z_TYPE(value) <= IS_NULL) {
					if (PHALCON_GLOBAL(orm).not_null_validations) {
						// Not allow null value and not has default value
						if (!phalcon_fast_in_array(field, &not_null_attributes) && !phalcon_array_isset(&default_values, field)) {
							phalcon_array_append(&fields, &attribute_field, PH_COPY);
							phalcon_array_update(&bind_params, &attribute_field, &PHALCON_GLOBAL(z_null), PH_COPY);
							phalcon_array_update(&bind_types, &attribute_field, &bind_skip, PH_COPY);
						}
					}
				} else {
					ZVAL_COPY(&convert_value, &value);
					PHALCON_MM_ADD_ENTRY(&convert_value);
					if (PHALCON_GLOBAL(orm).enable_auto_convert) {
						if (Z_TYPE(value) != IS_OBJECT || !instanceof_function(Z_OBJCE(value), phalcon_db_rawvalue_ce)) {
							if (phalcon_array_isset_fetch(&field_type, &data_types, field, PH_READONLY) && Z_TYPE(field_type) == IS_LONG) {
								switch(Z_LVAL(field_type)) {
									case PHALCON_DB_COLUMN_TYPE_JSON:
										RETURN_MM_ON_FAILURE(phalcon_json_encode(&convert_value, &value, 0));
										PHALCON_MM_ADD_ENTRY(&convert_value);
										break;
									case PHALCON_DB_COLUMN_TYPE_BYTEA:
										PHALCON_MM_CALL_METHOD(&convert_value, &read_connection, "escapebytea", &value);
										PHALCON_MM_ADD_ENTRY(&convert_value);
										break;
									case PHALCON_DB_COLUMN_TYPE_ARRAY:
									case PHALCON_DB_COLUMN_TYPE_INT_ARRAY:
										PHALCON_MM_CALL_METHOD(&convert_value, &read_connection, "escapearray", &value, &field_type);
										PHALCON_MM_ADD_ENTRY(&convert_value);
										break;
									default:
										break;
								}
							}
						}
					}

					phalcon_array_append(&fields, &attribute_field, PH_COPY);
					phalcon_array_update(&bind_params, &attribute_field, &convert_value, PH_COPY);
					phalcon_array_update(&bind_types, &attribute_field, &field_bind_type, PH_COPY);
				}
			}
		}
	} ZEND_HASH_FOREACH_END();

	/**
	 * If there is an identity field we add it using "null" or "default"
	 */
	identity_field_is_not_false = PHALCON_IS_NOT_EMPTY_STRING(identity_field);
	if (identity_field_is_not_false) {
		/**
		 * Check if the model has a column map
		 */
		if (Z_TYPE(column_map) == IS_ARRAY) {
			if (!phalcon_array_isset_fetch(&column_name, &column_map, identity_field, PH_READONLY)) {
				PHALCON_CONCAT_SVS(&exception_message, "Identity column '", identity_field, "' isn't part of the column map");
				PHALCON_MM_ADD_ENTRY(&exception_message);
				PHALCON_MM_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
				return;
			}
		} else {
			ZVAL_COPY_VALUE(&column_name, identity_field);
		}

		PHALCON_MM_CALL_METHOD(&default_value, &read_connection, "getdefaultidvalue");
		PHALCON_MM_ADD_ENTRY(&default_value);

		/**
		 * Not all the database systems require an explicit value for identity columns
		 */
		PHALCON_MM_CALL_METHOD(&use_explicit_identity, &read_connection, "useexplicitidvalue");

		/**
		 * Check if the developer set an explicit value for the column
		 */
		if (phalcon_property_isset_fetch_zval(&column_value, getThis(), &column_name, PH_READONLY) && PHALCON_IS_NOT_EMPTY(&column_value)) {
			/**
			 * The field is valid we look for a bind value (normally int)
			 */
			if (!phalcon_array_isset_fetch(&column_type, &bind_data_types, identity_field, PH_READONLY)) {
				PHALCON_CONCAT_SVS(&exception_message, "Identity column '", identity_field, "' isn't part of the table columns");
				PHALCON_MM_ADD_ENTRY(&exception_message);
				PHALCON_MM_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
				return;
			}

			if (Z_TYPE(column_value) != IS_OBJECT || !instanceof_function(Z_OBJCE(column_value), phalcon_db_rawvalue_ce)) {
				ignore_last_lnsertid = 1;
			}

			/**
			 * Add the explicit value to the field list if the user has defined a value for it
			 */
			phalcon_array_append(&fields, &column_name, PH_COPY);
			phalcon_array_update(&bind_params, &column_name, &column_value, PH_COPY);
			phalcon_array_update(&bind_types, &column_name, &column_type, PH_COPY);
		} else if (zend_is_true(&use_explicit_identity)) {
			phalcon_array_append(&fields, &column_name, PH_COPY);
			phalcon_array_update(&bind_params, &column_name, &default_value, PH_COPY);
			phalcon_array_update(&bind_types, &column_name, &bind_skip, PH_COPY);
		}
	}

	phalcon_get_called_class(&model_name);
	PHALCON_MM_ADD_ENTRY(&model_name);

	phalcon_fast_join_str(&phql_join_fields, SL(", "), &fields);
	PHALCON_MM_ADD_ENTRY(&phql_join_fields);
	PHALCON_CONCAT_SVSVS(&phql, "INSERT INTO [", &model_name, "] (", &phql_join_fields ,") VALUES ");

	phalcon_fast_join_str(&phql_join_values, SL(":, :"), &fields);
	PHALCON_MM_ADD_ENTRY(&phql_join_values);
	PHALCON_SCONCAT_SVS(&phql, " (:", &phql_join_values, ":) ");
	PHALCON_MM_ADD_ENTRY(&phql);

	PHALCON_MM_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_MM_ADD_ENTRY(&models_manager);
	PHALCON_MM_CALL_METHOD(&query, &models_manager, "createquery", &phql);
	PHALCON_MM_ADD_ENTRY(&query);

	PHALCON_MM_CALL_METHOD(NULL, &query, "setconnection", connection);
	PHALCON_MM_CALL_METHOD(NULL, &query, "setbindparams", &bind_params);
	PHALCON_MM_CALL_METHOD(NULL, &query, "setbindtypes", &bind_types);

	if (ignore_last_lnsertid) {
		PHALCON_MM_CALL_METHOD(NULL, &query, "ignorelastinsertid");
	}

	PHALCON_MM_CALL_METHOD(&status, &query, "execute");
	PHALCON_MM_ADD_ENTRY(&status);

	if (Z_TYPE(status) == IS_OBJECT) {
		PHALCON_MM_CALL_METHOD(return_value, &status, "success");
		if (zend_is_true(return_value) && !ignore_last_lnsertid && identity_field_is_not_false) {
			phalcon_update_property_zval_zval(getThis(), &column_name, return_value);
		}
	} else {
		RETVAL_FALSE;
	}
	RETURN_MM();
}

/**
 * Sends a pre-build UPDATE SQL statement to the relational database system
 *
 * @param Phalcon\Mvc\Model\MetadataInterface $metaData
 * @param Phalcon\Db\AdapterInterface $connection
 * @param string|array $table
 * @return boolean|int
 */
PHP_METHOD(Phalcon_Mvc_Model, _doLowUpdate){

	zval *connection, read_connection = {}, bind_params = {}, bind_types = {}, models_manager = {}, use_dynamic_update = {}, exception_message = {};
	zval snapshot = {}, bind_data_types = {}, automatic_attributes = {}, data_types = {}, column_map = {}, columns = {};
	zval model_name = {}, phql = {}, phql_updates = {}, phql_join_updates = {}, *field, attribute_field = {}, value = {};
	zval unique_key = {}, unique_params = {}, unique_types = {}, merged_params = {}, merged_types = {};
	zval query = {}, status = {}, type = {}, message = {};
	int i_use_dynamic_update; /* To keep static code analyzer happy */

	phalcon_fetch_params(1, 1, 0, &connection);

	if (Z_TYPE_P(connection) != IS_OBJECT) {
		/**
		 * Create/Get the current database connection
		 */
		PHALCON_MM_CALL_METHOD(&read_connection, getThis(), "getreadconnection");
		PHALCON_MM_ADD_ENTRY(&read_connection);
	} else {
		ZVAL_COPY_VALUE(&read_connection, connection);
	}

	PHALCON_MM_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_MM_ADD_ENTRY(&models_manager);

	/**
	 * Check if the model must use dynamic update
	 */
	PHALCON_MM_CALL_METHOD(&use_dynamic_update, &models_manager, "isusingdynamicupdate", getThis());
	i_use_dynamic_update = zend_is_true(&use_dynamic_update);
	if (i_use_dynamic_update) {
		PHALCON_MM_CALL_METHOD(&snapshot, getThis(), "getsnapshotdata");
		PHALCON_MM_ADD_ENTRY(&snapshot);
		if (Z_TYPE(snapshot) != IS_ARRAY) {
			i_use_dynamic_update = 0;
		}
	}

	PHALCON_MM_CALL_METHOD(&bind_data_types, getThis(), "getbindtypes");
	PHALCON_MM_ADD_ENTRY(&bind_data_types);
	PHALCON_MM_CALL_METHOD(&automatic_attributes, getThis(), "getautomaticupdateattributes");
	PHALCON_MM_ADD_ENTRY(&automatic_attributes);
	PHALCON_MM_CALL_METHOD(&data_types, getThis(), "getdatatypes");
	PHALCON_MM_ADD_ENTRY(&data_types);
	PHALCON_MM_CALL_SELF(&column_map, "getcolumnmap");
	PHALCON_MM_ADD_ENTRY(&column_map);

	/**
	 * We only make the update based on the non-primary attributes, values in primary
	 * key attributes are ignored
	 */
	if (PHALCON_GLOBAL(orm).allow_update_primary) {
		PHALCON_MM_CALL_METHOD(&columns, getThis(), "getattributes");
	} else {
		PHALCON_MM_CALL_METHOD(&columns, getThis(), "getnonprimarykeyattributes");
	}
	PHALCON_MM_ADD_ENTRY(&columns);

	array_init(&phql_updates);
	PHALCON_MM_ADD_ENTRY(&phql_updates);
	array_init(&bind_params);
	PHALCON_MM_ADD_ENTRY(&bind_params);
	array_init(&bind_types);
	PHALCON_MM_ADD_ENTRY(&bind_types);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(columns), field) {
		zval field_type = {}, convert_value = {}, bind_type = {}, changed = {}, snapshot_value = {}, phql_update = {};
		if (!phalcon_array_isset(&automatic_attributes, field)) {
			/**
			 * Check a bind type for field to update
			 */
			if (!phalcon_array_isset(&bind_data_types, field)) {
				PHALCON_CONCAT_SVS(&exception_message, "Column '", field, "' have not defined a bind data type");
				PHALCON_MM_ADD_ENTRY(&exception_message);
				PHALCON_MM_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
				return;
			}

			/**
			 * Check if the model has a column map
			 */
			if (Z_TYPE(column_map) == IS_ARRAY) {
				if (!phalcon_array_isset_fetch(&attribute_field, &column_map, field, PH_READONLY)) {
					PHALCON_CONCAT_SVS(&exception_message, "Column '", field, "' isn't part of the column map");
					PHALCON_MM_ADD_ENTRY(&exception_message);
					PHALCON_MM_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}
			} else {
				ZVAL_COPY_VALUE(&attribute_field, field);
			}

			/**
			 * If a field isn't set we pass a null value
			 */
			if (phalcon_property_isset_fetch_zval(&value, getThis(), &attribute_field, PH_READONLY)) {
				ZVAL_COPY(&convert_value, &value);
				PHALCON_MM_ADD_ENTRY(&convert_value);
				if (PHALCON_GLOBAL(orm).enable_auto_convert) {
					if (Z_TYPE(value) != IS_OBJECT || !instanceof_function(Z_OBJCE(value), phalcon_db_rawvalue_ce)) {
						if (phalcon_array_isset_fetch(&field_type, &data_types, field, PH_READONLY) && Z_TYPE(field_type) == IS_LONG) {
							switch(Z_LVAL(field_type)) {
								case PHALCON_DB_COLUMN_TYPE_JSON:
									RETURN_MM_ON_FAILURE(phalcon_json_encode(&convert_value, &value, 0));
									PHALCON_MM_ADD_ENTRY(&convert_value);
									break;
								case PHALCON_DB_COLUMN_TYPE_BYTEA:
									PHALCON_MM_CALL_METHOD(&convert_value, &read_connection, "escapebytea", &value);
									PHALCON_MM_ADD_ENTRY(&convert_value);
									break;
								case PHALCON_DB_COLUMN_TYPE_ARRAY:
								case PHALCON_DB_COLUMN_TYPE_INT_ARRAY:
									PHALCON_MM_CALL_METHOD(&convert_value, &read_connection, "escapearray", &value, &field_type);
									PHALCON_MM_ADD_ENTRY(&convert_value);
									break;
								default:
									break;
							}
						}
					}
				}

				/**
				 * When dynamic update is not used we pass every field to the update
				 */
				if (!i_use_dynamic_update || (Z_TYPE(value) == IS_OBJECT && instanceof_function(Z_OBJCE(value), phalcon_db_rawvalue_ce))) {
					PHALCON_CONCAT_VSVS(&phql_update, &attribute_field, "= :", &attribute_field, ":");
					phalcon_array_append(&phql_updates, &phql_update, 0);

					phalcon_array_update(&bind_params, &attribute_field, &convert_value, PH_COPY);

					phalcon_array_fetch(&bind_type, &bind_data_types, field, PH_NOISY|PH_READONLY);
					phalcon_array_update(&bind_types, &attribute_field, &bind_type, PH_COPY);
				} else {
					/**
					 * If the field is not part of the snapshot we add them as changed
					 */
					if (!phalcon_array_isset_fetch(&snapshot_value, &snapshot, &attribute_field, PH_READONLY)) {
						ZVAL_TRUE(&changed);
					} else {
						if (!PHALCON_IS_EQUAL(&convert_value, &snapshot_value)) {
							ZVAL_TRUE(&changed);
						} else {
							ZVAL_FALSE(&changed);
						}
					}

					/**
					 * Only changed values are added to the SQL Update
					 */
					if (zend_is_true(&changed)) {
						PHALCON_CONCAT_VSVS(&phql_update, &attribute_field, "= :", &attribute_field, ":");
						phalcon_array_append(&phql_updates, &phql_update, 0);

						phalcon_array_update(&bind_params, &attribute_field, &convert_value, PH_COPY);

						phalcon_array_fetch(&bind_type, &bind_data_types, field, PH_NOISY|PH_READONLY);
						phalcon_array_update(&bind_types, &attribute_field, &bind_type, PH_COPY);
					}
				}
			} else {
				PHALCON_CONCAT_VS(&phql_update, &attribute_field, "= NULL");
				phalcon_array_append(&phql_updates, &phql_update, 0);
			}
		}
	} ZEND_HASH_FOREACH_END();

	/**
	 * If there is no fields to update we return true
	 */
	if (!phalcon_fast_count_ev(&phql_updates)) {
		if (PHALCON_GLOBAL(orm).enable_strict) {
			RETURN_MM_FALSE;
		}
		RETURN_MM_TRUE;
	}

	phalcon_fast_join_str(&phql_join_updates, SL(", "), &phql_updates);
	PHALCON_MM_ADD_ENTRY(&phql_join_updates);

	PHALCON_MM_CALL_METHOD(&unique_key, getThis(), "getuniquekey");
	PHALCON_MM_ADD_ENTRY(&unique_key);
	PHALCON_MM_CALL_METHOD(&unique_params, getThis(), "getuniqueparams");
	PHALCON_MM_ADD_ENTRY(&unique_params);
	PHALCON_MM_CALL_METHOD(&unique_types, getThis(), "getuniquetypes");
	PHALCON_MM_ADD_ENTRY(&unique_types);

	if (Z_TYPE(unique_params) == IS_ARRAY) {
		phalcon_add_function(&merged_params, &bind_params, &unique_params);
	} else {
		ZVAL_COPY(&merged_params, &bind_params);
	}
	PHALCON_MM_ADD_ENTRY(&merged_params);

	if (Z_TYPE(unique_types) == IS_ARRAY) {
		phalcon_add_function(&merged_types, &bind_types, &unique_types);
	} else {
		ZVAL_COPY(&merged_types, &bind_types);
	}
	PHALCON_MM_ADD_ENTRY(&merged_types);

	phalcon_get_called_class(&model_name);
	PHALCON_MM_ADD_ENTRY(&model_name);
	PHALCON_CONCAT_SVSVSV(&phql, "UPDATE [", &model_name, "] SET ", &phql_join_updates, " WHERE ", &unique_key);
	PHALCON_MM_ADD_ENTRY(&phql);

	PHALCON_MM_CALL_METHOD(&query, &models_manager, "createquery", &phql);
	PHALCON_MM_ADD_ENTRY(&query);
	PHALCON_MM_CALL_METHOD(NULL, &query, "setconnection", connection);
	PHALCON_MM_CALL_METHOD(NULL, &query, "setbindparams", &merged_params);
	PHALCON_MM_CALL_METHOD(NULL, &query, "setbindtypes", &merged_types);

	PHALCON_MM_CALL_METHOD(&status, &query, "execute");
	PHALCON_MM_ADD_ENTRY(&status);

	if (Z_TYPE(status) == IS_OBJECT) {
		PHALCON_MM_CALL_METHOD(return_value, &status, "success");
	} else {
		RETVAL_FALSE;
	}
	if (!zend_is_true(return_value)) {
		PHALCON_MM_ZVAL_STRING(&type, "InvalidUpdateAttempt");
		PHALCON_MM_ZVAL_STRING(&message, "Record updated fail");

		PHALCON_MM_CALL_METHOD(NULL, getThis(), "appendmessage", &message, &PHALCON_GLOBAL(z_null), &type);
	}
	RETURN_MM();
}

/**
 * Saves related records that must be stored prior to save the master record
 *
 * @param Phalcon\Db\AdapterInterface $connection
 * @param Phalcon\Mvc\ModelInterface[] $related
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _preSaveRelatedRecords){

	zval *connection, *related, *nesting, class_name = {}, manager = {}, *record;
	zend_string *str_key;

	phalcon_fetch_params(0, 2, 0, &connection, &related);

	nesting = &PHALCON_GLOBAL(z_false);

	phalcon_get_class(&class_name, getThis(), 0);

	/**
	 * Start an implicit transaction
	 */
	PHALCON_CALL_METHOD(NULL, connection, "begin", nesting);
	PHALCON_CALL_METHOD(&manager, getThis(), "getmodelsmanager");

	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(related), str_key, record) {
		zval tmp = {}, relation = {}, type = {}, columns = {}, referenced_fields = {}, status = {}, referenced_value = {};
		if (str_key) {
			ZVAL_STR(&tmp, str_key);
			/**
			 * Try to get a relation with the same name
			 */
			PHALCON_CALL_METHOD(&relation, &manager, "getrelationbyalias", &class_name, &tmp);
			if (Z_TYPE(relation) == IS_OBJECT) {
				/**
				 * Get the relation type
				 */
				PHALCON_CALL_METHOD(&type, &relation, "gettype");

				/**
				 * Only belongsTo are stored before save the master record
				 */
				if (PHALCON_IS_LONG(&type, 0)) {
					if (Z_TYPE_P(record) != IS_OBJECT) {
						PHALCON_CALL_METHOD(NULL, connection, "rollback", nesting);
						PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Only objects can be stored as part of belongs-to relations");
						zval_ptr_dtor(&relation);
						return;
					}

					PHALCON_CALL_METHOD(&columns, &relation, "getfields");
					if (Z_TYPE(columns) == IS_ARRAY) {
						PHALCON_CALL_METHOD(NULL, connection, "rollback", nesting);
						PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Not implemented");
						zval_ptr_dtor(&columns);
						zval_ptr_dtor(&relation);
						return;
					}

					/**
					 * If dynamic update is enabled, saving the record must not take any action
					 */
					PHALCON_CALL_METHOD(&status, record, "save");
					if (!zend_is_true(&status)) {
						/**
						 * Get the validation messages generated by the referenced model
						 */
						if (phalcon_mvc_model_get_messages_from_model(getThis(), record, record) == FAILURE) {
							return;
						}

						/**
						 * Rollback the implicit transaction
						 */
						PHALCON_CALL_METHOD(NULL, connection, "rollback", nesting);
						RETURN_FALSE;
					}

					/**
					 * Read the attribute from the referenced model and assigns it to the current model
					 */
					PHALCON_CALL_METHOD(&referenced_fields, &relation, "getreferencedfields");
					PHALCON_CALL_METHOD(&referenced_value, record, "readattribute", &referenced_fields);
					zval_ptr_dtor(&referenced_fields);

					/**
					 * Assign it to the model
					 */
					phalcon_update_property_zval_zval(getThis(), &columns, &referenced_value);
					zval_ptr_dtor(&columns);
					zval_ptr_dtor(&referenced_value);
				}
			}
			zval_ptr_dtor(&relation);
		}
	} ZEND_HASH_FOREACH_END();
	zval_ptr_dtor(&manager);

	RETURN_TRUE;
}

/**
 * Save the related records assigned in the has-one/has-many relations
 *
 * @param Phalcon\Db\AdapterInterface $connection
 * @param Phalcon\Mvc\ModelInterface[] $related
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _postSaveRelatedRecords){

	zval *connection, *related, *nesting, class_name = {}, manager = {}, *record;
	zend_string *str_key;

	phalcon_fetch_params(0, 2, 0, &connection, &related);

	nesting = &PHALCON_GLOBAL(z_false);

	phalcon_get_class(&class_name, getThis(), 0);

	PHALCON_CALL_METHOD(&manager, getThis(), "getmodelsmanager");

	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(related), str_key, record) {
		zval tmp = {}, relation = {}, columns = {}, type = {}, referenced_fields = {}, related_records = {}, value = {}, is_through = {}, intermediate_model_name = {}, exception_message = {};
		zval intermediate_fields = {}, intermediate_referenced_fields = {}, *record_after;
		if (!str_key){
			continue;
		}
		ZVAL_STR(&tmp, str_key);
		/**
		 * Try to get a relation with the same name
		 */
		PHALCON_CALL_METHOD(&relation, &manager, "getrelationbyalias", &class_name, &tmp);
		if (Z_TYPE(relation) != IS_OBJECT && Z_TYPE_P(record) != IS_ARRAY) {
			PHALCON_CALL_METHOD(NULL, connection, "rollback", nesting);

			PHALCON_CONCAT_SVSVS(&exception_message, "There are no defined relations for the model \"", &class_name, "\" using alias \"", &tmp, "\"");
			PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
			zval_ptr_dtor(&relation);
			zval_ptr_dtor(&manager);
			return;
		}

		PHALCON_CALL_METHOD(&type, &relation, "gettype");

		/**
		 * Discard belongsTo relations
		 */
		if (PHALCON_IS_LONG(&type, 0)) {
			zval_ptr_dtor(&relation);
			continue;
		}

		if (Z_TYPE_P(record) != IS_OBJECT && Z_TYPE_P(record) != IS_ARRAY) {
			PHALCON_CALL_METHOD(NULL, connection, "rollback", nesting);
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Only objects/arrays can be stored as part of has-many/has-one/has-many-to-many relations");
			zval_ptr_dtor(&relation);
			zval_ptr_dtor(&manager);
			return;
		}

		PHALCON_CALL_METHOD(&columns, &relation, "getfields");

		if (Z_TYPE(columns) == IS_ARRAY) {
			PHALCON_CALL_METHOD(NULL, connection, "rollback", nesting);
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Not implemented");
			zval_ptr_dtor(&relation);
			zval_ptr_dtor(&manager);
			zval_ptr_dtor(&columns);
			return;
		}
		PHALCON_CALL_METHOD(&referenced_fields, &relation, "getreferencedfields");

		/**
		 * Create an implicit array for has-many/has-one records
		 */
		if (Z_TYPE_P(record) == IS_OBJECT) {
			array_init_size(&related_records, 1);
			phalcon_array_append(&related_records, record, PH_COPY);
		} else {
			ZVAL_COPY(&related_records, record);
		}

		if (!phalcon_isset_property_zval(getThis(), &columns)) {
			PHALCON_CALL_METHOD(NULL, connection, "rollback", nesting);

			PHALCON_CONCAT_SVS(&exception_message, "The column '", &columns, "' needs to be present in the model");
			PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
			zval_ptr_dtor(&columns);
			return;
		}

		/**
		 * Get the value of the field from the current model
		 */
		phalcon_read_property_zval(&value, getThis(), &columns, PH_READONLY);

		/**
		 * Check if the relation is a has-many-to-many
		 */
		PHALCON_CALL_METHOD(&is_through, &relation, "isthrough");

		/**
		 * Get the rest of intermediate model info
		 */
		if (zend_is_true(&is_through)) {
			PHALCON_CALL_METHOD(&intermediate_model_name, &relation, "getintermediatemodel");
			PHALCON_CALL_METHOD(&intermediate_fields, &relation, "getintermediatefields");
			PHALCON_CALL_METHOD(&intermediate_referenced_fields, &relation, "getintermediatereferencedfields");
		}

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(related_records), record_after) {
			zval intermediate_model = {}, intermediate_value = {}, status = {};
			/**
			 * For non has-many-to-many relations just assign the local value in the referenced
			 * model
			 */
			if (!zend_is_true(&is_through)) {
				PHALCON_CALL_METHOD(NULL, record_after, "writeattribute", &referenced_fields, &value);
			}

			/**
			 * Save the record and get messages
			 */
			PHALCON_CALL_METHOD(&status, record_after, "save");
			if (!zend_is_true(&status)) {
				if (zend_is_true(&is_through)) {
					zval_ptr_dtor(&intermediate_model_name);
					zval_ptr_dtor(&intermediate_fields);
					zval_ptr_dtor(&intermediate_referenced_fields);
				}
				/**
				 * Get the validation messages generated by the referenced model
				 */
				if (phalcon_mvc_model_get_messages_from_model(getThis(), record_after, record) == FAILURE) {
					return;
				}

				/**
				 * Rollback the implicit transaction
				 */
				PHALCON_CALL_METHOD(NULL, connection, "rollback", nesting);
				RETURN_FALSE;
			}

			if (zend_is_true(&is_through)) {
				/**
				 * Create a new instance of the intermediate model
				 */
				PHALCON_CALL_METHOD(&intermediate_model, &manager, "load", &intermediate_model_name, &PHALCON_GLOBAL(z_true));

				/**
				 * Write value in the intermediate model
				 */
				PHALCON_CALL_METHOD(NULL, &intermediate_model, "writeattribute", &intermediate_fields, &value);

				/**
				 * Get the value from the referenced model
				 */
				phalcon_read_property_zval(&intermediate_value, record_after, &referenced_fields, PH_READONLY);

				/**
				 * Write the intermediate value in the intermediate model
				 */
				PHALCON_CALL_METHOD(NULL, &intermediate_model, "writeattribute", &intermediate_referenced_fields, &intermediate_value);

				/**
				 * Save the record and get messages
				 */
				PHALCON_CALL_METHOD(&status, &intermediate_model, "save");
				if (!zend_is_true(&status)) {
					zval_ptr_dtor(&intermediate_model_name);
					zval_ptr_dtor(&intermediate_fields);
					zval_ptr_dtor(&intermediate_referenced_fields);
					/**
					 * Get the validation messages generated by the referenced model
					 */
					if (phalcon_mvc_model_get_messages_from_model(getThis(), &intermediate_model, record) == FAILURE) {
						return;
					}

					/**
					 * Rollback the implicit transaction
					 */
					PHALCON_CALL_METHOD(NULL, connection, "rollback", nesting);
					zval_ptr_dtor(&intermediate_model);
					RETURN_FALSE;
				}
				zval_ptr_dtor(&intermediate_model);
			}
		} ZEND_HASH_FOREACH_END();
		zval_ptr_dtor(&columns);
		zval_ptr_dtor(&referenced_fields);
		if (zend_is_true(&is_through)) {
			zval_ptr_dtor(&intermediate_model_name);
			zval_ptr_dtor(&intermediate_fields);
			zval_ptr_dtor(&intermediate_referenced_fields);
		}
	} ZEND_HASH_FOREACH_END();
	zval_ptr_dtor(&manager);

	/**
	 * Commit the implicit transaction
	 */
	PHALCON_CALL_METHOD(NULL, connection, "commit", nesting);
	RETURN_TRUE;
}

/**
 * Inserts or updates a model instance. Returning true on success or false otherwise.
 *
 *<code>
 *	//Creating a new robot
 *	$robot = new Robots();
 *	$robot->type = 'mechanical';
 *	$robot->name = 'Astro Boy';
 *	$robot->year = 1952;
 *	$robot->save();
 *
 *	//Updating a robot name
 *	$robot = Robots::findFirst("id=100");
 *	$robot->name = "Biomass";
 *	$robot->save();
 *</code>
 *
 * @param array $data
 * @param array $whiteList
 * @param array $exists
 * @param array $existsCheck
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, save){

	zval *data = NULL, *white_list = NULL, *_exists = NULL, *exists_check = NULL, exists = {}, attributes = {}, bind_params = {}, *attribute;
	zval type = {}, message = {}, event_name = {}, status = {}, transaction = {}, write_connection = {}, related = {}, identity_field = {};
	zval success = {}, new_success = {};
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(1, 0, 4, &data, &white_list, &_exists, &exists_check);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!white_list) {
		white_list = &PHALCON_GLOBAL(z_null);
	}

	if (_exists) {
		ZVAL_COPY_VALUE(&exists, _exists);
	}

	if (!exists_check) {
		exists_check = &PHALCON_GLOBAL(z_true);
	}

	/**
	 * Get the reversed column map for future renamings
	 */
	PHALCON_MM_CALL_SELF(&attributes, "getcolumnmap");
	if (Z_TYPE(attributes) != IS_ARRAY) {
		PHALCON_MM_CALL_METHOD(&attributes, getThis(), "getattributes");
	}
	PHALCON_MM_ADD_ENTRY(&attributes);

	/**
	 * Assign the values passed
	 */
	if (Z_TYPE_P(data) != IS_NULL) {
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "assign", data, &attributes, white_list);
	}

	array_init(&bind_params);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(attributes), attribute) {
		zval value = {};

		if (phalcon_isset_property_zval(getThis(), attribute)) {
			phalcon_read_property_zval(&value, getThis(), attribute, PH_READONLY);
			phalcon_array_update(&bind_params, attribute, &value, PH_COPY);
		}
	} ZEND_HASH_FOREACH_END();
	PHALCON_MM_ADD_ENTRY(&bind_params);

	phalcon_read_property(&transaction, getThis(), SL("_transaction"), PH_READONLY);
	if (Z_TYPE(transaction) == IS_OBJECT) {
		if (instanceof_function_ex(Z_OBJCE(transaction), phalcon_db_adapterinterface_ce, 1)) {
			PHALCON_MM_ZVAL_COPY(&write_connection, &transaction);
		} else {
			PHALCON_MM_CALL_METHOD(&write_connection, &transaction, "getconnection");
			PHALCON_MM_ADD_ENTRY(&write_connection);
			PHALCON_MM_VERIFY_INTERFACE(&write_connection, phalcon_db_adapterinterface_ce);
		}
	} else {
		ZVAL_NULL(&write_connection);
	}

	if (Z_TYPE(write_connection) != IS_OBJECT) {
		phalcon_read_property(&related, getThis(), SL("_related"), PH_READONLY);
		if (Z_TYPE(related) == IS_ARRAY && phalcon_fast_count_ev(&related)) {
				/**
				 * Create/Get the current database connection
				 */
				PHALCON_MM_CALL_METHOD(&write_connection, getThis(), "getwriteconnection", &PHALCON_GLOBAL(z_null), &bind_params, &PHALCON_GLOBAL(z_null));
				PHALCON_MM_ADD_ENTRY(&write_connection);
		}
	}

	/**
	 * We need to check if the record exists
	 */
	if (!_exists) {
		PHALCON_MM_CALL_METHOD(&exists, getThis(), "exists", &PHALCON_GLOBAL(z_false), &write_connection);
	} else {
		if (zend_is_true(exists_check)) {
			PHALCON_MM_CALL_METHOD(&exists, getThis(), "exists", &PHALCON_GLOBAL(z_false), &write_connection);
			if (!zend_is_true(&exists)) {
				if (zend_is_true(&exists)) {
					PHALCON_MM_ZVAL_STRING(&type, "InvalidCreateAttempt");
					PHALCON_MM_CALL_CE_STATIC(&message, phalcon_validation_ce, "getmessage", &type);
					PHALCON_MM_CALL_METHOD(NULL, getThis(), "appendmessage", &message, &PHALCON_GLOBAL(z_null), &type);
					RETURN_MM_FALSE;
				}
			} else {
				if (!zend_is_true(&exists)) {
					PHALCON_MM_ZVAL_STRING(&type, "InvalidUpdateAttempt");
					PHALCON_MM_CALL_CE_STATIC(&message, phalcon_validation_ce, "getmessage", &type);
					PHALCON_MM_CALL_METHOD(NULL, getThis(), "appendmessage", &message, &PHALCON_GLOBAL(z_null), &type);
					RETURN_MM_FALSE;
				}
			}
		} else {
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "_rebuild");
		}
	}

	if (zend_is_true(&exists)) {
		phalcon_update_property_long(getThis(), SL("_operationMade"), PHALCON_MODEL_OP_UPDATE);
	} else {
		phalcon_update_property_long(getThis(), SL("_operationMade"), PHALCON_MODEL_OP_CREATE);
	}

	PHALCON_MM_ZVAL_STRING(&event_name, "beforeOperation");
	PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);
	if (PHALCON_IS_FALSE(&status)) {
		RETURN_MM_FALSE;
	}

	/**
	 * Save related records in belongsTo relationships
	 */
	if (Z_TYPE(related) == IS_ARRAY && phalcon_fast_count_ev(&related)) {
		PHALCON_MM_CALL_METHOD(&status, getThis(), "_presaverelatedrecords", &write_connection, &related);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_MM_FALSE;
		}
	}

	/**
	 * Clean the messages
	 */
	phalcon_update_property_empty_array(getThis(), SL("_errorMessages"));

	/**
	 * Query the identity field
	 */
	PHALCON_MM_CALL_METHOD(&identity_field, getThis(), "getidentityfield");
	PHALCON_MM_ADD_ENTRY(&identity_field);

	/**
	 * _preSave() makes all the validations
	 */
	PHALCON_MM_CALL_METHOD(&status, getThis(), "_presave", &exists, &identity_field);
	if (PHALCON_IS_FALSE(&status)) {
		/**
		 * Rollback the current transaction if there was validation errors
		 */
		if (Z_TYPE(related) == IS_ARRAY && phalcon_fast_count_ev(&related)) {
			PHALCON_MM_CALL_METHOD(NULL, &write_connection, "rollback", &PHALCON_GLOBAL(z_false));
		}

		/**
		 * Throw exceptions on failed saves?
		 */
		if (unlikely(PHALCON_GLOBAL(orm).exception_on_failed_save)) {
			zval error_messages = {}, exception = {};
			phalcon_read_property(&error_messages, getThis(), SL("_errorMessages"), PH_READONLY);

			/**
			 * Launch a Phalcon\Mvc\Model\ValidationFailed to notify that the save failed
			 */
			object_init_ex(&exception, phalcon_mvc_model_validationfailed_ce);
			PHALCON_MM_ADD_ENTRY(&exception);
			PHALCON_MM_CALL_METHOD(NULL, &exception, "__construct", getThis(), &error_messages);

			phalcon_throw_exception(&exception);
			RETURN_MM();
		}

		RETURN_MM_FALSE;
	}

	/**
	 * Depending if the record exists we do an update or an insert operation
	 */
	if (zend_is_true(&exists)) {
		PHALCON_MM_CALL_METHOD(&success, getThis(), "_dolowupdate", &write_connection);
	} else {
		PHALCON_MM_CALL_METHOD(&success, getThis(), "_dolowinsert", &write_connection, &identity_field);
	}
	PHALCON_MM_ADD_ENTRY(&success);

	/**
	 * _postSave() makes all the validations
	 */
	PHALCON_MM_CALL_METHOD(&new_success, getThis(), "_postsave", &success, &exists);

	if (Z_TYPE(related) == IS_ARRAY && phalcon_fast_count_ev(&related)) {
		/**
		 * Rollbacks the implicit transaction if the master save has failed
		 */
		if (PHALCON_IS_FALSE(&new_success)) {
			PHALCON_MM_CALL_METHOD(NULL, &write_connection, "rollback", &PHALCON_GLOBAL(z_false));

			/**
			 * Throw exceptions on failed saves?
			 */
			if (unlikely(PHALCON_GLOBAL(orm).exception_on_failed_save)) {
				zval error_messages = {}, exception = {};
				phalcon_read_property(&error_messages, getThis(), SL("_errorMessages"), PH_READONLY);

				/**
				 * Launch a Phalcon\Mvc\Model\ValidationFailed to notify that the save failed
				 */
				object_init_ex(&exception, phalcon_mvc_model_validationfailed_ce);
				PHALCON_MM_ADD_ENTRY(&exception);
				PHALCON_MM_CALL_METHOD(NULL, &exception, "__construct", getThis(), &error_messages);

				phalcon_throw_exception(&exception);
				RETURN_MM();
			}
			RETURN_MM_FALSE;
		}

		/**
		 * Save the post-related records
		 */
		if (Z_TYPE(related) == IS_ARRAY && phalcon_fast_count_ev(&related)) {
			PHALCON_MM_CALL_METHOD(&status, getThis(), "_postsaverelatedrecords", &write_connection, &related);
			if (PHALCON_IS_FALSE(&status)) {
				RETURN_MM_FALSE;
			}

			ZEND_HASH_FOREACH_KEY(Z_ARRVAL(related), idx, str_key) {
				zval tmp = {};
				if (str_key) {
					ZVAL_STR(&tmp, str_key);
				} else {
					ZVAL_LONG(&tmp, idx);
				}

				phalcon_unset_property_array(getThis(), SL("_relatedResult"), &tmp);

			} ZEND_HASH_FOREACH_END();
		}
	}

	/**
	 * Change the dirty state to persistent
	 */
	if (zend_is_true(&new_success)) {
		if (!zend_is_true(&exists)) {
			zval default_values = {};
			PHALCON_MM_CALL_METHOD(&default_values, getThis(), "getdefaultvalues");
			PHALCON_MM_ADD_ENTRY(&default_values);

			if (phalcon_fast_count_ev(&default_values)) {
				zval snapshot = {}, *value;
				zend_string *str_key;

				PHALCON_MM_CALL_METHOD(NULL, getThis(), "exists", &PHALCON_GLOBAL(z_true));
				PHALCON_MM_CALL_METHOD(&snapshot, getThis(), "getsnapshotdata");
				PHALCON_MM_ADD_ENTRY(&snapshot);
				if (Z_TYPE(snapshot) == IS_ARRAY) {
					ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL(snapshot), str_key, value) {
						zval key = {};
						if (!str_key) {
							continue;
						}
						ZVAL_STR(&key, str_key);
						if (!phalcon_array_isset(&default_values, &key)) {
							continue;
						}

						if (phalcon_isset_property_zval(getThis(), &key)) {
							zval attribute_value = {};
							phalcon_read_property_zval(&attribute_value, getThis(), &key, PH_READONLY);
							if (Z_TYPE(attribute_value) != IS_NULL) {
								continue;
							}
						}

						phalcon_update_property_zval_zval(getThis(), &key, value);
					} ZEND_HASH_FOREACH_END();
				}
			}
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "_rebuild");
		} else {
			zval snapshot = {};
			PHALCON_MM_CALL_METHOD(&snapshot, getThis(), "getfieldvalues");
			PHALCON_MM_ADD_ENTRY(&snapshot);
			PHALCON_MM_CALL_METHOD(NULL, getThis(), "setsnapshotdata", &snapshot);
			if (PHALCON_GLOBAL(orm).allow_update_primary) {
				PHALCON_MM_CALL_METHOD(NULL, getThis(), "_rebuild");
			}
		}

		phalcon_update_property_long(getThis(), SL("_dirtyState"), PHALCON_MODEL_DIRTY_STATE_PERSISTEN);
		PHALCON_MM_ZVAL_STRING(&event_name, "afterOperation");
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
	} else {
		/**
		 * Throw exceptions on failed saves?
		 */
		if (unlikely(PHALCON_GLOBAL(orm).exception_on_failed_save)) {
			zval error_messages = {}, exception = {};
			phalcon_read_property(&error_messages, getThis(), SL("_errorMessages"), PH_READONLY);

			/**
			 * Launch a Phalcon\Mvc\Model\ValidationFailed to notify that the save failed
			 */
			object_init_ex(&exception, phalcon_mvc_model_validationfailed_ce);
			PHALCON_MM_ADD_ENTRY(&exception);
			PHALCON_MM_CALL_METHOD(NULL, &exception, "__construct", getThis(), &error_messages);

			phalcon_throw_exception(&exception);
			RETURN_MM();
		}
	}

	if (zend_is_true(&new_success)) {
		if (PHALCON_GLOBAL(orm).enable_strict) {
			RETURN_MM_CTOR(&success);
		}
	}
	RETURN_MM_NCTOR(&new_success);
}

/**
 * Inserts a model instance. If the instance already exists in the persistance it will throw an exception
 * Returning true on success or false otherwise.
 *
 *<code>
 *	//Creating a new robot
 *	$robot = new Robots();
 *	$robot->type = 'mechanical';
 *	$robot->name = 'Astro Boy';
 *	$robot->year = 1952;
 *	$robot->create();
 *
 *  //Passing an array to create
 *  $robot = new Robots();
 *  $robot->create(array(
 *      'type' => 'mechanical',
 *      'name' => 'Astroy Boy',
 *      'year' => 1952
 *  ));
 *</code>
 *
 * @param array $data
 * @param array $whiteList
 * @param boolean $existsCheck
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, create){

	zval *data = NULL, *white_list = NULL, *exists_check = NULL;

	phalcon_fetch_params(0, 0, 3, &data, &white_list, &exists_check);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!white_list) {
		white_list = &PHALCON_GLOBAL(z_null);
	}

	if (!exists_check) {
		exists_check = &PHALCON_GLOBAL(z_false);
	}

	/**
	 * Using save() anyways
	 */
	PHALCON_CALL_SELF(return_value, "save", data, white_list, &PHALCON_GLOBAL(z_false), exists_check);
}

/**
 * Updates a model instance. If the instance doesn't exist in the persistance it will throw an exception
 * Returning true on success or false otherwise.
 *
 *<code>
 *	//Updating a robot name
 *	$robot = Robots::findFirst("id=100");
 *	$robot->name = "Biomass";
 *	$robot->update();
 *</code>
 *
 * @param array $data
 * @param array $whiteList
 * @param boolean $existsCheck
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, update){

	zval *data = NULL, *white_list = NULL, *exists_check = NULL;

	phalcon_fetch_params(0, 0, 3, &data, &white_list, &exists_check);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!white_list) {
		white_list = &PHALCON_GLOBAL(z_null);
	}

	if (!exists_check) {
		exists_check = &PHALCON_GLOBAL(z_false);
	}

	/**
	 * Call save() anyways
	 */
	PHALCON_CALL_SELF(return_value, "save", data, white_list, &PHALCON_GLOBAL(z_true), exists_check);
}

/**
 * Deletes a model instance. Returning true on success or false otherwise.
 *
 * <code>
 *$robot = Robots::findFirst("id=100");
 *$robot->delete();
 *
 *foreach (Robots::find("type = 'mechanical'") as $robot) {
 *   $robot->delete();
 *}
 * </code>
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, delete){

	zval event_name = {}, status = {}, check_foreign_keys = {},  write_connection = {}, unique_key = {}, unique_params = {}, unique_types = {};
	zval skipped = {}, model_name = {}, phql = {}, models_manager = {}, query = {}, success = {};

	PHALCON_MM_INIT();

	phalcon_update_property_empty_array(getThis(), SL("_errorMessages"));

	/**
	 * Operation made is OP_DELETE
	 */
	phalcon_update_property_long(getThis(), SL("_operationMade"), PHALCON_MODEL_OP_DELETE);

	PHALCON_MM_ZVAL_STRING(&event_name, "beforeOperation");
	PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);

	if (PHALCON_IS_FALSE(&status)) {
		RETURN_MM_FALSE;
	}

	/**
	 * Check if deleting the record violates a virtual foreign key
	 */
	if (PHALCON_GLOBAL(orm).virtual_foreign_keys) {
		PHALCON_MM_CALL_METHOD(&check_foreign_keys, getThis(), "_checkforeignkeysreverserestrict");
		PHALCON_MM_ADD_ENTRY(&check_foreign_keys);
		if (PHALCON_IS_FALSE(&check_foreign_keys)) {
			RETURN_MM_FALSE;
		}
	}

	PHALCON_MM_CALL_METHOD(&unique_key, getThis(), "getuniquekey");
	PHALCON_MM_ADD_ENTRY(&unique_key);
	PHALCON_MM_CALL_METHOD(&unique_params, getThis(), "getuniqueparams");
	PHALCON_MM_ADD_ENTRY(&unique_params);
	PHALCON_MM_CALL_METHOD(&unique_types, getThis(), "getuniquetypes");
	PHALCON_MM_ADD_ENTRY(&unique_types);

	/**
	 * We can't create dynamic SQL without a primary key
	 */
	if (PHALCON_IS_EMPTY(&unique_key)) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "A primary key must be defined in the model in order to perform the operation");
		return;
	}

	PHALCON_MM_CALL_METHOD(&write_connection, getThis(), "getwriteconnection");
	PHALCON_MM_ADD_ENTRY(&write_connection);

	phalcon_update_property_bool(getThis(), SL("_skipped"), 0);

	/**
	 * Fire the beforeDelete event
	 */
 	PHALCON_MM_ZVAL_STRING(&event_name, "beforeDelete");
	PHALCON_MM_CALL_METHOD(&status, getThis(), "fireeventcancel", &event_name);

	if (PHALCON_IS_FALSE(&status)) {
		RETURN_MM_FALSE;
	} else {
		/**
		 * The operation can be skipped
		 */
		phalcon_read_property(&skipped, getThis(), SL("_skipped"), PH_READONLY);
		if (PHALCON_IS_TRUE(&skipped)) {
			RETURN_MM_TRUE;
		}
	}

	phalcon_get_called_class(&model_name);
	PHALCON_MM_ADD_ENTRY(&model_name);

	PHALCON_CONCAT_SVSV(&phql, "DELETE FROM ", &model_name, " WHERE ", &unique_key);
	PHALCON_MM_ADD_ENTRY(&phql);

	PHALCON_MM_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_MM_ADD_ENTRY(&models_manager);
	PHALCON_MM_CALL_METHOD(&query, &models_manager, "createquery", &phql);
	PHALCON_MM_ADD_ENTRY(&query);
	PHALCON_MM_CALL_METHOD(NULL, &query, "setconnection", &write_connection);
	PHALCON_MM_CALL_METHOD(NULL, &query, "setbindparams", &unique_params);
	PHALCON_MM_CALL_METHOD(NULL, &query, "setbindtypes", &unique_types);

	PHALCON_MM_CALL_METHOD(NULL, &write_connection, "begin", &PHALCON_GLOBAL(z_false));

	PHALCON_MM_CALL_METHOD(&status, &query, "execute");
	PHALCON_MM_ADD_ENTRY(&status);

	ZVAL_FALSE(&success);
	if (Z_TYPE(status) == IS_OBJECT) {
		PHALCON_MM_CALL_METHOD(&success, &status, "success");
	}

	if (!zend_is_true(&success)) {
		/**
		 * Check if there is virtual foreign keys with cascade action
		 */
		if (PHALCON_GLOBAL(orm).virtual_foreign_keys) {
			PHALCON_MM_CALL_METHOD(&check_foreign_keys, getThis(), "_checkforeignkeysreversecascade");
			PHALCON_MM_ADD_ENTRY(&check_foreign_keys);
			if (PHALCON_IS_FALSE(&check_foreign_keys)) {
				PHALCON_MM_CALL_METHOD(NULL, &write_connection, "rollback", &PHALCON_GLOBAL(z_false));
				RETURN_MM_FALSE;
			}
		}
	}

	if (!zend_is_true(&success)) {
		PHALCON_MM_CALL_METHOD(NULL, &write_connection, "rollback", &PHALCON_GLOBAL(z_false));
	} else {
		PHALCON_MM_CALL_METHOD(NULL, &write_connection, "commit", &PHALCON_GLOBAL(z_false));
	}

	/**
	 * Force perform the record existence checking again
	 */
	phalcon_update_property_long(getThis(), SL("_dirtyState"), PHALCON_MODEL_DIRTY_STATE_DETACHED);

	if (zend_is_true(&success)) {
		PHALCON_MM_ZVAL_STRING(&event_name, "afterDelete");
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);

		PHALCON_MM_ZVAL_STRING(&event_name, "afterOperation");
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);
	}

	RETURN_MM_CTOR(&success);
}

/**
 * Returns the type of the latest operation performed by the ORM
 * Returns one of the OP_* class constants
 *
 * @return int
 */
PHP_METHOD(Phalcon_Mvc_Model, getOperationMade){


	RETURN_MEMBER(getThis(), "_operationMade");
}

/**
 * Refreshes the model attributes re-querying the record from the database
 */
PHP_METHOD(Phalcon_Mvc_Model, refresh){

	zval *force = NULL, dirty_state = {};

	phalcon_fetch_params(1, 0, 1, &force);

	if (!force) {
		force = &PHALCON_GLOBAL(z_false);
	}

	PHALCON_MM_CALL_METHOD(&dirty_state, getThis(), "getdirtystate");

	if (!zend_is_true(force) && !PHALCON_IS_LONG(&dirty_state, 0)) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The record cannot be refreshed because it does not exist or is deleted1");
		return;
	}

	PHALCON_MM_CALL_METHOD(NULL, getThis(), "exists", &PHALCON_GLOBAL(z_true));

	if (phalcon_method_exists_ex(return_value, SL("afterrefresh")) == SUCCESS) {
		PHALCON_MM_CALL_METHOD(NULL, return_value, "afterrefresh");
	}

	RETURN_MM_THIS();
}

/**
 * Skips the current operation forcing a success state
 *
 * @param boolean $skip
 */
PHP_METHOD(Phalcon_Mvc_Model, skipOperation){

	zval *skip;

	phalcon_fetch_params(0, 1, 0, &skip);

	phalcon_update_property(getThis(), SL("_skipped"), skip);

}

/**
 * Reads an attribute value by its name
 *
 * <code>
 * echo $robot->readAttribute('name');
 * </code>
 *
 * @param string $attribute
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Model, readAttribute){

	zval *attribute;

	phalcon_fetch_params(0, 1, 0, &attribute);

	phalcon_read_property_zval(return_value, getThis(), attribute, PH_NOISY);
}

/**
 * Writes an attribute value by its name
 *
 * <code>
 * 	$robot->writeAttribute('name', 'Rosey');
 * </code>
 *
 * @param string $attribute
 * @param mixed $value
 */
PHP_METHOD(Phalcon_Mvc_Model, writeAttribute){

	zval *attribute, *value;

	phalcon_fetch_params(0, 2, 0, &attribute, &value);

	phalcon_update_property_zval_zval(getThis(), attribute, value);

}

/**
 * Sets a list of attributes that must be skipped from the
 * generated INSERT/UPDATE statement
 *
 *<code>
 *
 *class Robots extends \Phalcon\Mvc\Model
 *{
 *
 *   public function initialize()
 *   {
 *       $this->skipAttributes(array('price'));
 *   }
 *
 *}
 *</code>
 *
 * @param array $attributes
 * @param boolean $replace
 */
PHP_METHOD(Phalcon_Mvc_Model, skipAttributes){

	zval *attributes, *replace = NULL, reverse_column_map = {}, keys_attributes = {}, *attribute, meta_data = {};

	phalcon_fetch_params(0, 1, 1, &attributes, &replace);

	if (!replace) {
		replace = &PHALCON_GLOBAL(z_false);
	}

	PHALCON_CALL_METHOD(&reverse_column_map, getThis(), "getreversecolumnmap");

	array_init(&keys_attributes);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(attributes), attribute) {
		zval real_attribute = {};
		if (!phalcon_array_isset_fetch(&real_attribute, &reverse_column_map, attribute, PH_NOISY|PH_READONLY)) {
			ZVAL_COPY_VALUE(&real_attribute, attribute);
		}
		phalcon_array_update(&keys_attributes, &real_attribute, &PHALCON_GLOBAL(z_null), PH_COPY);
	} ZEND_HASH_FOREACH_END();

	zval_ptr_dtor(&reverse_column_map);

	PHALCON_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_CALL_METHOD(NULL, &meta_data, "setautomaticcreateattributes", getThis(), &keys_attributes, replace);
	PHALCON_CALL_METHOD(NULL, &meta_data, "setautomaticupdateattributes", getThis(), &keys_attributes, replace);
	zval_ptr_dtor(&meta_data);
}

/**
 * Sets a list of attributes that must be skipped from the
 * generated INSERT statement
 *
 *<code>
 *
 *class Robots extends \Phalcon\Mvc\Model
 *{
 *
 *   public function initialize()
 *   {
 *       $this->skipAttributesOnCreate(array('created_at'));
 *   }
 *
 *}
 *</code>
 *
 * @param array $attributes
 * @param boolean $replace
 */
PHP_METHOD(Phalcon_Mvc_Model, skipAttributesOnCreate){

	zval *attributes, *replace = NULL, reverse_column_map = {}, keys_attributes = {}, *attribute, meta_data = {};

	phalcon_fetch_params(0, 1, 1, &attributes, &replace);

	if (!replace) {
		replace = &PHALCON_GLOBAL(z_false);
	}

	PHALCON_CALL_METHOD(&reverse_column_map, getThis(), "getreversecolumnmap");

	array_init(&keys_attributes);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(attributes), attribute) {
		zval real_attribute = {};
		if (!phalcon_array_isset_fetch(&real_attribute, &reverse_column_map, attribute, PH_NOISY|PH_READONLY)) {
			ZVAL_COPY_VALUE(&real_attribute, attribute);
		}
		phalcon_array_update(&keys_attributes, &real_attribute, &PHALCON_GLOBAL(z_null), PH_COPY);
	} ZEND_HASH_FOREACH_END();

	zval_ptr_dtor(&reverse_column_map);

	PHALCON_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_CALL_METHOD(NULL, &meta_data, "setautomaticcreateattributes", getThis(), &keys_attributes, replace);
	zval_ptr_dtor(&meta_data);
}

/**
 * Returns attributes that must be ignored from the INSERT SQL generation
 *
 *<code>
 * $robot = Robots::findFirst();
 * print_r($robot->getSkipAttributesOnCreate());
 *</code>
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getSkipAttributesOnCreate){

	zval meta_data = {};

	PHALCON_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_CALL_METHOD(return_value, &meta_data, "getautomaticcreateattributes", getThis());
	zval_ptr_dtor(&meta_data);
}

/**
 * Sets a list of attributes that must be skipped from the
 * generated UPDATE statement
 *
 *<code>
 *
 *class Robots extends \Phalcon\Mvc\Model
 *{
 *
 *   public function initialize()
 *   {
 *       $this->skipAttributesOnUpdate(array('modified_in'));
 *   }
 *
 *}
 *</code>
 *
 * @param array $attributes
 * @param boolean $replace
 */
PHP_METHOD(Phalcon_Mvc_Model, skipAttributesOnUpdate){

	zval *attributes, *replace = NULL, reverse_column_map = {}, keys_attributes = {}, *attribute, meta_data = {};

	phalcon_fetch_params(0, 1, 1, &attributes, &replace);

	if (!replace) {
		replace = &PHALCON_GLOBAL(z_false);
	}

	PHALCON_CALL_METHOD(&reverse_column_map, getThis(), "getreversecolumnmap");

	array_init(&keys_attributes);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(attributes), attribute) {
		zval real_attribute = {};
		if (!phalcon_array_isset_fetch(&real_attribute, &reverse_column_map, attribute, PH_NOISY|PH_READONLY)) {
			ZVAL_COPY_VALUE(&real_attribute, attribute);
		}
		phalcon_array_update(&keys_attributes, &real_attribute, &PHALCON_GLOBAL(z_null), PH_COPY);
	} ZEND_HASH_FOREACH_END();

	zval_ptr_dtor(&reverse_column_map);

	PHALCON_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_CALL_METHOD(NULL, &meta_data, "setautomaticupdateattributes", getThis(), &keys_attributes, replace);
	zval_ptr_dtor(&meta_data);
}

/**
 * Returns attributes that must be ignored from the UPDATE SQL generation
 *
 *<code>
 * $robot = Robots::findFirst();
 * print_r($robot->getSkipAttributesOnUpdate());
 *</code>
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getSkipAttributesOnUpdate){

	zval meta_data = {};

	PHALCON_CALL_METHOD(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_CALL_METHOD(return_value, &meta_data, "getautomaticupdateattributes", getThis());
	zval_ptr_dtor(&meta_data);
}

/**
 * Setup a 1-1 relation between two models
 *
 *<code>
 *
 *class Robots extends \Phalcon\Mvc\Model
 *{
 *
 *   public function initialize()
 *   {
 *       $this->hasOne('id', 'RobotsDescription', 'robots_id');
 *   }
 *
 *}
 *</code>
 *
 * @param mixed $fields
 * @param string $referenceModel
 * @param mixed $referencedFields
 * @param   array $options
 * @return  Phalcon\Mvc\Model\Relation
 */
PHP_METHOD(Phalcon_Mvc_Model, hasOne){

	zval *fields, *reference_model, *referenced_fields, *options = NULL, models_manager = {};

	phalcon_fetch_params(0, 3, 1, &fields, &reference_model, &referenced_fields, &options);

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_CALL_METHOD(return_value, &models_manager, "addhasone", getThis(), fields, reference_model, referenced_fields, options);
	zval_ptr_dtor(&models_manager);
}

/**
 * Setup a relation reverse 1-1  between two models
 *
 *<code>
 *
 *class RobotsParts extends \Phalcon\Mvc\Model
 *{
 *
 *   public function initialize()
 *   {
 *       $this->belongsTo('robots_id', 'Robots', 'id');
 *   }
 *
 *}
 *</code>
 *
 * @param mixed $fields
 * @param string $referenceModel
 * @param mixed $referencedFields
 * @param   array $options
 * @return  Phalcon\Mvc\Model\Relation
 */
PHP_METHOD(Phalcon_Mvc_Model, belongsTo){

	zval *fields, *reference_model, *referenced_fields, *options = NULL, models_manager = {};

	phalcon_fetch_params(0, 3, 1, &fields, &reference_model, &referenced_fields, &options);

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_CALL_METHOD(return_value, &models_manager, "addbelongsto", getThis(), fields, reference_model, referenced_fields, options);
	zval_ptr_dtor(&models_manager);
}

/**
 * Setup a relation 1-n between two models
 *
 *<code>
 *
 *class Robots extends \Phalcon\Mvc\Model
 *{
 *
 *   public function initialize()
 *   {
 *       $this->hasMany('id', 'RobotsParts', 'robots_id');
 *   }
 *
 *}
 *</code>
 *
 * @param mixed $fields
 * @param string $referenceModel
 * @param mixed $referencedFields
 * @param   array $options
 * @return  Phalcon\Mvc\Model\Relation
 */
PHP_METHOD(Phalcon_Mvc_Model, hasMany){

	zval *fields, *reference_model, *referenced_fields, *options = NULL, models_manager = {};

	phalcon_fetch_params(0, 3, 1, &fields, &reference_model, &referenced_fields, &options);

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_CALL_METHOD(return_value, &models_manager, "addhasmany", getThis(), fields, reference_model, referenced_fields, options);
	zval_ptr_dtor(&models_manager);
}

/**
 * Setup a relation n-n between two models through an intermediate relation
 *
 *<code>
 *
 *class Robots extends \Phalcon\Mvc\Model
 *{
 *
 *   public function initialize()
 *   {
 *       //Setup a many-to-many relation to Parts through RobotsParts
 *       $this->hasManyToMany(
 *			'id',
 *			'RobotsParts',
 *			'robots_id',
 *			'parts_id',
 *			'Parts',
 *			'id'
 *		);
 *   }
 *
 *}
 *</code>
 *
 * @param string $fields
 * @param string $intermediateModel
 * @param string $intermediateFields
 * @param string $intermediateReferencedFields
 * @param string $referencedModel
 * @param   string $referencedFields
 * @param   array $options
 * @return  Phalcon\Mvc\Model\Relation
 */
PHP_METHOD(Phalcon_Mvc_Model, hasManyToMany){

	zval *fields, *intermediate_model, *intermediate_fields, *intermediate_referenced_fields, *reference_model;
	zval *referenced_fields, *options = NULL, models_manager = {};

	phalcon_fetch_params(0, 6, 1, &fields, &intermediate_model, &intermediate_fields, &intermediate_referenced_fields, &reference_model, &referenced_fields, &options);

	PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");

	PHALCON_CALL_METHOD(return_value, &models_manager, "addhasmanytomany", getThis(), fields, intermediate_model, intermediate_fields, intermediate_referenced_fields, reference_model, referenced_fields, options ? options : &PHALCON_GLOBAL(z_null));
	zval_ptr_dtor(&models_manager);
}

/**
 * Setups a behavior in a model
 *
 *<code>
 *
 *use Phalcon\Mvc\Model\Behavior\Timestampable;
 *
 *class Robots extends \Phalcon\Mvc\Model
 *{
 *
 *   public function initialize()
 *   {
 *		$this->addBehavior(new Timestampable(array(
 *			'onCreate' => array(
 *				'field' => 'created_at',
 *				'format' => 'Y-m-d'
 *			)
 *		)));
 *   }
 *
 *}
 *</code>
 *
 * @param Phalcon\Mvc\Model\BehaviorInterface $behavior
 */
PHP_METHOD(Phalcon_Mvc_Model, addBehavior){

	zval *behavior, models_manager = {};

	phalcon_fetch_params(0, 1, 0, &behavior);

	PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_CALL_METHOD(NULL, &models_manager, "addbehavior", getThis(), behavior);
	zval_ptr_dtor(&models_manager);
}

/**
 * Sets the record's snapshot data.
 * This method is used internally to set snapshot data when the model was set up to keep snapshot data
 *
 * @param array $data
 * @param array $columnMap
 */
PHP_METHOD(Phalcon_Mvc_Model, setSnapshotData){

	zval *data, *column_map = NULL, snapshot = {}, *value, exception_message = {};
	zend_string *str_key;

	phalcon_fetch_params(1, 1, 1, &data, &column_map);

	if (!column_map) {
		column_map = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(data) != IS_ARRAY) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The snapshot data must be an array");
		return;
	}

	/**
	 * Build the snapshot based on a column map
	 */
	if (Z_TYPE_P(column_map) == IS_ARRAY) {
		array_init(&snapshot);
		PHALCON_MM_ADD_ENTRY(&snapshot);
		ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(data), str_key, value) {
			zval key = {}, attribute = {};
			if (str_key) {
				ZVAL_STR(&key, str_key);

				/**
				 * Every field must be part of the column map
				 */
				if (!phalcon_array_isset(column_map, &key)) {
					PHALCON_CONCAT_SVS(&exception_message, "Column \"", &key, "\" doesn't make part of the column map");
					PHALCON_MM_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}

				phalcon_array_fetch(&attribute, column_map, &key, PH_NOISY|PH_READONLY);
				phalcon_array_update(&snapshot, &attribute, value, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();

		phalcon_update_property(getThis(), SL("_snapshot"), &snapshot);
	} else {
		phalcon_update_property(getThis(), SL("_snapshot"), data);
	}
	RETURN_MM();
}

/**
 * Checks if the object has internal snapshot data
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, hasSnapshotData){

	zval snapshot = {};

	phalcon_read_property(&snapshot, getThis(), SL("_snapshot"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(snapshot) == IS_ARRAY) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Returns the internal snapshot data
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getSnapshotData){


	RETURN_MEMBER(getThis(), "_snapshot");
}

/**
 * Check if a specific attribute has changed
 * This only works if the model is keeping data snapshots
 *
 * @param string $fieldName
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, hasChanged){

	zval *field_name = NULL, snapshot = {}, dirty_state = {}, column_map = {}, attributes = {}, all_attributes = {}, exception_message = {}, attribute_value = {}, original_value = {};
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 0, 1, &field_name);

	if (!field_name) {
		field_name = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&snapshot, getThis(), SL("_snapshot"), PH_READONLY);
	if (Z_TYPE(snapshot) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The record doesn't have a valid data snapshot");
		return;
	}

	if (Z_TYPE_P(field_name) != IS_STRING) {
		if (Z_TYPE_P(field_name) != IS_NULL) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The field name must be string");
			return;
		}
	}

	phalcon_read_property(&dirty_state, getThis(), SL("_dirtyState"), PH_NOISY|PH_READONLY);

	/**
	 * Dirty state must be DIRTY_PERSISTENT to make the checking
	 */
	if (!PHALCON_IS_LONG(&dirty_state, 0)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Change checking cannot be performed because the object has not been persisted or is deleted");
		return;
	}

	/**
	 * The reversed column map is an array if the model has a column map
	 */
	PHALCON_CALL_METHOD(&column_map, getThis(), "getreversecolumnmap");

	/**
	 * Data types are field indexed
	 */
	if (Z_TYPE(column_map) != IS_ARRAY) {
		PHALCON_CALL_METHOD(&attributes, getThis(), "getdatatypes");
		ZVAL_COPY_VALUE(&all_attributes, &attributes);
	} else {
		ZVAL_COPY_VALUE(&all_attributes, &column_map);
	}

	/**
	 * If a field was specified we only check it
	 */
	if (Z_TYPE_P(field_name) == IS_STRING) {
		/**
		 * We only make this validation over valid fields
		 */
		if (Z_TYPE(column_map) == IS_ARRAY) {
			if (!phalcon_array_isset(&column_map, field_name)) {
				PHALCON_CONCAT_SVS(&exception_message, "The field '", field_name, "' is not part of the model");
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
				return;
			}
		} else if (!phalcon_array_isset(&attributes, field_name)) {
			PHALCON_CONCAT_SVS(&exception_message, "The field '", field_name, "' is not part of the model");
			PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
			return;
		}

		/**
		 * The field is not part of the model, throw exception
		 */
		if (!phalcon_isset_property_zval(getThis(), field_name)) {
			PHALCON_CONCAT_SVS(&exception_message, "The field '", field_name, "' is not defined on the model");
			PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
			return;
		}

		/**
		 * The field is not part of the data snapshot, throw exception
		 */
		if (!phalcon_array_isset(&snapshot, field_name)) {
			PHALCON_CONCAT_SVS(&exception_message, "The field '", field_name, "' was not found in the snapshot");
			PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
			return;
		}

		phalcon_read_property_zval(&attribute_value, getThis(), field_name, PH_READONLY);

		phalcon_array_fetch(&original_value, &snapshot, field_name, PH_NOISY|PH_READONLY);

		/**
		 * Check if the field has changed
		 */
		if (PHALCON_IS_EQUAL(&attribute_value, &original_value)) {
			RETURN_FALSE;
		} else {
			RETURN_TRUE;
		}
	}

	/**
	 * Check every attribute in the model
	 */
	ZEND_HASH_FOREACH_KEY(Z_ARRVAL(all_attributes), idx, str_key) {
		zval name = {};
		if (str_key) {
			ZVAL_STR(&name, str_key);
		} else {
			ZVAL_LONG(&name, idx);
		}

		/**
		 * If some attribute is not present in the snapshot, we assume the record as
		 * changed
		 */
		if (!phalcon_array_isset(&snapshot, &name)) {
			RETURN_TRUE;
		}

		/**
		 * If some attribute is not present in the model, we assume the record as changed
		 */
		if (!phalcon_property_isset_fetch_zval(&attribute_value, getThis(), &name, PH_READONLY)) {
			RETURN_TRUE;
		}

		phalcon_read_property_zval(&attribute_value, getThis(), &name, PH_READONLY);

		phalcon_array_fetch(&original_value, &snapshot, &name, PH_NOISY|PH_READONLY);

		/**
		 * Check if the field has changed
		 */
		if (!PHALCON_IS_EQUAL(&attribute_value, &original_value)) {
			RETURN_TRUE;
		}
	} ZEND_HASH_FOREACH_END();
	zval_ptr_dtor(&all_attributes);

	RETURN_FALSE;
}

/**
 * Returns a list of changed values
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getChangedFields){

	zval snapshot = {}, dirty_state = {}, column_map = {}, attributes = {}, all_attributes = {}, changed = {}, attribute_value = {}, original_value = {};
	zend_string *str_key;
	ulong idx;

	PHALCON_MM_INIT();

	array_init(&changed);
	PHALCON_MM_ADD_ENTRY(&changed);

	phalcon_read_property(&snapshot, getThis(), SL("_snapshot"), PH_READONLY);
	if (Z_TYPE(snapshot) != IS_ARRAY) {
		RETURN_MM_CTOR(&changed);
	}

	phalcon_read_property(&dirty_state, getThis(), SL("_dirtyState"), PH_READONLY);

	/**
	 * Dirty state must be DIRTY_PERSISTENT to make the checking
	 */
	if (!PHALCON_IS_LONG(&dirty_state, 0)) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Change checking cannot be performed because the object has not been persisted or is deleted");
		return;
	}

	/**
	 * The reversed column map is an array if the model has a column map
	 */
	PHALCON_MM_CALL_METHOD(&column_map, getThis(), "getreversecolumnmap");
	PHALCON_MM_ADD_ENTRY(&column_map);

	/**
	 * Data types are field indexed
	 */
	if (Z_TYPE(column_map) != IS_ARRAY) {
		PHALCON_MM_CALL_METHOD(&attributes, getThis(), "getdatatypes");
		PHALCON_MM_ADD_ENTRY(&attributes);
		ZVAL_COPY_VALUE(&all_attributes, &attributes);
	} else {
		ZVAL_COPY_VALUE(&all_attributes, &column_map);
	}

	/**
	 * Check every attribute in the model
	 */
	ZEND_HASH_FOREACH_KEY(Z_ARRVAL(all_attributes), idx, str_key) {
		zval tmp = {};
		if (str_key) {
			ZVAL_STR(&tmp, str_key);
		} else {
			ZVAL_LONG(&tmp, idx);
		}

		/**
		 * If some attribute is not present in the snapshot, we assume the record as
		 * changed
		 */
		if (!phalcon_array_isset(&snapshot, &tmp)) {
			phalcon_array_append(&changed, &tmp, PH_COPY);
			continue;
		}

		/**
		 * If some attribute is not present in the model, we assume the record as changed
		 */
		if (!phalcon_property_isset_fetch_zval(&attribute_value, getThis(), &tmp, PH_READONLY)) {
			phalcon_array_append(&changed, &tmp, PH_COPY);
			continue;
		}

		phalcon_array_fetch(&original_value, &snapshot, &tmp, PH_NOISY|PH_READONLY);

		/**
		 * Check if the field has changed
		 */
		if (!PHALCON_IS_EQUAL(&attribute_value, &original_value)) {
			phalcon_array_append(&changed, &tmp, PH_COPY);
			continue;
		}
	} ZEND_HASH_FOREACH_END();

	RETURN_MM_CTOR(&changed);
}

/**
 * Sets if a model must use dynamic update instead of the all-field update
 *
 *<code>
 *
 *class Robots extends \Phalcon\Mvc\Model
 *{
 *
 *   public function initialize()
 *   {
 *		$this->useDynamicUpdate(true);
 *   }
 *
 *}
 *</code>
 *
 * @param boolean $dynamicUpdate
 */
PHP_METHOD(Phalcon_Mvc_Model, useDynamicUpdate){

	zval *dynamic_update, models_manager = {};

	phalcon_fetch_params(0, 1, 0, &dynamic_update);

	PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_CALL_METHOD(NULL, &models_manager, "usedynamicupdate", getThis(), dynamic_update);
	zval_ptr_dtor(&models_manager);
}

/**
 * Returns related records based on defined relations
 *
 * @param string $alias
 * @param array $arguments
 * @return Phalcon\Mvc\Model\ResultsetInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, getRelated){

	zval *alias, *arguments = NULL, models_manager = {}, class_name = {}, relation = {}, exception_message = {}, call_object = {}, model_args = {};

	phalcon_fetch_params(0, 1, 1, &alias, &arguments);

	if (!arguments) {
		arguments = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");

	phalcon_get_class(&class_name, getThis(), 0);

	/**
	 * Query the relation by alias
	 */
	PHALCON_CALL_METHOD(&relation, &models_manager, "getrelationbyalias", &class_name, alias);
	if (Z_TYPE(relation) != IS_OBJECT) {
		PHALCON_CONCAT_SVSVS(&exception_message, "There is no defined relations for the model \"", &class_name, "\" using alias \"", alias, "\"");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
		zval_ptr_dtor(&models_manager);
		zval_ptr_dtor(&class_name);
		return;
	}
	zval_ptr_dtor(&class_name);

	/**
	 * Call the 'getRelationRecords' in the models manager
	 */
	array_init_size(&call_object, 2);
	phalcon_array_append(&call_object, &models_manager, 0);
	add_next_index_stringl(&call_object, SL("getRelationRecords"));

	array_init_size(&model_args, 4);
	phalcon_array_append(&model_args, &relation, 0);
	add_next_index_null(&model_args);
	phalcon_array_append(&model_args, getThis(), PH_COPY);
	phalcon_array_append(&model_args, arguments, PH_COPY);

	PHALCON_CALL_USER_FUNC_ARRAY(return_value, &call_object, &model_args);
	zval_ptr_dtor(&call_object);
	zval_ptr_dtor(&model_args);
}

/**
 * Returns related records defined relations depending on the method name
 *
 * @param string $modelName
 * @param string $method
 * @param array $arguments
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Model, _getRelatedRecords){

	zval *model_name, *method, *arguments, models_manager = {}, alias = {}, relation = {}, query_method = {}, extra_args = {}, call_args = {}, call_object = {};

	phalcon_fetch_params(0, 3, 0, &model_name, &method, &arguments);

	PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");

	ZVAL_NULL(&query_method);
	/**
	 * Calling find/findFirst if the method starts with "get"
	 */
	if (phalcon_start_with_str(method, SL("get"))) {
		phalcon_substr(&alias, method, 3, 0);
		PHALCON_CALL_METHOD(&relation, &models_manager, "getrelationbyalias", model_name, &alias);
		zval_ptr_dtor(&alias);
	}

	/**
	 * Calling count if the method starts with "count"
	 */
	if (Z_TYPE(relation) != IS_OBJECT) {
		if (phalcon_start_with_str(method, SL("count"))) {
			ZVAL_STRING(&query_method, "count");
			phalcon_substr(&alias, method, 5, 0);
			PHALCON_CALL_METHOD(&relation, &models_manager, "getrelationbyalias", model_name, &alias);
			zval_ptr_dtor(&alias);
		}
	}

	/**
	 * If the relation was found perform the query via the models manager
	 */
	if (Z_TYPE(relation) == IS_OBJECT) {
		if (Z_TYPE_P(arguments) != IS_ARRAY || !phalcon_array_isset_fetch_long(&extra_args, arguments, 0, PH_READONLY)) {
			ZVAL_NULL(&extra_args);
		}

		array_init_size(&call_args, 4);
		phalcon_array_append(&call_args, &relation, 0);
		phalcon_array_append(&call_args, &query_method, 0);
		phalcon_array_append(&call_args, getThis(), PH_COPY);
		phalcon_array_append(&call_args, &extra_args, PH_COPY);

		array_init_size(&call_object, 2);
		phalcon_array_append(&call_object, &models_manager, 0);
		add_next_index_stringl(&call_object, SL("getRelationRecords"));
		PHALCON_CALL_USER_FUNC_ARRAY(return_value, &call_object, &call_args);
		zval_ptr_dtor(&call_args);
		zval_ptr_dtor(&call_object);
		return;
	}
	zval_ptr_dtor(&models_manager);
	zval_ptr_dtor(&query_method);
	zval_ptr_dtor(&relation);

	RETURN_NULL();
}

/**
 * Handles method calls when a method is not implemented
 *
 * @param string $method
 * @param array $arguments
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Model, __call){

	zval *method, *arguments = NULL, model_name = {}, records = {}, models_manager = {}, status = {}, exception_message = {};

	phalcon_fetch_params(0, 1, 1, &method, &arguments);

	if (!arguments) {
		arguments = &PHALCON_GLOBAL(z_null);
	}

	phalcon_get_class(&model_name, getThis(), 0);

	/**
	 * Check if there is a default action using the magic getter
	 */
	PHALCON_CALL_METHOD(&records, getThis(), "_getrelatedrecords", &model_name, method, arguments);
	if (Z_TYPE(records) != IS_NULL) {
		RETVAL_ZVAL(&records, 0, 0);
		zval_ptr_dtor(&model_name);
		return;
	}

	PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");

	/**
	 * Try to find a replacement for the missing method in a behavior/listener
	 */
	PHALCON_CALL_METHOD(&status, &models_manager, "missingmethod", getThis(), method, arguments);
	zval_ptr_dtor(&models_manager);
	if (Z_TYPE(status) != IS_NULL) {
		zval_ptr_dtor(&model_name);
		RETURN_CTOR(&status);
	}

	/**
	 * The method doesn't exist throw an exception
	 */
	PHALCON_CONCAT_SVSVS(&exception_message, "The method \"", method, "\" doesn't exist on model \"", &model_name, "\"");
	zval_ptr_dtor(&model_name);
	PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
}

/**
 * Handles method calls when a static method is not implemented
 *
 * @param string $method
 * @param array $arguments
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Model, __callStatic){

	zval *method, *arguments = NULL, extra_method = {}, model_name = {}, exception_message = {}, value = {}, model = {}, attributes = {};
	zval field = {}, extra_method_first = {},  conditions = {}, bind_params = {}, parameters = {};
	zend_class_entry *ce0;
	const char *type = NULL;

	phalcon_fetch_params(0, 1, 1, &method, &arguments);

	if (!arguments) {
		arguments = &PHALCON_GLOBAL(z_null);
	}

	/**
	 * Check if the method starts with 'findFirst'
	 */
	if (phalcon_start_with_str(method, SL("findFirstBy"))) {
		type = "findfirst";
		phalcon_substr(&extra_method, method, 11, 0);
	}

	/**
	 * Check if the method starts with 'find'
	 */
	if (Z_TYPE(extra_method) <= IS_NULL) {
		if (phalcon_start_with_str(method, SL("findBy"))) {
			type = "find";
			phalcon_substr(&extra_method, method, 6, 0);
		}
	}

	/**
	 * Check if the method starts with 'count'
	 */
	if (Z_TYPE(extra_method) <= IS_NULL) {
		if (phalcon_start_with_str(method, SL("countBy"))) {
			type = "count";
			phalcon_substr(&extra_method, method, 7, 0);
		}
	}

	/**
	 * The called class is the model
	 */
	phalcon_get_called_class(&model_name);
	if (!type) {
		/**
		 * The method doesn't exist throw an exception
		 */
		PHALCON_CONCAT_SVSVS(&exception_message, "The static method \"", method, "\" doesn't exist on model \"", &model_name, "\"");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
		zval_ptr_dtor(&extra_method);
		zval_ptr_dtor(&model_name);
		zval_ptr_dtor(&exception_message);
		return;
	}

	if (!phalcon_array_isset_long(arguments, 0)) {
		PHALCON_CONCAT_SVS(&exception_message, "The static method \"", method, "\" requires one argument");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
		zval_ptr_dtor(&extra_method);
		zval_ptr_dtor(&model_name);
		zval_ptr_dtor(&exception_message);
		return;
	}

	phalcon_array_fetch_long(&value, arguments, 0, PH_NOISY|PH_READONLY);
	ce0 = phalcon_fetch_class(&model_name, ZEND_FETCH_CLASS_DEFAULT);
	zval_ptr_dtor(&model_name);

	PHALCON_OBJECT_INIT(&model, ce0);
	if (phalcon_has_constructor(&model)) {
		PHALCON_CALL_METHOD(NULL, &model, "__construct");
	}

	/**
	 * Get the attributes
	 */
	PHALCON_CALL_METHOD(&attributes, &model, "getreversecolumnmap");
	if (Z_TYPE(attributes) != IS_ARRAY) {
		PHALCON_CALL_METHOD(&attributes, &model, "getdatatypes");
	}
	zval_ptr_dtor(&model);

	/**
	 * Check if the extra-method is an attribute
	 */
	if (phalcon_array_isset(&attributes, &extra_method)) {
		ZVAL_COPY(&field, &extra_method);
	} else {
		/**
		 * Lowercase the first letter of the extra-method
		 */
		phalcon_lcfirst(&extra_method_first, &extra_method);
		if (phalcon_array_isset(&attributes, &extra_method_first)) {
			ZVAL_COPY(&field, &extra_method_first);
		} else {
			/**
			 * Get the possible real method name
			 */
			phalcon_uncamelize(&field, &extra_method);
			if (!phalcon_array_isset(&attributes, &field)) {
				PHALCON_CONCAT_SVS(&exception_message, "Cannot resolve attribute \"", &extra_method, "' in the model");
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
				zval_ptr_dtor(&extra_method);
				zval_ptr_dtor(&field);
				zval_ptr_dtor(&attributes);
				zval_ptr_dtor(&extra_method_first);
				zval_ptr_dtor(&exception_message);
				return;
			}
		}
		zval_ptr_dtor(&extra_method_first);
	}
	zval_ptr_dtor(&attributes);

	PHALCON_CONCAT_VS(&conditions, &field, " = ?0");
	zval_ptr_dtor(&extra_method);
	zval_ptr_dtor(&field);

	array_init_size(&bind_params, 1);
	phalcon_array_append(&bind_params, &value, PH_COPY);

	array_init_size(&parameters, 2);
	phalcon_array_update_string(&parameters, IS(conditions), &conditions, 0);
	phalcon_array_update_str(&parameters, SL("bind"), &bind_params, 0);

	/**
	 * Execute the query
	 */
	PHALCON_RETURN_CALL_CE_STATIC(ce0, type, &parameters);
	zval_ptr_dtor(&parameters);
}

/**
 * Magic method to assign values to the the model
 *
 * @param string $property
 * @param mixed $value
 */
PHP_METHOD(Phalcon_Mvc_Model, __set){

	zval *property, *value, lower_property = {}, has = {}, model_name = {}, possible_setter = {}, manager = {}, relation = {}, referenced_model_name = {}, type = {};
	zval referenced_model = {}, values = {}, related = {}, *item, exception_message = {};
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 2, 0, &property, &value);
	PHALCON_ENSURE_IS_STRING(property);

	phalcon_fast_strtolower(&lower_property, property);

	phalcon_get_class(&model_name, getThis(), 0);

	PHALCON_CALL_METHOD(&has, getThis(), "hasAttribute", property);
	if (zend_is_true(&has)) {
		if (PHALCON_GLOBAL(orm).enable_property_method) {
			PHALCON_CONCAT_SV(&possible_setter, "__set", property);
			phalcon_strtolower_inplace(&possible_setter);

			/*
			 * Check method is not
			 */
			if (phalcon_method_exists_ce(phalcon_mvc_model_ce, &possible_setter) != SUCCESS) {
				if (phalcon_method_exists(getThis(), &possible_setter) == SUCCESS) {
					PHALCON_CALL_METHOD(NULL, getThis(), Z_STRVAL(possible_setter), value);
					zval_ptr_dtor(&possible_setter);
					RETURN_CTOR(value);
				}
			}
			zval_ptr_dtor(&possible_setter);
		}

		if (phalcon_isset_property_zval(getThis(), property)) {
			if (PHALCON_PROPERTY_IS_PRIVATE_ZVAL(getThis(), property)) {
				zend_error(E_ERROR, "Cannot access private property %s::%s", Z_STRVAL(model_name), Z_STRVAL_P(property));
				return;
			}

			if (PHALCON_PROPERTY_IS_PROTECTED_ZVAL(getThis(), property)) {
				zend_error(E_ERROR, "Cannot access protected property %s::%s", Z_STRVAL(model_name), Z_STRVAL_P(property));
				return;
			}
		}

		phalcon_update_property_zval_zval(getThis(), property, value);
		zval_ptr_dtor(&model_name);
		zval_ptr_dtor(&lower_property);
		RETURN_CTOR(value);
	}

	PHALCON_CALL_METHOD(&manager, getThis(), "getmodelsmanager");
	PHALCON_CALL_METHOD(&relation, &manager, "getrelationbyalias", &model_name, &lower_property);
	zval_ptr_dtor(&model_name);

	/**
	 * Values are probably relationships if they are objects
	 */
	if (Z_TYPE(relation) == IS_OBJECT) {
		PHALCON_CALL_METHOD(&referenced_model_name, &relation, "getreferencedmodel");

		if (Z_TYPE_P(value) == IS_NULL) {
			phalcon_unset_property_array(getThis(), SL("_related"), &lower_property);
			phalcon_unset_property_array(getThis(), SL("_relatedResult"), &lower_property);
		} else {
			PHALCON_CALL_METHOD(&type, &relation, "gettype");

			if (PHALCON_IS_LONG(&type, 0)) {
				if (Z_TYPE_P(value) == IS_OBJECT) {
					phalcon_get_class(&model_name, value, 0);

					if (instanceof_function_ex(Z_OBJCE_P(value), phalcon_mvc_model_resultsetinterface_ce, 1)) {
						phalcon_update_property_array(getThis(), SL("_relatedResult"), &lower_property, value);
					} else if (!instanceof_function_ex(Z_OBJCE_P(value), phalcon_mvc_modelinterface_ce, 1) || !PHALCON_IS_EQUAL(&referenced_model_name, &model_name)) {
						PHALCON_CONCAT_SVSVS(&exception_message, "Property \"", property, "\" must be an model `", &referenced_model_name, "`");
						PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
						zval_ptr_dtor(&model_name);
						goto end;
					} else {
						phalcon_update_property_array(getThis(), SL("_related"), &lower_property, value);
						phalcon_update_property_long(getThis(), SL("_dirtyState"), PHALCON_MODEL_DIRTY_STATE_TRANSIENT);
					}
					zval_ptr_dtor(&model_name);
				} else if (Z_TYPE_P(value) == IS_ARRAY) {
					PHALCON_CALL_METHOD(&referenced_model, &manager, "load", &referenced_model_name, &PHALCON_GLOBAL(z_false));
					if (Z_TYPE(referenced_model) == IS_OBJECT) {
						PHALCON_CALL_METHOD(NULL, &referenced_model, "assign", value);
					}
					zval_ptr_dtor(&referenced_model);
				} else {
					PHALCON_CONCAT_SVSVS(&exception_message, "Property \"", property, "\" must be an model `", &referenced_model_name, "`");
					PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
					goto end;
				}
			} else {
				if (Z_TYPE_P(value) != IS_ARRAY) {
					array_init_size(&values, 1);
					phalcon_array_append(&values, value, PH_COPY);
				} else {
					ZVAL_COPY(&values, value);
				}

				array_init(&related);

				ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(values), idx, str_key, item) {
					zval tmp = {};
					if (str_key) {
						ZVAL_STR(&tmp, str_key);
					} else {
						ZVAL_LONG(&tmp, idx);
					}

					if (Z_TYPE_P(item) == IS_OBJECT) {
						phalcon_get_class(&model_name, item, 0);

						if (instanceof_function_ex(Z_OBJCE_P(item), phalcon_mvc_model_resultsetinterface_ce, 1)) {
							phalcon_update_property_array(getThis(), SL("_relatedResult"), &lower_property, item);
							continue;
						}

						if (!instanceof_function_ex(Z_OBJCE_P(item), phalcon_mvc_modelinterface_ce, 1) || !PHALCON_IS_EQUAL(&referenced_model_name, &model_name)) {
							PHALCON_CONCAT_SVSVS(&exception_message, "Property \"", property, "\" must be an model `", &referenced_model_name, "`");
							PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
							zval_ptr_dtor(&model_name);
							zval_ptr_dtor(&referenced_model_name);
							goto end;
						}
						zval_ptr_dtor(&model_name);

						phalcon_array_append(&related, item, PH_COPY);
					} else if (Z_TYPE_P(item) == IS_ARRAY) {
						zval tmp_referenced_model_name = {};
						PHALCON_CALL_METHOD(&tmp_referenced_model_name, &relation, "getreferencedmodel");
						PHALCON_CALL_METHOD(&referenced_model, &manager, "load", &tmp_referenced_model_name, &PHALCON_GLOBAL(z_false));
						zval_ptr_dtor(&tmp_referenced_model_name);
						if (Z_TYPE(referenced_model) == IS_OBJECT) {
							PHALCON_CALL_METHOD(NULL, &referenced_model, "assign", item);
						}
						zval_ptr_dtor(&referenced_model);
					} else {
						PHALCON_CONCAT_SVSVS(&exception_message, "Property \"", property, "\" must be an model `", &referenced_model_name, "`");
						PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
						zval_ptr_dtor(&referenced_model_name);
						goto end;
					}
				} ZEND_HASH_FOREACH_END();
				zval_ptr_dtor(&values);

				if (phalcon_fast_count_ev(&related)) {
					phalcon_update_property_array(getThis(), SL("_related"), &lower_property, &related);
					phalcon_update_property_long(getThis(), SL("_dirtyState"), PHALCON_MODEL_DIRTY_STATE_TRANSIENT);
				}
				zval_ptr_dtor(&related);
			}
		}
		zval_ptr_dtor(&referenced_model_name);
	} else {
		phalcon_update_property_zval_zval(getThis(), property, value);
	}
end:
	zval_ptr_dtor(&lower_property);
	zval_ptr_dtor(&relation);
	zval_ptr_dtor(&manager);
}

/**
 * Magic method to get related records using the relation alias as a property
 *
 * @param string $property
 * @return Phalcon\Mvc\Model\Resultset
 */
PHP_METHOD(Phalcon_Mvc_Model, __get){

	zval *property, has = {}, possible_getter = {}, model_name = {}, manager = {}, lower_property = {};
	zval related = {}, related_result = {}, relation = {}, call_args = {}, call_object = {}, result = {};

	phalcon_fetch_params(0, 1, 0, &property);
	PHALCON_ENSURE_IS_STRING(property);

	phalcon_fast_strtolower(&lower_property, property);
	PHALCON_CALL_METHOD(&has, getThis(), "hasAttribute", property);
	if (zend_is_true(&has)) {
		if (PHALCON_GLOBAL(orm).enable_property_method) {
			PHALCON_CONCAT_SV(&possible_getter, "__get", &lower_property);

			if (phalcon_method_exists_ce(phalcon_mvc_model_ce, &possible_getter) != SUCCESS) {
				if (phalcon_method_exists(getThis(), &possible_getter) == SUCCESS) {
					PHALCON_CALL_ZVAL_METHOD(return_value, getThis(), &possible_getter);
					zval_ptr_dtor(&possible_getter);
					zval_ptr_dtor(&lower_property);
					return;
				}
			}
			zval_ptr_dtor(&possible_getter);
		}
		RETURN_NULL();
	}

	phalcon_read_property(&related, getThis(), SL("_related"), PH_READONLY);
	if (Z_TYPE(related) == IS_ARRAY) {
		if (phalcon_array_isset_fetch(&result, &related, &lower_property, PH_READONLY)) {
			zval_ptr_dtor(&lower_property);
			RETURN_CTOR(&result);
		}
	}

	phalcon_read_property(&related_result, getThis(), SL("_relatedResult"), PH_READONLY);
	if (Z_TYPE(related_result) == IS_ARRAY) {
		if (phalcon_array_isset_fetch(&result, &related_result, &lower_property, PH_READONLY)) {
			zval_ptr_dtor(&lower_property);
			RETURN_CTOR(&result);
		}
	}

	phalcon_get_class(&model_name, getThis(), 0);

	PHALCON_CALL_METHOD(&manager, getThis(), "getmodelsmanager");
	PHALCON_CALL_METHOD(&relation, &manager, "getrelationbyalias", &model_name, &lower_property);
	zval_ptr_dtor(&model_name);

	/**
	 * Check if the property is a relationship
	 */
	if (Z_TYPE(relation) == IS_OBJECT) {
		array_init_size(&call_args, 4);
		phalcon_array_append(&call_args, &relation, 0);
		add_next_index_null(&call_args);
		phalcon_array_append(&call_args, getThis(), PH_COPY);
		add_next_index_null(&call_args);

		array_init_size(&call_object, 2);
		phalcon_array_append(&call_object, &manager, 0);
		add_next_index_stringl(&call_object, SL("getRelationRecords"));

		/**
		 * Get the related records
		 */
		PHALCON_CALL_USER_FUNC_ARRAY(&result, &call_object, &call_args);
		zval_ptr_dtor(&call_args);
		zval_ptr_dtor(&call_object);

		/**
		 * Assign the result to the object
		 */
		if (Z_TYPE(result) == IS_OBJECT) {
			/**
			 * For belongs-to relations we store the object in the related bag
			 */
			if (instanceof_function_ex(Z_OBJCE(result), phalcon_mvc_modelinterface_ce, 1)) {
				phalcon_update_property_array(getThis(), SL("_related"), &lower_property, &result);
			} else if (instanceof_function_ex(Z_OBJCE(result), phalcon_mvc_model_resultsetinterface_ce, 1)) {
				phalcon_update_property_array(getThis(), SL("_relatedResult"), &lower_property, &result);
			}
		}
		zval_ptr_dtor(&lower_property);
		RETVAL_ZVAL(&result, 0, 0);
		return;
	}
	zval_ptr_dtor(&lower_property);
	zval_ptr_dtor(&manager);
	zval_ptr_dtor(&relation);

	/**
	 * A notice is shown if the property is not defined and it isn't a relationship
	 */
	/* TODO see if segfault is possible */
	PHALCON_CALL_PARENT(return_value, phalcon_mvc_model_ce, getThis(), "__get", property);
}

/**
 * Magic method to check if a property is a valid relation
 *
 * @param string $property
 */
PHP_METHOD(Phalcon_Mvc_Model, __isset){

	zval *property, model_name = {}, manager = {}, relation = {};

	phalcon_fetch_params(0, 1, 0, &property);

	phalcon_get_class(&model_name, getThis(), 0);

	PHALCON_CALL_METHOD(&manager, getThis(), "getmodelsmanager");

	/**
	 * Check if the property is a relationship
	 */
	PHALCON_CALL_METHOD(&relation, &manager, "getrelationbyalias", &model_name, property);
	zval_ptr_dtor(&manager);
	zval_ptr_dtor(&model_name);
	if (Z_TYPE(relation) == IS_OBJECT) {
		RETVAL_TRUE;
	} else {
		RETVAL_FALSE;
	}
	zval_ptr_dtor(&relation);
}

/**
 * Serializes the object ignoring connections, services, related objects or static properties
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model, serialize){

	zval attributes = {}, column_map = {}, data = {}, *attribute, exception_message = {};

	/**
	 * We get the model's attributes to only serialize them
	 */
	PHALCON_CALL_METHOD(&attributes, getThis(), "getattributes");

	/**
	 * Reverse column map
	 */
	PHALCON_CALL_SELF(&column_map, "getcolumnmap");

	array_init(&data);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(attributes), attribute) {
		zval attribute_field = {}, attribute_value = {};
		/**
		 * Check if the columns must be renamed
		 */
		if (Z_TYPE(column_map) == IS_ARRAY) {
			if (!phalcon_array_isset_fetch(&attribute_field, &column_map, attribute, PH_READONLY)) {
				PHALCON_CONCAT_SVS(&exception_message, "Column \"", attribute, "\" doesn't make part of the column map");
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
				zval_ptr_dtor(&column_map);
				zval_ptr_dtor(&attributes);
				zval_ptr_dtor(&data);
				return;
			}
		} else {
			ZVAL_COPY_VALUE(&attribute_field, attribute);
		}

		if (phalcon_property_isset_fetch_zval(&attribute_value, getThis(), &attribute_field, PH_READONLY)) {
			phalcon_array_update(&data, &attribute_field, &attribute_value, PH_COPY);
		} else {
			phalcon_array_update(&data, &attribute_field, &PHALCON_GLOBAL(z_null), PH_COPY);
		}
	} ZEND_HASH_FOREACH_END();
	zval_ptr_dtor(&column_map);
	zval_ptr_dtor(&attributes);

	/**
	 * Use the standard serialize function to serialize the array data
	 */
	phalcon_serialize(return_value, &data);
	zval_ptr_dtor(&data);
}

/**
 * Unserializes the object from a serialized string
 *
 * @param string $data
 */
PHP_METHOD(Phalcon_Mvc_Model, unserialize){

	zval *data, attributes = {}, models_manager = {}, *value;
	zend_string *str_key;

	phalcon_fetch_params(0, 1, 0, &data);

	if (Z_TYPE_P(data) == IS_STRING) {
		phalcon_unserialize(&attributes, data);
		if (Z_TYPE(attributes) == IS_ARRAY) {
			PHALCON_CALL_METHOD(&models_manager, getThis(), "getmodelsmanager");

			/**
			 * Try to initialize the model
			 */
			PHALCON_CALL_METHOD(NULL, &models_manager, "initialize", getThis());
			zval_ptr_dtor(&models_manager);

			/**
			 * Update the objects attributes
			 */
			ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL(attributes), str_key, value) {
				if (str_key) {
					phalcon_update_property_string_zval(getThis(), str_key, value);
				}
			} ZEND_HASH_FOREACH_END();

			zval_ptr_dtor(&attributes);
			RETURN_NULL();
		}
	}
	PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Invalid serialization data");
}

/**
 * Returns a simple representation of the object that can be used with var_dump
 *
 *<code>
 * var_dump($robot->dump());
 *</code>
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, dump){

	phalcon_get_object_vars(return_value, getThis(), 1);
}

PHP_METHOD(Phalcon_Mvc_Model, getFieldValues){

	zval *attribute = NULL, column_map = {};

	PHALCON_MM_INIT();

	array_init(return_value);

	/**
	 * Reverse column map
	 */
	PHALCON_MM_CALL_SELF(&column_map, "getcolumnmap");
	PHALCON_MM_ADD_ENTRY(&column_map);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(column_map), attribute) {
		zval attribute_value = {};
		if (phalcon_property_isset_fetch_zval(&attribute_value, getThis(), attribute, PH_READONLY)) {
			phalcon_array_update(return_value, attribute, &attribute_value, PH_COPY);
		} else {
			phalcon_array_update(return_value, attribute, &PHALCON_GLOBAL(z_null), PH_COPY);
		}
	} ZEND_HASH_FOREACH_END();

	RETURN_MM();
}

/**
 * Returns the instance as an array representation
 *
 *<code>
 * print_r($robot->toArray());
 *</code>
 *
 * @param array $columns
 * @param bool $mustColumn
 * @param bool $negate
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, toArray){

	zval *columns = NULL, *must_column = NULL, *negate = NULL, column_map = {}, data = {}, event_name = {};
	HashTable *properties;
	zval *value;
	zend_string *key;

	phalcon_fetch_params(1, 0, 3, &columns, &must_column, &negate);

	if (!columns) {
		columns = &PHALCON_GLOBAL(z_null);
	}

	if (!must_column || Z_TYPE_P(must_column) == IS_NULL) {
		must_column = PHALCON_GLOBAL(orm).must_column ? &PHALCON_GLOBAL(z_true) : &PHALCON_GLOBAL(z_false);
	}

	if (!negate) {
		negate = &PHALCON_GLOBAL(z_false);
	}

	PHALCON_MM_ZVAL_STRING(&event_name, "beforeToArray");
	PHALCON_MM_CALL_METHOD(NULL, getThis(), "fireevent", &event_name);

	array_init(&data);
	PHALCON_MM_ADD_ENTRY(&data);

	if (zend_is_true(must_column)) {
		zval *attribute;
		/**
		 * Reverse column map
		 */
		PHALCON_MM_CALL_SELF(&column_map, "getcolumnmap");
		PHALCON_MM_ADD_ENTRY(&column_map);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(column_map), attribute) {
			zval attribute_value = {};

			if (Z_TYPE_P(columns) == IS_ARRAY) {
				if (likely(!zend_is_true(negate))) {
					if (!phalcon_fast_in_array(attribute, columns)) {
						continue;
					}
				} else {
					if (phalcon_fast_in_array(attribute, columns)) {
						continue;
					}
				}
			}

			if (PHALCON_GLOBAL(orm).enable_property_method) {
				zval possible_getter = {};
				PHALCON_CONCAT_SV(&possible_getter, "__get", attribute);
				phalcon_strtolower_inplace(&possible_getter);
				PHALCON_MM_ADD_ENTRY(&possible_getter);
				if (phalcon_method_exists(getThis(), &possible_getter) == SUCCESS) {
					zval possible_value = {};
					PHALCON_MM_CALL_ZVAL_METHOD(&possible_value, getThis(), &possible_getter);
					phalcon_array_update(&data, attribute, &possible_value, 0);
				} else if (phalcon_property_isset_fetch_zval(&attribute_value, getThis(), attribute, PH_READONLY)) {
					phalcon_array_update(&data, attribute, &attribute_value, PH_COPY);
				} else {
					phalcon_array_update(&data, attribute, &PHALCON_GLOBAL(z_null), PH_COPY);
				}
			} else if (phalcon_property_isset_fetch_zval(&attribute_value, getThis(), attribute, PH_READONLY)) {
				phalcon_array_update(&data, attribute, &attribute_value, PH_COPY);
			} else {
				phalcon_array_update(&data, attribute, &PHALCON_GLOBAL(z_null), PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();
	} else {
		/**
		 * Reverse column map
		 */
		PHALCON_MM_CALL_SELF(&column_map, "getreversecolumnmap");
		PHALCON_MM_ADD_ENTRY(&column_map);

		properties = Z_OBJ_HT_P(getThis())->get_properties(getThis());

		ZEND_HASH_FOREACH_STR_KEY_VAL(properties, key, value) {
			zval field = {};
			if (key) {
				if (Z_ISREF_P(value) && Z_REFCOUNT_P(value) == 1) {
					value = Z_REFVAL_P(value);
				}
				/*
				if (Z_REFCOUNTED_P(value)) {
					Z_ADDREF_P(value);
				}
				*/
				if (ZSTR_VAL(key)[0] == 0) {
					const char *prop_name, *class_name;
					size_t prop_len;
					zend_unmangle_property_name_ex(key, &class_name, &prop_name, &prop_len);
					ZVAL_STRINGL(&field, prop_name, prop_len);
				} else {
					ZVAL_STR(&field, zend_string_dup(key, 0));
				}
				PHALCON_MM_ADD_ENTRY(&field);

				if (Z_TYPE_P(columns) == IS_ARRAY) {
					if (likely(!zend_is_true(negate))) {
						if (!phalcon_fast_in_array(&field, columns)) {
							continue;
						}
					} else {
						if (phalcon_fast_in_array(&field, columns)) {
							continue;
						}
					}
				}

				if (phalcon_array_isset(&column_map, &field)) {
					if (PHALCON_GLOBAL(orm).enable_property_method) {
						zval possible_getter = {};
						PHALCON_CONCAT_SV(&possible_getter, "__get", &field);
						phalcon_strtolower_inplace(&possible_getter);
						PHALCON_MM_ADD_ENTRY(&possible_getter);
						if (phalcon_method_exists(getThis(), &possible_getter) == SUCCESS) {
							zval possible_value = {};
							PHALCON_MM_CALL_ZVAL_METHOD(&possible_value, getThis(), &possible_getter);
							phalcon_array_update(&data, &field, &possible_value, 0);
						} else {
							phalcon_array_update(&data, &field, value, PH_COPY);
						}
					} else {
						phalcon_array_update(&data, &field, value, PH_COPY);
					}
				} else if (!zend_is_true(must_column) && phalcon_property_exists(getThis(), Z_STRVAL(field), Z_STRLEN(field), PH_DYNAMIC)) {
					phalcon_array_update(&data, &field, value, PH_COPY);
				}
			}
		} ZEND_HASH_FOREACH_END();
	}

	PHALCON_MM_ZVAL_STRING(&event_name, "afterToArray");
	PHALCON_MM_CALL_METHOD(return_value, getThis(), "fireevent", &event_name, &data);

	if (Z_TYPE_P(return_value) == IS_ARRAY) {
		RETURN_MM();
	}
	RETURN_MM_CTOR(&data);
}

/**
 * Enables/disables options in the ORM
 * Available options:
 * events                — Enables/Disables globally the internal events
 * virtualForeignKeys    — Enables/Disables virtual foreign keys
 * columnRenaming        — Enables/Disables column renaming
 * notNullValidations    — Enables/Disables automatic not null validation
 * exceptionOnFailedSave — Enables/Disables throws an exception if the saving process fails
 * phqlLiterals          — Enables/Disables literals in PHQL this improves the security of applications
 * propertyMethod        — Enables/Disables property method
 * autoConvert           — Enables/Disables auto convert
 * strict                — Enables/Disables strict mode
 * strict                — Enables/Disables strict mode
 *
 * @param array $options
 */
PHP_METHOD(Phalcon_Mvc_Model, setup){

	zval *options, disable_events = {}, virtual_foreign_keys = {}, not_null_validations = {}, length_validations = {}, use_mb_strlen = {};
	zval exception_on_failed_save = {};
	zval phql_literals = {}, property_method = {}, auto_convert = {}, allow_update_primary = {}, enable_strict = {}, must_column = {};

	phalcon_fetch_params(0, 1, 0, &options);

	if (Z_TYPE_P(options) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Options must be an array");
		return;
	}

	/**
	 * Enables/Disables globally the internal events
	 */
	if (phalcon_array_isset_fetch_str(&disable_events, options, SL("events"), PH_READONLY)) {
		PHALCON_GLOBAL(orm).events = zend_is_true(&disable_events);
	}

	/**
	 * Enables/Disables virtual foreign keys
	 */
	if (phalcon_array_isset_fetch_str(&virtual_foreign_keys, options, SL("virtualForeignKeys"), PH_READONLY)) {
		PHALCON_GLOBAL(orm).virtual_foreign_keys = zend_is_true(&virtual_foreign_keys);
	}

	/**
	 * Enables/Disables automatic not null validation
	 */
	if (phalcon_array_isset_fetch_str(&not_null_validations, options, SL("notNullValidations"), PH_READONLY)) {
		PHALCON_GLOBAL(orm).not_null_validations = zend_is_true(&not_null_validations);
	}

	/**
	 * Enables/Disables automatic length validation
	 */
	if (phalcon_array_isset_fetch_str(&length_validations, options, SL("lengthValidations"), PH_READONLY)) {
		PHALCON_GLOBAL(orm).length_validations = zend_is_true(&length_validations);
	}
	if (phalcon_array_isset_fetch_str(&use_mb_strlen, options, SL("useMbStrlen"), PH_READONLY)) {
		PHALCON_GLOBAL(orm).use_mb_strlen = zend_is_true(&use_mb_strlen);
	}

	/**
	 * Enables/Disables throws an exception if the saving process fails
	 */
	if (phalcon_array_isset_fetch_str(&exception_on_failed_save, options, SL("exceptionOnFailedSave"), PH_READONLY)) {
		PHALCON_GLOBAL(orm).exception_on_failed_save = zend_is_true(&exception_on_failed_save);
	}

	/**
	 * Enables/Disables literals in PHQL this improves the security of applications
	 */
	if (phalcon_array_isset_fetch_str(&phql_literals, options, SL("phqlLiterals"), PH_READONLY)) {
		PHALCON_GLOBAL(orm).enable_literals = zend_is_true(&phql_literals);
	}

	/**
	 * Enables/Disables property method
	 */
	if (phalcon_array_isset_fetch_str(&property_method, options, SL("propertyMethod"), PH_READONLY)) {
		PHALCON_GLOBAL(orm).enable_property_method = zend_is_true(&property_method);
	}

	/**
	 * Enables/Disables auto convert
	 */
	if (phalcon_array_isset_fetch_str(&auto_convert, options, SL("autoConvert"), PH_READONLY)) {
		PHALCON_GLOBAL(orm).enable_auto_convert = zend_is_true(&auto_convert);
	}

	/**
	 * Enables/Disables allow update primary
	 */
	if (phalcon_array_isset_fetch_str(&allow_update_primary, options, SL("allowUpdatePrimary"), PH_READONLY)) {
		PHALCON_GLOBAL(orm).allow_update_primary = zend_is_true(&allow_update_primary);
	}

	/**
	 * Enables/Disables strict mode
	 */
	if (phalcon_array_isset_fetch_str(&enable_strict, options, SL("strict"), PH_READONLY)) {
		PHALCON_GLOBAL(orm).enable_strict = zend_is_true(&enable_strict);
	}

	/**
	 * Enables/Disables must column mode
	 */
	if (phalcon_array_isset_fetch_str(&must_column, options, SL("mustColumn"), PH_READONLY)) {
		PHALCON_GLOBAL(orm).must_column = zend_is_true(&must_column);
	}
}

/**
 * Allows to delete a set of records that match the specified conditions
 *
 * <code>
 * $robot = Robots::remove("id=100")
 * </code>
 *
 * @param 	array $parameters
 * @return	boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, remove){

	zval *parameters = NULL, dependency_injector = {}, service_name = {}, model_name = {}, manager = {}, model = {}, write_connection = {}, schema = {}, source = {}, delete_conditions = {};
	zval bind_params = {}, bind_types = {}, query = {}, phql = {}, intermediate = {}, dialect = {}, table_conditions = {}, where_conditions = {}, where_expression = {}, success = {};

	phalcon_fetch_params(0, 1, 0, &parameters);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_CE_STATIC(&dependency_injector, phalcon_di_ce, "getdefault");

	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
		return;
	}

	phalcon_get_called_class(&model_name);
	ZVAL_STR(&service_name, IS(modelsManager));

	PHALCON_CALL_METHOD(&manager, &dependency_injector, "getshared", &service_name);
	zval_ptr_dtor(&dependency_injector);
	PHALCON_CALL_METHOD(&model, &manager, "load", &model_name);
	PHALCON_CALL_METHOD(&schema, &model, "getschema");
	PHALCON_CALL_METHOD(&source, &model, "getsource");
	zval_ptr_dtor(&model);

	array_init_size(&table_conditions, 2);
	phalcon_array_append(&table_conditions, &schema, PH_COPY);
	phalcon_array_append(&table_conditions, &source, PH_COPY);

	if (Z_TYPE_P(parameters) == IS_STRING) {
		ZVAL_COPY_VALUE(&delete_conditions, parameters);
	} else if (Z_TYPE_P(parameters) == IS_ARRAY) {
		if (!phalcon_array_isset_fetch_long(&delete_conditions, parameters, 0, PH_READONLY)) {
			if (!phalcon_array_isset_fetch_str(&delete_conditions, parameters, SL("conditions"), PH_READONLY)) {
				PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Must set up the conditions");
				return;
			}
		}

		if (!phalcon_array_isset_fetch_long(&bind_params, parameters, 1, PH_READONLY)) {
			if (phalcon_array_isset_str(parameters, SL("bind"))) {
				phalcon_array_fetch_str(&bind_params, parameters, SL("bind"), PH_NOISY|PH_READONLY);
			}
		}

		if (!phalcon_array_isset_fetch_long(&bind_types, parameters, 2, PH_READONLY)) {
			if (phalcon_array_isset_str(parameters, SL("bindTypes"))) {
				phalcon_array_fetch_str(&bind_types, parameters, SL("bindTypes"), PH_NOISY|PH_READONLY);
			}
		}
	}

	/**
	 * Process the PHQL
	 */
	if (PHALCON_IS_NOT_EMPTY(&delete_conditions)) {
		PHALCON_CONCAT_SVSV(&phql, "DELETE FROM ", &model_name, " WHERE ", &delete_conditions);
	} else {
		PHALCON_CONCAT_SV(&phql, "DELETE FROM ", &model_name);
	}
	zval_ptr_dtor(&model_name);

	PHALCON_CALL_METHOD(&query, &manager, "createquery", &phql);
	zval_ptr_dtor(&manager);
	zval_ptr_dtor(&phql);

	PHALCON_CALL_METHOD(&intermediate, &query, "parse");
	zval_ptr_dtor(&query);
	PHALCON_CALL_METHOD(&write_connection, getThis(), "getwriteconnection", &intermediate, &bind_params, &bind_types);
	PHALCON_CALL_METHOD(&dialect, &write_connection, "getdialect");

	if (phalcon_array_isset_fetch_str(&where_conditions, &intermediate, SL("where"), PH_READONLY)) {
		if (Z_TYPE(where_conditions) == IS_ARRAY) {
			PHALCON_CALL_METHOD(&where_expression, &dialect, "getsqlexpression", &where_conditions);
		} else {
			ZVAL_COPY(&where_expression, &where_conditions);
		}
	}
	zval_ptr_dtor(&dialect);
	zval_ptr_dtor(&intermediate);

	PHALCON_CALL_METHOD(&success, &write_connection, "delete", &table_conditions, &where_expression, &bind_params, &bind_types);
	zval_ptr_dtor(&where_expression);
	zval_ptr_dtor(&table_conditions);

	if (PHALCON_IS_TRUE(&success)) {
		PHALCON_CALL_METHOD(&success, &write_connection, "affectedRows");
	}
	zval_ptr_dtor(&write_connection);

	if (zend_is_true(&success)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/*
 * Reset the model data
 *
 * <code>
 * $robot = Robots::findFirst();
 * $robot->reset();
 * </code>
 */
PHP_METHOD(Phalcon_Mvc_Model, reset){

	phalcon_update_property_null(getThis(), SL("_uniqueParams"));
	phalcon_update_property_null(getThis(), SL("_snapshot"));
	phalcon_update_property_null(getThis(), SL("_relatedResult"));
	phalcon_update_property_null(getThis(), SL("_related"));
}

/**
 * Sanitizes a value with a specified single or set of filters
 *
 *<code>
 *use Phalcon\Mvc\Model\Validator\ExclusionIn as ExclusionIn;
 *
 *class Subscriptors extends Phalcon\Mvc\Model
 *{
 *
 *	public function filters()
 *  {
 * 		$this->filter('status', 'int');
 *	}
 *
 *}
 *</code>
 *
 * @param string $field
 * @param string|array $filters
 * @param mixed $defaultValue
 * @param boolean $notAllowEmpty
 * @param boolean $recursive
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, filter){

	zval *field, *filters, *default_value = NULL, *not_allow_empty = NULL, *recursive = NULL;
	zval value = {}, filterd_value = {}, filter = {}, dependency_injector = {}, service = {};

	phalcon_fetch_params(0, 2, 3, &field, &filters, &default_value, &not_allow_empty, &recursive);

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	if (!not_allow_empty) {
		not_allow_empty = &PHALCON_GLOBAL(z_false);
	}

	if (!recursive || Z_TYPE_P(recursive) == IS_NULL) {
		recursive = &PHALCON_GLOBAL(z_true);
	}

	if (phalcon_isset_property_zval(getThis(), field)) {
		phalcon_read_property_zval(&value, getThis(), field, PH_READONLY);

		if (!PHALCON_IS_EMPTY(&value) && Z_TYPE_P(filters) != IS_NULL) {
			phalcon_read_property(&filter, getThis(), SL("_filter"), PH_READONLY);

			if (Z_TYPE(filter) != IS_OBJECT) {
				PHALCON_CALL_METHOD(&dependency_injector, getThis(), "getdi");
				if (Z_TYPE(dependency_injector) != IS_OBJECT) {
					PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "A dependency injection object is required to access the 'filter' service");
					return;
				}

				ZVAL_STR(&service, IS(filter));

				PHALCON_CALL_METHOD(&filter, &dependency_injector, "getshared", &service);
				PHALCON_VERIFY_INTERFACE(&filter, phalcon_filterinterface_ce);
				phalcon_update_property(getThis(), SL("_filter"), &filter);
			}

			PHALCON_CALL_METHOD(&filterd_value, &filter, "sanitize", &value, filters, recursive);

			if ((PHALCON_IS_EMPTY(&filterd_value) && zend_is_true(not_allow_empty)) || PHALCON_IS_FALSE(&filterd_value)) {
				phalcon_update_property_zval_zval(getThis(), field, default_value);
			} else {
				phalcon_update_property_zval_zval(getThis(), field, &filterd_value);
			}
		} else if (PHALCON_IS_EMPTY(&value) && zend_is_true(not_allow_empty)) {
			phalcon_update_property_zval_zval(getThis(), field, default_value);
		}
	} else {
		phalcon_update_property_zval_zval(getThis(), field, default_value);
	}

	RETURN_THIS();
}

/**
 * Whether the record is not new and deleted
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, isRecord){

	zval dirty_state = {};

	phalcon_read_property(&dirty_state, getThis(), SL("_dirtyState"), PH_NOISY|PH_READONLY);

	if (phalcon_get_intval(&dirty_state) == PHALCON_MODEL_DIRTY_STATE_PERSISTEN) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Whether the record is new and deleted
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, isNewRecord){

	zval dirty_state = {};

	phalcon_read_property(&dirty_state, getThis(), SL("_dirtyState"), PH_NOISY|PH_READONLY);

	if (phalcon_get_intval(&dirty_state) == PHALCON_MODEL_DIRTY_STATE_TRANSIENT) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Whether the record is new and deleted
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, isDeletedRecord){

	zval dirty_state = {};

	phalcon_read_property(&dirty_state, getThis(), SL("_dirtyState"), PH_NOISY|PH_READONLY);

	if (phalcon_get_intval(&dirty_state) == PHALCON_MODEL_DIRTY_STATE_DETACHED) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Returns serialised model as array for json_encode.
 *
 *<code>
 * $robot = Robots::findFirst();
 * echo json_encode($robot);
 *</code>
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, jsonSerialize) {

	PHALCON_CALL_METHOD(return_value, getThis(), "toarray");
}

PHP_METHOD(Phalcon_Mvc_Model, __debugInfo){

	PHALCON_CALL_PARENT(return_value, phalcon_mvc_model_ce, getThis(), "__debuginfo");
/*
	if (likely(!PHALCON_GLOBAL(debug).enable_debug)) {
		phalcon_array_unset_str(return_value, SL("_modelsMetaData"), 0);
	}
*/
}
