
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

#include "websocket/server.h"
#include "websocket/eventloopinterface.h"
#include "websocket/connection.h"

#include <php_network.h>
#include <zend_interfaces.h>

/**
 * Phalcon\Websocket\Server
 *
 */
zend_class_entry *phalcon_websocket_server_ce;

PHP_METHOD(Phalcon_Websocket_Server, __construct);
PHP_METHOD(Phalcon_Websocket_Server, setEventLoop);
PHP_METHOD(Phalcon_Websocket_Server, serviceFd);
PHP_METHOD(Phalcon_Websocket_Server, run);
PHP_METHOD(Phalcon_Websocket_Server, stop);
PHP_METHOD(Phalcon_Websocket_Server, broadcast);
PHP_METHOD(Phalcon_Websocket_Server, on);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_server___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_server_seteventloop, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_OBJECT, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_server_servicefd, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_RESOURCE, 0)
	ZEND_ARG_TYPE_INFO(0, events, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_server_broadcast, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, ignored, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_phalcon_websocket_server_on, 0, 2, IS_TRUE|IS_FALSE, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, event, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_websocket_server_method_entry[] = {
	PHP_ME(Phalcon_Websocket_Server, __construct, arginfo_phalcon_websocket_server___construct, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Server, setEventLoop, arginfo_phalcon_websocket_server_seteventloop, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Server, serviceFd, arginfo_phalcon_websocket_server_serviceFd, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Server, run, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Server, stop, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Server, on, arginfo_phalcon_websocket_server_on, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Server, broadcast, arginfo_phalcon_websocket_server_broadcast, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

enum ws_protocols {
	PROTOCOL_EXTPHP = 0,

	PROTOCOLS_COUNT
};

int callback_ext_php(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	phalcon_websocket_server_object *intern = phalcon_websocket_server_object_from_ctx(wsi->context);
	ws_connection_obj * wsconn;
	zval *connection = user;
	zval retval;
	int n, return_code = 0;
	zend_string *text;
	struct lws_pollargs *pa = in;
	ws_callback *user_cb;

	switch (reason) {
		/** External event loop **/
		case LWS_CALLBACK_ADD_POLL_FD:
			if (intern->eventloop) {
				return_code = invoke_eventloop_cb(intern, "add", pa->fd, pa->events);
			}
			break;

		case LWS_CALLBACK_DEL_POLL_FD:
			if (intern->eventloop) {
				return_code = invoke_eventloop_cb(intern, "delete", pa->fd, -1);
			}
			break;

		case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
			if (intern->eventloop) {
				return_code = invoke_eventloop_cb(intern, "setmode", pa->fd, pa->events);
			}
			break;

		case LWS_CALLBACK_LOCK_POLL:
			if (intern->eventloop) {
				return_code = invoke_eventloop_cb(intern, "lock", 0, -1);
			}
			break;

		case LWS_CALLBACK_UNLOCK_POLL:
			if (intern->eventloop) {
				return_code = invoke_eventloop_cb(intern, "unlock", 0, -1);
			}
			break;
		/** End external event loop **/

		case LWS_CALLBACK_WSI_CREATE:
			printf("WSI create\n");
			break;

		case LWS_CALLBACK_WSI_DESTROY:
			printf("Destroy WSI\n");
			break;

		case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
			if (intern->callbacks[PHP_CB_SERVER_FILTER_CONNECTION]) {
				user_cb = intern->callbacks[PHP_CB_SERVER_FILTER_CONNECTION];
				int sockfd = (int) in;
				struct sockaddr_storage addr;
				int len = sizeof(addr);
				int port; char ipstr[64] = "";
				getpeername(sockfd, (struct sockaddr*) &addr, &len);
				if (addr.ss_family == AF_INET) {
					struct sockaddr_in *s = (struct sockaddr_in *)&addr;
					port = ntohs(s->sin_port);
					lws_plat_inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
				} else { // AF_INET6
					struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
					port = ntohs(s->sin6_port);
					lws_plat_inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
				}

				ZVAL_NULL(&retval);
				zval params[2];
				ZVAL_STRINGL(&params[0], ipstr, len);
				ZVAL_LONG(&params[1], port);
				user_cb->fci->param_count = 2;
				user_cb->fci->params = params;
				user_cb->fci->retval = &retval;
				if (SUCCESS != zend_call_function(user_cb->fci, user_cb->fcc TSRMLS_CC)) {
					php_error_docref(NULL, E_WARNING, "Unable to call close callback");
				}
				zval_dtor(&retval);
			}
			break;

		case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
			if (intern->callbacks[PHP_CB_SERVER_FILTER_HEADERS]) {
				user_cb = intern->callbacks[PHP_CB_SERVER_FILTER_HEADERS];
				ZVAL_FALSE(&retval);
				zval params[2];
				get_token_as_array(&params[0], wsi);
				ZVAL_OBJ(&params[1], &intern->std);
				user_cb->fci->param_count = 2;
				user_cb->fci->params = params;
				user_cb->fci->retval = &retval;
				if (SUCCESS != zend_call_function(user_cb->fci, user_cb->fcc TSRMLS_CC)) {
					php_error_docref(NULL, E_WARNING, "Unable to call filter callback");
				}

				if (!zval_is_true(&retval)) {
					return_code = -1;
				}

				zval_dtor(&retval);
			}
			break;

		case LWS_CALLBACK_ESTABLISHED:
			object_init_ex(connection, ws_connection_ce);
			wsconn = (ws_connection_obj *) Z_OBJ_P(connection);
			wsconn->id = ++intern->next_id;
			printf("Accept connection %lu\n", wsconn->id);
			wsconn->wsi = wsi;

			zval_addref_p(connection);
			add_index_zval(&intern->connections, wsconn->id, connection);

			wsconn->connected = 1;

			if (intern->callbacks[PHP_CB_SERVER_ACCEPT]) {
				user_cb = intern->callbacks[PHP_CB_SERVER_ACCEPT];
				ZVAL_NULL(&retval);
				zval params[2];
				ZVAL_OBJ(&params[0], &intern->std);
				ZVAL_COPY_VALUE(&params[1], connection);
				user_cb->fci->param_count = 2;
				user_cb->fci->params = params;
				user_cb->fci->retval = &retval;
				user_cb->fci->no_separation = (zend_bool) 0;

				printf("Ready to call accept handler\n");
				int res = zend_call_function(user_cb->fci, user_cb->fcc TSRMLS_CC);
				if (SUCCESS != res) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to call accept callback");
				}
				zval_dtor(&retval);
			}
			break;

		case LWS_CALLBACK_RECEIVE:
			printf("Receive data.\n");
			if (intern->callbacks[PHP_CB_SERVER_DATA]) {
				user_cb = intern->callbacks[PHP_CB_SERVER_DATA];
				ZVAL_NULL(&retval);
				zval params[3];
				// TODO Check if message is complete before calling CB
				ZVAL_OBJ(&params[0], &intern->std);
				ZVAL_COPY_VALUE(&params[1], connection);
				ZVAL_COPY_VALUE(&params[2], connection);
				ZVAL_STRINGL(&params[2], in, len);
				user_cb->fci->param_count = 3;
				user_cb->fci->params = params;
				user_cb->fci->retval = &retval;
				if (SUCCESS != zend_call_function(user_cb->fci, user_cb->fcc TSRMLS_CC)) {
					php_error_docref(NULL, E_WARNING, "Unable to call data callback");
				}

				// Free string if needed
				zval_delref_p(&params[2]);
				if (zval_refcount_p(&params[2]) < 1) {
					zend_string_free(Z_STR(params[2]));
				}

				// If return is different from true (or assimilated), close connection
				if (!zval_is_true(&retval)) {
					wsconn = (ws_connection_obj *) Z_OBJ_P(connection);
					wsconn->connected = 0;
					return_code = -1;
				}

				zval_dtor(&retval);
			}
			break;

		case LWS_CALLBACK_SERVER_WRITEABLE:
			wsconn = (ws_connection_obj *) Z_OBJ_P(connection);
			if (!wsconn->connected) {
				return_code = -1;
				break;
			}

			while (wsconn->read_ptr != wsconn->write_ptr) {
				text = wsconn->buf[wsconn->read_ptr];
				n = lws_write(wsi, text->val, text->len, LWS_WRITE_TEXT);

				if (n < 0) {
					lwsl_err("Write to socket %lu failed with code %d\n", wsconn->id, n);
					return 1;
				}
				if (n < (int) text->len) {
					// TODO Implements partial write
					wsconn->buf[wsconn->read_ptr] = NULL;
					wsconn->read_ptr = (wsconn->read_ptr + 1) % WEBSOCKET_CONNECTION_BUFFER_SIZE;
					lwsl_err("Partial write\n");
					return -1;
				}

				// Cleanup
				wsconn->buf[wsconn->read_ptr] = NULL;
				wsconn->read_ptr = (wsconn->read_ptr + 1) % WEBSOCKET_CONNECTION_BUFFER_SIZE;
				zend_string_delref(text);
				if (zend_string_refcount(text) < 1) {
					zend_string_free(text);
				}
			}

			break;

		case LWS_CALLBACK_CLOSED:
		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
			wsconn = (ws_connection_obj *) Z_OBJ_P(connection);
			wsconn->connected = 0;
			printf("Close %lu.\n", wsconn->id);

			// Drop from active connections
			zend_hash_index_del(Z_ARRVAL(intern->connections), wsconn->id);
			zval_delref_p(connection);

			if (intern->callbacks[PHP_CB_SERVER_CLOSE]) {
				user_cb = intern->callbacks[PHP_CB_SERVER_CLOSE];
				ZVAL_NULL(&retval);
				zval params[2];
				ZVAL_COPY_VALUE(&params[0], intern->std);
				ZVAL_COPY_VALUE(&params[1], connection);
				user_cb->fci->param_count = 2;
				user_cb->fci->params = params;
				user_cb->fci->retval = &retval;
				if (SUCCESS != zend_call_function(user_cb->fci, user_cb->fcc TSRMLS_CC)) {
					php_error_docref(NULL, E_WARNING, "Unable to call close callback");
				}
				zval_dtor(&retval);
			}
			break;

		case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
		case LWS_CALLBACK_PROTOCOL_INIT:
		case LWS_CALLBACK_PROTOCOL_DESTROY:
		case LWS_CALLBACK_GET_THREAD_ID:
			// Ignore
			break;

		default:
			printf(" – CB PHP Non-handled action (reason: %d)\n", reason);
	}

	return intern->exit_request == 1 ? -1 : return_code;
}

struct _ws_protocol_storage {
	zval* php_obj;
	ws_server_obj *obj;
};

static struct lws_protocols protocols[] = {
	{
		"php_userspace",					/* Name */
		callback_ext_php,					/* Callback */
		sizeof(struct _ws_protocol_storage)	/* Per_session_data_size */
	},
	{ NULL, NULL, 0 }
};

zend_object_handlers phalcon_websocket_server_objectect_handler;
zend_object* phalcon_websocket_server_create_object_handler(zend_class_entry *ce)
{
	int i;
	phalcon_websocket_server_object *intern = emalloc(sizeof(phalcon_websocket_server_object));
	memset(intern, 0, sizeof(phalcon_websocket_server_object));

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_websocket_server_objectect_handlers;

	// Set LibWebsockets default options
	memset(&intern->info, 0, sizeof(intern->info));
	intern->info->_unused[0] = intern
	intern->info.uid = -1;
	intern->info.gid = -1;
	intern->info.ssl_private_key_filepath = intern->info.ssl_cert_filepath = NULL;	//FIXME HTTPS
	intern->info.protocols = protocols;
	intern->info.extensions = NULL;
	intern->info.options = 0;

	intern->exit_request = 0;
	for (i = 0; i < PHP_CB_SERVER_COUNT; ++i) {
		intern->callbacks[i] = NULL;
	}

	ZVAL_NULL(&intern->eventloop);
	ALLOC_HASHTABLE(intern->eventloop_sockets);

	intern->next_id = 0;
	array_init_size(&intern->connections, 20);

	return &intern->std;
}

void phalcon_websocket_server_free_object_storage_handler(phalcon_websocket_server_object *intern)
{
	int i;

	for (i = 0; i < PHP_CB_SERVER_COUNT; ++i) {
		if (NULL != intern->callbacks[i]) {
		    Z_DELREF(intern->callbacks[i]->fci->function_name);
			efree(intern->callbacks[i]->fci);
			efree(intern->callbacks[i]->fcc);
			efree(intern->callbacks[i]);
		}
	}


	if (intern->context) {
		lws_context_destroy(intern->context);
		intern->context ＝ NULL;
	}

	if (intern->info.user) {
		efree(intern->info.user);
		intern->info.user = NULL;
	}

	zval_dtor(&intern->eventloop);

	FREE_HASHTABLE(intern->eventloop_sockets);
	intern->eventloop_sockets = NULL;

	zval_dtor(&intern->connections);
	zend_object_std_dtor(&intern->std);
	efree(intern);
}

zend_bool phalcon_websocket_server_invoke_eventloop_cb(phalcon_websocket_server_object *intern, const char *func, php_socket_t sock, int flags)
{
	zval retval, el;
	zval php_flags;
	zval *php_sock;
	zval params[2];
	unsigned int param_count = 0;
	php_stream *stream;

	if (sock) {
		php_sock = zend_hash_index_find(intern->eventloop_sockets, sock);
		if (NULL == php_sock) {
			stream = php_stream_fopen_from_fd(sock, "rb", NULL);
			php_sock = emalloc(sizeof(zval));
			ZVAL_RES(php_sock, stream->res);
			Z_ADDREF_P(php_sock);
			zend_hash_index_add(intern->eventloop_sockets, sock, php_sock);
		}
		ZVAL_COPY_VALUE(&params[param_count++], php_sock);
	}
	if (flags >= 0) {
		ZVAL_LONG(&params[param_count++], flags);
	}
	ZVAL_NULL(&retval);
	zend_call_method(&intern->eventloop, Z_OBJCE_P(intern->eventloop), NULL, func, strlen(func), &retval, param_count, &params[0], &params[1]);

	if (zval_is_true(&retval)) {
		return 0;
	}
	return -1;
}

/**
 * Phalcon\Websocket\Server initializer
 */
PHALCON_INIT_CLASS(Phalcon_Websocket_Server){

	PHALCON_REGISTER_CLASS(Phalcon\\Websocket, Server, websocket_server, phalcon_websocket_server_method_entry, 0);

	phalcon_websocket_server_ce->create_object = phalcon_websocket_server_create_object_handler;
	memcpy(&phalcon_websocket_server_objectect_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	phalcon_websocket_server_objectect_handlers.free_obj = (zend_object_free_obj_t) phalcon_websocket_server_free_object_storage_handler;

	zend_declare_class_constant_long(phalcon_websocket_server_ce, SL("ON_ACCEPT"), PHP_CB_SERVER_ACCEPT);
	zend_declare_class_constant_long(phalcon_websocket_server_ce, SL("ON_CLOSE"), PHP_CB_SERVER_CLOSE);
	zend_declare_class_constant_long(phalcon_websocket_server_ce, SL("ON_DATA"), PHP_CB_SERVER_DATA);
	zend_declare_class_constant_long(phalcon_websocket_server_ce, SL("ON_TICK"), PHP_CB_SERVER_TICK);
	zend_declare_class_constant_long(phalcon_websocket_server_ce, SL("ON_FILTER_CONNECTION"), PHP_CB_SERVER_FILTER_CONNECTION);
	zend_declare_class_constant_long(phalcon_websocket_server_ce, SL("ON_FILTER_HEADERS"), PHP_CB_SERVER_FILTER_HEADERS);

	return SUCCESS;
}

/**
 * Phalcon\Websocket\Server constructor
 *
 * @param int $port
 */
PHP_METHOD(Phalcon_Websocket_Server, __construct)
{
	phalcon_websocket_server_object *intern;
	long port = 8080;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(port)
	ZEND_PARSE_PARAMETERS_END();

	intern = phalcon_websocket_server_object_from_obj(getThis());
	if (intern != NULL) {
		intern->info.port = port;
	}
}

/**
 * Set external event loop
 *
 * @param Phalcon\Websocket\Eventloop $eventloop
 */
PHP_METHOD(Phalcon_Websocket_Server, setEventLoop)
{
	zval *el;
	phalcon_websocket_server_object *intern;

	ZEND_PARSE_PARAMETERS_START(1, 1);
		Z_PARAM_OBJECT_OF_CLASS(el, phalcon_websocket_eventloopinterface_ce)
	ZEND_PARSE_PARAMETERS_END();

	intern = phalcon_websocket_server_object_from_obj(getThis());
	ZVAL_COPY(&intern->eventloop, el);
	zend_hash_init(intern->eventloop_sockets, 20, NULL, ZVAL_PTR_DTOR, 0);
}

/**
 * Service a socket (used with external event loop)
 */
PHP_METHOD(Phalcon_Websocket_Server, serviceFd)
{
	phalcon_websocket_server_object *intern;
	struct lws_pollfd pollfd;
	zval* res;
	php_stream *stream;
	php_socket_t fd;
	zend_long revents;

	ZEND_PARSE_PARAMETERS_START(2, 2);
		Z_PARAM_RESOURCE(res)
		Z_PARAM_LONG(revents);
	ZEND_PARSE_PARAMETERS_END();

	php_stream_from_zval(stream, res);
	if (FAILURE == php_stream_cast(stream, PHP_STREAM_AS_FD, (void **)&pollfd.fd, REPORT_ERRORS)) {
		// TODO Warning, error ?
		RETURN_FALSE;
	}

	pollfd.events = 1;
	pollfd.revents = revents;
	intern = phalcon_websocket_server_object_from_obj(getThis());
	lws_service_fd(WEBSOCKET_G(context), &pollfd);

	if (pollfd.revents) {
		RETURN_FALSE;
	} else {
		RETURN_TRUE;
	}
}

/**
 * Launch WebSocket server
 */
PHP_METHOD(Phalcon_Websocket_Server, run)
{
	phalcon_websocket_server_object *intern;
	ws_connection_obj *conn;
	struct lws *context;
	int n = 0;
	unsigned int ms = 0, oldMs = 0, tickInterval = 1000 / PHALCON_WEBSOCKET_FREQUENCY;
	int nextTick = 0;
	struct timeval tv;
	zend_string *text;

	// Start WebSocket
	intern = phalcon_websocket_server_object_from_obj(getThis());

	intern->context = lws_create_context(&intern->info);

	// If an external event loop is used, nothing more to do
	if (intern->eventloop) {
		return;
	}

	gettimeofday(&tv, NULL);
	oldMs = ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	while (n >= 0 && !intern->exit_request) {
		if (nextTick <= 0) {
			if (intern->callbacks[PHP_CB_SERVER_TICK]) {
				zval retval;
				ZVAL_NULL(&retval);
				zval params[1] = { *getThis() };
				intern->callbacks[PHP_CB_SERVER_TICK]->fci->param_count = 1;
				intern->callbacks[PHP_CB_SERVER_TICK]->fci->params = params;
				intern->callbacks[PHP_CB_SERVER_TICK]->fci->retval = &retval;
				if (SUCCESS != zend_call_function(intern->callbacks[PHP_CB_SERVER_TICK]->fci, intern->callbacks[PHP_CB_SERVER_TICK]->fcc TSRMLS_CC)) {
					php_error_docref(NULL, E_WARNING, "Unable to call tick callback");
				}
			}
			nextTick = tickInterval;
		}

		n = lws_service(intern->context, nextTick);

		gettimeofday(&tv, NULL);
		ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
		nextTick -= ms - oldMs;
		oldMs = ms;
	}

	// Disconnect users
	text = zend_string_init(ZEND_STRL("Server terminated"), 0);
	ZEND_HASH_FOREACH(Z_ARR(intern->connections), 0);
		conn = (ws_connection_obj *) Z_OBJ_P(_z);
		phalcon_websocket_connection_close(conn, text);
		zval_delref_p(_z);
		zend_hash_index_del(Z_ARR(intern->connections), _p->h);
	ZEND_HASH_FOREACH_END();

	lws_context_destroy(intern->context);
	intern->context ＝ NULL;
}

/**
 * Stop WebSocket server
 */
PHP_METHOD(Phalcon_Websocket_Server, stop)
{
	phalcon_websocket_server_object *intern;
	ws_connection_obj *conn;

	intern = phalcon_websocket_server_object_from_obj(getThis());

	// Mark as stopped
	intern->exit_request = 1;

	if (intern->eventloop) {
		// TODO In case of EventLoop, disconnect users here
	}
}

/**
 * Broadcast a message to all connected clients
 */
PHP_METHOD(Phalcon_Websocket_Server, broadcast)
{
	phalcon_websocket_server_object *intern;
	zend_string *str;
	ws_connection_obj *conn;
	long ignoredId = -1;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_STR(str)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(ignoredId)
	ZEND_PARSE_PARAMETERS_END();

	intern = phalcon_websocket_server_object_from_obj(getThis());
	ZEND_HASH_FOREACH(Z_ARR(intern->connections), 0);
		// TODO Test return? Interrupt if a write fail?
		conn = (ws_connection_obj *) Z_OBJ_P(_z);
		if (conn->id == ignoredId) {
			continue;
		}
		phalcon_websocket_connection_write(conn, str);
	ZEND_HASH_FOREACH_END();
	efree(str);
}

/**
 * Register a callback for specified event
 */
PHP_METHOD(Phalcon_Websocket_Server, on)
{
	phalcon_websocket_server_object *intern;
    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
	long event;

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_LONG(event)
		Z_PARAM_FUNC_EX(fci, fcc, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	if (event < 0 || event >= PHP_CB_SERVER_COUNT || !ZEND_FCI_INITIALIZED(fci)) {
		php_error_docref(NULL, E_WARNING, "Try to add an invalid callback");
		RETURN_FALSE;
	}

	intern = phalcon_websocket_server_object_from_obj(getThis());
	intern->callbacks[event] = emalloc(sizeof(ws_callback));
	intern->callbacks[event]->fci = emalloc(sizeof(zend_fcall_info));
	intern->callbacks[event]->fcc = emalloc(sizeof(zend_fcall_info_cache));

	memcpy(intern->callbacks[event]->fci, &fci, sizeof(zend_fcall_info));
	memcpy(intern->callbacks[event]->fcc, &fcc, sizeof(zend_fcall_info_cache));

	Z_ADDREF(intern->callbacks[event]->fci->function_name);

	RETURN_TRUE;
}
