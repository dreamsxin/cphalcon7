
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

#include "kernel.h"
#include "exception.h"
#include "arr.h"

#include "kernel/main.h"
#include "kernel/require.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/concat.h"

/**
 * Phalcon\Kernel
 *
 * This class allows to change the internal behavior of the framework in runtime
 */
zend_class_entry *phalcon_kernel_ce;

PHP_METHOD(Phalcon_Kernel, preComputeHashKey);
PHP_METHOD(Phalcon_Kernel, preComputeHashKey32);
PHP_METHOD(Phalcon_Kernel, preComputeHashKey64);
PHP_METHOD(Phalcon_Kernel, setBasePath);
PHP_METHOD(Phalcon_Kernel, message);
PHP_METHOD(Phalcon_Kernel, setMessages);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_kernel_precomputehashkey, 0, 0, 1)
	ZEND_ARG_INFO(0, arrKey)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_kernel_setbasepath, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_kernel_message, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, file, IS_STRING, 0)
	ZEND_ARG_INFO(0, key_path)
	ZEND_ARG_INFO(0, default_value)
	ZEND_ARG_TYPE_INFO(0, ext, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_kernel_setmessages, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, messages, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_kernel_method_entry[] = {
	PHP_ME(Phalcon_Kernel, preComputeHashKey,   arginfo_phalcon_kernel_precomputehashkey, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, preComputeHashKey32, arginfo_phalcon_kernel_precomputehashkey, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, preComputeHashKey64, arginfo_phalcon_kernel_precomputehashkey, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, setBasePath, arginfo_phalcon_kernel_setbasepath, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, message, arginfo_phalcon_kernel_message, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, setMessages, arginfo_phalcon_kernel_setmessages, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Kernel initializer
 */
PHALCON_INIT_CLASS(Phalcon_Kernel){

	PHALCON_REGISTER_CLASS(Phalcon, Kernel, kernel, phalcon_kernel_method_entry, 0);

	zend_declare_property_null(phalcon_kernel_ce, SL("_defaultMessages"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_kernel_ce, SL("_messages"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_kernel_ce, SL("_basePath"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);

	zend_declare_class_constant_string(phalcon_kernel_ce, SL("EXT"), "php");

	return SUCCESS;
}

/**
 * Produces a pre-computed hash key based on a string. This function produce different numbers in 32bit/64bit processors
 *
 * @param string $arrKey
 * @return string
 */
PHP_METHOD(Phalcon_Kernel, preComputeHashKey){

	char *arKey, *strKey;
	unsigned int nKeyLength;
	register ulong hash = 5381;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arKey, &nKeyLength) == FAILURE) {
		RETURN_NULL();
	}

	nKeyLength++;

	/* variant with the hash unrolled eight times */
	for (; nKeyLength >= 8; nKeyLength -= 8) {
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
	}

	switch (nKeyLength) {
		case 7: hash = ((hash << 5) + hash) + *arKey++;
		/* no break */
		case 6: hash = ((hash << 5) + hash) + *arKey++;
		/* no break */
		case 5: hash = ((hash << 5) + hash) + *arKey++;
		/* no break */
		case 4: hash = ((hash << 5) + hash) + *arKey++;
		/* no break */
		case 3: hash = ((hash << 5) + hash) + *arKey++;
		/* no break */
		case 2: hash = ((hash << 5) + hash) + *arKey++;
		/* no break */
		case 1: hash = ((hash << 5) + hash) + *arKey++; break;
	}

	strKey = emalloc(24);
	snprintf(strKey, 24, "%lu", hash);

	RETURN_STRING(strKey);
}

/**
 * Produces a pre-computed hash key based on a string. This function produce a hash for a 32bits processor
 *
 * @param string $arrKey
 * @return string
 */
PHP_METHOD(Phalcon_Kernel, preComputeHashKey32){

	char *arKey, *strKey;
	unsigned int nKeyLength;
	ulong hash;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arKey, &nKeyLength) == FAILURE) {
		RETURN_NULL();
	}

	nKeyLength++;
	hash = zend_inline_hash_func(arKey, nKeyLength) & 0xFFFFFFFFul;
	strKey = emalloc(24);
	snprintf(strKey, 24, "%lu", hash);

	RETURN_STRING(strKey);
}

/**
 * Produces a pre-computed hash key based on a string. This function produce a hash for a 64bits processor
 *
 * @param string $arrKey
 * @return string
 */
PHP_METHOD(Phalcon_Kernel, preComputeHashKey64){

	char *arKey, *strKey;
	unsigned int nKeyLength;
	register unsigned long long hash = 5381;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arKey, &nKeyLength) == FAILURE) {
		RETURN_NULL();
	}

	nKeyLength++;

	/* variant with the hash unrolled eight times */
	for (; nKeyLength >= 8; nKeyLength -= 8) {
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
	}

	switch (nKeyLength) {
		case 7: hash = ((hash << 5) + hash) + *arKey++;
		/* no break */
		case 6: hash = ((hash << 5) + hash) + *arKey++;
		/* no break */
		case 5: hash = ((hash << 5) + hash) + *arKey++;
		/* no break */
		case 4: hash = ((hash << 5) + hash) + *arKey++;
		/* no break */
		case 3: hash = ((hash << 5) + hash) + *arKey++;
		/* no break */
		case 2: hash = ((hash << 5) + hash) + *arKey++;
		/* no break */
		case 1: hash = ((hash << 5) + hash) + *arKey++; break;
	}

	strKey = emalloc(24);
	snprintf(strKey, 24, "%llu", hash);

	RETURN_STRING(strKey);
}

/**
 * Sets base path
 *
 * @param string $basePath
 * @return Phalcon\Config\Adapter
 */
PHP_METHOD(Phalcon_Kernel, setBasePath){

	zval *base_path;

	phalcon_fetch_params(0, 1, 0, &base_path);

	phalcon_add_trailing_slash(base_path);
	phalcon_update_static_property_ce(phalcon_kernel_ce, SL("_basePath"), base_path);
}

/**
 * Get a message from a file. Messages are arbitrary strings that are stored
 * in the `config/messages/` directory and reference by a key
 *
 * @param string $file file name
 * @param string|array $path key path to get
 * @param mixed $default default value if the path does not exist
 * @return string|array
 */
PHP_METHOD(Phalcon_Kernel, message){

	zval *file, *path = NULL, *default_value = NULL, *_ext = NULL, ext = {}, file_messages1 = {}, *base_path, file_path = {};
	zval *_default_messages, default_messages = {}, validation_messages = {}, file_messages2 = {};

	phalcon_fetch_params(0, 1, 3, &file, &path, &default_value, &_ext);

	if (!path) {
		path = &PHALCON_GLOBAL(z_null);
	}

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	if (!_ext) {
		ZVAL_STRING(&ext, "php");
	} else {
		PHALCON_CPY_WRT(&ext, _ext);
	}

	if (!phalcon_read_static_property_array_ce(&file_messages1, phalcon_kernel_ce, SL("_messages"), file)) {
		base_path = phalcon_read_static_property_ce(phalcon_kernel_ce, SL("_basePath"));

		PHALCON_CONCAT_VSVSV(&file_path, base_path, "messages/", file, ".", &ext);
		if (phalcon_require_ret(&file_messages1, Z_STRVAL(file_path)) != FAILURE) {
			if (Z_TYPE(file_messages1) != IS_ARRAY) {
				zend_throw_exception_ex(phalcon_exception_ce, 0, "Messages file '%s' value must be array", Z_STRVAL(file_path));
				return;
			};
			phalcon_update_static_property_array_ce(phalcon_kernel_ce, SL("_messages"), file, &file_messages1);
		} else {
			array_init(&file_messages1);
		}
	}

	_default_messages = phalcon_read_static_property_ce(phalcon_kernel_ce, SL("_defaultMessages"));
	if (!_default_messages || Z_TYPE_P(_default_messages) != IS_ARRAY) {
		array_init(&default_messages);
		array_init(&validation_messages);
		phalcon_array_update_str_str(&validation_messages, SL("Alnum"),             SL("Field :field must contain only letters and numbers"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("Alpha"),             SL("Field :field must contain only letters"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("Between"),           SL("Field :field must be within the range of :min to :max"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("Confirmation"),      SL("Field :field must be the same as :with"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("Digit"),             SL("Field :field must be numeric"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("Numericality"),      SL("Field :field does not have a valid numeric format"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("Email"),             SL("Field :field must be an email address"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("ExclusionIn"),       SL("Field :field must not be a part of list: :domain"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("FileEmpty"),         SL("Field :field must not be empty"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("FileIniSize"),       SL("File :field exceeds the maximum file size"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("FileMaxResolution"), SL("File :field must not exceed :max resolution"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("FileMinResolution"), SL("File :field must be at least :min resolution"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("FileSize"),          SL("File :field exceeds the size of :max"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("FileMaxSize"),       SL("File :field the size must not exceed :max"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("FileMinSize"),       SL("File :field the size must be at least :min"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("FileType"),          SL("File :field must be of type: :types"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("FileValid"),         SL("Field :field is not valid"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("ImageMaxWidth"),     SL("Image :field the width must not exceed :max"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("ImageMinWidth"),     SL("Image :field the width must be at least :min"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("ImageMaxHeight"),    SL("Image :field the height must not exceed :max"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("ImageMinHeight"),    SL("Image :field the height must be at least :min"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("Identical"),         SL("Field :field does not have the expected value"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("InclusionIn"),       SL("Field :field must be a part of list: :domain"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("PresenceOf"),        SL("Field :field is required"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("Regex"),             SL("Field :field does not match the required format"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("TooLong"),           SL("Field :field must not exceed :max characters long"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("TooShort"),          SL("Field :field must be at least :min characters long"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("TooLarge"),          SL("Field :field scale is out of range"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("Uniqueness"),        SL("Field :field must be unique"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("Url"),               SL("Field :field must be a url"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("Json"),              SL("Field :field must be a json"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("Date"),      		SL("Field :field is not a valid date"), PH_COPY);

		phalcon_array_update_str(&default_messages, SL("validation"), &validation_messages, PH_COPY);
		phalcon_update_static_property_ce(phalcon_kernel_ce, SL("_defaultMessages"), &default_messages);
		_default_messages = &default_messages;
	}

	if (phalcon_array_isset_fetch(&file_messages2, _default_messages, file, 0)) {
		phalcon_array_merge_recursive_n(&file_messages2, &file_messages1);
	} else {
		PHALCON_CPY_WRT(&file_messages2, &file_messages1);
	}

	if (Z_TYPE_P(path) != IS_NULL) {
		if  (Z_TYPE(file_messages2) == IS_ARRAY) {
			PHALCON_CALL_CE_STATIC(return_value, phalcon_arr_ce, "path", &file_messages2, path, default_value);
		} else {
			RETURN_CTOR(default_value);
		}
	} else {
		RETURN_CTOR(&file_messages2);
	}
}

/**
 * Get a message from a file. Messages are arbitrary strings that are stored
 * in the `config/messages/` directory and reference by a key
 *
 * @param string $file file name
 */
PHP_METHOD(Phalcon_Kernel, setMessages){

	zval *messages;

	phalcon_fetch_params(0, 1, 0, &messages);

	phalcon_update_static_property_ce(phalcon_kernel_ce, SL("_messages"), messages);
}
