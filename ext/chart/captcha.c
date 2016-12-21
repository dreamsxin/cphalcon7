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
 * $captcha = new \Phalcon\Chart\Captcha(NULL, NULL, 30, 150, 50);
 * echo $captcha = $captcha->render('Phalcon', 15, -10);
 *</code>
 */
zend_class_entry *phalcon_chart_captcha_ce;

PHP_METHOD(Phalcon_Chart_Captcha, __construct);
PHP_METHOD(Phalcon_Chart_Captcha, setFont);
PHP_METHOD(Phalcon_Chart_Captcha, setFontSize);
PHP_METHOD(Phalcon_Chart_Captcha, render);
PHP_METHOD(Phalcon_Chart_Captcha, save);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_chart_captcha_setfont, 0, 0, 1)
	ZEND_ARG_INFO(0, font)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_chart_captcha_setfontsize, 0, 0, 1)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_chart_captcha_render, 0, 0, 1)
	ZEND_ARG_INFO(0, word)
	ZEND_ARG_INFO(0, margin)
	ZEND_ARG_INFO(0, foreground)
	ZEND_ARG_INFO(0, background)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_chart_captcha_save, 0, 0, 0)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_chart_captcha_method_entry[] = {
	PHP_ME(Phalcon_Chart_Captcha, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
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
	zend_declare_property_null(phalcon_chart_captcha_ce, SL("_draw"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_chart_captcha_ce, SL("_word"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_chart_captcha_ce, SL("_font"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_chart_captcha_ce, SL("_fontSize"), 40, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_chart_captcha_ce, SL("_width"), 150, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_chart_captcha_ce, SL("_height"), 50, ZEND_ACC_PROTECTED);

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

	zval *word = NULL, *font = NULL, *_font_size = NULL, *width = NULL, *height = NULL, font_size = {}, imagick = {}, draw = {};
	zend_class_entry *imagick_ce, *draw_ce;

	phalcon_fetch_params(0, 0, 5, &word, &font, &_font_size, &width, &height);

	if (phalcon_class_str_exists(SL("imagick"), 0) == NULL) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_chart_exception_ce, "Imagick is not installed, or the extension is not loaded");
		return;
	}

	if (_font_size && Z_TYPE_P(_font_size) != IS_NULL) {
		phalcon_update_property_zval(getThis(), SL("_fontSize"), _font_size);
	}
	phalcon_read_property(&font_size, getThis(), SL("_fontSize"), PH_NOISY);

	imagick_ce = phalcon_fetch_str_class(SL("Imagick"), ZEND_FETCH_CLASS_AUTO);
	draw_ce = phalcon_fetch_str_class(SL("ImagickDraw"), ZEND_FETCH_CLASS_AUTO);

	object_init_ex(&imagick, imagick_ce);
	if (phalcon_has_constructor(&imagick)) {
		PHALCON_CALL_METHODW(NULL, &imagick, "__construct");
	}

	phalcon_update_property_zval(getThis(), SL("_imagick"), &imagick);

	object_init_ex(&draw, draw_ce);
	if (phalcon_has_constructor(&draw)) {
		PHALCON_CALL_METHODW(NULL, &draw, "__construct");
	}

	phalcon_update_property_zval(getThis(), SL("_draw"), &draw);

	if (font && Z_TYPE_P(font) != IS_NULL) {
		PHALCON_CALL_METHODW(NULL, &draw, "setfont", font);
	}

	PHALCON_CALL_METHODW(NULL, &draw, "setfontsize", &font_size);

	if (width && Z_TYPE_P(width) != IS_NULL) {
		phalcon_update_property_zval(getThis(), SL("_width"), width);
	}

	if (height && Z_TYPE_P(height) != IS_NULL) {
		phalcon_update_property_zval(getThis(), SL("_height"), height);
	}
}

/**
 * Sets a font
 *
 * @param string $font
 * @return boolean
 */
PHP_METHOD(Phalcon_Chart_Captcha, setFont){

	zval *font, draw = {};

	phalcon_fetch_params(0, 1, 0, &font);

	phalcon_read_property(&draw, getThis(), SL("_draw"), PH_NOISY);

	PHALCON_CALL_METHODW(NULL, &draw, "setfont", font);

	phalcon_update_property_zval(getThis(), SL("_font"), font);
	RETURN_THISW();
}

/**
 * Sets a font size
 *
 * @param string $fontSize
 * @return boolean
 */
PHP_METHOD(Phalcon_Chart_Captcha, setFontSize){

	zval *font_size, draw = {};

	phalcon_fetch_params(0, 1, 0, &font_size);

	phalcon_read_property(&draw, getThis(), SL("_draw"), PH_NOISY);

	PHALCON_CALL_METHODW(NULL, &draw, "setfontsize", font_size);

	phalcon_update_property_zval(getThis(), SL("_fontSize"), font_size);
	RETURN_THISW();
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

	zval *_word = NULL, *offset_x = NULL, *offset_y = NULL, *_foreground = NULL, *_background = NULL, *_width = NULL, *_height = NULL, word = {}, width = {};
	zval height = {}, foreground = {}, background = {}, imagick = {}, draw = {}, imagickpixel = {}, gradient = {}, columns = {}, rows = {}, random = {}, top_color = {};
	zval bottom_color = {}, pseudostring = {}, composite = {}, gravity = {}, min = {}, max = {}, roll1 = {}, roll2 = {};
	zval corner1 = {}, corner2 = {}, format = {};
	zend_class_entry  *imagick_ce, *imagickpixel_ce;

	phalcon_fetch_params(0, 0, 6, &_word, &offset_x, &offset_y, &_foreground, &_background, &_width, &_height);

	if (_word) {
		phalcon_update_property_zval(getThis(), SL("_word"), _word);
	}
	phalcon_read_property(&word, getThis(), SL("_word"), PH_NOISY);

	if (!offset_x) {
		offset_x  = &PHALCON_GLOBAL(z_zero);
	}

	if (!offset_y) {
		offset_y  = &PHALCON_GLOBAL(z_zero);
	}

	if (!_foreground || Z_TYPE_P(_foreground) == IS_NULL) {
		ZVAL_STRING(&foreground, "#ffffff");
	} else if (Z_TYPE_P(_foreground) == IS_NULL) {
		ZVAL_STRING(&foreground, "#ffffff");
	} else {
		PHALCON_CPY_WRT(&foreground, _foreground);
	}

	if (!_background || Z_TYPE_P(_background) == IS_NULL) {
		ZVAL_STRING(&background, "#000000");
	} else if (Z_TYPE_P(_background) == IS_NULL) {
		ZVAL_STRING(&background, "#000000");
	} else {
		PHALCON_CPY_WRT(&background, _background);
	}

	if (_width) {
		phalcon_update_property_zval(getThis(), SL("_width"), _width);
	}
	phalcon_read_property(&width, getThis(), SL("_width"), PH_NOISY);

	if (_height) {
		phalcon_update_property_zval(getThis(), SL("_height"), _height);
	}
	phalcon_read_property(&height, getThis(), SL("_height"), PH_NOISY);

	phalcon_return_property(&imagick, getThis(), SL("_imagick"));
	phalcon_return_property(&draw, getThis(), SL("_draw"));

	imagick_ce = phalcon_fetch_str_class(SL("Imagick"), ZEND_FETCH_CLASS_AUTO);
	imagickpixel_ce = phalcon_fetch_str_class(SL("ImagickPixel"), ZEND_FETCH_CLASS_AUTO);

	object_init_ex(&imagickpixel, imagickpixel_ce);
	PHALCON_CALL_METHODW(NULL, &imagickpixel, "__construct", &foreground);
	PHALCON_CALL_METHODW(NULL, &draw, "setfillcolor", &imagickpixel);
	PHALCON_CALL_METHODW(NULL, &imagickpixel, "setcolor", &background);
	PHALCON_CALL_METHODW(NULL, &imagick, "newimage", &width,  &height, &imagickpixel);
	if (!_background || Z_TYPE_P(_background) == IS_NULL) {
		object_init_ex(&gradient, imagick_ce);
		if (phalcon_has_constructor(&gradient)) {
			PHALCON_CALL_METHODW(NULL, &gradient, "__construct");
		}
		ZVAL_LONG(&columns, 80);
		ZVAL_LONG(&rows, 80);

		object_init_ex(&random, phalcon_random_ce);
		PHALCON_CALL_METHODW(&top_color, &random, "color");
		PHALCON_CALL_METHODW(&bottom_color, &random, "color");

		PHALCON_CONCAT_SVSV(&pseudostring, "gradient:", &top_color, "-", &bottom_color);
		PHALCON_CALL_METHODW(NULL, &gradient, "newpseudoimage", &columns, &rows, &pseudostring);

		phalcon_get_class_constant(&composite, imagick_ce, SL("COMPOSITE_OVER"));
		PHALCON_CALL_METHODW(NULL, &imagick, "compositeimage", &gradient, &composite, &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));
	}

	phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_CENTER"));

	PHALCON_CALL_METHODW(NULL, &draw, "setgravity", &gravity);
	PHALCON_CALL_METHODW(NULL, &imagick, "annotateimage", &draw, offset_x, offset_y, &PHALCON_GLOBAL(z_zero), &word);

	ZVAL_LONG(&min, 20);
	ZVAL_LONG(&max, 50);

	PHALCON_CALL_FUNCTIONW(&roll1, "rand", &min, &max);

	ZVAL_LONG(&min, 30);

	PHALCON_CALL_FUNCTIONW(&corner1, "rand", &min, &max);
	PHALCON_CALL_FUNCTIONW(&corner2, "rand", &min, &max);
	PHALCON_CALL_METHODW(NULL, &imagick, "rollimage", &roll1,  &PHALCON_GLOBAL(z_zero));

	ZVAL_LONG(&corner1, -Z_LVAL(corner1));

	PHALCON_CALL_METHODW(NULL, &imagick, "swirlimage", &corner1);

	ZVAL_LONG(&roll2, -Z_LVAL(roll1)*2);

	PHALCON_CALL_METHODW(NULL, &imagick, "rollimage", &roll2,  &PHALCON_GLOBAL(z_zero));
	PHALCON_CALL_METHODW(NULL, &imagick, "swirlimage", &corner2);
	PHALCON_CALL_METHODW(NULL, &imagick, "rollimage", &roll1,  &PHALCON_GLOBAL(z_zero));

	ZVAL_STRING(&format, "png");

	PHALCON_CALL_METHODW(NULL, &imagick, "setImageFormat", &format);
	PHALCON_CALL_METHODW(NULL, &imagick, "stripImage");

	PHALCON_RETURN_CALL_METHODW(&imagick, "getImageBlob");
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

	phalcon_read_property(&imagick, getThis(), SL("_imagick"), PH_NOISY);

	PHALCON_RETURN_CALL_METHODW(&imagick, "writeImage", filename);
}
