
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

#include "validation/validator/file.h"
#include "validation/validator.h"
#include "validation/validatorinterface.h"
#include "validation/message.h"
#include "validation/exception.h"
#include "validation.h"

#include <main/SAPI.h>
#include <ext/spl/spl_directory.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/exception.h"

#include "interned-strings.h"

/**
 * Phalcon\Validation\Validator\File
 *
 * Checks if a value has a correct FILE format
 *
 *<code>
 *use Phalcon\Validation\Validator\File as FileValidator;
 *
 *$validator->add('file', new FileValidator(array(
 *	 'mimes' => array('image/png', 'image/gif'),
 *	 'minsize' => 100,
 *	 'maxsize' => 10000,
 *   'message' => 'The file is not valid'
 *)));
 *</code>
 */
zend_class_entry *phalcon_validation_validator_file_ce;

PHP_METHOD(Phalcon_Validation_Validator_File, validate);
PHP_METHOD(Phalcon_Validation_Validator_File, valid);

static const zend_function_entry phalcon_validation_validator_file_method_entry[] = {
	PHP_ME(Phalcon_Validation_Validator_File, validate, arginfo_phalcon_validation_validatorinterface_validate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Validation_Validator_File, valid, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Validation\Validator\File initializer
 */
PHALCON_INIT_CLASS(Phalcon_Validation_Validator_File){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Validation\\Validator, File, validation_validator_file, phalcon_validation_validator_ce, phalcon_validation_validator_file_method_entry, 0);

	zend_class_implements(phalcon_validation_validator_file_ce, 1, phalcon_validation_validatorinterface_ce);

	return SUCCESS;
}

/**
 * Executes the validation
 *
 * @param Phalcon\Validation $validator
 * @param string $attribute
 * @return boolean
 */
PHP_METHOD(Phalcon_Validation_Validator_File, validate)
{
	zval *validaton, *attribute, *_allow_empty = NULL, value = {}, allow_empty = {}, mimes = {}, minsize = {}, maxsize = {}, minwidth = {}, maxwidth = {}, minheight = {}, maxheight = {};
	zval valid = {}, type = {}, code = {}, message_str = {}, message = {}, join_mimes = {}, label = {}, pairs = {}, prepared = {};
	zend_class_entry *ce = Z_OBJCE_P(getThis());

	phalcon_fetch_params(0, 2, 1, &validaton, &attribute, &_allow_empty);
	PHALCON_VERIFY_INTERFACE_EX(validaton, phalcon_validationinterface_ce, phalcon_validation_exception_ce);

	PHALCON_CALL_METHOD(&value, validaton, "getvalue", attribute);

	RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&allow_empty, ce, getThis(), ISV(allowEmpty)));
	if (Z_TYPE(allow_empty) == IS_NULL) {
		if (_allow_empty && zend_is_true(_allow_empty)) {
			ZVAL_COPY(&allow_empty, _allow_empty);
		}
	}
	if (zend_is_true(&allow_empty) && PHALCON_IS_EMPTY_STRING(&value)) {
		zval_ptr_dtor(&allow_empty);
		zval_ptr_dtor(&value);
		RETURN_TRUE;
	}
	zval_ptr_dtor(&allow_empty);

	RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&mimes, ce, getThis(), "mimes"));
	RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&minsize, ce, getThis(), "minsize"));
	RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&maxsize, ce, getThis(), "maxsize"));
	RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&minwidth, ce, getThis(), "minwidth"));
	RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&maxwidth, ce, getThis(), "maxwidth"));
	RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&minheight, ce, getThis(), "minheight"));
	RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&maxheight, ce, getThis(), "maxheight"));

	PHALCON_CALL_SELF(&valid, "valid", &value, &minsize, &maxsize, &mimes, &minwidth, &maxwidth, &minheight, &maxheight);

	if (PHALCON_IS_FALSE(&valid)) {
		phalcon_read_property(&type, getThis(), SL("_type"), PH_READONLY);

		RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&label, ce, getThis(), ISV(label)));
		if (!zend_is_true(&label)) {
			PHALCON_CALL_METHOD(&label, validaton, "getlabel", attribute);
		}

		RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&code, ce, getThis(), ISV(code)));
		if (Z_TYPE(code) == IS_NULL) {
			ZVAL_LONG(&code, 0);
		}

		array_init(&pairs);
		phalcon_array_update_str(&pairs, SL(":field"), &label, PH_COPY);
		zval_ptr_dtor(&label);

		if (phalcon_compare_strict_string(&type, SL("TooLarge"))) {
			phalcon_array_update_str(&pairs, SL(":max"), &maxsize, PH_COPY);

			RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
			if (!zend_is_true(&message_str)) {
				RETURN_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validaton), validaton, "FileMaxSize"));
			}

			PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

			phalcon_validation_message_construct_helper(&message, &prepared, attribute, "TooLarge", &code);
		} else if (phalcon_compare_strict_string(&type, SL("TooSmall"))) {
			phalcon_array_update_str(&pairs, SL(":min"), &minsize, PH_COPY);

			RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
			if (!zend_is_true(&message_str)) {
				RETURN_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validaton), validaton, "FileMinSize"));
			}

			PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

			phalcon_validation_message_construct_helper(&message, &prepared, attribute, "TooSmall", &code);
		} else if (phalcon_compare_strict_string(&type, SL("MimeValid"))) {
			phalcon_fast_join_str(&join_mimes, SL(", "), &mimes);
			phalcon_array_update_str(&pairs, SL(":mimes"), &join_mimes, PH_COPY);

			RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
			if (!zend_is_true(&message_str)) {
				RETURN_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validaton), validaton, "FileType"));
			}

			PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

			phalcon_validation_message_construct_helper(&message, &prepared, attribute, "FileType", &code);
		} else if (phalcon_compare_strict_string(&type, SL("TooLarge"))) {
			phalcon_array_update_str(&pairs, SL(":max"), &maxsize, PH_COPY);

			RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
			if (!zend_is_true(&message_str)) {
				RETURN_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validaton), validaton, "FileMaxSize"));
			}

			PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

			phalcon_validation_message_construct_helper(&message, &prepared, attribute, "TooLarge", &code);
		} else if (phalcon_compare_strict_string(&type, SL("TooNarrow"))) {
			phalcon_array_update_str(&pairs, SL(":min"), &minwidth, PH_COPY);

			RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
			if (!zend_is_true(&message_str)) {
				RETURN_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validaton), validaton, "ImageMinWidth"));
			}

			PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

			phalcon_validation_message_construct_helper(&message, &prepared, attribute, "TooNarrow", &code);
		} else if (phalcon_compare_strict_string(&type, SL("TooWide"))) {
			phalcon_array_update_str(&pairs, SL(":max"), &maxwidth, PH_COPY);

			RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
			if (!zend_is_true(&message_str)) {
				RETURN_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validaton), validaton, "ImageMaxWidth"));
			}

			PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

			phalcon_validation_message_construct_helper(&message, &prepared, attribute, "TooWide", &code);
		}  else if (phalcon_compare_strict_string(&type, SL("TooShort"))) {
			phalcon_array_update_str(&pairs, SL(":min"), &minheight, PH_COPY);

			RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
			if (!zend_is_true(&message_str)) {
				RETURN_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validaton), validaton, "ImageMinHeight"));
			}

			PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

			phalcon_validation_message_construct_helper(&message, &prepared, attribute, "TooShort", &code);
		} else if (phalcon_compare_strict_string(&type, SL("TooLong"))) {
			phalcon_array_update_str(&pairs, SL(":max"), &maxheight, PH_COPY);

			RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
			if (!zend_is_true(&message_str)) {
				RETURN_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validaton), validaton, "ImageMaxHeight"));
			}

			PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

			phalcon_validation_message_construct_helper(&message, &prepared, attribute, "TooLong", &code);
		} else {
			RETURN_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
			if (!zend_is_true(&message_str)) {
				RETURN_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validaton), validaton, "FileValid"));
			}

			PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

			phalcon_validation_message_construct_helper(&message, &prepared, attribute, "File", &code);
		}
		zval_ptr_dtor(&message_str);
		zval_ptr_dtor(&pairs);
		zval_ptr_dtor(&prepared);

		PHALCON_CALL_METHOD(NULL, validaton, "appendmessage", &message);
		zval_ptr_dtor(&message);
		RETVAL_FALSE;
	} else {
		RETVAL_TRUE;
	}

	zval_ptr_dtor(&minsize);
	zval_ptr_dtor(&mimes);
	zval_ptr_dtor(&minwidth);
	zval_ptr_dtor(&maxwidth);
	zval_ptr_dtor(&minheight);
	zval_ptr_dtor(&maxheight);
}

/**
 * Executes the validation
 *
 * @param string $file
 * @param int $minsize
 * @param int $maxsize
 * @param array $mimes
 * @param int $minwidth
 * @param int $maxwidth
 * @param int $minheight
 * @param int $maxheight
 * @return boolean
 */
PHP_METHOD(Phalcon_Validation_Validator_File, valid)
{
	zval *value, *minsize = NULL, *maxsize = NULL, *mimes = NULL, *minwidth = NULL, *maxwidth = NULL, *minheight = NULL, *maxheight = NULL;
	zval file = {}, size = {}, *constant, pathname = {}, width = {}, height = {}, valid = {};
	zend_class_entry *imagick_ce;

	phalcon_fetch_params(0, 1, 7, &value, &minsize, &maxsize, &mimes, &minwidth, &maxwidth, &minheight, &maxheight);

	if (Z_TYPE_P(value) == IS_STRING) {
		object_init_ex(&file, spl_ce_SplFileInfo);
		if (phalcon_has_constructor(&file)) {
			PHALCON_CALL_METHOD(NULL, &file, "__construct", value);
		}
	} else if (Z_TYPE_P(value) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(value), spl_ce_SplFileInfo, 0)) {
		ZVAL_COPY(&file, value);
	}

	if (Z_TYPE(file) <= IS_NULL) {
		phalcon_update_property_str(getThis(), SL("_type"), SL("TypeUnknow"));
		RETURN_FALSE;
	}

	if (!minsize) {
		minsize = &PHALCON_GLOBAL(z_null);
	}

	if (!maxsize) {
		maxsize = &PHALCON_GLOBAL(z_null);
	}

	if (!mimes) {
		mimes = &PHALCON_GLOBAL(z_null);
	}

	if (!minwidth) {
		minwidth = &PHALCON_GLOBAL(z_null);
	}

	if (!maxwidth) {
		maxwidth = &PHALCON_GLOBAL(z_null);
	}

	if (!minheight) {
		minheight = &PHALCON_GLOBAL(z_null);
	}

	if (!maxheight) {
		maxheight = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_METHOD(&valid, &file, "isfile");

	if (!zend_is_true(&valid)) {
		phalcon_update_property_str(getThis(), SL("_type"), SL("FileValid"));
		zval_ptr_dtor(&file);
		RETURN_FALSE;
	}

	PHALCON_CALL_METHOD(&size, &file, "getsize");

	if (!PHALCON_IS_EMPTY(minsize)) {
		is_smaller_or_equal_function(&valid, minsize, &size);
		if (!zend_is_true(&valid)) {
			phalcon_update_property_str(getThis(), SL("_type"), SL("TooSmall"));
			zval_ptr_dtor(&size);
			zval_ptr_dtor(&file);
			RETURN_FALSE;
		}
	}

	if (!PHALCON_IS_EMPTY(maxsize)) {
		is_smaller_or_equal_function(&valid, &size, maxsize);
		if (!zend_is_true(&valid)) {
			phalcon_update_property_str(getThis(), SL("_type"), SL("TooLarge"));
			zval_ptr_dtor(&size);
			zval_ptr_dtor(&file);
			RETURN_FALSE;
		}
	}
	zval_ptr_dtor(&size);

	PHALCON_CALL_METHOD(&pathname, &file, "getpathname");

	if (Z_TYPE_P(mimes) == IS_ARRAY) {
		zval mime = {}, finfo = {};
		if ((constant = zend_get_constant_str(SL("FILEINFO_MIME_TYPE"))) == NULL) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_validation_exception_ce, "Undefined constant `FILEINFO_MIME_TYPE`");
			return;
		}

		PHALCON_CALL_FUNCTION(&finfo, "finfo_open", constant);

		if (Z_TYPE(finfo) != IS_RESOURCE) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_validation_exception_ce, "Opening fileinfo database failed");
			zval_ptr_dtor(&pathname);
			zval_ptr_dtor(&file);
			return;
		}

		PHALCON_CALL_FUNCTION(&mime, "finfo_file", &finfo, &pathname);
		PHALCON_CALL_FUNCTION(NULL, "finfo_close", &finfo);

		if (!phalcon_fast_in_array(&mime, mimes)) {
			phalcon_update_property_str(getThis(), SL("_type"), SL("MimeValid"));
			zval_ptr_dtor(&mime);
			zval_ptr_dtor(&finfo);
			zval_ptr_dtor(&pathname);
			zval_ptr_dtor(&file);
			RETURN_FALSE;
		}
		zval_ptr_dtor(&mime);
		zval_ptr_dtor(&finfo);
	}

	if (phalcon_class_str_exists(SL("imagick"), 0) != NULL) {
		zval image = {};
		imagick_ce = phalcon_fetch_str_class(SL("Imagick"), ZEND_FETCH_CLASS_AUTO);

		object_init_ex(&image, imagick_ce);
		PHALCON_CALL_METHOD(NULL, &image, "__construct", &pathname);

		PHALCON_CALL_METHOD(&width, &image, "getImageWidth");
		PHALCON_CALL_METHOD(&height, &image, "getImageHeight");
		zval_ptr_dtor(&image);
	} else if (phalcon_function_exists_ex(SL("getimagesize")) != FAILURE) {
		zval imageinfo = {};
		PHALCON_CALL_FUNCTION(&imageinfo, "getimagesize", &pathname);
		if (!phalcon_array_isset_fetch_long(&width, &imageinfo, 0, PH_COPY)) {
			ZVAL_LONG(&width, -1);
		}

		if (!phalcon_array_isset_fetch_long(&height, &imageinfo, 1, PH_COPY)) {
			ZVAL_LONG(&height, -1);
		}		
		zval_ptr_dtor(&imageinfo);
	} else {
		ZVAL_LONG(&width, -1);
		ZVAL_LONG(&height, -1);
	}
	zval_ptr_dtor(&pathname);

	if (!PHALCON_IS_EMPTY(minwidth)) {
		is_smaller_or_equal_function(&valid, minwidth, &width);
		if (!zend_is_true(&valid)) {
			phalcon_update_property_str(getThis(), SL("_type"), SL("TooNarrow"));
			RETURN_FALSE;
		}
	}

	if (!PHALCON_IS_EMPTY(maxwidth)) {
		is_smaller_or_equal_function(&valid, &width, maxwidth);
		if (!zend_is_true(&valid)) {
			phalcon_update_property_str(getThis(), SL("_type"), SL("TooWide"));
			RETURN_FALSE;
		}
	}

	if (!PHALCON_IS_EMPTY(minheight)) {
		is_smaller_or_equal_function(&valid, minheight, &height);
		if (!zend_is_true(&valid)) {
			phalcon_update_property_str(getThis(), SL("_type"), SL("TooShort"));
			RETURN_FALSE;
		}
	}

	if (!PHALCON_IS_EMPTY(maxheight)) {
		is_smaller_or_equal_function(&valid, &height, maxheight);
		if (!zend_is_true(&valid)) {
			phalcon_update_property_str(getThis(), SL("_type"), SL("TooLong"));
			RETURN_FALSE;
		}
	}

	RETURN_TRUE;
}
