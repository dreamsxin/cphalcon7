
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

#ifndef PHALCON_IMAGE_ADAPTER_H
#define PHALCON_IMAGE_ADAPTER_H

#include "php_phalcon.h"

extern zend_class_entry *phalcon_image_adapter_ce;

PHALCON_INIT_CLASS(Phalcon_Image_Adapter);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter__resize, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter__liquidrescale, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	ZEND_ARG_INFO(0, delta_x)
	ZEND_ARG_INFO(0, regidity)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter__crop, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	ZEND_ARG_INFO(0, offset_x)
	ZEND_ARG_INFO(0, offset_y)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter__rotate, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, degrees, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter__flip, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, direction, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter__sharpen, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, amount, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter__reflection, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, opacity, IS_LONG, 0)
	ZEND_ARG_INFO(0, fade_in)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter__watermark, 0, 0, 4)
	ZEND_ARG_INFO(0, watermark)
	ZEND_ARG_TYPE_INFO(0, offset_x, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, offset_y, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, opacity, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter__text, 0, 0, 9)
	ZEND_ARG_INFO(0, text)
	ZEND_ARG_TYPE_INFO(0, offset_x, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, offset_y, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, opacity, IS_LONG, 0)
	ZEND_ARG_INFO(0, r)
	ZEND_ARG_INFO(0, g)
	ZEND_ARG_INFO(0, b)
	ZEND_ARG_INFO(0, size)
	ZEND_ARG_INFO(0, fontfile)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter__mask, 0, 0, 1)
	ZEND_ARG_INFO(0, mask)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter__blur, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, radius, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter__pixelate, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, amount, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter__background, 0, 0, 4)
	ZEND_ARG_INFO(0, r)
	ZEND_ARG_INFO(0, g)
	ZEND_ARG_INFO(0, b)
	ZEND_ARG_TYPE_INFO(0, opacity, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter__save, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, file, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, opacity, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, interlacing, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_image_adapter__render, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, type, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, opacity, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, interlacing, IS_LONG, 1)
ZEND_END_ARG_INFO()

#endif /* PHALCON_IMAGE_ADAPTER_H */
