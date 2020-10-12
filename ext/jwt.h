
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

#ifndef PHALCON_JWT_H
#define PHALCON_JWT_H

#include "php_phalcon.h"

/** JWT algorithm types. */
typedef enum jwt_alg {
  JWT_ALG_NONE = 0,
  JWT_ALG_HS256,
  JWT_ALG_HS384,
  JWT_ALG_HS512,
  JWT_ALG_RS256,
  JWT_ALG_RS384,
  JWT_ALG_RS512,
  JWT_ALG_ES256,
  JWT_ALG_ES384,
  JWT_ALG_ES512,
  JWT_ALG_TERM
} jwt_alg_t;

#define JWT_ALG_INVAL JWT_ALG_TERM

/** Opaque JWT object. */
typedef struct jwt {
  jwt_alg_t alg;
  zend_string *key;
  zend_string *str;
} jwt_t;

char *jwt_b64_url_encode(zend_string *input);
void jwt_b64_url_encode_ex(char *str);
zend_string *jwt_b64_url_decode(const char *src);

int jwt_sign_sha_hmac(jwt_t *jwt, char **out, unsigned int *len);
int jwt_verify_sha_hmac(jwt_t *jwt, const char *sig);

int jwt_sign_sha_pem(jwt_t *jwt, char **out, unsigned int *len);
int jwt_verify_sha_pem(jwt_t *jwt, const char *sig_b64);

extern zend_class_entry *phalcon_jwt_ce;

PHALCON_INIT_CLASS(Phalcon_JWT);

#endif /* PHALCON_JWT_H */
