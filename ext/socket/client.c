
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

#include "socket/client.h"
#include "socket.h"
#include "socket/exception.h"

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
 * Phalcon\Socket\Client
 *
 *<code>
 *
 *	$client = new Phalcon\Socket\Client('127.0.0.1', 8989);
 *	if ($client->connect()) {
 *	    $client->write('Hello world!');
 *	    while($ret = $client->read(1024, PHP_NORMAL_READ)) {
 *          echo $ret;
 *	    }
 *	} else {
 *	    echo 'connect fail'.PHP_EOL;
 *	}
 *
 *</code>
 */
zend_class_entry *phalcon_socket_client_ce;

PHP_METHOD(Phalcon_Socket_Client, __construct);
PHP_METHOD(Phalcon_Socket_Client, connect);
PHP_METHOD(Phalcon_Socket_Client, read);
PHP_METHOD(Phalcon_Socket_Client, write);
PHP_METHOD(Phalcon_Socket_Client, recv);
PHP_METHOD(Phalcon_Socket_Client, send);
PHP_METHOD(Phalcon_Socket_Client, keepAlive);
PHP_METHOD(Phalcon_Socket_Client, shutdown);
PHP_METHOD(Phalcon_Socket_Client, close);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_client___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, address)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, domain, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, protocol, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_client_read, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, legnth, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_client_write, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, buffer, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_client_recv, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, legnth, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, flag, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_client_send, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, buffer, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, flag, IS_LONG, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_socket_client_method_entry[] = {
	PHP_ME(Phalcon_Socket_Client, __construct, arginfo_phalcon_socket_client___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Socket_Client, connect, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Client, read, arginfo_phalcon_socket_client_read, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Client, write, arginfo_phalcon_socket_client_write, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Client, recv, arginfo_phalcon_socket_client_recv, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Client, send, arginfo_phalcon_socket_client_send, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Client, keepAlive, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Client, shutdown, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Client, close, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Socket\Server initializer
 */
PHALCON_INIT_CLASS(Phalcon_Socket_Client){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Socket, Client, socket_client, phalcon_socket_ce,  phalcon_socket_client_method_entry, 0);

	return SUCCESS;
}

/**
 * Phalcon\Socket\Client constructor
 */
PHP_METHOD(Phalcon_Socket_Client, __construct){

	zval *address, *port = NULL, *_domain = NULL, *_type = NULL, *_protocol = NULL, domain = {}, type = {}, protocol = {}, socket = {};
	zval filter_type = {}, filter_option = {}, filtered = {};

	phalcon_fetch_params(0, 1, 4, &address, &port, &_domain, &_type, &_protocol);

	if (Z_TYPE_P(address) == IS_RESOURCE) {
		phalcon_update_property(getThis(), SL("_socket"), address);
	} else {
		if (!port) {
			port = &PHALCON_GLOBAL(z_zero);
		}

		phalcon_update_property(getThis(), SL("_address"), address);
		phalcon_update_property(getThis(), SL("_port"), port);

		if (!_domain || Z_TYPE_P(_domain) == IS_NULL) {
			ZVAL_LONG(&filter_type, 275); // FILTER_VALIDATE_IP
			ZVAL_LONG(&filter_option, 1048576); // FILTER_FLAG_IPV4
			PHALCON_CALL_FUNCTION(&filtered, "filter_var", address, &filter_type, &filter_option);
			if (zend_is_true(&filtered)) {
				ZVAL_LONG(&domain, PHALCON_SOCKET_AF_INET);
			} else {
				ZVAL_LONG(&filter_option, 2097152); // FILTER_FLAG_IPV6
				PHALCON_CALL_FUNCTION(&filtered, "filter_var", address, &filter_type, &filter_option);
				if (zend_is_true(&filtered)) {
					ZVAL_LONG(&domain, PHALCON_SOCKET_AF_INET6);
				} else {
					if ( Z_LVAL_P(port) > 0) {
						ZVAL_LONG(&domain, PHALCON_SOCKET_AF_INET);
					} else {
						ZVAL_LONG(&domain, PHALCON_SOCKET_AF_UNIX);
					}
				}
			}
		} else {
			ZVAL_COPY_VALUE(&domain, _domain);
		}
		if (!_type || Z_TYPE_P(_type) == IS_NULL) {
			phalcon_read_property(&type, getThis(), SL("_type"), PH_NOISY|PH_READONLY);
		} else {
			ZVAL_COPY_VALUE(&type, _type);
		}

		if (!_protocol || Z_TYPE_P(_protocol) == IS_NULL) {
			phalcon_read_property(&protocol, getThis(), SL("_protocol"), PH_NOISY|PH_READONLY);
		} else {
			ZVAL_COPY_VALUE(&protocol, _protocol);
		}

		PHALCON_CALL_FUNCTION(&socket, "socket_create", &domain, &type, &protocol);
		if (Z_TYPE(socket) != IS_RESOURCE) {
			PHALCON_CALL_METHOD(NULL, getThis(), "_throwsocketexception");
			return;
		}

		phalcon_update_property(getThis(), SL("_socket"), &socket);
	}
}

/**
 *  Connect to a socket
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Socket_Client, connect){

	zval address = {}, port = {}, socket = {};

	phalcon_read_property(&address, getThis(), SL("_address"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&port, getThis(), SL("_port"), PH_NOISY|PH_READONLY);

	if (!zend_is_true(&address)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_socket_exception_ce, "The socket cant't connect");
		RETURN_FALSE;
	}

	phalcon_read_property(&socket, getThis(), SL("_socket"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_FUNCTION(return_value, "socket_connect", &socket, &address, &port);
	if (!PHALCON_IS_TRUE(return_value)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "_throwsocketexception");
	}
}


/**
 * Reads a maximum of length bytes from a socket
 *
 * @param int $length
 * @param int $type
 * @return string
 */
PHP_METHOD(Phalcon_Socket_Client, read){

	zval *length, *type = NULL, socket = {};

	phalcon_fetch_params(0, 1, 1, &length, &type);

	phalcon_read_property(&socket, getThis(), SL("_socket"), PH_NOISY|PH_READONLY);

	if (!type) {
		PHALCON_CALL_FUNCTION(return_value, "socket_read", &socket, length);
	} else {
		PHALCON_CALL_FUNCTION(return_value, "socket_read", &socket, length, type);
	}

	if (PHALCON_IS_FALSE(return_value)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "_throwsocketexception");
	}
}

/**
 * Write to a socket
 *
 * @param string $buffer
 * @param int $length
 * @return Phalcon\Socket\Client
 */
PHP_METHOD(Phalcon_Socket_Client, write){

	zval *buffer, *length = NULL, socket = {}, writebuf = {}, writelen = {}, ret = {};
	int len;

	phalcon_fetch_params(0, 1, 1, &buffer, &length);

	if (!length || Z_TYPE_P(length) != IS_LONG) {
		len = Z_STRLEN_P(buffer);
	} else {
		len = Z_LVAL_P(length);
	}

	phalcon_read_property(&socket, getThis(), SL("_socket"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(socket) != IS_RESOURCE) {
		RETURN_FALSE;
	}

	ZVAL_LONG(&ret, 0);
	ZVAL_DUP(&writebuf, buffer);
	while(1) {
		ZVAL_LONG(&writelen, len);
		PHALCON_CALL_FUNCTION(&ret, "socket_write", &socket, &writebuf, &writelen);

		if (Z_TYPE(ret) == IS_LONG && Z_LVAL(ret) < len) {
			len -= Z_LVAL(ret);
			phalcon_substr(&writebuf, &writebuf, Z_LVAL(ret), len);
		} else {
			break;
		}
    }

	if (PHALCON_IS_FALSE(&ret)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "_throwsocketexception");
	}
	RETURN_THIS();
}

/**
 * Receives data from a connected socket
 *
 * @param int $length
 * @param int $flag
 * @return string
 */
PHP_METHOD(Phalcon_Socket_Client, recv){

	zval *length, *flag, socket = {}, ret = {};

	phalcon_fetch_params(0, 2, 0, &length, &flag);

	phalcon_read_property(&socket, getThis(), SL("_socket"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(socket) != IS_RESOURCE) {
		RETURN_FALSE;
	}

	PHALCON_CALL_FUNCTION(&ret, "socket_recv", &socket, return_value, length, flag);

	if (PHALCON_IS_FALSE(&ret)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "_throwsocketexception");
	}
}

/**
 * Sends data to a connected socket
 *
 * @param string $buffer
 * @param int $length
 * @param int $flag
 * @return Phalcon\Socket\Client
 */
PHP_METHOD(Phalcon_Socket_Client, send){

	zval *buffer, *length, *flag, socket = {}, writebuf = {}, writelen = {}, ret = {};
	int len;

	phalcon_fetch_params(0, 3, 0, &buffer, &length, &flag);

	phalcon_read_property(&socket, getThis(), SL("_socket"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(socket) != IS_RESOURCE) {
		RETURN_FALSE;
	}

	ZVAL_LONG(&ret, 0);
	ZVAL_DUP(&writebuf, buffer);

	while(1) {
		ZVAL_LONG(&writelen, len);
		PHALCON_CALL_FUNCTION(&ret, "socket_send", &socket, &writebuf, &writelen, flag);

		if (Z_TYPE(ret) == IS_LONG && Z_LVAL(ret) < len) {
			len -= Z_LVAL(ret);
			phalcon_substr(&writebuf, &writebuf, Z_LVAL(ret), len);
		} else {
			break;
		}
    }

	if (PHALCON_IS_FALSE(&ret)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "_throwsocketexception");
	}
	RETURN_THIS();
}

/**
 * Enable keepalive
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Socket_Client, keepAlive){

	zval socket = {}, *sol_socket, *so_keepalive;

	phalcon_read_property(&socket, getThis(), SL("_socket"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(socket) != IS_RESOURCE) {
		RETURN_FALSE;
	}
	if ((sol_socket = zend_get_constant_str(SL("SOL_SOCKET"))) == NULL) {
		RETURN_FALSE;
	}
	if ((so_keepalive = zend_get_constant_str(SL("SO_KEEPALIVE"))) == NULL) {
		RETURN_FALSE;
	}

	PHALCON_CALL_FUNCTION(NULL, "socket_set_option", &socket, sol_socket, so_keepalive, &PHALCON_GLOBAL(z_one));

	RETURN_TRUE;
}

/**
 * Shutdown
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Socket_Client, shutdown){

	zval socket = {};

	phalcon_read_property(&socket, getThis(), SL("_socket"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(socket) != IS_RESOURCE) {
		RETURN_FALSE;
	}
	PHALCON_CALL_FUNCTION(return_value, "socket_shutdown", &socket);
}

/**
 * Close
 *
 */
PHP_METHOD(Phalcon_Socket_Client, close){

	zval socket = {};

	phalcon_read_property(&socket, getThis(), SL("_socket"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(socket) == IS_RESOURCE) {
		PHALCON_CALL_FUNCTION(NULL, "socket_close", &socket);
		phalcon_update_property_null(getThis(), SL("_socket"));
	}
}
