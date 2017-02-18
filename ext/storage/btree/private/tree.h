#ifndef PHALCON_STORAGE_BTREE_TREE_H_
#define PHALCON_STORAGE_BTREE_TREE_H_

#include "storage/btree/private/writer.h"
#include "storage/btree/private/pages.h"

#include <pthread.h>

#define PHALCON_STORAGE_BTREE__HEAD_SIZE  sizeof(uint64_t) * 4

#define PHALCON_STORAGE_BTREE_TREE_PRIVATE         \
    PHALCON_STORAGE_BTREE_WRITER_PRIVATE           \
    pthread_rwlock_t rwlock;    \
    _phalcon_storage_btree_tree_head_t head;       \
    phalcon_storage_btree_compare_cb compare_cb;

typedef struct _phalcon_storage_btree_tree_head_s _phalcon_storage_btree_tree_head_t;

int _phalcon_storage_btree_init(phalcon_storage_btree_db_t *tree);
void _phalcon_storage_btree_destroy(phalcon_storage_btree_db_t *tree);

int _phalcon_storage_btree_tree_read_head(_phalcon_storage_btree_writer_t *w, void *data);
int _phalcon_storage_btree_tree_write_head(_phalcon_storage_btree_writer_t *w, void *data);

int _phalcon_storage_btree_default_compare_cb(const phalcon_storage_btree_key_t *a, const phalcon_storage_btree_key_t *b);
int _phalcon_storage_btree_default_filter_cb(void *arg, const phalcon_storage_btree_key_t *key);


struct _phalcon_storage_btree_tree_head_s {
    uint64_t offset;
    uint64_t config;
    uint64_t page_size;
    uint64_t hash;

    _phalcon_storage_btree_page_t *page;
};

#endif /* PHALCON_STORAGE_BTREE_TREE_H_ */
