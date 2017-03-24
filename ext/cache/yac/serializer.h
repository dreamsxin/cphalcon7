/*
  +----------------------------------------------------------------------+
  | Yet Another Cache                                                    |
  +----------------------------------------------------------------------+
  | Copyright (c) 2013-2013 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:  Xinchen Hui   <laruence@php.net>                            |
  +----------------------------------------------------------------------+
*/

#ifndef PHALCON_CACHE_YAC_SERIALIZER_H
#define PHALCON_CACHE_YAC_SERIALIZER_H

int phalcon_cache_yac_serializer_php_pack(zval *pzval, smart_str *buf, char **msg);
zval * phalcon_cache_yac_serializer_php_unpack(char *content, size_t len, char **msg, zval *rv);

#endif	/* PHALCON_CACHE_YAC_SERIALIZER_H */
