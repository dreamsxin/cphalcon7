
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

#include "binary/reader.h"
#include "binary.h"
#include "binary/exception.h"

#include <ext/standard/php_array.h>
#include <ext/spl/spl_array.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/concat.h"
#include "kernel/object.h"
#include "kernel/exception.h"

#include "interned-strings.h"

/**
 * Phalcon\Binary\Reader
 *
 * Provides utilities to work with binary data
 */
zend_class_entry *phalcon_binary_reader_ce;

PHP_METHOD(Phalcon_Binary_Reader, __construct);
PHP_METHOD(Phalcon_Binary_Reader, getEndian);
PHP_METHOD(Phalcon_Binary_Reader, getInput);
PHP_METHOD(Phalcon_Binary_Reader, getContent);
PHP_METHOD(Phalcon_Binary_Reader, setPosition);
PHP_METHOD(Phalcon_Binary_Reader, getPosition);
PHP_METHOD(Phalcon_Binary_Reader, getEofPosition);
PHP_METHOD(Phalcon_Binary_Reader, isEof);
PHP_METHOD(Phalcon_Binary_Reader, read);
PHP_METHOD(Phalcon_Binary_Reader, readChar);
PHP_METHOD(Phalcon_Binary_Reader, readUnsignedChar);
PHP_METHOD(Phalcon_Binary_Reader, readInt16);
PHP_METHOD(Phalcon_Binary_Reader, readUnsignedInt16);
PHP_METHOD(Phalcon_Binary_Reader, readInt);
PHP_METHOD(Phalcon_Binary_Reader, readUnsignedInt);
PHP_METHOD(Phalcon_Binary_Reader, readInt32);
PHP_METHOD(Phalcon_Binary_Reader, readUnsignedInt32);
PHP_METHOD(Phalcon_Binary_Reader, readFloat);
PHP_METHOD(Phalcon_Binary_Reader, readDouble);
PHP_METHOD(Phalcon_Binary_Reader, readString);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, endian)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_setposition, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, position, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_read, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_readstring, 0, 0, 0)
	ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_arr_method_entry[] = {
	PHP_ME(Phalcon_Binary_Reader, __construct, arginfo_phalcon_binary___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Binary_Reader, getEndian, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, getInput, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, getContent, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, setPosition, arginfo_phalcon_binary_setposition, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, getPosition, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, getEofPosition, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, isEof, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, read, arginfo_phalcon_binary_read, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readChar, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readUnsignedChar, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readInt16, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readUnsignedInt16, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readInt, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readUnsignedInt, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readInt32, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readUnsignedInt32, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readFloat, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readDouble, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readString, arginfo_phalcon_binary_readstring, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Binary initializer
 */
PHALCON_INIT_CLASS(Phalcon_Binary_Reader){

	PHALCON_REGISTER_CLASS(Phalcon\\Binary, Reader, binary_reader, phalcon_arr_method_entry, 0);

	zend_declare_property_long(phalcon_binary_reader_ce, SL("_endian"), PHALCON_BINARY_ENDIAN_BIG, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_binary_reader_ce, SL("_data"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_binary_reader_ce, SL("_input"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_binary_reader_ce, SL("_position"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_binary_reader_ce, SL("_eofPosition"), 0, ZEND_ACC_PROTECTED);
	return SUCCESS;
}

/**
 * Phalcon\Binary\Reader constructor
 *
 * @param  string|resource $data
 * @param  int $endian
 * @throws \InvalidArgumentException
 */
PHP_METHOD(Phalcon_Binary_Reader, __construct){

	zval *data, *endian = NULL, filename = {}, mode = {}, handler = {}, fstat = {}, size = {}, content = {};

	phalcon_fetch_params(0, 1, 1, &data, &endian);

	if (Z_TYPE_P(data) == IS_STRING) {
		phalcon_update_property_zval(getThis(), SL("_data"), data);

		ZVAL_STRING(&filename, "php://memory");
		ZVAL_STRING(&mode, "br+");
		PHALCON_CALL_FUNCTIONW(&handler, "fopen", &filename, &mode);
		PHALCON_CALL_FUNCTIONW(NULL, "fwrite", &handler, data);
		PHALCON_CALL_FUNCTIONW(NULL, "rewind", &handler);

		PHALCON_CALL_FUNCTIONW(&fstat, "fstat", &handler);
		if (phalcon_array_isset_fetch_str(&size, &fstat, SL("size"))) {
			phalcon_update_property_zval(getThis(), SL("_eofPosition"), &size);
		}
		phalcon_update_property_zval(getThis(), SL("_input"), &handler);
	} else if (Z_TYPE_P(data) == IS_RESOURCE) {
		phalcon_update_property_zval(getThis(), SL("_input"), data);

		PHALCON_CALL_FUNCTIONW(&fstat, "fstat", data);
		if (phalcon_array_isset_fetch_str(&size, &fstat, SL("size"))) {
			phalcon_update_property_zval(getThis(), SL("_eofPosition"), &size);
		}

		PHALCON_CALL_FUNCTIONW(NULL, "rewind", data);
		PHALCON_CALL_FUNCTIONW(&content, "fread", data, &size);
		PHALCON_CALL_FUNCTIONW(NULL, "rewind", data);
		phalcon_update_property_zval(getThis(), SL("_data"), &content);
	} else {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_binary_exception_ce, "Data must be set as string or resource");
		return;
	}

	if (endian && Z_TYPE_P(endian) != IS_NULL) {
		if (Z_TYPE_P(endian) != IS_LONG || Z_LVAL_P(endian) < 0 || Z_LVAL_P(endian) > 2) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_binary_exception_ce, "Endian must be set as big or little");
		}
		phalcon_update_property_zval(getThis(), SL("_endian"), endian);
	}
}

/**
 * Gets the endian
 *
 * @return int
 */
PHP_METHOD(Phalcon_Binary_Reader, getEndian){


	RETURN_MEMBER(getThis(), "_endian");
}

/**
 * Gets the input
 *
 * @return int
 */
PHP_METHOD(Phalcon_Binary_Reader, getInput){


	RETURN_MEMBER(getThis(), "_input");
}

/**
 * Gets the data
 *
 * @return int
 */
PHP_METHOD(Phalcon_Binary_Reader, getContent){


	RETURN_MEMBER(getThis(), "_data");
}

/**
 *
 */
PHP_METHOD(Phalcon_Binary_Reader, setPosition){

	zval position = {}, input = {};

	phalcon_fetch_params(0, 1, 0, &position);

	phalcon_read_property(&input, getThis(), SL("_input"), PH_NOISY);
	PHALCON_CALL_FUNCTIONW(return_value, "fseek", &input, &position);

	phalcon_update_property_zval(getThis(), SL("_position"), &position);
}

/**
 * Gets the current postion
 *
 * @return int
 */
PHP_METHOD(Phalcon_Binary_Reader, getPosition){


	RETURN_MEMBER(getThis(), "_position");
}

/**
 * Gets the eof postion
 *
 * @return int
 */
PHP_METHOD(Phalcon_Binary_Reader, getEofPosition){


	RETURN_MEMBER(getThis(), "_eofPosition");
}

/**
 * Check if 
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Binary_Reader, isEof){

	zval position = {}, eof_position = {};

	phalcon_read_property(&position, getThis(), SL("_position"), PH_NOISY);
	phalcon_read_property(&eof_position, getThis(), SL("_eofPosition"), PH_NOISY);

	if (PHALCON_GE(&position, &eof_position)) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}

/**
 *
 */
PHP_METHOD(Phalcon_Binary_Reader, read){

	zval *length, position = {}, eof_position = {}, result = {}, input = {};

	phalcon_fetch_params(0, 1, 0, &length);

	phalcon_read_property(&position, getThis(), SL("_position"), PH_NOISY);
	phalcon_read_property(&eof_position, getThis(), SL("_eofPosition"), PH_NOISY);
	phalcon_add_function(&result, &position, length);
	if (PHALCON_GT(&result, &eof_position)) {
		PHALCON_THROW_EXCEPTION_FORMATW(phalcon_binary_exception_ce, "Outside of input, postion: %d, total length: %d", Z_LVAL(position), Z_LVAL(eof_position));
		return;
	}

	phalcon_read_property(&input, getThis(), SL("_input"), PH_NOISY);
	PHALCON_CALL_FUNCTIONW(return_value, "fread", &input, length);
	phalcon_update_property_zval(getThis(), SL("_position"), &result);
}

/**
 *
 */
PHP_METHOD(Phalcon_Binary_Reader, readChar){

	zval length = {}, bytes = {}, format = {}, result = {};

	ZVAL_LONG(&length, 1);
	PHALCON_CALL_METHODW(&bytes, getThis(), "read", &length);

	ZVAL_STRING(&format, "c");
	PHALCON_CALL_FUNCTIONW(&result, "unpack", &format, &bytes);
	if (!phalcon_array_isset_fetch_long(return_value, &result, 1)) {
		RETURN_NULL();
	}
}

/**
 *
 */
PHP_METHOD(Phalcon_Binary_Reader, readUnsignedChar){

	zval length = {}, bytes = {}, format = {}, result = {};

	ZVAL_LONG(&length, 1);
	PHALCON_CALL_METHODW(&bytes, getThis(), "read", &length);

	ZVAL_STRING(&format, "C");
	PHALCON_CALL_FUNCTIONW(&result, "unpack", &format, &bytes);
	if (!phalcon_array_isset_fetch_long(return_value, &result, 1)) {
		RETURN_NULL();
	}
}

/**
 *
 */
PHP_METHOD(Phalcon_Binary_Reader, readInt16){

	zval length = {}, bytes = {}, format = {}, result = {};

	ZVAL_LONG(&length, 2);
	PHALCON_CALL_METHODW(&bytes, getThis(), "read", &length);

	ZVAL_STRING(&format, "s");

	PHALCON_CALL_FUNCTIONW(&result, "unpack", &format, &bytes);
	if (!phalcon_array_isset_fetch_long(return_value, &result, 1)) {
		RETURN_NULL();
	}
}

/**
 *
 */
PHP_METHOD(Phalcon_Binary_Reader, readUnsignedInt16){

	zval length = {}, bytes = {}, endian = {}, format = {}, result = {};

	ZVAL_LONG(&length, 2);
	PHALCON_CALL_METHODW(&bytes, getThis(), "read", &length);

	phalcon_read_property(&endian, getThis(), SL("_endian"), PH_NOISY);
	if (Z_LVAL(endian) == PHALCON_BINARY_ENDIAN_BIG) {
		ZVAL_STRING(&format, "n");
	} else if (Z_LVAL(endian) == PHALCON_BINARY_ENDIAN_LITTLE) {
		ZVAL_STRING(&format, "v");
	} else if (Z_LVAL(endian) == PHALCON_BINARY_ENDIAN_MACHINE) {
		ZVAL_STRING(&format, "S");
	}

	PHALCON_CALL_FUNCTIONW(&result, "unpack", &format, &bytes);
	if (!phalcon_array_isset_fetch_long(return_value, &result, 1)) {
		RETURN_NULL();
	}
}

/**
 *
 */
PHP_METHOD(Phalcon_Binary_Reader, readInt){

	zval length = {}, bytes = {}, format = {}, result = {};

	ZVAL_LONG(&length, sizeof(int));
	PHALCON_CALL_METHODW(&bytes, getThis(), "read", &length);

	ZVAL_STRING(&format, "i");

	PHALCON_CALL_FUNCTIONW(&result, "unpack", &format, &bytes);
	if (!phalcon_array_isset_fetch_long(return_value, &result, 1)) {
		RETURN_NULL();
	}
}

/**
 *
 */
PHP_METHOD(Phalcon_Binary_Reader, readUnsignedInt){

	zval length = {}, bytes = {}, format = {}, result = {};

	ZVAL_LONG(&length, sizeof(int));
	PHALCON_CALL_METHODW(&bytes, getThis(), "read", &length);

	ZVAL_STRING(&format, "I");

	PHALCON_CALL_FUNCTIONW(&result, "unpack", &format, &bytes);
	if (!phalcon_array_isset_fetch_long(return_value, &result, 1)) {
		RETURN_NULL();
	}
}

/**
 *
 */
PHP_METHOD(Phalcon_Binary_Reader, readInt32){

	zval length = {}, bytes = {}, format = {}, result = {};

	ZVAL_LONG(&length, 4);
	PHALCON_CALL_METHODW(&bytes, getThis(), "read", &length);

	ZVAL_STRING(&format, "l");

	PHALCON_CALL_FUNCTIONW(&result, "unpack", &format, &bytes);
	if (!phalcon_array_isset_fetch_long(return_value, &result, 1)) {
		RETURN_NULL();
	}
}

/**
 *
 */
PHP_METHOD(Phalcon_Binary_Reader, readUnsignedInt32){

	zval length = {}, bytes = {}, endian = {}, format = {}, result = {};

	ZVAL_LONG(&length, 4);
	PHALCON_CALL_METHODW(&bytes, getThis(), "read", &length);

	phalcon_read_property(&endian, getThis(), SL("_endian"), PH_NOISY);
	if (Z_LVAL(endian) == PHALCON_BINARY_ENDIAN_BIG) {
		ZVAL_STRING(&format, "N");
	} else if (Z_LVAL(endian) == PHALCON_BINARY_ENDIAN_LITTLE) {
		ZVAL_STRING(&format, "V");
	} else if (Z_LVAL(endian) == PHALCON_BINARY_ENDIAN_MACHINE) {
		ZVAL_STRING(&format, "L");
	}

	PHALCON_CALL_FUNCTIONW(&result, "unpack", &format, &bytes);
	if (!phalcon_array_isset_fetch_long(return_value, &result, 1)) {
		RETURN_NULL();
	}
}

/**
 *
 */
PHP_METHOD(Phalcon_Binary_Reader, readFloat){

	zval length = {}, bytes = {}, format = {}, result = {};

	ZVAL_LONG(&length, sizeof(float));
	PHALCON_CALL_METHODW(&bytes, getThis(), "read", &length);

	ZVAL_STRING(&format, "f");

	PHALCON_CALL_FUNCTIONW(&result, "unpack", &format, &bytes);
	if (!phalcon_array_isset_fetch_long(return_value, &result, 1)) {
		RETURN_NULL();
	}
}

/**
 *
 */
PHP_METHOD(Phalcon_Binary_Reader, readDouble){

	zval length = {}, bytes = {}, format = {}, result = {};

	ZVAL_LONG(&length, sizeof(double));
	PHALCON_CALL_METHODW(&bytes, getThis(), "read", &length);

	ZVAL_STRING(&format, "d");

	PHALCON_CALL_FUNCTIONW(&result, "unpack", &format, &bytes);
	if (!phalcon_array_isset_fetch_long(return_value, &result, 1)) {
		RETURN_NULL();
	}
}

/**
 *
 */
PHP_METHOD(Phalcon_Binary_Reader, readString){

	zval *length = NULL, position = {}, eof_position = {}, format = {}, data = {}, result = {}, len = {};

	phalcon_fetch_params(0, 0, 1, &length);

	if (length) {
		PHALCON_CALL_METHODW(return_value, getThis(), "read", length);
	} else {
		phalcon_read_property(&position, getThis(), SL("_position"), PH_NOISY);
		phalcon_read_property(&eof_position, getThis(), SL("_eofPosition"), PH_NOISY);
		if (PHALCON_GE(&position, &eof_position)) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_binary_exception_ce, "Not enough input");
			return;
		}
		PHALCON_CONCAT_SVS(&format, "@", &position, "/Z*");

		phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY);
		PHALCON_CALL_FUNCTIONW(&result, "unpack", &format, &data);
		if (!phalcon_array_isset_fetch_long(return_value, &result, 1)) {
			RETURN_NULL();
		} else {
			ZVAL_LONG(&len, Z_STRLEN_P(return_value));
			phalcon_add_function(&result, &position, &len);
			if (!PHALCON_GE(&result, &eof_position)) {
				ZVAL_LONG(&len, Z_STRLEN_P(return_value) + 1);
			}

			PHALCON_CALL_METHODW(NULL, getThis(), "read", &len);
		}
	}
}
