
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

#include "kernel/string.h"

/**
 * Phalcon\Websocket\Client
 *
 */
zend_class_entry *phalcon_websocket_client_ce;

PHP_METHOD(Phalcon_Websocket_Client, __construct);
PHP_METHOD(Phalcon_Websocket_Client, on);
PHP_METHOD(Phalcon_Websocket_Client, connect);
PHP_METHOD(Phalcon_Websocket_Client, send);
PHP_METHOD(Phalcon_Websocket_Client, sendAsJson);
PHP_METHOD(Phalcon_Websocket_Client, isConnected);
PHP_METHOD(Phalcon_Websocket_Client, disconnect);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_client___construct, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 1)
ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_phalcon_websocket_client_on, 0, 2, IS_TRUE|IS_FALSE, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, event, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 1)
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
	PHP_ME(Phalcon_Websocket_Client, connect, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Client, send, arginfo_phalcon_websocket_client_send, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Client, sendJson, arginfo_phalcon_websocket_client_sendjson, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Client, isConnected, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Client, disconnect, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

static struct lws_protocols protocols[] = {
	{
		"php_userspace",					/* Name */
		callback_ext_php,					/* Callback */
		sizeof(zval)
	},
	{ NULL, NULL, 0 }
};



static int callback_ext_php(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	phalcon_websocket_server_object *intern = phalcon_websocket_server_object_from_ctx(wsi->context);
	phalcon_websocket_connection_object * wsconn;
	zval *connection = user;
	zval retval;
	int n, return_code = 0;
	zend_string *text;
	struct lws_pollargs *pa = in;
	ws_callback *user_cb;

	switch (reason) {
		case LWS_CALLBACK_WSI_CREATE:
			printf("WSI create\n");
			break;

		case LWS_CALLBACK_WSI_DESTROY:
			printf("Destroy WSI\n");
			break;

		case LWS_CALLBACK_CLIENT_ESTABLISHED:
			object_init_ex(connection, phalcon_websocket_connection_ce);
			wsconn = phalcon_websocket_connection_object_from_obj(connection);
			printf("Accept connection %lu\n", wsconn->id);
			wsconn->wsi = wsi;

			zval_addref_p(connection);
			ZVAL_COPY_VALUE(&intern->connection, connection);

			wsconn->connected = 1;

			if (intern->callbacks[PHP_CB_CLIENT_ACCEPT]) {
				user_cb = intern->callbacks[PHP_CB_CLIENT_ACCEPT];
				ZVAL_NULL(&retval);
				zval params[1];
				ZVAL_OBJ(&params[0], &intern->std);
				user_cb->fci->param_count = 1;
				user_cb->fci->params = params;
				user_cb->fci->retval = &retval;
				user_cb->fci->no_separation = (zend_bool) 0;

				printf("Ready to call accept handler\n");
				int res = zend_call_function(user_cb->fci, user_cb->fcc);
				if (SUCCESS != res) {
					php_error_docref(NULL, E_WARNING, "Unable to call accept callback");
				}
				zval_dtor(&retval);
			}
			break;

		case LWS_CALLBACK_CLIENT_RECEIVE:
			printf("Receive data.\n");
			if (intern->callbacks[PHP_CB_CLIENT_DATA]) {
				user_cb = intern->callbacks[PHP_CB_CLIENT_DATA];
				ZVAL_NULL(&retval);
				zval params[2];
				ZVAL_OBJ(&params[0], &intern->std);
				ZVAL_STRINGL(&params[1], in, len);
				user_cb->fci->param_count = 2;
				user_cb->fci->params = params;
				user_cb->fci->retval = &retval;
				if (SUCCESS != zend_call_function(user_cb->fci, user_cb->fcc)) {
					php_error_docref(NULL, E_WARNING, "Unable to call data callback");
				}

				// Free string if needed
				zval_delref_p(&params[1]);
				if (zval_refcount_p(&params[1]) < 1) {
					zend_string_free(Z_STR(params[1]));
				}

				// If return is different from true (or assimilated), close connection
				if (!zval_is_true(&retval)) {
					wsconn = phalcon_websocket_connection_object_from_obj(connection);
					wsconn->connected = 0;
					return_code = -1;
				}

				zval_dtor(&retval);
			}
			break;

		case LWS_CALLBACK_CLIENT_WRITEABLE:
			wsconn = phalcon_websocket_connection_object_from_obj(connection);
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
			wsconn = phalcon_websocket_connection_object_from_obj(connection);
			wsconn->connected = 0;
			printf("Close %lu.\n", wsconn->id);

			zval_delref_p(connection);
			ZVAL_NULL(intern->connection);

			if (intern->callbacks[PHP_CB_CLIENT_CLOSE]) {
				user_cb = intern->callbacks[PHP_CB_CLIENT_CLOSE];
				ZVAL_NULL(&retval);
				zval params[1];
				ZVAL_OBJ(&params[0], &intern->std);
				user_cb->fci->param_count = 1;
				user_cb->fci->params = params;
				user_cb->fci->retval = &retval;
				if (SUCCESS != zend_call_function(user_cb->fci, user_cb->fcc)) {
					php_error_docref(NULL, E_WARNING, "Unable to call close callback");
				}
				zval_dtor(&retval);
			}
			break;

		default:
			printf(" – CB PHP Non-handled action (reason: %d)\n", reason);
	}

	return intern->exit_request == 1 ? -1 : return_code;
}

zend_object_handlers phalcon_websocket_client_object_handlers;
zend_object* phalcon_websocket_client_create_object_handler(zend_class_entry *ce)
{
	int i;
	phalcon_websocket_client_object *intern = emalloc(sizeof(phalcon_websocket_client_object));
	memset(intern, 0, sizeof(phalcon_websocket_client_object));

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

	return &intern->std;
}

void phalcon_websocket_client_free_object_storage_handler(phalcon_websocket_client_object *intern)
{
	int i;

	for (i = 0; i < PHP_CB_CLIENT_COUNT; ++i) {
		if (NULL != intern->callbacks[i]) {
		    Z_DELREF(intern->callbacks[i]->fci->function_name);
			efree(intern->callbacks[i]->fci);
			efree(intern->callbacks[i]->fcc);
			efree(intern->callbacks[i]);
		}
	}

	zend_object_std_dtor(&intern->std);
	efree(intern);
}

/**
 * Phalcon\Websocket\Connection initializer
 */
PHALCON_INIT_CLASS(Phalcon_Websocket_Connection){

	PHALCON_REGISTER_CLASS(Phalcon\\Websocket, Server, websocket_client, phalcon_websocket_client_method_entry, 0);

	phalcon_websocket_client_ce->create_object = phalcon_websocket_client_create_object_handler;
	memcpy(&phalcon_websocket_client_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	phalcon_websocket_client_object_handlers.free_obj = (zend_object_free_obj_t) phalcon_websocket_client_free_object_storage_handler;

	zend_declare_property_null(phalcon_websocket_client_ce, SL("_host"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_websocket_client_ce, SL("_port"), 8080, ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_websocket_client_ce, SL("_path"), "/", ZEND_ACC_PROTECTED);
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

	phalcon_fetch_params(0, 1, 1, &host, &port, &path);

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
	phalcon_websocket_client_object *intern;
    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
	long event;

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_LONG(event)
		Z_PARAM_FUNC_EX(fci, fcc, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	if (event < 0 || event >= PHP_CB_CLIENT_COUNT || !ZEND_FCI_INITIALIZED(fci)) {
		php_error_docref(NULL, E_WARNING, "Try to add an invalid callback");
		RETURN_FALSE;
	}

	intern = phalcon_websocket_connection_object_from_obj(getThis());
	intern->callbacks[event] = emalloc(sizeof(ws_callback));
	intern->callbacks[event]->fci = emalloc(sizeof(zend_fcall_info));
	intern->callbacks[event]->fcc = emalloc(sizeof(zend_fcall_info_cache));

	memcpy(intern->callbacks[event]->fci, &fci, sizeof(zend_fcall_info));
	memcpy(intern->callbacks[event]->fcc, &fcc, sizeof(zend_fcall_info_cache));

    Z_ADDREF(intern->callbacks[event]->fci->function_name);

	RETURN_TRUE;
}

/**
 * Establish connection with remote server
 */
PHP_METHOD(Phalcon_Websocket_Client, connect)
{
	zval host = {}, port = {}, path = {};
	phalcon_websocket_client_object *intern;
	struct lws *wsi;
	char address[256], path[256];
	int n = 0;
	unsigned int ms = 0, oldMs = 0, tickInterval = 1000 / PHALCON_WEBSOCKET_FREQUENCY;
	int nextTick = 0;
	struct timeval tv;
	zend_string *text;

	phalcon_read_property(&host, getThis(), SL("_host"), PH_NOISY);
	phalcon_read_property(&port, getThis(), SL("_port"), PH_NOISY);
	phalcon_read_property(&path, getThis(), SL("_path"), PH_NOISY);

	intern = phalcon_websocket_connection_object_from_obj(getThis());
	intern->context = lws_create_context(&intern->info);
	if (intern->context == NULL) {
        RETURN_FALSE;
    }

	sprintf(address, "%s:%u", Z_STRVAL(host), Z_LVAL(port));

    wsi = lws_client_connect(context, Z_STRVAL(host), Z_LVAL(port), 0, Z_STRVAL(path), address, NULL, protocol.name, -1);
    if (wsi == NULL) {
        RETURN_FALSE;
    }

	gettimeofday(&tv, NULL);
	oldMs = ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	while (n >= 0 && !intern->exit_request) {
		if (nextTick <= 0) {
			if (intern->callbacks[PHP_CB_CLIENT_TICK]) {
				zval retval;
				ZVAL_NULL(&retval);
				zval params[1] = { *getThis() };
				intern->callbacks[PHP_CB_CLIENT_TICK]->fci->param_count = 1;
				intern->callbacks[PHP_CB_CLIENT_TICK]->fci->params = params;
				intern->callbacks[PHP_CB_CLIENT_TICK]->fci->retval = &retval;
				if (SUCCESS != zend_call_function(intern->callbacks[PHP_CB_CLIENT_TICK]->fci, intern->callbacks[PHP_CB_CLIENT_TICK]->fcc)) {
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

	text = zend_string_init(ZEND_STRL("Client terminated"), 0);

	conn = phalcon_websocket_connection_object_from_obj(Z_OBJ(intern->connection));
	phalcon_websocket_connection_close(conn, text);
	zval_delref(intern->connection);

	lws_context_destroy(intern->context);
	intern->context ＝ NULL;
}

/**
 * Send data to the client
 */
PHP_METHOD(Phalcon_Websocket_Client, send)
{
	zval *text;
	phalcon_websocket_client_object *intern;

	phalcon_fetch_params(0, 1, 0, &text);

	intern = phalcon_websocket_connection_object_from_obj(getThis());
	PHALCON_CALL_METHODW(return_value, &intern->connection, "send", text);
}

/**
 * Send data to the client as JSON string
 */
PHP_METHOD(Phalcon_Websocket_Client, sendJson)
{
	zval *val, text = {};
	phalcon_websocket_client_object *intern;

	phalcon_fetch_params(0, 1, 0, &val);

	intern = phalcon_websocket_connection_object_from_obj(getThis());

	phalcon_json_encode(&text, val, PHP_JSON_UNESCAPED_UNICODE|PHP_JSON_UNESCAPED_SLASHES);

	PHALCON_CALL_METHODW(return_value, &intern->connection, "sendjson", &text);
	smart_str_free(&text);
}

/**
 * Check is connection is established
 */
PHP_METHOD(Phalcon_Websocket_Client, isConnected)
{
	phalcon_websocket_client_object *intern;

	intern = phalcon_websocket_connection_object_from_obj(getThis());

	PHALCON_CALL_METHODW(return_value, &intern->connection, "isconnected");
}

/**
 * Close connection to the client
 */
PHP_METHOD(Phalcon_Websocket_Client, disconnect)
{
	phalcon_websocket_client_object *intern;

	intern = phalcon_websocket_connection_object_from_obj(getThis());
	PHALCON_CALL_METHODW(return_value, &intern->connection, "disconnect");
}
