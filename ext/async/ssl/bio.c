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
#include "async/async_ssl.h"

#ifdef HAVE_ASYNC_SSL

static BIO_METHOD *php_method;

#if OPENSSL_VERSION_NUMBER < 0x10100000L

BIO_METHOD *BIO_meth_new(int type, const char *name)
{
	BIO_METHOD *method = calloc(1, sizeof(BIO_METHOD));

	if (method != NULL) {
		method->type = type;
		method->name = name;
	}
	
	return method;
}

#endif

static int bio_php_read(BIO *b, char *buf, int size)
{
	async_ssl_bio_php *impl;
	size_t len;
	
	impl = (async_ssl_bio_php *) BIO_get_data(b);
	
	BIO_clear_retry_flags(b);
	
	if (impl->len == 0) {
		BIO_set_retry_read(b);
		
		return -1;
	}
	
	len = MIN(size, impl->len);
	
	memcpy(buf, impl->buf, len);
	impl->len -= len;
	
	if (impl->len) {
		memmove(impl->buf, impl->buf + len, impl->len);
	}
	
	return (int) len;
}

static long bio_php_ctrl(BIO *b, int cmd, long num, void *ptr)
{
	async_ssl_bio_php *impl;
	
	impl = (async_ssl_bio_php *) BIO_get_data(b);
	
	switch (cmd) {
	case BIO_CTRL_EOF:
		return (impl->len == 0);
	case BIO_CTRL_GET_CLOSE:
		return BIO_get_shutdown(b);
	case BIO_CTRL_SET_CLOSE:
		BIO_set_shutdown(b, (int) num);
		break;
	case BIO_CTRL_WPENDING:
		return 0;
	case BIO_CTRL_PENDING:
		return (long) impl->len;
	case BIO_CTRL_PUSH:
	case BIO_CTRL_POP:
		return 0;
	}
	
	return 1;
}

static int bio_php_free(BIO *b)
{
	async_ssl_bio_php *impl;
	
	if (b == NULL) {
		return 0;
	}
	
	impl = (async_ssl_bio_php *) BIO_get_data(b);
	
	efree(impl);
	
	return 1;
}

ASYNC_API BIO_METHOD *BIO_s_php()
{
	return php_method;
}

ASYNC_API BIO *BIO_new_php(size_t size)
{
	BIO *b;
	async_ssl_bio_php *impl;
	
	size -= ASYNC_SSL_BIO_OVERHEAD;
	
	impl = emalloc(size);
	impl->size = size;
	impl->len = 0;
	
	b = BIO_new(BIO_s_php());
	BIO_set_data(b, impl);
	BIO_set_init(b, 1);
	
	return b;
}

#endif

void async_ssl_bio_init()
{
#ifdef HAVE_ASYNC_SSL
	php_method = BIO_meth_new(BIO_TYPE_PHP, "PHP memory buffer");
	
	BIO_meth_set_read(php_method, bio_php_read);
	BIO_meth_set_ctrl(php_method, bio_php_ctrl);
	BIO_meth_set_destroy(php_method, bio_php_free);
#endif
}
