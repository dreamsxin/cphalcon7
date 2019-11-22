
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

#include "image.h"
#include "image/adapter/gd.h"
#include "image/adapter/imagick.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/object.h"
#include "kernel/array.h"

/**
 * Phalcon\Image
 *
 * Image manipulation support. Allows images to be resized, cropped, etc.
 *
 *<code>
 *	$image = Phalcon\Image::factory("upload/test.jpg");
 *	$image->resize(200, 200);
 *	$image->save();
 *</code>
 */
zend_class_entry *phalcon_image_ce;

PHP_METHOD(Phalcon_Image, factory);
PHP_METHOD(Phalcon_Image, splitHexColor);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_factory, 0, 0, 1)
	ZEND_ARG_INFO(0, file)
	ZEND_ARG_INFO(0, width)
	ZEND_ARG_INFO(0, height)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_splithexcolor, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, color, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_image_method_entry[] = {
	PHP_ME(Phalcon_Image, factory, arginfo_phalcon_image_factory, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image, splitHexColor, arginfo_phalcon_image_splithexcolor, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Image initializer
 */
PHALCON_INIT_CLASS(Phalcon_Image){

	PHALCON_REGISTER_CLASS(Phalcon, Image, image, phalcon_image_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	/* Resizing constraints */
	zend_declare_class_constant_long(phalcon_image_ce, SL("NONE"),    PHALCON_IMAGE_NONE);
	zend_declare_class_constant_long(phalcon_image_ce, SL("WIDTH"),   PHALCON_IMAGE_WIDTH);
	zend_declare_class_constant_long(phalcon_image_ce, SL("HEIGHT"),  PHALCON_IMAGE_HEIGHT);
	zend_declare_class_constant_long(phalcon_image_ce, SL("AUTO"),    PHALCON_IMAGE_AUTO);
	zend_declare_class_constant_long(phalcon_image_ce, SL("INVERSE"), PHALCON_IMAGE_INVERSE);
	zend_declare_class_constant_long(phalcon_image_ce, SL("PRECISE"), PHALCON_IMAGE_PRECISE);
	zend_declare_class_constant_long(phalcon_image_ce, SL("TENSILE"), PHALCON_IMAGE_TENSILE);
	zend_declare_class_constant_long(phalcon_image_ce, SL("NARROW"),  PHALCON_IMAGE_NARROW);

	/* Flipping directions */
	zend_declare_class_constant_long(phalcon_image_ce, SL("HORIZONTAL"), PHALCON_IMAGE_HORIZONTAL);
	zend_declare_class_constant_long(phalcon_image_ce, SL("VERTICAL"),   PHALCON_IMAGE_VERTICAL);

	/* Driver: GD, ImageMagick, etc */
	zend_declare_class_constant_long(phalcon_image_ce, SL("GD"),      PHALCON_IMAGE_GD);
	zend_declare_class_constant_long(phalcon_image_ce, SL("IMAGICK"), PHALCON_IMAGE_IMAGICK);

	return SUCCESS;
}

/**
 * Loads an image and prepares it for manipulation
 *
 * @return Phalcon\Image\AdapterInterface
 **/
PHP_METHOD(Phalcon_Image, factory)
{
	zval *file, *width = NULL, *height = NULL;

	phalcon_fetch_params(0, 1, 2, &file, &width, &height);

	if (!width) {
		width = &PHALCON_GLOBAL(z_null);
	}

	if (!height) {
		height = &PHALCON_GLOBAL(z_null);
	}

	if (phalcon_class_str_exists(SL("imagick"), 0) != NULL) {
		object_init_ex(return_value, phalcon_image_adapter_imagick_ce);
		PHALCON_CALL_METHOD(NULL, return_value, "__construct", file, width, height);
	} else {
		object_init_ex(return_value, phalcon_image_adapter_gd_ce);
		PHALCON_CALL_METHOD(NULL, return_value, "__construct", file, width, height);
	}
}


void str_repeat(zval *return_value, zend_string *input_str, zend_long mult)
{
	zend_string	*result;		/* Resulting string */
	size_t result_len;		/* Length of the resulting string */

	if (mult < 0) {
		return;
	}

	/* Don't waste our time if it's empty */
	/* ... or if the multiplier is zero */
	if (ZSTR_LEN(input_str) == 0 || mult == 0)
		RETURN_EMPTY_STRING();

	/* Initialize the result string */
	result = zend_string_safe_alloc(ZSTR_LEN(input_str), mult, 0, 0);
	result_len = ZSTR_LEN(input_str) * mult;

	/* Heavy optimization for situations where input string is 1 byte long */
	if (ZSTR_LEN(input_str) == 1) {
		memset(ZSTR_VAL(result), *ZSTR_VAL(input_str), mult);
	} else {
		char *s, *e, *ee;
		ptrdiff_t l=0;
		memcpy(ZSTR_VAL(result), ZSTR_VAL(input_str), ZSTR_LEN(input_str));
		s = ZSTR_VAL(result);
		e = ZSTR_VAL(result) + ZSTR_LEN(input_str);
		ee = ZSTR_VAL(result) + result_len;

		while (e<ee) {
			l = (e-s) < (ee-e) ? (e-s) : (ee-e);
			memmove(e, s, l);
			e += l;
		}
	}

	ZSTR_VAL(result)[result_len] = '\0';

	RETURN_NEW_STR(result);
}

PHP_METHOD(Phalcon_Image, splitHexColor)
{
	zval *str, newcolor = {};
	unsigned int color[4];
	const char *value;
	int len, count;

	phalcon_fetch_params(0, 1, 2, &str);

	len = Z_STRLEN_P(str);

	if(len < 6) {
		str_repeat(&newcolor, Z_STR_P(str), 6);
	} else {
		ZVAL_COPY(&newcolor, str);
	}
	value = Z_STRVAL(newcolor);
	if(len == 8) {
		count = sscanf(value, "%02x%02x%02x%02x%n", &color[0], &color[1], &color[2], &color[3], &len);
		if(count < 4 || len != 8) {
			RETVAL_FALSE;
		} else {
			array_init(return_value);
			phalcon_array_append_long(return_value, color[0], 0);
			phalcon_array_append_long(return_value, color[1], 0);
			phalcon_array_append_long(return_value, color[2], 0);
			phalcon_array_append_long(return_value, color[3], 0);
		}
	} else {
		count = sscanf(value, "%02x%02x%02x%n", &color[0], &color[1], &color[2], &len);
		if(count < 3 || len != 6) {
			RETVAL_FALSE;
		} else {
			array_init(return_value);
			phalcon_array_append_long(return_value, color[0], 0);
			phalcon_array_append_long(return_value, color[1], 0);
			phalcon_array_append_long(return_value, color[2], 0);
		}
	}
	zval_ptr_dtor(&newcolor);
}
