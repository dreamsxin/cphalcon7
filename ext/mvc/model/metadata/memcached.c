
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

#include "mvc/model/metadata/memcached.h"
#include "mvc/model/metadata.h"
#include "mvc/model/metadatainterface.h"
#include "mvc/model/exception.h"
#include "cache/backend/memcached.h"
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
 * Phalcon\Mvc\Model\MetaData\Memcached
 *
 * Stores model meta-data in the Memcached cache. Data will erased if the web server is restarted
 *
 * By default meta-data is stored for 48 hours (172800 seconds)
 *
 * You can query the meta-data by printing memcached_get('$PMM$') or memcached_get('$PMM$my-app-id')
 *
 *<code>
 *	$metaData = new Phalcon\Mvc\Model\Metadata\Memcached(array(
 *     'servers' => array(
 *         array('host' => 'localhost', 'port' => 11211, 'weight' => 1),
 *     ),
 *     'client' => array(
 *         Memcached::OPT_HASH => Memcached::HASH_MD5,
 *         Memcached::OPT_PREFIX_KEY => 'prefix.',
 *     ),
 *		'prefix' => 'my-app-id',
 *		'lifetime' => 86400
 *	));
 *</code>
 */
zend_class_entry *phalcon_mvc_model_metadata_memcached_ce;

PHP_METHOD(Phalcon_Mvc_Model_MetaData_Memcached, __construct);
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Memcached, read);
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Memcached, write);
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Memcached, reset);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_metadata_memcached___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_metadata_memcached_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_MetaData_Memcached, __construct, arginfo_phalcon_mvc_model_metadata_memcached___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Model_MetaData_Memcached, read, arginfo_phalcon_mvc_model_metadatainterface_read, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_MetaData_Memcached, write, arginfo_phalcon_mvc_model_metadatainterface_write, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_MetaData_Memcached, reset, arginfo_phalcon_mvc_model_metadatainterface_reset, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\MetaData\Memcached initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_MetaData_Memcached){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\MetaData, Memcached, mvc_model_metadata_memcached, phalcon_mvc_model_metadata_ce, phalcon_mvc_model_metadata_memcached_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_model_metadata_memcached_ce, SL("_lifetime"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_metadata_memcached_ce, SL("_memcached"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_mvc_model_metadata_memcached_ce, 1, phalcon_mvc_model_metadatainterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Model\MetaData\Memcached constructor
 *
 * @param array $options
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Memcached, __construct)
{
	zval *options = NULL, servers = {}, client = {}, lifetime = {}, prefix = {}, frontend_data = {}, memcached = {}, option = {};

	phalcon_fetch_params(0, 1, 0, &options);

	if (Z_TYPE_P(options) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The options must be an array");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&servers, options, SL("servers"), PH_READONLY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "No servers given in options");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&client, options, SL("client"), PH_READONLY)) {
		ZVAL_NULL(&client);
	}

	if (!phalcon_array_isset_fetch_str(&lifetime, options, SL("lifetime"), PH_READONLY)) {
		ZVAL_LONG(&lifetime, 8600);
	}

	phalcon_update_property(getThis(), SL("_lifetime"), &lifetime);

	if (!phalcon_array_isset_fetch_str(&prefix, options, SL("prefix"), PH_COPY)) {
		ZVAL_EMPTY_STRING(&prefix);
	}

	/* create memcached instance */
	array_init_size(&option, 1);

	phalcon_array_update_str(&option, SL("lifetime"), &lifetime, PH_COPY);

	object_init_ex(&frontend_data, phalcon_cache_frontend_data_ce);

	PHALCON_CALL_METHOD(NULL, &frontend_data, "__construct", &option);
	zval_ptr_dtor(&option);

	array_init(&option);

	phalcon_array_update_str_str(&option, SL("statsKey"), SL("$PMM$"), 0);
	phalcon_array_update_str(&option, SL("servers"), &servers, PH_COPY);

	if (Z_TYPE(client) > IS_NULL) {
		phalcon_array_update_str(&option, SL("client"), &client, PH_COPY);
	}

	phalcon_array_update_str(&option, SL("prefix"), &prefix, PH_COPY);
	zval_ptr_dtor(&prefix);

	object_init_ex(&memcached, phalcon_cache_backend_memcached_ce);

	PHALCON_CALL_METHOD(NULL, &memcached, "__construct", &frontend_data, &option);
	zval_ptr_dtor(&frontend_data);
	zval_ptr_dtor(&option);

	phalcon_update_property(getThis(), SL("_memcached"), &memcached);
	zval_ptr_dtor(&memcached);

	phalcon_update_property_empty_array(getThis(), SL("_metaData"));
}

/**
 * Reads metadata from Memcached
 *
 * @param  string $key
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Memcached, read){

	zval *key, lifetime = {}, memcached = {};

	phalcon_fetch_params(0, 1, 0, &key);

	phalcon_read_property(&memcached, getThis(), SL("_memcached"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&lifetime, getThis(), SL("_lifetime"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(memcached) == IS_OBJECT) {
		PHALCON_RETURN_CALL_METHOD(&memcached, "get", key, &lifetime);
	} else {
		RETURN_NULL();
	}
}

/**
 *  Writes the metadata to Memcached
 *
 * @param string $key
 * @param array $data
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Memcached, write){

	zval *key, *data, lifetime = {}, memcached = {};

	phalcon_fetch_params(0, 2, 0, &key, &data);

	phalcon_read_property(&memcached, getThis(), SL("_memcached"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&lifetime, getThis(), SL("_lifetime"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(memcached) == IS_OBJECT) {
		PHALCON_CALL_METHOD(NULL, &memcached, "save", key, data, &lifetime);
	}
}

PHP_METHOD(Phalcon_Mvc_Model_MetaData_Memcached, reset){

	zval memcached = {};

	phalcon_read_property(&memcached, getThis(), SL("_memcached"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(memcached) == IS_OBJECT) {
		PHALCON_CALL_METHOD(NULL, &memcached, "flush");
	}

	PHALCON_CALL_PARENT(NULL, phalcon_mvc_model_metadata_memcached_ce, getThis(), "reset");
}
