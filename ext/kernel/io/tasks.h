
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

#ifndef PHALCON_KERNEL_IO_TASKS_H
#define PHALCON_KERNEL_IO_TASKS_H

#include "kernel/io/sockets.h"
#include "kernel/io/client.h"

#define PHALCON_IO_BLOCK_SIZE 128

typedef struct {
	phalcon_io_client_info *ci;
} phalcon_io_task_info;

typedef struct {
	pthread_mutex_t lock;
	pthread_cond_t	cond;
	int must_Exit;
	phalcon_io_thread_t *threads;
	int max_threads;
	int first_free_thread;
	phalcon_io_task_info *tasks;
	int max_tasks;
	int first_free_task;
	int first_task;
} phalcon_io_tasks_data;

void phalcon_io_init_tasks ();
void phalcon_io_clean_tasks ();
void phalcon_io_push_thread (phalcon_io_thread_t thread);
void phalcon_io_delete_thread (phalcon_io_thread_t thread);
int  phalcon_io_get_running_task_threads ();
int  phalcon_io_start_one_task_thread ();
void phalcon_io_stop_one_task_thread ();
int  phalcon_io_push_task (phalcon_io_client_info *ci);
phalcon_io_client_info *phalcon_io_pop_task ();
int	 phalcon_io_enqueue_task (phalcon_io_client_info *ci);
phalcon_io_callback_t phalcon_io_tasks_thread (void *data);

#endif /* PHALCON_KERNEL_IO_TASKS_H */
