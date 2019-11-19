
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
  | Authors: ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#ifndef PHALCON_CHART_QRCODE_QR_H
#define PHALCON_CHART_QRCODE_QR_H

#include "php_phalcon.h"

#define PNG_SKIP_SETJMP_CHECK 1
#include <png.h>
#include <qrencode.h>

#define INCHES_PER_METER (100.0/2.54)

// PNG_COLOR_TYPE_RGB
// PNG_COLOR_TYPE_RGB_ALPHA
enum QRImageType {
	PNG_TYPE,
	PNG32_TYPE
};

int qr_write(FILE *fp, const QRcode *qrcode, int size, int margin, unsigned int fg_color[4], unsigned int bg_color[4], enum QRImageType type);

#endif	/* PHALCON_CHART_QRCODE_QR_H */