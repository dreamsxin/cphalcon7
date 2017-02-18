#ifndef PHALCON_STORAGE_BTREE_WRITER_H_
#define PHALCON_STORAGE_BTREE_WRITER_H_

#include <stdint.h>

#define PHALCON_STORAGE_BTREE_WRITER_PRIVATE	\
    int fd;                     \
    char *filename;             \
    uint64_t filesize;          \
    char padding[PHALCON_STORAGE_BTREE_PADDING];

typedef struct _phalcon_storage_btree_writer_s _phalcon_storage_btree_writer_t;
typedef int (*_phalcon_storage_btree_writer_cb)(_phalcon_storage_btree_writer_t *w, void *data);

enum comp_type {
    kNotCompressed = 0,
    kCompressed = 1
};

int _phalcon_storage_btree_writer_create(_phalcon_storage_btree_writer_t *w, const char *filename);
int _phalcon_storage_btree_writer_destroy(_phalcon_storage_btree_writer_t *w);

int _phalcon_storage_btree_writer_fsync(_phalcon_storage_btree_writer_t *w);

int _phalcon_storage_btree_writer_compact_name(_phalcon_storage_btree_writer_t *w, char **compact_name);
int _phalcon_storage_btree_writer_compact_finalize(_phalcon_storage_btree_writer_t *s, _phalcon_storage_btree_writer_t *t);

int _phalcon_storage_btree_writer_read(_phalcon_storage_btree_writer_t *w,
                    const enum comp_type comp,
                    const uint64_t offset,
                    uint64_t *size,
                    void **data);
int _phalcon_storage_btree_writer_write(_phalcon_storage_btree_writer_t *w,
                     const enum comp_type comp,
                     const void *data,
                     uint64_t *offset,
                     uint64_t *size);

int _phalcon_storage_btree_writer_find(_phalcon_storage_btree_writer_t *w,
                    const enum comp_type comp,
                    const uint64_t size,
                    void *data,
                    _phalcon_storage_btree_writer_cb seek,
                    _phalcon_storage_btree_writer_cb miss);

struct _phalcon_storage_btree_writer_s {
    PHALCON_STORAGE_BTREE_WRITER_PRIVATE
};

#endif /* PHALCON_STORAGE_BTREE_WRITER_H_ */
