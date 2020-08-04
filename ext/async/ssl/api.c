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
#include "async/async_ssl.h"
#include "kernel/backend.h"

ASYNC_API zend_class_entry *async_tls_client_encryption_ce;
ASYNC_API zend_class_entry *async_tls_server_encryption_ce;
ASYNC_API zend_class_entry *async_tls_info_ce;

static zend_object_handlers async_tls_client_encryption_handlers;
static zend_object_handlers async_tls_server_encryption_handlers;
static zend_object_handlers async_tls_info_handlers;

static zend_string *str_protocol;
static zend_string *str_cipher_name;
static zend_string *str_cipher_bits;
static zend_string *str_alpn_protocol;


static zend_always_inline async_tls_info *async_tls_info_obj(zend_object *object)
{
	return (async_tls_info *)((char *) object - XtOffsetOf(async_tls_info, std));
}

static zend_always_inline uint32_t async_tls_info_prop_offset(zend_string *name)
{
	return zend_get_property_info(async_tls_info_ce, name, 1)->offset;
}

static zend_always_inline zend_string *create_alpn_proto_list(zval *protos, uint32_t count)
{
	zend_string *tmp;
	
	char buffer[4096];
	char *pos;
	uint32_t len;
	
	uint32_t size;
	uint32_t i;
	
	pos = buffer;
	size = 0;
	
	for (i = 0; i < count; i++) {
		tmp = Z_STR_P(protos + i);
		len = (unsigned char) ZSTR_LEN(tmp);
		
		if (len == 0) {
			continue;
		}
		
		size += len + 1;
		
		if (size > sizeof(buffer)) {
			zend_throw_error(NULL, "ALPN protocol list size must not exceed %lu bytes", sizeof(buffer));
			return NULL;
		}
		
		*pos = (unsigned char) len;
		memcpy(++pos, ZSTR_VAL(tmp), len);
		
		pos += len;
	}
	
	if (size == 0) {
		return NULL;
	}
	
	return zend_string_init(buffer, size, 0);
}

static zend_always_inline void dispose_cert(async_tls_cert *cert)
{
	if (cert->host != NULL) {
		zend_string_release(cert->host);
		cert->host = NULL;
	}
	
	if (cert->file != NULL) {
		zend_string_release(cert->file);
		cert->file = NULL;
	}
	
	if (cert->key != NULL) {
		zend_string_release(cert->key);
		cert->key = NULL;
	}
	
	if (cert->passphrase != NULL) {
		zend_string_release(cert->passphrase);
		cert->passphrase = NULL;
	}
	
#ifdef HAVE_ASYNC_SSL
	if (cert->ctx != NULL) {
		SSL_CTX_free(cert->ctx);
		cert->ctx = NULL;
	}
#endif
}


static zend_object *async_tls_client_encryption_object_create(zend_class_entry *ce)
{
	async_tls_client_encryption *encryption;

	encryption = ecalloc(1, sizeof(async_tls_client_encryption));

	zend_object_std_init(&encryption->std, ce);
	encryption->std.handlers = &async_tls_client_encryption_handlers;

	encryption->settings.verify_depth = ASYNC_SSL_DEFAULT_VERIFY_DEPTH;

	return &encryption->std;
}

async_tls_client_encryption *async_clone_client_encryption(async_tls_client_encryption *encryption)
{
	async_tls_client_encryption *result;

	result = (async_tls_client_encryption *) async_tls_client_encryption_object_create(async_tls_client_encryption_ce);
	result->settings.allow_self_signed = encryption->settings.allow_self_signed;
	result->settings.verify_depth = encryption->settings.verify_depth;

	if (encryption->settings.peer_name != NULL) {
		result->settings.peer_name = zend_string_copy(encryption->settings.peer_name);
	}
	
	if (encryption->alpn != NULL) {
		result->alpn = zend_string_copy(encryption->alpn);
	}
	
	if (encryption->capath != NULL) {
		result->capath = zend_string_copy(encryption->capath);
	}
	
	if (encryption->cafile != NULL) {
		result->cafile = zend_string_copy(encryption->cafile);
	}

	return result;
}

static void async_tls_client_encryption_object_destroy(zend_object *object)
{
	async_tls_client_encryption *encryption;

	encryption = (async_tls_client_encryption *) object;

	if (encryption->settings.peer_name != NULL) {
		zend_string_release(encryption->settings.peer_name);
	}
	
	if (encryption->alpn != NULL) {
		zend_string_release(encryption->alpn);
	}
	
	if (encryption->capath != NULL) {
		zend_string_release(encryption->capath);
	}
	
	if (encryption->cafile != NULL) {
		zend_string_release(encryption->cafile);
	}

	zend_object_std_dtor(&encryption->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_tls_client_encryption_with_allow_self_signed, 0, 1, Phalcon\\Async\\Network\\TlsClientEncryption, 0)
	ZEND_ARG_TYPE_INFO(0, allow, _IS_BOOL, 0)
ZEND_END_ARG_INFO();

PHP_METHOD(TlsClientEncryption, withAllowSelfSigned)
{
	async_tls_client_encryption *encryption;

	zend_bool allow;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_BOOL(allow)
	ZEND_PARSE_PARAMETERS_END();

	encryption = async_clone_client_encryption((async_tls_client_encryption *) Z_OBJ_P(getThis()));
	encryption->settings.allow_self_signed = allow;

	RETURN_OBJ(&encryption->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_tls_client_encryption_with_verify_depth, 0, 1, Phalcon\\Async\\Network\\TlsClientEncryption, 0)
	ZEND_ARG_TYPE_INFO(0, depth, IS_LONG, 0)
ZEND_END_ARG_INFO();

PHP_METHOD(TlsClientEncryption, withVerifyDepth)
{
	async_tls_client_encryption *encryption;

	zend_long depth;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_LONG(depth)
	ZEND_PARSE_PARAMETERS_END();

	ASYNC_CHECK_ERROR(depth < 1, "Verify depth must not be less than 1");

	encryption = async_clone_client_encryption((async_tls_client_encryption *) Z_OBJ_P(getThis()));
	encryption->settings.verify_depth = (int) depth;

	RETURN_OBJ(&encryption->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_tls_client_encryption_with_peer_name, 0, 1, Phalcon\\Async\\Network\\TlsClientEncryption, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO();

PHP_METHOD(TlsClientEncryption, withPeerName)
{
	async_tls_client_encryption *encryption;

	zend_string *name;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STR(name)
	ZEND_PARSE_PARAMETERS_END();

	encryption = async_clone_client_encryption((async_tls_client_encryption *) Z_OBJ_P(getThis()));
	
	if (encryption->settings.peer_name != NULL) {
		zend_string_release(encryption->settings.peer_name);
	}
	
	encryption->settings.peer_name = zend_string_copy(name);

	RETURN_OBJ(&encryption->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_tls_client_encryption_with_alpn_protocols, 0, 1, Phalcon\\Async\\Network\\TlsClientEncryption, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, protocols, IS_STRING, 0)
ZEND_END_ARG_INFO();

PHP_METHOD(TlsClientEncryption, withAlpnProtocols)
{
	async_tls_client_encryption *encryption;

	zval *protos;
	
	uint32_t count;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, -1)
		Z_PARAM_OPTIONAL
		Z_PARAM_VARIADIC('+', protos, count)
	ZEND_PARSE_PARAMETERS_END();

	encryption = async_clone_client_encryption((async_tls_client_encryption *) Z_OBJ_P(getThis()));
	
	if (encryption->alpn != NULL) {
		zend_string_release(encryption->alpn);
	}
	
	encryption->alpn = create_alpn_proto_list(protos, count);
	
	RETURN_OBJ(&encryption->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_tls_client_encryption_with_capath, 0, 1, Phalcon\\Async\\Network\\TlsClientEncryption, 0)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
ZEND_END_ARG_INFO();

PHP_METHOD(TlsClientEncryption, withCertificateAuthorityPath)
{
	async_tls_client_encryption *encryption;
	
	zend_string *capath;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STR(capath)
	ZEND_PARSE_PARAMETERS_END();
	
	encryption = async_clone_client_encryption((async_tls_client_encryption *) Z_OBJ_P(getThis()));
	
	if (encryption->capath != NULL) {
		zend_string_release(encryption->capath);
	}
	
	encryption->capath = zend_string_copy(capath);
	
	RETURN_OBJ(&encryption->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_tls_client_encryption_with_cafile, 0, 1, Phalcon\\Async\\Network\\TlsClientEncryption, 0)
	ZEND_ARG_TYPE_INFO(0, file, IS_STRING, 0)
ZEND_END_ARG_INFO();

PHP_METHOD(TlsClientEncryption, withCertificateAuthorityFile)
{
	async_tls_client_encryption *encryption;
		
	zend_string *cafile;
	char path[MAXPATHLEN];
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STR(cafile)
	ZEND_PARSE_PARAMETERS_END();
	
	ASYNC_CHECK_ERROR(!VCWD_REALPATH(ZSTR_VAL(cafile), path), "Failed to locate CA file: %s", ZSTR_VAL(cafile));
	
	encryption = async_clone_client_encryption((async_tls_client_encryption *) Z_OBJ_P(getThis()));
	
	if (encryption->cafile != NULL) {
		zend_string_release(encryption->cafile);
	}
	
	encryption->cafile = zend_string_init(path, strlen(path), 0);
	
	RETURN_OBJ(&encryption->std);
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_WAKEUP(TlsClientEncryption, async_tls_client_encryption_ce)
//LCOV_EXCL_STOP

static const zend_function_entry async_tls_client_encryption_functions[] = {
	PHP_ME(TlsClientEncryption, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(TlsClientEncryption, withAllowSelfSigned, arginfo_tls_client_encryption_with_allow_self_signed, ZEND_ACC_PUBLIC)
	PHP_ME(TlsClientEncryption, withVerifyDepth, arginfo_tls_client_encryption_with_verify_depth, ZEND_ACC_PUBLIC)
	PHP_ME(TlsClientEncryption, withPeerName, arginfo_tls_client_encryption_with_peer_name, ZEND_ACC_PUBLIC)
	PHP_ME(TlsClientEncryption, withAlpnProtocols, arginfo_tls_client_encryption_with_alpn_protocols, ZEND_ACC_PUBLIC)
	PHP_ME(TlsClientEncryption, withCertificateAuthorityPath, arginfo_tls_client_encryption_with_capath, ZEND_ACC_PUBLIC)
	PHP_ME(TlsClientEncryption, withCertificateAuthorityFile, arginfo_tls_client_encryption_with_cafile, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


static zend_object *async_tls_server_encryption_object_create(zend_class_entry *ce)
{
	async_tls_server_encryption *encryption;

	encryption = ecalloc(1, sizeof(async_tls_server_encryption));

	zend_object_std_init(&encryption->std, ce);
	encryption->std.handlers = &async_tls_server_encryption_handlers;

	return &encryption->std;
}

async_tls_server_encryption *async_ssl_create_server_encryption()
{
	return (async_tls_server_encryption *) async_tls_server_encryption_object_create(async_tls_server_encryption_ce);
}

static async_tls_server_encryption *clone_server_encryption(async_tls_server_encryption *encryption)
{
	async_tls_server_encryption *result;

	result = (async_tls_server_encryption *) async_tls_server_encryption_object_create(async_tls_server_encryption_ce);
	
	if (encryption->cert.host != NULL) {
		result->cert.host = zend_string_copy(encryption->cert.host);
	}
	
	if (encryption->cert.file != NULL) {
		result->cert.file = zend_string_copy(encryption->cert.file);
	}

	if (encryption->cert.key != NULL) {
		result->cert.key = zend_string_copy(encryption->cert.key);
	}

	if (encryption->cert.passphrase != NULL) {
		result->cert.passphrase = zend_string_copy(encryption->cert.passphrase);
	}
	
	if (encryption->alpn != NULL) {
		result->alpn = zend_string_copy(encryption->alpn);
	}
	
	if (encryption->capath != NULL) {
		result->capath = zend_string_copy(encryption->capath);
	}
	
	if (encryption->cafile != NULL) {
		result->cafile = zend_string_copy(encryption->cafile);
	}

	return result;
}

static void async_tls_server_encryption_object_destroy(zend_object *object)
{
	async_tls_server_encryption *encryption;

	encryption = (async_tls_server_encryption *) object;

	dispose_cert(&encryption->cert);

	if (encryption->alpn != NULL) {
		zend_string_release(encryption->alpn);
	}
	
	if (encryption->capath != NULL) {
		zend_string_release(encryption->capath);
	}
	
	if (encryption->cafile != NULL) {
		zend_string_release(encryption->cafile);
	}

	zend_object_std_dtor(&encryption->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_tls_server_encryption_with_default_certificate, 0, 1, Phalcon\\Async\\Network\\TlsServerEncryption, 0)
	ZEND_ARG_TYPE_INFO(0, cert, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, passphrase, IS_STRING, 1)
ZEND_END_ARG_INFO();

PHP_METHOD(TlsServerEncryption, withDefaultCertificate)
{
	async_tls_server_encryption *encryption;

	zend_string *file;

	zval *key;
	zval *passphrase;
	
	char path[MAXPATHLEN];

	key = NULL;
	passphrase = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 3)
		Z_PARAM_STR(file)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(key)		
		Z_PARAM_ZVAL(passphrase)
	ZEND_PARSE_PARAMETERS_END();

	encryption = clone_server_encryption((async_tls_server_encryption *) Z_OBJ_P(getThis()));
	
	ASYNC_CHECK_ERROR(!VCWD_REALPATH(ZSTR_VAL(file), path), "Failed to locate certificate: %s", ZSTR_VAL(file));

	encryption->cert.file = zend_string_init(path, strlen(path), 0);
	
	if (key == NULL || Z_TYPE_P(key) == IS_NULL) {
		encryption->cert.key = zend_string_copy(encryption->cert.file);
	} else {
		ASYNC_CHECK_ERROR(!VCWD_REALPATH(ZSTR_VAL(Z_STR_P(key)), path), "Failed to locate private key: %s", ZSTR_VAL(Z_STR_P(key)));
		
		encryption->cert.key = zend_string_init(path, strlen(path), 0);
	}

	if (passphrase != NULL && Z_TYPE_P(passphrase) != IS_NULL) {
		encryption->cert.passphrase = zend_string_copy(Z_STR_P(passphrase));
	}

	RETURN_OBJ(&encryption->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_tls_server_encryption_with_certificate, 0, 2, Phalcon\\Async\\Network\\TlsServerEncryption, 0)
	ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, cert, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, passphrase, IS_STRING, 1)
ZEND_END_ARG_INFO();

PHP_METHOD(TlsServerEncryption, withCertificate)
{
	async_tls_server_encryption *encryption;
	async_tls_cert *cert;
	async_tls_cert *current;

	zend_string *host;
	zend_string *file;

	zval *key;
	zval *passphrase;
	
	char path[MAXPATHLEN];
	
#ifdef HAVE_ASYNC_SSL
	char *cafile;
	char *capath;
#endif
	
	key = NULL;
	passphrase = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 4)
		Z_PARAM_STR(host)
		Z_PARAM_STR(file)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(key)
		Z_PARAM_ZVAL(passphrase)
	ZEND_PARSE_PARAMETERS_END();

	ASYNC_CHECK_ERROR(!VCWD_REALPATH(ZSTR_VAL(file), path), "Failed to locate certificate: %s", ZSTR_VAL(file));

	encryption = clone_server_encryption((async_tls_server_encryption *) Z_OBJ_P(getThis()));

	cert = ecalloc(1, sizeof(async_tls_cert));

	cert->host = zend_string_copy(host);
	cert->file = zend_string_init(path, strlen(path), 0);
	
	if (key != NULL && Z_TYPE_P(key) != IS_NULL) {
		if (!VCWD_REALPATH(ZSTR_VAL(Z_STR_P(key)), path)) {
			dispose_cert(cert);
			efree(cert);
			
			zend_throw_error(NULL, "Failed to locate private key: %s", ZSTR_VAL(Z_STR_P(key)));
			return;
		}
	
		cert->key = zend_string_copy(Z_STR_P(key));
	} else {
		cert->key = zend_string_copy(cert->file);
	}

	if (passphrase != NULL && Z_TYPE_P(passphrase) != IS_NULL) {
		cert->passphrase = zend_string_copy(Z_STR_P(passphrase));
	}

#ifdef HAVE_ASYNC_SSL
	cafile = (encryption->cafile == NULL) ? NULL : ZSTR_VAL(encryption->cafile);
	capath = (encryption->capath == NULL) ? NULL : ZSTR_VAL(encryption->capath);

	cert->ctx = async_ssl_create_context(SSL_OP_SINGLE_DH_USE, cafile, capath);

	SSL_CTX_set_default_passwd_cb(cert->ctx, async_ssl_cert_passphrase_cb);
	SSL_CTX_set_default_passwd_cb_userdata(cert->ctx, cert);

	SSL_CTX_use_certificate_chain_file(cert->ctx, ZSTR_VAL(cert->file));	
	SSL_CTX_use_PrivateKey_file(cert->ctx, ZSTR_VAL(cert->key), SSL_FILETYPE_PEM);
#endif

	current = encryption->certs.first;

	while (current != NULL) {
		if (zend_string_equals(current->host, cert->host)) {
			ASYNC_LIST_REMOVE(&encryption->certs, current);
			break;
		}

		current = current->next;
	}

	ASYNC_LIST_APPEND(&encryption->certs, cert);

	RETURN_OBJ(&encryption->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_tls_server_encryption_with_alpn_protocols, 0, 1, Phalcon\\Async\\Network\\TlsServerEncryption, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, protocols, IS_STRING, 0)
ZEND_END_ARG_INFO();

PHP_METHOD(TlsServerEncryption, withAlpnProtocols)
{
	async_tls_server_encryption *encryption;

	zval *protos;
	
	uint32_t count;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, -1)
		Z_PARAM_OPTIONAL
		Z_PARAM_VARIADIC('+', protos, count)
	ZEND_PARSE_PARAMETERS_END();

	encryption = clone_server_encryption((async_tls_server_encryption *) Z_OBJ_P(getThis()));
	encryption->alpn = create_alpn_proto_list(protos, count);
	
	RETURN_OBJ(&encryption->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_tls_server_encryption_with_capath, 0, 1, Phalcon\\Async\\Network\\TlsServerEncryption, 0)
	ZEND_ARG_TYPE_INFO(0, file, IS_STRING, 0)
ZEND_END_ARG_INFO();

PHP_METHOD(TlsServerEncryption, withCertificateAuthorityPath)
{
	async_tls_server_encryption *encryption;
		
	zend_string *capath;
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STR(capath)
	ZEND_PARSE_PARAMETERS_END();
	
	encryption = clone_server_encryption((async_tls_server_encryption *) Z_OBJ_P(getThis()));
	
	if (encryption->capath != NULL) {
		zend_string_release(encryption->capath);
	}
	
	encryption->capath = zend_string_copy(capath);
	
	RETURN_OBJ(&encryption->std);
}

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_tls_server_encryption_with_cafile, 0, 1, Phalcon\\Async\\Network\\TlsServerEncryption, 0)
	ZEND_ARG_TYPE_INFO(0, file, IS_STRING, 0)
ZEND_END_ARG_INFO();

PHP_METHOD(TlsServerEncryption, withCertificateAuthorityFile)
{
	async_tls_server_encryption *encryption;
		
	zend_string *cafile;
	char path[MAXPATHLEN];
	
	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_STR(cafile)
	ZEND_PARSE_PARAMETERS_END();
	
	ASYNC_CHECK_ERROR(!VCWD_REALPATH(ZSTR_VAL(cafile), path), "Failed to locate CA file: %s", ZSTR_VAL(cafile));
	
	encryption = clone_server_encryption((async_tls_server_encryption *) Z_OBJ_P(getThis()));
	
	if (encryption->cafile != NULL) {
		zend_string_release(encryption->cafile);
	}
	
	encryption->cafile = zend_string_init(path, strlen(path), 0);
	
	RETURN_OBJ(&encryption->std);
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_WAKEUP(TlsServerEncryption, async_tls_server_encryption_ce)
//LCOV_EXCL_STOP

static const zend_function_entry async_tls_server_encryption_functions[] = {
	PHP_ME(TlsServerEncryption, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_ME(TlsServerEncryption, withDefaultCertificate, arginfo_tls_server_encryption_with_default_certificate, ZEND_ACC_PUBLIC)
	PHP_ME(TlsServerEncryption, withCertificate, arginfo_tls_server_encryption_with_certificate, ZEND_ACC_PUBLIC)
	PHP_ME(TlsServerEncryption, withAlpnProtocols, arginfo_tls_server_encryption_with_alpn_protocols, ZEND_ACC_PUBLIC)
	PHP_ME(TlsServerEncryption, withCertificateAuthorityPath, arginfo_tls_server_encryption_with_capath, ZEND_ACC_PUBLIC)
	PHP_ME(TlsServerEncryption, withCertificateAuthorityFile, arginfo_tls_server_encryption_with_cafile, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

#ifdef HAVE_ASYNC_SSL

async_tls_info *async_tls_info_object_create(SSL *ssl)
{
	async_tls_info *info;
	
	const SSL_CIPHER *cipher;

	info = ecalloc(1, sizeof(async_tls_info) + zend_object_properties_size(async_tls_info_ce));

	zend_object_std_init(&info->std, async_tls_info_ce);
	info->std.handlers = &async_tls_info_handlers;
	
	object_properties_init(&info->std, async_tls_info_ce);

	cipher = SSL_get_current_cipher(ssl);
	
	ZVAL_STRING(OBJ_PROP(&info->std, async_tls_info_prop_offset(str_protocol)), SSL_get_version(ssl));
	ZVAL_STRING(OBJ_PROP(&info->std, async_tls_info_prop_offset(str_cipher_name)), SSL_CIPHER_get_name(cipher));
	ZVAL_LONG(OBJ_PROP(&info->std, async_tls_info_prop_offset(str_cipher_bits)), SSL_CIPHER_get_bits(cipher, NULL));
	
#ifdef ASYNC_TLS_ALPN
	const unsigned char *protos;
	unsigned int plen;
	
	SSL_get0_alpn_selected(ssl, &protos, &plen);
			
	if (plen > 0) {
		ZVAL_STRINGL(OBJ_PROP(&info->std, async_tls_info_prop_offset(str_alpn_protocol)), (char *) protos, plen);
	}
#endif

	return info;
}

#endif

static void async_tls_info_object_destroy(zend_object *object)
{
	async_tls_info *info;
	
	info = async_tls_info_obj(object);
	
	zend_object_std_dtor(&info->std);
}

//LCOV_EXCL_START
ASYNC_METHOD_NO_CTOR(TlsInfo, async_tls_info_ce)
ASYNC_METHOD_NO_WAKEUP(TlsInfo, async_tls_info_ce)
//LCOV_EXCL_STOP

static const zend_function_entry async_tls_info_functions[] = {
	PHP_ME(TlsInfo, __construct, arginfo_no_ctor, ZEND_ACC_PRIVATE)
	PHP_ME(TlsInfo, __wakeup, arginfo_no_wakeup, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

void async_ssl_ce_register()
{
	zend_class_entry ce;
	
#if PHP_VERSION_ID >= 70400
	zval tmp;
#endif

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Network", "TlsClientEncryption", async_tls_client_encryption_functions);
	async_tls_client_encryption_ce = zend_register_internal_class(&ce);
	async_tls_client_encryption_ce->ce_flags |= ZEND_ACC_FINAL;
	async_tls_client_encryption_ce->create_object = async_tls_client_encryption_object_create;

	memcpy(&async_tls_client_encryption_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_tls_client_encryption_handlers.free_obj = async_tls_client_encryption_object_destroy;
	async_tls_client_encryption_handlers.clone_obj = NULL;

	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Network", "TlsServerEncryption", async_tls_server_encryption_functions);
	async_tls_server_encryption_ce = zend_register_internal_class(&ce);
	async_tls_server_encryption_ce->ce_flags |= ZEND_ACC_FINAL;
	async_tls_server_encryption_ce->create_object = async_tls_server_encryption_object_create;

	memcpy(&async_tls_server_encryption_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_tls_server_encryption_handlers.free_obj = async_tls_server_encryption_object_destroy;
	async_tls_server_encryption_handlers.clone_obj = NULL;
	
	INIT_NS_CLASS_ENTRY(ce, "Phalcon\\Async\\Network", "TlsInfo", async_tls_info_functions);
	async_tls_info_ce = zend_register_internal_class(&ce);
	async_tls_info_ce->ce_flags |= ZEND_ACC_FINAL;

	memcpy(&async_tls_info_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	async_tls_info_handlers.offset = XtOffsetOf(async_tls_info, std);
	async_tls_info_handlers.free_obj = async_tls_info_object_destroy;
	async_tls_info_handlers.clone_obj = NULL;
	async_tls_info_handlers.write_property = async_prop_write_handler_readonly;
	
	str_protocol = zend_new_interned_string(zend_string_init(ZEND_STRL("protocol"), 1));
	str_cipher_name = zend_new_interned_string(zend_string_init(ZEND_STRL("cipher_name"), 1));
	str_cipher_bits = zend_new_interned_string(zend_string_init(ZEND_STRL("cipher_bits"), 1));
	str_alpn_protocol = zend_new_interned_string(zend_string_init(ZEND_STRL("alpn_protocol"), 1));

#if PHP_VERSION_ID >= 80000
	ZVAL_STRING(&tmp, "");
	zend_declare_typed_property(async_tls_info_ce, str_protocol, &tmp, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CODE(IS_STRING, 0, 0));
	zend_declare_typed_property(async_tls_info_ce, str_cipher_name, &tmp, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CODE(IS_STRING, 0, 0));
	zval_ptr_dtor(&tmp);

	ZVAL_LONG(&tmp, 0);
	zend_declare_typed_property(async_tls_info_ce, str_cipher_bits, &tmp, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CODE(IS_LONG, 0, 0));

	ZVAL_NULL(&tmp);
	zend_declare_typed_property(async_tls_info_ce, str_alpn_protocol, &tmp, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_CODE(IS_STRING, 1, 0));
#elif PHP_VERSION_ID < 70400
	zend_declare_property_null(async_tls_info_ce, ZEND_STRL("protocol"), ZEND_ACC_PUBLIC);
	zend_declare_property_null(async_tls_info_ce, ZEND_STRL("cipher_name"), ZEND_ACC_PUBLIC);
	zend_declare_property_null(async_tls_info_ce, ZEND_STRL("cipher_bits"), ZEND_ACC_PUBLIC);
	zend_declare_property_null(async_tls_info_ce, ZEND_STRL("alpn_protocol"), ZEND_ACC_PUBLIC);
#else
	ZVAL_STRING(&tmp, "");
	zend_declare_typed_property(async_tls_info_ce, str_protocol, &tmp, ZEND_ACC_PUBLIC, NULL, IS_STRING);
	zend_declare_typed_property(async_tls_info_ce, str_cipher_name, &tmp, ZEND_ACC_PUBLIC, NULL, IS_STRING);
	zval_ptr_dtor(&tmp);

	ZVAL_LONG(&tmp, 0);
	zend_declare_typed_property(async_tls_info_ce, str_cipher_bits, &tmp, ZEND_ACC_PUBLIC, NULL, IS_LONG);

	ZVAL_NULL(&tmp);
	zend_declare_typed_property(async_tls_info_ce, str_alpn_protocol, &tmp, ZEND_ACC_PUBLIC, NULL, ZEND_TYPE_ENCODE(IS_STRING, 1));
#endif

#ifdef HAVE_ASYNC_SSL
	async_ssl_bio_init();
	async_ssl_engine_init();
#endif
}

void async_ssl_ce_unregister()
{
	zend_string_release(str_protocol);
	zend_string_release(str_cipher_name);
	zend_string_release(str_cipher_bits);
	zend_string_release(str_alpn_protocol);
}
