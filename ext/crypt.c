
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

PHP_METHOD(Phalcon_Crypt, setCipher);
PHP_METHOD(Phalcon_Crypt, getCipher);
PHP_METHOD(Phalcon_Crypt, setMode);
PHP_METHOD(Phalcon_Crypt, getMode);
PHP_METHOD(Phalcon_Crypt, setKey);
PHP_METHOD(Phalcon_Crypt, getKey);
PHP_METHOD(Phalcon_Crypt, setPadding);
PHP_METHOD(Phalcon_Crypt, getPadding);
PHP_METHOD(Phalcon_Crypt, encrypt);
PHP_METHOD(Phalcon_Crypt, decrypt);
PHP_METHOD(Phalcon_Crypt, encryptBase64);
PHP_METHOD(Phalcon_Crypt, decryptBase64);
PHP_METHOD(Phalcon_Crypt, getAvailableCiphers);
PHP_METHOD(Phalcon_Crypt, getAvailableModes);
PHP_METHOD(Phalcon_Crypt, getAvailableCiphers);
PHP_METHOD(Phalcon_Crypt, beforeEncrypt);
PHP_METHOD(Phalcon_Crypt, afterEncrypt);
PHP_METHOD(Phalcon_Crypt, beforeDecrypt);
PHP_METHOD(Phalcon_Crypt, afterDecrypt);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_crypt_getpadding, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_crypt_setpadding, 0, 0, 1)
	ZEND_ARG_INFO(0, scheme)
ZEND_END_ARG_INFO()

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
	PHP_ME(Phalcon_Crypt, setCipher, arginfo_phalcon_cryptinterface_setcipher, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, getCipher, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, setMode, arginfo_phalcon_cryptinterface_setmode, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, getMode, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, setKey, arginfo_phalcon_cryptinterface_setkey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, getKey, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, setPadding, arginfo_phalcon_crypt_setpadding, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, getPadding, arginfo_phalcon_crypt_getpadding, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, encrypt, arginfo_phalcon_cryptinterface_encrypt, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, decrypt, arginfo_phalcon_cryptinterface_decrypt, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, encryptBase64, arginfo_phalcon_cryptinterface_encryptbase64, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, decryptBase64, arginfo_phalcon_cryptinterface_decryptbase64, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, getAvailableCiphers, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Crypt, getAvailableModes, NULL, ZEND_ACC_PUBLIC)
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
	zend_declare_property_string(phalcon_crypt_ce, SL("_mode"), "cbc", ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_crypt_ce, SL("_cipher"), "rijndael-256", ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_crypt_ce, SL("_padding"), 0, ZEND_ACC_PROTECTED);	
	zend_declare_property_null(phalcon_crypt_ce, SL("_beforeEncrypt"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_crypt_ce, SL("_afterEncrypt"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_crypt_ce, SL("_beforeDecrypt"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_crypt_ce, SL("_afterDecrypt"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_crypt_ce, 1, phalcon_cryptinterface_ce);

	zend_declare_class_constant_long(phalcon_crypt_ce, SL("PADDING_DEFAULT"),        PHALCON_CRYPT_PADDING_DEFAULT       );
	zend_declare_class_constant_long(phalcon_crypt_ce, SL("PADDING_ANSI_X_923"),     PHALCON_CRYPT_PADDING_ANSI_X_923    );
	zend_declare_class_constant_long(phalcon_crypt_ce, SL("PADDING_PKCS7"),          PHALCON_CRYPT_PADDING_PKCS7         );
	zend_declare_class_constant_long(phalcon_crypt_ce, SL("PADDING_ISO_10126"),      PHALCON_CRYPT_PADDING_ISO_10126     );
	zend_declare_class_constant_long(phalcon_crypt_ce, SL("PADDING_ISO_IEC_7816_4"), PHALCON_CRYPT_PADDING_ISO_IEC_7816_4);
	zend_declare_class_constant_long(phalcon_crypt_ce, SL("PADDING_ZERO"),           PHALCON_CRYPT_PADDING_ZERO          );
	zend_declare_class_constant_long(phalcon_crypt_ce, SL("PADDING_SPACE"),          PHALCON_CRYPT_PADDING_SPACE         );

	return SUCCESS;
}

/**
 * Sets the cipher algorithm
 *
 * @param string $cipher
 * @return Phalcon\Encrypt
 */
PHP_METHOD(Phalcon_Crypt, setCipher){

	zval *cipher;

	phalcon_fetch_params(0, 1, 0, &cipher);

	phalcon_update_property_zval(getThis(), SL("_cipher"), cipher);
	RETURN_THISW();
}

/**
 * Returns the current cipher
 *
 * @return string
 */
PHP_METHOD(Phalcon_Crypt, getCipher){


	RETURN_MEMBER(getThis(), "_cipher");
}

/**
 * Sets the encrypt/decrypt mode
 *
 * @param string $cipher
 * @return Phalcon\Encrypt
 */
PHP_METHOD(Phalcon_Crypt, setMode){

	zval *mode;

	phalcon_fetch_params(0, 1, 0, &mode);
	PHALCON_ENSURE_IS_STRING(mode);

	phalcon_update_property_zval(getThis(), SL("_mode"), mode);
	RETURN_THISW();
}

/**
 * Returns the current encryption mode
 *
 * @return string
 */
PHP_METHOD(Phalcon_Crypt, getMode){


	RETURN_MEMBER(getThis(), "_mode");
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
	RETURN_THISW();
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
 * @brief Phalcon\CryptInterface Phalcon\Crypt::setPadding(int $scheme)
 *
 * @param int scheme Padding scheme
 * @return Phalcon\CryptInterface
 */
PHP_METHOD(Phalcon_Crypt, setPadding) {

	zval *scheme;

	phalcon_fetch_params(0, 1, 0, &scheme);
	PHALCON_ENSURE_IS_LONG(scheme);

	phalcon_update_property_zval(getThis(), SL("_padding"), scheme);
	RETURN_THISW();
}

/**
 * Returns the padding scheme
 *
 * @brief int Phalcon\Crypt::getPadding()
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
 * Encrypts a text
 *
 *<code>
 *	$encrypted = $crypt->encrypt("Ultra-secret text", "encrypt password");
 *</code>
 *
 * @param string $text
 * @param string $key
 * @return string
 */
PHP_METHOD(Phalcon_Crypt, encrypt){

	zval *source, *key = NULL, handler = {}, arguments = {}, value = {}, text = {}, encrypt_key = {}, cipher = {}, mode = {}, td = {}, iv_size = {}, rand = {}, iv = {}, block_size = {};
	zval padding_type = {}, padded = {}, encrypt = {};

	phalcon_fetch_params(0, 1, 1, &source, &key);

	phalcon_read_property(&handler, getThis(), SL("_beforeEncrypt"), PH_NOISY);

	if (phalcon_is_callable(&handler)) {
		PHALCON_SEPARATE_PARAM(source);

		array_init_size(&arguments, 1);
		phalcon_array_append(&arguments, source, PH_COPY);

		PHALCON_CALL_USER_FUNC_ARRAYW(&value, &handler, &arguments);

		source = &value;
	}

	/* Do not use make_printable_zval() here: we need the conversion with type juggling */
	if (Z_TYPE_P(source) != IS_STRING) {
		phalcon_cast(&text, source, IS_STRING);
	} else {
		PHALCON_CPY_WRT(&text, source);
	}

	if (phalcon_function_exists_ex(SL("mcrypt_module_open")) == FAILURE) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_crypt_exception_ce, "mcrypt extension is required");
		return;
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
		PHALCON_THROW_EXCEPTION_STRW(phalcon_crypt_exception_ce, "Encryption key cannot be empty");
		return;
	}

	phalcon_read_property(&cipher, getThis(), SL("_cipher"), PH_NOISY);
	phalcon_read_property(&mode, getThis(), SL("_mode"), PH_NOISY);

	PHALCON_CALL_FUNCTIONW(&td, "mcrypt_module_open", &cipher, &PHALCON_GLOBAL(z_null), &mode, &PHALCON_GLOBAL(z_null));
	PHALCON_CALL_FUNCTIONW(&iv_size, "mcrypt_enc_get_iv_size", &td);
	if (unlikely(Z_TYPE(iv_size) != IS_LONG)) {
		convert_to_long(&iv_size);
	}

	if (Z_STRLEN(encrypt_key) > Z_LVAL(iv_size)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_crypt_exception_ce, "Size of key is too large for this algorithm");
		return;
	}

	ZVAL_LONG(&rand, 2);

	PHALCON_CALL_FUNCTIONW(&iv, "mcrypt_create_iv", &iv_size, &rand);
	if (unlikely(Z_TYPE(iv) != IS_STRING)) {
		convert_to_string(&iv);
	}

	PHALCON_CALL_FUNCTIONW(&block_size, "mcrypt_get_block_size", &cipher, &mode);
	if (unlikely(Z_TYPE(block_size) != IS_LONG)) {
		convert_to_long(&block_size);
	}

	phalcon_read_property(&padding_type, getThis(), SL("_padding"), PH_NOISY);

	assert(Z_TYPE(padding_type) == IS_LONG);
	assert(Z_TYPE(block_size) == IS_LONG);
	assert(Z_TYPE(mode) == IS_STRING);
	assert(Z_TYPE(text) == IS_STRING);

	phalcon_crypt_pad_text(&padded, &text, &mode, Z_LVAL(block_size), Z_LVAL(padding_type));
	assert(Z_TYPE(padded) == IS_STRING);

	PHALCON_CALL_FUNCTIONW(&encrypt, "mcrypt_encrypt", &cipher, &encrypt_key, &padded, &mode, &iv);

	PHALCON_CALL_FUNCTIONW(NULL, "mcrypt_module_close", &td);

	PHALCON_CONCAT_VV(return_value, &iv, &encrypt);

	phalcon_read_property(&handler, getThis(), SL("_afterEncrypt"), PH_NOISY);

	if (phalcon_is_callable(&handler)) {
		array_init_size(&arguments, 1);
		phalcon_array_append(&arguments, return_value, PH_COPY);

		PHALCON_CALL_USER_FUNC_ARRAYW(&value, &handler, &arguments);

		RETURN_CTORW(&value);
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
 * @return string
 */
PHP_METHOD(Phalcon_Crypt, decrypt){

	zval *text, *key = NULL, _key = {}, handler = {}, arguments = {}, value = {}, cipher = {}, mode = {}, td = {}, iv_size = {}, key_size = {}, text_size = {}, iv = {}, text_to_decipher = {};
	zval decrypted = {}, block_size = {}, padding_type = {};

	phalcon_fetch_params(0, 1, 1, &text, &key);

	if (phalcon_function_exists_ex(SL("mcrypt_module_open")) == FAILURE) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_crypt_exception_ce, "mcrypt extension is required");
		return;
	}

	if (!key || Z_TYPE_P(key) == IS_NULL) {
		phalcon_read_property(&_key, getThis(), SL("_key"), PH_NOISY);
		key = &_key;
	}

	if (PHALCON_IS_EMPTY(key)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_crypt_exception_ce, "Decryption key cannot be empty");
		return;
	}

	phalcon_read_property(&handler, getThis(), SL("_beforeDecrypt"), PH_NOISY);

	if (phalcon_is_callable(&handler)) {
		PHALCON_SEPARATE_PARAM(text);

		array_init_size(&arguments, 1);
		phalcon_array_append(&arguments, text, PH_COPY);

		PHALCON_CALL_USER_FUNC_ARRAYW(&value, &handler, &arguments);

		text = &value;
	}

	phalcon_read_property(&cipher, getThis(), SL("_cipher"), PH_NOISY);
	phalcon_read_property(&mode, getThis(), SL("_mode"), PH_NOISY);

	PHALCON_CALL_FUNCTIONW(&td, "mcrypt_module_open", &cipher, &PHALCON_GLOBAL(z_null), &mode, &PHALCON_GLOBAL(z_null));
	PHALCON_CALL_FUNCTIONW(&iv_size, "mcrypt_enc_get_iv_size", &td);
	if (unlikely(Z_TYPE(iv_size) != IS_LONG)) {
		convert_to_long(&iv_size);
	}

	phalcon_fast_strlen(&key_size, key);
	if (PHALCON_GT(&key_size, &iv_size)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_crypt_exception_ce, "Size of key is too large for this algorithm");
		return;
	}

	phalcon_fast_strlen(&text_size, text);
	if (PHALCON_GT(&key_size, &text_size)) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_crypt_exception_ce, "Size of IV is larger than text to decrypt");
		return;
	}

	phalcon_substr(&iv, text, 0, Z_LVAL(iv_size));
	phalcon_substr(&text_to_decipher, text, Z_LVAL(iv_size), 0);

	PHALCON_CALL_FUNCTIONW(&decrypted, "mcrypt_decrypt", &cipher, key, &text_to_decipher, &mode, &iv);
	if (unlikely(Z_TYPE(decrypted) != IS_STRING)) {
		convert_to_string(&decrypted);
	}

	PHALCON_CALL_FUNCTIONW(&block_size, "mcrypt_get_block_size", &cipher, &mode);
	if (unlikely(Z_TYPE(block_size) != IS_LONG)) {
		convert_to_long(&block_size);
	}

	PHALCON_CALL_FUNCTIONW(NULL, "mcrypt_module_close", &td);

	phalcon_read_property(&padding_type, getThis(), SL("_padding"), PH_NOISY);

	assert(Z_TYPE(padding_type) == IS_LONG);
	assert(Z_TYPE(block_size) == IS_LONG);
	assert(Z_TYPE(mode) == IS_STRING);
	assert(Z_TYPE(decrypted) == IS_STRING);

	phalcon_crypt_unpad_text(return_value, &decrypted, &mode, Z_LVAL(block_size), Z_LVAL(padding_type));

	phalcon_read_property(&handler, getThis(), SL("_afterDecrypt"), PH_NOISY);

	if (phalcon_is_callable(&handler)) {
		array_init_size(&arguments, 1);
		phalcon_array_append(&arguments, return_value, PH_COPY);

		PHALCON_CALL_USER_FUNC_ARRAYW(&value, &handler, &arguments);

		RETURN_CTORW(&value);
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

	zval *text, *key = NULL, *safe = NULL, encrypted = {};

	phalcon_fetch_params(0, 1, 2, &text, &key, &safe);

	if (!key) {
		key = &PHALCON_GLOBAL(z_null);
	}

	if (!safe) {
		safe = &PHALCON_GLOBAL(z_false);
	}

	PHALCON_CALL_METHODW(&encrypted, getThis(), "encrypt", text, key);
	phalcon_base64_encode(return_value, &encrypted);

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

	zval *text, *key = NULL, *safe = NULL, decrypt_text = {};
	zend_string *decoded;
	char *tmp;

	phalcon_fetch_params(0, 1, 2, &text, &key, &safe);
	PHALCON_ENSURE_IS_STRING(text);

	if (!key) {
		key = &PHALCON_GLOBAL(z_null);
	}

	if (!safe) {
		safe = &PHALCON_GLOBAL(z_false);
	}

	if (zend_is_true(safe)) {
		tmp = estrndup(Z_STRVAL_P(text), Z_STRLEN_P(text));
		php_strtr(tmp, Z_STRLEN_P(text), "-_", "+/", 2);
		decoded = php_base64_decode((unsigned char*)tmp, Z_STRLEN_P(text));
		efree(tmp);
	} else {
		decoded = php_base64_decode((unsigned char*)(Z_STRVAL_P(text)), Z_STRLEN_P(text));
	}

	if (!decoded) {
		RETURN_FALSE;
	}

	ZVAL_STR(&decrypt_text, decoded);
	PHALCON_RETURN_CALL_METHODW(getThis(), "decrypt", &decrypt_text, key);
}

/**
 * Returns a list of available cyphers
 *
 * @return array
 */
PHP_METHOD(Phalcon_Crypt, getAvailableCiphers){

	PHALCON_RETURN_CALL_FUNCTIONW("mcrypt_list_algorithms");
}

/**
 * Returns a list of available modes
 *
 * @return array
 */
PHP_METHOD(Phalcon_Crypt, getAvailableModes){

	PHALCON_RETURN_CALL_FUNCTIONW("mcrypt_list_modes");
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
		PHALCON_THROW_EXCEPTION_STRW(phalcon_crypt_exception_ce, "Handler must be an callable");
		return;
	}
	
	phalcon_update_property_zval(getThis(), SL("_beforeEncrypt"), handler);
	RETURN_THISW();
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
		PHALCON_THROW_EXCEPTION_STRW(phalcon_crypt_exception_ce, "Handler must be an callable");
		return;
	}
	
	phalcon_update_property_zval(getThis(), SL("_afterEncrypt"), handler);
	RETURN_THISW();
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
	RETURN_THISW();
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
		PHALCON_THROW_EXCEPTION_STRW(phalcon_crypt_exception_ce, "Handler must be an callable");
		return;
	}
	
	phalcon_update_property_zval(getThis(), SL("_afterDecrypt"), handler);
	RETURN_THISW();
}
