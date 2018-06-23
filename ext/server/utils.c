
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

#include "server/utils.h"

#include <main/SAPI.h>
#include <ext/date/php_date.h>

/* Http parser */
struct http_parser_settings http_parser_request_settings = {
    .on_message_begin = phalcon_http_parser_on_message_begin,
    .on_url = phalcon_http_parser_on_url,
    .on_status = phalcon_http_parser_on_status,
    .on_header_field = phalcon_http_parser_on_header_field,
    .on_header_value = phalcon_http_parser_on_header_value,
    .on_headers_complete = phalcon_http_parser_on_headers_complete,
    .on_body = phalcon_http_parser_on_body,
    .on_message_complete = phalcon_http_parser_on_message_complete,
    .on_chunk_header = phalcon_http_parser_on_chunk_header,
    .on_chunk_complete = phalcon_http_parser_on_chunk_complete
};

phalcon_http_parser_data *phalcon_http_parser_data_new(struct http_parser_settings *request_settings, int type)
{
    phalcon_http_parser_data *data = ecalloc(1, sizeof(phalcon_http_parser_data));

    data->parser = emalloc(sizeof(struct http_parser));
    http_parser_init(data->parser, type);
    data->parser->data = data;

    array_init(&data->head);

    data->settings = request_settings;
    data->state = HTTP_PARSER_STATE_NONE;

    return data;
}

void phalcon_http_parser_data_free(phalcon_http_parser_data *data)
{
    if (!data) return;
	zval_ptr_dtor(&data->head);
	smart_str_free(&data->url);
	smart_str_free(&data->body);
	if (data->last_key) {
		zend_string_free(data->last_key);
	}
    efree(data->parser);
    efree(data);
    data = NULL;
}

int phalcon_http_parser_on_message_begin(http_parser *p)
{
    phalcon_http_parser_data *data = (phalcon_http_parser_data *)p->data;
    data->state = HTTP_PARSER_STATE_BEGIN;
    return 0;
}

int phalcon_http_parser_on_url(http_parser *p, const char *at, size_t length)
{
    phalcon_http_parser_data *data = (phalcon_http_parser_data *)p->data;
    data->state = HTTP_PARSER_STATE_URL;
    smart_str_appendl(&data->url, at, length);
    return 0;
}

int phalcon_http_parser_on_status(http_parser *p, const char *at, size_t length)
{
    phalcon_http_parser_data *data = (phalcon_http_parser_data *)p->data;
    data->state = HTTP_PARSER_STATE_STATUS;
    return 0;
}

int phalcon_http_parser_on_header_field(http_parser *p, const char *at, size_t length)
{
    phalcon_http_parser_data *data = (phalcon_http_parser_data *)p->data;
	int old_len = 0;
	if (data->state == HTTP_PARSER_STATE_VALUE) {
		zend_string_free(data->last_key);
		data->last_key = zend_string_init(at, length, 0);
	} else if (data->last_key) {
		old_len = ZSTR_LEN(data->last_key);
		data->last_key = zend_string_extend(data->last_key, old_len + length + 1, 0);
		memcpy(&(ZSTR_VAL(data->last_key)[old_len]), at, length);
		ZSTR_VAL(data->last_key)[old_len + length] = '\0';
	} else {
		data->last_key = zend_string_init(at, length, 0);
	}
    data->state = HTTP_PARSER_STATE_FIELD;
    return 0;
}

int phalcon_http_parser_on_header_value(http_parser *p, const char *at, size_t length)
{
	zval *element;
    phalcon_http_parser_data *data = (phalcon_http_parser_data *)p->data;
	int old_len = 0;

	if (data->state == HTTP_PARSER_STATE_VALUE && (element = zend_hash_find(Z_ARRVAL(data->head), data->last_key)) != NULL) {
		old_len = Z_STRLEN_P(element);
		Z_STR_P(element) = zend_string_extend(Z_STR_P(element), old_len + length + 1, 0);
		memcpy(&Z_STRVAL_P(element)[old_len], at, length);
		Z_STRVAL_P(element)[old_len + length] = '\0';
	} else {
    	add_assoc_stringl_ex(&data->head, ZSTR_VAL(data->last_key), ZSTR_LEN(data->last_key), (char*)at, length);
	}
    data->state = HTTP_PARSER_STATE_VALUE;
    return 0;
}

int phalcon_http_parser_on_headers_complete(http_parser *p)
{
    phalcon_http_parser_data *data = (phalcon_http_parser_data *)p->data;
    data->state = HTTP_PARSER_STATE_HEADER_END;
    return 0;
}

int phalcon_http_parser_on_body(http_parser *p, const char *at, size_t length)
{
    phalcon_http_parser_data *data = (phalcon_http_parser_data *)p->data;
    data->state = HTTP_PARSER_STATE_BODY;
    smart_str_appendl(&data->body, at, length);
    return 0;
}

int phalcon_http_parser_on_message_complete(http_parser *p)
{
    phalcon_http_parser_data *data = (phalcon_http_parser_data *)p->data;
    data->state = HTTP_PARSER_STATE_END;
	smart_str_0(&data->url);
	smart_str_0(&data->body);
    return 0;
}

int phalcon_http_parser_on_chunk_header(http_parser *p)
{
    phalcon_http_parser_data *data = (phalcon_http_parser_data *)p->data;
    data->state = HTTP_PARSER_STATE_CHUNK;
    return 0;
}

int phalcon_http_parser_on_chunk_complete(http_parser *p)
{
    phalcon_http_parser_data *data = (phalcon_http_parser_data *)p->data;
    data->state = HTTP_PARSER_STATE_END;
    return 0;
}

typedef struct _http_response_status_code_pair {
	const int code;
	const char *str;
} http_response_status_code_pair;

static http_response_status_code_pair http_status_map[] = {
	{ 100, "Continue" },
	{ 101, "Switching Protocols" },
	{ 200, "OK" },
	{ 201, "Created" },
	{ 202, "Accepted" },
	{ 203, "Non-Authoritative Information" },
	{ 204, "No Content" },
	{ 205, "Reset Content" },
	{ 206, "Partial Content" },
	{ 300, "Multiple Choices" },
	{ 301, "Moved Permanently" },
	{ 302, "Found" },
	{ 303, "See Other" },
	{ 304, "Not Modified" },
	{ 305, "Use Proxy" },
	{ 307, "Temporary Redirect" },
	{ 308, "Permanent Redirect" },
	{ 400, "Bad Request" },
	{ 401, "Unauthorized" },
	{ 402, "Payment Required" },
	{ 403, "Forbidden" },
	{ 404, "Not Found" },
	{ 405, "Method Not Allowed" },
	{ 406, "Not Acceptable" },
	{ 407, "Proxy Authentication Required" },
	{ 408, "Request Timeout" },
	{ 409, "Conflict" },
	{ 410, "Gone" },
	{ 411, "Length Required" },
	{ 412, "Precondition Failed" },
	{ 413, "Request Entity Too Large" },
	{ 414, "Request-URI Too Long" },
	{ 415, "Unsupported Media Type" },
	{ 416, "Requested Range Not Satisfiable" },
	{ 417, "Expectation Failed" },
	{ 426, "Upgrade Required" },
	{ 428, "Precondition Required" },
	{ 429, "Too Many Requests" },
	{ 431, "Request Header Fields Too Large" },
	{ 451, "Unavailable For Legal Reasons"},
	{ 500, "Internal Server Error" },
	{ 501, "Not Implemented" },
	{ 502, "Bad Gateway" },
	{ 503, "Service Unavailable" },
	{ 504, "Gateway Timeout" },
	{ 505, "HTTP Version Not Supported" },
	{ 506, "Variant Also Negotiates" },
	{ 511, "Network Authentication Required" },
	/* to allow search with while() loop */
	{ 0, NULL }
};

static int status_comp(const void *a, const void *b)
{
	const http_response_status_code_pair *pa = (const http_response_status_code_pair *) a;
	const http_response_status_code_pair *pb = (const http_response_status_code_pair *) b;

	if (pa->code < pb->code) {
		return -1;
	} else if (pa->code > pb->code) {
		return 1;
	}

	return 0;
}

static const size_t http_status_map_len = (sizeof(http_status_map) / sizeof(http_response_status_code_pair)) - 1;

static const char *get_status_string(int code)
{
	http_response_status_code_pair needle = {code, NULL}, *result = NULL;

	result = bsearch(&needle, http_status_map, http_status_map_len, sizeof(needle), status_comp);

	if (result) {
		return result->str;
	}

	return "Unknown Status Code";
}

static void append_http_status_line(smart_str *buffer, int protocol_version, int response_code, int persistent)
{
	if (!response_code) {
		response_code = 200;
	}
	smart_str_appendl_ex(buffer, "HTTP", 4, persistent);
	smart_str_appendc_ex(buffer, '/', persistent);
	smart_str_append_long_ex(buffer, protocol_version / 100, persistent);
	smart_str_appendc_ex(buffer, '.', persistent);
	smart_str_append_long_ex(buffer, protocol_version % 100, persistent);
	smart_str_appendc_ex(buffer, ' ', persistent);
	smart_str_append_long_ex(buffer, response_code, persistent);
	smart_str_appendc_ex(buffer, ' ', persistent);
	smart_str_appends_ex(buffer, get_status_string(response_code), persistent);
	smart_str_appendl_ex(buffer, "\r\n", 2, persistent);
}

static void append_essential_headers(smart_str* buffer)
{
	struct timeval tv = {0};

	if (!gettimeofday(&tv, NULL)) {
		zend_string *dt = php_format_date("r", 1, tv.tv_sec, 1);
		smart_str_appendl_ex(buffer, "Date: ", 6, 0);
		smart_str_appends_ex(buffer, dt->val, 0);
		smart_str_appendl_ex(buffer, "\r\n", 2, 0);
		zend_string_release(dt);
	}

	smart_str_appendl_ex(buffer, "Connection: close\r\n", sizeof("Connection: close\r\n") - 1, 0);
}

zend_string *phalcon_server_http_get_headers(char* headers)
{
	sapi_header_struct *h;
	zend_llist_position pos;
	smart_str buffer = {0};
	int protocol_version = 100;

	if (SG(sapi_headers).http_status_line) {
		smart_str_appends(&buffer, SG(sapi_headers).http_status_line);
		smart_str_appendl(&buffer, "\r\n", 2);
	} else {
		append_http_status_line(&buffer, protocol_version, SG(sapi_headers).http_response_code, 0);
	}

	append_essential_headers(&buffer);

	h = (sapi_header_struct*)zend_llist_get_first_ex(&SG(sapi_headers).headers, &pos);
	while (h) {
		if (h->header_len) {
			smart_str_appendl(&buffer, h->header, h->header_len);
			smart_str_appendl(&buffer, "\r\n", 2);
		}
		h = (sapi_header_struct*)zend_llist_get_next_ex(&SG(sapi_headers).headers, &pos);
	}
	if (headers) {
		smart_str_appends(&buffer, headers);
	}
	smart_str_appendl(&buffer, "\r\n", 2);
	smart_str_0(&buffer);

	return buffer.s;
}
