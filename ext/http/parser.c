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
zend_class_entry *phalcon_http_parser_ce;

PHP_METHOD(Phalcon_Http_Parser, __construct);
PHP_METHOD(Phalcon_Http_Parser, execute);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_parser___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_parser_execute, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, body, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_http_parser_method_entry[] = {
	PHP_ME(Phalcon_Http_Parser, __construct, arginfo_phalcon_http_parser___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Http_Parser, execute, arginfo_phalcon_http_parser_execute, ZEND_ACC_PUBLIC)
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
	zend_string *tmp;
} phalcon_http_parser_context;

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
	phalcon_http_parser_context *result = p->data;
	result->finished = 1;

	if (result->tmp != NULL) {
		zend_string_free(result->tmp);
		result->tmp = NULL;
	}

	return 0;
}

#define PHALCON_HTTP_PARSER_PARSE_URL(flag, name) \
	if (result->handle.field_set & (1 << flag)) { \
		add_assoc_stringl(&result->data, #name, (char*)(at+result->handle.field_data[flag].off), result->handle.field_data[flag].len); \
	}

static int phalcon_on_url_cb(http_parser *p, const char *at, size_t len)
{
	phalcon_http_parser_context *result = p->data;

	http_parser_parse_url(at, len, 0, &result->handle);
	add_assoc_stringl(&result->data, "QUERY_STRING", (char*)at, len);

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
	int tmp_len = 0;
	phalcon_http_parser_context *result = p->data;

	if (result->was_header_value) {
		if (result->tmp != NULL) {
			zend_string_free(result->tmp);
		}
		result->tmp = zend_string_init(at, len, 0);
	} else {
		if (result->tmp != NULL) {
			tmp_len = ZSTR_LEN(result->tmp);
			result->tmp = zend_string_extend(result->tmp, len + tmp_len + 1, 0);
			memcpy(&ZSTR_VAL(result->tmp)[tmp_len], at, len);
			ZSTR_VAL(result->tmp)[len + tmp_len] = '\0';
		} else {
			result->tmp = zend_string_init(at, len, 0);
		}
	}

	result->was_header_value = 0;
	return 0;
}

static int phalcon_on_header_value_cb(http_parser *p, const char *at, size_t len)
{
	int tmp_len = 0;
	phalcon_http_parser_context *result = p->data;

	if (result->was_header_value) {
		zval *element;

		if ((element = zend_hash_find(Z_ARRVAL(result->headers), result->tmp)) != NULL) {
			tmp_len = Z_STRLEN_P(element);
			Z_STR_P(element) = zend_string_extend(Z_STR_P(element), tmp_len + len + 1, 0);
			memcpy(&Z_STRVAL_P(element)[tmp_len], at, len);
			Z_STRVAL_P(element)[tmp_len + len] = '\0';
		}
	} else {
		add_assoc_stringl_ex(&result->headers, ZSTR_VAL(result->tmp), ZSTR_LEN(result->tmp), (char*)at, len);
	}

	result->was_header_value = 1;
	return 0;
}

static int phalcon_on_body_cb(http_parser *p, const char *at, size_t len)
{
	phalcon_http_parser_context *result = p->data;

	add_assoc_stringl(&result->data, "BODY", (char*)at, len);

	return 0;
}

/**
 * Phalcon\Http\Parser initializer
 */
PHALCON_INIT_CLASS(Phalcon_Http_Parser){

	PHALCON_REGISTER_CLASS(Phalcon\\Http, Parser, http_parser, phalcon_http_parser_method_entry, 0);

	zend_declare_property_long(phalcon_http_parser_ce, SL("_type"), HTTP_REQUEST, ZEND_ACC_PROTECTED);

	zend_declare_class_constant_long(phalcon_http_parser_ce, SL("TYPE_REQUEST"), HTTP_REQUEST);
	zend_declare_class_constant_long(phalcon_http_parser_ce, SL("HTTP_RESPONSE"), HTTP_RESPONSE);
	zend_declare_class_constant_long(phalcon_http_parser_ce, SL("TYPE_BOTH"), HTTP_BOTH);
	return SUCCESS;
}

/**
 * Phalcon\Http\Parser constructor
 *
 * @param int $type
 */
PHP_METHOD(Phalcon_Http_Parser, __construct)
{
	zval *type = NULL;

	phalcon_fetch_params(0, 0, 1, &type);

	if (type && Z_TYPE_P(type) == IS_LONG) {
		switch (Z_LVAL_P(type)) {
			case HTTP_REQUEST:
			case HTTP_RESPONSE:
			case HTTP_BOTH:
				phalcon_update_property(getThis(), SL("_type"), type);
				break;
			default:
				break;
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

	zval *body, type = {};
	phalcon_http_parser_context *ctx = NULL;
	http_parser_settings settings;
	int body_len;
	char versiphalcon_on_buffer[4] = {0};
	size_t nparsed = 0;

	phalcon_fetch_params(0, 1, 0, &body);

	phalcon_read_property(&type, getThis(), SL("_type"), PH_NOISY|PH_READONLY);

	body_len = Z_STRLEN_P(body);

	ctx = emalloc(sizeof(phalcon_http_parser_context));
	http_parser_init(&ctx->parser, Z_LVAL(type));

	array_init(&ctx->headers);
	array_init(&ctx->data);

	ctx->finished = 0;
	ctx->was_header_value = 1;
	ctx->tmp = NULL;

	if (Z_LVAL(type) == HTTP_RESPONSE) {
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

	ctx->parser.data = ctx;
	nparsed = http_parser_execute(&ctx->parser, &settings, Z_STRVAL_P(body), body_len);

	if (nparsed != body_len) {
		efree(ctx);
		RETURN_FALSE;
	}

	ZVAL_COPY(return_value, &ctx->data);

	if (ctx->is_response == 0) {
		add_assoc_string(return_value, "REQUEST_METHOD", (char*)http_method_str(ctx->parser.method));
	} else {
		add_assoc_long(return_value, "STATUS_CODE", (long)ctx->parser.status_code);
	}
	add_assoc_long(return_value, "UPGRADE", (long)ctx->parser.upgrade);

	snprintf(versiphalcon_on_buffer, 4, "%d.%d", ctx->parser.http_major, ctx->parser.http_minor);

	phalcon_array_update_str_str(&ctx->headers, SL("VERSION"), versiphalcon_on_buffer, 4, PH_COPY);
	phalcon_array_update_str(return_value, SL("HEADERS"), &ctx->headers, PH_COPY);
	efree(ctx);
}
