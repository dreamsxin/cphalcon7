
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

#ifndef PHALCON_CHART_CAPTCHA_TINY_H
#define PHALCON_CHART_CAPTCHA_TINY_H

#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>

#include "php_phalcon.h"
#include "kernel/file.h"

#define NDOTS 100
#define GIFSIZE 17646

typedef struct _phalcon_chart_captcha_tiny_object {
	unsigned char word[6];
	unsigned char im[70*200];
	unsigned char gif[GIFSIZE];
	zend_object std;
} phalcon_chart_captcha_tiny_object;

static inline phalcon_chart_captcha_tiny_object *phalcon_chart_captcha_tiny_object_from_obj(zend_object *obj) {
	return (phalcon_chart_captcha_tiny_object*)((char*)(obj) - XtOffsetOf(phalcon_chart_captcha_tiny_object, std));
}

extern zend_class_entry *phalcon_chart_captcha_tiny_ce;

PHALCON_INIT_CLASS(Phalcon_Chart_Captcha_Tiny);

#endif /* PHALCON_CHART_CAPTCHA_TINY_H */
