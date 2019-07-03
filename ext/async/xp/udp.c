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

#include "async/async_xp.h"

static php_stream_transport_factory orig_udp_factory;
static php_stream_ops udp_socket_ops;

#define ASYNC_XP_SOCKET_UDP_FLAG_RECEIVING (1 << 7)
#define ASYNC_XP_SOCKET_UDP_FLAG_CONNECTED (1 << 6)

typedef struct _async_xp_socket_data_udp {
	ASYNC_XP_SOCKET_DATA_BASE
    uv_udp_t handle;
    php_sockaddr_storage dest;
    async_op_list senders;
    async_op_list receivers;
} async_xp_socket_data_udp;

typedef struct _async_xp_udp_receive_op {
	async_op base;
	int code;
	unsigned int flags;
	php_stream_xport_param *xparam;
	char *buf;
	size_t len;
} async_xp_udp_receive_op;


static int udp_socket_bind(php_stream *stream, async_xp_socket_data *data, php_stream_xport_param *xparam)
{
	php_sockaddr_storage dest;
	unsigned int flags;
	
	char *ip;
	int port;
	int code;
	zval *tmp;
	
	flags = 0;
	
	ip = NULL;
	ip = async_xp_parse_ip(xparam->inputs.name, xparam->inputs.namelen, &port, xparam->want_errortext, &xparam->outputs.error_text);
	code = async_dns_lookup_ip(ip, &dest, IPPROTO_UDP);
	
	if (ip != NULL) {
		efree(ip);
	}
	
	if (UNEXPECTED(code != 0)) {
		ASYNC_XP_SOCKET_REPORT_NETWORK_ERROR(code, xparam);
		
		return FAILURE;
	}
	
	async_socket_set_port((struct sockaddr *) &dest, port);
	
	if (PHP_STREAM_CONTEXT(stream)) {
		tmp = php_stream_context_get_option(PHP_STREAM_CONTEXT(stream), "socket", "so_reuseport");
		
		if (tmp != NULL && Z_TYPE_P(tmp) != IS_NULL && zend_is_true(tmp)) {
			flags |= UV_UDP_REUSEADDR ;
		}

#ifdef HAVE_IPV6
		if (dest.ss_family == AF_INET6) {
			tmp = php_stream_context_get_option(PHP_STREAM_CONTEXT(stream), "socket", "ipv6_v6only");
			
			if (tmp != NULL && Z_TYPE_P(tmp) != IS_NULL && zend_is_true(tmp)) {
				flags |= UV_UDP_IPV6ONLY;
			}
		}
#endif
	}
	
	code = uv_udp_bind((uv_udp_t *) &data->handle, (const struct sockaddr *) &dest, flags);
	
	if (code != 0) {
		ASYNC_XP_SOCKET_REPORT_NETWORK_ERROR(code, xparam);
	
		return FAILURE;
	}
	
	if (PHP_STREAM_CONTEXT(stream)) {
		tmp = php_stream_context_get_option(PHP_STREAM_CONTEXT(stream), "socket", "so_broadcast");
		
		if (tmp != NULL && Z_TYPE_P(tmp) != IS_NULL && zend_is_true(tmp)) {
			code = uv_udp_set_broadcast((uv_udp_t *) &data->handle, 1);
			
			if (code < 0) {
				ASYNC_XP_SOCKET_REPORT_NETWORK_ERROR(code, xparam);
				
				return FAILURE;
			}
		}
	}
	
	return SUCCESS;
}

ASYNC_CALLBACK udp_socket_send_cb(uv_udp_send_t *req, int status)
{
	async_uv_op *op;
	
	op = (async_uv_op *) req->data;
	
	ZEND_ASSERT(op != NULL);
	
	op->code = status;
	
	ASYNC_FINISH_OP(op);
}

static int do_send(async_xp_socket_data *data, const struct sockaddr *dest, char *buf, size_t len)
{
	async_xp_socket_data_udp *udp;
	async_uv_op *op;

	uv_udp_send_t req;
	uv_buf_t bufs[1];
	int code;
	
	udp = (async_xp_socket_data_udp *) data;

	bufs[0] = uv_buf_init(buf, (unsigned int) len);
	
	if (dest == NULL) {
		dest = (const struct sockaddr *) &udp->dest;
	}
	
	code = uv_udp_try_send((uv_udp_t *) &data->handle, bufs, 1, dest);
	
	if (code >= 0) {
		return SUCCESS;
	}
	
	if (UNEXPECTED(code < 0 && code != UV_EAGAIN)) {
		return code;
	}
	
	code = uv_udp_send(&req, (uv_udp_t *) &data->handle, bufs, 1, dest, udp_socket_send_cb);
	
	if (EXPECTED(code >= 0)) {
		ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_uv_op));
		ASYNC_APPEND_OP(&udp->senders, op);
		
		req.data = op;
		
		if (async_await_op((async_op *) op) == FAILURE) {
			ASYNC_FORWARD_OP_ERROR(op);
			ASYNC_FREE_OP(op);
			
			return FAILURE;
		}
		
		code = op->code;
		
		ASYNC_FREE_OP(op);
	}
	
	return code;
}

static int udp_socket_send(php_stream *stream, async_xp_socket_data *data, php_stream_xport_param *xparam)
{
	return do_send(data, (const struct sockaddr *) xparam->inputs.addr, xparam->inputs.buf, xparam->inputs.buflen);
}

ASYNC_CALLBACK udp_socket_receive_cb(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned int flags)
{
	async_xp_socket_data_udp *udp;
	async_xp_udp_receive_op *op;
	
	udp = (async_xp_socket_data_udp *) handle->data;
	
	ZEND_ASSERT(udp->receivers.first != NULL);
	
	if (nread == 0) {
		return;
	}
	
	ASYNC_NEXT_CUSTOM_OP(&udp->receivers, op, async_xp_udp_receive_op);
	
	op->code = (int) nread;
	op->flags = flags;
	
	if (nread > 0 && op->xparam != NULL) {
		php_network_populate_name_from_sockaddr(
			(struct sockaddr *) addr,
			sizeof(struct sockaddr),
			op->xparam->want_textaddr ? &op->xparam->outputs.textaddr : NULL,
			op->xparam->want_addr ? &op->xparam->outputs.addr : NULL,
			op->xparam->want_addr ? &op->xparam->outputs.addrlen : NULL
		);
	}
	
	ASYNC_FINISH_OP(op);
	
	if (udp->receivers.first == NULL) {
		uv_udp_recv_stop(handle);
		
		udp->flags &= ~ASYNC_XP_SOCKET_UDP_FLAG_RECEIVING;
	}
}

ASYNC_CALLBACK udp_socket_receive_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	async_xp_socket_data_udp *udp;
	async_xp_udp_receive_op *op;
	
	udp = (async_xp_socket_data_udp *) handle->data;
	op = (async_xp_udp_receive_op *) udp->receivers.first;
	
	ZEND_ASSERT(op != NULL);
	
	buf->base = op->buf;
	buf->len = (uv_buf_size_t) op->len;
}

static int udp_socket_receive(php_stream *stream, async_xp_socket_data *data, php_stream_xport_param *xparam)
{
	async_xp_socket_data_udp *udp;
	async_xp_udp_receive_op *op;
	
	int code;
	
	udp = (async_xp_socket_data_udp *) data;
	
	if (!(udp->flags & ASYNC_XP_SOCKET_UDP_FLAG_RECEIVING)) {
		uv_udp_recv_start(&udp->handle, udp_socket_receive_alloc, udp_socket_receive_cb);
		
		udp->flags |= ASYNC_XP_SOCKET_UDP_FLAG_RECEIVING;
	}
	
	ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_xp_udp_receive_op));
	ASYNC_APPEND_OP(&udp->receivers, op);
	
	op->xparam = xparam;
	op->buf = xparam->inputs.buf;
	op->len = xparam->inputs.buflen;
	
	if (async_await_op((async_op *) op) == FAILURE) {
		ASYNC_FORWARD_OP_ERROR(op);
		ASYNC_FREE_OP(op);
		
		return FAILURE;
	}
	
	code = op->code;
	
	ASYNC_FREE_OP(op);
	
	return code;
}

static int udp_socket_get_peer(async_xp_socket_data *data, zend_bool remote, zend_string **textaddr, struct sockaddr **addr, socklen_t *len)
{
	php_sockaddr_storage sa;
	int sl;
	int code;
	
	memset(&sa, 0, sizeof(php_sockaddr_storage));
	sl = sizeof(php_sockaddr_storage);
	
	if (remote) {
		return FAILURE;
	}
	
	code = uv_udp_getsockname((uv_udp_t *) &data->handle, (struct sockaddr *) &sa, &sl);
	
	if (UNEXPECTED(code < 0)) {
		return code;
	}

	php_network_populate_name_from_sockaddr((struct sockaddr *) &sa, sl, textaddr, addr, len);
	
	return SUCCESS;
}

static int udp_socket_connect(php_stream *stream, async_xp_socket_data *data, php_stream_xport_param *xparam)
{
	async_xp_socket_data_udp *udp;
	php_sockaddr_storage dest;
	
	char *ip;
	int port;
	int code;

	udp = (async_xp_socket_data_udp *) data;
	
	code = uv_ip4_addr("0.0.0.0", 0, (struct sockaddr_in *) &dest);
	
	if (UNEXPECTED(code != 0)) {
		return FAILURE;
	}	
	
	code = uv_udp_bind((uv_udp_t *) &data->handle, (const struct sockaddr *) &dest, 0);
	
	if (code != 0) {
		ASYNC_XP_SOCKET_REPORT_NETWORK_ERROR(code, xparam);
		
		return FAILURE;
	}
	
	ip = NULL;
	ip = async_xp_parse_ip(xparam->inputs.name, xparam->inputs.namelen, &port, xparam->want_errortext, &xparam->outputs.error_text);
	code = async_dns_lookup_ip(ip, &udp->dest, IPPROTO_TCP);
	
	if (ip != NULL) {
		efree(ip);
	}
	
	if (UNEXPECTED(code != 0)) {
		ASYNC_XP_SOCKET_REPORT_NETWORK_ERROR(code, xparam);
		
		return FAILURE;
	}
	
	async_socket_set_port((struct sockaddr *) &udp->dest, port);
	
	data->flags |= ASYNC_XP_SOCKET_UDP_FLAG_CONNECTED;
	
	return SUCCESS;
}

static size_t udp_socket_write(php_stream *stream, async_xp_socket_data *data, const char *buf, size_t count)
{
	int code;
	
	ZEND_ASSERT(data->flags & ASYNC_XP_SOCKET_UDP_FLAG_CONNECTED);
	
	code = do_send(data, NULL, (char *) buf, count);
	
	if (code == 0) {
		return count;
	}

	return FAILURE;
}

static size_t udp_socket_read(php_stream *stream, async_xp_socket_data *data, char *buf, size_t count)
{
	async_xp_socket_data_udp *udp;
	async_xp_udp_receive_op *op;
	
	int code;
	
	ZEND_ASSERT(data->flags & ASYNC_XP_SOCKET_UDP_FLAG_CONNECTED);
	
	udp = (async_xp_socket_data_udp *) data;
	
	if (!(udp->flags & ASYNC_XP_SOCKET_UDP_FLAG_RECEIVING)) {
		uv_udp_recv_start(&udp->handle, udp_socket_receive_alloc, udp_socket_receive_cb);
		
		udp->flags |= ASYNC_XP_SOCKET_UDP_FLAG_RECEIVING;
	}
	
	ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_xp_udp_receive_op));
	ASYNC_APPEND_OP(&udp->receivers, op);
	
	op->buf = buf;
	op->len = count;
	
	if (async_await_op((async_op *) op) == FAILURE) {
		ASYNC_FORWARD_OP_ERROR(op);
		ASYNC_FREE_OP(op);
		
		return FAILURE;
	}
	
	code = op->code;
	
	ASYNC_FREE_OP(op);
	
	return code;
}

static php_stream *udp_socket_factory(const char *proto, size_t plen, const char *res, size_t reslen,
	const char *pid, int options, int flags, struct timeval *timeout, php_stream_context *context STREAMS_DC)
{
	async_xp_socket_data_udp *data;
	php_stream *stream;

	data = ecalloc(1, sizeof(async_xp_socket_data_udp));
	
	data->flags = ASYNC_XP_SOCKET_FLAG_DGRAM;
	
	stream = async_xp_socket_create((async_xp_socket_data *) data, &udp_socket_ops, pid STREAMS_CC);

	if (UNEXPECTED(stream == NULL)) {
		efree(data);
	
		return NULL;
	}
 	
 	uv_udp_init(&data->scheduler->loop, &data->handle);
 	
 	data->bind = udp_socket_bind;
 	data->send = udp_socket_send;
 	data->receive = udp_socket_receive;
 	
 	data->connect = udp_socket_connect;
 	data->write = udp_socket_write;
 	data->read = udp_socket_read;
 	
 	data->get_peer = udp_socket_get_peer;
 	
	return stream;
}

void async_udp_socket_init()
{
	async_xp_socket_populate_ops(&udp_socket_ops, "udp_socket/async");

	if (ASYNC_G(udp_enabled)) {
		orig_udp_factory = async_xp_socket_register("udp", udp_socket_factory);
	}
	
	php_stream_xport_register("async-udp", udp_socket_factory);
}

void async_udp_socket_shutdown()
{
	php_stream_xport_unregister("async-udp");
	
	if (ASYNC_G(udp_enabled)) {
		php_stream_xport_register("udp", orig_udp_factory);
	}
}