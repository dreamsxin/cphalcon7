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

#include "http/parser.h"

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
PHP_METHOD(Phalcon_Http_Parser, status);
PHP_METHOD(Phalcon_Http_Parser, execute);
PHP_METHOD(Phalcon_Http_Parser, parseCookie);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_parser___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_parser_execute, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, body, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, output, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_parser_parsecookies, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, str, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_http_parser_method_entry[] = {
	PHP_ME(Phalcon_Http_Parser, __construct, arginfo_phalcon_http_parser___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Http_Parser, status, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Parser, execute, arginfo_phalcon_http_parser_execute, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Parser, parseCookie, arginfo_phalcon_http_parser_parsecookies, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_object_handlers phalcon_http_parser_object_handlers;
zend_object* phalcon_http_parser_object_create_handler(zend_class_entry *ce)
{
	phalcon_http_parser_object *intern = ecalloc(1, sizeof(phalcon_http_parser_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_http_parser_object_handlers;

	return &intern->std;
}

void phalcon_http_parser_object_free_handler(zend_object *object)
{
	phalcon_http_parser_object *intern = phalcon_http_parser_object_from_obj(object);
	if (intern->data) {
		phalcon_http_parser_data_free(intern->data);
	}
	zend_object_std_dtor(object);
}

/**
 * Phalcon\Http\Parser initializer
 */
PHALCON_INIT_CLASS(Phalcon_Http_Parser){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Http, Parser, http_parser, phalcon_http_parser_method_entry, 0);

	zend_declare_property_long(phalcon_http_parser_ce, SL("_type"), HTTP_REQUEST, ZEND_ACC_PROTECTED);

	zend_declare_class_constant_long(phalcon_http_parser_ce, SL("TYPE_REQUEST"), HTTP_REQUEST);
	zend_declare_class_constant_long(phalcon_http_parser_ce, SL("HTTP_RESPONSE"), HTTP_RESPONSE);
	zend_declare_class_constant_long(phalcon_http_parser_ce, SL("TYPE_BOTH"), HTTP_BOTH);

	zend_declare_class_constant_long(phalcon_http_parser_ce, SL("STATUS_NONE"), HTTP_PARSER_STATE_NONE);
	zend_declare_class_constant_long(phalcon_http_parser_ce, SL("STATUS_BEGIN"), HTTP_PARSER_STATE_BEGIN);
	zend_declare_class_constant_long(phalcon_http_parser_ce, SL("STATUS_URL"), HTTP_PARSER_STATE_URL);
	zend_declare_class_constant_long(phalcon_http_parser_ce, SL("STATUS_STATUS"), HTTP_PARSER_STATE_STATUS);
	zend_declare_class_constant_long(phalcon_http_parser_ce, SL("STATUS_FIELD"), HTTP_PARSER_STATE_FIELD);
	zend_declare_class_constant_long(phalcon_http_parser_ce, SL("STATUS_VALUE"), HTTP_PARSER_STATE_VALUE);
	zend_declare_class_constant_long(phalcon_http_parser_ce, SL("STATUS_HEADER_END"), HTTP_PARSER_STATE_HEADER_END);
	zend_declare_class_constant_long(phalcon_http_parser_ce, SL("STATUS_BODY"), HTTP_PARSER_STATE_BODY);
	zend_declare_class_constant_long(phalcon_http_parser_ce, SL("STATUS_CHUNK"), HTTP_PARSER_STATE_CHUNK);
	zend_declare_class_constant_long(phalcon_http_parser_ce, SL("STATUS_END"), HTTP_PARSER_STATE_END);

	return SUCCESS;
}

extern struct http_parser_settings http_parser_request_settings;
/**
 * Phalcon\Http\Parser constructor
 *
 * @param int $type
 */
PHP_METHOD(Phalcon_Http_Parser, __construct)
{
	zval *type = NULL;
	phalcon_http_parser_object *intern;
	int t = HTTP_REQUEST;

	phalcon_fetch_params(0, 0, 1, &type);

	if (type && Z_TYPE_P(type) == IS_LONG) {
		switch (Z_LVAL_P(type)) {
			case HTTP_REQUEST:
			case HTTP_RESPONSE:
			case HTTP_BOTH:
				phalcon_update_property(getThis(), SL("_type"), type);
				t = Z_LVAL_P(type);
				break;
			default:
				break;
		}
	}

	intern = phalcon_http_parser_object_from_obj(Z_OBJ_P(getThis()));
	intern->data = phalcon_http_parser_data_new(&http_parser_request_settings, t);
}

/**
 * Gets parse status
 *
 * @return int
 */
PHP_METHOD(Phalcon_Http_Parser, status){

	phalcon_http_parser_object *intern;

	intern = phalcon_http_parser_object_from_obj(Z_OBJ_P(getThis()));

	RETURN_LONG(intern->data->state);
}

/**
 * parse
 *
 * @param string $body http message.
 * @return array|boolean $result
 */
PHP_METHOD(Phalcon_Http_Parser, execute){

	zval *body, *output = NULL, type = {};
	phalcon_http_parser_object *intern;
	int body_len;
	char versiphalcon_on_buffer[4] = {0};
	size_t nparsed = 0;

	phalcon_fetch_params(0, 1, 1, &body, &output);

	if (!output) {
		output = &PHALCON_GLOBAL(z_false);
	}

	body_len = Z_STRLEN_P(body);

	phalcon_read_property(&type, getThis(), SL("_type"), PH_NOISY|PH_READONLY);

	intern = phalcon_http_parser_object_from_obj(Z_OBJ_P(getThis()));

	nparsed = http_parser_execute(intern->data->parser, intern->data->settings, Z_STRVAL_P(body), Z_STRLEN_P(body));

	if (nparsed != body_len) {
		RETURN_FALSE;
	}

	if (intern->data->parser->state < HTTP_PARSER_STATE_END && !zend_is_true(output)) {
		RETURN_TRUE;
	}

	array_init(return_value);

	if (intern->data->parser->state >= HTTP_PARSER_STATE_URL && intern->data->url.s) {
		zval url = {};
		smart_str_0(&intern->data->url);
		ZVAL_STR(&url, intern->data->url.s);
		phalcon_array_update_str(return_value, SL("QUERY_STRING"), &url, PH_COPY);
	}

	if (intern->data->parser->state >= HTTP_PARSER_STATE_BODY && intern->data->body.s) {
		zval body = {};
		smart_str_0(&intern->data->body);
		ZVAL_STR(&body, intern->data->body.s);
		phalcon_array_update_str(return_value, SL("BODY"), &body, PH_COPY);
	}

	if (intern->data->parser->state >= HTTP_PARSER_STATE_STATUS) {
		if (Z_LVAL(type) == HTTP_REQUEST) {
			add_assoc_string(return_value, "REQUEST_METHOD", (char*)http_method_str(intern->data->parser->method));
		} else {
			add_assoc_long(return_value, "STATUS_CODE", (long)intern->data->parser->status_code);
		}
	}
	if (intern->data->parser->state >= HTTP_PARSER_STATE_HEADER_END) {
		add_assoc_long(return_value, "UPGRADE", (long)intern->data->parser->upgrade);

		snprintf(versiphalcon_on_buffer, 4, "%d.%d", intern->data->parser->http_major, intern->data->parser->http_minor);

		phalcon_array_update_str_str(return_value, SL("VERSION"), versiphalcon_on_buffer, 4, 0);
		phalcon_array_update_str(return_value, SL("HEADERS"), &intern->data->head, PH_COPY);
	}
}

/**
 * parse cookies
 *
 * @param string $str
 * @return array
 */
PHP_METHOD(Phalcon_Http_Parser, parseCookie){

	zval *str, cookies = {}, *cookie = NULL;

	phalcon_fetch_params(1, 1, 0, &str);

	array_init(return_value);
	phalcon_fast_explode_str(&cookies, SL(";"), str);
	if (Z_TYPE(cookies) != IS_ARRAY) {
		RETURN_MM();
	}
	PHALCON_MM_ADD_ENTRY(&cookies);

	ZEND_HASH_FOREACH_VAL(Z_ARR(cookies), cookie) {
		zval parts = {};
		phalcon_fast_explode_str2(&parts, SL("="), cookie, 2);
	
		if (Z_TYPE(parts) != IS_ARRAY) {
			continue;
		}
		PHALCON_MM_ADD_ENTRY(&parts);

		if (phalcon_fast_count_int(&parts) == 2) {
			zval key = {}, value = {};
			phalcon_array_fetch_long(&key, &parts, 0, PH_NOISY|PH_READONLY);
			phalcon_array_fetch_long(&value, &parts, 1, PH_NOISY|PH_READONLY);
			phalcon_array_update(return_value, &key, &value, PH_COPY);
		}
	} ZEND_HASH_FOREACH_END();
	RETURN_MM();
}
