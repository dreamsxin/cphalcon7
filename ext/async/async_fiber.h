
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
  |          Martin Schröder <m.schroeder2007@gmail.com>                   |
  +------------------------------------------------------------------------+
*/

#ifndef ASYNC_FIBER_H
#define ASYNC_FIBER_H

#define ASYNC_FIBER_VM_STACK_SIZE 4096

#ifdef PHP_WIN32
#define ASYNC_FIBER_CALLBACK static VOID __stdcall
typedef LPFIBER_START_ROUTINE async_fiber_cb;
#else
#define ASYNC_FIBER_CALLBACK static void
typedef void (* async_fiber_cb)(void *arg);
#endif

#define BACKUP_EG(name) (fiber)->name = EG(name)
#define RESTORE_EG(name) EG(name) = (fiber)->name

typedef enum {
	ASYNC_FIBER_SUSPEND_NONE,
	ASYNC_FIBER_SUSPEND_PREPEND,
	ASYNC_FIBER_SUSPEND_APPEND
} async_fiber_suspend_type;

const char *async_fiber_backend_info();

async_fiber *async_fiber_create_root();
async_fiber *async_fiber_create();
zend_bool async_fiber_init(async_fiber *fiber, async_context *context, async_fiber_cb func, void *arg, size_t stack_size);
void async_fiber_destroy(async_fiber *fiber);

void async_fiber_suspend(async_task_scheduler *scheduler);
void async_fiber_switch(async_task_scheduler *scheduler, async_fiber *to, async_fiber_suspend_type suspend);

#define async_fiber_copy_og(to, from) memcpy(to, from, sizeof(zend_output_globals));

static zend_always_inline void async_fiber_capture_og(async_context *context)
{
	if (UNEXPECTED(context->output.context->output.handler == NULL)) {
		context->output.context->output.handler = emalloc(sizeof(zend_output_globals));
	}
	
	async_fiber_copy_og(context->output.context->output.handler, &OG(handlers));
}

static zend_always_inline void async_fiber_restore_og(async_context *context)
{
	if (UNEXPECTED(context->output.context->output.handler == NULL)) {
		php_output_activate();
	} else {
		async_fiber_copy_og(&OG(handlers), context->output.context->output.handler);
	}
}

static zend_always_inline void async_fiber_capture_state(async_fiber *fiber)
{
	fiber->scheduler = ASYNC_G(scheduler);
	fiber->context = ASYNC_G(context);
	fiber->task = ASYNC_G(task);
	
	BACKUP_EG(vm_stack);
#if PHP_VERSION_ID >= 70300
	BACKUP_EG(vm_stack_page_size);
#endif
	BACKUP_EG(current_execute_data);
	BACKUP_EG(exception_class);
	BACKUP_EG(error_handling);
	BACKUP_EG(error_reporting);
	BACKUP_EG(bailout);
	
	fiber->vm_stack->top = EG(vm_stack_top);
	fiber->vm_stack->end = EG(vm_stack_end);
	
	async_fiber_capture_og(fiber->context);
}

static zend_always_inline void async_fiber_restore_state(async_fiber *fiber)
{
	ASYNC_G(scheduler) = fiber->scheduler;
	ASYNC_G(context) = fiber->context;
	ASYNC_G(task) = fiber->task;
	
	RESTORE_EG(vm_stack);
#if PHP_VERSION_ID >= 70300
	RESTORE_EG(vm_stack_page_size);
#endif
	RESTORE_EG(current_execute_data);
	RESTORE_EG(exception_class);
	RESTORE_EG(error_handling);
	RESTORE_EG(error_reporting);
	RESTORE_EG(bailout);
	
	EG(vm_stack_top) = fiber->vm_stack->top;
	EG(vm_stack_end) = fiber->vm_stack->end;
	
	async_fiber_restore_og(fiber->context);
}

#endif
