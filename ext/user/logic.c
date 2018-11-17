
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

#include "user/logic.h"
#include "di/injectable.h"

#include "kernel/main.h"
#include "kernel/object.h"
#include "kernel/fcall.h"

/**
 * Phalcon\User\Logic
 *
 * This class can be used to provide user business logic an easy access to services in the application
 */
zend_class_entry *phalcon_user_logic_ce;

PHP_METHOD(Phalcon_User_Logic, __construct);
PHP_METHOD(Phalcon_User_Logic, getActionName);
PHP_METHOD(Phalcon_User_Logic, getActionParams);
PHP_METHOD(Phalcon_User_Logic, setContent);
PHP_METHOD(Phalcon_User_Logic, getContent);
PHP_METHOD(Phalcon_User_Logic, call);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_user_logic_method___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, actionName)
	ZEND_ARG_INFO(0, arguments)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_user_logic_method_setcontent, 0, 0, 1)
	ZEND_ARG_INFO(0, content)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_user_logic_method_call, 0, 0, 0)
	ZEND_ARG_INFO(0, actionName)
	ZEND_ARG_INFO(0, arguments)
ZEND_END_ARG_INFO()

const zend_function_entry phalcon_user_logic_method_entry[] = {
	PHP_ME(Phalcon_User_Logic,	__construct,		arginfo_phalcon_user_logic_method___construct,	ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(Phalcon_User_Logic,	getActionName,		NULL,											ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_User_Logic,	getActionParams,	NULL,											ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_User_Logic,	setContent,			arginfo_phalcon_user_logic_method_setcontent,	ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_User_Logic,	getContent,			NULL,											ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_User_Logic,	call,				arginfo_phalcon_user_logic_method_call,			ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\User\Logic initializer
 */
PHALCON_INIT_CLASS(Phalcon_User_Logic){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\User, Logic, user_logic, phalcon_di_injectable_ce, phalcon_user_logic_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_user_logic_ce, SL("_actionName"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_user_logic_ce, SL("_actionParams"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_user_logic_ce, SL("_content"), ZEND_ACC_PROTECTED);
	return SUCCESS;
}

/**
 * Constructor for Phalcon\User\Logic
 *
 * @param string $actionName
 * @param array $params
 */
PHP_METHOD(Phalcon_User_Logic, __construct){

	zval *action = NULL, *params = NULL;

	phalcon_fetch_params(0, 0, 2, &action, &params);

	if (action) {
		phalcon_update_property(getThis(), SL("_actionName"), action);
	}

	if (params) {
		PHALCON_SEPARATE_PARAM(params);
		phalcon_update_property(getThis(), SL("_actionParams"), params);
	}
}

/**
 * Gets the action name
 *
 * @return string
 */
PHP_METHOD(Phalcon_User_Logic, getActionName){


	RETURN_MEMBER(getThis(), "_actionName");
}

/**
 * Gets action params
 *
 * @return array
 */
PHP_METHOD(Phalcon_User_Logic, getActionParams){


	RETURN_MEMBER(getThis(), "_actionParams");
}

/**
 * Sets content
 *
 * @param mixed $content
 */
PHP_METHOD(Phalcon_User_Logic, setContent){

	zval *content;

	phalcon_fetch_params(0, 1, 0, &content);

	phalcon_update_property(getThis(), SL("_content"), content);
}

/**
 * Gets content
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_User_Logic, getContent){


	RETURN_MEMBER(getThis(), "_content");
}

/**
 * Loads an logic and prepares it for manipulation
 *
 * @param string $actionName
 * @param array $params
 * @return object
 */
PHP_METHOD(Phalcon_User_Logic, call){

	zval *action = NULL, *params = NULL, logic_name = {};
	zend_class_entry *ce0;

	phalcon_fetch_params(0, 0, 2, &action, &params);

	if (!action) {
		action = &PHALCON_GLOBAL(z_null);
	}

	if (!params) {
		params = &PHALCON_GLOBAL(z_null);
	}

	phalcon_get_called_class(&logic_name);

	ce0 = phalcon_class_exists(&logic_name, 1);
	PHALCON_OBJECT_INIT(return_value, ce0);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", action, params);
}
