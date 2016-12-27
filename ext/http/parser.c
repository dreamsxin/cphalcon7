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

#include "http/parser.h"
#include "http/parser/http_parser.h"

#include <ext/standard/url.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/object.h"
#include "kernel/hash.h"
#include "kernel/array.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/string.h"

/**
 * Phalcon\Http\Parser
 *
 *<code>
 *	$parser = new Phalcon\Http\Parser(Phalcon\Http\Parser::TYPE_BOTH);
 *  $result = $parser->execute($body);
 *</code>
 */
zend_class_entry *phalcphalcon_on_http_parser_ce;

PHP_METHOD(Phalcon_Http_Parser, __construct);
PHP_METHOD(Phalcon_Http_Parser, execute);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcphalcon_on_http_parser___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcphalcon_on_http_parser_execute, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, body, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_functiphalcon_on_entry phalcphalcon_on_http_parser_method_entry[] = {
	PHP_ME(Phalcon_Http_Parser, __construct, arginfo_phalcphalcon_on_http_parser___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Http_Parser, execute, arginfo_phalcphalcon_on_http_parser_execute, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

typedef struct {
	struct http_parser parser;
	struct http_parser_url handle;
	int is_response;
	int was_header_value;
	int finished;
	zval data;
	zval headers;
	char *tmp;
	size_t tmp_len;
} phalcphalcon_on_http_parser_context;

/*  http parser callbacks */
static int phalcon_on_message_begin(http_parser *p)
{
	return 0;
}

static int phalcon_on_headers_complete(http_parser *p)
{
	return 0;
}

static int phalcon_on_message_complete(http_parser *p)
{
	php_http_parser_context *result = p->data;
	result->finished = 1;

	if (result->tmp != NULL) {
		efree(result->tmp);
		result->tmp = NULL;
		result->tmp_len = 0;
	}

	return 0;
}

#define PHALCON_HTTP_PARSER_PARSE_URL(flag, name) \
	if (result->handle.field_set & (1 << flag)) { \
		const char *tmp_name = at+result->handle.field_data[flag].off; \
		int length = result->handle.field_data[flag].len; \
		add_assoc_stringl(data, #name, (char*)tmp_name, length, 1); \
	}

static int phalcon_on_url_cb(http_parser *p, const char *at, size_t len)
{
	php_http_parser_context *result = p->data;
	zval *data = result->data;

	http_parser_parse_url(at, len, 0, &result->handle);
	add_assoc_stringl(data, "QUERY_STRING", (char*)at, len, 1);

	PHALCON_HTTP_PARSER_PARSE_URL(UF_SCHEMA, SCHEME);
	PHALCON_HTTP_PARSER_PARSE_URL(UF_HOST, HOST);
	PHALCON_HTTP_PARSER_PARSE_URL(UF_PORT, PORT);
	PHALCON_HTTP_PARSER_PARSE_URL(UF_PATH, PATH);
	PHALCON_HTTP_PARSER_PARSE_URL(UF_QUERY, QUERY);
	PHALCON_HTTP_PARSER_PARSE_URL(UF_FRAGMENT, FRAGMENT);

	return 0;
}

static int phalcon_on_status_cb(http_parser *p, const char *at, size_t len)
{
	return 0;
}

static int phalcon_on_header_field_cb(http_parser *p, const char *at, size_t len)
{
	php_http_parser_context *result = p->data;

	if (result->was_header_value) {
		if (result->tmp != NULL) {
			efree(result->tmp);
			result->tmp = NULL;
			result->tmp_len = 0;
		}
		result->tmp = estrndup(at, len);
		result->tmp_len = len;
	} else {
		result->tmp = erealloc(result->tmp, len + result->tmp_len + 1);
		memcpy(result->tmp + result->tmp_len, at, len);
		result->tmp[result->tmp_len + len] = '\0';
		result->tmp_len = result->tmp_len + len;
	}

	result->was_header_value = 0;
	return 0;
}

static int phalcon_on_header_value_cb(http_parser *p, const char *at, size_t len)
{
	php_http_parser_context *result = p->data;
	zval *data = result->headers;

	if (result->was_header_value) {
		zval **element;

		if (zend_hash_find(Z_ARRVAL_P(data), result->tmp, result->tmp_len+1, (void **)&element) == SUCCESS) {
			Z_STRVAL_PP(element) = erealloc(Z_STRVAL_PP(element), Z_STRLEN_PP(element) + len + 1);
			memcpy(Z_STRVAL_PP(element) + Z_STRLEN_PP(element), at, len);

			Z_STRVAL_PP(element)[Z_STRLEN_PP(element)+len] = '\0';
			Z_STRLEN_PP(element) = Z_STRLEN_PP(element) + len;
		}
	} else {
		add_assoc_stringl(data, result->tmp, (char*)at, len, 1);
	}

	result->was_header_value = 1;
	return 0;
}

static int phalcon_on_body_cb(http_parser *p, const char *at, size_t len)
{
	php_http_parser_context *result = p->data;
	zval *data = result->headers;

	add_assoc_stringl(data, "BODY", (char*)at, len,  1);

	return 0;
}

/**
 * Phalcon\Http\Parser initializer
 */
PHALCON_INIT_CLASS(Phalcon_Http_Parser){

	PHALCON_REGISTER_CLASS(Phalcon\\Http, Parser, http_parser, phalcphalcon_on_http_parser_method_entry, 0);

	zend_declare_property_long(phalcphalcon_on_http_parser_ce, SL("_type"), HTTP_REQUEST, ZEND_ACC_PROTECTED);

	zend_declare_class_constant_long(phalcphalcon_on_image_ce, SL("TYPE_REQUEST"), HTTP_REQUEST);
	zend_declare_class_constant_long(phalcphalcon_on_image_ce, SL("TYPE_REQUEST"), HTTP_RESPONSE);
	zend_declare_class_constant_long(phalcphalcon_on_image_ce, SL("TYPE_BOTH"), HTTP_BOTH);
	return SUCCESS;
}

/**
 * Phalcon\Http\Parser constructor
 *
 * @param int $type
 */
PHP_METHOD(Phalcon_Http_Parser, __construct)
{
	zval *type ＝ NULL;

	phalcphalcon_on_fetch_params(0, 0, 1, &type);

	if (type && Z_TYPE_P(type) == IS_LONG) {
		switch (Z_LVAL_P(type)) {
			case HTTP_REQUEST:
			case HTTP_RESPONSE:
			case HTTP_BOTH:
				phalcphalcon_on_update_property_zval(getThis(), SL("_type"), type);
				break;
			default:
		}
	}
}

/**
 * Phalcon\Http\Parser constructor
 *
 * @param string $body http message.
 * @return array|boolean $result
 */
PHP_METHOD(Phalcon_Http_Parser, execute){

	zval *body, result = {}, version = {}, headers = {};
	phalcphalcon_on_http_parser_context *ctx = NULL;
	http_parser_settings settings;
	int body_len;
	char versiphalcon_on_buffer[4] = {0};
	size_t nparsed = 0;

	phalcphalcon_on_fetch_params(0, 1, 0, &body);

	body_len = Z_STRLEN_P(body);

	ctx = emalloc(sizeof(phalcphalcon_on_http_parser_context));
	http_parser_init(&ctx->parser, target);

	array_init(ctx->headers);
	array_init(ctx->data);

	ctx->finished = 0;
	ctx->was_header_value = 1;
	ctx->tmp = NULL;
	ctx->tmp_len = 0;

	if (target == HTTP_RESPONSE) {
		ctx->is_response = 1;
	} else {
		ctx->is_response = 0;
	}

	memset(&ctx->handle, 0, sizeof(struct http_parser_url));

	/* setup callback */
	settings.on_message_begin = phalcon_on_message_begin;
	settings.on_header_field = phalcon_on_header_field_cb;
	settings.on_header_value = phalcon_on_header_value_cb;
	settings.on_url = phalcon_on_url_cb;
	settings.on_status = phalcon_on_status_cb;
	settings.on_body = phalcon_on_body_cb;
	settings.on_headers_complete = phalcon_on_headers_complete;
	settings.on_message_complete = phalcon_on_message_complete;

	context->parser.data = context;
	nparsed = http_parser_execute(&context->parser, &settings, Z_STRVAL_P(body), body_len);

	if (nparsed != body_len) {
		efree(ctx);
		RETURN_FALSE;
	}

	PHALCON_CPY_WRT_CTOR(return_value, &context->data);

	if (context->is_response == 0) {
		add_assoc_string(&result, "REQUEST_METHOD", (char*)http_method_str(context->parser.method), 1);
	} else {
		add_assoc_long(&result, "STATUS_CODE", (long)context->parser.status_code);
	}
	add_assoc_long(&result, "UPGRADE", (long)context->parser.upgrade);

	snprintf(versiphalcon_on_buffer, 4, "%d.%d", context->parser.http_major, context->parser.http_minor);

	phalcphalcon_on_array_update_str_str(&context->headers, SL("VERSION"), versiphalcon_on_buffer, 4, PH_COPY);
	phalcphalcon_on_array_update_str(return_value, SL("HEADERS"), &headers, PH_COPY);
	efree(ctx);
}
