
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

#include "storage/wiredtiger/cursor.h"
#include "storage/wiredtiger.h"
#include "storage/wiredtiger/pack.h"
#include "storage/exception.h"

#include "zend_smart_str.h"

#include <wiredtiger.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"
#include "kernel/file.h"
#include "kernel/exception.h"

#include "internal/arginfo.h"

/**
 * Phalcon\Storage\Wiredtiger\Cursor
 *
 * It can be used to replace APC or local memstoraged.
 */
zend_class_entry *phalcon_storage_wiredtiger_cursor_ce;

PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, __construct);
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, reconfigure);
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, set);
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, get);
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, delete);
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, current);
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, key);
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, next);
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, prev);
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, rewind);
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, last);
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, valid);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_wiredtiger_cursor___construct, 0, 0, 2)
	ZEND_ARG_OBJ_INFO(0, db, Phalcon\\Storage\\Wiredtiger, 0)
	ZEND_ARG_TYPE_INFO(0, uri, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, config, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_wiredtiger_cursor_reconfigure, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, config, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_wiredtiger_cursor_set, 0, 0, 2)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_wiredtiger_cursor_get, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_storage_wiredtiger_cursor_delete, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_storage_wiredtiger_cursor_method_entry[] = {
	PHP_ME(Phalcon_Storage_Wiredtiger_Cursor, __construct, arginfo_phalcon_storage_wiredtiger_cursor___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Storage_Wiredtiger_Cursor, reconfigure, arginfo_phalcon_storage_wiredtiger_cursor_reconfigure, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Wiredtiger_Cursor, set, arginfo_phalcon_storage_wiredtiger_cursor_set, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Wiredtiger_Cursor, get, arginfo_phalcon_storage_wiredtiger_cursor_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Wiredtiger_Cursor, delete, arginfo_phalcon_storage_wiredtiger_cursor_delete, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Wiredtiger_Cursor, current, arginfo_iterator_current, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Wiredtiger_Cursor, key, arginfo_iterator_key, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Wiredtiger_Cursor, next, arginfo_iterator_next, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Wiredtiger_Cursor, prev, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Wiredtiger_Cursor, rewind, arginfo_iterator_rewind, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Wiredtiger_Cursor, last, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Storage_Wiredtiger_Cursor, valid, arginfo_iterator_valid, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_object_handlers phalcon_storage_wiredtiger_cursor_object_handlers;
zend_object* phalcon_storage_wiredtiger_cursor_object_create_handler(zend_class_entry *ce)
{
	phalcon_storage_wiredtiger_cursor_object *intern = ecalloc(1, sizeof(phalcon_storage_wiredtiger_cursor_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_storage_wiredtiger_cursor_object_handlers;

	intern->db = NULL;
	intern->cursor = NULL;
	intern->started_iterating = 0;

	intern->current.key.data = NULL;
	intern->current.key.size = 0;
	intern->current.value.data = NULL;
	intern->current.value.size = 0;

	return &intern->std;
}

void phalcon_storage_wiredtiger_cursor_object_free_handler(zend_object *object)
{
	phalcon_storage_wiredtiger_cursor_object *intern = phalcon_storage_wiredtiger_cursor_object_from_obj(object);
/*
	if (intern->cursor) {
		intern->cursor->close(intern->cursor);
	}
*/
	if (intern->db) {
		zval_ptr_dtor(intern->db);
	}
}

/**
 * Phalcon\Storage\Wiredtiger_Cursor initializer
 */
PHALCON_INIT_CLASS(Phalcon_Storage_Wiredtiger_Cursor){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Storage, Wiredtiger_Cursor, storage_wiredtiger_cursor, phalcon_storage_wiredtiger_cursor_method_entry, 0);

	zend_declare_property_null(phalcon_storage_wiredtiger_cursor_ce, SL("_uri"), ZEND_ACC_PROTECTED);
	zend_declare_property_string(phalcon_storage_wiredtiger_cursor_ce, SL("_config"), "raw", ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_storage_wiredtiger_cursor_ce, 1, zend_ce_iterator);

	return SUCCESS;
}

/**
 * Phalcon\Storage\Wiredtiger\Cursor constructor
 *
 * @param Phalcon\Storage\Wiredtiger $db
 * @param string $uri
 * @param string $config
 */
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, __construct)
{
	zval *db, *uri, *_config = NULL, config = {};
	phalcon_storage_wiredtiger_object *db_intern;
	phalcon_storage_wiredtiger_cursor_object *intern;
	int ret;

	phalcon_fetch_params(0, 2, 1, &db, &uri, &_config);

	phalcon_update_property_zval(getThis(), SL("_uri"), uri);

	if (_config && Z_TYPE_P(_config) == IS_STRING) {
		phalcon_update_property_zval(getThis(), SL("_config"), _config);
	}

	phalcon_read_property(&config, getThis(), SL("_config"), PH_NOISY);

	db_intern = phalcon_storage_wiredtiger_object_from_obj(Z_OBJ_P(db));
	intern = phalcon_storage_wiredtiger_cursor_object_from_obj(Z_OBJ_P(getThis()));

	ret = db_intern->session->open_cursor(db_intern->session, Z_STRVAL_P(uri), NULL, Z_STRVAL(config), &intern->cursor);
	if (ret != PHALCON_STORAGE_WIREDTIGER_OK) {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Error opening a cursor %s: %s", Z_STRVAL_P(uri), wiredtiger_strerror(ret));
		return;
	}
}

/**
 * Reconfigure the cursor to overwrite the record
 *
 * @param string $config
 */
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, reconfigure)
{
	zval *config;
	phalcon_storage_wiredtiger_cursor_object *intern;
	int ret;

	phalcon_fetch_params(0, 1, 0, &config);

	intern = phalcon_storage_wiredtiger_cursor_object_from_obj(Z_OBJ_P(getThis()));

	ret = intern->cursor->reconfigure(intern->cursor, Z_STRVAL_P(config));
	if (ret != PHALCON_STORAGE_WIREDTIGER_OK) {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Error Reconfigure a cursor %s: %s", Z_STRVAL_P(config), wiredtiger_strerror(ret));
		return;
	}

	phalcon_update_property_zval(getThis(), SL("_config"), config);
	RETURN_TRUE;
}

/**
 * Sets data
 *
 * @param mixed $key
 * @param mixed $value
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, set)
{
	zval *key, *value;
	phalcon_storage_wiredtiger_cursor_object *intern;
	phalcon_storage_wiredtiger_pack_item pk = { 0, }, pv = { 0, };
	WT_ITEM item;
	int ret, autoindex = 0;

	phalcon_fetch_params(0, 2, 0, &key, &value);

	intern = phalcon_storage_wiredtiger_cursor_object_from_obj(Z_OBJ_P(getThis()));

	if (PHALCON_IS_EMPTY(key)) {
		if (strcmp(intern->cursor->key_format, "r") != 0) {
			PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Error key %s: %s", Z_STRVAL_P(key), intern->cursor->key_format);
			return;
		}
		autoindex = 1;
	} else if (strcmp(intern->cursor->key_format, "r") == 0 && (Z_TYPE_P(key) == IS_LONG && Z_LVAL_P(key) == 0)) {
		autoindex = 1;
	} else {
		ret = phalcon_storage_wiredtiger_pack_key(intern, &pk, key);
		if (ret != 0) {
			phalcon_storage_wiredtiger_pack_item_free(&pk);
			PHALCON_THROW_EXCEPTION_STR(phalcon_storage_exception_ce, "Error pack key");
			return;
		}

		item.data = pk.data;
		item.size = pk.size;

		intern->cursor->set_key(intern->cursor, &item);
	}

	/* Set value */
	ret = phalcon_storage_wiredtiger_pack_value(intern, &pv, value);

	if (ret != 0) {
		phalcon_storage_wiredtiger_pack_item_free(&pk);
		phalcon_storage_wiredtiger_pack_item_free(&pv);
		PHALCON_THROW_EXCEPTION_STR(phalcon_storage_exception_ce, "Error pack value");
		return;
	}

	item.data = pv.data;
	item.size = pv.size;

	intern->cursor->set_value(intern->cursor, &item);

	if (autoindex) {
		ret = intern->cursor->insert(intern->cursor);
	} else {
		ret = intern->cursor->update(intern->cursor);
	}

	phalcon_storage_wiredtiger_pack_item_free(&pk);
	phalcon_storage_wiredtiger_pack_item_free(&pv);

	if (ret != 0) {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "%s", wiredtiger_strerror(ret));
		return;
	} else {
		RETVAL_TRUE;
	}
}

/**
 * Gets data
 *
 * @param mixed $key
 * @return mixed
 */
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, get)
{
	zval *key;
	phalcon_storage_wiredtiger_cursor_object *intern;
	phalcon_storage_wiredtiger_pack_item pk = { 0, };
	WT_ITEM item;
	int ret;

	phalcon_fetch_params(0, 1, 0, &key);

	intern = phalcon_storage_wiredtiger_cursor_object_from_obj(Z_OBJ_P(getThis()));

	ret = phalcon_storage_wiredtiger_pack_key(intern, &pk, key);
	if (ret != 0) {
		phalcon_storage_wiredtiger_pack_item_free(&pk);
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Error pack key: %s", wiredtiger_strerror(ret));
		return;
	}

	item.data = pk.data;
	item.size = pk.size;

	intern->cursor->set_key(intern->cursor, &item);

	if (intern->cursor->search(intern->cursor) != 0) {
		 phalcon_storage_wiredtiger_pack_item_free(&pk);
		 RETURN_NULL();
	} else {
		ret = intern->cursor->get_value(intern->cursor, &item);
		phalcon_storage_wiredtiger_pack_item_free(&pk);
		if (ret != 0) {
			PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Error get value: %s", wiredtiger_strerror(ret));
			return;
		}

		ret = phalcon_storage_wiredtiger_unpack_value(intern, return_value, &item);
		if (ret != 0) {
			phalcon_storage_wiredtiger_pack_item_free(&pk);
			PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Error unpack value: %s", wiredtiger_strerror(ret));
 			return;
		}
	}
	phalcon_storage_wiredtiger_pack_item_free(&pk);
}

/**
 * Delete data
 *
 * @param mixed $key
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, delete)
{
	zval *key;
	phalcon_storage_wiredtiger_cursor_object *intern;
	phalcon_storage_wiredtiger_pack_item pk = { 0, };
	WT_ITEM item;
	int ret;

	phalcon_fetch_params(0, 1, 0, &key);

	intern = phalcon_storage_wiredtiger_cursor_object_from_obj(Z_OBJ_P(getThis()));

	ret = phalcon_storage_wiredtiger_pack_key(intern, &pk, key);
	if (ret != 0) {
		phalcon_storage_wiredtiger_pack_item_free(&pk);
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Error pack key: %s", wiredtiger_strerror(ret));
		return;
	}

	item.data = pk.data;
	item.size = pk.size;

	intern->cursor->set_key(intern->cursor, &item);

	/* Get value */
	if (intern->cursor->search(intern->cursor) != 0) {
		RETVAL_FALSE;
	} else {
		ret = intern->cursor->remove(intern->cursor);
		if (ret != 0) {
			phalcon_storage_wiredtiger_pack_item_free(&pk);
			PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Error remove value %s", wiredtiger_strerror(ret));
			return;
		} else {
			RETVAL_TRUE;
		}
	}

	phalcon_storage_wiredtiger_pack_item_free(&pk);
}

static int phalcon_storage_wiredtiger_cursor_load_current(phalcon_storage_wiredtiger_cursor_object *intern)
{
	int ret;

	ret = intern->cursor->get_key(intern->cursor, &intern->current.key);
	if (ret != 0) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_storage_exception_ce, "Can't get key");
		return FAILURE;
	}

	/* Get value */
	ret = intern->cursor->get_value(intern->cursor, &intern->current.value);
	if (ret != 0) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_storage_exception_ce, "Can't get value");
		return FAILURE;
	}

	return SUCCESS;
}

/**
 * Gets current value
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, current)
{
	phalcon_storage_wiredtiger_cursor_object *intern;
	int ret;

	intern = phalcon_storage_wiredtiger_cursor_object_from_obj(Z_OBJ_P(getThis()));

    if (!intern->started_iterating) {
        if (intern->cursor->next(intern->cursor) != 0) {
            RETURN_FALSE;
        }
		intern->started_iterating = 1;
    }

	phalcon_storage_wiredtiger_cursor_load_current(intern);

	if (!intern->current.value.data) {
		RETURN_FALSE;
	} else if (intern->current.value.size == 0) {
		RETURN_EMPTY_STRING();
	} else {
		ret = phalcon_storage_wiredtiger_unpack_value(intern, return_value, &intern->current.value);
		if (ret != 0) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_storage_exception_ce, "Can't get value");
			RETURN_FALSE;
		}
	}
}

/**
 * Gets current key
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, key)
{
	phalcon_storage_wiredtiger_cursor_object *intern;
	int ret;

	intern = phalcon_storage_wiredtiger_cursor_object_from_obj(Z_OBJ_P(getThis()));

    if (!intern->started_iterating) {
        if (intern->cursor->next(intern->cursor) != 0) {
            RETURN_FALSE;
        }
		intern->started_iterating = 1;
    }

	phalcon_storage_wiredtiger_cursor_load_current(intern);

	if (!intern->current.key.data) {
		RETURN_FALSE;
	} else if (intern->current.key.size == 0) {
		RETURN_EMPTY_STRING();
	} else {
		ret = phalcon_storage_wiredtiger_unpack_key(intern, return_value, &intern->current.key);
		if (ret != 0) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_storage_exception_ce, "Can't get key");
			RETURN_FALSE;
		}
	}
}

/**
 * Moves cursor to next row
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, next)
{
	phalcon_storage_wiredtiger_cursor_object *intern;

	intern = phalcon_storage_wiredtiger_cursor_object_from_obj(Z_OBJ_P(getThis()));

	if (intern->cursor->next(intern->cursor) != 0) {
		RETURN_FALSE;
	}

	intern->started_iterating = 1;

	RETURN_TRUE;
}

/**
 * Moves cursor to prev row
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, prev)
{
	phalcon_storage_wiredtiger_cursor_object *intern;

	intern = phalcon_storage_wiredtiger_cursor_object_from_obj(Z_OBJ_P(getThis()));

	if (intern->cursor->prev(intern->cursor) != 0) {
		RETURN_FALSE;
	}

	intern->started_iterating = 1;

	RETURN_TRUE;
}

/**
 * Rewinds cursor to it's beginning
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, rewind)
{
	phalcon_storage_wiredtiger_cursor_object *intern;
	int ret;

	intern = phalcon_storage_wiredtiger_cursor_object_from_obj(Z_OBJ_P(getThis()));

	ret = intern->cursor->reset(intern->cursor);
	if (ret != 0) {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Invalid rewind: %s", wiredtiger_strerror(ret));
		RETURN_FALSE;
	}

	if (intern->cursor->next(intern->cursor) != 0) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

/**
 * Rewinds cursor to it's last
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, last)
{
	phalcon_storage_wiredtiger_cursor_object *intern;
	int ret;

	intern = phalcon_storage_wiredtiger_cursor_object_from_obj(Z_OBJ_P(getThis()));

	ret = intern->cursor->reset(intern->cursor);
	if (ret != 0) {
		PHALCON_THROW_EXCEPTION_FORMAT(phalcon_storage_exception_ce, "Invalid rewind: %s", wiredtiger_strerror(ret));
		RETURN_FALSE;
	}

	while (intern->cursor->prev(intern->cursor) != 0);

	RETURN_TRUE;
}

/**
 * Check whether has rows to fetch
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Storage_Wiredtiger_Cursor, valid)
{
	phalcon_storage_wiredtiger_cursor_object *intern;

	intern = phalcon_storage_wiredtiger_cursor_object_from_obj(Z_OBJ_P(getThis()));

	if (intern->current.key.data != NULL &&
		intern->current.value.data != NULL) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}
