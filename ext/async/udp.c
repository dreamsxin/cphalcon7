
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

#include "kernel/backend.h"
#include "async/async_stream.h"
#include "async/async_socket.h"

#define ASYNC_SOCKET_UDP_TTL 200
#define ASYNC_SOCKET_UDP_MULTICAST_LOOP 250
#define ASYNC_SOCKET_UDP_MULTICAST_TTL 251

ASYNC_API zend_class_entry *async_udp_socket_ce;
ASYNC_API zend_class_entry *async_udp_datagram_ce;

static zend_object_handlers async_udp_socket_handlers;
static zend_object_handlers async_udp_datagram_handlers;

static zend_object *async_udp_datagram_object_create(zend_class_entry *ce);

#define ASYNC_UDP_SOCKET_CONST(name, value) \
	zend_declare_class_constant_long(async_udp_socket_ce, name, sizeof(name)-1, (zend_long)value);

#define ASYNC_UDP_FLAG_RECEIVING 1

typedef struct {
	zend_object std;
	
	zend_string *data;
	php_sockaddr_storage peer;
} async_udp_datagram;

typedef struct {
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

typedef struct {
	async_op base;
	async_udp_socket *socket;
	async_udp_datagram *datagram;
	async_context *context;
	int code;
	uv_udp_send_t req;
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
	
	if (!uv_is_closing((uv_handle_t *) &socket->handle)) {
		ASYNC_ADDREF(&socket->std);
		
		uv_close((uv_handle_t *) &socket->handle, socket_closed);
	}
	
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

static ZEND_METHOD(UdpSocket, bind)
{
	async_udp_socket *socket;

	zend_string *name;
	zend_long port;

	uv_os_fd_t sock;

	php_sockaddr_storage dest;
	int code;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_STR(name)
		Z_PARAM_LONG(port)
	ZEND_PARSE_PARAMETERS_END();
	
	code = async_dns_lookup_ip(ZSTR_VAL(name), &dest, IPPROTO_UDP);
	
	ASYNC_CHECK_EXCEPTION(code < 0, async_socket_exception_ce, "Failed to assemble IP address: %s", uv_strerror(code));
	
	async_socket_set_port((struct sockaddr *) &dest, port);
	
	socket = async_udp_socket_object_create();
	socket->name = zend_string_copy(name);
	
	code = uv_udp_bind(&socket->handle, (const struct sockaddr *) &dest, UV_UDP_REUSEADDR);
	
	if (UNEXPECTED(code != 0)) {
		zend_throw_exception_ex(async_socket_exception_ce, 0, "Failed to bind UDP socket: %s", uv_strerror(code));
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

static ZEND_METHOD(UdpSocket, multicast)
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
	
	code = uv_ip4_addr("0.0.0.0", (int) port, &dest);

	if (UNEXPECTED(code != 0)) {
		zend_throw_exception_ex(async_socket_exception_ce, 0, "Failed to assemble IP address: %s", uv_strerror(code));
		ASYNC_DELREF(&socket->std);
		return;
	}
	
	code = uv_udp_bind(&socket->handle, (const struct sockaddr *) &dest, UV_UDP_REUSEADDR);
	
	if (UNEXPECTED(code != 0)) {
		zend_throw_exception_ex(async_socket_exception_ce, 0, "Failed to bind UDP socket: %s", uv_strerror(code));
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

static ZEND_METHOD(UdpSocket, close)
{
	async_udp_socket *socket;

	zval error;
	zval *val;

	val = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();

	socket = (async_udp_socket *) Z_OBJ_P(getThis());
	
	if (socket->cancel.func == NULL) {
		return;
	}
	
	ASYNC_PREPARE_EXCEPTION(&error, async_stream_closed_exception_ce, "Socket has been closed");
	
	if (val != NULL && Z_TYPE_P(val) != IS_NULL) {
		zend_exception_set_previous(Z_OBJ_P(&error), Z_OBJ_P(val));
		GC_ADDREF(Z_OBJ_P(val));
	}
	
	ASYNC_LIST_REMOVE(&socket->scheduler->shutdown, &socket->cancel);
	
	socket->cancel.func(socket, &error);

	zval_ptr_dtor(&error);
}

static ZEND_METHOD(UdpSocket, flush)
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

static ZEND_METHOD(UdpSocket, getAddress)
{
	async_udp_socket *socket;

	ZEND_PARSE_PARAMETERS_NONE();

	socket = (async_udp_socket *) Z_OBJ_P(getThis());

	RETURN_STR_COPY(socket->ip);
}

static ZEND_METHOD(UdpSocket, getPort)
{
	async_udp_socket *socket;

	ZEND_PARSE_PARAMETERS_NONE();

	socket = (async_udp_socket *) Z_OBJ_P(getThis());
	
	RETURN_LONG(socket->port);
}

static ZEND_METHOD(UdpSocket, setOption)
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
	
	async_uv_op *op;
	
	socket = (async_udp_socket *) udp->data;
	
	ZEND_ASSERT(socket != NULL);
	ZEND_ASSERT(socket->receivers.first != NULL);
	
	if (UNEXPECTED(nread == 0)) {
		efree(buffer->base);
		
		return;
	}
	
	ASYNC_NEXT_CUSTOM_OP(&socket->receivers, op, async_uv_op);
	
	if (EXPECTED(nread > 0)) {
		datagram = (async_udp_datagram *) async_udp_datagram_object_create(async_udp_datagram_ce);
		datagram->data = zend_string_init(buffer->base, (int) nread, 0);
		
		memcpy(&datagram->peer, addr, async_socket_addr_size(addr));
		
		efree(buffer->base);
		
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
	buffer->base = emalloc(8192);
	buffer->len = 8192;
}

static ZEND_METHOD(UdpSocket, receive)
{
	async_udp_socket *socket;
	async_context *context;
	async_uv_op *op;
	
	int code;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
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
	
	ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_uv_op));
	ASYNC_APPEND_OP(&socket->receivers, op);
	
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

static ZEND_METHOD(UdpSocket, send)
{
	async_udp_socket *socket;
	async_udp_datagram *datagram;
	async_udp_send_op *op;
	
	uv_buf_t buffers[1];
	int code;
	
	zval *val;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();
	
	socket = (async_udp_socket *) Z_OBJ_P(getThis());
	datagram = (async_udp_datagram *) Z_OBJ_P(val);
	
	if (UNEXPECTED(Z_TYPE_P(&socket->error) != IS_UNDEF)) {
		ASYNC_FORWARD_ERROR(&socket->error);
		return;
	}
	
	buffers[0] = uv_buf_init(ZSTR_VAL(datagram->data), (unsigned int) ZSTR_LEN(datagram->data));
	
	if (0 && socket->senders.first == NULL) {
	    code = uv_udp_try_send(&socket->handle, buffers, 1, (const struct sockaddr *) &datagram->peer);
	
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
	
	code = uv_udp_send(&op->req, &socket->handle, buffers, 1, (const struct sockaddr *) &datagram->peer, socket_sent);
	
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

ASYNC_CALLBACK socket_sent_async(uv_udp_send_t *req, int status)
{
	async_udp_send_op *op;
	
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
	
	ASYNC_DELREF(&op->context->std);
	ASYNC_DELREF(&op->datagram->std);
	ASYNC_DELREF(&op->socket->std);
	
	ASYNC_FREE_OP(op);
}

static ZEND_METHOD(UdpSocket, sendAsync)
{
	async_udp_socket *socket;
	async_udp_datagram *datagram;
	async_udp_send_op *op;
	
	uv_buf_t buffers[1];
	int code;
	
	zval *val;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();
	
	socket = (async_udp_socket *) Z_OBJ_P(getThis());
	datagram = (async_udp_datagram *) Z_OBJ_P(val);
	
	if (UNEXPECTED(Z_TYPE_P(&socket->error) != IS_UNDEF)) {
		ASYNC_FORWARD_ERROR(&socket->error);
		return;
	}
	
	buffers[0] = uv_buf_init(ZSTR_VAL(datagram->data), (unsigned int) ZSTR_LEN(datagram->data));
	
	if (0 && socket->senders.first == NULL) {
	    code = uv_udp_try_send(&socket->handle, buffers, 1, (const struct sockaddr *) &datagram->peer);
	
	    if (EXPECTED(code >= 0)) {
	        RETURN_LONG(0);
	    }
	
	    ASYNC_CHECK_EXCEPTION(code != UV_EAGAIN, async_socket_exception_ce, "Failed to send UDP data: %s", uv_strerror(code));
	}
	
	ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_udp_send_op));
	
	op->req.data = op;
	op->socket = socket;
	op->datagram = datagram;
	op->context = async_context_ref();
	
	code = uv_udp_send(&op->req, &socket->handle, buffers, 1, (const struct sockaddr *) &datagram->peer, socket_sent_async);
	
	if (UNEXPECTED(code != 0)) {
		ASYNC_FREE_OP(op);		
		zend_throw_exception_ex(async_socket_exception_ce, 0, "Failed to send UDP data: %s", uv_strerror(code));
		
		return;
	}
	
	ASYNC_ADDREF(&socket->std);
	ASYNC_ADDREF(&datagram->std);
	
	ASYNC_APPEND_OP(&socket->senders, op);
	ASYNC_UNREF_ENTER(op->context, socket);
	
	RETURN_LONG(socket->handle.send_queue_size);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_udp_socket_bind, 0, 2, Phalcon\\Async\\Network\\UdpSocket, 0)
	ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_udp_socket_multicast, 0, 2, Phalcon\\Async\\Network\\UdpSocket, 0)
	ZEND_ARG_TYPE_INFO(0, group, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_udp_socket_receive, 0, 0, Phalcon\\Async\\Network\\UdpDatagram, 0)
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_udp_socket_send, 0, 1, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, datagram, Phalcon\\Async\\Network\\UdpDatagram, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_udp_socket_send_async, 0, 1, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, datagram, Phalcon\\Async\\Network\\UdpDatagram, 0)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_udp_socket_send, 0, 1, IS_VOID, NULL, 0)
	ZEND_ARG_OBJ_INFO(0, datagram, Phalcon\\Async\\Network\\UdpDatagram, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_udp_socket_send_async, 0, 1, IS_LONG, NULL, 0)
	ZEND_ARG_OBJ_INFO(0, datagram, Phalcon\\Async\\Network\\UdpDatagram, 0)
ZEND_END_ARG_INFO()
#endif

static const zend_function_entry async_udp_socket_functions[] = {
	ZEND_ME(UdpSocket, bind, arginfo_udp_socket_bind, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(UdpSocket, multicast, arginfo_udp_socket_multicast, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(UdpSocket, close, arginfo_stream_close, ZEND_ACC_PUBLIC)
	ZEND_ME(UdpSocket, flush, arginfo_socket_stream_flush, ZEND_ACC_PUBLIC)
	ZEND_ME(UdpSocket, getAddress, arginfo_socket_get_address, ZEND_ACC_PUBLIC)
	ZEND_ME(UdpSocket, getPort, arginfo_socket_get_port, ZEND_ACC_PUBLIC)
	ZEND_ME(UdpSocket, setOption, arginfo_socket_set_option, ZEND_ACC_PUBLIC)
	ZEND_ME(UdpSocket, receive, arginfo_udp_socket_receive, ZEND_ACC_PUBLIC)
	ZEND_ME(UdpSocket, send, arginfo_udp_socket_send, ZEND_ACC_PUBLIC)
	ZEND_ME(UdpSocket, sendAsync, arginfo_udp_socket_send_async, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static zend_object *async_udp_datagram_object_create(zend_class_entry *ce)
{
	async_udp_datagram *datagram;
	
	datagram = ecalloc(1, sizeof(async_udp_datagram));
	
	zend_object_std_init(&datagram->std, ce);
	datagram->std.handlers = &async_udp_datagram_handlers;
	
	return &datagram->std;
}

static void async_udp_datagram_object_destroy(zend_object *object)
{
	async_udp_datagram *datagram;

	datagram = (async_udp_datagram *) object;
	
	if (EXPECTED(datagram->data != NULL)) {
		zend_string_release(datagram->data);
	}
	
	zend_object_std_dtor(&datagram->std);
}

static zval *read_datagram_property(zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
	async_udp_datagram *datagram;
	
	zend_string *ip;
	uint16_t port;
	char *key;
	
	datagram = (async_udp_datagram *) Z_OBJ_P(object);
	
	key = Z_STRVAL_P(member);
	
	if (strcmp(key, "data") == 0) {
		ZVAL_STR_COPY(rv, datagram->data);
	} else if (strcmp(key, "address") == 0) {
		async_socket_get_peer((const struct sockaddr *) &datagram->peer, &ip, NULL);
		ZVAL_STR(rv, ip);
	} else if (strcmp(key, "port") == 0) {
		async_socket_get_peer((const struct sockaddr *) &datagram->peer, NULL, &port);
		ZVAL_LONG(rv, port);
	} else {
		rv = &EG(uninitialized_zval);
	}
	
	return rv;
}

static int has_datagram_property(zval *object, zval *member, int has_set_exists, void **cache_slot)
{
	zval rv;
	zval *val;

    val = read_datagram_property(object, member, 0, cache_slot, &rv);

    if (val == &EG(uninitialized_zval)) {
    	return 0;
    }

    switch (has_set_exists) {
    	case ZEND_PROPERTY_EXISTS:
    	case ZEND_PROPERTY_ISSET:
    		zval_ptr_dtor(val);
    		return 1;
    }

    convert_to_boolean(val);

    return (Z_TYPE_P(val) == IS_TRUE) ? 1 : 0;
}

static ZEND_METHOD(UdpDatagram, __construct)
{
	async_udp_datagram *datagram;
	
	zend_string *data;
	zend_string *address;
	zend_long port;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 3, 3)
		Z_PARAM_STR(data)
		Z_PARAM_STR(address)
		Z_PARAM_LONG(port)
	ZEND_PARSE_PARAMETERS_END();
	
	datagram = (async_udp_datagram *) Z_OBJ_P(getThis());
	datagram->data = zend_string_copy(data);
	
	if (UNEXPECTED(0 != async_dns_lookup_ip(ZSTR_VAL(address), &datagram->peer, IPPROTO_UDP))) {
		zend_throw_error(NULL, "Failed to assemble peer IP address");		
		return;
	}
	
	async_socket_set_port((struct sockaddr *) &datagram->peer, port);
}

static ZEND_METHOD(UdpDatagram, __debugInfo)
{
	async_udp_datagram *datagram;
	
	zend_string *ip;
	uint16_t port;

	ZEND_PARSE_PARAMETERS_NONE();

	if (USED_RET()) {
		datagram = (async_udp_datagram *) Z_OBJ_P(getThis());
		
		async_socket_get_peer((const struct sockaddr *) &datagram->peer, &ip, &port);
	
		array_init(return_value);

		add_assoc_str(return_value, "data", zend_string_copy(datagram->data));
		add_assoc_str(return_value, "address", ip);
		add_assoc_long(return_value, "port", port);
	}
}

static ZEND_METHOD(UdpDatagram, withData)
{
	async_udp_datagram *datagram;
	async_udp_datagram *result;
	
	zend_string *data;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STR(data)
	ZEND_PARSE_PARAMETERS_END();
	
	datagram = (async_udp_datagram *) Z_OBJ_P(getThis());
	result = (async_udp_datagram *) async_udp_datagram_object_create(async_udp_datagram_ce);
	
	result->data = zend_string_copy(data);
	result->peer = datagram->peer;

	RETURN_OBJ(&result->std);
}

static ZEND_METHOD(UdpDatagram, withPeer)
{
	async_udp_datagram *datagram;
	async_udp_datagram *result;
	
	zend_string *address;
	zend_long port;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_STR(address)
		Z_PARAM_LONG(port)
	ZEND_PARSE_PARAMETERS_END();
	
	datagram = (async_udp_datagram *) Z_OBJ_P(getThis());
	result = (async_udp_datagram *) async_udp_datagram_object_create(async_udp_datagram_ce);
	
	if (UNEXPECTED(0 != async_dns_lookup_ip(ZSTR_VAL(address), &result->peer, IPPROTO_UDP))) {
		ASYNC_DELREF(&result->std);		
		zend_throw_error(NULL, "Failed to assemble peer IP address");
				
		return;
	}
	
	async_socket_set_port((struct sockaddr *) &result->peer, port);
	
	result->data = zend_string_copy(datagram->data);

	RETURN_OBJ(&result->std);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_udp_datagram_ctor, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, address, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_udp_datagram_debug_info, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_udp_datagram_debug_info, 0, 0, IS_ARRAY, NULL, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_udp_datagram_with_data, 0, 1, Phalcon\\Async\\Network\\UdpDatagram, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_udp_datagram_with_peer, 0, 2, Phalcon\\Async\\Network\\UdpDatagram, 0)
	ZEND_ARG_TYPE_INFO(0, address, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry async_udp_datagram_functions[] = {
	ZEND_ME(UdpDatagram, __construct, arginfo_udp_datagram_ctor, ZEND_ACC_PUBLIC)
	ZEND_ME(UdpDatagram, __debugInfo, arginfo_udp_datagram_debug_info, ZEND_ACC_PUBLIC)
	ZEND_ME(UdpDatagram, withData, arginfo_udp_datagram_with_data, ZEND_ACC_PUBLIC)
	ZEND_ME(UdpDatagram, withPeer, arginfo_udp_datagram_with_peer, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


void async_udp_socket_ce_register()
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Network\\UdpSocket", async_udp_socket_functions);
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

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Network\\UdpDatagram", async_udp_datagram_functions);
	async_udp_datagram_ce = zend_register_internal_class(&ce);
	async_udp_datagram_ce->ce_flags |= ZEND_ACC_FINAL;
	async_udp_datagram_ce->create_object = async_udp_datagram_object_create;
	async_udp_datagram_ce->serialize = zend_class_serialize_deny;
	async_udp_datagram_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_udp_datagram_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_udp_datagram_handlers.free_obj = async_udp_datagram_object_destroy;
	async_udp_datagram_handlers.clone_obj = NULL;
	async_udp_datagram_handlers.has_property = has_datagram_property;
	async_udp_datagram_handlers.read_property = read_datagram_property;
}

#endif
