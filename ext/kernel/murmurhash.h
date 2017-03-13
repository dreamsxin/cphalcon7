
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

#ifdef ZEND_ENABLE_ZVAL_LONG64

static zend_always_inline zend_ulong MurmurHash2(const void* key, zend_ulong len, uint32_t seed)
{
	const unsigned char* data2;
	const zend_ulong m = sizeof(zend_ulong) == 8 ? 0xc6a4a7935bd1e995 : 0;
	const int r = 47;

	zend_ulong h = seed ^ (len * m);

	const zend_ulong* data = (const zend_ulong *)key;
	const zend_ulong* end = data + (len / 8);

	while (data != end)
	{
		zend_ulong k = *data++;

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;
	}

	data2 = (const unsigned char*)data;

	switch (len & 7)
	{
		case 7: h ^= (zend_ulong)data2[6] << 48;
		case 6: h ^= (zend_ulong)data2[5] << 40;
		case 5: h ^= (zend_ulong)data2[4] << 32;
		case 4:	h ^= (zend_ulong)data2[3] << 24;
		case 3:	h ^= (zend_ulong)data2[2] << 16;
		case 2:	h ^= (zend_ulong)data2[1] << 8;
		case 1:	h ^= (zend_ulong)data2[0];
		h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}
#else
static zend_always_inline zend_ulong MurmurHash2(const void* key, zend_ulong len, uint32_t seed)
{
	/* 'm' and 'r' are mixing constants generated offline.
	   They're not really 'magic', they just happen to work well. */
	const zend_ulong m = 0x5bd1e995;
	const int r = 24;

	zend_ulong h = seed ^ len;

	/* Mix 4 bytes at a time into the hash */
	const unsigned char * data = (const unsigned char *)key;

	while (len >= 4)
	{
		zend_ulong k = *(zend_ulong *)data;

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}

	/* Handle the last few bytes of the input array */
	switch (len)
	{
		case 3: h ^= data[2] << 16;
		case 2: h ^= data[1] << 8;
		case 1:	h ^= data[0];
		h *= m;
	};

	/* Do a few final mixes of the hash to ensure the last few
	   bytes are well-incorporated. */

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}
#endif

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
