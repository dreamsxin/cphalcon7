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
#include "async/async_xp.h"


#define ASYNC_XP_SOCKET_EOF(data) ((data)->astream ? ((data)->astream->flags & ASYNC_STREAM_EOF && (data)->astream->buffer.len == 0) : 0)

async_xp_socket_ssl *async_xp_socket_create_ssl(int options, char *cafile, char *capath)
{
	async_xp_socket_ssl *ssl;
	
	ssl = ecalloc(1, sizeof(async_xp_socket_ssl));
	ssl->refcount = 1;
	
#ifdef HAVE_ASYNC_SSL
	ssl->ctx = async_ssl_create_context(options, cafile, capath);
	ssl->encryption = async_ssl_create_server_encryption();
#endif
	
	return ssl;
}

void async_xp_socket_release_ssl(async_xp_socket_ssl *ssl)
{
	if (--ssl->refcount == 0) {
#ifdef HAVE_ASYNC_SSL
		if (ssl->ctx != NULL) {
			SSL_CTX_free(ssl->ctx);
		}
		
		if (ssl->encryption != NULL) {
			ASYNC_DELREF(&ssl->encryption->std);
		}
		
		efree(ssl);
#endif
	}
}

char *async_xp_parse_ip(const char *str, size_t str_len, int *portno, int get_err, zend_string **err)
{
    char *colon;
    char *host = NULL;

#ifdef HAVE_IPV6
    char *p;

    if (*(str) == '[' && str_len > 1) {
        /* IPV6 notation to specify raw address with port (i.e. [fe80::1]:80) */
        p = (char*) memchr(str + 1, ']', str_len - 2);
        
        if (!p || *(p + 1) != ':') {
            if (get_err) {
                *err = strpprintf(0, "Failed to parse IPv6 address \"%s\"", str);
            }
            return NULL;
        }
        
        *portno = atoi(p + 2);
        return estrndup(str + 1, p - str - 1);
    }
#endif

    if (str_len) {
        colon = (char*) memchr(str, ':', str_len - 1);
    } else {
        colon = NULL;
    }
    
    if (colon) {
        *portno = atoi(colon + 1);
        host = estrndup(str, colon - str);
    } else {
        if (get_err) {
            *err = strpprintf(0, "Failed to parse address \"%s\"", str);
        }
        return NULL;
    }

    return host;
}

#if PHP_VERSION_ID >= 70400
static ssize_t async_xp_socket_write(php_stream *stream, const char *buf, size_t count)
#else
static size_t async_xp_socket_write(php_stream *stream, const char *buf, size_t count)
#endif
{
	async_xp_socket_data *data;
	async_stream_write_req write;
	
	zval ref;

	data = (async_xp_socket_data *) stream->abstract;
	
	if (data->write != NULL) {
		return data->write(stream, data, buf, count);
	}
	
	ZVAL_RES(&ref, stream->res);
	
	memset(&write, 0, sizeof(async_stream_write_req));

	write.in.len = count;
	write.in.buffer = (char *) buf;
	write.in.ref = &ref;
	
	if (UNEXPECTED(FAILURE == async_stream_write(data->astream, &write))) {
		return 0;
	}
	
	return count;
}

#if PHP_VERSION_ID >= 70400
static ssize_t async_xp_socket_read(php_stream *stream, char *buf, size_t count)
#else
static size_t async_xp_socket_read(php_stream *stream, char *buf, size_t count)
#endif
{
	async_xp_socket_data *data;
	async_stream_read_req read;
	
	int code;
	
	data = (async_xp_socket_data *) stream->abstract;
	
	if (data->read != NULL) {
		return data->read(stream, data, buf, count);
	}
	
	data->flags &= ~ASYNC_XP_SOCKET_FLAG_TIMED_OUT;
	
	read.in.len = count;
	read.in.buffer = buf;
	read.in.handle = NULL;
	read.in.timeout = data->timeout;
	read.in.flags = 0;
	
	code = async_stream_read(data->astream, &read);
	
	if (ASYNC_XP_SOCKET_EOF(data)) {
		stream->eof = 1;
	}
	
	if (EXPECTED(code == SUCCESS)) {
		return read.out.len;
	}
	
	if (UNEXPECTED(EG(exception))) {
		return 0;
	}
	
#ifdef HAVE_ASYNC_SSL
	if (read.out.ssl_error) {
		return 0;
	}
#endif

	switch (read.out.error) {
	case UV_ETIMEDOUT:
		data->flags |= ASYNC_XP_SOCKET_FLAG_TIMED_OUT;
		break;
	case UV_EOF:
		stream->eof = 1;
		break;
	default:
		php_error_docref(NULL, E_WARNING, "Read operation failed: %s", uv_strerror(read.out.error));
	}
	
	return 0;
}

ASYNC_CALLBACK dispose_timer(uv_handle_t *handle)
{
	async_xp_socket_data *data;
	
	data = (async_xp_socket_data *) handle->data;
	
	ZEND_ASSERT(data != NULL);
	
	if (data->ssl != NULL) {
		async_xp_socket_release_ssl(data->ssl);
	}
	
	async_task_scheduler_unref(data->scheduler);
	
	efree(data);
}

ASYNC_CALLBACK close_cb(void *arg)
{
	async_xp_socket_data *data;
	
	data = (async_xp_socket_data *) arg;
	
	ZEND_ASSERT(data != NULL);
	
#ifdef HAVE_ASYNC_SSL
	if (data->astream->ssl.ssl != NULL) {
		async_ssl_dispose_engine(&data->astream->ssl, (data->ssl == NULL) ? 1 : 0);
	}
#endif
	
	async_stream_free(data->astream);
	
	if (data->peer != NULL) {
		zend_string_release(data->peer);
		data->peer = NULL;
	}
	
	if (uv_is_closing((uv_handle_t *) &data->timer)) {
		if (data->ssl != NULL) {
			async_xp_socket_release_ssl(data->ssl);
		}
		
		async_task_scheduler_unref(data->scheduler);
		
		efree(data);
	} else {
		ASYNC_UV_CLOSE(&data->timer, dispose_timer);
	}
}

ASYNC_CALLBACK close_dgram_cb(uv_handle_t *handle)
{
	async_xp_socket_data *data;
	
	data = (async_xp_socket_data *) handle->data;
	
	ZEND_ASSERT(data != NULL);
	
	if (data->peer != NULL) {
		zend_string_release(data->peer);
		data->peer = NULL;
	}
	
	if (uv_is_closing((uv_handle_t *) &data->timer)) {
		if (data->ssl != NULL) {
			async_xp_socket_release_ssl(data->ssl);
		}
		
		async_task_scheduler_unref(data->scheduler);
		
		efree(data);
	} else {
		ASYNC_UV_CLOSE(&data->timer, dispose_timer);
	}
}

static int async_xp_socket_close(php_stream *stream, int close_handle)
{
	async_xp_socket_data *data;

	data = (async_xp_socket_data *) stream->abstract;
	
	if (data->astream == NULL) {
		if (!(data->flags & ASYNC_XP_SOCKET_FLAG_INIT) || uv_is_closing(&data->handle)) {
			if (data->peer != NULL) {
				zend_string_release(data->peer);
				data->peer = NULL;
			}

			ASYNC_UV_CLOSE(&data->timer, dispose_timer);
		} else {
			ASYNC_UV_CLOSE(&data->handle, close_dgram_cb);
		}
	} else {
		async_stream_close_cb(data->astream, close_cb, data);
	}
	
	return 0;
}

static int async_xp_socket_flush(php_stream *stream)
{
    return 0;
}

static int async_xp_socket_cast(php_stream *stream, int castas, void **ret)
{
	async_xp_socket_data *data;
	php_socket_t fd;
	
	data = (async_xp_socket_data *) stream->abstract;
	
	switch (castas) {
	case PHP_STREAM_AS_FD_FOR_SELECT:
	case PHP_STREAM_AS_FD:
		if (0 != uv_fileno((const uv_handle_t *) &data->handle, (uv_os_fd_t *) &fd)) {
			return FAILURE;
		}
		
		if (ret) {
			*(php_socket_t *)ret = fd;
		}
	
		return SUCCESS;
	}
	
	return FAILURE;
}

static int async_xp_socket_stat(php_stream *stream, php_stream_statbuf *ssb)
{
	return FAILURE;
}

#ifdef HAVE_ASYNC_SSL

#ifdef ASYNC_TLS_ALPN

static zend_string *create_alpn_proto_list(zend_string *alpn)
{
	unsigned char buffer[4096];
	unsigned char *pos;
	char *c;
		
	unsigned int len;
	unsigned char j;
	unsigned int i;
	
	len = (unsigned int) ZSTR_LEN(alpn);
	pos = buffer;
	
	for (c = ZSTR_VAL(alpn), j = 0, i = 0; i < len; i++) {
		if (*c == ',') {
			if (j > 0) {
				*pos = j;
				memcpy(++pos, c - j, j);
			}
			
			pos += j;
			j = 0;
		} else {
			j++;
		}
		
		c++;
	}
	
	if (j > 0) {
		*pos = j;
		memcpy(++pos, c- j, j);
	}
	
	return zend_string_init((char *) buffer, len + 1, 0);
}

#endif

static int setup_server_encryption(php_stream *stream, async_xp_socket_data *data, php_stream_xport_param *xparam)
{
	zval *cert;
	zval *val;
	
	int options;
	char path[MAXPATHLEN];
	
	char *cafile;
	char *capath;
	
	// TODO: Support multiple certs (SNI).
	
	if (!ASYNC_XP_SOCKET_SSL_OPT(stream, "local_cert", cert)) {
		return SUCCESS;
	}
	
	options = 0;
	
	if (!ASYNC_XP_SOCKET_SSL_OPT(stream, "single_dh_use", val) || zend_is_true(val)) {
		options |= SSL_OP_SINGLE_DH_USE;
	}
	
	if (!ASYNC_XP_SOCKET_SSL_OPT(stream, "honor_cipher_order", val) || zend_is_true(val)) {
		options |= SSL_OP_CIPHER_SERVER_PREFERENCE;
	}
	
	cafile = NULL;
	capath = NULL;
	
	ASYNC_XP_SOCKET_SSL_OPT_STRING(stream, "cafile", cafile);
	ASYNC_XP_SOCKET_SSL_OPT_STRING(stream, "capath", capath);
	
	data->ssl = async_xp_socket_create_ssl(options, cafile, capath);
	
	if (!VCWD_REALPATH(ZSTR_VAL(Z_STR_P(cert)), path)) {
		php_error_docref(NULL, E_WARNING, "Unable to find local cert file `%s'", ZSTR_VAL(Z_STR_P(cert)));
		
		return FAILURE;
	}
	
	data->ssl->encryption->cert.file = zend_string_init(path, strlen(path), 0);
	
	if (ASYNC_XP_SOCKET_SSL_OPT(stream, "local_pk", val)) {
		if (VCWD_REALPATH(ZSTR_VAL(Z_STR_P(val)), path)) {
			data->ssl->encryption->cert.key = zend_string_init(path, strlen(path), 0);
		}
	} else {
		data->ssl->encryption->cert.key = zend_string_copy(data->ssl->encryption->cert.file);
	}
	
	if (ASYNC_XP_SOCKET_SSL_OPT(stream, "passphrase", val)) {
		data->ssl->encryption->cert.passphrase = zend_string_copy(Z_STR_P(val));
	}
	
	SSL_CTX_set_default_passwd_cb_userdata(data->ssl->ctx, &data->ssl->encryption->cert);
	
	if (1 != SSL_CTX_use_certificate_chain_file(data->ssl->ctx, ZSTR_VAL(data->ssl->encryption->cert.file))) {
		php_error_docref(NULL, E_WARNING, "Unable to set local cert chain file `%s'", ZSTR_VAL(data->ssl->encryption->cert.file));
		
		return FAILURE;
	}
	
	if (1 != SSL_CTX_use_PrivateKey_file(data->ssl->ctx, ZSTR_VAL(data->ssl->encryption->cert.key), SSL_FILETYPE_PEM)) {
		php_error_docref(NULL, E_WARNING, "Unable to set private key file `%s'", ZSTR_VAL(data->ssl->encryption->cert.key));
		
		return FAILURE;
	}
	
	if (!SSL_CTX_check_private_key(data->ssl->ctx)) {
		php_error_docref(NULL, E_WARNING, "Private key does not match certificate!");
	}
	
	if (!ASYNC_XP_SOCKET_SSL_OPT(stream, "SNI_enabled", val) || zend_is_true(val)) {
		async_ssl_setup_server_sni(data->ssl->ctx, data->ssl->encryption);
	}
	
#ifdef ASYNC_TLS_ALPN
	if (ASYNC_XP_SOCKET_SSL_OPT(stream, "alpn_protocols", val)) {
		data->ssl->encryption->alpn = create_alpn_proto_list(Z_STR_P(val));
		
		async_ssl_setup_server_alpn(data->ssl->ctx, data->ssl->encryption);
	}
#endif
	
	return SUCCESS;
}

#endif

static int async_xp_socket_xport_api(php_stream *stream, async_xp_socket_data *data, php_stream_xport_param *xparam STREAMS_DC)
{
	switch (xparam->op) {
	case STREAM_XPORT_OP_ACCEPT:
		xparam->outputs.returncode = (data->accept == NULL) ? FAILURE : data->accept(stream, data, xparam);

		if (xparam->outputs.returncode == SUCCESS) {
			ZEND_ASSERT(((async_xp_socket_data *) xparam->outputs.client->abstract)->astream != NULL);
		}
		break;
	case STREAM_XPORT_OP_BIND:
		xparam->outputs.returncode = (data->bind == NULL) ? FAILURE : data->bind(stream, data, xparam);
		break;
	case STREAM_XPORT_OP_CONNECT:
	case STREAM_XPORT_OP_CONNECT_ASYNC:
		xparam->outputs.returncode = (data->connect == NULL) ? FAILURE : data->connect(stream, data, xparam);

		if (xparam->outputs.returncode == SUCCESS && !(data->flags & ASYNC_XP_SOCKET_FLAG_DGRAM)) {
			ZEND_ASSERT(data->astream != NULL);
		}
		break;
	case STREAM_XPORT_OP_GET_NAME:
	case STREAM_XPORT_OP_GET_PEER_NAME:
		xparam->outputs.returncode = data->get_peer(
			data,
			(xparam->op == STREAM_XPORT_OP_GET_NAME) ? 0 : 1,
			xparam->want_textaddr ? &xparam->outputs.textaddr : NULL,
			xparam->want_addr ? &xparam->outputs.addr : NULL,
			xparam->want_addr ? &xparam->outputs.addrlen : NULL
		);
		break;
	case STREAM_XPORT_OP_LISTEN:
		xparam->outputs.returncode = (data->listen == NULL) ? FAILURE : data->listen(stream, data, xparam);
		
#ifdef HAVE_ASYNC_SSL
		if (xparam->outputs.returncode == SUCCESS) {
			xparam->outputs.returncode = setup_server_encryption(stream, data, xparam);
		}
#endif
		break;
	case STREAM_XPORT_OP_RECV:
		xparam->outputs.returncode = (data->receive == NULL) ? FAILURE : data->receive(stream, data, xparam);
		break;
	case STREAM_XPORT_OP_SEND:
		xparam->outputs.returncode = (data->send == NULL) ? FAILURE : data->send(stream, data, xparam);
		break;
	case STREAM_XPORT_OP_SHUTDOWN:
		xparam->outputs.returncode = (data->shutdown == NULL) ? SUCCESS : data->shutdown(stream, data, xparam->how);
		break;
	}
	
	return PHP_STREAM_OPTION_RETURN_OK;
}

#ifdef HAVE_ASYNC_SSL

static int cert_passphrase_cb(char *buf, int size, int rwflag, void *obj)
{
	zend_string *key;
	
	key = (zend_string *) obj;
	
	if (key == NULL) {
		return 0;
	}
	
	strncpy(buf, ZSTR_VAL(key), ZSTR_LEN(key));
	
	return (int) ZSTR_LEN(key);
}

static char *cipher_get_version(const SSL_CIPHER *c, char *buffer, size_t max_len)
{
	const char *version;
	
	version = SSL_CIPHER_get_version(c);

	strncpy(buffer, version, max_len);
	if (max_len <= strlen(version)) {
		buffer[max_len - 1] = 0;
	}

	return buffer;
}

#endif

#ifdef HAVE_ASYNC_SSL

static int setup_ssl(php_stream *stream, async_xp_socket_data *data, php_stream_xport_crypto_param *cparam)
{	
	zval *val;
	
	char *cafile;
	char *capath;
	
	int options;
		
	if (data->ssl != NULL) {
		data->astream->ssl.ctx = data->ssl->ctx;
		
		return SUCCESS;
	}
	
	options = SSL_OP_SINGLE_DH_USE;
	
	cafile = NULL;
	capath = NULL;
	
	ASYNC_XP_SOCKET_SSL_OPT_STRING(stream, "cafile", cafile);
	ASYNC_XP_SOCKET_SSL_OPT_STRING(stream, "capath", capath);
	
	data->astream->ssl.ctx = async_ssl_create_context(options, cafile, capath);
	
	SSL_CTX_set_default_passwd_cb(data->astream->ssl.ctx, cert_passphrase_cb);
	
	data->astream->ssl.settings.verify_depth = ASYNC_SSL_DEFAULT_VERIFY_DEPTH;
	
	if (ASYNC_XP_SOCKET_SSL_OPT(stream, "peer_name", val)) {
		data->astream->ssl.settings.peer_name = zend_string_copy(Z_STR_P(val));
	} else if (data->peer != NULL) {
		data->astream->ssl.settings.peer_name = zend_string_copy(data->peer);
	}
	
	if (ASYNC_XP_SOCKET_SSL_OPT(stream, "verify_depth", val)) {
		data->astream->ssl.settings.verify_depth = (int) Z_LVAL_P(val);
	}
	
	if (ASYNC_XP_SOCKET_SSL_OPT(stream, "allow_self_signed", val)) {
		if (zend_is_true(val)) {
			data->astream->ssl.settings.allow_self_signed = 1;
		}
	}
	
	if (ASYNC_XP_SOCKET_SSL_OPT(stream, "SNI_enabled", val) && !zend_is_true(val)) {
		data->astream->ssl.settings.disable_sni = 1;
	}
	
	// TODO: Implement client cert authentication...
	
#ifdef ASYNC_TLS_ALPN
	if (ASYNC_XP_SOCKET_SSL_OPT(stream, "alpn_protocols", val)) {
		async_ssl_setup_client_alpn(data->astream->ssl.ctx, create_alpn_proto_list(Z_STR_P(val)), 1);
	}
#endif
	
	return SUCCESS;
}

static int toggle_ssl(php_stream *stream, async_xp_socket_data *data, php_stream_xport_crypto_param *cparam)
{
	async_ssl_handshake_data handshake;
	
	int code;
	
	memset(&handshake, 0, sizeof(async_ssl_handshake_data));
		
	if (data->flags & ASYNC_XP_SOCKET_FLAG_ACCEPTED) {
		data->astream->ssl.settings.mode = ASYNC_SSL_MODE_SERVER;
	} else {
		data->astream->ssl.settings.mode = ASYNC_SSL_MODE_CLIENT;
		
		async_ssl_setup_verify_callback(data->astream->ssl.ctx, &data->astream->ssl.settings);
	}
	
	handshake.settings = &data->astream->ssl.settings;
	
	async_ssl_create_buffered_engine(&data->astream->ssl, data->astream->buffer.size);
	async_ssl_setup_encryption(data->astream->ssl.ssl, handshake.settings);
	
	code = async_stream_ssl_handshake(data->astream, &handshake);
	
	if (code != SUCCESS) {
		async_ssl_dispose_engine(&data->astream->ssl, (data->ssl == NULL) ? 1 : 0);
		
		if (handshake.ssl_error != SSL_ERROR_NONE) {
			php_error_docref(NULL, E_WARNING, "SSL error: %s\n", ERR_reason_error_string(handshake.ssl_error));
		}
		
		return FAILURE;
	}
	
	return 1;
}

#endif

static int async_xp_socket_set_option(php_stream *stream, int option, int value, void *ptrparam)
{
	async_xp_socket_data *data;

#ifdef HAVE_ASYNC_SSL
	zval crypto;
	
	const SSL_CIPHER *cipher;
	char cipher_version[32];
	
#ifdef ASYNC_TLS_ALPN
	const unsigned char *proto;
	unsigned int plen;
#endif

#endif
	
	data = (async_xp_socket_data *) stream->abstract;
	
	switch (option) {
	case PHP_STREAM_OPTION_XPORT_API:
		return async_xp_socket_xport_api(stream, data, (php_stream_xport_param *) ptrparam STREAMS_CC);
#ifdef HAVE_ASYNC_SSL
	case PHP_STREAM_OPTION_CRYPTO_API: {
		php_stream_xport_crypto_param *cparam = (php_stream_xport_crypto_param *) ptrparam;
		
		switch (cparam->op) {
			case STREAM_XPORT_CRYPTO_OP_SETUP:
				cparam->outputs.returncode = (data->flags & ASYNC_XP_SOCKET_FLAG_DGRAM) ? FAILURE : setup_ssl(stream, data, cparam);
				
				return PHP_STREAM_OPTION_RETURN_OK;
			case STREAM_XPORT_CRYPTO_OP_ENABLE:
				cparam->outputs.returncode = (data->flags & ASYNC_XP_SOCKET_FLAG_DGRAM) ? FAILURE : toggle_ssl(stream, data, cparam);
				
				return PHP_STREAM_OPTION_RETURN_OK;
		}
	}
#endif
	case PHP_STREAM_OPTION_META_DATA_API:
		add_assoc_bool((zval *) ptrparam, "timed_out", (data->flags & ASYNC_XP_SOCKET_FLAG_TIMED_OUT) ? 1 : 0);
		add_assoc_bool((zval *) ptrparam, "blocked", (data->flags & ASYNC_XP_SOCKET_FLAG_BLOCKING) ? 1 : 0);
		add_assoc_bool((zval *) ptrparam, "eof", ASYNC_XP_SOCKET_EOF(data));
		
#ifdef HAVE_ASYNC_SSL
		if (data->astream != NULL && data->astream->ssl.ssl != NULL) {
			array_init(&crypto);
			
			cipher = SSL_get_current_cipher(data->astream->ssl.ssl);
			
			add_assoc_string(&crypto, "protocol", SSL_get_version(data->astream->ssl.ssl));
			add_assoc_string(&crypto, "cipher_name", (char *) SSL_CIPHER_get_name(cipher));
			add_assoc_long(&crypto, "cipher_bits", SSL_CIPHER_get_bits(cipher, NULL));
			add_assoc_string(&crypto, "cipher_version", cipher_get_version(cipher, cipher_version, 32));
			
#ifdef ASYNC_TLS_ALPN
			SSL_get0_alpn_selected(data->astream->ssl.ssl, &proto, &plen);
			
			if (plen > 0) {
				add_assoc_stringl(&crypto, "alpn_protocol", (char *) proto, plen);
			}
#endif
			
			add_assoc_zval((zval *) ptrparam, "crypto", &crypto);
		}
#endif
		
		return PHP_STREAM_OPTION_RETURN_OK;
	case PHP_STREAM_OPTION_BLOCKING:
		if (value) {
			data->flags |= ASYNC_XP_SOCKET_FLAG_BLOCKING;
		} else {
			data->flags = (data->flags | ASYNC_XP_SOCKET_FLAG_BLOCKING) ^ ASYNC_XP_SOCKET_FLAG_BLOCKING;
		}

		return PHP_STREAM_OPTION_RETURN_OK;
	case PHP_STREAM_OPTION_CHECK_LIVENESS:		
		if (EXPECTED(data->astream != NULL && async_socket_is_alive(data->astream))) {
			return PHP_STREAM_OPTION_RETURN_OK;
		}
		
		return PHP_STREAM_OPTION_RETURN_ERR;
	case PHP_STREAM_OPTION_READ_TIMEOUT: {		
		struct timeval tv = *(struct timeval *) ptrparam;
		
		data->timeout = ((uint64_t) tv.tv_sec * 1000) + (uint64_t) (tv.tv_usec / 1000);
		
		if (data->timeout == 0 && tv.tv_usec > 0) {
			data->timeout = 1;
		}

		return PHP_STREAM_OPTION_RETURN_OK;
	}
	case PHP_STREAM_OPTION_READ_BUFFER:
		if (value == PHP_STREAM_BUFFER_NONE) {
			stream->readbuf = perealloc(stream->readbuf, 0, stream->is_persistent);
			stream->flags |= PHP_STREAM_FLAG_NO_BUFFER;
		} else {
			stream->readbuflen = MAX(*((size_t *) ptrparam), 0x8000);
			stream->readbuf = perealloc(stream->readbuf, stream->readbuflen, stream->is_persistent);
			stream->flags &= ~PHP_STREAM_FLAG_NO_BUFFER;
		}

		return PHP_STREAM_OPTION_RETURN_OK;
	}

	return PHP_STREAM_OPTION_RETURN_NOTIMPL;
}

void async_xp_socket_populate_ops(php_stream_ops *ops, const char *label)
{
	ops->write = async_xp_socket_write;
	ops->read = async_xp_socket_read;
	ops->close = async_xp_socket_close;
	ops->flush = async_xp_socket_flush;
	ops->label = label;
	ops->seek = NULL;
	ops->cast = async_xp_socket_cast;
	ops->stat = async_xp_socket_stat;
	ops->set_option = async_xp_socket_set_option;
}

php_stream *async_xp_socket_create(async_xp_socket_data *data, php_stream_ops *ops, const char *pid STREAMS_DC)
{
	async_task_scheduler *scheduler;
	php_stream *stream;
	
	stream = php_stream_alloc_rel(ops, data, pid, "r+");
	
	if (stream == NULL) {
		return NULL;
	}
	
	scheduler = async_task_scheduler_ref();
	
	data->scheduler = scheduler;
	data->stream = stream;
	data->flags |= ASYNC_XP_SOCKET_FLAG_BLOCKING;
	data->handle.data = data;
	data->timer.data = data;
	
	uv_timer_init(&data->scheduler->loop, &data->timer);
 	
	return stream;
}

php_stream_transport_factory async_xp_socket_register(const char *protocol, php_stream_transport_factory factory)
{
	php_stream_transport_factory prev;
	HashTable *hash;
	
	hash = php_stream_xport_get_hash();
	
	prev = (php_stream_transport_factory) zend_hash_str_find_ptr(hash, ZEND_STRL(protocol));
	php_stream_xport_register(protocol, factory);
	
	return prev;
}
