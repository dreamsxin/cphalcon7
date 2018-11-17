
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
#include "di.h"
#include "translate/adapterinterface.h"

#include "kernel/main.h"
#include "kernel/require.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/concat.h"

#include "interned-strings.h"

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
PHP_METHOD(Phalcon_Kernel, setMessagesDir);
PHP_METHOD(Phalcon_Kernel, message);
PHP_METHOD(Phalcon_Kernel, setMessages);
PHP_METHOD(Phalcon_Kernel, getMessages);
PHP_METHOD(Phalcon_Kernel, setAliasDir);
PHP_METHOD(Phalcon_Kernel, alias);
PHP_METHOD(Phalcon_Kernel, setAlias);
PHP_METHOD(Phalcon_Kernel, getAlias);
PHP_METHOD(Phalcon_Kernel, evalFile);
PHP_METHOD(Phalcon_Kernel, evalString);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_kernel_precomputehashkey, 0, 0, 1)
	ZEND_ARG_INFO(0, arrKey)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_kernel_setbasepath, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_kernel_setmessagesdir, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_kernel_message, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, file, IS_STRING, 0)
	ZEND_ARG_INFO(0, key_path)
	ZEND_ARG_INFO(0, default_value)
	ZEND_ARG_TYPE_INFO(0, ext, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, absolute_path, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_kernel_setmessages, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, messages, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_kernel_setaliasdir, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_kernel_alias, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, file, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, str, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, ext, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, absolute_path, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_kernel_setalias, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, alias, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_kernel_evalfile, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, vars, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, object, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_kernel_evalstring, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, vars, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, object, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_kernel_method_entry[] = {
	PHP_ME(Phalcon_Kernel, preComputeHashKey,   arginfo_phalcon_kernel_precomputehashkey, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, preComputeHashKey32, arginfo_phalcon_kernel_precomputehashkey, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, preComputeHashKey64, arginfo_phalcon_kernel_precomputehashkey, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, setBasePath, arginfo_phalcon_kernel_setbasepath, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, setMessagesDir, arginfo_phalcon_kernel_setmessagesdir, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, message, arginfo_phalcon_kernel_message, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, setMessages, arginfo_phalcon_kernel_setmessages, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, getMessages, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, setAliasDir, arginfo_phalcon_kernel_setaliasdir, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, alias, arginfo_phalcon_kernel_alias, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, setAlias, arginfo_phalcon_kernel_setalias, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, getAlias, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, evalFile, arginfo_phalcon_kernel_evalfile, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, evalString, arginfo_phalcon_kernel_evalstring, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Kernel initializer
 */
PHALCON_INIT_CLASS(Phalcon_Kernel){

	PHALCON_REGISTER_CLASS(Phalcon, Kernel, kernel, phalcon_kernel_method_entry, 0);

	zend_declare_property_null(phalcon_kernel_ce, SL("_defaultMessages"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_kernel_ce, SL("_basePath"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_kernel_ce, SL("_messages"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_string(phalcon_kernel_ce, SL("_messagesDir"), "messages/", ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_kernel_ce, SL("_alias"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_string(phalcon_kernel_ce, SL("_aliasDir"), "alias/", ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);

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
 */
PHP_METHOD(Phalcon_Kernel, setBasePath){

	zval *base_path, path = {};

	phalcon_fetch_params(0, 1, 0, &base_path);

	phalcon_add_trailing_slash(&path, base_path);
	phalcon_update_static_property_ce(phalcon_kernel_ce, SL("_basePath"), &path);
	zval_ptr_dtor(&path);
}

/**
 * Sets messages dir
 *
 * @param string $messagesDir
 */
PHP_METHOD(Phalcon_Kernel, setMessagesDir){

	zval *messages_dir, path = {};

	phalcon_fetch_params(0, 1, 0, &messages_dir);

	phalcon_add_trailing_slash(&path, messages_dir);
	phalcon_update_static_property_ce(phalcon_kernel_ce, SL("_messagesDir"), &path);
	zval_ptr_dtor(&path);
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

	zval *file, *path = NULL, *default_value = NULL, *_ext = NULL, *absolute_path = NULL, ext = {}, messages = {}, base_path = {}, messages_dir = {}, file_path = {};
	zval default_messages = {}, validation_messages = {}, file_messages = {}, value = {}, dependency_injector = {}, service = {}, translate = {};

	phalcon_fetch_params(1, 1, 4, &file, &path, &default_value, &_ext, &absolute_path);

	if (!path) {
		path = &PHALCON_GLOBAL(z_null);
	}

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	if (!_ext || Z_TYPE_P(_ext) == IS_NULL) {
		PHALCON_MM_ZVAL_STRING(&ext, "php");
	} else {
		PHALCON_MM_ZVAL_COPY(&ext, _ext);
	}

	if (!absolute_path) {
		absolute_path = &PHALCON_GLOBAL(z_false);
	}

	if (unlikely(zend_is_true(absolute_path))) {
		PHALCON_CONCAT_VSV(&file_path, file, ".", &ext);
	} else {
		phalcon_read_static_property_ce(&base_path, phalcon_kernel_ce, SL("_basePath"), PH_READONLY);
		phalcon_read_static_property_ce(&messages_dir, phalcon_kernel_ce, SL("_messagesDir"), PH_READONLY);

		PHALCON_CONCAT_VVVSV(&file_path, &base_path, &messages_dir, file, ".", &ext);
	}
	PHALCON_MM_ADD_ENTRY(&file_path);

	phalcon_read_static_property_ce(&messages, phalcon_kernel_ce, SL("_messages"), PH_READONLY);
	if (Z_TYPE(messages) != IS_ARRAY) {
		array_init_size(&validation_messages, 35);
		phalcon_array_update_str_str(&validation_messages, SL("Alnum"),             SL("Field :field must contain only letters and numbers"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("Alpha"),             SL("Field :field must contain only letters"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("Between"),           SL("Field :field must be within the range of :min to :max"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("Confirmation"),      SL("Field :field must be the same as :with"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("Digit"),             SL("Field :field must be numeric"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("Numericality"),      SL("Field :field does not have a valid numeric format"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("Email"),             SL("Field :field must be an email address"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("ExclusionIn"),       SL("Field :field must not be a part of list: :domain"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("FileEmpty"),         SL("Field :field must not be empty"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("FileIniSize"),       SL("File :field exceeds the maximum file size"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("FileMaxResolution"), SL("File :field must not exceed :max resolution"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("FileMinResolution"), SL("File :field must be at least :min resolution"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("FileSize"),          SL("File :field exceeds the size of :max"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("FileMaxSize"),       SL("File :field the size must not exceed :max"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("FileMinSize"),       SL("File :field the size must be at least :min"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("FileType"),          SL("File :field must be of type: :types"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("FileValid"),         SL("Field :field is not valid"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("ImageMaxWidth"),     SL("Image :field the width must not exceed :max"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("ImageMinWidth"),     SL("Image :field the width must be at least :min"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("ImageMaxHeight"),    SL("Image :field the height must not exceed :max"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("ImageMinHeight"),    SL("Image :field the height must be at least :min"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("Identical"),         SL("Field :field does not have the expected value"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("InclusionIn"),       SL("Field :field must be a part of list: :domain"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("PresenceOf"),        SL("Field :field is required"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("Regex"),             SL("Field :field does not match the required format"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("TooLong"),           SL("Field :field must not exceed :max characters long"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("TooShort"),          SL("Field :field must be at least :min characters long"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("TooLarge"),          SL("Field :field scale is out of range"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("Uniqueness"),        SL("Field :field must be unique"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("Url"),               SL("Field :field must be a url"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("Json"),              SL("Field :field must be a json"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("Date"),              SL("Field :field is not a valid date"), 0);

		phalcon_array_update_str_str(&validation_messages, SL("InvalidCreateAttempt"),	SL("Record cannot be created because it already exists"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("InvalidUpdateAttempt"),	SL("Record cannot be updated because it does not exist"), 0);
		phalcon_array_update_str_str(&validation_messages, SL("ConstraintViolation"),	SL("Value of field :field does not exist on referenced table"), 0);

		array_init(&messages);
		phalcon_array_update_str(&messages, SL("validation"), &validation_messages, 0);

		phalcon_read_static_property_ce(&default_messages, phalcon_kernel_ce, SL("_defaultMessages"), PH_READONLY);
		if (Z_TYPE(default_messages) == IS_ARRAY) {
			phalcon_array_merge_recursive_n(&messages, &default_messages);
		}
		phalcon_update_static_property_ce(phalcon_kernel_ce, SL("_messages"), &messages);
		zval_ptr_dtor(&messages);
	}

	if (!phalcon_array_isset_fetch(&file_messages, &messages, &file_path, PH_READONLY)) {
		zval file_messages2 = {};
		if (!phalcon_array_isset_fetch(&file_messages, &messages, file, PH_READONLY)) {
			array_init(&file_messages);
			phalcon_array_update(&messages, &file_path, &file_messages, 0);
		}
		if (Z_TYPE(file_messages) != IS_ARRAY) {
			zend_throw_exception_ex(phalcon_exception_ce, 0, "Messages file '%s' value must be array", Z_STRVAL(file_path));
			RETURN_MM();
		}
		if (phalcon_require_ret(&file_messages2, Z_STRVAL(file_path)) != FAILURE) {
			phalcon_array_merge_recursive_n(&file_messages, &file_messages2);
			PHALCON_MM_ADD_ENTRY(&file_messages2);
		}
	}

	if (Z_TYPE_P(path) != IS_NULL) {
		PHALCON_MM_CALL_CE_STATIC(&dependency_injector, phalcon_di_ce, "getdefault");
		PHALCON_MM_ADD_ENTRY(&dependency_injector);
		ZVAL_STR(&service, IS(translate));

		PHALCON_MM_CALL_METHOD(&translate, &dependency_injector, "getshared", &service, &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_true));
		PHALCON_MM_ADD_ENTRY(&translate);
		if (unlikely(Z_TYPE(translate) == IS_OBJECT)) {
			PHALCON_MM_VERIFY_INTERFACE(&translate, phalcon_translate_adapterinterface_ce);
		}

		if  (Z_TYPE(file_messages) == IS_ARRAY) {
			PHALCON_MM_CALL_CE_STATIC(&value, phalcon_arr_ce, "path", &file_messages, path, default_value);
			PHALCON_MM_ADD_ENTRY(&value);
		} else {
			PHALCON_MM_ZVAL_COPY(&value, default_value);
		}

		if (unlikely(Z_TYPE(translate) == IS_OBJECT)) {
			PHALCON_MM_CALL_METHOD(return_value, &translate, "query", &value);
			RETURN_MM();
		} else {
			RETURN_MM_CTOR(&value);
		}
	} else {
		RETURN_MM_CTOR(&file_messages);
	}
}

/**
 * Sets the messages
 *
 * @param array messages
 */
PHP_METHOD(Phalcon_Kernel, setMessages){

	zval *messages;

	phalcon_fetch_params(0, 1, 0, &messages);

	phalcon_update_static_property_ce(phalcon_kernel_ce, SL("_messages"), messages);
}


/**
 * Get the messages
 *
 * @return array
 */
PHP_METHOD(Phalcon_Kernel, getMessages){


	phalcon_read_static_property_ce(return_value, phalcon_kernel_ce, SL("_messages"), PH_COPY);
}

/**
 * Sets alias dir
 *
 * @param string $aliasDir
 */
PHP_METHOD(Phalcon_Kernel, setAliasDir){

	zval *alias_dir, path = {};

	phalcon_fetch_params(0, 1, 0, &alias_dir);

	phalcon_add_trailing_slash(&path, alias_dir);
	phalcon_update_static_property_ce(phalcon_kernel_ce, SL("_aliasDir"), &path);
	zval_ptr_dtor(&path);
}

/**
 * Resolver alias from a file.
 *
 * @param string $file file name
 * @param string $str
 * @return string|array
 */
PHP_METHOD(Phalcon_Kernel, alias){

	zval *file, *str, *_ext = NULL, *absolute_path = NULL, ext = {}, alias = {}, base_path = {}, alias_dir = {}, file_path = {};
	zval file_alias = {};

	phalcon_fetch_params(0, 2, 2, &file, &str, &_ext, &absolute_path);

	if (!_ext || Z_TYPE_P(_ext) == IS_NULL) {
		ZVAL_STRING(&ext, "php");
	} else {
		ZVAL_COPY(&ext, _ext);
	}

	if (!absolute_path) {
		absolute_path = &PHALCON_GLOBAL(z_false);
	}

	if (unlikely(zend_is_true(absolute_path))) {
		PHALCON_CONCAT_VSV(&file_path, file, ".", &ext);
	} else {
		phalcon_read_static_property_ce(&base_path, phalcon_kernel_ce, SL("_basePath"), PH_READONLY);
		phalcon_read_static_property_ce(&alias_dir, phalcon_kernel_ce, SL("_aliasDir"), PH_READONLY);

		PHALCON_CONCAT_VVVSV(&file_path, &base_path, &alias_dir, file, ".", &ext);
	}
	zval_ptr_dtor(&ext);

	phalcon_read_static_property_ce(&alias, phalcon_kernel_ce, SL("_alias"), PH_READONLY);
	if (Z_TYPE(alias) != IS_ARRAY) {
		convert_to_array(&alias);
		phalcon_update_static_property_ce(phalcon_kernel_ce, SL("_alias"), &alias);
		zval_ptr_dtor(&alias);
	}

	if (!phalcon_array_isset_fetch(&file_alias, &alias, &file_path, PH_READONLY)) {
		if (phalcon_require_ret(&file_alias, Z_STRVAL(file_path)) != FAILURE) {
			if (Z_TYPE(file_alias) != IS_ARRAY) {
				zend_throw_exception_ex(phalcon_exception_ce, 0, "Messages file '%s' value must be array", Z_STRVAL(file_path));
				return;
			}
		} else {
			array_init(&file_alias);
		}
		phalcon_update_static_property_array_ce(phalcon_kernel_ce, SL("_alias"), &file_path, &file_alias);
		zval_ptr_dtor(&file_alias);
	}
	zval_ptr_dtor(&file_path);

	if  (Z_TYPE(file_alias) == IS_ARRAY) {
		PHALCON_CALL_FUNCTION(return_value, "strtr", str, &file_alias);
	} else {
		RETURN_CTOR(str);
	}
}

/**
 * Sets the alias
 *
 * @param array alias
 */
PHP_METHOD(Phalcon_Kernel, setAlias){

	zval *alias;

	phalcon_fetch_params(0, 1, 0, &alias);

	phalcon_update_static_property_ce(phalcon_kernel_ce, SL("_alias"), alias);
}


/**
 * Get the alias
 *
 * @return array
 */
PHP_METHOD(Phalcon_Kernel, getAlias){


	phalcon_read_static_property_ce(return_value, phalcon_kernel_ce, SL("_alias"), PH_COPY);
}

/**
 * Evaluate a PHP file
 *
 * @param string $filename
 * @param array $vars
 * @param object $object
 */
PHP_METHOD(Phalcon_Kernel, evalFile){

	zval *file, *vars = NULL, *object = NULL;

	phalcon_fetch_params(0, 1, 2, &file, &vars, &object);

	phalcon_exec_file(return_value, object, file, vars);
}

/**
 * Evaluate a string as PHP code
 *
 * @param string $code
 * @param array $vars
 * @param object $object
 */
PHP_METHOD(Phalcon_Kernel, evalString){

	zval *code, *vars = NULL, *object = NULL;

	phalcon_fetch_params(0, 1, 2, &code, &vars, &object);

	phalcon_exec_code(return_value, object, code, vars);
}
