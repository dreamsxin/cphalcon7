
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

#include "websocket/connection.h"

#include <zend_smart_str.h>

#include "kernel/main.h"
#include "kernel/string.h"
/**
 * Phalcon\Websocket\Connection
 *
 */
zend_class_entry *phalcon_websocket_connection_ce;

PHP_METHOD(Phalcon_Websocket_Connection, send);
PHP_METHOD(Phalcon_Websocket_Connection, sendJson);
PHP_METHOD(Phalcon_Websocket_Connection, isConnected);
PHP_METHOD(Phalcon_Websocket_Connection, getUid);
PHP_METHOD(Phalcon_Websocket_Connection, disconnect);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_connection_send, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_connection_sendjson, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, payload, 0, 0)
ZEND_END_ARG_INFO()

const zend_function_entry phalcon_websocket_connection_method_entry[] = {
	PHP_ME(Phalcon_Websocket_Connection, send, arginfo_phalcon_websocket_connection_send, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Connection, sendJson, arginfo_phalcon_websocket_connection_sendjson, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Connection, isConnected, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Connection, getUid, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Connection, disconnect, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

int phalcon_websocket_connection_write(phalcon_websocket_connection_object *conn, zend_string *text) {
	if (!conn->connected) {
		php_error_docref(NULL, E_WARNING, "Client is disconnected\n");
		return -1;
	}
	if (((conn->write_ptr + 1) % WEBSOCKET_CONNECTION_BUFFER_SIZE) == conn->read_ptr) {
		php_error_docref(NULL, E_WARNING, "Write buffer is full\n");
		return -1;
	}

	zend_string_addref(text);
	conn->buf[conn->write_ptr] = text;
	conn->write_ptr = (conn->write_ptr + 1) % WEBSOCKET_CONNECTION_BUFFER_SIZE;

	lws_callback_on_writable(conn->wsi);

	return text->len;
}

void phalcon_websocket_connection_close(phalcon_websocket_connection_object *conn, zend_string *reason) {
	conn->connected = 0;
	printf("Send close to %lu\n", conn->id);
	lws_close_reason(conn->wsi, LWS_CLOSE_STATUS_NORMAL, (unsigned char *)reason->val, reason->len);
	lws_callback_on_writable(conn->wsi);
}

zend_object_handlers phalcon_websocket_connection_object_handlers;
zend_object* phalcon_websocket_connection_create_object_handler(zend_class_entry *ce)
{
	phalcon_websocket_connection_object *intern = emalloc(sizeof(phalcon_websocket_connection_object));
	memset(intern, 0, sizeof(phalcon_websocket_connection_object));

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_websocket_connection_object_handlers;

	intern->connected = 0;
	intern->wsi = NULL;
	intern->read_ptr = 0;
	intern->write_ptr = 0;

	return &intern->std;
}

void phalcon_websocket_connection_free_object_storage_handler(zend_object *object)
{
	phalcon_websocket_connection_object *intern;
	intern = phalcon_websocket_connection_object_from_obj(object);
	while (intern->read_ptr != intern->write_ptr) {
		zend_string_delref(intern->buf[intern->read_ptr]);
		if (zend_string_refcount(intern->buf[intern->read_ptr]) < 1) {
			zend_string_free(intern->buf[intern->read_ptr]);
		}

		intern->read_ptr = (intern->read_ptr + 1) % WEBSOCKET_CONNECTION_BUFFER_SIZE;
	}

	zend_object_std_dtor(&intern->std);
	efree(intern);
}

/**
 * Phalcon\Websocket\Connection initializer
 */
PHALCON_INIT_CLASS(Phalcon_Websocket_Connection){

	PHALCON_REGISTER_CLASS(Phalcon\\Websocket, Connection, websocket_connection, phalcon_websocket_connection_method_entry, 0);

	phalcon_websocket_connection_ce->create_object = phalcon_websocket_connection_create_object_handler;
	memcpy(&phalcon_websocket_connection_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	phalcon_websocket_connection_object_handlers.free_obj = (zend_object_free_obj_t) phalcon_websocket_connection_free_object_storage_handler;

	return SUCCESS;
}

/**
 * Send data to the client
 */
PHP_METHOD(Phalcon_Websocket_Connection, send)
{
	phalcon_websocket_connection_object *intern;
	zend_string *text;
	int n;

	ZEND_PARSE_PARAMETERS_START(1, 1);
		Z_PARAM_STR(text)
	ZEND_PARSE_PARAMETERS_END();

	intern = phalcon_websocket_connection_object_from_obj(Z_OBJ_P(getThis()));
	n = phalcon_websocket_connection_write(intern, text);
	efree(text);
	if (-1 == n) {
		RETURN_FALSE;
	}
	RETURN_LONG(n);
}

/**
 * Send data to the client as JSON string
 */
PHP_METHOD(Phalcon_Websocket_Connection, sendJson)
{
	phalcon_websocket_connection_object *intern;
	zval *val, text = {};
	int n;

	ZEND_PARSE_PARAMETERS_START(1, 1);
		Z_PARAM_ZVAL(val);
	ZEND_PARSE_PARAMETERS_END();

	intern = phalcon_websocket_connection_object_from_obj(Z_OBJ_P(getThis()));
	RETURN_ON_FAILURE(phalcon_json_encode(&text, val, 0));
	n = phalcon_websocket_connection_write(intern, Z_STR(text));
	if (-1 == n) {
		RETURN_FALSE;
	}
	RETURN_LONG(n);
}

/*＊
 ＊ Check is connection is established
 */
PHP_METHOD(Phalcon_Websocket_Connection, isConnected)
{
	phalcon_websocket_connection_object *intern;

	intern = phalcon_websocket_connection_object_from_obj(Z_OBJ_P(getThis()));

	RETURN_BOOL(intern->connected);
}

/**
 * Get connection unique ID
 */
PHP_METHOD(Phalcon_Websocket_Connection, getUid)
{
	phalcon_websocket_connection_object *intern;

	intern = phalcon_websocket_connection_object_from_obj(Z_OBJ_P(getThis()));

	RETURN_LONG(intern->id);
}

/**
 * Close connection to the client
 */
PHP_METHOD(Phalcon_Websocket_Connection, disconnect)
{
	phalcon_websocket_connection_object *intern;
	zend_string *reason;

	ZEND_PARSE_PARAMETERS_START(0, 1);
		Z_PARAM_OPTIONAL
		Z_PARAM_STR(reason)
	ZEND_PARSE_PARAMETERS_END();

	intern = phalcon_websocket_connection_object_from_obj(Z_OBJ_P(getThis()));
	phalcon_websocket_connection_close(intern, reason);
}
