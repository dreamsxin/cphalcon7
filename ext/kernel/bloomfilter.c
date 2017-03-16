
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

#include "kernel/bloomfilter.h"

#include <sys/file.h>

static zend_always_inline void __bloomfilter_calc(phalcon_bloomfilter *bloomfilter)
{
    uint32_t m, k;

    m = (uint32_t) ceil(-1 * log(bloomfilter->false_positive) * bloomfilter->max_items / 0.6185);
    m = (m - m % 64) + 64;

    k = (uint32_t) (0.6931 * m / bloomfilter->max_items);
    k++;

    bloomfilter->bits = m;
    bloomfilter->hash_num = k;
    bloomfilter->bytes = bloomfilter->bits / 8;
}

static zend_always_inline void __bloomfilter_hash(phalcon_bloomfilter *bloomfilter, const void * key, int len)
{
    int i;
    uint32_t bits = bloomfilter->bits;
    zend_ulong hash1 = MurmurHash2(key, len, bloomfilter->seed);
#ifdef ZEND_ENABLE_ZVAL_LONG64
    zend_ulong hash2 = MurmurHash2(key, len, ((uint32_t)((hash1>>32)^(hash1))));
#else
	zend_ulong hash2 = MurmurHash2(key, len, hash1);
#endif

    for (i = 0; i < (int)bloomfilter->hash_num; i++)
    {
        bloomfilter->hash_value[i] = (hash1 + i*hash2) % bits;
    }
}

int phalcon_bloomfilter_init(phalcon_bloomfilter *bloomfilter, uint32_t seed, uint32_t max_items, double false_positive)
{
    if (bloomfilter == NULL || (false_positive <= 0) || (false_positive >= 1))
        return -1;

    phalcon_bloomfilter_free(bloomfilter);

    bloomfilter->max_items = max_items;
    bloomfilter->false_positive = false_positive;
    bloomfilter->seed = seed;

    __bloomfilter_calc(bloomfilter);
    bloomfilter->data = (unsigned char *) ecalloc(1, bloomfilter->bytes);
    if (NULL == bloomfilter->data)
        return -1;

    bloomfilter->hash_value = (uint32_t*) emalloc(bloomfilter->hash_num * sizeof(uint32_t));
    if (NULL == bloomfilter->hash_value)
        return -1;

    bloomfilter->flag = 1;
    return 0;
}

int phalcon_bloomfilter_add(phalcon_bloomfilter *bloomfilter, const void * key, int len)
{
	int i;
    if ((bloomfilter == NULL) || bloomfilter->flag != 1 || key == NULL || len <= 0)
        return -1;

    // hash key到bloomfilter中
    __bloomfilter_hash(bloomfilter, key, len);
    for (i = 0; i < (int)bloomfilter->hash_num; i++)
    {
        PHALCON_BLOOMFILTER_SETBIT(bloomfilter, bloomfilter->hash_value[i]);
    }

    // 增加count数
    bloomfilter->count++;
    if (bloomfilter->count <= bloomfilter->max_items)
        return 0;
    else
        return 1;
}

int phalcon_bloomfilter_check(phalcon_bloomfilter *bloomfilter, const void * key, int len)
{
	int i;
    if (bloomfilter == NULL || bloomfilter->flag != 1 || key == NULL || len <= 0)
        return -1;

    __bloomfilter_hash(bloomfilter, key, len);
    for (i = 0; i < (int)bloomfilter->hash_num; i++) {
        if (PHALCON_BLOOMFILTER_GETBIT(bloomfilter, bloomfilter->hash_value[i]) == 0)
            return 1;
    }

    return 0;
}

int phalcon_bloomfilter_free(phalcon_bloomfilter *bloomfilter)
{
    if (bloomfilter == NULL)
        return -1;

    bloomfilter->flag = 0;
    bloomfilter->count = 0;

    if (bloomfilter->data) {
        efree(bloomfilter->data);
        bloomfilter->data = NULL;
    }
    if (bloomfilter->hash_value) {
        efree(bloomfilter->hash_value);
        bloomfilter->hash_value = NULL;
    }
    return 0;
}

int phalcon_bloomfilter_load(phalcon_bloomfilter *bloomfilter, char *filename)
{
	int ret, fd;
	FILE *fp;
	static phalcon_bloomfilter_file_head file_head = {0};

    if (bloomfilter == NULL || filename == NULL)
        return -1;

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        return -1;
    }
	fd = fileno(fp);
    if (flock(fd, LOCK_SH) !=0) {
		ret = -1;
		goto end;
    }

    ret = fread((void*)&file_head, sizeof(file_head), 1, fp);
    if (ret != 1) {
		ret = -1;
        goto end;
    }

    if ((file_head.file_magic_code != PHALCON_BLOOMFILTER_FILE_MAGIC_CODE) || (file_head.bits != file_head.bytes*8)) {
		ret = -1;
        goto end;
	}

    phalcon_bloomfilter_free(bloomfilter);
    bloomfilter->max_items = file_head.max_items;
    bloomfilter->false_positive = file_head.false_positive;
    bloomfilter->bits = file_head.bits;
    bloomfilter->hash_num = file_head.hash_num;
    bloomfilter->seed = file_head.seed;
    bloomfilter->count = file_head.count;
    bloomfilter->bytes = file_head.bytes;

    bloomfilter->data = (unsigned char *) emalloc(bloomfilter->bytes);
    if (NULL == bloomfilter->data) {
		ret = -1;
        goto end;
	}

    bloomfilter->hash_value = (uint32_t*) emalloc(bloomfilter->hash_num * sizeof(uint32_t));
    if (NULL == bloomfilter->hash_value) {
		ret = -1;
        goto end;
	}


    // 将后面的Data部分读入 data
    ret = fread((void*)(bloomfilter->data), 1, bloomfilter->bytes, fp);
    if ((uint32_t)ret != bloomfilter->bytes) {
		ret = -1;
        goto end;
    }

    bloomfilter->flag = 1;
	ret = 0;

end:
	flock(fd, LOCK_UN);
	fclose(fp);
	return ret;
}

int phalcon_bloomfilter_save(phalcon_bloomfilter *bloomfilter, char *filename)
{
	int ret, fd;
	FILE *fp;
	static phalcon_bloomfilter_file_head file_head = {0};

    if (bloomfilter == NULL || bloomfilter->flag !=1 || filename == NULL)
        return -1;

    fp = fopen(filename, "wb");
    if (fp == NULL) {
        return -1;
    }
	fd = fileno(fp);
    if (flock(fd, LOCK_EX) !=0) {
		ret = -1;
		goto end;
    }

    file_head.file_magic_code = PHALCON_BLOOMFILTER_FILE_MAGIC_CODE;
    file_head.seed = bloomfilter->seed;
    file_head.count = bloomfilter->count;
    file_head.max_items = bloomfilter->max_items;
    file_head.false_positive = bloomfilter->false_positive;
    file_head.bits = bloomfilter->bits;
    file_head.hash_num = bloomfilter->hash_num;
    file_head.bytes = bloomfilter->bytes;

    ret = fwrite((const void*)&file_head, sizeof(file_head), 1, fp);
    if (ret != 1) {
		ret = -1;
		goto end;
    }

    ret = fwrite(bloomfilter->data, 1, bloomfilter->bytes, fp);
    if ((uint32_t)ret != bloomfilter->bytes) {
		ret = -1;
        goto end;
    }

	ret = 0;

end:
	flock(fd, LOCK_UN);
	fclose(fp);

    return ret;
}
