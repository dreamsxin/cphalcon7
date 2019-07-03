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

#include "async/async_fiber.h"
#include "async/async_stack.h"

#include <ucontext.h>

static int counter = 0;
static size_t record_size = 0;

typedef struct _async_fiber_ucontext {
	ucontext_t ctx;
	async_fiber_stack stack;
	int id;
	zend_bool initialized;
	zend_bool root;
} async_fiber_ucontext;

typedef struct _async_fiber_record_ucontext {
	async_fiber_ucontext *fiber;
	async_fiber_cb func;
	void *arg;
} async_fiber_record_ucontext;


const char *async_fiber_backend_info()
{
	return "ucontext (POSIX.1-2001, deprecated since POSIX.1-2004)";
}

async_fiber *async_fiber_create_root()
{
	async_fiber_ucontext *fiber;

	fiber = ecalloc(1, sizeof(async_fiber_ucontext));

	fiber->initialized = 1;
	fiber->root = 1;

	return (async_fiber *) fiber;
}

async_fiber *async_fiber_create()
{
	async_fiber_ucontext *fiber;

	fiber = ecalloc(1, sizeof(async_fiber_ucontext));

	fiber->id = ++counter;

	return (async_fiber *) fiber;
}

static void async_fiber_ucontext_run(void *arg)
{
	async_fiber_record_ucontext *record;
	
	record = (async_fiber_record_ucontext *) arg;
	
	ZEND_ASSERT(record != NULL);
	
	record->func(record->arg);
}

zend_bool async_fiber_init(async_fiber *fiber, async_context *context, async_fiber_cb func, void *arg, size_t stack_size)
{
	async_fiber_ucontext *impl;
	async_fiber_record_ucontext *record;
	
	void *sp;

	impl = (async_fiber_ucontext *) fiber;

	ZEND_ASSERT(impl->initialized == 0);

	if (UNEXPECTED(!async_fiber_stack_allocate(&impl->stack, stack_size))) {
		return 0;
	}
	
	if (UNEXPECTED(!record_size)) {
		record_size = (size_t) ceil((double) sizeof(async_fiber_record_ucontext) / 64) * 64;
	}

	if (UNEXPECTED(-1 == getcontext(&impl->ctx))) {
		return 0;
	}

	sp = (void *) (impl->stack.size - record_size + (char *) impl->stack.pointer);

	record = (async_fiber_record_ucontext *) sp;
	record->fiber = impl;
	record->func = func;
	record->arg = arg;
	
	impl->ctx.uc_link = 0;
	impl->ctx.uc_stack.ss_sp = impl->stack.pointer;
	impl->ctx.uc_stack.ss_size = (sp - impl->stack.pointer) - 64;
	impl->ctx.uc_stack.ss_flags = 0;
	
	makecontext(&impl->ctx, (void (*)()) async_fiber_ucontext_run, 1, record);

	impl->initialized = 1;
	
	fiber->context = context;

	return 1;
}

void async_fiber_destroy(async_fiber *fiber)
{
	async_fiber_ucontext *impl;

	impl = (async_fiber_ucontext *) fiber;

	if (EXPECTED(impl != NULL)) {
		if (EXPECTED(!impl->root && impl->initialized)) {
			async_fiber_stack_free(&impl->stack);
		}

		efree(impl);
	}
}

void async_fiber_suspend(async_task_scheduler *scheduler)
{
	async_fiber *current;
	async_fiber *next;
	
	async_fiber_ucontext *from;
	async_fiber_ucontext *to;
	
	current = ASYNC_G(fiber);
	
	ZEND_ASSERT(scheduler != NULL);
	ZEND_ASSERT(current != NULL);
	
	if (scheduler->fibers.first == NULL) {
		next = scheduler->runner;
	} else {
		ASYNC_LIST_EXTRACT_FIRST(&scheduler->fibers, next);
		
		next->flags &= ~ASYNC_FIBER_FLAG_QUEUED;
	}
	
	if (UNEXPECTED(current == next)) {
		return;
	}
	
	from = (async_fiber_ucontext *) current;
	to = (async_fiber_ucontext *) next;
	
	ZEND_ASSERT(next != NULL);
	ZEND_ASSERT(from->initialized);
	ZEND_ASSERT(to->initialized);
	
	// ASYNC_DEBUG_LOG("SUSPEND: %d -> %d\n", from->id, to->id);
	
	async_fiber_capture_state(current);	
	ASYNC_G(fiber) = next;
	
	ASYNC_CHECK_FATAL(-1 == swapcontext(&from->ctx, &to->ctx), "Failed to switch fiber context");
	
	async_fiber_restore_state(current);
}

void async_fiber_switch(async_task_scheduler *scheduler, async_fiber *next, async_fiber_suspend_type suspend)
{
	async_fiber *current;
	
	async_fiber_ucontext *from;
	async_fiber_ucontext *to;
	
	ZEND_ASSERT(next != NULL);
	ZEND_ASSERT(scheduler != NULL);
	
	current = ASYNC_G(fiber);
	
	ZEND_ASSERT(current != NULL);

	from = (async_fiber_ucontext *) current;
	to = (async_fiber_ucontext *) next;

	ZEND_ASSERT(from->initialized);
	ZEND_ASSERT(to->initialized);
	ZEND_ASSERT(from != to);
	
	if (EXPECTED(!(current->flags & ASYNC_FIBER_FLAG_QUEUED))) {
		switch (suspend) {
		case ASYNC_FIBER_SUSPEND_PREPEND:
			ASYNC_LIST_PREPEND(&scheduler->fibers, current);
			current->flags |= ASYNC_FIBER_FLAG_QUEUED;
			break;
		case ASYNC_FIBER_SUSPEND_APPEND:
			ASYNC_LIST_APPEND(&scheduler->fibers, current);
			current->flags |= ASYNC_FIBER_FLAG_QUEUED;
			break;
		case ASYNC_FIBER_SUSPEND_NONE:
			break;
		}
	}
	
	// ASYNC_DEBUG_LOG("SWITCH: %d -> %d\n", from->id, to->id);
	
	async_fiber_capture_state(current);	
	ASYNC_G(fiber) = next;
	
	ASYNC_CHECK_FATAL(-1 == swapcontext(&from->ctx, &to->ctx), "Failed to switch fiber context");
	
	async_fiber_restore_state(current);
}
