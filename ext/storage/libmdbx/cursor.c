
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

#include "storage/libmdbx/cursor.h"
#include "storage/exception.h"

#include "kernel/main.h"
#include "kernel/exception.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"
#include "kernel/file.h"
#include "kernel/variables.h"

#include "internal/arginfo.h"

/**
 * Phalcon\Storage\Libmdbx\Cursor
 *
 */
zend_class_entry *phalcon_storage_libmdbx_cursor_ce;

PHP_METHOD(Phalcon_Storage_Libmdbx_Cursor, __construct);
PHP_METHOD(Phalcon_Storage_Libmdbx_Cursor, current);
PHP_METHOD(Phalcon_Storage_Libmdbx_Cursor, key);
PHP_METHOD(Phalcon_Storage_Libmdbx_Cursor, next);
PHP_METHOD(Phalcon_Storage_Libmdbx_Cursor, prev);
PHP_METHOD(Phalcon_Storage_Libmdbx_Cursor, rewind);
PHP_METHOD(Phalcon_Storage_Libmdbx_Cursor, last);
PHP_METHOD(Phalcon_Storage_Libmdbx_Cursor, valid);

static const zend_function_entry phalcon_storage_libmdbx_cursor_method_entry[] = {
	PHP_ME(Phalcon_Storage_Libmdbx_Cursor, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR|ZEND_ACC_FINAL)
	PHP_ME(Phalcon_Storage_Libmdbx_Cursor, current, arginfo_iterator_current, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Libmdbx_Cursor, key, arginfo_iterator_key, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Libmdbx_Cursor, next, arginfo_iterator_next, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Libmdbx_Cursor, prev, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Libmdbx_Cursor, rewind, arginfo_iterator_rewind, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Libmdbx_Cursor, last, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Libmdbx_Cursor, valid, arginfo_iterator_valid, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_object_handlers phalcon_storage_libmdbx_cursor_object_handlers;
zend_object* phalcon_storage_libmdbx_cursor_object_create_handler(zend_class_entry *ce)
{
	phalcon_storage_libmdbx_cursor_object *intern = ecalloc(1, sizeof(phalcon_storage_libmdbx_cursor_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_storage_libmdbx_cursor_object_handlers;

	return &intern->std;
}

void phalcon_storage_libmdbx_cursor_object_free_handler(zend_object *object)
{
	phalcon_storage_libmdbx_cursor_object *intern = phalcon_storage_libmdbx_cursor_object_from_obj(object);
	if (intern->cursor) {
		mdbx_cursor_close(intern->cursor);
	}
}

/**
 * Phalcon\Storage\Libmdbx\Cursor initializer
 */
PHALCON_INIT_CLASS(Phalcon_Storage_Libmdbx_Cursor){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Storage\\Libmdbx, Cursor, storage_libmdbx_cursor, phalcon_storage_libmdbx_cursor_method_entry, 0);

	zend_class_implements(phalcon_storage_libmdbx_cursor_ce, 1, zend_ce_iterator);

	return SUCCESS;
}

/**
 * Phalcon\Storage\Lmdb\Cursor constructor
 *
 */
PHP_METHOD(Phalcon_Storage_Libmdbx_Cursor, __construct)
{
	/* this constructor shouldn't be called as it's private */
	zend_throw_exception(NULL, "An object of this type cannot be created with the new operator.", 0);
}

/**
 * Gets current value
 *
 * @return string
 */
PHP_METHOD(Phalcon_Storage_Libmdbx_Cursor, current)
{
	phalcon_storage_libmdbx_cursor_object *intern;

	intern = phalcon_storage_libmdbx_cursor_object_from_obj(Z_OBJ_P(getThis()));

	if (!intern->start) {
		intern->rc = mdbx_cursor_get(intern->cursor, &intern->k, &intern->v, MDBX_NEXT);
		intern->start = 1;
	}

	if (intern->rc == MDBX_SUCCESS) {
		zval s = {};
		ZVAL_STRINGL(&s, (char *) intern->v.iov_base, (int) intern->v.iov_len);
		phalcon_unserialize(return_value, &s);
	} else if (intern->rc == MDBX_NOTFOUND) {
		RETVAL_FALSE;
	} else {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Failed to get item from a database (%s)", mdbx_strerror(intern->rc));
		return;
	}
}

/**
 * Gets current key
 *
 * @return string
 */
PHP_METHOD(Phalcon_Storage_Libmdbx_Cursor, key)
{
	phalcon_storage_libmdbx_cursor_object *intern;

	intern = phalcon_storage_libmdbx_cursor_object_from_obj(Z_OBJ_P(getThis()));

	if (!intern->start) {
		intern->rc = mdbx_cursor_get(intern->cursor, &intern->k, &intern->v, MDBX_NEXT);
		intern->start = 1;
	}

	if (intern->rc == MDBX_SUCCESS) {
		ZVAL_STRINGL(return_value, (char *) intern->k.iov_base, (int) intern->k.iov_len);
	} else if (intern->rc == MDBX_NOTFOUND) {
		RETVAL_FALSE;
	} else {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Failed to get key from a database (%s)", mdbx_strerror(intern->rc));
		return;
	}
}

/**
 * Moves cursor to next row
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Libmdbx_Cursor, next)
{
	phalcon_storage_libmdbx_cursor_object *intern;

	intern = phalcon_storage_libmdbx_cursor_object_from_obj(Z_OBJ_P(getThis()));
    intern->rc = mdbx_cursor_get(intern->cursor, &intern->k, &intern->v, MDBX_NEXT);
	intern->start = 1;
	if (intern->rc == MDBX_SUCCESS) {
		RETURN_TRUE;
	} else if (intern->rc == MDBX_NOTFOUND) {
		RETVAL_FALSE;
	} else {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Failed to move next (%s)", mdbx_strerror(intern->rc));
		return;
	}
}

/**
 * Moves cursor to prev row
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Libmdbx_Cursor, prev)
{
	phalcon_storage_libmdbx_cursor_object *intern;

	intern = phalcon_storage_libmdbx_cursor_object_from_obj(Z_OBJ_P(getThis()));

    intern->rc = mdbx_cursor_get(intern->cursor, &intern->k, &intern->v, MDBX_PREV);
	intern->start = 1;
	if (intern->rc == MDBX_SUCCESS) {
		RETURN_TRUE;
	} else if (intern->rc == MDBX_NOTFOUND) {
		RETVAL_FALSE;
	} else {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Failed to move prev (%s)", mdbx_strerror(intern->rc));
		return;
	}
}

/**
 * Rewinds cursor to it's beginning
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Libmdbx_Cursor, rewind)
{
	phalcon_storage_libmdbx_cursor_object *intern;

	intern = phalcon_storage_libmdbx_cursor_object_from_obj(Z_OBJ_P(getThis()));

    intern->rc = mdbx_cursor_get(intern->cursor, &intern->k, &intern->v, MDBX_FIRST);
	intern->start = 1;
	if (intern->rc == MDBX_SUCCESS) {
		RETURN_TRUE;
	} else if (intern->rc == MDBX_NOTFOUND) {
		RETVAL_FALSE;
	} else {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Failed to rewind (%s)", mdbx_strerror(intern->rc));
		return;
	}
}

/**
 * Rewinds cursor to it's last
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Libmdbx_Cursor, last)
{
	phalcon_storage_libmdbx_cursor_object *intern;

	intern = phalcon_storage_libmdbx_cursor_object_from_obj(Z_OBJ_P(getThis()));

    intern->rc = mdbx_cursor_get(intern->cursor, &intern->k, &intern->v, MDBX_LAST);
	intern->start = 1;
	if (intern->rc == MDBX_SUCCESS) {
		RETURN_TRUE;
	} else if (intern->rc == MDBX_NOTFOUND) {
		RETVAL_FALSE;
	} else {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Failed to move last (%s)", mdbx_strerror(intern->rc));
		return;
	}
}

/**
 * Checks if current position is valid
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Libmdbx_Cursor, valid)
{
	phalcon_storage_libmdbx_cursor_object *intern;

	intern = phalcon_storage_libmdbx_cursor_object_from_obj(Z_OBJ_P(getThis()));

	if (!intern->start) {
		intern->rc = mdbx_cursor_get(intern->cursor, &intern->k, &intern->v, MDBX_NEXT);
		intern->start = 1;
	}
	if (intern->rc == MDBX_SUCCESS) {
		RETVAL_TRUE;
	} else if (intern->rc == MDBX_NOTFOUND) {
		RETVAL_FALSE;
	} else {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Failed to valid (%s)", mdbx_strerror(intern->rc));
		return;
	}
}
