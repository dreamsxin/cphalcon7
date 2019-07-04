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
#include "async/async_xp.h"

#include <ext/standard/url.h>

static php_stream_transport_factory orig_tcp_factory;
static php_stream_transport_factory orig_tls_factory;

static php_stream_ops tcp_socket_ops;

typedef struct _async_xp_socket_data_tcp {
	ASYNC_XP_SOCKET_DATA_BASE
    uv_tcp_t handle;
    uint16_t pending;
    async_op_list ops;
    zend_bool encrypt;
} async_xp_socket_data_tcp;


ASYNC_CALLBACK free_cb(uv_handle_t *handle)
{
	efree(handle->data);
}

static int tcp_socket_bind(php_stream *stream, async_xp_socket_data *data, php_stream_xport_param *xparam)
{
	php_sockaddr_storage dest;
	php_socket_t sock;
	unsigned int flags;
	
	char *ip;
	int port;
	int code;
	zval *tmp;
	
	flags = 0;
	
	ip = NULL;
	ip = async_xp_parse_ip(xparam->inputs.name, xparam->inputs.namelen, &port, xparam->want_errortext, &xparam->outputs.error_text);
	code = async_dns_lookup_ip(ip, &dest, IPPROTO_TCP);
	
	if (ip != NULL) {
		efree(ip);
	}
	
	if (UNEXPECTED(code != 0)) {
		return FAILURE;
	}
	
	async_socket_set_port((struct sockaddr *) &dest, port);
	
	code = uv_tcp_init_ex(&data->scheduler->loop, (uv_tcp_t *) &data->handle, dest.ss_family);
	
	if (UNEXPECTED(code < 0)) {
		return FAILURE;
	}
	
	data->flags |= ASYNC_XP_SOCKET_FLAG_INIT;
	
	if (UNEXPECTED(0 != uv_fileno((const uv_handle_t *) &data->handle, (uv_os_fd_t *) &sock))) {
		return FAILURE;
	}
	
	async_socket_set_reuseaddr(sock, 1);
	
	if (PHP_STREAM_CONTEXT(stream)) {		
		tmp = php_stream_context_get_option(PHP_STREAM_CONTEXT(stream), "socket", "so_reuseport");
		
		if (tmp != NULL && Z_TYPE_P(tmp) != IS_NULL && zend_is_true(tmp)) {
			async_socket_set_reuseport(sock, 1);
		}
	
#ifdef HAVE_IPV6
		if (dest.ss_family == AF_INET6) {
			tmp = php_stream_context_get_option(PHP_STREAM_CONTEXT(stream), "socket", "ipv6_v6only");
			
			if (tmp != NULL && Z_TYPE_P(tmp) != IS_NULL && zend_is_true(tmp)) {
				flags |= UV_TCP_IPV6ONLY;
			}
		}
#endif
	}
	
	code = uv_tcp_bind((uv_tcp_t *) &data->handle, (const struct sockaddr *) &dest, flags);
	
	return code;
}

ASYNC_CALLBACK tcp_socket_listen_cb(uv_stream_t *server, int status)
{
	async_xp_socket_data_tcp *tcp;
	async_uv_op *op;
	
	tcp = (async_xp_socket_data_tcp *) server->data;
	
	if (status == 0) {
		tcp->pending++;
	}
	
	while (tcp->ops.first != NULL) {
		ASYNC_NEXT_CUSTOM_OP(&tcp->ops, op, async_uv_op);
		
		op->code = status;
		
		ASYNC_FINISH_OP(op);
	}
}

static int tcp_socket_listen(php_stream *stream, async_xp_socket_data *data, php_stream_xport_param *xparam)
{
	int code;
	
	code = uv_listen((uv_stream_t *) &data->handle, xparam->inputs.backlog, tcp_socket_listen_cb);
	
	if (UNEXPECTED(code < 0)) {
		ASYNC_XP_SOCKET_REPORT_NETWORK_ERROR(code, xparam);
		
		return FAILURE;
	}

	return code;
}

ASYNC_CALLBACK tcp_socket_connect_cb(uv_connect_t *req, int status)
{
	async_uv_op *op;	
	
	op = (async_uv_op *) req->data;
	
	ZEND_ASSERT(op != NULL);
	
	op->code = (status == UV_ECANCELED) ? UV_ETIMEDOUT : status;
	
	ASYNC_FINISH_OP(op);
}

ASYNC_CALLBACK connect_timer_cb(uv_timer_t *timer)
{
	async_xp_socket_data_tcp *tcp;
	
	tcp = (async_xp_socket_data_tcp *) timer->data;
	
	// Need to close the socket right here to cancel connect, anything else will segfault later...
	if (tcp->flags & ASYNC_XP_SOCKET_FLAG_INIT) {
		ASYNC_UV_CLOSE(&tcp->handle, NULL);
	}
}

static int tcp_socket_connect(php_stream *stream, async_xp_socket_data *data, php_stream_xport_param *xparam)
{
	async_xp_socket_data_tcp *tcp;
	async_uv_op *op;
	
	uv_connect_t req;
	php_sockaddr_storage dest;
	uint64_t timeout;
	
	char *ip;
	int port;
	int code;
	
	tcp = (async_xp_socket_data_tcp *) data;
	
	ip = NULL;
	ip = async_xp_parse_ip(xparam->inputs.name, xparam->inputs.namelen, &port, xparam->want_errortext, &xparam->outputs.error_text);
	code = async_dns_lookup_ip(ip, &dest, IPPROTO_TCP);
	
	if (ip != NULL) {
		efree(ip);
	}
	
	if (UNEXPECTED(code != 0)) {
		return FAILURE;
	}
	
	async_socket_set_port((struct sockaddr *) &dest, port);
	
	uv_tcp_init(&data->scheduler->loop, (uv_tcp_t *) &data->handle);
	
	data->flags |= ASYNC_XP_SOCKET_FLAG_INIT;
	
	code = uv_tcp_connect(&req, (uv_tcp_t *) &data->handle, (const struct sockaddr *) &dest, tcp_socket_connect_cb);
	
	if (EXPECTED(code == 0)) {
		ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_uv_op));
		ASYNC_APPEND_OP(&tcp->ops, op);
		
		req.data = op;
		
		timeout = ((uint64_t) xparam->inputs.timeout->tv_sec) * 1000 + ((uint64_t) xparam->inputs.timeout->tv_usec) / 1000;
		
		if (timeout > 0) {
			uv_timer_start(&data->timer, connect_timer_cb, timeout, 0);
		}
		
		code = async_await_op((async_op *) op);
		
		if (timeout > 0) {
			uv_timer_stop(&data->timer);
		}
		
		if (code == FAILURE) {
			ASYNC_FORWARD_OP_ERROR(op);
			ASYNC_FREE_OP(op);
			
			return FAILURE;
		}
		
		code = op->code;
		
		ASYNC_FREE_OP(op);
	}
	
	if (UNEXPECTED(code < 0)) {
		ASYNC_XP_SOCKET_REPORT_NETWORK_ERROR(code, xparam);
		
		return FAILURE;
	}
	
	data->astream = async_stream_init((uv_stream_t *) &data->handle, 0);
	
	if (tcp->encrypt) {
		if (FAILURE == php_stream_xport_crypto_setup(stream, 0, NULL)) {
			php_error_docref(NULL, E_WARNING, "Failed to setup crypto");
			xparam->outputs.returncode = -1;
			
			return FAILURE;
		}
		
		if (FAILURE == php_stream_xport_crypto_enable(stream, 1)) {
			php_error_docref(NULL, E_WARNING, "Failed to enable crypto");
			xparam->outputs.returncode = -1;
			
			return FAILURE;
		}
	}
	
	return SUCCESS;
}

static int tcp_socket_shutdown(php_stream *stream, async_xp_socket_data *data, int how)
{
	zval ref;

	int flag;
	
	switch (how) {
	case ASYNC_XP_SOCKET_SHUT_RD:
		flag = ASYNC_STREAM_SHUT_RD;
		break;
	case ASYNC_XP_SOCKET_SHUT_WR:
		flag = ASYNC_STREAM_SHUT_WR;
		break;
	default:
		flag = ASYNC_STREAM_SHUT_RDWR;
	}
	
	ZVAL_RES(&ref, stream->res);
	async_stream_shutdown(data->astream, flag, &ref);
	
	return SUCCESS;
}

static int tcp_socket_get_peer(async_xp_socket_data *data, zend_bool remote, zend_string **textaddr, struct sockaddr **addr, socklen_t *len)
{
	php_sockaddr_storage sa;
	int sl;
	int code;
	
	memset(&sa, 0, sizeof(php_sockaddr_storage));
	sl = sizeof(php_sockaddr_storage);
	
	if (remote) {
		code = uv_tcp_getpeername((uv_tcp_t *) &data->handle, (struct sockaddr *) &sa, &sl);
	} else {
		code = uv_tcp_getsockname((uv_tcp_t *) &data->handle, (struct sockaddr *) &sa, &sl);
	}
	
	if (UNEXPECTED(code < 0)) {
		return code;
	}

	php_network_populate_name_from_sockaddr((struct sockaddr *) &sa, sl, textaddr, addr, len);
	
	return SUCCESS;
}

static int tcp_socket_accept(php_stream *stream, async_xp_socket_data *data, php_stream_xport_param *xparam)
{
	async_xp_socket_data_tcp *server;
	async_xp_socket_data_tcp *client;
	async_uv_op *op;
	
	int code;
	
	server = (async_xp_socket_data_tcp *) data;
	code = 0;
	
	client = ecalloc(1, sizeof(async_xp_socket_data_tcp));
	
	uv_tcp_init(&server->scheduler->loop, &client->handle);
	
	client->flags |= ASYNC_XP_SOCKET_FLAG_INIT;	
	client->handle.data = client;

	do {
		if (server->pending > 0) {
			server->pending--;
			
			code = uv_accept((uv_stream_t *) &server->handle, (uv_stream_t *) &client->handle);
			
			if (code == 0) {
				xparam->outputs.client = async_xp_socket_create((async_xp_socket_data *) client, &tcp_socket_ops, NULL STREAMS_CC);
				
				break;
			}
		}
		
		ASYNC_ALLOC_CUSTOM_OP(op, sizeof(async_uv_op));
		ASYNC_APPEND_OP(&server->ops, op);
		
		if (async_await_op((async_op *) op) == FAILURE) {
			ASYNC_FORWARD_OP_ERROR(op);
			ASYNC_FREE_OP(op);
			
			break;
		}
		
		code = op->code;
		
		ASYNC_FREE_OP(op);
		
		if (code < 0) {
			break;
		}
	} while (xparam->outputs.client == NULL);
	
	if (UNEXPECTED(xparam->outputs.client == NULL)) {
		ASYNC_UV_CLOSE(&client->handle, free_cb);
	
		return code;
	}
	
	xparam->outputs.client->ctx = stream->ctx;
	
	if (stream->ctx) {
		GC_ADDREF(stream->ctx);
	}
	
	client->flags |= ASYNC_XP_SOCKET_FLAG_ACCEPTED;
	client->astream = async_stream_init((uv_stream_t *) &client->handle, 0);
	
	client->shutdown = tcp_socket_shutdown;
	client->get_peer = tcp_socket_get_peer;

	if (server->ssl != NULL) {
		client->ssl = server->ssl;
		client->ssl->refcount++;
	}

	if (server->encrypt) {
		if (FAILURE == php_stream_xport_crypto_setup(xparam->outputs.client, 0, NULL)) {
			php_error_docref(NULL, E_WARNING, "Failed to setup crypto");
			xparam->outputs.returncode = -1;
			
			php_stream_close(xparam->outputs.client);
			
			return FAILURE;
		}
		
		if (FAILURE == php_stream_xport_crypto_enable(xparam->outputs.client, 1)) {
			php_error_docref(NULL, E_WARNING, "Failed to enable crypto");
			xparam->outputs.returncode = -1;
			
			php_stream_close(xparam->outputs.client);
			
			return FAILURE;
		}
	}

	return SUCCESS;
}

static zend_string *parse_host(const char *resource, size_t rlen)
{
	php_url *url;
	zend_string *host;

	const char *tmp;
	size_t len;

	if (!resource) {
		return NULL;
	}
	
	url = php_url_parse_ex(resource, rlen);
	
	if (!url) {
		return NULL;
	}
	
	host = NULL;

	if (url->host) {
#if PHP_VERSION_ID >= 70300
		tmp = ZSTR_VAL(url->host);
		len = ZSTR_LEN(url->host);
#else
		tmp = (const char *)url->host;
		len = strlen(url->host);
#endif
		while (len && tmp[len - 1] == '.') {
			len--;
		}
		
		if (len) {
			host = zend_string_init(tmp, len, 0);
		}
	}

	php_url_free(url);
	
	return host;
}

static php_stream *tcp_socket_factory(const char *proto, size_t plen, const char *res, size_t reslen,
	const char *pid, int options, int flags, struct timeval *timeout, php_stream_context *context STREAMS_DC)
{
	async_xp_socket_data_tcp *data;
	php_stream *stream;

	data = ecalloc(1, sizeof(async_xp_socket_data_tcp));
	
	stream = async_xp_socket_create((async_xp_socket_data *) data, &tcp_socket_ops, pid STREAMS_CC);

	if (UNEXPECTED(stream == NULL)) {
		efree(data);
	
		return NULL;
	}
 	
 	data->connect = tcp_socket_connect;
 	data->bind = tcp_socket_bind;
 	data->listen = tcp_socket_listen;
 	data->accept = tcp_socket_accept;
	data->shutdown = tcp_socket_shutdown;
	data->get_peer = tcp_socket_get_peer;
	
	data->peer = parse_host(res, reslen);
	
	if (strncmp(proto, "tcp", plen) != 0 && strncmp(proto, "async-tcp", plen) != 0) {
		data->encrypt = 1;
	}
	
	return stream;
}

void async_tcp_socket_init()
{
	async_xp_socket_populate_ops(&tcp_socket_ops, "tcp_socket/async");

	if (ASYNC_G(tcp_enabled)) {
		orig_tcp_factory = async_xp_socket_register("tcp", tcp_socket_factory);
		orig_tls_factory = async_xp_socket_register("tls", tcp_socket_factory);
	}
	
	php_stream_xport_register("async-tcp", tcp_socket_factory);
	php_stream_xport_register("async-tls", tcp_socket_factory);
}

void async_tcp_socket_shutdown()
{
	php_stream_xport_unregister("async-tls");
	php_stream_xport_unregister("async-tcp");

	if (ASYNC_G(tcp_enabled)) {
		php_stream_xport_register("tls", orig_tls_factory);
		php_stream_xport_register("tcp", orig_tcp_factory);
	}
}