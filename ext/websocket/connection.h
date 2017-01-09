
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

#ifndef WEBSOCKET_CONNECTION_H
#define WEBSOCKET_CONNECTION_H

#include "php_phalcon.h"

#include <libwebsockets.h>

typedef struct _phalcon_websocket_connection_object {
	// ID (unique on server)
	zend_ulong id;

	// Connection state (Connected/Disconnected)
	zend_bool connected;

	// Write buffer
	zend_string *buf[WEBSOCKET_CONNECTION_BUFFER_SIZE];
	unsigned int read_ptr;
	unsigned int write_ptr;

	// LibWebSockets context
	struct lws *wsi;
	zend_object std;
} phalcon_websocket_connection_object;

static inline phalcon_websocket_connection_object *phalcon_websocket_connection_object_from_obj(zend_object *obj) {
	return (phalcon_websocket_connection_object*)((char*)(obj) - XtOffsetOf(phalcon_websocket_server_object, std));
}


int phalcon_websocket_connection_write(ws_connection_obj *conn, zend_string *text);
void phalcon_websocket_connection_close(ws_connection_obj *conn, zend_string *reason);
zend_object* phalcon_websocket_connection_create_object_handler(zend_class_entry *ce);
void phalcon_websocket_connection_free_object_storage_handler(phalcon_websocket_connection_object *intern);

extern zend_class_entry *phalcon_websocket_connection_ce;

PHALCON_INIT_CLASS(Phalcon_Websocket_Connection);

#endif /* WEBSOCKET_CONNECTION_H */
