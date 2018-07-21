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
	phalcon_fast_strlen(return_value, str);
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

/**
 * Immediate function resolution for str_replace function
 */
void phalcon_fast_str_replace(zval *retval, zval *search, zval *replace, zval *subject)
{
	zval replace_copy = {}, search_copy = {};
	int copy_replace = 0, copy_search = 0;

	if (Z_TYPE_P(subject) != IS_STRING) {
		ZVAL_NULL(retval);
		zend_error(E_WARNING, "Invalid arguments supplied for str_replace()");
		return;
	}

	/**
	 * Fallback to userland function if the first parameter is an array
	 */
	if (Z_TYPE_P(search) == IS_ARRAY) {
		do {
			PHALCON_CALL_FUNCTION(retval, "str_replace", search, replace, subject);
			return;
		} while(0);
	}

	if (Z_TYPE_P(replace) != IS_STRING) {
		copy_replace = zend_make_printable_zval(replace, &replace_copy);
		if (copy_replace) {
			replace = &replace_copy;
		}
	}

	if (Z_TYPE_P(search) != IS_STRING) {
		copy_search = zend_make_printable_zval(search, &search_copy);
		if (copy_search) {
			search = &search_copy;
		}
	}

	if (Z_STRLEN_P(subject) == 0) {
		ZVAL_STRINGL(retval, "", 0);
		return;
	}

	ZVAL_STR(retval, php_str_to_str(Z_STRVAL_P(subject),
			Z_STRLEN_P(subject),
			Z_STRVAL_P(search),
			Z_STRLEN_P(search),
			Z_STRVAL_P(replace),
			Z_STRLEN_P(replace)));

	if (copy_replace) {
		zval_ptr_dtor(replace);
	}

	if (copy_search) {
		zval_ptr_dtor(search);
	}
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

int phalcon_preg_match(zval *retval, zval *regex, zval *subject, zval *matches)
{
	int result;

	if (matches) {
		PHALCON_CALL_FUNCTION_FLAG(result, retval, "preg_match", regex, subject, matches);
	} else {
		PHALCON_CALL_FUNCTION_FLAG(result, retval, "preg_match", regex, subject);
	}

	return result;
}

int phalcon_json_encode(zval *retval, zval *v, int opts)
{
	zval zopts = {};
	int flag;

	ZVAL_LONG(&zopts, opts);

	PHALCON_CALL_FUNCTION_FLAG(flag, retval, "json_encode", v, &zopts);

	return flag;
}

int phalcon_json_decode(zval *retval, zval *v, zend_bool assoc)
{
	zval *zassoc = assoc ? &PHALCON_GLOBAL(z_true) : &PHALCON_GLOBAL(z_false);
	zval *params[] = { v, zassoc };

	return phalcon_call_function_with_params(retval, SL("json_decode"), 2, params);
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
		ZVAL_DUP(return_value, s);
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
	if (Z_TYPE_P(params) == IS_ARRAY || Z_TYPE_P(params) == IS_OBJECT) {
		smart_str formstr = { 0 };
		int res;

		res = php_url_encode_hash_ex(HASH_OF(params), &formstr, NULL, 0, NULL, 0, NULL, 0, (Z_TYPE_P(params) == IS_OBJECT ? params : NULL), sep, PHP_QUERY_RFC1738);

		if (res == SUCCESS) {
			if (!formstr.s) {
				ZVAL_EMPTY_STRING(return_value);
			} else {
				smart_str_0(&formstr);
				ZVAL_STR(return_value, formstr.s);
			}

			return SUCCESS;
		}

		smart_str_free(&formstr);
		ZVAL_FALSE(return_value);
	}
	else {
		ZVAL_NULL(return_value);
	}

	return FAILURE;
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

void phalcon_add_trailing_slash(zval* v)
{
	PHALCON_ENSURE_IS_STRING(v);
	if (Z_STRLEN_P(v)) {
		int len = Z_STRLEN_P(v);
		char *c = Z_STRVAL_P(v);

#ifdef PHP_WIN32
		if (c[len - 1] != '/' && c[len - 1] != '\\')
#else
		if (c[len - 1] != PHP_DIR_SEPARATOR)
#endif
		{
			SEPARATE_ZVAL(v);
			Z_STR_P(v) = zend_string_extend(Z_STR_P(v), len+2, 0);
			c = Z_STRVAL_P(v);

			if (c != NULL) {
				c[len]   = PHP_DIR_SEPARATOR;
				c[len + 1] = 0;

				ZVAL_STRINGL(v, c, len+1);
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
