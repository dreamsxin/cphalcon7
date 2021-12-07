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
#include "async/async_helper.h"

#include "kernel/backend.h"
#include <ext/standard/php_mt_rand.h>

#if PHP_VERSION_ID < 70200
#include <ext/spl/spl_iterators.h>
#ifndef zend_ce_countable
#define zend_ce_countable     spl_ce_Countable
#endif
#endif

ASYNC_API zend_class_entry *async_channel_ce;
ASYNC_API zend_class_entry *async_channel_closed_exception_ce;
ASYNC_API zend_class_entry *async_channel_group_ce;
ASYNC_API zend_class_entry *async_channel_iterator_ce;
ASYNC_API zend_class_entry *async_channel_select_ce;

static zend_object_handlers async_channel_handlers;
static zend_object_handlers async_channel_group_handlers;
static zend_object_handlers async_channel_iterator_handlers;
static zend_object_handlers async_channel_select_handlers;

static zend_string *str_key;
static zend_string *str_value;

#define ASYNC_CHANNEL_FLAG_CLOSED 1

typedef struct _async_channel_state {
	/* Refcount being used by channel and ietartor objects to share the state. */
	uint32_t refcount;

	/* Channel flags. */
	uint8_t flags;
	
	/* reference to the task scheduler. */
	async_task_scheduler *scheduler;
	
	/* Error object that has been passed as close reason. */
	zval error;
	
	/* Shutdown callback registered with the scheduler. */
	async_cancel_cb cancel;
	
	/* Pending send operations. */
	async_op_list senders;
	
	/* Pending receive operations. */
	async_op_list receivers;
	
	/* Array ring buffer used by the channel. */
	struct {
		uint16_t size;
		uint16_t len;
		uint16_t rpos;
		uint16_t wpos;
		zval *data;
	} buffer;
} async_channel_state;

typedef struct _async_channel {
	/* PHP object handle. */
	zend_object std;
	
	/* Reference to the channel state being shared with iterators. */
	async_channel_state *state;
} async_channel;

#define ASYNC_CHANNEL_ITERATOR_FLAG_FETCHING 1

typedef struct _async_channel_iterator {
	/* PHP object handle. */
	zend_object std;
	
	/* Iterator flags. */
	uint8_t flags;
	
	/* Reference to the channel state of the iterated channel. */
	async_channel_state *state;
	
	/* Current key (ascending counter). */
	zend_long pos;
	
	/* Current entry. */
	zval entry;
	
	/* Reused receive operation. */
	async_op op;
} async_channel_iterator;

typedef struct _async_channel_group_entry {
	/* Base async op data. */
	async_op base;
	
	/* Wrapped channel iterator. */
	async_channel_iterator *it;
	
	/* Key being used to register the iterator with the channel group. */
	zval key;
} async_channel_group_entry;

typedef struct _async_channel_send_op {
	async_op base;
	zval value;
	
	/* Timer being used to timeout a send. */
	uv_timer_t timer;

	async_channel_group_entry *entry;
} async_channel_send_op;

typedef struct _async_channel_group_select_op {
	/* Base async op data. */
	async_op base;
	
	/* Number of pending select operations, needed to stop select if all channels are closed. */
	uint32_t pending;
	
	/* Refers to the registration of the iterator that completed the select. */
	async_channel_group_entry *entry;
} async_channel_group_select_op;

typedef struct _async_channel_group_send_op {
	/* Base async op data. */
	async_op base;
	
	/* Additional op flags. */
	uint8_t flags;
	
	/* Timer being used to timeout a send. */
	uv_timer_t timer;
	
	/* Number of send operations. */
	uint32_t count;
	
	/* Refers to the registration of the iterator that completed the select. */
	async_channel_group_entry *entry;
	
	/* individual send ops, need to be allocated along with containing struct! */
	async_channel_send_op *sends;
} async_channel_group_send_op;

#define ASYNC_CHANNEL_GROUP_FLAG_SHUFFLE 1
#define ASYNC_CHANNEL_GROUP_FLAG_CHECK_CLOSED (1 << 1)

typedef struct _async_channel_group {
	/* PHP object handle. */
	zend_object std;
	
	/* Group flags. */
	uint8_t flags;
	
	/* Reference to the task scheudler. */
	async_task_scheduler *scheduler;
	
	/* Number of (supposedly) unclosed channel iterators. */
	uint32_t count;
	
	/* Array of registered channel iterators (closed channels will be removed without leaving gaps). */
	async_channel_group_entry *entries;
	
	/* Basic select operation being used to suspend the calling task. */
	async_channel_group_select_op select;
	
	/* Timer being used to stop select, only initialized if timeout > 0. */
	uv_timer_t timer;
} async_channel_group;

typedef struct _async_channel_select {
	zend_object std;
} async_channel_select;

static async_channel_iterator *async_channel_iterator_object_create(async_channel_state *state);

#define ASYNC_CHANNEL_READABLE_NONBLOCK(state) ((state)->receivers.first != NULL || (state)->buffer.len > 0)
#define ASYNC_CHANNEL_READABLE(state) (!((state)->flags & ASYNC_CHANNEL_FLAG_CLOSED) || ASYNC_CHANNEL_READABLE_NONBLOCK(state))

static zend_always_inline void forward_error(zval *cause, zend_execute_data *exec)
{
	zend_execute_data *prev;
	zval error;
	
	ASYNC_PREPARE_EXCEPTION(&error, exec, async_channel_closed_exception_ce, "Channel has been closed");

	zend_exception_set_previous(Z_OBJ_P(&error), Z_OBJ_P(cause));
	Z_ADDREF_P(cause);
	
	prev = EG(current_execute_data);
	EG(current_execute_data) = exec;

	exec->opline--;
#if PHP_VERSION_ID >= 80000
	zend_throw_exception_internal(Z_OBJ(error));
#else
	zend_throw_exception_internal(&error);
#endif
	exec->opline++;

	EG(current_execute_data) = prev;
}

static zend_always_inline int fetch_noblock(async_channel_state *state, zval *entry)
{
	async_channel_send_op *send;
	
	// Get next message from buffer.
	if (state->buffer.len > 0) {
		ZVAL_ZVAL(entry, &state->buffer.data[state->buffer.rpos], 0, 0);
		
		state->buffer.rpos = (state->buffer.rpos + 1) % state->buffer.size;
		
		if (state->senders.first != NULL) {
			ASYNC_NEXT_CUSTOM_OP(&state->senders, send, async_channel_send_op);
			
			ZVAL_COPY(&state->buffer.data[state->buffer.wpos], &send->value);
			
			state->buffer.wpos = (state->buffer.wpos + 1) % state->buffer.size;
			
			ASYNC_FINISH_OP(send);
		} else {
			state->buffer.len--;
		}
		
		return SUCCESS;
	}
	
	// Get next message from first pending send operation.
	if (state->senders.first != NULL) {
		ASYNC_NEXT_CUSTOM_OP(&state->senders, send, async_channel_send_op);
		
		ZVAL_COPY(entry, &send->value);
		
		ASYNC_FINISH_OP(send);
		
		return SUCCESS;
	}
	
	return FAILURE;
}

ASYNC_CALLBACK dispose_state(void *arg, zval *error)
{
	async_channel_state *state;
	async_op *op;
	
	state = (async_channel_state *) arg;
	
	ZEND_ASSERT(state != NULL);
	
	state->cancel.func = NULL;
	state->flags |= ASYNC_CHANNEL_FLAG_CLOSED;
	
	if (Z_TYPE_P(&state->error) == IS_UNDEF) {
		if (error != NULL) {
			ZVAL_COPY(&state->error, error);
		}
	}
	
	while (state->receivers.first != NULL) {
		ASYNC_NEXT_OP(&state->receivers, op);
		
		if (Z_TYPE_P(&state->error) == IS_UNDEF) {
			ASYNC_FINISH_OP(op);
		} else {
			ASYNC_FAIL_OP(op, &state->error);
		}
	}
	
	while (state->senders.first != NULL) {
		ASYNC_NEXT_OP(&state->senders, op);
		
		if (Z_TYPE_P(&state->error) == IS_UNDEF) {
			ASYNC_FINISH_OP(op);
		} else {
			ASYNC_FAIL_OP(op, &state->error);
		}
	}
}

static zend_always_inline async_channel_state *create_state()
{
	async_channel_state *state;
	
	state = ecalloc(1, sizeof(async_channel_state));
	
	state->refcount = 1;
	state->scheduler = async_task_scheduler_ref();
	
	state->cancel.object = state;
	state->cancel.func = dispose_state;
	
	ASYNC_LIST_APPEND(&state->scheduler->shutdown, &state->cancel);
	
	return state;
}

static zend_always_inline void release_state(async_channel_state *state)
{
	if (0 != --state->refcount) {
		return;
	}
	
	if (state->cancel.func != NULL) {
		ASYNC_LIST_REMOVE(&state->scheduler->shutdown, &state->cancel);
		
		state->cancel.func(state, NULL);
	}
	
	zval_ptr_dtor(&state->error);
	
	if (state->buffer.size > 0) {
		while (state->buffer.len > 0) {
			zval_ptr_dtor(&state->buffer.data[state->buffer.rpos]);
			
			state->buffer.rpos = (state->buffer.rpos + 1) % state->buffer.size;
			state->buffer.len--;
		}
		
		efree(state->buffer.data);
	}
	
	async_task_scheduler_unref(state->scheduler);
	
	efree(state);
}


static zend_object *async_channel_object_create(zend_class_entry *ce)
{
	async_channel *channel;
	
	channel = ecalloc(1, sizeof(async_channel));
	
	zend_object_std_init(&channel->std, ce);
	channel->std.handlers = &async_channel_handlers;
	
	channel->state = create_state();
	
	return &channel->std;
}

static void async_channel_object_dtor(zend_object *object)
{
	async_channel_state *state;
	
	state = ((async_channel *) object)->state;
	
	if (state->cancel.func != NULL) {
		ASYNC_LIST_REMOVE(&state->scheduler->shutdown, &state->cancel);
		
		state->cancel.func(state, NULL);
	}
}

static void async_channel_object_destroy(zend_object *object)
{
	async_channel *channel;
	
	channel = (async_channel *) object;
	
	release_state(channel->state);
	
	zend_object_std_dtor(&channel->std);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_channel_ctor, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, capacity, IS_LONG, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Channel, __construct)
{
	async_channel *channel;
	
	zend_long size;
	
	size = 0;
		
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(size)
	ZEND_PARSE_PARAMETERS_END();
	
	ASYNC_CHECK_ERROR(size < 0, "Channel buffer size must not be negative");
	ASYNC_CHECK_ERROR(size > 0xFFFF, "Maximum channel buffer size is %d", (int) size);
	
	channel = (async_channel *) Z_OBJ_P(getThis());
		
	channel->state->buffer.size = (uint16_t) size;
	
	if (size > 0) {
		channel->state->buffer.data = ecalloc(channel->state->buffer.size, sizeof(zval));
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_channel_get_iterator, 0, 0, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Channel, getIterator)
{
	async_channel *channel;
	async_channel_iterator *it;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	channel = (async_channel *) Z_OBJ_P(getThis());
	
	it = async_channel_iterator_object_create(channel->state);
	
	RETURN_OBJ(&it->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_channel_close, 0, 0, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 1)
ZEND_END_ARG_INFO();

static PHP_METHOD(Channel, close)
{
	async_channel_state *state;
	
	zval *val;

	val = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_OBJECT_OF_CLASS_EX(val, zend_ce_throwable, 1, 0)
	ZEND_PARSE_PARAMETERS_END();
	
	state = ((async_channel *) Z_OBJ_P(getThis()))->state;
	
	if (EXPECTED(state->cancel.func != NULL)) {
		ASYNC_LIST_REMOVE(&state->scheduler->shutdown, &state->cancel);
		
		state->cancel.func(state, (val == NULL || Z_TYPE_P(val) == IS_NULL) ? NULL : val);
	}
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_channel_is_closed, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(Channel, isClosed)
{
	async_channel_state *state;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	state = ((async_channel *) Z_OBJ_P(getThis()))->state;
	
	RETURN_BOOL(state->cancel.func == NULL);
}

ASYNC_CALLBACK dispose_channel_send_timer_cb(uv_handle_t *handle)
{
	async_channel_send_op *send;
	
	send = (async_channel_send_op *) handle->data;
	
	ZEND_ASSERT(send != NULL);
	
	ASYNC_FREE_OP(send);
}

ASYNC_CALLBACK timeout_channel_send_cb(uv_timer_t *timer)
{
	async_channel_send_op *send;
	
	send = (async_channel_send_op *) timer->data;
	
	ZEND_ASSERT(send != NULL);
	
	ASYNC_FINISH_OP(send);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_channel_send, 0, 1, IS_VOID, 0)
	ZEND_ARG_INFO(0, message)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 1)
ZEND_END_ARG_INFO();

static PHP_METHOD(Channel, send)
{
	async_channel_state *state;
	async_context *context;
	async_channel_send_op *send;
	async_op *op;
	
	zval *val;
	zend_long timeout;
	zval *millis = NULL;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 2)
		Z_PARAM_ZVAL(val)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(millis)
	ZEND_PARSE_PARAMETERS_END();

	if (millis == NULL || Z_TYPE_P(millis) == IS_NULL) {
		timeout = -1;
	} else {
		timeout = Z_LVAL_P(millis);
		
		ASYNC_CHECK_ERROR(timeout < 0, "Timeout must not be negative");
	}

	state = ((async_channel *) Z_OBJ_P(getThis()))->state;
	
	if (UNEXPECTED(Z_TYPE_P(&state->error) != IS_UNDEF)) {
		forward_error(&state->error, execute_data);
		return;
	}
	
	ASYNC_CHECK_EXCEPTION(state->flags & ASYNC_CHANNEL_FLAG_CLOSED, async_channel_closed_exception_ce, "Channel has been closed");
	
	// Fast forward message to first waiting receiver.
	if (state->receivers.first != NULL) {	
		ASYNC_NEXT_OP(&state->receivers, op);
		ASYNC_RESOLVE_OP(op, val);

		return;
	}
	
	// Put message into channel buffer.
	if (state->buffer.len < state->buffer.size) {
		ZVAL_COPY(&state->buffer.data[state->buffer.wpos], val);
		
		state->buffer.wpos = (state->buffer.wpos + 1) % state->buffer.size;
		state->buffer.len++;
		
		return;
	}

	// non-blocking select early return.
	if (timeout == 0) {
		return;
	}
	
	ASYNC_ALLOC_CUSTOM_OP(send, sizeof(async_channel_send_op));
	ASYNC_APPEND_OP(&state->senders, send);
	
	ZVAL_COPY(&send->value, val);
	
	context = async_context_get();
	
	if (!async_context_is_background(context)) {
		ASYNC_BUSY_ENTER(state->scheduler);
	}

	if (timeout > 0) {
		uv_timer_init(&state->scheduler->loop, &send->timer);
		uv_timer_start(&send->timer, timeout_channel_send_cb, timeout, 0);
		
		send->timer.data = send;
	}

	if (async_await_op((async_op *) send) == FAILURE) {
		forward_error(&send->base.result, execute_data);
	}
	
	if (!async_context_is_background(context)) {
		ASYNC_BUSY_EXIT(state->scheduler);
	}

	zval_ptr_dtor(&send->value);

	if (timeout > 0) {
		ASYNC_UV_CLOSE(&send->timer, dispose_channel_send_timer_cb);
	} else {
		ASYNC_FREE_OP(send);
	}
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_WAKEUP(Channel, async_channel_ce)
//LCOV_EXCL_STOP

static const zend_function_entry channel_functions[] = {
	PHP_ME(Channel, __construct, arginfo_channel_ctor, ZEND_ACC_PUBLIC)
	PHP_ME(Channel, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(Channel, getIterator, arginfo_channel_get_iterator, ZEND_ACC_PUBLIC)
	PHP_ME(Channel, close, arginfo_channel_close, ZEND_ACC_PUBLIC)
	PHP_ME(Channel, isClosed, arginfo_channel_is_closed, ZEND_ACC_PUBLIC)
	PHP_ME(Channel, send, arginfo_channel_send, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


static zend_always_inline async_channel_select *async_channel_select_obj(zend_object *object)
{
	return (async_channel_select *)((char *) object - XtOffsetOf(async_channel_select, std));
}

static zend_always_inline uint32_t async_monitor_event_prop_offset(zend_string *name)
{
	return zend_get_property_info(async_channel_select_ce, name, 1)->offset;
}

static async_channel_select *async_channel_select_object_create(zval *key, zval *value)
{
	async_channel_select *select;
	
	select = ecalloc(1, sizeof(async_channel_select) + zend_object_properties_size(async_channel_select_ce));
	
	zend_object_std_init(&select->std, async_channel_select_ce);	
	select->std.handlers = &async_channel_select_handlers;
	
	object_properties_init(&select->std, async_channel_select_ce);
	
	ZVAL_COPY(OBJ_PROP(&select->std, async_monitor_event_prop_offset(str_key)), key);
	ZVAL_COPY(OBJ_PROP(&select->std, async_monitor_event_prop_offset(str_value)), value);
	
	return select;
}

static void async_channel_select_object_destroy(zend_object *object)
{
	async_channel_select *select;
	
	select = async_channel_select_obj(object);
	
	zend_object_std_dtor(&select->std);
}


/* Performs a Fisher–Yates shuffle to randomize the channel entries array in-place. */
static void shuffle_group(async_channel_group *group)
{
	async_channel_group_entry entry;
	
	uint32_t i;
	uint32_t j;
	
	for (i = group->count - 1; i > 0; i--) {
		j = (uint32_t) php_mt_rand_common(0, i);
		entry = group->entries[i];
		
		group->entries[i] = group->entries[j];
		group->entries[j] = entry;
	}
}

static void compact_group(async_channel_group *group, async_channel_group_entry *selected)
{
	async_channel_group_entry *entry;
	async_channel_state *state;
	
	uint32_t i;
	uint32_t j;
	
	for (i = 0; i < group->count; i++) {
		entry = &group->entries[i];
		state = entry->it->state;
	
		if (state->flags & ASYNC_CHANNEL_FLAG_CLOSED) {
			if (Z_TYPE_P(&state->error) == IS_UNDEF || entry == selected) {
				ASYNC_DELREF(&entry->it->std);
				zval_ptr_dtor(&entry->key);
			
				for (j = i + 1; j < group->count; j++) {
					group->entries[j - 1] = group->entries[j];
				}
				
				group->count--;
				i--;
			}
		}
	}
}

static zend_object *async_channel_group_object_create(zend_class_entry *ce)
{
	async_channel_group *group;
	
	group = ecalloc(1, sizeof(async_channel_group));
	
	zend_object_std_init(&group->std, ce);
	group->std.handlers = &async_channel_group_handlers;
	
	group->scheduler = async_task_scheduler_ref();
	
	return &group->std;
}

ASYNC_CALLBACK dispose_group_timer(uv_handle_t *handle)
{
	async_channel_group *group;
	
	group = (async_channel_group *) handle->data;
	
	ASYNC_DELREF(&group->std);
}

static void async_channel_group_object_dtor(zend_object *object)
{
	async_channel_group *group;
	
	group = (async_channel_group *) object;
	
	ASYNC_UV_TRY_CLOSE_REF(&group->std, &group->timer, dispose_group_timer);
}

static void async_channel_group_object_destroy(zend_object *object)
{
	async_channel_group *group;
	
	uint32_t i;
	
	group = (async_channel_group *) object;
	
	if (group->entries != NULL) {
		for (i = 0; i < group->count; i++) {
			ASYNC_DELREF(&group->entries[i].it->std);
			zval_ptr_dtor(&group->entries[i].key);
		}
	
		efree(group->entries);
	}
	
	async_task_scheduler_unref(group->scheduler);
	
	zend_object_std_dtor(&group->std);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_channel_group_ctor, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, channels, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, shuffle, _IS_BOOL, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(ChannelGroup, __construct)
{
	async_channel_group *group;
	
	HashTable *map;
	zval *entry;
	zval tmp;
	
	zend_long shuffle;
	zend_long h;
	zend_string *k;
	
	shuffle = 0;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 3)
		Z_PARAM_ARRAY_HT(map)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(shuffle)
	ZEND_PARSE_PARAMETERS_END();
	
	group = (async_channel_group *) Z_OBJ_P(getThis());

	uv_timer_init(&group->scheduler->loop, &group->timer);
	uv_unref((uv_handle_t *) &group->timer);
	
	group->timer.data = group;
	
	if (shuffle) {
		group->flags |= ASYNC_CHANNEL_GROUP_FLAG_SHUFFLE;
	}
	
	group->entries = ecalloc(zend_array_count(map), sizeof(async_channel_group_entry));
	
	ZEND_HASH_FOREACH_KEY_VAL(map, h, k, entry) {
		if (UNEXPECTED(Z_TYPE_P(entry) != IS_OBJECT)) {
			zend_throw_error(NULL, "Select requires all inputs to be objects");
			return;
		}
		
		if (!instanceof_function(Z_OBJCE_P(entry), async_channel_iterator_ce)) {
			if (UNEXPECTED(!instanceof_function(Z_OBJCE_P(entry), zend_ce_aggregate))) {
				zend_throw_error(NULL, "Select requires all inputs to be channel iterators or provide such an iterator via IteratorAggregate");
				return;
			}

#if PHP_VERSION_ID >= 80000
			zend_call_method_with_0_params(Z_OBJ_P(entry), Z_OBJCE_P(entry), &Z_OBJCE_P(entry)->iterator_funcs_ptr->zf_new_iterator, "getiterator", &tmp);
#elif PHP_VERSION_ID >= 70300
			zend_call_method_with_0_params(entry, Z_OBJCE_P(entry), &Z_OBJCE_P(entry)->iterator_funcs_ptr->zf_new_iterator, "getiterator", &tmp);
#else
			zend_call_method_with_0_params(entry, Z_OBJCE_P(entry), &Z_OBJCE_P(entry)->iterator_funcs.zf_new_iterator, "getiterator", &tmp);
#endif
			if (UNEXPECTED(EG(exception) || Z_TYPE_P(&tmp) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(&tmp), async_channel_iterator_ce))) {
				ASYNC_ENSURE_ERROR("Aggregated iterator is not a channel iterator");
				zval_ptr_dtor(&tmp);
				return;
			}
			
			group->entries[group->count].it = (async_channel_iterator *) Z_OBJ_P(&tmp);
		} else {
			Z_ADDREF_P(entry);
			
			group->entries[group->count].it = (async_channel_iterator *) Z_OBJ_P(entry);
		}
		
		if (k == NULL) {
			ZVAL_LONG(&group->entries[group->count].key, h);
		} else {
			ZVAL_STR_COPY(&group->entries[group->count].key, k);
		}
		
		group->count++;
	} ZEND_HASH_FOREACH_END();
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_channel_group_count, 0, 0, 0)
ZEND_END_ARG_INFO();

static PHP_METHOD(ChannelGroup, count)
{
	async_channel_group *group;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	group = (async_channel_group *) Z_OBJ_P(getThis());
	
	RETURN_LONG(group->count);
}

ASYNC_CALLBACK continue_select_cb(async_op *op)
{
	async_channel_group *group;
	async_channel_group_entry *entry;
	
	group = (async_channel_group *) op->arg;
	entry = (async_channel_group_entry *) op;
	
	group->select.pending--;
	
	entry->it->flags &= ~ASYNC_CHANNEL_ITERATOR_FLAG_FETCHING;
	
	if (UNEXPECTED(entry->it->state->flags & ASYNC_CHANNEL_FLAG_CLOSED)) {
		group->flags |= ASYNC_CHANNEL_GROUP_FLAG_CHECK_CLOSED;
	}
	
	if (UNEXPECTED(op->status == ASYNC_STATUS_FAILED)) {
		group->select.entry = entry;
		
		ASYNC_FAIL_OP(&group->select, &op->result);
		
		return;
	}
	
	if (UNEXPECTED(entry->it->state->flags & ASYNC_CHANNEL_FLAG_CLOSED)) {
		if (group->select.pending == 0) {
			ASYNC_FINISH_OP(&group->select);
		}
		
		return;
	}
	
	group->select.entry = entry;
	
	ASYNC_RESOLVE_OP(&group->select, &op->result);
}

ASYNC_CALLBACK timeout_select(uv_timer_t *timer)
{
	async_channel_group *group;
	
	group = (async_channel_group *) timer->data;
	
	ASYNC_FINISH_OP(&group->select);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_channel_group_select, 0, 0, Phalcon\\Async\\ChannelSelect, 1)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 1)
ZEND_END_ARG_INFO();

static PHP_METHOD(ChannelGroup, select)
{
	async_channel_group *group;
	async_channel_select *select;
	async_channel_group_entry *entry;
	async_channel_group_entry *first;
	async_channel_state *state;
	async_context *context;
	
	zend_long timeout;
	zval *millis;
	zval tmp;
	
	uint32_t i;
	uint32_t j;
	
	millis = NULL;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(millis)
	ZEND_PARSE_PARAMETERS_END();
	
	if (millis == NULL || Z_TYPE_P(millis) == IS_NULL) {
		timeout = -1;
	} else {
		timeout = Z_LVAL_P(millis);
		
		ASYNC_CHECK_ERROR(timeout < 0, "Timeout must not be negative");
	}
	
	group = (async_channel_group *) Z_OBJ_P(getThis());
	
	if (group->flags & ASYNC_CHANNEL_GROUP_FLAG_SHUFFLE) {
		shuffle_group(group);
	}
	
	for (first = NULL, i = 0; i < group->count; i++) {
		entry = &group->entries[i];
		state = entry->it->state;
		
		if (ASYNC_CHANNEL_READABLE_NONBLOCK(state)) {
			if (first == NULL) {
				first = entry;
			}
			
			continue;
		}
		
		// Perform error forwarding and inline compaction of the group.
		if (state->flags & ASYNC_CHANNEL_FLAG_CLOSED) {
			ASYNC_DELREF(&entry->it->std);
			zval_ptr_dtor(&entry->key);
		
			for (j = i + 1; j < group->count; j++) {
				group->entries[j - 1] = group->entries[j];
			}
			
			group->count--;
			i--;
			
			if (UNEXPECTED(Z_TYPE_P(&state->error) != IS_UNDEF)) {
				forward_error(&state->error, execute_data);
				return;
			}
		}
	}
	
	// Perform a non-blocking select if a channel is ready.
	if (first != NULL && fetch_noblock(first->it->state, &tmp) == SUCCESS) {
		select = async_channel_select_object_create(&first->key, &tmp);
		
		zval_ptr_dtor(&tmp);
		
		RETURN_OBJ(&select->std);
	}
	
	// No more channels left or non-blocking select early return.
	if (group->count == 0 || timeout == 0) {
		return;
	}
	
	// Register select operations with input channels and start the race.
	group->select.base.status = ASYNC_STATUS_PENDING;
	group->select.base.flags = 0;
	
	group->select.pending = group->count;
	group->select.entry = NULL;
	
	for (i = 0; i < group->count; i++) {
		entry = &group->entries[i];
	
		entry->base.status = ASYNC_STATUS_RUNNING;
		entry->base.flags = 0;
		entry->base.callback = continue_select_cb;
		entry->base.arg = group;
	
		ASYNC_APPEND_OP(&entry->it->state->receivers, entry);
		
		entry->it->flags |= ASYNC_CHANNEL_ITERATOR_FLAG_FETCHING;
	}
	
	context = async_context_get();
	
	if (!async_context_is_background(context)) {
		ASYNC_BUSY_ENTER(group->scheduler);
	}
	
	if (timeout > 0) {
		uv_timer_start(&group->timer, timeout_select, timeout, 0);
	}
	
	if (async_await_op((async_op *) &group->select) == FAILURE) {
		forward_error(&group->select.base.result, execute_data);
	}
	
	if (timeout > 0) {
		uv_timer_stop(&group->timer);
	}
	
	if (!async_context_is_background(context)) {
		ASYNC_BUSY_EXIT(group->scheduler);
	}
	
	// Populate return values.
	if (EXPECTED(EG(exception) == NULL) && group->select.entry != NULL) {
		select = async_channel_select_object_create(&group->select.entry->key, &group->select.base.result);
	} else {
		select = NULL;
	}
	
	// Cleanup pending operations.
	for (i = 0; i < group->count; i++) {
		entry = &group->entries[i];
		
		zval_ptr_dtor(&entry->base.result);
		
		if (entry->base.list) {
			entry->it->flags &= ~ASYNC_CHANNEL_ITERATOR_FLAG_FETCHING;
		
			ASYNC_LIST_REMOVE(&entry->it->state->receivers, (async_op *) entry);
		}
	}
	
	if (group->flags & ASYNC_CHANNEL_GROUP_FLAG_CHECK_CLOSED) {
		group->flags &= ~ASYNC_CHANNEL_GROUP_FLAG_CHECK_CLOSED;
		
		compact_group(group, group->select.entry);
	}
	
	zval_ptr_dtor(&group->select.base.result);
	
	if (EXPECTED(select != NULL)) {
		RETURN_OBJ(&select->std);
	}
}

ASYNC_CALLBACK dispose_send_timer_cb(uv_handle_t *handle)
{
	async_channel_group_send_op *send;
	
	send = (async_channel_group_send_op *) handle->data;
	
	ZEND_ASSERT(send != NULL);
	
	ASYNC_FREE_OP(send);
}

ASYNC_CALLBACK timeout_send_cb(uv_timer_t *timer)
{
	async_channel_group_send_op *send;
	
	send = (async_channel_group_send_op *) timer->data;
	
	ZEND_ASSERT(send != NULL);
	
	ASYNC_FINISH_OP(send);
}

ASYNC_CALLBACK continue_send_cb(async_op *op)
{
	async_channel_send_op *send;
	async_channel_group_send_op *group;
	
	zval error;
	
	send = (async_channel_send_op *) op;
	group = (async_channel_group_send_op *) op->arg;
	
	if (UNEXPECTED(send->entry->it->state->flags & ASYNC_CHANNEL_FLAG_CLOSED)) {
		group->flags |= ASYNC_CHANNEL_GROUP_FLAG_CHECK_CLOSED;
	}
	
	if (UNEXPECTED(op->status == ASYNC_STATUS_FAILED)) {
		group->entry = send->entry;
		
		if (EXPECTED(send->entry->it->state->flags & ASYNC_CHANNEL_FLAG_CLOSED)) {
			ASYNC_PREPARE_SCHEDULER_EXCEPTION(&error, async_channel_closed_exception_ce, "Channel has been closed");
			
			zend_exception_set_previous(Z_OBJ_P(&error), Z_OBJ_P(&op->result));
			Z_ADDREF_P(&op->result);
			
			ASYNC_FAIL_OP(group, &error);
			
			zval_ptr_dtor(&error);
		} else {		
			ASYNC_FAIL_OP(group, &op->result);
		}
		
		return;
	}
	
	if (UNEXPECTED(send->entry->it->state->flags & ASYNC_CHANNEL_FLAG_CLOSED)) {
		ASYNC_PREPARE_SCHEDULER_EXCEPTION(&error, async_channel_closed_exception_ce, "Channel has been closed");
		ASYNC_FAIL_OP(group, &error);
		
		zval_ptr_dtor(&error);
		
		return;
	}
	
	group->entry = send->entry;
	
	ASYNC_RESOLVE_OP(group, &op->result);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_channel_group_send, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 1)
ZEND_END_ARG_INFO();

static PHP_METHOD(ChannelGroup, send)
{
	async_channel_group *group;
	async_channel_group_entry *entry;
	async_channel_group_entry *first;
	async_channel_state *state;
	async_context *context;
	
	async_channel_group_send_op *send;
	async_channel_send_op *op;
	async_op *receiver;
	
	zend_long timeout;
	zval *millis;
	zval *val;
		
	uint32_t i;
	uint32_t j;
	
	millis = NULL;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 2)
		Z_PARAM_ZVAL(val)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(millis)
	ZEND_PARSE_PARAMETERS_END();
	
	if (millis == NULL || Z_TYPE_P(millis) == IS_NULL) {
		timeout = -1;
	} else {
		timeout = Z_LVAL_P(millis);
		
		ASYNC_CHECK_ERROR(timeout < 0, "Timeout must not be negative");
	}
	
	group = (async_channel_group *) Z_OBJ_P(getThis());
	
	if (group->flags & ASYNC_CHANNEL_GROUP_FLAG_SHUFFLE) {
		shuffle_group(group);
	}
	
	// Check for writable channels first.
	for (first = NULL, i = 0; i < group->count; i++) {
		entry = &group->entries[i];
		state = entry->it->state;
		
		if (first == NULL) {
			if (state->receivers.first != NULL) {
				first = entry;
			} else if (state->buffer.len < state->buffer.size) {
				first = entry;
			}
		}
		
		// Perform inline compaction of the group.
		if (state->flags & ASYNC_CHANNEL_FLAG_CLOSED) {
			ASYNC_DELREF(&entry->it->std);
			zval_ptr_dtor(&entry->key);
			
			for (j = i + 1; j < group->count; j++) {
				group->entries[j - 1] = group->entries[j];
			}
			
			group->count--;
			i--;
			
			if (UNEXPECTED(Z_TYPE_P(&state->error) != IS_UNDEF)) {
				forward_error(&state->error, execute_data);
				return;
			}
			
			zend_throw_exception(async_channel_closed_exception_ce, "Channel has been closed", 0);
			return;
		}
	}
	
	// Fast forward value into receiver or channel buffer.
	if (first) {
		state = first->it->state;
				
		if (state->receivers.first != NULL) {
			ASYNC_NEXT_OP(&state->receivers, receiver);
			ASYNC_RESOLVE_OP(receiver, val);	
		} else {
			ZVAL_COPY(&state->buffer.data[state->buffer.wpos], val);
			
			state->buffer.wpos = (state->buffer.wpos + 1) % state->buffer.size;
			state->buffer.len++;
		}
		
		RETURN_ZVAL(&first->key, 1, 0);
	}
	
	// No more channels left or non-blocking send early return.
	if (group->count == 0 || timeout == 0) {
		return;
	}
	
	ASYNC_ALLOC_CUSTOM_OP(send, sizeof(async_channel_group_send_op) + group->count * sizeof(async_channel_send_op));
	
	send->count = group->count;
	
	op = (async_channel_send_op *) ((char *) send + XtOffsetOf(async_channel_group_send_op, sends));
	
	for (i = 0; i < group->count; i++) {
		entry = &group->entries[i];
		
		ZVAL_ZVAL(&op->value, val, 0, 0);
		
		op->entry = entry;
		op->base.status = ASYNC_STATUS_RUNNING;
		op->base.callback = continue_send_cb;
		op->base.arg = send;
		
		ASYNC_APPEND_OP(&entry->it->state->senders, op);
		
		op++;
	}
	
	context = async_context_get();
	
	if (!async_context_is_background(context)) {
		ASYNC_BUSY_ENTER(group->scheduler);
	}
	
	if (timeout > 0) {
		uv_timer_init(&group->scheduler->loop, &send->timer);
		uv_timer_start(&send->timer, timeout_send_cb, timeout, 0);
		
		send->timer.data = send;
	}
	
	if (async_await_op((async_op *) send) == FAILURE) {	
		ASYNC_FORWARD_OP_ERROR(send);
	}
	
	if (!async_context_is_background(context)) {
		ASYNC_BUSY_EXIT(group->scheduler);
	}
	
	if (send->entry && USED_RET() && !EG(exception)) {
		ZVAL_COPY(return_value, &send->entry->key);
	}
	
	op = (async_channel_send_op *) ((char *) send + XtOffsetOf(async_channel_group_send_op, sends));
	
	for (i = 0; i < send->count; i++) {
		zval_ptr_dtor(&op->base.result);
		
		if (op->base.list) {
			ASYNC_LIST_REMOVE(&op->entry->it->state->senders, (async_op *) op);
		}
	
		op++;
	}
	
	if (send->flags & ASYNC_CHANNEL_GROUP_FLAG_CHECK_CLOSED) {
		compact_group(group, send->entry);
	}
	
	if (timeout > 0) {
		ASYNC_UV_CLOSE(&send->timer, dispose_send_timer_cb);
	} else {
		ASYNC_FREE_OP(send);
	}
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_WAKEUP(ChannelGroup, async_channel_group_ce)
//LCOV_EXCL_STOP

static const zend_function_entry channel_group_functions[] = {
	PHP_ME(ChannelGroup, __construct, arginfo_channel_group_ctor, ZEND_ACC_PUBLIC)
	PHP_ME(ChannelGroup, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(ChannelGroup, count, arginfo_channel_group_count, ZEND_ACC_PUBLIC)
	PHP_ME(ChannelGroup, select, arginfo_channel_group_select, ZEND_ACC_PUBLIC)
	PHP_ME(ChannelGroup, send, arginfo_channel_group_send, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


static zend_always_inline void fetch_next_entry(async_channel_iterator *it, zend_execute_data *exec)
{
	async_channel_state *state;
	async_context *context;
	
	ASYNC_CHECK_ERROR(it->flags & ASYNC_CHANNEL_ITERATOR_FLAG_FETCHING, "Cannot advance iterator while already awaiting next channel value");
	
	state = it->state;
	
	if (fetch_noblock(state, &it->entry) == SUCCESS) {
		it->pos++;
	
		return;
	}
	
	// Queue up receiver and mark the iterator as fetching next value.
	it->flags |= ASYNC_CHANNEL_ITERATOR_FLAG_FETCHING;
	
	ASYNC_APPEND_OP(&state->receivers, &it->op);
	
	context = async_context_get();
	
	if (!async_context_is_background(context)) {
		ASYNC_BUSY_ENTER(state->scheduler);
	}
	
	if (async_await_op(&it->op) == FAILURE) {
		forward_error(&it->op.result, exec);
	} else if (!(state->flags & ASYNC_CHANNEL_FLAG_CLOSED)) {
		it->pos++;
		
		ZVAL_COPY(&it->entry, &it->op.result);
	}
	
	if (!async_context_is_background(context)) {
		ASYNC_BUSY_EXIT(state->scheduler);
	}
	
	ASYNC_RESET_OP(&it->op);
	
	it->flags &= ~ASYNC_CHANNEL_ITERATOR_FLAG_FETCHING;
}

static zend_always_inline void advance_iterator(async_channel_iterator *it, zend_execute_data *exec)
{
	if (Z_TYPE_P(&it->entry) == IS_UNDEF) {
		if (it->pos < 0 && ASYNC_CHANNEL_READABLE(it->state)) {
			fetch_next_entry(it, exec);
		} else if (Z_TYPE_P(&it->state->error) != IS_UNDEF) {
			forward_error(&it->state->error, exec);
		}
	}
}

static async_channel_iterator *async_channel_iterator_object_create(async_channel_state *state)
{
	async_channel_iterator *it;
	
	it = ecalloc(1, sizeof(async_channel_iterator));
	
	zend_object_std_init(&it->std, async_channel_iterator_ce);
	it->std.handlers = &async_channel_iterator_handlers;
	
	it->pos = -1;
	it->state = state;
	
	state->refcount++;
	
	return it;
}

static void async_channel_iterator_object_destroy(zend_object *object)
{
	async_channel_iterator *it;
	
	it = (async_channel_iterator *) object;
	
	zval_ptr_dtor(&it->entry);
	
	release_state(it->state);
	
	zend_object_std_dtor(&it->std);
}

static PHP_METHOD(ChannelIterator, rewind)
{
	async_channel_iterator *it;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	it = (async_channel_iterator *) Z_OBJ_P(getThis());
	
	advance_iterator(it, execute_data);
}

static PHP_METHOD(ChannelIterator, valid)
{
	async_channel_iterator *it;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	it = (async_channel_iterator *) Z_OBJ_P(getThis());
	
	advance_iterator(it, execute_data);
	
	RETURN_BOOL(Z_TYPE_P(&it->entry) != IS_UNDEF);
}

static PHP_METHOD(ChannelIterator, current)
{
	async_channel_iterator *it;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	it = (async_channel_iterator *) Z_OBJ_P(getThis());
	
	advance_iterator(it, execute_data);
	
	if (EXPECTED(Z_TYPE_P(&it->entry) != IS_UNDEF)) {
		RETURN_ZVAL(&it->entry, 1, 0);
	}
}

static PHP_METHOD(ChannelIterator, key)
{
	async_channel_iterator *it;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	it = (async_channel_iterator *) Z_OBJ_P(getThis());
	
	advance_iterator(it, execute_data);
	
	if (EXPECTED(Z_TYPE_P(&it->entry) != IS_UNDEF)) {
		RETURN_LONG(it->pos);
	}
}

static PHP_METHOD(ChannelIterator, next)
{
	async_channel_iterator *it;

	ZEND_PARSE_PARAMETERS_NONE();
	
	it = (async_channel_iterator *) Z_OBJ_P(getThis());
	
	if (EXPECTED(Z_TYPE_P(&it->entry) != IS_UNDEF)) {
		zval_ptr_dtor(&it->entry);
		ZVAL_UNDEF(&it->entry);
	}
	
	if (ASYNC_CHANNEL_READABLE(it->state)) {
		fetch_next_entry(it, execute_data);
	} else if (Z_TYPE_P(&it->state->error) != IS_UNDEF) {
		forward_error(&it->state->error, execute_data);
	}
}

ZEND_BEGIN_ARG_INFO(arginfo_channel_iterator_void, 0)
ZEND_END_ARG_INFO();

//LCOV_EXCL_START
ASYNC_METHOD_NO_CTOR(ChannelIterator, async_channel_iterator_ce)
ASYNC_METHOD_NO_WAKEUP(ChannelIterator, async_channel_iterator_ce)
//LCOV_EXCL_STOP

static const zend_function_entry channel_iterator_functions[] = {
	PHP_ME(ChannelIterator, __construct, arginfo_no_ctor, ZEND_ACC_PRIVATE)
	PHP_ME(ChannelIterator, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(ChannelIterator, rewind, arginfo_channel_iterator_void, ZEND_ACC_PUBLIC)
	PHP_ME(ChannelIterator, valid, arginfo_channel_iterator_void, ZEND_ACC_PUBLIC)
	PHP_ME(ChannelIterator, current, arginfo_channel_iterator_void, ZEND_ACC_PUBLIC)
	PHP_ME(ChannelIterator, key, arginfo_channel_iterator_void, ZEND_ACC_PUBLIC)
	PHP_ME(ChannelIterator, next, arginfo_channel_iterator_void, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


static const zend_function_entry empty_funcs[] = {
	PHP_FE_END
};

void async_channel_ce_register()
{
	zend_class_entry ce;

	str_key = zend_new_interned_string(zend_string_init(ZEND_STRL("key"), 1));
	str_value = zend_new_interned_string(zend_string_init(ZEND_STRL("value"), 1));

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async", "Channel", channel_functions);
	async_channel_ce = zend_register_internal_class(&ce);
	async_channel_ce->ce_flags |= ZEND_ACC_FINAL;
	async_channel_ce->create_object = async_channel_object_create;
#if PHP_VERSION_ID >= 80100
	async_channel_ce->ce_flags |= ZEND_ACC_NOT_SERIALIZABLE;
#else
	async_channel_ce->serialize = zend_class_serialize_deny;
	async_channel_ce->unserialize = zend_class_unserialize_deny;
#endif
	memcpy(&async_channel_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_channel_handlers.free_obj = async_channel_object_destroy;
	async_channel_handlers.dtor_obj = async_channel_object_dtor;
	async_channel_handlers.clone_obj = NULL;
	
	zend_class_implements(async_channel_ce, 1, zend_ce_aggregate);
	
	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async", "ChannelGroup", channel_group_functions);
	async_channel_group_ce = zend_register_internal_class(&ce);
	async_channel_group_ce->ce_flags |= ZEND_ACC_FINAL;
	async_channel_group_ce->create_object = async_channel_group_object_create;
#if PHP_VERSION_ID >= 80100
	async_channel_group_ce->ce_flags |= ZEND_ACC_NOT_SERIALIZABLE;
#else
	async_channel_group_ce->serialize = zend_class_serialize_deny;
	async_channel_group_ce->unserialize = zend_class_unserialize_deny;
#endif
	memcpy(&async_channel_group_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_channel_group_handlers.free_obj = async_channel_group_object_destroy;
	async_channel_group_handlers.dtor_obj = async_channel_group_object_dtor;
	async_channel_group_handlers.clone_obj = NULL;
	
	zend_class_implements(async_channel_group_ce, 1, zend_ce_countable);
	
	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async", "ChannelIterator", channel_iterator_functions);
	async_channel_iterator_ce = zend_register_internal_class(&ce);
	async_channel_iterator_ce->ce_flags |= ZEND_ACC_FINAL;
#if PHP_VERSION_ID >= 80100
	async_channel_iterator_ce->ce_flags |= ZEND_ACC_NOT_SERIALIZABLE;
#else
	async_channel_iterator_ce->serialize = zend_class_serialize_deny;
	async_channel_iterator_ce->unserialize = zend_class_unserialize_deny;
#endif

	memcpy(&async_channel_iterator_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_channel_iterator_handlers.free_obj = async_channel_iterator_object_destroy;
	async_channel_iterator_handlers.clone_obj = NULL;
	
	zend_class_implements(async_channel_iterator_ce, 1, zend_ce_iterator);
	
	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async", "ChannelSelect", empty_funcs);
	async_channel_select_ce = zend_register_internal_class(&ce);
	async_channel_select_ce->ce_flags |= ZEND_ACC_FINAL;
	async_channel_select_ce->create_object = NULL;
#if PHP_VERSION_ID >= 80100
	async_channel_select_ce->ce_flags |= ZEND_ACC_NOT_SERIALIZABLE;
#else
	async_channel_select_ce->serialize = zend_class_serialize_deny;
	async_channel_select_ce->unserialize = zend_class_unserialize_deny;
#endif

	memcpy(&async_channel_select_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_channel_select_handlers.offset = XtOffsetOf(async_channel_select, std);
	async_channel_select_handlers.free_obj = async_channel_select_object_destroy;
	async_channel_select_handlers.clone_obj = NULL;
	async_channel_select_handlers.write_property = async_prop_write_handler_readonly;
	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async", "ChannelClosedException", empty_funcs);
	async_channel_closed_exception_ce = zend_register_internal_class(&ce);

	zend_do_inheritance(async_channel_closed_exception_ce, zend_ce_exception);
	
	zend_declare_property_null(async_channel_select_ce, ZEND_STRL("key"), ZEND_ACC_PUBLIC);
	zend_declare_property_null(async_channel_select_ce, ZEND_STRL("value"), ZEND_ACC_PUBLIC);
}

void async_channel_ce_unregister()
{
	zend_string_release(str_key);
	zend_string_release(str_value);
}