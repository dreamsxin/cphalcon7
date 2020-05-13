
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

#ifndef PHALCON_BINARY_H
#define PHALCON_BINARY_H

#include "php_phalcon.h"

#define PHALCON_BINARY_ENDIAN_MACHINE		0
#define PHALCON_BINARY_ENDIAN_BIG			1
#define PHALCON_BINARY_ENDIAN_LITTLE		2

#define PHALCON_BINARY_TYPE_CHAR				0
#define PHALCON_BINARY_TYPE_UNSIGNEDCHAR		1
#define PHALCON_BINARY_TYPE_INT16				2
#define PHALCON_BINARY_TYPE_UNSIGNEDINT16		3
#define PHALCON_BINARY_TYPE_INT					4
#define PHALCON_BINARY_TYPE_UNSIGNEDINT			5
#define PHALCON_BINARY_TYPE_INT32				6
#define PHALCON_BINARY_TYPE_UNSIGNEDINT32		7
#define PHALCON_BINARY_TYPE_FLOAT				8
#define PHALCON_BINARY_TYPE_DOUBLE				9
#define PHALCON_BINARY_TYPE_STRING				10
#define PHALCON_BINARY_TYPE_HEXSTRING			11

extern zend_class_entry *phalcon_binary_ce;

PHALCON_INIT_CLASS(Phalcon_Binary);

#endif /* PHALCON_BINARY_H */
