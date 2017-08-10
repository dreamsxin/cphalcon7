
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

#include "http/client/adapter.h"
#include "http/client/adapterinterface.h"
#include "http/client/exception.h"
#include "http/uri.h"

#include "kernel/main.h"
#include "kernel/exception.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/array.h"
#include "kernel/file.h"

/**
 * Phalcon\Http\Client\Adapter
 */
zend_class_entry *phalcon_http_client_adapter_ce;

PHP_METHOD(Phalcon_Http_Client_Adapter, setUserAgent);
PHP_METHOD(Phalcon_Http_Client_Adapter, setAuth);
PHP_METHOD(Phalcon_Http_Client_Adapter, setHeader);
PHP_METHOD(Phalcon_Http_Client_Adapter, setHeaders);
PHP_METHOD(Phalcon_Http_Client_Adapter, setData);
PHP_METHOD(Phalcon_Http_Client_Adapter, setFile);
PHP_METHOD(Phalcon_Http_Client_Adapter, setFiles);
PHP_METHOD(Phalcon_Http_Client_Adapter, getPath);
PHP_METHOD(Phalcon_Http_Client_Adapter, get);
PHP_METHOD(Phalcon_Http_Client_Adapter, head);
PHP_METHOD(Phalcon_Http_Client_Adapter, post);
PHP_METHOD(Phalcon_Http_Client_Adapter, put);
PHP_METHOD(Phalcon_Http_Client_Adapter, delete);
PHP_METHOD(Phalcon_Http_Client_Adapter, setUri);
PHP_METHOD(Phalcon_Http_Client_Adapter, getUri);
PHP_METHOD(Phalcon_Http_Client_Adapter, setBaseUri);
PHP_METHOD(Phalcon_Http_Client_Adapter, setMethod);
PHP_METHOD(Phalcon_Http_Client_Adapter, setTimeOut);
PHP_METHOD(Phalcon_Http_Client_Adapter, send);

static const zend_function_entry phalcon_http_client_adapter_method_entry[] = {
	PHP_ME(Phalcon_Http_Client_Adapter, setUserAgent, arginfo_phalcon_http_client_adapterinterface_setuseragent, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Adapter, setAuth, arginfo_phalcon_http_client_adapterinterface_setauth, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Adapter, setHeader, arginfo_phalcon_http_client_adapterinterface_setheader, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Adapter, setHeaders, arginfo_phalcon_http_client_adapterinterface_setheaders, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Adapter, setData, arginfo_phalcon_http_client_adapterinterface_setdata, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Adapter, setFile, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Adapter, setFiles, arginfo_phalcon_http_client_adapterinterface_setfiles, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Adapter, getPath, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Adapter, get, arginfo_phalcon_http_client_adapterinterface_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Adapter, head, arginfo_phalcon_http_client_adapterinterface_head, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Adapter, post, arginfo_phalcon_http_client_adapterinterface_post, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Adapter, put, arginfo_phalcon_http_client_adapterinterface_put, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Adapter, delete, arginfo_phalcon_http_client_adapterinterface_delete, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Adapter, setUri, arginfo_phalcon_http_client_adapterinterface_seturi, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Adapter, getUri, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Adapter, setBaseUri, arginfo_phalcon_http_client_adapterinterface_setbaseuri, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Adapter, setMethod, arginfo_phalcon_http_client_adapterinterface_setmethod, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Adapter, setTimeOut, arginfo_phalcon_http_client_adapterinterface_settimeout, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Client_Adapter, send, arginfo_phalcon_http_client_adapterinterface_send, ZEND_ACC_PUBLIC)

	ZEND_FENTRY(sendInternal, NULL, NULL, ZEND_ACC_PROTECTED|ZEND_ACC_ABSTRACT)

	PHP_FE_END
};

/**
 * Phalcon\Http\Client\Adapter initializer
 */
PHALCON_INIT_CLASS(Phalcon_Http_Client_Adapter){

	PHALCON_REGISTER_CLASS(Phalcon\\Http\\Client, Adapter, http_client_adapter, phalcon_http_client_adapter_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_http_client_adapter_ce, SL("_header"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_client_adapter_ce, SL("_base_uri"), ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_http_client_adapter_ce, SL("_method"), "GET", ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_client_adapter_ce, SL("_useragent"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_client_adapter_ce, SL("_username"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_client_adapter_ce, SL("_password"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_client_adapter_ce, SL("_authtype"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_client_adapter_ce, SL("_digest"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_client_adapter_ce, SL("_entity_body"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_client_adapter_ce, SL("_data"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_client_adapter_ce, SL("_type"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_client_adapter_ce, SL("_files"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_http_client_adapter_ce, SL("_timeout"), 30, ZEND_ACC_PROTECTED);

	zend_declare_class_constant_stringl(phalcon_http_client_adapter_ce, SL("VERSION"), SL("0.0.1"));
	zend_declare_class_constant_stringl(phalcon_http_client_adapter_ce, SL("AUTH_TYPE_ANY"), SL("any"));
	zend_declare_class_constant_stringl(phalcon_http_client_adapter_ce, SL("AUTH_TYPE_BASIC"), SL("basic"));
	zend_declare_class_constant_stringl(phalcon_http_client_adapter_ce, SL("AUTH_TYPE_DIGEST"), SL("digest"));

	zend_class_implements(phalcon_http_client_adapter_ce, 1, phalcon_http_client_adapterinterface_ce);

	return SUCCESS;
}

/**
 * Sets the value of the userAgent property
 *
 * @param string $useragent
 * @return Phalcon\Http\Client\Adapter
 */
PHP_METHOD(Phalcon_Http_Client_Adapter, setUserAgent){

	zval *useragent;

	phalcon_fetch_params(0, 1, 0, &useragent);

	phalcon_update_property(getThis(), SL("_useragent"), useragent);

	RETURN_THIS();
}

/**
 * Set authentication credential
 *
 * @param string $username
 * @param string $password
 * @param string $authtype
 * @param array $digest
 * @param string $entityBody
 * @return Phalcon\Http\Client\Adapter
 */
PHP_METHOD(Phalcon_Http_Client_Adapter, setAuth){

	zval *username, *password, *_authtype = NULL, *digest = NULL, *entity_body = NULL, authtype = {};

	phalcon_fetch_params(0, 2, 3, &username, &password, &_authtype, &digest, &entity_body);

	phalcon_update_property(getThis(), SL("_username"), username);
	phalcon_update_property(getThis(), SL("_password"), password);

	if (!_authtype) {
		ZVAL_STRING(&authtype, "any");
	} else {
		ZVAL_COPY(&authtype, _authtype);
	}

	phalcon_update_property(getThis(), SL("_authtype"), &authtype);
	zval_ptr_dtor(&authtype);

	if (digest && Z_TYPE_P(digest) == IS_ARRAY) {
		phalcon_update_property(getThis(), SL("_digest"), digest);
	}

	if (entity_body) {
		phalcon_update_property(getThis(), SL("_entity_body"), entity_body);
	}

	RETURN_THIS();
}

/**
 * Set header
 *
 * @param string $name
 * @param string $value
 * @return Phalcon\Http\Client\Adapter
 */
PHP_METHOD(Phalcon_Http_Client_Adapter, setHeader){

	zval *name, *value, header = {};

	phalcon_fetch_params(0, 2, 0, &name, &value);

	phalcon_read_property(&header, getThis(), SL("_header"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(NULL, &header, "set", name, value);

	RETURN_THIS();
}

/**
 * Set headers
 *
 * @param array $headers
 * @return Phalcon\Http\Client\Adapter
 */
PHP_METHOD(Phalcon_Http_Client_Adapter, setHeaders){

	zval *headers, header = {};

	phalcon_fetch_params(0, 1, 0, &headers);

	phalcon_read_property(&header, getThis(), SL("_header"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(NULL, &header, "setMultiple", headers);

	RETURN_THIS();
}

/**
 * Set data
 *
 * @param array|string $data
 * @param string $type example: application/json
 * @return Phalcon\Http\Client\Adapter
 */
PHP_METHOD(Phalcon_Http_Client_Adapter, setData){

	zval *data, *type = NULL;

	phalcon_fetch_params(0, 1, 1, &data, &type);

	if (Z_TYPE_P(data) != IS_NULL && Z_TYPE_P(data) != IS_STRING && Z_TYPE_P(data) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_http_client_exception_ce, "data must be string or array");
		return;
	}

	phalcon_update_property(getThis(), SL("_data"), data);

	if (type &&  Z_TYPE_P(type) != IS_NULL) {
		if (Z_TYPE_P(type) != IS_STRING) {
			convert_to_string(type);
			phalcon_update_property(getThis(), SL("_type"), type);
			zval_ptr_dtor(type);
		} else {
			phalcon_update_property(getThis(), SL("_type"), type);
		}
	}

	RETURN_THIS();
}

/**
 * Set send file
 *
 * @param string $file
 * @return Phalcon\Http\Client\Adapter
 */
PHP_METHOD(Phalcon_Http_Client_Adapter, setFile){

	zval *file;

	phalcon_fetch_params(0, 1, 0, &file);

	if (Z_TYPE_P(file) != IS_ARRAY) {
		phalcon_update_property_array_append(getThis(), SL("_files"), file);
	} else {
		phalcon_update_property(getThis(), SL("_files"), file);
	}

	// zend_error_noreturn(E_DEPRECATED, "Method Phalcon\\Http\\Client\\Adapter::setFile is deprecated, please use Phalcon\\Http\\Client\\Adapter::setFiles instead");

	RETURN_THIS();
}

/**
 * Set send files
 *
 * @param array|string $files
 * @return Phalcon\Http\Client\Adapter
 */
PHP_METHOD(Phalcon_Http_Client_Adapter, setFiles){

	zval *files;

	phalcon_fetch_params(0, 1, 0, &files);

	if (Z_TYPE_P(files) != IS_ARRAY) {
		zval arr = {};
		array_init(&arr);
		phalcon_array_append(&arr, files, PH_COPY);
		phalcon_update_property(getThis(), SL("_files"), &arr);
		zval_ptr_dtor(&arr);
	} else {
		phalcon_update_property(getThis(), SL("_files"), files);
	}

	RETURN_THIS();
}

/**
 * Retrieve the URI path
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Client_Adapter, getPath){

	zval base_uri = {};

	phalcon_read_property(&base_uri, getThis(), SL("_base_uri"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_METHOD(return_value, &base_uri, "getpath");
}

/**
 * Send GET request
 *
 * @param string $uri
 * @param string $data
 * @return Phalcon\Http\Client\Response
 */
PHP_METHOD(Phalcon_Http_Client_Adapter, get){

	zval *uri = NULL, *data = NULL, *type = NULL;

	phalcon_fetch_params(0, 0, 3, &uri, &data, &type);

	if (uri && PHALCON_IS_NOT_EMPTY(uri)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "seturi", uri);
	}

	if (data) {
		if (!type) {
			type = &PHALCON_GLOBAL(z_null);
		}
		PHALCON_CALL_METHOD(NULL, getThis(), "setdata", data, type);
	}

	phalcon_update_property_str(getThis(), SL("_method"), SL("GET"));

	PHALCON_RETURN_CALL_METHOD(getThis(), "send");
}

/**
 * Send HEAD request
 *
 * @param string $uri
 * @param string $data
 * @return Phalcon\Http\Client\Response
 */
PHP_METHOD(Phalcon_Http_Client_Adapter, head){

	zval *uri = NULL, *data = NULL, *type = NULL;

	phalcon_fetch_params(0, 0, 3, &uri, &data, &type);

	if (uri && PHALCON_IS_NOT_EMPTY(uri)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "seturi", uri);
	}

	if (data) {
		if (!type) {
			type = &PHALCON_GLOBAL(z_null);
		}
		PHALCON_CALL_METHOD(NULL, getThis(), "setdata", data, type);
	}

	phalcon_update_property_str(getThis(), SL("_method"), SL("HEAD"));

	PHALCON_RETURN_CALL_METHOD(getThis(), "send");
}

/**
 * Send POST request
 *
 * @param string $uri
 * @param string $data
 * @return Phalcon\Http\Client\Response
 */
PHP_METHOD(Phalcon_Http_Client_Adapter, post){

	zval *uri = NULL, *data = NULL, *type = NULL;

	phalcon_fetch_params(0, 0, 3, &uri, &data, &type);

	if (uri && PHALCON_IS_NOT_EMPTY(uri)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "seturi", uri);
	}

	if (data) {
		if (!type) {
			type = &PHALCON_GLOBAL(z_null);
		}
		PHALCON_CALL_METHOD(NULL, getThis(), "setdata", data, type);
	}

	phalcon_update_property_str(getThis(), SL("_method"), SL("POST"));

	PHALCON_RETURN_CALL_METHOD(getThis(), "send");
}

/**
 * Send PUT request
 *
 * @param string $uri
 * @param string $data
 * @return Phalcon\Http\Client\Response
 */
PHP_METHOD(Phalcon_Http_Client_Adapter, put){

	zval *uri = NULL, *data = NULL, *type = NULL;

	phalcon_fetch_params(0, 0, 3, &uri, &data, &type);

	if (uri && PHALCON_IS_NOT_EMPTY(uri)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "seturi", uri);
	}

	if (data) {
		if (!type) {
			type = &PHALCON_GLOBAL(z_null);
		}
		PHALCON_CALL_METHOD(NULL, getThis(), "setdata", data, type);
	}

	phalcon_update_property_str(getThis(), SL("_method"), SL("PUT"));

	PHALCON_RETURN_CALL_METHOD(getThis(), "send");
}

/**
 * Send DELETE request
 *
 * @param string $uri
 * @param string $data
 * @return Phalcon\Http\Client\Response
 */
PHP_METHOD(Phalcon_Http_Client_Adapter, delete){

	zval *uri = NULL, *data = NULL, *type = NULL;

	phalcon_fetch_params(0, 0, 3, &uri, &data, &type);

	if (uri && PHALCON_IS_NOT_EMPTY(uri)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "seturi", uri);
	}

	if (data) {
		if (!type) {
			type = &PHALCON_GLOBAL(z_null);
		}
		PHALCON_CALL_METHOD(NULL, getThis(), "setdata", data, type);
	}

	phalcon_update_property_str(getThis(), SL("_method"), SL("DELETE"));

	PHALCON_RETURN_CALL_METHOD(getThis(), "send");
}

/**
 * Set URI
 *
 * @param string $uri
 * @return Phalcon\Http\Client\Adapter
 */
PHP_METHOD(Phalcon_Http_Client_Adapter, setUri){

	zval *uri, base_uri = {};

	phalcon_fetch_params(0, 1, 0, &uri);

	phalcon_read_property(&base_uri, getThis(), SL("_base_uri"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(base_uri) == IS_OBJECT) {
		PHALCON_CALL_METHOD(NULL, &base_uri, "extend", uri);
	} else {
		PHALCON_CALL_METHOD(NULL, getThis(), "setbaseuri", uri);
	}

	RETURN_THIS();
}

/**
 * Get URI
 *
 * @return Phalcon\Http\Uri
 */
PHP_METHOD(Phalcon_Http_Client_Adapter, getUri){

	zval base_uri = {};

	phalcon_read_property(&base_uri, getThis(), SL("_base_uri"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(base_uri) != IS_OBJECT) {
		PHALCON_CALL_METHOD(NULL, getThis(), "setbaseuri");
		phalcon_read_property(&base_uri, getThis(), SL("_base_uri"), PH_NOISY|PH_READONLY);
	}

	RETURN_CTOR(&base_uri);
}

/**
 * Set base URI
 *
 * @param string $uri
 * @return Phalcon\Http\Client\Adapter
 */
PHP_METHOD(Phalcon_Http_Client_Adapter, setBaseUri){

	zval *uri = NULL, base_uri = {};

	phalcon_fetch_params(0, 0, 1, &uri);

	if (!uri) {
		uri = &PHALCON_GLOBAL(z_null);
	}

	object_init_ex(&base_uri, phalcon_http_uri_ce);
	PHALCON_CALL_METHOD(NULL, &base_uri, "__construct", uri);

	phalcon_update_property(getThis(), SL("_base_uri"), &base_uri);
	zval_ptr_dtor(&base_uri);

	RETURN_THIS();
}

/**
 * Set method
 *
 * @param string $uri
 * @return Phalcon\Http\Client\Adapter
 */
PHP_METHOD(Phalcon_Http_Client_Adapter, setMethod){

	zval *method;

	phalcon_fetch_params(0, 1, 0, &method);

	phalcon_update_property(getThis(), SL("_method"), method);

	RETURN_THIS();
}

/**
 * Set the request timeout
 *
 * @param string $uri
 * @return Phalcon\Http\Client\Adapter
 */
PHP_METHOD(Phalcon_Http_Client_Adapter, setTimeOut){

	zval *time;

	phalcon_fetch_params(0, 1, 0, &time);

	phalcon_update_property(getThis(), SL("_timeout"), time);

	RETURN_THIS();
}

/**
 * Send request
 *
 * @return Phalcon\Http\Client\Response
 */
PHP_METHOD(Phalcon_Http_Client_Adapter, send){

	zval *uri = NULL;

	phalcon_fetch_params(0, 0, 1, &uri);

	if (uri) {
		PHALCON_CALL_METHOD(NULL, getThis(), "seturi", uri);
	}

	PHALCON_RETURN_CALL_METHOD(getThis(), "sendinternal");
}
