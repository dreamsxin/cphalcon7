
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

#ifndef PHALCON_WEBSOCKET_CONNECTION_H
#define PHALCON_WEBSOCKET_CONNECTION_H

#include "php_phalcon.h"

#ifdef PHALCON_USE_WEBSOCKET
#include <libwebsockets.h>

#define PHALCON_WEBSOCKET_FREQUENCY 0.2
#define PHALCON_WEBSOCKET_CONNECTION_BUFFER_SIZE 30

typedef struct _phalcon_websocket_connection_object {
	// ID (unique on server)
	zend_ulong id;

	// Connection state (Connected/Disconnected)
	zend_bool connected;

	// Write queue
	zval queue;

	// LibWebSockets context
	struct lws *wsi;
	zend_object std;
} phalcon_websocket_connection_object;

static inline phalcon_websocket_connection_object *phalcon_websocket_connection_object_from_obj(zend_object *obj) {
	return (phalcon_websocket_connection_object*)((char*)(obj) - XtOffsetOf(phalcon_websocket_connection_object, std));
}

int phalcon_websocket_connection_write(phalcon_websocket_connection_object *conn, zval *text, zval *type);
void phalcon_websocket_connection_close(phalcon_websocket_connection_object *conn, zend_string *reason);

extern zend_class_entry *phalcon_websocket_connection_ce;

PHALCON_INIT_CLASS(Phalcon_Websocket_Connection);

#endif
#endif /* PHALCON_WEBSOCKET_CONNECTION_H */
