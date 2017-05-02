
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

#ifndef PHALCON_IMAGE_ADAPTERINTERFACE_H
#define PHALCON_IMAGE_ADAPTERINTERFACE_H

#include "php_phalcon.h"

extern zend_class_entry *phalcon_image_adapterinterface_ce;

PHALCON_INIT_CLASS(Phalcon_Image_AdapterInterface);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapterinterface_resize, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, master, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapterinterface_liquidrescale, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, delta_x, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, rigidity, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapterinterface_crop, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, offset_x, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, offset_y, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapterinterface_rotate, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, degrees, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapterinterface_flip, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, direction, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapterinterface_sharpen, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, amount, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapterinterface_reflection, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, opacity, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, fade_in, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapterinterface_watermark, 0, 0, 1)
	ZEND_ARG_INFO(0, watermark)
	ZEND_ARG_TYPE_INFO(0, offset_x, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, offset_y, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, opacity, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapterinterface_text, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, offset_x, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, offset_y, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, opacity, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, color, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, fontfile, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapterinterface_line, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, sx, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, sy, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, ex, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, ey, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, color, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapterinterface_polygon, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, coordinates, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, color, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapterinterface_mask, 0, 0, 1)
	ZEND_ARG_INFO(0, mask)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapterinterface_background, 0, 0, 1)
	ZEND_ARG_INFO(0, color)
	ZEND_ARG_TYPE_INFO(0, opacity, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapterinterface_blur, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, radius, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapterinterface_pixelate, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, amount, IS_LONG, 1)
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_phalcon_image_adapterinterface_save, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, file, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, opacity, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, interlacing, IS_LONG, 1)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_phalcon_image_adapterinterface_save, 0, 0, _IS_BOOL, 0, 0)
	ZEND_ARG_TYPE_INFO(0, file, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, opacity, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, interlacing, IS_LONG, 1)
ZEND_END_ARG_INFO()
#endif

#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_phalcon_image_adapterinterface_render, 0, 0, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, opacity, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, interlacing, IS_LONG, 1)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_phalcon_image_adapterinterface_render, 0, 0, IS_STRING, 0, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, opacity, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, interlacing, IS_LONG, 1)
ZEND_END_ARG_INFO()
#endif

#endif /* PHALCON_IMAGE_ADAPTERINTERFACE_H */
