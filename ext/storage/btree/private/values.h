#ifndef PHALCON_STORAGE_BTREE_VALUES_H_
#define PHALCON_STORAGE_BTREE_VALUES_H_

#include "storage/btree/private/tree.h"
#include <stdint.h>

#define PHALCON_STORAGE_BTREE__KV_HEADER_SIZE 24
#define PHALCON_STORAGE_BTREE__KV_SIZE(kv)  PHALCON_STORAGE_BTREE__KV_HEADER_SIZE + kv.length
#define PHALCON_STORAGE_BTREE__STOVAL(str, key)		\
    key.value = (char *) str;           \
    key.length = strlen(str) + 1;

#define PHALCON_STORAGE_BTREE_KEY_PRIVATE                  \
    uint64_t _prev_offset;              \
    uint64_t _prev_length;

typedef struct _phalcon_storage_btree_kv_s _phalcon_storage_btree_kv_t;


int _phalcon_storage_btree_value_load(phalcon_storage_btree_db_t *t,
                   const uint64_t offset,
                   const uint64_t length,
                   phalcon_storage_btree_value_t *value);
int _phalcon_storage_btree_value_save(phalcon_storage_btree_db_t *t,
                   const phalcon_storage_btree_value_t *value,
                   const _phalcon_storage_btree_kv_t *previous,
                   uint64_t *offset,
                   uint64_t *length);

int _phalcon_storage_btree_kv_copy(const _phalcon_storage_btree_kv_t *source, _phalcon_storage_btree_kv_t *target, int alloc);

struct _phalcon_storage_btree_kv_s {
    PHALCON_STORAGE_BTREE_KEY_FIELDS

    uint64_t offset;
    uint64_t config;

    uint8_t allocated;
};

#endif /* PHALCON_STORAGE_BTREE_VALUES_H_ */
