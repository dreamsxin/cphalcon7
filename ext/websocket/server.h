
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

#ifndef PHALCON_WEBSOCKET_SERVER_H
#define PHALCON_WEBSOCKET_SERVER_H

#include "php_phalcon.h"

#ifdef PHALCON_USE_WEBSOCKET
#include <libwebsockets.h>

enum php_server_callbacks {
	PHP_CB_SERVER_ACCEPT,
	PHP_CB_SERVER_CLOSE,

	PHP_CB_SERVER_DATA,

	PHP_CB_SERVER_TICK,
	PHP_CB_SERVER_ERROR,
	PHP_CB_SERVER_COUNT
};

typedef struct _phalcon_websocket_server_object {
	struct lws_context *context;
	struct lws_context_creation_info info;

	// Available PHP callbacks
	zval callbacks[PHP_CB_SERVER_COUNT];

	// External event loop
	zval eventloop;
	HashTable *eventloop_sockets;

	// Current connections
	zend_ulong next_id;
	zval connections;

	int write_protocol;

	zend_bool exit_request;
	zend_object std;
} phalcon_websocket_server_object;

static inline phalcon_websocket_server_object *phalcon_websocket_server_object_from_obj(zend_object *obj) {
	return (phalcon_websocket_server_object*)((char*)(obj) - XtOffsetOf(phalcon_websocket_server_object, std));
}

static inline phalcon_websocket_server_object *phalcon_websocket_server_object_from_ctx(struct lws_context *ctx) {
	return (phalcon_websocket_server_object*)((char*)(ctx) - XtOffsetOf(phalcon_websocket_server_object, context));
}

zend_bool phalcon_websocket_server_invoke_eventloop_cb(phalcon_websocket_server_object *intern, const char *func, int fd, int flags);

extern int  phalcon_websocket_server_handle;
#define phalcon_websocket_server_handle_name "websocket_server"

extern zend_class_entry *phalcon_websocket_server_ce;

PHALCON_INIT_CLASS(Phalcon_Websocket_Server);

#endif

#endif /* PHALCON_WEBSOCKET_SERVER_H */
