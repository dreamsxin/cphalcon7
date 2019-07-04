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

static int async_index;

#ifdef PHP_WIN32
#include "win32/winutil.h"
#include "win32/time.h"
#include <Wincrypt.h>
/* These are from Wincrypt.h, they conflict with OpenSSL */
#undef X509_NAME
#undef X509_CERT_PAIR
#undef X509_EXTENSIONS
#endif

#define ASYNC_SSL_RETURN_VERIFY_ERROR(ctx) do { \
	X509_STORE_CTX_set_error(ctx, X509_V_ERR_APPLICATION_VERIFICATION); \
	return 0; \
} while (0);

int async_ssl_cert_passphrase_cb(char *buf, int size, int rwflag, void *obj)
{
	async_tls_cert *cert;

	cert = (async_tls_cert *) obj;

	if (UNEXPECTED(cert == NULL || cert->passphrase == NULL)) {
		return 0;
	}
	
	strcpy(buf, ZSTR_VAL(cert->passphrase));

	return (int) ZSTR_LEN(cert->passphrase);
}

static int ssl_servername_cb(SSL *ssl, int *ad, void *arg)
{
	async_tls_cert_list *q;
	async_tls_cert *cert;
	zend_string *key;

	const char *name;

	if (UNEXPECTED(ssl == NULL)) {
		return SSL_TLSEXT_ERR_NOACK;
	}

	name = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);

	if (UNEXPECTED(name == NULL || name[0] == '\0')) {
		return SSL_TLSEXT_ERR_NOACK;
	}

	q = (async_tls_cert_list *) arg;
	key = zend_string_init(name, strlen(name), 0);
	
	cert = q->first;

	while (cert != NULL) {
		if (zend_string_equals(key, cert->host)) {
			SSL_set_SSL_CTX(ssl, cert->ctx);
			zend_string_release(key);
			
			return SSL_TLSEXT_ERR_OK;
		}

		cert = cert->next;
	}

	if (0 != strcmp(ZSTR_VAL(key), "127.0.0.1") && 0 != strcmp(ZSTR_VAL(key), "::")) {
		zend_string_release(key);
		
		return SSL_TLSEXT_ERR_OK;
	}

	zend_string_release(key);

	ASYNC_STR(key, "localhost");
	
	while (cert != NULL) {
		if (zend_string_equals(key, cert->host)) {
			SSL_set_SSL_CTX(ssl, cert->ctx);
			zend_string_release(key);
			
			return SSL_TLSEXT_ERR_OK;
		}

		cert = cert->next;
	}
	
	zend_string_release(key);

	return SSL_TLSEXT_ERR_OK;
}

static zend_bool async_ssl_match_hostname(const char *subjectname, const char *certname)
{
	char *wildcard = NULL;
	ptrdiff_t prefix_len;
	size_t suffix_len, subject_len;

	if (strcasecmp(subjectname, certname) == 0) {
		return 1;
	}

	if (!(wildcard = strchr(certname, '*')) || memchr(certname, '.', wildcard - certname)) {
		return 0;
	}

	prefix_len = wildcard - certname;
	if (prefix_len && strncasecmp(subjectname, certname, prefix_len) != 0) {
		return 0;
	}

	suffix_len = strlen(wildcard + 1);
	subject_len = strlen(subjectname);

	if (suffix_len <= subject_len) {
		return strcasecmp(wildcard + 1, subjectname + subject_len - suffix_len) == 0 && memchr(subjectname + prefix_len, '.', subject_len - suffix_len - prefix_len) == NULL;
	}

	return strcasecmp(wildcard + 2, subjectname + subject_len - suffix_len);
}

static int async_ssl_check_san_names(zend_string *peer_name, X509 *cert, X509_STORE_CTX *ctx)
{
	GENERAL_NAMES *names;
	GENERAL_NAME *entry;

	unsigned char *cn;
	int count;
	int i;

	names = X509_get_ext_d2i(cert, NID_subject_alt_name, 0, 0);

	if (names == NULL) {
		return 0;
	}

	for (count = sk_GENERAL_NAME_num(names), i = 0; i < count; i++) {
		entry = sk_GENERAL_NAME_value(names, i);

		if (UNEXPECTED(entry == NULL || GEN_DNS != entry->type)) {
			continue;
		}

		ASN1_STRING_to_UTF8(&cn, entry->d.dNSName);

		if ((size_t) ASN1_STRING_length(entry->d.dNSName) != strlen((const char *) cn)) {
			OPENSSL_free(cn);
			break;
		}
		
		if (async_ssl_match_hostname(ZSTR_VAL(peer_name), (const char *) cn)) {
			OPENSSL_free(cn);
			sk_GENERAL_NAME_pop_free(names, GENERAL_NAME_free);
			return 1;
		}

		OPENSSL_free(cn);
	}

	sk_GENERAL_NAME_pop_free(names, GENERAL_NAME_free);

	return 0;
}

static int ssl_verify_callback(int preverify, X509_STORE_CTX *ctx)
{
	async_ssl_settings *settings;
	SSL *ssl;

	X509 *cert;
	X509_NAME_ENTRY *entry;
	ASN1_STRING *str;

	unsigned char *cn;

	int depth;
	int err;
	int i;

	cert = X509_STORE_CTX_get_current_cert(ctx);
	depth = X509_STORE_CTX_get_error_depth(ctx);
	err = X509_STORE_CTX_get_error(ctx);

	ssl = X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
	settings = (async_ssl_settings *) SSL_get_ex_data(ssl, async_index);

	if (depth == 0 && err == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT) {
		if (settings != NULL && settings->allow_self_signed) {
			err = 0;
			preverify = 1;
			X509_STORE_CTX_set_error(ctx, X509_V_OK);
		}
	}

	if (depth > settings->verify_depth) {
		X509_STORE_CTX_set_error(ctx, X509_V_ERR_CERT_CHAIN_TOO_LONG);
		return 0;
	}

	if (!cert || err || settings == NULL) {
		return preverify;
	}

	if (depth == 0) {
		if (async_ssl_check_san_names(settings->peer_name, cert, ctx)) {
			return preverify;
		}

		if (UNEXPECTED((i = X509_NAME_get_index_by_NID(X509_get_subject_name((X509 *) cert), NID_commonName, -1)) < 0)) {
			ASYNC_SSL_RETURN_VERIFY_ERROR(ctx);
		}

		if (UNEXPECTED(NULL == (entry = X509_NAME_get_entry(X509_get_subject_name((X509 *) cert), i)))) {
			ASYNC_SSL_RETURN_VERIFY_ERROR(ctx);
		}

		if (UNEXPECTED(NULL == (str = X509_NAME_ENTRY_get_data(entry)))) {
			ASYNC_SSL_RETURN_VERIFY_ERROR(ctx);
		}

		ASN1_STRING_to_UTF8(&cn, str);

		if ((size_t) ASN1_STRING_length(str) != strlen((const char *) cn)) {
			OPENSSL_free(cn);
			ASYNC_SSL_RETURN_VERIFY_ERROR(ctx);
		}

		if (!async_ssl_match_hostname(ZSTR_VAL(settings->peer_name), (const char *) cn)) {
			OPENSSL_free(cn);
			ASYNC_SSL_RETURN_VERIFY_ERROR(ctx);
		}

		OPENSSL_free(cn);
	}

	return preverify;
}

SSL_CTX *async_ssl_create_context(int options, char *cafile, char *capath)
{
	SSL_CTX *ctx;

	int mask;

	mask = SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3;	
	mask |= SSL_OP_NO_COMPRESSION | SSL_OP_NO_TICKET;
	mask &= ~SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS;

	ctx = SSL_CTX_new(SSLv23_method());

	SSL_CTX_set_options(ctx, options | mask);
	SSL_CTX_set_cipher_list(ctx, "HIGH:!SSLv2:!aNULL:!eNULL:!EXPORT:!DES:!MD5:!RC4:!ADH");

	if (EXPECTED(cafile == NULL)) {
		cafile = zend_ini_string("openssl.cafile", sizeof("openssl.cafile")-1, 0);
		cafile = (cafile && strlen(cafile)) ? cafile : NULL;
	}
	
	if (EXPECTED(capath == NULL)) {
		capath = zend_ini_string("openssl.capath", sizeof("openssl.capath")-1, 0);
		capath = (capath && strlen(capath)) ? capath : NULL;
	}

	if (cafile || capath) {
		SSL_CTX_load_verify_locations(ctx, cafile, capath);
	} else {
		SSL_CTX_set_default_verify_paths(ctx);
	}
	
	SSL_CTX_set_default_passwd_cb(ctx, async_ssl_cert_passphrase_cb);

	return ctx;
}

static zend_always_inline int configure_engine(SSL_CTX *ctx, SSL *ssl, BIO *rbio, BIO *wbio)
{
	SSL_set_bio(ssl, rbio, wbio);
	SSL_set_mode(ssl, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

#ifdef SSL_MODE_RELEASE_BUFFERS
	SSL_set_mode(ssl, SSL_get_mode(ssl) | SSL_MODE_RELEASE_BUFFERS);
#endif

	SSL_set_read_ahead(ssl, 1);

	return SUCCESS;
}

int async_ssl_create_buffered_engine(async_ssl_engine *engine, size_t size)
{
	engine->ssl = SSL_new(engine->ctx);
	engine->rbio = BIO_new_php(size);
	engine->wbio = BIO_new(BIO_s_mem());
	
	BIO_set_mem_eof_return(engine->wbio, -1);
	
	return configure_engine(engine->ctx, engine->ssl, engine->rbio, engine->wbio);
}

void async_ssl_dispose_engine(async_ssl_engine *engine, zend_bool ctx)
{
	if (engine->ssl != NULL) {
		SSL_free(engine->ssl);
		
		engine->ssl = NULL;
		engine->rbio = NULL;
		engine->wbio = NULL;
	}

	if (engine->ctx != NULL && ctx) {
		SSL_CTX_free(engine->ctx);
		engine->ctx = NULL;
	}
	
	if (engine->settings.peer_name != NULL) {
		zend_string_release(engine->settings.peer_name);
		engine->settings.peer_name = NULL;
	}
}

#ifdef PHP_WIN32

#define RETURN_CERT_VERIFY_FAILURE(code) do { \
	X509_STORE_CTX_set_error(x509_store_ctx, code); \
	return 0; \
} while (0)

#define PHP_X509_NAME_ENTRY_TO_UTF8(ne, i, out) ASN1_STRING_to_UTF8(&out, X509_NAME_ENTRY_get_data(X509_NAME_get_entry(ne, i)))

#define php_win_err_free(err) do { \
	if (err && err[0]) \
		free(err); \
} while (0)

static int ssl_win32_verify_callback(X509_STORE_CTX *x509_store_ctx, void *arg)
{
	PCCERT_CONTEXT cert_ctx = NULL;
	PCCERT_CHAIN_CONTEXT cert_chain_ctx = NULL;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	X509 *cert = x509_store_ctx->cert;
#else
	X509 *cert = X509_STORE_CTX_get0_cert(x509_store_ctx);
#endif

	async_ssl_settings *settings;
	SSL* ssl;
	zend_bool is_self_signed = 0;

	ssl = X509_STORE_CTX_get_ex_data(x509_store_ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
	settings = (async_ssl_settings *) SSL_get_ex_data(ssl, async_index);

	{ /* First convert the x509 struct back to a DER encoded buffer and let Windows decode it into a form it can work with */
		unsigned char *der_buf = NULL;
		int der_len;

		der_len = i2d_X509(cert, &der_buf);
		if (der_len < 0) {
			unsigned long err_code, e;
			char err_buf[512];

			while ((e = ERR_get_error()) != 0) {
				err_code = e;
			}

			php_error_docref(NULL, E_WARNING, "Error encoding X509 certificate: %d: %s", err_code, ERR_error_string(err_code, err_buf));
			RETURN_CERT_VERIFY_FAILURE(SSL_R_CERTIFICATE_VERIFY_FAILED);
		}

		cert_ctx = CertCreateCertificateContext(X509_ASN_ENCODING, der_buf, der_len);
		OPENSSL_free(der_buf);

		if (cert_ctx == NULL) {
			char *err = php_win_err();
			php_error_docref(NULL, E_WARNING, "Error creating certificate context: %s", err);
			php_win_err_free(err);
			RETURN_CERT_VERIFY_FAILURE(SSL_R_CERTIFICATE_VERIFY_FAILED);
		}
	}

	{ /* Next fetch the relevant cert chain from the store */
		CERT_ENHKEY_USAGE enhkey_usage = {0};
		CERT_USAGE_MATCH cert_usage = {0};
		CERT_CHAIN_PARA chain_params = {sizeof(CERT_CHAIN_PARA)};
		LPSTR usages[] = {szOID_PKIX_KP_SERVER_AUTH, szOID_SERVER_GATED_CRYPTO, szOID_SGC_NETSCAPE};
		DWORD chain_flags = 0;
		unsigned long allowed_depth;
		unsigned int i;

		enhkey_usage.cUsageIdentifier = 3;
		enhkey_usage.rgpszUsageIdentifier = usages;
		cert_usage.dwType = USAGE_MATCH_TYPE_OR;
		cert_usage.Usage = enhkey_usage;
		chain_params.RequestedUsage = cert_usage;
		chain_flags = CERT_CHAIN_CACHE_END_CERT | CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT;

		if (!CertGetCertificateChain(NULL, cert_ctx, NULL, NULL, &chain_params, chain_flags, NULL, &cert_chain_ctx)) {
			char *err = php_win_err();
			php_error_docref(NULL, E_WARNING, "Error getting certificate chain: %s", err);
			php_win_err_free(err);
			CertFreeCertificateContext(cert_ctx);
			RETURN_CERT_VERIFY_FAILURE(SSL_R_CERTIFICATE_VERIFY_FAILED);
		}

		/* check if the cert is self-signed */
		if (cert_chain_ctx->cChain > 0 && cert_chain_ctx->rgpChain[0]->cElement > 0
			&& (cert_chain_ctx->rgpChain[0]->rgpElement[0]->TrustStatus.dwInfoStatus & CERT_TRUST_IS_SELF_SIGNED) != 0) {
			is_self_signed = 1;
		}

		/* check the depth */
		allowed_depth = settings->verify_depth;

		for (i = 0; i < cert_chain_ctx->cChain; i++) {
			if (cert_chain_ctx->rgpChain[i]->cElement > allowed_depth) {
				CertFreeCertificateChain(cert_chain_ctx);
				CertFreeCertificateContext(cert_ctx);
				RETURN_CERT_VERIFY_FAILURE(X509_V_ERR_CERT_CHAIN_TOO_LONG);
			}
		}
	}

	{ /* Then verify it against a policy */
		SSL_EXTRA_CERT_CHAIN_POLICY_PARA ssl_policy_params = {sizeof(SSL_EXTRA_CERT_CHAIN_POLICY_PARA)};
		CERT_CHAIN_POLICY_PARA chain_policy_params = {sizeof(CERT_CHAIN_POLICY_PARA)};
		CERT_CHAIN_POLICY_STATUS chain_policy_status = {sizeof(CERT_CHAIN_POLICY_STATUS)};
		LPWSTR server_name = NULL;
		BOOL verify_result;

		{ /* This looks ridiculous and it is - but we validate the name ourselves using the peer_name
		     ctx option, so just use the CN from the cert here */

			X509_NAME *cert_name;
			unsigned char *cert_name_utf8;
			int index, cert_name_utf8_len;
			DWORD num_wchars;

			cert_name = X509_get_subject_name(cert);
			index = X509_NAME_get_index_by_NID(cert_name, NID_commonName, -1);
			if (index < 0) {
				php_error_docref(NULL, E_WARNING, "Unable to locate certificate CN");
				CertFreeCertificateChain(cert_chain_ctx);
				CertFreeCertificateContext(cert_ctx);
				RETURN_CERT_VERIFY_FAILURE(SSL_R_CERTIFICATE_VERIFY_FAILED);
			}

			cert_name_utf8_len = PHP_X509_NAME_ENTRY_TO_UTF8(cert_name, index, cert_name_utf8);

			num_wchars = MultiByteToWideChar(CP_UTF8, 0, (char*)cert_name_utf8, -1, NULL, 0);
			if (num_wchars == 0) {
				php_error_docref(NULL, E_WARNING, "Unable to convert %s to wide character string", cert_name_utf8);
				OPENSSL_free(cert_name_utf8);
				CertFreeCertificateChain(cert_chain_ctx);
				CertFreeCertificateContext(cert_ctx);
				RETURN_CERT_VERIFY_FAILURE(SSL_R_CERTIFICATE_VERIFY_FAILED);
			}

			server_name = emalloc((num_wchars * sizeof(WCHAR)) + sizeof(WCHAR));

			num_wchars = MultiByteToWideChar(CP_UTF8, 0, (char*)cert_name_utf8, -1, server_name, num_wchars);
			if (num_wchars == 0) {
				php_error_docref(NULL, E_WARNING, "Unable to convert %s to wide character string", cert_name_utf8);
				efree(server_name);
				OPENSSL_free(cert_name_utf8);
				CertFreeCertificateChain(cert_chain_ctx);
				CertFreeCertificateContext(cert_ctx);
				RETURN_CERT_VERIFY_FAILURE(SSL_R_CERTIFICATE_VERIFY_FAILED);
			}

			OPENSSL_free(cert_name_utf8);
		}

		ssl_policy_params.dwAuthType = (settings->mode == ASYNC_SSL_MODE_CLIENT) ? AUTHTYPE_CLIENT : AUTHTYPE_SERVER;
		ssl_policy_params.pwszServerName = server_name;
		chain_policy_params.pvExtraPolicyPara = &ssl_policy_params;

		verify_result = CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_SSL, cert_chain_ctx, &chain_policy_params, &chain_policy_status);

		efree(server_name);
		CertFreeCertificateChain(cert_chain_ctx);
		CertFreeCertificateContext(cert_ctx);

		if (!verify_result) {
			char *err = php_win_err();
			php_error_docref(NULL, E_WARNING, "Error verifying certificate chain policy: %s", err);
			php_win_err_free(err);
			RETURN_CERT_VERIFY_FAILURE(SSL_R_CERTIFICATE_VERIFY_FAILED);
		}

		if (chain_policy_status.dwError != 0) {
			/* The chain does not match the policy */
			if (is_self_signed && chain_policy_status.dwError == CERT_E_UNTRUSTEDROOT && settings->allow_self_signed) {
				/* allow self-signed certs */
				X509_STORE_CTX_set_error(x509_store_ctx, X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT);
			} else {
				RETURN_CERT_VERIFY_FAILURE(SSL_R_CERTIFICATE_VERIFY_FAILED);
			}
		}
	}

	return 1;
}

#endif

void async_ssl_setup_verify_callback(SSL_CTX *ctx, async_ssl_settings *settings)
{
	SSL_CTX_set_default_passwd_cb_userdata(ctx, NULL);
	SSL_CTX_set_verify_depth(ctx, settings->verify_depth);

#ifdef PHP_WIN32
		SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
		SSL_CTX_set_cert_verify_callback(ctx, ssl_win32_verify_callback, settings);
#else
		SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, ssl_verify_callback);
#endif
}

int async_ssl_setup_encryption(SSL *ssl, async_ssl_settings *settings)
{
	return SSL_set_ex_data(ssl, async_index, settings);
}

void async_ssl_setup_server_sni(SSL_CTX *ctx, async_tls_server_encryption *encryption)
{
	SSL_CTX_set_tlsext_servername_callback(ctx, ssl_servername_cb);
	SSL_CTX_set_tlsext_servername_arg(ctx, &encryption->certs);
}

#ifdef ASYNC_TLS_ALPN

static int alpn_select_cb(SSL *ssl, const unsigned char **out, unsigned char *outlen, const unsigned char *in, unsigned int inlen, void *arg)
{
	async_tls_server_encryption *tls;
	
	unsigned char *protos;
	unsigned int plen;
	
	tls = (async_tls_server_encryption *) arg;
	
	ZEND_ASSERT(tls != NULL);
	ZEND_ASSERT(tls->alpn != NULL);
	
	protos = (unsigned char *) ZSTR_VAL(tls->alpn);
	plen = (unsigned int) ZSTR_LEN(tls->alpn);

	if (OPENSSL_NPN_NEGOTIATED != SSL_select_next_proto((unsigned char **) out, outlen, protos, plen, in, inlen)) {
		return SSL_TLSEXT_ERR_NOACK;
	}

	return SSL_TLSEXT_ERR_OK;
}

#endif

void async_ssl_setup_client_alpn(SSL_CTX *ctx, zend_string *alpn, zend_bool release)
{
#ifdef ASYNC_TLS_ALPN
	const unsigned char *protos;
	unsigned int plen;

	if (alpn != NULL) {
		protos = (unsigned char *) ZSTR_VAL(alpn);
		plen = (unsigned int) ZSTR_LEN(alpn);
		
		SSL_CTX_set_alpn_protos(ctx, protos, plen);
	}
#endif

	if (release && alpn != NULL) {
		zend_string_release(alpn);
	}
}

void async_ssl_setup_server_alpn(SSL_CTX *ctx, async_tls_server_encryption *encryption)
{
#ifdef ASYNC_TLS_ALPN
	if (encryption->alpn != NULL) {
		SSL_CTX_set_alpn_select_cb(ctx, alpn_select_cb, encryption);
	}
#endif
}

#endif

void async_ssl_engine_init()
{
#ifdef HAVE_ASYNC_SSL
	async_index = SSL_get_ex_new_index(0, "async", NULL, NULL, NULL);
#endif
}
