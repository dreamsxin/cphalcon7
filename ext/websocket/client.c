
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
 * $client->on(Phalcon\Websocket\Client::ON_DATA, function($client, $conn){
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

#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_phalcon_websocket_client_on, 0, 2, IS_TRUE|IS_FALSE, 0)
	ZEND_ARG_TYPE_INFO(0, event, IS_LONG, 0)
	ZEND_ARG_CALLABLE_INFO(0, callback, 0)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_phalcon_websocket_client_on, 0, 2, IS_TRUE|IS_FALSE, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, event, IS_LONG, 0)
	ZEND_ARG_CALLABLE_INFO(0, callback, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_client_connect, 0, 0, 0)
	ZEND_ARG_CALLABLE_INFO(0, accept, 1)
	ZEND_ARG_CALLABLE_INFO(0, close, 1)
	ZEND_ARG_CALLABLE_INFO(0, data, 1)
	ZEND_ARG_CALLABLE_INFO(0, tick, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_client_send, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_client_sendjson, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, payload, 0, 0)
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
	zend_string *text;

	ZVAL_OBJ(&obj, &intern->std);

	switch (reason) {
		case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
		case LWS_CALLBACK_LOCK_POLL:
		case LWS_CALLBACK_UNLOCK_POLL:
			break;
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

			Z_TRY_DELREF(intern->connection);
			ZVAL_COPY(&intern->connection, connection);

			connection_object->connected = 1;

			if (Z_TYPE(intern->callbacks[PHP_CB_CLIENT_ACCEPT]) != IS_NULL) {
				PHALCON_CALL_USER_FUNC_FLAG(flag, NULL, &intern->callbacks[PHP_CB_CLIENT_ACCEPT], &obj, connection);
				if (SUCCESS != flag) {
					php_error_docref(NULL, E_WARNING, "Unable to call accept callback");
				}
			}
			break;

		case LWS_CALLBACK_CLIENT_RECEIVE:
			lwsl_notice("Receive data\n");
			if (Z_TYPE(intern->callbacks[PHP_CB_CLIENT_DATA]) != IS_NULL) {
				zval data = {};
				ZVAL_STRINGL(&data, in, len);

				PHALCON_CALL_USER_FUNC_FLAG(flag, NULL, &intern->callbacks[PHP_CB_CLIENT_DATA], &obj, connection, &data);
				if (SUCCESS != flag) {
					php_error_docref(NULL, E_WARNING, "Unable to call data callback");
				}

				// If return is different from true (or assimilated), close connection
				if (!zval_is_true(&retval)) {
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

			while (connection_object->read_ptr != connection_object->write_ptr) {
				text = connection_object->buf[connection_object->read_ptr];
				n = lws_write(wsi, (unsigned char *)ZSTR_VAL(text), ZSTR_LEN(text), LWS_WRITE_TEXT);

				if (n < 0) {
					lwsl_err("Write to socket %lu failed with code %d\n", connection_object->id, n);
					return 1;
				}
				if (n < (int) text->len) {
					// TODO Implements partial write
					connection_object->buf[connection_object->read_ptr] = NULL;
					connection_object->read_ptr = (connection_object->read_ptr + 1) % PHALCON_WEBSOCKET_CONNECTION_BUFFER_SIZE;
					lwsl_err("Partial write\n");
					return -1;
				}

				connection_object->buf[connection_object->read_ptr] = NULL;
				connection_object->read_ptr = (connection_object->read_ptr + 1) % PHALCON_WEBSOCKET_CONNECTION_BUFFER_SIZE;
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
		case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH:
		case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
		case LWS_CALLBACK_PROTOCOL_INIT:
		case LWS_CALLBACK_PROTOCOL_DESTROY:
		case LWS_CALLBACK_GET_THREAD_ID:
		case LWS_CALLBACK_HTTP_DROP_PROTOCOL:
			break;
		default:
			lwsl_notice(" – Callback Non-handled action (reason: %d)\n", reason);
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
		    Z_TRY_DELREF(intern->callbacks[i]);
		}
	}

	if (intern->context) {
		lws_context_destroy(intern->context);
		intern->context = NULL;
	}

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

	zend_declare_class_constant_long(phalcon_websocket_client_ce, SL("ON_ACCEPT"), PHP_CB_CLIENT_ACCEPT);
	zend_declare_class_constant_long(phalcon_websocket_client_ce, SL("ON_CLOSE"), PHP_CB_CLIENT_CLOSE);
	zend_declare_class_constant_long(phalcon_websocket_client_ce, SL("ON_DATA"), PHP_CB_CLIENT_DATA);
	zend_declare_class_constant_long(phalcon_websocket_client_ce, SL("ON_TICK"), PHP_CB_CLIENT_TICK);

	return SUCCESS;
}

/**
 * Phalcon\Websocket\Client constructor
 *
 * @param string $host
 * @param int $port
 */
PHP_METHOD(Phalcon_Websocket_Client, __construct)
{
	zval *host, *port = NULL, *path = NULL;

	phalcon_fetch_params(0, 1, 2, &host, &port, &path);

	phalcon_update_property_zval(getThis(), SL("_host"), host);
	if (port) {
		phalcon_update_property_zval(getThis(), SL("_port"), port);
	}
	if (path) {
		phalcon_update_property_zval(getThis(), SL("_path"), path);
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

	if (Z_TYPE_P(func) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(func), zend_ce_closure, 0)) {
			PHALCON_CALL_CE_STATIC(&callback, zend_ce_closure, "bind", func, getThis());
	} else {
		PHALCON_CPY_WRT(&callback, func);
	}

	event = Z_LVAL_P(ev);
	if (event < 0 || event >= PHP_CB_CLIENT_COUNT) {
		php_error_docref(NULL, E_WARNING, "Try to add an invalid event callback");
		RETURN_FALSE;
	}

	intern = phalcon_websocket_client_object_from_obj(Z_OBJ_P(getThis()));
	ZVAL_COPY(&intern->callbacks[event], &callback);

	RETURN_TRUE;
}

/**
 * Establish connection with remote server
 */
PHP_METHOD(Phalcon_Websocket_Client, connect)
{
	zval *accept = NULL, *close = NULL, *data = NULL, *tick = NULL, event = {};
	zval host = {}, port = {}, path = {};
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

	phalcon_read_property(&host, getThis(), SL("_host"), PH_NOISY);
	phalcon_read_property(&port, getThis(), SL("_port"), PH_NOISY);
	phalcon_read_property(&path, getThis(), SL("_path"), PH_NOISY);

	intern = phalcon_websocket_client_object_from_obj(Z_OBJ_P(getThis()));
	intern->context = lws_create_context(&intern->info);
	if (intern->context == NULL) {
        RETURN_FALSE;
    }

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
 */
PHP_METHOD(Phalcon_Websocket_Client, send)
{
	zval *text;
	phalcon_websocket_client_object *intern;

	phalcon_fetch_params(0, 1, 0, &text);

	intern = phalcon_websocket_client_object_from_obj(Z_OBJ_P(getThis()));
	PHALCON_CALL_METHOD(return_value, &intern->connection, "send", text);
}

/**
 * Send data to the client as JSON string
 */
PHP_METHOD(Phalcon_Websocket_Client, sendJson)
{
	zval *val, text = {};
	phalcon_websocket_client_object *intern;

	phalcon_fetch_params(0, 1, 0, &val);

	intern = phalcon_websocket_client_object_from_obj(Z_OBJ_P(getThis()));

	RETURN_ON_FAILURE(phalcon_json_encode(&text, val, 0));

	PHALCON_CALL_METHOD(return_value, &intern->connection, "sendjson", &text);
}

/**
 * Check is connection is established
 */
PHP_METHOD(Phalcon_Websocket_Client, isConnected)
{
	phalcon_websocket_client_object *intern;

	intern = phalcon_websocket_client_object_from_obj(Z_OBJ_P(getThis()));

	PHALCON_CALL_METHOD(return_value, &intern->connection, "isconnected");
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
