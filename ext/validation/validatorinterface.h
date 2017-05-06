
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
  +------------------------------------------------------------------------+
*/

#ifndef PHALCON_VALIDATION_VALIDATORINTERFACE_H
#define PHALCON_VALIDATION_VALIDATORINTERFACE_H

#include "php_phalcon.h"

#include "validationinterface.h"

extern zend_class_entry *phalcon_validation_validatorinterface_ce;

PHALCON_INIT_CLASS(Phalcon_Validation_ValidatorInterface);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_validatorinterface_issetoption, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_validatorinterface_getoption, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_validatorinterface_setoption, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_validation_validatorinterface_validate, 0, 0, 2)
	ZEND_ARG_OBJ_INFO(0, validator, Phalcon\\ValidationInterface, 0)
	ZEND_ARG_INFO(0, attribute)
	ZEND_ARG_TYPE_INFO(0, allowEmpty, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

#endif /* PHALCON_VALIDATION_VALIDATORINTERFACE_H */
