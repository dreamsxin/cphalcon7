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

#include "http/response.h"
#include "http/responseinterface.h"
#include "http/response/exception.h"
#include "http/response/headers.h"
#include "di.h"
#include "diinterface.h"
#include "di/injectable.h"
#include "mvc/urlinterface.h"
#include "mvc/viewinterface.h"

#include <ext/date/php_date.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/hash.h"
#include "kernel/array.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/file.h"
#include "kernel/variables.h"

#include "interned-strings.h"

/**
 * Phalcon\Http\Response
 *
 * Part of the HTTP cycle is return responses to the clients.
 * Phalcon\HTTP\Response is the Phalcon component responsible to achieve this task.
 * HTTP responses are usually composed by headers and body.
 *
 *<code>
 *	$response = new Phalcon\Http\Response();
 *	$response->setStatusCode(200, "OK");
 *	$response->setContent("<html><body>Hello</body></html>");
 *	$response->send();
 *</code>
 */
zend_class_entry *phalcon_http_response_ce;

PHP_METHOD(Phalcon_Http_Response, __construct);
PHP_METHOD(Phalcon_Http_Response, setStatusCode);
PHP_METHOD(Phalcon_Http_Response, setHeaders);
PHP_METHOD(Phalcon_Http_Response, getHeaders);
PHP_METHOD(Phalcon_Http_Response, setCookies);
PHP_METHOD(Phalcon_Http_Response, getCookies);
PHP_METHOD(Phalcon_Http_Response, setHeader);
PHP_METHOD(Phalcon_Http_Response, setRawHeader);
PHP_METHOD(Phalcon_Http_Response, resetHeaders);
PHP_METHOD(Phalcon_Http_Response, setExpires);
PHP_METHOD(Phalcon_Http_Response, setNotModified);
PHP_METHOD(Phalcon_Http_Response, setContentType);
PHP_METHOD(Phalcon_Http_Response, setEtag);
PHP_METHOD(Phalcon_Http_Response, redirect);
PHP_METHOD(Phalcon_Http_Response, setContent);
PHP_METHOD(Phalcon_Http_Response, setJsonContent);
PHP_METHOD(Phalcon_Http_Response, setBsonContent);
PHP_METHOD(Phalcon_Http_Response, appendContent);
PHP_METHOD(Phalcon_Http_Response, getContent);
PHP_METHOD(Phalcon_Http_Response, isSent);
PHP_METHOD(Phalcon_Http_Response, sendHeaders);
PHP_METHOD(Phalcon_Http_Response, sendCookies);
PHP_METHOD(Phalcon_Http_Response, send);
PHP_METHOD(Phalcon_Http_Response, setFileToSend);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_response___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, content)
	ZEND_ARG_INFO(0, code)
	ZEND_ARG_INFO(0, status)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_response_setheaders, 0, 0, 1)
	ZEND_ARG_INFO(0, headers)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_response_setcookies, 0, 0, 1)
	ZEND_ARG_INFO(0, cookies)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_response_setetag, 0, 0, 1)
	ZEND_ARG_INFO(0, etag)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_http_response_method_entry[] = {
	PHP_ME(Phalcon_Http_Response, __construct, arginfo_phalcon_http_response___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Http_Response, setStatusCode, arginfo_phalcon_http_responseinterface_setstatuscode, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, setHeaders, arginfo_phalcon_http_response_setheaders, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, getHeaders, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, setCookies, arginfo_phalcon_http_response_setcookies, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, getCookies, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, setHeader, arginfo_phalcon_http_responseinterface_setheader, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, setRawHeader, arginfo_phalcon_http_responseinterface_setrawheader, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, resetHeaders, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, setExpires, arginfo_phalcon_http_responseinterface_setexpires, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, setNotModified, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, setContentType, arginfo_phalcon_http_responseinterface_setcontenttype, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, setEtag, arginfo_phalcon_http_response_setetag, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, redirect, arginfo_phalcon_http_responseinterface_redirect, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, setContent, arginfo_phalcon_http_responseinterface_setcontent, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, setJsonContent, arginfo_phalcon_http_responseinterface_setjsoncontent, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, setBsonContent, arginfo_phalcon_http_responseinterface_setbsoncontent, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, appendContent, arginfo_phalcon_http_responseinterface_appendcontent, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, getContent, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, isSent, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, sendHeaders, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, sendCookies, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, send, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Response, setFileToSend, arginfo_phalcon_http_responseinterface_setfiletosend, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Http\Response initializer
 */
PHALCON_INIT_CLASS(Phalcon_Http_Response){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Http, Response, http_response, phalcon_di_injectable_ce, phalcon_http_response_method_entry, 0);

	zend_declare_property_bool(phalcon_http_response_ce, SL("_sent"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_response_ce, SL("_content"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_response_ce, SL("_headers"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_response_ce, SL("_cookies"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_response_ce, SL("_file"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_http_response_ce, 1, phalcon_http_responseinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Http\Response constructor
 *
 * @param string $content
 * @param int $code
 * @param string $status
 */
PHP_METHOD(Phalcon_Http_Response, __construct){

	zval *content = NULL, *code = NULL, *status = NULL;

	phalcon_fetch_params(0, 0, 3, &content, &code, &status);

	if (!status) {
		status = &PHALCON_GLOBAL(z_null);
	}

	if (content && Z_TYPE_P(content) != IS_NULL) {
		phalcon_update_property_zval(getThis(), SL("_content"), content);
	}

	if (code && Z_TYPE_P(code) != IS_NULL) {
		PHALCON_CALL_METHODW(NULL, getThis(), "setstatuscode", code, status);
	}
}

/**
 * Sets the HTTP response code
 *
 *<code>
 *	$response->setStatusCode(404, "Not Found");
 *</code>
 *
 * @param int $code
 * @param string $message
 * @return Phalcon\Http\ResponseInterface
 */
PHP_METHOD(Phalcon_Http_Response, setStatusCode)
{
	zval *code, *message, headers = {}, current_headers_raw = {}, header_value = {}, status_value = {}, status_header = {};
	zend_string *str_key;

	phalcon_fetch_params(0, 2, 0, &code, &message);

	PHALCON_CALL_METHODW(&headers, getThis(), "getheaders");

	/** 
	 * We use HTTP/1.1 instead of HTTP/1.0
	 *
	 * Before that we would like to unset any existing HTTP/x.y headers
	 */
	PHALCON_CALL_METHODW(&current_headers_raw, &headers, "toarray");

	if (Z_TYPE(current_headers_raw) == IS_ARRAY) {
		ZEND_HASH_FOREACH_STR_KEY(Z_ARRVAL(current_headers_raw), str_key) {
			zval header_name = {};
			if (str_key) {
				ZVAL_STR(&header_name, str_key);
				if ((size_t)(Z_STRLEN(header_name)) > sizeof("HTTP/x.y ")-1 && !memcmp(Z_STRVAL(header_name), "HTTP/", 5)) {
					PHALCON_CALL_METHODW(NULL, &headers, "remove", &header_name);
				}
			}
		} ZEND_HASH_FOREACH_END();
	}

	PHALCON_CONCAT_SVSV(&header_value, "HTTP/1.1 ", code, " ", message);
	PHALCON_CALL_METHODW(NULL, &headers, "setraw", &header_value);

	/** 
	 * We also define a 'Status' header with the HTTP status
	 */
	PHALCON_CONCAT_VSV(&status_value, code, " ", message);

	ZVAL_STRING(&status_header, "Status");
	PHALCON_CALL_METHODW(NULL, &headers, "set", &status_header, &status_value);
	phalcon_update_property_zval(getThis(), SL("_headers"), &headers);
	RETURN_THISW();
}

/**
 * Sets a headers bag for the response externally
 *
 * @param Phalcon\Http\Response\HeadersInterface $headers
 * @return Phalcon\Http\ResponseInterface
 */
PHP_METHOD(Phalcon_Http_Response, setHeaders){

	zval *headers;

	phalcon_fetch_params(0, 1, 0, &headers);

	phalcon_update_property_zval(getThis(), SL("_headers"), headers);
	RETURN_THISW();
}

/**
 * Returns headers set by the user
 *
 * @return Phalcon\Http\Response\HeadersInterface
 */
PHP_METHOD(Phalcon_Http_Response, getHeaders){

	zval *headers;

	headers = phalcon_read_property(getThis(), SL("_headers"), PH_NOISY);
	if (Z_TYPE_P(headers) == IS_NULL) {
		/** 
		 * A Phalcon\Http\Response\Headers bag is temporary used to manage the headers
		 * before sending them to the client
		 */
		object_init_ex(return_value, phalcon_http_response_headers_ce);
		phalcon_update_property_zval(getThis(), SL("_headers"), return_value);
		return;
	}

	RETURN_CTORW(headers);
}

/**
 * Sets a cookies bag for the response externally
 *
 * @param Phalcon\Http\Response\CookiesInterface $cookies
 * @return Phalcon\Http\ResponseInterface
 */
PHP_METHOD(Phalcon_Http_Response, setCookies){

	zval *cookies;

	phalcon_fetch_params(0, 1, 0, &cookies);

	if (Z_TYPE_P(cookies) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_http_response_exception_ce, "The cookies bag is not valid");
		return;
	}
	phalcon_update_property_zval(getThis(), SL("_cookies"), cookies);

	RETURN_THISW();
}

/**
 * Returns coookies set by the user
 *
 * @return Phalcon\Http\Response\CookiesInterface
 */
PHP_METHOD(Phalcon_Http_Response, getCookies){


	RETURN_MEMBER(getThis(), "_cookies");
}

/**
 * Overwrites a header in the response
 *
 *<code>
 *	$response->setHeader("Content-Type", "text/plain");
 *</code>
 *
 * @param string $name
 * @param string $value
 * @return Phalcon\Http\ResponseInterface
 */
PHP_METHOD(Phalcon_Http_Response, setHeader)
{
	zval *name, *value, headers = {};

	phalcon_fetch_params(0, 2, 0, &name, &value);

	PHALCON_CALL_METHODW(&headers, getThis(), "getheaders");
	PHALCON_CALL_METHODW(NULL, &headers, "set", name, value);

	RETURN_THISW();
}

/**
 * Send a raw header to the response
 *
 *<code>
 *	$response->setRawHeader("HTTP/1.1 404 Not Found");
 *</code>
 *
 * @param string $header
 * @return Phalcon\Http\ResponseInterface
 */
PHP_METHOD(Phalcon_Http_Response, setRawHeader)
{
	zval *header, headers = {};

	phalcon_fetch_params(0, 1, 0, &header);

	PHALCON_CALL_METHODW(&headers, getThis(), "getheaders");
	PHALCON_CALL_METHODW(NULL, &headers, "setraw", header);

	RETURN_THISW();
}

/**
 * Resets all the stablished headers
 *
 * @return Phalcon\Http\ResponseInterface
 */
PHP_METHOD(Phalcon_Http_Response, resetHeaders)
{
	zval headers = {};

	PHALCON_CALL_METHODW(&headers, getThis(), "getheaders");
	PHALCON_CALL_METHODW(NULL, &headers, "reset");

	RETURN_THISW();
}

/**
 * Sets a Expires header to use HTTP cache
 *
 *<code>
 *	$this->response->setExpires(new DateTime());
 *</code>
 *
 * @param \DateTime $datetime
 * @return \Phalcon\Http\ResponseInterface
 */
PHP_METHOD(Phalcon_Http_Response, setExpires)
{
	zval *datetime, headers = {}, date = {}, utc_zone = {}, timezone = {}, format = {}, utc_format = {}, utc_date = {}, expires_header = {};
	zend_class_entry *datetime_ce, *datetimezone_ce;

	phalcon_fetch_params(0, 1, 0, &datetime);

	datetime_ce = php_date_get_date_ce();
	PHALCON_VERIFY_CLASS_EX(datetime, datetime_ce, phalcon_http_response_exception_ce, 1);

	PHALCON_CALL_METHODW(&headers, getThis(), "getheaders");

	if (phalcon_clone(&date, datetime) == FAILURE) {
		return;
	}

	/** 
	 * All the expiration times are sent in UTC
	 */
	ZVAL_STRING(&utc_zone, "UTC");

	datetimezone_ce = php_date_get_timezone_ce();
	object_init_ex(&timezone, datetimezone_ce);

	PHALCON_CALL_METHODW(NULL, &timezone, "__construct", &utc_zone);

	/** 
	 * Change the timezone to utc
	 */
	PHALCON_CALL_METHODW(NULL, &date, "settimezone", &timezone);

	ZVAL_STRING(&format, "D, d M Y H:i:s");

	PHALCON_CALL_METHODW(&utc_format, &date, "format", &format);

	PHALCON_CONCAT_VS(&utc_date, &utc_format, " GMT");

	/** 
	 * The 'Expires' header set this info
	 */
	ZVAL_STRING(&expires_header, "Expires");

	PHALCON_CALL_METHODW(NULL, getThis(), "setheader", &expires_header, &utc_date);

	RETURN_THISW();
}

/**
 * Sends a Not-Modified response
 *
 * @return Phalcon\Http\ResponseInterface
 */
PHP_METHOD(Phalcon_Http_Response, setNotModified)
{
	zval code = {}, status = {};

	ZVAL_LONG(&code, 304);
	ZVAL_STRING(&status, "Not modified");

	PHALCON_CALL_METHODW(NULL, getThis(), "setstatuscode", &code, &status);

	RETURN_THISW();
}

/**
 * Sets the response content-type mime, optionally the charset
 *
 *<code>
 *	$response->setContentType('application/pdf');
 *	$response->setContentType('text/plain', 'UTF-8');
 *</code>
 *
 * @param string $contentType
 * @param string $charset
 * @return Phalcon\Http\ResponseInterface
 */
PHP_METHOD(Phalcon_Http_Response, setContentType){

	zval *content_type, *charset = NULL, headers = {}, name = {}, header_value = {};

	phalcon_fetch_params(0, 1, 1, &content_type, &charset);

	PHALCON_CALL_METHODW(&headers, getThis(), "getheaders");

	ZVAL_STRING(&name, "Content-Type");

	if (!charset || Z_TYPE_P(charset) == IS_NULL) {
		PHALCON_CALL_METHODW(NULL, &headers, "set", &name, content_type);
	} else {
		PHALCON_CONCAT_VSV(&header_value, content_type, "; charset=", charset);

		PHALCON_CALL_METHODW(NULL, &headers, "set", &name, &header_value);
	}

	RETURN_THISW();
}

/**
 * Set a custom ETag
 *
 *<code>
 *	$response->setEtag(md5(time()));
 *</code>
 *
 * @param string $etag
 */
PHP_METHOD(Phalcon_Http_Response, setEtag){

	zval *etag, name = {}, headers = {};

	phalcon_fetch_params(0, 1, 0, &etag);

	ZVAL_STRING(&name, "ETag");

	PHALCON_CALL_METHODW(&headers, getThis(), "getheaders");
	PHALCON_CALL_METHODW(NULL, &headers, "set", &name, etag);

	RETURN_THISW();
}

/**
 * Redirect by HTTP to another action or URL
 *
 *<code>
 *  //Using a string redirect (internal/external)
 *	$response->redirect("posts/index");
 *	$response->redirect("http://en.wikipedia.org", true);
 *	$response->redirect("http://www.example.com/new-location", true, 301);
 *
 *	//Making a redirection based on a named route
 *	$response->redirect(array(
 *		"for" => "index-lang",
 *		"lang" => "jp",
 *		"controller" => "index"
 *	));
 *</code>
 *
 * @param string|array $location
 * @param boolean $externalRedirect
 * @param int $statusCode
 * @return Phalcon\Http\ResponseInterface
 */
PHP_METHOD(Phalcon_Http_Response, redirect){

	zval *location = NULL, *external_redirect = NULL, *_status_code = NULL, status_code = {}, header = {}, matched = {}, pattern = {}, dependency_injector = {}, service_name = {};
	zval url = {}, view = {}, status_text = {}, header_name = {};

	static const char* redirect_phrases[] = {
		/* 300 */ "Multiple Choices",
		/* 301 */ "Moved Permanently",
		/* 302 */ "Found",
		/* 303 */ "See Other",
		/* 304 */ "Not Modified",
		/* 305 */ "Use Proxy",
		/* 306 */ "Switch Proxy",
		/* 307 */ "Temporary Redirect",
		/* 308 */ "Permanent Redirect"
	};

	phalcon_fetch_params(0, 0, 3, &location, &external_redirect, &_status_code);

	if (!location) {
		location = &PHALCON_GLOBAL(z_null);
	} else if (Z_TYPE_P(location) != IS_STRING && Z_TYPE_P(location) != IS_ARRAY) {
		PHALCON_SEPARATE_PARAM(location);
		convert_to_string(location);
	}

	if (!external_redirect) {
		external_redirect = &PHALCON_GLOBAL(z_false);
	}

	if (!_status_code) {
		ZVAL_LONG(&status_code, 302);
	} else {
		PHALCON_CPY_WRT_CTOR(&status_code, _status_code);
		if (unlikely(Z_TYPE(status_code) != IS_LONG)) {
			convert_to_long(&status_code);			
		}
	}

	if (Z_TYPE_P(location) == IS_STRING && zend_is_true(external_redirect)) {
		PHALCON_CPY_WRT(&header, location);
	} else if (Z_TYPE_P(location) == IS_STRING && strstr(Z_STRVAL_P(location), "://")) {
		ZVAL_STRING(&pattern, "/^[^:\\/?#]++:/");
		RETURN_ON_FAILURE(phalcon_preg_match(&matched, &pattern, location, NULL));
		if (zend_is_true(&matched)) {
			PHALCON_CPY_WRT(&header, location);
		} else {
			ZVAL_NULL(&header);
		}
	} else {
		ZVAL_NULL(&header);
	}

	PHALCON_CALL_METHODW(&dependency_injector, getThis(), "getdi");

	if (Z_TYPE(header) < IS_NULL) {
		ZVAL_STRING(&service_name, ISV(url));

		PHALCON_CALL_METHODW(&url, &dependency_injector, "getshared", &service_name);
		PHALCON_VERIFY_INTERFACEW(&url, phalcon_mvc_urlinterface_ce);

		PHALCON_CALL_METHODW(&header, &url, "get", location);
	}

	ZVAL_STRING(&service_name, ISV(view));

	PHALCON_CALL_METHODW(&view, &dependency_injector, "get", &service_name, &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_true));
	if (Z_TYPE(view) == IS_OBJECT && instanceof_function(Z_OBJCE(view), phalcon_mvc_viewinterface_ce)) {
		PHALCON_CALL_METHODW(NULL, &view, "disable");
	}

	/* The HTTP status is 302 by default, a temporary redirection */
	if (Z_LVAL(status_code) < 300 || Z_LVAL(status_code) > 308) {
		ZVAL_STRING(&status_text, "Redirect");
		if (!Z_LVAL(status_code)) {
			ZVAL_LONG(&status_code, 302);
		}
	} else {
		ZVAL_STRING(&status_text, redirect_phrases[Z_LVAL(status_code) - 300]);
	}

	PHALCON_CALL_METHODW(NULL, getThis(), "setstatuscode", &status_code, &status_text);

	/** 
	 * Change the current location using 'Location'
	 */
	ZVAL_STRING(&header_name, "Location");
	PHALCON_CALL_METHODW(NULL, getThis(), "setheader", &header_name, &header);

	RETURN_THISW();
}

/**
 * Sets HTTP response body
 *
 *<code>
 *	$response->setContent("<h1>Hello!</h1>");
 *</code>
 *
 * @param string $content
 * @return Phalcon\Http\ResponseInterface
 */
PHP_METHOD(Phalcon_Http_Response, setContent){

	zval *content;

	phalcon_fetch_params(0, 1, 0, &content);

	phalcon_update_property_zval(getThis(), SL("_content"), content);
	RETURN_THISW();
}

/**
 * Sets HTTP response body. The parameter is automatically converted to JSON
 *
 *<code>
 *	$response->setJsonContent(array("status" => "OK"));
 *	$response->setJsonContent(array("status" => "OK"), JSON_NUMERIC_CHECK);
*</code>
 *
 * @param string $content
 * @param int $jsonOptions bitmask consisting on http://www.php.net/manual/en/json.constants.php
 * @return Phalcon\Http\ResponseInterface
 */
PHP_METHOD(Phalcon_Http_Response, setJsonContent){

	zval *content, *json_options = NULL, json_content = {};
	int options = 0;

	phalcon_fetch_params(0, 1, 1, &content, &json_options);

	if (json_options) {
		options = phalcon_get_intval(json_options);
	}

	RETURN_ON_FAILURE(phalcon_json_encode(&json_content, content, options));
	phalcon_update_property_zval(getThis(), SL("_content"), &json_content);
	RETURN_THISW();
}

/**
 * Sets HTTP response body. The parameter is automatically converted to BSON
 *
 *<code>
 *	$response->setBsonContent(array("status" => "OK", "pic" => new MongoBinData(file_get_contents("/var/www/phalconphp.jpg")));
*</code>
 *
 * @param mixed $content
 * @return Phalcon\Http\ResponseInterface
 */
PHP_METHOD(Phalcon_Http_Response, setBsonContent){

	zval *content, content_type = {}, bson_content = {};

	phalcon_fetch_params(0, 1, 0, &content);

	ZVAL_STRING(&content_type, "application/bson");
	PHALCON_CALL_METHODW(NULL, getThis(), "setContentType", &content_type);

	PHALCON_CALL_FUNCTIONW(&bson_content, "bson_encode", content);
	phalcon_update_property_zval(getThis(), SL("_content"), &bson_content);
	RETURN_THISW();
}

/**
 * Appends a string to the HTTP response body
 *
 * @param string $content
 * @return Phalcon\Http\ResponseInterface
 */
PHP_METHOD(Phalcon_Http_Response, appendContent){

	zval *content, *_content, temp_content = {};

	phalcon_fetch_params(0, 1, 0, &content);

	_content = phalcon_read_property(getThis(), SL("_content"), PH_NOISY);

	concat_function(&temp_content, _content, content);

	phalcon_update_property_zval(getThis(), SL("_content"), &temp_content);
	RETURN_THISW();
}

/**
 * Gets the HTTP response body
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Response, getContent){


	RETURN_MEMBER(getThis(), "_content");
}

/**
 * Check if the response is already sent
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Response, isSent){


	RETURN_MEMBER(getThis(), "_sent");
}

/**
 * Sends headers to the client
 *
 * @return Phalcon\Http\ResponseInterface
 */
PHP_METHOD(Phalcon_Http_Response, sendHeaders){

	zval *headers;

	headers = phalcon_read_property(getThis(), SL("_headers"), PH_NOISY);
	if (Z_TYPE_P(headers) == IS_OBJECT) {
		PHALCON_CALL_METHODW(NULL, headers, "send");
	}

	RETURN_THISW();
}

/**
 * Sends cookies to the client
 *
 * @return Phalcon\Http\ResponseInterface
 */
PHP_METHOD(Phalcon_Http_Response, sendCookies){

	zval *cookies;

	cookies = phalcon_read_property(getThis(), SL("_cookies"), PH_NOISY);
	if (Z_TYPE_P(cookies) == IS_OBJECT) {
		PHALCON_CALL_METHODW(NULL, cookies, "send");
	}

	RETURN_THISW();
}

/**
 * Prints out HTTP response to the client
 *
 * @return Phalcon\Http\ResponseInterface
 */
PHP_METHOD(Phalcon_Http_Response, send){

	zval *sent, *headers, *cookies, *content, *file;

	sent = phalcon_read_property(getThis(), SL("_sent"), PH_NOISY);
	if (PHALCON_IS_FALSE(sent)) {
		/* Send headers */
		headers = phalcon_read_property(getThis(), SL("_headers"), PH_NOISY);
		if (Z_TYPE_P(headers) == IS_OBJECT) {
			PHALCON_CALL_METHODW(NULL, headers, "send");
		}

		cookies = phalcon_read_property(getThis(), SL("_cookies"), PH_NOISY);
		if (Z_TYPE_P(cookies) == IS_OBJECT) {
			PHALCON_CALL_METHODW(NULL, cookies, "send");
		}

		/* Output the response body */
		content = phalcon_read_property(getThis(), SL("_content"), PH_NOISY);
		if (Z_TYPE_P(content) != IS_NULL) {
			zend_print_zval(content, 0);
		}
		else {
			file = phalcon_read_property(getThis(), SL("_file"), PH_NOISY);

			if (Z_TYPE_P(file) == IS_STRING && Z_STRLEN_P(file)) {
				php_stream *stream;

				stream = php_stream_open_wrapper(Z_STRVAL_P(file), "rb", REPORT_ERRORS, NULL);
				if (stream != NULL) {
					php_stream_passthru(stream);
					php_stream_close(stream);
				}
			}
		}

		phalcon_update_property_bool(getThis(), SL("_sent"), 1);

		RETURN_THISW();
	}

	PHALCON_THROW_EXCEPTION_STRW(phalcon_http_response_exception_ce, "Response was already sent");
}

/**
 * Sets an attached file to be sent at the end of the request
 *
 * @param string $filePath
 * @param string $attachmentName
 */
PHP_METHOD(Phalcon_Http_Response, setFileToSend){

	zval *file_path, *attachment_name = NULL, *attachment = NULL, base_path = {}, headers = {}, content_description = {}, content_disposition = {}, content_transfer = {};

	phalcon_fetch_params(0, 1, 2, &file_path, &attachment_name, &attachment);

	if (!attachment_name) {
		attachment_name = &PHALCON_GLOBAL(z_null);
	}

	if (!attachment) {
		attachment = &PHALCON_GLOBAL(z_true);
	}

	if (Z_TYPE_P(attachment_name) != IS_STRING) {
		phalcon_basename(&base_path, file_path);
	} else {
		PHALCON_CPY_WRT_CTOR(&base_path, attachment_name);
	}

	if (zend_is_true(attachment)) {
		PHALCON_CALL_METHODW(&headers, getThis(), "getheaders");

		ZVAL_STRING(&content_description, "Content-Description: File Transfer");
		PHALCON_CALL_METHODW(NULL, &headers, "setraw", &content_description);

		PHALCON_CONCAT_SV(&content_disposition, "Content-Disposition: attachment; filename=", &base_path);
		PHALCON_CALL_METHODW(NULL, &headers, "setraw", &content_disposition);

		ZVAL_STRING(&content_transfer, "Content-Transfer-Encoding: binary");
		PHALCON_CALL_METHODW(NULL, &headers, "setraw", &content_transfer);
	}

	phalcon_update_property_zval(getThis(), SL("_file"), file_path);

	RETURN_THISW();
}
