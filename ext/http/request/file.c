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

#include "http/request/file.h"
#include "http/request/fileinterface.h"
#include "http/request/exception.h"

#include <main/SAPI.h>
#include <ext/spl/spl_directory.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/file.h"

/**
 * Phalcon\Http\Request\File
 *
 * Provides OO wrappers to the $_FILES superglobal
 *
 *<code>
 *	class PostsController extends \Phalcon\Mvc\Controller
 *	{
 *
 *		public function uploadAction()
 *		{
 *			//Check if the user has uploaded files
 *			if ($this->request->hasFiles() == true) {
 *				//Print the real file names and their sizes
 *				foreach ($this->request->getUploadedFiles() as $file){
 *					echo $file->getName(), " ", $file->getSize(), "\n";
 *				}
 *			}
 *		}
 *
 *	}
 *</code>
 */
zend_class_entry *phalcon_http_request_file_ce;

PHP_METHOD(Phalcon_Http_Request_File, __construct);
PHP_METHOD(Phalcon_Http_Request_File, getSize);
PHP_METHOD(Phalcon_Http_Request_File, getName);
PHP_METHOD(Phalcon_Http_Request_File, getTempName);
PHP_METHOD(Phalcon_Http_Request_File, getType);
PHP_METHOD(Phalcon_Http_Request_File, getRealType);
PHP_METHOD(Phalcon_Http_Request_File, getError);
PHP_METHOD(Phalcon_Http_Request_File, getKey);
PHP_METHOD(Phalcon_Http_Request_File, isUploadedFile);
PHP_METHOD(Phalcon_Http_Request_File, moveTo);
PHP_METHOD(Phalcon_Http_Request_File, __set_state);
PHP_METHOD(Phalcon_Http_Request_File, getExtension);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_request_file___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, file)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_http_request_file___set_state, 0, 0, 1)
	ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_http_request_file_method_entry[] = {
	PHP_ME(Phalcon_Http_Request_File, __construct, arginfo_phalcon_http_request_file___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Http_Request_File, getSize, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request_File, getName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request_File, getTempName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request_File, getType, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request_File, getRealType, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request_File, getError, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request_File, getKey, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request_File, isUploadedFile, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request_File, moveTo, arginfo_phalcon_http_request_fileinterface_moveto, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Http_Request_File, __set_state, arginfo_phalcon_http_request_file___set_state, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Http_Request_File, getExtension, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Http\Request\File initializer
 */
PHALCON_INIT_CLASS(Phalcon_Http_Request_File){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Http\\Request, File, http_request_file, spl_ce_SplFileInfo, phalcon_http_request_file_method_entry, 0);

	zend_declare_property_null(phalcon_http_request_file_ce, SL("_name"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_request_file_ce, SL("_tmp"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_request_file_ce, SL("_size"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_request_file_ce, SL("_type"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_request_file_ce, SL("_real_type"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_request_file_ce, SL("_error"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_request_file_ce, SL("_key"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_http_request_file_ce, SL("_extension"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_http_request_file_ce, 1, phalcon_http_request_fileinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Http\Request\File constructor
 *
 * @param array $file
 */
PHP_METHOD(Phalcon_Http_Request_File, __construct){

	zval *file, *key = NULL, name = {}, temp_name = {}, size = {}, type = {}, error = {}, *constant, extension = {};

	phalcon_fetch_params(0, 1, 1, &file, &key);

	if (Z_TYPE_P(file) != IS_ARRAY) {
		if (phalcon_file_exists(file) == FAILURE) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_http_request_exception_ce, "Phalcon\\Http\\Request\\File requires a valid uploaded file");
			return;
		}

		phalcon_update_property(getThis(), SL("_tmp"), file);

		PHALCON_CALL_PARENT(NULL, phalcon_http_request_file_ce, getThis(), "__construct", file);
	} else {
		if (phalcon_array_isset_fetch_str(&name, file, SL("name"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_name"), &name);

			if ((constant = zend_get_constant_str(SL("PATHINFO_EXTENSION"))) != NULL) {
				PHALCON_CALL_FUNCTION(&extension, "pathinfo", &name, constant);
				phalcon_update_property(getThis(), SL("_extension"), &extension);
			}
		}

		if (phalcon_array_isset_fetch_str(&temp_name, file, SL("tmp_name"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_tmp"), &temp_name);
		}

		if (phalcon_array_isset_fetch_str(&size, file, SL("size"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_size"), &size);
		}

		if (phalcon_array_isset_fetch_str(&type, file, SL("type"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_type"), &type);
		}

		if (phalcon_array_isset_fetch_str(&error, file, SL("error"), PH_READONLY)) {
			phalcon_update_property(getThis(), SL("_error"), &error);
		}

		PHALCON_CALL_PARENT(NULL, phalcon_http_request_file_ce, getThis(), "__construct", &temp_name);
	}

	if (key) {
		phalcon_update_property(getThis(), SL("_key"), key);
	}
}

/**
 * Returns the file size of the uploaded file
 *
 * @return int
 */
PHP_METHOD(Phalcon_Http_Request_File, getSize){


	RETURN_MEMBER(getThis(), "_size");
}

/**
 * Returns the real name of the uploaded file
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request_File, getName){


	RETURN_MEMBER(getThis(), "_name");
}

/**
 * Returns the temporary name of the uploaded file
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request_File, getTempName){


	RETURN_MEMBER(getThis(), "_tmp");
}

/**
 * Returns the mime type reported by the browser
 * This mime type is not completely secure, use getRealType() instead
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request_File, getType){


	RETURN_MEMBER(getThis(), "_type");
}

/**
 * Gets the real mime type of the upload file using finfo
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request_File, getRealType){

	zval mime = {}, *constant, finfo = {}, temp_file = {}, ret = {};

	phalcon_read_property(&mime, getThis(), SL("_real_type"), PH_NOISY|PH_READONLY);

	if (Z_TYPE(mime) == IS_STRING) {
		RETURN_CTOR(&mime);
	}

	if ((constant = zend_get_constant_str(SL("FILEINFO_MIME_TYPE"))) == NULL) {
		RETURN_NULL();
	}

	PHALCON_CALL_FUNCTION(&finfo, "finfo_open", constant);

	if (Z_TYPE(finfo) != IS_RESOURCE) {
		RETURN_NULL();
	}

	phalcon_read_property(&temp_file, getThis(), SL("_tmp"), PH_NOISY|PH_READONLY);

	PHALCON_CALL_FUNCTION(&ret, "finfo_file", &finfo, &temp_file);
	PHALCON_CALL_FUNCTION(NULL, "finfo_close", &finfo);

	if (zend_is_true(&ret)) {
		phalcon_update_property(getThis(), SL("_real_type"), &ret);
		RETURN_CTOR(&ret);
	}

	RETURN_NULL();
}

/**
 * Returns the error code
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request_File, getError){


	RETURN_MEMBER(getThis(), "_error");
}

/**
 * Returns the file key
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request_File, getKey){

	RETURN_MEMBER(getThis(), "_key");
}

/**
 * Checks whether the file has been uploaded via Post.
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request_File, isUploadedFile) {

	zval tmp_name = {};

	if (!SG(rfc1867_uploaded_files)) {
		RETURN_FALSE;
	}

	PHALCON_CALL_SELF(&tmp_name, "gettempname");
	if (Z_TYPE(tmp_name) == IS_STRING && zend_hash_exists(SG(rfc1867_uploaded_files), Z_STR(tmp_name))) {
		RETVAL_TRUE;
	} else {
		RETVAL_FALSE;
	}
}

/**
 * Moves the temporary file to a destination within the application
 *
 * @param string $destination
 * @return boolean
 */
PHP_METHOD(Phalcon_Http_Request_File, moveTo){

	zval *destination, temp_file = {};

	phalcon_fetch_params(0, 1, 0, &destination);

	phalcon_read_property(&temp_file, getThis(), SL("_tmp"), PH_NOISY|PH_READONLY);
	PHALCON_RETURN_CALL_FUNCTION("move_uploaded_file", &temp_file, destination);
}

PHP_METHOD(Phalcon_Http_Request_File, __set_state) {

	zval *data;

	phalcon_fetch_params(0, 1, 0, &data);

	object_init_ex(return_value, phalcon_http_request_file_ce);

	PHALCON_CALL_METHOD(NULL, return_value, "__construct", data);
}

/**
 * Returns the file extension
 *
 * @return string
 */
PHP_METHOD(Phalcon_Http_Request_File, getExtension){


	RETURN_MEMBER(getThis(), "_extension");
}
