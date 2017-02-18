#ifndef _PHALCON_STORAGE_BTREE_BPLUS_H_
#define _PHALCON_STORAGE_BTREE_BPLUS_H_

#define PHALCON_STORAGE_BTREE_PADDING 64

#define PHALCON_STORAGE_BTREE_KEY_FIELDS   \
    uint64_t length;    \
    char *value;

#include <stdint.h> /* uintx_t */
#include "storage/btree/private/errors.h"

typedef struct phalcon_storage_btree_db_s phalcon_storage_btree_db_t;

typedef struct phalcon_storage_btree_key_s phalcon_storage_btree_key_t;
typedef struct phalcon_storage_btree_key_s phalcon_storage_btree_value_t;

typedef int (*phalcon_storage_btree_compare_cb)(const phalcon_storage_btree_key_t *a, const phalcon_storage_btree_key_t *b);
typedef int (*phalcon_storage_btree_update_cb)(void *arg,
                            const phalcon_storage_btree_value_t *previous,
                            const phalcon_storage_btree_value_t *value);
typedef int (*phalcon_storage_btree_remove_cb)(void *arg,
                            const phalcon_storage_btree_value_t *value);
typedef void (*phalcon_storage_btree_range_cb)(void *arg,
                            const phalcon_storage_btree_key_t *key,
                            const phalcon_storage_btree_value_t *value);
typedef int (*phalcon_storage_btree_filter_cb)(void* arg, const phalcon_storage_btree_key_t *key);

#include "private/tree.h"

/*
 * Open and close database
 */
int phalcon_storage_btree_open(phalcon_storage_btree_db_t *tree, const char *filename);
int phalcon_storage_btree_close(phalcon_storage_btree_db_t *tree);

/*
 * Get one value by key
 */
int phalcon_storage_btree_get(phalcon_storage_btree_db_t *tree, const phalcon_storage_btree_key_t *key, phalcon_storage_btree_value_t *value);
int phalcon_storage_btree_gets(phalcon_storage_btree_db_t *tree, const char *key, char **value);

/*
 * Get previous value
 */
int phalcon_storage_btree_get_previous(phalcon_storage_btree_db_t *tree,
                    const phalcon_storage_btree_value_t *value,
                    phalcon_storage_btree_value_t *previous);

/*
 * Set one value by key (without solving conflicts, overwrite)
 */
int phalcon_storage_btree_set(phalcon_storage_btree_db_t *tree,
           const phalcon_storage_btree_key_t *key,
           const phalcon_storage_btree_value_t *value);
int phalcon_storage_btree_sets(phalcon_storage_btree_db_t *tree,
            const char *key,
            const char *value);

/*
 * Update or create value by key (with solving conflicts)
 */
int phalcon_storage_btree_update(phalcon_storage_btree_db_t *tree,
              const phalcon_storage_btree_key_t *key,
              const phalcon_storage_btree_value_t *value,
              phalcon_storage_btree_update_cb update_cb,
              void *arg);
int phalcon_storage_btree_updates(phalcon_storage_btree_db_t *tree,
               const char *key,
               const char *value,
               phalcon_storage_btree_update_cb update_cb,
               void *arg);

/*
 * Set multiple values by keys
 */
int phalcon_storage_btree_bulk_set(phalcon_storage_btree_db_t *tree,
                const uint64_t count,
                const phalcon_storage_btree_key_t **keys,
                const phalcon_storage_btree_value_t **values);
int phalcon_storage_btree_bulk_sets(phalcon_storage_btree_db_t *tree,
                 const uint64_t count,
                 const char **keys,
                 const char **values);

/*
 * Update multiple values by keys
 */
int phalcon_storage_btree_bulk_update(phalcon_storage_btree_db_t *tree,
                   const uint64_t count,
                   const phalcon_storage_btree_key_t **keys,
                   const phalcon_storage_btree_value_t **values,
                   phalcon_storage_btree_update_cb update_cb,
                   void *arg);
int phalcon_storage_btree_bulk_updates(phalcon_storage_btree_db_t *tree,
                    const uint64_t count,
                    const char **keys,
                    const char **values,
                    phalcon_storage_btree_update_cb update_cb,
                    void *arg);

/*
 * Remove one value by key
 */
int phalcon_storage_btree_remove(phalcon_storage_btree_db_t *tree, const phalcon_storage_btree_key_t *key);
int phalcon_storage_btree_removes(phalcon_storage_btree_db_t *tree, const char *key);

/*
 * Remove value by key only if it's equal to specified one
 */
int phalcon_storage_btree_removev(phalcon_storage_btree_db_t *tree,
               const phalcon_storage_btree_key_t *key,
               phalcon_storage_btree_remove_cb remove_cb,
               void *arg);
int phalcon_storage_btree_removevs(phalcon_storage_btree_db_t *tree,
                const char *key,
                phalcon_storage_btree_remove_cb remove_cb,
                void *arg);

/*
 * Get all values in range
 * Note: value will be automatically efreed after invokation of callback
 */
int phalcon_storage_btree_get_range(phalcon_storage_btree_db_t *tree,
                 const phalcon_storage_btree_key_t *start,
                 const phalcon_storage_btree_key_t *end,
                 phalcon_storage_btree_range_cb cb,
                 void *arg);
int phalcon_storage_btree_get_ranges(phalcon_storage_btree_db_t *tree,
                  const char *start,
                  const char *end,
                  phalcon_storage_btree_range_cb cb,
                  void *arg);

/*
 * Get values in range (with custom key-filter)
 * Note: value will be automatically efreed after invokation of callback
 */
int phalcon_storage_btree_get_filtered_range(phalcon_storage_btree_db_t *tree,
                          const phalcon_storage_btree_key_t *start,
                          const phalcon_storage_btree_key_t *end,
                          phalcon_storage_btree_filter_cb filter,
                          phalcon_storage_btree_range_cb cb,
                          void *arg);
int phalcon_storage_btree_get_filtered_ranges(phalcon_storage_btree_db_t *tree,
                           const char *start,
                           const char *end,
                           phalcon_storage_btree_filter_cb filter,
                           phalcon_storage_btree_range_cb cb,
                           void *arg);

/*
 * Run compaction on database
 */
int phalcon_storage_btree_compact(phalcon_storage_btree_db_t *tree);

/*
 * Set compare function to define order of keys in database
 */
void phalcon_storage_btree_set_compare_cb(phalcon_storage_btree_db_t *tree, phalcon_storage_btree_compare_cb cb);

/*
 * Ensure that all data is written to disk
 */
int phalcon_storage_btree_fsync(phalcon_storage_btree_db_t *tree);

struct phalcon_storage_btree_db_s {
    PHALCON_STORAGE_BTREE_TREE_PRIVATE
};

struct phalcon_storage_btree_key_s {
    PHALCON_STORAGE_BTREE_KEY_FIELDS
    PHALCON_STORAGE_BTREE_KEY_PRIVATE
};

#endif /* _PHALCON_STORAGE_BTREE_BPLUS_H_ */
