
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


#include "image/adapter/gd.h"
#include "image.h"
#include "image/adapter.h"
#include "image/adapterinterface.h"
#include "image/exception.h"

#include <ext/standard/php_versioning.h>

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
 * Phalcon\Image\Adapter\GD
 *
 * Image manipulation support. Allows images to be resized, cropped, etc.
 *
 *<code>
 *	$image = new Phalcon\Image\Adapter\GD("upload/test.jpg");
 *	$image->resize(200, 200)->rotate(90)->crop(100, 100);
 *	if ($image->save()) {
 *		echo 'success';
 *	}
 *</code>
 */
zend_class_entry *phalcon_image_adapter_gd_ce;

PHP_METHOD(Phalcon_Image_Adapter_GD, check);
PHP_METHOD(Phalcon_Image_Adapter_GD, __construct);
PHP_METHOD(Phalcon_Image_Adapter_GD, _resize);
PHP_METHOD(Phalcon_Image_Adapter_GD, _liquidRescale);
PHP_METHOD(Phalcon_Image_Adapter_GD, _crop);
PHP_METHOD(Phalcon_Image_Adapter_GD, _rotate);
PHP_METHOD(Phalcon_Image_Adapter_GD, _flip);
PHP_METHOD(Phalcon_Image_Adapter_GD, _sharpen);
PHP_METHOD(Phalcon_Image_Adapter_GD, _reflection);
PHP_METHOD(Phalcon_Image_Adapter_GD, _watermark);
PHP_METHOD(Phalcon_Image_Adapter_GD, _text);
PHP_METHOD(Phalcon_Image_Adapter_GD, _mask);
PHP_METHOD(Phalcon_Image_Adapter_GD, _background);
PHP_METHOD(Phalcon_Image_Adapter_GD, _blur);
PHP_METHOD(Phalcon_Image_Adapter_GD, _pixelate);
PHP_METHOD(Phalcon_Image_Adapter_GD, _save);
PHP_METHOD(Phalcon_Image_Adapter_GD, _render);
PHP_METHOD(Phalcon_Image_Adapter_GD, _create);
PHP_METHOD(Phalcon_Image_Adapter_GD, __destruct);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter_gd___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, file)
	ZEND_ARG_INFO(0, width)
	ZEND_ARG_INFO(0, height)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter_gd__create, 0, 0, 2)
	ZEND_ARG_INFO(0, width)
	ZEND_ARG_INFO(0, height)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_image_adapter_gd_method_entry[] = {
	PHP_ME(Phalcon_Image_Adapter_GD, check, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Image_Adapter_GD, __construct, arginfo_phalcon_image_adapter_gd___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Image_Adapter_GD, _resize, arginfo_phalcon_image_adapter__resize, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_GD, _liquidRescale, arginfo_phalcon_image_adapter__liquidrescale, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_GD, _crop, arginfo_phalcon_image_adapter__crop, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_GD, _rotate, arginfo_phalcon_image_adapter__rotate, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_GD, _flip, arginfo_phalcon_image_adapter__flip, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_GD, _sharpen, arginfo_phalcon_image_adapter__sharpen, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_GD, _reflection, arginfo_phalcon_image_adapter__reflection, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_GD, _watermark, arginfo_phalcon_image_adapter__watermark, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_GD, _text, arginfo_phalcon_image_adapter__text, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_GD, _mask, arginfo_phalcon_image_adapter__mask, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_GD, _background, arginfo_phalcon_image_adapter__background, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_GD, _blur, arginfo_phalcon_image_adapter__blur, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_GD, _pixelate, arginfo_phalcon_image_adapter__pixelate, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_GD, _save, arginfo_phalcon_image_adapter__save, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_GD, _render, arginfo_phalcon_image_adapter__render, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_GD, _create, arginfo_phalcon_image_adapter_gd__create, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Image_Adapter_GD, __destruct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_DTOR)
	PHP_FE_END
};

/**
 * Phalcon\Image\Adapter\GD initializer
 */
PHALCON_INIT_CLASS(Phalcon_Image_Adapter_GD){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Image\\Adapter, GD, image_adapter_gd, phalcon_image_adapter_ce,  phalcon_image_adapter_gd_method_entry, 0);

	zend_class_implements(phalcon_image_adapter_gd_ce, 1, phalcon_image_adapterinterface_ce);

	return SUCCESS;
}

/**
 * Checks if GD is enabled
 *
 * @return  boolean
 */
PHP_METHOD(Phalcon_Image_Adapter_GD, check){

	zval gd_version, ret = NULL, gd_info, version, exception_message, pattern, matches;

	if (phalcon_function_exists_ex(SL("gd_info")) == FAILURE) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_image_exception_ce, "GD is either not installed or not enabled, check your configuration");
		return;
	}

	if (!phalcon_get_constant(&gd_version, SL("GD_VERSION"))) {
		PHALCON_CALL_FUNCTIONW(&gd_info, "gd_info");

		if (phalcon_array_isset_fetch_str(&gd_version, &gd_info, SL("GD Version"))) {
			ZVAL_STRING(&pattern, "#\\d+\\.\\d+(?:\\.\\d+)?#");
			RETURN_ON_FAILURE(phalcon_preg_match(&ret, &pattern, &gd_version, &matches));

			if (zend_is_true(&ret)) {
				if (!phalcon_array_isset_fetch_long(&version, &matches, 0)) {
					ZVAL_EMPTY_STRING(&version);
				}
			} else {
				ZVAL_EMPTY_STRING(&version);
			}
		} else {
			ZVAL_COPY_VALUE(&version, &gd_version);
		}
	}

	if (-1 == php_version_compare(Z_STRVAL_P(&gd_version), "2.0.1")) {
		PHALCON_CONCAT_SV(&exception_message, "Phalcon\\Image\\Adapter\\GD requires GD version '2.0.1' or greater, you have '", &gd_version);
		PHALCON_THROW_EXCEPTION_ZVALW(phalcon_image_exception_ce, &exception_message);
		return;
	}

	phalcon_update_static_property_ce(phalcon_image_adapter_gd_ce, SL("_checked"), &PHALCON_GLOBAL(z_true));

	RETURN_TRUE;
}

/**
 * Phalcon\Image\GD constructor
 *
 * @param string $file
 */
PHP_METHOD(Phalcon_Image_Adapter_GD, __construct){

	zval *file, *width = NULL, *height = NULL, exception_message, checked, realpath, img_width, img_height, type, mime, format, image, imageinfo, saveflag, blendmode;

	phalcon_fetch_params(0, 1, 2, &file, &width, &height);

	if (Z_TYPE_P(file) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_image_exception_ce, "file parameter should be a string");
		return;
	}

	phalcon_return_static_property_ce(&checked, phalcon_image_adapter_gd_ce, SL("_checked"));

	if (!zend_is_true(&checked)) {
		PHALCON_CALL_CE_STATICW(NULL, phalcon_image_adapter_gd_ce, "check");
	}

	phalcon_update_property_this(getThis(), SL("_file"), file);

	if (phalcon_file_exists(file) != FAILURE) {
		phalcon_file_realpath(&realpath, file);
		if (unlikely(Z_TYPE(realpath) != IS_STRING)) {
			convert_to_string(&realpath);
		}

		phalcon_update_property_this(getThis(), SL("_realpath"), &realpath);

		PHALCON_CALL_FUNCTIONW(&imageinfo, "getimagesize", realpath);

		if (phalcon_array_isset_fetch_long(&img_width, &imageinfo, 0)) {
			phalcon_update_property_this(getThis(), SL("_width"), &img_width);
		}

		if (phalcon_array_isset_fetch_long(&img_height, &imageinfo, 1)) {
			phalcon_update_property_this(getThis(), SL("_height"), &img_height);
		}

		if (phalcon_array_isset_fetch_long(&type, &imageinfo, 2)) {
			convert_to_long(&type);
			phalcon_update_property_this(getThis(), SL("_type"), &type);
		} else {
			ZVAL_LONG(&type, -1);
		}

		if (phalcon_array_isset_fetch_str(&mime, &imageinfo, SL("mime"))) {
			convert_to_string(&mime);
			phalcon_update_property_this(getThis(), SL("_mime"), &mime);
		}

		assert(Z_TYPE(type) == IS_LONG);

		switch (Z_LVAL(type)) {
			case 1: // GIF
				ZVAL_STRING(&format, "gif");
				PHALCON_CALL_FUNCTIONW(&image, "imagecreatefromgif", &realpath);
				break;

			case 2: // JPEG
				ZVAL_STRING(&format, "jpg");
				PHALCON_CALL_FUNCTIONW(&image, "imagecreatefromjpeg", &realpath);
				break;

			case 3: // PNG
				ZVAL_STRING(&format, "png");
				PHALCON_CALL_FUNCTIONW(&image, "imagecreatefrompng", &realpath);
				break;

			default:
				if (PHALCON_IS_NOT_EMPTY(&mime)) {
					zend_throw_exception_ex(phalcon_image_exception_ce, 0, "Installed GD does not support '%s' images", Z_STRVAL(mime));
				} else {
					zend_throw_exception_ex(phalcon_image_exception_ce, 0, "Installed GD does not support such images");
				}
				return;
		}

		if (Z_TYPE(image) != IS_RESOURCE) {
			assert(Z_TYPE(realpath) == IS_STRING);
			zend_throw_exception_ex(phalcon_image_exception_ce, 0, "Failed to create image from file '%s'", Z_STRVAL(realpath));
			return;
		}

		phalcon_update_property_this(getThis(), SL("_format"), &format);

		ZVAL_TRUE(&saveflag);

		PHALCON_CALL_FUNCTIONW(NULL, "imagesavealpha", image, &saveflag);
	} else if (width && height) {
		PHALCON_CALL_FUNCTIONW(&image, "imagecreatetruecolor", width, height);

		if (Z_TYPE(image) != IS_RESOURCE) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_image_exception_ce, "imagecreatetruecolor() failed");
			return;
		}

		ZVAL_TRUE(&blendmode);
		ZVAL_TRUE(&saveflag);

		PHALCON_CALL_FUNCTIONW(NULL, "imagealphablending", &image, &blendmode);
		PHALCON_CALL_FUNCTIONW(NULL, "imagesavealpha", &image, &saveflag);
		
		phalcon_update_property_this(getThis(), SL("_realpath"), file);
		phalcon_update_property_this(getThis(), SL("_width"), width);
		phalcon_update_property_this(getThis(), SL("_height"), height);

		ZVAL_LONG(&type, 3);

		phalcon_update_property_this(getThis(), SL("_type"), &type);

		ZVAL_STRING(&format, "png");

		phalcon_update_property_this(getThis(), SL("_format"), &format);

		ZVAL_STRING(&mime, "image/png");

		phalcon_update_property_this(getThis(), SL("_mime"), &mime);
	} else {
		PHALCON_CONCAT_SVS(&exception_message, "Failed to create image from file '", file, "'");
		PHALCON_THROW_EXCEPTION_ZVALW(phalcon_image_exception_ce, &exception_message);
		return;
	}

	phalcon_update_property_this(getThis(), SL("_image"), &image);
}

/**
 * Execute a resize.
 *
 * @param int $width
 * @param int $height
 */
PHP_METHOD(Phalcon_Image_Adapter_GD, _resize) {

	zval *width, *height, image, tmp_image;

	phalcon_fetch_params(0, 2, 0, &width, &height);

	phalcon_return_property(&image, getThis(), SL("_image"));

	PHALCON_CALL_FUNCTIONW(&tmp_image, "imagescale", &image, width, height);

	if (Z_TYPE(tmp_image) == IS_RESOURCE) {
		PHALCON_CALL_FUNCTIONW(NULL, "imagedestroy", &image);
		phalcon_update_property_this(getThis(), SL("_image"), &tmp_image);

		phalcon_update_property_this(getThis(), SL("_width"), width);
		phalcon_update_property_this(getThis(), SL("_height"), height);
	}
}

/**
 * This method scales the images using liquid rescaling method. Only support Imagick
 *
 * @param int $width   new width
 * @param int $height  new height
 * @param int $delta_x How much the seam can traverse on x-axis. Passing 0 causes the seams to be straight. 
 * @param int $rigidity Introduces a bias for non-straight seams. This parameter is typically 0.
 * @return Phalcon\Image\Adapter
 */
PHP_METHOD(Phalcon_Image_Adapter_GD, _liquidRescale){

	zend_throw_exception_ex(phalcon_image_exception_ce, 0, "The GD does not support liquidRescale");
}

/**
 * Execute a crop.
 *
 * @param int $width
 * @param int $height
 * @param int $offset_x
 * @param int $offset_y
 */
PHP_METHOD(Phalcon_Image_Adapter_GD, _crop)
{
	zval *width, *height, *offset_x, *offset_y, image, tmp_image, rect;

	phalcon_fetch_params(0, 4, 0, &width, &height, &offset_x, &offset_y);

	phalcon_return_property(&image, getThis(), SL("_image"));

	array_init_size(&rect, 4);
	phalcon_array_update_str(&rect, SL("x"), offset_x, PH_COPY);
	phalcon_array_update_str(&rect, SL("y"), offset_y, PH_COPY);
	phalcon_array_update_str(&rect, SL("width"), width, PH_COPY);
	phalcon_array_update_str(&rect, SL("height"), height, PH_COPY);

	PHALCON_CALL_FUNCTIONW(&tmp_image, "imagecrop", &image, &rect);

	if (Z_TYPE(tmp_image) == IS_RESOURCE) {
		PHALCON_CALL_FUNCTIONW(NULL, "imagedestroy", &image);
		phalcon_update_property_this(getThis(), SL("_image"), &tmp_image);
		phalcon_update_property_this(getThis(), SL("_width"), width);
		phalcon_update_property_this(getThis(), SL("_height"), height);
	}
}

/**
 * Execute a rotation.
 *
 * @param int $degrees
 */
PHP_METHOD(Phalcon_Image_Adapter_GD, _rotate) {

	zval *_degrees, degrees, image, tmp_image, color, alpha, transparent, ignore_transparent, saveflag, w, h;
	int tmp_degrees;

	phalcon_fetch_params(0, 1, 0, &_degrees);
	ZVAL_COPY_VALUE(&degrees, _degrees);

	ZVAL_LONG(&color, 0);
	ZVAL_LONG(&alpha, 127);

	phalcon_return_property(&image, getThis(), SL("_image"));

	PHALCON_CALL_FUNCTIONW(&transparent, "imagecolorallocatealpha", &image, &color, &color, &color, &alpha);

	tmp_degrees = phalcon_get_intval(&degrees);

	ZVAL_LONG(&degrees, 360 - tmp_degrees);
	ZVAL_LONG(&ignore_transparent, 1);

	PHALCON_CALL_FUNCTIONW(&tmp_image, "imagerotate", &image, &degrees, &transparent, &ignore_transparent);

	ZVAL_TRUE(&saveflag);

	PHALCON_CALL_FUNCTIONW(NULL, "imagesavealpha", &tmp_image, &saveflag);
	PHALCON_CALL_FUNCTIONW(&w, "imagesx", &tmp_image);
	PHALCON_CALL_FUNCTIONW(&h, "imagesy", &tmp_image);
	PHALCON_CALL_FUNCTIONW(NULL, "imagedestroy", &image);
	phalcon_update_property_this(getThis(), SL("_image"), &tmp_image);
	phalcon_update_property_this(getThis(), SL("_width"), &w);
	phalcon_update_property_this(getThis(), SL("_height"), &h);
}

/**
 * Execute a flip.
 *
 * @param int $direction
 */
PHP_METHOD(Phalcon_Image_Adapter_GD, _flip) {

	zval *direction, image, mode;

	phalcon_fetch_params(0, 1, 0, &direction);

	phalcon_return_property(&image, getThis(), SL("_image"));

	if (Z_LVAL_P(direction) == PHALCON_IMAGE_HORIZONTAL) {
		if (!phalcon_get_constant(&mode, SL("IMG_FLIP_HORIZONTAL"))) {
			return;
		}
	} else {
		if (!phalcon_get_constant(&mode, SL("IMG_FLIP_VERTICAL"))) {
			return;
		}
	}

	PHALCON_CALL_FUNCTIONW(NULL, "imageflip", &image, &mode);
}

/**
 * Execute a sharpen.
 *
 * @param int $amount
 */
PHP_METHOD(Phalcon_Image_Adapter_GD, _sharpen) {

	zval *amount, tmp, tmp_amount, matrix, item, image, ret, width, height;
	int a;
	double b;

	phalcon_fetch_params(0, 1, 0, &amount);

	phalcon_return_property(&image, getThis(), SL("_image"));

	a = phalcon_get_intval(amount);

	if (a > 100) {
		a = 100;
	} else if (a < 1) {
		a = 1;
	}

	b = -18 + (a * 0.08);

	if (b < 0) {
		b = -b;
	}

	ZVAL_LONG(&tmp_amount, (long int)(floor(a*100.0+0.5)/100));

	array_init_size(&matrix, 3);

	/* 1 */
	array_init_size(&item, 3);
	phalcon_array_append_long(&item, -1, PH_COPY);
	phalcon_array_append_long(&item, -1, PH_COPY);
	phalcon_array_append_long(&item, -1, PH_COPY);

	phalcon_array_append(&matrix, &item, PH_COPY);

	/* 2 */
	array_init_size(&item, 3);
	phalcon_array_append_long(&item, -1, PH_COPY);
	phalcon_array_append(&item, &tmp_amount, PH_COPY);
	phalcon_array_append_long(&item, -1, PH_COPY);

	phalcon_array_append(&matrix, &item, PH_COPY);

	/* 3 */
	array_init_size(&item, 3);
	phalcon_array_append_long(&item, -1, PH_COPY);
	phalcon_array_append_long(&item, -1, PH_COPY);
	phalcon_array_append_long(&item, -1, PH_COPY);

	phalcon_array_append(&matrix, &item, PH_COPY);

	b = b - 8;

	ZVAL_DOUBLE(&tmp_amount, b);
	ZVAL_LONG(&tmp, 0);

	PHALCON_CALL_FUNCTIONW(&ret, "imageconvolution", &image, &matrix, &tmp_amount, &tmp);

	if (zend_is_true(ret)) {
		PHALCON_CALL_FUNCTIONW(&width, "imagesx", &image);
		PHALCON_CALL_FUNCTIONW(&height, "imagesy", &image);

		phalcon_update_property_this(getThis(), SL("_width"), &width);
		phalcon_update_property_this(getThis(), SL("_height"), &height);
	}
}

/**
 * Execute a reflection.
 *
 * @param int $height
 * @param int $opacity
 * @param boolean $fade_in
 */
PHP_METHOD(Phalcon_Image_Adapter_GD, _reflection) {

	zval *_height, *opacity, *fade_in, height, tmp, reflection , line, image, image_width, image_height, dst, dst_opacity, filtertype;
	int h0, h1, tmp_opacity, int_opacity, offset;
	double stepping;

	phalcon_fetch_params(0, 3, 0, &_height, &opacity, &fade_in);

	ZVAL_COPY_VALUE(&height, _height);

	phalcon_return_property(&image, getThis(), SL("_image"));
	phalcon_return_property(&image_width, getThis(), SL("_width"));
	phalcon_return_property(&image_height, getThis(), SL("_height"));

	if (!phalcon_get_constant(&filtertype, SL("IMG_FILTER_COLORIZE")))) {
		return
	}

	h0 = phalcon_get_intval(&height);
	h1 = phalcon_get_intval(&image_height);

	if (unlikely(h0 == 0)) {
		h0 = 1;
	}

	tmp_opacity = phalcon_get_intval(&opacity);

	tmp_opacity = (int)((tmp_opacity * 127 / 100) - 127 + 0.5);

	if (tmp_opacity < 0) {
		tmp_opacity = -tmp_opacity;
	}

	if (tmp_opacity < 127) {
		stepping = (127 - tmp_opacity) / h0;
	} else {
		stepping = 127 / h0;
	}

	ZVAL_DOUBLE(&height, h0 + h1);

	PHALCON_CALL_METHODW(&reflection, getThis(), "_create", &image_width, &height);

	ZVAL_LONG(&dst, 0);

	PHALCON_CALL_FUNCTIONW(NULL, "imagecopy", &reflection, &image, &dst, &dst, &dst, &dst, &image_width, &image_height);

	ZVAL_LONG(&tmp, 1);

	for (offset = 0; h0 >= offset; offset++) {
		zval src_y, dst_y, dst_opacity;
		ZVAL_LONG(&src_y, h1 - offset - 1);
		ZVAL_LONG(&dst_y, h1 + offset);

		if (zend_is_true(&fade_in)) {
			int_opacity = (int)(tmp_opacity + (stepping * (h0 - offset)) + 0.5);
			ZVAL_LONG(&dst_opacity, int_opacity);
		} else {
			int_opacity = (int)(tmp_opacity + (stepping * offset) + 0.5);
			ZVAL_LONG(&dst_opacity, int_opacity);
		}

		PHALCON_CALL_METHODW(&line, getThis(), "_create", &image_width, &tmp);
		PHALCON_CALL_FUNCTIONW(NULL, "imagecopy", &line, &image, &dst, &dst, &dst, &src_y, &image_width, &tmp);
		PHALCON_CALL_FUNCTIONW(NULL, "imagefilter", &line, &filtertype, &dst, &dst, &dst, &dst_opacity);
		PHALCON_CALL_FUNCTIONW(NULL, "imagecopy", &reflection, &line, &dst, &dst_y, &dst, &dst, &image_width, &tmp);
	}

	PHALCON_CALL_FUNCTIONW(NULL, "imagedestroy", &image);
	phalcon_update_property_this(getThis(), SL("_image"), &reflection);

	PHALCON_CALL_FUNCTIONW(&image_width, "imagesx", &reflection);
	PHALCON_CALL_FUNCTIONW(&image_height, "imagesy", &reflection);

	phalcon_update_property_this(getThis(), SL("_width"), &image_width);
	phalcon_update_property_this(getThis(), SL("_height"), &image_height);
}

/**
 * Execute a watermarking.
 *
 * @param Phalcon\Image\Adapter $watermark
 * @param int $offset_x
 * @param int $offset_y
 * @param int $opacity
 */
PHP_METHOD(Phalcon_Image_Adapter_GD, _watermark) {

	zval *watermark, *offset_x = NULL, *offset_y = NULL, *opacity = NULL, op, image, overlay, blob, saveflag, width, height, color, tmp, effect, blendmode, ret;
	int int_opacity;
	double num;

	phalcon_fetch_params(0, 4, 0, &watermark, &offset_x, &offset_y, &opacity);

	phalcon_return_property(&image, getThis(), SL("_image"));

	PHALCON_CALL_METHODW(&blob, watermark, "render");
	PHALCON_CALL_FUNCTIONW(&overlay, "imagecreatefromstring", &blob);

	ZVAL_TRUE(&saveflag);

	PHALCON_CALL_FUNCTIONW(NULL, "imagesavealpha", &overlay, &saveflag);
	PHALCON_CALL_FUNCTIONW(&width, "imagesx", &overlay);
	PHALCON_CALL_FUNCTIONW(&height, "imagesy", &overlay);

	int_opacity = Z_LVAL_P(opacity);
	if (int_opacity < 100) {
		num = (int_opacity * 127.0 / 100) - 127;

		if (num < 0) {
			num = -num;
		}

		int_opacity = (int)num;

		ZVAL_LONG(&op, int_opacity);
		ZVAL_LONG(&tmp, 127);

		PHALCON_CALL_FUNCTIONW(&color, "imagecolorallocatealpha", &overlay, &tmp, &tmp, &tmp, &op);

		if (!phalcon_get_constant(&effect, SL("IMG_EFFECT_OVERLAY"))) {
			return;
		}

		PHALCON_CALL_FUNCTIONW(NULL, "imagelayereffect", &overlay, &effect);

		ZVAL_LONG(&tmp, 0);

		PHALCON_CALL_FUNCTIONW(NULL, "imagefilledrectangle", &overlay, &tmp, &tmp, &width, &height, &color);
	}

	ZVAL_LONG(&blendmode, 1);
	PHALCON_CALL_FUNCTIONW(NULL, "imagealphablending", &image, &blendmode);

	ZVAL_LONG(tmp, 0);

	PHALCON_CALL_FUNCTIONW(&ret, "imagecopy", &image, &overlay, offset_x, offset_y, &tmp, &tmp, &width, &height);

	RETVAL_BOOL(zend_is_true(ret));
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
PHP_METHOD(Phalcon_Image_Adapter_GD, _text) {
	zval *text, *offset_x, *offset_y, *opacity, *r, *g, *b, *size, *fontfile = NULL;
	zval image, image_width, image_height, tmp, space,  s0, s1, s4, s5, width, height, color;
	int w, h, w1, h1, x, y, i;

	phalcon_fetch_params(0, 9, 0, &text, &offset_x, &offset_y, &opacity, &r, &g, &b, &size, &fontfile);
	PHALCON_SEPARATE_PARAM(offset_x);
	PHALCON_SEPARATE_PARAM(offset_y);
	PHALCON_SEPARATE_PARAM(opacity);

	phalcon_return_property(&image, getThis(), SL("_image"));
	phalcon_return_property(&image_width, getThis(), SL("_width"));
	phalcon_return_property(&image_height, getThis(), SL("_height"));
	
	w = phalcon_get_intval(&image_width);
	h = phalcon_get_intval(&image_height);

	i = Z_LVAL_P(opacity);

	i = (int)((i * 127 / 100) - 127);

	if (i < 0) {
		i *= -1;
	}

	ZVAL_LONG(opacity, i);
	ZVAL_LONG(&tmp, 0);

	if (Z_TYPE_P(fontfile) == IS_STRING) {
		PHALCON_CALL_FUNCTIONW(&space, "imagettfbbox", size, &tmp, fontfile, text);

		if (
			!phalcon_array_isset_fetch_long(&s0, &space, 0) || 
			!phalcon_array_isset_fetch_long(&s1, &space, 1) || 
			!phalcon_array_isset_fetch_long(&s4, &space, 4) || 
			!phalcon_array_isset_fetch_long(&s5, &space, 5)
		) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_image_exception_ce, "Call to imagettfbbox() failed");
			return;
		}

		w1 = phalcon_get_intval(&s4) - phalcon_get_intval(&s0);
		if (w1 < 0) {
			w1 = -w1;
		}
		w1 += 10;

		ZVAL_LONG(&width, w1);

		h1 = phalcon_get_intval(&s5) - phalcon_get_intval(&s1);
		if (h1 < 0) {
			h1 = -h1;
		}
		h1 += 10;

		ZVAL_LONG(&height, h1);

		if (Z_TYPE_P(offset_x) == IS_LONG ) {
			x = phalcon_get_intval(offset_x);
			if (x < 0) {
				x = (int)(w - w1 + x + 0.5);
			}
		} else if (zend_is_true(offset_x)) {
			x = (int)(w - w1);
		} else {
			x = (int)(((w - w1) / 2) + 0.5);
		}

		ZVAL_LONG(offset_x, x);

		if (Z_TYPE_P(offset_y) == IS_LONG ) {
			y = phalcon_get_intval(offset_y);
			if (y < 0) {
				y = (int)(h - h1 + y + 0.5);
			}
		} else if (zend_is_true(offset_y)) {
			y = (int)(h - h1);
		} else {
			y = (int)(((h - h1) / 2) + 0.5);
		}

		ZVAL_LONG(offset_y, y);

		PHALCON_CALL_FUNCTIONW(&color, "imagecolorallocatealpha", &image, r, g, b, opacity);
		PHALCON_CALL_FUNCTIONW(NULL, "imagettftext", &image, size, &tmp, offset_x, offset_y, &color, fontfile, text);
	} else {
		PHALCON_CALL_FUNCTIONW(&width, "imagefontwidth", size);
		PHALCON_CALL_FUNCTIONW(&height, "imagefontheight", size);

		i = Z_STRLEN_P(text);

		w1 =  phalcon_get_intval(&width) * i;
		h1 =  phalcon_get_intval(&height);

		ZVAL_LONG(&width, w1);
		ZVAL_LONG(&height, h1);

		if (Z_TYPE_P(offset_x) == IS_LONG ) {
			x = phalcon_get_intval(offset_x);
			if (x < 0) {
				x = (int)(w - w1 + x);
			}
		} else if (zend_is_true(offset_x)) {
			x = (int)(w - w1);
		} else {
			x = (int)((w - w1) / 2);
		}

		ZVAL_LONG(offset_x, x);

		if (Z_TYPE_P(offset_y) == IS_LONG ) {
			y = phalcon_get_intval(offset_y);
			if (y < 0) {
				y = (int)(h - h1 + y);
			}
		} else if (zend_is_true(offset_y)) {
			y = (int)(h - h1);
		} else {
			y = (int)((h - h1) / 2);
		}

		ZVAL_LONG(offset_y, y);

		PHALCON_CALL_FUNCTIONW(&color, "imagecolorallocatealpha", &image, r, g, b, opacity);
		PHALCON_CALL_FUNCTIONW(NULL, "imagestring", &image, size, offset_x, offset_y, text, &color);
	}
}

/**
 * Composite one image onto another

 *
 * @param Phalcon\Image\Adapter $mask  mask Image instance
 */
PHP_METHOD(Phalcon_Image_Adapter_GD, _mask){

	zval *mask, image, mask_image, blob, mask_image_width, mask_image_height, newimage, image_width, image_height, saveflag, color, c, alpha, temp_image;
	int x, y, w, h, i;

	phalcon_fetch_params(0, 1, 0, &mask);

	phalcon_return_property(&image, getThis(), SL("_image"));

	PHALCON_CALL_METHODW(&blob, mask, "render");
	PHALCON_CALL_FUNCTIONW(&mask_image, "imagecreatefromstring", &blob);

	ZVAL_TRUE(&saveflag);

	PHALCON_CALL_FUNCTIONW(NULL, "imagesavealpha", &mask_image, &saveflag);

	PHALCON_CALL_FUNCTIONW(&mask_image_width, "imagesx", &mask_image);
	PHALCON_CALL_FUNCTIONW(&mask_image_height, "imagesy", &mask_image);

	phalcon_return_property(&image_width, getThis(), SL("_width"));
	phalcon_return_property(&image_height, getThis(), SL("_height"));

	PHALCON_CALL_METHODW(&newimage, getThis(), "_create", &image_width, &image_height);

	PHALCON_CALL_FUNCTIONW(NULL, "imagesavealpha", &newimage, &saveflag);

	ZVAL_LONG(&c, 0);
	ZVAL_LONG(&alpha, 127);

	PHALCON_CALL_FUNCTIONW(&color, "imagecolorallocatealpha", &newimage, &c, &c, &c, &alpha);
	PHALCON_CALL_FUNCTIONW(NULL, "imagefill", &newimage, &c, &c, &color);

	if(!PHALCON_IS_EQUAL(&image_width, &mask_image_width) || !PHALCON_IS_EQUAL(&image_height, &mask_image_height)) {
		PHALCON_CALL_FUNCTIONW(&temp_image, "imagecreatetruecolor", &image_width, &image_height);
		PHALCON_CALL_FUNCTIONW(NULL, "imagecopyresampled", &temp_image, &mask_image, &c, &c, &c, &c, &image_width, &image_height, &mask_image_width, &mask_image_height);
		PHALCON_CALL_FUNCTIONW(NULL, "imagedestroy", &mask_image);

		ZVAL_COPY_VALUE(&mask_image, &temp_image);
	}
	
	w = phalcon_get_intval(&image_width);
	h = phalcon_get_intval(&image_height);

	for (x=0; x < w; x++) {
		zval zx;
		ZVAL_LONG(&zx, x);
		for (y=0; y < h; y++) {
			zval zy, index, red, index2, color, r, g, b;
			ZVAL_LONG(&zy, y);

			PHALCON_CALL_FUNCTIONW(&index, "imagecolorat", &mask_image, &zx, &zy);
			PHALCON_CALL_FUNCTIONW(&alpha, "imagecolorsforindex", &mask_image, &index);

			if (phalcon_array_isset_fetch_str(&red, &alpha, SL("red"))) {
				i = (int)(127 - (phalcon_get_intval(&red) / 2));
				ZVAL_LONG(&alpha, i);
			}

			PHALCON_CALL_FUNCTIONW(&index2, "imagecolorat", &image, &zx, &zy);
			PHALCON_CALL_FUNCTIONW(&color, "imagecolorsforindex", &image, &index2);

			phalcon_array_isset_fetch_str(&r, &color, SL("red"));
			phalcon_array_isset_fetch_str(&g, &color, SL("green"));
			phalcon_array_isset_fetch_str(&b, &color, SL("blue"));

			PHALCON_CALL_FUNCTIONW(&color, "imagecolorallocatealpha", &newimage, &r, &g, &b, &alpha);
			PHALCON_CALL_FUNCTIONW(NULL, "imagesetpixel", &newimage, &zx, &zy, &color);
		}
	}

	PHALCON_CALL_FUNCTIONW(NULL, "imagedestroy", &image);
	PHALCON_CALL_FUNCTIONW(NULL, "imagedestroy", &mask_image);

	phalcon_update_property_this(getThis(), SL("_image"), &newimage);
}

/**
 * Execute a background.
 *
 * @param int $r
 * @param int $g
 * @param int $b
 * @param int $opacity
 */
PHP_METHOD(Phalcon_Image_Adapter_GD, _background) {

	zval *r, *g, *b, *opacity, op, image, background, width, height, color, tmp, blendmode, ret;
	int int_opacity;
	double num;

	phalcon_fetch_params(0, 4, 0, &r, &g, &b, &opacity);

	phalcon_return_property(&image, getThis(), SL("_image"));
	phalcon_return_property(&width, getThis(), SL("_width"));
	phalcon_return_property(&height, getThis(), SL("_height"));

	int_opacity = Z_LVAL_P(opacity);

	num = (int_opacity * 127.0 / 100) - 127;

	if (num < 0) {
		num = -num;
	}

	int_opacity = (int)num;

	PHALCON_CALL_METHODW(&background, getThis(), "_create", &width, &height);

	ZVAL_LONG(&op, int_opacity);

	PHALCON_CALL_FUNCTIONW(&color, "imagecolorallocatealpha", &background, r, g, b, &op);

	ZVAL_LONG(&tmp, 0);

	PHALCON_CALL_FUNCTIONW(NULL, "imagefilledrectangle", &background, &tmp, &tmp, &width, &height, &color);

	ZVAL_TRUE(&blendmode);

	PHALCON_CALL_FUNCTIONW(NULL, "imagealphablending", &background, &blendmode);
	PHALCON_CALL_FUNCTIONW(&ret, "imagecopy", &background, &image, &tmp, &tmp, &tmp, &tmp, &width, &height);

	if (zend_is_true(&ret)) {
		PHALCON_CALL_FUNCTIONW(NULL, "imagedestroy", &image);
		phalcon_update_property_this(getThis(), SL("_image"), &background);
	}
}

/**
 * Blur image
 *
 * @param int $radius Blur radius
 */
PHP_METHOD(Phalcon_Image_Adapter_GD, _blur){

	zval *radius, image, constant;
	int r, i;

	phalcon_fetch_params(0, 1, 0, &radius);	

	phalcon_return_property(&image, getThis(), SL("_image"));

	if (!phalcon_get_constant(&constant, SL("IMG_FILTER_GAUSSIAN_BLUR"))) {
		return;
	}

	r = phalcon_get_intval(radius);

	for (i = 0; i < r; i++) {
		PHALCON_CALL_FUNCTIONW(NULL, "imagefilter", &image, &constant);
	}
}

/**
 * Pixelate image
 *
 * @param int $amount amount to pixelate
 */
PHP_METHOD(Phalcon_Image_Adapter_GD, _pixelate){

	zval *amount, image, width, height, color;
	int a, x, y, x1, y1, w, h;

	phalcon_fetch_params(0, 1, 0, &amount);

	phalcon_return_property(&image, getThis(), SL("_image"));
	phalcon_return_property(&width, getThis(), SL("_width"));
	phalcon_return_property(&height, getThis(), SL("_height"));

	a = phalcon_get_intval(amount);
	w = phalcon_get_intval(&width);
	h = phalcon_get_intval(&height);

	for(x = 0; x < w; x += a) {
		for (y = 0; y < h; y += a) {
			zval tmp1, tmp2, tmp3, tmp4;
			x1 = (int)(x + a/2 + 0.5);
			y1 = (int)(y + a/2 + 0.5);

			ZVAL_LONG(&tmp1, x1)
			ZVAL_LONG(&tmp2, y1)

			PHALCON_CALL_FUNCTIONW(&color, "imagecolorat", &image, &tmp1, &tmp2);

			ZVAL_LONG(&tmp1, x)
			ZVAL_LONG(&tmp2, y)

			x1 = x + a;
			y1 = y + a;

			ZVAL_LONG(&tmp3, x1)
			ZVAL_LONG(&tmp4, y1)

			PHALCON_CALL_FUNCTIONW(NULL, "imagefilledrectangle", &image, &tmp1, &tmp2, &tmp3, &tmp4, &color);
		}
	}
}

/**
 * Execute a save.
 *
 * @param string $file
 * @param int $quality
 * @return boolean
 */
PHP_METHOD(Phalcon_Image_Adapter_GD, _save) {

	zval *file = NULL, *quality = NULL, q, ret, extension, type, mime, constant, image;
	const char *func_name = "imagegif";
	char *ext;

	phalcon_fetch_params(0, 2, 0, &file, &quality);

	if (!phalcon_get_constant(&constant, SL("PATHINFO_EXTENSION"))) {
		return;
	}

	PHALCON_CALL_FUNCTIONW(&ret, "pathinfo", file, &constant);

	phalcon_fast_strtolower(&extension, &ret);

	ext = Z_STRVAL(extension);

	phalcon_return_property(&image, getThis(), SL("_image"));

	if (strcmp(ext, "gif") == 0) {
		ZVAL_LONG(&type, 1);
		func_name = "imagegif";
	} else if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0) {
		ZVAL_LONG(&type, 2);
		ZVAL_COPY(&q, quality);
		func_name = "imagejpeg";
	} else if (strcmp(ext, "png") == 0) {
		ZVAL_LONG(&type, 3);
		func_name = "imagepng";
	} else {
		zend_throw_exception_ex(phalcon_image_exception_ce, 0, "Installed GD does not support '%s' images", Z_STRVAL(extension));
		return;
	}

	if (Z_TYPE(q) == IS_LONG) {
		PHALCON_CALL_FUNCTIONW(&ret, func_name, &image, file, &q);
	} else {
		PHALCON_CALL_FUNCTIONW(&ret, func_name, &image, file);
	}

	if (zend_is_true(ret)) {
		phalcon_update_property_this(getThis(), SL("_type"), &type);

		PHALCON_CALL_FUNCTIONW(&mime, "image_type_to_mime_type", &type);
		phalcon_update_property_this(getThis(), SL("_mime"), &mime);

		RETVAL_TRUE;
	} else {
		RETVAL_FALSE;
	}

	return;
}

/**
 * Execute a render.
 *
 * @param string $type
 * @param int $quality
 * @return string
 */
PHP_METHOD(Phalcon_Image_Adapter_GD, _render) {

	zval *extension = NULL, *quality = NULL, q, file, ret, type, mime, image;
	const char *func_name = "imagegif";
	char *ext;

	phalcon_fetch_params(0, 2, 0, &extension, &quality);

	phalcon_fast_strtolower(&ret, extension);

	ext = Z_STRVAL(ret);

	if (strcmp(ext, "gif") == 0) {
		ZVAL_LONG(&type, 1);
		func_name = "imagegif";
	} else if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0) {
		ZVAL_LONG(&type, 2);
		ZVAL_COPY(&q, quality);
		func_name = "imagejpeg";
	} else if (strcmp(ext, "png") == 0) {
		ZVAL_LONG(&type, 3);
		func_name = "imagepng";
	} else {
		zend_throw_exception_ex(phalcon_image_exception_ce, 0, "Installed GD does not support '%s' images", Z_STRVAL_P(extension));
		return;
	}
	
	phalcon_return_property(&image, getThis(), SL("_image"));

	phalcon_ob_start();

	if (Z_TYPE_P(q) == IS_LONG) {
		PHALCON_CALL_FUNCTIONW(&ret, func_name, &image, &file, &q);
	} else {
		PHALCON_CALL_FUNCTIONW(&ret, func_name, &image, &file);
	}

	phalcon_ob_get_contents(return_value);
	phalcon_ob_end_clean();

	if (zend_is_true(&ret)) {
		phalcon_update_property_this(getThis(), SL("_type"), &type);

		PHALCON_CALL_FUNCTIONW(&mime, "image_type_to_mime_type", &type);
		phalcon_update_property_this(getThis(), SL("_mime"), &mime);
	}

	return;
}

/**
 * Create an empty image with the given width and height.
 *
 * @param int $width
 * @param int $height
 * @return resource
 */
PHP_METHOD(Phalcon_Image_Adapter_GD, _create) {

	zval *width, *height, image, blendmode, saveflag;

	phalcon_fetch_params(0, 2, 0, &width, &height);

	PHALCON_CALL_FUNCTIONW(&image, "imagecreatetruecolor", width, height);

	if (Z_TYPE(image) != IS_RESOURCE) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_image_exception_ce, "imagecreatetruecolor() failed");
		return;
	}

	ZVAL_FALSE(&blendmode);
	ZVAL_TRUE(&saveflag);

	PHALCON_CALL_FUNCTIONW(NULL, "imagealphablending", &image, &blendmode);
	PHALCON_CALL_FUNCTIONW(NULL, "imagesavealpha", &image, &saveflag);

	RETURN_CTORW(&image);
}

/**
 * Destroys the loaded image to free up resources.
 */
PHP_METHOD(Phalcon_Image_Adapter_GD, __destruct){

	zval image;

	phalcon_return_property(&image, getThis(), SL("_image"));

	if (Z_TYPE_P(image) == IS_RESOURCE) {
		PHALCON_CALL_FUNCTIONW(NULL, "imagedestroy", &image);
	}
}
