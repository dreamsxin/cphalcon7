
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

#include "socket/server.h"
#include "socket.h"
#include "socket/client.h"
#include "socket/exception.h"

#if HAVE_EPOLL

#include <sys/epoll.h>
#include <errno.h>

#endif /* HAVE_EPOLL */

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
 * Phalcon\Socket\Server
 */
zend_class_entry *phalcon_socket_server_ce;

PHP_METHOD(Phalcon_Socket_Server, __construct);
PHP_METHOD(Phalcon_Socket_Server, listen);
PHP_METHOD(Phalcon_Socket_Server, accept);
PHP_METHOD(Phalcon_Socket_Server, run);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_server___construct, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, address, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, domain, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, protocol, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_server_bind, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, address, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_server_listen, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, backlog, IS_LONG, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_socket_server_method_entry[] = {
	PHP_ME(Phalcon_Socket_Server, __construct, arginfo_phalcon_socket_server___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Socket_Server, listen, arginfo_phalcon_socket_server_listen, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, accept, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, run, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Socket\Server initializer
 */
PHALCON_INIT_CLASS(Phalcon_Socket_Server){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Socket, Server, socket_server, phalcon_socket_ce,  phalcon_socket_server_method_entry, 0);

	return SUCCESS;
}

/**
 * Phalcon\Socket\Server constructor
 *
 * @param string $address
 * @param int $port
 * @param int $domain
 * @param int $type
 * @param int $protocol
 * @throws \Phalcon\Socket\Exception
 */
PHP_METHOD(Phalcon_Socket_Server, __construct){

	zval *address, *port, *_domain = NULL, *_type = NULL, *_protocol = NULL, domain = {}, type = {}, protocol = {}, socket = {};

	phalcon_fetch_params(0, 2, 3, &address, &port, &_domain, &_type, &_protocol);

	if (!_domain || Z_TYPE_P(_domain) == IS_NULL) {
		phalcon_read_property(&domain, getThis(), SL("_domain"), PH_NOISY);
	} else {
		PHALCON_CPY_WRT(&domain, _domain);
	}

	if (!_type || Z_TYPE_P(_type) == IS_NULL) {
		phalcon_read_property(&type, getThis(), SL("_type"), PH_NOISY);
	} else {
		PHALCON_CPY_WRT(&type, _type);
	}

	if (!_protocol || Z_TYPE_P(_protocol) == IS_NULL) {
		phalcon_read_property(&protocol, getThis(), SL("_protocol"), PH_NOISY);
	} else {
		PHALCON_CPY_WRT(&protocol, _protocol);
	}

	PHALCON_CALL_FUNCTIONW(&socket, "socket_create", &domain, &type, &protocol);
	if (Z_TYPE(socket) != IS_RESOURCE) {
		PHALCON_CALL_METHODW(NULL, getThis(), "_throwsocketexception");
		return;
	}
	phalcon_update_property_zval(getThis(), SL("_socket"), &socket);

	PHALCON_CALL_FUNCTIONW(return_value, "socket_bind", &socket, address, port);
	if (!PHALCON_IS_TRUE(return_value)) {
		PHALCON_CALL_METHODW(NULL, getThis(), "_throwsocketexception");
	}
}

/**
 * Listens for a connection on a socket
 *
 * @param int $backlog
 * @return boolean
 */
PHP_METHOD(Phalcon_Socket_Server, listen){

	zval *backlog = NULL, socket = {};

	phalcon_fetch_params(0, 0, 1, &backlog);

	if (!backlog) {
		backlog = &PHALCON_GLOBAL(z_zero);
	}

	phalcon_read_property(&socket, getThis(), SL("_socket"), PH_NOISY);

	PHALCON_CALL_FUNCTIONW(return_value, "socket_listen", &socket, backlog);
	if (!PHALCON_IS_TRUE(return_value)) {
		PHALCON_CALL_METHODW(NULL, getThis(), "_throwsocketexception");
	}
}

/**
 * Accept a connection
 *
 * @return Phalcon\Socket\Server
 */
PHP_METHOD(Phalcon_Socket_Server, accept){

	zval socket = {}, client_socket = {};

	phalcon_read_property(&socket, getThis(), SL("_socket"), PH_NOISY);

	PHALCON_CALL_FUNCTIONW(&client_socket, "socket_accept", &socket);

	if (PHALCON_IS_FALSE(&client_socket)) {
		RETURN_FALSE;
	} else {
		object_init_ex(return_value, phalcon_socket_client_ce);
		PHALCON_CALL_METHODW(NULL, return_value, "__construct", &client_socket);
	}
}

/**
 * Run the Server
 *
 */
PHP_METHOD(Phalcon_Socket_Server, run){

}
