
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

#include <ext/spl/spl_dllist.h>

#include "kernel/main.h"
#include "kernel/string.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"

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
	ZEND_ARG_TYPE_INFO(0, writeProtocol, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_websocket_connection_sendjson, 0, 0, 1)
	ZEND_ARG_INFO(0, payload)
	ZEND_ARG_TYPE_INFO(0, writeProtocol, IS_LONG, 1)
ZEND_END_ARG_INFO()

const zend_function_entry phalcon_websocket_connection_method_entry[] = {
	PHP_ME(Phalcon_Websocket_Connection, send, arginfo_phalcon_websocket_connection_send, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Connection, sendJson, arginfo_phalcon_websocket_connection_sendjson, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Connection, isConnected, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Connection, getUid, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Websocket_Connection, disconnect, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

int phalcon_websocket_connection_write(phalcon_websocket_connection_object *conn, zval *text, zval *type) {
	zval count = {}, item = {};
	int flag;
	if (!conn->connected) {
		php_error_docref(NULL, E_WARNING, "Client is disconnected\n");
		return -1;
	}
	if (Z_STRLEN_P(text) > 1024) {
		php_error_docref(NULL, E_WARNING, "Message must not exceed 1024 characters long\n");
		return -1;
	}
	PHALCON_CALL_METHOD_FLAG(flag, &count, &conn->queue, "count");
	if (PHALCON_GT_LONG(&count, PHALCON_WEBSOCKET_CONNECTION_BUFFER_SIZE)) {
		php_error_docref(NULL, E_WARNING, "Write buffer is full\n");
		return -1;
	}
	array_init_size(&item, 2);
	phalcon_array_append(&item, text, PH_COPY);
	phalcon_array_append(&item, type, PH_COPY);
	PHALCON_CALL_METHOD_FLAG(flag, NULL, &conn->queue, "enqueue", &item);
	zval_ptr_dtor(&item);

	if (flag != SUCCESS) {
		php_error_docref(NULL, E_WARNING, "Write buffer enqueue fail\n");
		return -1;
	}
	lws_callback_on_writable(conn->wsi);

	return Z_STRLEN_P(text);
}

void phalcon_websocket_connection_close(phalcon_websocket_connection_object *conn, zend_string *reason) {
	if (conn->connected) {
		conn->connected = 0;
		printf("Send close to %lu\n", conn->id);
		lws_close_reason(conn->wsi, LWS_CLOSE_STATUS_NORMAL, (unsigned char *)reason->val, reason->len);
		lws_callback_on_writable(conn->wsi);
	}
}

zend_object_handlers phalcon_websocket_connection_object_handlers;
zend_object* phalcon_websocket_connection_object_create_handler(zend_class_entry *ce)
{
	phalcon_websocket_connection_object *intern = ecalloc(1, sizeof(phalcon_websocket_connection_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_websocket_connection_object_handlers;

	object_init_ex(&intern->queue, spl_ce_SplQueue);

	intern->connected = 0;
	intern->wsi = NULL;

	return &intern->std;
}

void phalcon_websocket_connection_object_free_handler(zend_object *object)
{
	phalcon_websocket_connection_object *intern;
	intern = phalcon_websocket_connection_object_from_obj(object);
	zval_ptr_dtor(&intern->queue);

	zend_object_std_dtor(object);
}

/**
 * Phalcon\Websocket\Connection initializer
 */
PHALCON_INIT_CLASS(Phalcon_Websocket_Connection){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Websocket, Connection, websocket_connection, phalcon_websocket_connection_method_entry, 0);

	return SUCCESS;
}

/**
 * Send data to the client
 *
 * @param string $data
 * @param int $writeProtocol
 */
PHP_METHOD(Phalcon_Websocket_Connection, send)
{
	zval *data, *write_protocol = NULL;
	phalcon_websocket_connection_object *intern;
	int n;

	phalcon_fetch_params(0, 1, 1, &data, &write_protocol);

	if (!write_protocol) {
		write_protocol = &PHALCON_GLOBAL(z_null);
	}

	intern = phalcon_websocket_connection_object_from_obj(Z_OBJ_P(getThis()));
	n = phalcon_websocket_connection_write(intern, data, write_protocol);
	if (-1 == n) {
		RETURN_FALSE;
	}
	RETURN_LONG(n);
}

/**
 * Send data to the client as JSON string
 *
 * @param mixed $text
 * @param int $writeProtocol
 */
PHP_METHOD(Phalcon_Websocket_Connection, sendJson)
{
	zval *data, *write_protocol = NULL, text = {};
	phalcon_websocket_connection_object *intern;
	int n;

	phalcon_fetch_params(0, 1, 1, &data, &write_protocol);

	if (!write_protocol) {
		write_protocol = &PHALCON_GLOBAL(z_null);
	}

	intern = phalcon_websocket_connection_object_from_obj(Z_OBJ_P(getThis()));
	RETURN_ON_FAILURE(phalcon_json_encode(&text, data, 0));
	n = phalcon_websocket_connection_write(intern, &text, write_protocol);
	zval_ptr_dtor(&text);
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
