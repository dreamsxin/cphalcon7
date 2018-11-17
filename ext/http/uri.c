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

#include "http/uri.h"

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
#include "kernel/debug.h"

/**
 * Phalcon\Http\Uri
 *
 *<code>
 *	$uri1 = new Phalcon\Http\Uri('http://phalconphp.com/foo/bar/baz?var1=a&var2=1');
 *
 *	$uri2 = $uri1->resolve('/last');
 *	echo $uri2->build(); // http://phalconphp.com/last?var1=a&var2=1
 *
 *
 *	$uri3 = $uri1->resolve('last');
 *	echo $uri3->build(); // http://phalconphp.com/foo/bar/baz/last?var1=a&var2=1
 *
 *	$uri4 = new Phalcon\Http\Uri(array(
 *	    'scheme' => 'https',
 *	    'host' => 'admin.example.com',
 *	    'user' => 'john',
 *	    'pass' => 'doe'
 *	));
 *
 *	$uri5 = $uri1->resolve($uri4);
 *	echo $uri5->build(); // https://john:doe@admin.example.com/foo/bar/baz?var1=a&var2=1
 *</code>
 */
zend_class_entry *phalcon_http_uri_ce;

PHP_METHOD(Phalcon_Http_Uri, __construct);
PHP_METHOD(Phalcon_Http_Uri, __toString);
PHP_METHOD(Phalcon_Http_Uri, __unset);
PHP_METHOD(Phalcon_Http_Uri, __set);
PHP_METHOD(Phalcon_Http_Uri, __get);
PHP_METHOD(Phalcon_Http_Uri, __isset);
PHP_METHOD(Phalcon_Http_Uri, getParts);
PHP_METHOD(Phalcon_Http_Uri, getPath);
PHP_METHOD(Phalcon_Http_Uri, build);
PHP_METHOD(Phalcon_Http_Uri, resolve);
PHP_METHOD(Phalcon_Http_Uri, extend);
PHP_METHOD(Phalcon_Http_Uri, extendQuery);
PHP_METHOD(Phalcon_Http_Uri, extendPath);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_uri___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, uri)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_uri___unset, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_uri___set, 0, 0, 2)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_uri___get, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_uri___isset, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_uri_resolve, 0, 0, 1)
	ZEND_ARG_INFO(0, uri)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_uri_extend, 0, 0, 1)
	ZEND_ARG_INFO(0, uri)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_uri_extendquery, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, param, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_uri_extendpath, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, param, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_http_uri_method_entry[] = {
	PHP_ME(Phalcon_Http_Uri, __construct, arginfo_phalcon_http_uri___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Http_Uri, __toString, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Uri, __unset, arginfo_phalcon_http_uri___unset, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Uri, __set, arginfo_phalcon_http_uri___set, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Uri, __get, arginfo_phalcon_http_uri___get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Uri, __isset, arginfo_phalcon_http_uri___isset, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Uri, getParts, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Uri, getPath, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Uri, build, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Uri, resolve, arginfo_phalcon_http_uri_resolve, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Uri, extend, arginfo_phalcon_http_uri_extend, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Uri, extendQuery, arginfo_phalcon_http_uri_extendquery, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Uri, extendPath, arginfo_phalcon_http_uri_extendpath, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Http\Uri initializer
 */
PHALCON_INIT_CLASS(Phalcon_Http_Uri){

	PHALCON_REGISTER_CLASS(Phalcon\\Http, Uri, http_uri, phalcon_http_uri_method_entry, 0);

	zend_declare_property_null(phalcon_http_uri_ce, SL("_parts"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Http\Uri constructor
 *
 * @param mixed $uri
 */
PHP_METHOD(Phalcon_Http_Uri, __construct)
{
	zval *uri = NULL, parts = {}, query = {}, params = {};

	phalcon_fetch_params(1, 0, 1, &uri);

	if (!uri || PHALCON_IS_EMPTY(uri)) {
		phalcon_update_property_empty_array(getThis(), SL("_parts"));
	} else if (Z_TYPE_P(uri) == IS_STRING) {
		PHALCON_MM_CALL_FUNCTION(&parts, "parse_url", uri);
		PHALCON_MM_ADD_ENTRY(&parts);
		if (phalcon_array_isset_fetch_str(&query, &parts, SL("query"), PH_READONLY)) {
			ZVAL_MAKE_REF(&params);
			PHALCON_MM_CALL_FUNCTION(NULL, "parse_str", &query, &params);
			ZVAL_UNREF(&params);
			phalcon_array_update_str(&parts, SL("query"), &params, 0);
		}

		phalcon_update_property(getThis(), SL("_parts"), &parts);
	} else if (Z_TYPE_P(uri) == IS_ARRAY) {
		phalcon_update_property(getThis(), SL("_parts"), uri);
	} else if (Z_TYPE_P(uri) == IS_OBJECT && Z_OBJCE_P(uri) == phalcon_http_uri_ce) {
		phalcon_read_property(&parts, uri, SL("_parts"), PH_SEPARATE);
		phalcon_update_property(getThis(), SL("_parts"), &parts);
		zval_ptr_dtor(&parts);
	} else {
		phalcon_update_property_empty_array(getThis(), SL("_parts"));
	}
	RETURN_MM();
}

/**
 * Magic __toString method returns uri
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Uri, __toString){

	PHALCON_RETURN_CALL_SELF("build");
}

/**
 * Magic __unset method
 *
 * @param string $key
 */
PHP_METHOD(Phalcon_Http_Uri, __unset){

	zval *key;

	phalcon_fetch_params(0, 1, 0, &key);

	phalcon_unset_property_array(getThis(), SL("_parts"), key);
}

/**
 * Magic __set method
 *
 * @param string $key
 */
PHP_METHOD(Phalcon_Http_Uri, __set){

	zval *key, *value;

	phalcon_fetch_params(0, 2, 0, &key, &value);

	phalcon_update_property_array(getThis(), SL("_parts"), key, value);
}

/**
 * Magic __get method
 *
 * @param string $key
 * @return string
 */
PHP_METHOD(Phalcon_Http_Uri, __get){

	zval *key, parts = {};

	phalcon_fetch_params(0, 1, 0, &key);

	phalcon_read_property(&parts, getThis(), SL("_parts"), PH_NOISY|PH_READONLY);

	if (!phalcon_array_isset_fetch(return_value, &parts, key,PH_COPY)) {
		 RETURN_NULL();
	}
}

/**
 * Magic __isset method
 *
 * @param string $key
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Uri, __isset){

	zval *key, parts = {};

	phalcon_fetch_params(0, 1, 0, &key);

	phalcon_read_property(&parts, getThis(), SL("_parts"), PH_NOISY|PH_READONLY);

	RETURN_BOOL(phalcon_array_isset(&parts, key));
}

/**
 * Returns parts
 *
 * @return array
 */
PHP_METHOD(Phalcon_Http_Uri, getParts){

	RETURN_MEMBER(getThis(), "_parts");
}

/**
 * Retrieve the URI path
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Uri, getPath)
{
	zval parts = {}, value = {};

	phalcon_read_property(&parts, getThis(), SL("_parts"), PH_NOISY|PH_READONLY);

	if (!phalcon_array_isset_fetch_str(&value, &parts, SL("path"), PH_READONLY)) {
		 RETURN_NULL();
	}

	RETURN_CTOR(&value);
}

/**
 * Returns uri
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Uri, build)
{
	zval parts = {}, uri = {}, scheme = {}, host = {}, user = {}, pass = {}, port = {}, path = {}, query = {}, fragment = {}, tmp = {};

	phalcon_read_property(&parts, getThis(), SL("_parts"), PH_NOISY|PH_READONLY);
	PHALCON_MM_INIT();
	if (phalcon_array_isset_fetch_str(&scheme, &parts, SL("scheme"), PH_READONLY) && PHALCON_IS_NOT_EMPTY(&scheme)) {
		if (phalcon_array_isset_fetch_str(&host, &parts, SL("host"), PH_READONLY) && PHALCON_IS_NOT_EMPTY(&host)) {
			if (phalcon_array_isset_fetch_str(&user, &parts, SL("user"), PH_READONLY) && PHALCON_IS_NOT_EMPTY(&user)) {
				if (phalcon_array_isset_fetch_str(&pass, &parts, SL("pass"), PH_READONLY) && PHALCON_IS_NOT_EMPTY(&pass)) {
					PHALCON_CONCAT_VSVSVSV(&uri, &scheme, "://", &user, ":", &pass, "@", &host);
					PHALCON_MM_ADD_ENTRY(&uri);
				} else {
					PHALCON_CONCAT_VSVSV(&uri, &scheme, "://", &user, "@", &host);
					PHALCON_MM_ADD_ENTRY(&uri);
				}
			} else {
				PHALCON_CONCAT_VSV(&uri, &scheme, "://", &host);
				PHALCON_MM_ADD_ENTRY(&uri);
			}
		} else {
			PHALCON_CONCAT_VS(&uri, &scheme, ":");
			PHALCON_MM_ADD_ENTRY(&uri);
		}
	}

	if (phalcon_array_isset_fetch_str(&port, &parts, SL("port"), PH_READONLY) && PHALCON_IS_NOT_EMPTY(&port)) {
		PHALCON_SCONCAT_SV(&uri, ":", &port);
		PHALCON_MM_ADD_ENTRY(&uri);
	}

	if (phalcon_array_isset_fetch_str(&path, &parts, SL("path"), PH_READONLY) && PHALCON_IS_NOT_EMPTY(&path)) {
		if (!phalcon_start_with_str(&path, SL("/"))) {
			PHALCON_SCONCAT_SV(&uri, "/", &path);
			PHALCON_MM_ADD_ENTRY(&uri);
		} else {
			PHALCON_SCONCAT(&uri, &path);
			PHALCON_MM_ADD_ENTRY(&uri);
		}
	}

	if (phalcon_array_isset_fetch_str(&query, &parts, SL("query"), PH_READONLY) && PHALCON_IS_NOT_EMPTY(&query)) {
		if (phalcon_http_build_query(&tmp, &query, "&") == SUCCESS) {
			PHALCON_SCONCAT_SV(&uri, "?", &tmp);
			zval_ptr_dtor(&tmp);
			PHALCON_MM_ADD_ENTRY(&uri);
		}
	}

	if (phalcon_array_isset_fetch_str(&fragment, &parts, SL("fragment"), PH_READONLY) && PHALCON_IS_NOT_EMPTY(&fragment)) {
		PHALCON_SCONCAT_SV(&uri, "#", &fragment);
		PHALCON_MM_ADD_ENTRY(&uri);
	}

	RETURN_MM_CTOR(&uri);
}

PHP_METHOD(Phalcon_Http_Uri, resolve)
{
	zval *uri, self = {};

	phalcon_fetch_params(1, 1, 0, &uri);

	object_init_ex(&self, phalcon_http_uri_ce);
	PHALCON_MM_ADD_ENTRY(&self);
	PHALCON_MM_CALL_METHOD(NULL, &self, "__construct", getThis());
	PHALCON_MM_CALL_METHOD(NULL, &self, "extend", uri);

	RETURN_MM_CTOR(&self);
}

PHP_METHOD(Phalcon_Http_Uri, extend)
{
	zval *uri, parts = {}, parts2 = {}, path = {}, path2 = {};

	phalcon_fetch_params(1, 1, 0, &uri);

	phalcon_read_property(&parts, getThis(), SL("_parts"), PH_NOISY|PH_READONLY);

	if (Z_TYPE_P(uri) != IS_OBJECT || Z_OBJCE_P(uri) != phalcon_http_uri_ce) {
		zval self = {};
		object_init_ex(&self, phalcon_http_uri_ce);
		PHALCON_MM_ADD_ENTRY(&self);
		PHALCON_MM_CALL_METHOD(NULL, &self, "__construct", uri);
		PHALCON_MM_CALL_METHOD(&parts2, &self, "getParts");
	} else {
		PHALCON_MM_CALL_METHOD(&parts2, uri, "getParts");
	}
	zval_ptr_dtor(&parts2);

	if (phalcon_array_isset_fetch_str(&path, &parts, SL("path"), PH_READONLY)) {
		if (phalcon_array_isset_fetch_str(&path2, &parts2, SL("path"), PH_READONLY)) {
			if (!phalcon_start_with_str(&path2, SL("/"))) {
				zval tmp = {};
				if (!phalcon_end_with_str(&path, SL("/"))) {
					PHALCON_CONCAT_VSV(&tmp, &path, "/", &path2);
				} else {
					PHALCON_CONCAT_VV(&tmp, &path, &path2);
				}
				phalcon_array_update_str(&parts, SL("path"), &tmp, 0);
				phalcon_array_unset_str(&parts2, SL("path"), 0);
			}
		}
	}

	phalcon_array_merge_recursive_n(&parts, &parts2);

	RETURN_MM_THIS();
}

PHP_METHOD(Phalcon_Http_Uri, extendQuery){

	zval *params, parts = {}, query = {};

	phalcon_fetch_params(0, 1, 0, &params);

	phalcon_read_property(&parts, getThis(), SL("_parts"), PH_NOISY|PH_READONLY);

	if (phalcon_array_isset_fetch_str(&query, &parts, SL("query"), PH_READONLY)) {
		phalcon_array_merge_recursive_n(&query, params);
	} else {
		phalcon_array_update_str(&parts, SL("query"), params, PH_COPY);
	}

	RETURN_THIS();
}

PHP_METHOD(Phalcon_Http_Uri, extendPath){

	zval *uri_path, parts = {}, path = {};

	phalcon_fetch_params(0, 1, 0, &uri_path);

	phalcon_read_property(&parts, getThis(), SL("_parts"), PH_NOISY|PH_READONLY);

	if (!phalcon_start_with_str(uri_path, SL("/"))) {
		if (phalcon_array_isset_fetch_str(&path, &parts, SL("path"), PH_READONLY)) {
			zval tmp = {};
			if (!phalcon_end_with_str(&path, SL("/"))) {
				PHALCON_CONCAT_VSV(&tmp, &path, "/", uri_path);
			} else {
				PHALCON_CONCAT_VV(&tmp, &path, uri_path);
			}
			phalcon_array_update_str(&parts, SL("path"), &tmp, 0);
		}
	} else {
		phalcon_array_update_str(&parts, SL("path"), uri_path, PH_COPY);
	}

	RETURN_THIS();
}
