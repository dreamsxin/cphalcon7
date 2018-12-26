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

#include "kernel/string.h"

#include <ctype.h>

#include <Zend/zend_smart_str.h>
#include <ext/standard/php_string.h>
#include <ext/standard/php_rand.h>
#include <ext/standard/php_lcg.h>
#include <ext/standard/php_http.h>
#include <ext/standard/base64.h>
#include <ext/standard/md5.h>
#include <ext/standard/crc32.h>
#include <ext/standard/url.h>
#include <ext/standard/html.h>
#include <ext/date/php_date.h>
#include <ext/pcre/php_pcre.h>

#ifdef PHALCON_USE_PHP_JSON
#include <ext/json/php_json.h>
#endif

#ifdef PHALCON_USE_PHP_MBSTRING
# include <ext/mbstring/mbstring.h>
# include <ext/mbstring/php_unicode.h>
#endif

#include "kernel/main.h"
#include "kernel/operators.h"
#include "kernel/fcall.h"

/**
 * Fast call to php strlen
 */
void phalcon_fast_strlen(zval *return_value, zval *str)
{
	zval copy = {};
	int use_copy = 0;

	if (Z_TYPE_P(str) != IS_STRING) {
		use_copy = zend_make_printable_zval(str, &copy);
		if (use_copy) {
			str = &copy;
		}
	}

	ZVAL_LONG(return_value, Z_STRLEN_P(str));

	if (unlikely(use_copy)) {
		zval_ptr_dtor(str);
	}
}

void phalcon_strlen(zval *return_value, zval *str)
{
#ifdef PHALCON_USE_PHP_MBSTRING
	zval copy = {};
	int use_copy = 0, n;
	mbfl_string string;

	if (Z_TYPE_P(str) != IS_STRING) {
		use_copy = zend_make_printable_zval(str, &copy);
	} else {
		ZVAL_COPY_VALUE(&copy, str);
	}

	if (ZEND_SIZE_T_UINT_OVFL(Z_STRLEN(copy))) {
		php_error_docref(NULL, E_WARNING, "String overflows the max allowed length of %u", UINT_MAX);
		return;
	}

	mbfl_string_init(&string);

	string.len = (uint32_t)Z_STRLEN(copy);
	string.no_language = MBSTRG(language);
	string.no_encoding = MBSTRG(current_internal_encoding)->no_encoding;

	n = mbfl_strlen(&string);
	if (n >= 0) {
		ZVAL_LONG(return_value, n);
	} else {
		ZVAL_FALSE(return_value);
	}
	if (unlikely(use_copy)) {
		zval_ptr_dtor(&copy);
	}
#else
	int flag;
	zval str_value = {};
	phalcon_strval(&str_value, str);
	PHALCON_CALL_FUNCTION_FLAG(flag, return_value, "mb_strlen", &str_value);
	zval_ptr_dtor(&str_value);
	if (flag != SUCCESS) {
		ZVAL_FALSE(return_value);
	}
#endif
}

/**
 * Fast call to php strlen
 */
int phalcon_fast_strlen_ev(zval *str)
{
	zval copy = {};
	int use_copy = 0, length;

	if (Z_TYPE_P(str) != IS_STRING) {
		use_copy = zend_make_printable_zval(str, &copy);
		if (use_copy) {
			str = &copy;
		}
	}

	length = Z_STRLEN_P(str);
	if (use_copy) {
		zval_ptr_dtor(str);
	}

	return length;
}

/**
 * Fast call to php strtolower
 */
void phalcon_fast_strtolower(zval *return_value, zval *str)
{
	zval copy;
	int use_copy = 0;
	char *lower_str;
	unsigned int length;

	if (Z_TYPE_P(str) != IS_STRING) {
		use_copy = zend_make_printable_zval(str, &copy);
		if (use_copy) {
			str = &copy;
		}
	}

	length = Z_STRLEN_P(str);
	lower_str = estrndup(Z_STRVAL_P(str), length);
	php_strtolower(lower_str, length);

	if (use_copy) {
		zval_dtor(str);
	}

	ZVAL_STRINGL(return_value, lower_str, length);
	efree(lower_str);
}

void phalcon_strtolower(zval *return_value, zval *str)
{
#ifdef PHALCON_USE_PHP_MBSTRING
	zval copy = {};
	int use_copy = 0;
	const char *from_encoding = MBSTRG(current_internal_encoding)->mime_name;
	char *newstr;
	size_t ret_len;

	if (unlikely(Z_TYPE_P(str) != IS_STRING)) {
		use_copy = zend_make_printable_zval(str, &copy);
	} else {
		ZVAL_COPY_VALUE(&copy, str);
	}

	newstr = php_unicode_convert_case(PHP_UNICODE_CASE_LOWER, Z_STRVAL(copy), Z_STRLEN(copy), &ret_len, from_encoding);

	if (newstr) {
		ZVAL_STRINGL(return_value, newstr, ret_len);
		efree(newstr);
	} else {
		ZVAL_FALSE(return_value);
	}
	
	if (unlikely(use_copy)) {
		zval_ptr_dtor(&copy);
	}
#else
	phalcon_fast_strtolower(return_value, str);
#endif
}

void phalcon_strtolower_inplace(zval *s) {
	if (likely(Z_TYPE_P(s) == IS_STRING)) {
		zend_str_tolower(Z_STRVAL_P(s), Z_STRLEN_P(s));
	}
}

/**
 * Fast call to PHP strtoupper() function
 */
void phalcon_fast_strtoupper(zval *return_value, zval *str)
{
	zval copy = {};
	int use_copy = 0;
	char *lower_str;
	unsigned int length;

	if (Z_TYPE_P(str) != IS_STRING) {
		use_copy = zend_make_printable_zval(str, &copy);
		if (use_copy) {
			str = &copy;
		}
	}

	length = Z_STRLEN_P(str);
	lower_str = estrndup(Z_STRVAL_P(str), length);
	php_strtoupper(lower_str, length);

	if (use_copy) {
		zval_ptr_dtor(str);
	}

	ZVAL_STRINGL(return_value, lower_str, length);
}

void phalcon_strtoupper(zval *return_value, zval *str)
{
#ifdef PHALCON_USE_PHP_MBSTRING
	zval copy = {};
	int use_copy = 0;
	const char *from_encoding = MBSTRG(current_internal_encoding)->mime_name;
	char *newstr;
	size_t ret_len;

	if (Z_TYPE_P(str) != IS_STRING) {
		use_copy = zend_make_printable_zval(str, &copy);
	} else {
		ZVAL_COPY_VALUE(&copy, str);
	}

	newstr = php_unicode_convert_case(PHP_UNICODE_CASE_UPPER, Z_STRVAL(copy), Z_STRLEN(copy), &ret_len, from_encoding);

	if (newstr) {
		ZVAL_STRINGL(return_value, newstr, ret_len);
		efree(newstr);
	} else {
		ZVAL_FALSE(return_value);
	}
	if (unlikely(use_copy)) {
		zval_ptr_dtor(&copy);
	}
#else
	phalcon_fast_strtoupper(return_value, str);
#endif
}

/**
 * Fast call to php join  function
 */
void phalcon_fast_join(zval *result, zval *glue, zval *pieces){

	if (Z_TYPE_P(glue) != IS_STRING || Z_TYPE_P(pieces) != IS_ARRAY) {
		ZVAL_NULL(result);
		zend_error(E_WARNING, "Invalid arguments supplied for join()");
		return;
	}

	php_implode(Z_STR_P(glue), pieces, result);
}

/**
 * Appends to a smart_str a printable version of a zval
 */
void phalcon_append_printable_zval(smart_str *implstr, zval *tmp)
{
	zval tmp_val = {};
	unsigned int str_len;

	switch (Z_TYPE_P(tmp)) {
		case IS_STRING:
			smart_str_appendl(implstr, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
			break;

		case IS_LONG:
			smart_str_append_long(implstr, Z_LVAL_P(tmp));
			break;

		case IS_TRUE:
			smart_str_appendl(implstr, "1", sizeof("1") - 1);
			break;

		case IS_FALSE:
			break;

		case IS_NULL:
			break;

		case IS_DOUBLE: {
			char *stmp;
			str_len = spprintf(&stmp, 0, "%.*G", (int) EG(precision), Z_DVAL_P(tmp));
			smart_str_appendl(implstr, stmp, str_len);
			efree(stmp);
			break;
		}

		case IS_OBJECT: {
			zval expr = {};
			int copy = zend_make_printable_zval(tmp, &expr);
			smart_str_appendl(implstr, Z_STRVAL(expr), Z_STRLEN(expr));
			if (copy) {
				zval_ptr_dtor(&expr);
			}
			break;
		}

		default:
			ZVAL_DUP(&tmp_val, tmp);
			convert_to_string(&tmp_val);
			smart_str_append(implstr, Z_STR(tmp_val));
			break;
	}
}

/**
 * Fast join function
 * This function is an adaption of the php_implode function
 *
 */
void phalcon_fast_join_str(zval *return_value, char *glue, unsigned int glue_length, zval *pieces){

	zval *tmp;
	smart_str implstr = {0};
	unsigned int numelems, i = 0;

	if (Z_TYPE_P(pieces) != IS_ARRAY) {
		php_error_docref(NULL, E_WARNING, "Invalid arguments supplied for fast_join()");
		RETURN_EMPTY_STRING();
	}

	numelems = zend_hash_num_elements(Z_ARRVAL_P(pieces));

	if (numelems == 0) {
		RETURN_EMPTY_STRING();
	}

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(pieces), tmp) {
		phalcon_append_printable_zval(&implstr, tmp);
		if (++i != numelems) {
			smart_str_appendl(&implstr, glue, glue_length);
		}
	} ZEND_HASH_FOREACH_END();

	smart_str_0(&implstr);

	if (implstr.s) {
		RETURN_NEW_STR(implstr.s);
	} else {
		smart_str_free(&implstr);
		RETURN_EMPTY_STRING();
	}
}

/**
 * Convert dash/underscored texts returning camelized
 */
void phalcon_camelize(zval *return_value, const zval *str){

	int i, len;
	smart_str camelize_str = {0};
	char *marker, ch;

	if (unlikely(Z_TYPE_P(str) != IS_STRING)) {
		zend_error(E_WARNING, "Invalid arguments supplied for camelize()");
		RETURN_EMPTY_STRING();
		return;
	}

	marker = Z_STRVAL_P(str);
	len    = Z_STRLEN_P(str);

	for (i = 0; i < len; i++) {
		ch = marker[i];
		if (i == 0 || ch == '-' || ch == '_' || ch == '\\') {
			if (ch == '-' || ch == '_') {
				i++;
			} else if (ch == '\\') {
				smart_str_appendc(&camelize_str, marker[i]);
				i++;
			}

			if (i < len) {
				smart_str_appendc(&camelize_str, toupper(marker[i]));
			}
		} else {
			smart_str_appendc(&camelize_str, tolower(marker[i]));
		}
	}

	smart_str_0(&camelize_str);

	if (camelize_str.s) {
		ZVAL_NEW_STR(return_value, camelize_str.s);
	} else {
		ZVAL_EMPTY_STRING(return_value);
	}

}

/**
 * Convert dash/underscored texts returning camelized
 */
void phalcon_uncamelize(zval *return_value, const zval *str){

	int i;
	smart_str uncamelize_str = {0};
	char *marker, ch;

	if (Z_TYPE_P(str) != IS_STRING) {
		zend_error(E_WARNING, "Invalid arguments supplied for camelize()");
		return;
	}

	marker = Z_STRVAL_P(str);
	for (i = 0; i < Z_STRLEN_P(str); i++) {
		ch = *marker;
		if (ch == '\0') {
			break;
		}
		if (ch >= 'A' && ch <= 'Z') {
			if (i > 0) {
				smart_str_appendc(&uncamelize_str, '_');
			}
			smart_str_appendc(&uncamelize_str, (*marker) + 32);
		} else {
			smart_str_appendc(&uncamelize_str, (*marker));
		}
		marker++;
	}
	smart_str_0(&uncamelize_str);

	if (uncamelize_str.s) {
		RETURN_NEW_STR(uncamelize_str.s);
	} else {
		RETURN_EMPTY_STRING();
	}
}

void phalcon_camelize_delim(zval *return_value, const zval *str, const zval *delimiter){

	int i, len, first = 0;
	smart_str camelize_str = {0};
	char *marker, ch, delim;

	if (unlikely(Z_TYPE_P(str) != IS_STRING)) {
		zend_error(E_WARNING, "Invalid arguments supplied for camelize()");
		RETURN_EMPTY_STRING();
	}

	if (delimiter == NULL || Z_TYPE_P(delimiter) == IS_NULL) {
		delim = '_';
	} else if (Z_TYPE_P(delimiter) == IS_STRING && Z_STRLEN_P(delimiter) == 1) {
		delim = *(Z_STRVAL_P(delimiter));
	} else {
		zend_error(E_WARNING, "Second argument passed to the camelize() must be a string of one character");
		RETURN_EMPTY_STRING();
	}

	marker = Z_STRVAL_P(str);
	len    = Z_STRLEN_P(str);

	for (i = 0; i < len; i++) {

		ch = marker[i];

		if (first == 0) {

			if (ch == delim) {
				continue;
			}

			first = 1;
			smart_str_appendc(&camelize_str, toupper(ch));
			continue;
		}

		if (ch == delim) {
			if (i != (len - 1)) {
				i++;
				ch = marker[i];
				smart_str_appendc(&camelize_str, toupper(ch));
			}
			continue;
		}

		smart_str_appendc(&camelize_str, tolower(ch));
	}

	smart_str_0(&camelize_str);

	if (camelize_str.s) {
		RETURN_STR(camelize_str.s);
	} else {
		RETURN_EMPTY_STRING();
	}
}

/**
 * Convert a camelized to a dash/underscored texts (an optional delimiter can be specified)
 */
void phalcon_uncamelize_delim(zval *return_value, const zval *str, const zval *delimiter){

	unsigned int i;
	smart_str uncamelize_str = {0};
	char *marker, ch, delim;

	if (Z_TYPE_P(str) != IS_STRING) {
		zend_error(E_WARNING, "Invalid arguments supplied for uncamelize()");
		RETURN_EMPTY_STRING();
	}

	if (delimiter == NULL || Z_TYPE_P(delimiter) == IS_NULL) {
		delim = '_';
	} else if (Z_TYPE_P(delimiter) == IS_STRING && Z_STRLEN_P(delimiter) == 1) {
		delim = *(Z_STRVAL_P(delimiter));
	} else {
		zend_error(E_WARNING, "Second argument passed to the uncamelize() must be a string of one character");
		RETURN_EMPTY_STRING();
	}

	marker = Z_STRVAL_P(str);
	for (i = 0; i < Z_STRLEN_P(str); i++) {
		ch = *marker;
		if (ch == '\0') {
			break;
		}
		if (ch >= 'A' && ch <= 'Z') {
			if (i > 0) {
				smart_str_appendc(&uncamelize_str, delim);
			}
			smart_str_appendc(&uncamelize_str, (*marker) + 32);
		} else {
			smart_str_appendc(&uncamelize_str, (*marker));
		}
		marker++;
	}
	smart_str_0(&uncamelize_str);

	if (uncamelize_str.s) {
		RETURN_STR(uncamelize_str.s);
	} else {
		RETURN_EMPTY_STRING();
	}
}

/**
 * Fast call to explode php function
 */
void phalcon_fast_explode(zval *result, zval *delimiter, zval *str){

	if (unlikely(Z_TYPE_P(str) != IS_STRING || Z_TYPE_P(delimiter) != IS_STRING)) {
		ZVAL_NULL(result);
		zend_error(E_WARNING, "Invalid arguments supplied for explode()");
		return;
	}

	array_init(result);
	php_explode(Z_STR_P(delimiter), Z_STR_P(str), result, LONG_MAX);
}

/**
 * Fast call to explode php function
 */
void phalcon_fast_explode_str(zval *result, const char *delimiter, unsigned int delimiter_length, zval *str)
{
	zend_string *delim;

	if (unlikely(Z_TYPE_P(str) != IS_STRING)) {
		ZVAL_NULL(result);
		zend_error(E_WARNING, "Invalid arguments supplied for explode()");
		return;
	}

	delim = zend_string_init(delimiter, delimiter_length, 0);

	array_init(result);
	php_explode(delim, Z_STR_P(str), result, LONG_MAX);
	zend_string_release(delim);
}

/**
 * Fast call to explode php function
 */
void phalcon_fast_explode_str_str(zval *result, const char *delimiter, unsigned int delimiter_length, const char *str, unsigned int str_length)
{
	zend_string *delim, *s;

	delim = zend_string_init(delimiter, delimiter_length, 0);
	s = zend_string_init(str, str_length, 0);

	array_init(result);
	php_explode(delim, s, result, LONG_MAX);
	zend_string_release(s);
	zend_string_release(delim);
}

/**
 * Check if a string is contained into another
 */
int phalcon_memnstr(const zval *haystack, const zval *needle) {

	if (Z_TYPE_P(haystack) != IS_STRING || Z_TYPE_P(needle) != IS_STRING) {
		zend_error(E_WARNING, "Invalid arguments supplied for memnstr()");
		return 0;
	}

	if (Z_STRLEN_P(haystack) >= Z_STRLEN_P(needle)) {
		return php_memnstr(Z_STRVAL_P(haystack), Z_STRVAL_P(needle), Z_STRLEN_P(needle), Z_STRVAL_P(haystack) + Z_STRLEN_P(haystack)) ? 1 : 0;
	}

	return 0;
}

int phalcon_memnstr_string(const zval *haystack, const zend_string *needle) {

	if (Z_TYPE_P(haystack) != IS_STRING) {
		zend_error(E_WARNING, "Invalid arguments supplied for memnstr()");
		return 0;
	}

	if (Z_STRLEN_P(haystack) >= ZSTR_LEN(needle)) {
		return zend_memnstr(Z_STRVAL_P(haystack), ZSTR_VAL(needle), ZSTR_LEN(needle), Z_STRVAL_P(haystack) + Z_STRLEN_P(haystack)) ? 1 : 0;
	}

	return 0;
}

int phalcon_memnstr_string_string(zend_string *haystack, zend_string *needle) {

	if (ZSTR_LEN(haystack) >= ZSTR_LEN(needle)) {
		return zend_memnstr(ZSTR_VAL(haystack), ZSTR_VAL(needle), ZSTR_LEN(needle), ZSTR_VAL(haystack) + ZSTR_LEN(haystack)) ? 1 : 0;
	}

	return 0;
}

/**
 * Check if a string is contained into another
 */
int phalcon_memnstr_str(const zval *haystack, char *needle, unsigned int needle_length) {

	if (Z_TYPE_P(haystack) != IS_STRING) {
		zend_error(E_WARNING, "Invalid arguments supplied for memnstr()");
		return 0;
	}

	if ((uint)(Z_STRLEN_P(haystack)) >= needle_length) {
		return php_memnstr(Z_STRVAL_P(haystack), needle, needle_length, Z_STRVAL_P(haystack) + Z_STRLEN_P(haystack)) ? 1 : 0;
	}

	return 0;
}

/**
 * Check if a string is contained into another
 */
int phalcon_memnstr_str_str(const char *haystack, unsigned int haystack_length, char *needle, unsigned int needle_length) {

	if (haystack && haystack_length >= needle_length) {
		return php_memnstr(haystack, needle, needle_length, haystack + strlen(haystack)) ? 1 : 0;
	}

	return 0;
}

int phalcon_same_name(const char *key, const char *name, uint32_t name_len)
{
	char *lcname = zend_str_tolower_dup(name, name_len);
	int ret = memcmp(lcname, key, name_len) == 0;
	efree(lcname);
	return ret;
}

void phalcon_strtr(zval *return_value, zval *str, zval *str_from, zval *str_to) {

	if (Z_TYPE_P(str) != IS_STRING || Z_TYPE_P(str_from) != IS_STRING || Z_TYPE_P(str_to) != IS_STRING) {
		zend_error(E_WARNING, "Invalid arguments supplied for strtr()");
		return;
	}

	ZVAL_STR(return_value, zend_string_dup(Z_STR_P(str), 0));

	php_strtr(Z_STRVAL_P(return_value),
			  Z_STRLEN_P(return_value),
			  Z_STRVAL_P(str_from),
			  Z_STRVAL_P(str_to),
			  MIN(Z_STRLEN_P(str_from),
			  Z_STRLEN_P(str_to)));
}

void phalcon_strtr_str(zval *return_value, zval *str, char *str_from, unsigned int str_from_length, char *str_to, unsigned int str_to_length) {

	if (Z_TYPE_P(str) != IS_STRING) {
		zend_error(E_WARNING, "Invalid arguments supplied for strtr()");
		return;
	}

	ZVAL_NEW_STR(return_value, zend_string_dup(Z_STR_P(str), 0));

	php_strtr(Z_STRVAL_P(return_value),
			  Z_STRLEN_P(return_value),
			  str_from,
			  str_to,
			  MIN(str_from_length,
			  str_to_length));
}

/* {{{ php_strtr_array */
static void php_strtr_array(zval *return_value, zend_string *input, HashTable *pats)
{
	char *str = ZSTR_VAL(input);
	size_t slen = ZSTR_LEN(input);
	zend_ulong num_key;
	zend_string *str_key;
	size_t len, pos, old_pos;
	int num_keys = 0;
	size_t minlen = 128*1024;
	size_t maxlen = 0;
	HashTable str_hash;
	zval *entry;
	char *key;
	smart_str result = {0};
	zend_ulong bitset[256/sizeof(zend_ulong)];
	zend_ulong *num_bitset;

	/* we will collect all possible key lengths */
	num_bitset = ecalloc((slen + sizeof(zend_ulong)) / sizeof(zend_ulong), sizeof(zend_ulong));
	memset(bitset, 0, sizeof(bitset));

	/* check if original array has numeric keys */
	ZEND_HASH_FOREACH_STR_KEY(pats, str_key) {
		if (UNEXPECTED(!str_key)) {
			num_keys = 1;
		} else {
			len = ZSTR_LEN(str_key);
			if (UNEXPECTED(len < 1)) {
				RETURN_FALSE;
			} else if (UNEXPECTED(len > slen)) {
				/* skip long patterns */
				continue;
			}
			if (len > maxlen) {
				maxlen = len;
			}
			if (len < minlen) {
				minlen = len;
			}
			/* remember possible key length */
			num_bitset[len / sizeof(zend_ulong)] |= Z_UL(1) << (len % sizeof(zend_ulong));
			bitset[((unsigned char)ZSTR_VAL(str_key)[0]) / sizeof(zend_ulong)] |= Z_UL(1) << (((unsigned char)ZSTR_VAL(str_key)[0]) % sizeof(zend_ulong));
		}
	} ZEND_HASH_FOREACH_END();

	if (UNEXPECTED(num_keys)) {
		zend_string *key_used;
		/* we have to rebuild HashTable with numeric keys */
		zend_hash_init(&str_hash, zend_hash_num_elements(pats), NULL, NULL, 0);
		ZEND_HASH_FOREACH_KEY_VAL(pats, num_key, str_key, entry) {
			if (UNEXPECTED(!str_key)) {
				key_used = zend_long_to_str(num_key);
				len = ZSTR_LEN(key_used);
				if (UNEXPECTED(len > slen)) {
					/* skip long patterns */
					continue;
				}
				if (len > maxlen) {
					maxlen = len;
				}
				if (len < minlen) {
					minlen = len;
				}
				/* remember possible key length */
				num_bitset[len / sizeof(zend_ulong)] |= Z_UL(1) << (len % sizeof(zend_ulong));
				bitset[((unsigned char)ZSTR_VAL(key_used)[0]) / sizeof(zend_ulong)] |= Z_UL(1) << (((unsigned char)ZSTR_VAL(key_used)[0]) % sizeof(zend_ulong));
			} else {
				key_used = str_key;
				len = ZSTR_LEN(key_used);
				if (UNEXPECTED(len > slen)) {
					/* skip long patterns */
					continue;
				}
			}
			zend_hash_add(&str_hash, key_used, entry);
			if (UNEXPECTED(!str_key)) {
				zend_string_release(key_used);
			}
		} ZEND_HASH_FOREACH_END();
		pats = &str_hash;
	}

	if (UNEXPECTED(minlen > maxlen)) {
		/* return the original string */
		if (pats == &str_hash) {
			zend_hash_destroy(&str_hash);
		}
		efree(num_bitset);
		RETURN_STR_COPY(input);
	}

	old_pos = pos = 0;
	while (pos <= slen - minlen) {
		key = str + pos;
		if (bitset[((unsigned char)key[0]) / sizeof(zend_ulong)] & (Z_UL(1) << (((unsigned char)key[0]) % sizeof(zend_ulong)))) {
			len = maxlen;
			if (len > slen - pos) {
				len = slen - pos;
			}
			while (len >= minlen) {
				if ((num_bitset[len / sizeof(zend_ulong)] & (Z_UL(1) << (len % sizeof(zend_ulong))))) {
					entry = zend_hash_str_find(pats, key, len);
					if (entry != NULL) {
						zend_string *s = zval_get_string(entry);
						smart_str_appendl(&result, str + old_pos, pos - old_pos);
						smart_str_append(&result, s);
						old_pos = pos + len;
						pos = old_pos - 1;
						zend_string_release(s);
						break;
					}
				}
				len--;
			}
		}
		pos++;
	}

	if (result.s) {
		smart_str_appendl(&result, str + old_pos, slen - old_pos);
		smart_str_0(&result);
		RETVAL_NEW_STR(result.s);
	} else {
		smart_str_free(&result);
		RETVAL_STR_COPY(input);
	}

	if (pats == &str_hash) {
		zend_hash_destroy(&str_hash);
	}
	efree(num_bitset);
}
/* }}} */

void phalcon_strtr_array(zval *return_value, zval *str, zval *replace_pairs) {

	if (Z_TYPE_P(str) != IS_STRING|| Z_TYPE_P(replace_pairs) != IS_ARRAY) {
		zend_error(E_WARNING, "Invalid arguments supplied for strtr()");
		return;
	}

	ZVAL_NEW_STR(return_value, zend_string_dup(Z_STR_P(str), 0));

	php_strtr_array(return_value, Z_STR_P(str), Z_ARRVAL_P(replace_pairs));
}

/**
 * Inmediate function resolution for strpos function
 */
int phalcon_fast_strpos(zval *return_value, const zval *haystack, const zval *needle) {

	const char *found = NULL;

	if (unlikely(Z_TYPE_P(haystack) != IS_STRING || Z_TYPE_P(needle) != IS_STRING)) {
		if (return_value) {
			ZVAL_NULL(return_value);
		}
		zend_error(E_WARNING, "Invalid arguments supplied for strpos()");
		return 0;
	}

	if (!Z_STRLEN_P(needle)) {
		if (return_value) {
			ZVAL_NULL(return_value);
		}
		zend_error(E_WARNING, "Empty delimiter");
		return 0;
	}

	found = php_memnstr(Z_STRVAL_P(haystack), Z_STRVAL_P(needle), Z_STRLEN_P(needle), Z_STRVAL_P(haystack) + Z_STRLEN_P(haystack));

	if (found) {
		if (return_value) {
			ZVAL_LONG(return_value, found-Z_STRVAL_P(haystack));
		}
		return 1;
	}
	if (return_value) {
		ZVAL_FALSE(return_value);
	}
	return 0;
}

/**
 * Inmediate function resolution for strpos function
 */
int phalcon_fast_strpos_str(zval *return_value, const zval *haystack, const char *needle, unsigned int needle_length) {

	const char *found = NULL;

	if (unlikely(Z_TYPE_P(haystack) != IS_STRING)) {
		ZVAL_NULL(return_value);
		zend_error(E_WARNING, "Invalid arguments supplied for strpos()");
		return 0;
	}

	found = zend_memnstr(Z_STRVAL_P(haystack), needle, needle_length, Z_STRVAL_P(haystack) + Z_STRLEN_P(haystack));

	if (found) {
		if (return_value) {
			ZVAL_LONG(return_value, found-Z_STRVAL_P(haystack));
		}
		return 1;
	}
	if (return_value) {
		ZVAL_FALSE(return_value);
	}
	return 0;
}

/**
 * Inmediate function resolution for stripos function
 */
int phalcon_fast_stripos_str(zval *return_value, const zval *haystack, const char *needle, unsigned int needle_length) {

	const char *found = NULL;
	char *needle_dup, *haystack_dup;

	if (unlikely(Z_TYPE_P(haystack) != IS_STRING)) {
		ZVAL_NULL(return_value);
		zend_error(E_WARNING, "Invalid arguments supplied for stripos()");
		return 0;
	}

	haystack_dup = estrndup(Z_STRVAL_P(haystack), Z_STRLEN_P(haystack));
	zend_str_tolower(haystack_dup, Z_STRLEN_P(haystack));

	needle_dup = estrndup(needle, needle_length);
	zend_str_tolower(needle_dup, needle_length);

	found = zend_memnstr(haystack_dup, needle, needle_length, haystack_dup + Z_STRLEN_P(haystack));

	efree(haystack_dup);
	efree(needle_dup);

	if (found) {
		if (return_value) {
			ZVAL_LONG(return_value, found-Z_STRVAL_P(haystack));
		}
		return 1;
	}

	if (return_value) {
		ZVAL_FALSE(return_value);
	}
	return 0;
}

/**
 * Inmediate function resolution for strrpos function
 */
int phalcon_fast_strrpos(zval *return_value, const zval *haystack, const zval *needle) {

	const char *found = NULL;

	if (unlikely(Z_TYPE_P(haystack) != IS_STRING || Z_TYPE_P(needle) != IS_STRING)) {
		if (return_value) {
			ZVAL_NULL(return_value);
		}
		zend_error(E_WARNING, "Invalid arguments supplied for strrpos()");
		return 0;
	}

	if (!Z_STRLEN_P(needle)) {
		if (return_value) {
			ZVAL_NULL(return_value);
		}
		zend_error(E_WARNING, "Empty delimiter");
		return 0;
	}

	found = zend_memnrstr(Z_STRVAL_P(haystack), Z_STRVAL_P(needle), Z_STRLEN_P(needle), Z_STRVAL_P(haystack) + Z_STRLEN_P(haystack));

	if (found) {
		if (return_value) {
			ZVAL_LONG(return_value, found-Z_STRVAL_P(haystack));
		}
		return 1;
	}
	if (return_value) {
		ZVAL_FALSE(return_value);
	}
	return 0;
}

/**
 * Inmediate function resolution for strrpos function
 */
int phalcon_fast_strrpos_str(zval *return_value, const zval *haystack, const char *needle, unsigned int needle_length) {

	const char *found = NULL;

	if (unlikely(Z_TYPE_P(haystack) != IS_STRING)) {
		ZVAL_NULL(return_value);
		zend_error(E_WARNING, "Invalid arguments supplied for strpos()");
		return 0;
	}

	found = zend_memnrstr(Z_STRVAL_P(haystack), needle, needle_length, Z_STRVAL_P(haystack) + Z_STRLEN_P(haystack));

	if (found) {
		if (return_value) {
			ZVAL_LONG(return_value, found-Z_STRVAL_P(haystack));
		}
		return 1;
	}
	if (return_value) {
		ZVAL_FALSE(return_value);
	}
	return 0;
}

/**
 * Inmediate function resolution for strripos function
 */
int phalcon_fast_strripos_str(zval *return_value, const zval *haystack, const char *needle, unsigned int needle_length) {

	const char *found = NULL;
	char *needle_dup, *haystack_dup;

	if (unlikely(Z_TYPE_P(haystack) != IS_STRING)) {
		ZVAL_NULL(return_value);
		zend_error(E_WARNING, "Invalid arguments supplied for stripos()");
		return 0;
	}

	haystack_dup = estrndup(Z_STRVAL_P(haystack), Z_STRLEN_P(haystack));
	zend_str_tolower(haystack_dup, Z_STRLEN_P(haystack));

	needle_dup = estrndup(needle, needle_length);
	zend_str_tolower(needle_dup, needle_length);

	found = zend_memnrstr(haystack_dup, needle, needle_length, haystack_dup + Z_STRLEN_P(haystack));

	efree(haystack_dup);
	efree(needle_dup);

	if (found) {
		if (return_value) {
			ZVAL_LONG(return_value, found-Z_STRVAL_P(haystack));
		}
		return 1;
	}

	if (return_value) {
		ZVAL_FALSE(return_value);
	}
	return 0;
}

/**
 * Fast call to PHP trim() function
 */
zend_string* phalcon_trim(zval *str, zval *charlist, int where)
{
	zval copy = {};
	zend_string *s;
	int use_copy = 0;

	if (Z_TYPE_P(str) != IS_STRING) {
		use_copy = zend_make_printable_zval(str, &copy);
		if (use_copy) {
			str = &copy;
		}
	}

	if (charlist && Z_TYPE_P(charlist) != IS_STRING) {
		convert_to_string(charlist);
	}

	if (charlist) {
		s = php_trim(Z_STR_P(str), Z_STRVAL_P(charlist), Z_STRLEN_P(charlist), where);
	} else {
		s = php_trim(Z_STR_P(str), NULL, 0, where);
	}

	return s;
}

/**
 * Fast call to PHP strip_tags() function
 */
void phalcon_fast_strip_tags(zval *return_value, zval *str)
{
	zval copy = {};
	int use_copy = 0;
	char *stripped;
	size_t len;

	if (Z_TYPE_P(str) != IS_STRING) {
		use_copy = zend_make_printable_zval(str, &copy);
		if (use_copy) {
			str = &copy;
		}
	}

	stripped = estrndup(Z_STRVAL_P(str), Z_STRLEN_P(str));
	len = php_strip_tags(stripped, Z_STRLEN_P(str), NULL, NULL, 0);

	ZVAL_STRINGL(return_value, stripped, len);
}

/**
 * Fast call to PHP trim() function
 */
void phalcon_fast_trim(zval *return_value, zval *str, zval *charlist, int where)
{
	zval copy = {};
	int use_copy = 0;
	zend_string *trimmed;

	if (Z_TYPE_P(str) != IS_STRING) {
		use_copy = zend_make_printable_zval(str, &copy);
		if (use_copy) {
			str = &copy;
		}
	}

	if (charlist && Z_TYPE_P(charlist) == IS_STRING) {
		trimmed = php_trim(Z_STR_P(str), Z_STRVAL_P(charlist), Z_STRLEN_P(charlist), where);
	} else {
		trimmed = php_trim(Z_STR_P(str), NULL, 0, where);
	}
	ZVAL_STR(return_value, trimmed);
}

static zend_string *php_str_to_str_ex(zend_string *haystack,
	char *needle, size_t needle_len, char *str, size_t str_len, zend_long *replace_count)
{
	zend_string *new_str;

	if (needle_len < ZSTR_LEN(haystack)) {
		char *end;
		char *e, *s, *p, *r;

		if (needle_len == str_len) {
			new_str = NULL;
			end = ZSTR_VAL(haystack) + ZSTR_LEN(haystack);
			for (p = ZSTR_VAL(haystack); (r = (char*)php_memnstr(p, needle, needle_len, end)); p = r + needle_len) {
				if (!new_str) {
					new_str = zend_string_init(ZSTR_VAL(haystack), ZSTR_LEN(haystack), 0);
				}
				memcpy(ZSTR_VAL(new_str) + (r - ZSTR_VAL(haystack)), str, str_len);
				(*replace_count)++;
			}
			if (!new_str) {
				goto nothing_todo;
			}
			return new_str;
		} else {
			size_t count = 0;
			char *o = ZSTR_VAL(haystack);
			char *n = needle;
			char *endp = o + ZSTR_LEN(haystack);

			while ((o = (char*)php_memnstr(o, n, needle_len, endp))) {
				o += needle_len;
				count++;
			}
			if (count == 0) {
				/* Needle doesn't occur, shortcircuit the actual replacement. */
				goto nothing_todo;
			}
			if (str_len > needle_len) {
				new_str = zend_string_safe_alloc(count, str_len - needle_len, ZSTR_LEN(haystack), 0);
			} else {
				new_str = zend_string_alloc(count * (str_len - needle_len) + ZSTR_LEN(haystack), 0);
			}

			e = s = ZSTR_VAL(new_str);
			end = ZSTR_VAL(haystack) + ZSTR_LEN(haystack);
			for (p = ZSTR_VAL(haystack); (r = (char*)php_memnstr(p, needle, needle_len, end)); p = r + needle_len) {
				memcpy(e, p, r - p);
				e += r - p;
				memcpy(e, str, str_len);
				e += str_len;
				(*replace_count)++;
			}

			if (p < end) {
				memcpy(e, p, end - p);
				e += end - p;
			}

			*e = '\0';
			return new_str;
		}
	} else if (needle_len > ZSTR_LEN(haystack) || memcmp(ZSTR_VAL(haystack), needle, ZSTR_LEN(haystack))) {
nothing_todo:
		return zend_string_copy(haystack);
	} else {
		new_str = zend_string_init(str, str_len, 0);
		(*replace_count)++;
		return new_str;
	}
}

static zend_string *php_str_to_str_i_ex(zend_string *haystack, char *lc_haystack,
	zend_string *needle, char *str, size_t str_len, zend_long *replace_count)
{
	zend_string *new_str = NULL;
	zend_string *lc_needle;

	if (ZSTR_LEN(needle) < ZSTR_LEN(haystack)) {
		char *end;
		char *e, *s, *p, *r;

		if (ZSTR_LEN(needle) == str_len) {
			lc_needle = php_string_tolower(needle);
			end = lc_haystack + ZSTR_LEN(haystack);
			for (p = lc_haystack; (r = (char*)php_memnstr(p, ZSTR_VAL(lc_needle), ZSTR_LEN(lc_needle), end)); p = r + ZSTR_LEN(lc_needle)) {
				if (!new_str) {
					new_str = zend_string_init(ZSTR_VAL(haystack), ZSTR_LEN(haystack), 0);
				}
				memcpy(ZSTR_VAL(new_str) + (r - lc_haystack), str, str_len);
				(*replace_count)++;
			}
			zend_string_release(lc_needle);

			if (!new_str) {
				goto nothing_todo;
			}
			return new_str;
		} else {
			size_t count = 0;
			char *o = lc_haystack;
			char *n;
			char *endp = o + ZSTR_LEN(haystack);

			lc_needle = php_string_tolower(needle);
			n = ZSTR_VAL(lc_needle);

			while ((o = (char*)php_memnstr(o, n, ZSTR_LEN(lc_needle), endp))) {
				o += ZSTR_LEN(lc_needle);
				count++;
			}
			if (count == 0) {
				/* Needle doesn't occur, shortcircuit the actual replacement. */
				zend_string_release(lc_needle);
				goto nothing_todo;
			}

			if (str_len > ZSTR_LEN(lc_needle)) {
				new_str = zend_string_safe_alloc(count, str_len - ZSTR_LEN(lc_needle), ZSTR_LEN(haystack), 0);
			} else {
				new_str = zend_string_alloc(count * (str_len - ZSTR_LEN(lc_needle)) + ZSTR_LEN(haystack), 0);
			}

			e = s = ZSTR_VAL(new_str);
			end = lc_haystack + ZSTR_LEN(haystack);

			for (p = lc_haystack; (r = (char*)php_memnstr(p, ZSTR_VAL(lc_needle), ZSTR_LEN(lc_needle), end)); p = r + ZSTR_LEN(lc_needle)) {
				memcpy(e, ZSTR_VAL(haystack) + (p - lc_haystack), r - p);
				e += r - p;
				memcpy(e, str, str_len);
				e += str_len;
				(*replace_count)++;
			}

			if (p < end) {
				memcpy(e, ZSTR_VAL(haystack) + (p - lc_haystack), end - p);
				e += end - p;
			}
			*e = '\0';

			zend_string_release(lc_needle);

			return new_str;
		}
	} else if (ZSTR_LEN(needle) > ZSTR_LEN(haystack)) {
nothing_todo:
		return zend_string_copy(haystack);
	} else {
		lc_needle = php_string_tolower(needle);

		if (memcmp(lc_haystack, ZSTR_VAL(lc_needle), ZSTR_LEN(lc_needle))) {
			zend_string_release(lc_needle);
			goto nothing_todo;
		}
		zend_string_release(lc_needle);

		new_str = zend_string_init(str, str_len, 0);

		(*replace_count)++;
		return new_str;
	}
}

static zend_string* php_char_to_str_ex(zend_string *str, char from, char *to, size_t to_len, int case_sensitivity, zend_long *replace_count)
{
	zend_string *result;
	size_t char_count = 0;
	char lc_from = 0;
	char *source, *target, *source_end= ZSTR_VAL(str) + ZSTR_LEN(str);

	if (case_sensitivity) {
		char *p = ZSTR_VAL(str), *e = p + ZSTR_LEN(str);
		while ((p = memchr(p, from, (e - p)))) {
			char_count++;
			p++;
		}
	} else {
		lc_from = tolower(from);
		for (source = ZSTR_VAL(str); source < source_end; source++) {
			if (tolower(*source) == lc_from) {
				char_count++;
			}
		}
	}

	if (char_count == 0) {
		return zend_string_copy(str);
	}

	if (to_len > 0) {
		result = zend_string_safe_alloc(char_count, to_len - 1, ZSTR_LEN(str), 0);
	} else {
		result = zend_string_alloc(ZSTR_LEN(str) - char_count, 0);
	}
	target = ZSTR_VAL(result);

	if (case_sensitivity) {
		char *p = ZSTR_VAL(str), *e = p + ZSTR_LEN(str), *s = ZSTR_VAL(str);
		while ((p = memchr(p, from, (e - p)))) {
			memcpy(target, s, (p - s));
			target += p - s;
			memcpy(target, to, to_len);
			target += to_len;
			p++;
			s = p;
			if (replace_count) {
				*replace_count += 1;
			}
		}
		if (s < e) {
			memcpy(target, s, (e - s));
			target += e - s;
		}
	} else {
		for (source = ZSTR_VAL(str); source < source_end; source++) {
			if (tolower(*source) == lc_from) {
				if (replace_count) {
					*replace_count += 1;
				}
				memcpy(target, to, to_len);
				target += to_len;
			} else {
				*target = *source;
				target++;
			}
		}
	}
	*target = 0;
	return result;
}

static zend_long php_str_replace_in_subject(zval *search, zval *replace, zval *subject, zval *result, int case_sensitivity)
{
	zval		*search_entry,
				*replace_entry = NULL;
	zend_string	*tmp_result,
				*replace_entry_str = NULL;
	char		*replace_value = NULL;
	size_t		 replace_len = 0;
	zend_long	 replace_count = 0;
	zend_string	*subject_str;
	zend_string *lc_subject_str = NULL;
	uint32_t     replace_idx;

	/* Make sure we're dealing with strings. */
	subject_str = zval_get_string(subject);
	if (ZSTR_LEN(subject_str) == 0) {
		zend_string_release(subject_str);
		ZVAL_EMPTY_STRING(result);
		return 0;
	}

	/* If search is an array */
	if (Z_TYPE_P(search) == IS_ARRAY) {
		/* Duplicate subject string for repeated replacement */
		ZVAL_STR_COPY(result, subject_str);

		if (Z_TYPE_P(replace) == IS_ARRAY) {
			replace_idx = 0;
		} else {
			/* Set replacement value to the passed one */
			replace_value = Z_STRVAL_P(replace);
			replace_len = Z_STRLEN_P(replace);
		}

		/* For each entry in the search array, get the entry */
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(search), search_entry) {
			/* Make sure we're dealing with strings. */
			zend_string *search_str = zval_get_string(search_entry);
			if (ZSTR_LEN(search_str) == 0) {
				if (Z_TYPE_P(replace) == IS_ARRAY) {
					replace_idx++;
				}
				zend_string_release(search_str);
				continue;
			}

			/* If replace is an array. */
			if (Z_TYPE_P(replace) == IS_ARRAY) {
				/* Get current entry */
				while (replace_idx < Z_ARRVAL_P(replace)->nNumUsed) {
					replace_entry = &Z_ARRVAL_P(replace)->arData[replace_idx].val;
					if (Z_TYPE_P(replace_entry) != IS_UNDEF) {
						break;
					}
					replace_idx++;
				}
				if (replace_idx < Z_ARRVAL_P(replace)->nNumUsed) {
					/* Make sure we're dealing with strings. */
					replace_entry_str = zval_get_string(replace_entry);

					/* Set replacement value to the one we got from array */
					replace_value = ZSTR_VAL(replace_entry_str);
					replace_len = ZSTR_LEN(replace_entry_str);

					replace_idx++;
				} else {
					/* We've run out of replacement strings, so use an empty one. */
					replace_value = "";
					replace_len = 0;
				}
			}

			if (ZSTR_LEN(search_str) == 1) {
				zend_long old_replace_count = replace_count;

				tmp_result = php_char_to_str_ex(Z_STR_P(result),
								ZSTR_VAL(search_str)[0],
								replace_value,
								replace_len,
								case_sensitivity,
								&replace_count);
				if (lc_subject_str && replace_count != old_replace_count) {
					zend_string_release(lc_subject_str);
					lc_subject_str = NULL;
				}
			} else if (ZSTR_LEN(search_str) > 1) {
				if (case_sensitivity) {
					tmp_result = php_str_to_str_ex(Z_STR_P(result),
							ZSTR_VAL(search_str), ZSTR_LEN(search_str),
							replace_value, replace_len, &replace_count);
				} else {
					zend_long old_replace_count = replace_count;

					if (!lc_subject_str) {
						lc_subject_str = php_string_tolower(Z_STR_P(result));
					}
					tmp_result = php_str_to_str_i_ex(Z_STR_P(result), ZSTR_VAL(lc_subject_str),
							search_str, replace_value, replace_len, &replace_count);
					if (replace_count != old_replace_count) {
						zend_string_release(lc_subject_str);
						lc_subject_str = NULL;
					}
				}
			}

			zend_string_release(search_str);

			if (replace_entry_str) {
				zend_string_release(replace_entry_str);
				replace_entry_str = NULL;
			}
			zend_string_release(Z_STR_P(result));
			ZVAL_STR(result, tmp_result);

			if (Z_STRLEN_P(result) == 0) {
				if (lc_subject_str) {
					zend_string_release(lc_subject_str);
				}
				zend_string_release(subject_str);
				return replace_count;
			}
		} ZEND_HASH_FOREACH_END();
		if (lc_subject_str) {
			zend_string_release(lc_subject_str);
		}
	} else {
		ZEND_ASSERT(Z_TYPE_P(search) == IS_STRING);
		if (Z_STRLEN_P(search) == 1) {
			ZVAL_STR(result,
				php_char_to_str_ex(subject_str,
							Z_STRVAL_P(search)[0],
							Z_STRVAL_P(replace),
							Z_STRLEN_P(replace),
							case_sensitivity,
							&replace_count));
		} else if (Z_STRLEN_P(search) > 1) {
			if (case_sensitivity) {
				ZVAL_STR(result, php_str_to_str_ex(subject_str,
						Z_STRVAL_P(search), Z_STRLEN_P(search),
						Z_STRVAL_P(replace), Z_STRLEN_P(replace), &replace_count));
			} else {
				lc_subject_str = php_string_tolower(subject_str);
				ZVAL_STR(result, php_str_to_str_i_ex(subject_str, ZSTR_VAL(lc_subject_str),
						Z_STR_P(search),
						Z_STRVAL_P(replace), Z_STRLEN_P(replace), &replace_count));
				zend_string_release(lc_subject_str);
			}
		} else {
			ZVAL_STR_COPY(result, subject_str);
		}
	}
	zend_string_release(subject_str);
	return replace_count;
}

/**
 * Immediate function resolution for str_replace function
 */
void phalcon_fast_str_replace(zval *return_value, zval *search, zval *replace, zval *subject)
{
	zval *subject_entry;
	zval result;
	zend_string *string_key;
	zend_ulong num_key;
	zend_long count = 0;
	int case_sensitivity = 1;

	/* Make sure we're dealing with strings and do the replacement. */
	if (Z_TYPE_P(search) != IS_ARRAY) {
		convert_to_string_ex(search);
		if (Z_TYPE_P(replace) != IS_STRING) {
			convert_to_string_ex(replace);
		}
	} else if (Z_TYPE_P(replace) != IS_ARRAY) {
		convert_to_string_ex(replace);
	}

	/* if subject is an array */
	if (Z_TYPE_P(subject) == IS_ARRAY) {
		array_init(return_value);

		/* For each subject entry, convert it to string, then perform replacement
		   and add the result to the return_value array. */
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(subject), num_key, string_key, subject_entry) {
			ZVAL_DEREF(subject_entry);
			if (Z_TYPE_P(subject_entry) != IS_ARRAY && Z_TYPE_P(subject_entry) != IS_OBJECT) {
				count += php_str_replace_in_subject(search, replace, subject_entry, &result, case_sensitivity);
			} else {
				ZVAL_COPY(&result, subject_entry);
			}
			/* Add to return array */
			if (string_key) {
				zend_hash_add_new(Z_ARRVAL_P(return_value), string_key, &result);
			} else {
				zend_hash_index_add_new(Z_ARRVAL_P(return_value), num_key, &result);
			}
		} ZEND_HASH_FOREACH_END();
	} else {	/* if subject is not an array */
		count = php_str_replace_in_subject(search, replace, subject, return_value, case_sensitivity);
	}
}

#if PHP_VERSION_ID >= 70300
static zend_string *php_pcre_replace_array(HashTable *regex, zval *replace, zend_string *subject_str, size_t limit, size_t *replace_count)
{
	zval		*regex_entry;
	zend_string *result;
	zend_string *replace_str, *tmp_replace_str;

	if (Z_TYPE_P(replace) == IS_ARRAY) {
		uint32_t replace_idx = 0;
		HashTable *replace_ht = Z_ARRVAL_P(replace);

		/* For each entry in the regex array, get the entry */
		ZEND_HASH_FOREACH_VAL(regex, regex_entry) {
			/* Make sure we're dealing with strings. */
			zend_string *tmp_regex_str;
			zend_string *regex_str = zval_get_tmp_string(regex_entry, &tmp_regex_str);
			zval *zv;

			/* Get current entry */
			while (1) {
				if (replace_idx == replace_ht->nNumUsed) {
					replace_str = ZSTR_EMPTY_ALLOC();
					tmp_replace_str = NULL;
					break;
				}
				zv = &replace_ht->arData[replace_idx].val;
				replace_idx++;
				if (Z_TYPE_P(zv) != IS_UNDEF) {
					replace_str = zval_get_tmp_string(zv, &tmp_replace_str);
					break;
				}
			}

			/* Do the actual replacement and put the result back into subject_str
			   for further replacements. */
			result = php_pcre_replace(regex_str,
									  subject_str,
									  ZSTR_VAL(subject_str),
									  ZSTR_LEN(subject_str),
									  replace_str,
									  limit,
									  replace_count);
			zend_tmp_string_release(tmp_replace_str);
			zend_tmp_string_release(tmp_regex_str);
			zend_string_release_ex(subject_str, 0);
			subject_str = result;
			if (UNEXPECTED(result == NULL)) {
				break;
			}
		} ZEND_HASH_FOREACH_END();

	} else {
		replace_str = Z_STR_P(replace);

		/* For each entry in the regex array, get the entry */
		ZEND_HASH_FOREACH_VAL(regex, regex_entry) {
			/* Make sure we're dealing with strings. */
			zend_string *tmp_regex_str;
			zend_string *regex_str = zval_get_tmp_string(regex_entry, &tmp_regex_str);

			/* Do the actual replacement and put the result back into subject_str
			   for further replacements. */
			result = php_pcre_replace(regex_str,
									  subject_str,
									  ZSTR_VAL(subject_str),
									  ZSTR_LEN(subject_str),
									  replace_str,
									  limit,
									  replace_count);
			zend_tmp_string_release(tmp_regex_str);
			zend_string_release_ex(subject_str, 0);
			subject_str = result;

			if (UNEXPECTED(result == NULL)) {
				break;
			}
		} ZEND_HASH_FOREACH_END();
	}

	return subject_str;
}

static zend_always_inline zend_string *php_replace_in_subject(zval *regex, zval *replace, zval *subject, size_t limit, size_t *replace_count)
{
	zend_string *result;
	zend_string *subject_str = zval_get_string(subject);

	if (Z_TYPE_P(regex) != IS_ARRAY) {
		result = php_pcre_replace(Z_STR_P(regex),
								  subject_str,
								  ZSTR_VAL(subject_str),
								  ZSTR_LEN(subject_str),
								  Z_STR_P(replace),
								  limit,
								  replace_count);
		zend_string_release_ex(subject_str, 0);
	} else {
		result = php_pcre_replace_array(Z_ARRVAL_P(regex),
										replace,
										subject_str,
										limit,
										replace_count);
	}
	return result;
}
#else
# if PHP_VERSION_ID >= 70200
static zend_string *php_pcre_replace_array(HashTable *regex, zval *replace, zend_string *subject_str, int limit, int *replace_count)
{
	zval		*regex_entry;
	zend_string *result;
	zend_string *replace_str;

	if (Z_TYPE_P(replace) == IS_ARRAY) {
		uint32_t replace_idx = 0;
		HashTable *replace_ht = Z_ARRVAL_P(replace);

		/* For each entry in the regex array, get the entry */
		ZEND_HASH_FOREACH_VAL(regex, regex_entry) {
			/* Make sure we're dealing with strings. */
			zend_string *regex_str = zval_get_string(regex_entry);
			zval *zv;

			/* Get current entry */
			while (1) {
				if (replace_idx == replace_ht->nNumUsed) {
					replace_str = ZSTR_EMPTY_ALLOC();
					break;
				}
				zv = &replace_ht->arData[replace_idx].val;
				replace_idx++;
				if (Z_TYPE_P(zv) != IS_UNDEF) {
					replace_str = zval_get_string(zv);
					break;
				}
			}

			/* Do the actual replacement and put the result back into subject_str
			   for further replacements. */
			result = php_pcre_replace(regex_str,
									  subject_str,
									  ZSTR_VAL(subject_str),
									  (int)ZSTR_LEN(subject_str),
									  replace_str,
									  limit,
									  replace_count);
			zend_string_release(replace_str);
			zend_string_release(regex_str);
			zend_string_release(subject_str);
			subject_str = result;
			if (UNEXPECTED(result == NULL)) {
				break;
			}
		} ZEND_HASH_FOREACH_END();

	} else {
		replace_str = Z_STR_P(replace);

		/* For each entry in the regex array, get the entry */
		ZEND_HASH_FOREACH_VAL(regex, regex_entry) {
			/* Make sure we're dealing with strings. */
			zend_string *regex_str = zval_get_string(regex_entry);

			/* Do the actual replacement and put the result back into subject_str
			   for further replacements. */
			result = php_pcre_replace(regex_str,
									  subject_str,
									  ZSTR_VAL(subject_str),
									  (int)ZSTR_LEN(subject_str),
									  replace_str,
									  limit,
									  replace_count);
			zend_string_release(regex_str);
			zend_string_release(subject_str);
			subject_str = result;

			if (UNEXPECTED(result == NULL)) {
				break;
			}
		} ZEND_HASH_FOREACH_END();
	}

	return subject_str;
}

static zend_always_inline zend_string *php_replace_in_subject(zval *regex, zval *replace, zval *subject, int limit, int *replace_count)
{
	zend_string *result;
	zend_string *subject_str = zval_get_string(subject);

	if (UNEXPECTED(ZEND_SIZE_T_INT_OVFL(ZSTR_LEN(subject_str)))) {
		zend_string_release(subject_str);
		php_error_docref(NULL, E_WARNING, "Subject is too long");
		result = NULL;
	} else if (Z_TYPE_P(regex) != IS_ARRAY) {
		result = php_pcre_replace(Z_STR_P(regex),
								  subject_str,
								  ZSTR_VAL(subject_str),
								  (int)ZSTR_LEN(subject_str),
								  Z_STR_P(replace),
								  limit,
								  replace_count);
		zend_string_release(subject_str);
	} else {
		result = php_pcre_replace_array(Z_ARRVAL_P(regex),
										replace,
										subject_str,
										limit,
										replace_count);
	}
	return result;
}
# else
static zend_string *php_replace_in_subject(zval *regex, zval *replace, zval *subject, int limit, int is_callable_replace, int *replace_count)
{
	zval		*regex_entry,
				*replace_value,
				 empty_replace;
	zend_string *result;
	uint32_t replace_idx;
	zend_string	*subject_str = zval_get_string(subject);

	/* FIXME: This might need to be changed to ZSTR_EMPTY_ALLOC(). Check if this zval could be dtor()'ed somehow */
	ZVAL_EMPTY_STRING(&empty_replace);

	if (ZEND_SIZE_T_INT_OVFL(ZSTR_LEN(subject_str))) {
			php_error_docref(NULL, E_WARNING, "Subject is too long");
			return NULL;
	}

	/* If regex is an array */
	if (Z_TYPE_P(regex) == IS_ARRAY) {
		replace_value = replace;
		replace_idx = 0;

		/* For each entry in the regex array, get the entry */
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(regex), regex_entry) {
			zval replace_str;
			/* Make sure we're dealing with strings. */
			zend_string *regex_str = zval_get_string(regex_entry);

			ZVAL_UNDEF(&replace_str);
			/* If replace is an array and not a callable construct */
			if (Z_TYPE_P(replace) == IS_ARRAY && !is_callable_replace) {
				/* Get current entry */
				while (replace_idx < Z_ARRVAL_P(replace)->nNumUsed) {
					if (Z_TYPE(Z_ARRVAL_P(replace)->arData[replace_idx].val) != IS_UNDEF) {
						ZVAL_COPY(&replace_str, &Z_ARRVAL_P(replace)->arData[replace_idx].val);
						break;
					}
					replace_idx++;
				}
				if (!Z_ISUNDEF(replace_str)) {
					if (!is_callable_replace) {
						convert_to_string(&replace_str);
					}
					replace_value = &replace_str;
					replace_idx++;
				} else {
					/* We've run out of replacement strings, so use an empty one */
					replace_value = &empty_replace;
				}
			}

			/* Do the actual replacement and put the result back into subject_str
			   for further replacements. */
			if ((result = php_pcre_replace(regex_str,
										   subject_str,
										   ZSTR_VAL(subject_str),
										   (int)ZSTR_LEN(subject_str),
										   replace_value,
										   is_callable_replace,
										   limit,
										   replace_count)) != NULL) {
				zend_string_release(subject_str);
				subject_str = result;
			} else {
				zend_string_release(subject_str);
				zend_string_release(regex_str);
				zval_dtor(&replace_str);
				return NULL;
			}

			zend_string_release(regex_str);
			zval_dtor(&replace_str);
		} ZEND_HASH_FOREACH_END();

		return subject_str;
	} else {
		result = php_pcre_replace(Z_STR_P(regex),
								  subject_str,
								  ZSTR_VAL(subject_str),
								  (int)ZSTR_LEN(subject_str),
								  replace,
								  is_callable_replace,
								  limit,
								  replace_count);
		zend_string_release(subject_str);
		return result;
	}
}

static int preg_replace_impl(zval *return_value, zval *regex, zval *replace, zval *subject, zend_long limit_val, int is_callable_replace, int is_filter)
{
	zval		*subject_entry;
	zend_string	*result;
	zend_string	*string_key;
	zend_ulong	 num_key;
	int			 replace_count = 0, old_replace_count;

	if (Z_TYPE_P(replace) != IS_ARRAY && (Z_TYPE_P(replace) != IS_OBJECT || !is_callable_replace)) {
		convert_to_string_ex(replace);
	}

	if (Z_TYPE_P(regex) != IS_ARRAY) {
		convert_to_string_ex(regex);
	}

	/* if subject is an array */
	if (Z_TYPE_P(subject) == IS_ARRAY) {
		array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL_P(subject)));

		/* For each subject entry, convert it to string, then perform replacement
		   and add the result to the return_value array. */
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(subject), num_key, string_key, subject_entry) {
			old_replace_count = replace_count;
			if ((result = php_replace_in_subject(regex, replace, subject_entry, limit_val, is_callable_replace, &replace_count)) != NULL) {
				if (!is_filter || replace_count > old_replace_count) {
					/* Add to return array */
					zval zv;

					ZVAL_STR(&zv, result);
					if (string_key) {
						zend_hash_add_new(Z_ARRVAL_P(return_value), string_key, &zv);
					} else {
						zend_hash_index_add_new(Z_ARRVAL_P(return_value), num_key, &zv);
					}
				} else {
					zend_string_release(result);
				}
			}
		} ZEND_HASH_FOREACH_END();
	} else {
		/* if subject is not an array */
		old_replace_count = replace_count;
		if ((result = php_replace_in_subject(regex, replace, subject, limit_val, is_callable_replace, &replace_count)) != NULL) {
			if (!is_filter || replace_count > old_replace_count) {
				RETVAL_STR(result);
			} else {
				zend_string_release(result);
				RETVAL_NULL();
			}
		} else {
			RETVAL_NULL();
		}
	}

	return replace_count;
}
# endif
#endif

/**
 * Immediate function resolution for preg_replace function
 */
void phalcon_fast_preg_replace(zval *return_value, zval *regex, zval *replace, zval *subject)
{
#if PHP_VERSION_ID >= 70300
	zval *zcount = NULL;
	zend_long limit = -1;
	size_t replace_count = 0;
	zend_string	*result;
	size_t old_replace_count;
	int is_filter = 0;

	if (Z_TYPE_P(replace) != IS_ARRAY) {
		convert_to_string_ex(replace);
		if (Z_TYPE_P(regex) != IS_ARRAY) {
			convert_to_string_ex(regex);
		}
	} else {
		if (Z_TYPE_P(regex) != IS_ARRAY) {
			php_error_docref(NULL, E_WARNING, "Parameter mismatch, pattern is a string while replacement is an array");
			RETURN_FALSE;
		}
	}

	if (Z_TYPE_P(subject) != IS_ARRAY) {
		old_replace_count = replace_count;
		result = php_replace_in_subject(regex,
										replace,
										subject,
										limit,
										&replace_count);
		if (result != NULL) {
			if (!is_filter || replace_count > old_replace_count) {
				RETVAL_STR(result);
			} else {
				zend_string_release_ex(result, 0);
				RETVAL_NULL();
			}
		} else {
			RETVAL_NULL();
		}
	} else {
		/* if subject is an array */
		zval		*subject_entry, zv;
		zend_string	*string_key;
		zend_ulong	 num_key;

		array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL_P(subject)));

		/* For each subject entry, convert it to string, then perform replacement
		   and add the result to the return_value array. */
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(subject), num_key, string_key, subject_entry) {
			old_replace_count = replace_count;
			result = php_replace_in_subject(regex,
											replace,
											subject_entry,
											limit,
											&replace_count);
			if (result != NULL) {
				if (!is_filter || replace_count > old_replace_count) {
					/* Add to return array */
					ZVAL_STR(&zv, result);
					if (string_key) {
						zend_hash_add_new(Z_ARRVAL_P(return_value), string_key, &zv);
					} else {
						zend_hash_index_add_new(Z_ARRVAL_P(return_value), num_key, &zv);
					}
				} else {
					zend_string_release_ex(result, 0);
				}
			}
		} ZEND_HASH_FOREACH_END();
	}

	if (zcount) {
		zval_ptr_dtor(zcount);
		ZVAL_LONG(zcount, replace_count);
	}
#else
# if PHP_VERSION_ID >= 70200
	zend_long limit = -1;
	int replace_count = 0;
	zend_string	*result;
	int old_replace_count;
	int is_filter = 0;
	if (Z_TYPE_P(replace) != IS_ARRAY) {
		convert_to_string_ex(replace);
		if (Z_TYPE_P(regex) != IS_ARRAY) {
			convert_to_string_ex(regex);
		}
	} else {
		if (Z_TYPE_P(regex) != IS_ARRAY) {
			php_error_docref(NULL, E_WARNING, "Parameter mismatch, pattern is a string while replacement is an array");
			RETURN_FALSE;
		}
	}

	if (Z_TYPE_P(subject) != IS_ARRAY) {
		old_replace_count = replace_count;
		result = php_replace_in_subject(regex,
										replace,
										subject,
										limit,
										&replace_count);
		if (result != NULL) {
			if (!is_filter || replace_count > old_replace_count) {
				RETVAL_STR(result);
			} else {
				zend_string_release(result);
				RETVAL_NULL();
			}
		} else {
			RETVAL_NULL();
		}
	} else {
		/* if subject is an array */
		zval		*subject_entry, zv;
		zend_string	*string_key;
		zend_ulong	 num_key;

		array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL_P(subject)));

		/* For each subject entry, convert it to string, then perform replacement
		   and add the result to the return_value array. */
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(subject), num_key, string_key, subject_entry) {
			old_replace_count = replace_count;
			result = php_replace_in_subject(regex,
											replace,
											subject_entry,
											limit,
											&replace_count);
			if (result != NULL) {
				if (!is_filter || replace_count > old_replace_count) {
					/* Add to return array */
					ZVAL_STR(&zv, result);
					if (string_key) {
						zend_hash_add_new(Z_ARRVAL_P(return_value), string_key, &zv);
					} else {
						zend_hash_index_add_new(Z_ARRVAL_P(return_value), num_key, &zv);
					}
				} else {
					zend_string_release(result);
				}
			}
		} ZEND_HASH_FOREACH_END();
	}
# else
	zval *zcount = NULL;
	zend_long limit = -1;
	int replace_count;

	if (Z_TYPE_P(replace) == IS_ARRAY && Z_TYPE_P(regex) != IS_ARRAY) {
		php_error_docref(NULL, E_WARNING, "Parameter mismatch, pattern is a string while replacement is an array");
		RETURN_FALSE;
		return;
	}

	replace_count = preg_replace_impl(return_value, regex, replace, subject, limit, 0, 0);
	if (zcount) {
		zval_ptr_dtor(zcount);
		ZVAL_LONG(zcount, replace_count);
	}
# endif
#endif
}

void phalcon_pad_str(zval *return_value, zval *input, int pad_length, const char *pad_str, int pad_type)
{
	/* Helper variables */
	size_t num_pad_chars;
	size_t pad_str_len = strlen(pad_str);
	size_t i, left_pad=0, right_pad=0;
	zend_string *result = NULL;

	if (pad_length < 0  || (size_t)pad_length <= Z_STRLEN_P(input)) {
		RETURN_CTOR(input);
	}

	if (pad_str_len == 0) {
		php_error_docref(NULL, E_WARNING, "Padding string cannot be empty");
		return;
	}

	if (pad_type < PHALCON_PDA_LEFT || pad_type > PHALCON_PDA_BOTH) {
		php_error_docref(NULL, E_WARNING, "Padding type has to be STR_PAD_LEFT, STR_PAD_RIGHT, or STR_PAD_BOTH");
		return;
	}

	num_pad_chars = pad_length - Z_STRLEN_P(input);

	result = zend_string_safe_alloc(1, Z_STRLEN_P(input), num_pad_chars, 0);
	ZSTR_LEN(result) = 0;

	/* We need to figure out the left/right padding lengths. */
	switch (pad_type) {
		case PHALCON_PDA_RIGHT:
			left_pad = 0;
			right_pad = num_pad_chars;
			break;

		case PHALCON_PDA_LEFT:
			left_pad = num_pad_chars;
			right_pad = 0;
			break;

		case PHALCON_PDA_BOTH:
			left_pad = num_pad_chars / 2;
			right_pad = num_pad_chars - left_pad;
			break;
	}

	/* First we pad on the left. */
	for (i = 0; i < left_pad; i++)
		ZSTR_VAL(result)[ZSTR_LEN(result)++] = pad_str[i % pad_str_len];

	/* Then we copy the input string. */
	memcpy(ZSTR_VAL(result) + ZSTR_LEN(result), Z_STRVAL_P(input), Z_STRLEN_P(input));
	ZSTR_LEN(result) += Z_STRLEN_P(input);

	/* Finally, we pad on the right. */
	for (i = 0; i < right_pad; i++)
		ZSTR_VAL(result)[ZSTR_LEN(result)++] = pad_str[i % pad_str_len];

	ZSTR_VAL(result)[ZSTR_LEN(result)] = '\0';

	RETURN_NEW_STR(result);
}

/**
 * Checks if a zval string starts with a zval string
 */
int phalcon_start_with(const zval *str, const zval *compared, zval *case_sensitive){

	int sensitive = 0;
	int i;
	char *op1_cursor, *op2_cursor;

	if (Z_TYPE_P(str) != IS_STRING || Z_TYPE_P(compared) != IS_STRING) {
		return 0;
	}

	if (!Z_STRLEN_P(compared) || !Z_STRLEN_P(str) || Z_STRLEN_P(compared) > Z_STRLEN_P(str)) {
		return 0;
	}

	if (case_sensitive) {
		sensitive = zend_is_true(case_sensitive);
	}

	if (!sensitive) {
		return !memcmp(Z_STRVAL_P(str), Z_STRVAL_P(compared), Z_STRLEN_P(compared));
	}

	op1_cursor = Z_STRVAL_P(str);
	op2_cursor = Z_STRVAL_P(compared);
	for (i = 0; i < Z_STRLEN_P(compared); i++) {
		if (tolower(*op1_cursor) != tolower(*op2_cursor)) {
			return 0;
		}

		op1_cursor++;
		op2_cursor++;
	}

	return 1;
}

/**
 * Checks if a zval string starts with a string
 */
int phalcon_start_with_str(const zval *str, char *compared, unsigned int compared_length){

	if (Z_TYPE_P(str) != IS_STRING || compared_length > (uint)(Z_STRLEN_P(str))) {
		return 0;
	}

	return !memcmp(Z_STRVAL_P(str), compared, compared_length);
}

/**
 * Checks if a string starts with other string
 */
int phalcon_start_with_str_str(char *str, unsigned int str_length, char *compared, unsigned int compared_length){

	if (compared_length > str_length) {
		return 0;
	}

	return !memcmp(str, compared, compared_length);
}

/**
 * Checks if a zval string ends with a zval string
 */
int phalcon_end_with(const zval *str, const zval *compared, zval *case_sensitive){

	int sensitive = 0;
	int i;
	char *op1_cursor, *op2_cursor;

	if (Z_TYPE_P(str) != IS_STRING || Z_TYPE_P(compared) != IS_STRING) {
		return 0;
	}

	if (!Z_STRLEN_P(compared) || !Z_STRLEN_P(str) || Z_STRLEN_P(compared) > Z_STRLEN_P(str)) {
		return 0;
	}

	if (case_sensitive) {
		sensitive = zend_is_true(case_sensitive);
	}

	if (!sensitive) {
		return !memcmp(Z_STRVAL_P(str) + Z_STRLEN_P(str) - Z_STRLEN_P(compared), Z_STRVAL_P(compared), Z_STRLEN_P(compared));
	}

	op1_cursor = Z_STRVAL_P(str) + Z_STRLEN_P(str) - Z_STRLEN_P(compared);
	op2_cursor = Z_STRVAL_P(compared);

	for (i = 0; i < Z_STRLEN_P(compared); ++i) {
		if (tolower(*op1_cursor) != tolower(*op2_cursor)) {
			return 0;
		}

		++op1_cursor;
		++op2_cursor;
	}

	return 1;
}

/**
 * Checks if a zval string ends with a *char string
 */
int phalcon_end_with_str(const zval *str, char *compared, unsigned int compared_length){

	if (Z_TYPE_P(str) != IS_STRING) {
		return 0;
	}

	if (!compared_length || !Z_STRLEN_P(str) || compared_length > (uint)(Z_STRLEN_P(str))) {
		return 0;
	}

	return !memcmp(Z_STRVAL_P(str) + Z_STRLEN_P(str) - compared_length, compared, compared_length);
}

/**
 * Checks if a zval string equal with other string
 */
int phalcon_comparestr(const zval *str, const zval *compared, zval *case_sensitive){

	if (Z_TYPE_P(str) != IS_STRING || Z_TYPE_P(compared) != IS_STRING) {
		return 0;
	}

	if (!Z_STRLEN_P(compared) || !Z_STRLEN_P(str) || Z_STRLEN_P(compared) != Z_STRLEN_P(str)) {
		return 0;
	}

	if (Z_STRVAL_P(str) == Z_STRVAL_P(compared)) {
		return 1;
	}

	if (!case_sensitive || !zend_is_true(case_sensitive)) {
		return !strcmp(Z_STRVAL_P(str), Z_STRVAL_P(compared));
	}

	return !strcasecmp(Z_STRVAL_P(str), Z_STRVAL_P(compared));
}

/**
 * Checks if a zval string equal with a zval string
 */
int phalcon_comparestr_str(const zval *str, char *compared, unsigned int compared_length, zval *case_sensitive){

	if (Z_TYPE_P(str) != IS_STRING) {
		return 0;
	}

	if (!compared_length || !Z_STRLEN_P(str) || compared_length != (uint)(Z_STRLEN_P(str))) {
		return 0;
	}

	if (!case_sensitive || !zend_is_true(case_sensitive)) {
		return !strcmp(Z_STRVAL_P(str), compared);
	}

	return !strcasecmp(Z_STRVAL_P(str), compared);
}

/**
 * Random string
 */
void phalcon_random_string(zval *return_value, const zval *type, const zval *length) {

	long i, rand_type, ch;
	smart_str random_str = {0};

	if (Z_TYPE_P(type) != IS_LONG) {
		return;
	}

	if (Z_LVAL_P(type) > PHALCON_RANDOM_NOZERO) {
		return;
	}

	if (Z_TYPE_P(length) != IS_LONG) {
		return;
	}

	/** Generate seed */
	if (!BG(mt_rand_is_seeded)) {
		php_mt_srand(GENERATE_SEED());
	}

	for (i = 0; i < Z_LVAL_P(length); i++) {
#if PHP_VERSION_ID >= 70200
		switch (Z_LVAL_P(type)) {
			case PHALCON_RANDOM_ALNUM:
				rand_type = php_mt_rand_range(0, 3);
				break;
			case PHALCON_RANDOM_ALPHA:
				rand_type = php_mt_rand_range(1, 2);
				break;
			case PHALCON_RANDOM_HEXDEC:
				rand_type = php_mt_rand_range(0, 1);
				break;
			case PHALCON_RANDOM_NUMERIC:
				rand_type = 0;
				break;
			case PHALCON_RANDOM_NOZERO:
				rand_type = 5;
				break;
			default:
				continue;
		}

		switch (rand_type) {
			case 0:
				ch = php_mt_rand_range('0', '9');
				break;
			case 1:
				ch = php_mt_rand_range('a', 'f');
				break;
			case 2:
				ch = php_mt_rand_range('a', 'z');
				break;
			case 3:
				ch = php_mt_rand_range('A', 'Z');
				break;
			case 5:
				ch = php_mt_rand_range('1', '9');
				break;
			default:
				continue;
		}
#else
		switch (Z_LVAL_P(type)) {
			case PHALCON_RANDOM_ALNUM:
				rand_type = (long) (php_mt_rand() >> 1);
				RAND_RANGE(rand_type, 0, 3, PHP_MT_RAND_MAX);
				break;
			case PHALCON_RANDOM_ALPHA:
				rand_type = (long) (php_mt_rand() >> 1);
				RAND_RANGE(rand_type, 1, 2, PHP_MT_RAND_MAX);
				break;
			case PHALCON_RANDOM_HEXDEC:
				rand_type = (long) (php_mt_rand() >> 1);
				RAND_RANGE(rand_type, 0, 1, PHP_MT_RAND_MAX);
				break;
			case PHALCON_RANDOM_NUMERIC:
				rand_type = 0;
				break;
			case PHALCON_RANDOM_NOZERO:
				rand_type = 5;
				break;
			default:
				continue;
		}

		switch (rand_type) {
			case 0:
				ch = (long) (php_mt_rand() >> 1);
				RAND_RANGE(ch, '0', '9', PHP_MT_RAND_MAX);
				break;
			case 1:
				ch = (long) (php_mt_rand() >> 1);
				RAND_RANGE(ch, 'a', 'f', PHP_MT_RAND_MAX);
				break;
			case 2:
				ch = (long) (php_mt_rand() >> 1);
				RAND_RANGE(ch, 'a', 'z', PHP_MT_RAND_MAX);
				break;
			case 3:
				ch = (long) (php_mt_rand() >> 1);
				RAND_RANGE(ch, 'A', 'Z', PHP_MT_RAND_MAX);
				break;
			case 5:
				ch = (long) (php_mt_rand() >> 1);
				RAND_RANGE(ch, '1', '9', PHP_MT_RAND_MAX);
				break;
			default:
				continue;
		}
#endif
		smart_str_appendc(&random_str, (unsigned int) ch);
	}


	smart_str_0(&random_str);

	if (random_str.s) {
		RETURN_NEW_STR(random_str.s);
	} else {
		smart_str_free(&random_str);
		RETURN_EMPTY_STRING();
	}

}

/**
 * Removes slashes at the end of a string
 */
void phalcon_remove_extra_slashes(zval *return_value, const zval *str) {

	char *cursor, *removed_str;
	unsigned int i;

	if (Z_TYPE_P(str) != IS_STRING) {
		RETURN_EMPTY_STRING();
	}

	if (Z_STRLEN_P(str) > 1) {
		cursor = Z_STRVAL_P(str);
		cursor += (Z_STRLEN_P(str) - 1);
		for (i = Z_STRLEN_P(str); i > 0; i--) {
			if ((*cursor) == '/') {
				cursor--;
				continue;
			}
			break;
		}
	} else {
		i = Z_STRLEN_P(str);
	}

	if (i <= Z_STRLEN_P(str)) {
		removed_str = emalloc(i + 1);
		memcpy(removed_str, Z_STRVAL_P(str), i);
		removed_str[i] = '\0';
		RETVAL_STRINGL(removed_str, i);
		efree(removed_str);
		return;
	}

    RETURN_EMPTY_STRING();
}

/**
 * This function is not external in the Zend API so we redeclare it here in the extension
 */
int phalcon_spprintf(char **message, int max_len, char *format, ...)
{
    va_list arg;
    int len;

    va_start(arg, format);
    len = vspprintf(message, max_len, format, arg);
    va_end(arg);
    return len;
}

/**
 * Makes a substr like the PHP function. This function doesn't support negative lengths
 */
void phalcon_substr(zval *return_value, zval *str, unsigned long from, long length) {
	uint str_len;

	if (Z_TYPE_P(str) != IS_STRING) {

		if (Z_TYPE_P(str) == IS_NULL || PHALCON_IS_BOOL(str)) {
			ZVAL_FALSE(return_value);
			return;
		}

		if (Z_TYPE_P(str) == IS_LONG) {
			ZVAL_NULL(return_value);
			return;
		}

		zend_error(E_WARNING, "Invalid arguments supplied for phalcon_substr()");
		ZVAL_FALSE(return_value);
		return;
	}

	str_len = (uint)(Z_STRLEN_P(str));
	if (str_len < from){
		ZVAL_FALSE(return_value);
		return;
	}

	if (length < 0) {
		length = str_len - from + length;
	} else if (!length || (str_len < length + from)) {
		length = str_len - from;
	}

	if (length){
		ZVAL_STRINGL(return_value, Z_STRVAL_P(str) + from, length);
	} else {
		ZVAL_NULL(return_value);
	}
}

void phalcon_substr_string(zval *return_value, zend_string *str, unsigned long from, long length) {

	uint str_len = (uint)(ZSTR_LEN(str));

	if (str_len < from){
		ZVAL_FALSE(return_value);
		return;
	}

	if (length < 0) {
		length = str_len - from + length;
	} else if (!length || (str_len < length + from)) {
		length = str_len - from;
	}

	if (length){
		ZVAL_STRINGL(return_value, ZSTR_VAL(str) + from, length);
	} else {
		ZVAL_NULL(return_value);
	}
}

void phalcon_append_printable_array(smart_str *implstr, zval *value) {

	zval *tmp;
	unsigned int numelems, i = 0, str_len;

	numelems = zend_hash_num_elements(Z_ARRVAL_P(value));

	smart_str_appendc(implstr, '[');

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(value), tmp) {
		/**
		 * We don't serialize objects
		 */
		if (Z_TYPE_P(tmp) == IS_OBJECT) {
			smart_str_appendc(implstr, 'O');
			{
				char stmp[MAX_LENGTH_OF_LONG + 1];
				str_len = slprintf(stmp, sizeof(stmp), "%ld", (long)Z_OBJ_HANDLE_P(tmp));
				smart_str_appendl(implstr, stmp, str_len);
			}
		} else {
			if (Z_TYPE_P(tmp) == IS_ARRAY) {
				phalcon_append_printable_array(implstr, tmp);
			} else {
				phalcon_append_printable_zval(implstr, tmp);
			}
		}

		if (++i != numelems) {
			smart_str_appendc(implstr, ',');
		}
	} ZEND_HASH_FOREACH_END();

	smart_str_appendc(implstr, ']');
}

/**
 * Creates a unique key to be used as index in a hash
 */
void phalcon_unique_key(zval *return_value, zval *prefix, zval *value) {

	smart_str implstr = {0};

	if (Z_TYPE_P(prefix) == IS_STRING) {
		smart_str_appendl(&implstr, Z_STRVAL_P(prefix), Z_STRLEN_P(prefix));
	}

	if (Z_TYPE_P(value) == IS_ARRAY) {
		phalcon_append_printable_array(&implstr, value);
	} else {
		phalcon_append_printable_zval(&implstr, value);
	}

	smart_str_0(&implstr);

	if (implstr.s) {
		RETURN_NEW_STR(implstr.s);
	} else {
		smart_str_free(&implstr);
		RETURN_NULL();
	}
}

/**
 * Base 64 encode
 */
void phalcon_base64_encode(zval *return_value, zval *data)
{
	zval copy = {};
	zend_string *encoded;
	int use_copy = 0;

	if (Z_TYPE_P(data) != IS_STRING) {
		use_copy = zend_make_printable_zval(data, &copy);
		if (use_copy) {
			data = &copy;
		}
	}

	encoded = php_base64_encode((unsigned char *)(Z_STRVAL_P(data)), Z_STRLEN_P(data));

	if (use_copy) {
		zval_ptr_dtor(data);
	}

	if (encoded) {
		RETURN_NEW_STR(encoded);
	} else {
		RETURN_NULL();
	}
}

/**
 * Base 64 decode
 */
void phalcon_base64_decode(zval *return_value, zval *data)
{
	zval copy = {};
	zend_string *decoded;
	int use_copy = 0;

	if (Z_TYPE_P(data) != IS_STRING) {
		use_copy = zend_make_printable_zval(data, &copy);
		if (use_copy) {
			data = &copy;
		}
	}

	decoded = php_base64_decode((unsigned char *)(Z_STRVAL_P(data)), Z_STRLEN_P(data));

	if (use_copy) {
		zval_ptr_dtor(data);
	}

	if (decoded) {
		RETURN_NEW_STR(decoded);
	} else {
		RETURN_NULL();
	}
}

void phalcon_md5(zval *return_value, zval *str)
{
	zval copy = {};
	PHP_MD5_CTX ctx;
	unsigned char digest[16];
	char hexdigest[33];
	int use_copy = 0;

	if (Z_TYPE_P(str) != IS_STRING) {
		use_copy = zend_make_printable_zval(str, &copy);
		if (use_copy) {
			str = &copy;
		}
	}

	PHP_MD5Init(&ctx);
	PHP_MD5Update(&ctx, Z_STRVAL_P(str), Z_STRLEN_P(str));
	PHP_MD5Final(digest, &ctx);

	make_digest(hexdigest, digest);

	ZVAL_STRINGL(return_value, hexdigest, 32);
}

void phalcon_crc32(zval *return_value, zval *str)
{
	zval copy = {};
	int use_copy = 0;
	size_t nr;
	char *p;
	php_uint32 crc;
	php_uint32 crcinit = 0;

	if (Z_TYPE_P(str) != IS_STRING) {
		use_copy = zend_make_printable_zval(str, &copy);
		if (use_copy) {
			str = &copy;
		}
	}

	p = Z_STRVAL_P(str);
	nr = Z_STRLEN_P(str);

	crc = crcinit^0xFFFFFFFF;
	for (; nr--; ++p) {
		crc = ((crc >> 8) & 0x00FFFFFF) ^ crc32tab[(crc ^ (*p)) & 0xFF];
	}

	if (use_copy) {
		zval_ptr_dtor(str);
	}

	RETVAL_LONG(crc ^ 0xFFFFFFFF);
}

int phalcon_preg_match(zval *retval, zval *regex, zval *subject, zval *matches, zend_long flags, int global)
{
	pcre_cache_entry *pce;
	zend_long start_offset = 0;
	int use_flags = flags ? 1 : 0;

	if (Z_TYPE_P(subject) != IS_STRING) {
		convert_to_string_ex(subject);
	}

	ZVAL_FALSE(retval);
	if (ZEND_SIZE_T_INT_OVFL(Z_STRLEN_P(subject))) {
		php_error_docref(NULL, E_WARNING, "Subject is too long");
		return FAILURE;
	}

	/* Compile regex or get it from cache. */
	if ((pce = pcre_get_compiled_regex_cache(Z_STR_P(regex))) == NULL) {
		return FAILURE;
	}

	//pce->refcount++;
	php_pcre_match_impl(pce, Z_STRVAL_P(subject), Z_STRLEN_P(subject), retval, matches, global, use_flags, flags, start_offset);
	//pce->refcount--;
	return SUCCESS;
}

int phalcon_json_encode(zval *retval, zval *v, int opts)
{
	int flag;

#ifdef PHALCON_USE_PHP_JSON
	smart_str buf = {0};
# if PHP_VERSION_ID >= 70100
	flag = php_json_encode(&buf, v, opts);
# else
	JSON_G(error_code) = PHP_JSON_ERROR_NONE;
	php_json_encode(&buf, v, opts);
	if (JSON_G(error_code) != PHP_JSON_ERROR_NONE && !(opts & PHP_JSON_PARTIAL_OUTPUT_ON_ERROR)) {
		flag = FAILURE;
	} else {
		flag = SUCCESS;
	}
# endif
	if (flag == SUCCESS) {
		smart_str_0(&buf); /* copy? */
		if (buf.s) {
			ZVAL_NEW_STR(retval, buf.s);
			return flag;
		}
		ZVAL_EMPTY_STRING(retval);
		return flag;
	} else {
		smart_str_free(&buf);
		ZVAL_FALSE(retval);
	}
#else
	zval zopts = {};
	ZVAL_LONG(&zopts, opts);
	PHALCON_CALL_FUNCTION_FLAG(flag, retval, "json_encode", v, &zopts);
#endif
	return flag;
}

int phalcon_json_decode(zval *retval, zval *v, zend_bool assoc)
{
	if (Z_TYPE_P(v) == IS_NULL) {
		ZVAL_NULL(retval);
		return SUCCESS;
	}
#ifdef PHALCON_USE_PHP_JSON
	if (Z_TYPE_P(v) != IS_STRING) {
		convert_to_string_ex(v);
	}
# if PHP_VERSION_ID >= 70100
	return php_json_decode(retval, Z_STRVAL_P(v), Z_STRLEN_P(v), assoc, PHP_JSON_PARSER_DEFAULT_DEPTH);
# else
	JSON_G(error_code) = PHP_JSON_ERROR_NONE;

	if (!Z_STRLEN_P(v)) {
		JSON_G(error_code) = PHP_JSON_ERROR_SYNTAX;
		ZVAL_NULL(retval);
		return FAILURE;
	}
	php_json_decode(retval, Z_STRVAL_P(v), Z_STRLEN_P(v), assoc, PHP_JSON_PARSER_DEFAULT_DEPTH);
	return SUCCESS;
# endif
#else
	zval *zassoc = assoc ? &PHALCON_GLOBAL(z_true) : &PHALCON_GLOBAL(z_false);
	zval *params[] = { v, zassoc };

	return phalcon_call_function_with_params(retval, SL("json_decode"), 2, params);
#endif
}

void phalcon_lcfirst(zval *return_value, zval *s)
{
	char *c;
	int use_copy = 0;

	if (unlikely(Z_TYPE_P(s) != IS_STRING)) {
		use_copy = zend_make_printable_zval(s, return_value);
		if (!use_copy) {
			ZVAL_DUP(return_value, s);
		}
	} else {
		// 可被复制的（IS_TYPE_COPYABLE）
		ZVAL_STRINGL(return_value, Z_STRVAL_P(s), Z_STRLEN_P(s));
	}

	if (Z_STRLEN_P(s)) {
		c = Z_STRVAL_P(return_value);
		*c = tolower((unsigned char)*c);
	}
}

void phalcon_ucfirst(zval *return_value, zval *s)
{
	zval copy = {};
	char *c;
	int use_copy = 0;

	if (unlikely(Z_TYPE_P(s) != IS_STRING)) {
		use_copy = zend_make_printable_zval(s, &copy);
		if (use_copy) {
			s = &copy;
		}
	}

	if (!Z_STRLEN_P(s)) {
		ZVAL_EMPTY_STRING(return_value);
	}
	else {
		ZVAL_DUP(return_value, s);
		c = Z_STRVAL_P(return_value);
		*c = toupper((unsigned char)*c);
	}

	if (unlikely(use_copy)) {
		zval_ptr_dtor(&copy);
	}
}

int phalcon_http_build_query(zval *return_value, zval *params, char *sep)
{
	smart_str formstr = { 0 };
	int res;
	
	if (Z_TYPE_P(params) != IS_ARRAY && Z_TYPE_P(params) != IS_OBJECT) {
		ZVAL_FALSE(return_value);
		return FAILURE;
	}

	res = php_url_encode_hash_ex(HASH_OF(params), &formstr, NULL, 0, NULL, 0, NULL, 0, (Z_TYPE_P(params) == IS_OBJECT ? params : NULL), sep, PHP_QUERY_RFC1738);

	if (res == FAILURE) {
		smart_str_free(&formstr);
		ZVAL_FALSE(return_value);
		return FAILURE;
	}
	if (!formstr.s) {
		ZVAL_EMPTY_STRING(return_value);
	} else {
		smart_str_0(&formstr);
		ZVAL_NEW_STR(return_value, formstr.s);
	}
	return SUCCESS;
}

void phalcon_htmlspecialchars(zval *return_value, zval *string, zval *quoting, zval *charset)
{
	zval copy = {};
	zend_string *escaped;
	char *cs;
	int qs, use_copy = 0;

	if (unlikely(Z_TYPE_P(string) != IS_STRING)) {
		use_copy = zend_make_printable_zval(string, &copy);
		if (use_copy) {
			string = &copy;
		}
	}

	cs = (charset && Z_TYPE_P(charset) == IS_STRING) ? Z_STRVAL_P(charset) : NULL;
	qs = (quoting && Z_TYPE_P(quoting) == IS_LONG)   ? Z_LVAL_P(quoting)   : ENT_COMPAT;

	escaped = php_escape_html_entities_ex((unsigned char *)(Z_STRVAL_P(string)), Z_STRLEN_P(string), 0, qs, cs, 1);
	ZVAL_STR(return_value, escaped);

	if (unlikely(use_copy)) {
		zval_ptr_dtor(&copy);
	}
}

void phalcon_htmlentities(zval *return_value, zval *string, zval *quoting, zval *charset)
{
	zval copy = {};
	zend_string *escaped;
	char *cs;
	int qs, use_copy = 0;

	if (unlikely(Z_TYPE_P(string) != IS_STRING)) {
		use_copy = zend_make_printable_zval(string, &copy);
		if (use_copy) {
			string = &copy;
		}
	}

	cs = (charset && Z_TYPE_P(charset) == IS_STRING) ? Z_STRVAL_P(charset) : NULL;
	qs = (quoting && Z_TYPE_P(quoting) == IS_LONG)   ? Z_LVAL_P(quoting)   : ENT_COMPAT;

	escaped = php_escape_html_entities_ex((unsigned char *)(Z_STRVAL_P(string)), Z_STRLEN_P(string), 1, qs, cs, 1);
	ZVAL_STR(return_value, escaped);

	if (unlikely(use_copy)) {
		zval_ptr_dtor(&copy);
	}
}

void phalcon_strval(zval *return_value, zval *v)
{
	zval copy = {};
	int use_copy = 0;

	use_copy = zend_make_printable_zval(v, &copy);
	if (use_copy) {
		zval *tmp = &copy;
		ZVAL_ZVAL(return_value, tmp, 0, 0);
	}
	else {
		ZVAL_ZVAL(return_value, v, 1, 0);
	}
}

void phalcon_date(zval *return_value, zval *format, zval *timestamp)
{
	zval copy = {};
	long int ts;
	int use_copy = 0;
	zend_string *formatted;

	if (unlikely(Z_TYPE_P(format) != IS_STRING)) {
		use_copy = zend_make_printable_zval(format, &copy);
		if (use_copy) {
			format = &copy;
		}
	}

	ts = (timestamp) ? phalcon_get_intval(timestamp) : time(NULL);

	formatted = php_format_date(Z_STRVAL_P(format), Z_STRLEN_P(format), ts, 1);
	ZVAL_STR(return_value, formatted);

	if (unlikely(use_copy)) {
		zval_ptr_dtor(&copy);
	}
}

void phalcon_addslashes(zval *return_value, zval *str)
{
	zval copy = {};
	int use_copy = 0;

	if (unlikely(Z_TYPE_P(str) != IS_STRING)) {
		use_copy = zend_make_printable_zval(str, &copy);
		if (use_copy) {
			str = &copy;
		}
	}

#if PHP_VERSION_ID < 70300
	ZVAL_STR(return_value, php_addslashes(Z_STR_P(str), 0));
#else
	ZVAL_STR(return_value, php_addslashes(Z_STR_P(str)));
#endif

	if (unlikely(use_copy)) {
		zval_ptr_dtor(&copy);
	}
}

void phalcon_add_trailing_slash(zval* return_value, zval *v)
{
	ZVAL_COPY(return_value, v);
	PHALCON_ENSURE_IS_STRING(return_value);

	if (Z_STRLEN_P(return_value)) {
		int len = Z_STRLEN_P(return_value);
		char *c = Z_STRVAL_P(return_value);

#ifdef PHP_WIN32
		if (c[len - 1] != '/' && c[len - 1] != '\\')
#else
		if (c[len - 1] != PHP_DIR_SEPARATOR)
#endif
		{
			Z_STR_P(return_value) = zend_string_extend(Z_STR_P(return_value), len+2, 0);
			c = Z_STRVAL_P(return_value);

			if (c != NULL) {
				c[len]   = PHP_DIR_SEPARATOR;
				c[len + 1] = 0;
				zend_string_free(Z_STR_P(return_value));
				ZVAL_STRINGL(return_value, c, len+1);
			}
		}
	}
}

void phalcon_stripslashes(zval *return_value, zval *str)
{
	zval copy = {};
	int use_copy = 0;

	if (unlikely(Z_TYPE_P(str) != IS_STRING)) {
		use_copy = zend_make_printable_zval(str, &copy);
		if (use_copy) {
			str = &copy;
		}
	}

	ZVAL_NEW_STR(return_value, zend_string_dup(Z_STR_P(str), 0));
	php_stripslashes(Z_STR_P(return_value));

	if (unlikely(use_copy)) {
		zval_ptr_dtor(&copy);
	}
}

void phalcon_stripcslashes(zval *return_value, zval *str)
{
	zval copy = {};
	int use_copy = 0;

	if (unlikely(Z_TYPE_P(str) != IS_STRING)) {
		use_copy = zend_make_printable_zval(str, &copy);
		if (use_copy) {
			str = &copy;
		}
	}

	ZVAL_NEW_STR(return_value, zend_string_dup(Z_STR_P(str), 0));
	php_stripcslashes(Z_STR_P(return_value));

	if (unlikely(use_copy)) {
		zval_ptr_dtor(&copy);
	}
}

#ifdef PHALCON_USE_PHP_MBSTRING
void phalcon_detect_encoding(zval *return_value, zval *str, zval *charset, zend_bool strict)
{
	zval copy = {};
	int use_copy = 0;
	mbfl_string string;
	const mbfl_encoding *ret;
	const mbfl_encoding **elist, **list;
	size_t size;

	list = NULL;
	size = 0;

	if (unlikely(Z_TYPE_P(str) != IS_STRING)) {
		use_copy = zend_make_printable_zval(str, &copy);
		if (use_copy) {
			str = &copy;
		}
	}

	if (FAILURE == php_mb_parse_encoding_list(Z_STRVAL_P(charset), Z_STRLEN_P(charset), &list, &size, 0)) {
		if (list) {
			efree(list);
			list = NULL;
			size = 0;
		}
	}

	if (size <= 0) {
		php_error_docref(NULL, E_WARNING, "Illegal argument");
	}

	if (size > 0 && list != NULL) {
		elist = list;
	} else {
		elist = MBSTRG(current_detect_order_list);
		size = MBSTRG(current_detect_order_list_size);
	}

	mbfl_string_init(&string);
	string.no_language = MBSTRG(language);
	string.val = (unsigned char *)str;
	string.len = str_len;
	ret = mbfl_identify_encoding2(&string, elist, size, strict);

	if (list != NULL) {
		efree((void *)list);
	}

	if (ret == NULL) {
		ZVAL_FALSE(return_value);
	}

	ZVAL_STRING(return_value, (char *)ret->name);
}


void phalcon_convert_encoding(zval *return_value, zval *str, zval *to, zval *from)
{
	char *ret;

	ret = php_mb_convert_encoding(Z_STRVAL_P(str), Z_STRLEN_P(str), Z_STRVAL_P(to), Z_STRVAL_P(from), &size);
	if (ret != NULL) {
		ZVAL_STRINGL(return_value, ret, size);
		efree(ret);
	} else {
		ZVAL_FALSE(return_value);
	}
}
#endif
