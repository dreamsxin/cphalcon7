
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

#include "server/simple.h"
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

#include "kernel/io/threads.h"
#include "kernel/io/server.h"

/**
 * Phalcon\Server\Simple
 *
 *<code>
 * class App extends Phalcon\Application {
 *     public function handle($data = NULL) {
 *         $data = trim($data);
 *         if (empty($data)) {
 *             return 'Please input'.PHP_EOL;
 *         } else {
 *             return '>>> '.$data.PHP_EOL;
 *         }
 *    }
 * }
 * $server = new Phalcon\Server\Simple();
 * $server->start(new App);
 *
 *</code>
 */
zend_class_entry *phalcon_server_simple_ce;

PHP_METHOD(Phalcon_Server_Simple, __construct);
PHP_METHOD(Phalcon_Server_Simple, start);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_server_simple___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_server_simple_start, 0, 0, 0)
	ZEND_ARG_OBJ_INFO(0, application, Phalcon\\Application, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_server_simple_method_entry[] = {
	PHP_ME(Phalcon_Server_Simple, __construct, arginfo_phalcon_server_simple___construct, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Server_Simple, start, arginfo_phalcon_server_simple_start, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_object_handlers phalcon_server_simple_object_handlers;
zend_object* phalcon_server_simple_object_create_handler(zend_class_entry *ce)
{
	phalcon_server_simple_object *intern = ecalloc(1, sizeof(phalcon_server_simple_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_server_simple_object_handlers;

	return &intern->std;
}

void phalcon_server_simple_object_free_handler(zend_object *object)
{
	zend_object_std_dtor(object);
}

/**
 * Phalcon\Server\Server initializer
 */
PHALCON_INIT_CLASS(Phalcon_Server_Simple){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Server, Simple, server_simple, phalcon_server_simple_method_entry, 0);

	return SUCCESS;
}

/**
 * Phalcon\Server\Simple constructor
 *
 * @param string $host
 * @param int $port
 * @throws \Phalcon\Server\Exception
 */
PHP_METHOD(Phalcon_Server_Simple, __construct){

	zval *host = NULL, *port = NULL;

	phalcon_fetch_params(0, 0, 2, &host, &port);
}

void client_callback (phalcon_io_client_info *ci, int op) {
	if (op==PHALCON_IO_CLIENT_READ) {
		zval data = {}, response = {};
		int flag = 0;

		phalcon_io_threads_info *ti = (phalcon_io_threads_info *) ci->tpi;
		phalcon_server_simple_object *intern = (phalcon_server_simple_object *) ti->parent;

		if (Z_TYPE(intern->application) == IS_OBJECT) {
			ZVAL_STRING(&data, phalcon_io_get_buffer_data(ci->rb));
			PHALCON_CALL_METHOD_FLAG(flag, &response, &intern->application, "handle", &data);
			if (flag == FAILURE || Z_TYPE(response) != IS_STRING) {
				phalcon_io_write_message(ci, phalcon_io_get_buffer_data(ci->rb));
			} else {
				phalcon_io_write_message(ci, Z_STRVAL(response));
			}
			zval_ptr_dtor(&data);
		} else {
			phalcon_io_write_message(ci, Z_STRVAL(response));
		}
	}
}

/**
 * Run the Server
 *
 */
PHP_METHOD(Phalcon_Server_Simple, start){

	zval *application = NULL;
	phalcon_server_simple_object *intern;
	int port = 8080;
	void *server;

	phalcon_fetch_params(0, 0, 1, &application);

	intern = phalcon_server_simple_object_from_obj(Z_OBJ_P(getThis()));
	PHALCON_COPY_TO_STACK(&intern->application, application);

	if (phalcon_io_init_servers() < 0) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_server_exception_ce, "Init server failed");
		return;
	}

	if ((server = phalcon_io_create_server(intern, NULL, port, NULL, client_callback, 0)) == NULL) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_server_exception_ce, "Create server failed");
		return;
	}

	if (!phalcon_io_start_server(server)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_server_exception_ce, "Start server failed");
		return;
	}

	while (fgetc(stdin) != '!')     // accept messages until user press !
		;

	phalcon_io_stop_server(server);

}
