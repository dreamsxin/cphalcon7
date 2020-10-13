
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
  |          ZiHang Gao <ocdoco@gmail.com>                                 |
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#include "jwt.h"
#include "exception.h"


#include <zend_smart_str.h>
#include <zend_exceptions.h>

#include <ext/spl/spl_exceptions.h>
#include <ext/standard/base64.h>
#include <ext/json/php_json.h>
#include <ext/standard/info.h>
#include <ext/standard/php_string.h>

/* OpenSSL includes */
#include <openssl/conf.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/buffer.h>
#include <openssl/pem.h>

#include "kernel/main.h"

/* Routines to support crypto in JWT using OpenSSL. */

/* Functions to make libjwt backward compatible with OpenSSL version < 1.1.0
 * See https://wiki.openssl.org/index.php/1.1_API_Changes
 */
#if OPENSSL_VERSION_NUMBER < 0x10100000L

static void ECDSA_SIG_get0(const ECDSA_SIG *sig, const BIGNUM **pr, const BIGNUM **ps)
{
    if (pr != NULL)
        *pr = sig->r;
    if (ps != NULL)
        *ps = sig->s;
}

static int ECDSA_SIG_set0(ECDSA_SIG *sig, BIGNUM *r, BIGNUM *s)
{
    if (r == NULL || s == NULL)
        return 0;

    BN_clear_free(sig->r);
    BN_clear_free(sig->s);
    sig->r = r;
    sig->s = s;

    return 1;
}

#endif

int jwt_sign_sha_hmac(jwt_t *jwt, char **out, unsigned int *len) {

    const EVP_MD *alg;

    switch (jwt->alg) {
    /* HMAC */
    case JWT_ALG_HS256:
        alg = EVP_sha256();
        break;
    case JWT_ALG_HS384:
        alg = EVP_sha384();
        break;
    case JWT_ALG_HS512:
        alg = EVP_sha512();
        break;
    default:
        return EINVAL;
    }

    *out = emalloc(EVP_MAX_MD_SIZE);
    if (*out == NULL) {
        return ENOMEM;
    }
		
    HMAC(alg, ZSTR_VAL(jwt->key), ZSTR_LEN(jwt->key),
        (const unsigned char *)ZSTR_VAL(jwt->str), ZSTR_LEN(jwt->str), (unsigned char *)*out,
        len);

    return 0;
}

int jwt_verify_sha_hmac(jwt_t *jwt, const char *sig)
{
    unsigned char res[EVP_MAX_MD_SIZE];
    BIO *bmem = NULL, *b64 = NULL;
    unsigned int res_len;
    const EVP_MD *alg;
    char *buf;
    int len, ret = EINVAL;

    switch (jwt->alg) {
    case JWT_ALG_HS256:
        alg = EVP_sha256();
        break;
    case JWT_ALG_HS384:
        alg = EVP_sha384();
        break;
    case JWT_ALG_HS512:
        alg = EVP_sha512();
        break;
    default:
        return EINVAL;
    }

    b64 = BIO_new(BIO_f_base64());
    if (b64 == NULL)
        return ENOMEM;

    bmem = BIO_new(BIO_s_mem());
    if (bmem == NULL) {
        BIO_free(b64);
        return ENOMEM;
    }

    BIO_push(b64, bmem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    HMAC(alg, ZSTR_VAL(jwt->key), ZSTR_LEN(jwt->key),
        (const unsigned char *)ZSTR_VAL(jwt->str), ZSTR_LEN(jwt->str), res, &res_len);

    BIO_write(b64, res, res_len);

    (void)BIO_flush(b64);

    len = BIO_pending(bmem);
    if (len < 0)
        goto jwt_verify_hmac_done;

    buf = alloca(len + 1);
    if (!buf) {
        ret = ENOMEM;
        goto jwt_verify_hmac_done;
    }

    len = BIO_read(bmem, buf, len);
    buf[len] = '\0';

    jwt_b64_url_encode_ex(buf);

    /* And now... */
    ret = strcmp(buf, sig) ? EINVAL : 0;

jwt_verify_hmac_done:
    BIO_free_all(b64);

    return ret;
}

#define SIGN_ERROR(__err) { ret = __err; goto jwt_sign_sha_pem_done; }

int jwt_sign_sha_pem(jwt_t *jwt, char **out, unsigned int *len)
{
    EVP_MD_CTX *mdctx = NULL;
    ECDSA_SIG *ec_sig = NULL;
    const BIGNUM *ec_sig_r = NULL;
    const BIGNUM *ec_sig_s = NULL;
    BIO *bufkey = NULL;
    const EVP_MD *alg;
    int type;
    EVP_PKEY *pkey = NULL;
    int pkey_type;
    unsigned char *sig;
    int ret = 0;
    size_t slen;

    switch (jwt->alg) {
    /* RSA */
    case JWT_ALG_RS256:
        alg = EVP_sha256();
        type = EVP_PKEY_RSA;
        break;
    case JWT_ALG_RS384:
        alg = EVP_sha384();
        type = EVP_PKEY_RSA;
        break;
    case JWT_ALG_RS512:
        alg = EVP_sha512();
        type = EVP_PKEY_RSA;
        break;

    /* ECC */
    case JWT_ALG_ES256:
        alg = EVP_sha256();
        type = EVP_PKEY_EC;
        break;
    case JWT_ALG_ES384:
        alg = EVP_sha384();
        type = EVP_PKEY_EC;
        break;
    case JWT_ALG_ES512:
        alg = EVP_sha512();
        type = EVP_PKEY_EC;
        break;

    default:
        return EINVAL;
    }

    bufkey = BIO_new_mem_buf(ZSTR_VAL(jwt->key), ZSTR_LEN(jwt->key));
    if (bufkey == NULL)
        SIGN_ERROR(ENOMEM);

    /* This uses OpenSSL's default passphrase callback if needed. The
     * library caller can override this in many ways, all of which are
     * outside of the scope of LibJWT and this is documented in jwt.h. */
    pkey = PEM_read_bio_PrivateKey(bufkey, NULL, NULL, NULL);
    if (pkey == NULL)
        SIGN_ERROR(EINVAL);

    pkey_type = EVP_PKEY_id(pkey);
    if (pkey_type != type)
        SIGN_ERROR(EINVAL);

    mdctx = EVP_MD_CTX_create();
    if (mdctx == NULL)
        SIGN_ERROR(ENOMEM);

    /* Initialize the DigestSign operation using alg */
    if (EVP_DigestSignInit(mdctx, NULL, alg, NULL, pkey) != 1)
        SIGN_ERROR(EINVAL);

    /* Call update with the message */
    if (EVP_DigestSignUpdate(mdctx, ZSTR_VAL(jwt->str), ZSTR_LEN(jwt->str)) != 1)
        SIGN_ERROR(EINVAL);

    /* First, call EVP_DigestSignFinal with a NULL sig parameter to get length
     * of sig. Length is returned in slen */
    if (EVP_DigestSignFinal(mdctx, NULL, &slen) != 1)
        SIGN_ERROR(EINVAL);

    /* Allocate memory for signature based on returned size */
    sig = alloca(slen);
    if (sig == NULL)
        SIGN_ERROR(ENOMEM);

    /* Get the signature */
    if (EVP_DigestSignFinal(mdctx, sig, &slen) != 1)
        SIGN_ERROR(EINVAL);

    if (pkey_type != EVP_PKEY_EC) {
        *out = emalloc(slen);
        if (*out == NULL)
            SIGN_ERROR(ENOMEM);
        memcpy(*out, sig, slen);
        *len = slen;
    } else {
        unsigned int degree, bn_len, r_len, s_len, buf_len;
        unsigned char *raw_buf;
        EC_KEY *ec_key;

        /* For EC we need to convert to a raw format of R/S. */

        /* Get the actual ec_key */
        ec_key = EVP_PKEY_get1_EC_KEY(pkey);
        if (ec_key == NULL)
            SIGN_ERROR(ENOMEM);

        degree = EC_GROUP_get_degree(EC_KEY_get0_group(ec_key));

        EC_KEY_free(ec_key);

        /* Get the sig from the DER encoded version. */
        ec_sig = d2i_ECDSA_SIG(NULL, (const unsigned char **)&sig, slen);
        if (ec_sig == NULL)
            SIGN_ERROR(ENOMEM);

        ECDSA_SIG_get0(ec_sig, &ec_sig_r, &ec_sig_s);
        r_len = BN_num_bytes(ec_sig_r);
        s_len = BN_num_bytes(ec_sig_s);
        bn_len = (degree + 7) / 8;
        if ((r_len > bn_len) || (s_len > bn_len))
            SIGN_ERROR(EINVAL);

        buf_len = 2 * bn_len;
        raw_buf = alloca(buf_len);
        if (raw_buf == NULL)
            SIGN_ERROR(ENOMEM);

        /* Pad the bignums with leading zeroes. */
        memset(raw_buf, 0, buf_len);
        BN_bn2bin(ec_sig_r, raw_buf + bn_len - r_len);
        BN_bn2bin(ec_sig_s, raw_buf + buf_len - s_len);

        *out = emalloc(buf_len);
        if (*out == NULL)
            SIGN_ERROR(ENOMEM);
        memcpy(*out, raw_buf, buf_len);
        *len = buf_len;
    }

jwt_sign_sha_pem_done:
    if (bufkey)
        BIO_free(bufkey);
    if (pkey)
        EVP_PKEY_free(pkey);
    if (mdctx)
        EVP_MD_CTX_destroy(mdctx);
    if (ec_sig)
        ECDSA_SIG_free(ec_sig);

    return ret;
}

#define VERIFY_ERROR(__err) { ret = __err; goto jwt_verify_sha_pem_done; }

int jwt_verify_sha_pem(jwt_t *jwt, const char *sig_b64)
{
    unsigned char *sig = NULL;
    EVP_MD_CTX *mdctx = NULL;
    ECDSA_SIG *ec_sig = NULL;
    BIGNUM *ec_sig_r = NULL;
    BIGNUM *ec_sig_s = NULL;
    EVP_PKEY *pkey = NULL;
    const EVP_MD *alg;
    int type;
    int pkey_type;
    BIO *bufkey = NULL;
    int ret = 0;
    int slen;

    switch (jwt->alg) {
    /* RSA */
    case JWT_ALG_RS256:
        alg = EVP_sha256();
        type = EVP_PKEY_RSA;
        break;
    case JWT_ALG_RS384:
        alg = EVP_sha384();
        type = EVP_PKEY_RSA;
        break;
    case JWT_ALG_RS512:
        alg = EVP_sha512();
        type = EVP_PKEY_RSA;
        break;

    /* ECC */
    case JWT_ALG_ES256:
        alg = EVP_sha256();
        type = EVP_PKEY_EC;
        break;
    case JWT_ALG_ES384:
        alg = EVP_sha384();
        type = EVP_PKEY_EC;
        break;
    case JWT_ALG_ES512:
        alg = EVP_sha512();
        type = EVP_PKEY_EC;
        break;

    default:
        return EINVAL;
    }

    zend_string *sig_str = jwt_b64_url_decode(sig_b64);

    sig = (unsigned char *)ZSTR_VAL(sig_str);
    slen = ZSTR_LEN(sig_str);

    if (sig == NULL)
        VERIFY_ERROR(EINVAL);

    bufkey = BIO_new_mem_buf(ZSTR_VAL(jwt->key), ZSTR_LEN(jwt->key));
    if (bufkey == NULL)
        VERIFY_ERROR(ENOMEM);

    /* This uses OpenSSL's default passphrase callback if needed. The
     * library caller can override this in many ways, all of which are
     * outside of the scope of LibJWT and this is documented in jwt.h. */
    pkey = PEM_read_bio_PUBKEY(bufkey, NULL, NULL, NULL);
    if (pkey == NULL)
        VERIFY_ERROR(EINVAL);

    pkey_type = EVP_PKEY_id(pkey);
    if (pkey_type != type)
        VERIFY_ERROR(EINVAL);

    /* Convert EC sigs back to ASN1. */
    if (pkey_type == EVP_PKEY_EC) {
        unsigned int degree, bn_len;
        unsigned char *p;
        EC_KEY *ec_key;

        ec_sig = ECDSA_SIG_new();
        if (ec_sig == NULL)
            VERIFY_ERROR(ENOMEM);

        /* Get the actual ec_key */
        ec_key = EVP_PKEY_get1_EC_KEY(pkey);
        if (ec_key == NULL)
            VERIFY_ERROR(ENOMEM);

        degree = EC_GROUP_get_degree(EC_KEY_get0_group(ec_key));

        EC_KEY_free(ec_key);

        bn_len = (degree + 7) / 8;
        if ((bn_len * 2) != slen)
            VERIFY_ERROR(EINVAL);

        ec_sig_r = BN_bin2bn(sig, bn_len, NULL);
        ec_sig_s = BN_bin2bn(sig + bn_len, bn_len, NULL);
        if (ec_sig_r  == NULL || ec_sig_s == NULL)
            VERIFY_ERROR(EINVAL);

        ECDSA_SIG_set0(ec_sig, ec_sig_r, ec_sig_s);
        efree(sig);

        slen = i2d_ECDSA_SIG(ec_sig, NULL);
        sig = emalloc(slen);
        if (sig == NULL)
            VERIFY_ERROR(ENOMEM);

        p = sig;
        slen = i2d_ECDSA_SIG(ec_sig, &p);

        if (slen == 0)
            VERIFY_ERROR(EINVAL);
    }

    mdctx = EVP_MD_CTX_create();
    if (mdctx == NULL)
        VERIFY_ERROR(ENOMEM);

    /* Initialize the DigestVerify operation using alg */
    if (EVP_DigestVerifyInit(mdctx, NULL, alg, NULL, pkey) != 1)
        VERIFY_ERROR(EINVAL);

    /* Call update with the message */
    if (EVP_DigestVerifyUpdate(mdctx, ZSTR_VAL(jwt->str), ZSTR_LEN(jwt->str)) != 1)
        VERIFY_ERROR(EINVAL);

    /* Now check the sig for validity. */
    if (EVP_DigestVerifyFinal(mdctx, sig, slen) != 1) {
        VERIFY_ERROR(EINVAL);
    }

jwt_verify_sha_pem_done:
    if (bufkey)
        BIO_free(bufkey);
    if (pkey)
        EVP_PKEY_free(pkey);
    if (mdctx)
        EVP_MD_CTX_destroy(mdctx);
    if (sig)
        efree(sig);
    if (ec_sig)
        ECDSA_SIG_free(ec_sig);

    zend_string_free(sig_str);

    return ret;
}

/**
 * Phalcon\JWT
 *
 * RFC 7519 OAuth JSON Web Token (JWT)
 */
zend_class_entry *phalcon_jwt_ce;

/* Exceptions */
static zend_class_entry *jwt_signature_invalid_cex;
static zend_class_entry *jwt_before_valid_cex;
static zend_class_entry *jwt_expired_signature_cex;
static zend_class_entry *jwt_invalid_issuer_cex;
static zend_class_entry *jwt_invalid_aud_cex;
static zend_class_entry *jwt_invalid_jti_cex;
static zend_class_entry *jwt_invalid_iat_cex;
static zend_class_entry *jwt_invalid_sub_cex;

/* register internal class */
static zend_class_entry *jwt_register_class(const char *name)
{
    zend_class_entry ce;

	INIT_CLASS_ENTRY_EX(ce, name, strlen(name), NULL);
    return zend_register_internal_class_ex(&ce, zend_ce_exception);
}

PHP_METHOD(Phalcon_JWT, encode);
PHP_METHOD(Phalcon_JWT, decode);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_jwt_encode, 0, 0, 2)
    ZEND_ARG_ARRAY_INFO(0, payload, 1)
    ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 1)
    ZEND_ARG_TYPE_INFO(0, alg, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_jwt_decode, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, token, IS_STRING, 1)
    ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 1)
    ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_jwt_method_entry[] = {
	PHP_ME(Phalcon_JWT, encode,   arginfo_phalcon_jwt_encode, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Phalcon_JWT, decode, arginfo_phalcon_jwt_decode, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Kernel initializer
 */
PHALCON_INIT_CLASS(Phalcon_JWT){

	PHALCON_REGISTER_CLASS(Phalcon, JWT, jwt, phalcon_jwt_method_entry, 0);

    /* register exception class */
    jwt_signature_invalid_cex = jwt_register_class("Phalcon\\JWT\\SignatureInvalidException");
    jwt_before_valid_cex = jwt_register_class("Phalcon\\JWT\\BeforeValidException");
    jwt_expired_signature_cex = jwt_register_class("Phalcon\\JWT\\ExpiredSignatureException");
    jwt_invalid_issuer_cex = jwt_register_class("Phalcon\\JWT\\InvalidIssuerException");
    jwt_invalid_aud_cex = jwt_register_class("Phalcon\\JWT\\InvalidAudException");
    jwt_invalid_jti_cex = jwt_register_class("Phalcon\\JWT\\InvalidJtiException");
    jwt_invalid_iat_cex = jwt_register_class("Phalcon\\JWT\\InvalidIatException");
    jwt_invalid_sub_cex = jwt_register_class("Phalcon\\JWT\\InvalidSubException");

	return SUCCESS;
}

/* string to algorithm */
jwt_alg_t jwt_str_alg(const char *alg)
{
    if (alg == NULL)
        return JWT_ALG_INVAL;

    if (!strcasecmp(alg, "none"))
        return JWT_ALG_NONE;
    else if (!strcasecmp(alg, "HS256"))
        return JWT_ALG_HS256;
    else if (!strcasecmp(alg, "HS384"))
        return JWT_ALG_HS384;
    else if (!strcasecmp(alg, "HS512"))
        return JWT_ALG_HS512;
    else if (!strcasecmp(alg, "RS256"))
        return JWT_ALG_RS256;
    else if (!strcasecmp(alg, "RS384"))
        return JWT_ALG_RS384;
    else if (!strcasecmp(alg, "RS512"))
        return JWT_ALG_RS512;
    else if (!strcasecmp(alg, "ES256"))
        return JWT_ALG_ES256;
    else if (!strcasecmp(alg, "ES384"))
        return JWT_ALG_ES384;
    else if (!strcasecmp(alg, "ES512"))
        return JWT_ALG_ES512;

    return JWT_ALG_INVAL;
}

/* jwt sign */
static int jwt_sign(jwt_t *jwt, char **out, unsigned int *len)
{
    switch (jwt->alg) {
    /* HMAC */
    case JWT_ALG_HS256:
    case JWT_ALG_HS384:
    case JWT_ALG_HS512:
        return jwt_sign_sha_hmac(jwt, out, len);

    /* RSA */
    case JWT_ALG_RS256:
    case JWT_ALG_RS384:
    case JWT_ALG_RS512:

    /* ECC */
    case JWT_ALG_ES256:
    case JWT_ALG_ES384:
    case JWT_ALG_ES512:
        return jwt_sign_sha_pem(jwt, out, len);

    /* You wut, mate? */
    default:
        return EINVAL;
    }
}

/* jwt verify */
static int jwt_verify(jwt_t *jwt, const char *sig)
{
    switch (jwt->alg) {
    /* HMAC */
    case JWT_ALG_HS256:
    case JWT_ALG_HS384:
    case JWT_ALG_HS512:
        return jwt_verify_sha_hmac(jwt, sig);

    /* RSA */
    case JWT_ALG_RS256:
    case JWT_ALG_RS384:
    case JWT_ALG_RS512:

    /* ECC */
    case JWT_ALG_ES256:
    case JWT_ALG_ES384:
    case JWT_ALG_ES512:
        return jwt_verify_sha_pem(jwt, sig);

    /* You wut, mate? */
    default:
        return EINVAL;
    }
}

/* jwt new */
int jwt_new(jwt_t **jwt)
{
    if (!jwt) {
        return EINVAL;
    }

    *jwt = emalloc(sizeof(jwt_t));
    if (!*jwt) {
        return ENOMEM;
    }

    memset(*jwt, 0, sizeof(jwt_t));

    return 0;
}

/* jwt free */
void jwt_free(jwt_t *jwt)
{
    if (!jwt) {
        return;
    }

    efree(jwt);
}

/* base64 url safe encode */
void jwt_b64_url_encode_ex(char *str)
{
    int len = strlen(str);
    int i, t;

    for (i = t = 0; i < len; i++) {
        switch (str[i]) {
        case '+':
            str[t++] = '-';
            break;
        case '/':
            str[t++] = '_';
            break;
        case '=':
            break;
        default:
            str[t++] = str[i];
        }
    }

    str[t] = '\0';
}

/* base64 encode */
char *jwt_b64_url_encode(zend_string *input)
{
    zend_string *b64_str = php_base64_encode((const unsigned char *)ZSTR_VAL(input), ZSTR_LEN(input));

    /* replace str */
    char *new = estrdup(ZSTR_VAL(b64_str));
    jwt_b64_url_encode_ex(new);

    zend_string_free(b64_str);

    return new;
}

/* base64 decode */
zend_string *jwt_b64_url_decode(const char *src)
{
    char *new;
    int len, i, z;

    /* Decode based on RFC-4648 URI safe encoding. */
    len = strlen(src);
    new = alloca(len + 4);
    if (!new) {
        return NULL;
    }

    for (i = 0; i < len; i++) {
        switch (src[i]) {
        case '-':
            new[i] = '+';
            break;
        case '_':
            new[i] = '/';
            break;
        default:
            new[i] = src[i];
        }
    }
    z = 4 - (i % 4);
    if (z < 4) {
        while (z--) {
            new[i++] = '=';
        }	
    }
    new[i] = '\0';

    /* base64 decode */
    return php_base64_decode_ex((const unsigned char *)new, strlen(new), 1);
}

/* hash find string */
char *jwt_hash_str_find_str(zval *arr, char *key)
{
    char *str = NULL;
    zval *zv = zend_hash_str_find(Z_ARRVAL_P(arr), key, strlen(key));

    if (zv != NULL) {
        if (Z_TYPE_P(zv) == IS_STRING) {
            str = Z_STRVAL_P(zv);
        } else {
            php_error_docref(NULL, E_WARNING, "%s type must be string", key);
        }
    } 

    return str;
}

/* hash find long */
long jwt_hash_str_find_long(zval *arr, char *key)
{
    zval *zv = zend_hash_str_find(Z_ARRVAL_P(arr), key, strlen(key));

    if (zv != NULL) {
        if (Z_TYPE_P(zv) == IS_LONG) {
            return Z_LVAL_P(zv);
        } else {
            php_error_docref(NULL, E_WARNING, "%s type must be long", key);
        }
    }

    return 0;
}

/* verify string claims */
int jwt_verify_claims_str(zval *arr, char *key, char *str)
{
    char *rs = jwt_hash_str_find_str(arr, key);
    if (rs && str && strcmp(rs, str)) {
        return FAILURE;
    }

    return 0;
}

/* array equals */
int jwt_array_equals(zend_array *arr1, zend_array *arr2) {
    zend_ulong i;
    zval *value = NULL;

    if (arr1 && arr2) {
        if (zend_array_count(arr1) != zend_array_count(arr2)) {
            return FAILURE;
        }

        ZEND_HASH_FOREACH_NUM_KEY_VAL(arr1, i, value) {
            zval *tmp = zend_hash_index_find(arr2, i);

            if (value && tmp){
                if (Z_TYPE_P(value) == IS_STRING && Z_TYPE_P(tmp) == IS_STRING) {
                    if (strcmp(Z_STRVAL_P(value), Z_STRVAL_P(tmp))) {
                        return FAILURE;
                    }
                } else {
                    php_error_docref(NULL, E_WARNING, "Aud each item type must be string");
                }
            }
        } ZEND_HASH_FOREACH_END();
    }

    return 0;
}

/* verify body */
int jwt_verify_body(char *body, zval *return_value)
{
    zend_class_entry *ce = NULL;
    char *err_msg = NULL;
    time_t curr_time = time((time_t*)NULL);
    zend_string *vs = jwt_b64_url_decode(body);

#define FORMAT_CEX_TIME(t, cex) do {                                                            \
       struct tm *timeinfo;                                                                     \
       char buf[128];                                                                           \
       timeinfo = localtime(&curr_time);                                                                \
       strftime(buf, sizeof(buf), "Cannot handle token prior to %Y-%m-%d %H:%M:%S", timeinfo);  \
       ce = cex;                                                                                \
       err_msg = buf;                                                                           \
    } while(0);

#define FORMAT_CEX_MSG(msg, cex) do {   \
        ce = cex;                       \
        err_msg = msg;                  \
    } while(0);

    if (!vs) {
        FORMAT_CEX_MSG("Invalid body", spl_ce_UnexpectedValueException);
        goto done;
    }

    /* decode json to array */
    php_json_decode_ex(return_value, ZSTR_VAL(vs), ZSTR_LEN(vs), PHP_JSON_OBJECT_AS_ARRAY, 512);
    zend_string_free(vs);

    if (Z_TYPE(*return_value) == IS_ARRAY) {
        /* set expiration and not before */
        JWT_G(expiration) = jwt_hash_str_find_long(return_value, "exp");
        JWT_G(not_before) = jwt_hash_str_find_long(return_value, "nbf");
        JWT_G(iat) = jwt_hash_str_find_long(return_value, "iat");

        /* expiration */
        if (JWT_G(expiration) && (curr_time - JWT_G(leeway)) >= JWT_G(expiration)) {
            FORMAT_CEX_MSG("Expired token", jwt_expired_signature_cex);
			goto done;
        /* not before */
        } else if (JWT_G(not_before) && JWT_G(not_before) > (curr_time + JWT_G(leeway))) {
            FORMAT_CEX_TIME(JWT_G(not_before), jwt_before_valid_cex);
			goto done;
        /* iat */
        } else if (JWT_G(iat) && JWT_G(iat) > (curr_time + JWT_G(leeway))) {
            FORMAT_CEX_TIME(JWT_G(iat), jwt_invalid_iat_cex);
			goto done;
        /* iss */
        } else if (jwt_verify_claims_str(return_value, "iss", JWT_G(iss))) {
            FORMAT_CEX_MSG("Invalid Issuer", jwt_invalid_issuer_cex);
			goto done;
        /* jti */
        } else if (jwt_verify_claims_str(return_value, "jti", JWT_G(jti))) {
            FORMAT_CEX_MSG("Invalid Jti", jwt_invalid_jti_cex);
			goto done;
		}

        /* aud */
        size_t flag = 0;
        zval *zv_aud = zend_hash_str_find(Z_ARRVAL_P(return_value), "aud", strlen("aud"));

        if (zv_aud && JWT_G(aud)) {
            switch(Z_TYPE_P(zv_aud)) {
            case IS_ARRAY:
                if (jwt_array_equals(Z_ARRVAL_P(JWT_G(aud)), Z_ARRVAL_P(zv_aud))) flag = 1;
                break;
            case IS_STRING:
                if (strcmp(Z_STRVAL_P(JWT_G(aud)), Z_STRVAL_P(zv_aud))) flag = 1;
                break;
            default:
                php_error_docref(NULL, E_WARNING, "Aud type must be string or array");
                break;
            }

            if (flag) {
				FORMAT_CEX_MSG("Invalid Aud", jwt_invalid_aud_cex);
				goto done;
			}
        }

        /* sub */
        if (jwt_verify_claims_str(return_value, "sub", JWT_G(sub))) {
            FORMAT_CEX_MSG("Invalid Sub", jwt_invalid_sub_cex);
			goto done;
		}
    } else {
        FORMAT_CEX_MSG("Json decode error", spl_ce_UnexpectedValueException);
		goto done;
    }

done:
    if (err_msg) {
        zend_throw_exception(ce, err_msg, 0);
        return FAILURE;
    }

    return 0;
}

/* parse options */
int jwt_parse_options(zval *options)
{
    /* check options */
    if (options != NULL) {
        switch(Z_TYPE_P(options)) {
        case IS_ARRAY:
            {
                /* check algorithm */
                char *alg = jwt_hash_str_find_str(options, "algorithm");
                if (alg) {
                    JWT_G(algorithm) = alg;
                }
                
                /* options */
                JWT_G(leeway) = jwt_hash_str_find_long(options, "leeway");
                JWT_G(iss) = jwt_hash_str_find_str(options, "iss");
                JWT_G(jti) = jwt_hash_str_find_str(options, "jti");
                JWT_G(aud) = zend_hash_str_find(Z_ARRVAL_P(options), "aud", strlen("aud"));
                JWT_G(sub) = jwt_hash_str_find_str(options, "sub");
            }
            break;
        case IS_NULL:
        case IS_FALSE:
            JWT_G(algorithm) = "none";
            break;
        default:
            break;
        }
    }

    return 0;
}

/* Jwt encode */
static void php_jwt_encode(INTERNAL_FUNCTION_PARAMETERS) {
    zval *payload = NULL, header;
    zend_string *key = NULL;
    smart_str json_header = {0}, json_payload = {0};

    char *sig = NULL, *alg = "HS256", *buf = NULL;
    unsigned int sig_len;
    size_t alg_len;
    jwt_t *jwt = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "aS|s", &payload, &key, &alg, &alg_len) == FAILURE) {
        return;
    }

    /* init jwt */
    jwt_new(&jwt);

    /* check algorithm */
    jwt->alg = jwt_str_alg(alg);

    if (jwt->alg == JWT_ALG_INVAL) {
        zend_throw_exception(spl_ce_UnexpectedValueException, "Algorithm not supported", 0);
        goto encode_done;
    }

    /* init */
    array_init(&header);

    /* JWT header array */
    add_assoc_string(&header, "typ", "JWT");
    add_assoc_string(&header, "alg", alg);

    /* json encode */
    php_json_encode(&json_header, &header, 0);
    char *header_b64 = jwt_b64_url_encode(json_header.s);

    php_json_encode(&json_payload, payload, 0);
    char *payload_b64 = jwt_b64_url_encode(json_payload.s);

    zval_ptr_dtor(&header);
    smart_str_free(&json_header);
    smart_str_free(&json_payload);

	int buflen = strlen(header_b64) + strlen(payload_b64) + 2;
    buf = (char *)ecalloc(buflen, 1);
    strcpy(buf, header_b64);
    strcat(buf, ".");
    strcat(buf, payload_b64);

    efree(header_b64);
    efree(payload_b64);

    /* sign */
    if (jwt->alg == JWT_ALG_NONE) {
		buflen+=1;
        /* alg none */
        buf = (char *)erealloc(buf, buflen);
        strcat(buf, ".");
		buf[buflen]='\0';
    } else {
        /* set jwt struct */
        jwt->key = key;
        jwt->str = zend_string_init(buf, strlen(buf), 0);

        /* sign */
        if (jwt_sign(jwt, &sig, &sig_len)) {
            zend_throw_exception(spl_ce_DomainException, "OpenSSL unable to sign data", 0);
            zend_string_free(jwt->str);
            goto encode_done;
        }

        /* string concatenation */
        zend_string *sig_str = zend_string_init(sig, sig_len, 0);
        char *sig_b64 = jwt_b64_url_encode(sig_str);

		buflen = strlen(sig_b64) + strlen(buf) + 2;
        char *tmp = (char *)ecalloc(buflen, 1);
        sprintf(tmp, "%s.%s", buf, sig_b64);

        efree(buf);
        buf = tmp;

        efree(sig_b64);
        zend_string_free(jwt->str);
        zend_string_free(sig_str);
    }

encode_done:
    /* free */
    if (sig)
        efree(sig);

    jwt_free(jwt);

	RETVAL_STRINGL(buf, strlen(buf));
    efree(buf);
}

/* Jwt decode */
static void php_jwt_decode(INTERNAL_FUNCTION_PARAMETERS) {
    zend_string *token = NULL, *key = NULL;
    zval *options = NULL;
    smart_str buf = {0};
    char *body = NULL, *sig = NULL;
    jwt_t *jwt = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "SS|z", &token, &key, &options) == FAILURE) {
        return;
    }

    char *head = estrdup(ZSTR_VAL(token));

    /* jwt init */
    jwt_new(&jwt);

    /* Parse options */
    if (jwt_parse_options(options) == FAILURE) {
        zend_throw_exception(spl_ce_UnexpectedValueException, "Options parse error", 0);
        goto decode_done;
    }

    /* Algorithm */
    jwt->alg = jwt_str_alg(JWT_G(algorithm));

    if (jwt->alg == JWT_ALG_INVAL) {
        zend_throw_exception(spl_ce_UnexpectedValueException, "Algorithm not supported", 0);
        goto decode_done;
    }

    /* Find the components. */
    for (body = head; body[0] != '.'; body++) {
        if (body[0] == '\0') {
            goto decode_done;
        }	
    }

    body[0] = '\0';
    body++;

    for (sig = body; sig[0] != '.'; sig++) {
        if (sig[0] == '\0') {
            goto decode_done;
        }
    }

    sig[0] = '\0';
    sig++;

    /* verify head */
    zval zv;
    zend_string *json_h = jwt_b64_url_decode(head);

    if (!json_h) {
        zend_throw_exception(spl_ce_UnexpectedValueException, "Base64 decode error", 0);
        goto decode_done;
    }

    php_json_decode_ex(&zv, ZSTR_VAL(json_h), ZSTR_LEN(json_h), PHP_JSON_OBJECT_AS_ARRAY, 512);
    zend_string_free(json_h);

    if (Z_TYPE(zv) == IS_ARRAY) {
        zval *zalg = zend_hash_str_find(Z_ARRVAL(zv), "alg", strlen("alg"));

        zval_ptr_dtor(&zv);

        if (strcmp(Z_STRVAL_P(zalg), JWT_G(algorithm))) {
            zend_throw_exception(spl_ce_UnexpectedValueException, "Algorithm not allowed", 0);
            goto decode_done;
        }
    } else {
        zend_throw_exception(spl_ce_UnexpectedValueException, "Json decode error", 0);
        goto decode_done;
    }

    /* verify */
    if (jwt->alg == JWT_ALG_NONE) {
        /* done */
    } else {
        /* set jwt struct */
        jwt->key = key;

        smart_str_appends(&buf, head);
        smart_str_appends(&buf, ".");
        smart_str_appends(&buf, body);

        jwt->str = buf.s;

        if (jwt_verify(jwt, sig)) {
            zend_throw_exception(jwt_signature_invalid_cex, "Signature verification failed", 0);
            goto decode_done;
        }
    }

    /* verify body */
    if (jwt_verify_body(body, return_value) == FAILURE) {
        goto decode_done;
    }

decode_done:
    efree(head);
    jwt_free(jwt);
    smart_str_free(&buf);
}

/**
 *
 */
PHP_METHOD(Phalcon_JWT, encode)
{
    php_jwt_encode(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

/**
 *
 */
PHP_METHOD(Phalcon_JWT, decode)
{
    php_jwt_decode(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}