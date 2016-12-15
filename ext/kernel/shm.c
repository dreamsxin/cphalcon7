
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2014 Phalcon Team (http://www.phalconphp.com)       |
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

#include "kernel/shm.h"
#include "kernel/main.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>

struct phalcon_shma_s {
  int     owner;
  char    name[255];
  size_t  lenght;
  sem_t*  sem;
  int     fd;
  void*   mem;
};

phalcon_mshm_t* phalcon_mshm_create(char const* name, size_t sz)
{
  int rc = 0;
  struct phalcon_shma_s* shma = malloc(sizeof(*shma));
  memset(shma, 0, sizeof(*shma));

  strncpy(shma->name, name, sizeof(shma->name));
  shma->lenght = sz;

  shma->sem = sem_open(shma->name, O_CREAT,  0666, 1);
  if (SEM_FAILED == shma->sem) {
    rc = errno;
    shma->sem = NULL;
    goto error;
  }

  shma->owner = 1;
  shma->fd = shm_open(shma->name, O_CREAT | O_RDWR, 0666);


  if (shma->fd < 0) {
    rc = errno;
    shma->fd = 0;
    goto error;
  }

  if (ftruncate(shma->fd, shma->lenght)) {
    rc = errno;
    goto error;
  }

  shma->mem = mmap(0, shma->lenght, PROT_WRITE|PROT_READ, MAP_SHARED, shma->fd, 0);

  if ((void*)-1 == shma->mem) {
    rc = errno;
    shma->mem = NULL;
    goto error;
  }

  return shma;

error:
  phalcon_mshm_cleanup(shma);
  errno = rc;
  return NULL;
}

phalcon_mshm_t* phalcon_mshm_open(char const* name)
{
  int rc = 0;
  struct phalcon_shma_s* shma = malloc(sizeof(*shma));
  memset(shma, 0, sizeof(*shma));
  strncpy(shma->name, name, sizeof(shma->name));

  shma->sem = sem_open(shma->name, 0);
  if (SEM_FAILED == shma->sem) {
    rc = errno;
    shma->sem = NULL;
    goto error;
  }

  shma->fd = shm_open(shma->name, O_RDWR, 0666);
  if (shma->fd < 0) {
    rc = errno;
    shma->fd = 0;
    goto error;
  }

  struct stat info;
  if (fstat(shma->fd, &info)) {
    rc = errno;
    goto error;
  }

  shma->lenght = info.st_size;
  shma->mem = mmap(0, shma->lenght, PROT_WRITE|PROT_READ, MAP_SHARED, shma->fd, 0);

  if ((void*)-1 == shma->mem) {
    rc = errno;
    shma->mem = NULL;
    goto error;
  }

  return shma;

error:
  phalcon_mshm_cleanup(shma);
  errno  = rc;
  return NULL;
}

void phalcon_mshm_unlink(char const* name)
{
  sem_unlink(name);
  shm_unlink(name);
}

void phalcon_mshm_cleanup(phalcon_mshm_t* src)
{
  if (!src)
    return;

  struct phalcon_shma_s* shma = (struct phalcon_shma_s*)src;

  if (shma->mem) {
    munmap(shma->mem, shma->lenght);
  }

  if (shma->fd) {
    close(shma->fd);
  }


  if (shma->sem) {
    sem_close(shma->sem);
  }

  free(shma);
}

char const* phalcon_mshm_name(phalcon_mshm_t const* src)
{
  struct phalcon_shma_s* shma = (struct phalcon_shma_s*)src;
  return shma->name;
}


int phalcon_mshm_trylock(phalcon_mshm_t* src)
{
  struct phalcon_shma_s* shma = (struct phalcon_shma_s*)src;

  if (!shma->sem) {
    errno = EINVAL;
    return 1;
  }

  if (!sem_trywait(shma->sem)) {
    return 0;
  }

  return 1;
}

int phalcon_mshm_lock(phalcon_mshm_t* src)
{
  struct phalcon_shma_s* shma = (struct phalcon_shma_s*)src;

  if (!shma->sem) {
    errno = EINVAL;
    return 1;
  }

  if (!sem_wait(shma->sem)) {

    int v;
    sem_getvalue(shma->sem, &v);

    if (v) {
      return v;
    }

    return 0;
  }

  return 1;
}

int phalcon_mshm_unlock(phalcon_mshm_t* src)
{
  struct phalcon_shma_s* shma = (struct phalcon_shma_s*)src;

  if (!shma->sem) {
    errno = EINVAL;
    return 1;
  }

  if (!sem_post(shma->sem)) {
    return 0;
  }

  return 1;
}

int phalcon_mshm_unlock_force(phalcon_mshm_t* src)
{
  struct phalcon_shma_s* shma = (struct phalcon_shma_s*)src;
  if (!shma->sem) {
    errno = EINVAL;
    return 1;
  }

  while (1) {

    int v = 0;
    if (sem_getvalue(shma->sem, &v)) {
      break;
    }

    if (v)
      return 0;

    if (sem_post(shma->sem)) {
      break;
    }
  }

  return 1;
}

void* phalcon_mshm_memory_ptr(phalcon_mshm_t const* src)
{
  struct phalcon_shma_s* shma = (struct phalcon_shma_s*)src;
  if (!shma->mem) errno = EINVAL;
  return shma->mem;
}

size_t phalcon_mshm_memory_size(phalcon_mshm_t const* src)
{
  struct phalcon_shma_s* shma = (struct phalcon_shma_s*)src;
  struct stat sb;

  if (fstat(shma->fd, &sb)) {
    return 0;
  }

  return sb.st_size;
}
