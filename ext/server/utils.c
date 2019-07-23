
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
