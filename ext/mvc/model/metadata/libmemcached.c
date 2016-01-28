
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

#include "mvc/model/metadata/libmemcached.h"
#include "mvc/model/metadata.h"
#include "mvc/model/metadatainterface.h"
#include "mvc/model/exception.h"
#include "cache/backend/libmemcached.h"
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
 * Phalcon\Mvc\Model\MetaData\Libmemcached
 *
 * Stores model meta-data in the Libmemcached cache. Data will erased if the web server is restarted
 *
 * By default meta-data is stored for 48 hours (172800 seconds)
 *
 * You can query the meta-data by printing libmemcached_get('$PMM$') or libmemcached_get('$PMM$my-app-id')
 *
 *<code>
 *	$metaData = new Phalcon\Mvc\Model\Metadata\Libmemcached(array(
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
zend_class_entry *phalcon_mvc_model_metadata_libmemcached_ce;

PHP_METHOD(Phalcon_Mvc_Model_MetaData_Libmemcached, __construct);
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Libmemcached, read);
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Libmemcached, write);
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Libmemcached, reset);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_metadata_libmemcached___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_metadata_libmemcached_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_MetaData_Libmemcached, __construct, arginfo_phalcon_mvc_model_metadata_libmemcached___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Model_MetaData_Libmemcached, read, arginfo_phalcon_mvc_model_metadatainterface_read, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_MetaData_Libmemcached, write, arginfo_phalcon_mvc_model_metadatainterface_write, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_MetaData_Libmemcached, reset, arginfo_phalcon_mvc_model_metadatainterface_reset, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\MetaData\Libmemcached initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_MetaData_Libmemcached){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\MetaData, Libmemcached, mvc_model_metadata_libmemcached, phalcon_mvc_model_metadata_ce, phalcon_mvc_model_metadata_libmemcached_method_entry, 0);

	zend_declare_property_null(phalcon_mvc_model_metadata_libmemcached_ce, SL("_lifetime"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_mvc_model_metadata_libmemcached_ce, SL("_libmemcached"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_mvc_model_metadata_libmemcached_ce, 1, phalcon_mvc_model_metadatainterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Model\MetaData\Libmemcached constructor
 *
 * @param array $options
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Libmemcached, __construct){

	zval *options = NULL, servers, client, lifetime, prefix, frontend_data, libmemcached, option;

	phalcon_fetch_params(0, 1, 0, &options);
	
	if (Z_TYPE_P(options) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "The options must be an array");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&servers, options, SL("servers"))) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_mvc_model_exception_ce, "No servers given in options");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&client, options, SL("client"))) {
		ZVAL_NULL(&client);
	}

	if (!phalcon_array_isset_fetch_str(&lifetime, options, SL("lifetime"))) {
		ZVAL_LONG(&lifetime, 8600);
	}

	phalcon_update_property_this(getThis(), SL("_lifetime"), &lifetime);

	if (!phalcon_array_isset_fetch_str(&prefix, options, SL("prefix"))) {
		ZVAL_EMPTY_STRING(&prefix);
	}

	/* create libmemcached instance */
	array_init_size(&option, 1);

	phalcon_array_update_str(&option, SL("lifetime"), &lifetime, PH_COPY);

	object_init_ex(&frontend_data, phalcon_cache_frontend_data_ce);

	PHALCON_CALL_METHODW(NULL, frontend_data, "__construct", &option);

	array_init(&option);

	phalcon_array_update_str_str(&option, SL("statsKey"), SL("$PMM$"), PH_COPY);
	phalcon_array_update_str(&option, SL("servers"), &servers, PH_COPY);

	if (Z_TYPE(client) > IS_NULL) {
		phalcon_array_update_str(&option, SL("client"), &client, PH_COPY);
	}

	phalcon_array_update_str(&option, SL("prefix"), &prefix, PH_COPY);

	object_init_ex(&libmemcached, phalcon_cache_backend_libmemcached_ce);

	PHALCON_CALL_METHODW(NULL, &libmemcached, "__construct", &frontend_data, &option);

	phalcon_update_property_this(getThis(), SL("_libmemcached"), &libmemcached);
	
	phalcon_update_property_empty_array(getThis(), SL("_metaData"));
}

/**
 * Reads metadata from Libmemcached
 *
 * @param  string $key
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Libmemcached, read){

	zval *key, *lifetime, *libmemcached;

	phalcon_fetch_params(0, 1, 0, &key);
	
	lifetime = phalcon_read_property(getThis(), SL("_lifetime"), PH_NOISY);
	libmemcached = phalcon_read_property(getThis(), SL("_libmemcached"), PH_NOISY);

	if (Z_TYPE_P(libmemcached) == IS_OBJECT) {
		PHALCON_RETURN_CALL_METHODW(libmemcached, "get", key, lifetime);
	} else {
		RETURN_NULL();
	}
}

/**
 *  Writes the metadata to Libmemcached
 *
 * @param string $key
 * @param array $data
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Libmemcached, write){

	zval *key, *data, *lifetime, *libmemcached;

	phalcon_fetch_params(0, 2, 0, &key, &data);
	
	lifetime = phalcon_read_property(getThis(), SL("_lifetime"), PH_NOISY);
	libmemcached = phalcon_read_property(getThis(), SL("_libmemcached"), PH_NOISY);

	if (Z_TYPE_P(libmemcached) == IS_OBJECT) {
		PHALCON_CALL_METHODW(NULL, libmemcached, "save", key, data, lifetime);	
	}
}

PHP_METHOD(Phalcon_Mvc_Model_MetaData_Libmemcached, reset)
{
	zval *libmemcached;

	libmemcached = phalcon_read_property(getThis(), SL("_libmemcached"), PH_NOISY);

	if (Z_TYPE_P(libmemcached) == IS_OBJECT) {
		PHALCON_CALL_METHODW(NULL, libmemcached, "flush");	
	}

	PHALCON_CALL_PARENTW(NULL, phalcon_mvc_model_metadata_libmemcached_ce, getThis(), "reset");
}
