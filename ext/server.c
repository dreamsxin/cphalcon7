
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

#include "server.h"
#include "server/core.h"
#include "server/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/concat.h"
#include "kernel/object.h"
#include "kernel/exception.h"

/**
 * Phalcon\Server
 *
 * It‘s an implementation of the socket server
 *</code>
 */
zend_class_entry *phalcon_server_ce;

PHP_METHOD(Phalcon_Server, __construct);
PHP_METHOD(Phalcon_Server, start);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_server___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, config, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_server_method_entry[] = {
	PHP_ME(Phalcon_Server, __construct, arginfo_phalcon_server___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Server, start, NULL, ZEND_ACC_PUBLIC)
	PHP_ABSTRACT_ME(Phalcon_Server, onStart, NULL)
	PHP_ABSTRACT_ME(Phalcon_Server, onConnect, NULL)
	PHP_ABSTRACT_ME(Phalcon_Server, onReceive, NULL)
	PHP_ABSTRACT_ME(Phalcon_Server, onClose, NULL)
	PHP_FE_END
};

zend_object_handlers phalcon_server_object_handlers;
zend_object* phalcon_server_object_create_handler(zend_class_entry *ce)
{
	phalcon_server_object *intern = ecalloc(1, sizeof(phalcon_server_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_server_object_handlers;

	intern->ctx = NULL;

	return &intern->std;
}

void phalcon_server_object_free_handler(zend_object *object)
{
	phalcon_server_object *intern = phalcon_server_object_from_obj(object);

	if (intern->ctx) {
		efree(intern->ctx);
	}
}

/**
 * Phalcon\Server initializer
 */
PHALCON_INIT_CLASS(Phalcon_Server){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon, Server, server, phalcon_server_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	return SUCCESS;
}

/**
 * Phalcon\Server constructor
 *
 * @param array $config
 * @throws \Phalcon\Server\Exception
 */
PHP_METHOD(Phalcon_Server, __construct){

	zval *config;

	phalcon_fetch_params(0, 1, 0, &config);

	phalcon_update_property_zval(getThis(), SL("_config"), config);
}

/**
 * Run the Server
 *
 */
PHP_METHOD(Phalcon_Server, start){

}
