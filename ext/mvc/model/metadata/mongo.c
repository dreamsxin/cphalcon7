
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

#include "mvc/model/metadata/mongo.h"
#include "mvc/model/metadata.h"
#include "mvc/model/metadatainterface.h"
#include "mvc/model/exception.h"
#include "cache/backend/mongo.h"
#include "cache/frontend/data.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/concat.h"
#include "kernel/fcall.h"
#include "kernel/hash.h"

/**
 * Phalcon\Mvc\Model\MetaData\Mongo
 *
 * Stores model meta-data in the Mongo cache. Data will erased if the web server is restarted
 *
 * By default meta-data is stored for 48 hours (172800 seconds)
 *
 * You can query the meta-data by printing mongo_get('$PMM$') or mongo_get('$PMM$my-app-id')
 *
 *<code>
 *	$metaData = new Phalcon\Mvc\Model\Metadata\Mongo(array(
 *		//'mongo' => new MongoClient(),
 *		'server' => 'mongodb://localhost',
 *		'db' => 'caches',
 *		'collection' => 'metadata',
 *		'prefix' => 'my-app-id',
 *		'lifetime' => 86400
 *	));
 *</code>
 */
zend_class_entry *phalcon_mvc_model_metadata_mongo_ce;

PHP_METHOD(Phalcon_Mvc_Model_MetaData_Mongo, __construct);
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Mongo, read);
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Mongo, write);
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Mongo, reset);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_metadata_mongo___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_metadata_mongo_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_MetaData_Mongo, __construct, arginfo_phalcon_mvc_model_metadata_mongo___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Model_MetaData_Mongo, read, arginfo_phalcon_mvc_model_metadatainterface_read, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_MetaData_Mongo, write, arginfo_phalcon_mvc_model_metadatainterface_write, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_MetaData_Mongo, reset, arginfo_phalcon_mvc_model_metadatainterface_reset, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\MetaData\Mongo initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_MetaData_Mongo){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\MetaData, Mongo, mvc_model_metadata_mongo, phalcon_mvc_model_metadata_ce, phalcon_mvc_model_metadata_mongo_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_model_metadata_mongo_ce, SL("_lifetime"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_metadata_mongo_ce, SL("_mongo"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_mvc_model_metadata_mongo_ce, 1, phalcon_mvc_model_metadatainterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Model\MetaData\Mongo constructor
 *
 * @param array $options
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Mongo, __construct){

	zval *options, backend_options = {}, lifetime = {}, frontend_data = {}, mongo = {}, option = {};

	phalcon_fetch_params(1, 1, 0, &options);

	if (Z_TYPE_P(options) != IS_ARRAY) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The options must be an array");
		return;
	}

	if (!phalcon_array_isset_str(options, SL("collection"))) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The parameter 'collection' is required");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&lifetime, options, SL("lifetime"), PH_READONLY)) {
		ZVAL_LONG(&lifetime, 8600);
	}

	PHALCON_MM_ZVAL_DUP(&backend_options, options);

	phalcon_update_property(getThis(), SL("_lifetime"), &lifetime);

	array_init_size(&option, 1);
	PHALCON_MM_ADD_ENTRY(&option);

	phalcon_array_update_str(&option, SL("lifetime"), &lifetime, 0);

	object_init_ex(&frontend_data, phalcon_cache_frontend_data_ce);
	PHALCON_MM_ADD_ENTRY(&frontend_data);
	PHALCON_MM_CALL_METHOD(NULL, &frontend_data, "__construct", &option);

	phalcon_array_update_str_str(&backend_options, SL("statsKey"), SL("$PMM$"), 0);

	object_init_ex(&mongo, phalcon_cache_backend_mongo_ce);
	PHALCON_MM_ADD_ENTRY(&mongo);
	PHALCON_MM_CALL_METHOD(NULL, &mongo, "__construct", &frontend_data, &backend_options);

	phalcon_update_property(getThis(), SL("_mongo"), &mongo);
	phalcon_update_property_empty_array(getThis(), SL("_metaData"));
	RETURN_MM();
}

/**
 * Reads metadata from Mongo
 *
 * @param  string $key
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Mongo, read){

	zval *key, mongo = {}, lifetime = {};

	phalcon_fetch_params(0, 1, 0, &key);

	phalcon_read_property(&mongo, getThis(), SL("_mongo"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(mongo) == IS_OBJECT) {
		phalcon_read_property(&lifetime, getThis(), SL("_lifetime"), PH_NOISY|PH_READONLY);
		PHALCON_RETURN_CALL_METHOD(&mongo, "get", key, &lifetime);

		return;
	}

	RETURN_NULL();
}

/**
 *  Writes the metadata to Mongo
 *
 * @param string $key
 * @param array $data
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Mongo, write){

	zval *key, *data, mongo = {}, lifetime = {};

	phalcon_fetch_params(0, 2, 0, &key, &data);

	phalcon_read_property(&mongo, getThis(), SL("_mongo"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(mongo) == IS_OBJECT) {
		phalcon_read_property(&lifetime, getThis(), SL("_lifetime"), PH_NOISY|PH_READONLY);
		PHALCON_CALL_METHOD(NULL, &mongo, "save", key, data, &lifetime);
	}
}

PHP_METHOD(Phalcon_Mvc_Model_MetaData_Mongo, reset){

	zval mongo;

	phalcon_read_property(&mongo, getThis(), SL("_mongo"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(mongo) == IS_OBJECT) {
		PHALCON_CALL_METHOD(NULL, &mongo, "flush");
	}

	PHALCON_CALL_PARENT(NULL, phalcon_mvc_model_metadata_mongo_ce, getThis(), "reset");
}
