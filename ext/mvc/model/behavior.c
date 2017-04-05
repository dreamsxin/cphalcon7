
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

#include "mvc/model/behavior.h"
#include "mvc/model/behaviorinterface.h"

#include "kernel/main.h"
#include "kernel/object.h"
#include "kernel/array.h"

/**
 * Phalcon\Mvc\Model\Behavior
 *
 * This is an optional base class for ORM behaviors
 */
zend_class_entry *phalcon_mvc_model_behavior_ce;

PHP_METHOD(Phalcon_Mvc_Model_Behavior, __construct);
PHP_METHOD(Phalcon_Mvc_Model_Behavior, mustTakeAction);
PHP_METHOD(Phalcon_Mvc_Model_Behavior, getOptions);
PHP_METHOD(Phalcon_Mvc_Model_Behavior, notify);
PHP_METHOD(Phalcon_Mvc_Model_Behavior, missingMethod);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_mvc_model_behavior___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()


static const zend_function_entry phalcon_mvc_model_behavior_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Behavior, __construct, arginfo_phalcon_mvc_model_behavior___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Mvc_Model_Behavior, mustTakeAction, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model_Behavior, getOptions, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Mvc_Model_Behavior, notify, arginfo_phalcon_mvc_model_behaviorinterface_notify, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Mvc_Model_Behavior, missingMethod, arginfo_phalcon_mvc_model_behaviorinterface_missingmethod, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Behavior initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Behavior){

	PHALCON_REGISTER_CLASS(Phalcon\\Mvc\\Model, Behavior, mvc_model_behavior, phalcon_mvc_model_behavior_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_mvc_model_behavior_ce, SL("_options"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_mvc_model_behavior_ce, 1, phalcon_mvc_model_behaviorinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Mvc\Model\Behavior
 *
 * @param array $options
 */
PHP_METHOD(Phalcon_Mvc_Model_Behavior, __construct){

	zval *options = NULL;

	phalcon_fetch_params(0, 0, 1, &options);

	if (!options) {
		options = &PHALCON_GLOBAL(z_null);
	}

	phalcon_update_property(getThis(), SL("_options"), options);
}

/**
 * Checks whether the behavior must take action on certain event
 *
 * @param string $eventName
 */
PHP_METHOD(Phalcon_Mvc_Model_Behavior, mustTakeAction){

	zval *eventname, options = {};

	phalcon_fetch_params(0, 1, 0, &eventname);

	phalcon_read_property(&options, getThis(), SL("_options"), PH_NOISY|PH_READONLY);
	if (phalcon_array_isset(&options, eventname)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

/**
 * Returns the behavior options related to an event
 *
 * @param string $eventName
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_Behavior, getOptions){

	zval *eventname = NULL, options = {}, event_options = {};

	phalcon_fetch_params(0, 0, 1, &eventname);

	phalcon_read_property(&options, getThis(), SL("_options"), PH_NOISY|PH_READONLY);
	if (eventname && Z_TYPE_P(eventname) != IS_NULL) {
		if (phalcon_array_isset_fetch(&event_options, &options, eventname, PH_READONLY)) {
			RETURN_CTOR(&event_options);
		}
		RETURN_NULL();
	}

	RETURN_ZVAL(&options, 1, 0);
}

/**
 * This method receives the notifications from the EventsManager
 *
 * @param string $type
 * @param Phalcon\Mvc\ModelInterface $model
 */
PHP_METHOD(Phalcon_Mvc_Model_Behavior, notify){

	zval *type, *model;

	phalcon_fetch_params(0, 2, 0, &type, &model);

	RETURN_NULL();
}

/**
 * Acts as fallbacks when a missing method is called on the model
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @param string $method
 * @param array $arguments
 */
PHP_METHOD(Phalcon_Mvc_Model_Behavior, missingMethod){

	zval *model, *method, *arguments = NULL;

	phalcon_fetch_params(0, 2, 1, &model, &method, &arguments);

	RETURN_NULL();
}
