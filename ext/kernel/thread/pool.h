
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

#ifndef PHALCON_KERNEL_THREAD_POOL_H
#define PHALCON_KERNEL_THREAD_POOL_H

#include "php_phalcon.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <assert.h>

enum phalcon_thread_pool_schedule_type {
	PHALCON_THREAD_POOL_ROUND_ROBIN,
	PHALCON_THREAD_POOL_LEAST_LOAD
};

enum {
	PHALCON_THREAD_POOL_ERROR,
	PHALCON_THREAD_POOL_WARNING,
	PHALCON_THREAD_POOL_INFO,
	PHALCON_THREAD_POOL_DEBUG
};

#define PHALCON_THREAD_POOL_DEBUG( ...) do { \
	flockfile(stdout); \
	printf("###%p.%s: ", (void *)pthread_self(), __func__); \
	printf(__VA_ARGS__); \
	putchar('\n'); \
	fflush(stdout); \
	funlockfile(stdout);\
} while (0)

#define PHALCON_THREAD_POOL_WORK_QUEUE_POWER 16
#define PHALCON_THREAD_POOL_WORK_QUEUE_SIZE  (1 << PHALCON_THREAD_POOL_WORK_QUEUE_POWER)
#define PHALCON_THREAD_POOL_WORK_QUEUE_MASK  (PHALCON_THREAD_POOL_WORK_QUEUE_SIZE - 1)

/*
 * Just main thread can increase thread->in, we can make it safely.
 * However,  thread->out may be increased in both main thread and
 * worker thread during balancing thread load when new threads are added
 * to our thread pool...
*/
#define phalcon_thread_pool_out_val(thread)	 (__sync_val_compare_and_swap(&(thread)->out, 0, 0))
#define phalcon_thread_pool_queue_len(thread)   ((thread)->in - phalcon_thread_pool_out_val(thread))
#define phalcon_thread_pool_queue_empty(thread) (phalcon_thread_pool_queue_len(thread) == 0)
#define phalcon_thread_pool_queue_full(thread)  (phalcon_thread_pool_queue_len(thread) == PHALCON_THREAD_POOL_WORK_QUEUE_SIZE)
#define phalcon_thread_pool_queue_offset(val)		   ((val) & PHALCON_THREAD_POOL_WORK_QUEUE_MASK)

/* enough large for any system */
#define PHALCON_THREAD_POOL_MAX_NUM  512

typedef struct phalcon_thread_pool phalcon_thread_pool_t;

typedef struct phalcon_thread_pool_work {
	zval routine;
	zval args;
	struct phalcon_thread_pool_work *next;
} phalcon_thread_pool_work_t;

typedef struct {
	phalcon_thread_pool_t *pool;
	pthread_t id;
	pthread_mutex_t lock;
	pthread_cond_t cond;
	int shutdown;
	int num_works_done;
	unsigned int in;	/* offset from start of work_queue where to put work next */
	unsigned int out;   /* offset from start of work_queue where to get work next */
	phalcon_thread_pool_work_t work_queue[PHALCON_THREAD_POOL_WORK_QUEUE_SIZE];
} phalcon_thread_pool_thread_t;

typedef phalcon_thread_pool_thread_t* (*phalcon_thread_pool_schedule_func)(phalcon_thread_pool_t *pool);
struct phalcon_thread_pool {
	pthread_t main_thread;
	pthread_mutex_t lock;
	pthread_cond_t cond;
	int num_threads;
	phalcon_thread_pool_thread_t threads[PHALCON_THREAD_POOL_MAX_NUM];
	phalcon_thread_pool_schedule_func schedule_thread;
};

phalcon_thread_pool_t *phalcon_thread_pool_init(int num_worker_threads);
int phalcon_thread_pool_inc_threads(phalcon_thread_pool_t *pool, int num_inc);
int phalcon_thread_pool_dec_threads(phalcon_thread_pool_t *pool, int num_dec);
int phalcon_thread_pool_add_work(phalcon_thread_pool_t *pool, zval *routine, zval *arg);

/*
@finish:  1, complete remaining works before return
		  0, drop remaining works and return directly
*/
void phalcon_thread_pool_destroy(phalcon_thread_pool_t *pool, int finish);

/* set thread schedule algorithm, default is round-robin */
void phalcon_thread_pool_schedule_algorithm(phalcon_thread_pool_t *pool, enum phalcon_thread_pool_schedule_type type);

#endif /* PHALCON_KERNEL_THREAD_POOL_H */
