/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Martin Schröder <m.schroeder2007@gmail.com>                 |
  +----------------------------------------------------------------------+
*/

#include "async/core.h"

extern ZEND_API void (*async_orig_execute_ex)(zend_execute_data *exec);

static void execute_root(zend_execute_data *exec)
{
	async_task_scheduler *scheduler;

	scheduler = async_task_scheduler_get();

	zend_try {
		async_orig_execute_ex(exec);
	} zend_catch {
		async_task_scheduler_handle_exit(scheduler);
	} zend_end_try();
	
	if (UNEXPECTED(EG(exception))) {
		async_task_scheduler_handle_error(scheduler, EG(exception));
		zend_clear_exception();
	}

	async_task_scheduler_run(scheduler, exec);

	ASYNC_FORWARD_EXIT();
}

/* Custom executor being used to run the task scheduler before shutdown functions. */
ZEND_API void async_execute_ex(zend_execute_data *exec)
{
	if (UNEXPECTED(exec->prev_execute_data == NULL && ASYNC_G(task) == NULL)) {
		execute_root(exec);
	} else {
		async_orig_execute_ex(exec);
	}
}
