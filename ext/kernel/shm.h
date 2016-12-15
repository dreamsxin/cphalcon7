
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

#ifndef PHALCON_KERNEL_SHM_H
#define PHALCON_KERNEL_SHM_H

typedef void phalcon_mshm_t;

phalcon_mshm_t* phalcon_mshm_create(char const*, size_t);
phalcon_mshm_t* phalcon_mshm_open(char const*);

void phalcon_mshm_unlink(char const*);
void phalcon_mshm_cleanup(phalcon_mshm_t*);

char const* phalcon_mshm_name(phalcon_mshm_t const*);

int phalcon_mshm_trylock(phalcon_mshm_t*);
int phalcon_mshm_lock(phalcon_mshm_t*);
int phalcon_mshm_unlock(phalcon_mshm_t*);
int phalcon_mshm_unlock_force(phalcon_mshm_t*);

void* phalcon_mshm_memory_ptr(phalcon_mshm_t const*);
size_t phalcon_mshm_memory_size(phalcon_mshm_t const*);

#endif /* PHALCON_KERNEL_SHM_H */
