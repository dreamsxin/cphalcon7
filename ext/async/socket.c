/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Martin Schröder <m.schroeder2007@gmail.com>                 |
  +----------------------------------------------------------------------+
*/

#include "async/core.h"
#include "async/async_stream.h"
#include "async/async_socket.h"

ASYNC_API zend_class_entry *async_server_ce;
ASYNC_API zend_class_entry *async_socket_ce;
ASYNC_API zend_class_entry *async_socket_accept_exception_ce;
ASYNC_API zend_class_entry *async_socket_bind_exception_ce;
ASYNC_API zend_class_entry *async_socket_connect_exception_ce;
ASYNC_API zend_class_entry *async_socket_disconnect_exception_ce;
ASYNC_API zend_class_entry *async_socket_exception_ce;
ASYNC_API zend_class_entry *async_socket_listen_exception_ce;
ASYNC_API zend_class_entry *async_socket_stream_ce;


static PHP_METHOD(Socket, close) { }
static PHP_METHOD(Socket, getAddress) { }
static PHP_METHOD(Socket, getPort) { }
static PHP_METHOD(Socket, setOption) { }

static const zend_function_entry async_socket_functions[] = {
	PHP_ME(Socket, close, arginfo_stream_close, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_ME(Socket, getAddress, arginfo_socket_get_address, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_ME(Socket, getPort, arginfo_socket_get_port, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_ME(Socket, setOption, arginfo_socket_set_option, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_FE_END
};


static PHP_METHOD(SocketStream, getRemoteAddress) { }
static PHP_METHOD(SocketStream, getRemotePort) { }
static PHP_METHOD(SocketStream, isAlive) { }
static PHP_METHOD(SocketStream, flush) { }
static PHP_METHOD(SocketStream, getWriteQueueSize) { }

static const zend_function_entry async_socket_stream_functions[] = {
	PHP_ME(SocketStream, getRemoteAddress, arginfo_socket_stream_get_remote_address, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_ME(SocketStream, getRemotePort, arginfo_socket_stream_get_remote_port, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_ME(SocketStream, isAlive, arginfo_socket_stream_is_alive, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_ME(SocketStream, flush, arginfo_socket_stream_flush, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_ME(SocketStream, getWriteQueueSize, arginfo_socket_get_write_queue_size, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_FE_END
};


PHP_METHOD(Server, accept) { }

static const zend_function_entry async_server_functions[] = {
	PHP_ME(Server, accept, arginfo_server_accept, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_FE_END
};


static const zend_function_entry empty_funcs[] = {
	PHP_FE_END
};


void async_socket_ce_register()
{
	zend_class_entry ce;
	
	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Network", "Socket", async_socket_functions);
	async_socket_ce = zend_register_internal_interface(&ce);
	
	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Network", "SocketStream", async_socket_stream_functions);
	async_socket_stream_ce = zend_register_internal_interface(&ce);

	zend_class_implements(async_socket_stream_ce, 2, async_socket_ce, async_duplex_stream_ce);
	
	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Network", "Server", async_server_functions);
	async_server_ce = zend_register_internal_interface(&ce);

	zend_class_implements(async_server_ce, 1, async_socket_ce);

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Network", "SocketException", empty_funcs);
	async_socket_exception_ce = zend_register_internal_class(&ce);

	zend_do_inheritance(async_socket_exception_ce, async_stream_exception_ce);
	
	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Network", "SocketAcceptException", empty_funcs);
	async_socket_accept_exception_ce = zend_register_internal_class(&ce);

	zend_do_inheritance(async_socket_accept_exception_ce, async_socket_exception_ce);
	
	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Network", "SocketBindException", empty_funcs);
	async_socket_bind_exception_ce = zend_register_internal_class(&ce);

	zend_do_inheritance(async_socket_bind_exception_ce, async_socket_exception_ce);
	
	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Network", "SocketConnectException", empty_funcs);
	async_socket_connect_exception_ce = zend_register_internal_class(&ce);

	zend_do_inheritance(async_socket_connect_exception_ce, async_socket_exception_ce);
	
	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Network", "SocketDisconnectException", empty_funcs);
	async_socket_disconnect_exception_ce = zend_register_internal_class(&ce);

	zend_do_inheritance(async_socket_disconnect_exception_ce, async_socket_exception_ce);
	
	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Network", "SocketListenException", empty_funcs);
	async_socket_listen_exception_ce = zend_register_internal_class(&ce);

	zend_do_inheritance(async_socket_listen_exception_ce, async_socket_exception_ce);
}
