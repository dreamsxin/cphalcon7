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
   | Author: Xinchen Hui <laruence@php.net>                               |
   +----------------------------------------------------------------------+
*/

#ifndef PHALCON_CACHE_YAC_STORAGE_H
#define PHALCON_CACHE_YAC_STORAGE_H

#define PHALCON_CACHE_YAC_STORAGE_MAX_ENTRY_LEN  	(1 << 20)
#define PHALCON_CACHE_YAC_STORAGE_MAX_KEY_LEN		(48)
#define PHALCON_CACHE_YAC_STORAGE_FACTOR 			(1.25)
#define PHALCON_CACHE_YAC_KEY_KLEN_MASK			    (255)
#define PHALCON_CACHE_YAC_KEY_VLEN_BITS			    (8)
#define PHALCON_CACHE_YAC_KEY_KLEN(k)				((k).len & PHALCON_CACHE_YAC_KEY_KLEN_MASK)
#define PHALCON_CACHE_YAC_KEY_VLEN(k)				((k).len >> PHALCON_CACHE_YAC_KEY_VLEN_BITS)
#define PHALCON_CACHE_YAC_KEY_SET_LEN(k, kl, vl)	    ((k).len = (vl << PHALCON_CACHE_YAC_KEY_VLEN_BITS) | (kl & PHALCON_CACHE_YAC_KEY_KLEN_MASK))
#define PHALCON_CACHE_YAC_FULL_CRC_THRESHOLD         256

typedef struct {
	unsigned long atime;
	unsigned int len;
	char data[1];
} phalcon_cache_yac_kv_val;

typedef struct {
	unsigned long h;
	unsigned long crc;
	unsigned int ttl;
	unsigned int len;
	unsigned int flag;
	unsigned int size;
	phalcon_cache_yac_kv_val *val;
	unsigned char key[PHALCON_CACHE_YAC_STORAGE_MAX_KEY_LEN];
} phalcon_cache_yac_kv_key;

typedef struct _phalcon_cache_yac_item_list {
	unsigned int index;
	unsigned long h;
	unsigned long crc;
	unsigned int ttl;
	unsigned int k_len;
	unsigned int v_len;
	unsigned int flag;
	unsigned int size;
	unsigned char key[PHALCON_CACHE_YAC_STORAGE_MAX_KEY_LEN];
	struct _phalcon_cache_yac_item_list *next;
} phalcon_cache_yac_item_list;

typedef struct {
	volatile unsigned int pos;
	unsigned int size;
	void *p;
} phalcon_cache_yac_shared_segment;

typedef struct {
	unsigned long k_msize;
	unsigned long v_msize;
	unsigned int segments_num;
	unsigned int segment_size;
	unsigned int slots_num;
	unsigned int slots_size;
	unsigned int miss;
	unsigned int fails;
	unsigned int kicks;
	unsigned int recycles;
	unsigned long hits;
} phalcon_cache_yac_storage_info;

typedef struct {
	phalcon_cache_yac_kv_key  *slots;
	unsigned int slots_mask;
	unsigned int slots_num;
	unsigned int slots_size;
	unsigned int miss;
	unsigned int fails;
	unsigned int kicks;
	unsigned int recycles;
	unsigned long hits;
	phalcon_cache_yac_shared_segment **segments;
	unsigned int segments_num;
	unsigned int segments_num_mask;
	phalcon_cache_yac_shared_segment first_seg;
} phalcon_cache_yac_storage_globals;

extern phalcon_cache_yac_storage_globals *phalcon_cache_yac_storage;

#define PHALCON_CACHE_YAC_SG(element) (phalcon_cache_yac_storage->element)

int phalcon_cache_yac_storage_startup(unsigned long first_size, unsigned long size, char **err);
void phalcon_cache_yac_storage_shutdown(void);
int phalcon_cache_yac_storage_find(char *key, unsigned int len, char **data, unsigned int *size, unsigned int *flag, unsigned long tv);
int phalcon_cache_yac_storage_update(char *key, unsigned int len, char *data, unsigned int size, unsigned int falg, int ttl, int add, unsigned long tv);
void phalcon_cache_yac_storage_delete(char *key, unsigned int len, int ttl, unsigned long tv);
void phalcon_cache_yac_storage_flush(void);
const char * phalcon_cache_yac_storage_shared_yac_name(void);
phalcon_cache_yac_storage_info * phalcon_cache_yac_storage_get_info(void);
void phalcon_cache_yac_storage_free_info(phalcon_cache_yac_storage_info *info);
phalcon_cache_yac_item_list * phalcon_cache_yac_storage_dump(unsigned int limit);
void phalcon_cache_yac_storage_free_list(phalcon_cache_yac_item_list *list);
#define phalcon_cache_yac_storage_exists(ht, key, len)  phalcon_cache_yac_storage_find(ht, key, len, NULL)

#endif	/* PHALCON_CACHE_YAC_STORAGE_H */
