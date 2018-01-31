
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
  |          Vladimir Kolesnikov <vladimir@free-sevastopol.com>            |
  +------------------------------------------------------------------------+
*/

#include "image/adapter/imagick.h"
#include "image.h"
#include "image/adapter.h"
#include "image/adapterinterface.h"
#include "image/exception.h"

#include <string.h>
#include <stdlib.h>

#ifdef PHALCON_USE_MAGICKWAND
# if IM_MAGICKWAND_HEADER_STYLE_SEVEN
#  include <MagickWand/MagickWand.h>
# else
#  include <wand/MagickWand.h>
# endif
#endif

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/string.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/object.h"
#include "kernel/output.h"
#include "kernel/file.h"

/**
 * Phalcon\Image\Adapter\Imagick
 *
 * Image manipulation support. Allows images to be resized, cropped, etc.
 *
 *<code>
 *	$image = new Phalcon\Image\Adapter\Imagick("upload/test.jpg");
 *	$image->resize(200, 200)->rotate(90)->crop(100, 100);
 *	if ($image->save()) {
 *		echo 'success';
 *	}
 *</code>
 */
zend_class_entry *phalcon_image_adapter_imagick_ce;

PHP_METHOD(Phalcon_Image_Adapter_Imagick, check);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, __construct);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _resize);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _liquidRescale);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _crop);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _rotate);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _flip);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _sharpen);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _reflection);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _watermark);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _text);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _mask);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _background);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _blur);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _pixelate);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _save);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _render);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, __destruct);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, line);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, polygon);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, shadow);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, getInternalImInstance);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, setResourceLimit);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, colorize);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, gamma);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, levels);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, brightness_contrast);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, hsl);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, curves_graph);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, vignette);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, earlybird);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, inkwell);
PHP_METHOD(Phalcon_Image_Adapter_Imagick, convert);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter_imagick___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, file, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter_imagick_shadow, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, color, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, opacity, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, sigma, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter_imagick_setresourcelimit, 0, 0, 2)
	ZEND_ARG_INFO(0, resource)
	ZEND_ARG_INFO(0, limit)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter_imagick_colorize, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, color, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, composition, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter_imagick_gamma, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, gamma, IS_DOUBLE, 1)
	ZEND_ARG_TYPE_INFO(0, channel, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter_imagick_levels, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, gamma, IS_DOUBLE, 1)
	ZEND_ARG_TYPE_INFO(0, input_min, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, input_max, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, output_min, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, output_max, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, channel, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter_imagick_brightness_contrast, 0, 0, 2)
	ZEND_ARG_INFO(0, brightness)
	ZEND_ARG_INFO(0, contrast)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter_imagick_hsl, 0, 0, 0)
	ZEND_ARG_INFO(0, hue)
	ZEND_ARG_INFO(0, saturation)
	ZEND_ARG_INFO(0, lightness)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter_imagick_curves_graph, 0, 0, 1)
	ZEND_ARG_INFO(0, fx)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter_imagick_vignette, 0, 0, 1)
	ZEND_ARG_INFO(0, color)
	ZEND_ARG_TYPE_INFO(0, composition, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, crop_factor, IS_DOUBLE, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter_imagick_convert, 0, 0, 1)
	ZEND_ARG_INFO(0, command)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_image_adapter_imagick_method_entry[] = {
	PHP_ME(Phalcon_Image_Adapter_Imagick, check, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Adapter_Imagick, __construct, arginfo_phalcon_image_adapter_imagick___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Image_Adapter_Imagick, _resize, arginfo_phalcon_image_adapter__resize, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_Imagick, _liquidRescale, arginfo_phalcon_image_adapter__liquidrescale, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_Imagick, _crop, arginfo_phalcon_image_adapter__crop, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_Imagick, _rotate, arginfo_phalcon_image_adapter__rotate, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_Imagick, _flip, arginfo_phalcon_image_adapter__flip, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_Imagick, _sharpen, arginfo_phalcon_image_adapter__sharpen, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_Imagick, _reflection, arginfo_phalcon_image_adapter__reflection, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_Imagick, _watermark, arginfo_phalcon_image_adapter__watermark, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_Imagick, _text, arginfo_phalcon_image_adapter__text, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_Imagick, _mask, arginfo_phalcon_image_adapter__mask, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_Imagick, _background, arginfo_phalcon_image_adapter__background, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_Imagick, _blur, arginfo_phalcon_image_adapter__blur, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_Imagick, _pixelate, arginfo_phalcon_image_adapter__pixelate, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_Imagick, _save, arginfo_phalcon_image_adapter__save, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_Imagick, _render, arginfo_phalcon_image_adapter__render, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_Imagick, __destruct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_DTOR)
	PHP_ME(Phalcon_Image_Adapter_Imagick, line, arginfo_phalcon_image_adapterinterface_line, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter_Imagick, polygon, arginfo_phalcon_image_adapterinterface_polygon, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter_Imagick, shadow, arginfo_phalcon_image_adapter_imagick_shadow, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter_Imagick, getInternalImInstance, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter_Imagick, setResourceLimit, arginfo_phalcon_image_adapter_imagick_setresourcelimit, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Adapter_Imagick, colorize, arginfo_phalcon_image_adapter_imagick_colorize, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter_Imagick, gamma, arginfo_phalcon_image_adapter_imagick_gamma, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter_Imagick, levels, arginfo_phalcon_image_adapter_imagick_levels, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter_Imagick, brightness_contrast, arginfo_phalcon_image_adapter_imagick_brightness_contrast, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter_Imagick, hsl, arginfo_phalcon_image_adapter_imagick_hsl, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter_Imagick, curves_graph, arginfo_phalcon_image_adapter_imagick_curves_graph, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter_Imagick, vignette, arginfo_phalcon_image_adapter_imagick_vignette, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter_Imagick, earlybird, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter_Imagick, inkwell, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter_Imagick, convert, arginfo_phalcon_image_adapter_imagick_convert, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Image\Adapter\Imagick initializer
 */
PHALCON_INIT_CLASS(Phalcon_Image_Adapter_Imagick){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Image\\Adapter, Imagick, image_adapter_imagick, phalcon_image_adapter_ce, phalcon_image_adapter_imagick_method_entry, 0);

	zend_declare_property_null(phalcon_image_adapter_imagick_ce, SL("_version"), ZEND_ACC_PROTECTED | ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_image_adapter_imagick_ce, SL("_checked"), ZEND_ACC_PROTECTED | ZEND_ACC_STATIC);

	zend_class_implements(phalcon_image_adapter_imagick_ce, 1, phalcon_image_adapterinterface_ce);

	return SUCCESS;
}

/**
 * Checks if Imagick is enabled
 *
 * @return  boolean
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, check){

	zval *version;

	if (phalcon_class_str_exists(SL("imagick"), 0) == NULL) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Imagick is not installed, or the extension is not loaded");
		return;
	}

	if ((version = zend_get_constant_ex(SSL("imagick::IMAGICK_EXTNUM"), NULL, ZEND_FETCH_CLASS_SILENT)) == NULL) {
		zend_update_static_property_long(phalcon_image_adapter_imagick_ce, SL("_version"), 0);
	} else {
		zend_update_static_property_long(phalcon_image_adapter_imagick_ce, SL("_version"), phalcon_get_intval(version));
	}

	zend_update_static_property_bool(phalcon_image_adapter_imagick_ce, SL("_checked"), 1);
	RETURN_TRUE;
}

/**
 * Phalcon\Image\Imagick constructor
 *
 * @param string $file
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, __construct){

	zval *file, *w = NULL, *h = NULL, checked = {}, width = {}, height = {}, format = {}, type = {}, mime = {}, im = {}, ret = {}, mode = {};
	zend_class_entry *imagick_ce, *ce1;

	phalcon_fetch_params(0, 1, 2, &file, &w, &h);

	imagick_ce = phalcon_fetch_str_class(SL("Imagick"), ZEND_FETCH_CLASS_AUTO);

	phalcon_read_static_property_ce(&checked, phalcon_image_adapter_imagick_ce, SL("_checked"), PH_READONLY);

	if (!zend_is_true(&checked)) {
		PHALCON_CALL_CE_STATIC(NULL, phalcon_image_adapter_imagick_ce, "check");
	}

	phalcon_update_property(getThis(), SL("_file"), file);

	object_init_ex(&im, imagick_ce);
	if (phalcon_has_constructor(&im)) {
		PHALCON_CALL_METHOD(NULL, &im, "__construct");
	}

	if (phalcon_file_exists(file) != FAILURE) {
		zval realpath = {};
		phalcon_file_realpath(&realpath, file);
		phalcon_update_property(getThis(), SL("_realpath"), &realpath);

		PHALCON_CALL_METHOD(NULL, &im, "readImage", &realpath);
		zval_ptr_dtor(&realpath);

		PHALCON_CALL_METHOD(&width, &im, "getImageWidth");
		phalcon_update_property(getThis(), SL("_width"), &width);

		PHALCON_CALL_METHOD(&height, &im, "getImageHeight");
		phalcon_update_property(getThis(), SL("_height"), &height);

		PHALCON_CALL_METHOD(&type, &im, "getImageType");
		phalcon_update_property(getThis(), SL("_type"), &type);

		PHALCON_CALL_METHOD(&format, &im, "getImageFormat");
		phalcon_update_property(getThis(), SL("_format"), &format);

		PHALCON_CONCAT_SV(&mime, "image/", &format);

		phalcon_update_property(getThis(), SL("_mime"), &mime);

		PHALCON_CALL_METHOD(&ret, &im, "getImageAlphaChannel");

		if (!zend_is_true(&ret)) {
			phalcon_get_class_constant(&mode, imagick_ce, SL("ALPHACHANNEL_SET"));
			PHALCON_CALL_METHOD(NULL, &im, "setImageAlphaChannel", &mode);
		}

		if (phalcon_get_intval(&type) == 1) {
			PHALCON_CALL_METHOD(&ret, &im, "coalesceImages");
			PHALCON_CALL_METHOD(NULL, &im, "clear");
			PHALCON_CALL_METHOD(NULL, &im, "destroy");

			phalcon_update_property(getThis(), SL("_image"), &ret);
			zval_ptr_dtor(&ret);
		} else {
			phalcon_update_property(getThis(), SL("_image"), &im);
		}
	} else if (w && h) {
		zval imagickpixel = {};
		ce1 = phalcon_fetch_str_class(SL("ImagickPixel"), ZEND_FETCH_CLASS_AUTO);

		object_init_ex(&imagickpixel, ce1);

		if (phalcon_has_constructor(&imagickpixel)) {
			zval color = {};
			ZVAL_STRING(&color, "transparent");
			PHALCON_CALL_METHOD(NULL, &imagickpixel, "__construct", &color);
			zval_ptr_dtor(&color);
		}

		PHALCON_CALL_METHOD(NULL, &im, "newImage", w, h, &imagickpixel);
		zval_ptr_dtor(&imagickpixel);

		ZVAL_STRING(&format, "png");

		phalcon_update_property(getThis(), SL("_format"), &format);

		PHALCON_CALL_METHOD(&ret, &im, "setformat", &format);
		PHALCON_CALL_METHOD(&ret, &im, "setImageFormat", &format);

		phalcon_update_property(getThis(), SL("_image"), &im);
		phalcon_update_property(getThis(), SL("_realpath"), file);
		phalcon_update_property(getThis(), SL("_width"), w);
		phalcon_update_property(getThis(), SL("_height"), h);

		ZVAL_LONG(&type, 3);

		phalcon_update_property(getThis(), SL("_type"), &type);

		ZVAL_STRING(&mime, "image/png");

		phalcon_update_property(getThis(), SL("_mime"), &mime);
	} else {
		zend_throw_exception_ex(phalcon_image_exception_ce, 0, "Failed to create image from file '%s'", Z_STRVAL_P(file));
	}

	zval_ptr_dtor(&format);
	zval_ptr_dtor(&im);
	zval_ptr_dtor(&mime);
}

/**
 * Execute a resize.
 *
 * @param int $width
 * @param int $height
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _resize) {

	zval *width, *height, im = {}, w = {}, h = {}, next = {};

	phalcon_fetch_params(0, 2, 0, &width, &height);

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);

	PHALCON_CALL_METHOD(NULL, &im, "setIteratorIndex", &PHALCON_GLOBAL(z_null));

	do {
		PHALCON_CALL_METHOD(NULL, &im, "scaleImage", width, height);
		PHALCON_CALL_METHOD(&next, &im, "nextImage");
	} while (zend_is_true(&next));

	PHALCON_CALL_METHOD(&w, &im, "getImageWidth");
	PHALCON_CALL_METHOD(&h, &im, "getImageHeight");

	phalcon_update_property(getThis(), SL("_width"), &w);
	phalcon_update_property(getThis(), SL("_height"), &h);
}

/**
 * This method scales the images using liquid rescaling method. Only support Imagick
 *
 * @param int $width   new width
 * @param int $height  new height
 * @param int $delta_x How much the seam can traverse on x-axis. Passing 0 causes the seams to be straight.
 * @param int $rigidity Introduces a bias for non-straight seams. This parameter is typically 0.
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _liquidRescale){

	zval *width, *height, *delta_x = NULL, *rigidity = NULL, im = {}, ret = {}, w = {}, h = {}, next = {};

	phalcon_fetch_params(0, 4, 0, &width, &height, &delta_x, &rigidity);

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);

	PHALCON_CALL_METHOD(NULL, &im, "setIteratorIndex", &PHALCON_GLOBAL(z_zero));

	do {
		PHALCON_CALL_METHOD(&ret, &im, "liquidRescaleImage", width, height, delta_x, rigidity);

		if (!zend_is_true(&ret)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Imagick::liquidRescale failed");
			return;
		}

		PHALCON_CALL_METHOD(&next, &im, "nextImage");
	} while (zend_is_true(&next));

	PHALCON_CALL_METHOD(&w, &im, "getImageWidth");
	PHALCON_CALL_METHOD(&h, &im, "getImageHeight");

	phalcon_update_property(getThis(), SL("_width"), &w);
	phalcon_update_property(getThis(), SL("_height"), &h);
}

/**
 * Execute a crop.
 *
 * @param int $width
 * @param int $height
 * @param int $offset_x
 * @param int $offset_y
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _crop) {

	zval *width, *height, *offset_x, *offset_y, w = {}, h = {}, im = {}, next = {};

	phalcon_fetch_params(0, 4, 0, &width, &height, &offset_x, &offset_y);

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);

	PHALCON_CALL_METHOD(NULL, &im, "setIteratorIndex", &PHALCON_GLOBAL(z_zero));

	do {
		PHALCON_CALL_METHOD(NULL, &im, "cropImage", width, height, offset_x, offset_y);
		PHALCON_CALL_METHOD(NULL, &im, "setImagePage", width, height, &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));
		PHALCON_CALL_METHOD(&next, &im, "nextImage");
	} while (zend_is_true(&next));

	PHALCON_CALL_METHOD(&w, &im, "getImageWidth");
	PHALCON_CALL_METHOD(&h, &im, "getImageHeight");

	phalcon_update_property(getThis(), SL("_width"), &w);
	phalcon_update_property(getThis(), SL("_height"), &h);
}

/**
 * Execute a rotation.
 *
 * @param int $degrees
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _rotate) {

	zval *degrees, im = {}, background = {}, color = {}, ret = {}, w = {}, h = {}, next = {};
	zend_class_entry *imagick_pixel_ce;

	phalcon_fetch_params(0, 1, 0, &degrees);

	imagick_pixel_ce = phalcon_fetch_str_class(SL("ImagickPixel"), ZEND_FETCH_CLASS_AUTO);

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);

	ZVAL_STRING(&color, "transparent");

	object_init_ex(&background, imagick_pixel_ce);
	PHALCON_CALL_METHOD(NULL, &background, "__construct", &color);
	zval_ptr_dtor(&color);

	PHALCON_CALL_METHOD(NULL, &im, "setIteratorIndex", &PHALCON_GLOBAL(z_zero));

	do {
		PHALCON_CALL_METHOD(&ret, &im, "rotateImage", &background, degrees);

		if (!zend_is_true(&ret)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Imagick::rotate failed");
			return;
		}

		if (Z_TYPE(w) <= IS_NULL) {
			PHALCON_CALL_METHOD(&w, &im, "getImageWidth");
			PHALCON_CALL_METHOD(&h, &im, "getImageHeight");
		}

		PHALCON_CALL_METHOD(NULL, &im, "setImagePage", &w, &h, &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));
		PHALCON_CALL_METHOD(&next, &im, "nextImage");
	} while (zend_is_true(&next));
	zval_ptr_dtor(&background);

	phalcon_update_property(getThis(), SL("_width"), &w);
	phalcon_update_property(getThis(), SL("_height"), &h);
}

/**
 * Execute a flip.
 *
 * @param int $direction
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _flip)
{
	zval *direction, im = {}, next = {};
	char *method;

	phalcon_fetch_params(0, 1, 0, &direction);

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);
	method = (Z_LVAL_P(direction) == 11) ? "flopImage" : "flipImage";

	PHALCON_CALL_METHOD(NULL, &im, "setIteratorIndex", &PHALCON_GLOBAL(z_zero));

	do {
		PHALCON_CALL_METHOD(NULL, &im, method);
		PHALCON_CALL_METHOD(&next, &im, "nextImage");
	} while (zend_is_true(&next));
}

/**
 * Execute a sharpen.
 *
 * @param int $amount
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _sharpen) {

	zval *amount, a = {}, ret = {}, im = {}, next = {};
	int int_amount;

	phalcon_fetch_params(0, 1, 0, &amount);

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);

	int_amount = phalcon_get_intval(amount);
	int_amount = (int_amount < 5) ? 5 : int_amount;

	ZVAL_DOUBLE(&a, int_amount * 0.03);

	PHALCON_CALL_METHOD(NULL, &im, "setIteratorIndex", &PHALCON_GLOBAL(z_zero));

	do {
		PHALCON_CALL_METHOD(&ret, &im, "sharpenImage", &PHALCON_GLOBAL(z_zero), &a);

		if (!zend_is_true(&ret)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Imagick::sharpen failed");
			return;
		}

		PHALCON_CALL_METHOD(&next, &im, "nextImage");
	} while (zend_is_true(&next));
}

/**
 * Execute a reflection.
 *
 * @param int $height
 * @param int $opacity
 * @param boolean $fade_in
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _reflection) {

	zval *height, *opacity, *fade_in, im = {}, image_width = {}, image_height = {}, version = {}, reflection = {}, reflection_width = {}, reflection_height = {};
	zval fade = {}, pseudoString = {}, composite = {}, constant = {}, o = {}, channel = {}, image = {};
	zval background = {}, mode = {}, ret = {}, w = {}, h = {}, delay = {}, h0 = {}, colorspace = {}, next = {};
	zend_class_entry *imagick_ce, *imagick_pixel_ce;

	phalcon_fetch_params(0, 3, 0, &height, &opacity, &fade_in);

	imagick_ce       = phalcon_fetch_str_class(SL("Imagick"), ZEND_FETCH_CLASS_AUTO);
	imagick_pixel_ce = phalcon_fetch_str_class(SL("ImagickPixel"), ZEND_FETCH_CLASS_AUTO);

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);
	phalcon_read_property(&image_width, getThis(), SL("_width"), PH_READONLY);
	phalcon_read_property(&image_height, getThis(), SL("_height"), PH_READONLY);
	phalcon_read_static_property_ce(&version, phalcon_image_adapter_imagick_ce, SL("_version"), PH_READONLY);

	if (phalcon_get_intval(&version) >= 30100) {
		if (FAILURE == phalcon_clone(&reflection, &im)) {
			return;
		}
	} else {
		PHALCON_CALL_METHOD(&reflection, &im, "clone");
	}

	PHALCON_CALL_METHOD(NULL, &reflection, "setIteratorIndex", &PHALCON_GLOBAL(z_zero));

	do {
		PHALCON_CALL_METHOD(NULL, &reflection, "flipImage");
		PHALCON_CALL_METHOD(NULL, &reflection, "cropImage", &image_width, height, &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));
		PHALCON_CALL_METHOD(NULL, &reflection, "setImagePage", &image_width, height, &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));
		PHALCON_CALL_METHOD(&next, &reflection, "nextImage");
	} while (zend_is_true(&next));

	object_init_ex(&fade, imagick_ce);
	if (phalcon_has_constructor(&fade)) {
		PHALCON_CALL_METHOD(NULL, &fade, "__construct");
	}

	ZVAL_STRING(&pseudoString, (zend_is_true(fade_in) ? "gradient:black-transparent" : "gradient:transparent-black"));

	PHALCON_CALL_METHOD(&reflection_width, &reflection, "getImageWidth");
	PHALCON_CALL_METHOD(&reflection_height, &reflection, "getImageHeight");

	PHALCON_ENSURE_IS_LONG(&reflection_width);
	PHALCON_ENSURE_IS_LONG(&reflection_height);

	PHALCON_CALL_METHOD(NULL, &fade, "newPseudoImage", &reflection_width, &reflection_height, &pseudoString);
	zval_ptr_dtor(&pseudoString);

	phalcon_get_class_constant(&composite, imagick_ce, SL("COMPOSITE_DSTOUT"));

	PHALCON_CALL_METHOD(NULL, &reflection, "setIteratorIndex", &PHALCON_GLOBAL(z_zero));

	do {
		PHALCON_CALL_METHOD(&ret, &reflection, "compositeImage", &fade, &composite, &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));

		if (!zend_is_true(&ret)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Imagick::compositeImage failed");
			return;
		}

		PHALCON_CALL_METHOD(&next, &reflection, "nextImage");
	} while (zend_is_true(&next));
	zval_ptr_dtor(&fade);

	phalcon_get_class_constant(&constant, imagick_ce, SL("EVALUATE_MULTIPLY"));

	ZVAL_DOUBLE(&o, phalcon_get_intval(opacity) / 100);

	phalcon_get_class_constant(&channel, imagick_ce, SL("CHANNEL_ALPHA"));


	PHALCON_CALL_METHOD(NULL, &reflection, "setIteratorIndex", &PHALCON_GLOBAL(z_zero));

	do {
		PHALCON_CALL_METHOD(NULL, &reflection, "evaluateImage", &constant, &o, &channel);
		PHALCON_CALL_METHOD(&next, &reflection, "nextImage");
	} while (zend_is_true(&next));

	object_init_ex(&image, imagick_ce);
	if (phalcon_has_constructor(&image)) {
		PHALCON_CALL_METHOD(NULL, &image, "__construct");
	}

	ZVAL_LONG(&h0, phalcon_get_intval(&image_height) + phalcon_get_intval(height));

	object_init_ex(&background, imagick_pixel_ce);
	if (phalcon_has_constructor(&background)) {
		PHALCON_CALL_METHOD(NULL, &background, "__construct");
	}

	phalcon_get_class_constant(&mode, imagick_ce, SL("ALPHACHANNEL_SET"));

	PHALCON_CALL_METHOD(&colorspace, &im, "getcolorspace");

	phalcon_get_class_constant(&composite, imagick_ce, SL("COMPOSITE_SRC"));

	PHALCON_CALL_METHOD(NULL, &im, "setIteratorIndex", &PHALCON_GLOBAL(z_zero));
	PHALCON_CALL_METHOD(&delay, &im, "getImageDelay");

	do {
		PHALCON_CALL_METHOD(NULL, &image, "newImage", &image_width, &h0, &background);
		PHALCON_CALL_METHOD(NULL, &image, "setImageAlphaChannel", &mode);
		PHALCON_CALL_METHOD(NULL, &image, "setcolorspace", &colorspace);
		PHALCON_CALL_METHOD(NULL, &image, "setImageDelay", &delay);
		PHALCON_CALL_METHOD(&ret, &image, "compositeImage", &im, &composite, &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));

		if (!zend_is_true(&ret)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Imagick::compositeImage failed");
			return;
		}

		PHALCON_CALL_METHOD(&next, &im, "nextImage");
	} while (zend_is_true(&next));
	zval_ptr_dtor(&colorspace);
	zval_ptr_dtor(&background);

	phalcon_get_class_constant(&composite, imagick_ce, SL("COMPOSITE_OVER"));

	PHALCON_CALL_METHOD(NULL, &image, "setIteratorIndex", &PHALCON_GLOBAL(z_zero));
	PHALCON_CALL_METHOD(NULL, &reflection, "setIteratorIndex", &PHALCON_GLOBAL(z_zero));

	do {
		PHALCON_CALL_METHOD(&ret, &image, "compositeImage", &reflection, &composite, &PHALCON_GLOBAL(z_zero), &image_height);

		if (!zend_is_true(&ret)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Imagick::compositeImage failed");
			return;
		}

		PHALCON_CALL_METHOD(&next, &image, "nextImage");
		PHALCON_CALL_METHOD(NULL, &reflection, "nextImage");
	} while (zend_is_true(&next));
	zval_ptr_dtor(&reflection);

	PHALCON_CALL_METHOD(&w, &image, "getImageWidth");
	PHALCON_CALL_METHOD(&h, &image, "getImageHeight");

	phalcon_update_property(getThis(), SL("_width"), &w);
	phalcon_update_property(getThis(), SL("_height"), &h);
	phalcon_update_property(getThis(), SL("_image"), &image);
	zval_ptr_dtor(&image);
}

/**
 * Execute a watermarking.
 *
 * @param Phalcon\Image\Adapter $watermark
 * @param int $offset_x
 * @param int $offset_y
 * @param int $opacity
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _watermark) {

	zval *watermark_image, *offset_x, *offset_y, *opacity, op = {}, im = {}, watermark = {}, realpath = {}, blob = {}, ret = {}, channel = {};
	zval op_constant = {}, composite = {}, index = {}, next = {}, type = {};
	zend_class_entry *ce0;
	double num;

	phalcon_fetch_params(0, 4, 0, &watermark_image, &offset_x, &offset_y, &opacity);

	ce0 = phalcon_fetch_str_class(SL("Imagick"), ZEND_FETCH_CLASS_AUTO);

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);
	phalcon_read_property(&type, getThis(), SL("_type"), PH_READONLY);

	object_init_ex(&watermark, ce0);
	if (phalcon_has_constructor(&watermark)) {
		PHALCON_CALL_METHOD(NULL, &watermark, "__construct");
	}

	PHALCON_CALL_METHOD(&realpath, watermark_image, "getrealpath");
	PHALCON_CALL_METHOD(&blob, watermark_image, "render");
	PHALCON_CALL_METHOD(NULL, &watermark, "readImageBlob", &blob, &realpath);
	zval_ptr_dtor(&blob);
	zval_ptr_dtor(&realpath);

	phalcon_get_class_constant(&channel, ce0, SL("ALPHACHANNEL_ACTIVATE"));

	PHALCON_CALL_METHOD(&ret, &watermark, "getImageAlphaChannel");

	if (!PHALCON_IS_EQUAL(&ret, &channel)) {
		phalcon_get_class_constant(&channel, ce0, SL("ALPHACHANNEL_OPAQUE"));
		PHALCON_CALL_METHOD(NULL, &watermark, "setImageAlphaChannel", &channel);
	}

	if (phalcon_get_intval(opacity) < 100) {
		phalcon_get_class_constant(&op_constant, ce0, SL("EVALUATE_MULTIPLY"));

		num = Z_LVAL_P(opacity) / 100;
		ZVAL_DOUBLE(&op, num);

		phalcon_get_class_constant(&channel, ce0, SL("CHANNEL_ALPHA"));

		PHALCON_CALL_METHOD(NULL, &watermark, "evaluateImage", &op_constant, &op, &channel);
	}

	phalcon_get_class_constant(&composite, ce0, SL("COMPOSITE_DISSOLVE"));

	if (phalcon_get_intval(&type) == 1) {
		ZVAL_LONG(&index, 0);

		PHALCON_CALL_METHOD(NULL, &im, "setIteratorIndex", &index);

		do {
			PHALCON_CALL_METHOD(&ret, &im, "compositeImage", &watermark, &composite, offset_x, offset_y);

			if (!zend_is_true(&ret)) {
				PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Imagick::watermark failed");
				return;
			}

			PHALCON_CALL_METHOD(&next, &im, "nextImage");
		} while (zend_is_true(&next));
	} else {
		PHALCON_CALL_METHOD(&ret, &im, "compositeImage", &watermark, &composite, offset_x, offset_y);

		if (!zend_is_true(&ret)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Imagick::watermark failed");
		}
	}
	zval_ptr_dtor(&watermark);
}

/**
 * Execute a text
 *
 * @param string text
 * @param int $offset_x
 * @param int $offset_y
 * @param int $opacity
 * @param int $r
 * @param int $g
 * @param int $b
 * @param int $size
 * @param string $fontfile
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _text) {

	zval *text, *offset_x, *offset_y, *opacity, *r, *g, *b, *size, *fontfile;
	zval im = {}, draw = {}, pixel = {}, format = {}, color = {}, gravity = {}, tmp_a = {};
	zend_class_entry *imagick_ce, *imagick_draw_ce, *imagick_pixel_ce;
	int x, y;

	phalcon_fetch_params(0, 8, 1, &text, &offset_x, &offset_y, &opacity, &r, &g, &b, &size, &fontfile);
	PHALCON_SEPARATE_PARAM(offset_x);
	PHALCON_SEPARATE_PARAM(offset_y);

	if (!fontfile) {
		fontfile = &PHALCON_GLOBAL(z_null);
	}

	imagick_ce       = phalcon_fetch_str_class(SL("Imagick"), ZEND_FETCH_CLASS_AUTO);
	imagick_draw_ce  = phalcon_fetch_str_class(SL("ImagickDraw"), ZEND_FETCH_CLASS_AUTO);
	imagick_pixel_ce = phalcon_fetch_str_class(SL("ImagickPixel"), ZEND_FETCH_CLASS_AUTO);

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);

	object_init_ex(&draw, imagick_draw_ce);
	if (phalcon_has_constructor(&draw)) {
		PHALCON_CALL_METHOD(NULL, &draw, "__construct");
	}

	ZVAL_STRING(&format, "rgb(%d, %d, %d)");
	PHALCON_CALL_FUNCTION(&color, "sprintf", &format, r, g, b);
	zval_ptr_dtor(&format);

	object_init_ex(&pixel, imagick_pixel_ce);
	if (phalcon_has_constructor(&pixel)) {
		PHALCON_CALL_METHOD(NULL, &pixel, "__construct", &color);
	}
	zval_ptr_dtor(&color);

	PHALCON_CALL_METHOD(NULL, &draw, "setfillcolor", &pixel);
	zval_ptr_dtor(&pixel);

	if (fontfile && Z_TYPE_P(fontfile) == IS_STRING) {
		PHALCON_CALL_METHOD(NULL, &draw, "setfont", fontfile);
	}

	if (size) {
		PHALCON_CALL_METHOD(NULL, &draw, "setfontsize", size);
	}

	PHALCON_CALL_METHOD(NULL, &draw, "setfillopacity", opacity);

	ZVAL_LONG(&tmp_a, 0);

	if (PHALCON_IS_TRUE(offset_x)) {
		if (PHALCON_IS_TRUE(offset_y)) {
			ZVAL_LONG(offset_x, 0);
			ZVAL_LONG(offset_y, 0);

			phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_SOUTHEAST"));
		} else if (PHALCON_IS_FALSE(offset_y)) {
			ZVAL_LONG(offset_x, 0);
			ZVAL_LONG(offset_y, 0);

			phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_NORTHEAST"));
		}  else if (Z_TYPE_P(offset_y) == IS_LONG) {
			y = phalcon_get_intval(offset_y);
			if (y < 0) {
				ZVAL_LONG(offset_x, 0);
				ZVAL_LONG(offset_y, y * -1);

				phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_SOUTHEAST"));
			} else {
				ZVAL_LONG(offset_x, 0);

				phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_NORTHEAST"));
			}
		} else {
			ZVAL_LONG(offset_x, 0);
			ZVAL_LONG(offset_y, 0);

			phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_EAST"));
		}
	} else if (PHALCON_IS_FALSE(offset_x)) {
		if (PHALCON_IS_TRUE(offset_y)) {
			ZVAL_LONG(offset_x, 0);
			ZVAL_LONG(offset_y, 0);

			phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_SOUTHWEST"));
		} else if (PHALCON_IS_FALSE(offset_y)) {
			ZVAL_LONG(offset_x, 0);
			ZVAL_LONG(offset_y, 0);

			phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_NORTHWEST"));
		}  else if (Z_TYPE_P(offset_y) == IS_LONG) {
			y = phalcon_get_intval(offset_y);
			if (y < 0) {
				ZVAL_LONG(offset_x, 0);
				ZVAL_LONG(offset_y, y * -1);
				phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_SOUTHWEST"));
			} else {
				ZVAL_LONG(offset_x, 0);
				phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_NORTHWEST"));
			}
		} else {
			ZVAL_LONG(offset_x, 0);
			ZVAL_LONG(offset_y, 0);
			phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_WEST"));
		}
	} else if (Z_TYPE_P(offset_x) == IS_LONG) {
		x = phalcon_get_intval(offset_x);
		if (x < 0) {
			ZVAL_LONG(offset_x, x  * -1);

			if (PHALCON_IS_TRUE(offset_y)) {
				ZVAL_LONG(offset_y, 0);
				phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_SOUTHEAST"));
			} else if (PHALCON_IS_FALSE(offset_y)) {
				ZVAL_LONG(offset_y, 0);
				phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_NORTHEAST"));
			}  else if (Z_TYPE_P(offset_y) == IS_LONG) {
				y = phalcon_get_intval(offset_y);
				if (y < 0) {
					ZVAL_LONG(offset_y, y * -1);
					phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_SOUTHEAST"));
				} else {
					phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_NORTHEAST"));
				}
			} else {
				ZVAL_LONG(offset_x, 0);
				ZVAL_LONG(offset_y, 0);
				phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_EAST"));
			}
		} else {
			if (PHALCON_IS_TRUE(offset_y)) {
				ZVAL_LONG(offset_y, 0);
				phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_SOUTHWEST"));
			} else if (PHALCON_IS_FALSE(offset_y)) {
				ZVAL_LONG(offset_y, 0);
				phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_NORTHWEST"));
			}  else if (Z_TYPE_P(offset_y) == IS_LONG) {
				y = phalcon_get_intval(offset_y);
				if (y < 0) {
					ZVAL_LONG(offset_y, y * -1);
					phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_SOUTHWEST"));
				} else {
					phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_NORTHWEST"));
				}
			} else {
				ZVAL_LONG(offset_x, 0);
				ZVAL_LONG(offset_y, 0);
				phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_WEST"));
			}
		}
	} else {
		ZVAL_LONG(offset_x, 0);

		if (PHALCON_IS_TRUE(offset_y)) {
			ZVAL_LONG(offset_y, 0);
			phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_SOUTH"));
		} else if (PHALCON_IS_FALSE(offset_y)) {
			ZVAL_LONG(offset_y, 0);
			phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_NORTH"));
		}  else if (Z_TYPE_P(offset_y) == IS_LONG) {
			y = phalcon_get_intval(offset_y);
			if (y < 0) {
				ZVAL_LONG(offset_y, y * -1);

				phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_SOUTH"));
			} else {
				phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_NORTH"));
			}
		} else {
			ZVAL_LONG(offset_x, 0);
			ZVAL_LONG(offset_y, 0);
			phalcon_get_class_constant(&gravity, imagick_ce, SL("GRAVITY_CENTER"));
		}
	}

	PHALCON_CALL_METHOD(NULL, &draw, "setgravity", &gravity);
	PHALCON_CALL_METHOD(NULL, &im, "annotateImage", &draw, offset_x, offset_y, &tmp_a, text);
	zval_ptr_dtor(&draw);
}

/**
 * Composite one image onto another
 *
 * @param Phalcon\Image\Adapter $mask  mask Image instance
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _mask){

	zval *mask, im = {}, mask_im = {}, realpath = {}, blob = {}, matte = {}, composite = {}, next = {};
	zend_class_entry *imagick_ce;

	phalcon_fetch_params(0, 1, 0, &mask);

	imagick_ce = phalcon_fetch_str_class(SL("Imagick"), ZEND_FETCH_CLASS_AUTO);

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);

	object_init_ex(&mask_im, imagick_ce);
	if (phalcon_has_constructor(&mask_im)) {
		PHALCON_CALL_METHOD(NULL, &mask_im, "__construct");
	}

	PHALCON_CALL_METHOD(&realpath, mask, "getrealpath");
	PHALCON_CALL_METHOD(&blob, mask, "render");

	PHALCON_CALL_METHOD(NULL, &mask_im, "readImageBlob", &blob, &realpath);
	zval_ptr_dtor(&realpath);
	zval_ptr_dtor(&blob);

	ZVAL_LONG(&matte, 1);

	phalcon_get_class_constant(&composite, imagick_ce, SL("COMPOSITE_DSTIN"));

	PHALCON_CALL_METHOD(NULL, &im, "setIteratorIndex", &PHALCON_GLOBAL(z_zero));

	do {
		PHALCON_CALL_METHOD(NULL, &im, "setImageMatte", &matte);
		PHALCON_CALL_METHOD(NULL, &im, "compositeImage", &mask_im, &composite, &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));
		PHALCON_CALL_METHOD(&next, &im, "nextImage");
	} while (zend_is_true(&next));

	PHALCON_CALL_METHOD(NULL, &mask_im, "clear");
	zval_ptr_dtor(&mask_im);
}

/**
 * Execute a background.
 *
 * @param int $r
 * @param int $g
 * @param int $b
 * @param int $opacity
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _background) {

	zval *r, *g, *b, *opacity, op = {}, im = {}, color = {}, format = {}, background = {}, imagickpixel = {}, width = {}, height = {}, ret = {};
	zval mode = {}, op_constant = {}, channel = {}, composite = {}, next = {};
	zend_class_entry *imagick_ce, *imagick_pixel_ce;

	phalcon_fetch_params(0, 4, 0, &r, &g, &b, &opacity);

	imagick_ce       = phalcon_fetch_str_class(SL("Imagick"), ZEND_FETCH_CLASS_AUTO);
	imagick_pixel_ce = phalcon_fetch_str_class(SL("ImagickPixel"), ZEND_FETCH_CLASS_AUTO);

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);
	phalcon_read_property(&width, getThis(), SL("_width"), PH_READONLY);
	phalcon_read_property(&height, getThis(), SL("_height"), PH_READONLY);

	ZVAL_STRING(&format, "rgb(%d, %d, %d)");
	PHALCON_CALL_FUNCTION(&color, "sprintf", &format, r, g, b);
	zval_ptr_dtor(&format);

	object_init_ex(&background, imagick_ce);
	if (phalcon_has_constructor(&background)) {
		PHALCON_CALL_METHOD(NULL, &background, "__construct");
	}

	object_init_ex(&imagickpixel, imagick_pixel_ce);
	PHALCON_CALL_METHOD(NULL, &imagickpixel, "__construct", &color);
	zval_ptr_dtor(&color);
	PHALCON_CALL_METHOD(NULL, &background, "newImage", &width, &height, &imagickpixel);
	zval_ptr_dtor(&imagickpixel);
	PHALCON_CALL_METHOD(&ret, &background, "getImageAlphaChannel");

	if (!zend_is_true(&ret)) {
		phalcon_get_class_constant(&mode, imagick_ce, SL("ALPHACHANNEL_SET"));
		PHALCON_CALL_METHOD(NULL, &background, "setImageAlphaChannel", &mode);
	}

	object_init_ex(&imagickpixel, imagick_pixel_ce);

	ZVAL_STRING(&color, "transparent");

	PHALCON_CALL_METHOD(NULL, &imagickpixel, "__construct", &color);
	zval_ptr_dtor(&color);
	PHALCON_CALL_METHOD(NULL, &background, "setImageBackgroundColor", &imagickpixel);
	zval_ptr_dtor(&imagickpixel);

	phalcon_get_class_constant(&op_constant, imagick_ce, SL("EVALUATE_MULTIPLY"));

	ZVAL_DOUBLE(&op, phalcon_get_intval(opacity) / 100);

	phalcon_get_class_constant(&channel, imagick_ce, SL("CHANNEL_ALPHA"));

	PHALCON_CALL_METHOD(NULL, &background, "evaluateImage", &op_constant, &op, &channel);
	PHALCON_CALL_METHOD(&ret, &im, "getcolorspace");
	PHALCON_CALL_METHOD(NULL, &background, "setcolorspace", &ret);

	phalcon_get_class_constant(&composite, imagick_ce, SL("COMPOSITE_DISSOLVE"));

	PHALCON_CALL_METHOD(NULL, &im, "setIteratorIndex", &PHALCON_GLOBAL(z_zero));

	do {
		PHALCON_CALL_METHOD(&ret, &background, "compositeImage", &im, &composite, &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));

		if (!zend_is_true(&ret)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Imagick::background failed");
			zval_ptr_dtor(&background);
			return;
		}

		PHALCON_CALL_METHOD(&next, &im, "nextImage");
	} while (zend_is_true(&next));

	phalcon_update_property(getThis(), SL("_image"), &background);
	zval_ptr_dtor(&background);
}

/**
 * Blur image
 *
 * @param int $radius Blur radius
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _blur){

	zval *radius, im = {}, sigma = {}, next = {};

	phalcon_fetch_params(0, 1, 0, &radius);

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);

	ZVAL_DOUBLE(&sigma, 100);

	PHALCON_CALL_METHOD(NULL, &im, "setIteratorIndex", &PHALCON_GLOBAL(z_zero));

	do {
		PHALCON_CALL_METHOD(NULL, &im, "blurImage", radius, &sigma);
		PHALCON_CALL_METHOD(&next, &im, "nextImage");
	} while (zend_is_true(&next));
}

/**
 * Pixelate image
 *
 * @param int $amount amount to pixelate
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _pixelate){

	zval *amount, im = {}, width = {}, height = {}, tmp_width = {}, tmp_height = {}, next = {};
	int w, h;

	phalcon_fetch_params(0, 1, 0, &amount);

	if (Z_LVAL_P(amount) < 2) {
		ZVAL_LONG(amount, 2);
	}

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);
	phalcon_read_property(&width, getThis(), SL("_width"), PH_READONLY);
	phalcon_read_property(&height, getThis(), SL("_height"), PH_READONLY);

	w = (int)((phalcon_get_intval(&width) / Z_LVAL_P(amount)) + 0.5);
	h = (int)((phalcon_get_intval(&height) / Z_LVAL_P(amount)) + 0.5);

	ZVAL_LONG(&tmp_width, w);
	ZVAL_LONG(&tmp_height, h);

	PHALCON_CALL_METHOD(NULL, &im, "setIteratorIndex", &PHALCON_GLOBAL(z_zero));

	do {
		PHALCON_CALL_METHOD(NULL, &im, "scaleImage", &tmp_width, &tmp_height);
		PHALCON_CALL_METHOD(NULL, &im, "scaleImage", &width, &height);
		PHALCON_CALL_METHOD(&next, &im, "nextImage");
	} while (zend_is_true(&next));
}

/**
 * Execute a save.
 *
 * @param string $file
 * @param int $quality
 * @return boolean
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _save) {

	zval *file, *quality = NULL, *interlacing = NULL, *constant, ret = {}, format = {}, type = {}, mime = {}, im = {}, fp = {}, mode = {}, compression = {};
	zval interlace_scheme = {};
	zend_class_entry *imagick_ce;
	char *ext;

	phalcon_fetch_params(0, 1, 2, &file, &quality, &interlacing);

	imagick_ce = phalcon_fetch_str_class(SL("Imagick"), ZEND_FETCH_CLASS_AUTO);

	if ((constant = zend_get_constant_str(SL("PATHINFO_EXTENSION"))) == NULL) {
		return;
	}

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);

	PHALCON_CALL_FUNCTION(&ret, "pathinfo", file, constant);

	if (PHALCON_IS_NOT_EMPTY(&ret)) {
		phalcon_fast_strtolower(&format, &ret);

		ext = Z_STRVAL(format);

		if (strcmp(ext, "gif") == 0) {
			ZVAL_LONG(&type, 1);
		} else if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0) {
			ZVAL_LONG(&type, 2);
		} else if (strcmp(ext, "png") == 0) {
			ZVAL_LONG(&type, 3);
		} else {
			zend_throw_exception_ex(phalcon_image_exception_ce, 0, "Installed ImageMagick does not support '%s' images", ext);
			return;
		}

		PHALCON_CALL_METHOD(NULL, &im, "setformat", &format);
		PHALCON_CALL_METHOD(NULL, &im, "setImageFormat", &format);

		PHALCON_CONCAT_SV(&mime, "image/", &format);

		phalcon_update_property(getThis(), SL("_type"), &type);
		phalcon_update_property(getThis(), SL("_mime"), &mime);
		zval_ptr_dtor(&mime);
	} else {
		phalcon_read_property(&type, getThis(), SL("_type"), PH_READONLY);
	}

	PHALCON_CALL_METHOD(NULL, &im, "stripImage");

	if (phalcon_get_intval(&type) == 1) {
		PHALCON_CALL_METHOD(NULL, &im, "optimizeImageLayers");

		ZVAL_STRING(&mode, "w");

		PHALCON_CALL_FUNCTION(&fp, "fopen", file, &mode);
		if (Z_TYPE(fp) != IS_RESOURCE) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "The image file cannot be saved");
			return;
		}

		PHALCON_CALL_METHOD(&ret, &im, "writeImagesFile", &fp);
		PHALCON_CALL_FUNCTION(NULL, "fclose", &fp);
	} else {
		if (phalcon_get_intval(&type) == 2) {
			phalcon_get_class_constant(&compression, imagick_ce, SL("COMPRESSION_JPEG"));
			if (Z_TYPE(compression) == IS_LONG) {
				PHALCON_CALL_METHOD(NULL, &im, "setImageCompression", &compression);
			}
		}

		if (quality && Z_TYPE_P(quality) != IS_NULL) {
			PHALCON_CALL_METHOD(NULL, &im, "setImageCompressionQuality", quality);
		}
		if (interlacing && Z_TYPE_P(interlacing) >= IS_NULL) {
			if (zend_is_true(interlacing)) {
				phalcon_get_class_constant(&interlace_scheme, imagick_ce, SL("INTERLACE_LINE"));
			} else {
				phalcon_get_class_constant(&interlace_scheme, imagick_ce, SL("INTERLACE_NO"));
			}
			PHALCON_CALL_METHOD(NULL, &im, "setinterlacescheme", &interlace_scheme);
		}
		PHALCON_CALL_METHOD(&ret, &im, "writeImage", file);
	}

	if (zend_is_true(&ret)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Execute a render.
 *
 * @param string $type
 * @param int $quality
 * @return string
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, _render) {

	zval *extension, *quality = NULL, *interlacing = NULL, mime = {}, format = {}, type = {}, im = {}, image_string = {}, compression = {};
	zval interlace_scheme = {};
	zend_class_entry *imagick_ce;
	char *ext;

	phalcon_fetch_params(0, 1, 2, &extension, &quality, &interlacing);

	imagick_ce = phalcon_fetch_str_class(SL("Imagick"), ZEND_FETCH_CLASS_AUTO);

	phalcon_fast_strtolower(&format, extension);

	ext = Z_STRVAL(format);

	if (strncmp(ext, "gif", 3) == 0) {
		ZVAL_LONG(&type, 1);
	} else if (strncmp(ext, "jpg", 3) == 0 || strncmp(ext, "jpeg", 4) == 0) {
		ZVAL_LONG(&type, 2);
	} else if (strncmp(ext, "png", 3) == 0) {
		ZVAL_LONG(&type, 3);
	} else {
		phalcon_read_property(&type, getThis(), SL("_type"), PH_READONLY);
	}

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);

	if (zend_is_true(&type)) {
		PHALCON_CALL_METHOD(NULL, &im, "setformat", &format);
		PHALCON_CALL_METHOD(NULL, &im, "setImageFormat", &format);

		phalcon_update_property(getThis(), SL("_type"), &type);

		PHALCON_CONCAT_SV(&mime, "image/", &format);

		phalcon_update_property(getThis(), SL("_mime"), &mime);
		zval_ptr_dtor(&mime);
	}
	zval_ptr_dtor(&format);

	PHALCON_CALL_METHOD(NULL, &im, "stripImage");

	if (phalcon_get_intval(&type) == 1) {
		PHALCON_CALL_METHOD(NULL, &im, "optimizeImageLayers");
		PHALCON_CALL_METHOD(&image_string, &im, "getImagesBlob");
	} else {
		if (phalcon_get_intval(&type) == 2) {
			phalcon_get_class_constant(&compression, imagick_ce, SL("COMPRESSION_JPEG"));
			if (Z_TYPE(compression) == IS_LONG) {
				PHALCON_CALL_METHOD(NULL, &im, "setImageCompression", &compression);
			}
		}
		if (quality && Z_TYPE_P(quality) != IS_NULL) {
			PHALCON_CALL_METHOD(NULL, &im, "setImageCompressionQuality", quality);
		}
		if (interlacing && Z_TYPE_P(interlacing) >= IS_NULL) {
			if (zend_is_true(interlacing)) {
				phalcon_get_class_constant(&interlace_scheme, imagick_ce, SL("INTERLACE_LINE"));
			} else {
				phalcon_get_class_constant(&interlace_scheme, imagick_ce, SL("INTERLACE_NO"));
			}
			PHALCON_CALL_METHOD(NULL, &im, "setinterlacescheme", &interlace_scheme);
		}
		PHALCON_CALL_METHOD(&image_string, &im, "getImageBlob");
	}

	RETVAL_ZVAL(&image_string, 0, 0);
}

/**
 * Destroys the loaded image to free up resources.
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, __destruct){

	zval im = {};

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);

	if (Z_TYPE(im) == IS_OBJECT) {
		PHALCON_CALL_METHOD(NULL, &im, "destroy");
	}
}

/**
 * Draws a line
 *
 * @param int $sx
 * @param int $sy
 * @param int $ex
 * @param int $ey
 * @param string $color
 * @return Phalcon\Image\Adapter\Imagick
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, line){

	zval *sx, *sy, *ex, *ey, *color = NULL, image = {}, draw = {}, pixel = {};
	zend_class_entry *imagick_draw_ce, *imagick_pixel_ce;

	phalcon_fetch_params(0, 4, 1, &sx, &sy, &ex, &ey, &color);

	if (!color) {
		color = &PHALCON_GLOBAL(z_null);
	}
	phalcon_read_property(&image, getThis(), SL("_image"), PH_READONLY);

	if (Z_TYPE(image) == IS_RESOURCE) {
		imagick_draw_ce  = phalcon_fetch_str_class(SL("ImagickDraw"), ZEND_FETCH_CLASS_AUTO);
		imagick_pixel_ce = phalcon_fetch_str_class(SL("ImagickPixel"), ZEND_FETCH_CLASS_AUTO);

		object_init_ex(&draw, imagick_draw_ce);
		PHALCON_CALL_METHOD(NULL, &draw, "__construct");

		object_init_ex(&pixel, imagick_pixel_ce);
		PHALCON_CALL_METHOD(NULL, &pixel, "__construct", color);
		PHALCON_CALL_METHOD(NULL, &draw, "setstrokecolor", &pixel);
		zval_ptr_dtor(&pixel);
		PHALCON_CALL_METHOD(NULL, &draw, "line", sx, sy, ex, ey);

		PHALCON_CALL_METHOD(NULL, &image, "drawimage", &draw);

		zval_ptr_dtor(&draw);
	}
	RETURN_THIS();
}

/**
 * Draws a polygon
 *
 *<code>
 * $coordinates = array( array( 'x' => 4, 'y' => 6 ), array( 'x' => 8, 'y' => 10 ) );
 * $image->polygon($coordinates);
 *</code>
 *
 * @param array $coordinates array of x and y
 * @param string $color
 * @return Phalcon\Image\Adapter\Imagick
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, polygon){

	zval *coordinates, *color = NULL, image = {}, draw = {}, pixel = {}, *point, points = {};
	zend_class_entry *imagick_draw_ce, *imagick_pixel_ce;

	phalcon_fetch_params(0, 1, 1, &coordinates, &color);

	if (!color) {
		color = &PHALCON_GLOBAL(z_null);
	}

	if (!phalcon_fast_count_ev(coordinates)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Coordinates must be not empty");
		return;
	}
	phalcon_read_property(&image, getThis(), SL("_image"), PH_READONLY);

	if (Z_TYPE(image) == IS_RESOURCE) {
		imagick_draw_ce  = phalcon_fetch_str_class(SL("ImagickDraw"), ZEND_FETCH_CLASS_AUTO);
		imagick_pixel_ce = phalcon_fetch_str_class(SL("ImagickPixel"), ZEND_FETCH_CLASS_AUTO);

		object_init_ex(&draw, imagick_draw_ce);
		PHALCON_CALL_METHOD(NULL, &draw, "__construct");

		object_init_ex(&pixel, imagick_pixel_ce);
		PHALCON_CALL_METHOD(NULL, &pixel, "__construct", color);

		PHALCON_CALL_METHOD(NULL, &draw, "setstrokecolor", &pixel);
		PHALCON_CALL_METHOD(NULL, &draw, "setfillcolor", &pixel);
		zval_ptr_dtor(&pixel);

		array_init(&points);
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(coordinates), point) {
			zval newpoint = {}, x = {}, y = {};
			array_init(&newpoint);
			if (Z_TYPE_P(point) == IS_ARRAY) {
				if (phalcon_fast_count_int(point) != 2) {
					PHALCON_THROW_EXCEPTION_STR(phalcon_image_exception_ce, "Coordinates point error");
					return;
				}
				if (!phalcon_array_isset_fetch_long(&x, point, 0, PH_READONLY)) {
					phalcon_array_fetch_str(&x, point, SL("x"), PH_NOISY|PH_READONLY);
				}
				if (!phalcon_array_isset_fetch_long(&y, point, 0, PH_READONLY)) {
					phalcon_array_fetch_str(&y, point, SL("y"), PH_NOISY|PH_READONLY);
				}
			} else {
				ZVAL_COPY_VALUE(&x, &_p->val);
				_p++;
				ZVAL_COPY_VALUE(&y, &_p->val);
			}
			phalcon_array_update_str(&newpoint, SL("x"), &x, PH_COPY);
			phalcon_array_update_str(&newpoint, SL("y"), &y, PH_COPY);
			phalcon_array_append(&points, &newpoint, 0);
		} ZEND_HASH_FOREACH_END();

		PHALCON_CALL_METHOD(NULL, &draw, "polygon", &points);
		zval_ptr_dtor(&points);

		PHALCON_CALL_METHOD(NULL, &image, "drawimage", &draw);
		zval_ptr_dtor(&draw);
	}
	RETURN_THIS();
}

PHP_METHOD(Phalcon_Image_Adapter_Imagick, shadow)
{
	zval *_color = NULL, *_opacity = NULL, *_sigma = NULL, *_x = NULL, *_y = NULL, color = {}, opacity = {}, sigma = {}, x = {}, y = {}, im = {}, version = {}, shadow = {}, imagickpixel = {}, composite = {};
	zend_class_entry *imagickpixel_ce, *imagick_ce;

	phalcon_fetch_params(0, 0, 5, &color, &opacity, &_sigma, &_x, &_y);

	imagickpixel_ce = phalcon_fetch_str_class(SL("ImagickPixel"), ZEND_FETCH_CLASS_AUTO);
	imagick_ce = phalcon_fetch_str_class(SL("Imagick"), ZEND_FETCH_CLASS_AUTO);

	if (!_color || Z_TYPE_P(_color) == IS_NULL) {
		ZVAL_STRING(&color, "black");
	} else {
		ZVAL_COPY(&color, _color);
	}

	if (!_opacity || Z_TYPE_P(_opacity) == IS_NULL) {
		ZVAL_LONG(&opacity, 80);
	} else {
		ZVAL_COPY_VALUE(&opacity, _opacity);
	}

	if (!_sigma || Z_TYPE_P(_sigma) == IS_NULL) {
		ZVAL_LONG(&sigma, 3);
	} else {
		ZVAL_COPY_VALUE(&sigma, _sigma);
	}

	if (!_x || Z_TYPE_P(_x) == IS_NULL) {
		ZVAL_LONG(&x, 5);
	} else {
		ZVAL_COPY_VALUE(&x, _x);
	}

	if (!_y || Z_TYPE_P(_y) == IS_NULL) {
		ZVAL_LONG(&y, 5);
	} else {
		ZVAL_COPY_VALUE(&y, _y);
	}

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);
	phalcon_read_static_property_ce(&version, phalcon_image_adapter_imagick_ce, SL("_version"), PH_READONLY);

	if (phalcon_get_intval(&version) >= 30100) {
		if (FAILURE == phalcon_clone(&shadow, &im)) {
			return;
		}
	} else {
		PHALCON_CALL_METHOD(&shadow, &im, "clone");
	}

	object_init_ex(&imagickpixel, imagickpixel_ce);
	PHALCON_CALL_METHOD(NULL, &imagickpixel, "__construct", &color);
	zval_ptr_dtor(&color);
	PHALCON_CALL_METHOD(NULL, &shadow, "setimagebackgroundcolor", &imagickpixel);
	zval_ptr_dtor(&imagickpixel);
	PHALCON_CALL_METHOD(NULL, &shadow, "shadowimage", &opacity, &sigma, &x, &y);

	phalcon_get_class_constant(&composite, imagick_ce, SL("COMPOSITE_OVER"));
	PHALCON_CALL_METHOD(NULL, &shadow, "compositeimage", &im, &composite, &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));
	PHALCON_CALL_METHOD(NULL, &im, "clear");
	PHALCON_CALL_METHOD(NULL, &im, "destroy");

	phalcon_update_property(getThis(), SL("_image"), &shadow);
	zval_ptr_dtor(&shadow);

	RETURN_THIS();
}

PHP_METHOD(Phalcon_Image_Adapter_Imagick, getInternalImInstance)
{
	RETURN_MEMBER(getThis(), "_image");
}

PHP_METHOD(Phalcon_Image_Adapter_Imagick, setResourceLimit)
{
	zval *resource, *limit, im = {};
	zend_class_entry *imagick_ce;

	phalcon_fetch_params(0, 2, 0, &resource, &limit);

	imagick_ce = phalcon_fetch_str_class(SL("Imagick"), ZEND_FETCH_CLASS_AUTO);
	if (EXPECTED(imagick_ce != NULL)) {
		zval *params[2] = { resource, limit };

		object_init_ex(&im, imagick_ce);
		if (FAILURE == phalcon_call_method(NULL, &im, "setresourcelimit", 2, params)) {
			;
		}
		zval_ptr_dtor(&im);
	}
}

/**
 * Replicate Colorize function
 *
 * @param string $color a hex or rgb(a) color
 * @param int $composition use imagicks constants here
 * @return Phalcon\Image\Adapter\Imagick
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, colorize)
{
	zval *color, *_composition = NULL, composition = {}, pseudo_string = {}, im = {}, width = {}, height = {}, overlay = {};
	zend_class_entry *imagick_ce;

	phalcon_fetch_params(0, 1, 1, &color, &_composition);

	imagick_ce = phalcon_fetch_str_class(SL("Imagick"), ZEND_FETCH_CLASS_AUTO);

	if (!_composition || Z_TYPE_P(_composition) == IS_NULL) {
		phalcon_get_class_constant(&composition, imagick_ce, SL("COMPOSITE_MULTIPLY"));
	} else {
		ZVAL_COPY_VALUE(&composition, _composition);
	}

	PHALCON_CONCAT_SV(&pseudo_string, "canvas:", color);

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);
	phalcon_read_property(&width, getThis(), SL("_width"), PH_READONLY);
	phalcon_read_property(&height, getThis(), SL("_height"), PH_READONLY);

	object_init_ex(&overlay, imagick_ce);
	if (phalcon_has_constructor(&overlay)) {
		PHALCON_CALL_METHOD(NULL, &overlay, "__construct");
	}

	PHALCON_CALL_METHOD(NULL, &overlay, "newpseudoimage", &width, &height, &pseudo_string);
	zval_ptr_dtor(&pseudo_string);
	PHALCON_CALL_METHOD(NULL, &im, "compositeimage", &overlay, &composition, &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));
	PHALCON_CALL_METHOD(NULL, &overlay, "clear");
	PHALCON_CALL_METHOD(NULL, &overlay, "destroy");
	zval_ptr_dtor(&overlay);

	RETURN_THIS();
}

/**
 * Change the gamma of an image
 *
 * @param float $gamma normally between 0.8 and 2
 * @param int $channel Use the Imagick constants for this
 * @return Phalcon\Image\Adapter\Imagick
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, gamma)
{
	zval *gamma, *_channel = NULL, channel = {}, im = {};
	zend_class_entry *imagick_ce;

	phalcon_fetch_params(0, 1, 1, &gamma, &_channel);

	imagick_ce = phalcon_fetch_str_class(SL("Imagick"), ZEND_FETCH_CLASS_AUTO);

	if (!_channel || Z_TYPE_P(_channel) == IS_NULL) {
		phalcon_get_class_constant(&channel, imagick_ce, SL("CHANNEL_ALL"));
	} else {
		ZVAL_COPY_VALUE(&channel, _channel);
	}

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);

	PHALCON_CALL_METHOD(NULL, &im, "gammaimage", gamma, &channel);

	RETURN_THIS();
}

/**
 * Replicate Photoshop's levels function
 *
 * @param float $gamma
 * @param int $input_min between 0 and 255, same as photoshops
 * @param int $input_max between 0 and 255, same as photoshops
 * @param int $output_min between 0 and 255, same as photoshops
 * @param int $output_max between 0 and 255, same as photoshops
 * @param int $channel use imagemagicks constants
 * @return Phalcon\Image\Adapter\Imagick
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, levels)
{
	zval *_gamma = NULL, *_input_min = NULL, *_input_max = NULL, *_output_min = NULL, *_output_max = NULL, *_channel = NULL, gamma = {}, input_min = {};
	zval input_max = {}, output_min = {}, output_max = {}, channel = {}, im = {}, range = {}, quantum_range_long = {};
	zend_class_entry *imagick_ce;
	long tmp_input_min, tmp_input_max, tmp_output_min, tmp_output_max;

	phalcon_fetch_params(0, 0, 6, &_gamma, &_input_min, &_input_max, &_output_min, &_output_max, &_channel);

	imagick_ce = phalcon_fetch_str_class(SL("Imagick"), ZEND_FETCH_CLASS_AUTO);

	if (!_gamma || Z_TYPE_P(_gamma) == IS_NULL) {
		ZVAL_LONG(&gamma, 1);
	} else {
		ZVAL_COPY_VALUE(&gamma, _gamma);
	}

	if (!_input_min || Z_TYPE_P(_input_min) == IS_NULL) {
		ZVAL_LONG(&input_min, 0);
	} else {
		ZVAL_COPY_VALUE(&input_min, _input_min);
	}

	if (!_input_max || Z_TYPE_P(_input_max) == IS_NULL) {
		ZVAL_LONG(&input_max, 255);
	} else {
		ZVAL_COPY_VALUE(&input_max, _input_max);
	}

	if (!_output_min || Z_TYPE_P(_output_min) == IS_NULL) {
		ZVAL_LONG(&output_min, 0);
	} else {
		ZVAL_COPY_VALUE(&output_min, _output_min);
	}

	if (!_output_max || Z_TYPE_P(_output_max) == IS_NULL) {
		ZVAL_LONG(&output_max, 255);
	} else {
		ZVAL_COPY_VALUE(&output_max, _output_max);
	}

	if (!_channel || Z_TYPE_P(_channel) == IS_NULL) {
		phalcon_get_class_constant(&channel, imagick_ce, SL("CHANNEL_ALL"));
	} else {
		ZVAL_COPY_VALUE(&channel, _channel);
	}

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);

	PHALCON_CALL_METHOD(&range, &im, "getquantumrange");

	if (Z_TYPE(range) == IS_ARRAY &&
		phalcon_array_isset_fetch_str(&quantum_range_long, &range, SL("quantumRangeLong"), PH_READONLY)) {
		tmp_input_min = round(phalcon_get_intval(&input_min) * phalcon_get_intval(&quantum_range_long) / 255);
		tmp_input_max = round(phalcon_get_intval(&input_max) * phalcon_get_intval(&quantum_range_long) / 255);
		tmp_output_min = round(phalcon_get_intval(&output_min) * phalcon_get_intval(&quantum_range_long) / 255);
		tmp_output_max = round(phalcon_get_intval(&output_max) * phalcon_get_intval(&quantum_range_long) / 255);

		ZVAL_LONG(&input_min, tmp_input_min);
		ZVAL_LONG(&input_max, tmp_input_max);
		ZVAL_LONG(&output_min,-tmp_output_min);
		ZVAL_LONG(&output_max, phalcon_get_intval(&quantum_range_long) * 2 - tmp_output_max);

		PHALCON_CALL_METHOD(NULL, &im, "levelimage", &input_min, &gamma, &input_max, &channel);

		ZVAL_DOUBLE(&gamma, 1.0);

		PHALCON_CALL_METHOD(NULL, &im, "levelimage", &output_min, &gamma, &output_max, &channel);
	}
	zval_ptr_dtor(&range);

	RETURN_THIS();
}

/**
 * Replicate brightness/contrast photoshop function
 *
 * Now this one is a bit of a pain. PHP's extension doesn't provide us with this handle (yet?)
 * So we have to save the image to disk at this point, perform the function using the command line, and reload the image. yay.
 *
 * @param int $brightness this is should be -150 <= brightnes <= 150. 0 for no change.
 * @param int $contrast this is should be -150 <= contrast <= 150. 0 for no change.
 * @return Phalcon\Image\Adapter\Imagick
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, brightness_contrast)
{
	zval *brightness, *contrast, brightness_normalised = {}, contrast_normalised = {}, overlay = {}, columns = {}, rows = {}, pseudo_string = {}, background = {}, degrees = {}, alpha = {}, im = {};
	zend_class_entry *imagick_ce;
	double tmp_brightness, tmp_contrast;

	phalcon_fetch_params(0, 2, 0, &brightness, &contrast);

	imagick_ce = phalcon_fetch_str_class(SL("Imagick"), ZEND_FETCH_CLASS_AUTO);

	tmp_brightness = labs(phalcon_get_intval(brightness)) * 5 / 150.0;
	tmp_contrast = labs(phalcon_get_intval(contrast)) * 5 / 150.0;

	ZVAL_DOUBLE(&brightness_normalised, tmp_brightness);
	ZVAL_DOUBLE(&contrast_normalised, tmp_contrast);

	if (tmp_brightness != 0 || tmp_contrast != 0) {
		object_init_ex(&overlay, imagick_ce);
		if (phalcon_has_constructor(&overlay)) {
			PHALCON_CALL_METHOD(NULL, &overlay, "__construct");
		}

		ZVAL_LONG(&columns, 1);
		ZVAL_LONG(&rows, 1000);
		ZVAL_STRING(&pseudo_string, "gradient:");

		PHALCON_CALL_METHOD(NULL, &overlay, "newpseudoimage", &columns, &rows, &pseudo_string);
		zval_ptr_dtor(&pseudo_string);

		ZVAL_STRING(&background, "#fff");
		ZVAL_LONG(&degrees, 90);

		PHALCON_CALL_METHOD(NULL, &overlay, "rotateimage", &background, &degrees);
		zval_ptr_dtor(&background);

		if (tmp_contrast != 0) {
			ZVAL_LONG(&alpha, 50);

			PHALCON_CALL_METHOD(NULL, &overlay, "sigmoidalcontrastimage", phalcon_get_intval(contrast) > 0 ? &PHALCON_GLOBAL(z_true) : &PHALCON_GLOBAL(z_false), &contrast_normalised, &alpha);
		}

		if (tmp_brightness != 0) {
			ZVAL_LONG(&alpha, 0);

			PHALCON_CALL_METHOD(NULL, &overlay, "sigmoidalcontrastimage", phalcon_get_intval(brightness) > 0 ? &PHALCON_GLOBAL(z_true) : &PHALCON_GLOBAL(z_false), &brightness_normalised, &alpha);
		}

		phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);

		PHALCON_CALL_METHOD(NULL, &im, "clutimage", &overlay);

		PHALCON_CALL_METHOD(NULL, &overlay, "clear");
		PHALCON_CALL_METHOD(NULL, &overlay, "destroy");
		zval_ptr_dtor(&overlay);
	}

	RETURN_THIS();
}

/**
 * Replicate HSL function
 *
 * Imagemagick calls this 'modulate
 *
 * @param int $hue -100 <= hue <= 100. 0 is no change.
 * @param int $saturation -100 <= hue <= 100. 0 is no change.
 * @param int $lightness -100 <= hue <= 100. 0 is no change.
 * @return Phalcon\Image\Adapter\Imagick
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, hsl)
{
	zval *_hue = NULL, *_saturation = NULL, *_lightness = NULL, hue = {}, saturation = {}, lightness = {}, im = {};
	long tmp_hue, tmp_saturation, tmp_lightness;

	phalcon_fetch_params(0, 0, 3, &hue, &saturation, &lightness);

	if (!_hue) {
		ZVAL_LONG(&hue, 100);
	} else {
		tmp_hue = phalcon_get_intval(_hue) + 100;
		ZVAL_LONG(&hue, tmp_hue);
	}

	if (!_saturation) {
		ZVAL_LONG(&saturation, 100);
	} else {
		tmp_saturation = phalcon_get_intval(_saturation) + 100;
		ZVAL_LONG(&saturation, tmp_saturation);
	}

	if (!_lightness) {
		ZVAL_LONG(&lightness, 100);
	} else {
		tmp_lightness = phalcon_get_intval(_lightness) + 100;
		ZVAL_LONG(&lightness, tmp_lightness);
	}

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);

	PHALCON_CALL_METHOD(NULL, &im, "modulateImage", &lightness, &saturation, &hue);

	RETURN_THIS();
}

/**
 * Perform an imagemagick-style function on each pixel
 *
 * @param string $fx the function
 * @return Phalcon\Image\Adapter\Imagick
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, curves_graph)
{
	zval *fx, im = {};

	phalcon_fetch_params(0, 1, 0, &fx);

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);

	PHALCON_CALL_METHOD(NULL, &im, "fxImage", fx);

	RETURN_THIS();
}

/**
 * Adds a vignette to the image
 *
 * @param string $color the colour of the vignette
 * @param int $composition an imagick constant defining the composition to use
 * @param float $crop_factor defines the strenth of the vignette
 * @return Phalcon\Image\Adapter\Imagick
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, vignette)
{
	zval *color, *_composition = NULL, *_crop_factor = NULL, composition = {}, crop_factor = {}, im = {}, width = {}, height = {}, x1 = {}, y1 = {}, x2 = {}, y2 = {}, pseudo_string = {}, overlay = {};
	zend_class_entry *imagick_ce;

	phalcon_fetch_params(0, 1, 2, &color, &composition, &crop_factor);

	imagick_ce = phalcon_fetch_str_class(SL("Imagick"), ZEND_FETCH_CLASS_AUTO);

	if (!_composition || Z_TYPE_P(_composition) == IS_NULL) {
		phalcon_get_class_constant(&composition, imagick_ce, SL("COMPOSITE_DEFAULT"));
	} else {
		ZVAL_COPY_VALUE(&composition, _composition);
	}

	if (!_crop_factor || Z_TYPE_P(_crop_factor) == IS_NULL) {
		ZVAL_DOUBLE(&crop_factor, 1.5);
	} else {
		ZVAL_COPY_VALUE(&crop_factor, _crop_factor);
	}

	phalcon_read_property(&im, getThis(), SL("_image"), PH_READONLY);
	phalcon_read_property(&width, getThis(), SL("_width"), PH_READONLY);
	phalcon_read_property(&height, getThis(), SL("_height"), PH_READONLY);

	ZVAL_LONG(&x1, floor(phalcon_get_doubleval(&height) * phalcon_get_doubleval(&crop_factor)));
	ZVAL_LONG(&y1, floor(phalcon_get_doubleval(&width) * phalcon_get_doubleval(&crop_factor)));

	PHALCON_CONCAT_SV(&pseudo_string, "radial-gradient:rgba(0,0,0,0)-", color);

	object_init_ex(&overlay, imagick_ce);
	if (phalcon_has_constructor(&overlay)) {
		PHALCON_CALL_METHOD(NULL, &overlay, "__construct");
	}

	PHALCON_CALL_METHOD(NULL, &overlay, "newpseudoimage", &x1, &y1, &pseudo_string);
	zval_ptr_dtor(&pseudo_string);

	ZVAL_DOUBLE(&x2, (phalcon_get_doubleval(&width) - phalcon_get_doubleval(&x1)) / 2.0);
	ZVAL_DOUBLE(&y2, (phalcon_get_doubleval(&height) - phalcon_get_doubleval(&y1)) / 2.0);

	PHALCON_CALL_METHOD(NULL, &im, "compositeImage", &overlay, &composition, &x2, &y2);
	PHALCON_CALL_METHOD(NULL, &overlay, "clear");
	PHALCON_CALL_METHOD(NULL, &overlay, "destroy");
	zval_ptr_dtor(&overlay);

	RETURN_THIS();
}

/**
 * A sort-of sepia filter
 *
 * @return Phalcon\Image\Adapter\Imagick
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, earlybird)
{
	zval tmp0 = {}, tmp1 = {}, tmp2 = {}, tmp3 = {};
	zend_class_entry *imagick_ce;

	imagick_ce = phalcon_fetch_str_class(SL("Imagick"), ZEND_FETCH_CLASS_AUTO);

	ZVAL_LONG(&tmp0, -32);
	ZVAL_LONG(&tmp1, 1);

	PHALCON_CALL_SELF(NULL, "hsl", &PHALCON_GLOBAL(z_zero), &tmp0, &tmp1);

	ZVAL_DOUBLE(&tmp0, 1.19);

	PHALCON_CALL_SELF(NULL, "gamma", &tmp0);

	ZVAL_LONG(&tmp0, 1);
	ZVAL_LONG(&tmp1, 255);
	ZVAL_LONG(&tmp2, 27);

	phalcon_get_class_constant(&tmp3, imagick_ce, SL("CHANNEL_RED"));

	PHALCON_CALL_SELF(NULL, "levels", &tmp0, &PHALCON_GLOBAL(z_zero), &tmp1, &tmp2, &tmp1, &tmp3);

	ZVAL_LONG(&tmp0, 15);
	ZVAL_LONG(&tmp1, 36);

	PHALCON_CALL_SELF(NULL, "brightness_contrast", &tmp0, &tmp1);

	PHALCON_CALL_SELF(NULL, "hsl", &PHALCON_GLOBAL(z_zero), &tmp0, &PHALCON_GLOBAL(z_zero));

	ZVAL_DOUBLE(&tmp0, 0.92);
	ZVAL_LONG(&tmp1, 235);

	PHALCON_CALL_SELF(NULL, "levels", &tmp0, &PHALCON_GLOBAL(z_zero), &tmp1);

	ZVAL_STRING(&tmp0, "rgb(251,243,220)");

	PHALCON_CALL_SELF(NULL, "colorize", &tmp0);
	zval_ptr_dtor(&tmp0);

	ZVAL_STRING(&tmp0, "rgb(184,184,184)");

	phalcon_get_class_constant(&tmp1, imagick_ce, SL("COMPOSITE_COLORBURN"));

	PHALCON_CALL_SELF(NULL, "vignette", &tmp0, &tmp1);
	zval_ptr_dtor(&tmp0);

	ZVAL_STRING(&tmp0, "rgb(251,243,220)");

	phalcon_get_class_constant(&tmp1, imagick_ce, SL("COMPOSITE_MULTIPLY"));

	PHALCON_CALL_SELF(NULL, "vignette", &tmp0, &tmp1);
	zval_ptr_dtor(&tmp0);

	RETURN_THIS();
}

/**
 * A black and white filter
 *
 * @return Phalcon\Image\Adapter\Imagick
 */
PHP_METHOD(Phalcon_Image_Adapter_Imagick, inkwell)
{
	zval tmp0 = {}, tmp1 = {};

	ZVAL_LONG(&tmp0, -100);

	PHALCON_CALL_SELF(NULL, "hsl", &PHALCON_GLOBAL(z_zero), &tmp0, &PHALCON_GLOBAL(z_zero));

	ZVAL_STRING(&tmp0, "-0.062*u^3-0.104*u^2+1.601*u-0.175");

	PHALCON_CALL_SELF(NULL, "curves_graph", &tmp0);
	zval_ptr_dtor(&tmp0);

	ZVAL_LONG(&tmp0, -10);
	ZVAL_LONG(&tmp1, 48);

	PHALCON_CALL_SELF(NULL, "brightness_contrast", &tmp0, &tmp1);

	RETURN_THIS();
}

PHP_METHOD(Phalcon_Image_Adapter_Imagick, convert)
{
#ifdef PHALCON_USE_MAGICKWAND
	zval *command, command_parts = {}, *value;
	char **argv;
	int argc, i = 0;


	ExceptionInfo *exception = NULL;
	ImageInfo *image_info = NULL;
	MagickBooleanType status;

	phalcon_fetch_params(0, 1, 0, &command);

	if (Z_TYPE_P(command) != IS_ARRAY) {
		phalcon_fast_explode_str(&command_parts, SL(" "), command);
	} else {
		ZVAL_COPY(&command_parts, command);
	}

	argc = zend_hash_num_elements(Z_ARRVAL(command_parts));

	argv = emalloc(sizeof(char*) * argc);

	HashTable *ht;
	if (Z_TYPE_P(command) != IS_ARRAY) {
		ht = Z_ARRVAL(command_parts);
	} else {
		ht = Z_ARRVAL_P(command);
	}

	ZEND_HASH_FOREACH_VAL(ht, value) {
		convert_to_string(value);
		argv[i] = estrdup(Z_STRVAL_P(value));
		i++;
	} ZEND_HASH_FOREACH_END();

	MagickCoreGenesis(*argv, MagickTrue);
	exception = AcquireExceptionInfo();
	image_info = AcquireImageInfo();

	status = MagickCommandGenesis(image_info, ConvertImageCommand, argc, argv, (char **)NULL, exception);

	image_info = DestroyImageInfo(image_info);
	exception = DestroyExceptionInfo(exception);

	MagickCoreTerminus();

	for (i = 0; i < argc; i++) {
		efree(argv[i]);
	}

	efree(argv);
	zval_ptr_dtor(&command_parts);

	if (status == 0) {
		RETURN_TRUE;
	}

#endif

	RETURN_FALSE;
}
