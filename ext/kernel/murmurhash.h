
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
  |          Vladimir Kolesnikov <vladimir@extrememember.com>              |
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#ifndef PHALCON_KERNEL_MURMURHASH_H
#define PHALCON_KERNEL_MURMURHASH_H

#include "php_phalcon.h"

void MurmurHash3_x86_32 (const void* key, int len, uint32_t seed, void* out);
void MurmurHash3_x86_128(const void* key, int len, uint32_t seed, void* out);

#ifdef ZEND_ENABLE_ZVAL_LONG64
void MurmurHash3_x64_128(const void* key, int len, uint32_t seed, void* out);
#endif

zend_ulong MurmurHash2(const void* key, zend_ulong len, uint32_t seed);

static zend_always_inline zend_ulong phalcon_murmurhash_pointer(const void* key, uint32_t seed)
{
	return MurmurHash2(&key, sizeof key, seed);
}

static zend_always_inline zend_ulong phalcon_murmurhash_int(const void* key, uint32_t seed)
{
	return MurmurHash2(key, sizeof(int), seed);
}

static zend_always_inline zend_ulong phalcon_murmurhash_string(const void* key, uint32_t seed)
{
	return MurmurHash2(key, strlen((const char*)key), seed);
}

#endif /* PHALCON_KERNEL_MURMURHASH_H */
