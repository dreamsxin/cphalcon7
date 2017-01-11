
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

#include "kernel/mpool.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

size_t phalcon_memory_pool_size_hint(size_t itemsize, size_t nitem)
{
	size_t const tagssize = phalcon_memory_pool_size_aligned(itemsize) * nitem;
	size_t const bitssize = phalcon_memory_pool_size_bits(tagssize / sizeof(phalcon_memory_pool_tag));
	return tagssize + bitssize + sizeof(phalcon_memory_pool);
}

size_t phalcon_memory_pool_size_stuff(size_t memsize)
{
	size_t const asize = memsize - phalcon_memory_pool_size_hint(0, 0);
	size_t ntags = asize / sizeof(phalcon_memory_pool_tag);
	while (phalcon_memory_pool_size_bits(ntags)	+ ntags * sizeof(phalcon_memory_pool_tag) > asize) {
		--ntags;
	}
	return memsize - (ntags * sizeof(phalcon_memory_pool_tag) - sizeof(size_t));
}

phalcon_memory_pool* phalcon_memory_pool_attach(void* src)
{
	phalcon_memory_pool* p = (phalcon_memory_pool*) src;
	if (BOM == (BOM & p->bom)) return p;
	fprintf(stderr, "%s invalid bom marker\n", __func__);
	return NULL;
}

phalcon_memory_pool* phalcon_memory_pool_format(void* src, size_t nsrc)
{
	if (nsrc < phalcon_memory_pool_size_hint(0, 0) ) {
		fprintf(stderr, "%s need %zu or more memory \n", __func__,
						phalcon_memory_pool_size_hint(0, 0));
		return NULL;
	}

	size_t const asize = nsrc - phalcon_memory_pool_size_hint(0, 0);
	size_t ntags = asize / sizeof(phalcon_memory_pool_tag);
	while (phalcon_memory_pool_size_bits(ntags)	+ ntags * sizeof(phalcon_memory_pool_tag) > asize)
		--ntags;

	phalcon_memory_pool* p = (phalcon_memory_pool*) src;
	memset(p, 0, sizeof(*p));

	p->bom = BOM;
	p->size = nsrc;
	p->ntags = ntags;
	phalcon_memory_pool_clear(p);
	return p;
}

void phalcon_memory_pool_clear(phalcon_memory_pool* p)
{
	phalcon_memory_void_set(&p->bits, p + 1);
	uint8_t* bits = phalcon_memory_void_get(&p->bits);
	memset(bits, 0, phalcon_memory_pool_size_bits(p->ntags));

	phalcon_memory_void_set(&p->tags, bits + phalcon_memory_pool_size_bits(p->ntags));
	phalcon_memory_void_set(&p->freetag, phalcon_memory_void_get(&p->tags));

	phalcon_memory_pool_tag* tag = phalcon_memory_void_get(&p->freetag);
	tag->size = p->ntags * sizeof(phalcon_memory_pool_tag);
	phalcon_memory_pool_tag_link(tag, NULL);
}

size_t phalcon_memory_pool_memory_size(phalcon_memory_pool* p)
{
	return p->size;
}

size_t phalcon_memory_pool_capacity(phalcon_memory_pool* p)
{
	if (!p->ntags) return 0;
	return p->ntags * sizeof(phalcon_memory_pool_tag) - sizeof(size_t);
}

size_t phalcon_memory_pool_balance(phalcon_memory_pool* p)
{
	return p->balance;
}

double phalcon_memory_pool_load(phalcon_memory_pool* p)
{
	if (!p->ntags) return 100.;
	return (double)p->balance / (p->ntags * sizeof(phalcon_memory_pool_tag));
}

size_t phalcon_memory_pool_avail(phalcon_memory_pool* p)
{
		size_t avail = 0;
		phalcon_memory_pool_tag* tag = phalcon_memory_void_get(&p->freetag);
		while (tag != NULL) {
			avail += (tag->size - sizeof(size_t));
			tag = phalcon_memory_pool_tag_next(tag);
		}
		return avail;
}

void phalcon_memory_pool_dump(phalcon_memory_pool* p)
{
	size_t const avail = phalcon_memory_pool_avail(p);
	fprintf(stderr, "> page [%p] size: %zu tags: %zu capacity: %zu avail: %zu used: %zu balance: %zu load: %f\n",
		(void*)p, p->size, p->ntags, phalcon_memory_pool_capacity(p), avail, phalcon_memory_pool_capacity(p) - avail,
		p->balance, phalcon_memory_pool_load(p));
}

void* phalcon_memory_pool_alloc(phalcon_memory_pool* p, size_t size)
{
	size_t const aligned = phalcon_memory_pool_size_aligned(size);

	phalcon_memory_pool_tag* prev = NULL;
	phalcon_memory_pool_tag* tag = phalcon_memory_void_get(&p->freetag);
	while (tag && tag->size < aligned) {
		prev = tag;
		tag = phalcon_memory_pool_tag_next(tag);
	}

	if (!tag) {
		fprintf(stderr, "%s no memory\n", __func__);
		return NULL;
	}

	if (tag->size > aligned) {
		phalcon_memory_pool_tag* n = (phalcon_memory_pool_tag*)((char*)tag + aligned);
		n->size = tag->size - aligned;
		tag->size = aligned;
		phalcon_memory_pool_tag_link(n, phalcon_memory_pool_tag_next(tag));
		phalcon_memory_pool_tag_link(tag, n);
		phalcon_memory_pool_tag_merge(n, p);
	}

	if (prev) {
		phalcon_memory_pool_tag_link(prev, phalcon_memory_pool_tag_next(tag));
	} else {
		phalcon_memory_void_set(&p->freetag, phalcon_memory_pool_tag_next(tag));
	}

	phalcon_memory_pool_bits_drop(tag, p);
	p->balance += aligned;
	return phalcon_memory_pool_tag_tomem(tag);
}

void* phalcon_memory_pool_realloc(phalcon_memory_pool* p, void* ptr, size_t newsz)
{
	if (!ptr) return phalcon_memory_pool_alloc(p, newsz);

	size_t const aligned = phalcon_memory_pool_size_aligned(newsz);
	phalcon_memory_pool_tag* tag = phalcon_memory_pool_tag_ofmem(ptr);

	if (aligned == tag->size) {
		return ptr;
	}

	if (tag->size < aligned) {
		void* np = phalcon_memory_pool_alloc(p, newsz);
		if (!np) return NULL;
		memcpy(np, ptr, tag->size - sizeof(size_t));
		phalcon_memory_pool_free(p, ptr);
		return np;
	}

	phalcon_memory_pool_tag* n = (phalcon_memory_pool_tag*)((char*)tag + aligned);
	n->size = tag->size - aligned;
	tag->size = aligned;

	if (p->balance < n->size) {
		fprintf(stderr, "%s pool balance error\n", __func__);
		return NULL;
	}

	p->balance -= n->size;

	phalcon_memory_pool_tag* tmp = phalcon_memory_void_get(&p->freetag);

	if (!tmp || tmp > n) {
		phalcon_memory_void_set(&p->freetag, n);
		phalcon_memory_pool_tag_link(n, tmp);
		phalcon_memory_pool_tag_merge(n, p);
		return ptr;
	}

	tmp = phalcon_memory_pool_tag_left(tag, p);

	if (!tmp) {
		fprintf(stderr, "%s bits error \n", __func__);
		return NULL;
	}

	phalcon_memory_pool_tag_link(n, phalcon_memory_pool_tag_next(tmp));
	phalcon_memory_pool_tag_link(tmp, n);

	phalcon_memory_pool_tag_merge(n, p);
	phalcon_memory_pool_tag_merge(tmp, p);
	return ptr;
}

void* phalcon_memory_pool_zalloc(phalcon_memory_pool* p, size_t nbytes)
{
	void* r = phalcon_memory_pool_alloc(p, nbytes);
	if (r) memset(r, 0, nbytes);
	return r;
}

void phalcon_memory_pool_free(phalcon_memory_pool* p, void* ptr)
{
	if (!ptr) {
		return;
	}

	phalcon_memory_pool_tag* tag = phalcon_memory_pool_tag_ofmem(ptr);

	if (p->balance < tag->size) {
		fprintf(stderr, "%s pool balance error\n", __func__);
		return;
	}

	p->balance -= tag->size;

	phalcon_memory_pool_tag* tmp = phalcon_memory_void_get(&p->freetag);

	if (!tmp || tmp > tag) {
		phalcon_memory_pool_tag_link(tag, tmp);
		phalcon_memory_pool_tag_merge(tag, p);
		phalcon_memory_void_set(&p->freetag, tag);
		return;
	}

	tmp = phalcon_memory_pool_tag_left(tag, p);

	if (!tmp) {
		fprintf(stderr, "%s bits error \n", __func__);
		return;
	}

	phalcon_memory_pool_tag_link(tag, phalcon_memory_pool_tag_next(tmp));
	phalcon_memory_pool_tag_link(tmp, tag);

	phalcon_memory_pool_tag_merge(tag, p);
	phalcon_memory_pool_tag_merge(tmp, p);
}

void* phalcon_memory_pool_memdup(phalcon_memory_pool* p, void* d, size_t dsz)
{
	void* r = phalcon_memory_pool_alloc(p, dsz);
	if (r) {
		memcpy(r, d, dsz);
	}
	return r;
}
