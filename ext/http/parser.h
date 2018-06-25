
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
  +------------------------------------------------------------------------+
*/

#ifndef PHALCON_HTTP_PARSER_H
#define PHALCON_HTTP_PARSER_H

#include "php_phalcon.h"
#include "server/utils.h"

typedef struct _phalcon_http_parser_object {
	phalcon_http_parser_data *data;
	zend_object std;
} phalcon_http_parser_object;

static inline phalcon_http_parser_object *phalcon_http_parser_object_from_obj(zend_object *obj) {
	return (phalcon_http_parser_object*)((char*)(obj) - XtOffsetOf(phalcon_http_parser_object, std));
}

extern zend_class_entry *phalcon_http_parser_ce;

PHALCON_INIT_CLASS(Phalcon_Http_Parser);

#endif /* PHALCON_HTTP_PARSER_H */
