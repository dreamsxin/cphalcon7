
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

#include "async/core.h"

#if PHALCON_USE_UV

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

static zend_object_handlers async_channel_handlers;
static zend_object_handlers async_channel_group_handlers;
static zend_object_handlers async_channel_iterator_handlers;

#define ASYNC_CHANNEL_FLAG_CLOSED 1

typedef struct {
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

typedef struct {
	/* PHP object handle. */
	zend_object std;
	
	/* Reference to the channel state being shared with iterators. */
	async_channel_state *state;
} async_channel;

#define ASYNC_CHANNEL_ITERATOR_FLAG_FETCHING 1

typedef struct {
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

typedef struct {
	/* Base async op data. */
	async_op base;
	
	/* Wrapped channel iterator. */
	async_channel_iterator *it;
	
	/* Key being used to register the iterator with the channel group. */
	zval key;
} async_channel_group_entry;

typedef struct {
	async_op base;
	zval value;
	async_channel_group_entry *entry;
} async_channel_send_op;

typedef struct {
	/* Base async op data. */
	async_op base;
	
	/* Number of pending select operations, needed to stop select if all channels are closed. */
	uint32_t pending;
	
	/* Refers to the registration of the iterator that completed the select. */
	async_channel_group_entry *entry;
} async_channel_group_select_op;

typedef struct {
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

typedef struct {
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
	
	/* Timeout paramter, -1 when a blocking call is requested. */
	zend_long timeout;
	
	/* Timer being used to stop select, only initialized if timeout > 0. */
	uv_timer_t timer;
} async_channel_group;

static async_channel_iterator *async_channel_iterator_object_create(async_channel_state *state);

#define ASYNC_CHANNEL_READABLE_NONBLOCK(state) ((state)->receivers.first != NULL || (state)->buffer.len > 0)
#define ASYNC_CHANNEL_READABLE(state) (!((state)->flags & ASYNC_CHANNEL_FLAG_CLOSED) || ASYNC_CHANNEL_READABLE_NONBLOCK(state))

static zend_always_inline void forward_error(zval *cause)
{
	zval error;
	
	ASYNC_PREPARE_EXCEPTION(&error, async_channel_closed_exception_ce, "Channel has been closed");

	zend_exception_set_previous(Z_OBJ_P(&error), Z_OBJ_P(cause));
	Z_ADDREF_P(cause);
	
	EG(current_execute_data)->opline--;
	zend_throw_exception_internal(&error);
	EG(current_execute_data)->opline++;
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

static ZEND_METHOD(Channel, __construct)
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

static ZEND_METHOD(Channel, getIterator)
{
	async_channel *channel;
	async_channel_iterator *it;

	ZEND_PARSE_PARAMETERS_NONE();
	
	channel = (async_channel *) Z_OBJ_P(getThis());
	
	it = async_channel_iterator_object_create(channel->state);

	RETURN_OBJ(&it->std);
}

static ZEND_METHOD(Channel, close)
{
	async_channel_state *state;
	
	zval *val;

	val = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();
	
	state = ((async_channel *) Z_OBJ_P(getThis()))->state;
	
	if (EXPECTED(state->cancel.func != NULL)) {
		ASYNC_LIST_REMOVE(&state->scheduler->shutdown, &state->cancel);
		
		state->cancel.func(state, (val == NULL || Z_TYPE_P(val) == IS_NULL) ? NULL : val);
	}
}

static ZEND_METHOD(Channel, isClosed)
{
	async_channel_state *state;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	state = ((async_channel *) Z_OBJ_P(getThis()))->state;
	
	RETURN_BOOL(state->cancel.func == NULL);
}

static ZEND_METHOD(Channel, isReadyForReceive)
{
	async_channel_state *state;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	state = ((async_channel *) Z_OBJ_P(getThis()))->state;
	
	RETURN_BOOL(ASYNC_CHANNEL_READABLE_NONBLOCK(state));
}

static ZEND_METHOD(Channel, isReadyForSend)
{
	async_channel_state *state;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	state = ((async_channel *) Z_OBJ_P(getThis()))->state;
	
	RETURN_BOOL(state->cancel.func != NULL && (state->receivers.first != NULL || state->buffer.len < state->buffer.size));
}

static ZEND_METHOD(Channel, send)
{
	async_channel_state *state;
	async_context *context;
	async_channel_send_op *send;
	async_op *op;
	
	zval *val;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(val)
	ZEND_PARSE_PARAMETERS_END();
	
	state = ((async_channel *) Z_OBJ_P(getThis()))->state;
	
	if (UNEXPECTED(Z_TYPE_P(&state->error) != IS_UNDEF)) {
		forward_error(&state->error);
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
	
	ASYNC_ALLOC_CUSTOM_OP(send, sizeof(async_channel_send_op));
	ASYNC_APPEND_OP(&state->senders, send);
	
	ZVAL_COPY(&send->value, val);
	
	context = async_context_get();
	
	if (!async_context_is_background(context)) {
		ASYNC_BUSY_ENTER(state->scheduler);
	}
	
	if (async_await_op((async_op *) send) == FAILURE) {
		forward_error(&send->base.result);
	}
	
	if (!async_context_is_background(context)) {
		ASYNC_BUSY_EXIT(state->scheduler);
	}
	
	zval_ptr_dtor(&send->value);

	ASYNC_FREE_OP(send);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_channel_ctor, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, capacity, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_channel_get_iterator, 0, 0, 0)
ZEND_END_ARG_INFO()

#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_channel_close, 0, 0, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_channel_is_closed, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_channel_is_ready_for_receive, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_channel_is_ready_for_send, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_channel_send, 0, 1, IS_VOID, 0)
	ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_channel_close, 0, 0, IS_VOID, NULL, 0)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_channel_is_closed, 0, 0, _IS_BOOL, NULL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_channel_is_ready_for_receive, 0, 0, _IS_BOOL, NULL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_channel_is_ready_for_send, 0, 0, _IS_BOOL, NULL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_channel_send, 0, 1, IS_VOID, NULL, 0)
	ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()
#endif

static const zend_function_entry channel_functions[] = {
	ZEND_ME(Channel, __construct, arginfo_channel_ctor, ZEND_ACC_PUBLIC)
	ZEND_ME(Channel, getIterator, arginfo_channel_get_iterator, ZEND_ACC_PUBLIC)
	ZEND_ME(Channel, close, arginfo_channel_close, ZEND_ACC_PUBLIC)
	ZEND_ME(Channel, isClosed, arginfo_channel_is_closed, ZEND_ACC_PUBLIC)
	ZEND_ME(Channel, isReadyForReceive, arginfo_channel_is_ready_for_receive, ZEND_ACC_PUBLIC)
	ZEND_ME(Channel, isReadyForSend, arginfo_channel_is_ready_for_send, ZEND_ACC_PUBLIC)
	ZEND_ME(Channel, send, arginfo_channel_send, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


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
	
	if (group->timeout > 0 && !uv_is_closing((uv_handle_t *) &group->timer)) {
		ASYNC_ADDREF(&group->std);
		
		uv_close((uv_handle_t *) &group->timer, dispose_group_timer);
	}
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

static ZEND_METHOD(ChannelGroup, __construct)
{
	async_channel_group *group;
	
	HashTable *map;
	zval *t;
	zval *entry;
	zval tmp;
	
	zend_long timeout;
	zend_long shuffle;
	zend_long h;
	zend_string *k;
	
	t = NULL;
	shuffle = 0;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 3)
		Z_PARAM_ARRAY_HT(map)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(t)
		Z_PARAM_LONG(shuffle)
	ZEND_PARSE_PARAMETERS_END();
	
	group = (async_channel_group *) Z_OBJ_P(getThis());
	
	if (t == NULL || Z_TYPE_P(t) == IS_NULL) {
		timeout = -1;
	} else {
		timeout = Z_LVAL_P(t);
		
		ASYNC_CHECK_ERROR(timeout < 0, "Timeout must not be negative, use NULL to disable timeout");
		
		if (timeout > 0) {
			uv_timer_init(&group->scheduler->loop, &group->timer);
			uv_unref((uv_handle_t *) &group->timer);
			
			group->timer.data = group;
		}
	}
	
	if (shuffle) {
		group->flags |= ASYNC_CHANNEL_GROUP_FLAG_SHUFFLE;
	}
	
	group->timeout = timeout;
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

#if PHP_VERSION_ID >= 70300
			zend_call_method_with_0_params(entry, Z_OBJCE_P(entry), &Z_OBJCE_P(entry)->iterator_funcs_ptr->zf_new_iterator, "getiterator", &tmp);
#else
			zend_call_method_with_0_params(entry, Z_OBJCE_P(entry), &Z_OBJCE_P(entry)->iterator_funcs.zf_new_iterator, "getiterator", &tmp);
#endif
			if (UNEXPECTED(EG(exception) || Z_TYPE_P(&tmp) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(&tmp), async_channel_iterator_ce))) {
				zval_ptr_dtor(&tmp);
				
				if (!EG(exception)) {
					zend_throw_error(NULL, "Aggregated iterator is not a channel iterator");
				}
				
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

static ZEND_METHOD(ChannelGroup, count)
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

static ZEND_METHOD(ChannelGroup, select)
{
	async_channel_group *group;
	async_channel_group_entry *entry;
	async_channel_group_entry *first;
	async_channel_state *state;
	async_context *context;
	
	zval *val;
	zval tmp;
	
	uint32_t i;
	uint32_t j;
	
	val = NULL;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL_DEREF(val);
	ZEND_PARSE_PARAMETERS_END();
	
	if (val != NULL) {
		zval_ptr_dtor(val);
		ZVAL_NULL(val);
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
				forward_error(&state->error);
				return;
			}
		}
	}
	
	// Perform a non-blocking select if a channel is ready.
	if (first != NULL && fetch_noblock(first->it->state, &tmp) == SUCCESS) {
		if (val != NULL) {
			ZVAL_COPY(val, &tmp);
		}
		
		zval_ptr_dtor(&tmp);
		
		RETURN_ZVAL(&first->key, 1, 0);
	}
	
	// No more channels left or non-blocking select early return.
	if (group->count == 0 || group->timeout == 0) {
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
	}
	
	context = async_context_get();
	
	if (!async_context_is_background(context)) {
		ASYNC_BUSY_ENTER(group->scheduler);
	}
	
	if (group->timeout > 0) {
		uv_timer_start(&group->timer, timeout_select, group->timeout, 0);
	}
	
	if (async_await_op((async_op *) &group->select) == FAILURE) {
		forward_error(&group->select.base.result);
	}
	
	if (group->timeout > 0) {
		uv_timer_stop(&group->timer);
	}
	
	if (!async_context_is_background(context)) {
		ASYNC_BUSY_EXIT(group->scheduler);
	}
	
	// Populate return values.
	if (EXPECTED(EG(exception) == NULL) && group->select.entry != NULL) {
		if (val != NULL) {
			ZVAL_COPY(val, &group->select.base.result);
		}
		
		if (USED_RET()) {
			ZVAL_COPY(return_value, &group->select.entry->key);
		}
	}
	
	// Cleanup pending operations.
	for (i = 0; i < group->count; i++) {
		entry = &group->entries[i];
		
		zval_ptr_dtor(&entry->base.result);
		
		if (entry->base.list) {
			ASYNC_LIST_REMOVE(&entry->it->state->receivers, (async_op *) entry);
		}
	}
	
	if (group->flags & ASYNC_CHANNEL_GROUP_FLAG_CHECK_CLOSED) {
		group->flags &= ~ASYNC_CHANNEL_GROUP_FLAG_CHECK_CLOSED;
		
		compact_group(group, group->select.entry);
	}
	
	zval_ptr_dtor(&group->select.base.result);
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
			ASYNC_PREPARE_EXCEPTION(&error, async_channel_closed_exception_ce, "Channel has been closed");
			
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
		ASYNC_PREPARE_EXCEPTION(&error, async_channel_closed_exception_ce, "Channel has been closed");
		ASYNC_FAIL_OP(group, &error);
		
		zval_ptr_dtor(&error);
		
		return;
	}
	
	group->entry = send->entry;
	
	ASYNC_RESOLVE_OP(group, &op->result);
}

static ZEND_METHOD(ChannelGroup, send)
{
	async_channel_group *group;
	async_channel_group_entry *entry;
	async_channel_group_entry *first;
	async_channel_state *state;
	async_context *context;
	
	async_channel_group_send_op *send;
	async_channel_send_op *op;
	async_op *receiver;
		
	uint32_t i;
	uint32_t j;
	
	zval *val;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(val);
	ZEND_PARSE_PARAMETERS_END();
	
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
				forward_error(&state->error);
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
	if (group->count == 0 || group->timeout == 0) {
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
	
	if (group->timeout > 0) {
		uv_timer_init(&group->scheduler->loop, &send->timer);
		uv_timer_start(&send->timer, timeout_send_cb, group->timeout, 0);
		
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
	
	if (group->timeout > 0) {
		uv_close((uv_handle_t *) &send->timer, dispose_send_timer_cb);
	} else {
		ASYNC_FREE_OP(send);
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_channel_group_ctor, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, channels, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, shuffle, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_channel_group_count, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_channel_group_select, 0, 0, 0)
	ZEND_ARG_INFO(1, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_channel_group_send, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

static const zend_function_entry channel_group_functions[] = {
	ZEND_ME(ChannelGroup, __construct, arginfo_channel_group_ctor, ZEND_ACC_PUBLIC)
	ZEND_ME(ChannelGroup, count, arginfo_channel_group_count, ZEND_ACC_PUBLIC)
	ZEND_ME(ChannelGroup, select, arginfo_channel_group_select, ZEND_ACC_PUBLIC)
	ZEND_ME(ChannelGroup, send, arginfo_channel_group_send, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static zend_always_inline void fetch_next_entry(async_channel_iterator *it)
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
		forward_error(&it->op.result);
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

static zend_always_inline void advance_iterator(async_channel_iterator *it)
{
	if (Z_TYPE_P(&it->entry) == IS_UNDEF) {
		if (it->pos < 0 && ASYNC_CHANNEL_READABLE(it->state)) {
			fetch_next_entry(it);
		} else if (Z_TYPE_P(&it->state->error) != IS_UNDEF) {
			forward_error(&it->state->error);
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

static ZEND_METHOD(ChannelIterator, rewind)
{
	async_channel_iterator *it;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	it = (async_channel_iterator *) Z_OBJ_P(getThis());
	
	advance_iterator(it);
}

static ZEND_METHOD(ChannelIterator, valid)
{
	async_channel_iterator *it;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	it = (async_channel_iterator *) Z_OBJ_P(getThis());
	
	advance_iterator(it);
	
	RETURN_BOOL(Z_TYPE_P(&it->entry) != IS_UNDEF);
}

static ZEND_METHOD(ChannelIterator, current)
{
	async_channel_iterator *it;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	it = (async_channel_iterator *) Z_OBJ_P(getThis());
	
	advance_iterator(it);
	
	if (EXPECTED(Z_TYPE_P(&it->entry) != IS_UNDEF)) {
		RETURN_ZVAL(&it->entry, 1, 0);
	}
}

static ZEND_METHOD(ChannelIterator, key)
{
	async_channel_iterator *it;
	
	ZEND_PARSE_PARAMETERS_NONE();
	
	it = (async_channel_iterator *) Z_OBJ_P(getThis());
	
	advance_iterator(it);
	
	if (EXPECTED(Z_TYPE_P(&it->entry) != IS_UNDEF)) {
		RETURN_LONG(it->pos);
	}
}

static ZEND_METHOD(ChannelIterator, next)
{
	async_channel_iterator *it;

	ZEND_PARSE_PARAMETERS_NONE();
	
	it = (async_channel_iterator *) Z_OBJ_P(getThis());
	
	if (EXPECTED(Z_TYPE_P(&it->entry) != IS_UNDEF)) {
		zval_ptr_dtor(&it->entry);
		ZVAL_UNDEF(&it->entry);
	}
	
	if (ASYNC_CHANNEL_READABLE(it->state)) {
		fetch_next_entry(it);
	} else if (Z_TYPE_P(&it->state->error) != IS_UNDEF) {
		forward_error(&it->state->error);
	}
}

ZEND_BEGIN_ARG_INFO(arginfo_channel_iterator_void, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry channel_iterator_functions[] = {
	ZEND_ME(ChannelIterator, rewind, arginfo_channel_iterator_void, ZEND_ACC_PUBLIC)
	ZEND_ME(ChannelIterator, valid, arginfo_channel_iterator_void, ZEND_ACC_PUBLIC)
	ZEND_ME(ChannelIterator, current, arginfo_channel_iterator_void, ZEND_ACC_PUBLIC)
	ZEND_ME(ChannelIterator, key, arginfo_channel_iterator_void, ZEND_ACC_PUBLIC)
	ZEND_ME(ChannelIterator, next, arginfo_channel_iterator_void, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static const zend_function_entry empty_funcs[] = {
	ZEND_FE_END
};


void async_channel_ce_register()
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\Channel", channel_functions);
	async_channel_ce = zend_register_internal_class(&ce);
	async_channel_ce->ce_flags |= ZEND_ACC_FINAL;
	async_channel_ce->create_object = async_channel_object_create;
	async_channel_ce->serialize = zend_class_serialize_deny;
	async_channel_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_channel_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_channel_handlers.free_obj = async_channel_object_destroy;
	async_channel_handlers.dtor_obj = async_channel_object_dtor;
	async_channel_handlers.clone_obj = NULL;
	
	zend_class_implements(async_channel_ce, 1, zend_ce_aggregate);
	
	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\ChannelGroup", channel_group_functions);
	async_channel_group_ce = zend_register_internal_class(&ce);
	async_channel_group_ce->ce_flags |= ZEND_ACC_FINAL;
	async_channel_group_ce->create_object = async_channel_group_object_create;
	async_channel_group_ce->serialize = zend_class_serialize_deny;
	async_channel_group_ce->unserialize = zend_class_unserialize_deny;
	
	memcpy(&async_channel_group_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_channel_group_handlers.free_obj = async_channel_group_object_destroy;
	async_channel_group_handlers.dtor_obj = async_channel_group_object_dtor;
	async_channel_group_handlers.clone_obj = NULL;
	
	zend_class_implements(async_channel_group_ce, 1, zend_ce_countable);
	
	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\ChannelIterator", channel_iterator_functions);
	async_channel_iterator_ce = zend_register_internal_class(&ce);
	async_channel_iterator_ce->ce_flags |= ZEND_ACC_FINAL;
	async_channel_iterator_ce->serialize = zend_class_serialize_deny;
	async_channel_iterator_ce->unserialize = zend_class_unserialize_deny;

	memcpy(&async_channel_iterator_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_channel_iterator_handlers.free_obj = async_channel_iterator_object_destroy;
	async_channel_iterator_handlers.clone_obj = NULL;
	
	zend_class_implements(async_channel_iterator_ce, 1, zend_ce_iterator);
	
	INIT_CLASS_ENTRY(ce, "Phalcon\\Async\\ChannelClosedException", empty_funcs);
	async_channel_closed_exception_ce = zend_register_internal_class(&ce);

	zend_do_inheritance(async_channel_closed_exception_ce, zend_ce_exception);
}

#endif /* PHALCON_USE_UV */
