
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
  |          Martin Schröder <m.schroeder2007@gmail.com>                   |
  +------------------------------------------------------------------------+
*/

#include "async/core.h"

#if PHALCON_USE_UV

#include "async/async_stream.h"
#include "async/async_socket.h"

ASYNC_API zend_class_entry *async_server_ce;
ASYNC_API zend_class_entry *async_socket_ce;
ASYNC_API zend_class_entry *async_socket_exception_ce;
ASYNC_API zend_class_entry *async_socket_stream_ce;


ZEND_METHOD(Socket, close) { }
ZEND_METHOD(Socket, getAddress) { }
ZEND_METHOD(Socket, getPort) { }
ZEND_METHOD(Socket, setOption) { }

static const zend_function_entry async_socket_functions[] = {
	ZEND_ME(Socket, close, arginfo_stream_close, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	ZEND_ME(Socket, getAddress, arginfo_socket_get_address, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	ZEND_ME(Socket, getPort, arginfo_socket_get_port, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	ZEND_ME(Socket, setOption, arginfo_socket_set_option, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	ZEND_FE_END
};


ZEND_METHOD(SocketStream, getRemoteAddress) { }
ZEND_METHOD(SocketStream, getRemotePort) { }
ZEND_METHOD(SocketStream, flush) { }
ZEND_METHOD(SocketStream, writeAsync) { }
ZEND_METHOD(SocketStream, getWriteQueueSize) { }

static const zend_function_entry async_socket_stream_functions[] = {
	ZEND_ME(SocketStream, getRemoteAddress, arginfo_socket_stream_get_remote_address, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	ZEND_ME(SocketStream, getRemotePort, arginfo_socket_stream_get_remote_port, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	ZEND_ME(SocketStream, flush, arginfo_socket_stream_flush, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	ZEND_ME(SocketStream, writeAsync, arginfo_socket_stream_write_async, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	ZEND_ME(SocketStream, getWriteQueueSize, arginfo_socket_get_write_queue_size, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	ZEND_FE_END
};


ZEND_METHOD(Server, accept) { }

static const zend_function_entry async_server_functions[] = {
	ZEND_ME(Server, accept, arginfo_server_accept, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	ZEND_FE_END
};


static const zend_function_entry empty_funcs[] = {
	ZEND_FE_END
};


void async_socket_ce_register()
{
	zend_class_entry ce;
	
	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Network\\Socket", async_socket_functions);
	async_socket_ce = zend_register_internal_interface(&ce);
	
	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Network\\SocketStream", async_socket_stream_functions);
	async_socket_stream_ce = zend_register_internal_interface(&ce);

	zend_class_implements(async_socket_stream_ce, 2, async_socket_ce, async_duplex_stream_ce);
	
	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Network\\Server", async_server_functions);
	async_server_ce = zend_register_internal_interface(&ce);

	zend_class_implements(async_server_ce, 1, async_socket_ce);

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Network\\SocketException", empty_funcs);
	async_socket_exception_ce = zend_register_internal_class(&ce);

	zend_do_inheritance(async_socket_exception_ce, async_stream_exception_ce);
}

#endif
