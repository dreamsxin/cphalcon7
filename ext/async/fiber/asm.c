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

#undef ASM_CALLDECL
#if (defined(i386) || defined(__i386__) || defined(__i386) \
     || defined(__i486__) || defined(__i586__) || defined(__i686__) \
     || defined(__X86__) || defined(_X86_) || defined(__THW_INTEL__) \
     || defined(__I86__) || defined(__INTEL__) || defined(__IA32__) \
     || defined(_M_IX86) || defined(_I86_)) && defined(ZEND_WIN32)
#define ASM_CALLDECL __cdecl
#else
#define ASM_CALLDECL
#endif

typedef void* fcontext_t;

typedef struct _transfer_t {
    fcontext_t ctx;
    void *data;
} transfer_t;

extern fcontext_t ASM_CALLDECL make_fcontext(void *sp, size_t size, void (*fn)(transfer_t));
extern transfer_t ASM_CALLDECL jump_fcontext(fcontext_t to, void *vp);

static int counter = 0;
static size_t record_size = 0;

typedef struct _async_fiber_asm {
	async_fiber base;
	fcontext_t ctx;
	async_fiber_stack stack;
	int id;
	zend_bool initialized;
	zend_bool root;
} async_fiber_asm;

typedef struct _async_fiber_record_asm {
	async_fiber_asm *fiber;
	async_fiber_cb func;
	void *arg;
} async_fiber_record_asm;


const char *async_fiber_backend_info()
{
	return "asm (boost.context 1.70.0)";
}

async_fiber *async_fiber_create_root()
{
	async_fiber_asm *fiber;

	fiber = ecalloc(1, sizeof(async_fiber_asm));

	fiber->initialized = 1;
	fiber->root = 1;
	
	return (async_fiber *) fiber;
}

async_fiber *async_fiber_create()
{
	async_fiber_asm *fiber;

	fiber = ecalloc(1, sizeof(async_fiber_asm));

	fiber->id = ++counter;

	return (async_fiber *) fiber;
}

static void async_fiber_asm_run(transfer_t trans)
{
	async_fiber_record_asm *record;
	
	record = (async_fiber_record_asm *) trans.data;

	ZEND_ASSERT(record != NULL);
	
	trans = jump_fcontext(trans.ctx, record->fiber);

	((async_fiber_asm *) trans.data)->ctx = trans.ctx;

	record->func(record->arg);
}

zend_bool async_fiber_init(async_fiber *fiber, async_context *context, async_fiber_cb func, void *arg, size_t stack_size)
{
	async_fiber_asm *impl;
	async_fiber_record_asm *record;
	
	void *sp;

	impl = (async_fiber_asm *) fiber;

	ZEND_ASSERT(impl->initialized == 0);

	if (UNEXPECTED(!async_fiber_stack_allocate(&impl->stack, stack_size))) {
		return 0;
	}

	if (UNEXPECTED(!record_size)) {
		record_size = (size_t) ceil((double) sizeof(async_fiber_record_asm) / 64) * 64;
	}

	sp = (void *) (impl->stack.size - record_size + (char *) impl->stack.pointer);

	record = (async_fiber_record_asm *) sp;
	record->fiber = impl;
	record->func = func;
	record->arg = arg;

	sp -= 64;

	impl->ctx = make_fcontext(sp, sp - (void *) impl->stack.pointer, async_fiber_asm_run);
	impl->ctx = jump_fcontext(impl->ctx, record).ctx;
	
	impl->initialized = 1;
	
	fiber->context = context;

	return 1;
}

void async_fiber_destroy(async_fiber *fiber)
{
	async_fiber_asm *impl;

	impl = (async_fiber_asm *) fiber;

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
	
	async_fiber_asm *from;
	async_fiber_asm *to;
	
	transfer_t trans;
	
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
	
	from = (async_fiber_asm *) current;
	to = (async_fiber_asm *) next;
	
	ZEND_ASSERT(next != NULL);
	ZEND_ASSERT(from->initialized);
	ZEND_ASSERT(to->initialized);
	
//	ASYNC_DEBUG_LOG("SUSPEND: %d -> %d\n", from->id, to->id);
	
	async_fiber_capture_state(current);
	ASYNC_G(fiber) = next;

	trans = jump_fcontext(to->ctx, from);
	
	((async_fiber_asm *) trans.data)->ctx = trans.ctx;
	
	async_fiber_restore_state(current);
}

void async_fiber_switch(async_task_scheduler *scheduler, async_fiber *next, async_fiber_suspend_type suspend)
{
	async_fiber *current;
	
	async_fiber_asm *from;
	async_fiber_asm *to;
	
	transfer_t trans;
	
	ZEND_ASSERT(scheduler != NULL);
	ZEND_ASSERT(next != NULL);
	
	current = ASYNC_G(fiber);
	
	ZEND_ASSERT(current != NULL);

	from = (async_fiber_asm *) current;
	to = (async_fiber_asm *) next;

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
	
//	ASYNC_DEBUG_LOG("SWITCH: %d -> %d\n", from->id, to->id);
	
	async_fiber_capture_state(current);	
	ASYNC_G(fiber) = next;
	
	trans = jump_fcontext(to->ctx, from);
	
	((async_fiber_asm *) trans.data)->ctx = trans.ctx;
	
	async_fiber_restore_state(current);
}
