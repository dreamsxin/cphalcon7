
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

#include "validation/validator/between.h"
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
PHP_METHOD(Phalcon_Validation_Validator_File, validate){

	zval *validator, *attribute, *value = NULL, allow_empty;
	zval mimes, minsize, maxsize, minwidth, maxwidth, minheight, maxheight, *valid = NULL, *type, code, message_str, *message;
	zval *join_mimes, *label, *pairs, *prepared = NULL;
	zend_class_entry *ce = Z_OBJCE_P(getThis());

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 2, 0, &validator, &attribute);
	
	PHALCON_VERIFY_CLASS_EX(validator, phalcon_validation_ce, phalcon_validation_exception_ce, 1);

	PHALCON_CALL_METHOD(&value, validator, "getvalue", attribute);

	RETURN_MM_ON_FAILURE(phalcon_validation_validator_getoption_helper(&allow_empty, ce, getThis(), ISV(allowEmpty)));
	if (zend_is_true(&allow_empty) && phalcon_validation_validator_isempty_helper(value)) {
		RETURN_MM_TRUE;
	}

	RETURN_MM_ON_FAILURE(phalcon_validation_validator_getoption_helper(&mimes, ce, getThis(), "mimes"));
	RETURN_MM_ON_FAILURE(phalcon_validation_validator_getoption_helper(&minsize, ce, getThis(), "minsize"));
	RETURN_MM_ON_FAILURE(phalcon_validation_validator_getoption_helper(&maxsize, ce, getThis(), "maxsize"));
	RETURN_MM_ON_FAILURE(phalcon_validation_validator_getoption_helper(&minwidth, ce, getThis(), "minwidth"));
	RETURN_MM_ON_FAILURE(phalcon_validation_validator_getoption_helper(&maxwidth, ce, getThis(), "maxwidth"));
	RETURN_MM_ON_FAILURE(phalcon_validation_validator_getoption_helper(&minheight, ce, getThis(), "minheight"));
	RETURN_MM_ON_FAILURE(phalcon_validation_validator_getoption_helper(&maxheight, ce, getThis(), "maxheight"));

	PHALCON_CALL_SELF(&valid, "valid", value, minsize, maxsize, mimes, minwidth, maxwidth, minheight, maxheight);

	if (PHALCON_IS_FALSE(valid)) {
		type = phalcon_read_property(getThis(), SL("_type"), PH_NOISY);

		RETURN_MM_ON_FAILURE(phalcon_validation_validator_getoption_helper(&label, ce, getThis(), ISV(label)));
		if (!zend_is_true(&label)) {
			PHALCON_CALL_METHOD(&label, validator, "getlabel", attribute);
			if (!zend_is_true(&label)) {
				ZVAL_COPY_VALUE(&label, attribute);
			}
		}

		RETURN_MM_ON_FAILURE(phalcon_validation_validator_getoption_helper(&code, ce, getThis(), ISV(code)));
		if (Z_TYPE_P(&code) == IS_NULL) {
			ZVAL_LONG(&code, 0);
		}

		array_init_size(&pairs);
		Z_TRY_ADDREF_P(&label);
		add_assoc_zval_ex(&pairs, SL(":field"), &label);

		if (phalcon_compare_strict_string(type, SL("TooLarge"))) {
			Z_TRY_ADDREF_P(&maxsize);
			add_assoc_zval_ex(&pairs, SL(":max"), &maxsize);

			RETURN_MM_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
			if (!zend_is_true(message_str)) {
				RETURN_MM_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validator), validator, "FileMaxSize"));
			}

			PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

			message = phalcon_validation_message_construct_helper(prepared, attribute, "TooLarge", &code);
		} else if (phalcon_compare_strict_string(type, SL("TooSmall"))) {
			Z_TRY_ADDREF_P(&minsize);
			add_assoc_zval_ex(&pairs, SL(":min"), &minsize);

			RETURN_MM_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
			if (!zend_is_true(&message_str)) {
				RETURN_MM_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validator), validator, "FileMinSize"));
			}

			PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

			message = phalcon_validation_message_construct_helper(prepared, attribute, "TooSmall", &code);
		} else if (phalcon_compare_strict_string(type, SL("MimeValid"))) {
			phalcon_fast_join_str(&join_mimes, SL(", "), mimes);

			Z_TRY_ADDREF_P(&join_mimes);
			add_assoc_zval_ex(&pairs, SL(":mimes"), &join_mimes);

			RETURN_MM_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
			if (!zend_is_true(&message_str)) {
				RETURN_MM_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validator), validator, "FileType"));
			}

			PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

			message = phalcon_validation_message_construct_helper(prepared, attribute, "FileType", &code);
		} else if (phalcon_compare_strict_string(type, SL("TooLarge"))) {
			Z_TRY_ADDREF_P(&maxsize);
			add_assoc_zval_ex(&pairs, SL(":max"), &maxsize);

			RETURN_MM_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
			if (!zend_is_true(&message_str)) {
				RETURN_MM_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validator), validator, "FileMaxSize"));
			}

			PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

			message = phalcon_validation_message_construct_helper(prepared, attribute, "TooLarge", &code);
		} else if (phalcon_compare_strict_string(type, SL("TooNarrow"))) {
			Z_TRY_ADDREF_P(&minwidth);
			add_assoc_zval_ex(&pairs, SL(":min"), &minwidth);

			RETURN_MM_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
			if (!zend_is_true(message_str)) {
				RETURN_MM_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validator), validator, "ImageMinWidth"));
			}

			PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

			message = phalcon_validation_message_construct_helper(prepared, attribute, "TooNarrow", &code);
		} else if (phalcon_compare_strict_string(type, SL("TooWide"))) {
			Z_TRY_ADDREF_P(&maxwidth);
			add_assoc_zval_ex(&pairs, SL(":max"), &maxwidth);

			RETURN_MM_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
			if (!zend_is_true(&message_str)) {
				RETURN_MM_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validator), validator, "ImageMaxWidth"));
			}

			PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

			message = phalcon_validation_message_construct_helper(prepared, attribute, "TooWide", &code);
		}  else if (phalcon_compare_strict_string(type, SL("TooShort"))) {
			Z_TRY_ADDREF_P(&minheight);
			add_assoc_zval_ex(&pairs, SL(":min"), &minheight);

			RETURN_MM_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
			if (!zend_is_true(message_str)) {
				RETURN_MM_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validator), validator, "ImageMinHeight"));
			}

			PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

			message = phalcon_validation_message_construct_helper(prepared, attribute, "TooShort", &code);
		} else if (phalcon_compare_strict_string(type, SL("TooLong"))) {
			Z_TRY_ADDREF_P(&maxheight);
			add_assoc_zval_ex(&pairs, SL(":max"), &maxheight);

			RETURN_MM_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
			if (!zend_is_true(message_str)) {
				RETURN_MM_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validator), validator, "ImageMaxHeight"));
			}

			PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

			message = phalcon_validation_message_construct_helper(prepared, attribute, "TooLong", &code);
		} else {
			RETURN_MM_ON_FAILURE(phalcon_validation_validator_getoption_helper(&message_str, ce, getThis(), ISV(message)));
			if (!zend_is_true(message_str)) {
				RETURN_MM_ON_FAILURE(phalcon_validation_getdefaultmessage_helper(&message_str, Z_OBJCE_P(validator), validator, "FileValid"));
			}

			PHALCON_CALL_FUNCTION(&prepared, "strtr", &message_str, &pairs);

			message = phalcon_validation_message_construct_helper(prepared, attribute, "File", &code);
		}

		PHALCON_CALL_METHOD(NULL, validator, "appendmessage", &message);
		RETURN_MM_FALSE;
	}
	
	RETURN_MM_TRUE;
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
PHP_METHOD(Phalcon_Validation_Validator_File, valid){

	zval *value, *minsize = NULL, *maxsize = NULL, *mimes = NULL, *minwidth = NULL, *maxwidth = NULL, *minheight = NULL, *maxheight = NULL;
	zval *file = NULL, *size = NULL, *constant, *finfo = NULL, *pathname = NULL, *mime = NULL, *image, *imageinfo = NULL, width, height, *valid = NULL;
	zend_class_entry *imagick_ce;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 7, &value, &minsize, &maxsize, &mimes, &minwidth, &maxwidth, &minheight, &maxheight);

	if (Z_TYPE_P(value) == IS_STRING) {
		PHALCON_INIT_NVAR(file);
		object_init_ex(file, spl_ce_SplFileInfo);
		if (phalcon_has_constructor(file)) {
			PHALCON_CALL_METHOD(NULL, file, "__construct", value);
		}
	} else if (Z_TYPE_P(value) == IS_OBJECT && instanceof_function_ex(Z_OBJCE_P(value), spl_ce_SplFileInfo, 0)) {
		PHALCON_CPY_WRT(file, value);
	}

	if (!file) {
		phalcon_update_property_str(getThis(), SL("_type"), SL("TypeUnknow"));
		RETURN_MM_FALSE;
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

	PHALCON_CALL_METHOD(&valid, file, "isfile");

	if (!zend_is_true(valid)) {
		phalcon_update_property_str(getThis(), SL("_type"), SL("FileValid"));
		RETURN_MM_FALSE;
	}

	PHALCON_CALL_METHOD(&size, file, "getsize");

	if (!PHALCON_IS_EMPTY(minsize)) {
		PHALCON_INIT_NVAR(valid);
		is_smaller_or_equal_function(valid, minsize, size);
		if (!zend_is_true(valid)) {
			phalcon_update_property_str(getThis(), SL("_type"), SL("TooSmall"));
			RETURN_MM_FALSE;
		}
	}

	if (!PHALCON_IS_EMPTY(maxsize)) {
		PHALCON_INIT_NVAR(valid);
		is_smaller_or_equal_function(valid, size, maxsize);
		if (!zend_is_true(valid)) {
			phalcon_update_property_str(getThis(), SL("_type"), SL("TooLarge"));
			RETURN_MM_FALSE;
		}
	}

	PHALCON_CALL_METHOD(&pathname, file, "getpathname");

	if (Z_TYPE_P(mimes) == IS_ARRAY) {
		if ((constant = zend_get_constant_str(SL("FILEINFO_MIME_TYPE"))) == NULL) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_validation_exception_ce, "Undefined constant `FILEINFO_MIME_TYPE`");
			return;
		}

		PHALCON_CALL_FUNCTION(&finfo, "finfo_open", constant);

		if (Z_TYPE_P(finfo) != IS_RESOURCE) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_validation_exception_ce, "Opening fileinfo database failed");
			return;
		}		

		PHALCON_CALL_FUNCTION(&mime, "finfo_file", finfo, pathname);
		PHALCON_CALL_FUNCTION(NULL, "finfo_close", finfo);
		
		if (!phalcon_fast_in_array(mime, mimes)) {
			phalcon_update_property_str(getThis(), SL("_type"), SL("MimeValid"));
			RETURN_MM_FALSE;
		}
	}

	if (phalcon_class_str_exists(SL("imagick"), 0) != NULL) {
		imagick_ce = zend_fetch_class(SSL("Imagick"), ZEND_FETCH_CLASS_AUTO);

		PHALCON_INIT_VAR(image);
		object_init_ex(image, imagick_ce);
		PHALCON_CALL_METHOD(NULL, image, "__construct", pathname);

		PHALCON_CALL_METHOD(&width, image, "getImageWidth");
		PHALCON_CALL_METHOD(&height, image, "getImageHeight");
	} else if (phalcon_function_exists_ex(SL("getimagesize")) != FAILURE) {
		PHALCON_CALL_FUNCTION(&imageinfo, "getimagesize", pathname);
		if (!phalcon_array_isset_fetch_long(&width, imageinfo, 0)) {
			ZVAL_LONG(&width, -1);
		}

		if (!phalcon_array_isset_fetch_long(&height, imageinfo, 1)) {
			ZVAL_LONG(&height, -1);
		}
	} else {
		ZVAL_LONG(&width, -1);
		ZVAL_LONG(&height, -1);
	}

	if (!PHALCON_IS_EMPTY(minwidth)) {
		PHALCON_INIT_NVAR(valid);
		is_smaller_or_equal_function(valid, minwidth, &width);
		if (!zend_is_true(valid)) {
			phalcon_update_property_str(getThis(), SL("_type"), SL("TooNarrow"));
			RETURN_MM_FALSE;
		}
	}

	if (!PHALCON_IS_EMPTY(maxwidth)) {
		PHALCON_INIT_NVAR(valid);
		is_smaller_or_equal_function(valid, &width, maxwidth);
		if (!zend_is_true(valid)) {
			phalcon_update_property_str(getThis(), SL("_type"), SL("TooWide"));
			RETURN_MM_FALSE;
		}
	}

	if (!PHALCON_IS_EMPTY(minheight)) {
		PHALCON_INIT_NVAR(valid);
		is_smaller_or_equal_function(valid, minheight, &height);
		if (!zend_is_true(valid)) {
			phalcon_update_property_str(getThis(), SL("_type"), SL("TooShort"));
			RETURN_MM_FALSE;
		}
	}

	if (!PHALCON_IS_EMPTY(maxheight)) {
		PHALCON_INIT_NVAR(valid);
		is_smaller_or_equal_function(valid, &height, maxheight);
		if (!zend_is_true(valid)) {
			phalcon_update_property_str(getThis(), SL("_type"), SL("TooLong"));
			RETURN_MM_FALSE;
		}
	}

	RETURN_MM_TRUE;
}
