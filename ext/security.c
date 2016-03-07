
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

#include "security.h"
#include "security/exception.h"
#include "diinterface.h"
#include "di/injectable.h"
#include "http/requestinterface.h"
#include "session/adapterinterface.h"

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

#include "interned-strings.h"
#include "internal/arginfo.h"

static const unsigned char ascii64[] = "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

/**
 * Phalcon\Security
 *
 * This component provides a set of functions to improve the security in Phalcon applications
 *
 *<code>
 *	$login = $this->request->getPost('login');
 *	$password = $this->request->getPost('password');
 *
 *	$user = Users::findFirstByLogin($login);
 *	if ($user) {
 *		if ($this->security->checkHash($password, $user->password)) {
 *			//The password is valid
 *		}
 *	}
 *</code>
 */
zend_class_entry *phalcon_security_ce;

PHP_METHOD(Phalcon_Security, setRandomBytes);
PHP_METHOD(Phalcon_Security, getRandomBytes);
PHP_METHOD(Phalcon_Security, setWorkFactor);
PHP_METHOD(Phalcon_Security, getWorkFactor);
PHP_METHOD(Phalcon_Security, getSaltBytes);
PHP_METHOD(Phalcon_Security, hash);
PHP_METHOD(Phalcon_Security, checkHash);
PHP_METHOD(Phalcon_Security, isLegacyHash);
PHP_METHOD(Phalcon_Security, getTokenKey);
PHP_METHOD(Phalcon_Security, getToken);
PHP_METHOD(Phalcon_Security, checkToken);
PHP_METHOD(Phalcon_Security, getSessionToken);
PHP_METHOD(Phalcon_Security, destroyToken);
PHP_METHOD(Phalcon_Security, computeHmac);
PHP_METHOD(Phalcon_Security, deriveKey);
PHP_METHOD(Phalcon_Security, pbkdf2);
PHP_METHOD(Phalcon_Security, getDefaultHash);
PHP_METHOD(Phalcon_Security, setDefaultHash);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_security_setrandombytes, 0, 0, 1)
	ZEND_ARG_INFO(0, randomBytes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_security_setworkfactor, 0, 0, 1)
	ZEND_ARG_INFO(0, workFactor)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_security_hash, 0, 0, 1)
	ZEND_ARG_INFO(0, password)
	ZEND_ARG_INFO(0, workFactor)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_security_checkhash, 0, 0, 2)
	ZEND_ARG_INFO(0, password)
	ZEND_ARG_INFO(0, passwordHash)
	ZEND_ARG_INFO(0, maxPasswordLength)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_security_islegacyhash, 0, 0, 1)
	ZEND_ARG_INFO(0, passwordHash)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_security_gettokenkey, 0, 0, 0)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, numberBytes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_security_gettoken, 0, 0, 0)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, numberBytes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_security_checktoken, 0, 0, 0)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, tokenKey)
	ZEND_ARG_INFO(0, tokenValue)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_security_getsessiontoken, 0, 0, 0)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_security_destroytoken, 0, 0, 0)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, tokenKey)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_security_computehmac, 0, 0, 3)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, algo)
	ZEND_ARG_INFO(0, raw)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_security_derivekey, 0, 0, 2)
	ZEND_ARG_INFO(0, password)
	ZEND_ARG_INFO(0, salt)
	ZEND_ARG_INFO(0, hash)
	ZEND_ARG_INFO(0, iterations)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_security_setdefaulthash, 0, 0, 1)
	ZEND_ARG_INFO(0, hash)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_security_method_entry[] = {
	PHP_ME(Phalcon_Security, setRandomBytes, arginfo_phalcon_security_setrandombytes, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Security, getRandomBytes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Security, setWorkFactor, arginfo_phalcon_security_setworkfactor, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Security, getWorkFactor, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Security, getSaltBytes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Security, hash, arginfo_phalcon_security_hash, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Security, checkHash, arginfo_phalcon_security_checkhash, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Security, isLegacyHash, arginfo_phalcon_security_islegacyhash, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Security, getTokenKey, arginfo_phalcon_security_gettokenkey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Security, getToken, arginfo_phalcon_security_gettoken, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Security, checkToken, arginfo_phalcon_security_checktoken, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Security, getSessionToken, arginfo_phalcon_security_getsessiontoken, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Security, destroyToken, arginfo_phalcon_security_destroytoken, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Security, computeHmac, arginfo_phalcon_security_computehmac, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Security, deriveKey, arginfo_phalcon_security_derivekey, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Security, pbkdf2, arginfo_phalcon_security_derivekey, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Security, getDefaultHash, arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Security, setDefaultHash, arginfo_phalcon_security_setdefaulthash, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Security initializer
 */
PHALCON_INIT_CLASS(Phalcon_Security){

	PHALCON_REGISTER_CLASS_EX(Phalcon, Security, security, phalcon_di_injectable_ce, phalcon_security_method_entry, 0);

	zend_declare_property_long(phalcon_security_ce, SL("_workFactor"), 8, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_security_ce, SL("_numberBytes"), 16, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_security_ce, SL("_csrf"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_security_ce, SL("_defaultHash"), PHALCON_SECURITY_CRYPT_DEFAULT, ZEND_ACC_PROTECTED);

	zend_declare_class_constant_long(phalcon_security_ce, SL("CRYPT_DEFAULT"),    PHALCON_SECURITY_CRYPT_DEFAULT   );
	zend_declare_class_constant_long(phalcon_security_ce, SL("CRYPT_STD_DES"),    PHALCON_SECURITY_CRYPT_STD_DES   );
	zend_declare_class_constant_long(phalcon_security_ce, SL("CRYPT_EXT_DES"),    PHALCON_SECURITY_CRYPT_EXT_DES   );
	zend_declare_class_constant_long(phalcon_security_ce, SL("CRYPT_MD5"),        PHALCON_SECURITY_CRYPT_MD5       );
	zend_declare_class_constant_long(phalcon_security_ce, SL("CRYPT_BLOWFISH"),   PHALCON_SECURITY_CRYPT_BLOWFISH  );
	zend_declare_class_constant_long(phalcon_security_ce, SL("CRYPT_BLOWFISH_X"), PHALCON_SECURITY_CRYPT_BLOWFISH_X);
	zend_declare_class_constant_long(phalcon_security_ce, SL("CRYPT_BLOWFISH_Y"), PHALCON_SECURITY_CRYPT_BLOWFISH_Y);
	zend_declare_class_constant_long(phalcon_security_ce, SL("CRYPT_SHA256"),     PHALCON_SECURITY_CRYPT_SHA256    );
	zend_declare_class_constant_long(phalcon_security_ce, SL("CRYPT_SHA512"),     PHALCON_SECURITY_CRYPT_SHA512    );

	return SUCCESS;
}

/**
 * Sets a number of bytes to be generated by the openssl pseudo random generator
 *
 * @param string $randomBytes
 */
PHP_METHOD(Phalcon_Security, setRandomBytes){

	zval *random_bytes;

	phalcon_fetch_params(0, 1, 0, &random_bytes);

	PHALCON_ENSURE_IS_LONG(random_bytes);

	if (Z_LVAL_P(random_bytes) < 16) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_security_exception_ce, "At least 16 bytes are needed to produce a correct salt");
		return;
	}

	phalcon_update_property_this(getThis(), SL("_numberBytes"), random_bytes);
}

/**
 * Returns a number of bytes to be generated by the openssl pseudo random generator
 *
 * @return string
 */
PHP_METHOD(Phalcon_Security, getRandomBytes){


	RETURN_MEMBER(getThis(), "_numberBytes");
}

/**
 * Sets the default working factor for bcrypts password's salts
 *
 * @param int $workFactor
 */
PHP_METHOD(Phalcon_Security, setWorkFactor){

	zval *work_factor;

	phalcon_fetch_params(0, 1, 0, &work_factor);

	PHALCON_ENSURE_IS_LONG(work_factor);
	phalcon_update_property_this(getThis(), SL("_workFactor"), work_factor);
}

/**
 * Returns the default working factor for bcrypts password's salts
 *
 * @return int
 */
PHP_METHOD(Phalcon_Security, getWorkFactor){


	RETURN_MEMBER(getThis(), "_workFactor");
}

/**
 * Generate a >22-length pseudo random string to be used as salt for passwords
 *
 * @return string
 */
PHP_METHOD(Phalcon_Security, getSaltBytes)
{
	zval *number_bytes = NULL, *b64 = NULL, tmp, n;
	zend_string *encoded;
	int i_bytes, encode, fd;
	char *result = NULL;

	phalcon_fetch_params(0, 0, 2, &number_bytes, &b64);

	if (number_bytes) {
		i_bytes = phalcon_get_intval(number_bytes);
	} else {
		phalcon_return_property(&n, getThis(), SL("_numberBytes"));
		i_bytes = phalcon_get_intval(&n);
	}

	encode = (!b64 || zend_is_true(b64)) ? 1 : 0;

	if (phalcon_function_exists_ex(SL("openssl_random_pseudo_bytes")) == FAILURE) {
		ssize_t read_bytes = 0;
		fd = open("/dev/urandom", O_RDONLY);
		if (EXPECTED(fd >= 0)) {
			result = emalloc(i_bytes + 1);

			while (read_bytes < i_bytes) {
				ssize_t n = read(fd, result + read_bytes, i_bytes - read_bytes);
				if (n < 0) {
					break;
				}

				read_bytes += n;
			}

			close(fd);
		}

		if (UNEXPECTED(read_bytes != i_bytes)) {
			efree(result);
			RETURN_FALSE;
		}
	} else {
		ZVAL_LONG(&n, i_bytes);
		PHALCON_CALL_FUNCTIONW(&tmp, "openssl_random_pseudo_bytes", &n);

		if (Z_TYPE(tmp) != IS_STRING || Z_STRLEN(tmp) < i_bytes) {
			zval_dtor(&tmp);
			RETURN_FALSE;
		}

		result = Z_STRVAL(tmp);
		zval_dtor(&tmp);
	}

	result[i_bytes] = 0;

	if (encode) {
		encoded = php_base64_encode((unsigned char*)result, i_bytes);
		if (encoded) {
			assert(encoded->len >= i_bytes);
			php_strtr(encoded->val, encoded->len, "+=", "./", 2);
			encoded->val[i_bytes] = 0;
			RETVAL_STRINGL(encoded->val, i_bytes);
		} else {
			RETVAL_FALSE;
		}

		efree(result);
	} else {
		RETURN_STRINGL(result, i_bytes);
	}
}

/**
 * Creates a password hash using bcrypt with a pseudo random salt
 *
 * @param string $password
 * @param int $workFactor
 * @return string
 */
PHP_METHOD(Phalcon_Security, hash)
{
	zval *password, *work_factor = NULL, n_bytes, salt_bytes, default_hash, z_salt;
	char variant, *salt;
	int salt_len, i_factor, i_hash;

	phalcon_fetch_params(0, 1, 1, &password, &work_factor);
	PHALCON_ENSURE_IS_STRING(password);

	if (!work_factor || Z_TYPE_P(work_factor) == IS_NULL) {
		work_factor = phalcon_read_property(getThis(), SL("_workFactor"), PH_NOISY);
	}

	i_factor = phalcon_get_intval(work_factor);

	phalcon_return_property(&default_hash, getThis(), SL("_defaultHash"));
	i_hash = phalcon_get_intval(&default_hash);

	switch (i_hash) {
		default:
		case PHALCON_SECURITY_CRYPT_DEFAULT:
		case PHALCON_SECURITY_CRYPT_BLOWFISH:
			if (!PHALCON_GLOBAL(security.crypt_blowfish_supported)) RETURN_FALSE;
			variant = 'a';
			break;

		case PHALCON_SECURITY_CRYPT_BLOWFISH_X:
			if (!PHALCON_GLOBAL(security.crypt_blowfish_y_supported)) RETURN_FALSE;
			variant = 'x';
			break;

		case PHALCON_SECURITY_CRYPT_BLOWFISH_Y:
			if (!PHALCON_GLOBAL(security.crypt_blowfish_y_supported)) RETURN_FALSE;
			variant = 'y';
			break;

		case PHALCON_SECURITY_CRYPT_STD_DES:
			if (!PHALCON_GLOBAL(security.crypt_std_des_supported)) RETURN_FALSE;
			break;

		case PHALCON_SECURITY_CRYPT_EXT_DES:
			if (!PHALCON_GLOBAL(security.crypt_ext_des_supported)) RETURN_FALSE;
			break;

		case PHALCON_SECURITY_CRYPT_MD5:
			if (!PHALCON_GLOBAL(security.crypt_md5_supported)) RETURN_FALSE;
			break;

		case PHALCON_SECURITY_CRYPT_SHA256:
			if (!PHALCON_GLOBAL(security.crypt_sha256_supported)) RETURN_FALSE;
			variant = '5';
			break;

		case PHALCON_SECURITY_CRYPT_SHA512:
			if (!PHALCON_GLOBAL(security.crypt_sha512_supported)) RETURN_FALSE;
			variant = '6';
			break;
	}

	switch (i_hash) {
		case PHALCON_SECURITY_CRYPT_DEFAULT:
		case PHALCON_SECURITY_CRYPT_BLOWFISH:
		case PHALCON_SECURITY_CRYPT_BLOWFISH_X:
		case PHALCON_SECURITY_CRYPT_BLOWFISH_Y:
		default: {
			/*
			 * Blowfish hashing with a salt as follows: "$2a$", "$2x$" or "$2y$",
			 * a two digit cost parameter, "$", and 22 characters from the alphabet
			 * "./0-9A-Za-z". Using characters outside of this range in the salt
			 * will cause crypt() to return a zero-length string. The two digit cost
			 * parameter is the base-2 logarithm of the iteration count for the
			 * underlying Blowfish-based hashing algorithm and must be in
			 * range 04-31, values outside this range will cause crypt() to fail.
			 */
			ZVAL_LONG(&n_bytes, 22);
			PHALCON_CALL_METHODW(&salt_bytes, getThis(), "getsaltbytes", &n_bytes);
			if (Z_TYPE(salt_bytes) != IS_STRING) {
				zend_throw_exception_ex(phalcon_security_exception_ce, 0, "Unable to get random bytes for the salt");
				return;
			}

			if (i_factor < 4) {
				i_factor = 4;
			} else if (i_factor > 31) {
				i_factor = 31;
			}

			assert(Z_STRLEN(salt_bytes) == 22);
			salt_len = spprintf(&salt, 0, "$2%c$%02d$%.22s", variant, i_factor, Z_STRVAL(salt_bytes));
			assert(salt_len == 29);
			break;
		}

		case PHALCON_SECURITY_CRYPT_STD_DES: {
			/* Standard DES-based hash with a two character salt from the alphabet "./0-9A-Za-z". */
			ZVAL_LONG(&n_bytes, 2);
			PHALCON_CALL_METHODW(&salt_bytes, getThis(), "getsaltbytes", &n_bytes);
			if (Z_TYPE(salt_bytes) != IS_STRING) {
				zend_throw_exception_ex(phalcon_security_exception_ce, 0, "Unable to get random bytes for the salt");
				return;
			}

			assert(Z_STRLEN(salt_bytes) == 2);
			salt     = Z_STRVAL(salt_bytes);
			salt_len = Z_STRLEN(salt_bytes);
			break;
		}

		case PHALCON_SECURITY_CRYPT_EXT_DES: {
			char buf[4];
			/*
			 * Extended DES-based hash. The "salt" is a 9-character string
			 * consisting of an underscore followed by 4 bytes of iteration count
			 * and 4 bytes of salt. These are encoded as printable characters,
			 * 6 bits per character, least significant character first.
			 * The values 0 to 63 are encoded as "./0-9A-Za-z".
			 */
			buf[0] = ascii64[i_factor & 0x3F];
			buf[1] = ascii64[(i_factor >> 6)  & 0x3F];
			buf[2] = ascii64[(i_factor >> 12) & 0x3F];
			buf[3] = ascii64[(i_factor >> 18) & 0x3F];

			ZVAL_LONG(&n_bytes, 4);
			PHALCON_CALL_METHODW(&salt_bytes, getThis(), "getsaltbytes", &n_bytes);
			if (Z_TYPE(salt_bytes) != IS_STRING) {
				zend_throw_exception_ex(phalcon_security_exception_ce, 0, "Unable to get random bytes for the salt");
				return;
			}

			assert(Z_STRLEN(salt_bytes) == 4);
			salt_len = spprintf(&salt, 0, "_%c%c%c%c%.4s", buf[0], buf[1], buf[2], buf[3], Z_STRVAL(salt_bytes));
			assert(salt_len == 9);
			break;
		}

		case PHALCON_SECURITY_CRYPT_MD5: {
			/* MD5 hashing with a twelve character salt starting with $1$ */
			ZVAL_LONG(&n_bytes, 12);
			PHALCON_CALL_METHODW(&salt_bytes, getThis(), "getsaltbytes", &n_bytes);
			if (Z_TYPE(salt_bytes) != IS_STRING) {
				zend_throw_exception_ex(phalcon_security_exception_ce, 0, "Unable to get random bytes for the salt");
				return;
			}

			assert(Z_STRLEN(salt_bytes) == 12);
			salt_len = spprintf(&salt, 0, "$1$%.12s", Z_STRVAL(salt_bytes));
			assert(salt_len == 15);
			break;
		}

		case PHALCON_SECURITY_CRYPT_SHA256:
		/* SHA-256 hash with a sixteen character salt prefixed with $5$. */
		case PHALCON_SECURITY_CRYPT_SHA512: {
			/*
			 * SHA-512 hash with a sixteen character salt prefixed with $6$.
			 *
			 * If the salt string starts with 'rounds=<N>$', the numeric value of N
			 * is used to indicate how many times the hashing loop should be
			 * executed, much like the cost parameter on Blowfish.
			 * The default number of rounds is 5000, there is a minimum of 1000 and
			 * a maximum of 999,999,999. Any selection of N outside this range
			 * will be truncated to the nearest limit.
			 */
			ZVAL_LONG(&n_bytes, 16);
			PHALCON_CALL_METHODW(&salt_bytes, getThis(), "getsaltbytes", &n_bytes);
			if (Z_TYPE(salt_bytes) != IS_STRING) {
				zend_throw_exception_ex(phalcon_security_exception_ce, 0, "Unable to get random bytes for the salt");
				return;
			}

			assert(Z_STRLEN(salt_bytes) == 16);
			if (i_factor) {
				salt_len = spprintf(&salt, 0, "$%c$rounds=%d$%.16s", variant, i_factor, Z_STRVAL(salt_bytes));
			}
			else {
				salt_len = spprintf(&salt, 0, "$%c$%.16s", variant, Z_STRVAL(salt_bytes));
				assert(salt_len == 19);
			}

			break;
		}
	}

	ZVAL_STRINGL(&z_salt, salt, salt_len);

	PHALCON_RETURN_CALL_FUNCTIONW("crypt", password, &z_salt);

	if (Z_STRLEN_P(return_value) < 13) {
		zval_dtor(return_value);
		RETURN_FALSE;
	}
}

/**
 * Checks a plain text password and its hash version to check if the password matches
 *
 * @param string $password
 * @param string $passwordHash
 * @param int $maxPasswordLength
 * @return boolean
 */
PHP_METHOD(Phalcon_Security, checkHash){

	zval *password, *password_hash, *max_pass_length = NULL, hash;
	int check = 0;

	phalcon_fetch_params(0, 2, 1, &password, &password_hash, &max_pass_length);
	PHALCON_ENSURE_IS_STRING(password);
	PHALCON_ENSURE_IS_STRING(password_hash);

	if (max_pass_length) {
		PHALCON_ENSURE_IS_LONG(max_pass_length);
		if (Z_LVAL_P(max_pass_length) > 0 && Z_STRLEN_P(password) > Z_LVAL_P(max_pass_length)) {
			RETURN_FALSE;
		}
	}

	PHALCON_CALL_FUNCTIONW(&hash, "crypt", password, password_hash);

	if (UNEXPECTED(Z_TYPE(hash) != IS_STRING)) {
		convert_to_string(&hash);
	}

	if (Z_STRLEN(hash) == Z_STRLEN_P(password_hash)) {
		int n    = Z_STRLEN(hash);
		char *h1 = Z_STRVAL(hash);
		char *h2 = Z_STRVAL_P(password_hash);

		while (n) {
			check |= ((unsigned int)*h1) ^ ((unsigned int)*h2);
			++h1;
			++h2;
			--n;
		}

		zval_ptr_dtor(&hash);
		RETURN_BOOL(check == 0);
	}

	zval_ptr_dtor(&hash);
	RETURN_FALSE;
}

/**
 * Checks if a password hash is a valid bcrypt's hash
 *
 * @param string $password
 * @param string $passwordHash
 * @return boolean
 */
PHP_METHOD(Phalcon_Security, isLegacyHash){

	zval *password_hash;

	phalcon_fetch_params(0, 1, 0, &password_hash);

	if (phalcon_start_with_str(password_hash, SL("$2a$"))) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}

/**
 * Generates a pseudo random token key to be used as input's name in a CSRF check
 *
 * @param int $numberBytes
 * @return string
 */
PHP_METHOD(Phalcon_Security, getTokenKey){

	zval *name = NULL, *_number_bytes = NULL, number_bytes, key, random_bytes, base64bytes, safe_bytes, service, session;

	phalcon_fetch_params(0, 0, 2, &name, &number_bytes);

	if (!_number_bytes) {
		ZVAL_LONG(&number_bytes, 12);
	} else {
		ZVAL_COPY(&number_bytes, _number_bytes);
	}

	ZVAL_STRING(&key, "$PHALCON/CSRF/KEY$");

	if (name && !PHALCON_IS_EMPTY(name)) {
		PHALCON_SCONCAT(&key, name);
	}

	if (phalcon_function_exists_ex(SL("openssl_random_pseudo_bytes")) == FAILURE) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_security_exception_ce, "Openssl extension must be loaded");
		return;
	}

	PHALCON_CALL_FUNCTIONW(&random_bytes, "openssl_random_pseudo_bytes", &number_bytes);

	phalcon_base64_encode(&base64bytes, &random_bytes);
	phalcon_filter_alphanum(&safe_bytes, &base64bytes);

	ZVAL_STRING(&service, ISV(session));

	PHALCON_CALL_METHODW(&session, getThis(), "getresolveservice", &service, &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_true));
	PHALCON_VERIFY_INTERFACEW(&session, phalcon_session_adapterinterface_ce);

	PHALCON_CALL_METHODW(NULL, &session, "set", &key, &safe_bytes);

	RETURN_CTORW(&safe_bytes);
}

/**
 * Generates a pseudo random token value to be used as input's value in a CSRF check
 *
 * @param int $numberBytes
 * @return string
 */
PHP_METHOD(Phalcon_Security, getToken){

	zval *name = NULL, *_number_bytes = NULL, number_bytes, key, random_bytes, token, service, session;

	phalcon_fetch_params(0, 0, 2, &name, &number_bytes);

	if (!_number_bytes) {
		ZVAL_LONG(&number_bytes, 12);
	} else {
		ZVAL_COPY(&number_bytes, _number_bytes);
	}

	ZVAL_STRING(&key, "$PHALCON/CSRF$");

	if (name && !PHALCON_IS_EMPTY(name)) {
		PHALCON_SCONCAT(&key, name);
	}

	if (phalcon_function_exists_ex(SL("openssl_random_pseudo_bytes")) == FAILURE) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_security_exception_ce, "Openssl extension must be loaded");
		return;
	}

	PHALCON_CALL_FUNCTIONW(&random_bytes, "openssl_random_pseudo_bytes", &number_bytes);

	phalcon_md5(&token, &random_bytes);

	ZVAL_STRING(&service, ISV(session));

	PHALCON_CALL_METHODW(&session, getThis(), "getresolveservice", &service, &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_true));
	PHALCON_VERIFY_INTERFACEW(&session, phalcon_session_adapterinterface_ce);

	PHALCON_CALL_METHODW(NULL, &session, "set", &key, &token);

	RETURN_CTORW(token);
}

/**
 * Check if the CSRF token sent in the request is the same that the current in session
 *
 * @param string $tokenKey
 * @param string $tokenValue
 * @return boolean
 */
PHP_METHOD(Phalcon_Security, checkToken){

	zval *name = NULL, *_token_key = NULL, *token_value = NULL, token_key, service, session, key, request, token, session_token;

	phalcon_fetch_params(0, 0, 3, &name, &token_key, &token_value);

	if (!name) {
		name = &PHALCON_GLOBAL(z_null);
	}

	if (!token_key) {
		token_key = &PHALCON_GLOBAL(z_null);
	}

	if (!token_value) {
		token_value = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STRING(&service, ISV(session));

	PHALCON_CALL_METHODW(&session, getThis(), "getresolveservice", &service, &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_true));
	PHALCON_VERIFY_INTERFACEW(&session, phalcon_session_adapterinterface_ce);

	if (Z_TYPE(token_key) == IS_NULL) {
		ZVAL_STRING(&key, "$PHALCON/CSRF/KEY$");

		if (!PHALCON_IS_EMPTY(name)) {
			PHALCON_SCONCAT(&key, name);
		}

		PHALCON_CALL_METHODW(&token_key, &session, "get", &key);
	}

	if (Z_TYPE_P(token_value) == IS_NULL) {
		ZVAL_STRING(&service, ISV(request));

		PHALCON_CALL_METHODW(&session, getThis(), "getresolveservice", &service, &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_true));
		PHALCON_VERIFY_INTERFACEW(&request, phalcon_http_requestinterface_ce);

		/**
		 * We always check if the value is correct in post
		 */
		PHALCON_CALL_METHODW(&token, &request, "getpost", &token_key);
	} else {
		ZVAL_COPY(&token, token_value);
	}

	ZVAL_STRING(&key, "$PHALCON/CSRF$");

	if (!PHALCON_IS_EMPTY(name)) {
		PHALCON_SCONCAT(&key, name);
	}

	PHALCON_CALL_METHODW(&session_token, &session, "get", &key);

	/**
	 * The value is the same?
	 */
	is_equal_function(return_value, &token, &session_token);
}

/**
 * Returns the value of the CSRF token in session
 *
 * @return string
 */
PHP_METHOD(Phalcon_Security, getSessionToken){

	zval *name = NULL, service, session, key;

	phalcon_fetch_params(0, 0, 1, &name);

	if (!name) {
		name = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STRING(&key, "$PHALCON/CSRF$");

	if (!PHALCON_IS_EMPTY(name)) {
		PHALCON_SCONCAT(&key, name);
	}

	ZVAL_STRING(&service, ISV(session));

	PHALCON_CALL_METHODW(&session, getThis(), "getresolveservice", &service, &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_true));
	PHALCON_VERIFY_INTERFACEW(&session, phalcon_session_adapterinterface_ce);

	PHALCON_RETURN_CALL_METHODW(&session, "get", &key);
}

/**
 * Removes the value of the CSRF token and key from session
 */
PHP_METHOD(Phalcon_Security, destroyToken){

	zval *name = NULL, *token_key = NULL, service, session, key;

	PHALCON_MM_GROW();

	phalcon_fetch_params(0, 0, 2, &name, &token_key);

	if (!name) {
		name = &PHALCON_GLOBAL(z_null);
	}

	if (!token_key) {
		token_key = &PHALCON_GLOBAL(z_null);
	}

	ZVAL_STRING(&service, ISV(session));

	PHALCON_CALL_METHODW(&session, getThis(), "getresolveservice", &service, &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_true));
	PHALCON_VERIFY_INTERFACEW(&session, phalcon_session_adapterinterface_ce);

	ZVAL_STRING(&key, "$PHALCON/CSRF$");

	if (!PHALCON_IS_EMPTY(name)) {
		PHALCON_SCONCAT(&key, name);
	}

	PHALCON_CALL_METHODW(NULL, &session, "remove", &key);

	if (PHALCON_IS_EMPTY(token_key)) {
		ZVAL_STRING(&key, "$PHALCON/CSRF/KEY$");
		if (!PHALCON_IS_EMPTY(name)) {
			PHALCON_SCONCAT(&key, name);
		}
	} else {
		if (UNEXPECTED(Z_TYPE_P(token_key) != IS_STRING)) {
			PHALCON_ENSURE_IS_STRING(token_key);
		}

		ZVAL_STRING(&key, Z_STRVAL_P(token_key));
	}

	PHALCON_CALL_METHODW(NULL, &session, "remove", &key);
}

/**
 * string \Phalcon\Security::computeHmac(string $data, string $key, string $algo, bool $raw = false)
 *
 *
 */
PHP_METHOD(Phalcon_Security, computeHmac)
{
	zval *data, *key, *algo, *raw = NULL;

	phalcon_fetch_params(0, 3, 1, &data, &key, &algo, &raw);

	if (!raw) {
		raw = &PHALCON_GLOBAL(z_false);
	}

	PHALCON_RETURN_CALL_FUNCTIONW("hash_hmac", algo, data, key, raw);
}

/**
 * @internal
 * @brief This method is used only for internal tests, use Phalcon\Security::deriveKey() instead
 */
PHP_METHOD(Phalcon_Security, pbkdf2)
{
	zval *password, *salt, *hash = NULL, *iterations = NULL, *size = NULL, algo, tmp, computed_salt, result;
	div_t d;
	char *s_hash, *s;
	int i_iterations = 0, i_size = 0;
	int i_hash_len, block_count, i, j, k;
	int salt_len;

	phalcon_fetch_params(0, 2, 3, &password, &salt, &hash, &iterations, &size);
	PHALCON_ENSURE_IS_STRING(password);
	PHALCON_ENSURE_IS_STRING(salt);

	if (Z_STRLEN_P(salt) > INT_MAX - 4) {
		zend_throw_exception_ex(phalcon_security_exception_ce, 0, "Salt is too long: %d", Z_STRLEN_P(salt));
		return;
	}

	salt_len = Z_STRLEN_P(salt);

	s_hash = (!hash || Z_TYPE_P(hash) != IS_STRING) ? "sha512" : Z_STRVAL_P(hash);

	if (iterations) {
		PHALCON_ENSURE_IS_LONG(iterations);
		i_iterations = Z_LVAL_P(iterations);
	}

	if (i_iterations <= 0) {
		i_iterations = 5000;
	}

	if (size) {
		PHALCON_ENSURE_IS_LONG(size);
		i_size = Z_LVAL_P(size);
	}

	if (i_size < 0) {
		i_size = 0;
	}

	ZVAL_STRING(&algo, s_hash);

	PHALCON_CALL_FUNCTIONW(&tmp, "hash", &algo, &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_true));
	if (PHALCON_IS_FALSE(&tmp) || Z_TYPE(tmp) != IS_STRING) {
		RETURN_FALSE;
	}

	i_hash_len = Z_STRLEN(tmp);
	if (!i_size) {
		i_size = i_hash_len;
	}

	s = safe_emalloc(salt_len, 1, 5);
	s[salt_len + 4] = 0;
	memcpy(s, Z_STRVAL_P(salt), salt_len);
	ZVAL_STRINGL(&computed_salt, s, salt_len + 4);

	d           = div(i_size, i_hash_len);
	block_count = d.quot + (d.rem ? 1 : 0);

	for (i = 1; i <= block_count; ++i) {
		zval K1,  K2;
		s[salt_len+0] = (unsigned char)(i >> 24);
		s[salt_len+1] = (unsigned char)(i >> 16);
		s[salt_len+2] = (unsigned char)(i >> 8);
		s[salt_len+3] = (unsigned char)(i);

		PHALCON_CALL_FUNCTIONW(&K1, "hash_hmac", &algo, &computed_salt, password, &PHALCON_GLOBAL(z_true));
		if (Z_TYPE(K1) != IS_STRING) {
			RETURN_FALSE;
		}

		ZVAL_COPY_VALUE(&K2, &K1);

		for (j = 1; j < i_iterations; ++j) {
			char *k1, *k2;

			PHALCON_CALL_FUNCTIONW(&tmp, "hash_hmac", &algo, &K1, password, &PHALCON_GLOBAL(z_true));
			if (Z_TYPE(tmp) != IS_STRING) {
				RETURN_FALSE;
			}

			ZVAL_COPY_VALUE(&K1, &tmp);

			k1 = Z_STRVAL(K1);
			k2 = Z_STRVAL(K2);
			for (k = 0; k < Z_STRLEN(K2); ++k) {
				k2[k] ^= k1[k];
			}
		}

		phalcon_concat_self(&result, &K2);
	}

	if (i_size == i_hash_len) {
		RETVAL_STRINGL(Z_STRVAL(result), Z_STRLEN(result));
		ZVAL_NULL(&result);
	} else {
		phalcon_substr(return_value, &result, 0, i_size);
	}
}

/**
 * Derives a key from the given password (PBKDF2).
 *
 * @param string $password Source password
 * @param string $salt The salt to use for the derivation; this value should be generated randomly.
 * @param string $hash Hash function (SHA-512 by default)
 * @param int $iterations The number of internal iterations to perform for the derivation, by default 5000
 * @param int $size The length of the output string. If 0 is passed (the default), the entire output of the supplied hash algorithm is used
 * @return string The derived key
 */
PHP_METHOD(Phalcon_Security, deriveKey)
{
	zval *password, *salt, *hash = NULL, *iterations = NULL, *size = NULL, algo, iter, len;
	char* s_hash;
	int i_iterations = 0, i_size = 0;


	phalcon_fetch_params(0, 2, 3, &password, &salt, &hash, &iterations, &size);
	PHALCON_ENSURE_IS_STRING(password);
	PHALCON_ENSURE_IS_STRING(salt);

	if (Z_STRLEN_P(salt) > INT_MAX - 4) {
		zend_throw_exception_ex(phalcon_security_exception_ce, 0, "Salt is too long: %d", Z_STRLEN_P(salt));
		return;
	}

	s_hash = (!hash || Z_TYPE_P(hash) != IS_STRING) ? "sha512" : Z_STRVAL_P(hash);

	if (iterations) {
		PHALCON_ENSURE_IS_LONG(iterations);
		i_iterations = Z_LVAL_P(iterations);
	}

	if (i_iterations <= 0) {
		i_iterations = 5000;
	}

	if (size) {
		PHALCON_ENSURE_IS_LONG(size);
		i_size = Z_LVAL_P(size);
	}

	if (i_size < 0) {
		i_size = 0;
	}

	ZVAL_STRING(&algo, s_hash);
	ZVAL_LONG(&iter, i_iterations);
	ZVAL_LONG(&len, i_size);

	PHALCON_CALL_FUNCTIONW(return_value, "hash_pbkdf2", &algo, password, salt, &iter, &len, &PHALCON_GLOBAL(z_true));
}

/**
 * Sets the default hash
 */
PHP_METHOD(Phalcon_Security, setDefaultHash)
{
	zval *default_hash;

	phalcon_fetch_params(0, 1, 0, &default_hash);
	PHALCON_ENSURE_IS_LONG(default_hash);

	phalcon_update_property_this(getThis(), SL("_defaultHash"), default_hash);
}

/**
 * Returns the default hash
 */
PHP_METHOD(Phalcon_Security, getDefaultHash)
{
	RETURN_MEMBER(getThis(), "_defaultHash");
}
