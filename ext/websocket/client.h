/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Altahrïm <me@altahrim.net>                                   |
  +----------------------------------------------------------------------+
*/

#ifndef PHALCON_WEBSOCKET_CLIENT_H
#define PHALCON_WEBSOCKET_CLIENT_H

#ifdef PHALCON_USE_WEBSOCKET

#include "php_phalcon.h"

#include <libwebsockets.h>

#include "websocket/structures.h"

/***** Class \WebSocket\Client *****/

/*--- Definitions ---*/

enum php_client_callbacks {
	PHP_CB_CLIENT_ACCEPT,
	PHP_CB_CLIENT_CLOSE,

	PHP_CB_CLIENT_DATA,

	PHP_CB_CLIENT_TICK,

	PHP_CB_CLIENT_COUNT
};

typedef struct _phalcon_websocket_client_object {
	struct lws_context *context;
	struct lws_context_creation_info info;
	// Available PHP callbacks
	ws_callback *callbacks[PHP_CB_CLIENT_COUNT];
	zval connection;
	zend_bool exit_request;
	zend_object std;
} phalcon_websocket_client_object;

static inline phalcon_websocket_client_object *phalcon_websocket_client_object_from_obj(zend_object *obj) {
	return (phalcon_websocket_client_object*)((char*)(obj) - XtOffsetOf(phalcon_websocket_client_object, std));
}

static inline phalcon_websocket_client_object *phalcon_websocket_client_object_from_ctx(struct lws_context *ctx) {
	return (phalcon_websocket_client_object*)((char*)(ctx) - XtOffsetOf(phalcon_websocket_client_object, context));
}

zend_object* phalcon_websocket_create_object_handler(zend_class_entry *ce);
void phalcon_websocket_client_free_object_storage_handler(phalcon_websocket_client_object *intern);

extern zend_class_entry *phalcon_websocket_client_ce;

PHALCON_INIT_CLASS(Phalcon_Websocket_Client);

#endif

#endif /* PHALCON_WEBSOCKET_CLIENT_H */
