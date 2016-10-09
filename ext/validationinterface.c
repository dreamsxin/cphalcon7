
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

#include "validationinterface.h"
#include "kernel/main.h"

zend_class_entry *phalcon_validationinterface_ce;

static const zend_function_entry phalcon_validationinterface_method_entry[] = {
	PHP_ABSTRACT_ME(Phalcon_ValidationInterface, validate, arginfo_phalcon_validationinterface_validate)
	PHP_ABSTRACT_ME(Phalcon_ValidationInterface, add, arginfo_phalcon_validationinterface_add)
	PHP_ABSTRACT_ME(Phalcon_ValidationInterface, getMessages, NULL)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\ValidatorInterface initializer
 */
PHALCON_INIT_CLASS(Phalcon_ValidationInterface){

	PHALCON_REGISTER_INTERFACE(Phalcon, ValidationInterface, validationinterface, phalcon_validationinterface_method_entry);

	return SUCCESS;
}

/**
 * Validate a set of data according to a set of rules
 *
 * @param array|object $data
 * @param object $entity
 * @return Phalcon\Validation\Message\Group
 */
PHALCON_DOC_METHOD(Phalcon_ValidationInterface, validate);

/**
 * Adds a validator to a field
 *
 * @param string $attribute
 * @param Phalcon\Validation\ValidatorInterface
 * @return Phalcon\Validation
 */
PHALCON_DOC_METHOD(Phalcon_ValidationInterface, add);

/**
 * Returns the registered validators
 *
 * @return Phalcon\Validation\Message\Group
 */
PHALCON_DOC_METHOD(Phalcon_ValidationInterface, getMessages);
