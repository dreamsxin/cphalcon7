#ifndef PHALCON_STORAGE_BTREE_PAGES_H_
#define PHALCON_STORAGE_BTREE_PAGES_H_

#include "storage/btree/private/tree.h"
#include "storage/btree/private/values.h"

typedef struct _phalcon_storage_btree_page_s _phalcon_storage_btree_page_t;
typedef struct _phalcon_storage_btree_page_search_res_s _phalcon_storage_btree_page_search_res_t;

enum page_type {
    kPage = 0,
    kLeaf = 1
};

enum search_type {
    kNotLoad = 0,
    kLoad = 1
};

int _phalcon_storage_btree_page_create(phalcon_storage_btree_db_t *t,
                    const enum page_type type,
                    const uint64_t offset,
                    const uint64_t config,
                    _phalcon_storage_btree_page_t **page);
void _phalcon_storage_btree_page_destroy(phalcon_storage_btree_db_t *t, _phalcon_storage_btree_page_t *page);
int _phalcon_storage_btree_page_clone(phalcon_storage_btree_db_t *t, _phalcon_storage_btree_page_t *page, _phalcon_storage_btree_page_t **clone);

int _phalcon_storage_btree_page_read(phalcon_storage_btree_db_t *t, _phalcon_storage_btree_page_t *page);
int _phalcon_storage_btree_page_load(phalcon_storage_btree_db_t *t,
                  const uint64_t offset,
                  const uint64_t config,
                  _phalcon_storage_btree_page_t **page);
int _phalcon_storage_btree_page_save(phalcon_storage_btree_db_t *t, _phalcon_storage_btree_page_t *page);

int _phalcon_storage_btree_page_load_value(phalcon_storage_btree_db_t *t,
                        _phalcon_storage_btree_page_t *page,
                        const uint64_t index,
                        phalcon_storage_btree_value_t *value);
int _phalcon_storage_btree_page_save_value(phalcon_storage_btree_db_t *t,
                        _phalcon_storage_btree_page_t *page,
                        const uint64_t index,
                        const int cmp,
                        const phalcon_storage_btree_key_t *key,
                        const phalcon_storage_btree_value_t *value,
                        phalcon_storage_btree_update_cb cb,
                        void *arg);

int _phalcon_storage_btree_page_search(phalcon_storage_btree_db_t *t,
                    _phalcon_storage_btree_page_t *page,
                    const phalcon_storage_btree_key_t *key,
                    const enum search_type type,
                    _phalcon_storage_btree_page_search_res_t *result);
int _phalcon_storage_btree_page_get(phalcon_storage_btree_db_t *t,
                 _phalcon_storage_btree_page_t *page,
                 const phalcon_storage_btree_key_t *key,
                 phalcon_storage_btree_value_t *value);
int _phalcon_storage_btree_page_get_range(phalcon_storage_btree_db_t *t,
                       _phalcon_storage_btree_page_t *page,
                       const phalcon_storage_btree_key_t *start,
                       const phalcon_storage_btree_key_t *end,
                       phalcon_storage_btree_filter_cb filter,
                       phalcon_storage_btree_range_cb cb,
                       void *arg);
int _phalcon_storage_btree_page_insert(phalcon_storage_btree_db_t *t,
                    _phalcon_storage_btree_page_t *page,
                    const phalcon_storage_btree_key_t *key,
                    const phalcon_storage_btree_value_t *value,
                    phalcon_storage_btree_update_cb update_cb,
                    void *arg);
int _phalcon_storage_btree_page_bulk_insert(phalcon_storage_btree_db_t *t,
                         _phalcon_storage_btree_page_t *page,
                         const phalcon_storage_btree_key_t *limit,
                         uint64_t *count,
                         phalcon_storage_btree_key_t **keys,
                         phalcon_storage_btree_value_t **values,
                         phalcon_storage_btree_update_cb update_cb,
                         void *arg);
int _phalcon_storage_btree_page_remove(phalcon_storage_btree_db_t *t,
                    _phalcon_storage_btree_page_t *page,
                    const phalcon_storage_btree_key_t *key,
                    phalcon_storage_btree_remove_cb remove_cb,
                    void *arg);
int _phalcon_storage_btree_page_copy(phalcon_storage_btree_db_t *source, phalcon_storage_btree_db_t *target, _phalcon_storage_btree_page_t *page);

int _phalcon_storage_btree_page_remove_idx(phalcon_storage_btree_db_t *t, _phalcon_storage_btree_page_t *page, const uint64_t index);
int _phalcon_storage_btree_page_split(phalcon_storage_btree_db_t *t,
                   _phalcon_storage_btree_page_t *parent,
                   const uint64_t index,
                   _phalcon_storage_btree_page_t *child);
int _phalcon_storage_btree_page_split_head(phalcon_storage_btree_db_t *t, _phalcon_storage_btree_page_t **page);

void _phalcon_storage_btree_page_shiftr(phalcon_storage_btree_db_t *t, _phalcon_storage_btree_page_t *page, const uint64_t index);
void _phalcon_storage_btree_page_shiftl(phalcon_storage_btree_db_t *t, _phalcon_storage_btree_page_t *page, const uint64_t index);

struct _phalcon_storage_btree_page_s {
    enum page_type type;

    uint64_t length;
    uint64_t byte_size;

    uint64_t offset;
    uint64_t config;

    void *buff_;
    int is_head;

    _phalcon_storage_btree_kv_t keys[1];
};

struct _phalcon_storage_btree_page_search_res_s {
    _phalcon_storage_btree_page_t *child;

    uint64_t index;
    int cmp;
};

#endif /* PHALCON_STORAGE_BTREE_PAGES_H_ */
