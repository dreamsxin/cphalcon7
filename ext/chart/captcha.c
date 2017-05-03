/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2013 Phalcon Team (http://www.phalconphp.com)       |
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_phalcon.h"
#include "phalcon.h"

#include "php_open_temporary_file.h"

#include "kernel/main.h"
#include "kernel/memory.h"

#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/hash.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/output.h"

#include "chart/captcha.h"
#include "chart/exception.h"
#include "random.h"

/**
 * Phalcon\Chart\Captcha
 *
 *<code>
 * header('Content-Type: image/png');
 * $captcha = new \Phalcon\Chart\Captcha(Phalcon\Text::random(Phalcon\Text::RANDOM_ALNUM, 4), NULL, 30, 150, 50);
 * echo $captcha = $captcha->render();
 *</code>
 */
zend_class_entry *phalcon_chart_captcha_ce;

PHP_METHOD(Phalcon_Chart_Captcha, __construct);
PHP_METHOD(Phalcon_Chart_Captcha, setFont);
PHP_METHOD(Phalcon_Chart_Captcha, setFontSize);
PHP_METHOD(Phalcon_Chart_Captcha, render);
PHP_METHOD(Phalcon_Chart_Captcha, save);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_chart_captcha___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, word, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, font, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, fontSize, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, pad_size, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, pad_type, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_chart_captcha_setfont, 0, 0, 1)
	ZEND_ARG_INFO(0, font)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_chart_captcha_setfontsize, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_chart_captcha_render, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, word, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, offset_x, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, offset_y, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, foreground, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, background, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_chart_captcha_save, 0, 0, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_chart_captcha_method_entry[] = {
	PHP_ME(Phalcon_Chart_Captcha, __construct, arginfo_phalcon_chart_captcha___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Chart_Captcha, setFont, arginfo_phalcon_chart_captcha_setfont, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Chart_Captcha, setFontSize, arginfo_phalcon_chart_captcha_setfontsize, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Chart_Captcha, render, arginfo_phalcon_chart_captcha_render, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Chart_Captcha, save, arginfo_phalcon_chart_captcha_save, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Chart\Captcha initializer
 */
PHALCON_INIT_CLASS(Phalcon_Chart_Captcha){

	PHALCON_REGISTER_CLASS(Phalcon\\Chart, Captcha, chart_captcha, phalcon_chart_captcha_method_entry, 0);

	zend_declare_property_null(phalcon_chart_captcha_ce, SL("_imagick"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_chart_captcha_ce, SL("_word"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_chart_captcha_ce, SL("_font"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_chart_captcha_ce, SL("_fontSize"), 40, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_chart_captcha_ce, SL("_width"), 150, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_chart_captcha_ce, SL("_height"), 50, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_chart_captcha_ce, SL("_padType"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_chart_captcha_ce, SL("_padSize"), 1, ZEND_ACC_PROTECTED);

	zend_declare_class_constant_long(phalcon_chart_captcha_ce, SL("PAD_BOTH"), PHALCON_CHART_CAPTCHA_PAD_BOTH);
	zend_declare_class_constant_long(phalcon_chart_captcha_ce, SL("PAD_LEFT"), PHALCON_CHART_CAPTCHA_PAD_LEFT);
	zend_declare_class_constant_long(phalcon_chart_captcha_ce, SL("PAD_RIGHT"), PHALCON_CHART_CAPTCHA_PAD_RIGHT);
	return SUCCESS;
}

/**
 * Phalcon\Chart\Captcha constructor
 *
 *     $captcha = new \Phalcon\Chart\Captcha;
 *     $captcha->generate('Phalcon is a web framework');
 *     $captcha->save('qr.png');
 */
PHP_METHOD(Phalcon_Chart_Captcha, __construct){

	zval *word = NULL, *font = NULL, *font_size = NULL, *width = NULL, *height = NULL, *pad_size = NULL, *pad_type = NULL;

	phalcon_fetch_params(0, 1, 6, &word, &font, &font_size, &width, &height, &pad_size, &pad_type);

	if (phalcon_class_str_exists(SL("imagick"), 0) == NULL) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Imagick is not installed, or the extension is not loaded");
		return;
	}
	if (word && Z_TYPE_P(word) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_word"), word);
	}

	if (font && Z_TYPE_P(font) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_font"), font);
	}

	if (font_size && Z_TYPE_P(font_size) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_fontSize"), font_size);
	}

	if (width && Z_TYPE_P(width) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_width"), width);
	}

	if (height && Z_TYPE_P(height) != IS_NULL) {
		phalcon_update_property(getThis(), SL("_height"), height);
	}

	if (pad_size && Z_TYPE_P(pad_size) != IS_NULL && Z_LVAL_P(pad_size) > 0) {
		phalcon_update_property(getThis(), SL("_padSize"), pad_size);
	}

	if (pad_type && Z_TYPE_P(pad_type) != IS_NULL && Z_LVAL_P(pad_type) > 0 && Z_LVAL_P(pad_type) <= 2) {
		phalcon_update_property(getThis(), SL("_padType"), pad_type);
	}
}

/**
 * Sets a font
 *
 * @param string $font
 * @return boolean
 */
PHP_METHOD(Phalcon_Chart_Captcha, setFont){

	zval *font;

	phalcon_fetch_params(0, 1, 0, &font);

	phalcon_update_property(getThis(), SL("_font"), font);
	RETURN_THIS();
}

/**
 * Sets a font size
 *
 * @param int $fontSize
 * @return boolean
 */
PHP_METHOD(Phalcon_Chart_Captcha, setFontSize){

	zval *font_size;

	phalcon_fetch_params(0, 1, 0, &font_size);

	phalcon_update_property(getThis(), SL("_fontSize"), font_size);
	RETURN_THIS();
}

/**
 * Generate Captcha data
 *
 *<code>
 *     $captcha = new \Phalcon\Chart\Captcha;
 *     $captcha->reander('Phalcon is a web framework');
 *</code>
 *
 * @param string $word
 * @param string $margin
 * @return String
 */
PHP_METHOD(Phalcon_Chart_Captcha, render){

	zval *filename = NULL, *_word = NULL, *offset_x = NULL, *offset_y = NULL, *_foreground = NULL, *_background = NULL, *_width = NULL, *_height = NULL, random = {}, color_type = {};
	zval word = {}, width = {}, height = {}, foreground = {}, background = {}, imagick = {}, bgpixel = {}, draw = {}, font = {}, font_size = {}, fgpixel = {}, gradient = {};
	zval top_color = {}, bottom_color = {}, pseudostring = {}, composite = {}, gravity = {}, pad_type = {}, pad_size = {}, color = {}, pixel = {}, min = {};
	zval max = {}, roll1 = {}, roll2 = {}, corner1 = {}, corner2 = {}, format = {};
	zend_class_entry  *imagick_ce, *draw_ce, *imagickpixel_ce;

	phalcon_fetch_params(0, 0, 7, &filename, &_word, &offset_x, &offset_y, &_foreground, &_background, &_width, &_height);

	object_init_ex(&random, phalcon_random_ce);
	ZVAL_LONG(&color_type, PHALCON_RANDOM_COLOR_RGB);

	if (!_word || Z_TYPE_P(_word) == IS_NULL) {
		phalcon_read_property(&word, getThis(), SL("_word"), PH_NOISY|PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&word, _word);
	}

	if (!offset_x || Z_TYPE_P(offset_x) == IS_NULL) {
		offset_x  = &PHALCON_GLOBAL(z_zero);
	}

	if (!offset_y || Z_TYPE_P(offset_y) == IS_NULL) {
		offset_y  = &PHALCON_GLOBAL(z_zero);
	}

	if (!_foreground || Z_TYPE_P(_foreground) == IS_NULL) {
		PHALCON_CALL_METHOD(&foreground, &random, "color", &color_type);
	} else {
		ZVAL_COPY(&foreground, _foreground);
	}

	if (!_background || Z_TYPE_P(_background) == IS_NULL) {
		PHALCON_CALL_METHOD(&background, &random, "color", &color_type);
	} else {
		ZVAL_COPY(&background, _background);
	}

	if (!_width || Z_TYPE_P(_width) == IS_NULL) {
		phalcon_read_property(&width, getThis(), SL("_width"), PH_NOISY|PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&width, _width);
	}

	if (!_height || Z_TYPE_P(_height) == IS_NULL) {
		phalcon_read_property(&height, getThis(), SL("_height"), PH_NOISY|PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&height, _height);
	}

	imagick_ce = phalcon_fetch_str_class(SL("Imagick"), ZEND_FETCH_CLASS_AUTO);
	draw_ce = phalcon_fetch_str_class(SL("ImagickDraw"), ZEND_FETCH_CLASS_AUTO);
	imagickpixel_ce = phalcon_fetch_str_class(SL("ImagickPixel"), ZEND_FETCH_CLASS_AUTO);

	object_init_ex(&imagick, imagick_ce);
	if (phalcon_has_constructor(&imagick)) {
		PHALCON_CALL_METHOD(NULL, &imagick, "__construct");
	}
	object_init_ex(&bgpixel, imagickpixel_ce);
	PHALCON_CALL_METHOD(NULL, &bgpixel, "__construct", &background);
	zval_ptr_dtor(&background);
	PHALCON_CALL_METHOD(NULL, &imagick, "newimage", &width,  &height, &bgpixel);
	zval_ptr_dtor(&bgpixel);

	object_init_ex(&draw, draw_ce);
	if (phalcon_has_constructor(&draw)) {
		PHALCON_CALL_METHOD(NULL, &draw, "__construct");
	}
	phalcon_read_property(&font, getThis(), SL("_font"), PH_NOISY|PH_READONLY);
	if (zend_is_true(&font)) {
		PHALCON_CALL_METHOD(NULL, &draw, "setfont", &font);
	}
	phalcon_read_property(&font_size, getThis(), SL("_fontSize"), PH_NOISY|PH_READONLY);
	PHALCON_CALL_METHOD(NULL, &draw, "setfontsize", &font_size);

	object_init_ex(&fgpixel, imagickpixel_ce);
	PHALCON_CALL_METHOD(NULL, &fgpixel, "__construct", &foreground);
	zval_ptr_dtor(&foreground);
	PHALCON_CALL_METHOD(NULL, &draw, "setfillcolor", &fgpixel);
	zval_ptr_dtor(&fgpixel);

	if (!_background || Z_TYPE_P(_background) == IS_NULL) {
		object_init_ex(&gradient, imagick_ce);
		if (phalcon_has_constructor(&gradient)) {
			PHALCON_CALL_METHOD(NULL, &gradient, "__construct");
		}

		PHALCON_CALL_METHOD(&top_color, &random, "color");
		PHALCON_CALL_METHOD(&bottom_color, &random, "color");

		PHALCON_CONCAT_SVSV(&pseudostring, "gradient:", &top_color, "-", &bottom_color);
		zval_ptr_dtor(&top_color);
		zval_ptr_dtor(&bottom_color);
		PHALCON_CALL_METHOD(NULL, &gradient, "newpseudoimage", &width,  &height, &pseudostring);
		zval_ptr_dtor(&pseudostring);

		phalcon_get_class_constant(&composite, imagick_ce, SL("COMPOSITE_OVER"));
		PHALCON_CALL_METHOD(NULL, &imagick, "compositeimage", &gradient, &composite, &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));
		zval_ptr_dtor(&composite);
		zval_ptr_dtor(&gradient);
	}

	phalcon_read_property(&pad_type, getThis(), SL("_padType"), PH_NOISY|PH_READONLY);
	switch (Z_LVAL(pad_type)) {
		case PHALCON_CHART_CAPTCHA_PAD_LEFT:
			phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_WEST"));
			break;
		case PHALCON_CHART_CAPTCHA_PAD_RIGHT:
			phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_EAST"));
			break;
		default:
			phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_CENTER"));
			break;
	}

	PHALCON_CALL_METHOD(NULL, &draw, "setgravity", &gravity);
	zval_ptr_dtor(&gravity);
	PHALCON_CALL_METHOD(NULL, &imagick, "annotateimage", &draw, offset_x, offset_y, &PHALCON_GLOBAL(z_zero), &word);

	phalcon_read_property(&pad_size, getThis(), SL("_padSize"), PH_NOISY|PH_READONLY);
	if (Z_LVAL(pad_type) == PHALCON_CHART_CAPTCHA_PAD_BOTH || Z_LVAL(pad_type) == PHALCON_CHART_CAPTCHA_PAD_LEFT) {
		PHALCON_CALL_METHOD(&color, &random, "color", &color_type);
		PHALCON_CALL_METHOD(&word, &random, "alnum", &pad_size);
		object_init_ex(&pixel, imagickpixel_ce);
		PHALCON_CALL_METHOD(NULL, &pixel, "__construct", &color);
		zval_ptr_dtor(&color);
		PHALCON_CALL_METHOD(NULL, &draw, "setfillcolor", &pixel);
		zval_ptr_dtor(&pixel);

		phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_WEST"));

		PHALCON_CALL_METHOD(NULL, &draw, "setgravity", &gravity);
		zval_ptr_dtor(&gravity);
		PHALCON_CALL_METHOD(NULL, &imagick, "annotateimage", &draw, &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero), &word);
		zval_ptr_dtor(&word);
	}

	if (Z_LVAL(pad_type) == PHALCON_CHART_CAPTCHA_PAD_BOTH || Z_LVAL(pad_type) == PHALCON_CHART_CAPTCHA_PAD_RIGHT) {
		PHALCON_CALL_METHOD(&color, &random, "color", &color_type);
		PHALCON_CALL_METHOD(&word, &random, "alnum", &pad_size);
		object_init_ex(&pixel, imagickpixel_ce);
		PHALCON_CALL_METHOD(NULL, &pixel, "__construct", &color);
		zval_ptr_dtor(&color);
		PHALCON_CALL_METHOD(NULL, &draw, "setfillcolor", &pixel);
		zval_ptr_dtor(&pixel);

		phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_EAST"));

		PHALCON_CALL_METHOD(NULL, &draw, "setgravity", &gravity);
		zval_ptr_dtor(&gravity);
		PHALCON_CALL_METHOD(NULL, &imagick, "annotateimage", &draw, &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero), &word);
		zval_ptr_dtor(&word);
	}
	zval_ptr_dtor(&draw);

	ZVAL_LONG(&min, 20);
	ZVAL_LONG(&max, 50);

	PHALCON_CALL_FUNCTION(&roll1, "rand", &min, &max);

	ZVAL_LONG(&min, 30);

	PHALCON_CALL_FUNCTION(&corner1, "rand", &min, &max);
	PHALCON_CALL_FUNCTION(&corner2, "rand", &min, &max);
	PHALCON_CALL_METHOD(NULL, &imagick, "rollimage", &roll1,  &PHALCON_GLOBAL(z_zero));

	ZVAL_LONG(&corner1, -Z_LVAL(corner1));

	PHALCON_CALL_METHOD(NULL, &imagick, "swirlimage", &corner1);

	ZVAL_LONG(&roll2, -Z_LVAL(roll1)*2);

	PHALCON_CALL_METHOD(NULL, &imagick, "rollimage", &roll2,  &PHALCON_GLOBAL(z_zero));
	PHALCON_CALL_METHOD(NULL, &imagick, "swirlimage", &corner2);
	PHALCON_CALL_METHOD(NULL, &imagick, "rollimage", &roll1,  &PHALCON_GLOBAL(z_zero));

	ZVAL_STRING(&format, "png");

	PHALCON_CALL_METHOD(NULL, &imagick, "setImageFormat", &format);
	zval_ptr_dtor(&format);
	PHALCON_CALL_METHOD(NULL, &imagick, "stripImage");

	phalcon_update_property(getThis(), SL("_imagick"), &imagick);
	if (filename && zend_is_true(filename)) {
		PHALCON_CALL_METHOD(NULL, &imagick, "writeImage", filename);
	}
	PHALCON_RETURN_CALL_METHOD(&imagick, "getImageBlob");
	zval_ptr_dtor(&imagick);
}

/**
 * Save the image
 *
 *<code>
 *     $captcha = new \Phalcon\Chart\Captcha;
 *     $captcha->reander('Phalcon is a web framework');
 *     $captcha->save('captcha.png');
 *</code>
 *
 * @param filename $filename
 * @return boolean
 */
PHP_METHOD(Phalcon_Chart_Captcha, save){

	zval *filename, imagick = {};

	phalcon_fetch_params(0, 1, 0, &filename);

	phalcon_read_property(&imagick, getThis(), SL("_imagick"), PH_NOISY|PH_READONLY);
	if (Z_TYPE(imagick) != IS_OBJECT) {
		RETURN_FALSE;
	}
	PHALCON_RETURN_CALL_METHOD(&imagick, "writeImage", filename);
}
