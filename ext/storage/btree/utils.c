#include "storage/btree/private/utils.h"

#include <stdint.h> /* uint64_t */
#include <arpa/inet.h> /* nothl, htonl */

/* Thomas Wang, Integer Hash Functions. */
/* http://www.concentric.net/~Ttwang/tech/inthash.htm */
uint32_t _phalcon_storage_btree_compute_hash(uint32_t key)
{
    uint32_t hash = key;
    hash = ~hash + (hash << 15);  /* hash = (hash << 15) - hash - 1; */
    hash = hash ^ (hash >> 12);
    hash = hash + (hash << 2);
    hash = hash ^ (hash >> 4);
    hash = hash * 2057;  /* hash = (hash + (hash << 3)) + (hash << 11); */
    hash = hash ^ (hash >> 16);
    return hash;
}

uint64_t _phalcon_storage_btree_compute_hashl(uint64_t key)
{
    uint32_t keyh = key >> 32;
    uint32_t keyl = key & 0xffffffffLL;

    return ((uint64_t) _phalcon_storage_btree_compute_hash(keyh) << 32) |
                       _phalcon_storage_btree_compute_hash(keyl);
}

uint64_t _phalcon_htonll(uint64_t value)
{
    static const int num = 23;

    if (*(const char *) (&num) != num) return value;

    uint32_t high_part = htonl((uint32_t) (value >> 32));
    uint32_t low_part = htonl((uint32_t) (value & 0xffffffffLL));

    return ((uint64_t) low_part << 32) | high_part;
}

uint64_t _phalcon_ntohll(uint64_t value)
{
    static const int num = 23;

    if (*(const char *) (&num) != num) return value;

    uint32_t high_part = ntohl((uint32_t) (value >> 32));
    uint32_t low_part = ntohl((uint32_t) (value & 0xffffffffLL));

    return ((uint64_t) low_part << 32) | high_part;
}
