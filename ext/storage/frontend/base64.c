
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

#include "storage/frontend/base64.h"
#include "storage/frontendinterface.h"

#include "kernel/main.h"
#include "kernel/string.h"

/**
 * Phalcon\Storage\Frontend\Base64
 *
 * Allows to cache data converting/deconverting them to base64.
 *
 * This adapters uses the base64_encode/base64_decode PHP's functions
 */
zend_class_entry *phalcon_storage_frontend_base64_ce;

PHP_METHOD(Phalcon_Storage_Frontend_Base64, beforeStore);
PHP_METHOD(Phalcon_Storage_Frontend_Base64, afterRetrieve);

static const zend_function_entry phalcon_storage_frontend_base64_method_entry[] = {
	PHP_ME(Phalcon_Storage_Frontend_Base64, beforeStore, arginfo_phalcon_storage_frontendinterface_beforestore, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Frontend_Base64, afterRetrieve, arginfo_phalcon_storage_frontendinterface_afterretrieve, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


/**
 * Phalcon\Storage\Frontend\Base64 initializer
 */
PHALCON_INIT_CLASS(Phalcon_Storage_Frontend_Base64){

	PHALCON_REGISTER_CLASS(Phalcon\\Storage\\Frontend, Base64, storage_frontend_base64, phalcon_storage_frontend_base64_method_entry, 0);

	zend_class_implements(phalcon_storage_frontend_base64_ce, 1, phalcon_storage_frontendinterface_ce);

	return SUCCESS;
}

/**
 * Serializes data before storing them
 *
 * @param mixed $data
 * @return string
 */
PHP_METHOD(Phalcon_Storage_Frontend_Base64, beforeStore){

	zval *data;

	phalcon_fetch_params(0, 1, 0, &data);
	phalcon_base64_encode(return_value, data);
}

/**
 * Unserializes data after retrieval
 *
 * @param mixed $data
 * @return mixed
 */
PHP_METHOD(Phalcon_Storage_Frontend_Base64, afterRetrieve){

	zval *data;

	phalcon_fetch_params(0, 1, 0, &data);
	phalcon_base64_decode(return_value, data);
}
