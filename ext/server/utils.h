
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

#ifndef PHALCON_SERVER_UTILS_H
#define PHALCON_SERVER_UTILS_H

#include "php_phalcon.h"
#include "http/parser/http_parser.h"

#include <Zend/zend_smart_str.h>

int phalcon_http_parser_on_message_begin(http_parser *);
int phalcon_http_parser_on_url(http_parser *, const char *at, size_t length);
int phalcon_http_parser_on_status(http_parser *, const char *at, size_t length);
int phalcon_http_parser_on_header_field(http_parser *, const char *at, size_t length);
int phalcon_http_parser_on_header_value(http_parser *, const char *at, size_t length);
int phalcon_http_parser_on_headers_complete(http_parser *);
int phalcon_http_parser_on_body(http_parser *, const char *at, size_t length);
int phalcon_http_parser_on_message_complete(http_parser *);
int phalcon_http_parser_on_chunk_header(http_parser *);
int phalcon_http_parser_on_chunk_complete(http_parser *);

typedef enum {
    HTTP_PARSER_STATE_NONE, HTTP_PARSER_STATE_BEGIN, HTTP_PARSER_STATE_URL, HTTP_PARSER_STATE_STATUS, HTTP_PARSER_STATE_FIELD, HTTP_PARSER_STATE_VALUE, HTTP_PARSER_STATE_HEADER_END, HTTP_PARSER_STATE_BODY, HTTP_PARSER_STATE_CHUNK, HTTP_PARSER_STATE_END
} phalcon_parser_state;

typedef struct {
    struct http_parser *parser;
    struct http_parser_settings *settings;
    phalcon_parser_state state;
    zval head;
    smart_str url;
    smart_str body;
    zend_string *last_key;
} phalcon_http_parser_data;

phalcon_http_parser_data *phalcon_http_parser_data_new();
void phalcon_http_parser_data_free(phalcon_http_parser_data *hp);

extern char *http_200;
extern char *http_200_keepalive;

zend_string *phalcon_server_http_get_headers(char *headers);

#endif /* PHALCON_SERVER_UTILS_H */
