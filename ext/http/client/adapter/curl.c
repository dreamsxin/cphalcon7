
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
  +------------------------------------------------------------------------+
*/

#include "http/client/adapter/curl.h"
#include "http/client/adapter.h"
#include "http/client/adapterinterface.h"
#include "http/client/header.h"
#include "http/client/response.h"
#include "http/client/exception.h"
#include "http/uri.h"
#include "debug.h"

#include "kernel/main.h"
#include "kernel/exception.h"
#include "kernel/memory.h"
#include "kernel/operators.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/concat.h"
#include "kernel/fcall.h"
#include "kernel/file.h"
#include "kernel/hash.h"
#include "kernel/string.h"
#include "kernel/debug.h"

/**
 * Phalcon\Http\Client\Adapter\Curl
 */
zend_class_entry *phalcon_http_client_adapter_curl_ce;

PHP_METHOD(Phalcon_Http_Client_Adapter_Curl, __construct);
PHP_METHOD(Phalcon_Http_Client_Adapter_Curl, sendInternal);

static const zend_function_entry phalcon_http_client_adapter_curl_method_entry[] = {
	PHP_ME(Phalcon_Http_Client_Adapter_Curl, __construct, arginfo_phalcon_http_client_adapterinterface___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Http_Client_Adapter_Curl, sendInternal, NULL, ZEND_ACC_PROTECTED)
	PHP_FE_END
};

/**
 * Phalcon\Http\Client\Adapter\Curl initializer
 */
PHALCON_INIT_CLASS(Phalcon_Http_Client_Adapter_Curl){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Http\\Client\\Adapter, Curl, http_client_adapter_curl, phalcon_http_client_adapter_ce,  phalcon_http_client_adapter_curl_method_entry, 0);

	zend_declare_property_null(phalcon_http_client_adapter_curl_ce, SL("_curl"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_http_client_adapter_curl_ce, 1, phalcon_http_client_adapterinterface_ce);

	return SUCCESS;
}

PHP_METHOD(Phalcon_Http_Client_Adapter_Curl, __construct){

	zval *uri = NULL, *method = NULL, header = {}, curl = {}, options = {}, *constant;

	phalcon_fetch_params(1, 0, 2, &uri, &method);

	if (uri) {
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "setbaseuri", uri);
	}

	if (method) {
		zval upper_method = {};
		phalcon_fast_strtoupper(&upper_method, method);
		phalcon_update_property(getThis(), SL("_method"), &upper_method);
		zval_ptr_dtor(&upper_method);
	}

	object_init_ex(&header, phalcon_http_client_header_ce);
	PHALCON_MM_ADD_ENTRY(&header);
	PHALCON_MM_CALL_METHOD(NULL, &header, "__construct");

	phalcon_update_property(getThis(), SL("_header"), &header);

	PHALCON_MM_CALL_FUNCTION(&curl, "curl_init");
	PHALCON_MM_ADD_ENTRY(&curl);

	array_init(&options);
	PHALCON_MM_ADD_ENTRY(&options);

	if ((constant = zend_get_constant_str(SL("CURLOPT_RETURNTRANSFER"))) != NULL) {
		phalcon_array_update_zval_bool(&options, constant, 1, 0);
	}

	if ((constant = zend_get_constant_str(SL("CURLOPT_AUTOREFERER"))) != NULL) {
		phalcon_array_update_zval_bool(&options, constant, 1, 0);
	}

	if ((constant = zend_get_constant_str(SL("CURLOPT_FOLLOWLOCATION"))) != NULL) {
		phalcon_array_update_zval_bool(&options, constant, 1, 0);
	}

	if ((constant = zend_get_constant_str(SL("CURLOPT_MAXREDIRS"))) != NULL) {
		phalcon_array_update_zval_long(&options, constant, 20, 0);
	}

	if ((constant = zend_get_constant_str(SL("CURLOPT_HEADER"))) != NULL) {
		phalcon_array_update_zval_bool(&options, constant, 1, 0);
	}

	if ((constant = zend_get_constant_str(SL("CURLOPT_USERAGENT"))) != NULL) {
		phalcon_array_update_zval_str(&options, constant, SL("Phalcon HTTP Client(Curl)"), 0);
	}

	PHALCON_MM_CALL_FUNCTION(NULL, "curl_setopt_array", &curl, &options);

	phalcon_update_property(getThis(), SL("_curl"), &curl);

	RETURN_MM();
}

PHP_METHOD(Phalcon_Http_Client_Adapter_Curl, sendInternal){

	zval uri = {}, url = {}, method = {}, useragent = {}, data = {}, type = {}, files = {}, timeout = {}, curl = {}, username = {}, password = {}, authtype = {};
	zval *constant, header = {}, *constant1, curl_data = {}, *file, body = {}, boundary = {}, *value, key = {}, key_value = {}, headers = {};
	zval content = {}, errorno = {}, error = {}, headersize = {}, headerstr = {}, bodystr = {}, response = {};
	zend_string *str_key;
	ulong idx;

	PHALCON_MM_INIT();

	if ((constant = zend_get_constant_str(SL("CURLOPT_URL"))) == NULL) {
		RETURN_MM_FALSE;
	}

	PHALCON_MM_CALL_METHOD(&uri, getThis(), "geturi");
	PHALCON_MM_ADD_ENTRY(&uri);
	PHALCON_MM_CALL_METHOD(&url, &uri, "build");
	PHALCON_MM_ADD_ENTRY(&url);

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		zval debug_message = {};
		PHALCON_CONCAT_SV(&debug_message, "HTTP REQUEST URL: ", &url);
		PHALCON_DEBUG_LOG(&debug_message);
		zval_ptr_dtor(&debug_message);
	}

	phalcon_read_property(&method, getThis(), SL("_method"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&useragent, getThis(), SL("_useragent"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&type, getThis(), SL("_type"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&files, getThis(), SL("_files"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&timeout, getThis(), SL("_timeout"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&curl, getThis(), SL("_curl"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&username, getThis(), SL("_username"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&password, getThis(), SL("_password"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&authtype, getThis(), SL("_authtype"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_FUNCTION(NULL, "curl_setopt", &curl, constant, &url);

	if ((constant = zend_get_constant_str(SL("CURLOPT_CONNECTTIMEOUT"))) != NULL) {
		PHALCON_MM_CALL_FUNCTION(NULL, "curl_setopt", &curl, constant, &timeout);
	}

	if ((constant = zend_get_constant_str(SL("CURLOPT_TIMEOUT"))) != NULL) {
		PHALCON_MM_CALL_FUNCTION(NULL, "curl_setopt", &curl, constant, &timeout);
	}

	if (PHALCON_IS_NOT_EMPTY(&method) && (constant = zend_get_constant_str(SL("CURLOPT_CUSTOMREQUEST"))) != NULL) {
		PHALCON_MM_CALL_FUNCTION(NULL, "curl_setopt", &curl, constant, &method);
	}

	if (PHALCON_IS_NOT_EMPTY(&useragent) && (constant = zend_get_constant_str(SL("CURLOPT_USERAGENT"))) != NULL) {
		PHALCON_MM_CALL_FUNCTION(NULL, "curl_setopt", &curl, constant, &useragent);
	}

	phalcon_read_property(&header, getThis(), SL("_header"), PH_NOISY|PH_READONLY);

	if (PHALCON_IS_NOT_EMPTY(&username)) {
		if (PHALCON_IS_STRING(&authtype, "any")) {
			if ((constant = zend_get_constant_str(SL("CURLOPT_HTTPAUTH"))) != NULL && (constant1 = zend_get_constant_str(SL("CURLAUTH_ANY"))) != NULL) {
				PHALCON_MM_CALL_FUNCTION(NULL, "curl_setopt", &curl, constant, constant1);

				if ((constant = zend_get_constant_str(SL("CURLOPT_USERPWD"))) != NULL) {
					zval userpwd = {};
					PHALCON_CONCAT_VSV(&userpwd, &username, ":", &password);
					PHALCON_MM_ADD_ENTRY(&userpwd);
					PHALCON_MM_CALL_FUNCTION(NULL, "curl_setopt", &curl, constant, &userpwd);
				}
			}
		} else if (PHALCON_IS_STRING(&authtype, "basic")) {
			if ((constant = zend_get_constant_str(SL("CURLOPT_HTTPAUTH"))) != NULL && (constant1 = zend_get_constant_str(SL("CURLAUTH_BASIC"))) != NULL) {
				PHALCON_MM_CALL_FUNCTION(NULL, "curl_setopt", &curl, constant, constant1);

				if ((constant = zend_get_constant_str(SL("CURLOPT_USERPWD"))) != NULL) {
					zval userpwd = {};
					PHALCON_CONCAT_VSV(&userpwd, &username, ":", &password);
					PHALCON_MM_ADD_ENTRY(&userpwd);
					PHALCON_MM_CALL_FUNCTION(NULL, "curl_setopt", &curl, constant, &userpwd);
				}
			}
		} else if (PHALCON_IS_STRING(&authtype, "digest")) {
			if ((constant = zend_get_constant_str(SL("CURLOPT_HTTPAUTH"))) != NULL && (constant1 = zend_get_constant_str(SL("CURLAUTH_DIGEST"))) != NULL) {
				PHALCON_MM_CALL_FUNCTION(NULL, "curl_setopt", &curl, constant, constant1);

				if ((constant = zend_get_constant_str(SL("CURLOPT_USERPWD"))) != NULL) {
					zval userpwd = {};
					PHALCON_CONCAT_VSV(&userpwd, &username, ":", &password);
					PHALCON_MM_ADD_ENTRY(&userpwd);
					PHALCON_MM_CALL_FUNCTION(NULL, "curl_setopt", &curl, constant, &userpwd);
				}
			}
		}
	}

	if ((constant = zend_get_constant_str(SL("CURLOPT_SAFE_UPLOAD"))) != NULL) {
		PHALCON_MM_CALL_FUNCTION(NULL, "curl_setopt", &curl, constant, &PHALCON_GLOBAL(z_true));
	}

	if (Z_TYPE(data) == IS_STRING && PHALCON_IS_NOT_EMPTY(&data)) {
		PHALCON_MM_ZVAL_STRING(&key, "Content-Type");

		if (PHALCON_IS_EMPTY(&type)) {
			PHALCON_MM_CALL_METHOD(&key_value, &header, "get", &key);
			if (PHALCON_IS_EMPTY(&key_value)) {
				PHALCON_MM_ZVAL_STRING(&key_value, "application/x-www-form-urlencoded");
				PHALCON_MM_CALL_METHOD(NULL, &header, "set", &key, &key_value);
			}
		} else {
			ZVAL_COPY(&key_value, &type);
			PHALCON_MM_CALL_METHOD(NULL, &header, "set", &key, &key_value);
		}

		if ((constant = zend_get_constant_str(SL("CURLOPT_POSTFIELDS"))) != NULL) {
			PHALCON_MM_CALL_FUNCTION(NULL, "curl_setopt", &curl, constant, &data);
		}
	} else if (phalcon_class_str_exists(SL("CURLFile"), 0) != NULL) {
		if (Z_TYPE(data) != IS_ARRAY) {
			array_init(&curl_data);
			PHALCON_MM_ADD_ENTRY(&curl_data);
		} else {
			ZVAL_COPY_VALUE(&curl_data, &data);
		}

		if (Z_TYPE(files) == IS_ARRAY) {
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(files), file) {
				if (PHALCON_IS_NOT_EMPTY(file)) {
					zval curlfile = {};
					zend_class_entry *curlfile_ce;
					curlfile_ce = phalcon_fetch_str_class(SL("CURLFile"), ZEND_FETCH_CLASS_AUTO);

					object_init_ex(&curlfile, curlfile_ce);
					PHALCON_MM_ADD_ENTRY(&curlfile);
					PHALCON_MM_CALL_METHOD(NULL, &curlfile, "__construct", file);

					phalcon_array_append(&curl_data, &curlfile, PH_COPY);
				}
			} ZEND_HASH_FOREACH_END();
		}

		if ((constant = zend_get_constant_str(SL("CURLOPT_POSTFIELDS"))) != NULL) {
			PHALCON_MM_CALL_FUNCTION(NULL, "curl_setopt", &curl, constant, &curl_data);
		}
	} else {
		zval uniqid = {};
		PHALCON_MM_CALL_FUNCTION(&uniqid, "uniqid");
		PHALCON_MM_ADD_ENTRY(&uniqid);

		PHALCON_CONCAT_SV(&boundary, "--------------", &uniqid);
		PHALCON_MM_ADD_ENTRY(&boundary);

		if (Z_TYPE(data) == IS_ARRAY) {
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(data), idx, str_key, value) {
				zval name = {};
				if (str_key) {
					ZVAL_STR(&name, str_key);
				} else {
					ZVAL_LONG(&name, idx);
				}
				PHALCON_SCONCAT_SVS(&body, "--", &boundary, "\r\n");
				PHALCON_MM_ADD_ENTRY(&body);
				PHALCON_SCONCAT_SVSVS(&body, "Content-Disposition: form-data; name=\"", &name, "\"\r\n\r\n", value, "\r\n");
				PHALCON_MM_ADD_ENTRY(&body);
			} ZEND_HASH_FOREACH_END();
		}

		if (Z_TYPE(files) == IS_ARRAY) {
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(files), file) {
				if (PHALCON_IS_NOT_EMPTY(file)) {
					zval path_parts = {}, filename = {}, basename = {}, filedata = {};
					PHALCON_MM_CALL_FUNCTION(&path_parts, "pathinfo", file);
					PHALCON_MM_ADD_ENTRY(&path_parts);

					if (phalcon_array_isset_fetch_str(&filename, &path_parts, SL("filename"), PH_READONLY)
						&& phalcon_array_isset_fetch_str(&basename, &path_parts, SL("basename"), PH_READONLY)) {
						PHALCON_MM_CALL_FUNCTION(&filedata, "file_get_contents", file);
						PHALCON_MM_ADD_ENTRY(&filedata);

						PHALCON_SCONCAT_SVS(&body, "--", &boundary, "\r\n");
						PHALCON_MM_ADD_ENTRY(&body);
						PHALCON_SCONCAT_SVSVS(&body, "Content-Disposition: form-data; name=\"", &filename, "\"; filename=\"", &basename, "\"\r\n");
						PHALCON_MM_ADD_ENTRY(&body);
						PHALCON_SCONCAT_SVS(&body, "Content-Type: application/octet-stream\r\n\r\n", &filedata, "\r\n");
						PHALCON_MM_ADD_ENTRY(&body);
					}
				}
			} ZEND_HASH_FOREACH_END();
		}

		if (!PHALCON_IS_EMPTY(&body)) {
			PHALCON_SCONCAT_SVS(&body, "--", &boundary, "--\r\n");
			PHALCON_MM_ADD_ENTRY(&body);

			PHALCON_MM_ZVAL_STRING(&key, "Content-Type");
			PHALCON_CONCAT_SV(&key_value, "multipart/form-data; &boundary=", &boundary);
			PHALCON_MM_ADD_ENTRY(&key_value);

			PHALCON_MM_CALL_METHOD(NULL, &header, "set", &key, &key_value);

			PHALCON_MM_ZVAL_STRING(&key, "Content-Length");
			ZVAL_LONG(&key_value, Z_STRLEN_P(&body));

			PHALCON_MM_CALL_METHOD(NULL, &header, "set", &key, &key_value);
			if ((constant = zend_get_constant_str(SL("CURLOPT_POSTFIELDS"))) != NULL) {
				PHALCON_MM_CALL_FUNCTION(NULL, "curl_setopt", &curl, constant, &body);
			}
		}
	}

	if ((constant = zend_get_constant_str(SL("CURLOPT_HTTPHEADER"))) != NULL) {
		PHALCON_MM_CALL_METHOD(&headers, &header, "build");
		PHALCON_MM_ADD_ENTRY(&headers);
		PHALCON_MM_CALL_FUNCTION(NULL, "curl_setopt", &curl, constant, &headers);
	}

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		if ((constant = zend_get_constant_str(SL("CURLOPT_VERBOSE"))) != NULL) {
			PHALCON_MM_CALL_FUNCTION(NULL, "curl_setopt", &curl, constant, &PHALCON_GLOBAL(z_true));
		}
	}

	PHALCON_MM_CALL_FUNCTION(&content, "curl_exec", &curl);
	PHALCON_MM_ADD_ENTRY(&content);
	PHALCON_MM_CALL_FUNCTION(&errorno, "curl_errno", &curl);
	PHALCON_MM_ADD_ENTRY(&errorno);

	if (PHALCON_IS_TRUE(&errorno)) {
		PHALCON_MM_CALL_FUNCTION(&error, "curl_error", &curl);
		PHALCON_MM_ADD_ENTRY(&error);
		PHALCON_MM_THROW_EXCEPTION_ZVAL(phalcon_http_client_exception_ce, &error);
		return;
	}

	object_init_ex(&response, phalcon_http_client_response_ce);
	PHALCON_MM_ADD_ENTRY(&response);
	PHALCON_MM_CALL_METHOD(NULL, &response, "__construct");

	if ((constant = zend_get_constant_str(SL("CURLINFO_HTTP_CODE"))) != NULL) {
		zval httpcode = {};
		PHALCON_MM_CALL_FUNCTION(&httpcode, "curl_getinfo", &curl, constant);
		PHALCON_MM_ADD_ENTRY(&httpcode);
		PHALCON_MM_CALL_METHOD(NULL, &response, "setstatuscode", &httpcode);
	}


	if (Z_TYPE(content) == IS_STRING) {
		if ((constant = zend_get_constant_str(SL("CURLINFO_HEADER_SIZE"))) != NULL) {
			PHALCON_MM_CALL_FUNCTION(&headersize, "curl_getinfo", &curl, constant);
			PHALCON_MM_ADD_ENTRY(&headersize);

			if (Z_LVAL(headersize) > 0 ) {
				phalcon_substr(&headerstr, &content, 0 , Z_LVAL(headersize));
				PHALCON_MM_ADD_ENTRY(&headerstr);
				phalcon_substr(&bodystr, &content, Z_LVAL(headersize) , Z_STRLEN(content) - Z_LVAL(headersize));
				PHALCON_MM_ADD_ENTRY(&bodystr);

				PHALCON_MM_CALL_METHOD(NULL, &response, "setheader", &headerstr);
				PHALCON_MM_CALL_METHOD(NULL, &response, "setbody", &bodystr);
			} else {
				PHALCON_MM_CALL_METHOD(NULL, &response, "setbody", &content);
			}
		}
	}

	RETURN_MM_CTOR(&response);
}
