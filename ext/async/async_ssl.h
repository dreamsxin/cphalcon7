
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

#ifndef ASYNC_SSL_H
#define ASYNC_SSL_H

#include "async/core.h"
#include "async/async_buffer.h"

#define ASYNC_SSL_MODE_SERVER 0
#define ASYNC_SSL_MODE_CLIENT 1

#define ASYNC_SSL_DEFAULT_VERIFY_DEPTH 9

#ifndef OPENSSL_NO_TLSEXT
#define ASYNC_TLS_SNI 1
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
#define ASYNC_TLS_ALPN 1
#endif
#endif

#ifdef HAVE_ASYNC_SSL

#define BIO_TYPE_PHP (98 | BIO_TYPE_SOURCE_SINK)
#define ASYNC_SSL_BIO_OVERHEAD (2 * sizeof(size_t))

typedef struct {
	size_t size;
	size_t len;
	char buf[1];
} async_ssl_bio_php;

ASYNC_API BIO_METHOD *BIO_s_php();
ASYNC_API BIO *BIO_new_php(size_t size);

void async_ssl_bio_init();
void async_ssl_engine_init();

#if OPENSSL_VERSION_NUMBER < 0x10100000L

BIO_METHOD *BIO_meth_new(int type, const char *name);

#define BIO_meth_set_write(b, f) (b)->bwrite = (f)
#define BIO_meth_set_read(b, f) (b)->bread = (f)
#define BIO_meth_set_ctrl(b, f) (b)->ctrl = (f)
#define BIO_meth_set_create(b, f) (b)->create = (f)
#define BIO_meth_set_destroy(b, f) (b)->destroy = (f)

#define BIO_set_init(b, val) (b)->init = (val)
#define BIO_set_data(b, val) (b)->ptr = (val)
#define BIO_set_shutdown(b, val) (b)->shutdown = (val)
#define BIO_get_init(b) (b)->init
#define BIO_get_data(b) (b)->ptr
#define BIO_get_shutdown(b) (b)->shutdown

#endif

static zend_always_inline void async_ssl_bio_increment(BIO *b, int count)
{
	async_ssl_bio_php *impl;
	
	impl = (async_ssl_bio_php *) BIO_get_data(b);
	impl->len += count;
}

static zend_always_inline void async_ssl_bio_expose_buffer(BIO *b, char **buf, uv_buf_size_t *len)
{
	async_ssl_bio_php *impl;
	
	impl = (async_ssl_bio_php *) BIO_get_data(b);
	
	*buf = impl->buf + impl->len;
	*len = (uv_buf_size_t) (impl->size - impl->len);
}

#endif

typedef struct {
	/* SSL mode (ASYNC_SSL_MODE_SERVER or ASYNC_SSL_MODE_CLIENT). */
	zend_bool mode;

	/* If set self-signed server certificates are accepted. */
	zend_bool allow_self_signed;

	/* Can be used to opt out of SNI usage. */
	zend_bool disable_sni;

	/* Name of the peer to connect to. */
	zend_string *peer_name;

	/* Maximum verification cert chain length */
	int verify_depth;
} async_ssl_settings;

typedef struct {
	async_op base;
	int uv_error;
	int ssl_error;
} async_ssl_op;

typedef struct {
	async_ssl_settings *settings;
	zend_string *host;
	int uv_error;
	int ssl_error;
	zend_string *error;
} async_ssl_handshake_data;

typedef struct {
#ifdef HAVE_ASYNC_SSL
	/* SSL context.*/
	SSL_CTX *ctx;

	/* SSL encryption engine. */
	SSL *ssl;

	/* Holds bytes that need to be processed as SSL input data. */
	BIO *rbio;

	/* Holds processed encrypted bytes that need to be sent. */
	BIO *wbio;
	
	/* Number of available (decoded) input bytes. */
	size_t available;
	
	/* Number of consumed input bytes of an unfinished packet. */
	size_t pending;
	
	/* Current handshake operation. */
	async_ssl_op *handshake;

	/* SSL connection and encryption settings. */
	async_ssl_settings settings;
#endif
} async_ssl_engine;

typedef struct _async_tls_cert async_tls_cert;

struct _async_tls_cert {
	zend_string *host;
	zend_string *file;
	zend_string *key;
	zend_string *passphrase;
	async_tls_cert *next;
	async_tls_cert *prev;
#ifdef HAVE_ASYNC_SSL
	SSL_CTX *ctx;
#endif
};

typedef struct {
	async_tls_cert *first;
	async_tls_cert *last;
} async_tls_cert_list;

typedef struct {
	/* PHP object handle. */
	zend_object std;

	async_ssl_settings settings;
	
	zend_string *alpn;
	
	zend_string *cafile;
	zend_string *capath;
} async_tls_client_encryption;

typedef struct {
	/* PHP object handle. */
	zend_object std;

	async_tls_cert cert;
	async_tls_cert_list certs;
	
	zend_string *alpn;
	
	zend_string *cafile;
	zend_string *capath;
} async_tls_server_encryption;

typedef struct {
	/* PHP object handle. */
	zend_object std;
	
	const char *protocol;
	const char *cipher_name;
	int cipher_bits;
	
	zend_string *alpn;
} async_tls_info;

#ifdef HAVE_ASYNC_SSL

SSL_CTX *async_ssl_create_context();
int async_ssl_create_buffered_engine(async_ssl_engine *engine, size_t size);
void async_ssl_dispose_engine(async_ssl_engine *engine, zend_bool ctx);

async_tls_server_encryption *async_ssl_create_server_encryption();
async_tls_info *async_tls_info_object_create(SSL *ssl);

void async_ssl_setup_client_alpn(SSL_CTX *ctx, zend_string *alpn, zend_bool release);

void async_ssl_setup_server_sni(SSL_CTX *ctx, async_tls_server_encryption *encryption);
void async_ssl_setup_server_alpn(SSL_CTX *ctx, async_tls_server_encryption *encryption);

void async_ssl_setup_verify_callback(SSL_CTX *ctx, async_ssl_settings *settings);
int async_ssl_setup_encryption(SSL *ssl, async_ssl_settings *settings);

int async_ssl_cert_passphrase_cb(char *buf, int size, int rwflag, void *obj);

#endif

async_tls_client_encryption *async_clone_client_encryption(async_tls_client_encryption *encryption);

#endif
