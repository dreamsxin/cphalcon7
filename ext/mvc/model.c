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

#include "mvc/model.h"
#include "mvc/modelinterface.h"
#include "mvc/model/criteria.h"
#include "mvc/model/exception.h"
#include "mvc/model/managerinterface.h"
#include "mvc/model/manager.h"
#include "mvc/model/message.h"
#include "mvc/model/metadatainterface.h"
#include "mvc/model/metadata/memory.h"
#include "mvc/model/query/builder.h"
#include "mvc/model/query.h"
#include "mvc/model/resultinterface.h"
#include "mvc/model/resultsetinterface.h"
#include "mvc/model/validationfailed.h"
#include "mvc/model/validatorinterface.h"
#include "mvc/model/criteria.h"
#include "di.h"
#include "diinterface.h"
#include "di/injectable.h"
#include "events/eventsawareinterface.h"
#include "db/column.h"
#include "db/rawvalue.h"
#include "db/adapterinterface.h"
#include "filterinterface.h"

#include <ext/pdo/php_pdo_driver.h>

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
PHP_METHOD(Phalcon_Mvc_Model, getColumnMap);
PHP_METHOD(Phalcon_Mvc_Model, getReverseColumnMap);
PHP_METHOD(Phalcon_Mvc_Model, getColumns);
PHP_METHOD(Phalcon_Mvc_Model, getDataTypes);
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
PHP_METHOD(Phalcon_Mvc_Model, _reBuild);
PHP_METHOD(Phalcon_Mvc_Model, _exists);
PHP_METHOD(Phalcon_Mvc_Model, _groupResult);
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
PHP_METHOD(Phalcon_Mvc_Model, keepSnapshots);
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
PHP_METHOD(Phalcon_Mvc_Model, toArray);
PHP_METHOD(Phalcon_Mvc_Model, setup);
PHP_METHOD(Phalcon_Mvc_Model, remove);
PHP_METHOD(Phalcon_Mvc_Model, reset);
PHP_METHOD(Phalcon_Mvc_Model, filter);
PHP_METHOD(Phalcon_Mvc_Model, isRecord);
PHP_METHOD(Phalcon_Mvc_Model, isNewRecord);
PHP_METHOD(Phalcon_Mvc_Model, isDeletedRecord);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, dependencyInjector)
	ZEND_ARG_INFO(0, modelsManager)
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_validate, 0, 0, 1)
	ZEND_ARG_INFO(0, validator)
	ZEND_ARG_INFO(0, allow_empty)
	ZEND_ARG_INFO(0, ex)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_skipoperation, 0, 0, 1)
	ZEND_ARG_INFO(0, skip)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_skipattributes, 0, 0, 1)
	ZEND_ARG_INFO(0, attributes)
	ZEND_ARG_INFO(0, replace)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_skipattributesoncreate, 0, 0, 1)
	ZEND_ARG_INFO(0, attributes)
	ZEND_ARG_INFO(0, replace)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_skipattributesonupdate, 0, 0, 1)
	ZEND_ARG_INFO(0, attributes)
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_keepsnapshots, 0, 0, 1)
	ZEND_ARG_INFO(0, keepSnapshots)
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
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_toarray, 0, 0, 0)
	ZEND_ARG_INFO(0, columns)
	ZEND_ARG_INFO(0, renameColumns)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_filter, 0, 0, 2)
	ZEND_ARG_INFO(0, field)
	ZEND_ARG_INFO(0, filters)
	ZEND_ARG_INFO(0, defaultValue)
	ZEND_ARG_INFO(0, notAllowEmpty)
	ZEND_ARG_INFO(0, noRecursive)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model, __construct, arginfo_phalcon_mvc_model___construct, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Model, setEventsManager, arginfo_phalcon_events_eventsawareinterface_seteventsmanager, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getEventsManager, arginfo_phalcon_events_eventsawareinterface_geteventsmanager, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getModelsMetaData, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getModelsManager, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, setTransaction, arginfo_phalcon_mvc_modelinterface_settransaction, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getTransaction, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, setSource, arginfo_phalcon_mvc_model_setsource, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, getSource, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, setSchema, arginfo_phalcon_mvc_model_setschema, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, getSchema, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getColumnMap, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getReverseColumnMap, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getColumns, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, getDataTypes, NULL, ZEND_ACC_PUBLIC)
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
	PHP_ME(Phalcon_Mvc_Model, _reBuild, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, _exists, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, _groupResult, NULL, ZEND_ACC_PROTECTED|ZEND_ACC_STATIC)
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
	PHP_ME(Phalcon_Mvc_Model, keepSnapshots, arginfo_phalcon_mvc_model_keepsnapshots, ZEND_ACC_PROTECTED)
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
	PHP_ME(Phalcon_Mvc_Model, toArray, arginfo_phalcon_mvc_model_toarray, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, setup, arginfo_phalcon_mvc_model_setup, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model, remove, arginfo_phalcon_mvc_modelinterface_remove, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Mvc_Model, reset, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, filter, arginfo_phalcon_mvc_model_filter, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model, isRecord, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, isNewRecord, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model, isDeletedRecord, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc, Model, mvc_model, phalcon_di_injectable_ce, phalcon_mvc_model_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_mvc_model_ce, SL("_dependencyInjector"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_modelsManager"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_modelsMetaData"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_ce, SL("_errorMessages"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_mvc_model_ce, SL("_operationMade"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_mvc_model_ce, SL("_dirtyState"), 1, ZEND_ACC_PROTECTED);
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

	zend_declare_class_constant_long(phalcon_mvc_model_ce, SL("OP_NONE"), 0);
	zend_declare_class_constant_long(phalcon_mvc_model_ce, SL("OP_CREATE"), 1);
	zend_declare_class_constant_long(phalcon_mvc_model_ce, SL("OP_UPDATE"), 2);
	zend_declare_class_constant_long(phalcon_mvc_model_ce, SL("OP_DELETE"), 3);
	zend_declare_class_constant_long(phalcon_mvc_model_ce, SL("DIRTY_STATE_PERSISTENT"), 0);
	zend_declare_class_constant_long(phalcon_mvc_model_ce, SL("DIRTY_STATE_TRANSIENT"), 1);
	zend_declare_class_constant_long(phalcon_mvc_model_ce, SL("DIRTY_STATE_DETACHED"), 2);

	zend_class_implements(phalcon_mvc_model_ce, 3, phalcon_mvc_modelinterface_ce, phalcon_mvc_model_resultinterface_ce, zend_ce_serializable);

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
		if (Z_TYPE_P(message) == IS_OBJECT) {
			zval *params[] = { target };
			if (FAILURE == phalcon_call_method(NULL, message, "setmodel", 1, params)) {
				break;
			}
		}

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

	zval *di = NULL, *_models_manager = NULL, models_manager = {};

	phalcon_fetch_params(0, 0, 2, &di, &_models_manager);

	if (di) {
		if (Z_TYPE_P(di) != IS_OBJECT) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
			return;
		}
		PHALCON_VERIFY_INTERFACEW(di, phalcon_diinterface_ce);
		phalcon_update_property_zval(getThis(), SL("_dependencyInjector"), di);
	}

	if (_models_manager) {
		PHALCON_VERIFY_INTERFACEW(_models_manager, phalcon_mvc_model_managerinterface_ce);
		phalcon_update_property_zval(getThis(), SL("_modelsManager"), _models_manager);
	}

	PHALCON_CALL_METHODW(&models_manager, getThis(), "getmodelsmanager");

	if (Z_TYPE(models_manager) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "The injected service 'modelsManager' is not object (1)");
		return;
	}

	/**
	 * The manager always initializes the object
	 */
	PHALCON_CALL_METHODW(NULL, &models_manager, "initialize", getThis());

	/**
	 * This allows the developer to execute initialization stuff every time an instance
	 * is created
	 */
	if (phalcon_method_exists_ex(getThis(), SL("onconstruct")) == SUCCESS) {
		PHALCON_CALL_METHODW(NULL, getThis(), "onconstruct");
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

	phalcon_read_property(&models_manager, getThis(), SL("_modelsManager"), PH_NOISY);
	PHALCON_CALL_METHODW(NULL, &models_manager, "setcustomeventsmanager", getThis(), events_manager);
}

/**
 * Returns the custom events manager
 *
 * @return Phalcon\Events\ManagerInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, getEventsManager){

	zval models_manager = {};

	phalcon_read_property(&models_manager, getThis(), SL("_modelsManager"), PH_NOISY);
	PHALCON_RETURN_CALL_METHOD(&models_manager, "getcustomeventsmanager", getThis());
}

/**
 * Returns the models meta-data service related to the entity instance
 *
 * @return Phalcon\Mvc\Model\MetaDataInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, getModelsMetaData){

	zval meta_data = {}, service = {}, service_name = {};

	phalcon_read_property(&meta_data, getThis(), SL("_modelsMetaData"), PH_NOISY);
	if (Z_TYPE(meta_data) == IS_OBJECT) {
		PHALCON_CPY_WRT(&service, &meta_data);
	} else {
		PHALCON_STR(&service_name, "modelsMetadata");

		PHALCON_CALL_METHODW(&service, getThis(), "getresolveservice", &service_name, &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_true));
		if (Z_TYPE(service) == IS_OBJECT) {
			PHALCON_VERIFY_INTERFACEW(&service, phalcon_mvc_model_metadatainterface_ce);
		} else {
			object_init_ex(&service, phalcon_mvc_model_metadata_memory_ce);
		}

		/**
		 * Update the models-metada property
		 */
		phalcon_update_property_zval(getThis(), SL("_modelsMetaData"), &service);
	}

	RETURN_CTORW(&service);
}

/**
 * Returns the models manager related to the entity instance
 *
 * @return Phalcon\Mvc\Model\ManagerInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, getModelsManager){

	zval service_name = {};

	phalcon_return_property(return_value, getThis(), SL("_modelsManager"));

	if (Z_TYPE_P(return_value) != IS_OBJECT) {
		PHALCON_STR(&service_name, "modelsManager");
		PHALCON_CALL_METHODW(return_value, getThis(), "getresolveservice", &service_name);
		phalcon_update_property_zval(getThis(), SL("_modelsManager"), return_value);
	}
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
		phalcon_update_property_zval(getThis(), SL("_transaction"), transaction);
		RETURN_THISW();
	}
	PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Transaction should be an object");
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

	PHALCON_CALL_METHODW(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_CALL_METHODW(NULL, &models_manager, "setmodelsource", getThis(), source);
	RETURN_THISW();
}

/**
 * Returns table name mapped in the model
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model, getSource){

	zval models_manager = {};

	PHALCON_CALL_METHODW(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_RETURN_CALL_METHODW(&models_manager, "getmodelsource", getThis());
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

	PHALCON_CALL_METHODW(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_CALL_METHODW(NULL, &models_manager, "setmodelschema", getThis(), schema);
	RETURN_THISW();
}

/**
 * Returns schema name where table mapped is located
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model, getSchema){

	zval models_manager = {};

	PHALCON_CALL_METHODW(&models_manager, getThis(), "getmodelsmanager");
	PHALCON_RETURN_CALL_METHOD(&models_manager, "getmodelschema", getThis());
}

/**
 * Returns the column map if any
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getColumnMap){

	zval column_map = {}, meta_data = {};

	/**
	 * Check if column renaming is globally activated
	 */
	if (likely(PHALCON_GLOBAL(orm).column_renaming)) {
		phalcon_return_property(&column_map, getThis(), SL("_columnMap"));

		if (!zend_is_true(&column_map)) {
			PHALCON_CALL_METHODW(&meta_data, getThis(), "getmodelsmetadata");
			PHALCON_CALL_METHODW(&column_map, &meta_data, "getcolumnmap", getThis());

			phalcon_update_property_zval(getThis(), SL("_columnMap"), &column_map);
		}
	}

	RETURN_CTORW(&column_map);
}

/**
 * Returns the reverse column map if any
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getReverseColumnMap){

	zval meta_data = {}, column_map = {};

	if (likely(PHALCON_GLOBAL(orm).column_renaming)) {
		PHALCON_CALL_METHODW(&meta_data, getThis(), "getmodelsmetadata");
		PHALCON_CALL_METHODW(&column_map, &meta_data, "getreversecolumnmap", getThis());
	}

	RETURN_CTORW(&column_map);
}

/**
 * Returns the columns
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getColumns){

	zval meta_data = {};

	PHALCON_CALL_METHODW(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_RETURN_CALL_METHOD(&meta_data, "getattributes", getThis());
}

/**
 * Returns the columns data types
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getDataTypes){

	zval meta_data = {};

	PHALCON_CALL_METHODW(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_RETURN_CALL_METHOD(&meta_data, "getdatatypes", getThis());
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

	phalcon_read_property(&models_manager, getThis(), SL("_modelsManager"), PH_NOISY);
	PHALCON_CALL_METHODW(NULL, &models_manager, "setconnectionservice", getThis(), connection_service);

	RETURN_THISW();
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

	phalcon_read_property(&models_manager, getThis(), SL("_modelsManager"), PH_NOISY);
	PHALCON_CALL_METHODW(NULL, &models_manager, "setreadconnectionservice", getThis(), connection_service);

	RETURN_THISW();
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

	phalcon_read_property(&models_manager, getThis(), SL("_modelsManager"), PH_NOISY);
	PHALCON_CALL_METHODW(NULL, &models_manager, "setwriteconnectionservice", getThis(), connection_service);

	RETURN_THISW();
}

/**
 * Returns the DependencyInjection connection service name used to read data related the model
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model, getReadConnectionService){

	zval models_manager = {};

	phalcon_read_property(&models_manager, getThis(), SL("_modelsManager"), PH_NOISY);
	PHALCON_RETURN_CALL_METHOD(&models_manager, "getreadconnectionservice", getThis());
}

/**
 * Returns the DependencyInjection connection service name used to write data related to the model
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model, getWriteConnectionService){

	zval models_manager = {};

	phalcon_read_property(&models_manager, getThis(), SL("_modelsManager"), PH_NOISY);
	PHALCON_RETURN_CALL_METHOD(&models_manager, "getwriteconnectionservice", getThis());
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

	phalcon_update_property_zval(getThis(), SL("_dirtyState"), dirty_state);

	RETURN_THISW();
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
 * @param array $intermediate
 * @param array $bindParams
 * @param array $bindTypes
 * @return Phalcon\Db\AdapterInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, getReadConnection){

	zval *intermediate = NULL, *bind_params = NULL, *bind_types = NULL, transaction = {}, connection = {}, models_manager = {};

	phalcon_fetch_params(0, 0, 3, &intermediate, &bind_params, &bind_types);

	if (!intermediate) {
		intermediate = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	phalcon_return_property(&transaction, getThis(), SL("_transaction"));
	if (Z_TYPE(transaction) == IS_OBJECT) {
		if (instanceof_function_ex(Z_OBJCE(transaction), phalcon_db_adapterinterface_ce, 1)) {
			RETURN_CTORW(&transaction);
		}

		PHALCON_RETURN_CALL_METHODW(&transaction, "getconnection");
		return;
	}

	if (phalcon_method_exists_ex(getThis(), SL("selectreadconnection")) == SUCCESS) {
		PHALCON_CALL_METHODW(&connection, getThis(), "selectreadconnection", intermediate, bind_params, bind_types);
		if (Z_TYPE(connection) != IS_OBJECT) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "'selectReadConnection' didn't returned a valid connection");
			return;
		}

		RETURN_CTORW(&connection);
	}

	phalcon_read_property(&models_manager, getThis(), SL("_modelsManager"), PH_NOISY);
	PHALCON_RETURN_CALL_METHODW(&models_manager, "getreadconnection", getThis());
}

/**
 * Gets the connection used to write data to the model
 *
 * @param array $intermediate
 * @param array $bindParams
 * @param array $bindTypes
 * @return Phalcon\Db\AdapterInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, getWriteConnection){

	zval *intermediate = NULL, *bind_params = NULL, *bind_types = NULL, transaction = {}, connection = {}, models_manager = {};

	phalcon_fetch_params(0, 0, 3, &intermediate, &bind_params, &bind_types);

	if (!intermediate) {
		intermediate = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	phalcon_return_property(&transaction, getThis(), SL("_transaction"));

	if (Z_TYPE(transaction) == IS_OBJECT) {
		if (instanceof_function_ex(Z_OBJCE(transaction), phalcon_db_adapterinterface_ce, 1)) {
			RETURN_CTORW(&transaction);
		}

		PHALCON_RETURN_CALL_METHODW(&transaction, "getconnection");
		return;
	}

	if (phalcon_method_exists_ex(getThis(), SL("selectwriteconnection")) == SUCCESS) {
		PHALCON_CALL_METHODW(&connection, getThis(), "selectwriteconnection", intermediate, bind_params, bind_types);
		if (Z_TYPE(connection) != IS_OBJECT) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "'selectWriteConnection' didn't returned a valid connection");
			return;
		}

		RETURN_CTORW(&connection);
	}

	phalcon_return_property(&models_manager, getThis(), SL("_modelsManager"));
	PHALCON_RETURN_CALL_METHODW(&models_manager, "getwriteconnection", getThis());
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
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, assign){

	zval *data, *column_map = NULL, *value, exception_message = {};
	zend_string *str_key;

	phalcon_fetch_params(0, 1, 1, &data, &column_map);

	if (!column_map) {
		column_map = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(data) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Data to dump in the object must be an Array");
		return;
	}

	if (Z_TYPE_P(column_map) == IS_ARRAY) {
		ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(data), str_key, value) {
			zval key = {}, attribute = {};
			if (str_key) {
				ZVAL_STR(&key, str_key);
				/**
				 * Every field must be part of the column map
				 */
				if (phalcon_array_isset_fetch(&attribute, column_map, &key, 0)) {
					phalcon_update_property_zval_zval(getThis(), &attribute, value);
				} else {
					PHALCON_CONCAT_SVS(&exception_message, "Column \"", &key, "\" doesn't make part of the column map");
					PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}					
			}
		} ZEND_HASH_FOREACH_END();
	} else {
		ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(data), str_key, value) {
			zval key = {};
			if (str_key) {
				ZVAL_STR(&key, str_key);
				phalcon_update_property_zval_zval(getThis(), &key, value);
			}
		} ZEND_HASH_FOREACH_END();
	}
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

	zval *base, *data, *column_map, *dirty_state = NULL, *keep_snapshots = NULL, *source_model = NULL, data_types = {}, object = {}, *value, exception_message = {};
	zend_string *str_key;

	phalcon_fetch_params(0, 3, 3, &base, &data, &column_map, &dirty_state, &keep_snapshots, &source_model);

	if (!dirty_state) {
		dirty_state = &PHALCON_GLOBAL(z_zero);
	}

	if (!keep_snapshots) {
		keep_snapshots = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(data) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Data to dump in the object must be an Array");
		return;
	}

	if (source_model && Z_TYPE_P(source_model) == IS_OBJECT) {
		PHALCON_CALL_METHODW(&data_types, source_model, "getdatatypes");
	}

	if (phalcon_clone(&object, base) == FAILURE) {
		return;
	}

	/**
	 * Change the dirty state to persistent
	 */
	PHALCON_CALL_METHODW(NULL, &object, "setdirtystate", dirty_state);

	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(data), str_key, value) {
		zval key = {}, field_type = {}, convert_value = {}, attribute = {};
		if (str_key) {
			ZVAL_STR(&key, str_key);
			if (PHALCON_GLOBAL(orm).enable_auto_convert && zend_is_true(&data_types)) {
				if (phalcon_array_isset_fetch(&field_type, &data_types, &key, 0)) {
					if (phalcon_is_equal_long(&field_type, PHALCON_DB_COLUMN_TYPE_JSON)) {
						RETURN_ON_FAILURE(phalcon_json_decode(&convert_value, value, 0));
					} else {
						PHALCON_CPY_WRT(&convert_value, value);
					}
				} else {
					PHALCON_CPY_WRT(&convert_value, value);
				}
			} else {
				PHALCON_CPY_WRT(&convert_value, value);
			}

			/**
			 * Only string keys in the data are valid
			 */
			if (Z_TYPE_P(column_map) == IS_ARRAY) {
				/**
				 * Every field must be part of the column map
				 */
				if (phalcon_array_isset(column_map, &key)) {
					phalcon_array_fetch(&attribute, column_map, &key, PH_NOISY);
					phalcon_update_property_zval_zval(&object, &attribute, &convert_value);
				} else {
					PHALCON_CONCAT_SVS(&exception_message, "Column \"", &key, "\" doesn't make part of the column map");
					PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}
			} else {
				phalcon_update_property_zval_zval(&object, &key, &convert_value);
			}
		}
	} ZEND_HASH_FOREACH_END();

	if (zend_is_true(keep_snapshots)) {
		PHALCON_CALL_METHODW(NULL, &object, "setsnapshotdata", data, column_map);
		PHALCON_CALL_METHODW(NULL, &object, "build");
	}

	/**
	 * Call afterFetch, this allows the developer to execute actions after a record is
	 * fetched from the database
	 */
	if (phalcon_method_exists_ex(&object, SL("afterfetch")) == SUCCESS) {
		PHALCON_CALL_METHODW(NULL, &object, "afterfetch");
	}

	RETURN_CTORW(&object);
}

/**
 * Returns an hydrated result based on the data and the column map
 *
 * @param Phalcon\Mvc\Model $sourceModel
 * @param array $data
 * @param array $columnMap
 * @param int $hydrationMode
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Model, cloneResultMapHydrate){

	zval *data, *column_map, *hydration_mode, *source_model = NULL, hydrate = {}, data_types = {}, *value, exception_message = {};
	zend_string *str_key;

	phalcon_fetch_params(0, 3, 1, &data, &column_map, &hydration_mode, &source_model);

	if (Z_TYPE_P(data) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Data to hidrate must be an Array");
		return;
	}

	/**
	 * If there is no column map and the hydration mode is arrays return the data as it
	 * is
	 */
	if (Z_TYPE_P(column_map) != IS_ARRAY) { 
		if (PHALCON_IS_LONG(hydration_mode, 1)) {
			RETURN_CTORW(data);
		}
	}

	/**
	 * Create the destination object according to the hydration mode
	 */
	if (PHALCON_IS_LONG(hydration_mode, 1)) {
		array_init(&hydrate);
	} else {
		object_init(&hydrate);
	}

	if (source_model && Z_TYPE_P(source_model) == IS_OBJECT) {
		PHALCON_CALL_METHODW(&data_types, source_model, "getdatatypes");
	}

	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(data), str_key, value) {
		zval key = {}, field_type = {}, convert_value = {}, attribute = {};
		if (str_key) {
			ZVAL_STR(&key, str_key);

			if (PHALCON_GLOBAL(orm).enable_auto_convert && Z_TYPE(data_types) == IS_ARRAY) {
				if (phalcon_array_isset_fetch(&field_type, &data_types, &key, 0)) {
					if (phalcon_is_equal_long(&field_type, PHALCON_DB_COLUMN_TYPE_JSON)) {
						RETURN_ON_FAILURE(phalcon_json_decode(&convert_value, value, 1));
					} else {
						PHALCON_CPY_WRT(&convert_value, value);
					}
				} else {
					PHALCON_CPY_WRT(&convert_value, value);
				}
			} else {
				PHALCON_CPY_WRT(&convert_value, value);
			}

			if (Z_TYPE_P(column_map) == IS_ARRAY) {
				/**
				 * Every field must be part of the column map
				 */
				if (!phalcon_array_isset_fetch(&attribute, column_map, &key, 0)) {
					PHALCON_CONCAT_SVS(&exception_message, "Column \"", &key, "\" doesn't make part of the column map");
					PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}

				if (PHALCON_IS_LONG(hydration_mode, 1)) {
					phalcon_array_update_zval(&hydrate, &attribute, &convert_value, PH_COPY);
				} else {
					phalcon_update_property_zval_zval(&hydrate, &attribute, &convert_value);
				}
			} else {
				if (PHALCON_IS_LONG(hydration_mode, 1)) {
					phalcon_array_update_zval(&hydrate, &key, &convert_value, PH_COPY);
				} else {
					phalcon_update_property_zval_zval(&hydrate, &key, &convert_value);
				}
			}
		}
	} ZEND_HASH_FOREACH_END();

	RETURN_CTORW(&hydrate);
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

	zval *base, *data, *dirty_state = NULL, object = {}, *value = NULL;
	zend_string *str_key;

	phalcon_fetch_params(0, 2, 1, &base, &data, &dirty_state);

	if (!dirty_state) {
		dirty_state = &PHALCON_GLOBAL(z_zero);
	}

	if (Z_TYPE_P(data) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Data to dump in the object must be an Array");
		return;
	}

	/**
	 * Clone the base record
	 */
	if (phalcon_clone(&object, base) == FAILURE) {
		return;
	}

	/**
	 * Mark the object as persistent
	 */
	PHALCON_CALL_METHODW(NULL, &object, "setdirtystate", dirty_state);
	
	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(data), str_key, value) {
		if (!str_key) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Invalid key in array data provided to dumpResult()");
			return;
		}
		phalcon_update_property_string_zval(&object, str_key, value);
	} ZEND_HASH_FOREACH_END();

	/**
	 * Call afterFetch, this allows the developer to execute actions after a record is
	 * fetched from the database
	 */
	if (phalcon_method_exists_ex(&object, SL("afterfetch")) == SUCCESS) {
		PHALCON_CALL_METHODW(NULL, &object, "afterfetch");
	}

	RETURN_CTORW(&object);
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
 * //Get first 100 virtual robots ordered by name
 * $robots = Robots::find(array("type='virtual'", "order" => "name", "limit" => 100));
 * foreach ($robots as $robot) {
 *	   echo $robot->name, "\n";
 * }
 * </code>
 *
 * @param 	array $parameters
 * @return  Phalcon\Mvc\Model\ResultsetInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, find){

	zval *parameters = NULL, model_name = {}, dependency_injector = {}, service_name = {}, manager = {}, model = {};
	zval params = {}, has = {}, service_params = {}, builder = {}, event_name = {}, query = {}, cache = {}, resultset = {}, hydration = {};

	phalcon_fetch_params(0, 0, 1, &parameters);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	phalcon_get_called_class(&model_name);

	PHALCON_CALL_CE_STATICW(&dependency_injector, phalcon_di_ce, "getdefault");

	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
		return;
	}

	PHALCON_STR(&service_name, "modelsManager");

	PHALCON_CALL_METHODW(&manager, &dependency_injector, "getshared", &service_name);
	PHALCON_CALL_METHODW(&model, &manager, "load", &model_name);

	if (Z_TYPE_P(parameters) != IS_ARRAY) { 
		array_init(&params);
		if (Z_TYPE_P(parameters) != IS_NULL) {
			phalcon_array_append(&params, parameters, PH_COPY);
		}
	} else {
		PHALCON_CPY_WRT(&params, parameters);
	}

	/**
	 * Builds a query with the passed parameters
	 */
	PHALCON_STR(&service_name, "modelsQueryBuilder");

	PHALCON_CALL_METHODW(&has, &dependency_injector, "has", &service_name);
	if (zend_is_true(&has)) {
		array_init(&service_params);

		phalcon_array_append(&service_params, &params, PH_COPY);

		PHALCON_CALL_METHODW(&builder, &dependency_injector, "get", &service_name, &service_params);
	} else {
		object_init_ex(&builder, phalcon_mvc_model_query_builder_ce);
		PHALCON_CALL_METHODW(NULL, &builder, "__construct", &params);
	}

	PHALCON_CALL_METHODW(NULL, &builder, "from", &model_name);

	PHALCON_STR(&event_name, "beforeQuery");

	ZVAL_MAKE_REF(&builder);
	PHALCON_CALL_METHODW(NULL, &model, "fireevent", &event_name, &builder);
	ZVAL_UNREF(&builder);

	PHALCON_CALL_METHODW(&query, &builder, "getquery");

	/**
	 * Pass the cache options to the query
	 */
	if (phalcon_array_isset_fetch_str(&cache, &params, SL("cache"))) {
		PHALCON_CALL_METHODW(NULL, &query, "cache", &cache);
	}

	/**
	 * Execute the query passing the bind-params and casting-types
	 */
	PHALCON_CALL_METHODW(&resultset, &query, "execute");

	/**
	 * Define an hydration mode
	 */
	if (Z_TYPE(resultset) == IS_OBJECT) {
		if (phalcon_array_isset_fetch_str(&hydration, &params, SL("hydration"))) {
			PHALCON_CALL_METHODW(NULL, &resultset, "sethydratemode", &hydration);
		}

		PHALCON_STR(&event_name, "afterQuery");

		ZVAL_MAKE_REF(&resultset);
		PHALCON_CALL_METHODW(NULL, &model, "fireevent", &event_name, &resultset);
		ZVAL_UNREF(&resultset);
	}

	RETURN_CTORW(&resultset);
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
 * @param array $parameters
 * @param bool $autoCreate
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, findFirst){

	zval *parameters = NULL, *auto_create = NULL, params = {}, dependency_injector = {}, model_name = {}, service_name = {}, has = {}, manager = {};
	zval model = {}, service_params = {}, builder = {}, query = {}, cache = {}, event_name = {}, result = {}, hydration = {};

	phalcon_fetch_params(0, 0, 2, &parameters, &auto_create);

	if (parameters) {
		if (Z_TYPE_P(parameters) != IS_ARRAY) {
			array_init(&params);
			if (Z_TYPE_P(parameters) != IS_NULL) {
				phalcon_array_append(&params, parameters, PH_COPY);
			}
		} else {
			PHALCON_CPY_WRT(&params, parameters);
		}
	} else {
		ZVAL_NULL(&params);
	}

	if (!auto_create) {
		auto_create = &PHALCON_GLOBAL(z_false);
	}


	PHALCON_CALL_CE_STATICW(&dependency_injector, phalcon_di_ce, "getdefault");

	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
		return;
	}

	phalcon_get_called_class(&model_name);

	PHALCON_STR(&service_name, "modelsManager");

	PHALCON_CALL_METHODW(&has, &dependency_injector, "has", &service_name);
	if (zend_is_true(&has)) {
		PHALCON_CALL_METHODW(&manager, &dependency_injector, "getshared", &service_name);
	} else {
		object_init_ex(&manager, phalcon_mvc_model_manager_ce);
	}

	PHALCON_CALL_METHODW(&model, &manager, "load", &model_name, &PHALCON_GLOBAL(z_true));

	/**
	 * Builds a query with the passed parameters
	 */
	PHALCON_STR(&service_name, "modelsQueryBuilder");

	PHALCON_CALL_METHODW(&has, &dependency_injector, "has", &service_name);
	if (zend_is_true(&has)) {
		array_init(&service_params);
		phalcon_array_append(&service_params, &params, PH_COPY);
		PHALCON_CALL_METHODW(&builder, &dependency_injector, "get", &service_name, &service_params);
	} else {
		object_init_ex(&builder, phalcon_mvc_model_query_builder_ce);
		PHALCON_CALL_METHODW(NULL, &builder, "__construct", &params);
	}

	PHALCON_CALL_METHODW(NULL, &builder, "from", &model_name);

	PHALCON_STR(&event_name, "beforeQuery");

	ZVAL_MAKE_REF(&builder);
	PHALCON_CALL_METHODW(NULL, &model, "fireevent", &event_name, &builder);
	ZVAL_UNREF(&builder);

	/**
	 * We only want the first record
	 */
	PHALCON_CALL_METHODW(NULL, &builder, "limit", &PHALCON_GLOBAL(z_one));
	PHALCON_CALL_METHODW(&query, &builder, "getquery");

	/**
	 * Pass the cache options to the query
	 */
	if (phalcon_array_isset_fetch_str(&cache, &params, SL("cache"))) {
		PHALCON_CALL_METHODW(NULL, &query, "cache", &cache);
	}

	/**
	 * Return only the first row
	 */
	PHALCON_CALL_METHODW(NULL, &query, "setuniquerow", &PHALCON_GLOBAL(z_true));

	/**
	 * Execute the query passing the bind-params and casting-types
	 */
	PHALCON_CALL_METHODW(&result, &query, "execute");

	if (zend_is_true(&result)) {
		PHALCON_STR(&event_name, "afterQuery");

		ZVAL_MAKE_REF(&result);
		PHALCON_CALL_METHODW(NULL, &model, "fireevent", &event_name, &result);
		ZVAL_UNREF(&result);

		/**
		 * Define an hydration mode
		 */
		if (phalcon_array_isset_fetch_str(&hydration, &params, SL("hydration"))) {
			PHALCON_CALL_METHODW(NULL, &result, "sethydratemode", &hydration);
		}

		RETURN_CTORW(&result);
	} else if (zend_is_true(auto_create)) {
		RETURN_CTORW(&model);
	}

	RETURN_FALSE;
}

/**
 * Create a criteria for a specific model
 *
 * @param Phalcon\DiInterface $dependencyInjector
 * @return Phalcon\Mvc\Model\Criteria
 */
PHP_METHOD(Phalcon_Mvc_Model, query){

	zval *di = NULL, dependency_injector = {}, model_name = {}, service_name = {}, has = {}, criteria = {};

	phalcon_fetch_params(0, 0, 1, &di);

	if (!di || Z_TYPE_P(di) != IS_OBJECT) {
		/**
		 * Use the global dependency injector if there is no one defined
		 */
		PHALCON_CALL_CE_STATICW(&dependency_injector, phalcon_di_ce, "getdefault");

		if (Z_TYPE(dependency_injector) != IS_OBJECT) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
			return;
		}
	} else {
		PHALCON_CPY_WRT(&dependency_injector, di);
	}

	phalcon_get_called_class(&model_name);

	PHALCON_STR(&service_name, "modelsCriteria");

	PHALCON_CALL_METHODW(&has, &dependency_injector, "has", &service_name);
	if (zend_is_true(&has)) {
		PHALCON_CALL_METHODW(&criteria, &dependency_injector, "get", &service_name);
	} else {
		object_init_ex(&criteria, phalcon_mvc_model_criteria_ce);
	}

	PHALCON_CALL_METHODW(NULL, &criteria, "setdi", &dependency_injector);
	PHALCON_CALL_METHODW(NULL, &criteria, "setmodelname", &model_name);

	RETURN_CTORW(&criteria);
}

/**
 * Builds a unique primary key condition
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, build){

	zval meta_data = {}, read_connection = {};

	PHALCON_CALL_METHODW(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_CALL_METHODW(&read_connection, getThis(), "getreadconnection");

	PHALCON_RETURN_CALL_METHODW(getThis(), "_rebuild", &meta_data, &read_connection);
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
 * Builds a unique primary key condition
 *
 * @param Phalcon\Mvc\Model\MetadataInterface $metaData
 * @param Phalcon\Db\AdapterInterface $connection
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _reBuild){

	zval *meta_data, *connection, unique_params = {}, unique_key = {}, primary_keys = {}, bind_data_types = {}, number_primary = {}, column_map = {};
	zval unique_types = {}, number_empty = {}, where_pk = {}, *field, exception_message = {}, join_where = {};

	phalcon_fetch_params(0, 2, 0, &meta_data, &connection);

	phalcon_update_property_long(getThis(), SL("_seenRawvalues"), 0);

	/**
	 * Builds a unique primary key condition
	 */
	phalcon_return_property(&unique_params, getThis(), SL("_uniqueParams"));
	phalcon_return_property(&unique_key, getThis(), SL("_uniqueKey"));

	if (Z_TYPE(unique_key) == IS_NULL || Z_TYPE(unique_params) == IS_NULL) {
		PHALCON_CALL_METHODW(&primary_keys, meta_data, "getprimarykeyattributes", getThis());
		PHALCON_CALL_METHODW(&bind_data_types, meta_data, "getbindtypes", getThis());

		phalcon_fast_count(&number_primary, &primary_keys);
		if (!zend_is_true(&number_primary)) {
			RETURN_FALSE;
		}

		PHALCON_CALL_SELFW(&column_map, "getcolumnmap");

		ZVAL_LONG(&number_empty, 0);
		array_init(&where_pk);
		array_init(&unique_params);
		array_init(&unique_types);

		/**
		 * We need to create a primary key based on the current data
		 */
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(primary_keys), field) {
			zval attribute_field = {}, value = {}, v = {}, escaped_field = {}, pk_condition = {}, type = {};
			if (Z_TYPE(column_map) == IS_ARRAY) { 
				if (!phalcon_array_isset_fetch(&attribute_field, &column_map, field, 0)) {
					PHALCON_CONCAT_SVS(&exception_message, "Column '", field, "' isn't part of the column map");
					PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}
			} else {
				PHALCON_CPY_WRT(&attribute_field, field);
			}

			/**
			 * If the primary key attribute is set append it to the conditions
			 */
			if (phalcon_isset_property_zval(getThis(), &attribute_field)) {
				phalcon_read_property_zval(&value, getThis(), &attribute_field, PH_NOISY);

				/**
				 * We count how many fields are empty, if all fields are empty we don't perform an
				 * 'exist' check
				 */
				if (PHALCON_IS_EMPTY(&value)) {
					phalcon_increment(&number_empty);
				}

				PHALCON_CPY_WRT(&v, &value);
			} else {
				phalcon_increment(&number_empty);
				ZVAL_NULL(&v);
			}

			PHALCON_CALL_METHODW(&escaped_field, connection, "escapeidentifier", field);

			if (Z_TYPE(v) == IS_OBJECT && instanceof_function(Z_OBJCE(v), phalcon_db_rawvalue_ce)) {
				PHALCON_CONCAT_VSV(&pk_condition, &escaped_field, " = ", &v);
				phalcon_update_property_long(getThis(), SL("_seenRawvalues"), 1);
			} else {
				PHALCON_CONCAT_VS(&pk_condition, &escaped_field, " = ?");
				phalcon_array_append(&unique_params, &v, PH_COPY);
				ZVAL_NULL(&v);
			}

			if (!phalcon_array_isset(&bind_data_types, field)) {
				PHALCON_CONCAT_SVS(&exception_message, "Column '", field, "' isn't a part of the table columns");
				PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
				return;
			}

			if (Z_TYPE(v) > IS_NULL) {
				phalcon_array_fetch(&type, &bind_data_types, field, PH_NOISY);
				phalcon_array_append(&unique_types, &type, PH_COPY);
			}

			phalcon_array_append(&where_pk, &pk_condition, PH_COPY);
		} ZEND_HASH_FOREACH_END();

		/**
		 * There are no primary key fields defined, assume the record does not exist
		 */
		if (PHALCON_IS_EQUAL(&number_primary, &number_empty)) {
			RETURN_FALSE;
		}

		phalcon_fast_join_str(&join_where, SL(" AND "), &where_pk);

		/**
		 * The unique key is composed of 3 parts _uniqueKey, uniqueParams, uniqueTypes
		 */
		phalcon_update_property_zval(getThis(), SL("_uniqueKey"), &join_where);
		phalcon_update_property_zval(getThis(), SL("_uniqueParams"), &unique_params);
		phalcon_update_property_zval(getThis(), SL("_uniqueTypes"), &unique_types);
	}

	RETURN_TRUE;
}

/**
 * Checks if the current record already exists or not
 *
 * @param Phalcon\Mvc\Model\MetadataInterface $metaData
 * @param Phalcon\Db\AdapterInterface $connection
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _exists){

	zval *meta_data, *connection, table = {}, dirty_state = {}, unique_key = {}, build = {}, seen_rawvalues = {}, unique_params = {}, unique_types = {};
	zval schema = {}, source = {}, escaped_table = {}, select = {}, row = {}, column_map = {};

	phalcon_fetch_params(0, 2, 0, &meta_data, &connection);

	/**
	 * If we already know if the record exists we don't check it
	 */
	phalcon_read_property(&dirty_state, getThis(), SL("_dirtyState"), PH_NOISY);

	/**
	 * Builds a unique primary key condition
	 */
	phalcon_read_property(&unique_key, getThis(), SL("_uniqueKey"), PH_NOISY);

	if (Z_TYPE(unique_key) == IS_NULL) {
		ZVAL_TRUE(&dirty_state);

		PHALCON_CALL_METHODW(&build, getThis(), "_rebuild", meta_data, connection);

		if (!PHALCON_IS_TRUE(&build)) {
			RETURN_FALSE;
		}

		phalcon_read_property(&unique_key, getThis(), SL("_uniqueKey"), PH_NOISY);
	}

	if (!zend_is_true(&dirty_state)) {
		RETURN_TRUE;
	}

	phalcon_read_property(&seen_rawvalues, getThis(), SL("_seenRawvalues"), PH_NOISY);
	phalcon_read_property(&unique_params, getThis(), SL("_uniqueParams"), PH_NOISY);
	phalcon_read_property(&unique_types, getThis(), SL("_uniqueTypes"), PH_NOISY);

	PHALCON_CALL_METHODW(&schema, getThis(), "getschema");
	PHALCON_CALL_METHODW(&source, getThis(), "getsource");

	if (zend_is_true(&schema)) {
		array_init_size(&table, 2);
		phalcon_array_append(&table, &schema, PH_COPY);
		phalcon_array_append(&table, &source, PH_COPY);
	} else {
		PHALCON_CPY_WRT(&table, &source);
	}

	PHALCON_CALL_METHODW(&escaped_table, connection, "escapeidentifier", &table);

	/**
	 * Here we use a single COUNT(*) without PHQL to make the execution faster
	 */
	PHALCON_CONCAT_SVSVS(&select, "SELECT * FROM ", &escaped_table, " WHERE ", &unique_key, " LIMIT 1");
	PHALCON_CALL_METHODW(&row, connection, "fetchone", &select, &PHALCON_GLOBAL(z_null), &unique_params, &unique_types);

	if (zend_is_true(&seen_rawvalues)) {
		phalcon_update_property_zval(getThis(), SL("_uniqueKey"), &PHALCON_GLOBAL(z_null));
	}

	if (Z_TYPE(row) == IS_ARRAY && phalcon_fast_count_ev(&row)) {
		phalcon_update_property_long(getThis(), SL("_dirtyState"), 0);
		PHALCON_CALL_SELFW(&column_map, "getcolumnmap");
		PHALCON_CALL_METHODW(NULL, getThis(), "setsnapshotdata", &row, &column_map);
		RETURN_TRUE;
	}

	phalcon_update_property_long(getThis(), SL("_dirtyState"), 1);

	RETURN_FALSE;
}

/**
 * Generate a PHQL SELECT statement for an aggregate
 *
 * @param string $function
 * @param string $alias
 * @param array $parameters
 * @return Phalcon\Mvc\Model\ResultsetInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, _groupResult){

	zval *function, *alias, *parameters, params = {}, group_column = {}, distinct_column = {}, columns = {}, group_columns = {};
	zval model_name = {}, dependency_injector = {}, service_name = {}, manager = {}, model = {}, has = {}, service_params = {}, builder = {}, query = {};
	zval cache = {}, resultset = {}, number_rows = {}, first_row = {};

	phalcon_fetch_params(0, 3, 0, &function, &alias, &parameters);

	if (Z_TYPE_P(parameters) != IS_ARRAY) {
		if (Z_TYPE_P(parameters) != IS_NULL) {
			array_init_size(&params, 1);
			phalcon_array_append(&params, parameters, PH_COPY);
		} else {
			array_init(&params);
		}
	} else {
		PHALCON_CPY_WRT(&params, parameters);
	}

	if (!phalcon_array_isset_fetch_str(&group_column, &params, SL("column"))) {
		PHALCON_STR(&group_column, "*");
	}

	/**
	 * Builds the columns to query according to the received parameters
	 */
	if (phalcon_array_isset_fetch_str(&distinct_column, &params, SL("distinct"))) {
		PHALCON_CONCAT_VSVSV(&columns, function, "(DISTINCT ", &distinct_column, ") AS ", alias);
	} else {
		if (phalcon_array_isset_fetch_str(&group_columns, &params, SL("group"))) {
			PHALCON_CONCAT_VSVSVSV(&columns, &group_columns, ", ", function, "(", &group_column, ") AS ", alias);
		} else {
			PHALCON_CONCAT_VSVSV(&columns, function, "(", &group_column, ") AS ", alias);
		}
	}

	phalcon_get_called_class(&model_name);

	PHALCON_CALL_CE_STATICW(&dependency_injector, phalcon_di_ce, "getdefault");

	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
		return;
	}

	PHALCON_STR(&service_name, "modelsManager");

	PHALCON_CALL_METHODW(&manager, &dependency_injector, "getshared", &service_name);

	PHALCON_CALL_METHODW(&model, &manager, "load", &model_name);

	/**
	 * Builds a query with the passed parameters
	 */
	PHALCON_STR(&service_name, "modelsQueryBuilder");

	PHALCON_CALL_METHODW(&has, &dependency_injector, "has", &service_name);
	if (zend_is_true(&has)) {
		array_init(&service_params);

		phalcon_array_append(&service_params, &params, PH_COPY);

		PHALCON_CALL_METHODW(&builder, &dependency_injector, "get", &service_name, &service_params);
	} else {
		object_init_ex(&builder, phalcon_mvc_model_query_builder_ce);
		PHALCON_CALL_METHODW(NULL, &builder, "__construct", &params);
	}

	PHALCON_CALL_METHODW(NULL, &builder, "columns", &columns);
	PHALCON_CALL_METHODW(NULL, &builder, "from", &model_name);

	if (phalcon_method_exists_ex(&model, SL("beforequery")) == SUCCESS) {
		PHALCON_CALL_METHODW(NULL, &model, "beforequery", &builder);
	}

	PHALCON_CALL_METHODW(&query, &builder, "getquery");

	/**
	 * Pass the cache options to the query
	 */
	if (phalcon_array_isset_fetch_str(&cache, &params, SL("cache"))) {
		PHALCON_CALL_METHODW(NULL, &query, "cache", &cache);
	}

	/**
	 * Execute the query
	 */
	PHALCON_CALL_METHODW(&resultset, &query, "execute");

	/**
	 * Return the full resultset if the query is grouped
	 */
	if (phalcon_array_isset_str(&params, SL("group"))) {
		RETURN_CTOR(&resultset);
	}

	/**
	 * Return only the value in the first result
	 */
	phalcon_fast_count(&number_rows, &resultset);
	PHALCON_CALL_METHODW(&first_row, &resultset, "getfirst");

	phalcon_return_property_zval(return_value, &first_row, alias);
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

	phalcon_fetch_params(0, 0, 1, &parameters);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_STR(&function, "COUNT");
	PHALCON_STR(&alias, "rowcount");

	PHALCON_RETURN_CALL_SELFW("_groupresult", &function, &alias, parameters);
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

	PHALCON_STR(&function, "SUM");
	PHALCON_STR(&alias, "sumatory");

	PHALCON_RETURN_CALL_SELFW("_groupresult", &function, &alias, parameters);
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

	PHALCON_STR(&function, "MAX");
	PHALCON_STR(&alias, "maximum");

	PHALCON_RETURN_CALL_SELFW("_groupresult", &function, &alias, parameters);
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

	PHALCON_STR(&function, "MIN");
	PHALCON_STR(&alias, "minimum");

	PHALCON_RETURN_CALL_SELFW("_groupresult", &function, &alias, parameters);
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

	PHALCON_STR(&function, "AVG");
	PHALCON_STR(&alias, "average");

	PHALCON_RETURN_CALL_SELFW("_groupresult", &function, &alias, parameters);
}

/**
 * Fires an event, implicitly calls behaviors and listeners in the events manager are notified
 *
 * @param string $eventName
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, fireEvent){

	zval *eventname, *data = NULL, *cancelable = NULL, models_manager = {}, lower = {};

	phalcon_fetch_params(0, 1, 2, &eventname, &data, &cancelable);
	PHALCON_ENSURE_IS_STRING(eventname);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!cancelable) {
		cancelable = &PHALCON_GLOBAL(z_null);
	}

	if (likely(PHALCON_GLOBAL(orm).events)) {
		phalcon_fast_strtolower(&lower, eventname);

		/**
		 * Check if there is a method with the same name of the event
		 */
		if (phalcon_method_exists(getThis(), &lower) == SUCCESS) {
			PHALCON_CALL_METHODW(NULL, getThis(), Z_STRVAL(lower), data);
		}


		phalcon_read_property(&models_manager, getThis(), SL("_modelsManager"), PH_NOISY);

		/**
		 * Send a notification to the events manager
		 */
		PHALCON_RETURN_CALL_METHODW(&models_manager, "notifyevent", eventname, getThis());
	}
}

/**
 * Fires an event, implicitly calls behaviors and listeners in the events manager are notified
 * This method stops if one of the callbacks/listeners returns boolean false
 *
 * @param string $eventName
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, fireEventCancel){

	zval *eventname, *data = NULL, *cancelable = NULL, lower = {}, status = {}, models_manager = {};

	phalcon_fetch_params(0, 1, 2, &eventname, &data, &cancelable);
	PHALCON_ENSURE_IS_STRING(eventname);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!cancelable) {
		cancelable = &PHALCON_GLOBAL(z_null);
	}

	if (likely(PHALCON_GLOBAL(orm).events)) {
		phalcon_fast_strtolower(&lower, eventname);

		/**
		 * Check if there is a method with the same name of the event
		 */
		if (phalcon_method_exists(getThis(), &lower) == SUCCESS) {
			PHALCON_CALL_METHODW(&status, getThis(), Z_STRVAL(lower), data);
			if (PHALCON_IS_FALSE(&status)) {
				RETURN_FALSE;
			}
		}

		phalcon_read_property(&models_manager, getThis(), SL("_modelsManager"), PH_NOISY);

		/**
		 * Send a notification to the events manager
		 */
		PHALCON_CALL_METHODW(&status, &models_manager, "notifyevent", eventname, getThis());
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}
	}

	RETURN_TRUE;
}

/**
 * Cancel the current operation
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _cancelOperation){

	zval operation_made = {}, event_name = {};

	phalcon_read_property(&operation_made, getThis(), SL("_operationMade"), PH_NOISY);

	if (PHALCON_IS_LONG(&operation_made, 3)) {
		PHALCON_STR(&event_name, "notDeleted");
	} else {
		PHALCON_STR(&event_name, "notSaved");
	}

	PHALCON_CALL_METHODW(NULL, getThis(), "fireevent", &event_name);
}

/**
 * Appends a customized message on the validation process
 *
 * <code>
 * use \Phalcon\Mvc\Model\Message as Message;
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
 * @param Phalcon\Mvc\Model\MessageInterface $message
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, appendMessage){

	zval *message, *field = NULL, *t = NULL, *code = NULL, type = {}, custom_message = {}, exception_message = {}, model_message = {};
	zval message_message = {}, message_field = {}, message_type = {}, message_code = {}; 

	phalcon_fetch_params(0, 1, 3, &message, &field, &type, &code);

	if (!field) {
		field = &PHALCON_GLOBAL(z_null);
	}

	if (t) {
		PHALCON_CPY_WRT(&type, t);
	}

	if (!code) {
		code = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(message) != IS_OBJECT) {
		if (PHALCON_IS_EMPTY(field) || PHALCON_IS_EMPTY(&type)) {
			PHALCON_STR(&type, zend_zval_type_name(message));

			PHALCON_CONCAT_SVS(&exception_message, "Invalid message format '", &type, "'");
			PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
			return;
		}

		if (phalcon_method_exists_ex(getThis(), SL("messages")) == SUCCESS) {
			PHALCON_CALL_METHODW(&custom_message, getThis(), "messages", message, field, &type, code);
		} else {
			PHALCON_CPY_WRT(&custom_message, message);
		}

		object_init_ex(&model_message, phalcon_mvc_model_message_ce);
		PHALCON_CALL_METHODW(NULL, &model_message, "__construct", &custom_message, field, &type, code);

		phalcon_update_property_array_append(getThis(), SL("_errorMessages"), &model_message);
	} else {
		if (phalcon_method_exists_ex(getThis(), SL("messages")) == SUCCESS) {
			PHALCON_CALL_METHODW(&message_message, message, "getmessage");
			PHALCON_CALL_METHODW(&message_field, message, "getfield");
			PHALCON_CALL_METHODW(&message_type, message, "gettype");
			PHALCON_CALL_METHODW(&message_code, message, "getcode");
			PHALCON_CALL_METHODW(&custom_message, getThis(), "messages", &message_message, &message_field, &message_type, &message_code);

			PHALCON_CALL_METHODW(NULL, message, "setmessage", &custom_message);
		}

		phalcon_update_property_array_append(getThis(), SL("_errorMessages"), message);
	}

	RETURN_THISW();
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
 * @param array|Phalcon\Mvc\Model\ValidatorInterface $validator
 * @param boolean $allow_empty
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, validate){

	zval *validator, *allow_empty = NULL, *ex = NULL, field = {}, handler = {}, value = {}, arguments = {}, status = {};
	zval message_str = {}, pairs = {}, message = {}, type = {}, code = {}, option = {}, messages = {}, errors = {}, new_errors = {};

	phalcon_fetch_params(0, 1, 2, &validator, &allow_empty, &ex);

	if (!allow_empty) {
		allow_empty = &PHALCON_GLOBAL(z_false);
	}

	if (!ex) {
		ex = &PHALCON_GLOBAL(z_false);
	}

	if (Z_TYPE_P(validator) == IS_ARRAY) {
		if (!phalcon_array_isset_fetch_str(&field, validator, SL("field"))) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Invalid field");
			return;
		}

		if (!phalcon_array_isset_fetch_str(&handler, validator, SL("validator"))) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Invalid validator");
			return;
		}

		if (!phalcon_is_callable(&handler)) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Validator must be an callable");
			return;
		}

		PHALCON_CALL_METHODW(&value, getThis(), "readattribute", &field);

		if (zend_is_true(allow_empty)) {
			if (PHALCON_IS_EMPTY(&value)) {
				RETURN_THISW();
			}
		}

		array_init_size(&arguments, 1);
		phalcon_array_append(&arguments, &value, PH_COPY);

		PHALCON_CALL_USER_FUNC_ARRAYW(&status, &handler, &arguments);

		if (PHALCON_IS_FALSE(&status)) {
			if (phalcon_array_isset_fetch_str(&message_str, validator, SL("message"))) {
				array_init_size(&pairs, 1);
				Z_TRY_ADDREF(field);
				add_assoc_zval_ex(&pairs, SL(":field"), &field);

				PHALCON_CALL_FUNCTIONW(&message, "strtr", &message_str, &pairs);
			} else {
				PHALCON_CONCAT_SVS(&message, "Invalid '", &field, "' format");
			}

			if (!phalcon_array_isset_fetch_str(&type, validator, SL("type"))) {
				PHALCON_STR(&type, "Validator");
			}

			if (!phalcon_array_isset_fetch_str(&code, validator, SL("code"))) {
				PHALCON_CPY_WRT(&code, &PHALCON_GLOBAL(z_zero));
			}

			PHALCON_CALL_METHODW(NULL, getThis(), "appendmessage", &message, &field, &type, &code);

			if (zend_is_true(ex)) {
				PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &message);
				return;
			}
		}

		RETURN_THISW();
	}

	/**
	 * Valid validators are objects
	 */
	PHALCON_VERIFY_INTERFACE_EX(validator, phalcon_mvc_model_validatorinterface_ce, phalcon_mvc_model_exception_ce, 0);

	PHALCON_STR(&option, "field");

	PHALCON_CALL_METHODW(&field, validator, "getoption", &option);

	if (zend_is_true(allow_empty)) {
		PHALCON_CALL_METHODW(&value, getThis(), "readattribute", &field);

		if (PHALCON_IS_EMPTY(&value)) {
			RETURN_THISW();
		}
	}

	/**
	 * Call the validation, if it returns false we append the messages to the current
	 * object
	 */
	PHALCON_CALL_METHODW(&status, validator, "validate", getThis());
	if (PHALCON_IS_FALSE(&status)) {
		PHALCON_CALL_METHODW(&messages, validator, "getmessages");

		if (Z_TYPE(messages) == IS_ARRAY) {
			phalcon_read_property(&errors, getThis(), SL("_errorMessages"), PH_NOISY);
			if (Z_TYPE(errors) == IS_ARRAY) {
				phalcon_fast_array_merge(&new_errors, &errors, &messages);
				phalcon_update_property_zval(getThis(), SL("_errorMessages"), &new_errors);
			} else {
				phalcon_update_property_zval(getThis(), SL("_errorMessages"), &messages);
			}

			if (zend_is_true(ex)) {
				PHALCON_CONCAT_SVS(&message, "Validation '", &field, "' failed");
				PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &message);
				return;
			}
		} else {
			zend_throw_exception_ex(spl_ce_LogicException, 0, "Validator '%s' returned false but did not call appendMessage()", Z_OBJCE_P(validator)->name->val);
		}
	}

	RETURN_THISW();
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

	phalcon_read_property(&error_messages, getThis(), SL("_errorMessages"), PH_NOISY);
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

	if (!filter || Z_TYPE_P(filter) != IS_STRING) {
		RETURN_MEMBER(getThis(), "_errorMessages");
	}

	phalcon_read_property(&messages, getThis(), SL("_errorMessages"), PH_NOISY);
	if (Z_TYPE(messages) == IS_ARRAY) {
		array_init(return_value);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(messages), value) {
			PHALCON_CALL_METHODW(&field, value, "getfield");

			if (PHALCON_IS_EQUAL(filter, &field)) {
				phalcon_array_append(return_value, value, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();
	}
}

/**
 * Reads "belongs to" relations and check the virtual foreign keys when inserting or updating records
 * to verify that inserted/updated values are present in the related entity
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _checkForeignKeysRestrict){

	zval manager = {}, belongs_to = {}, error = {}, *relation, event_name;	

	/**
	 * Get the models manager
	 */
	phalcon_read_property(&manager, getThis(), SL("_modelsManager"), PH_NOISY);

	/**
	 * We check if some of the belongsTo relations act as virtual foreign key
	 */
	PHALCON_CALL_METHODW(&belongs_to, &manager, "getbelongsto", getThis());
	if (phalcon_fast_count_ev(&belongs_to)) {
		ZVAL_FALSE(&error);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(belongs_to), relation) {
			zval foreign_key = {}, action = {}, relation_class = {}, referenced_model = {}, conditions = {}, bind_params = {}, fields = {}, referenced_fields = {}, condition = {}, *field;
			zval value = {}, extra_conditions = {}, join_conditions = {}, parameters = {}, rowcount = {}, user_message = {}, joined_fields = {}, type = {};
			zend_string *str_key;
			ulong idx;

			PHALCON_CALL_METHODW(&foreign_key, relation, "getforeignkey");
			if (PHALCON_IS_NOT_FALSE(&foreign_key)) {
				/**
				 * Try to find a different action in the foreign key's options
				 */
				if (Z_TYPE(foreign_key) == IS_ARRAY && phalcon_array_isset_str(&foreign_key, SL("action"))) {
					phalcon_array_fetch_str(&action, &foreign_key, SL("action"), PH_NOISY);
				} else {
					/**
					 * By default action is restrict
					 */
					ZVAL_LONG(&action, 1);
				}

				/**
				 * Check only if the operation is restrict
				 */
				if (PHALCON_IS_LONG(&action, 1)) {
					PHALCON_CALL_METHODW(&relation_class, relation, "getreferencedmodel");

					/**
					 * Load the referenced model if needed
					 */
					PHALCON_CALL_METHODW(&referenced_model, &manager, "load", &relation_class);

					/**
					 * Since relations can have multiple columns or a single one, we need to build a
					 * condition for each of these cases
					 */
					array_init(&conditions);
					array_init(&bind_params);

					PHALCON_CALL_METHODW(&fields, relation, "getfields");
					PHALCON_CALL_METHODW(&referenced_fields, relation, "getreferencedfields");

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

							phalcon_read_property_zval(&value, getThis(), field, PH_NOISY);

							phalcon_array_fetch(&referenced_field, &referenced_fields, &position, PH_NOISY);

							PHALCON_CONCAT_SVSV(&condition, "[", &referenced_field, "] = ?", &position);
							phalcon_array_append(&conditions, &condition, PH_COPY);
							phalcon_array_append(&bind_params, &value, PH_COPY);

						} ZEND_HASH_FOREACH_END();

					} else {
						/**
						 * Create a simple condition
						 */
						phalcon_read_property_zval(&value, getThis(), &fields, PH_NOISY);

						PHALCON_CONCAT_SVS(&condition, "[", &referenced_fields, "] = ?0");
						phalcon_array_append(&conditions, &condition, PH_COPY);
						phalcon_array_append(&bind_params, &value, PH_COPY);
					}

					/**
					 * Check if the virtual foreign key has extra conditions
					 */
					if (phalcon_array_isset_fetch_str(&extra_conditions, &foreign_key, SL("conditions"))) {
						phalcon_array_append(&conditions, &extra_conditions, PH_COPY);
					}

					/**
					 * We don't trust the actual values in the object and pass the values using bound
					 * parameters
					 */
					phalcon_fast_join_str(&join_conditions, SL(" AND "), &conditions);

					array_init_size(&parameters, 2);
					phalcon_array_append(&parameters, &join_conditions, PH_COPY);
					phalcon_array_update_str(&parameters, SL("bind"), &bind_params, PH_COPY);

					/**
					 * Let's make the checking
					 */
					PHALCON_CALL_METHODW(&rowcount, &referenced_model, "count", &parameters);
					if (!zend_is_true(&rowcount)) {
						/**
						 * Get the user message or produce a new one
						 */
						if (!phalcon_array_isset_fetch_str(&user_message, &foreign_key, SL("message"))) {
							if (Z_TYPE(fields) == IS_ARRAY) {
								phalcon_fast_join_str(&joined_fields, SL(", "), &fields);
								PHALCON_CONCAT_SVS(&user_message, "Value of fields \"", &joined_fields, "\" does not exist on referenced table");
							} else {
								PHALCON_CONCAT_SVS(&user_message, "Value of field \"", &fields, "\" does not exist on referenced table");
							}
						}

						/**
						 * Create a message
						 */
						PHALCON_STR(&type, "ConstraintViolation");

						PHALCON_CALL_METHODW(NULL, getThis(), "appendmessage", &user_message, &fields, &type);

						ZVAL_TRUE(&error);
						break;
					}
				}
			}
		} ZEND_HASH_FOREACH_END();

		/**
		 * Call 'onValidationFails' if the validation fails
		 */
		if (PHALCON_IS_TRUE(&error)) {
			PHALCON_STR(&event_name, "onValidationFails");
			PHALCON_CALL_METHODW(NULL, getThis(), "fireevent", &event_name);
			PHALCON_CALL_METHODW(NULL, getThis(), "_canceloperation");
			RETURN_FALSE;
		}
	}

	RETURN_TRUE;
}

/**
 * Reads both "hasMany" and "hasOne" relations and checks the virtual foreign keys (restrict) when deleting records
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _checkForeignKeysReverseRestrict){

	zval manager = {}, relations = {}, error = {}, *relation, event_name;

	/**
	 * Get the models manager
	 */
	phalcon_read_property(&manager, getThis(), SL("_modelsManager"), PH_NOISY);

	/**
	 * We check if some of the hasOne/hasMany relations is a foreign key
	 */
	PHALCON_CALL_METHODW(&relations, &manager, "gethasoneandhasmany", getThis());
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
			PHALCON_CALL_METHODW(&foreign_key, relation, "getforeignkey");
			if (PHALCON_IS_NOT_FALSE(&foreign_key)) {
				/**
				 * By default action is restrict
				 */
				ZVAL_LONG(&action, 1);

				/**
				 * Try to find a different action in the foreign key's options
				 */
				if (Z_TYPE(foreign_key) == IS_ARRAY || phalcon_array_isset_str(&foreign_key, SL("action"))) {
					phalcon_array_fetch_str(&action, &foreign_key, SL("action"), PH_NOISY);
				}

				/**
				 * Check only if the operation is restrict
				 */
				if (PHALCON_IS_LONG(&action, 1)) {
					PHALCON_CALL_METHODW(&relation_class, relation, "getreferencedmodel");

					/**
					 * Load a plain instance from the models manager
					 */
					PHALCON_CALL_METHODW(&referenced_model, &manager, "load", &relation_class);
					PHALCON_CALL_METHODW(&fields, relation, "getfields");
					PHALCON_CALL_METHODW(&referenced_fields, relation, "getreferencedfields");

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

							phalcon_read_property_zval(&value, getThis(), field, PH_NOISY);

							phalcon_array_fetch(&referenced_field, &referenced_fields, &tmp, PH_NOISY);

							PHALCON_CONCAT_SVSV(&condition, "[", &referenced_field, "] = ?", &tmp);
							phalcon_array_append(&conditions, &condition, PH_COPY);
							phalcon_array_append(&bind_params, &value, PH_COPY);
						} ZEND_HASH_FOREACH_END();

					} else {
						phalcon_read_property_zval(&value, getThis(), &fields, PH_NOISY);

						PHALCON_CONCAT_SVS(&condition, "[", &referenced_fields, "] = ?0");
						phalcon_array_append(&conditions, &condition, PH_COPY);
						phalcon_array_append(&bind_params, &value, PH_COPY);
					}

					/**
					 * Check if the virtual foreign key has extra conditions
					 */
					if (phalcon_array_isset_fetch_str(&extra_conditions, &foreign_key, SL("conditions"))) {
						phalcon_array_append(&conditions, &extra_conditions, PH_COPY);
					}

					/**
					 * We don't trust the actual values in the object and then we're passing the values
					 * using bound parameters
					 */
					phalcon_fast_join_str(&join_conditions, SL(" AND "), &conditions);

					array_init_size(&parameters, 2);
					phalcon_array_append(&parameters, &join_conditions, PH_COPY);
					phalcon_array_update_str(&parameters, SL("bind"), &bind_params, PH_COPY);

					/**
					 * Let's make the checking
					 */
					PHALCON_CALL_METHODW(&rowcount, &referenced_model, "count", &parameters);
					if (zend_is_true(&rowcount)) {

						/**
						 * Create a new message
						 */
						if (!phalcon_array_isset_fetch_str(&user_message, &foreign_key, SL("message"))) {
							PHALCON_CONCAT_SV(&user_message, "Record is referenced by model ", &relation_class);
						}

						/**
						 * Create a message
						 */
						PHALCON_STR(&type, "ConstraintViolation");

						PHALCON_CALL_METHODW(NULL, getThis(), "appendmessage", &user_message, &fields, &type);

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
			PHALCON_STR(&event_name, "onValidationFails");
			PHALCON_CALL_METHODW(NULL, getThis(), "fireevent", &event_name);
			PHALCON_CALL_METHODW(NULL, getThis(), "_canceloperation");

			RETURN_FALSE;
		}
	}

	RETURN_TRUE;
}

/**
 * Reads both "hasMany" and "hasOne" relations and checks the virtual foreign keys (cascade) when deleting records
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _checkForeignKeysReverseCascade){

	zval manager = {}, relations = {}, *relation;

	/**
	 * Get the models manager
	 */
	phalcon_read_property(&manager, getThis(), SL("_modelsManager"), PH_NOISY);

	/**
	 * We check if some of the hasOne/hasMany relations is a foreign key
	 */
	PHALCON_CALL_METHODW(&relations, &manager, "gethasoneandhasmany", getThis());
	if (phalcon_fast_count_ev(&relations)) {

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(relations), relation) {
			zval foreign_key = {}, action = {}, relation_class = {}, referenced_model = {}, fields = {}, referenced_fields = {}, conditions = {}, bind_params = {}, *field;
			zval value = {}, condition = {}, extra_conditions = {}, join_conditions = {}, parameters = {}, resulset = {}, status = {};
			zend_string *str_key;
			ulong idx;

			/**
			 * Check if the relation has a virtual foreign key
			 */
			PHALCON_CALL_METHODW(&foreign_key, relation, "getforeignkey");
			if (PHALCON_IS_NOT_FALSE(&foreign_key)) {
				/**
				 * By default action is restrict
				 */
				ZVAL_LONG(&action, 0);

				/**
				 * Try to find a different action in the foreign key's options
				 */
				if (Z_TYPE(foreign_key) == IS_ARRAY) { 
					if (phalcon_array_isset_str(&foreign_key, SL("action"))) {
						phalcon_array_fetch_str(&action, &foreign_key, SL("action"), PH_NOISY);
					}
				}

				/**
				 * Check only if the operation is restrict
				 */
				if (PHALCON_IS_LONG(&action, 2)) {
					PHALCON_CALL_METHODW(&relation_class, relation, "getreferencedmodel");

					/**
					 * Load a plain instance from the models manager
					 */
					PHALCON_CALL_METHODW(&referenced_model, &manager, "load", &relation_class);
					PHALCON_CALL_METHODW(&fields, relation, "getfields");
					PHALCON_CALL_METHODW(&referenced_fields, relation, "getreferencedfields");

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

							if (phalcon_isset_property_zval(getThis(), field)) {
								phalcon_return_property_zval(&value, getThis(), field);
							}

							phalcon_array_fetch(&referenced_field, &referenced_fields, &tmp, PH_NOISY);

							PHALCON_CONCAT_SVSV(&condition, "[", &referenced_field, "] = ?", &tmp);
							phalcon_array_append(&conditions, &condition, PH_COPY);
							phalcon_array_append(&bind_params, &value, PH_COPY);
						} ZEND_HASH_FOREACH_END();

					} else {
						if (phalcon_isset_property_zval(getThis(), &fields)) {
							phalcon_return_property_zval(&value, getThis(), field);
						}

						PHALCON_CONCAT_SVS(&condition, "[", &referenced_fields, "] = ?0");
						phalcon_array_append(&conditions, &condition, PH_COPY);
						phalcon_array_append(&bind_params, &value, PH_COPY);
					}

					/**
					 * Check if the virtual foreign key has extra conditions
					 */
					if (phalcon_array_isset_fetch_str(&extra_conditions, &foreign_key, SL("conditions"))) {
						phalcon_array_append(&conditions, &extra_conditions, PH_COPY);
					}

					/**
					 * We don't trust the actual values in the object and then we're passing the values
					 * using bound parameters
					 */
					phalcon_fast_join_str(&join_conditions, SL(" AND "), &conditions);

					array_init_size(&parameters, 2);
					phalcon_array_append(&parameters, &join_conditions, PH_COPY);
					phalcon_array_update_str(&parameters, SL("bind"), &bind_params, PH_COPY);

					/**
					 * Let's make the checking
					 */
					PHALCON_CALL_METHODW(&resulset, &referenced_model, "find", &parameters);

					/**
					 * Delete the resultset
					 */
					PHALCON_CALL_METHODW(&status, &resulset, "delete");

					/**
					 * Stop the operation
					 */
					if (PHALCON_IS_FALSE(&status)) {
						RETURN_FALSE;
					}
				}
			}
		} ZEND_HASH_FOREACH_END();

	}

	RETURN_TRUE;
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

	zval *meta_data, *exists, *identity_field, event_name = {}, status = {}, attributes = {}, data_type_numeric = {}, data_types = {}, column_map = {};
	zval automatic_attributes = {}, default_values = {}, *error, *field, skipped = {}, exception_message = {};
	double num, max;

	phalcon_fetch_params(0, 3, 0, &meta_data, &exists, &identity_field);

	if (phalcon_method_exists_ex(getThis(), SL("filters")) == SUCCESS) {
		PHALCON_CALL_METHODW(NULL, getThis(), "filters");
	}

	/**
	 * Run Validation Callbacks Before
	 */
	if (likely(PHALCON_GLOBAL(orm).events)) {
		PHALCON_STR(&event_name, "beforeValidation");

		/**
		 * Call the beforeValidation
		 */
		PHALCON_CALL_METHODW(&status, getThis(), "fireeventcancel", &event_name);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}

		if (!zend_is_true(exists)) {
			PHALCON_STR(&event_name, "beforeValidationOnCreate");
		} else {
			PHALCON_STR(&event_name, "beforeValidationOnUpdate");
		}

		/**
		 * Call the specific beforeValidation event for the current action
		 */
		PHALCON_CALL_METHODW(&status, getThis(), "fireeventcancel", &event_name);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}
	}

	/**
	 * Check for Virtual foreign keys
	 */
	if (PHALCON_GLOBAL(orm).virtual_foreign_keys) {
		PHALCON_CALL_METHODW(&status, getThis(), "_checkforeignkeysrestrict");
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}
	}

	PHALCON_CALL_METHODW(&attributes, meta_data, "getattributes", getThis());
	PHALCON_CALL_METHODW(&data_type_numeric, meta_data, "getdatatypesnumeric", getThis());
	PHALCON_CALL_METHODW(&data_types, meta_data, "getdatatypes", getThis());

	PHALCON_CALL_SELFW(&column_map, "getcolumnmap");

	/**
	 * Get fields that must be omitted from the SQL generation
	 */
	if (zend_is_true(exists)) {
		PHALCON_CALL_METHODW(&automatic_attributes, meta_data, "getautomaticupdateattributes", getThis());
		array_init(&default_values);
	} else {
		PHALCON_CALL_METHODW(&automatic_attributes, meta_data, "getautomaticupdateattributes", getThis());
		PHALCON_CALL_METHODW(&default_values, meta_data, "getdefaultvalues", getThis());
	}

	error = &PHALCON_GLOBAL(z_false);

	PHALCON_STR(&event_name, "validation");

	/**
	 * Call the main validation event
	 */
	PHALCON_CALL_METHODW(&status, getThis(), "fireeventcancel", &event_name);
	if (PHALCON_IS_FALSE(&status)) {
		PHALCON_STR(&event_name, "onValidationFails");
		PHALCON_CALL_METHODW(NULL, getThis(), "fireevent", &event_name);

		RETURN_FALSE;
	}

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(attributes), field) {
		zval attribute_field = {}, value = {}, field_type = {}, is_not_null = {}, message = {}, type = {}, field_size = {}, field_scale = {}, str_value = {}, length = {}, pos = {};
		/**
		 * We don't check fields that must be omitted
		 */
		if (!phalcon_array_isset(&automatic_attributes, field)) {

			if (Z_TYPE(column_map) == IS_ARRAY) { 
				if (!phalcon_array_isset_fetch(&attribute_field, &column_map, field, 0)) {
					PHALCON_CONCAT_SVS(&exception_message, "Column '", field, "' isn't part of the column map");
					PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}
			} else {
				PHALCON_CPY_WRT(&attribute_field, field);
			}

			if (phalcon_isset_property_zval(getThis(), &attribute_field)) {
				/**
				 * Read the attribute from the this_ptr using the real or renamed name
				 */
				phalcon_return_property_zval(&value, getThis(), &attribute_field);
			} else {
				ZVAL_NULL(&value);
			}

			phalcon_array_fetch(&field_type, &data_types, field, PH_NOISY);

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

				PHALCON_CALL_METHODW(&is_not_null, meta_data, "isnotnull", getThis(), field);
				if (zend_is_true(&is_not_null) && !phalcon_array_isset(&default_values, field)) {
					PHALCON_CONCAT_VS(&message, &attribute_field, " is required");
					PHALCON_STR(&type, "PresenceOf");

					PHALCON_CALL_METHODW(NULL, getThis(), "appendmessage", &message, &attribute_field, &type);

					error = &PHALCON_GLOBAL(z_true);
				}
			} else if (Z_TYPE(value) != IS_OBJECT || !instanceof_function(Z_OBJCE(value), phalcon_db_rawvalue_ce)) {
				if (phalcon_array_isset(&data_type_numeric, field)) {
					if (!phalcon_is_numeric(&value)) {
						PHALCON_CONCAT_SVS(&message, "Value of field '", &attribute_field, "' must be numeric");
						PHALCON_STR(&type, "Numericality");

						PHALCON_CALL_METHODW(NULL, getThis(), "appendmessage", &message, &attribute_field, &type);

						error = &PHALCON_GLOBAL(z_true);
					} else if (!phalcon_is_equal_long(&field_type, PHALCON_DB_COLUMN_TYPE_INTEGER)) {
						PHALCON_CALL_METHODW(&field_size, meta_data, "getdatasize", getThis(), field);
						PHALCON_CALL_METHODW(&field_scale, meta_data, "getdatascale", getThis(), field);

						phalcon_strval(&str_value, &value);
						phalcon_fast_strlen(&length, &value);
						phalcon_fast_strpos_str(&pos, &str_value, SL("."));

						if (!phalcon_is_numeric(&pos)) {
							ZVAL_LONG(&pos, Z_LVAL(length) - 1);
						}

						if (phalcon_is_numeric(&field_scale) && PHALCON_LT_LONG(&field_scale, (Z_LVAL(length)-Z_LVAL(pos)-1))) {
							PHALCON_CONCAT_SVSV(&message, "Value of field '", field, "' scale is out of range for type ", &field_type);

							PHALCON_STR(&type, "tooLarge");

							PHALCON_CALL_METHODW(NULL, getThis(), "appendmessage", &message, &attribute_field, &type);

							error = &PHALCON_GLOBAL(z_true);
							continue;
						}

						if (PHALCON_GT_LONG(&pos, (Z_LVAL(field_size)-Z_LVAL(field_scale)))) {
							PHALCON_CONCAT_SVSV(&message, "Value of field '", field, "' is out of range for type ", &field_type);

							PHALCON_STR(&type, "tooLarge");

							PHALCON_CALL_METHODW(NULL, getThis(), "appendmessage", &message, &attribute_field, &type);

							error = &PHALCON_GLOBAL(z_true);
						}
					} else {
						PHALCON_CALL_METHODW(&field_size, meta_data, "getdatabytes", getThis(), field);

						phalcon_strval(&str_value, &value);
						phalcon_fast_strpos_str(&pos, &str_value, SL("."));

						if (phalcon_is_numeric(&pos)) {
							PHALCON_CONCAT_SVS(&message, "Value of field '", field, "' must be int");
							PHALCON_STR(&type, "Numericality");

							PHALCON_CALL_METHODW(NULL, getThis(), "appendmessage", &message, &attribute_field, &type);
							error = &PHALCON_GLOBAL(z_true);
						} else {
							num = phalcon_get_intval(&value);
							max = pow(2, (Z_LVAL(field_size) - 1)) - 1;

							if (num > max) {
								PHALCON_CONCAT_SVSV(&message, "Value of field '", field, "' is out of range for type ", &field_type);

								PHALCON_STR(&type, "tooLarge");

								PHALCON_CALL_METHODW(NULL, getThis(), "appendmessage", &message, &attribute_field, &type);

								error = &PHALCON_GLOBAL(z_true);
							}
						}
					}
				} else if (phalcon_is_equal_long(&field_type, PHALCON_DB_COLUMN_TYPE_VARCHAR)
					|| phalcon_is_equal_long(&field_type, PHALCON_DB_COLUMN_TYPE_CHAR)) {
					if (!PHALCON_GLOBAL(orm).length_validations) {
						continue;
					}

					PHALCON_CALL_METHODW(&field_size, meta_data, "getdatasize", getThis(), field);
					if (Z_TYPE(field_size) != IS_NULL) {
						if (phalcon_function_exists_ex(SL("mb_strlen")) == SUCCESS) {
							convert_to_string_ex(&value);
							PHALCON_CALL_FUNCTION(&length, "mb_strlen", &value);
						} else {
							phalcon_fast_strlen(&length, &value);
						}

						if (phalcon_greater(&length, &field_size)) {
							PHALCON_CONCAT_SVSVS(&message, "Value of field '", field, "' exceeds the maximum ", &field_size, " characters");

							PHALCON_STR(&type, "TooLong");

							PHALCON_CALL_METHODW(NULL, getThis(), "appendmessage", &message, &attribute_field, &type);

							error = &PHALCON_GLOBAL(z_true);
						}
					}
				}
			}
		}
	} ZEND_HASH_FOREACH_END();

	if (PHALCON_IS_TRUE(error)) {
		PHALCON_STR(&event_name, "onValidationFails");
		PHALCON_CALL_METHODW(NULL, getThis(), "fireevent", &event_name);
		PHALCON_CALL_METHODW(NULL, getThis(), "_canceloperation");

		RETURN_FALSE;
	}

	/**
	 * Run Validation
	 */
	if (likely(PHALCON_GLOBAL(orm).events)) {
		if (!zend_is_true(exists)) {
			PHALCON_STR(&event_name, "afterValidationOnCreate");
		} else {
			PHALCON_STR(&event_name, "afterValidationOnUpdate");
		}

		/**
		 * Run Validation Callbacks After
		 */
		PHALCON_CALL_METHODW(&status, getThis(), "fireeventcancel", &event_name);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}

		PHALCON_STR(&event_name, "afterValidation");

		PHALCON_CALL_METHODW(&status, getThis(), "fireeventcancel", &event_name);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}

		PHALCON_STR(&event_name, "beforeSave");

		/**
		 * Run Before Callbacks
		 */
		PHALCON_CALL_METHODW(&status, getThis(), "fireeventcancel", &event_name);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}

		if (zend_is_true(exists)) {
			PHALCON_STR(&event_name, "beforeUpdate");
		} else {
			PHALCON_STR(&event_name, "beforeCreate");
		}

		phalcon_update_property_bool(getThis(), SL("_skipped"), 0);

		/**
		 * The operation can be skipped here
		 */
		PHALCON_CALL_METHODW(&status, getThis(), "fireeventcancel", &event_name);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}

		/**
		 * Always return true if the operation is skipped
		 */
		phalcon_read_property(&skipped, getThis(), SL("_skipped"), PH_NOISY);
		if (PHALCON_IS_TRUE(&skipped)) {
			RETURN_TRUE;
		}
	}

	RETURN_TRUE;
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
		if (PHALCON_IS_TRUE(success)) {
			if (zend_is_true(exists)) {
				PHALCON_STR(&event_name, "afterUpdate");
			} else {
				PHALCON_STR(&event_name, "afterCreate");
			}
			PHALCON_CALL_METHODW(NULL, getThis(), "fireevent", &event_name);

			PHALCON_STR(&event_name, "afterSave");
			PHALCON_CALL_METHODW(NULL, getThis(), "fireevent", &event_name);

			RETURN_CTORW(success);
		}

		PHALCON_STR(&event_name, "notSave");
		PHALCON_CALL_METHODW(NULL, getThis(), "fireevent", &event_name);
		PHALCON_CALL_METHODW(NULL, getThis(), "_canceloperation");
		RETURN_FALSE;
	}

	RETURN_CTORW(success);
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

	zval *meta_data, *connection, *table, *identity_field, bind_skip = {}, fields = {}, values = {}, bind_types = {}, attributes = {}, bind_data_types = {};
	zval automatic_attributes = {}, not_null_attributes = {}, default_values = {}, data_types = {}, column_map = {}, *field, exception_message = {};
	zval default_value = {}, use_explicit_identity = {}, column_name = {}, column_value = {}, column_type = {};
	zval success = {}, sequence_name = {}, support_sequences = {}, schema = {}, source = {}, last_insert_id = {};
	int identity_field_is_not_false; /* scan-build insists on using flags */

	phalcon_fetch_params(0, 4, 0, &meta_data, &connection, &table, &identity_field);

	ZVAL_LONG(&bind_skip, 1024);

	array_init(&fields);
	array_init(&values);
	array_init(&bind_types);

	PHALCON_CALL_METHODW(&attributes, meta_data, "getattributes", getThis());
	PHALCON_CALL_METHODW(&bind_data_types, meta_data, "getbindtypes", getThis());
	PHALCON_CALL_METHODW(&automatic_attributes, meta_data, "getautomaticcreateattributes", getThis());
	PHALCON_CALL_METHODW(&not_null_attributes, meta_data, "getnotnullattributes", getThis());
	PHALCON_CALL_METHODW(&default_values, meta_data, "getdefaultvalues", getThis());
	PHALCON_CALL_METHODW(&data_types, meta_data, "getdatatypes", getThis());
	PHALCON_CALL_SELFW(&column_map, "getcolumnmap");

	/**
	 * All fields in the model makes part or the INSERT
	 */
	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(attributes), field) {
		zval attribute_field = {}, value = {}, field_type = {}, convert_value = {}, bind_type = {};
		if (!phalcon_array_isset(&automatic_attributes, field)) {
			/**
			 * Check if the model has a column map
			 */
			if (Z_TYPE(column_map) == IS_ARRAY) { 
				if (!phalcon_array_isset_fetch(&attribute_field, &column_map, field, 0)) {
					PHALCON_CONCAT_SVS(&exception_message, "Column '", field, "' isn't part of the column map");
					PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}
			} else {
				PHALCON_CPY_WRT_CTOR(&attribute_field, field);
			}

			/**
			 * Check every attribute in the model except identity field
			 */
			if (!PHALCON_IS_EQUAL(field, identity_field)) {
				/**
				 * This isset checks that the property be defined in the model
				 */
				if (phalcon_isset_property_zval(getThis(), &attribute_field)) {
					/**
					 * Every column must have a bind data type defined
					 */
					if (!phalcon_array_isset(&bind_data_types, field)) {
						PHALCON_CONCAT_SVS(&exception_message, "Column '", field, "' has not defined a bind data type");
						PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
						return;
					}

					phalcon_return_property_zval(&value, getThis(), &attribute_field);
				}

				if (Z_TYPE(value) <= IS_NULL) {
					if (PHALCON_GLOBAL(orm).not_null_validations) {
						// Not allow null value and not has default value
						if (!phalcon_fast_in_array(field, &not_null_attributes) || !phalcon_array_isset(&default_values, field)) {
							phalcon_array_append(&fields, field, PH_COPY);
							phalcon_array_append(&values, &PHALCON_GLOBAL(z_null), PH_COPY);
							phalcon_array_append(&bind_types, &bind_skip, PH_COPY);
						}
					}
				} else {
					PHALCON_CPY_WRT(&convert_value, &value);
					if (PHALCON_GLOBAL(orm).enable_auto_convert) {
						if (Z_TYPE(value) != IS_OBJECT || !instanceof_function(Z_OBJCE(value), phalcon_db_rawvalue_ce)) {
							phalcon_array_fetch(&field_type, &data_types, field, PH_NOISY);

							if (phalcon_is_equal_long(&field_type, PHALCON_DB_COLUMN_TYPE_JSON)) {
								RETURN_ON_FAILURE(phalcon_json_encode(&convert_value, &value, 0));
							}
						}
					}

					phalcon_array_append(&fields, field, PH_COPY);
					phalcon_array_append(&values, &convert_value, PH_COPY);

					phalcon_array_fetch(&bind_type, &bind_data_types, field, PH_NOISY);
					phalcon_array_append(&bind_types, &bind_type, PH_COPY);
				}
			}
		}
	} ZEND_HASH_FOREACH_END();

	/**
	 * If there is an identity field we add it using "null" or "default"
	 */
	identity_field_is_not_false = PHALCON_IS_NOT_FALSE(identity_field);
	if (identity_field_is_not_false) {
		PHALCON_CALL_METHODW(&default_value, connection, "getdefaultidvalue");

		/**
		 * Not all the database systems require an explicit value for identity columns
		 */
		PHALCON_CALL_METHODW(&use_explicit_identity, connection, "useexplicitidvalue");
		if (zend_is_true(&use_explicit_identity)) {
			phalcon_array_append(&fields, identity_field, PH_COPY);
		}

		/**
		 * Check if the model has a column map
		 */
		if (Z_TYPE(column_map) == IS_ARRAY) { 
			if (!phalcon_array_isset_fetch(&column_name, &column_map, identity_field, 0)) {
				PHALCON_CONCAT_SVS(&exception_message, "Identity column '", identity_field, "' isn't part of the column map");
				PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
				return;
			}
		} else {
			PHALCON_CPY_WRT(&column_name, identity_field);
		}

		/**
		 * Check if the developer set an explicit value for the column
		 */
		if (phalcon_property_isset_fetch_zval(&column_value, getThis(), &column_name)) {
			if (PHALCON_IS_EMPTY(&column_value)) {
				if (zend_is_true(&use_explicit_identity)) {
					phalcon_array_append(&values, &default_value, PH_COPY);
					phalcon_array_append(&bind_types, &bind_skip, PH_COPY);
				}
			} else {
				/**
				 * Add the explicit value to the field list if the user has defined a value for it
				 */
				if (!zend_is_true(&use_explicit_identity)) {
					phalcon_array_append(&fields, identity_field, PH_COPY);
				}

				/**
				 * The field is valid we look for a bind value (normally int)
				 */
				if (!phalcon_array_isset_fetch(&column_type, &bind_data_types, identity_field, 0)) {
					PHALCON_CONCAT_SVS(&exception_message, "Identity column '", identity_field, "' isn't part of the table columns");
					PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}

				phalcon_array_append(&values, &column_value, PH_COPY);
				phalcon_array_append(&bind_types, &column_type, PH_COPY);
			}
		} else if (zend_is_true(&use_explicit_identity)) {
			phalcon_array_append(&values, &default_value, PH_COPY);
			phalcon_array_append(&bind_types, &bind_skip, PH_COPY);
		}
	}

	/**
	 * The low level insert is performed
	 */
	PHALCON_CALL_METHODW(&success, connection, "insert", table, &values, &fields, &bind_types);
	if (identity_field_is_not_false) {
		/**
		 * We check if the model have sequences
		 */
		PHALCON_CALL_METHODW(&support_sequences, connection, "supportsequences");
		if (PHALCON_IS_TRUE(&support_sequences)) {
			if (phalcon_method_exists_ex(getThis(), SL("getsequencename")) == SUCCESS) {
				PHALCON_CALL_METHODW(&sequence_name, getThis(), "getsequencename");
			} else {
				PHALCON_CALL_METHODW(&schema, getThis(), "getschema"); 
				PHALCON_CALL_METHODW(&source, getThis(), "getsource");

				if (PHALCON_IS_EMPTY(&schema)) {	
					PHALCON_CONCAT_VSVS(&sequence_name, &source, "_", identity_field, "_seq");
				} else {
					PHALCON_CONCAT_VSVSVS(&sequence_name, &schema, ".", &source, "_", identity_field, "_seq");
				}
			}
		}

		/**
		 * Recover the last "insert id" and assign it to the object
		 */
		PHALCON_CALL_METHODW(&last_insert_id, connection, "lastinsertid", &sequence_name);
		phalcon_update_property_zval_zval(getThis(), &column_name, &last_insert_id);

		/**
		 * Since the primary key was modified, we delete the _uniqueParams to force any
		 * future update to re-build the primary key
		 */
		phalcon_update_property_null(getThis(), SL("_uniqueParams"));
	}

	RETURN_CTORW(&success);
}

/**
 * Sends a pre-build UPDATE SQL statement to the relational database system
 *
 * @param Phalcon\Mvc\Model\MetadataInterface $metaData
 * @param Phalcon\Db\AdapterInterface $connection
 * @param string|array $table
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, _doLowUpdate){

	zval *meta_data, *connection, *table, bind_skip = {}, fields = {}, values = {}, bind_types = {}, manager = {}, use_dynamic_update = {};
	zval snapshot = {}, bind_data_types = {}, non_primary = {}, automatic_attributes = {}, data_types = {}, column_map = {}, columns = {}, *field, exception_message = {};
	zval attribute_field = {}, value = {}, unique_key = {}, unique_params = {}, unique_types = {}, primary_keys = {}, conditions = {}, ret = {}, type = {}, message = {};
	int i_use_dynamic_update; /* To keep static code analyzer happy */

	phalcon_fetch_params(0, 3, 0, &meta_data, &connection, &table);

	ZVAL_LONG(&bind_skip, 1024);
	array_init(&fields);
	array_init(&values);
	array_init(&bind_types);

	phalcon_return_property(&manager, getThis(), SL("_modelsManager"));

	/**
	 * Check if the model must use dynamic update
	 */
	PHALCON_CALL_METHODW(&use_dynamic_update, &manager, "isusingdynamicupdate", getThis());
	i_use_dynamic_update = zend_is_true(&use_dynamic_update);
	if (i_use_dynamic_update) {
		phalcon_return_property(&snapshot, getThis(), SL("_snapshot"));
		if (Z_TYPE(snapshot) != IS_ARRAY) { 
			i_use_dynamic_update = 0;
		}
	}

	PHALCON_CALL_METHODW(&bind_data_types, meta_data, "getbindtypes", getThis());
	PHALCON_CALL_METHODW(&non_primary, meta_data, "getnonprimarykeyattributes", getThis());
	PHALCON_CALL_METHODW(&automatic_attributes, meta_data, "getautomaticupdateattributes", getThis());
	PHALCON_CALL_METHODW(&data_types, meta_data, "getdatatypes", getThis());
	PHALCON_CALL_SELFW(&column_map, "getcolumnmap");

	/**
	 * We only make the update based on the non-primary attributes, values in primary
	 * key attributes are ignored
	 */
	if (PHALCON_GLOBAL(orm).allow_update_primary) {
		PHALCON_CALL_SELFW(&columns, "getcolumns");
	} else {
		PHALCON_CPY_WRT(&columns, &non_primary);
	}

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(columns), field) {
		zval field_type = {}, convert_value = {}, bind_type = {}, changed = {}, snapshot_value = {};
		if (!phalcon_array_isset(&automatic_attributes, field)) {
			/**
			 * Check a bind type for field to update
			 */
			if (!phalcon_array_isset(&bind_data_types, field)) {
				PHALCON_CONCAT_SVS(&exception_message, "Column '", field, "' have not defined a bind data type");
				PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
				return;
			}

			/**
			 * Check if the model has a column map
			 */
			if (Z_TYPE(column_map) == IS_ARRAY) { 
				if (!phalcon_array_isset_fetch(&attribute_field, &column_map, field, 0)) {
					PHALCON_CONCAT_SVS(&exception_message, "Column '", field, "' isn't part of the column map");
					PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}
			} else {
				PHALCON_CPY_WRT(&attribute_field, field);
			}

			/**
			 * If a field isn't set we pass a null value
			 */
			if (phalcon_property_isset_fetch_zval(&value, getThis(), &attribute_field)) {
				PHALCON_CPY_WRT(&convert_value, &value);
				if (PHALCON_GLOBAL(orm).enable_auto_convert) {
					if (Z_TYPE(value) != IS_OBJECT || !instanceof_function(Z_OBJCE(value), phalcon_db_rawvalue_ce)) {
						phalcon_array_fetch(&field_type, &data_types, field, PH_NOISY);

						if (phalcon_is_equal_long(&field_type, PHALCON_DB_COLUMN_TYPE_JSON)) {
							RETURN_ON_FAILURE(phalcon_json_encode(&convert_value, &value, 0));
						}
					}
				}

				/**
				 * When dynamic update is not used we pass every field to the update
				 */
				if (!i_use_dynamic_update || (Z_TYPE(value) == IS_OBJECT && instanceof_function(Z_OBJCE(value), phalcon_db_rawvalue_ce))) {
					phalcon_array_append(&fields, field, PH_COPY);
					phalcon_array_append(&values, &convert_value, PH_COPY);

					phalcon_array_fetch(&bind_type, &bind_data_types, field, PH_NOISY);
					phalcon_array_append(&bind_types, &bind_type, PH_COPY);
				} else {
					/**
					 * If the field is not part of the snapshot we add them as changed
					 */
					if (!phalcon_array_isset_fetch(&snapshot_value, &snapshot, &attribute_field, 0)) {
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
						phalcon_array_append(&fields, field, PH_COPY);
						phalcon_array_append(&values, &convert_value, PH_COPY);

						phalcon_array_fetch(&bind_type, &bind_data_types, field, PH_NOISY);
						phalcon_array_append(&bind_types, &bind_type, PH_COPY);
					}
				}
			} else {
				phalcon_array_append(&fields, field, PH_COPY);
				phalcon_array_append(&values, &PHALCON_GLOBAL(z_null), PH_COPY);
				phalcon_array_append(&bind_types, &bind_skip, PH_COPY);
			}
		}
	} ZEND_HASH_FOREACH_END();

	/**
	 * If there is no fields to update we return true
	 */
	if (!phalcon_fast_count_ev(&fields)) {
		if (PHALCON_GLOBAL(orm).enable_strict) {
			RETURN_FALSE;
		}
		RETURN_TRUE;
	}

	phalcon_return_property(&unique_key, getThis(), SL("_uniqueKey"));
	phalcon_return_property(&unique_params, getThis(), SL("_uniqueParams"));
	phalcon_return_property(&unique_types, getThis(), SL("_uniqueTypes"));

	/**
	 * When unique params is null we need to rebuild the bind params
	 */
	if (Z_TYPE(unique_params) != IS_ARRAY) { 
		array_init(&unique_params);

		PHALCON_CALL_METHODW(&primary_keys, meta_data, "getprimarykeyattributes", getThis());

		/**
		 * We can't create dynamic SQL without a primary key
		 */
		if (!phalcon_fast_count_ev(&primary_keys)) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "A primary key must be defined in the model in order to perform the operation");
			return;
		}

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(primary_keys), field) {
			/**
			 * Check if the model has a column map
			 */
			if (Z_TYPE(column_map) == IS_ARRAY) { 
				if (!phalcon_array_isset_fetch(&attribute_field, &column_map, field, 0)) {
					PHALCON_CONCAT_SVS(&exception_message, "Column '", field, "' isn't part of the column map");
					PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}
			} else {
				PHALCON_CPY_WRT(&attribute_field, field);
			}
			if (phalcon_property_isset_fetch_zval(&value, getThis(), &attribute_field)) {
				phalcon_array_append(&unique_params, &value, PH_COPY);
			} else {
				phalcon_array_append(&unique_params, &PHALCON_GLOBAL(z_null), PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();
	}

	/**
	 * We build the conditions as an array
	 */
	array_init_size(&conditions, 3);
	phalcon_array_update_string(&conditions, IS(conditions), &unique_key, PH_COPY);
	phalcon_array_update_str(&conditions, SL("bind"), &unique_params, PH_COPY);
	phalcon_array_update_str(&conditions, SL("bindTypes"), &unique_types, PH_COPY);

	/**
	 * Perform the low level update
	 */
	PHALCON_CALL_METHODW(&ret, connection, "update", table, &fields, &values, &conditions, &bind_types);
	if (zend_is_true(&ret)) {
		PHALCON_CALL_METHODW(&ret, connection, "affectedrows");
		if (zend_is_true(&ret)) {
			if (PHALCON_GLOBAL(orm).allow_update_primary) {
				phalcon_update_property_null(getThis(), SL("_uniqueParams"));
				PHALCON_CALL_METHODW(NULL, getThis(), "_rebuild", meta_data, connection);
			}
			RETURN_TRUE;
		}
	}

	PHALCON_STR(&type, "InvalidUpdateAttempt");
	PHALCON_STR(&message, "Record updated fail");

	PHALCON_CALL_METHODW(NULL, getThis(), "appendmessage", &message, &PHALCON_GLOBAL(z_null), &type);

	RETURN_FALSE;
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
	PHALCON_CALL_METHODW(NULL, connection, "begin", nesting);
	PHALCON_CALL_METHODW(&manager, getThis(), "getmodelsmanager");

	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(related), str_key, record) {
		zval tmp = {}, relation = {}, type = {}, columns = {}, referenced_model = {}, referenced_fields = {}, status = {}, referenced_value = {};
		if (str_key) {
			ZVAL_STR(&tmp, str_key);
			/**
			 * Try to get a relation with the same name
			 */
			PHALCON_CALL_METHODW(&relation, &manager, "getrelationbyalias", &class_name, &tmp);
			if (Z_TYPE(relation) == IS_OBJECT) {
				/**
				 * Get the relation type
				 */
				PHALCON_CALL_METHODW(&type, &relation, "gettype");

				/**
				 * Only belongsTo are stored before save the master record
				 */
				if (PHALCON_IS_LONG(&type, 0)) {
					if (Z_TYPE_P(record) != IS_OBJECT) {
						PHALCON_CALL_METHODW(NULL, connection, "rollback", nesting);
						PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Only objects can be stored as part of belongs-to relations");
						return;
					}

					PHALCON_CALL_METHODW(&columns, &relation, "getfields");
					PHALCON_CALL_METHODW(&referenced_model, &relation, "getreferencedmodel");
					PHALCON_CALL_METHODW(&referenced_fields, &relation, "getreferencedfields");
					if (Z_TYPE(columns) == IS_ARRAY) { 
						PHALCON_CALL_METHODW(NULL, connection, "rollback", nesting);
						PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Not implemented");
						return;
					}

					/**
					 * If dynamic update is enabled, saving the record must not take any action
					 */
					PHALCON_CALL_METHODW(&status, record, "save");
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
						PHALCON_CALL_METHODW(NULL, connection, "rollback", nesting);
						RETURN_FALSE;
					}

					/**
					 * Read the attribute from the referenced model and assigns it to the current model
					 */
					PHALCON_CALL_METHODW(&referenced_value, record, "readattribute", &referenced_fields);

					/**
					 * Assign it to the model
					 */
					phalcon_update_property_zval_zval(getThis(), &columns, &referenced_value);
				}
			}
		}
	} ZEND_HASH_FOREACH_END();

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

	PHALCON_CALL_METHODW(&manager, getThis(), "getmodelsmanager");

	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(related), str_key, record) {
		zval tmp = {}, relation = {}, type = {}, columns = {}, referenced_model = {}, referenced_fields = {}, related_records = {}, value = {}, is_through = {}, intermediate_model_name = {}, exception_message = {};
		zval intermediate_fields = {}, intermediate_referenced_fields = {}, *record_after;
		if (str_key){
			ZVAL_STR(&tmp, str_key);
			/**
			 * Try to get a relation with the same name
			 */
			PHALCON_CALL_METHODW(&relation, &manager, "getrelationbyalias", &class_name, &tmp);
			if (Z_TYPE(relation) == IS_OBJECT) {
				PHALCON_CALL_METHODW(&type, &relation, "gettype");

				/**
				 * Discard belongsTo relations
				 */
				if (PHALCON_IS_LONG(&type, 0)) {
					continue;
				}

				if (Z_TYPE_P(record) != IS_OBJECT && Z_TYPE_P(record) != IS_ARRAY) {
					PHALCON_CALL_METHODW(NULL, connection, "rollback", nesting);
					PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Only objects/arrays can be stored as part of has-many/has-one/has-many-to-many relations");
					return;
				}

				PHALCON_CALL_METHODW(&columns, &relation, "getfields");
				PHALCON_CALL_METHODW(&referenced_model, &relation, "getreferencedmodel");
				PHALCON_CALL_METHODW(&referenced_fields, &relation, "getreferencedfields");

				if (Z_TYPE(columns) == IS_ARRAY) { 
					PHALCON_CALL_METHODW(NULL, connection, "rollback", nesting);
					PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Not implemented");
					return;
				}

				/**
				 * Create an implicit array for has-many/has-one records
				 */
				if (Z_TYPE_P(record) == IS_OBJECT) {
					array_init_size(&related_records, 1);
					phalcon_array_append(&related_records, record, PH_COPY);
				} else {
					PHALCON_CPY_WRT_CTOR(&related_records, record);
				}

				if (!phalcon_isset_property_zval(getThis(), &columns)) {
					PHALCON_CALL_METHODW(NULL, connection, "rollback", nesting);

					PHALCON_CONCAT_SVS(&exception_message, "The column '", &columns, "' needs to be present in the model");
					PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}

				/**
				 * Get the value of the field from the current model
				 */
				phalcon_return_property_zval(&value, getThis(), &columns);

				/**
				 * Check if the relation is a has-many-to-many
				 */
				PHALCON_CALL_METHODW(&is_through, &relation, "isthrough");

				/**
				 * Get the rest of intermediate model info
				 */
				if (zend_is_true(&is_through)) {
					PHALCON_CALL_METHODW(&intermediate_model_name, &relation, "getintermediatemodel");
					PHALCON_CALL_METHODW(&intermediate_fields, &relation, "getintermediatefields");
					PHALCON_CALL_METHODW(&intermediate_referenced_fields, &relation, "getintermediatereferencedfields");
				}

				ZEND_HASH_FOREACH_VAL(Z_ARRVAL(related_records), record_after) {
					zval intermediate_model = {}, intermediate_value = {}, status = {};
					/**
					 * For non has-many-to-many relations just assign the local value in the referenced
					 * model
					 */
					if (!zend_is_true(&is_through)) {
						PHALCON_CALL_METHODW(NULL, record_after, "writeattribute", &referenced_fields, &value);
					}

					/**
					 * Save the record and get messages
					 */
					PHALCON_CALL_METHODW(&status, record_after, "save");
					if (!zend_is_true(&status)) {
						/**
						 * Get the validation messages generated by the referenced model
						 */
						if (phalcon_mvc_model_get_messages_from_model(getThis(), record_after, record) == FAILURE) {
							return;
						}

						/**
						 * Rollback the implicit transaction
						 */
						PHALCON_CALL_METHODW(NULL, connection, "rollback", nesting);
						RETURN_FALSE;
					}

					if (zend_is_true(&is_through)) {
						/**
						 * Create a new instance of the intermediate model
						 */
						PHALCON_CALL_METHODW(&intermediate_model, &manager, "load", &intermediate_model_name, &PHALCON_GLOBAL(z_true));

						/**
						 * Write value in the intermediate model
						 */
						PHALCON_CALL_METHODW(NULL, &intermediate_model, "writeattribute", &intermediate_fields, &value);

						/**
						 * Get the value from the referenced model
						 */
						phalcon_return_property_zval(&intermediate_value, record_after, &referenced_fields);

						/**
						 * Write the intermediate value in the intermediate model
						 */
						PHALCON_CALL_METHODW(NULL, &intermediate_model, "writeattribute", &intermediate_referenced_fields, &intermediate_value);

						/**
						 * Save the record and get messages
						 */
						PHALCON_CALL_METHODW(&status, &intermediate_model, "save");
						if (!zend_is_true(&status)) {
							/**
							 * Get the validation messages generated by the referenced model
							 */
							if (phalcon_mvc_model_get_messages_from_model(getThis(), &intermediate_model, record) == FAILURE) {
								return;
							}

							/**
							 * Rollback the implicit transaction
							 */
							PHALCON_CALL_METHODW(NULL, connection, "rollback", nesting);
							RETURN_FALSE;
						}
					}
				} ZEND_HASH_FOREACH_END();

			} else {
				if (Z_TYPE_P(record) != IS_ARRAY) { 
					PHALCON_CALL_METHODW(NULL, connection, "rollback", nesting);

					PHALCON_CONCAT_SVSVS(&exception_message, "There are no defined relations for the model \"", &class_name, "\" using alias \"", &tmp, "\"");
					PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}
			}
		}
	} ZEND_HASH_FOREACH_END();

	/**
	 * Commit the implicit transaction
	 */
	PHALCON_CALL_METHODW(NULL, connection, "commit", nesting);
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

	zval *data = NULL, *white_list = NULL, *_exists = NULL, *exists_check = NULL, exists = {}, meta_data = {}, attributes = {}, bind_params = {}, *attribute, read_connection = {};
	zval type = {}, message = {}, event_name = {}, status = {}, write_connection = {}, related = {}, schema = {}, source = {}, table = {}, identity_field = {};
	zval error_messages = {}, exception = {}, success = {}, new_success = {}, snapshot_data = {};
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 0, 4, &data, &white_list, &_exists, &exists_check);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (!white_list) {
		white_list = &PHALCON_GLOBAL(z_null);
	}

	if (!exists_check) {
		exists_check = &PHALCON_GLOBAL(z_true);
	}

	if (_exists) {
		PHALCON_CPY_WRT_CTOR(&exists, _exists);
	}

	PHALCON_CALL_METHODW(&meta_data, getThis(), "getmodelsmetadata");

	/**
	 * Get the reversed column map for future renamings
	 */
	PHALCON_CALL_SELFW(&attributes, "getcolumnmap");
	if (Z_TYPE(attributes) != IS_ARRAY) {
		PHALCON_CALL_METHODW(&attributes, &meta_data, "getattributes", getThis());
	}

	/**
	 * Assign the values passed
	 */
	if (Z_TYPE_P(data) != IS_NULL) {
		if (Z_TYPE_P(data) != IS_ARRAY) { 
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Data passed to save() must be an array");
			return;
		}
	}

	array_init(&bind_params);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(attributes), attribute) {
		zval value = {}, possible_setter = {};
		if (Z_TYPE_P(data) == IS_ARRAY && phalcon_array_isset(data, attribute)) {
			/**
			 * If the white-list is an array check if the attribute is on that list
			 */
			if (Z_TYPE_P(white_list) != IS_ARRAY || !phalcon_fast_in_array(attribute, white_list)) {
				/**
				 * We check if the field has a setter
				 */
				phalcon_array_fetch(&value, data, attribute, PH_NOISY);

				if (PHALCON_GLOBAL(orm).enable_property_method) {
					PHALCON_CONCAT_SV(&possible_setter, "set", attribute);
					zend_str_tolower(Z_STRVAL(possible_setter), Z_STRLEN(possible_setter));
					if (phalcon_method_exists(getThis(), &possible_setter) == SUCCESS) {
						PHALCON_CALL_ZVAL_METHODW(NULL, getThis(), &possible_setter, &value);
					} else {
						phalcon_update_property_zval_zval(getThis(), attribute, &value);
					}
				} else {
					/**
					 * Otherwise we assign the attribute directly
					 */
					phalcon_update_property_zval_zval(getThis(), attribute, &value);
				}
			}
		}

		if (phalcon_isset_property_zval(getThis(), attribute)) {
			phalcon_return_property_zval(&value, getThis(), attribute);
			phalcon_array_update_zval(&bind_params, attribute, &value, PH_COPY);
		}
	} ZEND_HASH_FOREACH_END();

	/**
	 * Create/Get the current database connection
	 */
	PHALCON_CALL_METHODW(&read_connection, getThis(), "getreadconnection", &PHALCON_GLOBAL(z_null), &bind_params);

	/**
	 * We need to check if the record exists
	 */
	if (!_exists) {
		PHALCON_CALL_METHODW(&exists, getThis(), "_exists", &meta_data, &read_connection);
	} else {
		if (zend_is_true(exists_check)) {		
			if (!zend_is_true(&exists)) {
				PHALCON_CALL_METHODW(&exists, getThis(), "_exists", &meta_data, &read_connection);
				if (zend_is_true(&exists)) {
					PHALCON_STR(&type, "InvalidCreateAttempt");
					PHALCON_STR(&message, "Record cannot be created because it already exists");

					PHALCON_CALL_METHODW(NULL, getThis(), "appendmessage", &message, &PHALCON_GLOBAL(z_null), &type);
					RETURN_FALSE;
				}
			} else {
				PHALCON_CALL_METHODW(&exists, getThis(), "_exists", &meta_data, &read_connection);
				if (!zend_is_true(&exists)) {
					PHALCON_STR(&type, "InvalidUpdateAttempt");
					PHALCON_STR(&message, "Record cannot be updated because it does not exist");

					PHALCON_CALL_METHODW(NULL, getThis(), "appendmessage", &message, &PHALCON_GLOBAL(z_null), &type);
					RETURN_FALSE;
				}
			}
		} else {
			PHALCON_CALL_METHODW(NULL, getThis(), "_rebuild", &meta_data, &read_connection);
		}
	}

	if (zend_is_true(&exists)) {
		phalcon_update_property_long(getThis(), SL("_operationMade"), 2);
	} else {
		phalcon_update_property_long(getThis(), SL("_operationMade"), 1);
	}

	PHALCON_STR(&event_name, "beforeOperation");
	PHALCON_CALL_METHODW(&status, getThis(), "fireeventcancel", &event_name);
	if (PHALCON_IS_FALSE(&status)) {
		RETURN_FALSE;
	}

	/**
	 * Create/Get the current database connection
	 */
	PHALCON_CALL_METHODW(&write_connection, getThis(), "getwriteconnection", &PHALCON_GLOBAL(z_null), &bind_params, &PHALCON_GLOBAL(z_null));

	/**
	 * Save related records in belongsTo relationships
	 */
	phalcon_return_property(&related, getThis(), SL("_related"));
	if (Z_TYPE(related) == IS_ARRAY) {
		PHALCON_CALL_METHODW(&status, getThis(), "_presaverelatedrecords", &write_connection, &related);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}
	}

	PHALCON_CALL_METHODW(&schema, getThis(), "getschema");
	PHALCON_CALL_METHODW(&source, getThis(), "getsource");
	if (zend_is_true(&schema)) {
		array_init_size(&table, 2);
		phalcon_array_append(&table, &schema, PH_COPY);
		phalcon_array_append(&table, &source, PH_COPY);
	} else {
		PHALCON_CPY_WRT(&table, &source);
	}

	/**
	 * Clean the messages
	 */
	phalcon_update_property_empty_array(getThis(), SL("_errorMessages"));

	/**
	 * Query the identity field
	 */
	PHALCON_CALL_METHODW(&identity_field, &meta_data, "getidentityfield", getThis());

	/**
	 * _preSave() makes all the validations
	 */
	PHALCON_CALL_METHODW(&status, getThis(), "_presave", &meta_data, &exists, &identity_field);
	if (PHALCON_IS_FALSE(&status)) {
		/**
		 * Rollback the current transaction if there was validation errors
		 */
		if (Z_TYPE(related) == IS_ARRAY) {
			PHALCON_CALL_METHODW(NULL, &write_connection, "rollback", &PHALCON_GLOBAL(z_false));
		}

		/**
		 * Throw exceptions on failed saves?
		 */
		if (unlikely(PHALCON_GLOBAL(orm).exception_on_failed_save)) {
			phalcon_return_property(&error_messages, getThis(), SL("_errorMessages"));

			/**
			 * Launch a Phalcon\Mvc\Model\ValidationFailed to notify that the save failed
			 */
			object_init_ex(&exception, phalcon_mvc_model_validationfailed_ce);
			PHALCON_CALL_METHODW(NULL, &exception, "__construct", getThis(), &error_messages);

			phalcon_throw_exception(&exception);
			return;
		}

		RETURN_FALSE;
	}

	/**
	 * Depending if the record exists we do an update or an insert operation
	 */
	if (zend_is_true(&exists)) {
		PHALCON_CALL_METHODW(&success, getThis(), "_dolowupdate", &meta_data, &write_connection, &table);
	} else {
		PHALCON_CALL_METHODW(&success, getThis(), "_dolowinsert", &meta_data, &write_connection, &table, &identity_field);
	}

	/**
	 * _postSave() makes all the validations
	 */
	PHALCON_CALL_METHODW(&new_success, getThis(), "_postsave", &success, &exists);

	if (Z_TYPE(related) == IS_ARRAY) {
		/**
		 * Rollbacks the implicit transaction if the master save has failed
		 */
		if (PHALCON_IS_FALSE(&new_success)) {
			PHALCON_CALL_METHODW(NULL, &write_connection, "rollback", &PHALCON_GLOBAL(z_false));
			RETURN_FALSE;
		}

		/**
		 * Save the post-related records
		 */
		PHALCON_CALL_METHODW(&status, getThis(), "_postsaverelatedrecords", &write_connection, &related);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
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

	/**
	 * Change the dirty state to persistent
	 */
	if (zend_is_true(&new_success)) {
		phalcon_update_property_long(getThis(), SL("_dirtyState"), 0);
		PHALCON_CALL_METHODW(&snapshot_data, getThis(), "toarray");
		PHALCON_CALL_METHODW(NULL, getThis(), "setsnapshotdata", &snapshot_data);

		PHALCON_STR(&event_name, "afterOperation");
		PHALCON_CALL_METHODW(NULL, getThis(), "fireevent", &event_name);
	}

	RETURN_CTORW(&new_success);
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
		exists_check = &PHALCON_GLOBAL(z_true);
	}

	/**
	 * Using save() anyways
	 */
	PHALCON_RETURN_CALL_METHODW(getThis(), "save", data, white_list, &PHALCON_GLOBAL(z_false), exists_check);
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
		exists_check = &PHALCON_GLOBAL(z_true);
	}

	/**
	 * Call save() anyways
	 */
	PHALCON_RETURN_CALL_METHODW(getThis(), "save", data, white_list, &PHALCON_GLOBAL(z_true), exists_check);
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

	zval event_name = {}, status = {}, check_foreign_keys = {}, bind_params = {}, bind_types = {}, conditions = {}, values = {}, meta_data = {}, primary_keys = {}, bind_data_types = {}, column_map = {};
	zval *primary_key, write_connection = {}, delete_conditions = {}, skipped = {}, schema = {}, source = {}, table = {}, success = {};

	phalcon_update_property_empty_array(getThis(), SL("_errorMessages"));

	/**
	 * Operation made is OP_DELETE
	 */
	phalcon_update_property_long(getThis(), SL("_operationMade"), 3);

	PHALCON_STR(&event_name, "beforeOperation");
	PHALCON_CALL_METHODW(&status, getThis(), "fireeventcancel", &event_name);
	if (PHALCON_IS_FALSE(&status)) {
		RETURN_FALSE;
	}

	/**
	 * Check if deleting the record violates a virtual foreign key
	 */
	if (PHALCON_GLOBAL(orm).virtual_foreign_keys) {
		PHALCON_CALL_METHODW(&check_foreign_keys, getThis(), "_checkforeignkeysreverserestrict");
		if (PHALCON_IS_FALSE(&check_foreign_keys)) {
			RETURN_FALSE;
		}
	}

	array_init(&bind_params);
	array_init(&bind_types);
	array_init(&conditions);
	array_init(&values);

	PHALCON_CALL_METHODW(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_CALL_METHODW(&primary_keys, &meta_data, "getprimarykeyattributes", getThis());
	PHALCON_CALL_METHODW(&bind_data_types, &meta_data, "getbindtypes", getThis());
	PHALCON_CALL_SELFW(&column_map, "getcolumnmap");

	/**
	 * We can't create dynamic SQL without a primary key
	 */
	if (!phalcon_fast_count_ev(&primary_keys)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "A primary key must be defined in the model in order to perform the operation");
		return;
	}

	PHALCON_CALL_METHODW(&write_connection, getThis(), "getwriteconnection");

	/**
	 * Create a condition from the primary keys
	 */
	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(primary_keys), primary_key) {
		zval attribute_field = {}, attribute_value = {}, exception_message = {}, escaped_field = {}, primary_condition = {}, bind_type = {};
		/**
		 * Every column part of the primary key must be in the bind data types
		 */
		if (!phalcon_array_isset(&bind_data_types, primary_key)) {
			PHALCON_CONCAT_SVS(&exception_message, "Column '", primary_key, "' have not defined a bind data type");
			PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
			return;
		}

		/**
		 * Take the column values based on the column map if any
		 */
		if (Z_TYPE(column_map) == IS_ARRAY) { 
			if (!phalcon_array_isset_fetch(&attribute_field, &column_map, primary_key, 0)) {
				PHALCON_CONCAT_SVS(&exception_message, "Column '", primary_key, "' isn't part of the column map");
				PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
				return;
			}
		} else {
			PHALCON_CPY_WRT(&attribute_field, primary_key);
		}

		/**
		 * If the attribute is currently set in the object add it to the conditions
		 */
		if (!phalcon_property_isset_fetch_zval(&attribute_value, getThis(), &attribute_field)) {
			PHALCON_CONCAT_SVS(&exception_message, "Cannot delete the record because the primary key attribute: '", &attribute_field, "' wasn't set");
			PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
			return;
		}

		phalcon_array_update_zval(&bind_params, &attribute_field, &attribute_value, PH_COPY);

		phalcon_array_append(&values, &attribute_value, PH_COPY);

		/**
		 * Escape the column identifier
		 */
		PHALCON_CALL_METHODW(&escaped_field, &write_connection, "escapeidentifier", primary_key);

		PHALCON_CONCAT_VS(&primary_condition, &escaped_field, " = ?");
		phalcon_array_append(&conditions, &primary_condition, PH_COPY);

		phalcon_array_fetch(&bind_type, &bind_data_types, primary_key, PH_NOISY);
		phalcon_array_append(&bind_types, &bind_type, PH_COPY);

	} ZEND_HASH_FOREACH_END();

	PHALCON_CALL_METHODW(&write_connection, getThis(), "getwriteconnection", &PHALCON_GLOBAL(z_null), &bind_params);

	/**
	 * Join the conditions in the array using an AND operator
	 */
	phalcon_fast_join_str(&delete_conditions, SL(" AND "), &conditions);

	phalcon_update_property_bool(getThis(), SL("_skipped"), 0);

	PHALCON_STR(&event_name, "beforeDelete");

	/**
	 * Fire the beforeDelete event
	 */
	PHALCON_CALL_METHODW(&status, getThis(), "fireeventcancel", &event_name);
	if (PHALCON_IS_FALSE(&status)) {
		RETURN_FALSE;
	} else {
		/**
		 * The operation can be skipped
		 */
		phalcon_return_property(&skipped, getThis(), SL("_skipped"));
		if (PHALCON_IS_TRUE(&skipped)) {
			RETURN_TRUE;
		}
	}

	PHALCON_CALL_METHODW(&schema, getThis(), "getschema");
	PHALCON_CALL_METHODW(&source, getThis(), "getsource");
	if (zend_is_true(&schema)) {
		array_init_size(&table, 2);
		phalcon_array_append(&table, &schema, PH_COPY);
		phalcon_array_append(&table, &source, PH_COPY);
	} else {
		PHALCON_CPY_WRT(&table, &source);
	}

	PHALCON_CALL_METHODW(NULL, &write_connection, "begin", &PHALCON_GLOBAL(z_false));

	/**
	 * Do the deletion
	 */
	PHALCON_CALL_METHODW(&success, &write_connection, "delete", &table, &delete_conditions, &values, &bind_types);
	if (zend_is_true(&success)) {
		if (PHALCON_GLOBAL(orm).enable_strict) {
			PHALCON_CALL_METHODW(&success, &write_connection, "affectedrows");
			if (!zend_is_true(&success)) {
				RETURN_FALSE;
			}
		}
	}

	/**
	 * Check if there is virtual foreign keys with cascade action
	 */
	if (PHALCON_GLOBAL(orm).virtual_foreign_keys) {
		PHALCON_CALL_METHODW(&check_foreign_keys, getThis(), "_checkforeignkeysreversecascade");
		if (PHALCON_IS_FALSE(&check_foreign_keys)) {
			PHALCON_CALL_METHODW(NULL, &write_connection, "rollback", &PHALCON_GLOBAL(z_false));
			RETURN_FALSE;
		}
	}

	PHALCON_CALL_METHODW(NULL, &write_connection, "commit", &PHALCON_GLOBAL(z_false));

	/**
	 * Force perform the record existence checking again
	 */
	phalcon_update_property_long(getThis(), SL("_dirtyState"), 2);

	if (zend_is_true(&success)) {
		PHALCON_STR(&event_name, "afterDelete");
		PHALCON_CALL_METHODW(NULL, getThis(), "fireevent", &event_name);

		PHALCON_STR(&event_name, "afterOperation");
		PHALCON_CALL_METHODW(NULL, getThis(), "fireevent", &event_name);
	}

	RETURN_FALSE;
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

	zval dirty_state = {}, bind_params = {}, meta_data = {}, primary_keys = {}, number_primary = {}, column_map = {}, *field, exception_message = {};
	zval read_connection = {}, schema = {}, source = {}, table = {}, unique_key = {}, exists = {}, unique_params = {}, unique_types = {}, attributes = {}, fields = {};
	zval escaped_table = {}, select = {}, dialect = {}, sql = {}, fetch_type = {}, row = {};

	phalcon_read_property(&dirty_state, getThis(), SL("_dirtyState"), PH_NOISY);
	if (!PHALCON_IS_LONG(&dirty_state, 0)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "The record cannot be refreshed because it does not exist or is deleted1");
		return;
	}

	array_init(&bind_params);

	PHALCON_CALL_METHODW(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_CALL_METHODW(&primary_keys, &meta_data, "getprimarykeyattributes", getThis());

	phalcon_fast_count(&number_primary, &primary_keys);

	if (zend_is_true(&number_primary)) {
		/**
		 * Check if column renaming is globally activated
		 */
		PHALCON_CALL_SELFW(&column_map, "getcolumnmap");

		/**
		 * We need to create a primary key based on the current data
		 */
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(primary_keys), field) {
			zval attribute_field = {}, attribute_value = {};
			if (Z_TYPE(column_map) == IS_ARRAY) { 
				if (!phalcon_array_isset_fetch(&attribute_field, &column_map, field, 0)) {
					PHALCON_CONCAT_SVS(&exception_message, "Column '", field, "' isn't part of the column map");
					PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}
			} else {
				PHALCON_CPY_WRT(&attribute_field, field);
			}

			if (phalcon_property_isset_fetch_zval(&attribute_value, getThis(), &attribute_field)) {
				phalcon_array_update_zval(&bind_params, &attribute_field, &attribute_value, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();
	}

	PHALCON_CALL_METHODW(&read_connection, getThis(), "getreadconnection", &PHALCON_GLOBAL(z_null), &bind_params);
	PHALCON_CALL_METHODW(&schema, getThis(), "getschema");
	PHALCON_CALL_METHODW(&source, getThis(), "getsource");

	if (zend_is_true(&schema)) {
		array_init_size(&table, 2);
		phalcon_array_append(&table, &schema, PH_COPY);
		phalcon_array_append(&table, &source, PH_COPY);
	} else {
		PHALCON_CPY_WRT(&table, &source);
	}

	phalcon_return_property(&unique_key, getThis(), SL("_uniqueKey"));
	if (!zend_is_true(&unique_key)) {
		/**
		 * We need to check if the record exists
		 */
		PHALCON_CALL_METHODW(&exists, getThis(), "_exists", &meta_data, &read_connection);
		if (!zend_is_true(&exists)) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "The record cannot be refreshed because it does not exist or is deleted2");
			return;
		}

		phalcon_return_property(&unique_key, getThis(), SL("_uniqueKey"));
	}

	phalcon_return_property(&unique_params, getThis(), SL("_uniqueParams"));

	if (Z_TYPE(unique_params) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "The record cannot be refreshed because it does not exist or is deleted3");
		return;
	}

	phalcon_return_property(&unique_types, getThis(), SL("_uniqueTypes"));

	/**
	 * We only refresh the attributes in the model's metadata
	 */
	PHALCON_CALL_METHODW(&attributes, &meta_data, "getattributes", getThis());

	array_init(&fields);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(attributes), field) {
		zval field_item = {};
		array_init_size(&field_item, 1);
		phalcon_array_append(&field_item, field, PH_COPY);
		phalcon_array_append(&fields, &field_item, PH_COPY);
	} ZEND_HASH_FOREACH_END();

	PHALCON_CALL_METHODW(&escaped_table, &read_connection, "escapeidentifier", &table);

	array_init_size(&select, 3);
	phalcon_array_update_string(&select, IS(columns), &fields, PH_COPY);
	phalcon_array_update_string(&select, IS(tables), &escaped_table, PH_COPY);
	phalcon_array_update_string(&select, IS(where), &unique_key, PH_COPY);

	/**
	 * We directly build the SELECT to save resources
	 */
	PHALCON_CALL_METHODW(&dialect, &read_connection, "getdialect");
	PHALCON_CALL_METHODW(&sql, &dialect, "select", &select);

	ZVAL_LONG(&fetch_type, PDO_FETCH_ASSOC);

	PHALCON_CALL_METHODW(&row, &read_connection, "fetchone", &sql, &fetch_type, &unique_params, &unique_types);

	/**
	 * Assign the resulting array to the this_ptr object
	 */
	if (Z_TYPE(row) == IS_ARRAY) {
		PHALCON_CALL_SELFW(&column_map, "getcolumnmap");
		PHALCON_CALL_METHODW(NULL, getThis(), "assign", &row, &column_map);
	}
}

/**
 * Skips the current operation forcing a success state
 *
 * @param boolean $skip
 */
PHP_METHOD(Phalcon_Mvc_Model, skipOperation){

	zval *skip;

	phalcon_fetch_params(0, 1, 0, &skip);

	phalcon_update_property_zval(getThis(), SL("_skipped"), skip);

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

	zval *attribute, attribute_value = {};

	phalcon_fetch_params(0, 1, 0, &attribute);

	if (phalcon_property_isset_fetch_zval(&attribute_value, getThis(), attribute)) {
		RETURN_CTORW(&attribute_value);
	}
	RETURN_NULL();
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

	zval *attributes, *replace = NULL, keys_attributes = {}, *attribute, meta_data = {};

	phalcon_fetch_params(0, 1, 1, &attributes, &replace);

	if (Z_TYPE_P(attributes) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Attributes must be an array");
		return;
	}

	if (!replace) {
		replace = &PHALCON_GLOBAL(z_false);
	}

	array_init(&keys_attributes);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(attributes), attribute) {
		phalcon_array_update_zval(&keys_attributes, attribute, &PHALCON_GLOBAL(z_null), PH_COPY);
	} ZEND_HASH_FOREACH_END();

	PHALCON_CALL_METHODW(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_CALL_METHODW(NULL, &meta_data, "setautomaticcreateattributes", getThis(), &keys_attributes, replace);
	PHALCON_CALL_METHODW(NULL, &meta_data, "setautomaticupdateattributes", getThis(), &keys_attributes, replace);
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

	zval *attributes, *replace = NULL, keys_attributes = {}, *attribute, meta_data = {};

	phalcon_fetch_params(0, 1, 1, &attributes, &replace);

	if (Z_TYPE_P(attributes) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Attributes must be an array");
		return;
	}

	if (!replace) {
		replace = &PHALCON_GLOBAL(z_false);
	}

	array_init(&keys_attributes);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(attributes), attribute) {
		phalcon_array_update_zval(&keys_attributes, attribute, &PHALCON_GLOBAL(z_null), PH_COPY);
	} ZEND_HASH_FOREACH_END();

	PHALCON_CALL_METHODW(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_CALL_METHODW(NULL, &meta_data, "setautomaticcreateattributes", getThis(), &keys_attributes, replace);
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

	PHALCON_CALL_METHODW(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_RETURN_CALL_METHODW(&meta_data, "getautomaticcreateattributes", getThis());
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

	zval *attributes, *replace = NULL, keys_attributes = {}, *attribute, meta_data = {};

	phalcon_fetch_params(0, 1, 1, &attributes, &replace);

	if (Z_TYPE_P(attributes) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Attributes must be an array");
		return;
	}

	if (!replace) {
		replace = &PHALCON_GLOBAL(z_false);
	}

	array_init(&keys_attributes);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(attributes), attribute) {
		phalcon_array_update_zval(&keys_attributes, attribute, &PHALCON_GLOBAL(z_null), PH_COPY);
	} ZEND_HASH_FOREACH_END();

	PHALCON_CALL_METHODW(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_CALL_METHODW(NULL, &meta_data, "setautomaticupdateattributes", getThis(), &keys_attributes, replace);
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

	PHALCON_CALL_METHODW(&meta_data, getThis(), "getmodelsmetadata");
	PHALCON_RETURN_CALL_METHODW(&meta_data, "getautomaticupdateattributes", getThis());
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

	zval *fields, *reference_model, *referenced_fields, *options = NULL, manager = {};

	phalcon_fetch_params(0, 3, 1, &fields, &reference_model, &referenced_fields, &options);

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&manager, getThis(), SL("_modelsManager"), PH_NOISY);
	PHALCON_RETURN_CALL_METHODW(&manager, "addhasone", getThis(), fields, reference_model, referenced_fields, options);
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

	zval *fields, *reference_model, *referenced_fields, *options = NULL, manager = {};

	phalcon_fetch_params(0, 3, 1, &fields, &reference_model, &referenced_fields, &options);

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&manager, getThis(), SL("_modelsManager"), PH_NOISY);
	PHALCON_RETURN_CALL_METHODW(&manager, "addbelongsto", getThis(), fields, reference_model, referenced_fields, options);
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

	zval *fields, *reference_model, *referenced_fields, *options = NULL, manager = {};

	phalcon_fetch_params(0, 3, 1, &fields, &reference_model, &referenced_fields, &options);

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&manager, getThis(), SL("_modelsManager"), PH_NOISY);
	PHALCON_RETURN_CALL_METHODW(&manager, "addhasmany", getThis(), fields, reference_model, referenced_fields, options);
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
	zval *referenced_fields, *options = NULL, manager = {};

	phalcon_fetch_params(0, 6, 1, &fields, &intermediate_model, &intermediate_fields, &intermediate_referenced_fields, &reference_model, &referenced_fields, &options);

	phalcon_read_property(&manager, getThis(), SL("_modelsManager"), PH_NOISY);

	PHALCON_RETURN_CALL_METHODW(&manager, "addhasmanytomany", getThis(), fields, intermediate_model, intermediate_fields, intermediate_referenced_fields, reference_model, referenced_fields, options ? options : &PHALCON_GLOBAL(z_null));
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

	zval *behavior, manager = {};

	phalcon_fetch_params(0, 1, 0, &behavior);

	phalcon_read_property(&manager, getThis(), SL("_modelsManager"), PH_NOISY);
	PHALCON_CALL_METHODW(NULL, &manager, "addbehavior", getThis(), behavior);
}

/**
 * Sets if the model must keep the original record snapshot in memory
 *
 *<code>
 *
 *class Robots extends \Phalcon\Mvc\Model
 *{
 *
 *   public function initialize()
 *   {
 *		$this->keepSnapshots(true);
 *   }
 *
 *}
 *</code>
 *
 * @param boolean $keepSnapshots
 */
PHP_METHOD(Phalcon_Mvc_Model, keepSnapshots){

	zval *keep_snapshot, manager = {};

	phalcon_fetch_params(0, 1, 0, &keep_snapshot);

	phalcon_read_property(&manager, getThis(), SL("_modelsManager"), PH_NOISY);
	PHALCON_CALL_METHODW(NULL, &manager, "keepsnapshots", getThis(), keep_snapshot);
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

	phalcon_fetch_params(0, 1, 1, &data, &column_map);

	if (!column_map) {
		column_map = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(data) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "The snapshot data must be an array");
		return;
	}

	/**
	 * Build the snapshot based on a column map
	 */
	if (Z_TYPE_P(column_map) == IS_ARRAY) {
		array_init(&snapshot);

		ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(data), str_key, value) {
			zval key = {}, attribute = {};
			if (str_key) {
				ZVAL_STR(&key, str_key);

				/**
				 * Every field must be part of the column map
				 */
				if (!phalcon_array_isset(column_map, &key)) {
					PHALCON_CONCAT_SVS(&exception_message, "Column \"", &key, "\" doesn't make part of the column map");
					PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}

				phalcon_array_fetch(&attribute, column_map, &key, PH_NOISY);
				phalcon_array_update_zval(&snapshot, &attribute, value, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();

		phalcon_update_property_zval(getThis(), SL("_snapshot"), &snapshot);
		RETURN_NULL();
	}

	phalcon_update_property_zval(getThis(), SL("_snapshot"), data);
}

/**
 * Checks if the object has internal snapshot data
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, hasSnapshotData){

	zval snapshot = {};

	phalcon_read_property(&snapshot, getThis(), SL("_snapshot"), PH_NOISY);
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

	zval *field_name = NULL, snapshot = {}, dirty_state = {}, meta_data = {}, column_map = {}, attributes = {}, all_attributes = {}, exception_message = {}, attribute_value = {}, original_value = {};
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 0, 1, &field_name);

	if (!field_name) {
		field_name = &PHALCON_GLOBAL(z_null);
	}

	phalcon_return_property(&snapshot, getThis(), SL("_snapshot"));
	if (Z_TYPE(snapshot) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "The record doesn't have a valid data snapshot");
		return;
	}

	if (Z_TYPE_P(field_name) != IS_STRING) {
		if (Z_TYPE_P(field_name) != IS_NULL) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "The field name must be string");
			return;
		}
	}

	phalcon_read_property(&dirty_state, getThis(), SL("_dirtyState"), PH_NOISY);

	/**
	 * Dirty state must be DIRTY_PERSISTENT to make the checking 
	 */
	if (!PHALCON_IS_LONG(&dirty_state, 0)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Change checking cannot be performed because the object has not been persisted or is deleted");
		return;
	}

	/**
	 * Return the models meta-data
	 */
	PHALCON_CALL_METHODW(&meta_data, getThis(), "getmodelsmetadata");

	/**
	 * The reversed column map is an array if the model has a column map
	 */
	PHALCON_CALL_METHODW(&column_map, &meta_data, "getreversecolumnmap", getThis());

	/**
	 * Data types are field indexed
	 */
	if (Z_TYPE(column_map) != IS_ARRAY) { 
		PHALCON_CALL_METHODW(&attributes, &meta_data, "getdatatypes", getThis());
		PHALCON_CPY_WRT(&all_attributes, &attributes);
	} else {
		PHALCON_CPY_WRT(&all_attributes, &column_map);
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
				PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
				return;
			}
		} else if (!phalcon_array_isset(&attributes, field_name)) {
			PHALCON_CONCAT_SVS(&exception_message, "The field '", field_name, "' is not part of the model");
			PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
			return;
		}

		/**
		 * The field is not part of the model, throw exception
		 */
		if (!phalcon_isset_property_zval(getThis(), field_name)) {
			PHALCON_CONCAT_SVS(&exception_message, "The field '", field_name, "' is not defined on the model");
			PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
			return;
		}

		/**
		 * The field is not part of the data snapshot, throw exception
		 */
		if (!phalcon_array_isset(&snapshot, field_name)) {
			PHALCON_CONCAT_SVS(&exception_message, "The field '", field_name, "' was not found in the snapshot");
			PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
			return;
		}

		phalcon_return_property_zval(&attribute_value, getThis(), field_name);

		phalcon_array_fetch(&original_value, &snapshot, field_name, PH_NOISY);

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
		if (!phalcon_property_isset_fetch_zval(&attribute_value, getThis(), &name)) {
			RETURN_TRUE;
		}

		phalcon_return_property_zval(&attribute_value, getThis(), &name);

		phalcon_array_fetch(&original_value, &snapshot, &name, PH_NOISY);

		/**
		 * Check if the field has changed
		 */
		if (!PHALCON_IS_EQUAL(&attribute_value, &original_value)) {
			RETURN_TRUE;
		}
	} ZEND_HASH_FOREACH_END();

	RETURN_FALSE;
}

/**
 * Returns a list of changed values
 *
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model, getChangedFields){

	zval snapshot = {}, dirty_state = {}, meta_data = {}, column_map = {}, attributes = {}, all_attributes = {}, changed = {}, attribute_value = {}, original_value = {};
	zend_string *str_key;
	ulong idx;

	phalcon_return_property(&snapshot, getThis(), SL("_snapshot"));
	if (Z_TYPE(snapshot) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "The record doesn't have a valid data snapshot");
		return;
	}

	phalcon_return_property(&dirty_state, getThis(), SL("_dirtyState"));

	/**
	 * Dirty state must be DIRTY_PERSISTENT to make the checking 
	 */
	if (!PHALCON_IS_LONG(&dirty_state, 0)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Change checking cannot be performed because the object has not been persisted or is deleted");
		return;
	}

	/**
	 * Return the models meta-data
	 */
	PHALCON_CALL_METHODW(&meta_data, getThis(), "getmodelsmetadata");

	/**
	 * The reversed column map is an array if the model has a column map
	 */
	PHALCON_CALL_METHODW(&column_map, &meta_data, "getreversecolumnmap", getThis());

	/**
	 * Data types are field indexed
	 */
	if (Z_TYPE(column_map) != IS_ARRAY) { 
		PHALCON_CALL_METHODW(&attributes, &meta_data, "getdatatypes", getThis());
		PHALCON_CPY_WRT(&all_attributes, &attributes);
	} else {
		PHALCON_CPY_WRT(&all_attributes, &column_map);
	}

	array_init(&changed);

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
		if (!phalcon_property_isset_fetch_zval(&attribute_value, getThis(), &tmp)) {
			phalcon_array_append(&changed, &tmp, PH_COPY);
			continue;
		}

		phalcon_array_fetch(&original_value, &snapshot, &tmp, PH_NOISY);

		/**
		 * Check if the field has changed
		 */
		if (!PHALCON_IS_EQUAL(&attribute_value, &original_value)) {
			phalcon_array_append(&changed, &tmp, PH_COPY);
			continue;
		}
	} ZEND_HASH_FOREACH_END();

	RETURN_CTORW(&changed);
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

	zval *dynamic_update, manager = {};

	phalcon_fetch_params(0, 1, 0, &dynamic_update);

	phalcon_read_property(&manager, getThis(), SL("_modelsManager"), PH_NOISY);
	PHALCON_CALL_METHODW(NULL, &manager, "usedynamicupdate", getThis(), dynamic_update);
}

/**
 * Returns related records based on defined relations
 *
 * @param string $alias
 * @param array $arguments
 * @return Phalcon\Mvc\Model\ResultsetInterface
 */
PHP_METHOD(Phalcon_Mvc_Model, getRelated){

	zval *alias, *arguments = NULL, manager = {}, class_name = {}, relation = {}, exception_message = {}, call_object = {}, model_args = {};

	phalcon_fetch_params(0, 1, 1, &alias, &arguments);

	if (!arguments) {
		arguments = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&manager, getThis(), SL("_modelsManager"), PH_NOISY);

	phalcon_get_class(&class_name, getThis(), 0);

	/**
	 * Query the relation by alias
	 */
	PHALCON_CALL_METHODW(&relation, &manager, "getrelationbyalias", &class_name, alias);
	if (Z_TYPE(relation) != IS_OBJECT) {
		PHALCON_CONCAT_SVSVS(&exception_message, "There is no defined relations for the model \"", &class_name, "\" using alias \"", alias, "\"");
		PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
		return;
	}

	/**
	 * Call the 'getRelationRecords' in the models manager
	 */
	array_init_size(&call_object, 2);
	phalcon_array_append(&call_object, &manager, PH_COPY);
	add_next_index_stringl(&call_object, SL("getRelationRecords"));

	array_init_size(&model_args, 4);
	phalcon_array_append(&model_args, &relation, PH_COPY);
	add_next_index_null(&model_args);
	phalcon_array_append(&model_args, getThis(), PH_COPY);
	phalcon_array_append(&model_args, arguments, PH_COPY);

	PHALCON_CALL_USER_FUNC_ARRAYW(return_value, &call_object, &model_args);
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

	zval *model_name, *method, *arguments, manager = {}, alias = {}, relation = {}, query_method = {}, extra_args = {}, call_args = {}, call_object = {};

	phalcon_fetch_params(0, 3, 0, &model_name, &method, &arguments);

	phalcon_read_property(&manager, getThis(), SL("_modelsManager"), PH_NOISY);

	/**
	 * Calling find/findFirst if the method starts with "get"
	 */
	if (phalcon_start_with_str(method, SL("get"))) {
		phalcon_substr(&alias, method, 3, 0);
		PHALCON_CALL_METHODW(&relation, &manager, "getrelationbyalias", model_name, &alias);
	}

	/**
	 * Calling count if the method starts with "count"
	 */
	if (Z_TYPE(relation) != IS_OBJECT) {
		if (phalcon_start_with_str(method, SL("count"))) {
			PHALCON_STR(&query_method, "count");
			phalcon_substr(&alias, method, 5, 0);
			PHALCON_CALL_METHODW(&relation, &manager, "getrelationbyalias", model_name, &alias);
		}
	}

	/**
	 * If the relation was found perform the query via the models manager
	 */
	if (Z_TYPE(relation) == IS_OBJECT) {
		if (Z_TYPE_P(arguments) != IS_ARRAY || !phalcon_array_isset_fetch_long(&extra_args, arguments, 0)) {
			ZVAL_NULL(&extra_args);
		}

		array_init_size(&call_args, 4);
		phalcon_array_append(&call_args, &relation, PH_COPY);
		phalcon_array_append(&call_args, &query_method, PH_COPY);
		phalcon_array_append(&call_args, getThis(), PH_COPY);
		phalcon_array_append(&call_args, &extra_args, PH_COPY);

		array_init_size(&call_object, 2);
		phalcon_array_append(&call_object, &manager, PH_COPY);
		add_next_index_stringl(&call_object, SL("getRelationRecords"));
		PHALCON_CALL_USER_FUNC_ARRAYW(return_value, &call_object, &call_args);
		return;
	}

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
	PHALCON_CALL_METHODW(&records, getThis(), "_getrelatedrecords", &model_name, method, arguments);
	if (Z_TYPE(records) != IS_NULL) {
		RETURN_CTORW(&records);
	}

	phalcon_read_property(&models_manager, getThis(), SL("_modelsManager"), PH_NOISY);

	/**
	 * Try to find a replacement for the missing method in a behavior/listener
	 */
	PHALCON_CALL_METHODW(&status, &models_manager, "missingmethod", getThis(), method, arguments);
	if (Z_TYPE(status) != IS_NULL) {
		RETURN_CTORW(&status);
	}

	/**
	 * The method doesn't exist throw an exception
	 */
	PHALCON_CONCAT_SVSVS(&exception_message, "The method \"", method, "\" doesn't exist on model \"", &model_name, "\"");
	PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
}

/**
 * Handles method calls when a static method is not implemented
 *
 * @param string $method
 * @param array $arguments
 * @return mixed
 */
PHP_METHOD(Phalcon_Mvc_Model, __callStatic){

	zval *method, *arguments = NULL, extra_method = {}, model_name = {}, exception_message = {}, value = {}, model = {}, meta_data = {}, attributes = {};
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
		PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
		return;
	}

	if (!phalcon_array_isset_long(arguments, 0)) {
		PHALCON_CONCAT_SVS(&exception_message, "The static method \"", method, "\" requires one argument");
		PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
		return;
	}

	phalcon_array_fetch_long(&value, arguments, 0, PH_NOISY);
	ce0 = phalcon_fetch_class(&model_name, ZEND_FETCH_CLASS_DEFAULT);

	object_init_ex(&model, ce0);
	if (phalcon_has_constructor(&model)) {
		PHALCON_CALL_METHODW(NULL, &model, "__construct");
	}

	/**
	 * Get the model's meta-data
	 */
	PHALCON_CALL_METHODW(&meta_data, &model, "getmodelsmetadata");

	/**
	 * Get the attributes
	 */
	PHALCON_CALL_METHODW(&attributes, &meta_data, "getreversecolumnmap", &model);
	if (Z_TYPE(attributes) != IS_ARRAY) { 
		PHALCON_CALL_METHODW(&attributes, &meta_data, "getdatatypes", &model);
	}

	/**
	 * Check if the extra-method is an attribute
	 */
	if (phalcon_array_isset(&attributes, &extra_method)) {
		PHALCON_CPY_WRT(&field, &extra_method);
	} else {
		/**
		 * Lowercase the first letter of the extra-method
		 */
		phalcon_lcfirst(&extra_method_first, &extra_method);
		if (phalcon_array_isset(&attributes, &extra_method_first)) {
			PHALCON_CPY_WRT(&field, &extra_method_first);
		} else {
			/**
			 * Get the possible real method name
			 */
			phalcon_uncamelize(&field, &extra_method);
			if (!phalcon_array_isset(&attributes, &field)) {
				PHALCON_CONCAT_SVS(&exception_message, "Cannot resolve attribute \"", &extra_method, "' in the model");
				PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
				return;
			}
		}
	}

	PHALCON_CONCAT_VS(&conditions, &field, " = ?0");

	array_init_size(&bind_params, 1);
	phalcon_array_append(&bind_params, &value, PH_COPY);

	array_init_size(&parameters, 2);
	phalcon_array_update_string(&parameters, IS(conditions), &conditions, PH_COPY);
	phalcon_array_update_str(&parameters, SL("bind"), &bind_params, PH_COPY);

	/**
	 * Execute the query
	 */
	PHALCON_RETURN_CALL_CE_STATICW(ce0, type, &parameters);
}

/**
 * Magic method to assign values to the the model
 *
 * @param string $property
 * @param mixed $value
 */
PHP_METHOD(Phalcon_Mvc_Model, __set){

	zval *property, *value, lower_property = {}, model_name = {}, possible_setter = {}, meta_data = {}, column_map = {}, attributes = {}, manager = {}, relation = {}, referenced_model_name = {}, type = {};
	zval referenced_model = {}, values = {}, related = {}, *item, exception_message = {};
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 2, 0, &property, &value);

	phalcon_fast_strtolower(&lower_property, property);

	phalcon_get_class(&model_name, getThis(), 0);

	if (Z_TYPE_P(property) == IS_STRING) {
		if (PHALCON_GLOBAL(orm).enable_property_method) {
			PHALCON_CONCAT_SV(&possible_setter, "set", property);
			phalcon_strtolower_inplace(&possible_setter);

			/*
			 * Check method is not 
			 */
			if (phalcon_method_exists_ce(phalcon_mvc_model_ce, &possible_setter) != SUCCESS) {
				if (phalcon_method_exists(getThis(), &possible_setter) == SUCCESS) {
					PHALCON_CALL_METHODW(NULL, getThis(), Z_STRVAL(possible_setter), value);
					RETURN_CTORW(value);
				}
			}
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

		PHALCON_CALL_METHODW(&meta_data, getThis(), "getmodelsmetadata");
		PHALCON_CALL_METHODW(&column_map, &meta_data, "getreversecolumnmap", getThis());
		if (Z_TYPE(column_map) != IS_ARRAY) {
			PHALCON_CALL_METHODW(&attributes, &meta_data, "getdatatypes", getThis());

			if (phalcon_array_isset(&attributes, property)) {
				phalcon_update_property_zval_zval(getThis(), property, value);
				RETURN_CTORW(value);
			}
		} else {
			if (phalcon_array_isset(&column_map, property)) {
				phalcon_update_property_zval_zval(getThis(), property, value);
				RETURN_CTORW(value);
			}
		}

		PHALCON_CALL_METHODW(&manager, getThis(), "getmodelsmanager");
		PHALCON_CALL_METHODW(&relation, &manager, "getrelationbyalias", &model_name, &lower_property);

		if (Z_TYPE(relation) == IS_OBJECT) {
			PHALCON_CALL_METHODW(&referenced_model_name, &relation, "getreferencedmodel");
		}
	}

	/**
	 * Values are probably relationships if they are objects
	 */
	if (Z_TYPE(relation) == IS_OBJECT) {
		if (Z_TYPE_P(value) == IS_NULL) {
			phalcon_unset_property_array(getThis(), SL("_related"), &lower_property);
			phalcon_unset_property_array(getThis(), SL("_relatedResult"), &lower_property);
		} else {
			PHALCON_CALL_METHODW(&type, &relation, "gettype");

			if (PHALCON_IS_LONG(&type, 0)) {
				if (Z_TYPE_P(value) == IS_OBJECT) {
					phalcon_get_class(&model_name, value, 0);

					if (instanceof_function_ex(Z_OBJCE_P(value), phalcon_mvc_model_resultsetinterface_ce, 1)) {
						phalcon_update_property_array(getThis(), SL("_relatedResult"), &lower_property, value);
					} else if (!instanceof_function_ex(Z_OBJCE_P(value), phalcon_mvc_modelinterface_ce, 1) || !PHALCON_IS_EQUAL(&referenced_model_name, &model_name)) {
						PHALCON_CONCAT_SVSVS(&exception_message, "Property \"", property, "\" must be an model `", &referenced_model_name, "`");
						PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
						return;
					} else {
						phalcon_update_property_array(getThis(), SL("_related"), &lower_property, value);
						phalcon_update_property_long(getThis(), SL("_dirtyState"), 1);
					}
				} else if (Z_TYPE_P(value) == IS_ARRAY) {
					PHALCON_CALL_METHODW(&referenced_model, &manager, "load", &referenced_model_name, &PHALCON_GLOBAL(z_false));
					if (Z_TYPE(referenced_model) == IS_OBJECT) {
							PHALCON_CALL_METHODW(NULL, &referenced_model, "assign", value);	
					}
				} else {
					PHALCON_CONCAT_SVSVS(&exception_message, "Property \"", property, "\" must be an model `", &referenced_model_name, "`");
					PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
					return;
				}
			} else {
				if (Z_TYPE_P(value) != IS_ARRAY) {
					array_init_size(&values, 1);
					phalcon_array_append(&values, value, PH_COPY);
				} else {
					PHALCON_CPY_WRT(&values, value);
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
							PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
							return;
						}

						phalcon_array_append(&related, item, PH_COPY);
					} else if (Z_TYPE_P(item) == IS_ARRAY) {
						PHALCON_CALL_METHODW(&referenced_model_name, &relation, "getreferencedmodel");
						PHALCON_CALL_METHODW(&referenced_model, &manager, "load", &referenced_model_name, &PHALCON_GLOBAL(z_false));
						if (Z_TYPE(referenced_model) == IS_OBJECT) {
							PHALCON_CALL_METHODW(NULL, &referenced_model, "assign", item);
						}
					} else {
						PHALCON_CONCAT_SVSVS(&exception_message, "Property \"", property, "\" must be an model `", &referenced_model_name, "`");
						PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
						return;
					}
				} ZEND_HASH_FOREACH_END();

				if (phalcon_fast_count_ev(&related)) {
					phalcon_update_property_array(getThis(), SL("_related"), &lower_property, &related);
					phalcon_update_property_long(getThis(), SL("_dirtyState"), 1);
				}
			}
		}
	} else {
		phalcon_update_property_zval_zval(getThis(), property, value);
	}

	RETURN_CTORW(value);
}

/**
 * Magic method to get related records using the relation alias as a property
 *
 * @param string $property
 * @return Phalcon\Mvc\Model\Resultset
 */
PHP_METHOD(Phalcon_Mvc_Model, __get){

	zval *property, possible_getter = {}, meta_data = {}, column_map = {}, attributes = {}, model_name = {}, manager = {}, lower_property = {}, related_result = {}, relation = {}, call_args = {}, call_object = {}, result = {};

	phalcon_fetch_params(0, 1, 0, &property);

	if (Z_TYPE_P(property) == IS_STRING) {
		if (PHALCON_GLOBAL(orm).enable_property_method) {
			PHALCON_CONCAT_SV(&possible_getter, "get", property);
			phalcon_strtolower_inplace(&possible_getter);

			if (phalcon_method_exists_ce(phalcon_mvc_model_ce, &possible_getter) != SUCCESS) {
				if (phalcon_method_exists(getThis(), &possible_getter) == SUCCESS) {
					PHALCON_CALL_ZVAL_METHODW(return_value, getThis(), &possible_getter);
					return;
				}
			}
		}

		PHALCON_CALL_METHODW(&meta_data, getThis(), "getmodelsmetadata");
		PHALCON_CALL_METHODW(&column_map, &meta_data, "getreversecolumnmap", getThis());
		if (Z_TYPE(column_map) != IS_ARRAY) {
			PHALCON_CALL_METHODW(&attributes, &meta_data, "getdatatypes", getThis());

			if (phalcon_array_isset(&attributes, property)) {
				RETURN_NULL();
			}
		} else {
			if (phalcon_array_isset(&column_map, property)) {
				RETURN_NULL();
			}
		}
	}

	phalcon_get_class(&model_name, getThis(), 0);
	phalcon_fast_strtolower(&lower_property, property);

	phalcon_return_property(&related_result, getThis(), SL("_relatedResult"));
	if (Z_TYPE(related_result) == IS_ARRAY) {
		if (phalcon_array_isset_fetch(&result, &related_result, &lower_property, 0)) {
			RETURN_CTORW(&result);
		}
	}

	PHALCON_CALL_METHODW(&manager, getThis(), "getmodelsmanager");
	PHALCON_CALL_METHODW(&relation, &manager, "getrelationbyalias", &model_name, &lower_property);

	/**
	 * Check if the property is a relationship
	 */
	if (Z_TYPE(relation) == IS_OBJECT) {
		array_init_size(&call_args, 4);
		phalcon_array_append(&call_args, &relation, PH_COPY);
		add_next_index_null(&call_args);
		phalcon_array_append(&call_args, getThis(), PH_COPY);
		add_next_index_null(&call_args);

		array_init_size(&call_object, 2);
		phalcon_array_append(&call_object, &manager, PH_COPY);
		add_next_index_stringl(&call_object, SL("getRelationRecords"));

		/**
		 * Get the related records
		 */
		PHALCON_CALL_USER_FUNC_ARRAYW(&result, &call_object, &call_args);

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

		RETURN_CTORW(&result);
	}

	/**
	 * A notice is shown if the property is not defined and it isn't a relationship
	 */
	/* TODO see if segfault is possible */
	zend_error(E_NOTICE, "Access to undefined property %s::%s", Z_STRVAL(model_name), Z_STRVAL_P(property));
	RETURN_NULL();
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

	PHALCON_CALL_METHODW(&manager, getThis(), "getmodelsmanager");

	/**
	 * Check if the property is a relationship
	 */
	PHALCON_CALL_METHODW(&relation, &manager, "getrelationbyalias", &model_name, property);
	if (Z_TYPE(relation) == IS_OBJECT) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Serializes the object ignoring connections, services, related objects or static properties
 *
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model, serialize){

	zval meta_data = {}, attributes = {}, column_map = {}, data = {}, *attribute, exception_message = {};

	PHALCON_CALL_METHODW(&meta_data, getThis(), "getmodelsmetadata");

	/**
	 * We get the model's attributes to only serialize them
	 */
	PHALCON_CALL_METHODW(&attributes, &meta_data, "getattributes", getThis());

	/**
	 * Reverse column map
	 */
	PHALCON_CALL_SELFW(&column_map, "getcolumnmap");

	array_init(&data);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(attributes), attribute) {
		zval attribute_field = {}, attribute_value = {};
		/**
		 * Check if the columns must be renamed
		 */
		if (Z_TYPE(column_map) == IS_ARRAY) { 
			if (!phalcon_array_isset_fetch(&attribute_field, &column_map, attribute, 0)) {
				PHALCON_CONCAT_SVS(&exception_message, "Column \"", attribute, "\" doesn't make part of the column map");
				PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
				return;
			}
		} else {
			PHALCON_CPY_WRT(&attribute_field, attribute);
		}

		if (phalcon_property_isset_fetch_zval(&attribute_value, getThis(), &attribute_field)) {
			phalcon_array_update_zval(&data, &attribute_field, &attribute_value, PH_COPY);
		} else {
			phalcon_array_update_zval(&data, &attribute_field, &PHALCON_GLOBAL(z_null), PH_COPY);
		}
	} ZEND_HASH_FOREACH_END();

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
PHP_METHOD(Phalcon_Mvc_Model, unserialize){

	zval *data, attributes = {}, dependency_injector = {}, service = {}, manager = {}, *value;
	zend_string *str_key;

	phalcon_fetch_params(0, 1, 0, &data);

	if (Z_TYPE_P(data) == IS_STRING) {
		phalcon_unserialize(&attributes, data);
		if (Z_TYPE(attributes) == IS_ARRAY) {
			/**
			 * Obtain the default DI
			 */
			PHALCON_CALL_METHODW(&dependency_injector, getThis(), "getdi");

			if (Z_TYPE(dependency_injector) != IS_OBJECT) {
				PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
				return;
			}

			/**
			 * Gets the default modelsManager service
			 */
			PHALCON_STR(&service, "modelsManager");

			PHALCON_CALL_METHODW(&manager, &dependency_injector, "getshared", &service);
			if (Z_TYPE(manager) != IS_OBJECT) {
				PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "The injected service 'modelsManager' is not object (2)");
				return;
			}

			PHALCON_VERIFY_INTERFACEW(&manager, phalcon_mvc_model_managerinterface_ce);

			/**
			 * Update the models manager
			 */
			phalcon_update_property_zval(getThis(), SL("_modelsManager"), &manager);

			/**
			 * Try to initialize the model
			 */
			PHALCON_CALL_METHODW(NULL, &manager, "initialize", getThis());

			/**
			 * Update the objects attributes
			 */
			ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL(attributes), str_key, value) {
				if (str_key) {
					phalcon_update_property_string_zval(getThis(), str_key, value);
				}
			} ZEND_HASH_FOREACH_END();

			RETURN_NULL();
		}
	}
	PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Invalid serialization data");
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

	PHALCON_RETURN_CALL_FUNCTIONW("get_object_vars", getThis());
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
PHP_METHOD(Phalcon_Mvc_Model, toArray){

	zval *columns = NULL, *rename_columns = NULL, meta_data = {}, attributes = {}, column_map = {}, data = {}, *attribute, exception_message = {}, event_name = {};

	phalcon_fetch_params(0, 0, 2, &columns, &rename_columns);

	if (!columns) {
		columns = &PHALCON_GLOBAL(z_null);
	}

	if (!rename_columns) {
		rename_columns = &PHALCON_GLOBAL(z_true);
	}

	PHALCON_CALL_METHODW(&meta_data, getThis(), "getmodelsmetadata");

	/**
	 * Original attributes
	 */
	PHALCON_CALL_METHODW(&attributes, &meta_data, "getattributes", getThis());

	/**
	 * Reverse column map
	 */
	PHALCON_CALL_SELFW(&column_map, "getcolumnmap");

	array_init(&data);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(attributes), attribute) {
		zval attribute_field = {}, possible_getter = {}, possible_value = {}, attribute_value = {};
		/**
		 * Check if the columns must be renamed
		 */
		if (zend_is_true(rename_columns) && Z_TYPE(column_map) == IS_ARRAY) { 
			if (!phalcon_array_isset_fetch(&attribute_field, &column_map, attribute, 0)) {
				PHALCON_CONCAT_SVS(&exception_message, "Column \"", attribute, "\" doesn't make part of the column map");
				PHALCON_THROW_EXCEPTION_ZVALW(phalcon_mvc_model_exception_ce, &exception_message);
				return;
			}
		} else {
			PHALCON_CPY_WRT(&attribute_field, attribute);
		}

		if (Z_TYPE_P(columns) == IS_ARRAY) {
			if (!phalcon_fast_in_array(&attribute_field, columns) && !phalcon_fast_in_array(attribute, columns)) {
				continue;
			}
		}

		if (PHALCON_GLOBAL(orm).enable_property_method) {
			PHALCON_CONCAT_SV(&possible_getter, "get", &attribute_field);
			phalcon_strtolower_inplace(&possible_getter);
			if (phalcon_method_exists(getThis(), &possible_getter) == SUCCESS) {
				PHALCON_CALL_ZVAL_METHODW(&possible_value, getThis(), &possible_getter);
				phalcon_array_update_zval(&data, &attribute_field, &possible_value, PH_COPY);
			} else if (phalcon_property_isset_fetch_zval(&attribute_value, getThis(), &attribute_field)) {
				phalcon_array_update_zval(&data, &attribute_field, &attribute_value, PH_COPY);
			} else {
				phalcon_array_update_zval(&data, &attribute_field, &PHALCON_GLOBAL(z_null), PH_COPY);
			}
		} else if (phalcon_property_isset_fetch_zval(&attribute_value, getThis(), &attribute_field)) {
			phalcon_array_update_zval(&data, &attribute_field, &attribute_value, PH_COPY);
		} else {
			phalcon_array_update_zval(&data, &attribute_field, &PHALCON_GLOBAL(z_null), PH_COPY);
		}
	} ZEND_HASH_FOREACH_END();


	PHALCON_STR(&event_name, "afterToArray");

	ZVAL_MAKE_REF(&data);
	PHALCON_CALL_METHODW(NULL, getThis(), "fireevent", &event_name, &data);
	ZVAL_UNREF(&data);

	RETURN_CTORW(&data);
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
 *
 * @param array $options
 */
PHP_METHOD(Phalcon_Mvc_Model, setup){

	zval *options, disable_events = {}, virtual_foreign_keys = {}, column_renaming = {}, not_null_validations = {}, length_validations = {}, exception_on_failed_save = {}, phql_literals = {};
	zval phql_cache = {}, property_method = {}, auto_convert = {}, allow_update_primary = {}, enable_strict = {};

	phalcon_fetch_params(0, 1, 0, &options);

	if (Z_TYPE_P(options) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Options must be an array");
		return;
	}

	/**
	 * Enables/Disables globally the internal events
	 */
	if (phalcon_array_isset_fetch_str(&disable_events, options, SL("events"))) {
		PHALCON_GLOBAL(orm).events = zend_is_true(&disable_events);
	}

	/**
	 * Enables/Disables virtual foreign keys
	 */
	if (phalcon_array_isset_fetch_str(&virtual_foreign_keys, options, SL("virtualForeignKeys"))) {
		PHALCON_GLOBAL(orm).virtual_foreign_keys = zend_is_true(&virtual_foreign_keys);
	}

	/**
	 * Enables/Disables column renaming
	 */
	if (phalcon_array_isset_fetch_str(&column_renaming, options, SL("columnRenaming"))) {
		PHALCON_GLOBAL(orm).column_renaming = zend_is_true(&column_renaming);
	}

	/**
	 * Enables/Disables automatic not null validation
	 */
	if (phalcon_array_isset_fetch_str(&not_null_validations, options, SL("notNullValidations"))) {
		PHALCON_GLOBAL(orm).not_null_validations = zend_is_true(&not_null_validations);
	}

	/**
	 * Enables/Disables automatic length validation
	 */
	if (phalcon_array_isset_fetch_str(&length_validations, options, SL("lengthValidations"))) {
		PHALCON_GLOBAL(orm).length_validations = zend_is_true(&length_validations);
	}

	/**
	 * Enables/Disables throws an exception if the saving process fails
	 */
	if (phalcon_array_isset_fetch_str(&exception_on_failed_save, options, SL("exceptionOnFailedSave"))) {
		PHALCON_GLOBAL(orm).exception_on_failed_save = zend_is_true(&exception_on_failed_save);
	}

	/**
	 * Enables/Disables literals in PHQL this improves the security of applications
	 */
	if (phalcon_array_isset_fetch_str(&phql_literals, options, SL("phqlLiterals"))) {
		PHALCON_GLOBAL(orm).enable_literals = zend_is_true(&phql_literals);
	}

	/**
	 * Enables/Disables AST cache
	 */
	if (phalcon_array_isset_fetch_str(&phql_cache, options, SL("astCache"))) {
		PHALCON_GLOBAL(orm).enable_ast_cache = zend_is_true(&phql_cache);
	}

	/**
	 * Enables/Disables property method
	 */
	if (phalcon_array_isset_fetch_str(&property_method, options, SL("propertyMethod"))) {
		PHALCON_GLOBAL(orm).enable_property_method = zend_is_true(&property_method);
	}

	/**
	 * Enables/Disables auto convert
	 */
	if (phalcon_array_isset_fetch_str(&auto_convert, options, SL("autoConvert"))) {
		PHALCON_GLOBAL(orm).enable_auto_convert = zend_is_true(&auto_convert);
	}

	/**
	 * Enables/Disables allow update primary
	 */
	if (phalcon_array_isset_fetch_str(&allow_update_primary, options, SL("allowUpdatePrimary"))) {
		PHALCON_GLOBAL(orm).allow_update_primary = zend_is_true(&allow_update_primary);
	}

	/**
	 * Enables/Disables strict mode
	 */
	if (phalcon_array_isset_fetch_str(&enable_strict, options, SL("strict"))) {
		PHALCON_GLOBAL(orm).enable_strict = zend_is_true(&enable_strict);
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

	zval *parameters = NULL, dependency_injector = {}, service_name = {}, has = {}, model_name = {}, manager = {}, model = {}, write_connection = {}, schema = {}, source = {}, delete_conditions = {};
	zval bind_params = {}, bind_types = {}, service_params = {}, query = {}, phql = {}, intermediate = {}, dialect = {}, table_conditions = {}, where_conditions = {}, where_expression = {}, success = {};

	phalcon_fetch_params(0, 1, 0, &parameters);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_CE_STATICW(&dependency_injector, phalcon_di_ce, "getdefault");

	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "A dependency injector container is required to obtain the services related to the ORM");
		return;
	}

	phalcon_get_called_class(&model_name);
	PHALCON_STR(&service_name, "modelsManager");

	PHALCON_CALL_METHODW(&manager, &dependency_injector, "getshared", &service_name);
	PHALCON_CALL_METHODW(&model, &manager, "load", &model_name);
	PHALCON_CALL_METHODW(&schema, &model, "getschema");
	PHALCON_CALL_METHODW(&source, &model, "getsource");

	array_init_size(&table_conditions, 2);
	phalcon_array_append(&table_conditions, &schema, PH_COPY);
	phalcon_array_append(&table_conditions, &source, PH_COPY);

	if (Z_TYPE_P(parameters) == IS_STRING) {
		PHALCON_CPY_WRT_CTOR(&delete_conditions, parameters);
	} else if (Z_TYPE_P(parameters) == IS_ARRAY) {
		if (!phalcon_array_isset_fetch_long(&delete_conditions, parameters, 0)) {
			if (!phalcon_array_isset_fetch_str(&delete_conditions, parameters, SL("conditions"))) {
				PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "Must set up the conditions");
				return;
			}
		}

		if (!phalcon_array_isset_fetch_long(&bind_params, parameters, 1)) {
			if (phalcon_array_isset_str(parameters, SL("bind"))) {
				phalcon_array_fetch_str(&bind_params, parameters, SL("bind"), PH_NOISY);
			}
		}

		if (!phalcon_array_isset_fetch_long(&bind_types, parameters, 2)) {
			if (phalcon_array_isset_str(parameters, SL("bindTypes"))) {
				phalcon_array_fetch_str(&bind_types, parameters, SL("bindTypes"), PH_NOISY);
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

	PHALCON_STR(&service_name, "modelsQuery");

	PHALCON_CALL_METHODW(&has, &dependency_injector, "has", &service_name);
	if (zend_is_true(&has)) {
		array_init(&service_params);
		phalcon_array_append(&service_params, &phql, PH_COPY);
		phalcon_array_append(&service_params, &dependency_injector, PH_COPY);

		PHALCON_CALL_METHODW(&query, &dependency_injector, "get", &service_name, &service_params);
	} else {
		object_init_ex(&query, phalcon_mvc_model_query_ce);
		PHALCON_CALL_METHODW(NULL, &query, "__construct", &phql, &dependency_injector);
	}

	PHALCON_CALL_METHODW(&intermediate, &query, "parse");
	PHALCON_CALL_METHODW(&write_connection, getThis(), "getwriteconnection", &intermediate, &bind_params, &bind_types);
	PHALCON_CALL_METHODW(&dialect, &write_connection, "getdialect");

	if (phalcon_array_isset_fetch_str(&where_conditions, &intermediate, SL("where"))) {		
		if (Z_TYPE(where_conditions) == IS_ARRAY) { 
			PHALCON_CALL_METHODW(&where_expression, &dialect, "getsqlexpression", &where_conditions);
		} else {
			PHALCON_CPY_WRT(&where_expression, &where_conditions);
		}
	}

	PHALCON_CALL_METHODW(&success, &write_connection, "delete", &table_conditions, &where_expression, &bind_params, &bind_types);

	if (PHALCON_IS_TRUE(&success)) {
		PHALCON_CALL_METHODW(&success, &write_connection, "affectedRows");
	}

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
 * @param boolean $noRecursive
 * @return Phalcon\Mvc\Model
 */
PHP_METHOD(Phalcon_Mvc_Model, filter){

	zval *field, *filters, *default_value = NULL, *not_allow_empty = NULL, *norecursive = NULL;
	zval value = {}, filterd_value = {}, filter = {}, dependency_injector = {}, service = {};

	phalcon_fetch_params(0, 2, 3, &field, &filters, &default_value, &not_allow_empty, &norecursive);

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	if (!not_allow_empty) {
		not_allow_empty = &PHALCON_GLOBAL(z_false);
	}

	if (!norecursive) {
		norecursive = &PHALCON_GLOBAL(z_false);
	}

	if (phalcon_isset_property_zval(getThis(), field)) {	
		phalcon_return_property_zval(&value, getThis(), field);

		if (!PHALCON_IS_EMPTY(&value) && Z_TYPE_P(filters) != IS_NULL) {	
			phalcon_return_property(&filter, getThis(), SL("_filter"));

			if (Z_TYPE(filter) != IS_OBJECT) {
				phalcon_return_property(&dependency_injector, getThis(), SL("_dependencyInjector"));

				if (Z_TYPE(dependency_injector) != IS_OBJECT) {
					PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "A dependency injection object is required to access the 'filter' service");
					return;
				}

				PHALCON_STR(&service, ISV(filter));

				PHALCON_CALL_METHODW(&filter, &dependency_injector, "getshared", &service);
				PHALCON_VERIFY_INTERFACEW(&filter, phalcon_filterinterface_ce);
				phalcon_update_property_zval(getThis(), SL("_filter"), &filter);
			}

			PHALCON_CALL_METHODW(&filterd_value, &filter, "sanitize", &value, filters, norecursive);

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

	RETURN_THISW();
}

/**
 * Whether the record is not new and deleted
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model, isRecord){

	zval dirty_state = {};

	phalcon_read_property(&dirty_state, getThis(), SL("_dirtyState"), PH_NOISY);

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

	phalcon_read_property(&dirty_state, getThis(), SL("_dirtyState"), PH_NOISY);

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

	phalcon_read_property(&dirty_state, getThis(), SL("_dirtyState"), PH_NOISY);

	if (phalcon_get_intval(&dirty_state) == PHALCON_MODEL_DIRTY_STATE_DETACHED) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}	
