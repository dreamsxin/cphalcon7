
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
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_kernel_setmessages, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, messages, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_kernel_method_entry[] = {
	PHP_ME(Phalcon_Kernel, preComputeHashKey,   arginfo_phalcon_kernel_precomputehashkey, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, preComputeHashKey32, arginfo_phalcon_kernel_precomputehashkey, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, preComputeHashKey64, arginfo_phalcon_kernel_precomputehashkey, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, setBasePath, arginfo_phalcon_kernel_setbasepath, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, setMessagesDir, arginfo_phalcon_kernel_setmessagesdir, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, message, arginfo_phalcon_kernel_message, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, setMessages, arginfo_phalcon_kernel_setmessages, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Kernel, getMessages, arginfo_phalcon_kernel_setmessages, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
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
	zend_declare_property_string(phalcon_kernel_ce, SL("_messagesDir"), "messages/", ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);

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

	zval *base_path;

	phalcon_fetch_params(0, 1, 0, &base_path);

	phalcon_add_trailing_slash(base_path);
	phalcon_update_static_property_ce(phalcon_kernel_ce, SL("_basePath"), base_path);
}

/**
 * Sets base path
 *
 * @param string $messagesDir
 */
PHP_METHOD(Phalcon_Kernel, setMessagesDir){

	zval *messages_dir;

	phalcon_fetch_params(0, 1, 0, &messages_dir);

	phalcon_add_trailing_slash(messages_dir);
	phalcon_update_static_property_ce(phalcon_kernel_ce, SL("_messagesDir"), messages_dir);
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
	zval default_messages = {}, validation_messages = {}, file_messages = {}, file_messages2 = {}, value = {}, dependency_injector = {}, service = {}, translate = {};

	phalcon_fetch_params(0, 1, 4, &file, &path, &default_value, &_ext, &absolute_path);

	if (!path) {
		path = &PHALCON_GLOBAL(z_null);
	}

	if (!default_value) {
		default_value = &PHALCON_GLOBAL(z_null);
	}

	if (!_ext || Z_TYPE_P(_ext) == IS_NULL) {
		ZVAL_STRING(&ext, "php");
	} else {
		ZVAL_COPY_VALUE(&ext, _ext);
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

	phalcon_read_static_property_ce(&messages, phalcon_kernel_ce, SL("_messages"), PH_READONLY);
	if (Z_TYPE(messages) != IS_ARRAY) {
		convert_to_array(&messages);
		array_init_size(&validation_messages, 35);
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
		phalcon_array_update_str_str(&validation_messages, SL("Date"),              SL("Field :field is not a valid date"), PH_COPY);

		phalcon_array_update_str_str(&validation_messages, SL("InvalidCreateAttempt"),	SL("Record cannot be created because it already exists"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("InvalidUpdateAttempt"),	SL("Record cannot be updated because it does not exist"), PH_COPY);
		phalcon_array_update_str_str(&validation_messages, SL("ConstraintViolation"),	SL("Value of field :field does not exist on referenced table"), PH_COPY);

		phalcon_array_update_str(&messages, SL("validation"), &validation_messages, PH_READONLY);

		phalcon_read_static_property_ce(&default_messages, phalcon_kernel_ce, SL("_defaultMessages"), PH_READONLY);
		if (Z_TYPE(default_messages) == IS_ARRAY) {
			phalcon_array_merge_recursive_n2(&messages, &default_messages, PH_READONLY);
		}
		phalcon_update_static_property_ce(phalcon_kernel_ce, SL("_messages"), &messages);
		zval_ptr_dtor(&messages);
	}

	if (!phalcon_array_isset_fetch(&file_messages, &messages, file, PH_READONLY)) {
		array_init(&file_messages);
		phalcon_array_update(&messages, file, &file_messages, PH_COPY);
	}

	if (!phalcon_array_isset_fetch(&file_messages2, &messages, &file_path, PH_READONLY)) {
		if (phalcon_require_ret(&file_messages2, Z_STRVAL(file_path)) != FAILURE) {
			if (Z_TYPE(file_messages2) != IS_ARRAY) {
				zend_throw_exception_ex(phalcon_exception_ce, 0, "Messages file '%s' value must be array", Z_STRVAL(file_path));
				return;
			}
			phalcon_array_merge_recursive_n(&file_messages, &file_messages2);
		} else {
			array_init(&file_messages2);
		}
		phalcon_update_static_property_array_ce(phalcon_kernel_ce, SL("_messages"), &file_path, &file_messages2);
	}
	zval_ptr_dtor(&file_path);

	if (Z_TYPE_P(path) != IS_NULL) {
		if  (Z_TYPE(file_messages) == IS_ARRAY) {
			PHALCON_CALL_CE_STATIC(&value, phalcon_arr_ce, "path", &file_messages, path, default_value);
		} else {
			ZVAL_COPY_VALUE(&value, default_value);
		}
	} else {
		RETURN_CTOR(&file_messages);
	}

	PHALCON_CALL_CE_STATIC(&dependency_injector, phalcon_di_ce, "getdefault");

	ZVAL_STRING(&service, ISV(translate));

	PHALCON_CALL_METHOD(&translate, &dependency_injector, "getshared", &service, &PHALCON_GLOBAL(z_null), &PHALCON_GLOBAL(z_true));
	if (unlikely(Z_TYPE(translate) == IS_OBJECT)) {
		PHALCON_VERIFY_INTERFACE(&translate, phalcon_translate_adapterinterface_ce);
		PHALCON_CALL_METHOD(return_value, &translate, "query", &value);
	} else {
		RETURN_CTOR(&value);
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
