
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

#include "websocket/client.h"
#include "websocket/connection.h"

#include <zend_smart_str.h>
#include <Zend/zend_closures.h>

#include "kernel/main.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/string.h"

/**
 * Phalcon\Websocket\Client
 *
 *<code>
 * $client = new Phalcon\Websocket\Client('127.0.0.1', 8080);
 * $client->on(Phalcon\Websocket\Client::ON_ACCEPT, function($client, $conn){
 *     echo 'Accept'.PHP_EOL;
 * });
 * $client->on(Phalcon\Websocket\Client::ON_CLOSE, function(){
 *     echo 'Close'.PHP_EOL;
 * });
 * $client->on(Phalcon\Websocket\Client::ON_DATA, function($client, $conn, $data){
 *     echo 'Data'.PHP_EOL;
 * });
 * $client->connect();
 *<／code>
 */
zend_class_entry *phalcon_websocket_client_ce;

PHP_METHOD(Phalcon_Websocket_Client, __construct);
PHP_METHOD(Phalcon_Websocket_Client, on);
PHP_METHOD(Phalcon_Websocket_Client, connect);
PHP_METHOD(Phalcon_Websocket_Client, send);
PHP_METHOD(Phalcon_Websocket_Client, sendJson);
PHP_METHOD(Phalcon_Websocket_Client, isConnected);
PHP_METHOD(Phalcon_Websocket_Client, disconnect);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_client___construct, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 1)
ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_client_on, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, event, IS_LONG, 0)
	ZEND_ARG_CALLABLE_INFO(0, callback, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_client_connect, 0, 0, 0)
	ZEND_ARG_CALLABLE_INFO(0, accept, 1)
	ZEND_ARG_CALLABLE_INFO(0, close, 1)
	ZEND_ARG_CALLABLE_INFO(0, data, 1)
	ZEND_ARG_CALLABLE_INFO(0, tick, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_client_send, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, writeProtocol, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_client_sendjson, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, payload, 0, 0)
	ZEND_ARG_TYPE_INFO(0, writeProtocol, IS_LONG, 1)
ZEND_END_ARG_INFO()

const zend_function_entry phalcon_websocket_client_method_entry[] = {
	PHP_ME(Phalcon_Websocket_Client, __construct, arginfo_phalcon_websocket_client___construct, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Client, on, arginfo_phalcon_websocket_client_on, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Client, connect, arginfo_phalcon_websocket_client_connect, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Client, send, arginfo_phalcon_websocket_client_send, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Client, sendJson, arginfo_phalcon_websocket_client_sendjson, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Client, isConnected, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Client, disconnect, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

static int phalcon_websocket_client_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	phalcon_websocket_client_object *intern = (phalcon_websocket_client_object*)lws_context_user(lws_get_context(wsi));
	phalcon_websocket_connection_object * connection_object;
	zval *connection = user;
	zval retval = {}, obj = {};
	int n, return_code = 0, flag = 0;
	unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 1024 + LWS_SEND_BUFFER_POST_PADDING];

	ZVAL_OBJ(&obj, &intern->std);

	switch (reason) {
		case LWS_CALLBACK_WSI_CREATE:
			lwsl_notice("WSI create\n");
			break;

		case LWS_CALLBACK_WSI_DESTROY:
			lwsl_notice("Destroy WSI\n");
			break;

		case LWS_CALLBACK_CLIENT_ESTABLISHED:
			lwsl_notice("Accept\n");
			object_init_ex(connection, phalcon_websocket_connection_ce);
			connection_object = phalcon_websocket_connection_object_from_obj(Z_OBJ_P(connection));
			connection_object->wsi = wsi;

			zval_ptr_dtor(&intern->connection);
			ZVAL_COPY_VALUE(&intern->connection, connection);

			connection_object->connected = 1;

			if (Z_TYPE(intern->callbacks[PHP_CB_CLIENT_ACCEPT]) != IS_NULL) {
				PHALCON_CALL_USER_FUNC_FLAG(flag, NULL, &intern->callbacks[PHP_CB_CLIENT_ACCEPT], &obj, connection);
				if (SUCCESS != flag) {
					php_error_docref(NULL, E_WARNING, "Unable to call accept callback");
				}
			}
			break;

		case LWS_CALLBACK_CLIENT_RECEIVE:
			lwsl_notice("Receive data: %d\n", len);
			if (Z_TYPE(intern->callbacks[PHP_CB_CLIENT_DATA]) != IS_NULL) {
				zval data = {};
				ZVAL_STRINGL(&data, in, len);
				PHALCON_CALL_USER_FUNC_FLAG(flag, &retval, &intern->callbacks[PHP_CB_CLIENT_DATA], &obj, connection, &data);
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

				zval_dtor(&retval);
			}
			break;

		case LWS_CALLBACK_CLIENT_WRITEABLE:
			lwsl_notice("Writeable\n");
			connection_object = phalcon_websocket_connection_object_from_obj(Z_OBJ_P(connection));
			if (!connection_object->connected) {
				return_code = -1;
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

			if (Z_TYPE(intern->callbacks[PHP_CB_CLIENT_CLOSE]) != IS_NULL) {
				PHALCON_CALL_USER_FUNC_FLAG(flag, NULL, &intern->callbacks[PHP_CB_CLIENT_CLOSE], &obj, connection);
				if (SUCCESS != flag) {
					php_error_docref(NULL, E_WARNING, "Unable to call close callback");
				}
			}
			intern->exit_request = 1;
			break;

		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
			lwsl_notice("Error\n");
			if (Z_TYPE(intern->callbacks[PHP_CB_CLIENT_ERROR]) != IS_NULL) {
				PHALCON_CALL_USER_FUNC_FLAG(flag, NULL, &intern->callbacks[PHP_CB_CLIENT_ERROR], &obj);
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
		phalcon_websocket_client_callback,
		sizeof(zval)
	},
	{ NULL, NULL, 0 }
};

zend_object_handlers phalcon_websocket_client_object_handlers;
zend_object* phalcon_websocket_client_object_create_handler(zend_class_entry *ce)
{
	int i;
	phalcon_websocket_client_object *intern = ecalloc(1, sizeof(phalcon_websocket_client_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_websocket_client_object_handlers;

	memset(&intern->info, 0, sizeof(intern->info));
	intern->info.uid = -1;
	intern->info.gid = -1;
	intern->info.port = CONTEXT_PORT_NO_LISTEN;
	intern->info.ssl_private_key_filepath = intern->info.ssl_cert_filepath = NULL;	//FIXME HTTPS
	intern->info.protocols = protocols;
	intern->info.extensions = NULL;
	intern->info.options = 0;
	intern->info.user = intern;

	intern->exit_request = 0;
	for (i = 0; i < PHP_CB_CLIENT_COUNT; ++i) {
		ZVAL_NULL(&intern->callbacks[i]);
	}

	return &intern->std;
}

void phalcon_websocket_client_object_free_handler(zend_object *object)
{
	phalcon_websocket_client_object *intern;
	int i;
	intern = phalcon_websocket_client_object_from_obj(object);

	for (i = 0; i < PHP_CB_CLIENT_COUNT; ++i) {
		if (Z_TYPE(intern->callbacks[i]) != IS_NULL) {
		    zval_ptr_dtor(&intern->callbacks[i]);
		}
	}

	if (intern->context) {
		lws_context_destroy(intern->context);
		intern->context = NULL;
	}

	zval_ptr_dtor(&intern->connection);

	zend_object_std_dtor(object);
}

/**
 * Phalcon\Websocket\Client initializer
 */
PHALCON_INIT_CLASS(Phalcon_Websocket_Client){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Websocket, Client, websocket_client, phalcon_websocket_client_method_entry, 0);

	zend_declare_property_null(phalcon_websocket_client_ce, SL("_host"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_websocket_client_ce, SL("_port"), 8080, ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_websocket_client_ce, SL("_path"), "/", ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_websocket_client_ce, SL("_writeProtocol"), LWS_WRITE_BINARY, ZEND_ACC_PROTECTED);

	zend_declare_class_constant_long(phalcon_websocket_client_ce, SL("ON_ACCEPT"), PHP_CB_CLIENT_ACCEPT);
	zend_declare_class_constant_long(phalcon_websocket_client_ce, SL("ON_CLOSE"), PHP_CB_CLIENT_CLOSE);
	zend_declare_class_constant_long(phalcon_websocket_client_ce, SL("ON_DATA"), PHP_CB_CLIENT_DATA);
	zend_declare_class_constant_long(phalcon_websocket_client_ce, SL("ON_TICK"), PHP_CB_CLIENT_TICK);

	zend_declare_class_constant_long(phalcon_websocket_client_ce, SL("WRITE_TEXT"), LWS_WRITE_TEXT);
	zend_declare_class_constant_long(phalcon_websocket_client_ce, SL("WRITE_BINARY"), LWS_WRITE_BINARY);
	zend_declare_class_constant_long(phalcon_websocket_client_ce, SL("WRITE_CONTINUATION"), LWS_WRITE_CONTINUATION);
	zend_declare_class_constant_long(phalcon_websocket_client_ce, SL("WRITE_HTTP"), LWS_WRITE_HTTP);
	//zend_declare_class_constant_long(phalcon_websocket_client_ce, SL("WRITE_CLOSE"), LWS_WRITE_CLOSE);
	zend_declare_class_constant_long(phalcon_websocket_client_ce, SL("WRITE_PING"), LWS_WRITE_PING);
	zend_declare_class_constant_long(phalcon_websocket_client_ce, SL("WRITE_PONG"), LWS_WRITE_PONG);
	zend_declare_class_constant_long(phalcon_websocket_client_ce, SL("WRITE_NO_FIN"), LWS_WRITE_NO_FIN);
	zend_declare_class_constant_long(phalcon_websocket_client_ce, SL("WRITE_CLIENT_IGNORE_XOR_MASK"), LWS_WRITE_CLIENT_IGNORE_XOR_MASK);
	return SUCCESS;
}

/**
 * Phalcon\Websocket\Client constructor
 *
 * @param string $host
 * @param int $port
 * @param string $path
 * @param int $writeProtocol
 */
PHP_METHOD(Phalcon_Websocket_Client, __construct)
{
	zval *host, *port = NULL, *path = NULL, *write_protocol = NULL;

	phalcon_fetch_params(0, 1, 3, &host, &port, &path, &write_protocol);

	phalcon_update_property(getThis(), SL("_host"), host);
	if (port && Z_TYPE_P(port) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_port"), port);
	}
	if (path && Z_TYPE_P(path) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_path"), path);
	}
	if (write_protocol && Z_TYPE_P(write_protocol) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_writeProtocol"), write_protocol);
	}
}

/**
 * Register a callback for specified event
 */
PHP_METHOD(Phalcon_Websocket_Client, on)
{
	zval *ev, *func, callback = {};
	phalcon_websocket_client_object *intern;
	long event;

	phalcon_fetch_params(0, 2, 0, &ev, &func);

	event = Z_LVAL_P(ev);
	if (event < 0 || event >= PHP_CB_CLIENT_COUNT) {
		php_error_docref(NULL, E_WARNING, "Try to add an invalid event callback");
		RETURN_FALSE;
	}

	intern = phalcon_websocket_client_object_from_obj(Z_OBJ_P(getThis()));

	if (Z_TYPE_P(func) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(func), zend_ce_closure, 0)) {
		PHALCON_CALL_CE_STATIC(&intern->callbacks[event], zend_ce_closure, "bind", func, getThis());
	} else {
		ZVAL_COPY(&intern->callbacks[event], func);
	}

	RETURN_THIS();
}

/**
 * Establish connection with remote server
 *
 * @param callable $onAccept
 * @param callable $onClose
 * @param callable $onData
 * @param callable $onTick
 */
PHP_METHOD(Phalcon_Websocket_Client, connect)
{
	zval *accept = NULL, *close = NULL, *data = NULL, *tick = NULL, event = {};
	zval host = {}, port = {}, path = {}, write_protocol = {};
	phalcon_websocket_client_object *intern;
	struct lws_client_connect_info info_ws;
	struct lws *wsi;
	char address[256];
	int n = 0;
	unsigned int ms = 0, oldMs = 0, tickInterval = 1000 / PHALCON_WEBSOCKET_FREQUENCY;
	int nextTick = 0;
	struct timeval tv;

	phalcon_fetch_params(0, 0, 4, &accept, &close, &data, &tick);

	if (accept) {
		ZVAL_LONG(&event, PHP_CB_CLIENT_ACCEPT);
		PHALCON_CALL_METHOD(NULL, getThis(), "on", &event, accept);
	}
	if (close) {
		ZVAL_LONG(&event, PHP_CB_CLIENT_CLOSE);
		PHALCON_CALL_METHOD(NULL, getThis(), "on", &event, close);
	}
	if (data) {
		ZVAL_LONG(&event, PHP_CB_CLIENT_DATA);
		PHALCON_CALL_METHOD(NULL, getThis(), "on", &event, data);
	}
	if (tick) {
		ZVAL_LONG(&event, PHP_CB_CLIENT_TICK);
		PHALCON_CALL_METHOD(NULL, getThis(), "on", &event, tick);
	}

	phalcon_read_property(&host, getThis(), SL("_host"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&port, getThis(), SL("_port"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&path, getThis(), SL("_path"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&write_protocol, getThis(), SL("_writeProtocol"), PH_NOISY|PH_READONLY);

	intern = phalcon_websocket_client_object_from_obj(Z_OBJ_P(getThis()));
	intern->context = lws_create_context(&intern->info);
	if (intern->context == NULL) {
        RETURN_FALSE;
    }
	intern->write_protocol = Z_LVAL(write_protocol);

	sprintf(address, "%s:%u", Z_STRVAL(host), Z_LVAL(port));

	/* create a client websocket */
	memset(&info_ws, 0, sizeof(info_ws));
	info_ws.port = Z_LVAL(port);
	info_ws.path = Z_STRVAL(path);
	info_ws.context = intern->context;
	info_ws.ssl_connection = 0;
	info_ws.address = Z_STRVAL(host);
	info_ws.host = address;
	info_ws.origin = address;
	info_ws.protocol = protocols[0].name;
	info_ws.ietf_version_or_minus_one = -1;
	info_ws.client_exts = NULL;
	wsi = lws_client_connect_via_info(&info_ws);
	if (wsi == NULL) {
		RETURN_FALSE;
	}

	gettimeofday(&tv, NULL);
	oldMs = ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	while (n >= 0 && !intern->exit_request) {
		if (nextTick <= 0) {
			if (Z_TYPE(intern->callbacks[PHP_CB_CLIENT_TICK]) != IS_NULL) {
				int flag = 0;
				PHALCON_CALL_USER_FUNC_FLAG(flag, NULL, &intern->callbacks[PHP_CB_CLIENT_TICK], getThis());
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

	if (Z_TYPE(intern->connection) == IS_OBJECT) {
		PHALCON_CALL_METHOD(NULL, &intern->connection, "disconnect");
	}

	lws_context_destroy(intern->context);
	intern->context = NULL;
}

/**
 * Send data to the client
 *
 * @param string $text
 * @param int $writeProtocol
 */
PHP_METHOD(Phalcon_Websocket_Client, send)
{
	zval *text, *write_protocol = NULL;
	phalcon_websocket_client_object *intern;

	phalcon_fetch_params(0, 1, 1, &text, &write_protocol);

	if (!write_protocol) {
		write_protocol = &PHALCON_GLOBAL(z_null);
	}

	intern = phalcon_websocket_client_object_from_obj(Z_OBJ_P(getThis()));
	if (Z_TYPE(intern->connection) == IS_OBJECT) {
		PHALCON_CALL_METHOD(return_value, &intern->connection, "send", text, write_protocol);
	}
}

/**
 * Send data to the client as JSON string
 *
 * @param mixed $text
 * @param int $writeProtocol
 */
PHP_METHOD(Phalcon_Websocket_Client, sendJson)
{
	zval *text, *write_protocol = NULL;
	phalcon_websocket_client_object *intern;

	phalcon_fetch_params(0, 1, 1, &text, &write_protocol);

	if (!write_protocol) {
		write_protocol = &PHALCON_GLOBAL(z_null);
	}

	intern = phalcon_websocket_client_object_from_obj(Z_OBJ_P(getThis()));
	if (Z_TYPE(intern->connection) == IS_OBJECT) {
		PHALCON_CALL_METHOD(return_value, &intern->connection, "sendjson", text, write_protocol);
	}
}

/**
 * Check is connection is established
 */
PHP_METHOD(Phalcon_Websocket_Client, isConnected)
{
	phalcon_websocket_client_object *intern;

	intern = phalcon_websocket_client_object_from_obj(Z_OBJ_P(getThis()));
	if (Z_TYPE(intern->connection) == IS_OBJECT) {
		PHALCON_CALL_METHOD(return_value, &intern->connection, "isconnected");
	}
}

/**
 * Close connection to the client
 */
PHP_METHOD(Phalcon_Websocket_Client, disconnect)
{
	phalcon_websocket_client_object *intern;

	intern = phalcon_websocket_client_object_from_obj(Z_OBJ_P(getThis()));
	intern->exit_request = 1;
}
