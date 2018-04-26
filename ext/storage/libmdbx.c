
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

#include "storage/libmdbx.h"
#include "storage/libmdbx/cursor.h"
#include "storage/exception.h"

#include <ext/standard/file.h>
#include <zend_smart_str.h>

#include "kernel/main.h"
#include "kernel/exception.h"
#include "kernel/memory.h"
#include "kernel/variables.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"
#include "kernel/file.h"

/**
 * Phalcon\Storage\Libmdbx
 *
 */
zend_class_entry *phalcon_storage_libmdbx_ce;

PHP_METHOD(Phalcon_Storage_Libmdbx, __construct);
PHP_METHOD(Phalcon_Storage_Libmdbx, begin);
PHP_METHOD(Phalcon_Storage_Libmdbx, commit);
PHP_METHOD(Phalcon_Storage_Libmdbx, renew);
PHP_METHOD(Phalcon_Storage_Libmdbx, reset);
PHP_METHOD(Phalcon_Storage_Libmdbx, getAll);
PHP_METHOD(Phalcon_Storage_Libmdbx, get);
PHP_METHOD(Phalcon_Storage_Libmdbx, put);
PHP_METHOD(Phalcon_Storage_Libmdbx, del);
PHP_METHOD(Phalcon_Storage_Libmdbx, cursor);
PHP_METHOD(Phalcon_Storage_Libmdbx, copy);
PHP_METHOD(Phalcon_Storage_Libmdbx, drop);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_libmdbx___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, readers, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, mapsize, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_libmdbx_begin, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_libmdbx_get, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_libmdbx_put, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_libmdbx_del, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_libmdbx_copy, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_libmdbx_drop, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, delete, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_storage_libmdbx_method_entry[] = {
	PHP_ME(Phalcon_Storage_Libmdbx, __construct, arginfo_phalcon_storage_libmdbx___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Storage_Libmdbx, begin, arginfo_phalcon_storage_libmdbx_begin, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Libmdbx, commit, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Libmdbx, renew, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Libmdbx, reset, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Libmdbx, getAll, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Libmdbx, get, arginfo_phalcon_storage_libmdbx_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Libmdbx, put, arginfo_phalcon_storage_libmdbx_put, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Libmdbx, del, arginfo_phalcon_storage_libmdbx_del, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Libmdbx, cursor, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Libmdbx, copy, arginfo_phalcon_storage_libmdbx_copy, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Libmdbx, drop, arginfo_phalcon_storage_libmdbx_drop, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Storage_Libmdbx, set, put, arginfo_phalcon_storage_libmdbx_put, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Storage_Libmdbx, delete, del, arginfo_phalcon_storage_libmdbx_del, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_object_handlers phalcon_storage_libmdbx_object_handlers;
zend_object* phalcon_storage_libmdbx_object_create_handler(zend_class_entry *ce)
{
	phalcon_storage_libmdbx_object *intern = ecalloc(1, sizeof(phalcon_storage_libmdbx_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_storage_libmdbx_object_handlers;

	return &intern->std;
}

void phalcon_storage_libmdbx_object_free_handler(zend_object *object)
{
	phalcon_storage_libmdbx_object *intern;

	intern = phalcon_storage_libmdbx_object_from_obj(object);

	if (intern->txn) {
		mdbx_txn_abort(intern->txn);
		intern->txn = NULL;
	}

	if (intern->dbi) {
		mdbx_dbi_close(intern->env, intern->dbi);
		intern->dbi = 0;
	}

	if (intern->env) {
		mdbx_env_close(intern->env);
		intern->env = NULL;
	}
}

/**
 * Phalcon\Storage\Libmdbx initializer
 */
PHALCON_INIT_CLASS(Phalcon_Storage_Libmdbx){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Storage, Libmdbx, storage_libmdbx, phalcon_storage_libmdbx_method_entry, 0);

	zend_declare_property_null(phalcon_storage_libmdbx_ce, SL("_path"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_storage_libmdbx_ce, SL("_name"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_storage_libmdbx_ce, SL("_flag"), 0, ZEND_ACC_PROTECTED);

	// Environment Flags
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("NOSUBDIR"),	MDBX_NOSUBDIR);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("NOSYNC"),		MDBX_NOSYNC);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("RDONLY"),		MDBX_RDONLY);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("NOMETASYNC"),	MDBX_NOMETASYNC);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("WRITEMAP"),	MDBX_WRITEMAP);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("MAPASYNC"),	MDBX_MAPASYNC);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("NOTLS"),		MDBX_NOTLS);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("NORDAHEAD"),	MDBX_NORDAHEAD);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("NOMEMINIT"),	MDBX_NOMEMINIT);

	// Database Flags
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("REVERSEKEY"),	MDBX_REVERSEKEY);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("DUPSORT"),		MDBX_DUPSORT);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("INTEGERKEY"),	MDBX_INTEGERKEY);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("DUPFIXED"),	MDBX_DUPFIXED);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("INTEGERDUP"),	MDBX_INTEGERDUP);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("REVERSEDUP"),	MDBX_REVERSEDUP);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("CREATE"),		MDBX_CREATE);

	// Write Flags
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("NOOVERWRITE"),	MDBX_NOOVERWRITE);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("NODUPDATA"),	MDBX_NODUPDATA);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("CURRENT"),		MDBX_CURRENT);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("RESERVE"),		MDBX_RESERVE);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("APPEND"),		MDBX_APPEND);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("APPENDDUP"),	MDBX_APPENDDUP);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("MULTIPLE"),	MDBX_MULTIPLE);

	// Copy Flags
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("CP_COMPACT"),	MDBX_CP_COMPACT);

	// Return Codes
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("SUCCESS"),				MDBX_SUCCESS);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("KEYEXIST"),			MDBX_KEYEXIST);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("NOTFOUND"),			MDBX_NOTFOUND);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("PAGE_NOTFOUND"),		MDBX_PAGE_NOTFOUND);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("CORRUPTED"),			MDBX_CORRUPTED);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("PANIC"),				MDBX_PANIC);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("VERSION_MISMATCH"),	MDBX_VERSION_MISMATCH);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("INVALID"),				MDBX_INVALID);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("MAP_FULL"),			MDBX_MAP_FULL);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("DBS_FULL"),			MDBX_DBS_FULL);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("READERS_FULL"),		MDBX_READERS_FULL);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("TXN_FULL"),			MDBX_TXN_FULL);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("CURSOR_FULL"),			MDBX_CURSOR_FULL);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("PAGE_FULL"),			MDBX_PAGE_FULL);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("MAP_RESIZED"),			MDBX_MAP_RESIZED);

	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("INCOMPATIBLE"),		MDBX_INCOMPATIBLE);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("BAD_RSLOT"),			MDBX_BAD_RSLOT);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("BAD_TXN"),				MDBX_BAD_TXN);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("BAD_VALSIZE"),			MDBX_BAD_VALSIZE);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("BAD_DBI"),				MDBX_BAD_DBI);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("PROBLEM"),				MDBX_PROBLEM);
	zend_declare_class_constant_long(phalcon_storage_libmdbx_ce, SL("LAST_ERRCODE"),		MDBX_LAST_ERRCODE);

	return SUCCESS;
}

/**
 * Phalcon\Storage\Libmdbx constructor
 *
 * @param string $path
 * @param string $name
 * @param int $readers
 * @param int $mapsize
 */
PHP_METHOD(Phalcon_Storage_Libmdbx, __construct)
{
	zval *path, *name = NULL, *readers = NULL, *mapsize = NULL, *_flags = NULL;
	phalcon_storage_libmdbx_object *intern;
	int flags = 0, rc;

	phalcon_fetch_params(0, 1, 4, &path, &name, &readers, &mapsize, &_flags);

	if (!name) {
		name = &PHALCON_GLOBAL(z_null);
	}

	if (_flags && Z_TYPE_P(_flags) == IS_LONG) {
		flags = Z_LVAL_P(_flags);
	}

	if (phalcon_file_exists(path) == FAILURE) {
		php_stream_mkdir(Z_STRVAL_P(path), 0775, PHP_STREAM_MKDIR_RECURSIVE | REPORT_ERRORS, php_stream_context_from_zval(NULL, 0));
	}

	phalcon_update_property(getThis(), SL("_path"), path);

	intern = phalcon_storage_libmdbx_object_from_obj(Z_OBJ_P(getThis()));

	rc = mdbx_env_create(&intern->env);
	if (rc != MDBX_SUCCESS) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_storage_exception_ce, "Failed to create an environment handle");
		return;
	}
	if (readers && Z_TYPE_P(readers) == IS_LONG) {
		mdbx_env_set_maxreaders(intern->env, Z_LVAL_P(readers));
	}
	if (mapsize && Z_TYPE_P(mapsize) == IS_LONG) {
		mdbx_env_set_mapsize(intern->env, Z_LVAL_P(mapsize));
	} else {
		 mdbx_env_set_mapsize(intern->env, 256 * 1024 * 1024);
	}
	mdbx_env_set_maxdbs(intern->env, 256);

	rc = mdbx_env_open(intern->env, Z_STRVAL_P(path), flags, 0664);
	
	if (rc != MDBX_SUCCESS) {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Failed to open an environment handle (%s)", Z_STRVAL_P(path));
		return;
	}

	rc = mdbx_txn_begin(intern->env, NULL, 0, &intern->txn);
	if (rc != MDBX_SUCCESS) {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Failed to create a transaction for use with the environment (%s)", mdbx_strerror(rc));
		return;
	}
	if (Z_TYPE_P(name) == IS_STRING) {
		phalcon_update_property(getThis(), SL("_name"), name);
		rc = mdbx_dbi_open(intern->txn, Z_STRVAL_P(name), MDBX_CREATE, &intern->dbi);
	} else {
		rc = mdbx_dbi_open(intern->txn, NULL, 0, &intern->dbi);
	}
	if (rc != MDBX_SUCCESS) {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Failed to open a database in the environment (%s)", mdbx_strerror(rc));
		return;
	}
	rc = mdbx_txn_commit(intern->txn);
	if (rc != MDBX_SUCCESS) {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Failed to reset a read-only transaction (%s)", mdbx_strerror(rc));
		return;
	}
}

/**
 * Create a transaction for use with the environment
 *
 */
PHP_METHOD(Phalcon_Storage_Libmdbx, begin)
{
	zval *flags = NULL;
	phalcon_storage_libmdbx_object *intern;
	int rc;

	phalcon_fetch_params(0, 0, 1, &flags);

	intern = phalcon_storage_libmdbx_object_from_obj(Z_OBJ_P(getThis()));
	if (flags && Z_TYPE_P(flags) == IS_LONG) {
		rc = mdbx_txn_begin(intern->env, NULL, Z_LVAL_P(flags), &intern->txn);
	} else {
		rc = mdbx_txn_begin(intern->env, NULL, 0, &intern->txn);
	}
	if (rc != MDBX_SUCCESS) {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Failed to create a transaction for use with the environment (%s)", mdbx_strerror(rc));
		return;
	}

	RETURN_TRUE;
}

/**
 * Commit all the operations of a transaction into the database
 *
 */
PHP_METHOD(Phalcon_Storage_Libmdbx, commit)
{
	phalcon_storage_libmdbx_object *intern;
	int rc;

	intern = phalcon_storage_libmdbx_object_from_obj(Z_OBJ_P(getThis()));

	rc = mdbx_txn_commit(intern->txn);
	if (rc != MDBX_SUCCESS) {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Failed to commit all the operations of a transaction into the database (%s)", mdbx_strerror(rc));
		return;
	}
	RETURN_TRUE;
}

/**
 * Renew a read-only transaction
 *
 */
PHP_METHOD(Phalcon_Storage_Libmdbx, renew)
{
	phalcon_storage_libmdbx_object *intern;

	intern = phalcon_storage_libmdbx_object_from_obj(Z_OBJ_P(getThis()));

	mdbx_txn_renew(intern->txn);
}

/**
 * Reset a read-only transaction
 *
 */
PHP_METHOD(Phalcon_Storage_Libmdbx, reset)
{
	phalcon_storage_libmdbx_object *intern;

	intern = phalcon_storage_libmdbx_object_from_obj(Z_OBJ_P(getThis()));

	mdbx_txn_reset(intern->txn);
}

/**
 * Get all items from a database
 *
 * @return array
 */
PHP_METHOD(Phalcon_Storage_Libmdbx, getAll)
{
	MDBX_val k, v;
	MDBX_cursor *cursor;
	phalcon_storage_libmdbx_object *intern;
	int rc;

	intern = phalcon_storage_libmdbx_object_from_obj(Z_OBJ_P(getThis()));

	rc = mdbx_cursor_open(intern->txn, intern->dbi, &cursor);
	if (rc != MDBX_SUCCESS) {
		mdbx_dbi_close(intern->env, intern->dbi);
		mdbx_txn_abort(intern->txn);
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Failed to create a cursor handle (%s)", mdbx_strerror(rc));
		return;
	}
	array_init(return_value);
	while ((rc = mdbx_cursor_get(cursor, &k, &v, MDBX_NEXT)) == 0) {
		zval s = {}, u = {};
		ZVAL_STRINGL(&s, (char *) v.iov_base, (int) v.iov_len);
		phalcon_unserialize(&u, &s);
		zval_ptr_dtor(&s);
		phalcon_array_update_str(return_value, (char *) k.iov_base, (int) k.iov_len, &u, 0);
	}
	mdbx_cursor_close(cursor);
}

/**
 * Get items from a database
 *
 * @param string $key
 * @return string
 */
PHP_METHOD(Phalcon_Storage_Libmdbx, get)
{
	zval *key, s = {};
	MDBX_val k, v;
	phalcon_storage_libmdbx_object *intern;
	int rc;

	phalcon_fetch_params(0, 1, 0, &key);

	k.iov_len = Z_STRLEN_P(key);
	k.iov_base = Z_STRVAL_P(key);

	intern = phalcon_storage_libmdbx_object_from_obj(Z_OBJ_P(getThis()));

	rc = mdbx_get(intern->txn, intern->dbi, &k, &v);
	if (rc == MDBX_SUCCESS) {
		ZVAL_STRINGL(&s, (char *) v.iov_base, (int) v.iov_len);
		phalcon_unserialize(return_value, &s);
		zval_ptr_dtor(&s);
	} else if (rc == MDBX_NOTFOUND) {
		RETVAL_FALSE;
	} else {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Failed to get item from a database (%s)", mdbx_strerror(rc));
		return;
	}
}

/**
 * Store items into a database
 *
 * @param string $key
 * @param mixed $value
 * @return mixed
 */
PHP_METHOD(Phalcon_Storage_Libmdbx, put)
{
	zval *key, *value, s = {};
	MDBX_val k, v;
	phalcon_storage_libmdbx_object *intern;
	int rc;

	phalcon_fetch_params(0, 2, 0, &key, &value);

	phalcon_serialize(&s, value);

	k.iov_len = Z_STRLEN_P(key);
	k.iov_base = Z_STRVAL_P(key);
	v.iov_len = Z_STRLEN(s);
	v.iov_base = Z_STRVAL(s);

	intern = phalcon_storage_libmdbx_object_from_obj(Z_OBJ_P(getThis()));

	rc = mdbx_put(intern->txn, intern->dbi, &k, &v, 0);
	zval_ptr_dtor(&s);
	if (rc != MDBX_SUCCESS) {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Failed to store items into a database (%s)", mdbx_strerror(rc));
		return;
	}

	RETURN_TRUE;
}

/**
 * Delete items from a database
 *
 * @param string $keys
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Libmdbx, del)
{
	zval *key;
	MDBX_val k;
	phalcon_storage_libmdbx_object *intern;
	int rc;

	phalcon_fetch_params(0, 1, 0, &key);

	k.iov_len = Z_STRLEN_P(key);
	k.iov_base = Z_STRVAL_P(key);

	intern = phalcon_storage_libmdbx_object_from_obj(Z_OBJ_P(getThis()));

	rc = mdbx_del(intern->txn, intern->dbi, &k, NULL);
	if (rc == MDBX_SUCCESS) {
		RETVAL_TRUE;
	} else if (rc == MDBX_NOTFOUND) {
		RETVAL_FALSE;
	} else {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Failed to delete items from a database (%s)", mdbx_strerror(rc));
		return;
	}
}

/**
 * Create a cursor handle
 *
 * @return Phalcon\Storage\Libmdbx\Cursor
 */
PHP_METHOD(Phalcon_Storage_Libmdbx, cursor)
{
	MDBX_cursor *cursor;
	phalcon_storage_libmdbx_object *intern;
	phalcon_storage_libmdbx_cursor_object *cursor_intern;
	int rc;

	intern = phalcon_storage_libmdbx_object_from_obj(Z_OBJ_P(getThis()));

	rc = mdbx_cursor_open(intern->txn, intern->dbi, &cursor);
	if (rc != MDBX_SUCCESS) {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Failed to create a cursor handle (%s)", mdbx_strerror(rc));
		return;
	}

	object_init_ex(return_value, phalcon_storage_libmdbx_cursor_ce);
	cursor_intern = phalcon_storage_libmdbx_cursor_object_from_obj(Z_OBJ_P(return_value));
	cursor_intern->cursor = cursor;
}

/**
 * Copy an LIBMDBX environment to the specified path
 *
 * @param string $path
 * @param int $flags
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Libmdbx, copy)
{
	zval *path, *_flags = NULL;
	phalcon_storage_libmdbx_object *intern;
	int flags = 0, rc;

	phalcon_fetch_params(0, 1, 1, &path, &_flags);

	if (phalcon_file_exists(path) == FAILURE) {
		php_stream_mkdir(Z_STRVAL_P(path), 0775, PHP_STREAM_MKDIR_RECURSIVE | REPORT_ERRORS, php_stream_context_from_zval(NULL, 0));
	}

	if (_flags && Z_TYPE_P(_flags) == IS_LONG) {
		flags = Z_LVAL_P(_flags);
	}

	intern = phalcon_storage_libmdbx_object_from_obj(Z_OBJ_P(getThis()));

	rc = mdbx_env_copy(intern->env, Z_STRVAL_P(path), flags);
	if (rc != MDBX_SUCCESS) {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Failed to copy an LIBMDBX environment (%s)", mdbx_strerror(rc));
		return;
	}
	RETURN_TRUE;
}

/**
 * Empty or delete+close a database
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Libmdbx, drop)
{
	zval *del = NULL;
	phalcon_storage_libmdbx_object *intern;
	int rc;

	phalcon_fetch_params(0, 0, 1, &del);

	if (!del) {
		del = &PHALCON_GLOBAL(z_false);
	}

	intern = phalcon_storage_libmdbx_object_from_obj(Z_OBJ_P(getThis()));

	rc = mdbx_drop(intern->txn, intern->dbi, zend_is_true(del) ? 1 : 0);
	if (rc != MDBX_SUCCESS) {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Failed to drop a database (%s)", mdbx_strerror(rc));
		return;
	}

	RETURN_TRUE;
}
