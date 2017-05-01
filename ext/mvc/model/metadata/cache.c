
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

#include "mvc/model/metadata/cache.h"
#include "mvc/model/metadata.h"
#include "mvc/model/metadatainterface.h"
#include "mvc/model/exception.h"
#include "cache/backendinterface.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/concat.h"
#include "kernel/fcall.h"
#include "kernel/hash.h"

/**
 * Phalcon\Mvc\Model\MetaData\Cache
 *
 * Stores model meta-data in the Cache cache. Data will erased if the web server is restarted
 *
 * By default meta-data is stored for 48 hours (172800 seconds)
 *
 * You can query the meta-data by printing cache_get('$PMM$') or cache_get('$PMM$my-app-id')
 *
 *<code>
 *	$metaData = new Phalcon\Mvc\Model\Metadata\Cache(array(
 *		'service' => 'cache', // Service Name
 *		'lifetime' => 86400,
 *	));
 *</code>
 */
zend_class_entry *phalcon_mvc_model_metadata_cache_ce;

PHP_METHOD(Phalcon_Mvc_Model_MetaData_Cache, __construct);
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Cache, _getCache);
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Cache, read);
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Cache, write);
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Cache, reset);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_metadata_cache___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_metadata_cache_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_MetaData_Cache, __construct, arginfo_phalcon_mvc_model_metadata_cache___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Model_MetaData_Cache, _getCache, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model_MetaData_Cache, read, arginfo_phalcon_mvc_model_metadatainterface_read, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_MetaData_Cache, write, arginfo_phalcon_mvc_model_metadatainterface_write, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_MetaData_Cache, reset, arginfo_phalcon_mvc_model_metadatainterface_reset, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\MetaData\Cache initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_MetaData_Cache){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\MetaData, Cache, mvc_model_metadata_cache, phalcon_mvc_model_metadata_ce, phalcon_mvc_model_metadata_cache_method_entry, 0);

	zend_declare_property_long(phalcon_mvc_model_metadata_cache_ce, SL("_lifetime"), 8600, ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_mvc_model_metadata_cache_ce, SL("_cache"), "cache", ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_mvc_model_metadata_cache_ce, 1, phalcon_mvc_model_metadatainterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Model\MetaData\Cache constructor
 *
 * @param array $options
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Cache, __construct){

	zval *options = NULL, service = {}, lifetime = {};

	phalcon_fetch_params(0, 0, 1, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		if (phalcon_array_isset_fetch_str(&service, options, SL("service"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_cache"), &service);
		}

		if (phalcon_array_isset_fetch_str(&lifetime, options, SL("lifetime"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_lifetime"), &lifetime);
		}
	}

	phalcon_update_property_empty_array(getThis(), SL("_metaData"));
}

PHP_METHOD(Phalcon_Mvc_Model_MetaData_Cache, _getCache){

	zval cache = {}, tmp = {};

	phalcon_read_property(&cache, getThis(), SL("_cache"), PH_READONLY);

	if (Z_TYPE(cache) != IS_OBJECT) {
		PHALCON_CALL_METHOD(&tmp, getThis(), "getresolveservice", &cache);
		PHALCON_VERIFY_INTERFACE(&tmp, phalcon_cache_backendinterface_ce);

		phalcon_update_property(getThis(), SL("_cache"), &tmp);

		RETVAL_ZVAL(&tmp, 0, 0);
	} else {
		PHALCON_VERIFY_INTERFACE(&cache, phalcon_cache_backendinterface_ce);
		RETURN_CTOR(&cache);
	}
}

/**
 * Reads metadata from Cache
 *
 * @param  string $key
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Cache, read){

	zval *key, cache = {}, lifetime = {};

	phalcon_fetch_params(0, 1, 0, &key);

	PHALCON_CALL_METHOD(&cache, getThis(), "_getcache");

	if (Z_TYPE(cache) == IS_OBJECT) {
		phalcon_read_property(&lifetime, getThis(), SL("_lifetime"), PH_NOISY|PH_READONLY);
		PHALCON_RETURN_CALL_METHOD(&cache, "get", key, &lifetime);
	} else {
		ZVAL_NULL(return_value);
	}
	zval_ptr_dtor(&cache);
}

/**
 *  Writes the metadata to Cache
 *
 * @param string $key
 * @param array $data
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Cache, write){

	zval *key, *data, cache = {}, lifetime = {};

	phalcon_fetch_params(0, 2, 0, &key, &data);

	PHALCON_CALL_METHOD(&cache, getThis(), "_getcache");

	if (Z_TYPE(cache) == IS_OBJECT) {
		phalcon_read_property(&lifetime, getThis(), SL("_lifetime"), PH_NOISY|PH_READONLY);
		PHALCON_CALL_METHOD(NULL, &cache, "save", key, data, &lifetime);
	}
	zval_ptr_dtor(&cache);
}

PHP_METHOD(Phalcon_Mvc_Model_MetaData_Cache, reset)
{
	zval cache = {};

	PHALCON_CALL_METHOD(&cache, getThis(), "_getcache");

	if (Z_TYPE(cache) == IS_OBJECT) {
		PHALCON_CALL_METHOD(NULL, &cache, "flush");
	}

	PHALCON_CALL_PARENT(NULL, phalcon_mvc_model_metadata_cache_ce, getThis(), "reset");
	zval_ptr_dtor(&cache);
}
