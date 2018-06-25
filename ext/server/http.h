
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

#ifndef PHALCON_SERVER_HTTP_H
#define PHALCON_SERVER_HTTP_H

#include "php_phalcon.h"

#include "server/core.h"

typedef struct _phalcon_server_http_object {
	struct phalcon_server_context ctx;
	int enable_keepalive;
	zval application;
	zend_object std;
} phalcon_server_http_object;

static inline phalcon_server_http_object *phalcon_server_http_object_from_obj(zend_object *obj) {
	return (phalcon_server_http_object*)((char*)(obj) - XtOffsetOf(phalcon_server_http_object, std));
}

static inline phalcon_server_http_object *phalcon_server_http_object_from_ctx(struct phalcon_server_context *ctx) {
	return (phalcon_server_http_object*)((char*)(ctx) - XtOffsetOf(phalcon_server_http_object, ctx));
}

extern zend_class_entry *phalcon_server_http_ce;

PHALCON_INIT_CLASS(Phalcon_Server_Http);

#endif /* PHALCON_SERVER_HTTP_H */
