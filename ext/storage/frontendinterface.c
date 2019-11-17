
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

#include "storage/frontendinterface.h"

#include "kernel/main.h"

zend_class_entry *phalcon_storage_frontendinterface_ce;

static const zend_function_entry phalcon_storage_frontendinterface_method_entry[] = {
	PHP_ABSTRACT_ME(Phalcon_Storage_FrontendInterface, beforeStore, arginfo_phalcon_storage_frontendinterface_beforestore)
	PHP_ABSTRACT_ME(Phalcon_Storage_FrontendInterface, afterRetrieve, arginfo_phalcon_storage_frontendinterface_afterretrieve)
	PHP_FE_END
};


/**
 * Phalcon\Storage\FrontendInterface initializer
 */
PHALCON_INIT_CLASS(Phalcon_Storage_FrontendInterface){

	PHALCON_REGISTER_INTERFACE(Phalcon\\Storage, FrontendInterface, storage_frontendinterface, phalcon_storage_frontendinterface_method_entry);

	return SUCCESS;
}

/**
 * Serializes data before storing it
 *
 * @param mixed $data
 */
PHALCON_DOC_METHOD(Phalcon_Storage_FrontendInterface, beforeStore);

/**
 * Unserializes data after retrieving it
 *
 * @param mixed $data
 */
PHALCON_DOC_METHOD(Phalcon_Storage_FrontendInterface, afterRetrieve);
