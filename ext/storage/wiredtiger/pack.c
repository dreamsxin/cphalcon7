
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

#include "storage/wiredtiger/pack.h"

void phalcon_storage_wiredtiger_pack_item_free(phalcon_storage_wiredtiger_pack_item *item)
{
	if (item->data) {
		efree(item->data);
		item->data = NULL;
	}
	item->size = 0;
	item->asize = 0;
}

static int phalcon_storage_wiredtiger_pack_key_scalar(phalcon_storage_wiredtiger_cursor_object *intern, phalcon_storage_wiredtiger_pack_item *item, zval *key)
{
	int ret;
	const char *format;
	size_t format_len;
	WT_PACK_STREAM *stream;

	format = intern->cursor->key_format;
	format_len = strlen(format);

	item->asize = format_len;

	switch (format[0]) {
		case 'x':
			break;
		case 'b':
		case 'h':
		case 'i':
		case 'l':
		case 'q':
		case 'B':
		case 'H':
		case 'I':
		case 'L':
		case 'Q':
		case 'r':
		case 't':
			if (Z_TYPE_P(key) != IS_LONG) {
				convert_to_long(key);
			}
			item->asize += 4;
			break;
		case 's':
		case 'S':
		case 'u':
			if (Z_TYPE_P(key) != IS_STRING) {
				convert_to_string(key);
			}
			item->asize += Z_STRLEN_P(key);
			break;
	}

	item->data = emalloc(item->asize);
	if (!item->data) {
		php_error_docref(NULL, E_WARNING, "Invalid allocate memory");
		return 1;
	}

	ret = wiredtiger_pack_start(intern->cursor->session,
								intern->cursor->key_format,
								item->data, item->asize, &stream);
	if (ret != 0) {
		php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
		return ret;
	}

	switch (format[0]) {
		case 'x':
			break;
		case 'b':
		case 'h':
		case 'i':
		case 'l':
		case 'q':
			ret = wiredtiger_pack_int(stream, (int64_t)Z_LVAL_P(key));
			if (ret != 0) {
				php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
				return ret;
			}
			break;
		case 'B':
		case 'H':
		case 'I':
		case 'L':
		case 'Q':
		case 'r':
		case 't':
			ret = wiredtiger_pack_uint(stream, (uint64_t)Z_LVAL_P(key));
			if (ret != 0) {
				php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
				return ret;
			}
			break;
		case 's':
		case 'S':
			ret = wiredtiger_pack_str(stream, Z_STRVAL_P(key));
			if (ret != 0) {
				php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
				return ret;
			}
			break;
		case 'u': {
			WT_ITEM value;

			value.data = Z_STRVAL_P(key);
			value.size = Z_STRLEN_P(key);

			ret = wiredtiger_pack_item(stream, &value);
			if (ret != 0) {
				php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
				return ret;
			}
			break;
		}
	}

	ret = wiredtiger_pack_close(stream, &item->size);
	if (ret != 0) {
		php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
	}

	return 0;
}

static int phalcon_storage_wiredtiger_pack_key_array(phalcon_storage_wiredtiger_cursor_object *intern, phalcon_storage_wiredtiger_pack_item *item, zval *key)
{
	int ret;
	const char *format;
	size_t i, format_len;
	HashTable *ht;
	HashPosition pos;
	zval *tmp;
	WT_PACK_STREAM *stream;

	format = intern->cursor->key_format;
	format_len = strlen(format);

	item->asize = format_len;

	ht = HASH_OF(key);
	zend_hash_internal_pointer_reset_ex(ht, &pos);

	for (i = 0; i < format_len; i++) {
		switch (format[i]) {
			case 'x':
				break;
			case 'b':
			case 'h':
			case 'i':
			case 'l':
			case 'q':
			case 'B':
			case 'H':
			case 'I':
			case 'L':
			case 'Q':
			case 'r':
			case 't':
				zend_hash_move_forward_ex(ht, &pos);
				item->asize += 4;
				break;
			case 's':
			case 'S':
			case 'u':
				if ((tmp = zend_hash_get_current_data_ex(ht, &pos)) == NULL) {
					break;
				}
				zend_hash_move_forward_ex(ht, &pos);

				if (Z_TYPE_P(tmp) != IS_STRING) {
					convert_to_string(tmp);
				}
				item->asize += Z_STRLEN_P(tmp);
				break;
		}
	}

	item->data = emalloc(item->asize);
	if (!item->data) {
		php_error_docref(NULL, E_WARNING, "Invalid allocate memory");
		return 1;
	}

	ret = wiredtiger_pack_start(intern->cursor->session, intern->cursor->key_format, item->data, item->asize, &stream);
	if (ret != 0) {
		php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
		return ret;
	}

	zend_hash_internal_pointer_reset_ex(ht, &pos);

	for (i = 0; i < format_len; i++) {
		switch (format[i]) {
			case 'x':
				break;
			case 'b':
			case 'h':
			case 'i':
			case 'l':
			case 'q':
				if ((tmp = zend_hash_get_current_data_ex(ht, &pos)) == NULL) {
					break;
				}
				zend_hash_move_forward_ex(ht, &pos);

				if (Z_TYPE_P(tmp) != IS_LONG) {
					convert_to_long(tmp);
				}

				ret = wiredtiger_pack_int(stream, (int64_t)Z_LVAL_P(tmp));
				if (ret != 0) {
					php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
					return ret;
				}
				break;
			case 'B':
			case 'H':
			case 'I':
			case 'L':
			case 'Q':
			case 'r':
			case 't':
				if ((tmp = zend_hash_get_current_data_ex(ht, &pos)) == NULL) {
					break;
				}
				zend_hash_move_forward_ex(ht, &pos);

				if (Z_TYPE_P(tmp) != IS_LONG) {
					convert_to_long(tmp);
				}

				ret = wiredtiger_pack_uint(stream, (uint64_t)Z_LVAL_P(tmp));
				if (ret != 0) {
					php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
					return ret;
				}
				break;
			case 's':
			case 'S':
				if ((tmp = zend_hash_get_current_data_ex(ht, &pos)) == NULL) {
					break;
				}
				zend_hash_move_forward_ex(ht, &pos);

				if (Z_TYPE_P(tmp) != IS_STRING) {
					convert_to_string(tmp);
				}

				ret = wiredtiger_pack_str(stream, Z_STRVAL_P(tmp));
				if (ret != 0) {
					php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
					return ret;
				}
				break;
			case 'u': {
				WT_ITEM data;

			   if ((tmp = zend_hash_get_current_data_ex(ht, &pos)) == NULL) {
					break;
				}
				zend_hash_move_forward_ex(ht, &pos);

				if (Z_TYPE_P(tmp) != IS_STRING) {
					convert_to_string(tmp);
				}

				data.data = Z_STRVAL_P(tmp);
				data.size = Z_STRLEN_P(tmp);

				ret = wiredtiger_pack_item(stream, &data);
				if (ret != 0) {
					php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
					return ret;
				}
				break;
			}
		}
	}

	ret = wiredtiger_pack_close(stream, &item->size);
	if (ret != 0) {
		php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
	}

	return 0;
}

int phalcon_storage_wiredtiger_pack_key(phalcon_storage_wiredtiger_cursor_object *intern, phalcon_storage_wiredtiger_pack_item *item, zval *key)
{
	if (Z_TYPE_P(key) == IS_ARRAY) {
		return phalcon_storage_wiredtiger_pack_key_array(intern, item, key);
	} else {
		return phalcon_storage_wiredtiger_pack_key_scalar(intern, item, key);
	}
}

static int phalcon_storage_wiredtiger_pack_value_scalar(phalcon_storage_wiredtiger_cursor_object *intern, phalcon_storage_wiredtiger_pack_item *item, zval *value)
{
	int ret;
	const char *format;
	size_t format_len;
	WT_PACK_STREAM *stream;

	format = intern->cursor->value_format;
	format_len = strlen(format);

	item->asize = format_len;

	switch (format[0]) {
		case 'x':
			break;
		case 'b':
		case 'h':
		case 'i':
		case 'l':
		case 'q':
		case 'B':
		case 'H':
		case 'I':
		case 'L':
		case 'Q':
		case 'r':
		case 't':
			item->asize += 4;
			break;
		case 's':
		case 'S':
		case 'u':
			if (Z_TYPE_P(value) != IS_STRING) {
				convert_to_string(value);
			}
			item->asize += Z_STRLEN_P(value);
			break;
	}

	item->data = emalloc(item->asize);
	if (!item->data) {
		php_error_docref(NULL, E_WARNING, "Invalid allocate memory");
		return 1;
	}

	ret = wiredtiger_pack_start(intern->cursor->session, intern->cursor->value_format, item->data, item->asize, &stream);
	if (ret != 0) {
		php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
		return ret;
	}

	switch (format[0]) {
		case 'x':
			break;
		case 'b':
		case 'h':
		case 'i':
		case 'l':
		case 'q':
			if (Z_TYPE_P(value) != IS_LONG) {
				convert_to_long(value);
			}

			ret = wiredtiger_pack_int(stream, (int64_t)Z_LVAL_P(value));
			if (ret != 0) {
				php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
				return ret;
			}
			break;
		case 'B':
		case 'H':
		case 'I':
		case 'L':
		case 'Q':
		case 'r':
		case 't':
			if (Z_TYPE_P(value) != IS_LONG) {
				convert_to_long(value);
			}

			ret = wiredtiger_pack_uint(stream, (uint64_t)Z_LVAL_P(value));
			if (ret != 0) {
				php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
				return ret;
			}
			break;
		case 's':
		case 'S':
			if (Z_TYPE_P(value) != IS_STRING) {
				convert_to_string(value);
			}

			ret = wiredtiger_pack_str(stream, Z_STRVAL_P(value));
			if (ret != 0) {
				php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
				return ret;
			}
			break;
		case 'u': {
			WT_ITEM data = { 0, };

			if (Z_TYPE_P(value) != IS_STRING) {
				convert_to_string(value);
			}

			data.data = Z_STRVAL_P(value);
			data.size = Z_STRLEN_P(value);

			ret = wiredtiger_pack_item(stream, &data);
			if (ret != 0) {
				php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
				return ret;
			}
			break;
		}
	}

	ret = wiredtiger_pack_close(stream, &item->size);
	if (ret != 0) {
		php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
	}

	return 0;
}

static int phalcon_storage_wiredtiger_pack_value_array(phalcon_storage_wiredtiger_cursor_object *intern, phalcon_storage_wiredtiger_pack_item *item, zval *value)
{
	int ret;
	const char *format;
	size_t i, format_len;
	HashTable *ht;
	HashPosition pos;
	zval *tmp;
	WT_PACK_STREAM *stream;

	format = intern->cursor->value_format;
	format_len = strlen(format);

	item->asize = format_len;

	ht = HASH_OF(value);
	zend_hash_internal_pointer_reset_ex(ht, &pos);

	for (i = 0; i < format_len; i++) {
		switch (format[i]) {
			case 'x':
				break;
			case 'b':
			case 'h':
			case 'i':
			case 'l':
			case 'q':
			case 'B':
			case 'H':
			case 'I':
			case 'L':
			case 'Q':
			case 'r':
			case 't':
				zend_hash_move_forward_ex(ht, &pos);
				item->asize += 4;
				break;
			case 's':
			case 'S':
			case 'u':
				if ((tmp = zend_hash_get_current_data_ex(ht, &pos)) == NULL) {
					break;
				}
				zend_hash_move_forward_ex(ht, &pos);

				if (Z_TYPE_P(tmp) != IS_STRING) {
					convert_to_string(tmp);
				}

				item->asize += Z_STRLEN_P(tmp);
				break;
		}
	}

	item->data = emalloc(item->asize);
	if (!item->data) {
		php_error_docref(NULL, E_WARNING, "Invalid allocate memory");
		return 1;
	}

	ret = wiredtiger_pack_start(intern->cursor->session, intern->cursor->value_format, item->data, item->asize, &stream);
	if (ret != 0) {
		php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
		return ret;
	}

	zend_hash_internal_pointer_reset_ex(ht, &pos);

	for (i = 0; i < format_len; i++) {
		switch (format[i]) {
			case 'x':
				break;
			case 'b':
			case 'h':
			case 'i':
			case 'l':
			case 'q':
				if ((tmp = zend_hash_get_current_data_ex(ht, &pos)) == NULL) {
					break;
				}
				zend_hash_move_forward_ex(ht, &pos);

				if (Z_TYPE_P(tmp) != IS_LONG) {
					convert_to_long(tmp);
				}

				ret = wiredtiger_pack_int(stream, (int64_t)Z_LVAL_P(tmp));
				if (ret != 0) {
					php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
					return ret;
				}
				break;
			case 'B':
			case 'H':
			case 'I':
			case 'L':
			case 'Q':
			case 'r':
			case 't':
				if ((tmp = zend_hash_get_current_data_ex(ht, &pos)) == NULL) {
					break;
				}
				zend_hash_move_forward_ex(ht, &pos);

				if (Z_TYPE_P(tmp) != IS_LONG) {
					convert_to_long(tmp);
				}

				ret = wiredtiger_pack_uint(stream, (uint64_t)Z_LVAL_P(tmp));
				if (ret != 0) {
					php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
					return ret;
				}
				break;
			case 's':
			case 'S':
				if ((tmp = zend_hash_get_current_data_ex(ht, &pos)) == NULL) {
					break;
				}
				zend_hash_move_forward_ex(ht, &pos);

				if (Z_TYPE_P(tmp) != IS_STRING) {
					convert_to_string(tmp);
				}

				ret = wiredtiger_pack_str(stream, Z_STRVAL_P(tmp));
				if (ret != 0) {
					php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
					return ret;
				}
				break;
			case 'u': {
				WT_ITEM value = { 0, };

				if ((tmp = zend_hash_get_current_data_ex(ht, &pos)) == NULL) {
					break;
				}
				zend_hash_move_forward_ex(ht, &pos);

				if (Z_TYPE_P(tmp) != IS_STRING) {
					convert_to_string(tmp);
				}

				value.data = Z_STRVAL_P(tmp);
				value.size = Z_STRLEN_P(tmp);
				ret = wiredtiger_pack_item(stream, &value);
				if (ret != 0) {
					php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
					return ret;
				}
				break;
			}
		}
	}

	ret = wiredtiger_pack_close(stream, &item->size);
	if (ret != 0) {
		php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
	}

	return 0;
}

int phalcon_storage_wiredtiger_pack_value(phalcon_storage_wiredtiger_cursor_object *intern, phalcon_storage_wiredtiger_pack_item *item, zval *value)
{
	if (Z_TYPE_P(value) == IS_ARRAY) {
		return phalcon_storage_wiredtiger_pack_value_array(intern, item, value);
	} else {
		return phalcon_storage_wiredtiger_pack_value_scalar(intern, item, value);
	}
}

static int phalcon_storage_wiredtiger_unpack_ex(phalcon_storage_wiredtiger_cursor_object *intern, zval *return_value, const char *format, WT_ITEM *item)
{
	int ret;
	size_t i, size, format_len, len = 0;
	zval values = {};
	WT_PACK_STREAM *stream;

	array_init(&values);

	ret = wiredtiger_unpack_start(intern->cursor->session, format, item->data, item->size, &stream);
	if (ret != 0) {
		php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
		return ret;
	}

	format_len = strlen(format);
	size = 0;

	for (i = 0; i < format_len; i++) {
		switch (format[i]) {
			case 'x':
				add_next_index_null(&values);
				size = 0;
				break;
			case 'b':
			case 'h':
			case 'i':
			case 'l':
			case 'q': {
				int64_t value;
				ret = wiredtiger_unpack_int(stream, &value);
				if (ret != 0) {
					php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
					add_next_index_null(&values);
				} else {
					add_next_index_long(&values, value);
				}
				size = 0;
				break;
			}
			case 'B':
			case 'H':
			case 'I':
			case 'L':
			case 'Q':
			case 'r':
			case 't': {
				uint64_t value;
				ret = wiredtiger_unpack_uint(stream, &value);
				if (ret != 0) {
					php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
					add_next_index_null(&values);
				} else {
					add_next_index_long(&values, value);
				}
				size = 0;
				break;
			}
			case 's':
			case 'S': {
				const char *value = NULL;
				ret = wiredtiger_unpack_str(stream, &value);
				if (ret != 0) {
					php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
					add_next_index_null(&values);
				} else {
					len = strlen(value);
					if (size && size < len) {
						add_next_index_stringl(&values, value, size);
					} else {
						add_next_index_string(&values, value);
					}
				}
				size = 0;
				break;
			}
			case 'u': {
				WT_ITEM value = { 0, };
				ret = wiredtiger_unpack_item(stream, &value);
				if (ret != 0) {
					php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
					add_next_index_null(&values);
				} else {
					add_next_index_stringl(&values, value.data, value.size);
				}
				size = 0;
				break;
			}
			default:
				if (isdigit(format[i])) {
					if (size) {
						size = (size * 10) + (format[i] - '0');
					} else {
						size = (format[i] - '0');
					}
				} else {
					size = 0;
				}
		}
	}

	ret = wiredtiger_pack_close(stream, &size);
	if (ret != 0) {
		php_error_docref(NULL, E_WARNING, "%s", wiredtiger_strerror(ret));
	}

	switch (zend_hash_num_elements(HASH_OF(&values))) {
		case 0:
			RETVAL_FALSE;
			zval_ptr_dtor(&values);
			break;
		case 1: {
			zval *tmp = zend_hash_get_current_data(HASH_OF(&values));
			if (tmp) {
				RETVAL_ZVAL(tmp, 1, 1);
			}
			break;
		}
		default:
			RETVAL_ZVAL(&values, 1, 1);
			break;
	}

	return 0;
}

int phalcon_storage_wiredtiger_unpack_key(phalcon_storage_wiredtiger_cursor_object *intern, zval *return_value, WT_ITEM *item)
{
	return phalcon_storage_wiredtiger_unpack_ex(intern, return_value, intern->cursor->key_format, item);
}

int phalcon_storage_wiredtiger_unpack_value(phalcon_storage_wiredtiger_cursor_object *intern, zval *return_value, WT_ITEM *item)
{
	return phalcon_storage_wiredtiger_unpack_ex(intern, return_value, intern->cursor->value_format, item);
}
