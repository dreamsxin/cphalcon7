
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

#include <stddef.h>
#include <stdint.h>
#include <semaphore.h>

typedef struct _phalcon_shared_memory {
  int     owner;
  char    name[255];
  size_t  lenght;
  sem_t*  sem;
  int     fd;
  void*   mem;
} phalcon_shared_memory;

phalcon_shared_memory* phalcon_shared_memory_create(char const* name, size_t);
phalcon_shared_memory* phalcon_shared_memory_open(char const* name);

void phalcon_shared_memory_unlink(char const* name);
void phalcon_shared_memory_cleanup(phalcon_shared_memory* src);

char const* phalcon_shared_memory_name(phalcon_shared_memory const* src);

int phalcon_shared_memory_trylock(phalcon_shared_memory* src);
int phalcon_shared_memory_lock(phalcon_shared_memory* src);
int phalcon_shared_memory_unlock(phalcon_shared_memory* src);
int phalcon_shared_memory_unlock_force(phalcon_shared_memory* src);

void* phalcon_shared_memory_ptr(phalcon_shared_memory const* src);
size_t phalcon_shared_memory_size(phalcon_shared_memory const* src);

#endif /* PHALCON_KERNEL_SHM_H */
