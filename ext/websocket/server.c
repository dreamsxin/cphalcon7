
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
#include <Zend/zend_closures.h>

#include "kernel/main.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/object.h"

/**
 * Phalcon\Websocket\Server
 *
 *<code>
 * $server = new Phalcon\Websocket\Server(8080);
 * $server->on(Phalcon\Websocket\Server::ON_ACCEPT, function($server, $conn){
 *     echo 'Accept'.PHP_EOL;
 * });
 * $server->on(Phalcon\Websocket\Server::ON_CLOSE, function($server){
 *     echo 'Close'.PHP_EOL;
 * });
 * $server->on(Phalcon\Websocket\Server::ON_DATA, function($server, $conn, $data){
 *     echo 'Data'.PHP_EOL;
 * });
 * $server->run();
 *<／code>
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
	ZEND_ARG_OBJ_INFO(0, eventloop, Phalcon\\Websocket\\EventloopInterface, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_server_servicefd, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, fd, IS_RESOURCE, 0)
	ZEND_ARG_TYPE_INFO(0, events, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_server_broadcast, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, ignored, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, writeProtocol, IS_LONG, 1)
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_phalcon_websocket_server_on, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, event, IS_LONG, 0)
	ZEND_ARG_CALLABLE_INFO(0, callback, 0)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_phalcon_websocket_server_on, 0, 2, _IS_BOOL, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, event, IS_LONG, 0)
	ZEND_ARG_CALLABLE_INFO(0, callback, 0)
ZEND_END_ARG_INFO()
#endif

static const zend_function_entry phalcon_websocket_server_method_entry[] = {
	PHP_ME(Phalcon_Websocket_Server, __construct, arginfo_phalcon_websocket_server___construct, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Server, setEventLoop, arginfo_phalcon_websocket_server_seteventloop, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Server, serviceFd, arginfo_phalcon_websocket_server_servicefd, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Server, run, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Server, stop, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Server, on, arginfo_phalcon_websocket_server_on, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Server, broadcast, arginfo_phalcon_websocket_server_broadcast, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

static int phalcon_websocket_server_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	phalcon_websocket_server_object *intern = (phalcon_websocket_server_object*)lws_context_user(lws_get_context(wsi));
	phalcon_websocket_connection_object *connection_object;
	zval *connection = user, retval = {}, obj = {};
	int n, return_code = 0, flag = 0;
	struct lws_pollargs *pa = in;
	unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 1024 + LWS_SEND_BUFFER_POST_PADDING];

	ZVAL_OBJ(&obj, &intern->std);

	switch (reason) {
		case LWS_CALLBACK_ADD_POLL_FD:
			if (zend_is_true(&intern->eventloop)) {
				return_code = phalcon_websocket_server_invoke_eventloop_cb(intern, "add", pa->fd, pa->events);
			}
			break;

		case LWS_CALLBACK_DEL_POLL_FD:
			if (zend_is_true(&intern->eventloop)) {
				return_code = phalcon_websocket_server_invoke_eventloop_cb(intern, "delete", pa->fd, -1);
			}
			break;

		case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
			if (zend_is_true(&intern->eventloop)) {
				return_code = phalcon_websocket_server_invoke_eventloop_cb(intern, "setmode", pa->fd, pa->events);
			}
			break;

		case LWS_CALLBACK_LOCK_POLL:
			if (zend_is_true(&intern->eventloop)) {
				return_code = phalcon_websocket_server_invoke_eventloop_cb(intern, "lock", 0, -1);
			}
			break;

		case LWS_CALLBACK_UNLOCK_POLL:
			if (zend_is_true(&intern->eventloop)) {
				return_code = phalcon_websocket_server_invoke_eventloop_cb(intern, "unlock", 0, -1);
			}
			break;

		case LWS_CALLBACK_WSI_CREATE:
			lwsl_notice("WSI create\n");
			break;

		case LWS_CALLBACK_WSI_DESTROY:
			lwsl_notice("Destroy WSI\n");
			break;

		case LWS_CALLBACK_ESTABLISHED:
			lwsl_notice("Accept\n");
			object_init_ex(connection, phalcon_websocket_connection_ce);
			connection_object = phalcon_websocket_connection_object_from_obj(Z_OBJ_P(connection));
			connection_object->id = ++intern->next_id;
			connection_object->wsi = wsi;

			add_index_zval(&intern->connections, connection_object->id, connection);

			connection_object->connected = 1;

			if (Z_TYPE(intern->callbacks[PHP_CB_SERVER_ACCEPT]) != IS_NULL) {
				PHALCON_CALL_USER_FUNC_FLAG(flag, NULL, &intern->callbacks[PHP_CB_SERVER_ACCEPT], &obj, connection);
				if (SUCCESS != flag) {
					php_error_docref(NULL, E_WARNING, "Unable to call accept callback");
				}
			}
			break;

		case LWS_CALLBACK_RECEIVE:
			lwsl_notice("Receive data %d.\n", len);
			if (Z_TYPE(intern->callbacks[PHP_CB_SERVER_DATA]) != IS_NULL) {
				zval data = {};
				ZVAL_STRINGL(&data, in, len);
				PHALCON_CALL_USER_FUNC_FLAG(flag, &retval, &intern->callbacks[PHP_CB_SERVER_DATA], &obj, connection, &data);
				zval_ptr_dtor(&data);
				if (SUCCESS != flag) {
					php_error_docref(NULL, E_WARNING, "Unable to call data callback");
				}

				// If return is different from true (or assimilated), close connection
				if (SUCCESS != flag || Z_TYPE(retval) == IS_FALSE) {
					connection_object = phalcon_websocket_connection_object_from_obj(Z_OBJ_P(connection));
					connection_object->connected = 0;
					return_code = -1;
				}

				zval_ptr_dtor(&retval);
			}
			break;

		case LWS_CALLBACK_SERVER_WRITEABLE:
			lwsl_notice("Writeable\n");
			connection_object = phalcon_websocket_connection_object_from_obj(Z_OBJ_P(connection));
			if (!connection_object->connected) {
				return_code = -1;
				lwsl_notice("Loss connected\n");
				break;
			}

			while (1) {
				zval ret = {}, item = {}, text = {}, type = {};
				int flag, write_protocol;
				int len = 0;
				PHALCON_CALL_METHOD_FLAG(flag, &ret, &connection_object->queue, "isempty");
				if (flag != SUCCESS || zend_is_true(&ret)) {
					break;
				}
				PHALCON_CALL_METHOD_FLAG(flag, &item, &connection_object->queue, "dequeue");
				if (flag != SUCCESS) {
					break;
				}
				phalcon_array_fetch_long(&text, &item, 0, PH_READONLY);
				phalcon_array_fetch_long(&type, &item, 1, PH_READONLY);
				if (unlikely(Z_TYPE(type) == IS_LONG)) {
					write_protocol = Z_LVAL(type);
				} else {
					write_protocol = intern->write_protocol;
				}
				len += snprintf((char*)&buf[LWS_SEND_BUFFER_PRE_PADDING], 1024, "%s", (unsigned char *)Z_STRVAL(text));
				n = lws_write(wsi, &buf[LWS_SEND_BUFFER_PRE_PADDING], len, write_protocol);

				if (n < 0) {
					zval_ptr_dtor(&item);
					lwsl_err("Write to socket %lu failed with code %d\n", connection_object->id, n);
					return 1;
				}
				lwsl_notice("Write bytes %d\n", n);
				if (n < len) {
					// TODO Implements partial write
					zval_ptr_dtor(&item);
					lwsl_err("Partial write\n");
					return -1;
				}

				// Cleanup
				zval_ptr_dtor(&item);
			}

			break;

		case LWS_CALLBACK_CLOSED:
			lwsl_notice("Close\n");
			connection_object = phalcon_websocket_connection_object_from_obj(Z_OBJ_P(connection));
			connection_object->connected = 0;

			// Drop from active connections
			zend_hash_index_del(Z_ARRVAL(intern->connections), connection_object->id);

			if (Z_TYPE(intern->callbacks[PHP_CB_SERVER_CLOSE]) == IS_CALLABLE) {
				PHALCON_CALL_USER_FUNC_FLAG(flag, NULL, &intern->callbacks[PHP_CB_SERVER_CLOSE], &obj, connection);
				if (SUCCESS != flag) {
					php_error_docref(NULL, E_WARNING, "Unable to call close callback");
				}
			}
			Z_TRY_DELREF_P(connection);
			break;

		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
			lwsl_notice("Error\n");
			if (Z_TYPE(intern->callbacks[PHP_CB_SERVER_ERROR]) == IS_CALLABLE) {
				PHALCON_CALL_USER_FUNC_FLAG(flag, NULL, &intern->callbacks[PHP_CB_SERVER_ERROR], &obj);
				if (SUCCESS != flag) {
					php_error_docref(NULL, E_WARNING, "Unable to call error callback");
				}
			}
			intern->exit_request = -1;
			break;

		default:
			break;
	}

	return intern->exit_request == 1 ? -1 : return_code;
}

static struct lws_protocols protocols[] = {
	{
		"phalcon",
		phalcon_websocket_server_callback,
		sizeof(zval)
	},
	{ NULL, NULL, 0 }
};

zend_object_handlers phalcon_websocket_server_object_handlers;
zend_object* phalcon_websocket_server_object_create_handler(zend_class_entry *ce)
{
	int i;
	phalcon_websocket_server_object *intern = ecalloc(1, sizeof(phalcon_websocket_server_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_websocket_server_object_handlers;

	// Set LibWebsockets default options
	memset(&intern->info, 0, sizeof(struct lws_context_creation_info));
	intern->info.uid = -1;
	intern->info.gid = -1;
	intern->info.ssl_private_key_filepath = intern->info.ssl_cert_filepath = NULL;	//FIXME HTTPS
	intern->info.protocols = protocols;
	intern->info.extensions = NULL;
	intern->info.options = 0;
	intern->info.user = intern;

	intern->exit_request = 0;
	for (i = 0; i < PHP_CB_SERVER_COUNT; ++i) {
		ZVAL_NULL(&intern->callbacks[i]);
	}

	ZVAL_NULL(&intern->eventloop);
	ALLOC_HASHTABLE(intern->eventloop_sockets);

	intern->next_id = 0;
	array_init(&intern->connections);

	return &intern->std;
}

void phalcon_websocket_server_object_free_handler(zend_object *object)
{
	phalcon_websocket_server_object *intern;
	int i;
	intern = phalcon_websocket_server_object_from_obj(object);

	for (i = 0; i < PHP_CB_SERVER_COUNT; ++i) {
		if (Z_TYPE(intern->callbacks[i]) != IS_NULL) {
		    zval_ptr_dtor(&intern->callbacks[i]);
		}
	}

	if (intern->context) {
		lws_context_destroy(intern->context);
		intern->context = NULL;
	}

	if (intern->info.user) {
		efree(intern->info.user);
		intern->info.user = NULL;
	}

	zval_ptr_dtor(&intern->eventloop);

	FREE_HASHTABLE(intern->eventloop_sockets);
	intern->eventloop_sockets = NULL;

	zval_ptr_dtor(&intern->connections);
	zend_object_std_dtor(object);
}

zend_bool phalcon_websocket_server_invoke_eventloop_cb(phalcon_websocket_server_object *intern, const char *func, php_socket_t sock, int flags)
{
	zval retval, *php_sock, params[2];
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
		ZVAL_COPY(&params[param_count++], php_sock);
	}
	if (flags >= 0) {
		ZVAL_LONG(&params[param_count++], flags);
	}
	ZVAL_NULL(&retval);
	zend_call_method(&intern->eventloop, Z_OBJCE(intern->eventloop), NULL, func, strlen(func), &retval, param_count, &params[0], &params[1]);

	if (zval_is_true(&retval)) {
		return 0;
	}
	return -1;
}

/**
 * Phalcon\Websocket\Server initializer
 */
PHALCON_INIT_CLASS(Phalcon_Websocket_Server){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Websocket, Server, websocket_server, phalcon_websocket_server_method_entry, 0);

	zend_declare_property_long(phalcon_websocket_server_ce, SL("_port"), 8080, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_websocket_server_ce, SL("_writeProtocol"), LWS_WRITE_BINARY, ZEND_ACC_PROTECTED);

	zend_declare_class_constant_long(phalcon_websocket_server_ce, SL("ON_ACCEPT"), PHP_CB_SERVER_ACCEPT);
	zend_declare_class_constant_long(phalcon_websocket_server_ce, SL("ON_CLOSE"), PHP_CB_SERVER_CLOSE);
	zend_declare_class_constant_long(phalcon_websocket_server_ce, SL("ON_DATA"), PHP_CB_SERVER_DATA);
	zend_declare_class_constant_long(phalcon_websocket_server_ce, SL("ON_TICK"), PHP_CB_SERVER_TICK);

	zend_declare_class_constant_long(phalcon_websocket_server_ce, SL("WRITE_TEXT"), LWS_WRITE_TEXT);
	zend_declare_class_constant_long(phalcon_websocket_server_ce, SL("WRITE_BINARY"), LWS_WRITE_BINARY);
	zend_declare_class_constant_long(phalcon_websocket_server_ce, SL("WRITE_CONTINUATION"), LWS_WRITE_CONTINUATION);
	zend_declare_class_constant_long(phalcon_websocket_server_ce, SL("WRITE_HTTP"), LWS_WRITE_HTTP);
	//zend_declare_class_constant_long(phalcon_websocket_server_ce, SL("WRITE_CLOSE"), LWS_WRITE_CLOSE);
	zend_declare_class_constant_long(phalcon_websocket_server_ce, SL("WRITE_PING"), LWS_WRITE_PING);
	zend_declare_class_constant_long(phalcon_websocket_server_ce, SL("WRITE_PONG"), LWS_WRITE_PONG);
	zend_declare_class_constant_long(phalcon_websocket_server_ce, SL("WRITE_NO_FIN"), LWS_WRITE_NO_FIN);
	zend_declare_class_constant_long(phalcon_websocket_server_ce, SL("WRITE_CLIENT_IGNORE_XOR_MASK"), LWS_WRITE_CLIENT_IGNORE_XOR_MASK);
	return SUCCESS;
}

/**
 * Phalcon\Websocket\Server constructor
 *
 * @param int $port
 * @param int $writeProtocol
 */
PHP_METHOD(Phalcon_Websocket_Server, __construct)
{
	zval *_port = NULL, *_write_protocol = NULL, port = {}, write_protocol = {};
	phalcon_websocket_server_object *intern;

	phalcon_fetch_params(0, 0, 2, &port, &write_protocol);

	if (_port && Z_TYPE_P(_port) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_port"), _port);
	}
	if (_write_protocol && Z_TYPE_P(_write_protocol) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_writeProtocol"), _write_protocol);
	}
	phalcon_read_property(&port, getThis(), SL("_port"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&write_protocol, getThis(), SL("_writeProtocol"), PH_NOISY|PH_READONLY);

	intern = phalcon_websocket_server_object_from_obj(Z_OBJ_P(getThis()));
	if (intern != NULL) {
		intern->info.port = Z_LVAL(port);
		intern->write_protocol = Z_LVAL(write_protocol);
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

	intern = phalcon_websocket_server_object_from_obj(Z_OBJ_P(getThis()));
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
	intern = phalcon_websocket_server_object_from_obj(Z_OBJ_P(getThis()));
	lws_service_fd(intern->context, &pollfd);

	if (pollfd.revents) {
		RETURN_FALSE;
	} else {
		RETURN_TRUE;
	}
}

/**
 * Launch WebSocket server
 *
 * @param callable $onAccept
 * @param callable $onClose
 * @param callable $onData
 * @param callable $onTick
 */
PHP_METHOD(Phalcon_Websocket_Server, run)
{
	zval *accept = NULL, *close = NULL, *data = NULL, *tick = NULL, event = {};
	phalcon_websocket_server_object *intern;
	phalcon_websocket_connection_object *conn;
	int n = 0;
	unsigned int ms = 0, oldMs = 0, tickInterval = 1000 / PHALCON_WEBSOCKET_FREQUENCY;
	int nextTick = 0;
	struct timeval tv;
	zend_string *text;

	phalcon_fetch_params(0, 0, 4, &accept, &close, &data, &tick);

	if (accept) {
		ZVAL_LONG(&event, PHP_CB_SERVER_ACCEPT);
		PHALCON_CALL_METHOD(NULL, getThis(), "on", &event, accept);
	}
	if (close) {
		ZVAL_LONG(&event, PHP_CB_SERVER_CLOSE);
		PHALCON_CALL_METHOD(NULL, getThis(), "on", &event, close);
	}
	if (data) {
		ZVAL_LONG(&event, PHP_CB_SERVER_DATA);
		PHALCON_CALL_METHOD(NULL, getThis(), "on", &event, data);
	}
	if (tick) {
		ZVAL_LONG(&event, PHP_CB_SERVER_TICK);
		PHALCON_CALL_METHOD(NULL, getThis(), "on", &event, tick);
	}

	// Start WebSocket
	intern = phalcon_websocket_server_object_from_obj(Z_OBJ_P(getThis()));
	intern->context = lws_create_context(&intern->info);
	if (intern->context == NULL) {
        RETURN_FALSE;
    }

	// If an external event loop is used, nothing more to do
	if (zend_is_true(&intern->eventloop)) {
		return;
	}

	gettimeofday(&tv, NULL);
	oldMs = ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	while (n >= 0 && !intern->exit_request) {
		if (nextTick <= 0) {
			if (Z_TYPE(intern->callbacks[PHP_CB_SERVER_TICK]) != IS_NULL) {
				int flag = 0;
				PHALCON_CALL_USER_FUNC_FLAG(flag, NULL, &intern->callbacks[PHP_CB_SERVER_TICK], getThis());
				if (SUCCESS != flag) {
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
		conn = (phalcon_websocket_connection_object *) Z_OBJ_P(_z);
		phalcon_websocket_connection_close(conn, text);
		zval_delref_p(_z);
		zend_hash_index_del(Z_ARR(intern->connections), _p->h);
	ZEND_HASH_FOREACH_END();

	lws_context_destroy(intern->context);
	intern->context = NULL;
}

/**
 * Stop WebSocket server
 */
PHP_METHOD(Phalcon_Websocket_Server, stop)
{
	phalcon_websocket_server_object *intern;

	intern = phalcon_websocket_server_object_from_obj(Z_OBJ_P(getThis()));

	// Mark as stopped
	intern->exit_request = 1;
}

/**
 * Broadcast a message to all connected clients
 *
 * @param string $text
 * @param array $ignored
 * @param int $writeProtocol
 */
PHP_METHOD(Phalcon_Websocket_Server, broadcast)
{
	zval *text, *ignored = NULL, *write_protocol = NULL, *connection;
	phalcon_websocket_server_object *intern;

	phalcon_fetch_params(0, 1, 2, &text, &ignored, &write_protocol);

	if (!ignored) {
		ignored = &PHALCON_GLOBAL(z_null);
	}

	if (!write_protocol) {
		write_protocol = &PHALCON_GLOBAL(z_null);
	}

	intern = phalcon_websocket_server_object_from_obj(Z_OBJ_P(getThis()));
	ZEND_HASH_FOREACH_VAL(Z_ARR(intern->connections), connection);
		zval id = {};
		phalcon_websocket_connection_object *intern = phalcon_websocket_connection_object_from_obj(Z_OBJ_P(connection));
		// TODO Test return? Interrupt if a write fail?
		ZVAL_LONG(&id, intern->id);
		if (Z_TYPE_P(ignored) == IS_ARRAY && phalcon_fast_in_array(&id, ignored)) {
			continue;
		}
		phalcon_websocket_connection_write(intern, text, write_protocol);
	ZEND_HASH_FOREACH_END();
}

/**
 * Register a callback for specified event
 */
PHP_METHOD(Phalcon_Websocket_Server, on)
{
	zval *ev, *func, callback = {};
	phalcon_websocket_server_object *intern;
	long event;

	phalcon_fetch_params(0, 2, 0, &ev, &func);

	event = Z_LVAL_P(ev);
	if (event < 0 || event >= PHP_CB_SERVER_COUNT) {
		php_error_docref(NULL, E_WARNING, "Try to add an invalid event callback");
		RETURN_FALSE;
	}

	intern = phalcon_websocket_server_object_from_obj(Z_OBJ_P(getThis()));

	if (Z_TYPE_P(func) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(func), zend_ce_closure, 0)) {
		PHALCON_CALL_CE_STATIC(&intern->callbacks[event], zend_ce_closure, "bind", func, getThis());
	} else {
		ZVAL_COPY(&intern->callbacks[event], func);
	}
	RETURN_TRUE;
}
