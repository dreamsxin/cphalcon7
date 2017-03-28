
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

#include "kernel/exception.h"

#include <Zend/zend_exceptions.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"


/**
 * Throws a zval object as exception
 */
void phalcon_throw_exception(zval *object){
	Z_TRY_ADDREF_P(object);
	zend_throw_exception_object(object);
}

/**
 * Throws a zval object as exception
 */
void phalcon_throw_exception_debug(zval *object, const char *file, uint32_t line)
{
	zval curline = {}, exception = {};
	zend_class_entry *default_exception_ce;

	if (!object || Z_TYPE_P(object) != IS_OBJECT) {
		object_init_ex(&exception, zend_exception_get_default());
		PHALCON_CALL_METHOD(NULL, &exception, "__construct", &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_zero), object);
	} else {
		ZVAL_COPY_VALUE(&exception, object);
	}

	if (line > 0) {
		PHALCON_CALL_METHOD(&curline, &exception, "getline");
		if (PHALCON_IS_LONG(&curline, 0)) {
			default_exception_ce = zend_exception_get_default();
			zend_update_property_string(default_exception_ce, &exception, "file", sizeof("file")-1, file);
			zend_update_property_long(default_exception_ce, &exception, "line", sizeof("line")-1, line);
		}
	}

	zend_throw_exception_object(&exception);
}

/**
 * Throws an exception with a single string parameter
 */
void phalcon_throw_exception_string(zend_class_entry *ce, const char *message){

	zend_throw_exception_ex(ce, 0, "%s", message);
}

/**
 * Throws an exception with a single string parameter + debug info
 */
void phalcon_throw_exception_string_debug(zend_class_entry *ce, const char *message, uint32_t message_len, const char *file, uint32_t line)
{
	zval object = {}, msg = {};
	zend_class_entry *default_exception_ce;

	object_init_ex(&object, ce);

	ZVAL_STRINGL(&msg, message, message_len);

	PHALCON_CALL_METHOD(NULL, &object, "__construct", &PHALCON_GLOBAL(z_null), &msg);

	if (line > 0) {
		default_exception_ce = zend_exception_get_default();
		zend_update_property_string(default_exception_ce, &object, "file", sizeof("file")-1, file);
		zend_update_property_long(default_exception_ce, &object, "line", sizeof("line")-1, line);
	}

	zend_throw_exception_object(&object);
	zval_dtor(&msg);
}

/**
 * Throws an exception with a single zval parameter
 */
void phalcon_throw_exception_zval(zend_class_entry *ce, zval *message){

	zval object = {};
	object_init_ex(&object, ce);

	PHALCON_CALL_METHOD(NULL, &object, "__construct", message);
	zend_throw_exception_object(&object);
}

/**
 * Throws an exception with a single zval parameter
 */
void phalcon_throw_exception_zval_debug(zend_class_entry *ce, zval *message, const char *file, uint32_t line){

	zval object = {};
	zend_class_entry *default_exception_ce;

	object_init_ex(&object, ce);

	PHALCON_CALL_METHOD(NULL, &object, "__construct", &PHALCON_GLOBAL(z_null), message);

	if (line > 0) {
		default_exception_ce = zend_exception_get_default();
		zend_update_property_string(default_exception_ce, &object, "file", sizeof("file")-1, file);
		zend_update_property_long(default_exception_ce, &object, "line", sizeof("line")-1, line);
	}

	zend_throw_exception_object(&object);
}

/**
 * Throws an exception with a string format as parameter
 */
void phalcon_throw_exception_format(zend_class_entry *ce, const char *format, ...) {

	zval object = {}, msg = {};
	int len;
	char *buffer;
	va_list args;

	object_init_ex(&object, ce);

	va_start(args, format);
	len = vspprintf(&buffer, 0, format, args);
	va_end(args);

	ZVAL_STRINGL(&msg, buffer, len);

	PHALCON_CALL_METHOD(NULL, &object, "__construct", &msg);

	zend_throw_exception_object(&object);
	zval_dtor(&msg);
}
