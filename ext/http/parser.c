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
#include "http/parser/http_parser.h"
#include "server/utils.h"

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
 * Phalcon\Http\Parser constructor
 *
 * @param string $body http message.
 * @return array|boolean $result
 */
PHP_METHOD(Phalcon_Http_Parser, execute){

	zval *body, type = {};
	phalcon_http_parser_object *intern;
	int body_len;
	char versiphalcon_on_buffer[4] = {0};
	size_t nparsed = 0;

	phalcon_fetch_params(0, 1, 0, &body);

	body_len = Z_STRLEN_P(body);

	phalcon_read_property(&type, getThis(), SL("_type"), PH_NOISY|PH_READONLY);

	intern = phalcon_http_parser_object_from_obj(Z_OBJ_P(getThis()));

	nparsed = http_parser_execute(intern->data->parser, intern->data->settings, Z_STRVAL_P(body), Z_STRLEN_P(body));

	if (nparsed != body_len) {
		RETURN_FALSE;
	}

	if (intern->data->parser->state < HTTP_PARSER_STATE_END) {
		RETURN_TRUE;
	}

	array_init(return_value);

	if (intern->data->url.s) {
		zval url = {};
		ZVAL_STR(&url, intern->data->url.s);
		phalcon_array_update_str(return_value, SL("QUERY_STRING"), &url, PH_COPY);
	}

	if (intern->data->body.s) {
		zval body = {};
		ZVAL_STR(&body, intern->data->body.s);
		phalcon_array_update_str(return_value, SL("BODY"), &body, PH_COPY);
	}

	if (Z_LVAL(type) == HTTP_REQUEST) {
		add_assoc_string(return_value, "REQUEST_METHOD", (char*)http_method_str(intern->data->parser->method));
	} else {
		add_assoc_long(return_value, "STATUS_CODE", (long)intern->data->parser->status_code);
	}
	add_assoc_long(return_value, "UPGRADE", (long)intern->data->parser->upgrade);

	snprintf(versiphalcon_on_buffer, 4, "%d.%d", intern->data->parser->http_major, intern->data->parser->http_minor);

	phalcon_array_update_str_str(return_value, SL("VERSION"), versiphalcon_on_buffer, 4, 0);
	phalcon_array_update_str(return_value, SL("HEADERS"), &intern->data->head, PH_COPY);
}
