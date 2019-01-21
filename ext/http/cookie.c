
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

#include "http/cookie.h"
#include "http/cookie/exception.h"
#include "cryptinterface.h"
#include "diinterface.h"
#include "di/injectable.h"
#include "filterinterface.h"
#include "session/adapterinterface.h"

#include <ext/standard/head.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"
#include "kernel/array.h"
#include "kernel/file.h"
#include "kernel/concat.h"

#include "interned-strings.h"

/**
 * Phalcon\Http\Cookie
 *
 * Provide OO wrappers to manage a HTTP cookie
 */
zend_class_entry *phalcon_http_cookie_ce;

PHP_METHOD(Phalcon_Http_Cookie, __construct);
PHP_METHOD(Phalcon_Http_Cookie, setValue);
PHP_METHOD(Phalcon_Http_Cookie, getValue);
PHP_METHOD(Phalcon_Http_Cookie, send);
PHP_METHOD(Phalcon_Http_Cookie, restore);
PHP_METHOD(Phalcon_Http_Cookie, delete);
PHP_METHOD(Phalcon_Http_Cookie, useEncryption);
PHP_METHOD(Phalcon_Http_Cookie, isUsingEncryption);
PHP_METHOD(Phalcon_Http_Cookie, setExpiration);
PHP_METHOD(Phalcon_Http_Cookie, getExpiration);
PHP_METHOD(Phalcon_Http_Cookie, setPath);
PHP_METHOD(Phalcon_Http_Cookie, getPath);
PHP_METHOD(Phalcon_Http_Cookie, setDomain);
PHP_METHOD(Phalcon_Http_Cookie, getDomain);
PHP_METHOD(Phalcon_Http_Cookie, setSecure);
PHP_METHOD(Phalcon_Http_Cookie, getSecure);
PHP_METHOD(Phalcon_Http_Cookie, setHttpOnly);
PHP_METHOD(Phalcon_Http_Cookie, getHttpOnly);
PHP_METHOD(Phalcon_Http_Cookie, __toString);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_cookie___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, expire)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, secure)
	ZEND_ARG_INFO(0, domain)
	ZEND_ARG_INFO(0, httpOnly)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_cookie_setvalue, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_cookie_getvalue, 0, 0, 0)
	ZEND_ARG_INFO(0, filters)
	ZEND_ARG_INFO(0, defaultValue)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_cookie_useencryption, 0, 0, 1)
	ZEND_ARG_INFO(0, useEncryption)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_cookie_setexpiration, 0, 0, 1)
	ZEND_ARG_INFO(0, expire)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_cookie_setpath, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_cookie_setdomain, 0, 0, 1)
	ZEND_ARG_INFO(0, domain)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_cookie_setsecure, 0, 0, 1)
	ZEND_ARG_INFO(0, secure)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_cookie_sethttponly, 0, 0, 1)
	ZEND_ARG_INFO(0, httpOnly)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_http_cookie_method_entry[] = {
	PHP_ME(Phalcon_Http_Cookie, __construct, arginfo_phalcon_http_cookie___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Http_Cookie, setValue, arginfo_phalcon_http_cookie_setvalue, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Cookie, getValue, arginfo_phalcon_http_cookie_getvalue, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Cookie, send, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Cookie, restore, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Cookie, delete, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Cookie, useEncryption, arginfo_phalcon_http_cookie_useencryption, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Cookie, isUsingEncryption, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Cookie, setExpiration, arginfo_phalcon_http_cookie_setexpiration, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Cookie, getExpiration, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Cookie, setPath, arginfo_phalcon_http_cookie_setpath, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Cookie, getPath, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Cookie, setDomain, arginfo_phalcon_http_cookie_setdomain, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Cookie, getDomain, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Cookie, setSecure, arginfo_phalcon_http_cookie_setsecure, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Cookie, getSecure, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Cookie, setHttpOnly, arginfo_phalcon_http_cookie_sethttponly, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Cookie, getHttpOnly, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Cookie, __toString, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Http\Cookie initializer
 */
PHALCON_INIT_CLASS(Phalcon_Http_Cookie){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Http, Cookie, http_cookie, phalcon_di_injectable_ce, phalcon_http_cookie_method_entry, 0);

	zend_declare_property_bool(phalcon_http_cookie_ce, SL("_readed"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_http_cookie_ce, SL("_restored"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_http_cookie_ce, SL("_useEncryption"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_cookie_ce, SL("_filter"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_cookie_ce, SL("_name"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_cookie_ce, SL("_value"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_cookie_ce, SL("_expire"), ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_http_cookie_ce, SL("_path"), "/", ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_cookie_ce, SL("_domain"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_cookie_ce, SL("_secure"), ZEND_ACC_PROTECTED);
	zend_declare_property_bool(phalcon_http_cookie_ce, SL("_httpOnly"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_http_cookie_ce, SL("_samesite"), "Lax", ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Http\Cookie constructor
 *
 * @param string $name
 * @param mixed $value
 * @param int $expire
 * @param string $path
 * @param boolean $secure
 * @param string $domain
 * @param boolean $httpOnly
 */
PHP_METHOD(Phalcon_Http_Cookie, __construct){

	zval *name, *value = NULL, *expire = NULL, *path = NULL, *secure = NULL, *domain = NULL, *http_only = NULL, *samesite = NULL;

	phalcon_fetch_params(0, 1, 7, &name, &value, &expire, &path, &secure, &domain, &http_only, &samesite);
	PHALCON_ENSURE_IS_STRING(name);

	if (!expire) {
		expire = &PHALCON_GLOBAL(z_zero);
	}

	phalcon_update_property(getThis(), SL("_name"), name);

	if (value && Z_TYPE_P(value) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_value"), value);
		phalcon_update_property_bool(getThis(), SL("_readed"), 1);
	}

	phalcon_update_property(getThis(), SL("_expire"), expire);

	if (path && Z_TYPE_P(path) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_path"), path);
	} else {
		zval tmp = {};
		ZVAL_STRINGL(&tmp, "/", 1);
		phalcon_update_property(getThis(), SL("_path"), &tmp);
		zval_ptr_dtor(&tmp);
	}

	if (secure && Z_TYPE_P(secure) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_secure"), secure);
	}

	if (domain && Z_TYPE_P(domain) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_domain"), domain);
	}

	if (http_only && Z_TYPE_P(http_only) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_httpOnly"), http_only);
	}

	if (samesite && Z_TYPE_P(samesite) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_samesite"), samesite);
	}
}

/**
 * Sets the cookie's value
 *
 * @param string $value
 * @return Phalcon\Http\CookieInterface
 */
PHP_METHOD(Phalcon_Http_Cookie, setValue){

	zval *value;

	phalcon_fetch_params(0, 1, 0, &value);

	phalcon_update_property(getThis(), SL("_value"), value);
	phalcon_update_property_bool(getThis(), SL("_readed"), 1);
	RETURN_THIS();
}

/**
 * Returns the cookie's value
 *
 * @param string|array $filters
 * @param string $defaultValue
 * @return mixed
 */
PHP_METHOD(Phalcon_Http_Cookie, getValue)
{
	zval *filters = NULL, *default_value = NULL, restored = {}, dependency_injector = {}, readed = {}, name = {}, *_COOKIE, value = {}, encryption = {};
	zval service = {}, crypt = {}, decrypted_value = {}, filter = {};

	phalcon_fetch_params(1, 0, 2, &filters, &default_value);

	if (!filters) {
		filters = &PHALCON_GLOBAL(z_null);
	}

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&restored, getThis(), SL("_restored"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&restored)) {
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "restore");
	}

	PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi");
	PHALCON_MM_ADD_ENTRY(&dependency_injector);
	if (Z_TYPE(dependency_injector) != IS_OBJECT) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_http_cookie_exception_ce, "A dependency injection object is required to access the 'filter' service");
		return;
	}

	phalcon_read_property(&readed, getThis(), SL("_readed"), PH_NOISY|PH_READONLY);
	if (PHALCON_IS_FALSE(&readed)) {
		phalcon_read_property(&name, getThis(), SL("_name"), PH_NOISY|PH_READONLY);

		_COOKIE = phalcon_get_global_str(SL("_COOKIE"));
		if (phalcon_array_isset_fetch(&value, _COOKIE, &name, PH_READONLY)) {
			phalcon_read_property(&encryption, getThis(), SL("_useEncryption"), PH_NOISY|PH_READONLY);
			if (zend_is_true(&encryption) && PHALCON_IS_NOT_EMPTY(&value)) {
				ZVAL_STR(&service, IS(crypt));

				PHALCON_MM_CALL_METHOD(&crypt, &dependency_injector, "getshared", &service);
				PHALCON_MM_ADD_ENTRY(&crypt);
				PHALCON_MM_VERIFY_INTERFACE(&crypt, phalcon_cryptinterface_ce);

				/**
				 * Decrypt the value also decoding it with base64
				 */
				PHALCON_MM_CALL_METHOD(&decrypted_value, &crypt, "decryptbase64", &value);
				PHALCON_MM_ADD_ENTRY(&decrypted_value);
			} else {
				ZVAL_COPY_VALUE(&decrypted_value, &value);
			}

			/**
			 * Update the decrypted value
			 */
			phalcon_update_property(getThis(), SL("_value"), &decrypted_value);
			if (Z_TYPE_P(filters) != IS_NULL) {
				phalcon_read_property(&filter, getThis(), SL("_filter"), PH_READONLY);
				if (Z_TYPE(filter) != IS_OBJECT) {
					ZVAL_STR(&service, IS(filter));

					PHALCON_MM_CALL_METHOD(&filter, &dependency_injector, "getshared", &service);
					PHALCON_MM_ADD_ENTRY(&filter);
					PHALCON_MM_VERIFY_INTERFACE(&filter, phalcon_filterinterface_ce);
					phalcon_update_property(getThis(), SL("_filter"), &filter);
				}

				PHALCON_MM_RETURN_CALL_METHOD(&filter, "sanitize", &decrypted_value, filters);
				RETURN_MM();
			}

			/**
			 * Return the value without filtering
			 */
			RETURN_MM_CTOR(&decrypted_value);
			return;
		}
		RETURN_MM_CTOR(default_value);
	}

	phalcon_read_property(&value, getThis(), SL("_value"), PH_READONLY);

	RETURN_MM_CTOR(&value);
}

/**
 * Sends the cookie to the HTTP client
 * Stores the cookie definition in session
 *
 * @return Phalcon\Http\Cookie
 */
PHP_METHOD(Phalcon_Http_Cookie, send){

	zval name = {}, value = {}, expire = {}, domain = {}, path = {}, secure = {}, http_only = {}, samesite = {}, dependency_injector = {};
	zval service = {}, has_session = {}, definition = {}, session = {}, key = {}, encryption = {}, crypt = {}, encrypt_value = {};

	PHALCON_MM_INIT();

	phalcon_read_property(&name, getThis(), SL("_name"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&value, getThis(), SL("_value"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&expire, getThis(), SL("_expire"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&domain, getThis(), SL("_domain"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&path, getThis(), SL("_path"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&secure, getThis(), SL("_secure"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&http_only, getThis(), SL("_httpOnly"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&samesite, getThis(), SL("_samesite"), PH_NOISY|PH_READONLY);
	PHALCON_MM_CALL_METHOD(&dependency_injector, getThis(), "getdi");
	PHALCON_MM_ADD_ENTRY(&dependency_injector);

	if (Z_TYPE(dependency_injector) == IS_OBJECT) {
		ZVAL_STR(&service, IS(session));

		PHALCON_MM_CALL_METHOD(&has_session, &dependency_injector, "has", &service);
		PHALCON_MM_ADD_ENTRY(&has_session);
		if (zend_is_true(&has_session)) {
			array_init(&definition);
			PHALCON_MM_ADD_ENTRY(&has_session);
			if (!PHALCON_IS_LONG(&expire, 0)) {
				phalcon_array_update_str(&definition, SL("expire"), &expire, PH_COPY);
			}

			if (PHALCON_IS_NOT_EMPTY(&path)) {
				phalcon_array_update_str(&definition, SL("path"), &path, PH_COPY);
			}

			if (PHALCON_IS_NOT_EMPTY(&domain)) {
				phalcon_array_update_string(&definition, IS(domain), &domain, PH_COPY);
			}

			if (PHALCON_IS_NOT_EMPTY(&secure)) {
				phalcon_array_update_str(&definition, SL("secure"), &secure, PH_COPY);
			}

			if (PHALCON_IS_NOT_EMPTY(&http_only)) {
				phalcon_array_update_str(&definition, SL("httpOnly"), &http_only, PH_COPY);
			}

			/**
			 * The definition is stored in session
			 */
			if (phalcon_fast_count_ev(&definition)) {
				PHALCON_MM_CALL_METHOD(&session, &dependency_injector, "getshared", &service);
				PHALCON_MM_ADD_ENTRY(&session);

				if (Z_TYPE(session) != IS_NULL) {
					PHALCON_MM_VERIFY_INTERFACE(&session, phalcon_session_adapterinterface_ce);

					PHALCON_CONCAT_SV(&key, "_PHCOOKIE_", &name);
					PHALCON_MM_ADD_ENTRY(&key);
					PHALCON_MM_CALL_METHOD(NULL, &session, "set", &key, &definition);
				}
			}
		}
	}

	phalcon_read_property(&encryption, getThis(), SL("_useEncryption"), PH_READONLY);
	if (zend_is_true(&encryption) && PHALCON_IS_NOT_EMPTY(&value)) {
		if (Z_TYPE(dependency_injector) != IS_OBJECT) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_http_cookie_exception_ce, "A dependency injection object is required to access the 'filter' service");
			return;
		}

		ZVAL_STR(&service, IS(crypt));

		PHALCON_MM_CALL_METHOD(&crypt, &dependency_injector, "getshared", &service);
		PHALCON_MM_ADD_ENTRY(&crypt);
		PHALCON_MM_VERIFY_INTERFACE(&crypt, phalcon_cryptinterface_ce);

		/**
		 * Encrypt the value also coding it with base64
		 */
		PHALCON_MM_CALL_METHOD(&encrypt_value, &crypt, "encryptbase64", &value);
		PHALCON_MM_ADD_ENTRY(&encrypt_value);
	} else {
		ZVAL_COPY_VALUE(&encrypt_value, &value);
	}

	/**
	 * Sets the cookie using the standard 'setcookie' function
	 */
	convert_to_string_ex(&name);
	convert_to_long_ex(&expire);
	convert_to_string_ex(&domain);
	convert_to_string_ex(&path);
	convert_to_long_ex(&secure);
	convert_to_long_ex(&http_only);
	convert_to_string_ex(&encrypt_value);
#if PHP_VERSION_ID >= 70300
	php_setcookie(Z_STR(name), Z_STR(encrypt_value), Z_LVAL(expire), Z_STR(path), Z_STR(domain), Z_LVAL(secure), Z_LVAL(http_only), Z_STR(samesite), 1);
#else
	php_setcookie(Z_STR(name), Z_STR(encrypt_value), Z_LVAL(expire), Z_STR(path), Z_STR(domain), Z_LVAL(secure), 1, Z_LVAL(http_only));
#endif

	RETURN_MM_THIS();
}

/**
 * Reads the cookie-related info from the SESSION to restore the cookie as it was set
 * This method is automatically called internally so normally you don't need to call it
 *
 * @return Phalcon\Http\Cookie
 */
PHP_METHOD(Phalcon_Http_Cookie, restore)
{
	zval restored = {}, dependency_injector = {}, service = {}, session = {}, name = {}, key = {}, definition = {}, expire = {};
	zval domain = {}, path = {}, secure = {}, http_only = {};

	PHALCON_MM_INIT();
	phalcon_read_property(&restored, getThis(), SL("_restored"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&restored)) {
		PHALCON_CALL_METHOD(&dependency_injector, getThis(), "getdi");
		PHALCON_MM_ADD_ENTRY(&dependency_injector);
		if (Z_TYPE(dependency_injector) == IS_OBJECT) {
			ZVAL_STR(&service, IS(session));

			PHALCON_MM_CALL_METHOD(&session, &dependency_injector, "getshared", &service);
			PHALCON_MM_ADD_ENTRY(&session);
			PHALCON_MM_VERIFY_INTERFACE(&session, phalcon_session_adapterinterface_ce);

			phalcon_read_property(&name, getThis(), SL("_name"), PH_READONLY);

			PHALCON_CONCAT_SV(&key, "_PHCOOKIE_", &name);
			PHALCON_MM_ADD_ENTRY(&key);
			PHALCON_MM_CALL_METHOD(&definition, &session, "get", &key);
			PHALCON_MM_ADD_ENTRY(&definition);
			if (Z_TYPE(definition) == IS_ARRAY) {
				if (phalcon_array_isset_fetch_str(&expire, &definition, SL("expire"), PH_READONLY)) {
					phalcon_update_property(getThis(), SL("_expire"), &expire);
				}
				if (phalcon_array_isset_fetch_str(&domain, &definition, SL("domain"), PH_READONLY)) {
					phalcon_update_property(getThis(), SL("_domain"), &domain);
				}

				if (phalcon_array_isset_fetch_str(&path, &definition, SL("path"), PH_READONLY)) {
					phalcon_update_property(getThis(), SL("_path"), &path);
				}

				if (phalcon_array_isset_fetch_str(&secure, &definition, SL("secure"), PH_READONLY)) {
					phalcon_update_property(getThis(), SL("_secure"), &secure);
				}

				if (phalcon_array_isset_fetch_str(&http_only, &definition, SL("httpOnly"), PH_READONLY)) {
					phalcon_update_property(getThis(), SL("_httpOnly"), &http_only);
				}
			}
		}

		phalcon_update_property_bool(getThis(), SL("_restored"), 1);
	}

	RETURN_MM_THIS();
}

/**
 * Deletes the cookie by setting an expire time in the past
 *
 */
PHP_METHOD(Phalcon_Http_Cookie, delete)
{
	zval name = {}, domain = {}, path = {}, secure = {}, http_only = {}, samesite = {}, service = {}, session = {}, key = {};

	PHALCON_MM_INIT();

	phalcon_read_property(&name, getThis(), SL("_name"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&domain, getThis(), SL("_domain"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&path, getThis(), SL("_path"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&secure, getThis(), SL("_secure"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&http_only, getThis(), SL("_httpOnly"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&samesite, getThis(), SL("_samesite"), PH_NOISY|PH_READONLY);

	ZVAL_STR(&service, IS(session));

	PHALCON_MM_CALL_METHOD(&session, getThis(), "getresolveservice", &service);
	PHALCON_MM_ADD_ENTRY(&session);
	PHALCON_MM_VERIFY_INTERFACE(&session, phalcon_session_adapterinterface_ce);

	PHALCON_CONCAT_SV(&key, "_PHCOOKIE_", &name);
	PHALCON_MM_ADD_ENTRY(&key);
	PHALCON_MM_CALL_METHOD(NULL, &session, "remove", &key);

	phalcon_update_property_null(getThis(), SL("_value"));

	convert_to_string_ex(&name);
	convert_to_string_ex(&path);
	convert_to_string_ex(&domain);
	convert_to_long_ex(&secure);
	convert_to_long_ex(&http_only);
#if PHP_VERSION_ID >= 70300
	php_setcookie(Z_STR(name), NULL, time(NULL) - 691200, Z_STR(path), Z_STR(domain), Z_LVAL(secure), Z_LVAL(http_only), Z_STR(samesite), 1);
#else
	php_setcookie(Z_STR(name), NULL, time(NULL) - 691200, Z_STR(path), Z_STR(domain), Z_LVAL(secure), 1, Z_LVAL(http_only));
#endif
	RETURN_MM();
}

/**
 * Sets if the cookie must be encrypted/decrypted automatically
 *
 * @param boolean $useEncryption
 * @return Phalcon\Http\Cookie
 */
PHP_METHOD(Phalcon_Http_Cookie, useEncryption){

	zval *use_encryption;

	phalcon_fetch_params(0, 1, 0, &use_encryption);

	phalcon_update_property(getThis(), SL("_useEncryption"), use_encryption);
	RETURN_THIS();
}

/**
 * Check if the cookie is using implicit encryption
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Cookie, isUsingEncryption){


	RETURN_MEMBER(getThis(), "_useEncryption");
}

/**
 * Sets the cookie's expiration time
 *
 * @param int $expire
 * @return Phalcon\Http\Cookie
 */
PHP_METHOD(Phalcon_Http_Cookie, setExpiration){

	zval *expire, restored = {};

	phalcon_fetch_params(0, 1, 0, &expire);

	phalcon_read_property(&restored, getThis(), SL("_restored"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&restored)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "restore");
	}

	phalcon_update_property(getThis(), SL("_expire"), expire);

	RETURN_THIS();
}

/**
 * Returns the current expiration time
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Cookie, getExpiration){

	zval restored = {}, expire = {};

	phalcon_read_property(&restored, getThis(), SL("_restored"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&restored)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "restore");
	}

	phalcon_read_property(&expire, getThis(), SL("_expire"), PH_NOISY|PH_READONLY);

	RETURN_CTOR(&expire);
}

/**
 * Sets the cookie's expiration time
 *
 * @param string $path
 * @return Phalcon\Http\Cookie
 */
PHP_METHOD(Phalcon_Http_Cookie, setPath){

	zval *path, restored = {};

	phalcon_fetch_params(0, 1, 0, &path);

	phalcon_read_property(&restored, getThis(), SL("_restored"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&restored)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "restore");
	}

	phalcon_update_property(getThis(), SL("_path"), path);

	RETURN_THIS();
}

/**
 * Returns the current cookie's path
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Cookie, getPath){

	zval restored = {}, path = {};

	phalcon_read_property(&restored, getThis(), SL("_restored"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&restored)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "restore");
	}

	phalcon_read_property(&path, getThis(), SL("_path"), PH_NOISY|PH_READONLY);

	RETURN_CTOR(&path);
}

/**
 * Sets the domain that the cookie is available to
 *
 * @param string $domain
 * @return Phalcon\Http\Cookie
 */
PHP_METHOD(Phalcon_Http_Cookie, setDomain){

	zval *domain, restored = {};

	phalcon_fetch_params(0, 1, 0, &domain);

	phalcon_read_property(&restored, getThis(), SL("_restored"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&restored)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "restore");
	}

	phalcon_update_property(getThis(), SL("_domain"), domain);

	RETURN_THIS();
}

/**
 * Returns the domain that the cookie is available to
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Cookie, getDomain){

	zval restored = {}, domain = {};

	phalcon_read_property(&restored, getThis(), SL("_restored"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&restored)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "restore");
	}

	phalcon_read_property(&domain, getThis(), SL("_domain"), PH_NOISY|PH_READONLY);

	RETURN_CTOR(&domain);
}

/**
 * Sets if the cookie must only be sent when the connection is secure (HTTPS)
 *
 * @param boolean $secure
 * @return Phalcon\Http\Cookie
 */
PHP_METHOD(Phalcon_Http_Cookie, setSecure){

	zval *secure, restored = {};

	phalcon_fetch_params(0, 1, 0, &secure);

	phalcon_read_property(&restored, getThis(), SL("_restored"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&restored)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "restore");
	}

	phalcon_update_property(getThis(), SL("_secure"), secure);

	RETURN_THIS();
}

/**
 * Returns whether the cookie must only be sent when the connection is secure (HTTPS)
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Cookie, getSecure){

	zval restored = {}, secure = {};

	phalcon_read_property(&restored, getThis(), SL("_restored"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&restored)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "restore");
	}

	phalcon_read_property(&secure, getThis(), SL("_secure"), PH_NOISY|PH_READONLY);

	RETURN_CTOR(&secure);
}

/**
 * Sets if the cookie is accessible only through the HTTP protocol
 *
 * @param boolean $httpOnly
 * @return Phalcon\Http\Cookie
 */
PHP_METHOD(Phalcon_Http_Cookie, setHttpOnly){

	zval *http_only, restored = {};

	phalcon_fetch_params(0, 1, 0, &http_only);

	phalcon_read_property(&restored, getThis(), SL("_restored"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&restored)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "restore");
	}

	phalcon_update_property(getThis(), SL("_httpOnly"), http_only);

	RETURN_THIS();
}

/**
 * Returns if the cookie is accessible only through the HTTP protocol
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Cookie, getHttpOnly){

	zval restored = {}, http_only = {};

	phalcon_read_property(&restored, getThis(), SL("_restored"), PH_NOISY|PH_READONLY);
	if (!zend_is_true(&restored)) {
		PHALCON_CALL_METHOD(NULL, getThis(), "restore");
	}

	phalcon_read_property(&http_only, getThis(), SL("_httpOnly"), PH_NOISY|PH_READONLY);

	RETURN_CTOR(&http_only);
}

/**
 * Magic __toString method converts the cookie's value to string
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Http_Cookie, __toString){

	zval value = {}, e = {}, exception = {}, *m;

	phalcon_read_property(&value, getThis(), SL("_value"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(value) == IS_NULL) {
		if (FAILURE == phalcon_call_method(return_value, getThis(), "getvalue", 0, NULL)) {
			if (EG(exception)) {
				ZVAL_OBJ(&e, EG(exception));
				ZVAL_OBJ(&exception, zend_objects_clone_obj(&e));
				m = zend_read_property(Z_OBJCE(exception), &exception, SL("message"), 1, NULL);

				Z_TRY_ADDREF_P(m);
				if (Z_TYPE_P(m) != IS_STRING) {
					convert_to_string_ex(m);
				}

				zend_clear_exception();
				zend_error(E_ERROR, "%s", Z_STRVAL_P(m));
			}
		}

		convert_to_string(return_value);
		return;
	}

	RETURN_CTOR(&value);
}
