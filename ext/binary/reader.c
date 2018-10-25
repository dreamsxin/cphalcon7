
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

#include <main/php_streams.h>
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
 *
 *<code>
 *	$fp = fopen('unit-tests/assets/data.bin', 'rb');
 *	$bin = new Phalcon\Binary\Reader($fp);
 *	$v = $bin->readUnsignedChar();
 *</code>
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
PHP_METHOD(Phalcon_Binary_Reader, readHexString);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_reader___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, endian)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_reader_setposition, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, position, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, whence, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_reader_read, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_reader_readunsignedint16, 0, 0, 0)
	ZEND_ARG_INFO(0, endian)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_reader_readunsignedint32, 0, 0, 0)
	ZEND_ARG_INFO(0, endian)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_reader_readstring, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_reader_readhexstring, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 1)
	ZEND_ARG_INFO(0, lowNibble)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_binary_reader_method_entry[] = {
	PHP_ME(Phalcon_Binary_Reader, __construct, arginfo_phalcon_binary_reader___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Binary_Reader, getEndian, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, getInput, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, getContent, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, setPosition, arginfo_phalcon_binary_reader_setposition, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, getPosition, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, getEofPosition, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, isEof, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, read, arginfo_phalcon_binary_reader_read, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readChar, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readUnsignedChar, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readInt16, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readUnsignedInt16, arginfo_phalcon_binary_reader_readunsignedint16, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readInt, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readUnsignedInt, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readInt32, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readUnsignedInt32, arginfo_phalcon_binary_reader_readunsignedint32, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readFloat, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readDouble, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readString, arginfo_phalcon_binary_reader_readstring, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Reader, readHexString, arginfo_phalcon_binary_reader_readhexstring, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Binary initializer
 */
PHALCON_INIT_CLASS(Phalcon_Binary_Reader){

	PHALCON_REGISTER_CLASS(Phalcon\\Binary, Reader, binary_reader, phalcon_binary_reader_method_entry, 0);

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

	zval *data, *endian = NULL;

	phalcon_fetch_params(1, 1, 1, &data, &endian);

	if (Z_TYPE_P(data) == IS_STRING) {
		zval filename = {}, mode = {}, handler = {}, fstat = {}, size = {};
		phalcon_update_property(getThis(), SL("_data"), data);

		PHALCON_MM_ZVAL_STRING(&filename, "php://memory");
		PHALCON_MM_ZVAL_STRING(&mode, "br+");
		PHALCON_MM_CALL_FUNCTION(&handler, "fopen", &filename, &mode);
		PHALCON_MM_ADD_ENTRY(&handler);
		PHALCON_MM_CALL_FUNCTION(NULL, "fwrite", &handler, data);
		PHALCON_MM_CALL_FUNCTION(NULL, "rewind", &handler);

		PHALCON_MM_CALL_FUNCTION(&fstat, "fstat", &handler);
		PHALCON_MM_ADD_ENTRY(&fstat);
		if (phalcon_array_isset_fetch_str(&size, &fstat, SL("size"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_eofPosition"), &size);
		}
		phalcon_update_property(getThis(), SL("_input"), &handler);
	} else if (Z_TYPE_P(data) == IS_RESOURCE) {
		zval fstat = {}, size = {}, content = {};
		phalcon_update_property(getThis(), SL("_input"), data);

		PHALCON_MM_CALL_FUNCTION(&fstat, "fstat", data);
		PHALCON_MM_ADD_ENTRY(&fstat);
		if (phalcon_array_isset_fetch_str(&size, &fstat, SL("size"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_eofPosition"), &size);
		} else {
			ZVAL_LONG(&size, 0);
		}

		PHALCON_MM_CALL_FUNCTION(NULL, "rewind", data);
		PHALCON_MM_CALL_FUNCTION(&content, "fread", data, &size);
		PHALCON_MM_ADD_ENTRY(&content);
		PHALCON_MM_CALL_FUNCTION(NULL, "rewind", data);
		phalcon_update_property(getThis(), SL("_data"), &content);
	} else {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_binary_exception_ce, "Data must be set as string or resource");
		return;
	}

	if (endian && Z_TYPE_P(endian) != IS_NULL) {
		if (Z_TYPE_P(endian) != IS_LONG || Z_LVAL_P(endian) < 0 || Z_LVAL_P(endian) > 2) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_binary_exception_ce, "Endian must be set as big or little");
			return;
		}
		phalcon_update_property(getThis(), SL("_endian"), endian);
	}
	RETURN_MM();
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
 * Gets the binary data
 *
 * @return int
 */
PHP_METHOD(Phalcon_Binary_Reader, getContent){


	RETURN_MEMBER(getThis(), "_data");
}

/**
 * Sets the position in the the file pointer
 *
 * @param integer $position
 * @param integer $whence
 * @return boolean
 */
PHP_METHOD(Phalcon_Binary_Reader, setPosition){

	zval *position, *whence = NULL, input = {}, current_position = {}, eof_position = {}, pos = {};
	php_stream *stream;
	int seek_type = SEEK_SET, ret;

	phalcon_fetch_params(0, 1, 1, &position, &whence);

	if (whence && Z_TYPE_P(whence) == IS_LONG) {
		seek_type = Z_LVAL_P(whence);
	}

	phalcon_read_property(&input, getThis(), SL("_input"), PH_NOISY|PH_READONLY);
	php_stream_from_res(stream, Z_RES(input));
	if (seek_type == SEEK_CUR) {
		ret = php_stream_seek(stream, Z_LVAL_P(position), SEEK_CUR);
		if (ret >= 0) {
			phalcon_read_property(&current_position, getThis(), SL("_position"), PH_NOISY|PH_READONLY);
			ZVAL_LONG(&pos, Z_LVAL(current_position) + Z_LVAL_P(position));
			phalcon_update_property(getThis(), SL("_position"), &pos);
			RETURN_TRUE;
		}
	} else if (seek_type == SEEK_END) {
		ret = php_stream_seek(stream, Z_LVAL_P(position), SEEK_END);
		if (ret >= 0) {
			phalcon_read_property(&eof_position, getThis(), SL("_eofPosition"), PH_NOISY|PH_READONLY);
			ZVAL_LONG(&pos, Z_LVAL(eof_position) - Z_LVAL_P(position));
			phalcon_update_property(getThis(), SL("_position"), &pos);
			RETURN_TRUE;
		}
	} else {
		ret = php_stream_seek(stream, Z_LVAL_P(position), SEEK_SET);
		if (ret >= 0) {
			phalcon_update_property(getThis(), SL("_position"), position);
			RETURN_TRUE;
		}
	}


	RETURN_FALSE;
}

/**
 * Gets the current postion in the the file pointer
 *
 * @return int
 */
PHP_METHOD(Phalcon_Binary_Reader, getPosition){


	RETURN_MEMBER(getThis(), "_position");
}

/**
 * Gets the eof postion the file pointer
 *
 * @return int
 */
PHP_METHOD(Phalcon_Binary_Reader, getEofPosition){


	RETURN_MEMBER(getThis(), "_eofPosition");
}

/**
 * Checks if end of the file pointer was reached
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Binary_Reader, isEof){

	zval position = {}, eof_position = {};

	phalcon_read_property(&position, getThis(), SL("_position"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&eof_position, getThis(), SL("_eofPosition"), PH_NOISY|PH_READONLY);

	if (PHALCON_GE(&position, &eof_position)) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}

/**
 * Read num bytes from the current position in the file pointer
 *
 * @param integer $length
 * @return string
 */
PHP_METHOD(Phalcon_Binary_Reader, read){

	zval *length, position = {}, eof_position = {}, result = {}, input = {};

	phalcon_fetch_params(0, 1, 0, &length);

	phalcon_read_property(&position, getThis(), SL("_position"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&eof_position, getThis(), SL("_eofPosition"), PH_NOISY|PH_READONLY);
	phalcon_add_function(&result, &position, length);
	if (PHALCON_GT(&result, &eof_position)) {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_binary_exception_ce, "Outside of input, postion: %d, total length: %d", Z_LVAL(result), Z_LVAL(eof_position));
		return;
	}

	phalcon_read_property(&input, getThis(), SL("_input"), PH_NOISY|PH_READONLY);
	PHALCON_CALL_FUNCTION(return_value, "fread", &input, length);
	phalcon_update_property(getThis(), SL("_position"), &result);
}

/**
 * Read a signed char from the current position in the file pointer
 *
 * @return string
 */
PHP_METHOD(Phalcon_Binary_Reader, readChar){

	zval length = {}, bytes = {}, format = {}, result = {};

	PHALCON_MM_INIT();

	ZVAL_LONG(&length, 1);
	PHALCON_MM_CALL_METHOD(&bytes, getThis(), "read", &length);
	PHALCON_MM_ADD_ENTRY(&bytes);

	PHALCON_MM_ZVAL_STRING(&format, "c");
	PHALCON_MM_CALL_FUNCTION(&result, "unpack", &format, &bytes);
	PHALCON_MM_ADD_ENTRY(&result);

	if (!phalcon_array_isset_fetch_long(return_value, &result, 1, PH_COPY)) {
		RETURN_MM_NULL();
	}
	RETURN_MM();
}

/**
 * Read a unsigned char from the current position in the file pointer
 *
 * @return string
 */
PHP_METHOD(Phalcon_Binary_Reader, readUnsignedChar){

	zval length = {}, bytes = {}, format = {}, result = {};

	PHALCON_MM_INIT();

	ZVAL_LONG(&length, 1);
	PHALCON_MM_CALL_METHOD(&bytes, getThis(), "read", &length);
	PHALCON_MM_ADD_ENTRY(&bytes);

	PHALCON_MM_ZVAL_STRING(&format, "C");
	PHALCON_MM_CALL_FUNCTION(&result, "unpack", &format, &bytes);
	PHALCON_MM_ADD_ENTRY(&result);

	if (!phalcon_array_isset_fetch_long(return_value, &result, 1, PH_COPY)) {
		RETURN_MM_NULL();
	}
	RETURN_MM();
}

/**
 * Read a signed short int from the current position in the file pointer
 *
 * @return int
 */
PHP_METHOD(Phalcon_Binary_Reader, readInt16){

	zval length = {}, bytes = {}, format = {}, result = {};

	PHALCON_MM_INIT();

	ZVAL_LONG(&length, 2);
	PHALCON_MM_CALL_METHOD(&bytes, getThis(), "read", &length);
	PHALCON_MM_ADD_ENTRY(&bytes);

	PHALCON_MM_ZVAL_STRING(&format, "s");
	PHALCON_MM_CALL_FUNCTION(&result, "unpack", &format, &bytes);
	PHALCON_MM_ADD_ENTRY(&result);

	if (!phalcon_array_isset_fetch_long(return_value, &result, 1, PH_COPY)) {
		RETURN_MM_NULL();
	}
	RETURN_MM();
}

/**
 * Read a unsigned short int from the current position in the file pointer
 *
 * @return int
 */
PHP_METHOD(Phalcon_Binary_Reader, readUnsignedInt16){

	zval *_endian = NULL, length = {}, bytes = {}, endian = {}, format = {}, result = {};

	phalcon_fetch_params(1, 0, 1, &_endian);

	if (!_endian || Z_TYPE_P(_endian) == IS_NULL) {
		phalcon_read_property(&endian, getThis(), SL("_endian"), PH_NOISY|PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&endian, _endian);
	}

	ZVAL_LONG(&length, 2);
	PHALCON_MM_CALL_METHOD(&bytes, getThis(), "read", &length);
	PHALCON_MM_ADD_ENTRY(&bytes);

	if (Z_LVAL(endian) == PHALCON_BINARY_ENDIAN_BIG) {
		PHALCON_MM_ZVAL_STRING(&format, "n");
	} else if (Z_LVAL(endian) == PHALCON_BINARY_ENDIAN_LITTLE) {
		PHALCON_MM_ZVAL_STRING(&format, "v");
	} else if (Z_LVAL(endian) == PHALCON_BINARY_ENDIAN_MACHINE) {
		PHALCON_MM_ZVAL_STRING(&format, "S");
	}

	PHALCON_MM_CALL_FUNCTION(&result, "unpack", &format, &bytes);
	PHALCON_MM_ADD_ENTRY(&result);

	if (!phalcon_array_isset_fetch_long(return_value, &result, 1, PH_COPY)) {
		RETURN_MM_NULL();
	}
	RETURN_MM();
}

/**
 * Read a signed int from the current position in the file pointer
 *
 * @return int
 */
PHP_METHOD(Phalcon_Binary_Reader, readInt){

	zval length = {}, bytes = {}, format = {}, result = {};

	PHALCON_MM_INIT();

	ZVAL_LONG(&length, sizeof(int));
	PHALCON_MM_CALL_METHOD(&bytes, getThis(), "read", &length);
	PHALCON_MM_ADD_ENTRY(&bytes);

	PHALCON_MM_ZVAL_STRING(&format, "i");

	PHALCON_CALL_FUNCTION(&result, "unpack", &format, &bytes);
	PHALCON_MM_ADD_ENTRY(&result);

	if (!phalcon_array_isset_fetch_long(return_value, &result, 1, PH_COPY)) {
		RETURN_MM_NULL();
	}
	RETURN_MM();
}

/**
 * Read a unsigned int from the current position in the file pointer
 *
 * @return int
 */
PHP_METHOD(Phalcon_Binary_Reader, readUnsignedInt){

	zval length = {}, bytes = {}, format = {}, result = {};

	PHALCON_MM_INIT();

	ZVAL_LONG(&length, sizeof(int));
	PHALCON_MM_CALL_METHOD(&bytes, getThis(), "read", &length);
	PHALCON_MM_ADD_ENTRY(&bytes);

	PHALCON_MM_ZVAL_STRING(&format, "I");

	PHALCON_MM_CALL_FUNCTION(&result, "unpack", &format, &bytes);
	PHALCON_MM_ADD_ENTRY(&result);

	if (!phalcon_array_isset_fetch_long(return_value, &result, 1, PH_COPY)) {
		RETURN_MM_NULL();
	}
	RETURN_MM();
}

/**
 * Read a signed long int from the current position in the file pointer
 *
 * @return int
 */
PHP_METHOD(Phalcon_Binary_Reader, readInt32){

	zval length = {}, bytes = {}, format = {}, result = {};

	PHALCON_MM_INIT();

	ZVAL_LONG(&length, 4);
	PHALCON_MM_CALL_METHOD(&bytes, getThis(), "read", &length);
	PHALCON_MM_ADD_ENTRY(&bytes);

	PHALCON_MM_ZVAL_STRING(&format, "l");

	PHALCON_MM_CALL_FUNCTION(&result, "unpack", &format, &bytes);
	PHALCON_MM_ADD_ENTRY(&result);

	if (!phalcon_array_isset_fetch_long(return_value, &result, 1, PH_COPY)) {
		RETURN_MM_NULL();
	}
	RETURN_MM();
}

/**
 * Read a unsigned long int from the current position in the file pointer
 *
 * @return int
 */
PHP_METHOD(Phalcon_Binary_Reader, readUnsignedInt32){

	zval *_endian = NULL, length = {}, bytes = {}, endian = {}, format = {}, result = {};

	phalcon_fetch_params(1, 0, 1, &_endian);

	if (!_endian || Z_TYPE_P(_endian) == IS_NULL) {
		phalcon_read_property(&endian, getThis(), SL("_endian"), PH_NOISY|PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&endian, _endian);
	}

	ZVAL_LONG(&length, 4);
	PHALCON_MM_CALL_METHOD(&bytes, getThis(), "read", &length);
	PHALCON_MM_ADD_ENTRY(&bytes);

	if (Z_LVAL(endian) == PHALCON_BINARY_ENDIAN_BIG) {
		PHALCON_MM_ZVAL_STRING(&format, "N");
	} else if (Z_LVAL(endian) == PHALCON_BINARY_ENDIAN_LITTLE) {
		PHALCON_MM_ZVAL_STRING(&format, "V");
	} else if (Z_LVAL(endian) == PHALCON_BINARY_ENDIAN_MACHINE) {
		PHALCON_MM_ZVAL_STRING(&format, "L");
	}

	PHALCON_MM_CALL_FUNCTION(&result, "unpack", &format, &bytes);
	PHALCON_MM_ADD_ENTRY(&result);

	if (!phalcon_array_isset_fetch_long(return_value, &result, 1, PH_COPY)) {
		RETURN_MM_NULL();
	}
	RETURN_MM();
}

/**
 * Read a float from the current position in the file pointer
 *
 * @return float
 */
PHP_METHOD(Phalcon_Binary_Reader, readFloat){

	zval length = {}, bytes = {}, format = {}, result = {};

	PHALCON_MM_INIT();

	ZVAL_LONG(&length, sizeof(float));
	PHALCON_MM_CALL_METHOD(&bytes, getThis(), "read", &length);
	PHALCON_MM_ADD_ENTRY(&bytes);

	PHALCON_MM_ZVAL_STRING(&format, "f");

	PHALCON_MM_CALL_FUNCTION(&result, "unpack", &format, &bytes);
	PHALCON_MM_ADD_ENTRY(&result);

	if (!phalcon_array_isset_fetch_long(return_value, &result, 1, PH_COPY)) {
		RETURN_MM_NULL();
	}
	RETURN_MM();
}

/**
 * Read a double from the current position in the file pointer
 *
 * @return double
 */
PHP_METHOD(Phalcon_Binary_Reader, readDouble){

	zval length = {}, bytes = {}, format = {}, result = {};

	PHALCON_MM_INIT();

	ZVAL_LONG(&length, sizeof(double));
	PHALCON_MM_CALL_METHOD(&bytes, getThis(), "read", &length);
	PHALCON_MM_ADD_ENTRY(&bytes);

	PHALCON_MM_ZVAL_STRING(&format, "d");

	PHALCON_MM_CALL_FUNCTION(&result, "unpack", &format, &bytes);
	PHALCON_MM_ADD_ENTRY(&result);

	if (!phalcon_array_isset_fetch_long(return_value, &result, 1, PH_COPY)) {
		RETURN_MM_NULL();
	}
	RETURN_MM();
}

/**
 * Read string from the current position in the file pointer
 *
 * @return string
 */
PHP_METHOD(Phalcon_Binary_Reader, readString){

	zval *length = NULL, position = {}, eof_position = {}, format = {}, data = {}, result = {};

	phalcon_fetch_params(1, 0, 1, &length);

	if (length && Z_TYPE_P(length) != IS_NULL) {
		PHALCON_MM_CALL_METHOD(return_value, getThis(), "read", length);
		RETURN_MM();
	}

	phalcon_read_property(&position, getThis(), SL("_position"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&eof_position, getThis(), SL("_eofPosition"), PH_NOISY|PH_READONLY);
	if (PHALCON_GE(&position, &eof_position)) {
		PHALCON_MM_THROW_EXCEPTION_STR(phalcon_binary_exception_ce, "Not enough input");
		return;
	}
	PHALCON_CONCAT_SVS(&format, "@", &position, "/Z*");
	PHALCON_MM_ADD_ENTRY(&format);

	phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_READONLY);
	PHALCON_MM_CALL_FUNCTION(&result, "unpack", &format, &data);
	PHALCON_MM_ADD_ENTRY(&result);

	if (!phalcon_array_isset_fetch_long(return_value, &result, 1, PH_COPY)) {
		RETURN_MM_NULL();
	} else {
		zval len = {}, seek_type = {}, new_position = {};
		ZVAL_LONG(&len, Z_STRLEN_P(return_value));
		phalcon_add_function(&new_position, &position, &len);
		if (!PHALCON_GE(&new_position, &eof_position)) {
			ZVAL_LONG(&len, Z_STRLEN_P(return_value) + 1);
		}

		ZVAL_LONG(&seek_type, SEEK_CUR);
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "setposition", &len, &seek_type);
	}
	RETURN_MM();
}

/**
 * Read hex string from the current position in the file pointer
 *
 * @return string
 */
PHP_METHOD(Phalcon_Binary_Reader, readHexString){

	zval *length = NULL, *low_nibble = NULL, endian = {}, position = {}, eof_position = {}, format = {}, data = {}, result = {}, len = {}, seek_type = {};

	phalcon_fetch_params(1, 0, 2, &length, &low_nibble);

	if (!low_nibble || Z_TYPE_P(low_nibble) == IS_NULL) {
		phalcon_read_property(&endian, getThis(), SL("_endian"), PH_NOISY|PH_READONLY);
		if (Z_LVAL(endian) == PHALCON_BINARY_ENDIAN_LITTLE) {
			low_nibble = &PHALCON_GLOBAL(z_true);
		} else {
			low_nibble = &PHALCON_GLOBAL(z_false);
		}
	}

	if (length && Z_TYPE_P(length) != IS_NULL) {
		PHALCON_MM_CALL_METHOD(&data, getThis(), "read", length);

		// 低位是否在前半字节
		if (zend_is_true(low_nibble)) {
			PHALCON_CONCAT_SV(&format, "h", length);
		} else {
			PHALCON_CONCAT_SV(&format, "H", length);
		}
		PHALCON_MM_ADD_ENTRY(&format);
		PHALCON_MM_CALL_FUNCTION(&result, "unpack", &format, &data);
		PHALCON_MM_ADD_ENTRY(&result);
		if (!phalcon_array_isset_fetch_long(return_value, &result, 1, PH_COPY)) {
			RETURN_MM_NULL();
		}
	} else {
		phalcon_read_property(&position, getThis(), SL("_position"), PH_NOISY|PH_READONLY);
		phalcon_read_property(&eof_position, getThis(), SL("_eofPosition"), PH_NOISY|PH_READONLY);
		if (PHALCON_GE(&position, &eof_position)) {
			PHALCON_MM_THROW_EXCEPTION_STR(phalcon_binary_exception_ce, "Not enough input");
			return;
		}
		// 低位是否在前半字节
		if (zend_is_true(low_nibble)) {
			PHALCON_CONCAT_SVS(&format, "@", &position, "/h*");
		} else {
			PHALCON_CONCAT_SVS(&format, "@", &position, "/H*");
		}
		PHALCON_MM_ADD_ENTRY(&format);

		phalcon_read_property(&data, getThis(), SL("_data"), PH_NOISY|PH_COPY);
		PHALCON_MM_CALL_FUNCTION(&result, "unpack", &format, &data);
		if (!phalcon_array_isset_fetch_long(return_value, &result, 1, PH_COPY)) {
			RETURN_MM_NULL();
		}
		ZVAL_LONG(&len, Z_STRLEN_P(return_value)/2);
		ZVAL_LONG(&seek_type, SEEK_CUR);
		PHALCON_MM_CALL_METHOD(NULL, getThis(), "setposition", &len, &seek_type);
	}
	RETURN_MM();
}
