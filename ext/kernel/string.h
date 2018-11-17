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

#ifndef PHALCON_KERNEL_STRING_H
#define PHALCON_KERNEL_STRING_H

#include "php_phalcon.h"

#include <Zend/zend_smart_str.h>

#ifdef PHALCON_USE_PHP_JSON
# include <ext/json/php_json.h>
#endif

#define PHALCON_TRIM_LEFT  1
#define PHALCON_TRIM_RIGHT 2
#define PHALCON_TRIM_BOTH  3

#define PHALCON_PDA_LEFT  1
#define PHALCON_PDA_RIGHT 2
#define PHALCON_PDA_BOTH  3

#define PHALCON_RANDOM_ALNUM   0
#define PHALCON_RANDOM_ALPHA   1
#define PHALCON_RANDOM_HEXDEC  2
#define PHALCON_RANDOM_NUMERIC 3
#define PHALCON_RANDOM_NOZERO  4

#define PHALCON_STR_REPLACE(return_value, search, replace, subject) phalcon_fast_str_replace(return_value, search, replace, subject)
#define PHALCON_PREG_REPLACE(return_value, regex, replace, subject) phalcon_fast_preg_replace(return_value, regex, replace, subject)

#define phalcon_smart_str_appendl(dest, src, len) \
	smart_str_appendl_ex((dest), (src), (len), 0)

/** Fast char position */
int phalcon_memnstr(const zval *haystack, const zval *needle);
int phalcon_memnstr_string(const zval *haystack, const zend_string *needle);
int phalcon_memnstr_string_string(zend_string *haystack, zend_string *needle);
int phalcon_memnstr_str(const zval *haystack, char *needle, unsigned int needle_length);
int phalcon_memnstr_str_str(const char *haystack, unsigned int haystack_length, char *needle, unsigned int needle_length);
int phalcon_same_name(const char *key, const char *name, uint32_t name_len);

void phalcon_strtr(zval *return_value, zval *str, zval *str_from, zval *str_to);
void phalcon_strtr_str(zval *return_value, zval *str, char *str_from, unsigned int str_from_length, char *str_to, unsigned int str_to_length);
void phalcon_strtr_array(zval *return_value, zval *str, zval *replace_pairs);

/** Function replacement */
void phalcon_fast_strlen(zval *return_value, zval *str);
void phalcon_strlen(zval *return_value, zval *str);
int phalcon_fast_strlen_ev(zval *str);
void phalcon_fast_strtolower(zval *return_value, zval *str);
void phalcon_strtolower(zval *return_value, zval *str);
void phalcon_strtolower_inplace(zval *s);
void phalcon_fast_strtoupper(zval *return_value, zval *str);
void phalcon_strtoupper(zval *return_value, zval *str);
void phalcon_fast_join(zval *result, zval *glue, zval *pieces);
void phalcon_append_printable_zval(smart_str *implstr, zval *tmp);
void phalcon_fast_join_str(zval *result, char *glue, unsigned int glue_length, zval *pieces);
void phalcon_fast_explode(zval *result, zval *delimiter, zval *str);
void phalcon_fast_explode_str(zval *result, const char *delimiter, unsigned int delimiter_length, zval *str);
void phalcon_fast_explode_str_str(zval *result, const char *delimiter, unsigned int delimiter_length, const char *str, unsigned int str_length);
int phalcon_fast_strpos(zval *return_value, const zval *haystack, const zval *needle);
int phalcon_fast_strpos_str(zval *return_value, const zval *haystack, const char *needle, unsigned int needle_length);
int phalcon_fast_stripos_str(zval *return_value, const zval *haystack, const char *needle, unsigned int needle_length);
int phalcon_fast_strrpos(zval *return_value, const zval *haystack, const zval *needle);
int phalcon_fast_strrpos_str(zval *return_value, const zval *haystack, const char *needle, unsigned int needle_length);
int phalcon_fast_strripos_str(zval *return_value, const zval *haystack, const char *needle, unsigned int needle_length);
zend_string* phalcon_trim(zval *str, zval *charlist, int where);
void phalcon_fast_strip_tags(zval *return_value, zval *str);
void phalcon_fast_trim(zval *return_value, zval *str, zval *charlist, int where);
static inline void phalcon_fast_trim_str(zval *return_value, zval *str, const char *charlist, int where)
{
	zval c = {};

	ZVAL_STRING(&c, charlist);

	phalcon_fast_trim(return_value, str, &c, where);
	zval_ptr_dtor(&c);
}

void phalcon_fast_str_replace(zval *return_value, zval *search, zval *replace, zval *subject);
void phalcon_fast_preg_replace(zval *return_value, zval *search, zval *replace, zval *subject);

static inline void phalcon_fast_str_replace_str(zval *return_value, const char *search, const char *replace, zval *subject)
{
	zval s = {}, r = {};

	ZVAL_STRING(&s, search);
	ZVAL_STRING(&r, replace);

	phalcon_fast_str_replace(return_value, &s, &r, subject);
	zval_ptr_dtor(&s);
	zval_ptr_dtor(&r);
}

void phalcon_pad_str(zval *return_value, zval *input, int pad_length, const char *pad_str, int pad_type);

/** Camelize/Uncamelize */
void phalcon_camelize(zval *return_value, const zval *str);
void phalcon_uncamelize(zval *return_value, const zval *str);

void phalcon_camelize_delim(zval *return_value, const zval *str, const zval *delimiter);
void phalcon_uncamelize_delim(zval *return_value, const zval *str, const zval *delimiter);

/** Starts/Ends with */
int phalcon_start_with(const zval *str, const zval *compared, zval *case_sensitive);
int phalcon_start_with_str(const zval *str, char *compared, unsigned int compared_length);
int phalcon_start_with_str_str(char *str, unsigned int str_length, char *compared, unsigned int compared_length);
int phalcon_end_with(const zval *str, const zval *compared, zval *case_sensitive);
int phalcon_end_with_str(const zval *str, char *compared, unsigned int compared_length);

/** Compare */
int phalcon_comparestr(const zval *str, const zval *compared, zval *case_sensitive);
int phalcon_comparestr_str(const zval *str, char *compared, unsigned int compared_length, zval *case_sensitive);

/** Random string */
void phalcon_random_string(zval *return_value, const zval *type, const zval *length);

/* Strips extra slashes */
void phalcon_remove_extra_slashes(zval *return_value, const zval *str);

/** Generates a unique key for an array/object */
void phalcon_unique_key(zval *return_value, zval *prefix, zval *value);

/** spprintf */
int phalcon_spprintf(char **message, int max_len, char *format, ...);

/* Substr */
void phalcon_substr(zval *return_value, zval *str, unsigned long from, long length);
void phalcon_substr_string(zval *return_value, zend_string *str, unsigned long from, long length);

/** Preg-Match */
int phalcon_preg_match(zval *return_value, zval *regex, zval *subject, zval *matches, zend_long flags, int global);

/** Base64 */
void phalcon_base64_encode(zval *return_value, zval *data);
void phalcon_base64_decode(zval *return_value, zval *data);

/** Hash */
void phalcon_md5(zval *return_value, zval *str);
void phalcon_crc32(zval *return_value, zval *str);

/** JSON */
int phalcon_json_encode(zval *return_value, zval *v, int opts);
int phalcon_json_decode(zval *return_value, zval *v, zend_bool assoc);

/***/
void phalcon_lcfirst(zval *return_value, zval *s);
void phalcon_ucfirst(zval *return_value, zval *s);
int phalcon_http_build_query(zval *return_value, zval *params, char *sep);
void phalcon_htmlspecialchars(zval *return_value, zval *string, zval *quoting, zval *charset);
void phalcon_htmlentities(zval *return_value, zval *string, zval *quoting, zval *charset);
void phalcon_strval(zval *return_value, zval *v);
void phalcon_date(zval *return_value, zval *format, zval *timestamp);
void phalcon_addslashes(zval *return_value, zval *str);
void phalcon_add_trailing_slash(zval *return_value, zval* v);
void phalcon_stripslashes(zval *return_value, zval *str);
void phalcon_stripcslashes(zval *return_value, zval *str);

#ifdef PHALCON_USE_PHP_MBSTRING
void phalcon_detect_encoding(zval *return_value, zval *str, zval *charset, zend_bool strict);
void phalcon_convert_encoding(zval *return_value, zval *str, zval *to, zval *from);
#endif

#endif /* PHALCON_KERNEL_STRING_H */
