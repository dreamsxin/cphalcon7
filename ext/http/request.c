
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

PHP_METHOD(Phalcon_Http_Request, __construct);
PHP_METHOD(Phalcon_Http_Request, _get);
PHP_METHOD(Phalcon_Http_Request, get);
PHP_METHOD(Phalcon_Http_Request, getPost);
PHP_METHOD(Phalcon_Http_Request, getPut);
PHP_METHOD(Phalcon_Http_Request, getQuery);
PHP_METHOD(Phalcon_Http_Request, getServer);
PHP_METHOD(Phalcon_Http_Request, getEnv);
PHP_METHOD(Phalcon_Http_Request, getParam);
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_request__get, 0, 0, 6)
	ZEND_ARG_TYPE_INFO(0, data, IS_ARRAY, 1)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, filters)
	ZEND_ARG_INFO(0, defaultValue)
	ZEND_ARG_INFO(0, notAllowEmpty)
	ZEND_ARG_INFO(0, recursive_level)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_http_request_method_entry[] = {
	PHP_ME(Phalcon_Http_Request, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Http_Request, _get, arginfo_phalcon_http_request__get, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Http_Request, get, arginfo_phalcon_http_requestinterface_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getPost, arginfo_phalcon_http_requestinterface_getpost, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getPut, arginfo_phalcon_http_requestinterface_getput, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getQuery, arginfo_phalcon_http_requestinterface_getquery, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getServer, arginfo_phalcon_http_requestinterface_getserver, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getEnv, arginfo_phalcon_http_requestinterface_getenv, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request, getParam, arginfo_phalcon_http_requestinterface_getparam, ZEND_ACC_PUBLIC)
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

	zend_declare_property_null(phalcon_http_request_ce, SL("_rawBody"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_request_ce, SL("_put"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_request_ce, SL("_data"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_http_request_ce, 1, phalcon_http_requestinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Http\Request constructor
 */
PHP_METHOD(Phalcon_Http_Request, __construct){

	zval *data = NULL;

	phalcon_fetch_params(0, 0, 1, &data);

	if (!data || Z_TYPE_P(data) != IS_ARRAY) {
		phalcon_update_property_empty_array(getThis(), SL("_data"));
	} else {
		phalcon_update_property(getThis(), SL("_data"), data);
	}
}

/**
 * Internal get wrapper to filter
 *
 * @param string $name
 * @param string|array $filters
 * @param mixed $defaultValue
 * @param boolean $notAllowEmpty
 * @param int $recursiveLevel
 * @return mixed
 */
PHP_METHOD(Phalcon_Http_Request, _get)
{
	zval *data, *name, *filters, *default_value, *not_allow_empty, *recursive_level;
	zval value = {};

	phalcon_fetch_params(0, 6, 0, &data, &name, &filters, &default_value, &not_allow_empty, &recursive_level);

	if (Z_TYPE_P(name) != IS_NULL) {
		if (!phalcon_array_isset_fetch(&value, data, name, PH_READONLY)) {
			RETURN_CTOR(default_value);
		}
	} else {
		ZVAL_COPY_VALUE(&value, data);
	}

	if (Z_TYPE_P(filters) > IS_NULL) {
		zval service = {}, filter = {}, filter_value = {};
		ZVAL_STR(&service, IS(filter));

		PHALCON_CALL_METHOD(&filter, getThis(), "getservice", &service);
		PHALCON_VERIFY_INTERFACE(&filter, phalcon_filterinterface_ce);
		
		if (Z_TYPE_P(recursive_level) == IS_LONG) {
			PHALCON_CALL_METHOD(&filter_value, &filter, "sanitize", &value, filters, &PHALCON_GLOBAL(z_true), &PHALCON_GLOBAL(z_null), recursive_level);
		} else if (Z_TYPE_P(name) != IS_NULL && Z_TYPE_P(recursive_level) == IS_FALSE) {
			PHALCON_CALL_METHOD(&filter_value, &filter, "sanitize", &value, filters, &PHALCON_GLOBAL(z_false));
		} else {
			PHALCON_CALL_METHOD(&filter_value, &filter, "sanitize", &value, filters);
		}
		zval_ptr_dtor(&filter);

		if ((PHALCON_IS_EMPTY(&filter_value) && zend_is_true(not_allow_empty)) || PHALCON_IS_FALSE(&filter_value)) {
			RETURN_CTOR(default_value);
		}

		RETURN_NCTOR(&filter_value);
	}

	if (PHALCON_IS_EMPTY(&value) && zend_is_true(not_allow_empty)) {
		RETURN_CTOR(default_value);
	}

	RETURN_CTOR(&value);
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
 * @param int $recursiveLevel
 * @return mixed
 */
PHP_METHOD(Phalcon_Http_Request, get)
{
	zval *name = NULL, *filters = NULL, *default_value = NULL, *not_allow_empty = NULL, *recursive_level = NULL, *request;
	zval put = {}, merged = {}, data = {}, merged2 = {};

	phalcon_fetch_params(0, 0, 5, &name, &filters, &default_value, &not_allow_empty, &recursive_level);

	if (!name) {
		name = &PHALCON_GLOBAL(z_null);
	}

	if (!filters) {
		filters = &PHALCON_GLOBAL(z_null);
	}

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	if (!not_allow_empty || Z_TYPE_P(not_allow_empty) == IS_NULL) {
		not_allow_empty = &PHALCON_GLOBAL(z_false);
	}

	if (!recursive_level || Z_TYPE_P(recursive_level) == IS_NULL) {
		recursive_level = zend_is_true(name) ? &PHALCON_GLOBAL(z_false) : &PHALCON_GLOBAL(z_true);
	}

	request = phalcon_get_global_str(SL("_REQUEST"));

	PHALCON_CALL_METHOD(&put, getThis(), "getput");

	phalcon_fast_array_merge(&merged, request, &put);
	zval_ptr_dtor(&put);

	phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);

	phalcon_fast_array_merge(&merged2, &merged, &data);
	zval_ptr_dtor(&merged);

	PHALCON_RETURN_CALL_SELF("_get", &merged2, name, filters, default_value, not_allow_empty, recursive_level);
	zval_ptr_dtor(&merged2);
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
 * @param int $recursiveLevel
 * @return mixed
 */
PHP_METHOD(Phalcon_Http_Request, getPost)
{
	zval *name = NULL, *filters = NULL, *default_value = NULL, *not_allow_empty = NULL, *recursive_level = NULL, *post;

	phalcon_fetch_params(0, 0, 5, &name, &filters, &default_value, &not_allow_empty, &recursive_level);

	if (!name) {
		name = &PHALCON_GLOBAL(z_null);
	}

	if (!filters) {
		filters = &PHALCON_GLOBAL(z_null);
	}

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	if (!not_allow_empty || Z_TYPE_P(not_allow_empty) == IS_NULL) {
		not_allow_empty = &PHALCON_GLOBAL(z_false);
	}

	if (!recursive_level || Z_TYPE_P(recursive_level) == IS_NULL) {
		recursive_level = zend_is_true(name) ? &PHALCON_GLOBAL(z_false) : &PHALCON_GLOBAL(z_true);
	}

	post = phalcon_get_global_str(SL("_POST"));
	PHALCON_RETURN_CALL_SELF("_get", post, name, filters, default_value, not_allow_empty, recursive_level);
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
 * @param int $recursiveLevel
 * @return mixed
 */
PHP_METHOD(Phalcon_Http_Request, getPut)
{
	zval *name = NULL, *filters = NULL, *default_value = NULL, *not_allow_empty = NULL, *recursive_level = NULL, is_put = {}, put = {}, new_put = {};

	phalcon_fetch_params(0, 0, 5, &name, &filters, &default_value, &not_allow_empty, &recursive_level);

	if (!name) {
		name = &PHALCON_GLOBAL(z_null);
	}

	if (!filters) {
		filters = &PHALCON_GLOBAL(z_null);
	}

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	if (!not_allow_empty || Z_TYPE_P(not_allow_empty) == IS_NULL) {
		not_allow_empty = &PHALCON_GLOBAL(z_false);
	}

	if (!recursive_level || Z_TYPE_P(recursive_level) == IS_NULL) {
		recursive_level = zend_is_true(name) ? &PHALCON_GLOBAL(z_false) : &PHALCON_GLOBAL(z_true);
	}

	PHALCON_CALL_METHOD(&is_put, getThis(), "isput");

	if (!zend_is_true(&is_put)) {
		RETURN_EMPTY_ARRAY();
	} else {
		phalcon_read_property(&put, getThis(), SL("_put"), PH_NOISY|PH_READONLY);
		if (Z_TYPE(put) != IS_ARRAY) {
			zval raw = {};
			char *tmp;
			PHALCON_CALL_METHOD(&raw, getThis(), "getrawbody");
			PHALCON_ENSURE_IS_STRING(&raw);

			array_init(&new_put);

			tmp = estrndup(Z_STRVAL(raw), Z_STRLEN(raw));
			sapi_module.treat_data(PARSE_STRING, tmp, &new_put);
			efree(tmp);

			phalcon_update_property(getThis(), SL("_put"), &new_put);
			zval_ptr_dtor(&raw);
		} else {
			ZVAL_COPY(&new_put, &put);
		}
	}

	PHALCON_RETURN_CALL_SELF("_get", &new_put, name, filters, default_value, not_allow_empty, recursive_level);
	zval_ptr_dtor(&new_put);
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
 * @param int $recursiveLevel
 * @return mixed
 */
PHP_METHOD(Phalcon_Http_Request, getQuery){

	zval *name = NULL, *filters = NULL, *default_value = NULL, *not_allow_empty = NULL, *recursive_level = NULL, *get;

	phalcon_fetch_params(0, 0, 5, &name, &filters, &default_value, &not_allow_empty, &recursive_level);

	if (!name) {
		name = &PHALCON_GLOBAL(z_null);
	}

	if (!filters) {
		filters = &PHALCON_GLOBAL(z_null);
	}

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	if (!not_allow_empty || Z_TYPE_P(not_allow_empty) == IS_NULL) {
		not_allow_empty = &PHALCON_GLOBAL(z_false);
	}

	if (!recursive_level || Z_TYPE_P(recursive_level) == IS_NULL) {
		recursive_level = zend_is_true(name) ? &PHALCON_GLOBAL(z_false) : &PHALCON_GLOBAL(z_true);
	}

	get = phalcon_get_global_str(SL("_GET"));

	PHALCON_RETURN_CALL_SELF("_get", get, name, filters, default_value, not_allow_empty, recursive_level);
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
	if (!phalcon_array_isset_fetch(return_value, _SERVER, name, PH_COPY)) {
		RETURN_NULL();
	}
}

/**
 * Gets the value of an environment variable
 *
 * @param string $name
 * @return string|boolean
 */
PHP_METHOD(Phalcon_Http_Request, getEnv){

	zval *name;

	phalcon_fetch_params(0, 1, 0, &name);

	PHALCON_CALL_FUNCTION(return_value, "getenv", name);
}

/**
 * Gets a param by its name or numeric index
 *
 * @param  mixed $param
 * @param  string|array $filters
 * @param  mixed $defaultValue
 * @return mixed
 */
PHP_METHOD(Phalcon_Http_Request, getParam){

	zval *param, *filters = NULL, *default_value = NULL, dependency_injector = {}, service = {}, dispatcher = {};

	phalcon_fetch_params(1, 1, 2, &param, &filters, &default_value);

	if (!filters) {
		filters = &PHALCON_GLOBAL(z_null);
	}

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi");
	PHALCON_MM_ADD_ENTRY(&dependency_injector);
	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_http_request_exception_ce, "A dependency injection object is required to access the 'filter' service");
		return;
	}

	ZVAL_STR(&service, IS(dispatcher));

	PHALCON_MM_CALL_METHOD(&dispatcher, &dependency_injector, "getshared", &service);
	PHALCON_MM_ADD_ENTRY(&dispatcher);

	PHALCON_MM_CALL_METHOD(NULL, &dispatcher, "getparam", param, filters, default_value);
	RETURN_MM();
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
PHP_METHOD(Phalcon_Http_Request, hasPut)
{
	zval *name, is_put = {}, put = {}, raw = {}, new_put = {};
	char *tmp;

	phalcon_fetch_params(0, 1, 0, &name);

	PHALCON_CALL_METHOD(&is_put, getThis(), "isput");

	if (!zend_is_true(&is_put)) {
		phalcon_read_global_str(&new_put, SL("_PUT"));
	} else {
		phalcon_read_property(&put, getThis(), SL("_put"), PH_NOISY|PH_READONLY);
		if (Z_TYPE(put) != IS_ARRAY) {
			PHALCON_CALL_METHOD(&raw, getThis(), "getrawbody");

			array_init(&new_put);

			PHALCON_ENSURE_IS_STRING(&raw);
			tmp = estrndup(Z_STRVAL(raw), Z_STRLEN(raw));
			sapi_module.treat_data(PARSE_STRING, tmp, &new_put);

			phalcon_update_property(getThis(), SL("_put"), &new_put);
		} else {
			ZVAL_COPY_VALUE(&new_put, &put);
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

	zval *header, *_SERVER, key = {};

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
	zval *header, *_SERVER, key = {};

	phalcon_fetch_params(0, 1, 0, &header);

	_SERVER = phalcon_get_global_str(SL("_SERVER"));
	if (phalcon_array_isset_fetch(return_value, _SERVER, header, PH_COPY)) {
		return;
	}
	PHALCON_CONCAT_SV(&key, "HTTP_", header);
	if (phalcon_array_isset_fetch(return_value, _SERVER, &key, PH_COPY)) {
		zval_ptr_dtor(&key);
		return;
	}
	zval_ptr_dtor(&key);

	RETURN_EMPTY_STRING();
}

/**
 * Gets HTTP schema (http/https)
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getScheme){

	zval https_header = {}, https = {};

	ZVAL_STRING(&https_header, "HTTPS");

	PHALCON_CALL_METHOD(&https, getThis(), "getserver", &https_header);
	zval_ptr_dtor(&https_header);

	if (zend_is_true(&https)) {
		if (PHALCON_IS_STRING(&https, "off")) {
			RETVAL_STRING("http");
		} else {
			RETVAL_STRING("https");
		}
	} else {
		RETVAL_STRING("http");
	}
	zval_ptr_dtor(&https);
}

/**
 * Checks whether request has been made using ajax. Checks if $_SERVER['HTTP_X_REQUESTED_WITH']=='XMLHttpRequest'
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, isAjax)
{
	zval requested_header = {}, xml_http_request = {}, requested_with = {};

	ZVAL_STRING(&requested_header, "HTTP_X_REQUESTED_WITH");
	ZVAL_STRING(&xml_http_request, "XMLHttpRequest");

	PHALCON_CALL_METHOD(&requested_with, getThis(), "getheader", &requested_header);
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
	zval *server, content_type = {};

	server = phalcon_get_global_str(SL("_SERVER"));
	if (phalcon_array_isset_str(server, SL("HTTP_SOAPACTION"))) {
		RETURN_TRUE;
	}

	if (phalcon_array_isset_fetch_str(&content_type, server, SL("CONTENT_TYPE"), PH_READONLY)) {
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
	zval scheme = {}, https = {};

	PHALCON_CALL_METHOD(&scheme, getThis(), "getscheme");

	ZVAL_STRING(&https, "https");
	is_identical_function(return_value, &https, &scheme);
}

/**
 * Gets HTTP raw request body
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getRawBody)
{
	zval raw = {}, *zcontext = NULL;
	zend_string *content;
	php_stream_context *context;
	php_stream *stream;
	long int maxlen;

	phalcon_read_property(&raw, getThis(), SL("_rawBody"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(raw) == IS_STRING) {
		RETURN_CTOR(&raw);
	}

	context = php_stream_context_from_zval(zcontext, 0);
	stream = php_stream_open_wrapper_ex("php://input", "rb", REPORT_ERRORS, NULL, context);
	maxlen    = PHP_STREAM_COPY_ALL;

	if (!stream) {
		RETURN_FALSE;
	}

	content = php_stream_copy_to_mem(stream, maxlen, 0);
	if (content != NULL) {
		RETVAL_STR(content);
		phalcon_update_property(getThis(), SL("_rawBody"), return_value);
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
	zval raw_body = {}, *assoc = NULL;
	int ac = 0;

	phalcon_fetch_params(1, 0, 1, &assoc);

	if (assoc && zend_is_true(assoc)) {
		ac = 1;
	}

	PHALCON_MM_CALL_METHOD(&raw_body, getThis(), "getrawbody");
	PHALCON_MM_ADD_ENTRY(&raw_body);
	if (Z_TYPE(raw_body) == IS_STRING) {
		RETURN_MM_ON_FAILURE(phalcon_json_decode(return_value, &raw_body, ac));
	}
	RETURN_MM();
}

/**
 * Gets decoded BSON HTTP raw request body
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getBsonRawBody)
{
	zval raw_body = {};

	PHALCON_CALL_METHOD(&raw_body, getThis(), "getrawbody");
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

	zval *server, server_addr = {};

	server = phalcon_get_global_str(SL("_SERVER"));
	if (phalcon_array_isset_fetch_str(&server_addr, server, SL("SERVER_ADDR"), PH_READONLY)) {
		RETURN_CTOR(&server_addr);
	}

	RETURN_STRING("127.0.0.1");
}

/**
 * Gets active server name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getServerName){

	zval *server, server_name = {};

	server = phalcon_get_global_str(SL("_SERVER"));
	if (phalcon_array_isset_fetch_str(&server_name, server, SL("SERVER_NAME"), PH_READONLY)) {
		RETURN_CTOR(&server_name);
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
	zval longopts = {}, options = {}, host = {}, http_host = {}, scheme = {}, server_name = {}, name = {}, server_port = {}, port = {}, http = {}, standard_port = {}, is_std_name = {}, is_std_port = {}, is_std_http = {};
	zval https = {}, secure_port = {}, is_secure_scheme = {}, is_secure_port = {}, is_secure_http = {};

	if (unlikely(!strcmp(sapi_module.name, "cli"))) {
		array_init(&longopts);
		phalcon_array_append_str(&longopts, SL("host::"), 0);
		PHALCON_CALL_FUNCTION(&options, "getopt", &PHALCON_GLOBAL(z_null), &longopts);
		zval_ptr_dtor(&longopts);

		if (phalcon_array_isset_fetch_str(&host, &options, SL("host"), PH_COPY)) {
			zval_ptr_dtor(&options);
			RETVAL_ZVAL(&host, 0, 0);
			return;
		}
		zval_ptr_dtor(&options);
	}

	/**
	 * Get the server name from _SERVER['HTTP_HOST']
	 */
	ZVAL_STRING(&host, "HTTP_HOST");
	PHALCON_CALL_METHOD(&http_host, getThis(), "getserver", &host);
	zval_ptr_dtor(&host);

	if (zend_is_true(&http_host)) {
		RETVAL_ZVAL(&http_host, 0, 0);
		return;
	}
	zval_ptr_dtor(&http_host);


	/**
	 * Get current scheme
	 */
	PHALCON_CALL_METHOD(&scheme, getThis(), "getscheme");

	/**
	 * Get the server name from _SERVER['SERVER_NAME']
	 */
	ZVAL_STRING(&server_name, "SERVER_NAME");
	PHALCON_CALL_METHOD(&name, getThis(), "getserver", &server_name);
	zval_ptr_dtor(&server_name);

	/**
	 * Get the server port from _SERVER['SERVER_PORT']
	 */
	ZVAL_STRING(&server_port, "SERVER_PORT");
	PHALCON_CALL_METHOD(&port, getThis(), "getserver", &server_port);
	zval_ptr_dtor(&server_port);

	ZVAL_STRING(&http, "http");
	ZVAL_LONG(&standard_port, 80);

	/**
	 * Check if the request is a standard http
	 */
	is_equal_function(&is_std_name, &scheme, &http);
	zval_ptr_dtor(&http);

	is_equal_function(&is_std_port, &port, &standard_port);

	phalcon_and_function(&is_std_http, &is_std_name, &is_std_port);

	ZVAL_STRING(&https, "https");
	ZVAL_LONG(&secure_port, 443);

	/**
	 * Check if the request is a secure http request
	 */
	is_equal_function(&is_secure_scheme, &scheme, &https);
	zval_ptr_dtor(&scheme);
	zval_ptr_dtor(&https);

	is_equal_function(&is_secure_port, &port, &secure_port);
	phalcon_and_function(&is_secure_http, &is_secure_scheme, &is_secure_port);

	/**
	 * If is standard http we return the server name only
	 */
	if (PHALCON_IS_TRUE(&is_std_http)) {
		RETVAL_ZVAL(&name, 0, 0);
		return;
	}

	/**
	 * If is standard secure http we return the server name only
	 */
	if (PHALCON_IS_TRUE(&is_secure_http)) {
		RETVAL_ZVAL(&name, 0, 0);
		return;
	}

	PHALCON_CONCAT_VSV(return_value, &name, ":", &port);
	zval_ptr_dtor(&name);
	zval_ptr_dtor(&port);
}

/**
 * Gets most possible client IPv4 Address. This method search in $_SERVER['REMOTE_ADDR'] and optionally in $_SERVER['HTTP_X_FORWARDED_FOR']
 *
 * @param boolean $trustForwardedHeader
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getClientAddress){

	zval *trust_forwarded_header = NULL, *_SERVER, address = {};

	phalcon_fetch_params(0, 0, 1, &trust_forwarded_header);

	if (!trust_forwarded_header) {
		trust_forwarded_header = &PHALCON_GLOBAL(z_false);
	}

	_SERVER = phalcon_get_global_str(SL("_SERVER"));

	/**
	 * Proxies use this IP
	 */
	if (zend_is_true(trust_forwarded_header)) {
		if (!phalcon_array_isset_fetch_str(&address, _SERVER, SL("HTTP_X_FORWARDED_FOR"), PH_READONLY)) {
			if (!phalcon_array_isset_fetch_str(&address, _SERVER, SL("REMOTE_ADDR"), PH_READONLY)) {
				phalcon_array_fetch_str(&address, _SERVER, SL("REMOTE_ADDR"), PH_NOISY|PH_READONLY);
			}
		}
	} else if (!phalcon_array_isset_fetch_str(&address, _SERVER, SL("REMOTE_ADDR"), PH_READONLY)) {
		phalcon_array_fetch_str(&address, _SERVER, SL("REMOTE_ADDR"), PH_NOISY|PH_READONLY);
	}

	if (Z_TYPE(address) == IS_STRING) {
		if (phalcon_memnstr_str(&address, SL(","))) {
			zval addresses = {};
			/**
			 * The client address has multiples parts, only return the first part
			 */
			phalcon_fast_explode_str(&addresses, SL(","), &address);

			phalcon_array_fetch_long(return_value, &addresses, 0, PH_NOISY|PH_COPY);
			zval_ptr_dtor(&addresses);
			return;
		}

		RETURN_CTOR(&address);
	}

	RETURN_NULL();
}

static const char* phalcon_http_request_getmethod_helper()
{
	zval *value, *_SERVER, key = {};
	const char *method = SG(request_info).request_method;

	if (unlikely(!method)) {
		ZVAL_STRING(&key, "REQUEST_METHOD");

		_SERVER = phalcon_get_global_str(SL("_SERVER"));
		if (Z_TYPE_P(_SERVER) == IS_ARRAY) {
			value = phalcon_hash_get(Z_ARRVAL_P(_SERVER), &key, BP_VAR_UNSET);
			zval_ptr_dtor(&key);
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

	zval longopts = {}, options = {}, method = {};

	if (unlikely(!strcmp(sapi_module.name, "method"))) {
		array_init(&longopts);
		phalcon_array_append_str(&longopts, SL("method::"), 0);
		PHALCON_CALL_FUNCTION(&options, "getopt", &PHALCON_GLOBAL(z_null), &longopts);
		zval_ptr_dtor(&longopts);

		if (phalcon_array_isset_fetch_str(&method, &options, SL("method"), PH_COPY)) {
			zval_ptr_dtor(&options);
			RETVAL_ZVAL(&method, 0, 0);
			return;
		}
		zval_ptr_dtor(&options);
	}

	const char *m = phalcon_http_request_getmethod_helper();
	if (m) {
		RETURN_STRING(m);
	}

	RETURN_EMPTY_STRING();
}

/**
 * Gets HTTP URI which request has been made
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getURI){

	zval *value, *_SERVER, key = {};

	ZVAL_STRING(&key, "REQUEST_URI");

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

	zval *value, *_SERVER, key = {};

	ZVAL_STRING(&key, "QUERY_STRING");

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

	zval *server, user_agent = {};

	server = phalcon_get_global_str(SL("_SERVER"));
	if (phalcon_array_isset_fetch_str(&user_agent, server, SL("HTTP_USER_AGENT"), PH_READONLY)) {
		RETURN_CTOR(&user_agent);
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

	zval *methods, http_method = {}, *method;

	phalcon_fetch_params(0, 1, 0, &methods);

	PHALCON_CALL_METHOD(&http_method, getThis(), "getmethod");

	if (Z_TYPE_P(methods) == IS_STRING) {
		is_equal_function(return_value, methods, &http_method);
		zval_ptr_dtor(&http_method);
		return;
	}

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(methods), method) {
		if (PHALCON_IS_EQUAL(method, &http_method)) {
			zval_ptr_dtor(&http_method);
			RETURN_TRUE;
		}
	} ZEND_HASH_FOREACH_END();
	zval_ptr_dtor(&http_method);

	RETURN_FALSE;
}

/**
 * Checks whether HTTP method is POST. if $_SERVER['REQUEST_METHOD']=='POST'
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, isPost){

	zval post = {}, method = {};

	if (Z_OBJCE_P(getThis()) == phalcon_http_request_ce) {
		RETURN_BOOL(!strcmp(phalcon_http_request_getmethod_helper(), "POST"));
	}

	ZVAL_STR(&post, IS(POST));

	PHALCON_CALL_METHOD(&method, getThis(), "getmethod");
	is_equal_function(return_value, &method, &post);
	zval_ptr_dtor(&method);
}

/**
 * Checks whether HTTP method is GET. if $_SERVER['REQUEST_METHOD']=='GET'
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, isGet){

	zval get = {}, method = {};

	if (Z_OBJCE_P(getThis()) == phalcon_http_request_ce) {
		RETURN_BOOL(!strcmp(phalcon_http_request_getmethod_helper(), "GET"));
	}

	ZVAL_STR(&get, IS(GET));

	PHALCON_CALL_METHOD(&method, getThis(), "getmethod");
	is_equal_function(return_value, &method, &get);
	zval_ptr_dtor(&method);
}

/**
 * Checks whether HTTP method is PUT. if $_SERVER['REQUEST_METHOD']=='PUT'
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, isPut){

	zval put = {}, method = {};

	if (Z_OBJCE_P(getThis()) == phalcon_http_request_ce) {
		RETURN_BOOL(!strcmp(phalcon_http_request_getmethod_helper(), "PUT"));
	}

	ZVAL_STR(&put, IS(PUT));

	PHALCON_CALL_METHOD(&method, getThis(), "getmethod");
	is_equal_function(return_value, &method, &put);
	zval_ptr_dtor(&method);
}

/**
 * Checks whether HTTP method is PATCH. if $_SERVER['REQUEST_METHOD']=='PATCH'
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, isPatch){

	zval patch = {}, method = {};

	if (Z_OBJCE_P(getThis()) == phalcon_http_request_ce) {
		RETURN_BOOL(!strcmp(phalcon_http_request_getmethod_helper(), "PATCH"));
	}

	ZVAL_STR(&patch, IS(PATCH));

	PHALCON_CALL_METHOD(&method, getThis(), "getmethod");
	is_equal_function(return_value, &method, &patch);
	zval_ptr_dtor(&method);
}

/**
 * Checks whether HTTP method is HEAD. if $_SERVER['REQUEST_METHOD']=='HEAD'
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, isHead){

	zval head = {}, method = {};

	if (Z_OBJCE_P(getThis()) == phalcon_http_request_ce) {
		RETURN_BOOL(!strcmp(phalcon_http_request_getmethod_helper(), "HEAD"));
	}

	ZVAL_STR(&head, IS(HEAD));

	PHALCON_CALL_METHOD(&method, getThis(), "getmethod");
	is_equal_function(return_value, &method, &head);
	zval_ptr_dtor(&method);
}

/**
 * Checks whether HTTP method is DELETE. if $_SERVER['REQUEST_METHOD']=='DELETE'
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, isDelete){

	zval delete = {}, method = {};

	if (Z_OBJCE_P(getThis()) == phalcon_http_request_ce) {
		RETURN_BOOL(!strcmp(phalcon_http_request_getmethod_helper(), "DELETE"));
	}

	ZVAL_STR(&delete, IS(DELETE));

	PHALCON_CALL_METHOD(&method, getThis(), "getmethod");
	is_equal_function(return_value, &method, &delete);
	zval_ptr_dtor(&method);
}

/**
 * Checks whether HTTP method is OPTIONS. if $_SERVER['REQUEST_METHOD']=='OPTIONS'
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request, isOptions){

	zval options = {}, method = {};

	if (Z_OBJCE_P(getThis()) == phalcon_http_request_ce) {
		RETURN_BOOL(!strcmp(phalcon_http_request_getmethod_helper(), "OPTIONS"));
	}

	PHALCON_CALL_METHOD(&method, getThis(), "getmethod");
	is_equal_function(return_value, &method, &options);
	zval_ptr_dtor(&method);
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
		zval error = {};
		if (phalcon_array_isset_fetch_str(&error, file, SL("error"), PH_READONLY)) {
			if (Z_TYPE(error) < IS_ARRAY) {
				if (!zend_is_true(&error) || !only_successful) {
					++nfiles;
				}
			} else if (Z_TYPE(error) == IS_ARRAY) {
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
			zval *index, arr = {}, key = {}, file = {}, *params[2];
			smart_str prefix2 = { 0 };
			int res;

			index = phalcon_get_current_key_w(Z_ARRVAL_P(name), &pos_name);

			smart_str_append_smart_str(&prefix2, prefix);
			if (Z_TYPE_P(index) == IS_STRING) {
				smart_str_appendl(&prefix2, Z_STRVAL_P(index), Z_STRLEN_P(index));
			} else {
				smart_str_append_long(&prefix2, Z_LVAL_P(index));
			}

			if (Z_TYPE_P(derror) < IS_ARRAY) {
				smart_str_0(prefix);
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

					ZVAL_NEW_STR(&key, prefix2.s);

					object_init_ex(&file, phalcon_http_request_file_ce);

					params[0] = &arr;
					params[1] = &key;

					res = phalcon_call_method(NULL, &file, "__construct", 2, params);

					if (res != FAILURE) {
						add_next_index_zval(retval, &file);
					} else {
						break;
					}
				}
			} else if (Z_TYPE_P(derror) == IS_ARRAY) {
				smart_str_appendc(&prefix2, '.');
				phalcon_http_request_getuploadedfiles_helper(retval, dname, dtype, dtmp, derror, dsize, only_successful, &prefix2);
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

	phalcon_fetch_params(0, 0, 2, &not_errored, &dst_index);

	only_successful = not_errored ? phalcon_get_intval(not_errored) : 1;

	array_init(return_value);

	_FILES = phalcon_get_global_str(SL("_FILES"));
	if (Z_TYPE_P(_FILES) != IS_ARRAY || !zend_hash_num_elements(Z_ARRVAL_P(_FILES))) {
		return;
	}

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(_FILES), idx, str_key, value) {
		zval index = {}, error = {}, request_file = {}, name = {}, type = {}, tmp_name = {}, size = {};
		smart_str prefix = { 0 };
		if (str_key) {
			ZVAL_STR(&index, str_key);
		} else {
			ZVAL_LONG(&index, idx);
		}

		if (dst_index && !PHALCON_IS_EQUAL(dst_index, &index)) {
			continue;
		}

		if (phalcon_array_isset_fetch_str(&error, value, SL("error"), PH_READONLY)) {
			if (Z_TYPE(error) != IS_ARRAY) {
				if (!zend_is_true(&error) || !only_successful) {
					object_init_ex(&request_file, phalcon_http_request_file_ce);

					PHALCON_CALL_METHOD(NULL, &request_file, "__construct", value, &index);

					phalcon_array_append(return_value, &request_file, PH_COPY);
				}
			} else if (Z_TYPE(error) == IS_ARRAY) {
				phalcon_array_fetch_str(&name, value, SL("name"), PH_NOISY|PH_READONLY);
				phalcon_array_fetch_str(&type, value, SL("type"), PH_NOISY|PH_READONLY);
				phalcon_array_fetch_str(&tmp_name, value, SL("tmp_name"), PH_NOISY|PH_READONLY);
				phalcon_array_fetch_str(&size, value, SL("size"), PH_NOISY|PH_READONLY);

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
			zval header = {};
			ZVAL_STRINGL(&header, ZSTR_VAL(str_key) + 5, ZSTR_LEN(str_key) - 5);
			phalcon_array_update(return_value, &header, value, PH_COPY);
			zval_ptr_dtor(&header);
		}
	} ZEND_HASH_FOREACH_END();
}

/**
 * Gets web page that refers active request. ie: http://www.google.com
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getHTTPReferer){

	zval *_SERVER, http_referer = {};

	_SERVER = phalcon_get_global_str(SL("_SERVER"));
	if (phalcon_array_isset_fetch_str(&http_referer, _SERVER, SL("HTTP_REFERER"), PH_READONLY)) {
		RETURN_CTOR(&http_referer);
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

	zval *server_index, *name, quality_one = {}, http_server = {}, pattern = {}, parts = {}, *part;

	phalcon_fetch_params(0, 2, 0, &server_index, &name);

	ZVAL_DOUBLE(&quality_one, 1);

	array_init(return_value);

	PHALCON_CALL_METHOD(&http_server, getThis(), "getserver", server_index);

	ZVAL_STRING(&pattern, "/,\\s*/");
	PHALCON_CALL_FUNCTION(&parts, "preg_split", &pattern, &http_server);
	zval_ptr_dtor(&pattern);
	zval_ptr_dtor(&http_server);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(parts), part) {
		zval header_parts = {}, quality_part = {}, quality = {}, qualitys = {}, header_name = {};
		phalcon_fast_explode_str(&header_parts, SL(";"), part);
		if (phalcon_array_isset_fetch_long(&quality_part, &header_parts, 1, PH_READONLY)) {
			phalcon_substr(&quality, &quality_part, 2, 0);
		} else {
			ZVAL_COPY(&quality, &quality_one);
		}

		phalcon_array_fetch_long(&header_name, &header_parts, 0, PH_NOISY|PH_COPY);
		zval_ptr_dtor(&header_parts);

		array_init_size(&qualitys, 2);
		phalcon_array_update(&qualitys, name, &header_name, 0);
		phalcon_array_update_str(&qualitys, SL("quality"), &quality, 0);

		phalcon_array_append(return_value, &qualitys, 0);
	} ZEND_HASH_FOREACH_END();
	zval_ptr_dtor(&parts);
}

/**
 * Process a request header and return the one with best quality
 *
 * @param array $qualityParts
 * @param string $name
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, _getBestQuality){

	zval *quality_parts, *name, *accept, quality = {}, selected_name = {};

	phalcon_fetch_params(0, 2, 0, &quality_parts, &name);

	ZVAL_LONG(&quality, 0);
	ZVAL_EMPTY_STRING(&selected_name);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(quality_parts), accept) {
		zval accept_quality = {}, best_quality = {};

		phalcon_array_fetch_str(&accept_quality, accept, SL("quality"), PH_NOISY|PH_READONLY);

		is_smaller_function(&best_quality, &quality, &accept_quality);
		if (PHALCON_IS_TRUE(&best_quality)) {
			ZVAL_COPY_VALUE(&quality, &accept_quality);
			zval_ptr_dtor(&selected_name);
			phalcon_array_fetch(&selected_name, accept, name, PH_NOISY|PH_COPY);
		}
	} ZEND_HASH_FOREACH_END();

	RETVAL_ZVAL(&selected_name, 0, 0);
}

/**
 * Gets array with mime/types and their quality accepted by the browser/client from $_SERVER['HTTP_ACCEPT']
 *
 * @return array
 */
PHP_METHOD(Phalcon_Http_Request, getAcceptableContent)
{
	zval accept_header = {}, quality_index = {};

	ZVAL_STRING(&accept_header, "HTTP_ACCEPT");
	ZVAL_STRING(&quality_index, "accept");

	PHALCON_RETURN_CALL_METHOD(getThis(), "_getqualityheader", &accept_header, &quality_index);
	zval_ptr_dtor(&quality_index);
	zval_ptr_dtor(&accept_header);
}

/**
 * Gets best mime/type accepted by the browser/client from $_SERVER['HTTP_ACCEPT']
 *
 * @return array
 */
PHP_METHOD(Phalcon_Http_Request, getBestAccept)
{
	zval quality_index = {}, acceptable_content = {};

	ZVAL_STRING(&quality_index, "accept");

	PHALCON_CALL_METHOD(&acceptable_content, getThis(), "getacceptablecontent");
	PHALCON_RETURN_CALL_METHOD(getThis(), "_getbestquality", &acceptable_content, &quality_index);
	zval_ptr_dtor(&quality_index);
	zval_ptr_dtor(&acceptable_content);
}

/**
 * Gets charsets array and their quality accepted by the browser/client from $_SERVER['HTTP_ACCEPT_CHARSET']
 *
 * @return array
 */
PHP_METHOD(Phalcon_Http_Request, getClientCharsets)
{
	zval charset_header = {}, quality_index = {};

	ZVAL_STRING(&charset_header, "HTTP_ACCEPT_CHARSET");
	ZVAL_STRING(&quality_index, "charset");

	PHALCON_RETURN_CALL_METHOD(getThis(), "_getqualityheader", &charset_header, &quality_index);
	zval_ptr_dtor(&quality_index);
	zval_ptr_dtor(&charset_header);
}

/**
 * Gets best charset accepted by the browser/client from $_SERVER['HTTP_ACCEPT_CHARSET']
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getBestCharset)
{
	zval quality_index = {}, client_charsets = {};

	ZVAL_STRING(&quality_index, "charset");

	PHALCON_CALL_METHOD(&client_charsets, getThis(), "getclientcharsets");
	PHALCON_RETURN_CALL_METHOD(getThis(), "_getbestquality", &client_charsets, &quality_index);
	zval_ptr_dtor(&quality_index);
	zval_ptr_dtor(&client_charsets);
}

/**
 * Gets languages array and their quality accepted by the browser/client from $_SERVER['HTTP_ACCEPT_LANGUAGE']
 *
 * @return array
 */
PHP_METHOD(Phalcon_Http_Request, getLanguages)
{
	zval language_header = {}, quality_index = {};

	ZVAL_STRING(&language_header, "HTTP_ACCEPT_LANGUAGE");
	ZVAL_STRING(&quality_index, "language");

	PHALCON_RETURN_CALL_METHOD(getThis(), "_getqualityheader", &language_header, &quality_index);
	zval_ptr_dtor(&quality_index);
	zval_ptr_dtor(&language_header);
}

/**
 * Gets best language accepted by the browser/client from $_SERVER['HTTP_ACCEPT_LANGUAGE']
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request, getBestLanguage)
{
	zval languages = {}, quality_index = {};

	PHALCON_CALL_METHOD(&languages, getThis(), "getlanguages");

	ZVAL_STRING(&quality_index, "language");
	PHALCON_RETURN_CALL_METHOD(getThis(), "_getbestquality", &languages, &quality_index);
	zval_ptr_dtor(&quality_index);
	zval_ptr_dtor(&languages);
}

/**
 * Gets auth info accepted by the browser/client from $_SERVER['PHP_AUTH_USER']
 *
 * @return array
 */
PHP_METHOD(Phalcon_Http_Request, getBasicAuth)
{
	zval *_SERVER, *value, key = {};
	char *auth_user = SG(request_info).auth_user;
	char *auth_password = SG(request_info).auth_password;

	if (unlikely(!auth_user)) {
		_SERVER = phalcon_get_global_str(SL("_SERVER"));
		if (Z_TYPE_P(_SERVER) == IS_ARRAY) {
			ZVAL_STRING(&key, "PHP_AUTH_USER");

			value = phalcon_hash_get(Z_ARRVAL_P(_SERVER), &key, BP_VAR_UNSET);
			zval_ptr_dtor(&key);
			if (value && Z_TYPE_P(value) == IS_STRING) {
				auth_user = Z_STRVAL_P(value);
			}

			ZVAL_STRING(&key, "PHP_AUTH_PW");

			value = phalcon_hash_get(Z_ARRVAL_P(_SERVER), &key, BP_VAR_UNSET);
			zval_ptr_dtor(&key);
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
	phalcon_array_update_str_str(return_value, SL("username"), auth_user, strlen(auth_user), 0);
	phalcon_array_update_str_str(return_value, SL("password"), auth_password, strlen(auth_password), 0);
}

/**
 * Gets auth info accepted by the browser/client from $_SERVER['PHP_AUTH_DIGEST']
 *
 * @return array
 */
PHP_METHOD(Phalcon_Http_Request, getDigestAuth){

	zval *_SERVER, *value, pattern = {}, digest = {}, matches = {}, ret = {}, *match;
	const char *auth_digest = SG(request_info).auth_digest;

	PHALCON_MM_INIT();

	if (unlikely(!auth_digest)) {
		_SERVER = phalcon_get_global_str(SL("_SERVER"));
		if (Z_TYPE_P(_SERVER) == IS_ARRAY) {
			zval key = {};
			PHALCON_MM_ZVAL_STRING(&key, "PHP_AUTH_DIGEST");

			value = phalcon_hash_get(Z_ARRVAL_P(_SERVER), &key, BP_VAR_UNSET);
			if (value && Z_TYPE_P(value) == IS_STRING) {
				auth_digest = Z_STRVAL_P(value);
			}
		}
	}

	if (auth_digest) {
		PHALCON_MM_ZVAL_STRING(&pattern, "#(\\w+)=(['\"]?)([^'\", ]+)\\2#");
		PHALCON_MM_ZVAL_STRING(&digest, auth_digest);
		RETURN_MM_ON_FAILURE(phalcon_preg_match(&ret, &pattern, &digest, &matches, 2, 1));
		PHALCON_MM_ADD_ENTRY(&matches);

		if (zend_is_true(&ret) && Z_TYPE(matches) == IS_ARRAY) {
			array_init(return_value);

			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(matches), match) {
				zval tmp1 = {}, tmp2 = {};
				if (Z_TYPE_P(match) == IS_ARRAY && phalcon_array_isset_fetch_long(&tmp1, match, 1, PH_READONLY)
					&& phalcon_array_isset_fetch_long(&tmp2, match, 3, PH_READONLY)) {
					phalcon_array_update(return_value, &tmp1, &tmp2, PH_COPY);
				}
			} ZEND_HASH_FOREACH_END();
			RETURN_MM();
		}
	}

	RETURN_MM_NULL();
}
