
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

#include <Zend/zend_closures.h>

#if HAVE_EPOLL
# include <sys/epoll.h>
# include <errno.h>
#endif

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
PHP_METHOD(Phalcon_Socket_Server, getClient);
PHP_METHOD(Phalcon_Socket_Server, close);
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_server_getclient, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, socketId, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_server_close, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, socketId, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_socket_server_run, 0, 0, 0)
	ZEND_ARG_CALLABLE_INFO(0, onconnection, 1)
	ZEND_ARG_CALLABLE_INFO(0, onrecv, 1)
	ZEND_ARG_CALLABLE_INFO(0, onsend, 1)
	ZEND_ARG_CALLABLE_INFO(0, onclose, 1)
	ZEND_ARG_CALLABLE_INFO(0, onerror, 1)
	ZEND_ARG_CALLABLE_INFO(0, ontimeout, 1)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, usec, IS_LONG, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_socket_server_method_entry[] = {
	PHP_ME(Phalcon_Socket_Server, __construct, arginfo_phalcon_socket_server___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Socket_Server, listen, arginfo_phalcon_socket_server_listen, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, accept, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, getClient, arginfo_phalcon_socket_server_getclient, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, close, arginfo_phalcon_socket_server_close, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Socket_Server, run, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Socket\Server initializer
 */
PHALCON_INIT_CLASS(Phalcon_Socket_Server){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Socket, Server, socket_server, phalcon_socket_ce,  phalcon_socket_server_method_entry, 0);

	zend_declare_property_null(phalcon_socket_server_ce, SL("_clients"),		ZEND_ACC_PROTECTED);

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

	phalcon_update_property_empty_array(getThis(), SL("_clients"));
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

	zval socket = {}, client_socket = {}, socket_id = {};

	phalcon_read_property(&socket, getThis(), SL("_socket"), PH_NOISY);

	PHALCON_CALL_FUNCTIONW(&client_socket, "socket_accept", &socket);

	if (PHALCON_IS_FALSE(&client_socket)) {
		RETURN_FALSE;
	} else {
		object_init_ex(return_value, phalcon_socket_client_ce);
		PHALCON_CALL_METHODW(NULL, return_value, "__construct", &client_socket);
		PHALCON_CALL_METHODW(&socket_id, return_value, "getsocketid");

		phalcon_update_property_array(getThis(), SL("_clients"), &socket_id, return_value);
	}
}

/**
 * Gets a connection
 *
 * @return Phalcon\Socket\Client
 */
PHP_METHOD(Phalcon_Socket_Server, getClient){

	zval *socket_id;

	phalcon_fetch_params(0, 1, 0, &socket_id);

	phalcon_read_property_array(return_value, getThis(), SL("_clients"), socket_id);
}

/**
 * Close a connection
 *
 * @return Phalcon\Socket\Server
 */
PHP_METHOD(Phalcon_Socket_Server, close){

	zval *socket_id, client = {};

	phalcon_fetch_params(0, 1, 0, &socket_id);

	phalcon_read_property_array(&client, getThis(), SL("_clients"), socket_id);
	if (Z_TYPE(client) == IS_OBJECT) {
		PHALCON_CALL_METHODW(NULL, &client, "close");
	}
	phalcon_unset_property_array(getThis(), SL("_clients"), socket_id);
}

/**
 * Run the Server
 *
 */
PHP_METHOD(Phalcon_Socket_Server, run){

	zval *_onconnection = NULL, *_onrecv = NULL, *_onsend = NULL, *_onclose = NULL, *_onerror = NULL, *_ontimeout = NULL, *timeout = NULL, *usec = NULL;
	zval onconnection = {}, onrecv = {}, onsend = {}, onclose = {}, onerror = {}, ontimeout, socket = {}, maxlen = {};

	phalcon_fetch_params(0, 0, 8, &_onconnection, &_onrecv, &_onsend, &_onclose, &_onerror, &_ontimeout, &timeout, &usec);

	if (_onconnection) {
		if (instanceof_function_ex(Z_OBJCE_P(_onconnection), zend_ce_closure, 0)) {
				PHALCON_CALL_CE_STATICW(&onconnection, zend_ce_closure, "bind", _onconnection, getThis());
		} else {
			PHALCON_CPY_WRT(&onconnection, _onconnection);
		}
	}

	if (_onrecv) {
		if (instanceof_function_ex(Z_OBJCE_P(_onrecv), zend_ce_closure, 0)) {
				PHALCON_CALL_CE_STATICW(&onrecv, zend_ce_closure, "bind", _onrecv, getThis());
		} else {
			PHALCON_CPY_WRT(&onrecv, _onrecv);
		}
	}

	if (_onsend) {
		if (instanceof_function_ex(Z_OBJCE_P(_onsend), zend_ce_closure, 0)) {
				PHALCON_CALL_CE_STATICW(&onsend, zend_ce_closure, "bind", _onsend, getThis());
		} else {
			PHALCON_CPY_WRT(&onsend, _onsend);
		}
	}

	if (_onclose) {
		if (instanceof_function_ex(Z_OBJCE_P(_onclose), zend_ce_closure, 0)) {
				PHALCON_CALL_CE_STATICW(&onclose, zend_ce_closure, "bind", _onclose, getThis());
		} else {
			PHALCON_CPY_WRT(&onclose, _onclose);
		}
	}

	if (_onerror) {
		if (instanceof_function_ex(Z_OBJCE_P(_onerror), zend_ce_closure, 0)) {
				PHALCON_CALL_CE_STATICW(&onerror, zend_ce_closure, "bind", _onerror, getThis());
		} else {
			PHALCON_CPY_WRT(&onerror, _onerror);
		}
	}

	if (_ontimeout) {
		if (instanceof_function_ex(Z_OBJCE_P(_ontimeout), zend_ce_closure, 0)) {
				PHALCON_CALL_CE_STATICW(&ontimeout, zend_ce_closure, "bind", _ontimeout, getThis());
		} else {
			PHALCON_CPY_WRT(&ontimeout, _ontimeout);
		}
	}

	if (!timeout) {
		timeout = &PHALCON_GLOBAL(z_null);
	}

	if (!usec) {
		usec = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&socket, getThis(), SL("_socket"), PH_NOISY);
	phalcon_read_property(&maxlen, getThis(), SL("_maxlen"), PH_NOISY);

#if HAVE_EPOLL
//#else
	while(1) {
		zval r_array = {}, w_array = {}, e_array = {}, ret = {}, clients = {}, *client = NULL, *client_socket = NULL;

		array_init(&r_array);
		array_init(&w_array);
		array_init(&e_array);

		phalcon_array_append(&r_array, &socket, PH_COPY);

		phalcon_read_property(&clients, getThis(), SL("_clients"), PH_NOISY);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(clients), client) {
			zval socket = {};
			PHALCON_CALL_METHODW(&socket, client, "getsocket");
			phalcon_array_append(&r_array, &socket, PH_COPY);
			phalcon_array_append(&w_array, &socket, PH_COPY);
		} ZEND_HASH_FOREACH_END();

		ZVAL_MAKE_REF(&r_array);
		ZVAL_MAKE_REF(&w_array);
		ZVAL_MAKE_REF(&e_array);
		PHALCON_CALL_FUNCTIONW(&ret, "socket_select", &r_array, &w_array, &e_array, timeout, usec);
		ZVAL_UNREF(&r_array);
		ZVAL_UNREF(&w_array);
		ZVAL_UNREF(&e_array);

		if (PHALCON_IS_FALSE(&ret)) {
			PHALCON_CALL_METHODW(NULL, getThis(), "_throwsocketexception");
			return;
		}
		if (PHALCON_IS_LONG_IDENTICAL(&ret, 0)) {
			if (phalcon_method_exists_ex(getThis(), SL("ontimeout")) == SUCCESS) {
				PHALCON_CALL_METHODW(NULL, getThis(), "ontimeout");
			}
			if (Z_TYPE(ontimeout) > IS_NULL) {
				PHALCON_CALL_USER_FUNCW(NULL, &ontimeout);
			}
			continue;
		}

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(r_array), client_socket) {
			zval client_socket_id = {}, client = {}, *args, data = {};
			ZVAL_LONG(&client_socket_id, Z_RES_HANDLE_P(client_socket));
			if (PHALCON_IS_IDENTICAL(client_socket, &socket)) {
				PHALCON_CALL_METHODW(&client, getThis(), "accept");
				if (phalcon_method_exists_ex(getThis(), SL("onconnection")) == SUCCESS) {
					PHALCON_CALL_METHODW(NULL, getThis(), "onconnection", &client);
				}
				if (Z_TYPE(onconnection) > IS_NULL) {
					args = (zval *)safe_emalloc(1, sizeof(zval), 0);
					ZVAL_COPY(&args[0], &client);
					PHALCON_CALL_USER_FUNC_PARAMS(return_value, &onconnection, args, 1);
				}
			} else {
				PHALCON_CALL_METHODW(&client, getThis(), "getclient", &client_socket_id);
				PHALCON_CALL_METHODW(&data, &client, "read", &maxlen);
				if (PHALCON_IS_EMPTY_STRING(&data)) {
					if (phalcon_method_exists_ex(getThis(), SL("onclose")) == SUCCESS) {
						PHALCON_CALL_METHODW(NULL, getThis(), "onclose", &client);
					}
					if (Z_TYPE(onclose) > IS_NULL) {
						args = (zval *)safe_emalloc(1, sizeof(zval), 0);
						ZVAL_COPY(&args[0], &client);
						PHALCON_CALL_USER_FUNC_PARAMS(return_value, &onclose, args, 1);
					}
					PHALCON_CALL_METHODW(NULL, getThis(), "close", &client_socket_id);
				} else if (PHALCON_IS_FALSE(&data)) {
					if (phalcon_method_exists_ex(getThis(), SL("onerror")) == SUCCESS) {
						PHALCON_CALL_METHODW(NULL, getThis(), "onerror", &client);
					}
					if (Z_TYPE(onerror) > IS_NULL) {
						args = (zval *)safe_emalloc(1, sizeof(zval), 0);
						ZVAL_COPY(&args[0], &client);
						PHALCON_CALL_USER_FUNC_PARAMS(return_value, &onerror, args, 1);
					}
				} else {
					if (phalcon_method_exists_ex(getThis(), SL("onrecv")) == SUCCESS) {
						PHALCON_CALL_METHODW(NULL, getThis(), "onrecv", &client, &data);
					}
					if (Z_TYPE(onrecv) > IS_NULL) {
						args = (zval *)safe_emalloc(2, sizeof(zval), 0);
						ZVAL_COPY(&args[0], &client);
						ZVAL_COPY_VALUE(&args[1], &data);
						PHALCON_CALL_USER_FUNC_PARAMS(return_value, &onrecv, args, 2);
					}
				}
			}
		} ZEND_HASH_FOREACH_END();

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(w_array), client_socket) {
			zval client_socket_id = {}, client = {}, *args, data = {};
			ZVAL_LONG(&client_socket_id, Z_RES_HANDLE_P(client_socket));
				PHALCON_CALL_METHODW(&client, getThis(), "getclient", &client_socket_id);
				if (phalcon_method_exists_ex(getThis(), SL("on")) == SUCCESS) {
					PHALCON_CALL_METHODW(NULL, getThis(), "onsend", &client, &data);
				}
				if (Z_TYPE(onsend) > IS_NULL) {
					args = (zval *)safe_emalloc(1, sizeof(zval), 0);
					ZVAL_COPY(&args[0], &client);
					PHALCON_CALL_USER_FUNC_PARAMS(return_value, &onsend, args, 1);
				}
		} ZEND_HASH_FOREACH_END();
	}
#endif
}
