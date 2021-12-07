
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

#include <ext/standard/file.h>
#include <ext/standard/php_filestat.h>
#include <ext/standard/flock_compat.h>
#include <ext/spl/spl_directory.h>
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

#include <fcntl.h>

/**
 * Phalcon\Files
 *
 */
zend_class_entry *phalcon_files_ce;

PHP_METHOD(Phalcon_Files, createDirectory);
PHP_METHOD(Phalcon_Files, create);
PHP_METHOD(Phalcon_Files, copy);
PHP_METHOD(Phalcon_Files, move);
PHP_METHOD(Phalcon_Files, delete);
PHP_METHOD(Phalcon_Files, list);
PHP_METHOD(Phalcon_Files, getExtension);
PHP_METHOD(Phalcon_Files, setExtension);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_files_createdirectory, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, pathname, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, recursive, _IS_BOOL, 1)
	ZEND_ARG_TYPE_INFO(0, context, IS_RESOURCE, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_files_create, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 1)
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
	ZEND_ARG_TYPE_INFO(0, recursive, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_files_list, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, recursive, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_files_getextension, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_files_setextension, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, extension, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_files_method_entry[] = {
	PHP_ME(Phalcon_Files, createDirectory, arginfo_phalcon_files_createdirectory, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Files, create, arginfo_phalcon_files_create, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Files, copy, arginfo_phalcon_files_copy, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Files, move, arginfo_phalcon_files_move, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Files, delete, arginfo_phalcon_files_delete, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Files, list, arginfo_phalcon_files_list, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Files, getExtension, arginfo_phalcon_files_getextension, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Files, setExtension, arginfo_phalcon_files_setextension, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Files initializer
 */
PHALCON_INIT_CLASS(Phalcon_Files){

	PHALCON_REGISTER_CLASS(Phalcon, Files, files, phalcon_files_method_entry, 0);

	zend_declare_class_constant_long(phalcon_files_ce, SL("TYPE_NONE"), 0);
	zend_declare_class_constant_long(phalcon_files_ce, SL("TYPE_FILE"), 1);
	zend_declare_class_constant_long(phalcon_files_ce, SL("TYPE_DIR"),  2);

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

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|lbr", &dir, &dir_len, &mode, &recursive, &zcontext) == FAILURE) {
		RETURN_FALSE;
	}

	if (phalcon_file_exists_str(dir)) {
		RETURN_TRUE;
	}
	context = php_stream_context_from_zval(zcontext, 0);

	RETURN_BOOL(php_stream_mkdir(dir, (int)mode, (recursive ? PHP_STREAM_MKDIR_RECURSIVE : 0) | REPORT_ERRORS, context));
}

/**
 * Remove a directory
 *
 * @param string pathname
 * @param resource context
 * @return boolean
 */
PHP_METHOD(Phalcon_Files, RemoveDirectory){
	char *dir;
	size_t dir_len;
	zval *zcontext = NULL;
	php_stream_context *context;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_PATH(dir, dir_len)
		Z_PARAM_OPTIONAL
		Z_PARAM_RESOURCE_EX(zcontext, 1, 0)
	ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

	context = php_stream_context_from_zval(zcontext, 0);

	RETURN_BOOL(php_stream_rmdir(dir, REPORT_ERRORS, context));
}

/**
 * Create a file
 *
 * @param string $filename
 * @param string $data
 * @return boolean
 */
PHP_METHOD(Phalcon_Files, create){

	php_stream *stream;
	char *filename;
	size_t filename_len;
	char *data;
	size_t data_len;
	size_t numbytes = 0;
	zend_long flags = 0;
	zval *zcontext = NULL;
	php_stream_context *context = NULL;
	char mode[3] = "wb";

	ZEND_PARSE_PARAMETERS_START(2, 4)
		Z_PARAM_PATH(filename, filename_len)
		Z_PARAM_STRING(data, data_len)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(flags)
		Z_PARAM_RESOURCE_EX(zcontext, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	context = php_stream_context_from_zval(zcontext, flags & PHP_FILE_NO_DEFAULT_CONTEXT);

	if (flags & PHP_FILE_APPEND) {
		mode[0] = 'a';
	} else if (flags & LOCK_EX) {
		/* check to make sure we are dealing with a regular file */
		if (php_memnstr(filename, "://", sizeof("://") - 1, filename + filename_len)) {
			if (strncasecmp(filename, "file://", sizeof("file://") - 1)) {
				php_error_docref(NULL, E_WARNING, "Exclusive locks may only be set for regular files");
				RETURN_FALSE;
			}
		}
		mode[0] = 'c';
	}
	mode[2] = '\0';

	stream = php_stream_open_wrapper_ex(filename, mode, ((flags & PHP_FILE_USE_INCLUDE_PATH) ? USE_PATH : 0) | REPORT_ERRORS, NULL, context);
	if (stream == NULL) {
		RETURN_FALSE;
	}

	if (flags & LOCK_EX && (!php_stream_supports_lock(stream) || php_stream_lock(stream, LOCK_EX))) {
		php_stream_close(stream);
		php_error_docref(NULL, E_WARNING, "Exclusive locks are not supported for this stream");
		RETURN_FALSE;
	}

	if (mode[0] == 'c') {
		php_stream_truncate_set_size(stream, 0);
	}

	if (data_len>0) {
		numbytes = php_stream_write(stream, data, data_len);
		if (numbytes != data_len) {
			php_error_docref(NULL, E_WARNING, "Only "ZEND_LONG_FMT" of %zd bytes written, possibly out of free disk space", numbytes, data_len);
			numbytes = -1;
		}
	}

	php_stream_close(stream);

	if (numbytes == (size_t)-1) {
		RETURN_FALSE;
	}

	RETURN_LONG(numbytes);
}


/* {{{ php_copy_file
 */
PHPAPI int php_copy_file(const char *src, const char *dest)
{
	return php_copy_file_ctx(src, dest, 0, NULL);
}
/* }}} */

/* {{{ php_copy_file_ex
 */
PHPAPI int php_copy_file_ex(const char *src, const char *dest, int src_flg)
{
	return php_copy_file_ctx(src, dest, src_flg, NULL);
}
/* }}} */

/* {{{ php_copy_file_ctx
 */
PHPAPI int php_copy_file_ctx(const char *src, const char *dest, int src_flg, php_stream_context *ctx)
{
	php_stream *srcstream = NULL, *deststream = NULL;
	int ret = FAILURE;
	php_stream_statbuf src_s, dest_s;

	switch (php_stream_stat_path_ex(src, 0, &src_s, ctx)) {
		case -1:
			/* non-statable stream */
			goto safe_to_copy;
			break;
		case 0:
			break;
		default: /* failed to stat file, does not exist? */
			return ret;
	}
	if (S_ISDIR(src_s.sb.st_mode)) {
		php_error_docref(NULL, E_WARNING, "The first argument to copy() function cannot be a directory");
		return FAILURE;
	}

	switch (php_stream_stat_path_ex(dest, PHP_STREAM_URL_STAT_QUIET, &dest_s, ctx)) {
		case -1:
			/* non-statable stream */
			goto safe_to_copy;
			break;
		case 0:
			break;
		default: /* failed to stat file, does not exist? */
			return ret;
	}
	if (S_ISDIR(dest_s.sb.st_mode)) {
		php_error_docref(NULL, E_WARNING, "The second argument to copy() function cannot be a directory");
		return FAILURE;
	}
	if (!src_s.sb.st_ino || !dest_s.sb.st_ino) {
		goto no_stat;
	}
	if (src_s.sb.st_ino == dest_s.sb.st_ino && src_s.sb.st_dev == dest_s.sb.st_dev) {
		return ret;
	} else {
		goto safe_to_copy;
	}
no_stat:
	{
		char *sp, *dp;
		int res;

		if ((sp = expand_filepath(src, NULL)) == NULL) {
			return ret;
		}
		if ((dp = expand_filepath(dest, NULL)) == NULL) {
			efree(sp);
			goto safe_to_copy;
		}

		res =
#ifndef PHP_WIN32
			!strcmp(sp, dp);
#else
			!strcasecmp(sp, dp);
#endif

		efree(sp);
		efree(dp);
		if (res) {
			return ret;
		}
	}
safe_to_copy:

	srcstream = php_stream_open_wrapper_ex(src, "rb", src_flg | REPORT_ERRORS, NULL, ctx);

	if (!srcstream) {
		return ret;
	}

	deststream = php_stream_open_wrapper_ex(dest, "wb", REPORT_ERRORS, NULL, ctx);

	if (srcstream && deststream) {
		ret = php_stream_copy_to_stream_ex(srcstream, deststream, PHP_STREAM_COPY_ALL, NULL);
	}
	if (srcstream) {
		php_stream_close(srcstream);
	}
	if (deststream) {
		php_stream_close(deststream);
	}
	return ret;
}
/* }}} */

/**
 * 
 *
 * @param string $source
 * @param string $target
 * @return boolean
 */
PHP_METHOD(Phalcon_Files, copy){
	char *source, *target;
	size_t source_len, target_len;
	zval *zcontext = NULL;
	php_stream_context *context;

	ZEND_PARSE_PARAMETERS_START(2, 3)
		Z_PARAM_PATH(source, source_len)
		Z_PARAM_PATH(target, target_len)
		Z_PARAM_OPTIONAL
		Z_PARAM_RESOURCE_EX(zcontext, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	if (php_check_open_basedir(source)) {
		RETURN_FALSE;
	}

	context = php_stream_context_from_zval(zcontext, 0);

	if (php_copy_file_ctx(source, target, 0, context) == SUCCESS) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

/**
 * 
 *
 * @param string $source
 * @param string $target
 * @return boolean
 */
PHP_METHOD(Phalcon_Files, move){
	char *old_name, *new_name;
	size_t old_name_len, new_name_len;
	zval *zcontext = NULL;
	php_stream_wrapper *wrapper;
	php_stream_context *context;

	ZEND_PARSE_PARAMETERS_START(2, 3)
		Z_PARAM_PATH(old_name, old_name_len)
		Z_PARAM_PATH(new_name, new_name_len)
		Z_PARAM_OPTIONAL
		Z_PARAM_RESOURCE_EX(zcontext, 1, 0)
	ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

	wrapper = php_stream_locate_url_wrapper(old_name, NULL, 0);

	if (!wrapper || !wrapper->wops) {
		php_error_docref(NULL, E_WARNING, "Unable to locate stream wrapper");
		RETURN_FALSE;
	}

	if (!wrapper->wops->rename) {
		php_error_docref(NULL, E_WARNING, "%s wrapper does not support renaming", wrapper->wops->label ? wrapper->wops->label : "Source");
		RETURN_FALSE;
	}

	if (wrapper != php_stream_locate_url_wrapper(new_name, NULL, 0)) {
		php_error_docref(NULL, E_WARNING, "Cannot rename a file across wrapper types");
		RETURN_FALSE;
	}

	context = php_stream_context_from_zval(zcontext, 0);

	RETURN_BOOL(wrapper->wops->rename(wrapper, old_name, new_name, 0, context));
}

typedef enum {
	RIT_LEAVES_ONLY = 0,
	RIT_SELF_FIRST  = 1,
	RIT_CHILD_FIRST = 2
} RecursiveIteratorMode;

int rmtree_iterator(zend_object_iterator *iter, void *puser)
{
	zval *value;

	value = iter->funcs->get_current_data(iter);
	if (EG(exception)) {
		return ZEND_HASH_APPLY_STOP;
	}
	if (!value) {
		zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Iterator returned no value");
		return ZEND_HASH_APPLY_STOP;
	}

	switch (Z_TYPE_P(value)) {
		case IS_STRING:
			break;
		case IS_OBJECT:
		{
			char *fname;
			zval dummy;
			zend_bool is_dir = 0;
			spl_filesystem_object *intern = (spl_filesystem_object*)spl_filesystem_from_obj(Z_OBJ_P(value));
			switch (intern->type) {
				case SPL_FS_DIR:
				case SPL_FS_FILE:
				case SPL_FS_INFO:
#if PHP_VERSION_ID >= 80100
					php_stat(intern->file_name, FS_IS_DIR, &dummy);
					is_dir = zend_is_true(&dummy);
					fname = expand_filepath(ZSTR_VAL(intern->file_name), NULL);
#else
					php_stat(intern->file_name, intern->file_name_len, FS_IS_DIR, &dummy);
					is_dir = zend_is_true(&dummy);
					fname = expand_filepath(intern->file_name, NULL);
#endif
					if (!fname) {
						zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Could not resolve file path");
						return ZEND_HASH_APPLY_STOP;
					}
					if (is_dir) {
						php_stream_rmdir(fname, REPORT_ERRORS, NULL);
					} else {
						phalcon_unlink_str(fname);
					}
					efree(fname);
				default:
					return ZEND_HASH_APPLY_KEEP;
			}
		}
		default:
			zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Iterator returned an invalid value (must return a string)");
			return ZEND_HASH_APPLY_STOP;
	}
	return ZEND_HASH_APPLY_KEEP;
}

int recursive_directory_iterator_create(zval *iter, zval *path, long options)
{
	zval arg = {};
	ZVAL_LONG(&arg, options);
	if (SUCCESS != object_init_ex(iter, spl_ce_RecursiveDirectoryIterator)) {
		zend_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Unable to instantiate RecursiveDirectoryIterator for %s", Z_STRVAL_P(path));
		return FAILURE;
	}
#if PHP_VERSION_ID >= 80000
	zend_call_method_with_2_params(Z_OBJ_P(iter), spl_ce_RecursiveDirectoryIterator, &spl_ce_RecursiveDirectoryIterator->constructor, "__construct", NULL, path, &arg);
#else
	zend_call_method_with_2_params(iter, spl_ce_RecursiveDirectoryIterator, &spl_ce_RecursiveDirectoryIterator->constructor, "__construct", NULL, path, &arg);
#endif
	if (EG(exception)) {
		return FAILURE;
	}
	return SUCCESS;
}

int recursive_iterator_iterator_create(zval *iteriter, zval *iter, long options)
{
	zval arg = {};
	ZVAL_LONG(&arg, options);
	if (SUCCESS != object_init_ex(iteriter, spl_ce_RecursiveIteratorIterator)) {
		zend_throw_exception_ex(spl_ce_BadMethodCallException, 0, "Unable to instantiate RecursiveIteratorIterator.");
		return FAILURE;
	}

#if PHP_VERSION_ID >= 80000
	zend_call_method_with_2_params(Z_OBJ_P(iteriter), spl_ce_RecursiveIteratorIterator, &spl_ce_RecursiveIteratorIterator->constructor, "__construct", NULL, iter, &arg);
#else
	zend_call_method_with_2_params(iteriter, spl_ce_RecursiveIteratorIterator, &spl_ce_RecursiveIteratorIterator->constructor, "__construct", NULL, iter, &arg);
#endif
	if (EG(exception)) {
		return FAILURE;
	}
	return SUCCESS;
}

/**
 * 
 *
 * @param string $path
 * @return boolean
 */
PHP_METHOD(Phalcon_Files, delete){

	zval *path, *recursive = NULL, iter = {}, iteriter = {};
	int puser = 0;

	phalcon_fetch_params(0, 1, 1, &path, &recursive);
	if (recursive == NULL) {
		recursive = &PHALCON_GLOBAL(z_false);
	}
	if (phalcon_is_file(path)) {
		phalcon_unlink(return_value, path);
		return;
	}

	if (!zend_is_true(recursive)) {
		php_stream_rmdir(Z_STRVAL_P(path), REPORT_ERRORS, NULL);
		RETURN_TRUE;
	}

	if (recursive_directory_iterator_create(&iter, path, SPL_FILE_DIR_SKIPDOTS|SPL_FILE_DIR_UNIXPATHS) == FAILURE) {
		zval_ptr_dtor(&iter);
		RETURN_FALSE;
	}

	if (recursive_iterator_iterator_create(&iteriter, &iter, RIT_CHILD_FIRST) == FAILURE ) {
		zval_ptr_dtor(&iter);
		zval_ptr_dtor(&iteriter);
		RETURN_FALSE;
	}

	if (SUCCESS == spl_iterator_apply(&iteriter, (spl_iterator_apply_func_t) rmtree_iterator, (void *) &puser)) {
		if (phalcon_is_dir(path) ) {
			RETURN_BOOL(php_stream_rmdir(Z_STRVAL_P(path), REPORT_ERRORS, NULL));
		}
		zval_ptr_dtor(&iter);
		zval_ptr_dtor(&iteriter);
		RETURN_TRUE;
	}
	zval_ptr_dtor(&iter);
	zval_ptr_dtor(&iteriter);
	RETURN_FALSE;
}

int list_iterator(zend_object_iterator *iter, void *puser)
{
	zval *return_value = (zval*)puser;
	zval *value;

	value = iter->funcs->get_current_data(iter);
	if (EG(exception)) {
		return ZEND_HASH_APPLY_STOP;
	}
	if (!value) {
		zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Iterator returned no value");
		return ZEND_HASH_APPLY_STOP;
	}

	switch (Z_TYPE_P(value)) {
		case IS_STRING:
			phalcon_array_append(return_value, value, PH_COPY);
			break;
		case IS_OBJECT:
		{
			zval filename = {};
			spl_filesystem_object *intern = (spl_filesystem_object*)spl_filesystem_from_obj(Z_OBJ_P(value));
#if PHP_VERSION_ID >= 80100
			ZVAL_STR(&filename, intern->file_name);
#else
			ZVAL_STRINGL(&filename, intern->file_name, intern->file_name_len);
#endif
			phalcon_array_append(return_value, &filename, 0);
			break;
		}
		default:
			zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Iterator returned an invalid value (must return a string)");
			return ZEND_HASH_APPLY_STOP;
	}
	return ZEND_HASH_APPLY_KEEP;
}

int list_file_iterator(zend_object_iterator *iter, void *puser)
{
	zval *return_value = (zval*)puser;
	zval *value;

	value = iter->funcs->get_current_data(iter);
	if (EG(exception)) {
		return ZEND_HASH_APPLY_STOP;
	}
	if (!value) {
		zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Iterator returned no value");
		return ZEND_HASH_APPLY_STOP;
	}

	switch (Z_TYPE_P(value)) {
		case IS_STRING:
			break;
		case IS_OBJECT:
		{
			zval filename = {}, dummy = {};
			spl_filesystem_object *intern = (spl_filesystem_object*)spl_filesystem_from_obj(Z_OBJ_P(value));
			switch (intern->type) {
				case SPL_FS_DIR:
				case SPL_FS_FILE:
				case SPL_FS_INFO:
#if PHP_VERSION_ID >= 80100
					php_stat(intern->file_name, FS_IS_FILE, &dummy);
					if (zend_is_true(&dummy)) {
						ZVAL_STR(&filename, intern->file_name);
						phalcon_array_append(return_value, &filename, 0);
					}
#else
					php_stat(intern->file_name, intern->file_name_len, FS_IS_FILE, &dummy);
					if (zend_is_true(&dummy)) {
						ZVAL_STRINGL(&filename, intern->file_name, intern->file_name_len);
						phalcon_array_append(return_value, &filename, 0);
					}
#endif
					break;
				default:
					break;
			}
			break;
		}
		default:
			break;
	}
	return ZEND_HASH_APPLY_KEEP;
}

int list_dir_iterator(zend_object_iterator *iter, void *puser)
{
	zval *return_value = (zval*)puser;
	zval *value;

	value = iter->funcs->get_current_data(iter);
	if (EG(exception)) {
		return ZEND_HASH_APPLY_STOP;
	}
	if (!value) {
		zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Iterator returned no value");
		return ZEND_HASH_APPLY_STOP;
	}

	switch (Z_TYPE_P(value)) {
		case IS_STRING:
			break;
		case IS_OBJECT:
		{
			zval filename = {}, dummy = {};
			spl_filesystem_object *intern = (spl_filesystem_object*)spl_filesystem_from_obj(Z_OBJ_P(value));
			switch (intern->type) {
				case SPL_FS_DIR:
				case SPL_FS_FILE:
				case SPL_FS_INFO:
					php_stat(intern->file_name, intern->file_name_len, FS_IS_DIR, &dummy);
					if (zend_is_true(&dummy)) {
#if PHP_VERSION_ID >= 80100
						ZVAL_STR(&filename, intern->file_name);
#else
						ZVAL_STRINGL(&filename, intern->file_name, intern->file_name_len);
#endif
						phalcon_array_append(return_value, &filename, 0);
					}
					break;
				default:
					break;
			}
			break;
		}
		default:
			break;
	}
	return ZEND_HASH_APPLY_KEEP;
}

/**
 * 
 *
 * @param string $path
 * @param int $mode
 * @return boolean
 */
PHP_METHOD(Phalcon_Files, list){

    char *dirname = NULL;
    size_t dirname_len = 0;
	zend_long mode = 0;
	zend_bool recursive=0;
    php_stream * stream;

    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|lb", &dirname, &dirname_len, &mode, &recursive) == FAILURE) {
        php_error_docref(NULL, E_WARNING, "Wrong parameters.");
        RETURN_FALSE;
    }

    if (dirname_len < 1) {
        php_error_docref(NULL, E_WARNING, "Directory name cannot be empty");
        RETURN_FALSE;
    }

    if (!phalcon_is_dir_str(dirname) ) {
        RETURN_FALSE;
    }

	if (recursive) {
		zval path = {}, iter = {}, iteriter = {};
		ZVAL_STRINGL(&path, dirname, dirname_len);

		if (recursive_directory_iterator_create(&iter, &path, SPL_FILE_DIR_SKIPDOTS|SPL_FILE_DIR_UNIXPATHS) == FAILURE) {
			zval_ptr_dtor(&path);
			zval_ptr_dtor(&iter);
			RETURN_FALSE;
		}

		if (recursive_iterator_iterator_create(&iteriter, &iter, RIT_CHILD_FIRST) == FAILURE ) {
			zval_ptr_dtor(&path);
			zval_ptr_dtor(&iter);
			zval_ptr_dtor(&iteriter);
			RETURN_FALSE;
		}

		array_init(return_value);
		switch (mode) {
			case 1:	// file
				spl_iterator_apply(&iteriter, (spl_iterator_apply_func_t) list_file_iterator, (void *) return_value);
				break;
			case 2:	// dir
				spl_iterator_apply(&iteriter, (spl_iterator_apply_func_t) list_dir_iterator, (void *) return_value);
				break;
			default:
				spl_iterator_apply(&iteriter, (spl_iterator_apply_func_t) list_iterator, (void *) return_value);
				break;

		}
		zval_ptr_dtor(&path);
		zval_ptr_dtor(&iter);
		zval_ptr_dtor(&iteriter);
		return;
	}

    stream = php_stream_opendir(dirname, REPORT_ERRORS, NULL );
    if (!stream) {
        RETURN_FALSE;
    }
    // it's not fclose-able
    stream->flags |= PHP_STREAM_FLAG_NO_FCLOSE;

    array_init(return_value);

    php_stream_dirent entry;
    while (php_stream_readdir(stream, &entry)) {
		zval path = {};
        if (strcmp(entry.d_name, "..") == 0 || strcmp(entry.d_name, ".") == 0) {
            continue;
        }
		phalcon_path_concat(&path, dirname, dirname_len, entry.d_name, strlen(entry.d_name));
		switch (mode) {
			case 1:	// file
				if (phalcon_is_file(&path)) {
					add_next_index_string(return_value, entry.d_name);
				}
				break;
			case 2:	// dir
				if (phalcon_is_dir(&path)) {
					add_next_index_string(return_value, entry.d_name);
				}
				break;
			default:
				
				add_next_index_string(return_value, entry.d_name);
				break;

		}
		zval_ptr_dtor(&path);
    }
    php_stream_close(stream);
}

/**
 * Returns the file extension
 *
 * @param string $filename
 * @return string
 */
PHP_METHOD(Phalcon_Files, getExtension){
	char *filename = NULL;
	size_t filename_len = 0;
	char *dot;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &filename, &filename_len) == FAILURE) {
        php_error_docref(NULL, E_WARNING, "Wrong parameters.");
        RETURN_FALSE;
    }

	if (filename_len < 1) {
		RETURN_FALSE;
	}

	dot = strrchr(filename, (int) '.');
	if ( dot != NULL ) {
		char *extension;
		int extension_len;
		extension_len = filename_len - (dot - filename) - 1;

		if ( extension_len == 0 )
			RETURN_FALSE;

		extension = dot + 1;
		RETURN_STRINGL(extension, extension_len);
	}
	RETURN_FALSE;
}


/**
 * Replace the file extension
 *
 * @param string $filename
 * @param string $extension
 * @return string
 */
PHP_METHOD(Phalcon_Files, setExtension){
	char *filename, *extension, *dot;
	size_t filename_len;
	size_t extension_len;

	zend_string *newfilename = NULL;
	size_t newfilename_len;

	size_t basename_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss", &filename, &filename_len, &extension, &extension_len) == FAILURE) {
		php_error_docref(NULL, E_WARNING, "Wrong parameters.");
		RETURN_FALSE;
	}

	if ( filename_len < 1 ) {
		RETURN_FALSE;
	}

	dot = strrchr(filename, (int) '.');
	if ( dot != NULL ) {
		basename_len = dot - filename + 1; // contains the dot
		newfilename_len = basename_len + extension_len;
		newfilename = zend_string_alloc(newfilename_len, 0);

		memcpy(ZSTR_VAL(newfilename), filename, basename_len);
	} else {
		basename_len = filename_len;
		newfilename_len = basename_len + extension_len + 1;
		newfilename = zend_string_alloc(newfilename_len, 0);

		memcpy(ZSTR_VAL(newfilename), filename, basename_len);
		*(ZSTR_VAL(newfilename) + basename_len) = '.';
		basename_len++;
	}

	memcpy(ZSTR_VAL(newfilename) + basename_len, extension, extension_len );
	*(ZSTR_VAL(newfilename) + newfilename_len) = '\0';
	ZVAL_STR(return_value, newfilename);
}
