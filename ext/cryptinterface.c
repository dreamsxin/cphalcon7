
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
  +------------------------------------------------------------------------+
*/

#include "cryptinterface.h"
#include "kernel/main.h"

zend_class_entry *phalcon_cryptinterface_ce;

static const zend_function_entry phalcon_cryptinterface_method_entry[] = {
	PHP_ABSTRACT_ME(Phalcon_CryptInterface, setMethod, arginfo_phalcon_cryptinterface_setmethod)
	PHP_ABSTRACT_ME(Phalcon_CryptInterface, getMethod, arginfo_empty)
	PHP_ABSTRACT_ME(Phalcon_CryptInterface, setKey, arginfo_phalcon_cryptinterface_setkey)
	PHP_ABSTRACT_ME(Phalcon_CryptInterface, getKey, arginfo_empty)
	PHP_ABSTRACT_ME(Phalcon_CryptInterface, encrypt, arginfo_phalcon_cryptinterface_encrypt)
	PHP_ABSTRACT_ME(Phalcon_CryptInterface, decrypt, arginfo_phalcon_cryptinterface_decrypt)
	PHP_ABSTRACT_ME(Phalcon_CryptInterface, encryptBase64, arginfo_phalcon_cryptinterface_encryptbase64)
	PHP_ABSTRACT_ME(Phalcon_CryptInterface, decryptBase64, arginfo_phalcon_cryptinterface_decryptbase64)
	PHP_ABSTRACT_ME(Phalcon_CryptInterface, getAvailableMethods, arginfo_empty)
	PHP_FE_END
};

/**
 * Phalcon\CryptInterface initializer
 */
PHALCON_INIT_CLASS(Phalcon_CryptInterface){

	PHALCON_REGISTER_INTERFACE(Phalcon, CryptInterface, cryptinterface, phalcon_cryptinterface_method_entry);

	return SUCCESS;
}

/**
 * Sets the cipher method
 *
 * @param string $cipher
 * @return Phalcon\CryptInterface
 */
PHALCON_DOC_METHOD(Phalcon_CryptInterface, setMethod);

/**
 * Gets the cipher method
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_CryptInterface, getMethod);

/**
 * Sets the encryption key
 *
 * @param string $key
 * @return Phalcon\CryptInterface
 */
PHALCON_DOC_METHOD(Phalcon_CryptInterface, setKey);

/**
 * Returns the encryption key
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_CryptInterface, getKey);

/**
 * Encrypts a text
 *
 * @param string $text
 * @param string $key
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_CryptInterface, encrypt);

/**
 * Decrypts a text
 *
 * @param string $text
 * @param string $key
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_CryptInterface, decrypt);

/**
 * Encrypts a text returning the result as a base64 string
 *
 * @param string $text
 * @param string $key
 * @param bool $url_safe
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_CryptInterface, encryptBase64);

/**
 * Decrypt a text that is coded as a base64 string
 *
 * @param string $text
 * @param string $key
 * @param bool $url_safe
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_CryptInterface, decryptBase64);

/**
 * Returns a list of available methods
 *
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_CryptInterface, getAvailableMethods);
