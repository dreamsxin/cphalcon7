
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

#include "storage/frontend/json.h"
#include "storage/frontendinterface.h"

#include "kernel/main.h"
#include "kernel/string.h"
#include "kernel/string.h"

/**
 * Phalcon\Storage\Frontend\Json
 *
 * Allows to cache data converting/deconverting them to JSON.
 *
 * This adapters uses the json_encode/json_decode PHP's functions
 *
 * As the data is encoded in JSON other systems accessing the same backend could process them
 *</code>
 */
zend_class_entry *phalcon_storage_frontend_json_ce;

PHP_METHOD(Phalcon_Storage_Frontend_Json, beforeStore);
PHP_METHOD(Phalcon_Storage_Frontend_Json, afterRetrieve);

static const zend_function_entry phalcon_storage_frontend_json_method_entry[] = {
	PHP_ME(Phalcon_Storage_Frontend_Json, beforeStore, arginfo_phalcon_storage_frontendinterface_beforestore, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Frontend_Json, afterRetrieve, arginfo_phalcon_storage_frontendinterface_afterretrieve, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Storage\Frontend\Json initializer
 */
PHALCON_INIT_CLASS(Phalcon_Storage_Frontend_Json){

	PHALCON_REGISTER_CLASS(Phalcon\\Storage\\Frontend, Json, storage_frontend_json, phalcon_storage_frontend_json_method_entry, 0);

	zend_class_implements(phalcon_storage_frontend_json_ce, 1, phalcon_storage_frontendinterface_ce);

	return SUCCESS;
}

/**
 * Serializes data before storing it
 *
 * @param mixed $data
 * @return string
 */
PHP_METHOD(Phalcon_Storage_Frontend_Json, beforeStore){

	zval *data;

	phalcon_fetch_params(0, 1, 0, &data);
	RETURN_ON_FAILURE(phalcon_json_encode(return_value, data, 0));
}

/**
 * Unserializes data after retrieving it
 *
 * @param mixed $data
 * @return mixed
 */
PHP_METHOD(Phalcon_Storage_Frontend_Json, afterRetrieve){

	zval *data;

	phalcon_fetch_params(0, 1, 0, &data);
	RETURN_ON_FAILURE(phalcon_json_decode(return_value, data, 0));
}
