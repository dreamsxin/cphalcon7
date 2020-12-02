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

#include "async/async_tcp.h"
#include "async/async_ssl.h"
#include "async/async_stream.h"
#include "async/async_socket.h"
#include "async/async_pipe.h"
#include <Zend/zend_inheritance.h>
#include "kernel/backend.h"
#include "kernel/fcall.h"
#include "http/parser/http_parser.h"

#ifdef ZEND_WIN32
#include "win32/sockets.h"
#else
#include <sys/uio.h>
#endif

#define HTTP_HEADER "HTTP/1.1 200 OK\r\n" \
    "Content-Type: text/html\r\n" \
    "\r\n"

#define MAX_HTTP_HEADERS (20)

static http_parser_settings parser_settings;

/**
 * Represents a single http header.
 */
typedef struct {
    char *field;
    char *value;
    size_t field_length;
    size_t value_length;
} http_header_t;

/**
 * Represents a http request with internal dependencies.
 *
 * - write request for sending the response
 * - reference to tcp socket as write stream
 * - instance of http_parser parser
 * - string of the http url
 * - string of the http method
 * - amount of total header lines
 * - http header array
 * - body content
 */
typedef struct {
    uv_write_t req;
    async_tcp_socket *socket;
    http_parser parser;
    char *url;
    char *method;
    int header_lines;
    http_header_t headers[MAX_HTTP_HEADERS];
    char *body;
    size_t body_length;
    uv_buf_t resp_buf[2];
} http_request_t;

ASYNC_API zend_class_entry *async_httpserver_ce;

static zend_object_handlers async_httpserver_handlers;

ASYNC_CALLBACK httpserver_disposed(uv_handle_t *handle)
{
	async_tcp_server *server;

	server = (async_tcp_server *) handle->data;
	
	ZEND_ASSERT(server != NULL);

	ASYNC_DELREF(&server->std);
}

ASYNC_CALLBACK shutdown_httpserver(void *obj, zval *error)
{
	async_tcp_server *server;
	
	server = (async_tcp_server *) obj;
	
	ZEND_ASSERT(server != NULL);
	
	server->cancel.func = NULL;
	
	if (error != NULL && Z_TYPE_P(&server->error) == IS_UNDEF) {
		ZVAL_COPY(&server->error, error);
	}

	ASYNC_UV_TRY_CLOSE_REF(&server->std, &server->handle, httpserver_disposed);
}

static inline void phalcon_zend_fci_cache_addref(zend_fcall_info_cache *fci_cache)
{
    if (fci_cache->object) {
        GC_ADDREF(fci_cache->object);
    }
    if (fci_cache->function_handler->op_array.fn_flags & ZEND_ACC_CLOSURE) {
        GC_ADDREF(ZEND_CLOSURE_OBJECT(fci_cache->function_handler));
    }
}

static inline void phalcon_zend_fci_cache_release(zend_fcall_info_cache *fci_cache)
{
    if (fci_cache->object) {
        //OBJ_RELEASE(fci_cache->object);
    }
    if (fci_cache->function_handler && fci_cache->function_handler->op_array.fn_flags & ZEND_ACC_CLOSURE) {
        OBJ_RELEASE(ZEND_CLOSURE_OBJECT(fci_cache->function_handler));
    }
}

static async_tcp_server *async_httpserver_object_create(int domain)
{
	async_tcp_server *server;

	server = ecalloc(1, sizeof(async_tcp_server));

	zend_object_std_init(&server->std, async_httpserver_ce);
	server->std.handlers = &async_httpserver_handlers;
	
	server->scheduler = async_task_scheduler_ref();
	
	server->cancel.object = server;
	server->cancel.func = shutdown_httpserver;
	
	uv_tcp_init_ex(&server->scheduler->loop, &server->handle, domain);

	server->handle.data = server;
	
#ifdef HAVE_ASYNC_SSL
	server->settings.mode = ASYNC_SSL_MODE_SERVER;
#endif

	return server;
}

static void async_httpserver_object_dtor(zend_object *object)
{
	async_tcp_server *server;

	server = (async_tcp_server *) object;

	if (server->cancel.func != NULL) {
		server->cancel.func(server, NULL);
	}

	for (int i=0; i<=ASYNC_SERVER_EVENT_ONCLOSE; i++) {
		phalcon_zend_fci_cache_release(&server->server_callbacks[i]);
	}
}

static void async_httpserver_object_destroy(zend_object *object)
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

/* declare */
void on_close(uv_handle_t *handle);

/* http parser event */
int http_parser_on_message_begin(http_parser *parser/*parser*/) {
    http_request_t *http_request = parser->data;
    http_request->header_lines = 0;

    return 0;
}

int http_parser_on_headers_complete(http_parser *parser/*parser*/) {

    http_request_t *http_request = parser->data;

    const char *method = http_method_str((enum http_method)parser->method);

    http_request->method = malloc(sizeof(method));
    strncpy(http_request->method, method, strlen(method));

    return 0;
}

int http_parser_on_url(http_parser *parser/*parser*/, const char *at, size_t length) {

    http_request_t *http_request = parser->data;

    http_request->url = malloc(length + 1);

    strncpy(http_request->url, at, length);

    http_request->url[length] = '\0';

    return 0;
}

int http_parser_on_header_field(http_parser *parser/*parser*/, const char *at, size_t length) {

    http_request_t *http_request = parser->data;

    http_header_t *header = &http_request->headers[http_request->header_lines];

    header->field = malloc(length + 1);
    header->field_length = length;

    strncpy(header->field, at, length);

    header->field[length] = '\0';

    return 0;
}

int http_parser_on_header_value(http_parser *parser/*parser*/, const char *at, size_t length) {

    http_request_t *http_request = parser->data;

    http_header_t *header = &http_request->headers[http_request->header_lines];

    header->value = malloc(length + 1);
    header->value_length = length;

    strncpy(header->value, at, length);

    header->value[length] = '\0';

    ++http_request->header_lines;

    return 0;
}

int http_parser_on_body(http_parser *parser/*parser*/, const char *at, size_t length) {
    http_request_t *http_request = parser->data;

    http_request->body = malloc(length + 1);
    http_request->body_length = length;

    strncpy(http_request->body, at, length);

    http_request->body[length] = '\0';

    return 0;
}

void on_after_write(uv_write_t *req, int status){
    char *buf = NULL;
    int i = 0;
    http_header_t *header = NULL;
    http_request_t *http_request = req->data;
    buf = http_request->resp_buf[1].base;

    if (NULL != buf) {
        free(buf);
        buf = NULL;
    }

    if (http_request->url != NULL) {
        free(http_request->url);
        http_request->url = NULL;
    }

    if (http_request->method != NULL) {
        free(http_request->method);
        http_request->method = NULL;
    }

    for (i = 0; i < http_request->header_lines; ++i) {
        header = &http_request->headers[i];
        if (header->field != NULL) {
            free(header->field);
            header->field = NULL;
        }
        if (header->value != NULL) {
            free(header->value);
            header->value = NULL;
        }
    }

    if (!uv_is_closing((uv_handle_t*)req->handle)) {
        uv_close((uv_handle_t *) req->handle, on_close);
    }

    return;
}

void job(uv_work_t *req) {
	zval ret = {};

	http_request_t *http_request = (http_request_t *)req->data;
	async_tcp_server *server = http_request->socket->server;
	async_tcp_socket *client = http_request->socket;
	//zval arguments[1];ZVAL_COPY_VALUE(&arguments[0], &client->std);

	http_request->resp_buf[0].base = HTTP_HEADER;
	http_request->resp_buf[0].len = sizeof(HTTP_HEADER) - 1;

	server->server_callbacks[ASYNC_SERVER_EVENT_ONREQUEST].called_scope = client->std.ce;
	server->server_callbacks[ASYNC_SERVER_EVENT_ONREQUEST].object = &client->std;
	if (UNEXPECTED(phalcon_zend_call_function_ex(NULL, &server->server_callbacks[ASYNC_SERVER_EVENT_ONREQUEST], 0, NULL, &ret) == SUCCESS)) {
		if (Z_TYPE(ret) == IS_STRING) {
			http_request->resp_buf[1].base = strdup(Z_STRVAL(ret));
			http_request->resp_buf[1].len = (size_t)Z_STRLEN(ret);
			/* lets send our short http hello world response and close the socket */
			uv_write(&http_request->req, (uv_stream_t *)&http_request->socket->handle, http_request->resp_buf, 2, on_after_write);

			if (Z_TYPE(ret) > IS_NULL) {
				zval_ptr_dtor(&ret);
			}
			return;
		}
    }

	/* lets send our short http hello world response and close the socket */
	uv_write(&http_request->req, (uv_stream_t *)&http_request->socket->handle, http_request->resp_buf, 1, on_after_write);

	if (Z_TYPE(ret) > IS_NULL) {
		zval_ptr_dtor(&ret);
	}
}

void after_job(uv_work_t *req, int status) {
}

int http_parser_on_message_complete(http_parser *parser) {

    http_request_t *http_request = parser->data;

	uv_work_t req = {};
	req.data = (void *) http_request;
	uv_queue_work(async_loop_get(), &req, job, after_job);
    return 0;
}

/* uv event */
void on_close(uv_handle_t *handle) {

    http_request_t *http_request = (http_request_t *) handle->data;

    if (NULL != http_request) {
        free(http_request);
        http_request = NULL;
    }

    return;
}

void alloc_cb(uv_handle_t *handle/*handle*/, size_t suggested_size, uv_buf_t *buf) {
    *buf = uv_buf_init((char *) malloc(suggested_size), (unsigned int)suggested_size);

    return;
}

void on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    ssize_t parsed = 0;

    /* get back our http request*/
    http_request_t *http_request = stream->data;

    if (nread >= 0) {
        /*  call our http parser on the received tcp payload */
        parsed = (ssize_t) http_parser_execute(&http_request->parser, &parser_settings, buf->base, (size_t)nread);
        if (parsed < nread) {
            uv_close((uv_handle_t *) stream, on_close);
        }
    } else {
        uv_close((uv_handle_t *) stream, on_close);
    }

    if (NULL != buf->base) {
        free(buf->base);
    }

    return;
}

ASYNC_CALLBACK httpserver_connected(uv_stream_t *stream, int status)
{
	async_tcp_server *server;
	async_tcp_socket *socket;
	uv_os_fd_t sock;
	int code;

	server = (async_tcp_server *) stream->data;
	
	ZEND_ASSERT(server != NULL);

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

	server->server_callbacks[ASYNC_SERVER_EVENT_ONCONNECT].called_scope = server->std.ce;
	server->server_callbacks[ASYNC_SERVER_EVENT_ONCONNECT].object = &server->std;
	if (UNEXPECTED(phalcon_zend_call_function_ex(NULL, &server->server_callbacks[ASYNC_SERVER_EVENT_ONCONNECT], 0, NULL, NULL) != SUCCESS)) {
    }

    /* initialize a new http http_request struct */
    http_request_t *http_request = malloc(sizeof(http_request_t));
	http_request->socket = socket;

    socket->handle.data = http_request;
    http_request->parser.data = http_request;
    http_request->req.data = http_request;

	http_request->url = NULL;
	http_request->method = NULL;
	http_request->body = NULL;
	http_request->header_lines = 0;
	for (int i = 0; i < MAX_HTTP_HEADERS; ++i) {
		http_request->headers[i].field = NULL;
		http_request->headers[i].field_length = 0;
		http_request->headers[i].value = NULL;
		http_request->headers[i].value_length = 0;
	}
	/* initialize our http parser */
	http_parser_init(&http_request->parser, HTTP_REQUEST);
	uv_read_start((uv_stream_t *) &socket->handle, alloc_cb, on_read);
}

static void create_httpserver(async_tcp_server **result, INTERNAL_FUNCTION_PARAMETERS)
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

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 4)
		Z_PARAM_ZVAL(name)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(port)
		Z_PARAM_OBJECT_OF_CLASS_EX(tls, async_tls_server_encryption_ce, 1, 0)
		Z_PARAM_BOOL(reuseport)
	ZEND_PARSE_PARAMETERS_END();

	host = Z_STR_P(name);

	code = async_dns_lookup_ip(ZSTR_VAL(host), &addr, IPPROTO_TCP);
	
	ASYNC_CHECK_EXCEPTION(code < 0, async_socket_exception_ce, "Failed to assemble IP address: %s", uv_strerror(code));
	
	async_socket_set_port((struct sockaddr *) &addr, port);

	server = async_httpserver_object_create(addr.ss_family);
	
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

	async_socket_get_local_peer((php_socket_t) sock, &server->addr, &server->port);

	if (UNEXPECTED(SUCCESS != setup_server_tls(server, tls))) {
		ASYNC_DELREF(&server->std);
		return;
	}
	
	*result = server;
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_httpserver_bind, 0, 0, Phalcon\\Async\\Network\\HttpServer, 0)
	ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 1)
	ZEND_ARG_OBJ_INFO(0, tls, Phalcon\\Async\\Network\\TlsServerEncryption, 1)
	ZEND_ARG_TYPE_INFO(0, reuseport, _IS_BOOL, 1)
ZEND_END_ARG_INFO();

static PHP_METHOD(HttpServer, bind)
{
	async_tcp_server *server;
	
	create_httpserver(&server, INTERNAL_FUNCTION_PARAM_PASSTHRU);
	
	if (EXPECTED(server)) {
		RETURN_OBJ(&server->std);
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_httpserver_on, 0, 0, Phalcon\\Async\\Network\\HttpServer, 0)
	ZEND_ARG_CALLABLE_INFO(0, func, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(HttpServer, on)
{
	zend_long type = 0;
    zend_fcall_info fci;
    zend_fcall_info_cache fci_cache;
	async_tcp_server *server;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
        Z_PARAM_LONG(type)
        Z_PARAM_FUNC(fci, fci_cache)
	ZEND_PARSE_PARAMETERS_END();

	server = (async_tcp_server *) Z_OBJ_P(getThis());

	if (type > ASYNC_SERVER_EVENT_ONCLOSE) {
		RETURN_FALSE;
	}

	phalcon_zend_fci_cache_release(&server->server_callbacks[type]);

	phalcon_zend_fci_cache_addref(&fci_cache);

	server->server_callbacks[type] = fci_cache;
	RETURN_THIS();
}

static PHP_METHOD(HttpServer, start)
{
	async_tcp_server *server;
	
	int code;

    parser_settings.on_message_begin = http_parser_on_message_begin;
    parser_settings.on_url = http_parser_on_url;
    parser_settings.on_header_field = http_parser_on_header_field;
    parser_settings.on_header_value = http_parser_on_header_value;
    parser_settings.on_headers_complete = http_parser_on_headers_complete;
    parser_settings.on_body = http_parser_on_body;
    parser_settings.on_message_complete = http_parser_on_message_complete;

	server = (async_tcp_server *) Z_OBJ_P(getThis());
	
	code = uv_listen((uv_stream_t *) &server->handle, 128, httpserver_connected);

	if (UNEXPECTED(code != 0)) {
		zend_throw_exception_ex(async_socket_listen_exception_ce, 0, "Server failed to listen: %s", uv_strerror(code));
		return;
	}

	uv_run(async_loop_get(), UV_RUN_DEFAULT);
}

static PHP_METHOD(HttpServer, close)
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
	
	server->cancel.func(server, &error);

	zval_ptr_dtor(&error);
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_CTOR(HttpServer, async_httpserver_ce)
//LCOV_EXCL_STOP

static const zend_function_entry async_httpserver_functions[] = {
	PHP_ME(HttpServer, __construct, arginfo_no_ctor, ZEND_ACC_PRIVATE)
	PHP_ME(HttpServer, bind, arginfo_httpserver_bind, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(HttpServer, on, arginfo_httpserver_on, ZEND_ACC_PUBLIC)
	PHP_ME(HttpServer, start, arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(HttpServer, close, arginfo_stream_close, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

void async_httpserver_ce_register()
{
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Network", "HttpServer", async_httpserver_functions);
	async_httpserver_ce = zend_register_internal_class(&ce);
	async_httpserver_ce->ce_flags |= ZEND_ACC_FINAL;
	async_httpserver_ce->serialize = zend_class_serialize_deny;
	async_httpserver_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_httpserver_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_httpserver_handlers.dtor_obj = async_httpserver_object_dtor;
	async_httpserver_handlers.free_obj = async_httpserver_object_destroy;
	async_httpserver_handlers.clone_obj = NULL;
}
