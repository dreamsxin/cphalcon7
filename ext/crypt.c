
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
	PHP_ME(Phalcon_Crypt, setPadding, arginfo_phalcon_cryptinterface_setpadding, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, getPadding, NULL, ZEND_ACC_PUBLIC)
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
	zend_declare_property_long(phalcon_crypt_ce, SL("_padding"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_crypt_ce, SL("_beforeEncrypt"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_crypt_ce, SL("_afterEncrypt"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_crypt_ce, SL("_beforeDecrypt"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_crypt_ce, SL("_afterDecrypt"), ZEND_ACC_PROTECTED);

	zend_declare_class_constant_long(phalcon_crypt_ce, SL("PADDING_DEFAULT"),        PHALCON_CRYPT_PADDING_DEFAULT);
	zend_declare_class_constant_long(phalcon_crypt_ce, SL("PADDING_ANSI_X_923"),     PHALCON_CRYPT_PADDING_ANSI_X_923);
	zend_declare_class_constant_long(phalcon_crypt_ce, SL("PADDING_PKCS7"),          PHALCON_CRYPT_PADDING_PKCS7);
	zend_declare_class_constant_long(phalcon_crypt_ce, SL("PADDING_ISO_10126"),      PHALCON_CRYPT_PADDING_ISO_10126);
	zend_declare_class_constant_long(phalcon_crypt_ce, SL("PADDING_ISO_IEC_7816_4"), PHALCON_CRYPT_PADDING_ISO_IEC_7816_4);
	zend_declare_class_constant_long(phalcon_crypt_ce, SL("PADDING_ZERO"),           PHALCON_CRYPT_PADDING_ZERO);
	zend_declare_class_constant_long(phalcon_crypt_ce, SL("PADDING_SPACE"),          PHALCON_CRYPT_PADDING_SPACE);

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
		zval_ptr_dtor(&methods);
		return;
	}

	phalcon_update_property(getThis(), SL("_method"), method);
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

	phalcon_update_property(getThis(), SL("_key"), key);
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

	phalcon_update_property(getThis(), SL("_options"), options);
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
 * Sets the padding
 *
 * @param int $padding
 * @return Phalcon\CryptInterface
 */
PHP_METHOD(Phalcon_Crypt, setPadding) {

	zval *padding;

	phalcon_fetch_params(0, 1, 0, &padding);

	phalcon_update_property(getThis(), SL("_padding"), padding);
	RETURN_THIS();
}

/**
 * Returns the padding
 *
 * @return int
 */
PHP_METHOD(Phalcon_Crypt, getPadding) {

	RETURN_MEMBER(getThis(), "_padding");
}

/**
 * @brief Adds padding @a padding_type to @a text
 * @param return_value Result, possibly padded
 * @param text Message to be padded
 * @param mode Encryption mode; padding is applied only in CBC or ECB mode
 * @param block_size Cipher block size
 * @param padding_type Padding scheme
 * @see http://www.di-mgt.com.au/cryptopad.html
 */
static void phalcon_crypt_pad_text(zval *return_value, zval *text, zval *mode, uint block_size, int padding_type)
{
	uint padding_size, i;
	char padding[256];
	char *str_mode;

	assert(Z_TYPE_P(text) == IS_STRING);
	assert(Z_TYPE_P(mode) == IS_STRING);

	padding_size = 0;
	str_mode = Z_STRVAL_P(mode);

	if (!strcmp(str_mode, "ecb") || !strcmp(str_mode, "cbc")) {

		padding_size = block_size - (Z_STRLEN_P(text) % block_size);
		if (padding_size >= 256) {
			RETURN_FALSE;
		}

		switch (padding_type) {
			case PHALCON_CRYPT_PADDING_ANSI_X_923:
				memset(padding, 0, padding_size - 1);
				padding[padding_size-1] = (unsigned char)padding_size;
				break;

			case PHALCON_CRYPT_PADDING_PKCS7:
				memset(padding, padding_size, padding_size);
				break;

			case PHALCON_CRYPT_PADDING_ISO_10126:
				for (i = 0; i < padding_size - 1; ++i) {
					padding[i] = (unsigned char)rand();
				}

				padding[padding_size - 1] = (unsigned char)padding_size;
				break;

			case PHALCON_CRYPT_PADDING_ISO_IEC_7816_4:
				padding[0] = 0x80;
				memset(padding + 1, 0, padding_size - 1);
				break;

			case PHALCON_CRYPT_PADDING_ZERO:
				memset(padding, 0, padding_size);
				break;

			case PHALCON_CRYPT_PADDING_SPACE:
				memset(padding, 0x20, padding_size);
				break;

			default:
				padding_size = 0;
				break;
		}
	}

	if (!padding_size) {
		ZVAL_ZVAL(return_value, text, 1, 0);
	} else {
		assert(padding_size <= block_size);
		phalcon_concat_vs(return_value, text, padding, padding_size, 0);
	}
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
	zval method = {}, pos = {}, mode = {}, cipher = {}, iv_size = {}, iv = {}, block_size = {}, padding = {}, padded = {}, encrypt = {};

	phalcon_fetch_params(0, 1, 2, &source, &key, &options);

	if (phalcon_function_exists_ex(SL("openssl_encrypt")) == FAILURE) {
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
		ZVAL_COPY(&text, source);
	}

	if (!key || Z_TYPE_P(key) == IS_NULL) {
		phalcon_read_property(&encrypt_key, getThis(), SL("_key"), PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&encrypt_key, key);
	}

	if (PHALCON_IS_EMPTY(&encrypt_key)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_crypt_exception_ce, "Encryption key cannot be empty");
		return;
	}

	if (!options || Z_TYPE_P(options) == IS_NULL) {
		phalcon_read_property(&encrypt_options, getThis(), SL("_options"), PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&encrypt_options, options);
	}

	phalcon_read_property(&method, getThis(), SL("_method"), PH_NOISY|PH_READONLY);
/*
	PHALCON_CALL_FUNCTION(&methods, "openssl_get_cipher_methods", &PHALCON_GLOBAL(z_true));

	if (!phalcon_fast_in_array(&method, &methods)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_crypt_exception_ce, "Cipher algorithm is unknown");
		zval_ptr_dtor(&methods);
		return;
	}
	zval_ptr_dtor(&methods);
*/

	phalcon_read_property(&padding, getThis(), SL("_padding"), PH_NOISY|PH_READONLY);
	if (PHALCON_GT_LONG(&padding, 0) && phalcon_fast_strrpos_str(&pos, &method, SL("-"))) {
		zval tmp = {};
		phalcon_substr(&mode, &method, Z_LVAL(pos) + 1, 0);
		phalcon_substr(&tmp, &method, 0, Z_LVAL(pos));
		phalcon_fast_str_replace_str(&cipher, "-", "", &tmp);
		zval_ptr_dtor(&tmp);
		phalcon_strtolower_inplace(&cipher);
	}

	PHALCON_CALL_FUNCTION(&iv_size, "openssl_cipher_iv_length", &method);

	PHALCON_CALL_FUNCTION(&iv, "openssl_random_pseudo_bytes", &iv_size);

	if (PHALCON_GT_LONG(&padding, 0)) {
		if (PHALCON_LE_LONG(&iv_size, 0) && zend_is_true(&cipher)) {
			PHALCON_CALL_FUNCTION(&block_size, "openssl_cipher_iv_length", &cipher);
		} else {
			ZVAL_COPY(&block_size, &iv_size);
		}
		phalcon_crypt_pad_text(&padded, &text, &mode, Z_LVAL(block_size), Z_LVAL(padding));
		zval_ptr_dtor(&block_size);
	} else {
		ZVAL_COPY(&padded, &text);
	}
	zval_ptr_dtor(&cipher);
	zval_ptr_dtor(&mode);
	zval_ptr_dtor(&text);

	PHALCON_CALL_FUNCTION(&encrypt, "openssl_encrypt", &padded, &method, &encrypt_key, &encrypt_options, &iv);
	zval_ptr_dtor(&padded);
	PHALCON_CONCAT_VV(return_value, &iv, &encrypt);
	zval_ptr_dtor(&iv);
	zval_ptr_dtor(&encrypt);

	phalcon_read_property(&handler, getThis(), SL("_afterEncrypt"), PH_NOISY|PH_READONLY);

	if (phalcon_is_callable(&handler)) {
		array_init_size(&arguments, 1);
		phalcon_array_append(&arguments, return_value, PH_COPY);

		PHALCON_CALL_USER_FUNC_ARRAY(&value, &handler, &arguments);
		zval_ptr_dtor(&arguments);
		zval_ptr_dtor(return_value);

		RETVAL_ZVAL(&value, 0, 0);
	}
}

/**
 * @brief Removes padding @a padding_type from @a text
 * @param return_value Result, possibly unpadded
 * @param text Message to be unpadded
 * @param mode Encryption mode; unpadding is applied only in CBC or ECB mode
 * @param block_size Cipher block size
 * @param padding_type Padding scheme
 * @note If the function detects that the text was not padded, it will return it unmodified
 */
static void phalcon_crypt_unpad_text(zval *return_value, zval *text, zval *mode, uint block_size, int padding_type)
{
	uint padding_size;
	char padding[256];
	int i;
	char *str_mode;
	char *str_text;
	uint text_len;

	assert(Z_TYPE_P(text) == IS_STRING);
	assert(Z_TYPE_P(mode) == IS_STRING);

	padding_size = 0;
	str_mode = Z_STRVAL_P(mode);
	str_text = Z_STRVAL_P(text);
	text_len = Z_STRLEN_P(text);

	if (text_len && (text_len % block_size == 0) && (!strcmp(str_mode, "ecb") || !strcmp(str_mode, "cbc"))) {
		switch (padding_type) {
			case PHALCON_CRYPT_PADDING_ANSI_X_923:
				if ((unsigned char)(str_text[text_len - 1]) <= block_size) {
					padding_size = str_text[text_len - 1];

					memset(padding, 0, padding_size - 1);
					padding[padding_size-1] = (unsigned char)padding_size;

					if (memcmp(padding, str_text + text_len - padding_size, padding_size)) {
						padding_size = 0;
					}
				}

				break;

			case PHALCON_CRYPT_PADDING_PKCS7:
				if ((unsigned char)(str_text[text_len-1]) <= block_size) {
					padding_size = str_text[text_len-1];

					memset(padding, padding_size, padding_size);

					if (memcmp(padding, str_text + text_len - padding_size, padding_size)) {
						padding_size = 0;
					}
				}

				break;

			case PHALCON_CRYPT_PADDING_ISO_10126:
				padding_size = str_text[text_len-1];
				break;

			case PHALCON_CRYPT_PADDING_ISO_IEC_7816_4:
				i = text_len - 1;
				while (i > 0 && str_text[i] == 0x00 && padding_size < block_size) {
					++padding_size;
					--i;
				}

				padding_size = ((unsigned char)str_text[i] == 0x80) ? (padding_size + 1) : 0;
				break;

			case PHALCON_CRYPT_PADDING_ZERO:
				i = text_len - 1;
				while (i >= 0 && str_text[i] == 0x00 && padding_size <= block_size) {
					++padding_size;
					--i;
				}

				break;

			case PHALCON_CRYPT_PADDING_SPACE:
				i = text_len - 1;
				while (i >= 0 && str_text[i] == 0x20 && padding_size <= block_size) {
					++padding_size;
					--i;
				}

				break;

			default:
				break;
		}

		if (padding_size && padding_size <= block_size) {
			assert(padding_size <= text_len);
			if (padding_size < text_len) {
				phalcon_substr(return_value, text, 0, text_len - padding_size);
			}
			else {
				ZVAL_EMPTY_STRING(return_value);
			}
		}
		else {
			padding_size = 0;
		}
	}

	if (!padding_size) {
		ZVAL_ZVAL(return_value, text, 1, 0);
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
	zval method = {}, pos = {}, mode = {}, cipher = {}, iv_size = {}, iv = {}, block_size = {}, decrypted = {};
	zval padding = {}, unpadded = {}, text_to_decipher = {};

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
		zval_ptr_dtor(&arguments);
	} else {
		ZVAL_COPY(&value, source);
	}

	/* Do not use make_printable_zval() here: we need the conversion with type juggling */
	if (Z_TYPE(value) != IS_STRING) {
		phalcon_cast(&text, &value, IS_STRING);
	} else {
		ZVAL_COPY(&text, &value);
	}
	zval_ptr_dtor(&value);

	if (!key || Z_TYPE_P(key) == IS_NULL) {
		phalcon_read_property(&encrypt_key, getThis(), SL("_key"), PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&encrypt_key, key);
	}

	if (!options || Z_TYPE_P(options) == IS_NULL) {
		phalcon_read_property(&encrypt_options, getThis(), SL("_options"), PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&encrypt_options, options);
	}

	phalcon_read_property(&method, getThis(), SL("_method"), PH_NOISY|PH_READONLY);
/*
	PHALCON_CALL_FUNCTION(&methods, "openssl_get_cipher_methods", &PHALCON_GLOBAL(z_true));

	if (!phalcon_fast_in_array(&method, &methods)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_crypt_exception_ce, "Cipher algorithm is unknown");
		return;
	}
*/
	phalcon_read_property(&padding, getThis(), SL("_padding"), PH_NOISY|PH_READONLY);
	if (PHALCON_GT_LONG(&padding, 0) && phalcon_fast_strrpos_str(&pos, &method, SL("-"))) {
		zval tmp = {};
		phalcon_substr(&mode, &method, Z_LVAL(pos) + 1, 0);
		phalcon_substr(&tmp, &method, 0, Z_LVAL(pos));
		phalcon_fast_str_replace_str(&cipher, "-", "", &tmp);
		zval_ptr_dtor(&tmp);
		phalcon_strtolower_inplace(&cipher);
	}

	PHALCON_CALL_FUNCTION(&iv_size, "openssl_cipher_iv_length", &method);

	if (Z_LVAL(iv_size) <= 0) {
		ZVAL_NULL(&iv);
		ZVAL_COPY(&text_to_decipher, &text);
	} else {
		phalcon_substr(&iv, &text, 0, Z_LVAL(iv_size));
		phalcon_substr(&text_to_decipher, &text, Z_LVAL(iv_size), 0);
	}
	zval_ptr_dtor(&text);

	PHALCON_CALL_FUNCTION(&decrypted, "openssl_decrypt", &text_to_decipher, &method, &encrypt_key, &encrypt_options, &iv);
	zval_ptr_dtor(&text_to_decipher);
	zval_ptr_dtor(&iv);
	if (unlikely(Z_TYPE(decrypted) != IS_STRING)) {
		convert_to_string(&decrypted);
	}

	if (PHALCON_GT_LONG(&padding, 0)) {
		if (PHALCON_LE_LONG(&iv_size, 0) && zend_is_true(&cipher)) {
			PHALCON_CALL_FUNCTION(&block_size, "openssl_cipher_iv_length", &cipher);
		} else {
			ZVAL_COPY(&block_size, &iv_size);
		}
		phalcon_crypt_unpad_text(&unpadded, &decrypted, &mode, Z_LVAL(block_size), Z_LVAL(padding));
		zval_ptr_dtor(&block_size);
	} else {
		ZVAL_COPY(&unpadded, &decrypted);
	}
	zval_ptr_dtor(&mode);
	zval_ptr_dtor(&cipher);
	zval_ptr_dtor(&decrypted);

	phalcon_read_property(&handler, getThis(), SL("_afterDecrypt"), PH_NOISY|PH_READONLY);

	if (phalcon_is_callable(&handler)) {
		array_init_size(&arguments, 1);
		phalcon_array_append(&arguments, &unpadded, PH_COPY);

		PHALCON_CALL_USER_FUNC_ARRAY(return_value, &handler, &arguments);
		zval_ptr_dtor(&arguments);
		zval_ptr_dtor(&unpadded);
	} else {
		RETVAL_ZVAL(&unpadded, 0, 0);
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
	zval_ptr_dtor(&encrypt_value);
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
		ZVAL_COPY(&decrypt_text, text);
	}

	phalcon_base64_decode(&decrypt_value, &decrypt_text);
	zval_ptr_dtor(&decrypt_text);

	PHALCON_RETURN_CALL_METHOD(getThis(), "decrypt", &decrypt_value, key);
	zval_ptr_dtor(&decrypt_value);
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

	phalcon_update_property(getThis(), SL("_beforeEncrypt"), handler);
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

	phalcon_update_property(getThis(), SL("_afterEncrypt"), handler);
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

	phalcon_update_property(getThis(), SL("_beforeDecrypt"), handler);
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

	phalcon_update_property(getThis(), SL("_afterDecrypt"), handler);
	RETURN_THIS();
}
