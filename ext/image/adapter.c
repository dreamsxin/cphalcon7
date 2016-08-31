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

#include "image.h"
#include "image/adapter.h"
#include "image/adapterinterface.h"
#include "image/exception.h"

#include <ext/standard/php_math.h>
#include <Zend/zend_smart_str.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/array.h"
#include "kernel/file.h"

#include "internal/arginfo.h"

/**
 * Phalcon\Image\Adapter
 *
 * Base class for Phalcon\Image adapters
 */
zend_class_entry *phalcon_image_adapter_ce;

PHP_METHOD(Phalcon_Image_Adapter, getRealPath);
PHP_METHOD(Phalcon_Image_Adapter, getWidth);
PHP_METHOD(Phalcon_Image_Adapter, getHeight);
PHP_METHOD(Phalcon_Image_Adapter, getType);
PHP_METHOD(Phalcon_Image_Adapter, getMime);
PHP_METHOD(Phalcon_Image_Adapter, getImage);
PHP_METHOD(Phalcon_Image_Adapter, resize);
PHP_METHOD(Phalcon_Image_Adapter, liquidRescale);
PHP_METHOD(Phalcon_Image_Adapter, crop);
PHP_METHOD(Phalcon_Image_Adapter, rotate);
PHP_METHOD(Phalcon_Image_Adapter, flip);
PHP_METHOD(Phalcon_Image_Adapter, sharpen);
PHP_METHOD(Phalcon_Image_Adapter, reflection);
PHP_METHOD(Phalcon_Image_Adapter, watermark);
PHP_METHOD(Phalcon_Image_Adapter, text);
PHP_METHOD(Phalcon_Image_Adapter, mask);
PHP_METHOD(Phalcon_Image_Adapter, background);
PHP_METHOD(Phalcon_Image_Adapter, blur);
PHP_METHOD(Phalcon_Image_Adapter, pixelate);
PHP_METHOD(Phalcon_Image_Adapter, save);
PHP_METHOD(Phalcon_Image_Adapter, render);

static const zend_function_entry phalcon_image_adapter_method_entry[] = {
	PHP_ME(Phalcon_Image_Adapter, getRealPath,    arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter, getWidth,       arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter, getHeight,      arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter, getType,        arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter, getMime,        arginfo_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter, getImage,       arginfo_empty, ZEND_ACC_PUBLIC)

	PHP_ME(Phalcon_Image_Adapter, resize,         arginfo_phalcon_image_adapterinterface_resize,        ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter, liquidRescale,  arginfo_phalcon_image_adapterinterface_liquidrescale, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter, crop,           arginfo_phalcon_image_adapterinterface_crop,          ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter, rotate,         arginfo_phalcon_image_adapterinterface_rotate,        ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter, flip,           arginfo_phalcon_image_adapterinterface_flip,          ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter, sharpen,        arginfo_phalcon_image_adapterinterface_sharpen,       ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter, reflection,     arginfo_phalcon_image_adapterinterface_reflection,    ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter, watermark,      arginfo_phalcon_image_adapterinterface_watermark,     ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter, text,           arginfo_phalcon_image_adapterinterface_text,          ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter, mask,           arginfo_phalcon_image_adapterinterface_mask,          ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter, background,     arginfo_phalcon_image_adapterinterface_background,    ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter, blur,           arginfo_phalcon_image_adapterinterface_blur,          ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter, pixelate,       arginfo_phalcon_image_adapterinterface_pixelate,      ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter, save,           arginfo_phalcon_image_adapterinterface_save,          ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Image_Adapter, render,         arginfo_phalcon_image_adapterinterface_render,        ZEND_ACC_PUBLIC)

	ZEND_FENTRY(_resize,          NULL,           arginfo_phalcon_image_adapter__resize,        ZEND_ACC_PROTECTED | ZEND_ACC_ABSTRACT)
	ZEND_FENTRY(_liquidRescale,   NULL,           arginfo_phalcon_image_adapter__liquidrescale, ZEND_ACC_PROTECTED | ZEND_ACC_ABSTRACT)
	ZEND_FENTRY(_crop,            NULL,           arginfo_phalcon_image_adapter__crop,          ZEND_ACC_PROTECTED | ZEND_ACC_ABSTRACT)
	ZEND_FENTRY(_rotate,          NULL,           arginfo_phalcon_image_adapter__rotate,        ZEND_ACC_PROTECTED | ZEND_ACC_ABSTRACT)
	ZEND_FENTRY(_flip,            NULL,           arginfo_phalcon_image_adapter__flip,          ZEND_ACC_PROTECTED | ZEND_ACC_ABSTRACT)
	ZEND_FENTRY(_sharpen,         NULL,           arginfo_phalcon_image_adapter__sharpen,       ZEND_ACC_PROTECTED | ZEND_ACC_ABSTRACT)
	ZEND_FENTRY(_reflection,      NULL,           arginfo_phalcon_image_adapter__reflection,    ZEND_ACC_PROTECTED | ZEND_ACC_ABSTRACT)
	ZEND_FENTRY(_watermark,       NULL,           arginfo_phalcon_image_adapter__watermark,     ZEND_ACC_PROTECTED | ZEND_ACC_ABSTRACT)
	ZEND_FENTRY(_text,            NULL,           arginfo_phalcon_image_adapter__text,          ZEND_ACC_PROTECTED | ZEND_ACC_ABSTRACT)
	ZEND_FENTRY(_mask,            NULL,           arginfo_phalcon_image_adapter__mask,          ZEND_ACC_PROTECTED | ZEND_ACC_ABSTRACT)
	ZEND_FENTRY(_background,      NULL,           arginfo_phalcon_image_adapter__background,    ZEND_ACC_PROTECTED | ZEND_ACC_ABSTRACT)
	ZEND_FENTRY(_blur,            NULL,           arginfo_phalcon_image_adapter__blur,          ZEND_ACC_PROTECTED | ZEND_ACC_ABSTRACT)
	ZEND_FENTRY(_pixelate,        NULL,           arginfo_phalcon_image_adapter__pixelate,      ZEND_ACC_PROTECTED | ZEND_ACC_ABSTRACT)
	ZEND_FENTRY(_save,            NULL,           arginfo_phalcon_image_adapter__save,          ZEND_ACC_PROTECTED | ZEND_ACC_ABSTRACT)
	ZEND_FENTRY(_render,          NULL,           arginfo_phalcon_image_adapter__render,        ZEND_ACC_PROTECTED | ZEND_ACC_ABSTRACT)

	PHP_FE_END
};

/**
 * Phalcon\Image\Adapter initializer
 */
PHALCON_INIT_CLASS(Phalcon_Image_Adapter){

	PHALCON_REGISTER_CLASS(Phalcon\\Image, Adapter, image_adapter, phalcon_image_adapter_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_image_adapter_ce, SL("_image"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_image_adapter_ce, SL("_checked"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_image_adapter_ce, SL("_file"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_image_adapter_ce, SL("_realpath"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_image_adapter_ce, SL("_width"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_image_adapter_ce, SL("_height"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_image_adapter_ce, SL("_type"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_image_adapter_ce, SL("_format"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_image_adapter_ce, SL("_mime"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_image_adapter_ce, 1, phalcon_image_adapterinterface_ce);

	return SUCCESS;
}

/**
 * Returns the real path of the image file
 *
 * @return string
 */
PHP_METHOD(Phalcon_Image_Adapter, getRealPath){


	RETURN_MEMBER(getThis(), "_realpath");
}

/**
 * Returns the width of images
 *
 * @return int
 */
PHP_METHOD(Phalcon_Image_Adapter, getWidth){


	RETURN_MEMBER(getThis(), "_width");
}

/**
 * Returns the height of images
 *
 * @return int
 */
PHP_METHOD(Phalcon_Image_Adapter, getHeight){


	RETURN_MEMBER(getThis(), "_height");
}

/**
 * Returns the type of images
 *
 * @return int
 */
PHP_METHOD(Phalcon_Image_Adapter, getType){


	RETURN_MEMBER(getThis(), "_type");
}

/**
 * Returns the mime of images
 *
 * @return string
 */
PHP_METHOD(Phalcon_Image_Adapter, getMime){


	RETURN_MEMBER(getThis(), "_mime");
}

/**
 * Returns the image of images
 *
 * @return resource
 */
PHP_METHOD(Phalcon_Image_Adapter, getImage){


	RETURN_MEMBER(getThis(), "_image");
}

/**
 * Resize the image to the given size. Either the width or the height can
 * be omitted and the image will be resized proportionally.
 *
 * @param int $width   new width
 * @param int $height  new height
 * @param int $master  master dimension, if $master is TENSILE, the width and height must be specified
 * @return Phalcon\Image\Adapter
 */
PHP_METHOD(Phalcon_Image_Adapter, resize){

	zval *_width = NULL, *_height = NULL, *_master = NULL, width = {}, height = {}, image_width = {}, image_height = {};
	long tmp_image_width, tmp_image_height, tmp_width = 0, tmp_height = 0, master;

	phalcon_fetch_params(0, 0, 3, &_width, &_height, &_master);

	if (_width) {
		PHALCON_CPY_WRT_CTOR(&width, _width);
		if (Z_TYPE(width) != IS_LONG && Z_TYPE(width) > IS_NULL) {
			convert_to_long(&width);
		}
	}

	if (_height) {
		PHALCON_CPY_WRT_CTOR(&height, _height);
		if (Z_TYPE(height) != IS_LONG && Z_TYPE(height) > IS_NULL) {
			convert_to_long(&height);
		}
	}

	if (!_master) {
		master = PHALCON_IMAGE_AUTO;
	} else {
		master = phalcon_get_intval(_master);
	}

	if (PHALCON_IMAGE_TENSILE == master) {
		if (Z_TYPE(width) <= IS_NULL || Z_TYPE(height) <= IS_NULL) {
			PHALCON_THROW_EXCEPTION_STRW(phalcon_image_exception_ce, "width and height parameters must be specified");
			return;
		}
	} else {
		phalcon_return_property(&image_width, getThis(), SL("_width"));
		phalcon_return_property(&image_height, getThis(), SL("_height"));

		tmp_image_width  = phalcon_get_intval(&image_width);
		tmp_image_height = phalcon_get_intval(&image_height);

		if (Z_TYPE(width) != IS_LONG) {
			if (master == PHALCON_IMAGE_NONE) {
				tmp_width = tmp_image_width;
			} else {
				master = PHALCON_IMAGE_HEIGHT;
			}
		} else {
			tmp_width = Z_LVAL(width);
		}

		if (Z_TYPE(height) != IS_LONG) {
			if (master == PHALCON_IMAGE_NONE) {
				tmp_height = tmp_image_height;
			} else {
				master = PHALCON_IMAGE_WIDTH;
			}
		} else {
			tmp_height = Z_LVAL(height);
		}

		if (tmp_width <= 0) {
			tmp_width = 1;
		}

		if (tmp_height <= 0) {
			tmp_height = 1;
		}

		if (tmp_image_width <= 0) {
			tmp_image_width = 1;
		}

		if (tmp_image_height <= 0) {
			tmp_image_height = 1;
		}

		if (master == PHALCON_IMAGE_NARROW) {
			if (tmp_width > tmp_image_width) {
				tmp_width = tmp_image_width;
			}

			if (tmp_height > tmp_image_height) {
				tmp_height = tmp_image_height;
			}

			master = PHALCON_IMAGE_AUTO;
		}

		switch (master) {
			case PHALCON_IMAGE_AUTO:
				if ((tmp_image_width / tmp_width) > (tmp_image_height / tmp_height)) {
					tmp_height = (int)((tmp_image_height * tmp_width / tmp_image_width) + 0.5);
				} else {
					tmp_width = (int)((tmp_image_width * tmp_height / tmp_image_height) + 0.5);
				}
				break;

			case PHALCON_IMAGE_INVERSE:
				if ((tmp_image_width / tmp_width) > (tmp_image_height / tmp_height)) {
					tmp_width = (int)((tmp_image_width * tmp_height / tmp_image_height) + 0.5);
				} else {
					tmp_height = (int)((tmp_image_height * tmp_width / tmp_image_width) + 0.5);
				}
				break;
			case PHALCON_IMAGE_WIDTH:
				tmp_height = (int)((tmp_image_height * tmp_width / tmp_image_width) + 0.5);
				break;

			case PHALCON_IMAGE_HEIGHT:
				tmp_width = (int)((tmp_image_width * tmp_height / tmp_image_height) + 0.5);
				break;

			case PHALCON_IMAGE_PRECISE:
				if ((tmp_width / tmp_height) > (tmp_image_width / tmp_image_height)) {
					tmp_height = (int)((tmp_image_height * tmp_width / tmp_image_width) + 0.5);
				} else {
					tmp_width = (int)((tmp_image_width * tmp_height / tmp_image_height) + 0.5);
				}
				break;
		}

		ZVAL_LONG(&width, tmp_width);
		ZVAL_LONG(&height, tmp_height);
	}

	PHALCON_CALL_METHODW(NULL, getThis(), "_resize", &width, &height);

	RETURN_THISW();
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
PHP_METHOD(Phalcon_Image_Adapter, liquidRescale){

	zval *width, *height, *delta_x = NULL, *rigidity = NULL;

	phalcon_fetch_params(0, 2, 2, &width, &height, &delta_x, &rigidity);
	PHALCON_ENSURE_IS_LONG(width);
	PHALCON_ENSURE_IS_LONG(height);

	if (!delta_x) {
		delta_x = &PHALCON_GLOBAL(z_zero);
	} else {
		PHALCON_ENSURE_IS_LONG(delta_x);
	}

	if (!rigidity) {
		rigidity = &PHALCON_GLOBAL(z_zero);
	} else {
		PHALCON_ENSURE_IS_LONG(rigidity);
	}

	PHALCON_CALL_METHODW(NULL, getThis(), "_liquidrescale", width, height, delta_x, rigidity);

	RETURN_THISW();
}

/**
 * Crop an image to the given size. Either the width or the height can be
 * omitted and the current width or height will be used.
 *
 * @param int $width new width
 * @param int $height new height
 * @param int $offset_x offset from the left, if it's true then will center
 * @param int $offset_y offset from the top, if it's true then will middle
 * @return Phalcon\Image\Adapter
 */
PHP_METHOD(Phalcon_Image_Adapter, crop){

	zval *w, *h, *ofs_x = NULL, *ofs_y = NULL, image_width = {}, image_height = {}, width = {}, height = {}, offset_x = {}, offset_y = {};
	long tmp_max_width, tmp_max_height, tmp_width, tmp_height, tmp_image_width, tmp_image_height, tmp_offset_x, tmp_offset_y;

	phalcon_fetch_params(0, 2, 2, &w, &h, &ofs_x, &ofs_y);
	PHALCON_ENSURE_IS_LONG(w);
	PHALCON_ENSURE_IS_LONG(h);

	phalcon_return_property(&image_width, getThis(), SL("_width"));
	phalcon_return_property(&image_height, getThis(), SL("_height"));

	if (ofs_x && Z_TYPE_P(ofs_x) > IS_NULL && !PHALCON_IS_BOOL(ofs_x) && Z_TYPE_P(ofs_x) != IS_LONG) {
		SEPARATE_ZVAL_IF_NOT_REF(ofs_x);
		convert_to_long(ofs_x);
	}
		
	if (ofs_y && Z_TYPE_P(ofs_y) > IS_NULL && !PHALCON_IS_BOOL(ofs_y) && Z_TYPE_P(ofs_x) != IS_LONG) {
		SEPARATE_ZVAL_IF_NOT_REF(ofs_y);
		convert_to_long(ofs_y);
	}

	tmp_width        = Z_LVAL_P(w);
	tmp_height       = Z_LVAL_P(h);
	tmp_image_width  = phalcon_get_intval(&image_width);
	tmp_image_height = phalcon_get_intval(&image_height);

	if (tmp_width > tmp_image_width) {
		tmp_width = tmp_image_width;
	}

	if (tmp_height > tmp_image_height) {
		tmp_height = tmp_image_height;
	}

	if (!ofs_x) {
		tmp_offset_x = (int)(((tmp_image_width - tmp_width) / 2) + 0.5);
	} else if (PHALCON_IS_TRUE(ofs_x)) {
		tmp_offset_x = tmp_image_width - tmp_width;
	} else if (Z_TYPE_P(ofs_x) == IS_LONG) {
		if (Z_LVAL_P(ofs_x) < 0) {
			tmp_offset_x = (int)(tmp_image_width - tmp_width + Z_LVAL_P(ofs_x) + 0.5);
		} else {
			tmp_offset_x = Z_LVAL_P(ofs_x);
		}
	} else {
		tmp_offset_x = (int)(((tmp_image_width - tmp_width) / 2) + 0.5);
	}

	if (!ofs_y) {
		tmp_offset_y = (int)(((tmp_image_height - tmp_height) / 2) + 0.5);
	} else if (PHALCON_IS_TRUE(ofs_y)) {
		tmp_offset_y = tmp_image_height - tmp_height;
	} else if (Z_TYPE_P(ofs_y) == IS_LONG) {
		if (Z_LVAL_P(ofs_y) < 0) {
			tmp_offset_y = tmp_image_height - tmp_height + Z_LVAL_P(ofs_y);
		} else {
			tmp_offset_y = Z_LVAL_P(ofs_y);
		}
	} else {
		tmp_offset_y = (int)(((tmp_image_height - tmp_height) / 2) + 0.5);
	}

	tmp_max_width  = tmp_image_width  - tmp_offset_x;
	tmp_max_height = tmp_image_height - tmp_offset_y;

	if (tmp_width > tmp_max_width) {
		tmp_width = tmp_max_width;
	}

	if (tmp_height > tmp_max_height) {
		tmp_height = tmp_max_height;
	}

	ZVAL_LONG(&width,    tmp_width);
	ZVAL_LONG(&height,   tmp_height);
	ZVAL_LONG(&offset_x, tmp_offset_x);
	ZVAL_LONG(&offset_y, tmp_offset_y);

	PHALCON_CALL_METHODW(NULL, getThis(), "_crop", &width, &height, &offset_x, &offset_y);

	RETURN_THISW();
}

/**
 * Rotate the image by a given amount.
 *
 * @param int $degrees  degrees to rotate: -360-360
 * @return Phalcon\Image\Adapter
 */
PHP_METHOD(Phalcon_Image_Adapter, rotate){

	zval *degrees, d = {};
	long tmp_degrees;

	phalcon_fetch_params(0, 1, 0, &degrees);
	PHALCON_ENSURE_IS_LONG(degrees);

	tmp_degrees = Z_LVAL_P(degrees);

	if (tmp_degrees > 180) {
		tmp_degrees %= 360;
		if (tmp_degrees > 180) {
			tmp_degrees -= 360;
		};
	} else if (tmp_degrees < -180) {
		do {
			tmp_degrees += 360;
		} while (tmp_degrees < -180);
	}

	ZVAL_LONG(&d, tmp_degrees);
	PHALCON_CALL_METHODW(NULL, getThis(), "_rotate", &d);

	RETURN_THISW();
}

/**
 * Flip the image along the horizontal or vertical axis.
 *
 * @param $int $direction  direction: Image::HORIZONTAL, Image::VERTICAL
 * @return Phalcon\Image\Adapter
 */
PHP_METHOD(Phalcon_Image_Adapter, flip){

	zval *direction, dir = {};

	phalcon_fetch_params(0, 1, 0, &direction);
	PHALCON_ENSURE_IS_LONG(direction);

	ZVAL_LONG(&dir, (Z_LVAL_P(direction) != 11) ? 12 : 11);

	PHALCON_CALL_METHODW(NULL, getThis(), "_flip", &dir);

	RETURN_THISW();
}

/**
 * Sharpen the image by a given amount.
 *
 * @param int $amount amount to sharpen: 1-100
 * @return Phalcon\Image\Adapter
 */
PHP_METHOD(Phalcon_Image_Adapter, sharpen){

	zval *amount;

	phalcon_fetch_params(0, 1, 0, &amount);
	SEPARATE_ZVAL_IF_NOT_REF(amount);

	if (Z_TYPE_P(amount) != IS_LONG) {
		convert_to_long(amount);
	}

	if (Z_LVAL_P(amount) > 100) {
		ZVAL_LONG(amount, 100);
	} else if (Z_LVAL_P(amount) < 1) {
		ZVAL_LONG(amount, 1);
	}

	PHALCON_CALL_METHODW(NULL, getThis(), "_sharpen", amount);

	RETURN_THISW();
}

/**
 * Add a reflection to an image. The most opaque part of the reflection
 * will be equal to the opacity setting and fade out to full transparent.
 * Alpha transparency is preserved.
 *
 * @param int $height reflection height
 * @param int $opacity reflection opacity: 0-100
 * @param boolean $fade_in TRUE to fade in, FALSE to fade out
 * @return Phalcon\Image\Adapter
 */
PHP_METHOD(Phalcon_Image_Adapter, reflection){

	zval *h = NULL, *op = NULL, *fade_in = NULL, image_height = {}, height = {}, opacity = {};
	long tmp_image_height;

	phalcon_fetch_params(0, 0, 3, &h, &op, &fade_in);

	phalcon_return_property(&image_height, getThis(), SL("_height"));
	tmp_image_height = phalcon_get_intval(&image_height);

	if (!h || Z_TYPE_P(h) != IS_LONG || Z_LVAL_P(h) > tmp_image_height) {
		ZVAL_LONG(&height, tmp_image_height);
	} else {
		PHALCON_CPY_WRT_CTOR(&height, h);
	}

	if (!op) {
		ZVAL_LONG(&opacity, 100);
	} else {
		PHALCON_ENSURE_IS_LONG(op);

		if (Z_LVAL_P(op) > 100) {
			ZVAL_LONG(&opacity, 100);
		} else if (Z_LVAL_P(op) < 0) {
			ZVAL_LONG(&opacity, 0);
		} else {
			PHALCON_CPY_WRT_CTOR(&opacity, op);
		}
	}

	if (!fade_in) {
		fade_in = &PHALCON_GLOBAL(z_false);
	}

	PHALCON_CALL_METHODW(NULL, getThis(), "_reflection", &height, &opacity, fade_in);
	RETURN_THISW();
}

/**
 * Add a watermark to an image with a specified opacity. Alpha transparency
 * will be preserved.
 *
 * @param Phalcon\Image\Adapter $watermark  watermark Image instance
 * @param int $offset_x offset from the left, If less than 0 offset from the right, If true right the x offset
 * @param int $offset_y offset from the top, If less than 0 offset from the bottom, If true bottom the Y offset
 * @param int $opacity opacity of watermark: 1-100
 * @return Phalcon\Image\AdapterInterface
 */
PHP_METHOD(Phalcon_Image_Adapter, watermark){

	zval *watermark, *ofs_x = NULL, *ofs_y = NULL, *op = NULL, offset_x = {}, offset_y = {}, opacity = {}, image_width = {}, image_height = {};
	zval watermark_width = {}, watermark_height = {};
	long tmp_image_width, tmp_image_height, tmp_watermark_width, tmp_watermark_height, tmp_offset_x, tmp_offset_y;

	phalcon_fetch_params(0, 1, 3, &watermark, &ofs_x, &ofs_y, &op);
	PHALCON_VERIFY_INTERFACE_EX(watermark, phalcon_image_adapterinterface_ce, phalcon_image_exception_ce, 0);

	phalcon_return_property(&image_width, getThis(), SL("_width"));
	phalcon_return_property(&image_height, getThis(), SL("_height"));
	phalcon_return_property(&watermark_width, watermark, SL("_width"));
	phalcon_return_property(&watermark_height, watermark, SL("_height"));

	tmp_image_width      = phalcon_get_intval(&image_width);
	tmp_image_height     = phalcon_get_intval(&image_height);
	tmp_watermark_width  = phalcon_get_intval(&watermark_width);
	tmp_watermark_height = phalcon_get_intval(&watermark_height);

	if (!ofs_x) {
		tmp_offset_x = (int)(((tmp_image_width - tmp_watermark_width) / 2) + 0.5);
	} else if (Z_TYPE_P(ofs_x) == IS_LONG) {
		tmp_offset_x = Z_LVAL_P(ofs_x);
		if (tmp_offset_x < 0) {
			tmp_offset_x = (int)(tmp_image_width - tmp_watermark_width + tmp_offset_x + 0.5);
		}
	} else if (zend_is_true(ofs_x)) {
		tmp_offset_x = (int)(tmp_image_width - tmp_watermark_width);
	} else {
		tmp_offset_x = (int)(((tmp_image_width - tmp_watermark_width) / 2) + 0.5);
	}

	ZVAL_LONG(&offset_x, tmp_offset_x);

	if (!ofs_y) {
		tmp_offset_y = (int)(((tmp_image_height - tmp_watermark_height) / 2) + 0.5);
	} else if (Z_TYPE_P(ofs_y) == IS_LONG) {
		tmp_offset_y = Z_LVAL_P(ofs_y);
		if (tmp_offset_y < 0) {
			tmp_offset_y = (int)(tmp_image_height - tmp_watermark_height + tmp_offset_y + 0.5);
		}
	} else if (zend_is_true(ofs_y)) {
		tmp_offset_y = (int)(tmp_image_height - tmp_watermark_height);
	} else {
		tmp_offset_y = (int)(((tmp_image_height - tmp_watermark_height) / 2) + 0.5);
	}

	ZVAL_LONG(&offset_y, tmp_offset_y);

	if (!op) {
		ZVAL_LONG(&opacity, 100);
	} else {
		PHALCON_ENSURE_IS_LONG(op);

		if (Z_LVAL_P(op) < 1) {
			ZVAL_LONG(&opacity, 1);
		} else if (Z_LVAL_P(op) > 100) {
			ZVAL_LONG(&opacity, 100);
		} else {
			ZVAL_LONG(&opacity, Z_LVAL_P(op));
		}
	}

	PHALCON_CALL_METHODW(NULL, getThis(), "_watermark", watermark, &offset_x, &offset_y, &opacity);

	RETURN_THISW();
}


/**
 * Add a text to an image with a specified opacity.
 *
 * @param string text
 * @param int $offset_x offset from the left, If less than 0 offset from the right, If true right the x offset, If NULL center
 * @param int $offset_y offset from the top, If less than 0 offset from the bottom, If true bottom the Y offset, If NULL center
 * @param int $opacity opacity of text: 1-100
 * @param string $color hexadecimal color value
 * @param int $size font pointsize
 * @param string $fontfile font path
 * @return Phalcon\Image\Adapter
 */
PHP_METHOD(Phalcon_Image_Adapter, text){

	zval *text, *ofs_x = NULL, *ofs_y = NULL, *op = NULL, *fontcolor = NULL, *fontsize = NULL, *fontfile = NULL;
	zval offset_x = {}, offset_y = {}, opacity = {}, color = {}, size = {}, tmp = {}, r = {}, g = {}, b = {};
	zend_string *c;

	phalcon_fetch_params(0, 1, 6, &text, &ofs_x, &ofs_y, &op, &fontcolor, &fontsize, &fontfile);

	if (!ofs_x) {
		ZVAL_NULL(&offset_x);
	} else {
		PHALCON_CPY_WRT_CTOR(&offset_x, ofs_x);
	}

	if (!ofs_y) {
		ZVAL_NULL(&offset_x);
	} else {
		PHALCON_CPY_WRT_CTOR(&offset_y, ofs_y);
	}

	if (!op || Z_TYPE_P(op) == IS_NULL) {
		ZVAL_LONG(&opacity, 100);
	} else {
		PHALCON_ENSURE_IS_LONG(op);
		if (Z_LVAL_P(op) < 1) {
			ZVAL_LONG(&opacity, 1);
		} else if (Z_LVAL_P(op) > 100) {
			ZVAL_LONG(&opacity, 100);
		} else {
			ZVAL_LONG(&opacity, Z_LVAL_P(op));
		}
	}

	if (!fontcolor || Z_TYPE_P(fontcolor) == IS_NULL) {
		ZVAL_STRING(&color, "000000");
	} else {
		PHALCON_ENSURE_IS_STRING(fontcolor);
		if (Z_STRLEN_P(fontcolor) > 1 && Z_STRVAL_P(fontcolor)[0] == '#') {
			phalcon_substr(&color, fontcolor, 1, 0);
		} else {
			ZVAL_NEW_STR(&color, Z_STR_P(fontcolor));
		}
	}

	if (!fontsize || Z_TYPE_P(fontsize) == IS_NULL) {
		ZVAL_LONG(&size, 12);
	} else {
		PHALCON_ENSURE_IS_LONG(fontsize);
		ZVAL_LONG(&size, Z_LVAL_P(fontsize));
	}

	if (!fontfile) {
		fontfile = &PHALCON_GLOBAL(z_null);
	}

	if (Z_STRLEN(color) == 3) {
		/* Convert RGB to RRGGBB */
		c = Z_STR(color);
		assert(!IS_INTERNED(c));
		zend_string_realloc(c, 7, 0);
		c->val[6] = '\0';
		c->val[5] = c->val[2];
		c->val[4] = c->val[2];
		c->val[3] = c->val[1];
		c->val[2] = c->val[1];
		c->val[1] = c->val[0];
		ZVAL_STR(&color, c);
	}

	if (Z_STRLEN(color) < 6) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_image_exception_ce, "color is not valid");
		return;
	}

	phalcon_substr(&tmp, &color, 0, 2);
	_php_math_basetozval(&tmp, 16, &r);

	phalcon_substr(&tmp, &color, 2, 2);
	_php_math_basetozval(&tmp, 16, &g);

	phalcon_substr(&tmp, &color, 4, 2);
	_php_math_basetozval(&tmp, 16, &b);

	PHALCON_CALL_METHODW(NULL, getThis(), "_text", text, &offset_x, &offset_y, &opacity, &r, &g, &b, &size, fontfile);

	RETURN_THISW();
}

/**
 * Composite one image onto another
 *
 * @param Phalcon\Image\Adapter $mask  mask Image instance
 * @return Phalcon\Image\Adapter
 */
PHP_METHOD(Phalcon_Image_Adapter, mask){

	zval *mask;

	phalcon_fetch_params(0, 1, 0, &mask);
	PHALCON_CALL_METHODW(NULL, getThis(), "_mask", mask);
	RETURN_THISW();
}

/**
 * Set the background color of an image. This is only useful for images
 * with alpha transparency.
 *
 * @param string $color hexadecimal color value
 * @param int $opacity background opacity: 0-100
 * @return Phalcon\Image\Adapter
 */
PHP_METHOD(Phalcon_Image_Adapter, background){

	zval *bcolor, *_opacity = NULL, opacity = {}, color = {}, tmp = {}, r = {}, g = {}, b = {};
	zend_string *c;
	long i;

	phalcon_fetch_params(0, 1, 1, &bcolor, &_opacity);

	if (!_opacity) {
		ZVAL_LONG(&opacity, 100);
	} else {
		PHALCON_CPY_WRT_CTOR(&opacity, _opacity);
	}

	if (Z_TYPE_P(bcolor) == IS_NULL) {
		ZVAL_STRING(&color, "000000");
	} else {
		PHALCON_ENSURE_IS_STRING(bcolor);
		if (Z_STRLEN_P(bcolor) > 1 && Z_STRVAL_P(bcolor)[0] == '#') {
			phalcon_substr(&color, bcolor, 1, 0);
		}
		else {
			ZVAL_NEW_STR(&color, Z_STR_P(bcolor));
		}
	}

	if (Z_STRLEN(color) == 3) {
		/* Convert RGB to RRGGBB */
		c = Z_STR(color);
		assert(!IS_INTERNED(c));
		zend_string_realloc(c, 7, 0);
		c->val[6] = '\0';
		c->val[5] = c->val[2];
		c->val[4] = c->val[2];
		c->val[3] = c->val[1];
		c->val[2] = c->val[1];
		c->val[1] = c->val[0];
		ZVAL_STR(&color, c);
	}

	if (Z_STRLEN(color) < 6) {
		PHALCON_THROW_EXCEPTION_STRW(phalcon_image_exception_ce, "color is not valid");
		return;
	}

	phalcon_substr(&tmp, &color, 0, 2);
	_php_math_basetozval(&tmp, 16, &r);

	phalcon_substr(&tmp, &color, 2, 2);
	_php_math_basetozval(&tmp, 16, &g);

	phalcon_substr(&tmp, &color, 4, 2);
	_php_math_basetozval(&tmp, 16, &b);

	i = phalcon_get_intval(&opacity);

	if (i < 1) {
		ZVAL_LONG(&opacity, 1);
	} else if (i > 100) {
		ZVAL_LONG(&opacity, 100);
	}

	PHALCON_CALL_METHODW(NULL, getThis(), "_background", &r, &g, &b, &opacity);

	RETURN_THISW();
}

/**
 * Blur image
 *
 * @param int $radius Blur radius
 * @return Phalcon\Image\Adapter
 */
PHP_METHOD(Phalcon_Image_Adapter, blur){

	zval *_radius = NULL, radius = {};
	long r;

	phalcon_fetch_params(0, 0, 1, &_radius);

	if (!_radius) {
		ZVAL_LONG(&radius, 1);
	} else if (Z_TYPE_P(_radius) != IS_LONG) {
		ZVAL_LONG(&radius, 1);
	} else {
		r = phalcon_get_intval(_radius);
		if (r < 1) {
			ZVAL_LONG(&radius, 1);
		} else if (r > 100) {
			ZVAL_LONG(&radius, 100);
		} else {
			PHALCON_CPY_WRT(&radius, _radius);
		}
	}

	PHALCON_CALL_METHODW(NULL, getThis(), "_blur", &radius);

	RETURN_THISW();
}

/**
 * Pixelate image
 *
 * @param int $amount amount to pixelate
 * @return Phalcon\Image\Adapter
 */
PHP_METHOD(Phalcon_Image_Adapter, pixelate){

	zval *amount = NULL, amt = {};

	phalcon_fetch_params(0, 0, 1, &amount);

	if (!amount || Z_TYPE_P(amount) != IS_LONG) {
		ZVAL_LONG(&amt, 10);
	} else {
		PHALCON_ENSURE_IS_LONG(amount);
		if (Z_LVAL_P(amount) < 2) {
			ZVAL_LONG(&amt, 2);
		} else {
			ZVAL_LONG(&amt, Z_LVAL_P(amount));
		}
	}

	PHALCON_CALL_METHODW(NULL, getThis(), "_pixelate", &amt);

	RETURN_THISW();
}

/**
 * Save the image. If the filename is omitted, the original image will
 * be overwritten.
 *
 * @param string $file new image path
 * @param int $quality quality of image: 1-100
 * @return boolean
 */
PHP_METHOD(Phalcon_Image_Adapter, save){

	zval *fname = NULL, *q = NULL, file = {}, quality = {}, ret = {}, dir = {}, *constant;

	phalcon_fetch_params(0, 0, 2, &fname, &q);

	if (!fname) {
		phalcon_return_property(&file, getThis(), SL("_realpath"));
	} else {
		PHALCON_CPY_WRT_CTOR(&file, fname);
	}

	if (Z_TYPE(file) != IS_STRING) {
		convert_to_string_ex(&file);
	}

	if (!q || Z_TYPE_P(q) != IS_LONG) {
		ZVAL_LONG(&quality, 100);
	} else if (Z_LVAL_P(q) > 100) {
		ZVAL_LONG(&quality, 100);
	} else if (Z_LVAL_P(q) < 1) {
		ZVAL_LONG(&quality, 1);
	} else {
		ZVAL_LONG(&quality, Z_LVAL_P(q));
	}

	PHALCON_CALL_FUNCTIONW(&ret, "is_file", &file);

	if (zend_is_true(&ret)) {
		PHALCON_CALL_FUNCTIONW(&ret, "is_writable", &file);
		if (!zend_is_true(&ret)) {
			zend_throw_exception_ex(phalcon_image_exception_ce, 0, "File must be writable: '%s'", Z_STRVAL(file));
			return;
		}
	} else {
		if ((constant = zend_get_constant_str(SL("PATHINFO_DIRNAME"))) == NULL) {
			return;
		}

		PHALCON_CALL_FUNCTIONW(&ret, "pathinfo", &file, constant);

		phalcon_file_realpath(&dir, &ret);
		convert_to_string(&dir);

		phalcon_is_dir(&ret, &dir);

		if (!zend_is_true(&ret)) {
			zend_throw_exception_ex(phalcon_image_exception_ce, 0, "Directory must be writable: '%s'", Z_STRVAL(dir));
			return;
		}

		PHALCON_CALL_FUNCTIONW(&ret, "is_writable", &dir);
		if (!zend_is_true(&ret)) {
			zend_throw_exception_ex(phalcon_image_exception_ce, 0, "Directory must be writable: '%s'", Z_STRVAL(dir));
			return;
		}
	}

	PHALCON_RETURN_CALL_METHODW(getThis(), "_save", &file, &quality);
}

/**
 * Render the image and return the binary string.
 *
 * @param string $ext image type to return: png, jpg, gif, etc
 * @param int $quality quality of image: 1-100
 * @return Phalcon\Image\Adapter
 */
PHP_METHOD(Phalcon_Image_Adapter, render){

	zval *ext = NULL, *_quality = NULL, format = {}, quality = {};

	phalcon_fetch_params(0, 0, 2, &ext, &_quality);

	if (!ext) {
		phalcon_return_property(&format, getThis(), SL("_format"));

		if (PHALCON_IS_EMPTY(&format)) {
			ZVAL_STRING(&format, "png");
		} 
	} else {
		PHALCON_CPY_WRT(&format, ext);
	}

	if (!_quality) {
		ZVAL_LONG(&quality, 100);
	} else {
		PHALCON_CPY_WRT(&quality, _quality);
        if (Z_TYPE(quality) != IS_LONG) {
            convert_to_long(&quality);
        }
    }

	PHALCON_RETURN_CALL_METHODW(getThis(), "_render", &format, &quality);
}
