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
   | Authors: Xinchen Hui <laruence@php.net>                              |
   +----------------------------------------------------------------------+
*/

#include <errno.h>
#include <time.h>
#include <sys/types.h>

#include "php.h"
#include "cache/yac/storage.h"
#include "cache/yac/allocator.h"

int phalcon_cache_yac_allocator_startup(unsigned long k_size, unsigned long size, char **msg) /* {{{ */ {
	char *p;
	phalcon_cache_yac_shared_segment *segments = NULL;
	int i, segments_num, segments_array_size, segment_size;
	const phalcon_cache_yac_shared_yac_handlers *he;

	if ((he = &phalcon_cache_yac_shared_yac_handler)) {
		int ret = he->create_segments(k_size, size, &segments, &segments_num, msg);

		if (!ret) {
			if (segments) {
				int i;
				for (i = 0; i < segments_num; i++) {
					if (segments[i].p && segments[i].p != (void *)-1) {
						he->detach_segment(&segments[i]);
					}
				}
				free(segments);
			}
			return 0;
		}
	} else {
		return 0;
	}

	segment_size = he->segment_type_size();
	segments_array_size = (segments_num - 1) * segment_size;

	phalcon_cache_yac_storage = segments[0].p;
	memcpy(&PHALCON_CACHE_YAC_SG(first_seg), (char *)(&segments[0]), segment_size);

	PHALCON_CACHE_YAC_SG(segments_num) 		= segments_num - 1;
	PHALCON_CACHE_YAC_SG(segments_num_mask) 	= PHALCON_CACHE_YAC_SG(segments_num) - 1;
	PHALCON_CACHE_YAC_SG(segments)     		= (phalcon_cache_yac_shared_segment **)((char *)phalcon_cache_yac_storage + PHALCON_CACHE_YAC_SMM_ALIGNED_SIZE(sizeof(phalcon_cache_yac_storage_globals) + segment_size - sizeof(phalcon_cache_yac_shared_segment)));

	p = (char *)PHALCON_CACHE_YAC_SG(segments) + (sizeof(void *) * PHALCON_CACHE_YAC_SG(segments_num));
	memcpy(p, (char *)segments + segment_size, segments_array_size);
	for (i = 0; i < PHALCON_CACHE_YAC_SG(segments_num); i++) {
		PHALCON_CACHE_YAC_SG(segments)[i] = (phalcon_cache_yac_shared_segment *)p;
		p += segment_size;
	}
	PHALCON_CACHE_YAC_SG(slots) = (phalcon_cache_yac_kv_key *)((char *)PHALCON_CACHE_YAC_SG(segments)
			+ (PHALCON_CACHE_YAC_SG(segments_num) * sizeof(void *)) + PHALCON_CACHE_YAC_SMM_ALIGNED_SIZE(segments_array_size));

	free(segments);

	return 1;
}
/* }}} */

void phalcon_cache_yac_allocator_shutdown(void) /* {{{ */ {
	phalcon_cache_yac_shared_segment **segments;
	const phalcon_cache_yac_shared_yac_handlers *he;

	segments = PHALCON_CACHE_YAC_SG(segments);
	if (segments) {
		if ((he = &phalcon_cache_yac_shared_yac_handler)) {
			int i = 0;
			for (i = 0; i < PHALCON_CACHE_YAC_SG(segments_num); i++) {
				he->detach_segment(segments[i]);
			}
			he->detach_segment(&PHALCON_CACHE_YAC_SG(first_seg));
		}
	}
}
/* }}} */

static inline void *phalcon_cache_yac_allocator_alloc_algo2(unsigned long size, int hash) /* {{{ */ {
	phalcon_cache_yac_shared_segment *segment;
	unsigned int seg_size, retry, pos, current;

	current = hash & PHALCON_CACHE_YAC_SG(segments_num_mask);
	/* do we really need lock here? it depends the real life exam */
	retry = 3;
do_retry:
	segment = PHALCON_CACHE_YAC_SG(segments)[current];
	seg_size = segment->size;
	pos = segment->pos;
	if ((seg_size - pos) >= size) {
do_alloc:
		pos += size;
		segment->pos = pos;
		if (segment->pos == pos) {
			return (void *)((char *)segment->p + (pos - size));
		} else if (retry--) {
			goto do_retry;
		}
		return NULL;
    } else {
		int i, max;
		max = (PHALCON_CACHE_YAC_SG(segments_num) > 4)? 4 : PHALCON_CACHE_YAC_SG(segments_num);
		for (i = 1; i < max; i++) {
			segment = PHALCON_CACHE_YAC_SG(segments)[(current + i) & PHALCON_CACHE_YAC_SG(segments_num_mask)];
			seg_size = segment->size;
			pos = segment->pos;
			if ((seg_size - pos) >= size) {
				current = (current + i) & PHALCON_CACHE_YAC_SG(segments_num_mask);
				goto do_alloc;
			}
		}
		segment->pos = 0;
		pos = 0;
		++PHALCON_CACHE_YAC_SG(recycles);
		goto do_alloc;
	}
}
/* }}} */

#if 0
static inline void *phalcon_cache_yac_allocator_alloc_algo1(unsigned long size) /* {{{ */ {
    int i, j, picked_seg, atime;
    picked_seg = (PHALCON_CACHE_YAC_SG(current_seg) + 1) & PHALCON_CACHE_YAC_SG(segments_num_mask);

    atime = PHALCON_CACHE_YAC_SG(segments)[picked_seg]->atime;
    for (i = 0; i < 10; i++) {
        j = (picked_seg + 1) & PHALCON_CACHE_YAC_SG(segments_num_mask);
        if (PHALCON_CACHE_YAC_SG(segments)[j]->atime < atime) {
            picked_seg = j;
            atime = PHALCON_CACHE_YAC_SG(segments)[j]->atime;
        }
    }

    PHALCON_CACHE_YAC_SG(current_seg) = picked_seg;
    PHALCON_CACHE_YAC_SG(segments)[picked_seg]->pos = 0;
    return phalcon_cache_yac_allocator_alloc_algo2(size);
}
/* }}} */
#endif

unsigned long phalcon_cache_yac_allocator_real_size(unsigned long size) /* {{{ */ {
	unsigned long real_size = PHALCON_CACHE_YAC_SMM_TRUE_SIZE(size);

	if (real_size > PHALCON_CACHE_YAC_SG(segments)[0]->size) {
		return 0;
	}

	return real_size;
}
/* }}} */

void * phalcon_cache_yac_allocator_raw_alloc(unsigned long real_size, int hash) /* {{{ */ {

	return phalcon_cache_yac_allocator_alloc_algo2(real_size, hash);
	/*
    if (PHALCON_CACHE_YAC_SG(exhausted)) {
        return phalcon_cache_yac_allocator_alloc_algo1(real_size);
    } else {
        void *p;
        if ((p = phalcon_cache_yac_allocator_alloc_algo2(real_size))) {
            return p;
        }
        return phalcon_cache_yac_allocator_alloc_algo1(real_size);
    }
	*/
}
/* }}} */

#if 0
void phalcon_cache_yac_allocator_touch(void *p, unsigned long atime) /* {{{ */ {
	phalcon_cache_yac_shared_block_header h = *(phalcon_cache_yac_shared_block_header *)(p - sizeof(phalcon_cache_yac_shared_block_header));

	if (h.seg >= PHALCON_CACHE_YAC_SG(segments_num)) {
		return;
	}

	PHALCON_CACHE_YAC_SG(segments)[h.seg]->atime = atime;
}
/* }}} */
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
