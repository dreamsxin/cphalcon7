
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

#include "binary/writer.h"
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
 * Phalcon\Binary\Writer
 *
 * Provides utilities to work with binary data
 *
 *<code>
 *	$fp = fopen('unit-tests/assets/data.bin', 'wb');
 *	$bin = new Phalcon\Binary\Writer($fp);
 *	$bin->writeUnsignedChar(1);
 *</code>
 */
zend_class_entry *phalcon_binary_writer_ce;

PHP_METHOD(Phalcon_Binary_Writer, __construct);
PHP_METHOD(Phalcon_Binary_Writer, getEndian);
PHP_METHOD(Phalcon_Binary_Writer, getOutput);
PHP_METHOD(Phalcon_Binary_Writer, getContent);
PHP_METHOD(Phalcon_Binary_Writer, getPosition);
PHP_METHOD(Phalcon_Binary_Writer, write);
PHP_METHOD(Phalcon_Binary_Writer, writeChar);
PHP_METHOD(Phalcon_Binary_Writer, writeUnsignedChar);
PHP_METHOD(Phalcon_Binary_Writer, writeInt16);
PHP_METHOD(Phalcon_Binary_Writer, writeUnsignedInt16);
PHP_METHOD(Phalcon_Binary_Writer, writeInt);
PHP_METHOD(Phalcon_Binary_Writer, writeUnsignedInt);
PHP_METHOD(Phalcon_Binary_Writer, writeInt32);
PHP_METHOD(Phalcon_Binary_Writer, writeUnsignedInt32);
PHP_METHOD(Phalcon_Binary_Writer, writeFloat);
PHP_METHOD(Phalcon_Binary_Writer, writeDouble);
PHP_METHOD(Phalcon_Binary_Writer, writeString);
PHP_METHOD(Phalcon_Binary_Writer, writeHexString);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_writer___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_TYPE_INFO(0, endian, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_writer_write, 0, 0, 2)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_writer_writechar, 0, 0, 1)
	ZEND_ARG_INFO(0, byte)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_writer_writeunsignedchar, 0, 0, 1)
	ZEND_ARG_INFO(0, byte)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_writer_writeint16, 0, 0, 1)
	ZEND_ARG_INFO(0, num)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_writer_writeunsignedint16, 0, 0, 1)
	ZEND_ARG_INFO(0, num)
	ZEND_ARG_TYPE_INFO(0, endian, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_writer_writeint, 0, 0, 1)
	ZEND_ARG_INFO(0, num)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_writer_writeunsignedint, 0, 0, 1)
	ZEND_ARG_INFO(0, num)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_writer_writeint32, 0, 0, 1)
	ZEND_ARG_INFO(0, num)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_writer_writeunsignedint32, 0, 0, 1)
	ZEND_ARG_INFO(0, num)
	ZEND_ARG_TYPE_INFO(0, endian, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_writer_writefloat, 0, 0, 1)
	ZEND_ARG_INFO(0, num)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_writer_writedouble, 0, 0, 1)
	ZEND_ARG_INFO(0, num)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_writer_writestring, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 1)
	ZEND_ARG_INFO(0, exact)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_binary_writer_writehexstring, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, lowNibble, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_binary_writer_method_entry[] = {
	PHP_ME(Phalcon_Binary_Writer, __construct, arginfo_phalcon_binary_writer___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Binary_Writer, getEndian, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Writer, getOutput, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Writer, getContent, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Writer, getPosition, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Writer, write, arginfo_phalcon_binary_writer_write, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Writer, writeChar, arginfo_phalcon_binary_writer_writechar, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Writer, writeUnsignedChar, arginfo_phalcon_binary_writer_writeunsignedchar, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Writer, writeInt16, arginfo_phalcon_binary_writer_writeint16, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Writer, writeUnsignedInt16, arginfo_phalcon_binary_writer_writeunsignedint16, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Writer, writeInt, arginfo_phalcon_binary_writer_writeint, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Writer, writeUnsignedInt, arginfo_phalcon_binary_writer_writeunsignedint, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Writer, writeInt32, arginfo_phalcon_binary_writer_writeint32, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Writer, writeUnsignedInt32, arginfo_phalcon_binary_writer_writeunsignedint32, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Writer, writeFloat, arginfo_phalcon_binary_writer_writefloat, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Writer, writeDouble, arginfo_phalcon_binary_writer_writedouble, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Writer, writeString, arginfo_phalcon_binary_writer_writestring, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Binary_Writer, writeHexString, arginfo_phalcon_binary_writer_writehexstring, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Binary initializer
 */
PHALCON_INIT_CLASS(Phalcon_Binary_Writer){

	PHALCON_REGISTER_CLASS(Phalcon\\Binary, Writer, binary_writer, phalcon_binary_writer_method_entry, 0);

	zend_declare_property_long(phalcon_binary_writer_ce, SL("_endian"), PHALCON_BINARY_ENDIAN_BIG, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_binary_writer_ce, SL("_output"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_binary_writer_ce, SL("_position"), 0, ZEND_ACC_PROTECTED);
	return SUCCESS;
}

/**
 * Phalcon\Binary\Writer constructor
 *
 * @param  string|resource $data
 * @param  int $endian
 * @throws \InvalidArgumentException
 */
PHP_METHOD(Phalcon_Binary_Writer, __construct){

	zval *data = NULL, *endian = NULL;

	phalcon_fetch_params(1, 0, 2, &data, &endian);

	if (!data) {
		data = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(data) == IS_STRING || Z_TYPE_P(data) == IS_NULL) {
		zval filename = {}, mode = {}, handler = {}, fstat = {}, size = {};
		PHALCON_MM_ZVAL_STRING(&filename, "php://memory");
		PHALCON_MM_ZVAL_STRING(&mode, "br+");
		PHALCON_MM_CALL_FUNCTION(&handler, "fopen", &filename, &mode);
		PHALCON_MM_ADD_ENTRY(&handler);

		PHALCON_MM_CALL_FUNCTION(NULL, "fwrite", &handler, data);
		PHALCON_MM_CALL_FUNCTION(&fstat, "fstat", &handler);
		PHALCON_MM_ADD_ENTRY(&fstat);

		if (phalcon_array_isset_fetch_str(&size, &fstat, SL("size"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_position"), &size);
		}
		phalcon_update_property(getThis(), SL("_output"), &handler);
	} else if (Z_TYPE_P(data) == IS_RESOURCE) {
		zval fstat = {}, size = {};
		phalcon_update_property(getThis(), SL("_output"), data);

		PHALCON_MM_CALL_FUNCTION(&fstat, "fstat", data);
		PHALCON_MM_ADD_ENTRY(&fstat);
		if (phalcon_array_isset_fetch_str(&size, &fstat, SL("size"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_position"), &size);
		}
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
PHP_METHOD(Phalcon_Binary_Writer, getEndian){


	RETURN_MEMBER(getThis(), "_endian");
}

/**
 * Gets the ouput
 *
 * @return int
 */
PHP_METHOD(Phalcon_Binary_Writer, getOutput){


	RETURN_MEMBER(getThis(), "_output");
}

/**
 * Gets the ouput
 *
 * @return int
 */
PHP_METHOD(Phalcon_Binary_Writer, getContent){

	zval output = {}, position = {};

	phalcon_read_property(&output, getThis(), SL("_output"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&position, getThis(), SL("_position"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_FUNCTION(NULL, "rewind", &output);
	PHALCON_CALL_FUNCTION(return_value, "fread", &output, &position);
}

/**
 * Gets the current postion
 *
 * @return int
 */
PHP_METHOD(Phalcon_Binary_Writer, getPosition){


	RETURN_MEMBER(getThis(), "_position");
}

/**
 * Write bytes to the current position in the file pointer
 *
 * @return Phalcon\Binary\Writer
 */
PHP_METHOD(Phalcon_Binary_Writer, write){

	zval *data, *length, position = {}, result = {}, output = {};

	phalcon_fetch_params(0, 2, 0, &data, &length);

	phalcon_read_property(&position, getThis(), SL("_position"), PH_NOISY|PH_READONLY);
	phalcon_add_function(&result, &position, length);

	phalcon_read_property(&output, getThis(), SL("_output"), PH_NOISY|PH_READONLY);
	PHALCON_CALL_FUNCTION(return_value, "fwrite", &output, data, length);
	phalcon_update_property(getThis(), SL("_position"), &result);
}

/**
 * Write a signed char to the current position in the file pointer
 *
 * @return Phalcon\Binary\Writer
 */
PHP_METHOD(Phalcon_Binary_Writer, writeChar){

	zval *byte, length = {}, format = {}, result = {};

	phalcon_fetch_params(1, 1, 0, &byte);

	ZVAL_LONG(&length, 1);

	PHALCON_MM_ZVAL_STRING(&format, "c");
	PHALCON_CALL_FUNCTION(&result, "pack", &format, byte);
	PHALCON_MM_ADD_ENTRY(&result);

	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &result, &length);

	RETURN_MM_THIS();
}

/**
 * Write a unsigned char to the current position in the file pointer
 *
 * @return Phalcon\Binary\Writer
 */
PHP_METHOD(Phalcon_Binary_Writer, writeUnsignedChar){

	zval *byte, length = {}, format = {}, result = {};

	phalcon_fetch_params(1, 1, 0, &byte);

	ZVAL_LONG(&length, 1);

	PHALCON_MM_ZVAL_STRING(&format, "C");
	PHALCON_CALL_FUNCTION(&result, "pack", &format, byte);
	PHALCON_MM_ADD_ENTRY(&result);

	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &result, &length);

	RETURN_MM_THIS();
}

/**
 * Write a signed short int to the current position in the file pointer
 *
 * @return Phalcon\Binary\Writer
 */
PHP_METHOD(Phalcon_Binary_Writer, writeInt16){

	zval *num, length = {}, format = {}, result = {};

	phalcon_fetch_params(1, 1, 0, &num);

	ZVAL_LONG(&length, 2);

	PHALCON_MM_ZVAL_STRING(&format, "s");
	PHALCON_CALL_FUNCTION(&result, "pack", &format, num);
	PHALCON_MM_ADD_ENTRY(&result);

	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &result, &length);

	RETURN_MM_THIS();
}

/**
 * Write a unsigned short int to the current position in the file pointer
 *
 * @return Phalcon\Binary\Writer
 */
PHP_METHOD(Phalcon_Binary_Writer, writeUnsignedInt16){

	zval *_endian = NULL, *num, length = {}, endian = {}, format = {}, result = {};

	phalcon_fetch_params(1, 1, 1, &num, &_endian);

	if (!_endian || Z_TYPE_P(_endian) == IS_NULL) {
		phalcon_read_property(&endian, getThis(), SL("_endian"), PH_NOISY|PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&endian, _endian);
	}

	ZVAL_LONG(&length, 2);

	if (Z_LVAL(endian) == PHALCON_BINARY_ENDIAN_BIG) {
		ZVAL_STRING(&format, "n");
	} else if (Z_LVAL(endian) == PHALCON_BINARY_ENDIAN_LITTLE) {
		ZVAL_STRING(&format, "v");
	} else if (Z_LVAL(endian) == PHALCON_BINARY_ENDIAN_MACHINE) {
		ZVAL_STRING(&format, "S");
	}
	PHALCON_MM_ADD_ENTRY(&format);

	PHALCON_MM_CALL_FUNCTION(&result, "pack", &format, num);
	PHALCON_MM_ADD_ENTRY(&result);

	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &result, &length);

	RETURN_MM_THIS();
}

/**
 * Write a signed int to the current position in the file pointer
 *
 * @return Phalcon\Binary\Writer
 */
PHP_METHOD(Phalcon_Binary_Writer, writeInt){

	zval *num, length = {}, format = {}, result = {};

	phalcon_fetch_params(1, 1, 0, &num);

	ZVAL_LONG(&length, sizeof(int));

	PHALCON_MM_ZVAL_STRING(&format, "i");
	PHALCON_MM_CALL_FUNCTION(&result, "pack", &format, num);
	PHALCON_MM_ADD_ENTRY(&result);

	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &result, &length);

	RETURN_MM_THIS();
}

/**
 * Write a unsigned int to the current position in the file pointer
 *
 * @return Phalcon\Binary\Writer
 */
PHP_METHOD(Phalcon_Binary_Writer, writeUnsignedInt){

	zval *num, length = {}, format = {}, result = {};

	phalcon_fetch_params(1, 1, 0, &num);

	ZVAL_LONG(&length, sizeof(int));

	PHALCON_MM_ZVAL_STRING(&format, "I");
	PHALCON_MM_CALL_FUNCTION(&result, "pack", &format, num);
	PHALCON_MM_ADD_ENTRY(&result);

	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &result, &length);

	RETURN_MM_THIS();
}

/**
 * Write a signed long int to the current position in the file pointer
 *
 * @return Phalcon\Binary\Writer
 */
PHP_METHOD(Phalcon_Binary_Writer, writeInt32){

	zval *num, length = {}, format = {}, result = {};

	phalcon_fetch_params(1, 1, 0, &num);

	ZVAL_LONG(&length, 4);

	PHALCON_MM_ZVAL_STRING(&format, "l");
	PHALCON_MM_CALL_FUNCTION(&result, "pack", &format, num);
	PHALCON_MM_ADD_ENTRY(&result);

	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &result, &length);

	RETURN_MM_THIS();
}

/**
 * Write a unsigned long int to the current position in the file pointer
 *
 * @return Phalcon\Binary\Writer
 */
PHP_METHOD(Phalcon_Binary_Writer, writeUnsignedInt32){

	zval *_endian = NULL, *num, length = {}, endian = {}, format = {}, result = {};

	phalcon_fetch_params(1, 1, 1, &num, &_endian);

	if (!_endian || Z_TYPE_P(_endian) == IS_NULL) {
		phalcon_read_property(&endian, getThis(), SL("_endian"), PH_NOISY|PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&endian, _endian);
	}

	ZVAL_LONG(&length, 4);

	if (Z_LVAL(endian) == PHALCON_BINARY_ENDIAN_BIG) {
		ZVAL_STRING(&format, "N");
	} else if (Z_LVAL(endian) == PHALCON_BINARY_ENDIAN_LITTLE) {
		ZVAL_STRING(&format, "V");
	} else if (Z_LVAL(endian) == PHALCON_BINARY_ENDIAN_MACHINE) {
		ZVAL_STRING(&format, "L");
	}
	PHALCON_MM_ADD_ENTRY(&format);

	PHALCON_MM_CALL_FUNCTION(&result, "pack", &format, num);
	PHALCON_MM_ADD_ENTRY(&result);

	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &result, &length);

	RETURN_MM_THIS();
}

/**
 * Write a float to the current position in the file pointer
 *
 * @return Phalcon\Binary\Writer
 */
PHP_METHOD(Phalcon_Binary_Writer, writeFloat){

	zval *num, length = {}, format = {}, result = {};

	phalcon_fetch_params(1, 1, 0, &num);

	ZVAL_LONG(&length, sizeof(float));

	PHALCON_MM_ZVAL_STRING(&format, "f");
	PHALCON_MM_CALL_FUNCTION(&result, "pack", &format, num);
	PHALCON_MM_ADD_ENTRY(&result);

	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &result, &length);

	RETURN_MM_THIS();
}

/**
 * Write a double to the current position in the file pointer
 *
 * @return Phalcon\Binary\Writer
 */
PHP_METHOD(Phalcon_Binary_Writer, writeDouble){

	zval *num, length = {}, format = {}, result = {};

	phalcon_fetch_params(1, 1, 0, &num);

	ZVAL_LONG(&length, sizeof(double));

	PHALCON_MM_ZVAL_STRING(&format, "d");
	PHALCON_MM_CALL_FUNCTION(&result, "pack", &format, num);
	PHALCON_MM_ADD_ENTRY(&result);

	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &result, &length);

	RETURN_MM_THIS();
}

/**
 * Write string to the current position in the file pointer
 *
 * @return Phalcon\Binary\Writer
 */
PHP_METHOD(Phalcon_Binary_Writer, writeString){

	zval *str, *length = NULL, *exact = NULL, len = {}, format = {}, result = {};

	phalcon_fetch_params(1, 1, 2, &str, &length, &exact);

	if (length && Z_TYPE_P(length) != IS_NULL) {
		if (exact && zend_is_true(exact)) {
			PHALCON_CONCAT_SV(&format, "a", length);
		} else {
			PHALCON_CONCAT_SV(&format, "Z", length);
		}
	} else {
		if (exact && zend_is_true(exact)) {
			ZVAL_STRING(&format, "a*");
		} else {
			ZVAL_STRING(&format, "Z*");
		}
	}
	PHALCON_MM_ADD_ENTRY(&format);
	PHALCON_CALL_FUNCTION(&result, "pack", &format, str);
	PHALCON_MM_ADD_ENTRY(&result);

	ZVAL_LONG(&len, Z_STRLEN(result));

	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &result, &len);

	RETURN_MM_THIS();
}

/**
 * Write hex string to the current position in the file pointer
 *
 * @return Phalcon\Binary\Writer
 */
PHP_METHOD(Phalcon_Binary_Writer, writeHexString){

	zval *str, *length = NULL, *low_nibble = NULL, endian = {}, len = {}, format = {}, result = {};

	phalcon_fetch_params(1, 1, 2, &str, &length, &low_nibble);

	if (!low_nibble || Z_TYPE_P(low_nibble) == IS_NULL) {
		phalcon_read_property(&endian, getThis(), SL("_endian"), PH_NOISY|PH_READONLY);
		if (Z_LVAL(endian) == PHALCON_BINARY_ENDIAN_LITTLE) {
			low_nibble = &PHALCON_GLOBAL(z_true);
		} else {
			low_nibble = &PHALCON_GLOBAL(z_false);
		}
	}

	if (length && Z_TYPE_P(length) != IS_NULL) {
		if (zend_is_true(low_nibble)) {
			PHALCON_CONCAT_SV(&format, "h", length);
		} else {
			PHALCON_CONCAT_SV(&format, "H", length);
		}
	} else {
		if (zend_is_true(low_nibble)) {
			ZVAL_STRING(&format, "h*");
		} else {
			ZVAL_STRING(&format, "H*");
		}
	}
	PHALCON_MM_ADD_ENTRY(&format);
	PHALCON_MM_CALL_FUNCTION(&result, "pack", &format, str);
	PHALCON_MM_ADD_ENTRY(&result);

	ZVAL_LONG(&len, Z_STRLEN(result));

	PHALCON_MM_CALL_METHOD(NULL, getThis(), "write", &result, &len);

	RETURN_MM_THIS();
}
