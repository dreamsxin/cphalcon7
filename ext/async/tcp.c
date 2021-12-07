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
#include "async/async_ssl.h"
#include "async/async_stream.h"
#include "async/async_socket.h"
#include "async/async_pipe.h"
#include <Zend/zend_inheritance.h>
#include "kernel/backend.h"

#ifdef ZEND_WIN32
#include "win32/sockets.h"
#else
#include <sys/uio.h>
#endif

#define ASYNC_SOCKET_TCP_NODELAY 100
#define ASYNC_SOCKET_TCP_KEEPALIVE 101
#define ASYNC_SOCKET_TCP_SIMULTANEOUS_ACCEPTS 150

ASYNC_API zend_class_entry *async_tcp_socket_ce;
ASYNC_API zend_class_entry *async_tcp_server_ce;

static zend_object_handlers async_tcp_socket_handlers;
static zend_object_handlers async_tcp_server_handlers;

static zend_string *str_wildcard;

#define ASYNC_TCP_SERVER_FLAG_LAZY 1

typedef struct _async_tcp_server {
	/* PHP object handle. */
	zend_object std;

	/* Task scheduler being used. */
	async_task_scheduler *scheduler;

	/* UV TCP handle. */
	uv_tcp_t handle;
	
	uint8_t flags;

	/* Hostname or IP address that was used to establish the connection. */
	zend_string *name;

	zend_string *addr;
	uint16_t port;

	/* Number of pending connection attempts queued in the backlog. */
	zend_uchar pending;

	/* Error being used to close the server. */
	zval error;
	
	/* Number of referenced accept operations. */
	zend_uchar ref_count;

	/* Queue of tasks waiting to accept a socket connection. */
	async_op_list accepts;
	
	async_cancel_cb cancel;

#ifdef HAVE_ASYNC_SSL
	/* TLS server encryption settings. */
	async_tls_server_encryption *encryption;
	
	async_ssl_settings settings;

	/* Server SSL context (shared between all socket connections). */
	SSL_CTX *ctx;
#endif
} async_tcp_server;

typedef struct _async_tcp_socket {
	/* PHP object handle. */
	zend_object std;

	/* Task scheduler being used. */
	async_task_scheduler *scheduler;

	/* UV TCP handle. */
	uv_tcp_t handle;
	
	async_cancel_cb cancel;

	/* Hostname or IP address that was used to establish the connection. */
	zend_string *name;
	
	zend_string *local_addr;
	uint16_t local_port;
	
	zend_string *remote_addr;
	uint16_t remote_port;

	/* Refers to the (local) server that accepted the TCP socket connection. */
	async_tcp_server *server;
	
	async_stream *stream;

	/* Error being used to close the read stream. */
	zval read_error;

	/* Error being used to close the write stream. */
	zval write_error;

#ifdef HAVE_ASYNC_SSL
	/* TLS client encryption settings. */
	async_tls_client_encryption *encryption;
#endif
} async_tcp_socket;

static async_tcp_socket *async_tcp_socket_object_create();

#define ASYNC_TCP_SOCKET_CONST(name, value) \
	zend_declare_class_constant_long(async_tcp_socket_ce, name, sizeof(name)-1, (zend_long)value);

#define ASYNC_TCP_SERVER_CONST(name, value) \
	zend_declare_class_constant_long(async_tcp_server_ce, name, sizeof(name)-1, (zend_long)value);

typedef struct _async_tcp_write {
	uv_write_t request;
	async_tcp_socket *socket;
	zend_string *data;
	uv_buf_t *buffers;
	unsigned int nbufs;
	uv_buf_t *tmp;
} async_tcp_write;


ASYNC_CALLBACK socket_disposed(uv_handle_t *handle)
{
	async_tcp_socket *socket;

	socket = (async_tcp_socket *) handle->data;
	
	ZEND_ASSERT(socket != NULL);
	
	ASYNC_DELREF(&socket->std);
}

ASYNC_CALLBACK shutdown_socket(void *arg, zval *error)
{
	async_tcp_socket *socket;
	
	zval obj;

	socket = (async_tcp_socket *) arg;
	
	ZEND_ASSERT(socket != NULL);
	
	socket->cancel.func = NULL;

	if (error != NULL) {
		if (Z_TYPE_P(&socket->read_error) == IS_UNDEF) {
			ZVAL_COPY(&socket->read_error, error);
		}
		
		if (Z_TYPE_P(&socket->write_error) == IS_UNDEF) {
			ZVAL_COPY(&socket->write_error, error);
		}
	}
	
	if (socket->stream == NULL) {
		socket->handle.data = socket;

		ASYNC_UV_TRY_CLOSE_REF(&socket->std, &socket->handle, socket_disposed);
	} else {
		ZVAL_OBJ(&obj, &socket->std);
		
		async_stream_close(socket->stream, &obj);
	}
}


static async_tcp_socket *async_tcp_socket_object_create()
{
	async_tcp_socket *socket;

	socket = ecalloc(1, sizeof(async_tcp_socket));

	zend_object_std_init(&socket->std, async_tcp_socket_ce);
	socket->std.handlers = &async_tcp_socket_handlers;
	
	socket->scheduler = async_task_scheduler_ref();
	
	socket->cancel.object = socket;
	socket->cancel.func = shutdown_socket;
	
	ASYNC_LIST_APPEND(&socket->scheduler->shutdown, &socket->cancel);

	uv_tcp_init(&socket->scheduler->loop, &socket->handle);

	socket->stream = async_stream_init((uv_stream_t *) &socket->handle, 0);

	return socket;
}

static void async_tcp_socket_object_dtor(zend_object *object)
{
	async_tcp_socket *socket;

	socket = (async_tcp_socket *) object;
	
	if (socket->cancel.func != NULL) {
		ASYNC_LIST_REMOVE(&socket->scheduler->shutdown, &socket->cancel);
		
		socket->cancel.func(socket, NULL);
	}
}

static void async_tcp_socket_object_destroy(zend_object *object)
{
	async_tcp_socket *socket;
	
	socket = (async_tcp_socket *) object;

#ifdef HAVE_ASYNC_SSL
	if (socket->stream->ssl.ssl != NULL) {
		async_ssl_dispose_engine(&socket->stream->ssl, (socket->server == NULL) ? 1 : 0);
	}

	if (socket->encryption != NULL) {
		ASYNC_DELREF(&socket->encryption->std);
	}
#endif

	if (socket->stream != NULL) {
		async_stream_free(socket->stream);
	}

	if (socket->server != NULL) {
		ASYNC_DELREF(&socket->server->std);
	}
	
	zval_ptr_dtor(&socket->read_error);
	zval_ptr_dtor(&socket->write_error);

	if (socket->name != NULL) {
		zend_string_release(socket->name);
	}
	
	if (socket->local_addr != NULL) {
		zend_string_release(socket->local_addr);
	}
	
	if (socket->remote_addr != NULL) {
		zend_string_release(socket->remote_addr);
	}
	
	async_task_scheduler_unref(socket->scheduler);

	zend_object_std_dtor(&socket->std);
}

ASYNC_CALLBACK connect_cb(uv_connect_t *req, int status)
{
	async_uv_op *op;

	op = (async_uv_op *) req->data;

	ZEND_ASSERT(op != NULL);
	
	op->code = status;
	
	ASYNC_FINISH_OP(op);
}

static int setup_client_tls(async_tcp_socket *socket, zval *tls)
{
	if (tls != NULL && Z_TYPE_P(tls) != IS_NULL) {
#ifdef HAVE_ASYNC_SSL
		socket->encryption = async_clone_client_encryption((async_tls_client_encryption *) Z_OBJ_P(tls));
		socket->encryption->settings.mode = ASYNC_SSL_MODE_CLIENT;

		if (socket->encryption->settings.peer_name == NULL) {
			socket->encryption->settings.peer_name = zend_string_copy(socket->name);
		}
#else
		zend_throw_exception_ex(async_socket_exception_ce, 0, "Socket encryption requires async extension to be compiled with SSL support");
		
		return FAILURE;
#endif
	}
	
	return SUCCESS;
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_tcp_socket_connect, 0, 2, Phalcon\\Async\\Network\\TcpSocket, 0)
	ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, tls, Phalcon\\Async\\Network\\TlsClientEncryption, 1)
ZEND_END_ARG_INFO();

static PHP_METHOD(TcpSocket, connect)
{
	async_tcp_socket *socket;
	async_context *context;
	async_uv_op *op;
	
	zend_string *name;
	zend_long port;

	zval *tls;

	uv_connect_t req;
	uv_os_fd_t sock;
	php_sockaddr_storage dest;
	int code;

	tls = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 3)
	    Z_PARAM_STR(name)
		Z_PARAM_LONG(port)
		Z_PARAM_OPTIONAL
		Z_PARAM_OBJECT_OF_CLASS_EX(tls, async_tls_client_encryption_ce, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	ASYNC_CHECK_EXCEPTION(async_task_scheduler_get()->flags & ASYNC_TASK_SCHEDULER_FLAG_DISPOSED, async_socket_exception_ce, "Task scheduler has been disposed");
	ASYNC_CHECK_EXCEPTION(async_task_scheduler_get()->flags & ASYNC_TASK_SCHEDULER_FLAG_ERROR, async_socket_exception_ce, "Task scheduler was stopped due to an error");

	code = async_dns_lookup_ip(ZSTR_VAL(name), &dest, IPPROTO_TCP);

	ASYNC_CHECK_EXCEPTION(code < 0, async_socket_exception_ce, "Failed to assemble IP address: %s", uv_strerror(code));

	async_socket_set_port((struct sockaddr *) &dest, port);

	socket = async_tcp_socket_object_create();
	socket->name = zend_string_copy(name);

	code = uv_tcp_connect(&req, &socket->handle, (const struct sockaddr *) &dest, connect_cb);

	if (UNEXPECTED(code != 0)) {
		zend_throw_exception_ex(async_socket_connect_exception_ce, 0, "Failed to connect socket: %s", uv_strerror(code));
		ASYNC_DELREF(&socket->std);
		return;
	}
	
	ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_uv_op));
	
	req.data = op;
	
	context = async_context_get();
	
	if (!async_context_is_background(context) && 1 == ++socket->stream->ref_count) {
		uv_ref((uv_handle_t *) &socket->handle);
	}
	
	code = async_await_op((async_op *) op);
	
	if (!async_context_is_background(context) && 0 == --socket->stream->ref_count) {
		uv_unref((uv_handle_t *) &socket->handle);
	}
	
	if (UNEXPECTED(code == FAILURE)) {
		ASYNC_DELREF(&socket->std);
		ASYNC_FORWARD_OP_ERROR(op);
		ASYNC_FREE_OP(op);
		
		return;
	}
	
	code = op->code;
	
	ASYNC_FREE_OP(op);
	
	if (UNEXPECTED(code < 0)) {
		zend_throw_exception_ex(async_socket_connect_exception_ce, 0, "Failed to connect socket: %s", uv_strerror(code));
		ASYNC_DELREF(&socket->std);		
		return;
	}
	
	if (UNEXPECTED(SUCCESS != setup_client_tls(socket, tls))) {
		ASYNC_DELREF(&socket->std);
		return;
	}
	
	if (EXPECTED(0 == uv_fileno((const uv_handle_t *) &socket->handle, &sock))) {
		async_socket_get_local_peer((php_socket_t) sock, &socket->local_addr, &socket->local_port);
		async_socket_get_remote_peer((php_socket_t) sock, &socket->remote_addr, &socket->remote_port);
	}

	RETURN_OBJ(&socket->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_tcp_socket_import, 0, 1, Phalcon\\Async\\Network\\TcpSocket, 0)
	ZEND_ARG_OBJ_INFO(0, pipe, Phalcon\\Async\\Network\\Pipe, 0)
	ZEND_ARG_OBJ_INFO(0, tls, Phalcon\\Async\\Network\\TlsClientEncryption, 1)
ZEND_END_ARG_INFO();

static PHP_METHOD(TcpSocket, import)
{
	async_tcp_socket *socket;
	uv_os_fd_t sock;
	
	zval *conn;
	zval *tls;
	
	tls = NULL;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 2)
		Z_PARAM_OBJECT_OF_CLASS(conn, async_pipe_ce)
		Z_PARAM_OPTIONAL
		Z_PARAM_OBJECT_OF_CLASS_EX(tls, async_tls_client_encryption_ce, 1, 0)
	ZEND_PARSE_PARAMETERS_END();
	
	socket = async_tcp_socket_object_create();
	
	async_pipe_import_stream((async_pipe *) Z_OBJ_P(conn), (uv_stream_t *) &socket->handle);
	
	if (UNEXPECTED(EG(exception))) {
		ASYNC_DELREF(&socket->std);
		return;
	}
	
	if (SUCCESS != setup_client_tls(socket, tls)) {
		ASYNC_DELREF(&socket->std);
		return;
	}
	
	if (EXPECTED(0 == uv_fileno((const uv_handle_t *) &socket->handle, &sock))) {
		if (EXPECTED(SUCCESS == async_socket_get_local_peer((php_socket_t) sock, &socket->local_addr, &socket->local_port))) {
			socket->name = zend_string_copy(socket->local_addr);
		} else {
			socket->name = ZSTR_EMPTY_ALLOC();
		}
		
		async_socket_get_remote_peer((php_socket_t) sock, &socket->remote_addr, &socket->remote_port);
	}
	
	RETURN_OBJ(&socket->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tcp_socket_export, 0, 1, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, pipe, Phalcon\\Async\\Network\\Pipe, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(TcpSocket, export)
{
	async_tcp_socket *socket;
	
	zval *ipc;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_OBJECT_OF_CLASS(ipc, async_pipe_ce)
	ZEND_PARSE_PARAMETERS_END();
	
	socket = (async_tcp_socket *) Z_OBJ_P(getThis());

	async_pipe_export_stream((async_pipe *) Z_OBJ_P(ipc), (uv_stream_t *) &socket->handle);
}

#ifdef ZEND_WIN32
#define tcp_socket_pair(sock) socketpair(AF_INET, SOCK_STREAM, IPPROTO_IP, sock)
#else

static int tcp_socket_pair(php_socket_t sock[2])
{
	struct sockaddr_in address;
	int redirect;
	socklen_t size;
	
	sock[0] = sock[1] = redirect = -1;
	sock[0]	= socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	
	if (-1 == sock[0]) {
		goto error;
	}

	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_family = AF_INET;
	address.sin_port = 0;

	if (bind(sock[0], (struct sockaddr *) &address, sizeof(struct sockaddr_in)) != 0) {
		goto error;
	}
	
	size = sizeof(struct sockaddr_in);

	if (getsockname(sock[0], (struct sockaddr *) &address, &size) != 0) {
		goto error;
	}

	if (listen(sock[0], 2) != 0) {
		goto error;
	}

	sock[1] = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	
	if (-1 == sock[1]) {
		goto error;
	}

	address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	
	if (connect(sock[1], (struct sockaddr *) &address, sizeof(struct sockaddr_in)) != 0) {
		goto error;
	}

	redirect = accept(sock[0], (struct sockaddr *) &address, &size);
	
	if (-1 == redirect) {
		goto error;
	}

	closesocket(sock[0]);
	sock[0] = redirect;

	return 0;

error:
	close(redirect);
	close(sock[0]);
	close(sock[1]);
	
	errno = ECONNABORTED;
	
	return -1;
}

#endif

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tcp_socket_pair, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(TcpSocket, pair)
{
	async_tcp_socket *socket;

	php_socket_t tmp[2];
	zval sockets[2];
	
	int i;

	ZEND_PARSE_PARAMETERS_NONE();

	if (UNEXPECTED(!USED_RET())) {
		return;
	}
	
	i = tcp_socket_pair(tmp);
	
	ASYNC_CHECK_EXCEPTION(i != 0, async_socket_exception_ce, "Failed to create socket pair: %s", uv_strerror(uv_translate_sys_error(php_socket_errno())));

	array_init_size(return_value, 2);

	for (i = 0; i < 2; i++) {
		socket = async_tcp_socket_object_create();

		uv_tcp_open(&socket->handle, (uv_os_sock_t) tmp[i]);

		ZVAL_OBJ(&sockets[i], &socket->std);
		ASYNC_STR(socket->local_addr, "127.0.0.1");

		socket->remote_addr = zend_string_copy(socket->local_addr);

		zend_hash_index_update(Z_ARRVAL_P(return_value), i, &sockets[i]);
	}
}

static PHP_METHOD(TcpSocket, close)
{
	async_tcp_socket *socket;

	zval error;
	zval *val;

	val = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_OBJECT_OF_CLASS_EX(val, zend_ce_throwable, 1, 0)
	ZEND_PARSE_PARAMETERS_END();
	
	socket = (async_tcp_socket *) Z_OBJ_P(getThis());
	
	if (socket->cancel.func == NULL) {
		return;
	}

	ASYNC_PREPARE_EXCEPTION(&error, execute_data, async_stream_closed_exception_ce, "Socket has been closed");

	if (val != NULL && Z_TYPE_P(val) != IS_NULL) {
		zend_exception_set_previous(Z_OBJ_P(&error), Z_OBJ_P(val));
		GC_ADDREF(Z_OBJ_P(val));
	}

	ASYNC_LIST_REMOVE(&socket->scheduler->shutdown, &socket->cancel);

	socket->cancel.func(socket, &error);

	zval_ptr_dtor(&error);
}

static PHP_METHOD(TcpSocket, flush)
{
	async_tcp_socket *socket;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	socket = (async_tcp_socket *) Z_OBJ_P(getThis());
	
	if (Z_TYPE_P(&socket->write_error) != IS_UNDEF) {
		ASYNC_FORWARD_ERROR(&socket->write_error);
		return;
	}
	
	async_stream_flush(socket->stream);
}

static PHP_METHOD(TcpSocket, getAddress)
{
	async_tcp_socket *socket;

	ZEND_PARSE_PARAMETERS_NONE();

	socket = (async_tcp_socket *) Z_OBJ_P(getThis());
	
	RETURN_STR_COPY(socket->local_addr);
}

static PHP_METHOD(TcpSocket, getPort)
{
	async_tcp_socket *socket;

	ZEND_PARSE_PARAMETERS_NONE();

	socket = (async_tcp_socket *) Z_OBJ_P(getThis());
	
	RETURN_LONG(socket->local_port);
}

static PHP_METHOD(TcpSocket, getRemoteAddress)
{
	async_tcp_socket *socket;

	ZEND_PARSE_PARAMETERS_NONE();

	socket = (async_tcp_socket *) Z_OBJ_P(getThis());
	
	RETURN_STR_COPY(socket->remote_addr);
}

static PHP_METHOD(TcpSocket, getRemotePort)
{
	async_tcp_socket *socket;

	ZEND_PARSE_PARAMETERS_NONE();

	socket = (async_tcp_socket *) Z_OBJ_P(getThis());
	
	RETURN_LONG(socket->remote_port);
}

static PHP_METHOD(TcpSocket, setOption)
{
	async_tcp_socket *socket;

	zend_long option;
	zval *val;

	int code;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_LONG(option)
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();

	socket = (async_tcp_socket *) Z_OBJ_P(getThis());
	code = 0;

	switch ((int) option) {
	case ASYNC_SOCKET_TCP_NODELAY:
		code = uv_tcp_nodelay(&socket->handle, Z_LVAL_P(val) ? 1 : 0);
		break;
	case ASYNC_SOCKET_TCP_KEEPALIVE:
		code = uv_tcp_keepalive(&socket->handle, Z_LVAL_P(val) ? 1 : 0, (unsigned int) Z_LVAL_P(val));
		break;
	}

	RETURN_BOOL((code < 0) ? 0 : 1);
}

static PHP_METHOD(TcpSocket, isAlive)
{
	async_tcp_socket *socket;
	
	ZEND_PARSE_PARAMETERS_NONE();

	socket = (async_tcp_socket *) Z_OBJ_P(getThis());
	
	RETURN_BOOL(async_socket_is_alive(socket->stream));
}

static PHP_METHOD(TcpSocket, read)
{
	async_tcp_socket *socket;

	socket = (async_tcp_socket *) Z_OBJ_P(getThis());

	async_stream_call_read(socket->stream, &socket->read_error, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static PHP_METHOD(TcpSocket, getReadableStream)
{
	async_tcp_socket *socket;
	async_stream_reader *reader;

	ZEND_PARSE_PARAMETERS_NONE();

	socket = (async_tcp_socket *) Z_OBJ_P(getThis());
	reader = async_stream_reader_create(socket->stream, &socket->std, &socket->read_error);

	RETURN_OBJ(&reader->std);
}

static PHP_METHOD(TcpSocket, write)
{
	async_tcp_socket *socket;

	socket = (async_tcp_socket *) Z_OBJ_P(getThis());

	async_stream_call_write(socket->stream, &socket->write_error, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static PHP_METHOD(TcpSocket, getWriteQueueSize)
{
	async_tcp_socket *socket;
	
	ZEND_PARSE_PARAMETERS_NONE();

	socket = (async_tcp_socket *) Z_OBJ_P(getThis());
	
	RETURN_LONG((Z_TYPE_P(&socket->write_error) == IS_UNDEF) ? socket->handle.write_queue_size : 0);
}

static PHP_METHOD(TcpSocket, getWritableStream)
{
	async_tcp_socket *socket;
	async_stream_writer *writer;

	ZEND_PARSE_PARAMETERS_NONE();

	socket = (async_tcp_socket *) Z_OBJ_P(getThis());
	writer = async_stream_writer_create(socket->stream, &socket->std, &socket->write_error);

	RETURN_OBJ(&writer->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_tcp_socket_encrypt, 0, 0, Phalcon\\Async\\Network\\TlsInfo, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(TcpSocket, encrypt)
{
#ifndef HAVE_ASYNC_SSL
	zend_throw_exception_ex(async_socket_exception_ce, 0, "Async extension was not compiled with SSL support");
#else

	async_tcp_socket *socket;
	async_ssl_handshake_data handshake;
	
	char *cafile;
	char *capath;
	
	int code;
	int options;

	ZEND_PARSE_PARAMETERS_NONE();

	socket = (async_tcp_socket *) Z_OBJ_P(getThis());
	
	memset(&handshake, 0, sizeof(async_ssl_handshake_data));
	
	if (socket->server == NULL) {
		ASYNC_CHECK_ERROR(socket->encryption == NULL, "No encryption settings have been passed to TcpSocket::connect()");
	
		options = SSL_OP_SINGLE_DH_USE;
	
		cafile = (socket->encryption->cafile == NULL) ? NULL : ZSTR_VAL(socket->encryption->cafile);
		capath = (socket->encryption->capath == NULL) ? NULL : ZSTR_VAL(socket->encryption->capath);
	
		socket->stream->ssl.ctx = async_ssl_create_context(options, cafile, capath);
		
		async_ssl_setup_verify_callback(socket->stream->ssl.ctx, &socket->encryption->settings);
		
		handshake.settings = &socket->encryption->settings;
		handshake.host = socket->name;
		
		async_ssl_setup_client_alpn(socket->stream->ssl.ctx, socket->encryption->alpn, 0);
	} else {
		ASYNC_CHECK_EXCEPTION(socket->server->encryption == NULL, async_socket_exception_ce, "No encryption settings have been passed to TcpServer::listen()");

		socket->stream->ssl.ctx = socket->server->ctx;
		
		handshake.settings = &socket->server->settings;
	}
	
	async_ssl_create_buffered_engine(&socket->stream->ssl, socket->stream->buffer.size);
	async_ssl_setup_encryption(socket->stream->ssl.ssl, handshake.settings);
	
	uv_tcp_nodelay(&socket->handle, 1);
	
	code = async_stream_ssl_handshake(socket->stream, &handshake);

	uv_tcp_nodelay(&socket->handle, 0);
	
	if (UNEXPECTED(code == FAILURE)) {
		if (handshake.error != NULL) {
			zend_throw_exception_ex(async_socket_exception_ce, 0, "SSL handshake failed: %s", ZSTR_VAL(handshake.error));
			zend_string_release(handshake.error);
			return;
		}
	
		if (handshake.uv_error < 0) {
			zend_throw_exception_ex(async_socket_exception_ce, 0, "SSL handshake failed due to network error: %s", uv_strerror(handshake.uv_error));
			return;
		}
		
		if (handshake.ssl_error != SSL_ERROR_NONE) {
			zend_throw_exception_ex(async_socket_exception_ce, 0, "SSL handshake failed [%d]: %s", handshake.ssl_error, ERR_reason_error_string(handshake.ssl_error));
			return;
		}
	}
	
	if (UNEXPECTED(handshake.error != NULL)) {
		zend_string_release(handshake.error);
	}
	
	RETURN_OBJ(&async_tls_info_object_create(socket->stream->ssl.ssl)->std);
#endif
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_CTOR(TcpSocket, async_tcp_socket_ce)
ASYNC_METHOD_NO_WAKEUP(TcpSocket, async_tcp_socket_ce)
//LCOV_EXCL_STOP

static const zend_function_entry async_tcp_socket_functions[] = {
	PHP_ME(TcpSocket, __construct, arginfo_no_ctor, ZEND_ACC_PRIVATE)
	PHP_ME(TcpSocket, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(TcpSocket, connect, arginfo_tcp_socket_connect, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(TcpSocket, import, arginfo_tcp_socket_import, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(TcpSocket, pair, arginfo_tcp_socket_pair, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(TcpSocket, close, arginfo_stream_close, ZEND_ACC_PUBLIC)
	PHP_ME(TcpSocket, flush, arginfo_socket_stream_flush, ZEND_ACC_PUBLIC)
	PHP_ME(TcpSocket, getAddress, arginfo_socket_get_address, ZEND_ACC_PUBLIC)
	PHP_ME(TcpSocket, getPort, arginfo_socket_get_port, ZEND_ACC_PUBLIC)
	PHP_ME(TcpSocket, setOption, arginfo_socket_set_option, ZEND_ACC_PUBLIC)
	PHP_ME(TcpSocket, getRemoteAddress, arginfo_socket_stream_get_remote_address, ZEND_ACC_PUBLIC)
	PHP_ME(TcpSocket, getRemotePort, arginfo_socket_stream_get_remote_port, ZEND_ACC_PUBLIC)
	PHP_ME(TcpSocket, isAlive, arginfo_socket_stream_is_alive, ZEND_ACC_PUBLIC)
	PHP_ME(TcpSocket, read, arginfo_readable_stream_read, ZEND_ACC_PUBLIC)
	PHP_ME(TcpSocket, getReadableStream, arginfo_duplex_stream_get_readable_stream, ZEND_ACC_PUBLIC)
	PHP_ME(TcpSocket, write, arginfo_writable_stream_write, ZEND_ACC_PUBLIC)
	PHP_ME(TcpSocket, getWriteQueueSize, arginfo_socket_get_write_queue_size, ZEND_ACC_PUBLIC)
	PHP_ME(TcpSocket, getWritableStream, arginfo_duplex_stream_get_writable_stream, ZEND_ACC_PUBLIC)
	PHP_ME(TcpSocket, export, arginfo_tcp_socket_export, ZEND_ACC_PUBLIC)
	PHP_ME(TcpSocket, encrypt, arginfo_tcp_socket_encrypt, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


ASYNC_CALLBACK server_disposed(uv_handle_t *handle)
{
	async_tcp_server *server;

	server = (async_tcp_server *) handle->data;
	
	ZEND_ASSERT(server != NULL);

	ASYNC_DELREF(&server->std);
}

ASYNC_CALLBACK shutdown_server(void *obj, zval *error)
{
	async_tcp_server *server;
	async_uv_op *op;
	
	server = (async_tcp_server *) obj;
	
	ZEND_ASSERT(server != NULL);
	
	server->cancel.func = NULL;
	
	if (error != NULL && Z_TYPE_P(&server->error) == IS_UNDEF) {
		ZVAL_COPY(&server->error, error);
	}
	
	while (server->accepts.first != NULL) {
		ASYNC_NEXT_CUSTOM_OP(&server->accepts, op, async_uv_op);
		
		if (Z_TYPE_P(&server->error) != IS_UNDEF) {
			ASYNC_FAIL_OP(op, &server->error);
		} else {
			op->code = UV_ECANCELED;
		
			ASYNC_FINISH_OP(op);
		}
	}

	ASYNC_UV_TRY_CLOSE_REF(&server->std, &server->handle, server_disposed);
}

static async_tcp_server *async_tcp_server_object_create(int domain)
{
	async_tcp_server *server;

	server = ecalloc(1, sizeof(async_tcp_server));

	zend_object_std_init(&server->std, async_tcp_server_ce);
	server->std.handlers = &async_tcp_server_handlers;
	
	server->scheduler = async_task_scheduler_ref();
	
	server->cancel.object = server;
	server->cancel.func = shutdown_server;
	
	ASYNC_LIST_APPEND(&server->scheduler->shutdown, &server->cancel);
	
	uv_tcp_init_ex(&server->scheduler->loop, &server->handle, domain);

	server->handle.data = server;
	
#ifdef HAVE_ASYNC_SSL
	server->settings.mode = ASYNC_SSL_MODE_SERVER;
#endif

	return server;
}

static void async_tcp_server_object_dtor(zend_object *object)
{
	async_tcp_server *server;

	server = (async_tcp_server *) object;

	if (server->cancel.func != NULL) {
		ASYNC_LIST_REMOVE(&server->scheduler->shutdown, &server->cancel);
	
		server->cancel.func(server, NULL);
	}
}

static void async_tcp_server_object_destroy(zend_object *object)
{
	async_tcp_server *server;

	server = (async_tcp_server *) object;

#ifdef HAVE_ASYNC_SSL
	if (server->ctx != NULL) {
		SSL_CTX_free(server->ctx);
	}

	if (server->encryption != NULL) {
		ASYNC_DELREF(&server->encryption->std);
	}
#endif
	
	zval_ptr_dtor(&server->error);
	
	if (server->name != NULL) {
		zend_string_release(server->name);
	}
	
	if (server->addr != NULL) {
		zend_string_release(server->addr);
	}
	
	async_task_scheduler_unref(server->scheduler);

	zend_object_std_dtor(&server->std);
}

ASYNC_CALLBACK server_connected(uv_stream_t *stream, int status)
{
	async_tcp_server *server;
	async_uv_op *op;

	server = (async_tcp_server *) stream->data;
	
	ZEND_ASSERT(server != NULL);
	
	if (server->accepts.first == NULL) {
		server->pending++;
	} else {
		ASYNC_NEXT_CUSTOM_OP(&server->accepts, op, async_uv_op);
		
		op->code = status;
				
		ASYNC_FINISH_OP(op);
	}
}

static int setup_server_tls(async_tcp_server *server, zval *tls)
{
#ifdef HAVE_ASYNC_SSL
	int options;
	char *cafile;
	char *capath;
#endif

	if (tls != NULL && Z_TYPE_P(tls) != IS_NULL) {
#ifdef HAVE_ASYNC_SSL
		server->encryption = (async_tls_server_encryption *) Z_OBJ_P(tls);

		ASYNC_ADDREF(&server->encryption->std);
		
		options = SSL_OP_SINGLE_DH_USE | SSL_OP_CIPHER_SERVER_PREFERENCE;
		
		cafile = (server->encryption->cafile == NULL) ? NULL : ZSTR_VAL(server->encryption->cafile);
		capath = (server->encryption->capath == NULL) ? NULL : ZSTR_VAL(server->encryption->capath);
		
		server->ctx = async_ssl_create_context(options, cafile, capath);

		SSL_CTX_set_default_passwd_cb_userdata(server->ctx, &server->encryption->cert);

		SSL_CTX_use_certificate_chain_file(server->ctx, ZSTR_VAL(server->encryption->cert.file));
		SSL_CTX_use_PrivateKey_file(server->ctx, ZSTR_VAL(server->encryption->cert.key), SSL_FILETYPE_PEM);

		async_ssl_setup_server_sni(server->ctx, server->encryption);
		async_ssl_setup_server_alpn(server->ctx, server->encryption);
#else
		zend_throw_exception_ex(async_socket_exception_ce, 0, "Server encryption requires async extension to be compiled with SSL support");
		
		return FAILURE;
#endif
	}
	
	return SUCCESS;
}

static void create_server(async_tcp_server **result, INTERNAL_FUNCTION_PARAMETERS)
{
	async_tcp_server *server;

	zval *name;
	zend_long port;
	zend_string *host;

	zval *tls;
	zend_bool reuseport;

	php_sockaddr_storage addr;
	php_socket_t sock;
	int code;

	*result = NULL;
	
	name = NULL;
	port = 0;
	tls = NULL;
	reuseport = 0;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 4)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(name)
		Z_PARAM_LONG(port)
		Z_PARAM_OBJECT_OF_CLASS_EX(tls, async_tls_server_encryption_ce, 1, 0)
		Z_PARAM_BOOL(reuseport)
	ZEND_PARSE_PARAMETERS_END();

	if (name == NULL || Z_TYPE_P(name) == IS_NULL) {
		host = str_wildcard;
	} else {
		host = Z_STR_P(name);
	}

	code = async_dns_lookup_ip(ZSTR_VAL(host), &addr, IPPROTO_TCP);
	
	ASYNC_CHECK_EXCEPTION(code < 0, async_socket_exception_ce, "Failed to assemble IP address: %s", uv_strerror(code));
	
	async_socket_set_port((struct sockaddr *) &addr, port);

	server = async_tcp_server_object_create(addr.ss_family);
	
	if (UNEXPECTED(0 != uv_fileno((const uv_handle_t *) &server->handle, (uv_os_fd_t *) &sock))) {
		zend_throw_exception_ex(async_socket_bind_exception_ce, 0, "Failed to create socket handle");
		ASYNC_DELREF(&server->std);
		return;
	}
	
	server->name = zend_string_copy(host);
	server->port = (uint16_t) port;
	
	async_socket_set_reuseaddr(sock, 1);
	
	if (reuseport) {
		async_socket_set_reuseport(sock, 1);
	}
	
#ifdef HAVE_IPV6
	code = (addr.ss_family == AF_INET6) ? UV_TCP_IPV6ONLY : 0;
#else
	code = 0;
#endif

	code = uv_tcp_bind(&server->handle, (const struct sockaddr *) &addr, code);

	if (UNEXPECTED(code != 0)) {
		zend_throw_exception_ex(async_socket_bind_exception_ce, 0, "Failed to bind server: %s", uv_strerror(code));
		ASYNC_DELREF(&server->std);
		return;
	}

	uv_unref((uv_handle_t *) &server->handle);

	async_socket_get_local_peer((php_socket_t) sock, &server->addr, &server->port);

	if (UNEXPECTED(SUCCESS != setup_server_tls(server, tls))) {
		ASYNC_DELREF(&server->std);
		return;
	}
	
	*result = server;
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_tcp_server_bind, 0, 0, Phalcon\\Async\\Network\\TcpServer, 0)
	ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 1)
	ZEND_ARG_OBJ_INFO(0, tls, Phalcon\\Async\\Network\\TlsServerEncryption, 1)
	ZEND_ARG_TYPE_INFO(0, reuseport, _IS_BOOL, 1)
ZEND_END_ARG_INFO();

static PHP_METHOD(TcpServer, bind)
{
	async_tcp_server *server;
	
	create_server(&server, INTERNAL_FUNCTION_PARAM_PASSTHRU);
	
	if (EXPECTED(server)) {
		server->flags = ASYNC_TCP_SERVER_FLAG_LAZY;
		
		RETURN_OBJ(&server->std);
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_tcp_server_listen, 0, 0, Phalcon\\Async\\Network\\TcpServer, 0)
	ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 1)
	ZEND_ARG_OBJ_INFO(0, tls, Phalcon\\Async\\Network\\TlsServerEncryption, 1)
	ZEND_ARG_TYPE_INFO(0, reuseport, _IS_BOOL, 1)
ZEND_END_ARG_INFO();

static PHP_METHOD(TcpServer, listen)
{
	async_tcp_server *server;
	
	int code;
	
	create_server(&server, INTERNAL_FUNCTION_PARAM_PASSTHRU);
	
	if (EXPECTED(server)) {
		code = uv_listen((uv_stream_t *) &server->handle, 128, server_connected);

		if (UNEXPECTED(code != 0)) {
			zend_throw_exception_ex(async_socket_listen_exception_ce, 0, "Server failed to listen: %s", uv_strerror(code));
			ASYNC_DELREF(&server->std);
			return;
		}
	
		RETURN_OBJ(&server->std);
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_tcp_server_import, 0, 1, Phalcon\\Async\\Network\\TcpServer, 0)
	ZEND_ARG_OBJ_INFO(0, pipe, Phalcon\\Async\\Network\\Pipe, 0)
	ZEND_ARG_OBJ_INFO(0, tls, Phalcon\\Async\\Network\\TlsServerEncryption, 1)
ZEND_END_ARG_INFO();

static PHP_METHOD(TcpServer, import)
{
	async_tcp_server *server;
	uv_os_fd_t sock;
	
	zval *conn;
	zval *tls;
	
	tls = NULL;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 2)
		Z_PARAM_OBJECT_OF_CLASS(conn, async_pipe_ce)
		Z_PARAM_OPTIONAL
		Z_PARAM_OBJECT_OF_CLASS_EX(tls, async_tls_server_encryption_ce, 1, 0)
	ZEND_PARSE_PARAMETERS_END();
	
	server = async_tcp_server_object_create(AF_UNSPEC);
	server->flags = ASYNC_TCP_SERVER_FLAG_LAZY;
	
	async_pipe_import_stream((async_pipe *) Z_OBJ_P(conn), (uv_stream_t *) &server->handle);
	
	if (UNEXPECTED(EG(exception))) {
		ASYNC_DELREF(&server->std);
		return;
	}
	
	if (EXPECTED(0 == uv_fileno((const uv_handle_t *) &server->handle, &sock))) {
		if (SUCCESS == async_socket_get_local_peer((php_socket_t) sock, &server->addr, &server->port)) {
			server->name = zend_string_copy(server->addr);
		} else {
			server->name = ZSTR_EMPTY_ALLOC();
		}
	}

	uv_unref((uv_handle_t *) &server->handle);
	
	RETURN_OBJ(&server->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tcp_server_export, 0, 1, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, pipe, Phalcon\\Async\\Network\\Pipe, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(TcpServer, export)
{
	async_tcp_server *server;
	
	zval *ipc;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_OBJECT_OF_CLASS(ipc, async_pipe_ce)
	ZEND_PARSE_PARAMETERS_END();
	
	server = (async_tcp_server *) Z_OBJ_P(getThis());

	async_pipe_export_stream((async_pipe *) Z_OBJ_P(ipc), (uv_stream_t *) &server->handle);
}

static PHP_METHOD(TcpServer, close)
{
	async_tcp_server *server;

	zval error;
	zval *val;

	val = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_OBJECT_OF_CLASS_EX(val, zend_ce_throwable, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	server = (async_tcp_server *) Z_OBJ_P(getThis());

	if (server->cancel.func == NULL) {
		return;
	}

	ASYNC_PREPARE_EXCEPTION(&error, execute_data, async_socket_exception_ce, "Server has been closed");

	if (val != NULL && Z_TYPE_P(val) != IS_NULL) {
		zend_exception_set_previous(Z_OBJ_P(&error), Z_OBJ_P(val));
		GC_ADDREF(Z_OBJ_P(val));
	}
	
	ASYNC_LIST_REMOVE(&server->scheduler->shutdown, &server->cancel);
	
	server->cancel.func(server, &error);

	zval_ptr_dtor(&error);
}

static PHP_METHOD(TcpServer, getAddress)
{
	async_tcp_server *server;

	ZEND_PARSE_PARAMETERS_NONE();

	server = (async_tcp_server *) Z_OBJ_P(getThis());

	RETURN_STR_COPY(server->name);
}

static PHP_METHOD(TcpServer, getPort)
{
	async_tcp_server *server;

	ZEND_PARSE_PARAMETERS_NONE();

	server = (async_tcp_server *) Z_OBJ_P(getThis());
	
	RETURN_LONG(server->port);
}

static PHP_METHOD(TcpServer, setOption)
{
	async_tcp_server *server;

	zend_long option;
	zval *val;

	int code;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_LONG(option)
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();

	server = (async_tcp_server *) Z_OBJ_P(getThis());
	code = 0;

	switch ((int) option) {
	case ASYNC_SOCKET_TCP_SIMULTANEOUS_ACCEPTS:
		code = uv_tcp_simultaneous_accepts(&server->handle, Z_LVAL_P(val) ? 1 : 0);
		break;
	}

	RETURN_BOOL((code < 0) ? 0 : 1);
}

static PHP_METHOD(TcpServer, accept)
{
	async_tcp_server *server;
	async_tcp_socket *socket;
	async_context *context;
	
	async_uv_op *op;
	uv_os_fd_t sock;

	int code;

	ZEND_PARSE_PARAMETERS_NONE();

	server = (async_tcp_server *) Z_OBJ_P(getThis());
	
	if (server->flags & ASYNC_TCP_SERVER_FLAG_LAZY) {
		code = uv_listen((uv_stream_t *) &server->handle, 128, server_connected);
	
		ASYNC_CHECK_EXCEPTION(code != 0, async_socket_listen_exception_ce, "Server failed to listen: %s", uv_strerror(code));
		
		server->flags ^= ASYNC_TCP_SERVER_FLAG_LAZY;
	}

	if (server->pending == 0) {
		if (UNEXPECTED(Z_TYPE_P(&server->error) != IS_UNDEF)) {
			ASYNC_FORWARD_ERROR(&server->error);
			return;
		}
		
		ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_uv_op));
		ASYNC_APPEND_OP(&server->accepts, op);
		
		context = async_context_get();
		
		ASYNC_UNREF_ENTER(context, server);
		code = async_await_op((async_op *) op);
		ASYNC_UNREF_EXIT(context, server);
		
		if (UNEXPECTED(code == FAILURE)) {
			ASYNC_FORWARD_OP_ERROR(op);
			ASYNC_FREE_OP(op);
			
			return;
		}
		
		code = op->code;
		
		ASYNC_FREE_OP(op);
		
		ASYNC_CHECK_EXCEPTION(code < 0, async_socket_accept_exception_ce, "Failed to accept socket connection: %s", uv_strerror(code));
	} else {
		server->pending--;
	}

	socket = async_tcp_socket_object_create();

	code = uv_accept((uv_stream_t *) &server->handle, (uv_stream_t *) &socket->handle);

	if (UNEXPECTED(code != 0)) {
		zend_throw_exception_ex(async_socket_accept_exception_ce, 0, "Failed to accept socket connection: %s", uv_strerror(code));
		ASYNC_DELREF(&socket->std);
		
		return;
	}

	socket->server = server;
	
	if (EXPECTED(0 == uv_fileno((const uv_handle_t *) &socket->handle, &sock))) {
		async_socket_get_local_peer((php_socket_t) sock, &socket->local_addr, &socket->local_port);
		async_socket_get_remote_peer((php_socket_t) sock, &socket->remote_addr, &socket->remote_port);
	}

	ASYNC_ADDREF(&server->std);

	RETURN_OBJ(&socket->std);
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_CTOR(TcpServer, async_tcp_server_ce)
ASYNC_METHOD_NO_WAKEUP(TcpServer, async_tcp_server_ce)
//LCOV_EXCL_STOP

static const zend_function_entry async_tcp_server_functions[] = {
	PHP_ME(TcpServer, __construct, arginfo_no_ctor, ZEND_ACC_PRIVATE)
	PHP_ME(TcpServer, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(TcpServer, bind, arginfo_tcp_server_bind, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(TcpServer, listen, arginfo_tcp_server_listen, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(TcpServer, import, arginfo_tcp_server_import, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(TcpServer, close, arginfo_stream_close, ZEND_ACC_PUBLIC)
	PHP_ME(TcpServer, getAddress, arginfo_socket_get_address, ZEND_ACC_PUBLIC)
	PHP_ME(TcpServer, getPort, arginfo_socket_get_port, ZEND_ACC_PUBLIC)
	PHP_ME(TcpServer, setOption, arginfo_socket_set_option, ZEND_ACC_PUBLIC)
	PHP_ME(TcpServer, accept, arginfo_server_accept, ZEND_ACC_PUBLIC)
	PHP_ME(TcpServer, export, arginfo_tcp_server_export, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


ASYNC_CALLBACK finish_async_write(async_op *op)
{
	async_awaitable_impl *awaitable;

	awaitable = (async_awaitable_impl *) op->arg;

	ZEND_ASSERT(awaitable != NULL);

	if (op->status == ASYNC_STATUS_FAILED) {
		async_awaitable_fail(awaitable, &op->result);
	} else {
		async_awaitable_resolve(awaitable, NULL);
	}

	ASYNC_DELREF((zend_object *) awaitable->arg);
	ASYNC_DELREF(&awaitable->std);
}

static void intercept_write(async_context *context, zend_object *obj, INTERNAL_FUNCTION_PARAMETERS)
{
	async_tcp_socket *socket;
	async_stream_write_req write;
	async_awaitable_impl *awaitable;

	zend_string *data;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STR(data)
	ZEND_PARSE_PARAMETERS_END();

	socket = (async_tcp_socket *) obj;

	if (UNEXPECTED(Z_TYPE_P(&socket->write_error) != IS_UNDEF)) {
		ASYNC_FORWARD_ERROR(&socket->write_error);
		return;
	}

	memset(&write, 0, sizeof(async_stream_write_req));

	write.in.str = data;
	write.in.len = ZSTR_LEN(data);
	write.in.buffer = ZSTR_VAL(data);
	write.in.context = context;
	write.in.flags = ASYNC_STREAM_WRITE_REQ_FLAG_ASYNC;

	if (return_value) {
		awaitable = async_create_awaitable(EX(prev_execute_data)->prev_execute_data, socket);

		write.in.callback = finish_async_write;
		write.in.arg = awaitable;

		ASYNC_ADDREF(&awaitable->std);
	} else {
		awaitable = NULL;
	}

	if (UNEXPECTED(FAILURE == async_stream_write(socket->stream, &write))) {
		if (awaitable) {
			ASYNC_DELREF(&awaitable->std);
			ASYNC_DELREF(&awaitable->std);
		}

		forward_stream_write_error(socket->stream, &write);
	} else if (return_value) {
		ASYNC_ADDREF(&socket->std);

		RETVAL_OBJ(&awaitable->std);
	}
}

void async_tcp_ce_register()
{
	zend_class_entry ce;
	zend_function *func;
	
	str_wildcard = zend_new_interned_string(zend_string_init(ZEND_STRL("0.0.0.0"), 1));

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Network", "TcpSocket", async_tcp_socket_functions);
	async_tcp_socket_ce = zend_register_internal_class(&ce);
	async_tcp_socket_ce->ce_flags |= ZEND_ACC_FINAL;
#if PHP_VERSION_ID >= 80100
	async_tcp_socket_ce->ce_flags |= ZEND_ACC_NOT_SERIALIZABLE;
#else
	async_tcp_socket_ce->serialize = zend_class_serialize_deny;
	async_tcp_socket_ce->unserialize = zend_class_unserialize_deny;
#endif
	zend_class_implements(async_tcp_socket_ce, 1, async_socket_stream_ce);

	memcpy(&async_tcp_socket_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_tcp_socket_handlers.dtor_obj = async_tcp_socket_object_dtor;
	async_tcp_socket_handlers.free_obj = async_tcp_socket_object_destroy;
	async_tcp_socket_handlers.clone_obj = NULL;

	ASYNC_TCP_SOCKET_CONST("NODELAY", ASYNC_SOCKET_TCP_NODELAY);
	ASYNC_TCP_SOCKET_CONST("KEEPALIVE", ASYNC_SOCKET_TCP_KEEPALIVE);

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Network", "TcpServer", async_tcp_server_functions);
	async_tcp_server_ce = zend_register_internal_class(&ce);
	async_tcp_server_ce->ce_flags |= ZEND_ACC_FINAL;
#if PHP_VERSION_ID >= 80100
	async_tcp_server_ce->ce_flags |= ZEND_ACC_NOT_SERIALIZABLE;
#else
	async_tcp_server_ce->serialize = zend_class_serialize_deny;
	async_tcp_server_ce->unserialize = zend_class_unserialize_deny;
#endif
	zend_class_implements(async_tcp_server_ce, 1, async_server_ce);

	memcpy(&async_tcp_server_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_tcp_server_handlers.dtor_obj = async_tcp_server_object_dtor;
	async_tcp_server_handlers.free_obj = async_tcp_server_object_destroy;
	async_tcp_server_handlers.clone_obj = NULL;

	ASYNC_TCP_SERVER_CONST("SIMULTANEOUS_ACCEPTS", ASYNC_SOCKET_TCP_SIMULTANEOUS_ACCEPTS);

	if (NULL != (func = (zend_function *) zend_hash_str_find_ptr(&async_tcp_socket_ce->function_table, ZEND_STRL("write")))) {
		async_register_interceptor(func, intercept_write);
	}
}

void async_tcp_ce_unregister()
{
	zend_string_release(str_wildcard);
}
