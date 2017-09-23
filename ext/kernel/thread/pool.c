
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

static int phalcon_thread_pool_empty(phalcon_thread_pool_t *pool)
{
	int i;

	for (i = 0; i < pool->num_threads; i++)
		if (!phalcon_thread_pool_queue_empty(&pool->threads[i]))
			return 0;
	return 1;
}

static phalcon_thread_pool_thread_t* round_robin_schedule(phalcon_thread_pool_t *pool)
{
	static int cur_thread_index = -1;

	assert(pool && pool->num_threads > 0);
	cur_thread_index = (cur_thread_index + 1) % pool->num_threads ;
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

static void sig_send(pthread_t tid, int signo)
{
	pthread_kill(tid, signo);
}

static void sig_do_nothing(int signo)
{
	return;
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
	} while (!__sync_bool_compare_and_swap(&thread->out, tmp, tmp + 1));
	return work;
}

static void *phalcon_thread_pool_thread_start_routine(void *arg)
{
	phalcon_thread_pool_thread_t *thread = arg;
	phalcon_thread_pool_t *pool = thread->pool;
	phalcon_thread_pool_work_t *work = NULL;
	sigset_t signal_mask, oldmask;
	int rc, sig_caught;

	/* SIGUSR1 handler has been set in phalcon_thread_pool_init */
	__sync_fetch_and_add(&pool->num_threads, 1);
	sig_send(pool->main_thread, SIGUSR1);

	sigemptyset(&oldmask);
	sigemptyset(&signal_mask);
	sigaddset(&signal_mask, SIGUSR1);

	while (1) {
		rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
		if (rc != 0) {
			zend_error(E_NOTICE, "SIG_BLOCK failed!");
			pthread_exit(NULL);
		}
		while (phalcon_thread_pool_queue_empty(thread) && !thread->shutdown) {
			if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
				PHALCON_THREAD_POOL_DEBUG("I'm sleep");
			}
			rc = sigwait (&signal_mask, &sig_caught);
			if (rc != 0) {
				zend_error(E_NOTICE, "Sigwait failed!");
				pthread_exit(NULL);
			}
		}

		rc = pthread_sigmask(SIG_SETMASK, &oldmask, NULL);
		if (rc != 0) {
			zend_error(E_NOTICE, "SIG_SETMASK failed!");
			pthread_exit(NULL);
		}

		if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
			PHALCON_THREAD_POOL_DEBUG("I'm awake");
		}

		if (thread->shutdown) {
			if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
				PHALCON_THREAD_POOL_DEBUG("exit! %ld: %d\n", thread->id, thread->num_works_done);
			}
			
			while (!phalcon_thread_pool_queue_empty(thread)) {
				work = get_work_concurrently(thread);
				zval_ptr_dtor(&work->routine);
				zval_ptr_dtor(&work->args);
			}
			pthread_exit(NULL);
		}
		work = get_work_concurrently(thread);
		if (work) {
			phalcon_call_user_func_array(NULL, &work->routine, &work->args);
			zval_ptr_dtor(&work->routine);
			zval_ptr_dtor(&work->args);
#ifdef PHALCON_DEBUG
			thread->num_works_done++;
#endif
		}
		if (phalcon_thread_pool_queue_empty(thread)) {
			sig_send(pool->main_thread, SIGUSR1);
		}
	}
}

static int spawn_new_thread(phalcon_thread_pool_t *pool, int index)
{
	memset(&pool->threads[index], 0, sizeof(phalcon_thread_pool_thread_t));
	pool->threads[index].pool = pool;
	if (pthread_create(&pool->threads[index].id, NULL, phalcon_thread_pool_thread_start_routine, (void *)(&pool->threads[index])) != 0) {
		zend_error(E_ERROR, "Create pthread failed!");
		return -1;
	}
	return 0;
}

static int wait_for_thread_registration(phalcon_thread_pool_t *pool, int num_expected)
{
	sigset_t signal_mask, oldmask;
	int rc, sig_caught;

	sigemptyset (&oldmask);
	sigemptyset (&signal_mask);
	sigaddset (&signal_mask, SIGUSR1);
	rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
	if (rc != 0) {
		zend_error(E_NOTICE, "SIG_BLOCK failed!");
		return -1;
	}

	while (pool->num_threads < num_expected) {
		rc = sigwait (&signal_mask, &sig_caught);
		if (rc != 0) {
			zend_error(E_NOTICE, "Sigwait failed!");
			return -1;
		}
	}
	rc = pthread_sigmask(SIG_SETMASK, &oldmask, NULL);
	if (rc != 0) {
		zend_error(E_NOTICE, "SIG_SETMASK failed!");
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
	pool = emalloc(sizeof(phalcon_thread_pool_t));
	if (pool == NULL) {
		zend_error(E_ERROR, "Malloc failed!");
		return NULL;
	}

	memset(pool, 0, sizeof(phalcon_thread_pool_t));
	pool->num_threads = num_threads;
	pool->schedule_thread = round_robin_schedule;
	/* all threads are set SIGUSR1 with sig_do_nothing */
	if (signal(SIGUSR1, sig_do_nothing) == SIG_ERR) {
		zend_error(E_ERROR, "Signal failed!");
		return NULL;
	}
	pool->main_thread = pthread_self();
	for (i = 0; i < pool->num_threads; i++) {
		if (spawn_new_thread(pool, i) < 0) {
			//exit(-1);
			phalcon_thread_pool_destroy(pool, 0);
			return NULL;
		}
	}
	if (wait_for_thread_registration(pool, pool->num_threads) < 0) {
		//pthread_exit(NULL);
		phalcon_thread_pool_destroy(pool, 0);
		return NULL;
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
	ZVAL_COPY(&work->routine, routine);
	if (args) {
		ZVAL_COPY(&work->args, args);
	}
	work->next = NULL;
	thread->in++;
	if (phalcon_thread_pool_queue_len(thread) == 1) {
		if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
			zend_error(E_NOTICE, "Signal has task!");
		}
		sig_send(thread->id, SIGUSR1);
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
	}
	if (wait_for_thread_registration(pool, num_threads) < 0) {
		zend_error(E_ERROR, "thread registration fail!");
		pthread_exit(NULL);
	}
	pool->num_threads = num_threads;
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
	pool->num_threads -= num_dec;
	for (i = pool->num_threads; i < num_threads; i++) {
		pool->threads[i].shutdown = 1;
		sig_send(pool->threads[i].id, SIGUSR1);
	}
	for (i = pool->num_threads; i < num_threads; i++) {
		pthread_join(pool->threads[i].id, NULL);
		/* migrate remaining work to other threads */
		if (migrate_thread_work(pool, &pool->threads[i]) < 0) {
			zend_error(E_NOTICE, "Work lost during migration!");
		}
	}
	if (pool->num_threads == 0 && !phalcon_thread_pool_empty(pool)) {
		zend_error(E_NOTICE, "No thread in pool with work unfinished!");
	}
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
		sigset_t signal_mask, oldmask;
		int rc, sig_caught;
		if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
			PHALCON_THREAD_POOL_DEBUG("Wait all work done");
		}
		sigemptyset (&oldmask);
		sigemptyset (&signal_mask);
		sigaddset (&signal_mask, SIGUSR1);
		rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
		if (rc != 0) {
			zend_error(E_NOTICE, "SIG_BLOCK failed!");
			pthread_exit(NULL);
		}

		while (!phalcon_thread_pool_empty(pool)) {
			rc = sigwait(&signal_mask, &sig_caught);
			if (rc != 0) {
				zend_error(E_NOTICE, "Sigwait failed!");
				pthread_exit(NULL);
			}
		}

		rc = pthread_sigmask(SIG_SETMASK, &oldmask, NULL);
		if (rc != 0) {
			zend_error(E_NOTICE, "SIG_SETMASK failed!");
			pthread_exit(NULL);
		}
	}
	/* shutdown all threads */
	for (i = 0; i < pool->num_threads; i++) {
		pool->threads[i].shutdown = 1;
		/* wake up thread */
		if (pool->threads[i].id) sig_send(pool->threads[i].id, SIGUSR1);
	}
	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_THREAD_POOL_DEBUG("wait worker thread exit");
	}
	for (i = 0; i < pool->num_threads; i++) {
		if (pool->threads[i].id) pthread_join(pool->threads[i].id, NULL);
	}
	efree(pool);
}
