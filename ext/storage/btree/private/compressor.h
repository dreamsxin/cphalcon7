#ifndef PHALCON_STORAGE_BTREE_COMPRESSOR_H_
#define PHALCON_STORAGE_BTREE_COMPRESSOR_H_

#include "storage/btree/private/errors.h"

#include <unistd.h> /* size_t */
#include <string.h> /* memcpy */

static inline size_t _phalcon_storage_btree_max_compressed_size(size_t size)
{
    return size;
}

static inline int _phalcon_storage_btree_compress(const char *input,
                 size_t input_length,
                 char *compressed,
                 size_t *compressed_length)
{
    memcpy(compressed, input, input_length);
    *compressed_length = input_length;
    return PHALCON_STORAGE_BTREE_OK;
}

static inline int _phalcon_storage_btree_uncompressed_length(const char *compressed,
                            size_t compressed_length,
                            size_t *result)
{
    *result = compressed_length;
    return PHALCON_STORAGE_BTREE_OK;
}

static inline int _phalcon_storage_btree_uncompress(const char *compressed,
                   size_t compressed_length,
                   char *uncompressed,
                   size_t *uncompressed_length)
{
    memcpy(uncompressed, compressed, compressed_length);
    *uncompressed_length = compressed_length;
    return PHALCON_STORAGE_BTREE_OK;
}

#endif /* PHALCON_STORAGE_BTREE_COMPRESSOR_H_ */
