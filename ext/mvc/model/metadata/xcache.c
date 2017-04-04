
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
  |          Vladimir Kolesnikov <vladimir@extrememember.com>              |
  +------------------------------------------------------------------------+
*/

#include "mvc/model/metadata/xcache.h"
#include "mvc/model/metadata.h"
#include "mvc/model/metadatainterface.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/concat.h"
#include "kernel/fcall.h"
#include "kernel/hash.h"

/**
 * Phalcon\Mvc\Model\MetaData\Xcache
 *
 * Stores model meta-data in the XCache cache. Data will erased if the web server is restarted
 *
 * By default meta-data is stored for 48 hours (172800 seconds)
 *
 * You can query the meta-data by printing xcache_get('$PMM$') or xcache_get('$PMM$my-app-id')
 *
 *<code>
 *	$metaData = new Phalcon\Mvc\Model\Metadata\Xcache(array(
 *		'prefix' => 'my-app-id',
 *		'lifetime' => 86400
 *	));
 *</code>
 */
zend_class_entry *phalcon_mvc_model_metadata_xcache_ce;

PHP_METHOD(Phalcon_Mvc_Model_MetaData_Xcache, __construct);
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Xcache, read);
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Xcache, write);
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Xcache, reset);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_metadata_xcache___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_mvc_model_metadata_xcache_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_MetaData_Xcache, __construct, arginfo_phalcon_mvc_model_metadata_xcache___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Model_MetaData_Xcache, read, arginfo_phalcon_mvc_model_metadatainterface_read, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_MetaData_Xcache, write, arginfo_phalcon_mvc_model_metadatainterface_write, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_MetaData_Xcache, reset, arginfo_phalcon_mvc_model_metadatainterface_reset, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\MetaData\Xcache initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_MetaData_Xcache){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\MetaData, Xcache, mvc_model_metadata_xcache, phalcon_mvc_model_metadata_ce, phalcon_mvc_model_metadata_xcache_method_entry, 0);

	zend_declare_property_string(phalcon_mvc_model_metadata_xcache_ce, SL("_prefix"), "", ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_mvc_model_metadata_xcache_ce, SL("_ttl"), 172800, ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_mvc_model_metadata_xcache_ce, 1, phalcon_mvc_model_metadatainterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Model\MetaData\Xcache constructor
 *
 * @param array $options
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Xcache, __construct){

	zval *options = NULL, prefix = {}, lifetime = {};

	phalcon_fetch_params(0, 0, 1, &options);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		if (phalcon_array_isset_fetch_str(&prefix, options, SL("prefix"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_prefix"), &prefix);
		}

		if (phalcon_array_isset_fetch_str(&lifetime, options, SL("lifetime"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_ttl"), &lifetime);
		}
	}

	phalcon_update_property_empty_array(getThis(), SL("_metaData"));
}

/**
 * Reads metadata from XCache
 *
 * @param  string $key
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Xcache, read){

	zval *key, prefix = {}, xc_key = {};

	phalcon_fetch_params(0, 1, 0, &key);

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_NOISY|PH_READONLY);

	PHALCON_CONCAT_SVV(&xc_key, "$PMM$", &prefix, key);

	PHALCON_CALL_FUNCTION(return_value, "xcache_get", &xc_key);
}

/**
 *  Writes the metadata to XCache
 *
 * @param string $key
 * @param array $data
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData_Xcache, write){

	zval *key, *data, prefix = {}, xc_key = {}, ttl = {};

	phalcon_fetch_params(0, 2, 0, &key, &data);

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_NOISY|PH_READONLY);

	PHALCON_CONCAT_SVV(&xc_key, "$PMM$", &prefix, key);

	phalcon_read_property(&ttl, getThis(), SL("_ttl"), PH_NOISY|PH_READONLY);
	PHALCON_CALL_FUNCTION(NULL, "xcache_set", &xc_key, data, &ttl);
}

PHP_METHOD(Phalcon_Mvc_Model_MetaData_Xcache, reset){

	zval prefix = {}, real_key = {};

	phalcon_read_property(&prefix, getThis(), SL("_prefix"), PH_NOISY|PH_READONLY);

	phalcon_concat_svs(&real_key, SL("$PMM$"), &prefix, SL("meta-"), 0);
	PHALCON_CALL_FUNCTION(NULL, "xcache_unset_by_prefix", &real_key);

	PHALCON_CALL_PARENT(NULL, phalcon_mvc_model_metadata_xcache_ce, getThis(), "reset");
}
