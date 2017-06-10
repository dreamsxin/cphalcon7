#ifndef PHALCON_STORAGE_BTREE_UTILS_H_
#define PHALCON_STORAGE_BTREE_UTILS_H_

#include <stdint.h> /* uint64_t */

uint64_t _phalcon_storage_btree_compute_hashl(uint64_t key);
uint64_t _phalcon_htonll(uint64_t value);
uint64_t _phalcon_ntohll(uint64_t value);

#endif /* PHALCON_STORAGE_BTREE_UTILS_H_ */
