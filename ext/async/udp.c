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
#include "async/async_helper.h"
#include "async/async_stream.h"
#include "async/async_socket.h"
#include "kernel/backend.h"

#define ASYNC_SOCKET_UDP_TTL 200
#define ASYNC_SOCKET_UDP_MULTICAST_LOOP 250
#define ASYNC_SOCKET_UDP_MULTICAST_TTL 251

ASYNC_API zend_class_entry *async_udp_socket_ce;
ASYNC_API zend_class_entry *async_udp_datagram_ce;

static zend_object_handlers async_udp_socket_handlers;
static zend_object_handlers async_udp_datagram_handlers;

static zend_string *str_wildcard;

static zend_string *str_data;
static zend_string *str_address;
static zend_string *str_port;

static zend_object *async_udp_datagram_object_create(zend_class_entry *ce);

#define ASYNC_UDP_SOCKET_CONST(name, value) \
	zend_declare_class_constant_long(async_udp_socket_ce, name, sizeof(name)-1, (zend_long)value);

#define ASYNC_UDP_FLAG_RECEIVING 1
#define ASYNC_UDP_FLAG_CONNECTED (1 << 1)

typedef struct _async_udp_datagram {
	php_sockaddr_storage peer;

	zend_object std;
} async_udp_datagram;

static zend_always_inline async_udp_datagram *async_udp_datagram_obj(zend_object *object)
{
	return (async_udp_datagram *)((char *) object - XtOffsetOf(async_udp_datagram, std));
}

static zend_always_inline uint32_t async_udp_datagram_prop_offset(zend_string *name)
{
	return zend_get_property_info(async_udp_datagram_ce, name, 1)->offset;
}

typedef struct _async_udp_socket {
	zend_object std;
	
	uv_udp_t handle;
	async_task_scheduler *scheduler;
	
	uint8_t flags;
	
	zend_string *name;
	zend_string *ip;
	uint16_t port;
	
	zval error;
	zend_uchar ref_count;
	
	async_op_list receivers;
	async_op_list senders;
	async_op_list flush;
	async_cancel_cb cancel;
} async_udp_socket;

typedef struct _async_udp_recv_op {
	async_op base;
	int code;
	uv_buf_size_t size;
} async_udp_recv_op;

typedef struct _async_udp_send_op {
	async_op base;
	async_udp_socket *socket;
	async_udp_datagram *datagram;
	async_context *context;
	int code;
	uv_udp_send_t req;
	async_awaitable_impl *awaitable;
} async_udp_send_op;


ASYNC_CALLBACK socket_closed(uv_handle_t *handle)
{
	async_udp_socket *socket;
	
	socket = (async_udp_socket *) handle->data;
	
	ZEND_ASSERT(socket != NULL);

	ASYNC_DELREF(&socket->std);
}

ASYNC_CALLBACK socket_shutdown(void *obj, zval *error)
{
	async_udp_socket *socket;

	socket = (async_udp_socket *) obj;
	
	ZEND_ASSERT(socket != NULL);
	
	socket->cancel.func = NULL;
	
	if (error != NULL && Z_TYPE_P(&socket->error) == IS_UNDEF) {
		ZVAL_COPY(&socket->error, error);
	}

	ASYNC_UV_TRY_CLOSE_REF(&socket->std, &socket->handle, socket_closed);

	if (error != NULL) {
		while (socket->receivers.first != NULL) {
			ASYNC_FAIL_OP(socket->receivers.first, &socket->error);
		}
		
		while (socket->senders.first != NULL) {
			ASYNC_FAIL_OP(socket->senders.first, &socket->error);
		}
		
		while (socket->flush.first != NULL) {
			ASYNC_FAIL_OP(socket->flush.first, &socket->error);
		}
	}
}


static async_udp_socket *async_udp_socket_object_create()
{
	async_udp_socket *socket;

	socket = ecalloc(1, sizeof(async_udp_socket));

	zend_object_std_init(&socket->std, async_udp_socket_ce);
	socket->std.handlers = &async_udp_socket_handlers;
	
	socket->scheduler = async_task_scheduler_ref();
	
	uv_udp_init(&socket->scheduler->loop, &socket->handle);
	socket->handle.data = socket;
	
	uv_unref((uv_handle_t *) &socket->handle);
	
	ZVAL_UNDEF(&socket->error);
	
	socket->cancel.object = socket;
	socket->cancel.func = socket_shutdown;
	
	ASYNC_LIST_APPEND(&socket->scheduler->shutdown, &socket->cancel);

	return socket;
}

static void async_udp_socket_object_dtor(zend_object *object)
{
	async_udp_socket *socket;

	socket = (async_udp_socket *) object;

	if (socket->cancel.func != NULL) {
		ASYNC_LIST_REMOVE(&socket->scheduler->shutdown, &socket->cancel);
		
		socket->cancel.func(socket, NULL);
	}
}

static void async_udp_socket_object_destroy(zend_object *object)
{
	async_udp_socket *socket;

	socket = (async_udp_socket *) object;
	
	zval_ptr_dtor(&socket->error);

	if (socket->name != NULL) {
		zend_string_release(socket->name);
	}
	
	if (socket->ip != NULL) {
		zend_string_release(socket->ip);
	}
	
	async_task_scheduler_unref(socket->scheduler);
	
	zend_object_std_dtor(&socket->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_udp_socket_bind, 0, 0, Phalcon\\Async\\Network\\UdpSocket, 0)
	ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 1)
ZEND_END_ARG_INFO();

static PHP_METHOD(UdpSocket, bind)
{
	async_udp_socket *socket;

	zval *name;
	zend_long port;

	zend_string *host;
	uv_os_fd_t sock;
	
	php_sockaddr_storage dest;
	int code;
	
	name = NULL;
	port = 0;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 2)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(name)
		Z_PARAM_LONG(port)
	ZEND_PARSE_PARAMETERS_END();
	
	if (name == NULL || Z_TYPE_P(name) == IS_NULL) {
		host = str_wildcard;
	} else {
		host = Z_STR_P(name);
	}

	code = async_dns_lookup_ip(ZSTR_VAL(host), &dest, IPPROTO_UDP);
	
	ASYNC_CHECK_EXCEPTION(code < 0, async_socket_exception_ce, "Failed to assemble IP address: %s", uv_strerror(code));
	
	async_socket_set_port((struct sockaddr *) &dest, port);
	
	socket = async_udp_socket_object_create();
	socket->name = zend_string_copy(host);
	
	code = uv_udp_bind(&socket->handle, (const struct sockaddr *) &dest, UV_UDP_REUSEADDR);
	
	if (UNEXPECTED(code != 0)) {
		zend_throw_exception_ex(async_socket_bind_exception_ce, 0, "Failed to bind UDP socket: %s", uv_strerror(code));
		ASYNC_DELREF(&socket->std);
		return;
	}
	
	if (EXPECTED(0 == uv_fileno((const uv_handle_t *) &socket->handle, &sock))) {
		async_socket_get_local_peer((php_socket_t) sock, &socket->ip, &socket->port);
	}
	
	if (UNEXPECTED(EG(exception))) {
		ASYNC_DELREF(&socket->std);
		return;
	}
	
	RETURN_OBJ(&socket->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_udp_socket_connect, 0, 2, Phalcon\\Async\\Network\\UdpSocket, 0)
	ZEND_ARG_TYPE_INFO(0, remote_host, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, remote_port, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 1)
ZEND_END_ARG_INFO();

static PHP_METHOD(UdpSocket, connect)
{
	async_udp_socket *socket;

	zend_string *remote_host;
	zend_long remote_port;
	zval *name;
	zend_long port;

	zend_string *host;
	uv_os_fd_t sock;

	php_sockaddr_storage addr;
	php_sockaddr_storage dest;

	int code;

	name = NULL;
	port = 0;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 4)
		Z_PARAM_STR(remote_host)
		Z_PARAM_LONG(remote_port)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(name)
		Z_PARAM_LONG(port)
	ZEND_PARSE_PARAMETERS_END();

	if (name == NULL || Z_TYPE_P(name) == IS_NULL) {
		host = str_wildcard;
	} else {
		host = Z_STR_P(name);
	}

	ASYNC_CHECK_EXCEPTION(async_task_scheduler_get()->flags & ASYNC_TASK_SCHEDULER_FLAG_DISPOSED, async_socket_exception_ce, "Task scheduler has been disposed");
	ASYNC_CHECK_EXCEPTION(async_task_scheduler_get()->flags & ASYNC_TASK_SCHEDULER_FLAG_ERROR, async_socket_exception_ce, "Task scheduler was stopped due to an error");

	code = async_dns_lookup_ip(ZSTR_VAL(host), &addr, IPPROTO_UDP);

	ASYNC_CHECK_EXCEPTION(code < 0, async_socket_exception_ce, "Failed to assemble IP address: %s", uv_strerror(code));

	async_socket_set_port((struct sockaddr *) &addr, port);

	code = async_dns_lookup_ip(ZSTR_VAL(remote_host), &dest, IPPROTO_UDP);

	ASYNC_CHECK_EXCEPTION(code < 0, async_socket_exception_ce, "Failed to assemble IP address: %s", uv_strerror(code));

	async_socket_set_port((struct sockaddr *) &dest, remote_port);

	socket = async_udp_socket_object_create();
	socket->name = zend_string_copy(host);

	code = uv_udp_bind(&socket->handle, (const struct sockaddr *) &addr, UV_UDP_REUSEADDR);

	if (UNEXPECTED(code != 0)) {
		zend_throw_exception_ex(async_socket_bind_exception_ce, 0, "Failed to bind UDP socket: %s", uv_strerror(code));
		ASYNC_DELREF(&socket->std);
		return;
	}

	if (EXPECTED(0 == uv_fileno((const uv_handle_t *) &socket->handle, &sock))) {
		async_socket_get_local_peer((php_socket_t) sock, &socket->ip, &socket->port);
	}

	if (UNEXPECTED(EG(exception))) {
		ASYNC_DELREF(&socket->std);
		return;
	}

	code = uv_udp_connect(&socket->handle, (const struct sockaddr *) &dest);

	if (UNEXPECTED(code != 0)) {
		zend_throw_exception_ex(async_socket_connect_exception_ce, 0, "Failed to connect UDP socket: %s", uv_strerror(code));
		ASYNC_DELREF(&socket->std);
		return;
	}

	socket->flags |= ASYNC_UDP_FLAG_CONNECTED;

	RETURN_OBJ(&socket->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_udp_socket_multicast, 0, 2, Phalcon\\Async\\Network\\UdpSocket, 0)
	ZEND_ARG_TYPE_INFO(0, group, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(UdpSocket, multicast)
{
	async_udp_socket *socket;

	zend_string *group;
	zend_long port;
	
	uv_os_fd_t sock;
	
	struct sockaddr_in dest;
	int code;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_STR(group)
		Z_PARAM_LONG(port)
	ZEND_PARSE_PARAMETERS_END();
	
	socket = async_udp_socket_object_create();
	
	ASYNC_STR(socket->name, "localhost");
	
	code = uv_ip4_addr(ZSTR_VAL(str_wildcard), (int) port, &dest);

	if (UNEXPECTED(code != 0)) {
		zend_throw_exception_ex(async_socket_exception_ce, 0, "Failed to assemble IP address: %s", uv_strerror(code));
		ASYNC_DELREF(&socket->std);
		return;
	}
	
	code = uv_udp_bind(&socket->handle, (const struct sockaddr *) &dest, UV_UDP_REUSEADDR);
	
	if (UNEXPECTED(code != 0)) {
		zend_throw_exception_ex(async_socket_bind_exception_ce, 0, "Failed to bind UDP socket: %s", uv_strerror(code));
		ASYNC_DELREF(&socket->std);
		return;
	}
	
	code = uv_udp_set_membership(&socket->handle, ZSTR_VAL(group), NULL, UV_JOIN_GROUP);
	
	if (UNEXPECTED(code != 0)) {
		zend_throw_exception_ex(async_socket_exception_ce, 0, "Failed to join UDP multicast group: %s", uv_strerror(code));
		ASYNC_DELREF(&socket->std);
		return;
	}
	
	if (EXPECTED(0 == uv_fileno((const uv_handle_t *) &socket->handle, &sock))) {
		async_socket_get_local_peer((php_socket_t) sock, &socket->ip, &socket->port);
	}
	
	if (UNEXPECTED(EG(exception))) {
		ASYNC_DELREF(&socket->std);
		return;
	}
	
	RETURN_OBJ(&socket->std);
}

static PHP_METHOD(UdpSocket, close)
{
	async_udp_socket *socket;

	zval error;
	zval *val;

	val = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_OBJECT_OF_CLASS_EX(val, zend_ce_throwable, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	socket = (async_udp_socket *) Z_OBJ_P(getThis());
	
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

static PHP_METHOD(UdpSocket, flush)
{
	async_udp_socket *socket;
	async_op *op;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	socket = (async_udp_socket *) Z_OBJ_P(getThis());
	
	if (UNEXPECTED(Z_TYPE_P(&socket->error) != IS_UNDEF)) {
		ASYNC_FORWARD_ERROR(&socket->error);
		return;
	}
	
	if (socket->senders.first == NULL) {
		return;
	}
	
	ASYNC_ALLOC_OP(op);
	ASYNC_APPEND_OP(&socket->flush, op);
	
	if (UNEXPECTED(async_await_op(op) == FAILURE)) {
		ASYNC_FORWARD_OP_ERROR(op);
	}
	
	ASYNC_FREE_OP(op);
}

static PHP_METHOD(UdpSocket, getAddress)
{
	async_udp_socket *socket;

	ZEND_PARSE_PARAMETERS_NONE();

	socket = (async_udp_socket *) Z_OBJ_P(getThis());

	RETURN_STR_COPY(socket->ip);
}

static PHP_METHOD(UdpSocket, getPort)
{
	async_udp_socket *socket;

	ZEND_PARSE_PARAMETERS_NONE();

	socket = (async_udp_socket *) Z_OBJ_P(getThis());
	
	RETURN_LONG(socket->port);
}

static PHP_METHOD(UdpSocket, setOption)
{
	async_udp_socket *socket;

	zend_long option;
	zval *val;

	int code;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_LONG(option)
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();

	socket = (async_udp_socket *) Z_OBJ_P(getThis());
	code = 0;

	switch ((int) option) {
	case ASYNC_SOCKET_UDP_TTL:
		code = uv_udp_set_ttl(&socket->handle, (int) Z_LVAL_P(val));
		break;
	case ASYNC_SOCKET_UDP_MULTICAST_LOOP:
		code = uv_udp_set_multicast_loop(&socket->handle, Z_LVAL_P(val) ? 1 : 0);
		break;
	case ASYNC_SOCKET_UDP_MULTICAST_TTL:
		code = uv_udp_set_multicast_ttl(&socket->handle, (int) Z_LVAL_P(val));
		break;
	}

	RETURN_BOOL((code < 0) ? 0 : 1);
}

ASYNC_CALLBACK socket_received(uv_udp_t *udp, ssize_t nread, const uv_buf_t *buffer, const struct sockaddr *addr, unsigned int flags)
{
	async_udp_socket *socket;
	async_udp_datagram *datagram;
	
	async_udp_recv_op *op;
	
	zend_string *ip;
	uint16_t port;

	socket = (async_udp_socket *) udp->data;
	
	ZEND_ASSERT(socket != NULL);
	ZEND_ASSERT(socket->receivers.first != NULL);
	
	if (UNEXPECTED(nread == 0)) {
		efree(buffer->base);
		
		return;
	}
	
	ASYNC_NEXT_CUSTOM_OP(&socket->receivers, op, async_udp_recv_op);
	
	if (EXPECTED(nread > 0)) {
		datagram = async_udp_datagram_obj(async_udp_datagram_object_create(async_udp_datagram_ce));
		
		ZVAL_STRINGL(OBJ_PROP(&datagram->std, async_udp_datagram_prop_offset(str_data)), buffer->base, (size_t) nread);
		efree(buffer->base);

		if (EXPECTED(addr)) {
			memcpy(&datagram->peer, addr, async_socket_addr_size(addr));
			async_socket_get_peer(addr, &ip, &port);

			ZVAL_STR(OBJ_PROP(&datagram->std, async_udp_datagram_prop_offset(str_address)), ip);
			ZVAL_LONG(OBJ_PROP(&datagram->std, async_udp_datagram_prop_offset(str_port)), port);
		}
		
		ZVAL_OBJ(&op->base.result, &datagram->std);
	}
	
	op->code = (int) nread;
	
	ASYNC_FINISH_OP(op);
	
	if (socket->receivers.first == NULL) {
		uv_udp_recv_stop(udp);
		
		socket->flags &= ~ASYNC_UDP_FLAG_RECEIVING;
	}
}

ASYNC_CALLBACK socket_alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buffer)
{
	async_udp_socket *socket;

	socket = (async_udp_socket *) handle->data;

	ZEND_ASSERT(socket != NULL);
	ZEND_ASSERT(socket->receivers.first != NULL);

	buffer->len = ((async_udp_recv_op *) socket->receivers.first)->size;
	buffer->base = emalloc(buffer->len);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_udp_socket_receive, 0, 0, Phalcon\\Async\\Network\\UdpDatagram, 0)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 1)
ZEND_END_ARG_INFO();

static PHP_METHOD(UdpSocket, receive)
{
	async_udp_socket *socket;
	async_context *context;
	async_udp_recv_op *op;
	
	zend_long size;
	int code;
	
	size = 0;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(size)
	ZEND_PARSE_PARAMETERS_END();
	
	ASYNC_CHECK_ERROR(size < 0, "UDP receive buffer size must not be negative");

	if (size == 0) {
		size = 8192;
	}

	socket = (async_udp_socket *) Z_OBJ_P(getThis());
	
	if (UNEXPECTED(Z_TYPE_P(&socket->error) != IS_UNDEF)) {
		ASYNC_FORWARD_ERROR(&socket->error);
		return;
	}

	if (!(socket->flags & ASYNC_UDP_FLAG_RECEIVING)) {
		code = uv_udp_recv_start(&socket->handle, socket_alloc_buffer, socket_received);
		
		ASYNC_CHECK_EXCEPTION(code != 0, async_socket_exception_ce, "Failed to receive UDP data: %s", uv_strerror(code));
		
		socket->flags |= ASYNC_UDP_FLAG_RECEIVING;
	}
	
	context = async_context_get();
	
	ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_udp_recv_op));
	ASYNC_APPEND_OP(&socket->receivers, op);
	
	op->size = (uv_buf_size_t) size;

	ASYNC_UNREF_ENTER(context, socket);
	
	if (UNEXPECTED(async_await_op((async_op *) op) == FAILURE)) {
		ASYNC_FORWARD_OP_ERROR(op);
	}
	
	ASYNC_UNREF_EXIT(context, socket);
	
	if (EXPECTED(!EG(exception))) {
		if (op->code < 0) {
			zend_throw_exception_ex(async_stream_exception_ce, 0, "UDP receive error: %s", uv_strerror(op->code));
		} else if (USED_RET()) {
			ZVAL_COPY(return_value, &op->base.result);
		}
	}
	
	ASYNC_FREE_OP(op);
}

ASYNC_CALLBACK socket_sent(uv_udp_send_t *req, int status)
{
	async_udp_send_op *op;
	
	op = (async_udp_send_op *) req->data;
	
	ZEND_ASSERT(op != NULL);
	
	ASYNC_DELREF(&op->datagram->std);
	ASYNC_DELREF(&op->socket->std);
	
	if (UNEXPECTED(op->base.flags & ASYNC_OP_FLAG_CANCELLED)) {
		ASYNC_FREE_OP(op);
	} else {
		op->code = status;
		
		ASYNC_FINISH_OP(op);
	}	
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_udp_socket_send, 0, 1, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, datagram, Phalcon\\Async\\Network\\UdpDatagram, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(UdpSocket, send)
{
	async_udp_socket *socket;
	async_udp_datagram *datagram;
	async_udp_send_op *op;
	
	php_sockaddr_storage *dest;
	uv_buf_t buffers[1];
	
	zend_string *data;
	zval *val;

	int code;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_OBJECT_OF_CLASS(val, async_udp_datagram_ce)
	ZEND_PARSE_PARAMETERS_END();
	
	socket = (async_udp_socket *) Z_OBJ_P(getThis());
	datagram = async_udp_datagram_obj(Z_OBJ_P(val));
	
	if (UNEXPECTED(Z_TYPE_P(&socket->error) != IS_UNDEF)) {
		ASYNC_FORWARD_ERROR(&socket->error);
		return;
	}
	
	val = OBJ_PROP(&datagram->std, async_udp_datagram_prop_offset(str_address));

	if (socket->flags & ASYNC_UDP_FLAG_CONNECTED) {
		ASYNC_CHECK_EXCEPTION(Z_TYPE_P(val) != IS_NULL, async_socket_exception_ce, "Connected UDP socket cannot send to a different address");

		dest = NULL;
	} else {
		ASYNC_CHECK_EXCEPTION(Z_TYPE_P(val) == IS_NULL, async_socket_exception_ce, "Unconnected UDP socket requires a target address");

		dest = &datagram->peer;
	}

	data = Z_STR_P(OBJ_PROP(&datagram->std, async_udp_datagram_prop_offset(str_data)));
	buffers[0] = uv_buf_init(ZSTR_VAL(data), (unsigned int) ZSTR_LEN(data));
	
	if (socket->senders.first == NULL) {
	    code = uv_udp_try_send(&socket->handle, buffers, 1, (const struct sockaddr *) dest);
	    
	    if (EXPECTED(code >= 0)) {
	        return;
	    }
	    
	    ASYNC_CHECK_EXCEPTION(code != UV_EAGAIN, async_socket_exception_ce, "Failed to send UDP data: %s", uv_strerror(code));
	}
	
	ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_udp_send_op));
	
	op->req.data = op;
	op->socket = socket;
	op->datagram = datagram;
	op->context = async_context_get();
	
	code = uv_udp_send(&op->req, &socket->handle, buffers, 1, (const struct sockaddr *) dest, socket_sent);
	
	if (UNEXPECTED(code < 0)) {	
		ASYNC_FREE_OP(op);
		zend_throw_exception_ex(async_socket_exception_ce, 0, "Failed to send UDP data: %s", uv_strerror(code));
		
		return;
	}
	
	ASYNC_ADDREF(&socket->std);
	ASYNC_ADDREF(&datagram->std);
	
	ASYNC_APPEND_OP(&socket->senders, op);
	ASYNC_UNREF_ENTER(op->context, socket);
	
	if (UNEXPECTED(async_await_op((async_op *) op) == FAILURE)) {
		ASYNC_FORWARD_OP_ERROR(op);
	}
	
	ASYNC_UNREF_EXIT(op->context, socket);
	
	if (EXPECTED(!(op->base.flags & ASYNC_OP_FLAG_CANCELLED))) {
		ASYNC_FREE_OP(op);
	}
	
	if (socket->senders.first == NULL) {
		while (socket->flush.first != NULL) {
			ASYNC_FINISH_OP(socket->flush.first);
		}
	}
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_CTOR(UdpSocket, async_udp_socket_ce)
ASYNC_METHOD_NO_WAKEUP(UdpSocket, async_udp_socket_ce)
//LCOV_EXCL_STOP

static const zend_function_entry async_udp_socket_functions[] = {
	PHP_ME(UdpSocket, __construct, arginfo_no_ctor, ZEND_ACC_PRIVATE)
	PHP_ME(UdpSocket, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(UdpSocket, bind, arginfo_udp_socket_bind, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(UdpSocket, connect, arginfo_udp_socket_connect, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(UdpSocket, multicast, arginfo_udp_socket_multicast, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(UdpSocket, close, arginfo_stream_close, ZEND_ACC_PUBLIC)
	PHP_ME(UdpSocket, flush, arginfo_socket_stream_flush, ZEND_ACC_PUBLIC)
	PHP_ME(UdpSocket, getAddress, arginfo_socket_get_address, ZEND_ACC_PUBLIC)
	PHP_ME(UdpSocket, getPort, arginfo_socket_get_port, ZEND_ACC_PUBLIC)
	PHP_ME(UdpSocket, setOption, arginfo_socket_set_option, ZEND_ACC_PUBLIC)
	PHP_ME(UdpSocket, receive, arginfo_udp_socket_receive, ZEND_ACC_PUBLIC)
	PHP_ME(UdpSocket, send, arginfo_udp_socket_send, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


static zend_object *async_udp_datagram_object_create(zend_class_entry *ce)
{
	async_udp_datagram *datagram;
	
	datagram = ecalloc(1, sizeof(async_udp_datagram) + zend_object_properties_size(ce));
	
	zend_object_std_init(&datagram->std, ce);
	datagram->std.handlers = &async_udp_datagram_handlers;
	
	object_properties_init(&datagram->std, ce);

	return &datagram->std;
}

static void async_udp_datagram_object_destroy(zend_object *object)
{
	async_udp_datagram *datagram;

	datagram = async_udp_datagram_obj(object);
	
	zend_object_std_dtor(&datagram->std);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_udp_datagram_ctor, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, address, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 1)
ZEND_END_ARG_INFO();

static PHP_METHOD(UdpDatagram, __construct)
{
	async_udp_datagram *datagram;
	
	zend_string *data;
	zval *host;
	zend_long port;
	
	zend_string *ip;
	uint16_t p;

	host = NULL;
	port = 0;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 3)
		Z_PARAM_STR(data)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(host)
		Z_PARAM_LONG(port)
	ZEND_PARSE_PARAMETERS_END();
	
	datagram = async_udp_datagram_obj(Z_OBJ_P(getThis()));
	
	if (host == NULL || Z_TYPE_P(host) == IS_NULL) {
		ZVAL_NULL(OBJ_PROP(&datagram->std, async_udp_datagram_prop_offset(str_address)));
		ZVAL_NULL(OBJ_PROP(&datagram->std, async_udp_datagram_prop_offset(str_port)));
	} else {
		if (UNEXPECTED(0 != async_dns_lookup_ip(Z_STRVAL_P(host), &datagram->peer, IPPROTO_UDP))) {
			zend_throw_error(NULL, "Failed to assemble peer IP address");
			return;
		}

		async_socket_set_port((struct sockaddr *) &datagram->peer, port);
		async_socket_get_peer((const struct sockaddr *) &datagram->peer, &ip, &p);

		ZVAL_STR(OBJ_PROP(&datagram->std, async_udp_datagram_prop_offset(str_address)), ip);
		ZVAL_LONG(OBJ_PROP(&datagram->std, async_udp_datagram_prop_offset(str_port)), p);
	}
	
	ZVAL_STR_COPY(OBJ_PROP(&datagram->std, async_udp_datagram_prop_offset(str_data)), data);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_udp_datagram_with_data, 0, 1, Phalcon\\Async\\Network\\UdpDatagram, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(UdpDatagram, withData)
{
	async_udp_datagram *datagram;
	async_udp_datagram *result;
	
	zend_string *data;
	zval *tmp;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STR(data)
	ZEND_PARSE_PARAMETERS_END();
	
	datagram = async_udp_datagram_obj(Z_OBJ_P(getThis()));
	result = async_udp_datagram_obj(async_udp_datagram_object_create(async_udp_datagram_ce));
	
	result->peer = datagram->peer;
	
	ZVAL_STR_COPY(OBJ_PROP(&result->std, async_udp_datagram_prop_offset(str_data)), data);

	tmp = OBJ_PROP(&datagram->std, async_udp_datagram_prop_offset(str_address));
	ZVAL_COPY(OBJ_PROP(&result->std, async_udp_datagram_prop_offset(str_address)), tmp);

	tmp = OBJ_PROP(&datagram->std, async_udp_datagram_prop_offset(str_port));
	ZVAL_COPY(OBJ_PROP(&result->std, async_udp_datagram_prop_offset(str_port)), tmp);

	RETURN_OBJ(&result->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_udp_datagram_with_peer, 0, 2, Phalcon\\Async\\Network\\UdpDatagram, 0)
	ZEND_ARG_TYPE_INFO(0, address, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(UdpDatagram, withPeer)
{
	async_udp_datagram *datagram;
	async_udp_datagram *result;
	
	zend_string *address;
	zend_long port;
	
	zend_string *ip;
	zval *tmp;
	uint16_t p;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_STR(address)
		Z_PARAM_LONG(port)
	ZEND_PARSE_PARAMETERS_END();
	
	datagram = async_udp_datagram_obj(Z_OBJ_P(getThis()));
	result = async_udp_datagram_obj(async_udp_datagram_object_create(async_udp_datagram_ce));
	
	if (UNEXPECTED(0 != async_dns_lookup_ip(ZSTR_VAL(address), &result->peer, IPPROTO_UDP))) {
		ASYNC_DELREF(&result->std);		
		zend_throw_error(NULL, "Failed to assemble peer IP address");
				
		return;
	}
	
	async_socket_set_port((struct sockaddr *) &result->peer, port);
	async_socket_get_peer((const struct sockaddr *) &result->peer, &ip, &p);

	tmp = OBJ_PROP(&datagram->std, async_udp_datagram_prop_offset(str_data));
	ZVAL_COPY(OBJ_PROP(&result->std, async_udp_datagram_prop_offset(str_data)), tmp);

	ZVAL_STR(OBJ_PROP(&result->std, async_udp_datagram_prop_offset(str_address)), ip);
	ZVAL_LONG(OBJ_PROP(&result->std, async_udp_datagram_prop_offset(str_port)), p);
	
	RETURN_OBJ(&result->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_udp_datagram_without_peer, 0, 0, Phalcon\\Async\\Network\\UdpDatagram, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(UdpDatagram, withoutPeer)
{
	async_udp_datagram *datagram;
	async_udp_datagram *result;

	zval *tmp;

	ZEND_PARSE_PARAMETERS_NONE();

	datagram = async_udp_datagram_obj(Z_OBJ_P(getThis()));
	result = async_udp_datagram_obj(async_udp_datagram_object_create(async_udp_datagram_ce));

	tmp = OBJ_PROP(&datagram->std, async_udp_datagram_prop_offset(str_data));
	ZVAL_COPY(OBJ_PROP(&result->std, async_udp_datagram_prop_offset(str_data)), tmp);

	ZVAL_NULL(OBJ_PROP(&result->std, async_udp_datagram_prop_offset(str_address)));
	ZVAL_NULL(OBJ_PROP(&result->std, async_udp_datagram_prop_offset(str_port)));

	RETURN_OBJ(&result->std);
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_WAKEUP(UdpDatagram, async_udp_datagram_ce)
//LCOV_EXCL_STOP

static const zend_function_entry async_udp_datagram_functions[] = {
	PHP_ME(UdpDatagram, __construct, arginfo_udp_datagram_ctor, ZEND_ACC_PUBLIC)
	PHP_ME(UdpDatagram, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(UdpDatagram, withData, arginfo_udp_datagram_with_data, ZEND_ACC_PUBLIC)
	PHP_ME(UdpDatagram, withPeer, arginfo_udp_datagram_with_peer, ZEND_ACC_PUBLIC)
	PHP_ME(UdpDatagram, withoutPeer, arginfo_udp_datagram_without_peer, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


ASYNC_CALLBACK socket_sent_async(uv_udp_send_t *req, int status)
{
	async_udp_send_op *op;

	zval error;

	op = (async_udp_send_op *) req->data;

	ZEND_ASSERT(op != NULL);

	op->code = status;

	ASYNC_UNREF_EXIT(op->context, op->socket);

	if (op->base.list) {
		ASYNC_LIST_REMOVE(op->base.list, &op->base);
		op->base.list = NULL;
	}

	if (op->socket->senders.first == NULL) {
		while (op->socket->flush.first != NULL) {
			ASYNC_FINISH_OP(op->socket->flush.first);
		}
	}

	if (op->awaitable) {
		if (UNEXPECTED(status < 0)) {
			ASYNC_PREPARE_SCHEDULER_EXCEPTION(&error, async_socket_exception_ce, "Failed to send UDP data: %s", uv_strerror(status));
			async_awaitable_fail(op->awaitable, &error);
			zval_ptr_dtor(&error);
		} else {
			async_awaitable_resolve(op->awaitable, NULL);
		}

		ASYNC_DELREF(&op->awaitable->std);
	}

	ASYNC_DELREF(&op->context->std);
	ASYNC_DELREF(&op->datagram->std);
	ASYNC_DELREF(&op->socket->std);

	ASYNC_FREE_OP(op);
}

static void intercept_send(async_context *context, zend_object *obj, INTERNAL_FUNCTION_PARAMETERS)
{
	async_udp_socket *socket;
	async_udp_datagram *datagram;
	async_udp_send_op *op;

	php_sockaddr_storage *dest;
	uv_buf_t buffers[1];

	zend_string *data;
	zval *val;

	int code;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_OBJECT_OF_CLASS(val, async_udp_datagram_ce)
	ZEND_PARSE_PARAMETERS_END();

	socket = (async_udp_socket *) obj;
	datagram = async_udp_datagram_obj(Z_OBJ_P(val));

	if (UNEXPECTED(Z_TYPE_P(&socket->error) != IS_UNDEF)) {
		ASYNC_FORWARD_ERROR(&socket->error);
		return;
	}

	val = OBJ_PROP(&datagram->std, async_udp_datagram_prop_offset(str_address));

	if (socket->flags & ASYNC_UDP_FLAG_CONNECTED) {
		ASYNC_CHECK_EXCEPTION(Z_TYPE_P(val) != IS_NULL, async_socket_exception_ce, "Connected UDP socket cannot send to a different address");

		dest = NULL;
	} else {
		ASYNC_CHECK_EXCEPTION(Z_TYPE_P(val) == IS_NULL, async_socket_exception_ce, "Unconnected UDP socket requires a target address");

		dest = &datagram->peer;
	}

	data = Z_STR_P(OBJ_PROP(&datagram->std, async_udp_datagram_prop_offset(str_data)));
	buffers[0] = uv_buf_init(ZSTR_VAL(data), (unsigned int) ZSTR_LEN(data));

	if (socket->senders.first == NULL) {
	    code = uv_udp_try_send(&socket->handle, buffers, 1, (const struct sockaddr *) dest);

	    if (EXPECTED(code >= 0)) {
	    	if (return_value) {
	    		RETVAL_OBJ(&async_create_resolved_awaitable(EX(prev_execute_data), NULL)->std);
	    	}

	    	return;
	    }

	    ASYNC_CHECK_EXCEPTION(code != UV_EAGAIN, async_socket_exception_ce, "Failed to send UDP data: %s", uv_strerror(code));
	}

	ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_udp_send_op));

	op->req.data = op;
	op->socket = socket;
	op->datagram = datagram;
	op->context = context;

	if (return_value) {
		op->awaitable = async_create_awaitable(EX(prev_execute_data)->prev_execute_data, NULL);

		ASYNC_ADDREF(&op->awaitable->std);
	}

	code = uv_udp_send(&op->req, &socket->handle, buffers, 1, (const struct sockaddr *) dest, socket_sent_async);

	if (UNEXPECTED(code != 0)) {
		ASYNC_DELREF(&op->awaitable->std);
		ASYNC_DELREF(&op->awaitable->std);

		ASYNC_FREE_OP(op);
		zend_throw_exception_ex(async_socket_exception_ce, 0, "Failed to send UDP data: %s", uv_strerror(code));

		return;
	}

	ASYNC_ADDREF(&socket->std);
	ASYNC_ADDREF(&datagram->std);
	ASYNC_ADDREF(&op->context->std);

	ASYNC_APPEND_OP(&socket->senders, op);
	ASYNC_UNREF_ENTER(op->context, socket);

	if (return_value) {
		RETVAL_OBJ(&op->awaitable->std);
	}
}

void async_udp_socket_ce_register()
{
	zend_class_entry ce;
	zend_function *func;

#if PHP_VERSION_ID >= 70400
	zval tmp;
#endif

	str_wildcard = zend_new_interned_string(zend_string_init(ZEND_STRL("0.0.0.0"), 1));

	str_data = zend_new_interned_string(zend_string_init(ZEND_STRL("data"), 1));
	str_address = zend_new_interned_string(zend_string_init(ZEND_STRL("address"), 1));
	str_port = zend_new_interned_string(zend_string_init(ZEND_STRL("port"), 1));

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Network", "UdpSocket", async_udp_socket_functions);
	async_udp_socket_ce = zend_register_internal_class(&ce);
	async_udp_socket_ce->ce_flags |= ZEND_ACC_FINAL;
	async_udp_socket_ce->serialize = zend_class_serialize_deny;
	async_udp_socket_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_udp_socket_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_udp_socket_handlers.free_obj = async_udp_socket_object_destroy;
	async_udp_socket_handlers.dtor_obj = async_udp_socket_object_dtor;
	async_udp_socket_handlers.clone_obj = NULL;
	
	zend_class_implements(async_udp_socket_ce, 1, async_socket_ce);

	ASYNC_UDP_SOCKET_CONST("TTL", ASYNC_SOCKET_UDP_TTL);
	ASYNC_UDP_SOCKET_CONST("MULTICAST_LOOP", ASYNC_SOCKET_UDP_MULTICAST_LOOP);
	ASYNC_UDP_SOCKET_CONST("MULTICAST_TTL", ASYNC_SOCKET_UDP_MULTICAST_TTL);

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Network", "UdpDatagram", async_udp_datagram_functions);
	async_udp_datagram_ce = zend_register_internal_class(&ce);
	async_udp_datagram_ce->ce_flags |= ZEND_ACC_FINAL;
	async_udp_datagram_ce->create_object = async_udp_datagram_object_create;
	async_udp_datagram_ce->serialize = zend_class_serialize_deny;
	async_udp_datagram_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_udp_datagram_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_udp_datagram_handlers.offset = XtOffsetOf(async_udp_datagram, std);
	async_udp_datagram_handlers.free_obj = async_udp_datagram_object_destroy;
	async_udp_datagram_handlers.clone_obj = NULL;
	async_udp_datagram_handlers.write_property = async_prop_write_handler_readonly;

#if PHP_VERSION_ID < 70400
	zend_declare_property_null(async_udp_datagram_ce, ZEND_STRL("data"), ZEND_ACC_PUBLIC);
	zend_declare_property_null(async_udp_datagram_ce, ZEND_STRL("address"), ZEND_ACC_PUBLIC);
	zend_declare_property_null(async_udp_datagram_ce, ZEND_STRL("port"), ZEND_ACC_PUBLIC);
#else
	ZVAL_STRING(&tmp, "");
	zend_declare_typed_property(async_udp_datagram_ce, str_data, &tmp, ZEND_ACC_PUBLIC, NULL, IS_STRING);
	zend_declare_typed_property(async_udp_datagram_ce, str_address, &tmp, ZEND_ACC_PUBLIC, NULL, ZEND_TYPE_ENCODE(IS_STRING, 1));
	zval_ptr_dtor(&tmp);

	ZVAL_LONG(&tmp, 0);
	zend_declare_typed_property(async_udp_datagram_ce, str_port, &tmp, ZEND_ACC_PUBLIC, NULL, ZEND_TYPE_ENCODE(IS_LONG, 1));
#endif

	if (NULL != (func = (zend_function *) zend_hash_str_find_ptr(&async_udp_socket_ce->function_table, ZEND_STRL("send")))) {
		async_register_interceptor(func, intercept_send);
	}
}

void async_udp_socket_ce_unregister()
{
	zend_string_release(str_wildcard);

	zend_string_release(str_data);
	zend_string_release(str_address);
	zend_string_release(str_port);
}
