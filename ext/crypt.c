
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

#include "crypt.h"
#include "cryptinterface.h"
#include "crypt/exception.h"

#include <Zend/zend_smart_str.h>
#include <ext/standard/php_string.h>
#include <ext/standard/base64.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/exception.h"
#include "kernel/operators.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/concat.h"


/**
 * Phalcon\Crypt
 *
 * Provides encryption facilities to phalcon applications
 *
 *<code>
 *	$crypt = new Phalcon\Crypt();
 *
 *	$key = 'le password';
 *	$text = 'This is a secret text';
 *
 *	$encrypted = $crypt->encrypt($text, $key);
 *
 *	echo $crypt->decrypt($encrypted, $key);
 *</code>
 */
zend_class_entry *phalcon_crypt_ce;

PHP_METHOD(Phalcon_Crypt, setMethod);
PHP_METHOD(Phalcon_Crypt, getMethod);
PHP_METHOD(Phalcon_Crypt, setKey);
PHP_METHOD(Phalcon_Crypt, getKey);
PHP_METHOD(Phalcon_Crypt, setOptions);
PHP_METHOD(Phalcon_Crypt, getOptions);
PHP_METHOD(Phalcon_Crypt, setPadding);
PHP_METHOD(Phalcon_Crypt, getPadding);
PHP_METHOD(Phalcon_Crypt, encrypt);
PHP_METHOD(Phalcon_Crypt, decrypt);
PHP_METHOD(Phalcon_Crypt, encryptBase64);
PHP_METHOD(Phalcon_Crypt, decryptBase64);
PHP_METHOD(Phalcon_Crypt, getAvailableMethods);
PHP_METHOD(Phalcon_Crypt, beforeEncrypt);
PHP_METHOD(Phalcon_Crypt, afterEncrypt);
PHP_METHOD(Phalcon_Crypt, beforeDecrypt);
PHP_METHOD(Phalcon_Crypt, afterDecrypt);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_crypt_beforeencrypt, 0, 0, 1)
	ZEND_ARG_INFO(0, handler)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_crypt_afterencrypt, 0, 0, 1)
	ZEND_ARG_INFO(0, handler)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_crypt_beforedecrypt, 0, 0, 1)
	ZEND_ARG_INFO(0, handler)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_crypt_afterdecrypt, 0, 0, 1)
	ZEND_ARG_INFO(0, handler)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_crypt_method_entry[] = {
	PHP_ME(Phalcon_Crypt, setMethod, arginfo_phalcon_cryptinterface_setmethod, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, getMethod, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, setKey, arginfo_phalcon_cryptinterface_setkey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, getKey, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, setOptions, arginfo_phalcon_cryptinterface_setoptions, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, getOptions, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, encrypt, arginfo_phalcon_cryptinterface_encrypt, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, decrypt, arginfo_phalcon_cryptinterface_decrypt, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, encryptBase64, arginfo_phalcon_cryptinterface_encryptbase64, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, decryptBase64, arginfo_phalcon_cryptinterface_decryptbase64, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, getAvailableMethods, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, beforeEncrypt, arginfo_phalcon_crypt_beforeencrypt, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, afterEncrypt, arginfo_phalcon_crypt_afterencrypt, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, beforeDecrypt, arginfo_phalcon_crypt_beforedecrypt, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, afterDecrypt, arginfo_phalcon_crypt_afterdecrypt, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


/**
 * Phalcon\Crypt initializer
 */
PHALCON_INIT_CLASS(Phalcon_Crypt){

	PHALCON_REGISTER_CLASS(Phalcon, Crypt, crypt, phalcon_crypt_method_entry, 0);

	zend_declare_property_null(phalcon_crypt_ce, SL("_key"), ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_crypt_ce, SL("_method"), "aes-256-cbc", ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_crypt_ce, SL("_options"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_crypt_ce, SL("_beforeEncrypt"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_crypt_ce, SL("_afterEncrypt"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_crypt_ce, SL("_beforeDecrypt"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_crypt_ce, SL("_afterDecrypt"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_crypt_ce, 1, phalcon_cryptinterface_ce);

	return SUCCESS;
}

/**
 * Sets the cipher method
 *
 * @param string $method
 * @return Phalcon\Encrypt
 */
PHP_METHOD(Phalcon_Crypt, setMethod){

	zval *method, methods = {};

	phalcon_fetch_params(0, 1, 0, &method);

	PHALCON_CALL_FUNCTION(&methods, "openssl_get_cipher_methods");

	if (Z_TYPE(methods) != IS_ARRAY || !phalcon_fast_in_array(method, &methods)) {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_crypt_exception_ce, "Cipher method not available: %s", method);
		return;
	}

	phalcon_update_property_zval(getThis(), SL("_method"), method);
	RETURN_THIS();
}

/**
 * Returns the current cipher method
 *
 * @return string
 */
PHP_METHOD(Phalcon_Crypt, getMethod){


	RETURN_MEMBER(getThis(), "_method");
}

/**
 * Sets the encryption key
 *
 * @param string $key
 * @return Phalcon\Encrypt
 */
PHP_METHOD(Phalcon_Crypt, setKey){

	zval *key;

	phalcon_fetch_params(0, 1, 0, &key);
	PHALCON_ENSURE_IS_STRING(key);

	phalcon_update_property_zval(getThis(), SL("_key"), key);
	RETURN_THIS();
}


/**
 * Returns the encryption key
 *
 * @return string
 */
PHP_METHOD(Phalcon_Crypt, getKey){


	RETURN_MEMBER(getThis(), "_key");
}

/**
 * Sets the options
 *
 * @param int $options
 * @return Phalcon\CryptInterface
 */
PHP_METHOD(Phalcon_Crypt, setOptions) {

	zval *options;

	phalcon_fetch_params(0, 1, 0, &options);
	PHALCON_ENSURE_IS_LONG(options);

	phalcon_update_property_zval(getThis(), SL("_options"), options);
	RETURN_THIS();
}

/**
 * Returns the options
 *
 * @brief int Phalcon\Crypt::getOptions()
 * @return int
 */
PHP_METHOD(Phalcon_Crypt, getOptions) {

	RETURN_MEMBER(getThis(), "_options");
}

/**
 * Encrypts a text
 *
 *<code>
 *	$encrypted = $crypt->encrypt("Ultra-secret text", "encrypt password");
 *</code>
 *
 * @param string $text
 * @param string $key
 * @param int $option
 * @return string
 */
PHP_METHOD(Phalcon_Crypt, encrypt){

	zval *source, *key = NULL, *options = NULL, handler = {}, arguments = {}, value = {}, text = {}, encrypt_key = {}, encrypt_options = {};
	zval method = {}, iv_size = {}, iv = {}, encrypt = {};

	phalcon_fetch_params(0, 1, 2, &source, &key, &options);

	if (phalcon_function_exists_ex(SL("openssl_decrypt")) == FAILURE) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_crypt_exception_ce, "openssl extension is required");
		return;
	}

	phalcon_read_property(&handler, getThis(), SL("_beforeEncrypt"), PH_NOISY|PH_READONLY);

	if (phalcon_is_callable(&handler)) {
		PHALCON_SEPARATE_PARAM(source);

		array_init_size(&arguments, 1);
		phalcon_array_append(&arguments, source, PH_COPY);

		PHALCON_CALL_USER_FUNC_ARRAY(&value, &handler, &arguments);

		source = &value;
	}

	/* Do not use make_printable_zval() here: we need the conversion with type juggling */
	if (Z_TYPE_P(source) != IS_STRING) {
		phalcon_cast(&text, source, IS_STRING);
	} else {
		ZVAL_COPY_VALUE(&text, source);
	}

	if (!key || Z_TYPE_P(key) == IS_NULL) {
		phalcon_return_property(&encrypt_key, getThis(), SL("_key"));
	} else {
		PHALCON_CPY_WRT_CTOR(&encrypt_key, key);
		if (Z_TYPE(encrypt_key) != IS_STRING) {
			convert_to_string(&encrypt_key);
		}
	}

	if (PHALCON_IS_EMPTY(&encrypt_key)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_crypt_exception_ce, "Encryption key cannot be empty");
		return;
	}

	if (!options || Z_TYPE_P(options) == IS_NULL) {
		phalcon_return_property(&encrypt_options, getThis(), SL("_options"));
	} else {
		PHALCON_CPY_WRT_CTOR(&encrypt_options, options);
	}

	phalcon_read_property(&method, getThis(), SL("_method"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_FUNCTION(&iv_size, "openssl_cipher_iv_length", &method);
	PHALCON_CALL_FUNCTION(&iv, "openssl_random_pseudo_bytes", &iv_size);
	PHALCON_CALL_FUNCTION(&encrypt, "openssl_encrypt", &text, &method, &encrypt_key, &encrypt_options, &iv);
	PHALCON_CONCAT_VV(return_value, &iv, &encrypt);

	phalcon_read_property(&handler, getThis(), SL("_afterEncrypt"), PH_NOISY|PH_READONLY);

	if (phalcon_is_callable(&handler)) {
		array_init_size(&arguments, 1);
		phalcon_array_append(&arguments, return_value, PH_COPY);

		PHALCON_CALL_USER_FUNC_ARRAY(&value, &handler, &arguments);

		RETURN_CTOR(&value);
	}
}

/**
 * Decrypts an encrypted text
 *
 *<code>
 *	echo $crypt->decrypt($encrypted, "decrypt password");
 *</code>
 *
 * @param string $text
 * @param string $key
 * @param int $options
 * @return string
 */
PHP_METHOD(Phalcon_Crypt, decrypt){

	zval *source, *key = NULL, *options = NULL, handler = {}, arguments = {}, value = {}, text = {}, encrypt_key = {}, encrypt_options = {};
	zval method = {}, iv_size = {}, iv = {}, text_to_decipher = {};

	phalcon_fetch_params(0, 1, 2, &source, &key, &options);

	if (phalcon_function_exists_ex(SL("openssl_encrypt")) == FAILURE) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_crypt_exception_ce, "openssl extension is required");
		return;
	}

	phalcon_read_property(&handler, getThis(), SL("_beforeDecrypt"), PH_NOISY|PH_READONLY);

	if (phalcon_is_callable(&handler)) {
		PHALCON_SEPARATE_PARAM(source);

		array_init_size(&arguments, 1);
		phalcon_array_append(&arguments, source, PH_COPY);

		PHALCON_CALL_USER_FUNC_ARRAY(&value, &handler, &arguments);

		source = &value;
	}

	/* Do not use make_printable_zval() here: we need the conversion with type juggling */
	if (Z_TYPE_P(source) != IS_STRING) {
		phalcon_cast(&text, source, IS_STRING);
	} else {
		PHALCON_CPY_WRT_CTOR(&text, source);
	}

	if (!key || Z_TYPE_P(key) == IS_NULL) {
		phalcon_return_property(&encrypt_key, getThis(), SL("_key"));
	} else {
		PHALCON_CPY_WRT_CTOR(&encrypt_key, key);
		if (Z_TYPE(encrypt_key) != IS_STRING) {
			convert_to_string(&encrypt_key);
		}
	}

	if (!options || Z_TYPE_P(options) == IS_NULL) {
		phalcon_return_property(&encrypt_options, getThis(), SL("_options"));
	} else {
		ZVAL_COPY_VALUE(&encrypt_options, options);
	}

	phalcon_read_property(&method, getThis(), SL("_method"), PH_NOISY|PH_READONLY);
	PHALCON_CALL_FUNCTION(&iv_size, "openssl_cipher_iv_length", &method);

	if (Z_LVAL(iv_size) <= 0) {
		ZVAL_NULL(&iv);
		ZVAL_COPY_VALUE(&text_to_decipher, &text);
	} else {
		phalcon_substr(&iv, &text, 0, Z_LVAL(iv_size));
		phalcon_substr(&text_to_decipher, &text, Z_LVAL(iv_size), 0);
	}

	PHALCON_CALL_FUNCTION(return_value, "openssl_decrypt", &text_to_decipher, &method, &encrypt_key, &encrypt_options, &iv);
	if (unlikely(Z_TYPE_P(return_value) != IS_STRING)) {
		convert_to_string(return_value);
	}

	phalcon_read_property(&handler, getThis(), SL("_afterDecrypt"), PH_NOISY|PH_READONLY);

	if (phalcon_is_callable(&handler)) {
		array_init_size(&arguments, 1);
		phalcon_array_append(&arguments, return_value, PH_COPY);

		PHALCON_CALL_USER_FUNC_ARRAY(&value, &handler, &arguments);

		RETURN_CTOR(&value);
	}
}

/**
 * Encrypts a text returning the result as a base64 string
 *
 * @param string $text
 * @param string $key
 * @return string
 */
PHP_METHOD(Phalcon_Crypt, encryptBase64){

	zval *text, *key = NULL, *safe = NULL, encrypt_value = {};

	phalcon_fetch_params(0, 1, 2, &text, &key, &safe);

	if (!key) {
		key = &PHALCON_GLOBAL(z_null);
	}

	if (!safe) {
		safe = &PHALCON_GLOBAL(z_false);
	}

	PHALCON_CALL_METHOD(&encrypt_value, getThis(), "encrypt", text, key);

	phalcon_base64_encode(return_value, &encrypt_value);
	if (zend_is_true(safe)) {
		php_strtr(Z_STRVAL_P(return_value), Z_STRLEN_P(return_value), "+/", "-_", 2);
	}
}

/**
 * Decrypt a text that is coded as a base64 string
 *
 * @param string $text
 * @param string $key
 * @return string
 */
PHP_METHOD(Phalcon_Crypt, decryptBase64){

	zval *text, *key = NULL, *safe = NULL, decrypt_text = {}, decrypt_value = {};

	phalcon_fetch_params(0, 1, 2, &text, &key, &safe);
	PHALCON_ENSURE_IS_STRING(text);

	if (!key) {
		key = &PHALCON_GLOBAL(z_null);
	}

	if (!safe) {
		safe = &PHALCON_GLOBAL(z_false);
	}

	if (zend_is_true(safe)) {
		ZVAL_NEW_STR(&decrypt_text, zend_string_dup(Z_STR_P(text), 0));
		php_strtr(Z_STRVAL(decrypt_text), Z_STRLEN(decrypt_text), "-_", "+/", 2);
	} else {
		ZVAL_COPY_VALUE(&decrypt_text, text);
	}

	phalcon_base64_decode(&decrypt_value, &decrypt_text);

	PHALCON_RETURN_CALL_METHOD(getThis(), "decrypt", &decrypt_value, key);
}

/**
 * Returns a list of available modes
 *
 * @return array
 */
PHP_METHOD(Phalcon_Crypt, getAvailableMethods){

	PHALCON_RETURN_CALL_FUNCTION("openssl_get_cipher_methods");
}

/**
 * Adds a internal hooks before encrypts a text
 *
 * @param callable $handler
 */
PHP_METHOD(Phalcon_Crypt, beforeEncrypt){

	zval *handler;

	phalcon_fetch_params(0, 1, 0, &handler);

	if (Z_TYPE_P(handler) != IS_OBJECT && !phalcon_is_callable(handler)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_crypt_exception_ce, "Handler must be an callable");
		return;
	}

	phalcon_update_property_zval(getThis(), SL("_beforeEncrypt"), handler);
	RETURN_THIS();
}

/**
 * Adds a internal hooks after encrypts a text
 *
 * @param callable $handler
 */
PHP_METHOD(Phalcon_Crypt, afterEncrypt){

	zval *handler;

	phalcon_fetch_params(0, 1, 0, &handler);

	if (Z_TYPE_P(handler) != IS_OBJECT && !phalcon_is_callable(handler)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_crypt_exception_ce, "Handler must be an callable");
		return;
	}

	phalcon_update_property_zval(getThis(), SL("_afterEncrypt"), handler);
	RETURN_THIS();
}

/**
 * Adds a internal hooks before decrypts an encrypted text
 *
 * @param callable $handler
 */
PHP_METHOD(Phalcon_Crypt, beforeDecrypt){

	zval *handler;

	phalcon_fetch_params(0, 1, 0, &handler);

	if (Z_TYPE_P(handler) != IS_OBJECT && !phalcon_is_callable(handler)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_crypt_exception_ce, "Handler must be an callable");
		return;
	}

	phalcon_update_property_zval(getThis(), SL("_beforeDecrypt"), handler);
	RETURN_THIS();
}

/**
 * Adds a internal hooks after decrypts an encrypted text
 *
 * @param callable $handler
 */
PHP_METHOD(Phalcon_Crypt, afterDecrypt){

	zval *handler;

	phalcon_fetch_params(0, 1, 0, &handler);

	if (Z_TYPE_P(handler) != IS_OBJECT && !phalcon_is_callable(handler)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_crypt_exception_ce, "Handler must be an callable");
		return;
	}

	phalcon_update_property_zval(getThis(), SL("_afterDecrypt"), handler);
	RETURN_THIS();
}
