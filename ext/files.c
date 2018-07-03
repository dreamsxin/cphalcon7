
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

#include "files.h"
#include "di.h"

#include <main/php_streams.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/concat.h"
#include "kernel/object.h"
#include "kernel/hash.h"
#include "kernel/file.h"

#include "interned-strings.h"

/**
 * Phalcon\Files
 *
 */
zend_class_entry *phalcon_files_ce;

PHP_METHOD(Phalcon_Files, createDirectory);
PHP_METHOD(Phalcon_Files, createFile);
PHP_METHOD(Phalcon_Files, copy);
PHP_METHOD(Phalcon_Files, move);
PHP_METHOD(Phalcon_Files, delete);
PHP_METHOD(Phalcon_Files, list);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_files_createdirectory, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, pathname, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, recursive, _IS_BOOL, 1)
	ZEND_ARG_TYPE_INFO(0, context, IS_RESOURCE, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_files_createfile, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_files_copy, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, source, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, target, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_files_move, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, source, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, target, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_files_delete, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_files_list, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_files_method_entry[] = {
	PHP_ME(Phalcon_File, createDirectory, arginfo_phalcon_files_createdirectory, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_File, createFile, arginfo_phalcon_files_createfile, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_File, copy, arginfo_phalcon_files_copy, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_File, move, arginfo_phalcon_files_move, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_File, delete, arginfo_phalcon_files_delete, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_File, list, arginfo_phalcon_files_list, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Files initializer
 */
PHALCON_INIT_CLASS(Phalcon_Files){

	PHALCON_REGISTER_CLASS(Phalcon, File, file, phalcon_file_method_entry, 0);

	return SUCCESS;
}

/**
 * Create a directory
 *
 * @param string pathname
 * @param int mode
 * @param bool recursive
 * @param resource context
 * @return boolean
 */
PHP_METHOD(Phalcon_Files, createDirectory){

	char *dir;
	size_t dir_len;
	zval *zcontext = NULL;
	zend_long mode = 0777;
	zend_bool recursive = 0;
	php_stream_context *context;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "p|lbr", &dir, &dir_len, &mode, &recursive, &zcontext) == FAILURE) {
		RETURN_FALSE;
	}

	context = php_stream_context_from_zval(zcontext, 0);

	RETURN_BOOL(php_stream_mkdir(dir, (int)mode, (recursive ? PHP_STREAM_MKDIR_RECURSIVE : 0) | REPORT_ERRORS, context));
}

/**
 * 
 *
 * @param string $filename
 * @return boolean
 */
PHP_METHOD(Phalcon_Files, createFile){

	zval *filename;

	phalcon_fetch_params(0, 1, 0, &filename);
}

/**
 * 
 *
 * @param string $source
 * @param string $target
 * @return boolean
 */
PHP_METHOD(Phalcon_Files, copy){

	zval *source, *target;

	phalcon_fetch_params(0, 2, 0, &source, &target);
}

/**
 * 
 *
 * @param string $source
 * @param string $target
 * @return boolean
 */
PHP_METHOD(Phalcon_Files, move){

	zval *source, *target;

	phalcon_fetch_params(0, 2, 0, &source, &target);
}

/**
 * 
 *
 * @param string $path
 * @return boolean
 */
PHP_METHOD(Phalcon_Files, delete){

	zval *path;

	phalcon_fetch_params(0, 1, 0, &path);
}

/**
 * 
 *
 * @param string $path
 * @return boolean
 */
PHP_METHOD(Phalcon_Files, list){

	zval *path;

	phalcon_fetch_params(0, 1, 0, &path);
}
