
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2015 Phalcon Team (http://www.phalconphp.com)       |
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

#ifndef PHALCON_KERNEL_MEMORY_POOL_H
#define PHALCON_KERNEL_MEMORY_POOL_H

#include "kernel/memory.h"

#include <stddef.h>
#include <stdint.h>

static const size_t BOM = 0xfafafafafafafafa;

typedef struct _phalcon_memory_pool {
    size_t bom;
    size_t size;
    size_t balance;
    size_t ntags;
    phalcon_memory_void_value bits;
    phalcon_memory_void_value tags;
    phalcon_memory_void_value freetag;
} phalcon_memory_pool;

typedef struct _phalcon_memory_pool_tag {
  size_t size;
  phalcon_memory_void_value link;
} phalcon_memory_pool_tag;

static inline void* phalcon_memory_pool_tag_tomem(phalcon_memory_pool_tag* tag) {
  return (void*)&tag->link;
}

static inline phalcon_memory_pool_tag* phalcon_memory_pool_tag_ofmem(void* mem) {
  return (phalcon_memory_pool_tag *)((char*)mem - sizeof(size_t));
}

static inline phalcon_memory_pool_tag* phalcon_memory_pool_tag_next(phalcon_memory_pool_tag* tag) {
  return (phalcon_memory_pool_tag *)phalcon_memory_void_get(&tag->link);
}

static inline void phalcon_memory_pool_tag_link(phalcon_memory_pool_tag* tag, phalcon_memory_pool_tag* link) {
  phalcon_memory_void_set(&tag->link, link);
}


static size_t phalcon_memory_pool_size_aligned(size_t need) {
  size_t const ntags = (1 + (need + sizeof(size_t) - 1) / sizeof(phalcon_memory_pool_tag));
  return sizeof(phalcon_memory_pool_tag) * ntags;
}

static inline size_t phalcon_memory_pool_size_bits(size_t ntags)
{
  return (ntags + 7) / 8;
}

static inline void phalcon_memory_pool_bits_set(phalcon_memory_pool_tag* tag, phalcon_memory_pool* p) {
  uint8_t* const bits = phalcon_memory_void_get(&p->bits);
  size_t const bit = tag - (phalcon_memory_pool_tag*)phalcon_memory_void_get(&p->tags);
  bits[bit / 8] |= 1 << (bit % 8);
}

static inline void phalcon_memory_pool_bits_drop(phalcon_memory_pool_tag* tag, phalcon_memory_pool* p) {
  uint8_t* const bits = phalcon_memory_void_get(&p->bits);
  size_t const bit = tag - (phalcon_memory_pool_tag*)phalcon_memory_void_get(&p->tags);
  bits[bit / 8] &= ~(1 << (bit % 8));
}

static phalcon_memory_pool_tag* phalcon_memory_pool_tag_left(phalcon_memory_pool_tag* tag, phalcon_memory_pool* p)
{
  phalcon_memory_pool_tag* tags = phalcon_memory_void_get(&p->tags);
  uint8_t* bits = phalcon_memory_void_get(&p->bits);
  size_t bit = tag - tags;
  while (bit --> 0) {
    if (bits[bit / 8]) {
       if (1 << (bit % 8) == (bits[bit / 8] & (1 << (bit % 8))))
         return tags + bit;
    }
  }
  return NULL;
}

static void phalcon_memory_pool_tag_merge(phalcon_memory_pool_tag* tag, phalcon_memory_pool* p)
{
  phalcon_memory_pool_tag* n;
  while ((n = phalcon_memory_pool_tag_next(tag))) {

    if ((char*)n > (char*)tag + tag->size)
      break;

    tag->size += n->size;
    phalcon_memory_pool_tag_link(tag, phalcon_memory_pool_tag_next(n));
    phalcon_memory_pool_bits_drop(n, p);
  }

  phalcon_memory_pool_bits_set(tag, p);
}

size_t phalcon_memory_pool_size_hint(size_t itemsize, size_t nitem);
size_t phalcon_memory_pool_size_stuff(size_t memsize);

phalcon_memory_pool* phalcon_memory_pool_attach(void* src);
phalcon_memory_pool* phalcon_memory_pool_format(void* src, size_t nsrc);
void phalcon_memory_pool_clear(phalcon_memory_pool* p);
size_t phalcon_memory_pool_memory_size(phalcon_memory_pool* p);
size_t phalcon_memory_pool_capacity(phalcon_memory_pool* p);
size_t phalcon_memory_pool_balance(phalcon_memory_pool* p);
size_t phalcon_memory_pool_avail(phalcon_memory_pool* p);
double phalcon_memory_pool_load(phalcon_memory_pool* p);
void* phalcon_memory_pool_alloc(phalcon_memory_pool* p, size_t size);
void* phalcon_memory_pool_realloc(phalcon_memory_pool* p, void* ptr, size_t newsz);
void* phalcon_memory_pool_zalloc(phalcon_memory_pool* p, size_t);
void  phalcon_memory_pool_free(phalcon_memory_pool* p, void* ptr);
void* phalcon_memory_pool_memdup(phalcon_memory_pool* p, void* ptr, size_t dsz);

#endif /* PHALCON_KERNEL_MEMORY_POOL_H */
