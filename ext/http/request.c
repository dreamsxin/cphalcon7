
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

#include "http/request.h"
#include "http/requestinterface.h"
#include "http/request/exception.h"
#include "http/request/file.h"
#include "diinterface.h"
#include "di/injectable.h"
#include "filterinterface.h"

#include <main/php_variables.h>
#include <main/SAPI.h>
#include <Zend/zend_smart_str.h>
#include <ext/standard/file.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/file.h"
#include "kernel/hash.h"

#include "interned-strings.h"

/**
 * Phalcon\Http\Request
 *
 * <p>Encapsulates request information for easy and secure access from application controllers.</p>
 *
 * <p>The request object is a simple value object that is passed between the dispatcher and controller classes.
 * It packages the HTTP request environment.</p>
 *
 *<code>
 *	$request = new Phalcon\Http\Request();
 *	if ($request->isPost() == true) {
 *		if ($request->isAjax() == true) {
 *			echo 'Request was made using POST and AJAX';
 *		}
 *	}
 *</code>
 *
 */
zend_class_entry *phalcon_http_request_ce;

PHP_METHOD(Phalcon_Http_Request, _get);
PHP_METHOD(Phalcon_Http_Request, get);
PHP_METHOD(Phalcon_Http_Request, getPost);
PHP_METHOD(Phalcon_Http_Request, getPut);
PHP_METHOD(Phalcon_Http_Request, getQuery);
PHP_METHOD(Phalcon_Http_Request, getServer);
PHP_METHOD(Phalcon_Http_Request, has);
PHP_METHOD(Phalcon_Http_Request, hasPost);
PHP_METHOD(Phalcon_Http_Request, hasPut);
PHP_METHOD(Phalcon_Http_Request, hasQuery);
PHP_METHOD(Phalcon_Http_Request, hasServer);
PHP_METHOD(Phalcon_Http_Request, hasHeader);
PHP_METHOD(Phalcon_Http_Request, getHeader);
PHP_METHOD(Phalcon_Http_Request, getScheme);
PHP_METHOD(Phalcon_Http_Request, isAjax);
PHP_METHOD(Phalcon_Http_Request, isSoapRequested);
PHP_METHOD(Phalcon_Http_Request, isSecureRequest);
PHP_METHOD(Phalcon_Http_Request, getRawBody);
PHP_METHOD(Phalcon_Http_Request, getJsonRawBody);
PHP_METHOD(Phalcon_Http_Request, getBsonRawBody);
PHP_METHOD(Phalcon_Http_Request, getServerAddress);
PHP_METHOD(Phalcon_Http_Request, getServerName);
PHP_METHOD(Phalcon_Http_Request, getHttpHost);
PHP_METHOD(Phalcon_Http_Request, getClientAddress);
PHP_METHOD(Phalcon_Http_Request, getMethod);
PHP_METHOD(Phalcon_Http_Request, getURI);
PHP_METHOD(Phalcon_Http_Request, getQueryString);
PHP_METHOD(Phalcon_Http_Request, getUserAgent);
PHP_METHOD(Phalcon_Http_Request, isMethod);
PHP_METHOD(Phalcon_Http_Request, isPost);
PHP_METHOD(Phalcon_Http_Request, isGet);
PHP_METHOD(Phalcon_Http_Request, isPut);
PHP_METHOD(Phalcon_Http_Request, isPatch);
PHP_METHOD(Phalcon_Http_Request, isHead);
PHP_METHOD(Phalcon_Http_Request, isDelete);
PHP_METHOD(Phalcon_Http_Request, isOptions);
PHP_METHOD(Phalcon_Http_Request, hasFiles);
PHP_METHOD(Phalcon_Http_Request, getUploadedFiles);
PHP_METHOD(Phalcon_Http_Request, getHeaders);
PHP_METHOD(Phalcon_Http_Request, getHTTPReferer);
PHP_METHOD(Phalcon_Http_Request, _getQualityHeader);
PHP_METHOD(Phalcon_Http_Request, _getBestQuality);
PHP_METHOD(Phalcon_Http_Request, getAcceptableContent);
PHP_METHOD(Phalcon_Http_Request, getBestAccept);
PHP_METHOD(Phalcon_Http_Request, getClientCharsets);
PHP_METHOD(Phalcon_Http_Request, getBestCharset);
PHP_METHOD(Phalcon_Http_Request, getLanguages);
PHP_METHOD(Phalcon_Http_Request, getBestLanguage);
PHP_METHOD(Phalcon_Http_Request, getBasicAuth);
PHP_METHOD(Phalcon_Http_Request, getDigestAuth);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_request__get, 0, 0, 5)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, filters)
	ZEND_ARG_INFO(0, defaultValue)
	ZEND_ARG_INFO(0, notAllowEmpty)
	ZEND_ARG_INFO(0, noRecursive)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_http_request_method_entry[] = {
	PHP_ME(Phalcon_Http_Request, _get, arginfo_phalcon_http_request__get, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Http_Request, get, arginfo_phalcon_http_requestinterface_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getPost, arginfo_phalcon_http_requestinterface_getpost, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getPut, arginfo_phalcon_http_requestinterface_getput, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getQuery, arginfo_phalcon_http_requestinterface_getquery, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getServer, arginfo_phalcon_http_requestinterface_getserver, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, has, arginfo_phalcon_http_requestinterface_has, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, hasPost, arginfo_phalcon_http_requestinterface_haspost, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, hasPut, arginfo_phalcon_http_requestinterface_haspost, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, hasQuery, arginfo_phalcon_http_requestinterface_hasquery, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, hasServer, arginfo_phalcon_http_requestinterface_hasserver, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, hasHeader, arginfo_phalcon_http_requestinterface_hasheader, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getHeader, arginfo_phalcon_http_requestinterface_getheader, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getScheme, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, isAjax, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, isSoapRequested, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, isSecureRequest, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getRawBody, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getJsonRawBody, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getBsonRawBody, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getServerAddress, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getServerName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getHttpHost, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getClientAddress, arginfo_phalcon_http_requestinterface_getclientaddress, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getMethod, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getURI, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getQueryString, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getUserAgent, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, isMethod, arginfo_phalcon_http_requestinterface_ismethod, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, isPost, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, isGet, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, isPut, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, isPatch, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, isHead, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, isDelete, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, isOptions, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, hasFiles, arginfo_phalcon_http_requestinterface_hasfiles, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getUploadedFiles, arginfo_phalcon_http_requestinterface_getuploadedfiles, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getHeaders, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getHTTPReferer, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, _getQualityHeader, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Http_Request, _getBestQuality, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Http_Request, getAcceptableContent, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getBestAccept, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getClientCharsets, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getBestCharset, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getLanguages, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getBestLanguage, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getBasicAuth, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getDigestAuth, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Http\Request initializer
 */
PHALCON_INIT_CLASS(Phalcon_Http_Request){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Http, Request, http_request, phalcon_di_injectable_ce, phalcon_http_request_method_entry, 0);

	zend_declare_property_null(phalcon_http_request_ce, SL("_filter"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_request_ce, SL("_rawBody"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_request_ce, SL("_put"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_http_request_ce, 1, phalcon_http_requestinterface_ce);

	return SUCCESS;
}

/**
 * Internal get wrapper to filter
 *
 * @param string $name
 * @param string|array $filters
 * @param mixed $defaultValue
 * @param boolean $notAllowEmpty
 * @param boolean $noRecursive
 * @return mixed
 */
PHP_METHOD(Phalcon_Http_Request, _get)
{
	zval *data, *name, *filters, *default_value, *not_allow_empty, *norecursive;
	zval value, *dependency_injector, service, filter, filter_value;

	phalcon_fetch_params(0, 6, 0, &data, &name, &filters, &default_value, &not_allow_empty, &norecursive);

	if (Z_TYPE_P(name) != IS_NULL) {
		if (!phalcon_array_isset_fetch(&value, data, name)) {
			RETURN_CTORW(default_value);
		}
	} else {
		PHALCON_CPY_WRT_CTOR(&value, data);
	}

	if (Z_TYPE_P(filters) != IS_NULL) {
		phalcon_return_property(&filter, getThis(), SL("_filter"));
		if (Z_TYPE(filter) != IS_OBJECT) {
			dependency_injector = phalcon_read_property(getThis(), SL("_dependencyInjector"), PH_NOISY);
			if (Z_TYPE_P(dependency_injector) != IS_OBJECT) {
				PHALCON_THROW_EXCEPTION_STRW(phalcon_http_request_exception_ce, "A dependency injection object is required to access the 'filter' service");
				return;
			}

			PHALCON_STR(&service, ISV(filter));

			PHALCON_CALL_METHODW(&filter, dependency_injector, "getshared", &service);
			PHALCON_VERIFY_INTERFACEW(&filter, phalcon_filterinterface_ce);

			phalcon_update_property_this(getThis(), SL("_filter"), &filter);
		}

		PHALCON_CALL_METHODW(&filter_value, &filter, "sanitize", &value, filters, norecursive);

		if ((PHALCON_IS_EMPTY(&filter_value) && zend_is_true(not_allow_empty)) || PHALCON_IS_FALSE(&filter_value)) {
			RETURN_CTORW(default_value);
		}

		RETURN_CTORW(&filter_value);
	}

	if (PHALCON_IS_EMPTY(&value) && zend_is_true(not_allow_empty)) {
		RETURN_CTORW(default_value);
	}

	RETURN_CTORW(&value);
}

/**
 * Gets a variable from the $_REQUEST superglobal applying filters if needed.
 * If no parameters are given the $_REQUEST superglobal is returned
 *
 *<code>
 *	//Returns value from $_REQUEST["user_email"] without sanitizing
 *	$userEmail = $request->get("user_email");
 *
 *	//Returns value from $_REQUEST["user_email"] with sanitizing
 *	$userEmail = $request->get("user_email", "email");
 *</code>
 *
 * @param string $name
 * @param string|array $filters
 * @param mixed $defaultValue
 * @param boolean $notAllowEmpty
 * @param boolean $noRecursive
 * @return mixed
 */
PHP_METHOD(Phalcon_Http_Request, get)
{
	zval *name = NULL, *filters = NULL, *default_value = NULL, *not_allow_empty = NULL, *norecursive = NULL, *request;

	phalcon_fetch_params(0, 0, 5, &name, &filters, &default_value, &not_allow_empty, &norecursive);

	if (!name) {
		name = &PHALCON_GLOBAL(z_null);
	}

	if (!filters) {
		filters = &PHALCON_GLOBAL(z_null);
	}

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	if (!not_allow_empty) {
		not_allow_empty = &PHALCON_GLOBAL(z_false);
	}

	if (!norecursive) {
		norecursive = &PHALCON_GLOBAL(z_false);
	}

	request = phalcon_get_global_str(SL("_REQUEST"));

	PHALCON_RETURN_CALL_SELFW("_get", request, name, filters, default_value, not_allow_empty, norecursive);
}

/**
 * Gets a variable from the $_POST superglobal applying filters if needed
 * If no parameters are given the $_POST superglobal is returned
 *
 *<code>
 *	//Returns value from $_POST["user_email"] without sanitizing
 *	$userEmail = $request->getPost("user_email");
 *
 *	//Returns value from $_POST["user_email"] with sanitizing
 *	$userEmail = $request->getPost("user_email", "email");
 *</code>
 *
 * @param string $name
 * @param string|array $filters
 * @param mixed $defaultValue
 * @param boolean $notAllowEmpty
 * @param boolean $noRecursive
 * @return mixed
 */
PHP_METHOD(Phalcon_Http_Request, getPost)
{
	zval *name = NULL, *filters = NULL, *default_value = NULL, *not_allow_empty = NULL, *norecursive = NULL, *post;

	phalcon_fetch_params(0, 0, 5, &name, &filters, &default_value, &not_allow_empty, &norecursive);

	if (!name) {
		name = &PHALCON_GLOBAL(z_null);
	}

	if (!filters) {
		filters = &PHALCON_GLOBAL(z_null);
	}

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	if (!not_allow_empty) {
		not_allow_empty = &PHALCON_GLOBAL(z_false);
	}

	if (!norecursive) {
		norecursive = &PHALCON_GLOBAL(z_false);
	}

	post = phalcon_get_global_str(SL("_POST"));
	PHALCON_RETURN_CALL_SELFW("_get", post, name, filters, default_value, not_allow_empty, norecursive);
}

/**
 * Gets a variable from put request
 *
 *<code>
 *	$userEmail = $request->getPut("user_email");
 *
 *	$userEmail = $request->getPut("user_email", "email");
 *</code>
 *
 * @param string $name
 * @param string|array $filters
 * @param mixed $defaultValue
 * @param boolean $notAllowEmpty
 * @param boolean $noRecursive
 * @return mixed
 */
PHP_METHOD(Phalcon_Http_Request, getPut)
{
	zval *name = NULL, *filters = NULL, *default_value = NULL, *not_allow_empty = NULL, *norecursive = NULL;
	zval is_put, *put = NULL, raw, new_put;
	char *tmp;

	phalcon_fetch_params(0, 0, 5, &name, &filters, &default_value, &not_allow_empty, &norecursive);

	if (!name) {
		name = &PHALCON_GLOBAL(z_null);
	}

	if (!filters) {
		filters = &PHALCON_GLOBAL(z_null);
	}

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	if (!not_allow_empty) {
		not_allow_empty = &PHALCON_GLOBAL(z_false);
	}

	if (!norecursive) {
		norecursive = &PHALCON_GLOBAL(z_false);
	}

	PHALCON_CALL_METHODW(&is_put, getThis(), "isput");

	if (!zend_is_true(&is_put)) {
		put = phalcon_get_global_str(SL("_PUT"));
		PHALCON_CPY_WRT(&new_put, put);
	} else {
		put = phalcon_read_property(getThis(), SL("_put"), PH_NOISY);
		if (Z_TYPE_P(put) != IS_ARRAY) {
			PHALCON_CALL_METHODW(&raw, getThis(), "getrawbody");

			array_init(&new_put);

			PHALCON_ENSURE_IS_STRING(&raw);
			tmp = estrndup(Z_STRVAL(raw), Z_STRLEN(raw));

			sapi_module.treat_data(PARSE_STRING, tmp, &new_put);

			phalcon_update_property_this(getThis(), SL("_put"), &new_put);
		} else {
			PHALCON_CPY_WRT(&new_put, put);
		}
	}

	PHALCON_RETURN_CALL_SELFW("_get", &new_put, name, filters, default_value, not_allow_empty, norecursive);
}

/**
 * Gets variable from $_GET superglobal applying filters if needed
 * If no parameters are given the $_GET superglobal is returned
 *
 *<code>
 *	//Returns value from $_GET["id"] without sanitizing
 *	$id = $request->getQuery("id");
 *
 *	//Returns value from $_GET["id"] with sanitizing
 *	$id = $request->getQuery("id", "int");
 *
 *	//Returns value from $_GET["id"] with a default value
 *	$id = $request->getQuery("id", null, 150);
 *</code>
 *
 * @param string $name
 * @param string|array $filters
 * @param mixed $defaultValue
 * @param boolean $notAllowEmpty
 * @param boolean $noRecursive
 * @return mixed
 */
PHP_METHOD(Phalcon_Http_Request, getQuery){

	zval *name = NULL, *filters = NULL, *default_value = NULL, *not_allow_empty = NULL, *norecursive = NULL, *get;

	phalcon_fetch_params(0, 0, 5, &name, &filters, &default_value, &not_allow_empty, &norecursive);

	if (!name) {
		name = &PHALCON_GLOBAL(z_null);
	}

	if (!filters) {
		filters = &PHALCON_GLOBAL(z_null);
	}

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	if (!not_allow_empty) {
		not_allow_empty = &PHALCON_GLOBAL(z_false);
	}

	if (!norecursive) {
		norecursive = &PHALCON_GLOBAL(z_false);
	}

	get = phalcon_get_global_str(SL("_GET"));

	PHALCON_RETURN_CALL_SELFW("_get", get, name, filters, default_value, not_allow_empty, norecursive);
}

/**
 * Gets variable from $_SERVER superglobal
 *
 * @param string $name
 * @return mixed
 */
PHP_METHOD(Phalcon_Http_Request, getServer){

	zval *name, *_SERVER;

	phalcon_fetch_params(0, 1, 0, &name);

	_SERVER = phalcon_get_global_str(SL("_SERVER"));
	if (!phalcon_array_isset_fetch(return_value, _SERVER, name)) {
		RETURN_NULL();
	}
}

/**
 * Checks whether $_REQUEST superglobal has certain index
 *
 * @param string $name
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, has){

	zval *name, *_REQUEST;

	phalcon_fetch_params(0, 1, 0, &name);

	_REQUEST = phalcon_get_global_str(SL("_REQUEST"));
	RETURN_BOOL(phalcon_array_isset(_REQUEST, name));
}

/**
 * Checks whether $_POST superglobal has certain index
 *
 * @param string $name
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, hasPost){

	zval *name, *_POST;

	phalcon_fetch_params(0, 1, 0, &name);

	_POST = phalcon_get_global_str(SL("_POST"));
	RETURN_BOOL(phalcon_array_isset(_POST, name));
}

/**
 * Checks whether put has certain index
 *
 * @param string $name
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, hasPut){

	zval *name, is_put, *put = NULL, raw, new_put;
	char *tmp;

	phalcon_fetch_params(0, 1, 0, &name);

	PHALCON_CALL_METHODW(&is_put, getThis(), "isput");

	if (!zend_is_true(&is_put)) {
		put = phalcon_get_global_str(SL("_PUT"));
		PHALCON_CPY_WRT(&new_put, put);
	} else {
		put = phalcon_read_property(getThis(), SL("_put"), PH_NOISY);
		if (Z_TYPE_P(put) != IS_ARRAY) {
			PHALCON_CALL_METHODW(&raw, getThis(), "getrawbody");

			array_init(&new_put);

			PHALCON_ENSURE_IS_STRING(&raw);
			tmp = estrndup(Z_STRVAL(raw), Z_STRLEN(raw));
			sapi_module.treat_data(PARSE_STRING, tmp, &new_put);

			phalcon_update_property_this(getThis(), SL("_put"), &new_put);
		} else {
			PHALCON_CPY_WRT(&new_put, put);
		}
	}

	RETVAL_BOOL(phalcon_array_isset(&new_put, name));
}

/**
 * Checks whether $_GET superglobal has certain index
 *
 * @param string $name
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, hasQuery){

	zval *name, *_GET;

	phalcon_fetch_params(0, 1, 0, &name);

	_GET = phalcon_get_global_str(SL("_GET"));
	RETURN_BOOL(phalcon_array_isset(_GET, name));
}

/**
 * Checks whether $_SERVER superglobal has certain index
 *
 * @param string $name
 * @return mixed
 */
PHP_METHOD(Phalcon_Http_Request, hasServer){

	zval *name, *_SERVER;

	phalcon_fetch_params(0, 1, 0, &name);

	_SERVER = phalcon_get_global_str(SL("_SERVER"));
	RETURN_BOOL(phalcon_array_isset(_SERVER, name));
}

/**
 * Checks whether $_SERVER superglobal has certain index
 *
 * @param string $header
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, hasHeader){

	zval *header, *_SERVER, key;

	phalcon_fetch_params(0, 1, 0, &header);

	_SERVER = phalcon_get_global_str(SS("_SERVER") TSRMLS_CC);
	if (phalcon_array_isset(_SERVER, header)) {
		RETURN_TRUE;
	}

	PHALCON_CONCAT_SV(&key, "HTTP_", header);
	if (phalcon_array_isset(_SERVER, &key)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Gets HTTP header from request data
 *
 * @param string $header
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getHeader)
{
	zval *header, *_SERVER, key;

	phalcon_fetch_params(0, 1, 0, &header);

	_SERVER = phalcon_get_global_str(SL("_SERVER"));
	if (!phalcon_array_isset_fetch(return_value, _SERVER, header)) {
		PHALCON_CONCAT_SV(&key, "HTTP_", header);
		if (phalcon_array_isset_fetch(return_value, _SERVER, &key)) {
			return;
		}
	}

	RETURN_EMPTY_STRING();
}

/**
 * Gets HTTP schema (http/https)
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getScheme){

	zval https_header, https;

	PHALCON_STR(&https_header, "HTTPS");

	PHALCON_CALL_METHODW(&https, getThis(), "getserver", &https_header);
	if (zend_is_true(&https)) {
		if (PHALCON_IS_STRING(&https, "off")) {
			RETVAL_STRING("http");
		} else {
			RETVAL_STRING("https");
		}
	} else {
		RETVAL_STRING("http");
	}
}

/**
 * Checks whether request has been made using ajax. Checks if $_SERVER['HTTP_X_REQUESTED_WITH']=='XMLHttpRequest'
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, isAjax)
{
	zval requested_header, xml_http_request, requested_with;

	PHALCON_STR(&requested_header, "HTTP_X_REQUESTED_WITH");
	PHALCON_STR(&xml_http_request, "XMLHttpRequest");

	PHALCON_CALL_METHODW(&requested_with, getThis(), "getheader", &requested_header);
	is_equal_function(return_value, &requested_with, &xml_http_request);
	return;
}

/**
 * Checks whether request has been made using SOAP
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, isSoapRequested)
{
	zval *server, content_type;

	server = phalcon_get_global_str(SL("_SERVER"));
	if (phalcon_array_isset_str(server, SL("HTTP_SOAPACTION"))) {
		RETURN_TRUE;
	}

	if (phalcon_array_isset_fetch_str(&content_type, server, SL("CONTENT_TYPE"))) {
		if (phalcon_memnstr_str(&content_type, SL("application/soap+xml"))) {
			RETURN_TRUE;
		}
	}

	RETURN_FALSE;
}

/**
 * Checks whether request has been made using any secure layer
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, isSecureRequest)
{
	zval scheme, https;

	PHALCON_CALL_METHODW(&scheme, getThis(), "getscheme");

	PHALCON_STR(&https, "https");
	is_identical_function(return_value, &https, &scheme);
}

/**
 * Gets HTTP raw request body
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getRawBody)
{
	zval *raw;

	raw = phalcon_read_property(getThis(), SL("_rawBody"), PH_NOISY);
	if (Z_TYPE_P(raw) == IS_STRING) {
		RETURN_CTORW(raw);
	}

	zval *zcontext = NULL;
	php_stream_context *context = php_stream_context_from_zval(zcontext, 0);
	php_stream *stream = php_stream_open_wrapper_ex("php://input", "rb", REPORT_ERRORS, NULL, context);
	zend_string *content;
	long int maxlen    = PHP_STREAM_COPY_ALL;

	if (!stream) {
		RETURN_FALSE;
	}

	content = php_stream_copy_to_mem(stream, maxlen, 0);
	if (content != NULL) {
		RETVAL_STR(content);
		phalcon_update_property_this(getThis(), SL("_rawBody"), return_value);
	} else {
		RETVAL_FALSE;
	}

	php_stream_close(stream);
}

/**
 * Gets decoded JSON HTTP raw request body
 *
 * @param bool $assoc
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getJsonRawBody)
{
	zval raw_body, *assoc = NULL;
	int ac = 0;

	phalcon_fetch_params(0, 0, 1, &assoc);

	if (assoc && zend_is_true(assoc)) {
		ac = 1;
	}

	PHALCON_CALL_METHODW(&raw_body, getThis(), "getrawbody");
	if (Z_TYPE(raw_body) == IS_STRING) {
		RETURN_ON_FAILURE(phalcon_json_decode(return_value, &raw_body, ac));
		return;
	}
}

/**
 * Gets decoded BSON HTTP raw request body
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getBsonRawBody)
{
	zval raw_body;

	PHALCON_CALL_METHODW(&raw_body, getThis(), "getrawbody");
	if (Z_TYPE(raw_body) == IS_STRING) {
		PHALCON_RETURN_CALL_FUNCTION("bson_decode", &raw_body);
	}
}

/**
 * Gets active server address IP
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getServerAddress){

	zval *server, server_addr;

	server = phalcon_get_global_str(SL("_SERVER"));
	if (phalcon_array_isset_fetch_str(&server_addr, server, SL("SERVER_ADDR"))) {
		RETURN_CTORW(&server_addr);
	}

	RETURN_STRING("127.0.0.1");
}

/**
 * Gets active server name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getServerName){

	zval *server, server_name;

	server = phalcon_get_global_str(SL("_SERVER"));
	if (phalcon_array_isset_fetch_str(&server_name, server, SL("SERVER_NAME"))) {
		RETURN_CTORW(&server_name);
	}

	RETURN_STRING("localhost");
}

/**
 * Gets information about schema, host and port used by the request
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getHttpHost)
{
	zval host, http_host, scheme, server_name, name, server_port, port, http, standard_port, is_std_name, is_std_port, is_std_http;
	zval https, secure_port, is_secure_scheme, is_secure_port, is_secure_http;

	/**
	 * Get the server name from _SERVER['HTTP_HOST']
	 */
	PHALCON_STR(&host, "HTTP_HOST");

	PHALCON_CALL_METHODW(&http_host, getThis(), "getserver", &host);

	if (zend_is_true(&http_host)) {
		RETURN_CTORW(&http_host);
	}

	/**
	 * Get current scheme
	 */
	PHALCON_CALL_METHODW(&scheme, getThis(), "getscheme");

	/**
	 * Get the server name from _SERVER['SERVER_NAME']
	 */
	PHALCON_STR(&server_name, "SERVER_NAME");

	PHALCON_CALL_METHODW(&name, getThis(), "getserver", &server_name);

	/**
	 * Get the server port from _SERVER['SERVER_PORT']
	 */
	PHALCON_STR(&server_port, "SERVER_PORT");

	PHALCON_CALL_METHODW(&port, getThis(), "getserver", &server_port);

	PHALCON_STR(&http, "http");
	ZVAL_LONG(&standard_port, 80);

	/** 
	 * Check if the request is a standard http
	 */
	is_equal_function(&is_std_name, &scheme, &http);
	is_equal_function(&is_std_port, &port, &standard_port);

	phalcon_and_function(&is_std_http, &is_std_name, &is_std_port);

	PHALCON_STR(&https, "https");
	ZVAL_LONG(&secure_port, 443);

	/** 
	 * Check if the request is a secure http request
	 */
	is_equal_function(&is_secure_scheme, &scheme, &https);
	is_equal_function(&is_secure_port, &port, &secure_port);
	phalcon_and_function(&is_secure_http, &is_secure_scheme, &is_secure_port);

	/** 
	 * If is standard http we return the server name only
	 */
	if (PHALCON_IS_TRUE(&is_std_http)) {
		RETURN_CTORW(&name);
	}

	/** 
	 * If is standard secure http we return the server name only
	 */
	if (PHALCON_IS_TRUE(&is_secure_http)) {
		RETURN_CTORW(&name);
	}

	PHALCON_CONCAT_VSV(return_value, &name, ":", &port);

	return;
}

/**
 * Gets most possible client IPv4 Address. This method search in $_SERVER['REMOTE_ADDR'] and optionally in $_SERVER['HTTP_X_FORWARDED_FOR']
 *
 * @param boolean $trustForwardedHeader
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getClientAddress){

	zval *trust_forwarded_header = NULL, *_SERVER, address, addresses, first;

	phalcon_fetch_params(0, 0, 1, &trust_forwarded_header);

	if (!trust_forwarded_header) {
		trust_forwarded_header = &PHALCON_GLOBAL(z_false);
	}

	_SERVER = phalcon_get_global_str(SL("_SERVER"));

	/** 
	 * Proxies use this IP
	 */
	if (zend_is_true(trust_forwarded_header)) {
		if (!phalcon_array_isset_fetch_str(&address, _SERVER, SL("HTTP_X_FORWARDED_FOR"))) {
			if (!phalcon_array_isset_fetch_str(&address, _SERVER, SL("REMOTE_ADDR"))) {
				phalcon_array_fetch_str(&address, _SERVER, SL("REMOTE_ADDR"), PH_NOISY);
			}
		}
	} else if (!phalcon_array_isset_fetch_str(&address, _SERVER, SL("REMOTE_ADDR"))) {
		phalcon_array_fetch_str(&address, _SERVER, SL("REMOTE_ADDR"), PH_NOISY);
	}

	if (Z_TYPE(address) == IS_STRING) {
		if (phalcon_memnstr_str(&address, SL(","))) {
			/** 
			 * The client address has multiples parts, only return the first part
			 */
			phalcon_fast_explode_str(&addresses, SL(","), &address);

			phalcon_array_fetch_long(&first, &addresses, 0, PH_NOISY);
			RETURN_CTORW(&first);
		}

		RETURN_CTORW(&address);
	}

	RETURN_FALSE;
}

static const char* phalcon_http_request_getmethod_helper()
{
	zval *value;
	const char *method = SG(request_info).request_method;
	if (unlikely(!method)) {
		zval *_SERVER, key;
		PHALCON_STR(&key, "REQUEST_METHOD");

		_SERVER = phalcon_get_global_str(SL("_SERVER"));
		if (Z_TYPE_P(_SERVER) == IS_ARRAY) {
			value = phalcon_hash_get(Z_ARRVAL_P(_SERVER), &key, BP_VAR_UNSET);
			if (value && Z_TYPE_P(value) == IS_STRING) {
				return Z_STRVAL_P(value);
			}
		}

		return "";
	}

	return method;
}

/**
 * Gets HTTP method which request has been made
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getMethod){

	const char *method = phalcon_http_request_getmethod_helper();
	if (method) {
		RETURN_STRING(method);
	}

	RETURN_EMPTY_STRING();
}

/**
 * Gets HTTP URI which request has been made
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getURI){

	zval *value, *_SERVER, key;

	PHALCON_STR(&key, "REQUEST_URI");

	_SERVER = phalcon_get_global_str(SL("_SERVER"));
	value = (Z_TYPE_P(_SERVER) == IS_ARRAY) ? phalcon_hash_get(Z_ARRVAL_P(_SERVER), &key, BP_VAR_UNSET) : NULL;
	if (value && Z_TYPE_P(value) == IS_STRING) {
		RETURN_ZVAL(value, 1, 0);
	}

	RETURN_EMPTY_STRING();
}

/**
 * Gets query string which request has been made
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getQueryString){

	zval *value, *_SERVER, key;

	PHALCON_STR(&key, "QUERY_STRING");

	_SERVER = phalcon_get_global_str(SS("_SERVER") TSRMLS_CC);
	value = (Z_TYPE_P(_SERVER) == IS_ARRAY) ? phalcon_hash_get(Z_ARRVAL_P(_SERVER), &key, BP_VAR_UNSET) : NULL;
	if (value && Z_TYPE_P(value) == IS_STRING) {
		RETURN_ZVAL(value, 1, 0);
	}

	RETURN_EMPTY_STRING();
}

/**
 * Gets HTTP user agent used to made the request
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getUserAgent){

	zval *server, user_agent;

	server = phalcon_get_global_str(SL("_SERVER"));
	if (phalcon_array_isset_fetch_str(&user_agent, server, SL("HTTP_USER_AGENT"))) {
		RETURN_CTORW(&user_agent);
	}

	RETURN_EMPTY_STRING();
}

/**
 * Check if HTTP method match any of the passed methods
 *
 * @param string|array $methods
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, isMethod){

	zval *methods, http_method, *method;

	phalcon_fetch_params(0, 1, 0, &methods);

	PHALCON_CALL_METHODW(&http_method, getThis(), "getmethod");

	if (Z_TYPE_P(methods) == IS_STRING) {
		is_equal_function(return_value, methods, &http_method);
		return;
	}

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(methods), method) {
		if (PHALCON_IS_EQUAL(method, &http_method)) {
			RETURN_TRUE;
		}
	} ZEND_HASH_FOREACH_END();

	RETURN_FALSE;
}

/**
 * Checks whether HTTP method is POST. if $_SERVER['REQUEST_METHOD']=='POST'
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, isPost){

	zval post, method;

	if (Z_OBJCE_P(getThis()) == phalcon_http_request_ce) {
		RETURN_BOOL(!strcmp(phalcon_http_request_getmethod_helper(), "POST"));
	}

	PHALCON_STR(&post, ISV(POST));

	PHALCON_CALL_METHODW(&method, getThis(), "getmethod");
	is_equal_function(return_value, &method, &post);
}

/**
 * Checks whether HTTP method is GET. if $_SERVER['REQUEST_METHOD']=='GET'
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, isGet){

	zval get, method;

	if (Z_OBJCE_P(getThis()) == phalcon_http_request_ce) {
		RETURN_BOOL(!strcmp(phalcon_http_request_getmethod_helper(), "GET"));
	}

	PHALCON_STR(&get, ISV(GET));

	PHALCON_CALL_METHODW(&method, getThis(), "getmethod");
	is_equal_function(return_value, &method, &get);
}

/**
 * Checks whether HTTP method is PUT. if $_SERVER['REQUEST_METHOD']=='PUT'
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, isPut){

	zval put, method;

	if (Z_OBJCE_P(getThis()) == phalcon_http_request_ce) {
		RETURN_BOOL(!strcmp(phalcon_http_request_getmethod_helper(), "PUT"));
	}

	PHALCON_STR(&put, ISV(PUT));

	PHALCON_CALL_METHODW(&method, getThis(), "getmethod");
	is_equal_function(return_value, &method, &put);
}

/**
 * Checks whether HTTP method is PATCH. if $_SERVER['REQUEST_METHOD']=='PATCH'
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, isPatch){

	zval patch, method;

	if (Z_OBJCE_P(getThis()) == phalcon_http_request_ce) {
		RETURN_BOOL(!strcmp(phalcon_http_request_getmethod_helper(), "PATCH"));
	}

	PHALCON_STR(&patch, ISV(PATCH));

	PHALCON_CALL_METHODW(&method, getThis(), "getmethod");
	is_equal_function(return_value, &method, &patch);
}

/**
 * Checks whether HTTP method is HEAD. if $_SERVER['REQUEST_METHOD']=='HEAD'
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, isHead){

	zval head, method;

	if (Z_OBJCE_P(getThis()) == phalcon_http_request_ce) {
		RETURN_BOOL(!strcmp(phalcon_http_request_getmethod_helper(), "HEAD"));
	}

	PHALCON_STR(&head, ISV(HEAD));

	PHALCON_CALL_METHODW(&method, getThis(), "getmethod");
	is_equal_function(return_value, &method, &head);
}

/**
 * Checks whether HTTP method is DELETE. if $_SERVER['REQUEST_METHOD']=='DELETE'
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, isDelete){

	zval delete, method;

	if (Z_OBJCE_P(getThis()) == phalcon_http_request_ce) {
		RETURN_BOOL(!strcmp(phalcon_http_request_getmethod_helper(), "DELETE"));
	}

	PHALCON_STR(&delete, ISV(DELETE));

	PHALCON_CALL_METHODW(&method, getThis(), "getmethod");
	is_equal_function(return_value, &method, &delete);
}

/**
 * Checks whether HTTP method is OPTIONS. if $_SERVER['REQUEST_METHOD']=='OPTIONS'
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, isOptions){

	zval options, method;

	if (Z_OBJCE_P(getThis()) == phalcon_http_request_ce) {
		RETURN_BOOL(!strcmp(phalcon_http_request_getmethod_helper(), "OPTIONS"));
	}

	PHALCON_CALL_METHODW(&method, getThis(), "getmethod");
	is_equal_function(return_value, &method, &options);
}

static int phalcon_http_request_hasfiles_helper(zval *arr, int only_successful)
{
	zval *value;
	int nfiles = 0;

	assert(Z_TYPE_P(arr) == IS_ARRAY);
	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(arr), value) {
		if (Z_TYPE_P(value) < IS_ARRAY) {
			if (!zend_is_true(value) || !only_successful) {
				++nfiles;
			}
		} else if (Z_TYPE_P(value) == IS_ARRAY) {
			nfiles += phalcon_http_request_hasfiles_helper(value, only_successful);
		}
	} ZEND_HASH_FOREACH_END();

	return nfiles;
}

/**
 * Checks whether request includes attached files
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, hasFiles){

	zval *not_errored = NULL, *_FILES;
	zval *file;
	int nfiles = 0;
	int only_successful;

	phalcon_fetch_params(0, 0, 1, &not_errored);

	only_successful = not_errored ? phalcon_get_intval(not_errored) : 1;

	_FILES = phalcon_get_global_str(SL("_FILES"));
	if (unlikely(Z_TYPE_P(_FILES) != IS_ARRAY)) {
		RETURN_LONG(0);
	}

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(_FILES), file) {
		zval error;
		if (phalcon_array_isset_fetch_str(&error, file, SL("error"))) {
			if (Z_TYPE_P(&error) < IS_ARRAY) {
				if (!zend_is_true(&error) || !only_successful) {
					++nfiles;
				}
			} else if (Z_TYPE_P(&error) == IS_ARRAY) {
				nfiles += phalcon_http_request_hasfiles_helper(&error, only_successful);
			}
		}
	} ZEND_HASH_FOREACH_END();

	RETURN_LONG(nfiles);
}

static void phalcon_http_request_getuploadedfiles_helper(zval *retval, zval *name, zval *type, zval *tmp_name, zval *error, zval *size, int only_successful, smart_str *prefix)
{
	if (
		   Z_TYPE_P(name) == IS_ARRAY && Z_TYPE_P(type) == IS_ARRAY
		&& Z_TYPE_P(tmp_name) == IS_ARRAY && Z_TYPE_P(error) == IS_ARRAY
		&& Z_TYPE_P(size) == IS_ARRAY
	) {
		HashPosition pos_name, pos_type, pos_tmp, pos_error, pos_size;
		zval *dname, *dtype, *dtmp, *derror, *dsize;

		zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(name),     &pos_name);
		zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(type),     &pos_type);
		zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(tmp_name), &pos_tmp);
		zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(error),    &pos_error);
		zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(size),     &pos_size);

		while (
			(dname = zend_hash_get_current_data_ex(Z_ARRVAL_P(name), &pos_name)) != NULL &&
			(dtype = zend_hash_get_current_data_ex(Z_ARRVAL_P(type), &pos_type)) != NULL && 
			(dtmp = zend_hash_get_current_data_ex(Z_ARRVAL_P(tmp_name), &pos_tmp)) != NULL && 
			(derror = zend_hash_get_current_data_ex(Z_ARRVAL_P(error), &pos_error)) != NULL && 
			(dsize = zend_hash_get_current_data_ex(Z_ARRVAL_P(size), &pos_size)) != NULL
		) {
			zval *index, arr, key, file, *params[2];
			int res;

			index = phalcon_get_current_key_w(Z_ARRVAL_P(name), &pos_name);

			if (Z_TYPE_P(index) == IS_STRING) {
				smart_str_appendl(prefix, Z_STRVAL_P(index), Z_STRLEN_P(index));
			} else {
				smart_str_append_long(prefix, Z_LVAL_P(index));
			}

			if (Z_TYPE_P(derror) < IS_ARRAY) {
				if (!zend_is_true(derror) || !only_successful) {
					Z_TRY_ADDREF_P(dname);
					Z_TRY_ADDREF_P(dtype);
					Z_TRY_ADDREF_P(dtmp);
					Z_TRY_ADDREF_P(derror);
					Z_TRY_ADDREF_P(dsize);

					array_init_size(&arr, 5);
					add_assoc_zval_ex(&arr, ISL(name),      dname);
					add_assoc_zval_ex(&arr, ISL(type),      dtype);
					add_assoc_zval_ex(&arr, SL("tmp_name"), dtmp);
					add_assoc_zval_ex(&arr, SL("error"),    derror);
					add_assoc_zval_ex(&arr, SL("size"),     dsize);

					ZVAL_STR(&key, prefix->s);

					object_init_ex(&file, phalcon_http_request_file_ce);

					params[0] = &arr;
					params[0] = &key;

					res = phalcon_call_method(NULL, &file, "__construct", 2, params);

					zval_ptr_dtor(&arr);
					zval_ptr_dtor(&key);

					if (res != FAILURE) {
						add_next_index_zval(retval, &file);
					} else {
						break;
					}
				}
			} else if (Z_TYPE_P(derror) == IS_ARRAY) {
				smart_str_appendc(prefix, '.');
				phalcon_http_request_getuploadedfiles_helper(retval, dname, dtype, dtmp, derror, dsize, only_successful, prefix);
			}

			zend_hash_move_forward_ex(Z_ARRVAL_P(name),     &pos_name);
			zend_hash_move_forward_ex(Z_ARRVAL_P(type),     &pos_type);
			zend_hash_move_forward_ex(Z_ARRVAL_P(tmp_name), &pos_tmp);
			zend_hash_move_forward_ex(Z_ARRVAL_P(error),    &pos_error);
			zend_hash_move_forward_ex(Z_ARRVAL_P(size),     &pos_size);
		}
	}
}

/**
 * Gets attached files as Phalcon\Http\Request\File instances
 *
 * @param boolean $notErrored
 * @param string $index
 * @return Phalcon\Http\Request\File[]
 */
PHP_METHOD(Phalcon_Http_Request, getUploadedFiles){

	zval *dst_index = NULL, *not_errored = NULL, *_FILES, *value;
	zend_string *str_key;
	ulong idx;
	int only_successful;
	smart_str prefix = { 0 };

	phalcon_fetch_params(0, 0, 2, &not_errored, &dst_index);

	only_successful = not_errored ? phalcon_get_intval(not_errored) : 1;

	array_init(return_value);

	_FILES = phalcon_get_global_str(SL("_FILES"));
	if (Z_TYPE_P(_FILES) != IS_ARRAY || !zend_hash_num_elements(Z_ARRVAL_P(_FILES))) {
		return;
	}

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(_FILES), idx, str_key, value) {
		zval index, error, request_file, name, type, tmp_name, size;
		if (str_key) {
			ZVAL_STR(&index, str_key);
		} else {
			ZVAL_LONG(&index, idx);
		}

		if (dst_index && !PHALCON_IS_EQUAL(dst_index, &index)) {
			continue;
		}

		if (phalcon_array_isset_fetch_str(&error, value, SL("error"))) {
			if (Z_TYPE_P(&error) < IS_ARRAY) {				
				if (!zend_is_true(&error) || !only_successful) {
					object_init_ex(&request_file, phalcon_http_request_file_ce);

					PHALCON_CALL_METHODW(NULL, &request_file, "__construct", value, &index);

					phalcon_array_append(return_value, &request_file, PH_COPY);
				}
			} else if (Z_TYPE(error) == IS_ARRAY) {
				phalcon_array_fetch_str(&name, value, SL("name"), PH_NOISY);
				phalcon_array_fetch_str(&type, value, SL("type"), PH_NOISY);
				phalcon_array_fetch_str(&tmp_name, value, SL("tmp_name"), PH_NOISY);
				phalcon_array_fetch_str(&size, value, SL("size"), PH_NOISY);

				if (likely(Z_TYPE(index) == IS_STRING)) {
					smart_str_appendl(&prefix, Z_STRVAL(index), Z_STRLEN(index));
				} else {
					smart_str_append_long(&prefix, Z_LVAL(index));
				}

				smart_str_appendc(&prefix, '.');
				phalcon_http_request_getuploadedfiles_helper(return_value, &name, &type, &tmp_name, &error, &size, only_successful, &prefix);
			}
		}
	} ZEND_HASH_FOREACH_END();

	smart_str_free(&prefix);
}

/**
 * Returns the available headers in the request
 *
 * @return array
 */
PHP_METHOD(Phalcon_Http_Request, getHeaders){

	zval *_SERVER, *value;
	zend_string *str_key;

	array_init(return_value);
	_SERVER = phalcon_get_global_str(SL("_SERVER"));
	if (unlikely(Z_TYPE_P(_SERVER) != IS_ARRAY)) {
		return;
	}

	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(_SERVER), str_key, value) {
		if (str_key && ZSTR_LEN(str_key) > 5 && !memcmp(ZSTR_VAL(str_key), "HTTP_", 5)) {
			zval header;
			PHALCON_STRL(&header, ZSTR_VAL(str_key) + 5, ZSTR_LEN(str_key) - 5);
			phalcon_array_update_zval(return_value, &header, value, PH_COPY);
		}
	} ZEND_HASH_FOREACH_END();
}

/**
 * Gets web page that refers active request. ie: http://www.google.com
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getHTTPReferer){

	zval *_SERVER, http_referer;

	_SERVER = phalcon_get_global_str(SL("_SERVER"));
	if (phalcon_array_isset_fetch_str(&http_referer, _SERVER, SL("HTTP_REFERER"))) {
		RETURN_CTORW(&http_referer);
	}

	RETURN_EMPTY_STRING();
}

/**
 * Process a request header and return an array of values with their qualities
 *
 * @param string $serverIndex
 * @param string $name
 * @return array
 */
PHP_METHOD(Phalcon_Http_Request, _getQualityHeader){

	zval *server_index, *name, quality_one, http_server, pattern, parts, *part;

	phalcon_fetch_params(0, 2, 0, &server_index, &name);

	ZVAL_DOUBLE(&quality_one, 1);

	array_init(return_value);

	PHALCON_CALL_METHODW(&http_server, getThis(), "getserver", server_index);

	PHALCON_STR(&pattern, "/,\\s*/");
	PHALCON_CALL_FUNCTIONW(&parts, "preg_split", &pattern, &http_server);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(parts), part) {
		zval header_parts, quality_part, quality, header_name;
		phalcon_fast_explode_str(&header_parts, SL(";"), part);
		if (phalcon_array_isset_fetch_long(&quality_part, &header_parts, 1)) {
			phalcon_substr(&quality, &quality_part, 2, 0);
		} else {
			PHALCON_CPY_WRT_CTOR(&quality, &quality_one);
		}

		phalcon_array_fetch_long(&header_name, &header_parts, 0, PH_NOISY);

		array_init_size(&quality_part, 2);
		phalcon_array_update_zval(&quality_part, name, &header_name, PH_COPY);
		phalcon_array_update_str(&quality_part, SL("quality"), &quality, PH_COPY);

		phalcon_array_append(return_value, &quality_part, PH_COPY);
	} ZEND_HASH_FOREACH_END();
}

/**
 * Process a request header and return the one with best quality
 *
 * @param array $qualityParts
 * @param string $name
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, _getBestQuality){

	zval *quality_parts, *name, *accept, quality, selected_name;
	long int i = 0;

	phalcon_fetch_params(0, 2, 0, &quality_parts, &name);

	ZVAL_LONG(&quality, 0);

	ZVAL_EMPTY_STRING(&selected_name);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(quality_parts), accept) {
		zval selected_name, accept_quality, best_quality;
		if (i == 0) {
			phalcon_array_fetch_str(&quality, accept, SL("quality"), PH_NOISY);
			phalcon_array_fetch(&selected_name, accept, name, PH_NOISY);
		} else {
			phalcon_array_fetch_str(&accept_quality, accept, SL("quality"), PH_NOISY);

			is_smaller_function(&best_quality, &quality, &accept_quality);
			if (PHALCON_IS_TRUE(&best_quality)) {
				PHALCON_CPY_WRT_CTOR(&quality, &accept_quality);
				phalcon_array_fetch(&selected_name, accept, name, PH_NOISY);
			}
		}

		++i;
	} ZEND_HASH_FOREACH_END();

	RETURN_CTORW(&selected_name);
}

/**
 * Gets array with mime/types and their quality accepted by the browser/client from $_SERVER['HTTP_ACCEPT']
 *
 * @return array
 */
PHP_METHOD(Phalcon_Http_Request, getAcceptableContent){

	zval accept_header, quality_index;

	PHALCON_STR(&accept_header, "HTTP_ACCEPT");
	PHALCON_STR(&quality_index, "accept");

	PHALCON_RETURN_CALL_METHOD(getThis(), "_getqualityheader", &accept_header, &quality_index);
}

/**
 * Gets best mime/type accepted by the browser/client from $_SERVER['HTTP_ACCEPT']
 *
 * @return array
 */
PHP_METHOD(Phalcon_Http_Request, getBestAccept){

	zval quality_index, acceptable_content;

	PHALCON_STR(&quality_index, "accept");

	PHALCON_CALL_METHODW(&acceptable_content, getThis(), "getacceptablecontent");
	PHALCON_RETURN_CALL_METHOD(getThis(), "_getbestquality", &acceptable_content, &quality_index);
}

/**
 * Gets charsets array and their quality accepted by the browser/client from $_SERVER['HTTP_ACCEPT_CHARSET']
 *
 * @return array
 */
PHP_METHOD(Phalcon_Http_Request, getClientCharsets){

	zval charset_header, quality_index;

	PHALCON_STR(&charset_header, "HTTP_ACCEPT_CHARSET");
	PHALCON_STR(&quality_index, "charset");

	PHALCON_RETURN_CALL_METHOD(getThis(), "_getqualityheader", &charset_header, &quality_index);
}

/**
 * Gets best charset accepted by the browser/client from $_SERVER['HTTP_ACCEPT_CHARSET']
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getBestCharset){

	zval quality_index, client_charsets;

	PHALCON_STR(&quality_index, "charset");

	PHALCON_CALL_METHODW(&client_charsets, getThis(), "getclientcharsets");
	PHALCON_RETURN_CALL_METHOD(getThis(), "_getbestquality", &client_charsets, &quality_index);
}

/**
 * Gets languages array and their quality accepted by the browser/client from $_SERVER['HTTP_ACCEPT_LANGUAGE']
 *
 * @return array
 */
PHP_METHOD(Phalcon_Http_Request, getLanguages){

	zval language_header, quality_index;

	PHALCON_STR(&language_header, "HTTP_ACCEPT_LANGUAGE");
	PHALCON_STR(&quality_index, "language");

	PHALCON_RETURN_CALL_METHOD(getThis(), "_getqualityheader", &language_header, &quality_index);
}

/**
 * Gets best language accepted by the browser/client from $_SERVER['HTTP_ACCEPT_LANGUAGE']
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getBestLanguage){

	zval languages, quality_index;

	PHALCON_CALL_METHODW(&languages, getThis(), "getlanguages");

	PHALCON_STR(&quality_index, "language");
	PHALCON_RETURN_CALL_METHOD(getThis(), "_getbestquality", &languages, &quality_index);
}

/**
 * Gets auth info accepted by the browser/client from $_SERVER['PHP_AUTH_USER']
 *
 * @return array
 */
PHP_METHOD(Phalcon_Http_Request, getBasicAuth){

	zval *_SERVER;
	zval *value;
	char *auth_user = SG(request_info).auth_user;
	char *auth_password = SG(request_info).auth_password;

	if (unlikely(!auth_user)) {
		_SERVER = phalcon_get_global_str(SL("_SERVER"));
		if (Z_TYPE_P(_SERVER) == IS_ARRAY) {
			zval key;

			PHALCON_STR(&key, "PHP_AUTH_USER");

			value = phalcon_hash_get(Z_ARRVAL_P(_SERVER), &key, BP_VAR_UNSET);
			if (value && Z_TYPE_P(value) == IS_STRING) {
				auth_user = Z_STRVAL_P(value);
			}

			PHALCON_STR(&key, "PHP_AUTH_PW");

			value = phalcon_hash_get(Z_ARRVAL_P(_SERVER), &key, BP_VAR_UNSET);
			if (value && Z_TYPE_P(value) == IS_STRING) {
				auth_password = Z_STRVAL_P(value);
			}
		}
	}

	if (!auth_user) {
		RETURN_NULL();
	}

	if (!auth_password) {
		auth_password = "";
	}

	array_init_size(return_value, 2);
	phalcon_array_update_str_str(return_value, SL("username"), auth_user, strlen(auth_user), PH_COPY);
	phalcon_array_update_str_str(return_value, SL("password"), auth_password, strlen(auth_password), PH_COPY);
}

/**
 * Gets auth info accepted by the browser/client from $_SERVER['PHP_AUTH_DIGEST']
 *
 * @return array
 */
PHP_METHOD(Phalcon_Http_Request, getDigestAuth){

	zval *_SERVER, key, *value, pattern, digest, set_order, matches, ret, *match;
	const char *auth_digest = SG(request_info).auth_digest;

	if (unlikely(!auth_digest)) {
		_SERVER = phalcon_get_global_str(SL("_SERVER"));
		if (Z_TYPE_P(_SERVER) == IS_ARRAY) {
			PHALCON_STR(&key, "PHP_AUTH_DIGEST");

			value = phalcon_hash_get(Z_ARRVAL_P(_SERVER), &key, BP_VAR_UNSET);
			if (value && Z_TYPE_P(value) == IS_STRING) {
				auth_digest = Z_STRVAL_P(value);
			}
		}
	}

	if (auth_digest) {
		PHALCON_STR(&pattern, "#(\\w+)=(['\"]?)([^'\", ]+)\\2#");
		PHALCON_STR(&digest, auth_digest);
		ZVAL_LONG(&set_order, 2);

		ZVAL_MAKE_REF(&matches);
		PHALCON_CALL_FUNCTIONW(&ret, "preg_match_all", &pattern, &digest, &matches, &set_order);
		ZVAL_UNREF(&matches);

		if (zend_is_true(&ret) && Z_TYPE(matches) == IS_ARRAY) {
			array_init(return_value);

			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(matches), match) {
				zval tmp1, tmp2;
				if (Z_TYPE_P(match) == IS_ARRAY && phalcon_array_isset_fetch_long(&tmp1, match, 1) && phalcon_array_isset_fetch_long(&tmp2, match, 3)) {
					phalcon_array_update_zval(return_value, &tmp1, &tmp2, PH_COPY);
				}
			} ZEND_HASH_FOREACH_END();

			return;
		}
	}

	RETURN_NULL();
}
