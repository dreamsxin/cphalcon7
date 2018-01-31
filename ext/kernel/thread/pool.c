
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

#include "kernel/thread/pool.h"
#include "kernel/fcall.h"
#include "kernel/debug.h"

static int phalcon_thread_pool_empty(phalcon_thread_pool_t *pool)
{
	int i;
	for (i = 0; i < pool->num_threads; i++) {
		if (!phalcon_thread_pool_queue_empty(&pool->threads[i]))
			return 0;
	}
	return 1;
}

static phalcon_thread_pool_thread_t* round_robin_schedule(phalcon_thread_pool_t *pool)
{
	static int cur_thread_index = -1;
	phalcon_thread_pool_thread_t *thread;
	assert(pool && pool->num_threads > 0);
	do {
		cur_thread_index = (cur_thread_index + 1) % pool->num_threads ;
		thread = &pool->threads[cur_thread_index];
	} while(thread->shutdown && cur_thread_index < pool->num_threads - 1);

	return &pool->threads[cur_thread_index];
}

static phalcon_thread_pool_thread_t* least_load_schedule(phalcon_thread_pool_t *pool)
{
	int i;
	int min_num_works_index = 0;

	assert(pool && pool->num_threads > 0);
	/* To avoid race, we adapt the simplest min value algorithm instead of min-heap */
	for (i = 1; i < pool->num_threads; i++) {
		if (phalcon_thread_pool_queue_len(&pool->threads[i]) < phalcon_thread_pool_queue_len(&pool->threads[min_num_works_index])) {
			min_num_works_index = i;
		}
	}
	return &pool->threads[min_num_works_index];
}

static const phalcon_thread_pool_schedule_func schedule_alogrithms[] = {
	[PHALCON_THREAD_POOL_ROUND_ROBIN] = round_robin_schedule,
	[PHALCON_THREAD_POOL_LEAST_LOAD]  = least_load_schedule
};

void phalcon_thread_pool_schedule_algorithm(phalcon_thread_pool_t *pool, enum phalcon_thread_pool_schedule_type type)
{
	assert(pool);
	pool->schedule_thread = schedule_alogrithms[type];
}

static phalcon_thread_pool_work_t *get_work_concurrently(phalcon_thread_pool_thread_t *thread)
{
	phalcon_thread_pool_work_t *work = NULL;
	unsigned int tmp;

	do {
		work = NULL;
		if (phalcon_thread_pool_queue_len(thread) <= 0)
			break;
		tmp = thread->out;
		//prefetch work
		work = &thread->work_queue[phalcon_thread_pool_queue_offset(tmp)];

		//printf("\nget_work_concurrently:%p,i:%d\n", (void*)thread->id, phalcon_thread_pool_queue_offset(tmp));
		//zend_print_zval_r(&work->args, 0);
	} while (!__sync_bool_compare_and_swap(&thread->out, tmp, tmp + 1));
	return work;
}

static void format_wait_time(int microseconds, struct timespec *abstime)
{
	struct timeval now;
	long long absmsec;

	gettimeofday(&now, NULL);
	absmsec = now.tv_sec * 1000ll + now.tv_usec / 1000ll;
	absmsec += microseconds;

	abstime->tv_sec = absmsec / 1000ll;
	abstime->tv_nsec = absmsec % 1000ll * 1000000ll;
}

static void *phalcon_thread_pool_thread_start_routine(void *arg)
{
	phalcon_thread_pool_thread_t *thread = arg;
	phalcon_thread_pool_t *pool = thread->pool;
	phalcon_thread_pool_work_t *work = NULL;

	__sync_fetch_and_add(&pool->num_threads, 1);

	while (1) {		
		while (phalcon_thread_pool_queue_empty(thread) && !thread->shutdown) {
			struct timespec abstime;
			format_wait_time(1000, &abstime);

			pthread_mutex_lock(&thread->lock);
			pthread_cond_timedwait(&thread->cond, &thread->lock, &abstime);
			pthread_mutex_unlock(&thread->lock);
		}

		if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
			PHALCON_THREAD_POOL_DEBUG("I'm awake");
		}

		if (thread->shutdown) {
			pthread_cond_signal(&pool->cond);
			if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
				PHALCON_THREAD_POOL_DEBUG("exit! %ld: %d\n", (*(unsigned long*)&(thread->id)), thread->num_works_done);
			}
			pthread_exit(NULL);
		}

		work = get_work_concurrently(thread);
		if (work) {
			//PHALCON_THREAD_POOL_DEBUG("call task");
			//zend_print_zval_r(&work->args, 0);
			phalcon_call_user_func_array(NULL, &work->routine, &work->args);
			//zval_ptr_dtor(&work->routine);
			//zval_ptr_dtor(&work->args);
#ifdef PHALCON_DEBUG
			thread->num_works_done++;
#endif
		}
		pthread_cond_signal(&pool->cond);
		//sched_yield();
	}
}

static int spawn_new_thread(phalcon_thread_pool_t *pool, int index)
{
	memset(&pool->threads[index], 0, sizeof(phalcon_thread_pool_thread_t));
	pool->threads[index].pool = pool;
	pthread_mutex_init(&pool->threads[index].lock, NULL);
	pthread_cond_init(&pool->threads[index].cond, NULL);
	if (pthread_create(&pool->threads[index].id, NULL, phalcon_thread_pool_thread_start_routine, (void *)(&pool->threads[index])) != 0) {
		zend_error(E_ERROR, "Create pthread failed!");
		return -1;
	}
	return 0;
}

phalcon_thread_pool_t *phalcon_thread_pool_init(int num_threads)
{
	int i;
	phalcon_thread_pool_t *pool;

	if (num_threads <= 0) {
		return NULL;
	} else if (num_threads > PHALCON_THREAD_POOL_MAX_NUM) {
		zend_error(E_ERROR, "Too many threads!");
		return NULL;
	}
	pool = malloc(sizeof(phalcon_thread_pool_t));
	if (pool == NULL) {
		zend_error(E_ERROR, "Malloc failed!");
		return NULL;
	}

	memset(pool, 0, sizeof(phalcon_thread_pool_t));
	pool->num_threads = 0;
	pool->schedule_thread = round_robin_schedule;

	pthread_mutex_init(&pool->lock, NULL);
	pthread_cond_init(&pool->cond, NULL);

	pool->main_thread = pthread_self();
	for (i = 0; i < num_threads; i++) {
		if (spawn_new_thread(pool, i) < 0) {
			//exit(-1);
			phalcon_thread_pool_destroy(pool, 0);
			return NULL;
		}
	}

	return pool;
}

static int dispatch_work2thread(phalcon_thread_pool_t *pool, phalcon_thread_pool_thread_t *thread, zval *routine, zval *args)
{
	phalcon_thread_pool_work_t *work = NULL;

	if (phalcon_thread_pool_queue_full(thread)) {
		zend_error(E_NOTICE, "Queue of thread selected is full!");
		return -1;
	}
	work = &thread->work_queue[phalcon_thread_pool_queue_offset(thread->in)];
	zval_ptr_dtor(&work->routine);
	zval_ptr_dtor(&work->args);
	ZVAL_COPY(&work->routine, routine);
	if (args) {
		ZVAL_COPY(&work->args, args);
	} else {
		ZVAL_NULL(&work->args);
	}
	work->next = NULL;
	/*
	int i;
	printf("\ndispatch_work2thread:%p\n", (void*)thread->id);
	for (i =0; i <= thread->in; i++) {
		printf("\ni:%d\n", phalcon_thread_pool_queue_offset(i));
		work = &thread->work_queue[phalcon_thread_pool_queue_offset(i)];
		zend_print_zval_r(&work->args, 0);
	}
	*/
	thread->in++;
	if (phalcon_thread_pool_queue_len(thread) > 0) {
		if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
			zend_error(E_NOTICE, "Signal has task!");
		}
		pthread_cond_signal(&thread->cond);
	}
	return 0;
}

/*
 * Here, worker threads died with work undone can not change from->out
 *  and we can read it directly...
*/
static int migrate_thread_work(phalcon_thread_pool_t *pool, phalcon_thread_pool_thread_t *from)
{
	unsigned int i;
	phalcon_thread_pool_work_t *work;
	phalcon_thread_pool_thread_t *to;

	for (i = from->out; i < from->in; i++) {
		work = &from->work_queue[phalcon_thread_pool_queue_offset(i)];
		to = pool->schedule_thread(pool);
		if (dispatch_work2thread(pool, to, &work->routine, &work->args) < 0) {
			zval_ptr_dtor(&work->routine);
			zval_ptr_dtor(&work->args);
			return -1;
		}
	}
#ifdef PHALCON_DEBUG
	printf("%ld migrate_thread_work: %u\n", from->id, phalcon_thread_pool_queue_len(from));
#endif
	return 0;
}

static int isnegtive(int val)
{
	return val < 0;
}

static int ispositive(int val)
{
	return val > 0;
}

static int get_first_id(int arr[], int len, int (*fun)(int))
{
	int i;

	for (i = 0; i < len; i++)
		if (fun(arr[i]))
			return i;
	return -1;
}

/*
 * The load balance algorithm may not work so balanced because worker threads
 * are consuming work at the same time, which resulting in work count is not
 * real-time
*/
static void balance_thread_load(phalcon_thread_pool_t *pool)
{
	int count[PHALCON_THREAD_POOL_MAX_NUM];
	int i, sum = 0, avg;
	int first_neg_id, first_pos_id, tmp, migrate_num;
	phalcon_thread_pool_thread_t *from, *to;
	phalcon_thread_pool_work_t *work;

	for (i = 0; i < pool->num_threads; i++) {
		count[i] = phalcon_thread_pool_queue_len(&pool->threads[i]);
		sum += count[i];
	}
	avg = sum / pool->num_threads;
	if (avg == 0)
		return;
	for (i = 0; i < pool->num_threads; i++)
		count[i] -= avg;
	while (1) {
		first_neg_id = get_first_id(count, pool->num_threads, isnegtive);
		first_pos_id = get_first_id(count, pool->num_threads, ispositive);
		if (first_neg_id < 0)
			break;
		tmp = count[first_neg_id] + count[first_pos_id];
		if (tmp > 0) {
			migrate_num = -count[first_neg_id];
			count[first_neg_id] = 0;
			count[first_pos_id] = tmp;
		} else {
			migrate_num = count[first_pos_id];
			count[first_pos_id] = 0;
			count[first_neg_id] = tmp;
		}
		from = &pool->threads[first_pos_id];
		to = &pool->threads[first_neg_id];
		for (i = 0; i < migrate_num; i++) {
			work = get_work_concurrently(from);
			if (work) {
				if (dispatch_work2thread(pool, to, &work->routine, &work->args) < 0) {
					zval_ptr_dtor(&work->routine);
					zval_ptr_dtor(&work->args);
				}
			}
		}
	}
	from = &pool->threads[first_pos_id];
	/* Just migrate count[first_pos_id] - 1 works to other threads*/
	for (i = 1; i < count[first_pos_id]; i++) {
		to = &pool->threads[i - 1];
		if (to == from)
			continue;
		work = get_work_concurrently(from);
		if (work) {
			if (dispatch_work2thread(pool, to, &work->routine, &work->args) < 0) {
				zval_ptr_dtor(&work->routine);
				zval_ptr_dtor(&work->args);
			}
		}
	}
}

int phalcon_thread_pool_inc_threads(phalcon_thread_pool_t *pool, int num_inc)
{
	int i, num_threads;

	assert(pool && num_inc > 0);
	num_threads = pool->num_threads + num_inc;
	if (num_threads > PHALCON_THREAD_POOL_MAX_NUM) {
		zend_error(E_NOTICE, "Add too many threads!");
		return -1;
	}
	for (i = pool->num_threads; i < num_threads; i++) {
		if (spawn_new_thread(pool, i)<0) {
			zend_error(E_ERROR, "Spawn new thread fail!");
			//exit(-1);
			return -1;
		}
		pthread_mutex_unlock(&pool->lock);
	}
	balance_thread_load(pool);
	return 0;
}

int phalcon_thread_pool_dec_threads(phalcon_thread_pool_t *pool, int num_dec)
{
	int i, num_threads;

	assert(pool && num_dec > 0);
	if (num_dec > pool->num_threads) {
		num_dec = pool->num_threads;
	}
	num_threads = pool->num_threads;
	for (i = num_threads; i > num_threads - num_dec; i--) {
		pool->threads[i].shutdown = 1;
		pthread_cond_signal(&pool->threads[i].cond);
	}
	for (i = num_threads; i > num_threads - num_dec; i--) {
		pthread_join(pool->threads[i].id, NULL);
		/* migrate remaining work to other threads */
		if (migrate_thread_work(pool, &pool->threads[i]) < 0) {
			zend_error(E_NOTICE, "Work lost during migration!");
		}
	}
	if (pool->num_threads == 0 && !phalcon_thread_pool_empty(pool)) {
		zend_error(E_NOTICE, "No thread in pool with work unfinished!");
	}
	pool->num_threads = num_threads - num_dec;
	return 0;
}

int phalcon_thread_pool_add_work(phalcon_thread_pool_t *pool, zval *routine, zval *arg)
{
	phalcon_thread_pool_thread_t *thread;

	assert(pool);
	thread = pool->schedule_thread(pool);
	return dispatch_work2thread(pool, thread, routine, arg);
}


void phalcon_thread_pool_destroy(phalcon_thread_pool_t *pool, int finish)
{
	int i;

	assert(pool);
	if (finish == 1) {
		if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
			PHALCON_THREAD_POOL_DEBUG("Wait all work done");
		}

		while (!phalcon_thread_pool_empty(pool)) {
			struct timespec abstime;
			format_wait_time(1000, &abstime);

			pthread_mutex_lock(&pool->lock);
			pthread_cond_timedwait(&pool->cond, &pool->lock, &abstime);
			pthread_mutex_unlock(&pool->lock);
		}
	}

	/* shutdown all threads */
	for (i = 0; i < pool->num_threads; i++) {
		pool->threads[i].shutdown = 1;
		/* wake up thread */
		if (pool->threads[i].id) {
			pthread_cond_signal(&pool->threads[i].cond);
		}
	}
	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_THREAD_POOL_DEBUG("wait worker thread exit");
	}
	for (i = 0; i < pool->num_threads; i++) {
		phalcon_thread_pool_thread_t *thread = &pool->threads[i];
		if (pool->threads[i].id) {
			pthread_join(pool->threads[i].id, NULL);
		}
		pthread_cond_destroy(&thread->cond);
		pthread_mutex_destroy(&thread->lock);

		while (!phalcon_thread_pool_queue_empty(thread)) {
			phalcon_thread_pool_work_t *work = get_work_concurrently(thread);
			if (work) {
				zval_ptr_dtor(&work->routine);
				zval_ptr_dtor(&work->args);
			}
		}
	}
	pthread_cond_destroy(&pool->cond);
	pthread_mutex_destroy(&pool->lock);
	free(pool);
}
