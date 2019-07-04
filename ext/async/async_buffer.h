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

#ifndef ASYNC_BUFFER_H
#define ASYNC_BUFFER_H

typedef struct _async_ring_buffer {
	/* Base pointer being used to allocate and free buffer memory. */
	char *base;
	
	/* Current read position. */
	char *rpos;
	
	/* Current write position. */
	char *wpos;
	
	/* Buffer size. */
	size_t size;
	
	/* Number of buffered bytes. */
	size_t len;
} async_ring_buffer;

static zend_always_inline size_t async_ring_buffer_read_len(async_ring_buffer *buffer)
{
	if (buffer->len == 0) {
		return 0;
	}

	if (buffer->wpos >= buffer->rpos) {
		return buffer->len;
	}
	
	return buffer->size - (buffer->rpos - buffer->base);
}

static zend_always_inline size_t async_ring_buffer_write_len(async_ring_buffer *buffer)
{
	if (buffer->len == buffer->size) {
		return 0;
	}

	if (buffer->wpos >= buffer->rpos) {
		return buffer->size - (buffer->wpos - buffer->base);
	}
	
	return buffer->rpos - buffer->wpos;
}

static zend_always_inline size_t async_ring_buffer_read(async_ring_buffer *buffer, char *base, size_t len)
{
	size_t consumed;
	size_t count;
	
	consumed = 0;
	
	while (len > 0 && buffer->len > 0) {
		count = buffer->size - (buffer->rpos - buffer->base);
		
		if (count > buffer->len) {
			count = MIN(len, buffer->len);
		} else {
			count = MIN(len, count);
		}
		
		memcpy(base, buffer->rpos, count);
		
		buffer->rpos += count;
		buffer->len -= count;
		
		if ((buffer->rpos - buffer->base) == buffer->size) {
			buffer->rpos = buffer->base;
		}
		
		len -= count;
		base += count;
		consumed += count;
	}
	
	return consumed;
}

static zend_always_inline size_t async_ring_buffer_read_string(async_ring_buffer *buffer, zend_string **str, size_t len)
{
	zend_string *tmp;
	char *buf;
	
	len = MIN(buffer->len, len);
	
	if (len == 0) {
		*str = NULL;
	
		return 0;
	}
	
	tmp = zend_string_alloc(len, 0);
	
	buf = ZSTR_VAL(tmp);
	buf[len] = '\0';
	
	async_ring_buffer_read(buffer, buf, len);
		
	*str = tmp;

	return len;
}

static zend_always_inline void async_ring_buffer_write_move(async_ring_buffer *buffer, size_t offset)
{
	ZEND_ASSERT(offset >= 0);
	ZEND_ASSERT(offset <= buffer->size);

	buffer->wpos = buffer->base + ((buffer->wpos - buffer->base) + offset) % buffer->size;
	buffer->len += offset;
}

static zend_always_inline void async_ring_buffer_consume(async_ring_buffer *buffer, size_t len)
{
	ZEND_ASSERT(len > 0);
	ZEND_ASSERT(len <= buffer->len);
	
	buffer->rpos = buffer->base + ((buffer->rpos - buffer->base) + len) % buffer->size;
	buffer->len -= len;
}

#endif
