
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

#include "random.h"
#include "security/exception.h"
#include "diinterface.h"
#include "di/injectable.h"

#include <stdlib.h>

#ifdef PHALCON_USE_PHP_HASH
#include <ext/hash/php_hash.h>
#endif
#include <ext/standard/base64.h>
#include <ext/standard/php_string.h>
#include <ext/standard/php_crypt.h>
#include <main/spprintf.h>

#if PHP_WIN32
#include <win32/winutil.h>
#else
#include <fcntl.h>
#endif

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/exception.h"
#include "kernel/operators.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/filter.h"
#include "kernel/concat.h"
#include "kernel/file.h"
#include "kernel/array.h"

#include "interned-strings.h"
#include "internal/arginfo.h"

/**
 * Phalcon\Security\Random
 *
 * Secure random number generator class.
 *
 * Provides secure random number generator which is suitable for generating
 * session key in HTTP cookies, etc.
 *
 * It supports following secure random number generators:
 *
 * - libsodium
 * - openssl
 * - /dev/urandom
 *
 * A `Phalcon\Security\Random` could be mainly useful for:
 *
 * - Key generation (e.g. generation of complicated keys)
 * - Creating random passwords for new user accounts
 * - Encryption systems
 *
 *<code>
 *  $random = new \Phalcon\Security\Random();
 *
 *  // Random binary string
 *  $bytes = $random->bytes();
 *
 *  // Random hex string
 *  echo $random->hex(10); // a29f470508d5ccb8e289
 *  echo $random->hex(10); // 533c2f08d5eee750e64a
 *  echo $random->hex(11); // f362ef96cb9ffef150c9cd
 *  echo $random->hex(12); // 95469d667475125208be45c4
 *  echo $random->hex(13); // 05475e8af4a34f8f743ab48761
 *
 *  // Random base64 string
 *  echo $random->base64(12); // XfIN81jGGuKkcE1E
 *  echo $random->base64(12); // 3rcq39QzGK9fUqh8
 *  echo $random->base64();   // DRcfbngL/iOo9hGGvy1TcQ==
 *  echo $random->base64(16); // SvdhPcIHDZFad838Bb0Swg==
 *
 *  // Random URL-safe base64 string
 *  echo $random->base64Safe();           // PcV6jGbJ6vfVw7hfKIFDGA
 *  echo $random->base64Safe();           // GD8JojhzSTrqX7Q8J6uug
 *  echo $random->base64Safe(8);          // mGyy0evy3ok
 *  echo $random->base64Safe(null, true); // DRrAgOFkS4rvRiVHFefcQ==
 *
 *  // Random UUID
 *  echo $random->uuid(); // db082997-2572-4e2c-a046-5eefe97b1235
 *  echo $random->uuid(); // da2aa0e2-b4d0-4e3c-99f5-f5ef62c57fe2
 *  echo $random->uuid(); // 75e6b628-c562-4117-bb76-61c4153455a9
 *  echo $random->uuid(); // dc446df1-0848-4d05-b501-4af3c220c13d
 *
 *  // Random number between 0 and $len
 *  echo $random->number(256); // 84
 *  echo $random->number(256); // 79
 *  echo $random->number(100); // 29
 *  echo $random->number(300); // 40
 *
 *  // Random base58 string
 *  echo $random->base58();   // 4kUgL2pdQMSCQtjE
 *  echo $random->base58();   // Umjxqf7ZPwh765yR
 *  echo $random->base58(24); // qoXcgmw4A9dys26HaNEdCRj9
 *  echo $random->base58(7);  // 774SJD3vgP
 *</code>
 *
 * This class partially borrows SecureRandom library from Ruby
 *
 * @link http://ruby-doc.org/stdlib-2.2.2/libdoc/securerandom/rdoc/SecureRandom.html
 */
zend_class_entry *phalcon_security_random_ce;

PHP_METHOD(Phalcon_Security_Random, bytes);
PHP_METHOD(Phalcon_Security_Random, hex);
PHP_METHOD(Phalcon_Security_Random, base58);
PHP_METHOD(Phalcon_Security_Random, base64);
PHP_METHOD(Phalcon_Security_Random, base64Safe);
PHP_METHOD(Phalcon_Security_Random, uuid);
PHP_METHOD(Phalcon_Security_Random, number);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_security_random_bytes, 0, 0, 0)
	ZEND_ARG_INFO(0, len)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_security_random_hex, 0, 0, 0)
	ZEND_ARG_INFO(0, len)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_security_random_base58, 0, 0, 0)
	ZEND_ARG_INFO(0, len)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_security_random_base64, 0, 0, 0)
	ZEND_ARG_INFO(0, len)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_security_random_base64safe, 0, 0, 0)
	ZEND_ARG_INFO(0, len)
	ZEND_ARG_INFO(0, padding)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_security_random_number, 0, 0, 1)
	ZEND_ARG_INFO(0, len)
ZEND_END_ARG_INFO()


static const zend_function_entry phalcon_security_random_method_entry[] = {
	PHP_ME(Phalcon_Security_Random, bytes, arginfo_phalcon_security_random_bytes, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Security_Random, hex, arginfo_phalcon_security_random_hex, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Security_Random, base58, arginfo_phalcon_security_random_base58, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Security_Random, base64, arginfo_phalcon_security_random_base64, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Security_Random, base64Safe, arginfo_phalcon_security_random_base64safe, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Security_Random, uuid, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Security_Random, number, arginfo_phalcon_security_random_number, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Security\Random initializer
 */
PHALCON_INIT_CLASS(Phalcon_Security_Random){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Security, Random, security_random, phalcon_di_injectable_ce, phalcon_security_random_method_entry, 0);

	return SUCCESS;
}

/**
 * Generates a random binary string
 *
 * If $len is not specified, 16 is assumed. It may be larger in future.
 * The result may contain any byte: "x00" - "xFF".
 *
 *<code>
 *  $random = new \Phalcon\Security\Random();
 *
 *  $bytes = $random->bytes();
 *</code>
 *
 * @throws Exception If secure random number generator is not available or unexpected partial read
 */
PHP_METHOD(Phalcon_Security_Random, bytes){

	zval *len_param = NULL, len, file_path, mode, handle, buffer, ret;
	int l;

	phalcon_fetch_params(0, 0, 1, &len_param);

	if (!len_param) {
		l = 16;
	} else {
		l = phalcon_get_intval(len_param);
	}


	if (l <= 0) {
		l = 16;
	}

	ZVAL_LONG(&len, l);

	if ((phalcon_function_exists_ex(SS("\\sodium\\randombytes_buf")) == SUCCESS)) {
		PHALCON_RETURN_CALL_FUNCTIONW("\\sodium\\randombytes_buf", &len);
		return;
	}
	if ((phalcon_function_exists_ex(SS("openssl_random_pseudo_bytes")) == SUCCESS)) {
		PHALCON_RETURN_CALL_FUNCTIONW("openssl_random_pseudo_bytes", &len);
		return;
	}

	ZVAL_STRING(&file_path, "/dev/urandom");

	if (phalcon_file_exists(&file_path) == SUCCESS) {
		ZVAL_STRING(&mode, "rb");

		PHALCON_CALL_FUNCTIONW(&handle, "fopen", &file_path, &mode);

		if (!PHALCON_IS_FALSE(&handle)) {
			ZVAL_LONG(&buffer, 0);

			PHALCON_CALL_FUNCTIONW(NULL, "stream_set_read_buffer", &handle, &buffer);
			PHALCON_CALL_FUNCTIONW(&ret, "fread", &handle, &len);

			phalcon_fclose(&handle);
			if (phalcon_fast_strlen_ev(&ret) != l) {
				PHALCON_THROW_EXCEPTION_STRW(phalcon_security_exception_ce, "Unexpected partial read from random device");
				return;
			}
			RETURN_CTORW(&ret);
		}
	}

	PHALCON_THROW_EXCEPTION_STRW(phalcon_security_exception_ce, "No random device");
}

/**
 * Generates a random hex string
 *
 * If $len is not specified, 16 is assumed. It may be larger in future.
 * The length of the result string is usually greater of $len.
 *
 *<code>
 *  $random = new \Phalcon\Security\Random();
 *
 *  echo $random->hex(10); // a29f470508d5ccb8e289
 *</code>
 *
 * @throws Exception If secure random number generator is not available or unexpected partial read
 */
PHP_METHOD(Phalcon_Security_Random, hex){

	zval *len_param = NULL, data, format, ret;

	phalcon_fetch_params(0, 0, 1, &len_param);

	if (!len_param) {
		len_param = &PHALCON_GLOBAL(z_zero);
	}

	PHALCON_CALL_SELFW(&data, "bytes", len_param);

	ZVAL_STRING(&format, "H*");

	PHALCON_CALL_FUNCTIONW(&ret, "unpack", &format, &data);

	ZVAL_MAKE_REF(&ret);
	PHALCON_RETURN_CALL_FUNCTIONW("array_shift", &ret);
	ZVAL_UNREF(&ret);
}

/**
 * Generates a random base58 string
 *
 * If $len is not specified, 16 is assumed. It may be larger in future.
 * The result may contain alphanumeric characters except 0, O, I and l.
 *
 * It is similar to Base64 but has been modified to avoid both non-alphanumeric
 * characters and letters which might look ambiguous when printed.
 *
 *<code>
 *  $random = new \Phalcon\Security\Random();
 *
 *  echo $random->base58(); // 4kUgL2pdQMSCQtjE
 *</code>
 *
 * @link https://en.wikipedia.org/wiki/Base58
 * @throws Exception If secure random number generator is not available or unexpected partial read
 */
PHP_METHOD(Phalcon_Security_Random, base58){

	zval *len_param = NULL, byte_string, alphabet, format, data, bytes, *byte;

	phalcon_fetch_params(0, 0, 1, &len_param);

	if (!len_param) {
		len_param = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STRING(&byte_string, "");
	ZVAL_STRING(&alphabet, "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz");
	ZVAL_STRING(&format, "C*");

	PHALCON_CALL_SELFW(&data, "bytes", len_param);

	PHALCON_CALL_FUNCTIONW(&bytes, "unpack", &format, &data);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(bytes), byte) {
		zval idx, tmp;
		double d;
		unsigned char c;
		
		d = phalcon_safe_mod_zval_long(byte, 64);

		ZVAL_DOUBLE(&idx, d);

		if (d >= 58) {;
			ZVAL_LONG(&tmp, 57);
			PHALCON_CALL_SELFW(&idx, "number", &tmp);
		}

		c = PHALCON_STRING_OFFSET(&alphabet, phalcon_get_intval(&idx));
		phalcon_concat_self_char(&byte_string, c);
	} ZEND_HASH_FOREACH_END();

	RETURN_CTORW(&byte_string);
}

/**
 * Generates a random base64 string
 *
 * If $len is not specified, 16 is assumed. It may be larger in future.
 * The length of the result string is usually greater of $len.
 * Size formula: 4 *( $len / 3) and this need to be rounded up to a multiple of 4.
 *
 *<code>
 *  $random = new \Phalcon\Security\Random();
 *
 *  echo $random->base64(12); // 3rcq39QzGK9fUqh8
 *</code>
 *
 * @throws Exception If secure random number generator is not available or unexpected partial read
 */
PHP_METHOD(Phalcon_Security_Random, base64) {

	zval *len_param = NULL, data;

	phalcon_fetch_params(0, 0, 1, &len_param);

	if (!len_param) {
		len_param = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_SELFW(&data, "bytes", len_param);

	PHALCON_RETURN_CALL_FUNCTIONW("base64_encode", &data);
}

/**
 * Generates a random URL-safe base64 string
 *
 * If $len is not specified, 16 is assumed. It may be larger in future.
 * The length of the result string is usually greater of $len.
 *
 * By default, padding is not generated because "=" may be used as a URL delimiter.
 * The result may contain A-Z, a-z, 0-9, "-" and "_". "=" is also used if $padding is true.
 * See RFC 3548 for the definition of URL-safe base64.
 *
 *<code>
 *  $random = new \Phalcon\Security\Random();
 *
 *  echo $random->base64Safe(); // GD8JojhzSTrqX7Q8J6uug
 *</code>
 *
 * @link https://www.ietf.org/rfc/rfc3548.txt
 * @throws Exception If secure random number generator is not available or unexpected partial read
 */
PHP_METHOD(Phalcon_Security_Random, base64Safe) {

	zval *len_param = NULL, *padding_param = NULL, data, pattern, replacement, s, charlist;

	phalcon_fetch_params(0, 0, 2, &len_param, &padding_param);

	if (!len_param) {
		len_param = &PHALCON_GLOBAL(z_null);
	}

	if (!padding_param) {
		padding_param = &PHALCON_GLOBAL(z_false);
	}

	PHALCON_CALL_SELFW(&data, "base64", len_param);

	ZVAL_STRING(&pattern, "#[^a-z0-9_=-]+#i");
	ZVAL_STRING(&replacement, "");

	PHALCON_CALL_FUNCTIONW(&s, "preg_replace", &pattern, &replacement, &data);

	if (!zend_is_true(padding_param)) {
		ZVAL_STRING(&charlist, "=");

		ZVAL_STR(return_value, phalcon_trim(&s, &charlist, PHALCON_TRIM_BOTH));
		return;
	}

	RETURN_CTORW(&s);
}

/**
 * Generates a v4 random UUID (Universally Unique IDentifier)
 *
 * The version 4 UUID is purely random (except the version). It doesn't contain meaningful
 * information such as MAC address, time, etc. See RFC 4122 for details of UUID.
 *
 * This algorithm sets the version number (4 bits) as well as two reserved bits.
 * All other bits (the remaining 122 bits) are set using a random or pseudorandom data source.
 * Version 4 UUIDs have the form xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx where x is any hexadecimal
 * digit and y is one of 8, 9, A, or B (e.g., f47ac10b-58cc-4372-a567-0e02b2c3d479).
 *
 *<code>
 *  $random = new \Phalcon\Security\Random();
 *
 *  echo $random->uuid(); // 1378c906-64bb-4f81-a8d6-4ae1bfcdec22
 *</code>
 *
 * @link https://www.ietf.org/rfc/rfc4122.txt
 * @throws Exception If secure random number generator is not available or unexpected partial read
 */
PHP_METHOD(Phalcon_Security_Random, uuid) {

	zval len, bytes, data, format, arr, a2, a3, str;

	ZVAL_LONG(&len, 16);

	PHALCON_CALL_SELFW(&bytes, "bytes", &len);

	ZVAL_STRING(&format, "N1a/n1b/n1c/n1d/n1e/N1f");

	PHALCON_CALL_FUNCTIONW(&data, "unpack", &format, &bytes);
	PHALCON_CALL_FUNCTIONW(&arr, "array_values", &data);

	phalcon_array_fetch_long(&a2, &arr, 2, PH_NOISY | PH_READONLY);
	phalcon_array_update_long_long(&arr, 2, ((((int) (phalcon_get_numberval(&a2)) & 0x0fff)) | 0x4000), PH_COPY | PH_SEPARATE);

	phalcon_array_fetch_long(&a3, &arr, 3, PH_NOISY | PH_READONLY);
	phalcon_array_update_long_long(&arr, 3, ((((int) (phalcon_get_numberval(&a3)) & 0x3fff)) | 0x8000), PH_COPY | PH_SEPARATE);

	ZVAL_STRING(&str, "%08x-%04x-%04x-%04x-%04x%08x");

	ZVAL_MAKE_REF(&arr);
	PHALCON_CALL_FUNCTIONW(NULL, "array_unshift", &arr, &str);
	ZVAL_UNREF(&arr);

	ZVAL_STRING(&str, "sprintf");

	PHALCON_CALL_USER_FUNC_ARRAYW(return_value, &str, &arr);
}

/**
 * Generates a random number between 0 and $len
 *
 * Returns an integer: 0 <= result <= $len.
 *
 *<code>
 *  $random = new \Phalcon\Security\Random();
 *
 *  echo $random->number(16); // 8
 *</code>
 * @throws Exception If secure random number generator is not available, unexpected partial read or $len <= 0
 */
PHP_METHOD(Phalcon_Security_Random, number) {

	zval *len_param, bin, hex, hex_tmp, format, pack, mask, bytes, chr, rnd, ret, data, tmp, tmp1;
	unsigned char c;

	phalcon_fetch_params(0, 1, 0, &len_param);

	ZVAL_STRING(&bin, "");

	if (phalcon_get_intval(len_param) <= 0) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_security_exception_ce, "Require a positive integer > 0");
		return;
	}

	if ((phalcon_function_exists_ex(SS("\\sodium\\randombytes_uniform")) == SUCCESS)) {
		PHALCON_RETURN_CALL_FUNCTIONW("\\sodium\\randombytes_uniform", len_param);
		return;
	}

	PHALCON_CALL_FUNCTIONW(&hex, "dechex", len_param);

	if (((phalcon_fast_strlen_ev(&hex) & 1)) == 1) {
		PHALCON_CONCAT_SV(&hex_tmp, "0", &hex);
		ZVAL_COPY_VALUE(&hex, &hex_tmp);
	}

	ZVAL_STRING(&format, "H*");

	PHALCON_CALL_FUNCTIONW(&pack, "pack", &format, &hex);

	phalcon_concat_self(&bin, &pack);

	c = PHALCON_STRING_OFFSET(&bin, 0);

	ZVAL_LONG(&tmp, c);

	PHALCON_CALL_FUNCTIONW(&mask, "ord", &tmp);

	ZVAL_LONG(&tmp, ((int) (phalcon_get_numberval(&mask)) | (((int) (phalcon_get_numberval(&mask)) >> 1))));
	ZVAL_LONG(&mask, phalcon_get_numberval(&tmp));

	ZVAL_LONG(&tmp, ((int) (phalcon_get_numberval(&mask)) | (((int) (phalcon_get_numberval(&mask)) >> 2))));
	ZVAL_LONG(&mask, phalcon_get_numberval(&tmp));

	ZVAL_LONG(&tmp, ((int) (phalcon_get_numberval(&mask)) | (((int) (phalcon_get_numberval(&mask)) >> 4))));
	ZVAL_LONG(&mask, phalcon_get_numberval(&tmp));

	do {
		ZVAL_LONG(&tmp, phalcon_fast_strlen_ev(bin));

		PHALCON_CALL_SELFW(&bytes, "bytes", &tmp);

		phalcon_substr(&tmp, &bytes, 0, 1);

		PHALCON_CALL_FUNCTIONW(&tmp1, "ord", &tmp);

		phalcon_bitwise_and_function(&tmp, &tmp1, &mask TSRMLS_CC);

		PHALCON_CALL_FUNCTIONW(&chr, "chr", &tmp);

		ZVAL_LONG(&tmp, 0);
		ZVAL_LONG(&tmp1, 1);

		PHALCON_CALL_FUNCTIONW(&rnd, "substr_replace", &bytes, &chr, &tmp, &tmp1);
	} while (PHALCON_LT(&bin, &rnd));

	ZVAL_STRING(&format, "H*");

	PHALCON_CALL_FUNCTIONW(&ret, "unpack", format, rnd);

	ZVAL_MAKE_REF(&ret);
	PHALCON_CALL_FUNCTIONW(&data, "array_shift", &ret);
	ZVAL_UNREF(&ret);

	PHALCON_RETURN_CALL_FUNCTIONW("hexdec", &data);
}
