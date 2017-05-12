
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
  |          Didier Bertrand <diblibre@gmail.com>                          |
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#include "kernel/io/tasks.h"

#include <stdlib.h>		// calloc, free, realloc
#include <string.h>		// memmove, memset
#include <unistd.h>		// sleep
#include <stdio.h>		// fprintf

#include "kernel/io/support.h"
#include "kernel/io/sockets.h"
#include "kernel/io/threads.h"

static phalcon_io_tasks_data * td;

void phalcon_io_init_tasks()
{
	if (td != NULL)
		return;
	td = (phalcon_io_tasks_data *) calloc (1, sizeof(phalcon_io_tasks_data));
	td->threads = (phalcon_io_thread_t *) calloc(PHALCON_IO_BLOCK_SIZE, sizeof(phalcon_io_thread_t));
	td->max_threads = 0;
	td->first_free_thread = 0;
	td->tasks = (phalcon_io_task_info *) calloc(PHALCON_IO_BLOCK_SIZE, sizeof(phalcon_io_task_info));
	td->max_tasks = 0;
	td->first_free_task = 0;
	td->first_task = 0;
	pthread_mutex_init(&td->lock,NULL);
	pthread_cond_init(&td->cond,NULL);
	td->must_Exit = 0;
}

void phalcon_io_clean_tasks()
{
	int it;
	td->must_Exit = 1;
	pthread_cond_broadcast(&td->cond);
    for (it=0; it < td->first_free_thread; it++) {
    	if (td->threads[it]!=(phalcon_io_thread_t)0)
    		pthread_join(td->threads[it], NULL);
    	td->threads[it] = (phalcon_io_thread_t)0;
    }
    if (td)
    	free(td);
    td = NULL;
}

void phalcon_io_push_thread (phalcon_io_thread_t thread)
{
	if (td->first_free_thread >= td->max_threads) {
		td->max_threads += PHALCON_IO_BLOCK_SIZE;
		if (realloc ((void *)td->threads, td->max_threads) == NULL)
			phalcon_io_error_message("No memory (realloc)\n");

	}
	td->threads[td->first_free_thread++] = thread;
}

// remove a thread from the list
void phalcon_io_delete_thread (phalcon_io_thread_t thread)
{
	int it;
    for (it=0; it < td->first_free_thread; it++) {
    	if (td->threads[it]==thread) {
    		int nthreads = td->first_free_thread - it - 1;
    		if (nthreads > 0)
    			memmove (&td->threads[it], &td->threads[it+1], nthreads*sizeof(phalcon_io_thread_t));
    		--td->first_free_thread;
    		break;
    	}
     }
}

int phalcon_io_get_running_task_threads()
{
	return td? td->first_free_thread: 0;
}

int phalcon_io_start_one_task_thread()
{
	if (td==NULL)
		phalcon_io_init_tasks ();
	phalcon_io_thread_t pid;
	int ret = pthread_create(&pid, 0, phalcon_io_tasks_thread, td);
	phalcon_io_push_thread(pid);
	return ret;
}

void phalcon_io_stop_one_task_thread()
{
	fprintf (stderr, "phalcon_io_stop_one_task_thread\n");
	// push an empty task to signal the end
	if (phalcon_io_get_running_task_threads() > 0)
		phalcon_io_enqueue_task (NULL);
}

// return PHALCON_IO_ERROR if error or number of tasks pushed
int phalcon_io_push_task (phalcon_io_client_info *ci)
{
	if (td->first_free_task >= td->max_tasks) {
		td->max_tasks += PHALCON_IO_BLOCK_SIZE;
		if (realloc ((void *)td->tasks, td->max_tasks) == NULL) {
			phalcon_io_error_message("No memory (realloc)\n");
			return PHALCON_IO_ERROR;
		}
	}
	td->tasks[td->first_free_task++].ci = ci;
	return td->first_free_task - td->first_task;
}

phalcon_io_client_info *phalcon_io_pop_task ()
{
	phalcon_io_client_info *ci = NULL;
	if (td->first_task >= 0)
		ci = td->tasks[td->first_task++].ci;

	if (td->first_task >= td->first_free_task)
		td->first_task = td->first_free_task = 0;		// queue empty; reset pointers
	else if (td->first_task >= td->max_tasks/2) {		// half empty; shift data at the beginning
		int ntasks = td->first_free_task-td->first_task;
		memmove (&td->tasks[0], &td->tasks[td->first_task], ntasks*sizeof(phalcon_io_task_info));
		td->first_free_task -= td->first_task;
		td->first_task = 0;
	}
	return ci;
}

// return PHALCON_IO_ERROR if error or number of tasks enqueued
int phalcon_io_enqueue_task (phalcon_io_client_info *ci)
{
	int ntasks;
	phalcon_io_threads_info *ti = (phalcon_io_threads_info *)ci->tpi;
	if (!ti->allow_tasks)
		return PHALCON_IO_ERROR;
	phalcon_io_debug_message(PHALCON_IO_DEBUG_CLIENT, "Task push\n");

	if (phalcon_io_get_running_task_threads() < 1)
		phalcon_io_start_one_task_thread();
	pthread_mutex_lock(&td->lock);
	ntasks = phalcon_io_push_task(ci);
	pthread_mutex_unlock(&td->lock);
	if (ntasks < 0)
		return PHALCON_IO_ERROR;
	pthread_cond_signal(&td->cond);
	return ntasks;
}

phalcon_io_callback_t phalcon_io_tasks_thread(void *data)
{
	phalcon_io_client_info *ci;

	while (!td->must_Exit) {
		pthread_mutex_lock(&td->lock);
		if (td->first_free_task <= 0) {
			while (td->first_free_task==0 && !td->must_Exit)
				pthread_cond_wait(&td->cond, &td->lock);
			if (td->must_Exit) {
				pthread_mutex_unlock(&td->lock);
				break;
			}
		}
		ci = phalcon_io_pop_task();
		if (ci == NULL)					// request to stop thread
			phalcon_io_delete_thread (pthread_self());
		pthread_mutex_unlock(&td->lock);
		if (ci == NULL) {				// request to stop thread
			phalcon_io_delete_thread (pthread_self());
			pthread_detach(pthread_self());
			break;
		}

		phalcon_io_debug_message(PHALCON_IO_DEBUG_CLIENT, "Task pop\n");
		if (ci->operation == PHALCON_IO_OP_READ)
			phalcon_io_do_callback(ci, PHALCON_IO_CLIENT_DEFFERED_READ);
		else if (ci->operation == PHALCON_IO_OP_WRITE)
			phalcon_io_do_callback(ci, PHALCON_IO_CLIENT_DEFFERED_WRITE);
	}
	return (phalcon_io_callback_t) 0;
}
