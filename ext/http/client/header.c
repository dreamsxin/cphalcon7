
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2013 Phalcon Team (http://www.phalconphp.com)       |
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

#include "http/client/header.h"

#include <ext/spl/spl_iterators.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/operators.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/concat.h"
#include "kernel/fcall.h"
#include "kernel/exception.h"
#include "kernel/file.h"
#include "kernel/hash.h"
#include "kernel/string.h"

/**
 * Phalcon\Http\Client\Header
 */
zend_class_entry *phalcon_http_client_header_ce;

PHP_METHOD(Phalcon_Http_Client_Header, __construct);
PHP_METHOD(Phalcon_Http_Client_Header, set);
PHP_METHOD(Phalcon_Http_Client_Header, setStatusCode);
PHP_METHOD(Phalcon_Http_Client_Header, setMultiple);
PHP_METHOD(Phalcon_Http_Client_Header, addMultiple);
PHP_METHOD(Phalcon_Http_Client_Header, get);
PHP_METHOD(Phalcon_Http_Client_Header, getStatusCode);
PHP_METHOD(Phalcon_Http_Client_Header, getAll);
PHP_METHOD(Phalcon_Http_Client_Header, remove);
PHP_METHOD(Phalcon_Http_Client_Header, parse);
PHP_METHOD(Phalcon_Http_Client_Header, build);
PHP_METHOD(Phalcon_Http_Client_Header, count);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_client_header_set, 0, 0, 2)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_client_header_setstatuscode, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_client_header_setmultiple, 0, 0, 1)
	ZEND_ARG_INFO(0, values)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_client_header_addmultiple, 0, 0, 1)
	ZEND_ARG_INFO(0, values)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_client_header_get, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_client_header_remove, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_client_header_parse, 0, 0, 1)
	ZEND_ARG_INFO(0, content)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_client_header_build, 0, 0, 0)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()


static const zend_function_entry phalcon_http_client_header_method_entry[] = {
	PHP_ME(Phalcon_Http_Client_Header, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Http_Client_Header, set, arginfo_phalcon_http_client_header_set, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Header, setStatusCode, arginfo_phalcon_http_client_header_setstatuscode, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Header, setMultiple, arginfo_phalcon_http_client_header_setmultiple, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Header, addMultiple, arginfo_phalcon_http_client_header_addmultiple, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Header, get, arginfo_phalcon_http_client_header_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Header, getStatusCode, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Header, getAll, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Header, remove, arginfo_phalcon_http_client_header_remove, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Header, parse, arginfo_phalcon_http_client_header_parse, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Header, build, arginfo_phalcon_http_client_header_build, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Header, count, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Http\Client\Header initializer
 */
PHALCON_INIT_CLASS(Phalcon_Http_Client_Header){

	PHALCON_REGISTER_CLASS(Phalcon\\Http\\Client, Header, http_client_header, phalcon_http_client_header_method_entry, 0);

	zend_declare_property_null(phalcon_http_client_header_ce, SL("_messages"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_http_client_header_ce, SL("_fields"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_http_client_header_ce, SL("_version"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_http_client_header_ce, SL("_status_code"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_client_header_ce, SL("_status_message"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_client_header_ce, SL("_status"), ZEND_ACC_PROTECTED);

	zend_declare_class_constant_long(phalcon_http_client_header_ce, SL("BUILD_STATUS"), PHALCON_HTTP_CLIENT_HEADER_BUILD_STATUS);
	zend_declare_class_constant_long(phalcon_http_client_header_ce, SL("BUILD_FIELDS"), PHALCON_HTTP_CLIENT_HEADER_BUILD_FIELDS);

	zend_class_implements(phalcon_http_client_header_ce, 1, spl_ce_Countable);

	return SUCCESS;
}

/**
 * Phalcon\Http\Client\Header constructor
 */
PHP_METHOD(Phalcon_Http_Client_Header, __construct){

	zval *messages;

	PHALCON_MM_GROW();

	PHALCON_INIT_VAR(messages);
	array_init(messages);

	phalcon_array_update_long_string(messages, 100, SL("Continue"), PH_COPY);
	phalcon_array_update_long_string(messages, 101, SL("Switching Protocols"), PH_COPY);

	phalcon_array_update_long_string(messages, 200, SL("OK"), PH_COPY);
	phalcon_array_update_long_string(messages, 201, SL("Created"), PH_COPY);
	phalcon_array_update_long_string(messages, 202, SL("Accepted"), PH_COPY);
	phalcon_array_update_long_string(messages, 203, SL("Non-Authoritative Information"), PH_COPY);
	phalcon_array_update_long_string(messages, 204, SL("No Content"), PH_COPY);
	phalcon_array_update_long_string(messages, 205, SL("Reset Content"), PH_COPY);
	phalcon_array_update_long_string(messages, 206, SL("Partial Content"), PH_COPY);

	phalcon_array_update_long_string(messages, 300, SL("Multiple Choices"), PH_COPY);
	phalcon_array_update_long_string(messages, 301, SL("Moved Permanently"), PH_COPY);
	phalcon_array_update_long_string(messages, 302, SL("Found"), PH_COPY);
	phalcon_array_update_long_string(messages, 303, SL("See Other"), PH_COPY);
	phalcon_array_update_long_string(messages, 304, SL("Not Modified"), PH_COPY);
	phalcon_array_update_long_string(messages, 305, SL("Use Proxy"), PH_COPY);
	phalcon_array_update_long_string(messages, 306, SL("(Unused)"), PH_COPY);
	phalcon_array_update_long_string(messages, 307, SL("Temporary Redirect"), PH_COPY);

	phalcon_array_update_long_string(messages, 400, SL("Bad Request"), PH_COPY);
	phalcon_array_update_long_string(messages, 401, SL("Unauthorized"), PH_COPY);
	phalcon_array_update_long_string(messages, 402, SL("Payment Required"), PH_COPY);
	phalcon_array_update_long_string(messages, 403, SL("Forbidden"), PH_COPY);
	phalcon_array_update_long_string(messages, 404, SL("Not Found"), PH_COPY);
	phalcon_array_update_long_string(messages, 405, SL("Method Not Allowed"), PH_COPY);
	phalcon_array_update_long_string(messages, 406, SL("Not Acceptable"), PH_COPY);
	phalcon_array_update_long_string(messages, 407, SL("Proxy Authentication Required"), PH_COPY);
	phalcon_array_update_long_string(messages, 408, SL("Request Timeout"), PH_COPY);
	phalcon_array_update_long_string(messages, 409, SL("Conflict"), PH_COPY);
	phalcon_array_update_long_string(messages, 410, SL("Gone"), PH_COPY);
	phalcon_array_update_long_string(messages, 411, SL("Length Required"), PH_COPY);
	phalcon_array_update_long_string(messages, 412, SL("Precondition Failed"), PH_COPY);
	phalcon_array_update_long_string(messages, 413, SL("Request Entity Too Large"), PH_COPY);
	phalcon_array_update_long_string(messages, 414, SL("Request-URI Too Long"), PH_COPY);
	phalcon_array_update_long_string(messages, 415, SL("Unsupported Media Type"), PH_COPY);
	phalcon_array_update_long_string(messages, 416, SL("Requested Range Not Satisfiable"), PH_COPY);
	phalcon_array_update_long_string(messages, 417, SL("Expectation Failed"), PH_COPY);
	
	phalcon_array_update_long_string(messages, 500, SL("Internal Server Error"), PH_COPY);
	phalcon_array_update_long_string(messages, 501, SL("Not Implemented"), PH_COPY);
	phalcon_array_update_long_string(messages, 502, SL("Bad Gateway"), PH_COPY);
	phalcon_array_update_long_string(messages, 503, SL("Service Unavailable"), PH_COPY);
	phalcon_array_update_long_string(messages, 504, SL("Gateway Timeout"), PH_COPY);
	phalcon_array_update_long_string(messages, 505, SL("HTTP Version Not Supported"), PH_COPY);
	phalcon_array_update_long_string(messages, 506, SL("Bandwidth Limit Exceeded"), PH_COPY);

	phalcon_update_static_property_ce(phalcon_http_client_header_ce, SL("_messages"), messages);
	phalcon_update_property_empty_array(getThis(), SL("_fields"));

	PHALCON_MM_RESTORE();
}

PHP_METHOD(Phalcon_Http_Client_Header, set){

	zval *name, *value;

	phalcon_fetch_params(0, 2, 0, &name, &value);

	phalcon_update_property_array(getThis(), SL("_fields"), name, value);

}

PHP_METHOD(Phalcon_Http_Client_Header, setStatusCode){

	zval *value;

	phalcon_fetch_params(0, 1, 0, &value);

	phalcon_update_property_this(getThis(), SL("_status_code"), value);
}

PHP_METHOD(Phalcon_Http_Client_Header, setMultiple){

	zval *values;

	phalcon_fetch_params(0, 1, 0, &values);

	phalcon_update_property_this(getThis(), SL("_fields"), values);
}

PHP_METHOD(Phalcon_Http_Client_Header, addMultiple){

	zval *values, *fields;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 0, &values);

	fields = phalcon_read_property(getThis(), SL("_fields"), PH_NOISY);

	phalcon_array_merge_recursive_n(fields, values);

	phalcon_update_property_this(getThis(), SL("_fields"), fields);

	PHALCON_MM_RESTORE();
}

PHP_METHOD(Phalcon_Http_Client_Header, get){

	zval *name, *fields, *field;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 0, &name);

	fields = phalcon_read_property(getThis(), SL("_fields"), PH_NOISY);

	PHALCON_OBS_VAR(field);
	phalcon_array_fetch(&field, fields, name, PH_NOISY);

	RETURN_CTOR(field);
}

PHP_METHOD(Phalcon_Http_Client_Header, getStatusCode){

	RETURN_MEMBER(getThis(), "_status_code");
}

PHP_METHOD(Phalcon_Http_Client_Header, getAll){

	RETURN_MEMBER(getThis(), "_fields");
}

PHP_METHOD(Phalcon_Http_Client_Header, remove){

	zval *name, *fields;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 0, &name);

	fields = phalcon_read_property(getThis(), SL("_fields"), PH_NOISY);
	
	phalcon_array_unset(fields, name, 0);

	phalcon_update_property_this(getThis(), SL("_fields"), fields);

	PHALCON_MM_RESTORE();
}

PHP_METHOD(Phalcon_Http_Client_Header, parse){

	zval *content, *content_parts = NULL, *header = NULL, *header_parts = NULL, *name = NULL, *value = NULL, *trimmed = NULL;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 0, &content);

	if (PHALCON_IS_EMPTY(content)) {
		RETURN_MM_FALSE;
	}
	
	if (Z_TYPE_P(content) == IS_STRING) {
		PHALCON_INIT_VAR(content_parts);
		phalcon_fast_explode_str(content_parts, SL("\r\n"), content);
	} else if (Z_TYPE_P(content) == IS_ARRAY) {
		PHALCON_CPY_WRT_CTOR(content_parts, content);
	} else {
		RETURN_MM_FALSE;
	}	

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(content_parts), header) {
		if (Z_TYPE_P(header) == IS_STRING) {
			PHALCON_INIT_NVAR(header_parts);
			if (phalcon_memnstr_str(header , SL(":"))) {
				phalcon_fast_explode_str(header_parts, SL(":"), header);
			} else {
				if (phalcon_start_with_str(header , SL("HTTP/"))) {
					phalcon_fast_explode_str(header_parts, SL(" "), header);
					if (Z_TYPE_P(header_parts) == IS_ARRAY && phalcon_array_isset_long(header_parts, 1) && phalcon_array_isset_long(header_parts, 2)) {
						PHALCON_OBS_NVAR(value);
						phalcon_array_fetch_long(&value, header_parts, 1, PH_NOISY);
						phalcon_update_property_this(getThis(), SL("_status_code"), value);

						PHALCON_OBS_NVAR(value);
						phalcon_array_fetch_long(&value, header_parts, 2, PH_NOISY);
						phalcon_update_property_this(getThis(), SL("_status_message"), value);
					}
				}
				continue;
			}
		} else {
			PHALCON_CPY_WRT_CTOR(header_parts, header);
		}

		if (Z_TYPE_P(header_parts) == IS_ARRAY && phalcon_array_isset_long(header_parts, 0) && phalcon_array_isset_long(header_parts, 1)) {
				PHALCON_OBS_NVAR(name);
				phalcon_array_fetch_long(&name, header_parts, 0, PH_NOISY);

				PHALCON_OBS_NVAR(value);
				phalcon_array_fetch_long(&value, header_parts, 1, PH_NOISY);

				PHALCON_INIT_NVAR(trimmed);
				ZVAL_STR(trimmed, phalcon_trim(value, NULL, PHALCON_TRIM_BOTH));

				PHALCON_CALL_METHOD(NULL, getThis(), "set", name, trimmed);
		}
	} ZEND_HASH_FOREACH_END();

	PHALCON_MM_RESTORE();
}

PHP_METHOD(Phalcon_Http_Client_Header, build){

	zval *flags = NULL, *messages, *status_code, message, *version, *lines, *line = NULL;
	zval *fields, *value = NULL, *join_filed;
	zend_string *str_key;
	ulong idx;
	int f = 0;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 0, 1, &flags);

	if (flags) {
		f = phalcon_get_intval(flags);
	}

	PHALCON_INIT_VAR(lines);
	array_init(lines);

	messages = phalcon_read_static_property_ce(phalcon_http_client_header_ce, SL("_messages"));

	status_code = phalcon_read_property(getThis(), SL("_status_code"), PH_NOISY);
	
	if ((f & PHALCON_HTTP_CLIENT_HEADER_BUILD_STATUS) && phalcon_array_isset_fetch(&message, messages, status_code)) {
		version  = phalcon_read_property(getThis(), SL("_version "), PH_NOISY);

		PHALCON_INIT_NVAR(line);
		PHALCON_CONCAT_SVS(line, "HTTP/", version, " ");
		PHALCON_SCONCAT_VSV(line, status_code, " ", &message);

		phalcon_merge_append(lines, line);

	}

	fields = phalcon_read_property(getThis(), SL("_fields"), PH_NOISY);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(fields), idx, str_key, value) {
		zval filed, tmp;
		if (str_key) {
			ZVAL_STR(&filed, str_key);
		} else {
			ZVAL_LONG(&filed, idx);
		}

		PHALCON_CONCAT_VSV(&tmp, &filed, ": ", value);

		phalcon_merge_append(lines, &tmp);
	} ZEND_HASH_FOREACH_END();

	if (f & PHALCON_HTTP_CLIENT_HEADER_BUILD_FIELDS) {
		PHALCON_INIT_VAR(join_filed);
		phalcon_fast_join_str(join_filed, SL("\r\n"), lines);

		RETURN_CCTOR(join_filed);
	}

	RETURN_CCTOR(lines);
}

PHP_METHOD(Phalcon_Http_Client_Header, count){

	zval *fields;

	fields = phalcon_read_property(getThis(), SL("_fields"), PH_NOISY);
	phalcon_fast_count(return_value, fields);
}

