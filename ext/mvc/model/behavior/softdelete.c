
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

#include "mvc/model/behavior/softdelete.h"
#include "mvc/model/behavior.h"
#include "mvc/model/behaviorinterface.h"
#include "mvc/model/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/operators.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/exception.h"
#include "kernel/object.h"

/**
 * Phalcon\Mvc\Model\Behavior\SoftDelete
 *
 * Instead of permanently delete a record it marks the record as
 * deleted changing the value of a flag column
 */
zend_class_entry *phalcon_mvc_model_behavior_softdelete_ce;

PHP_METHOD(Phalcon_Mvc_Model_Behavior_SoftDelete, notify);

static const zend_function_entry phalcon_mvc_model_behavior_softdelete_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Behavior_SoftDelete, notify, arginfo_phalcon_mvc_model_behaviorinterface_notify, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Behavior\SoftDelete initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Behavior_SoftDelete){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\Behavior, SoftDelete, mvc_model_behavior_softdelete, phalcon_mvc_model_behavior_ce, phalcon_mvc_model_behavior_softdelete_method_entry, 0);

	zend_class_implements(phalcon_mvc_model_behavior_softdelete_ce, 1, phalcon_mvc_model_behaviorinterface_ce);

	return SUCCESS;
}

/**
 * Listens for notifications from the models manager
 *
 * @param string $type
 * @param Phalcon\Mvc\ModelInterface $model
 */
PHP_METHOD(Phalcon_Mvc_Model_Behavior_SoftDelete, notify){

	zval *type, *model, options = {}, value = {}, field = {}, actual_value = {}, update_model = {}, status = {}, messages = {}, *message, event_name = {};

	phalcon_fetch_params(0, 2, 0, &type, &model);

	if (PHALCON_IS_STRING(type, "beforeDelete")) {
		PHALCON_CALL_METHOD(&options, getThis(), "getoptions");

		/**
		 * 'value' is the value to be updated instead of delete the record
		 */
		if (!phalcon_array_isset_fetch_str(&value, &options, SL("value"), PH_READONLY)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The option 'value' is required");
			return;
		}

		/**
		 * 'field' is the attribute to be updated instead of delete the record
		 */
		if (!phalcon_array_isset_fetch_str(&field, &options, SL("field"), PH_READONLY)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The option 'field' is required");
			return;
		}

		/**
		 * Skip the current operation
		 */
		PHALCON_CALL_METHOD(NULL, model, "skipoperation", &PHALCON_GLOBAL(z_true));
		PHALCON_CALL_METHOD(&actual_value, model, "readattribute", &field);

		/**
		 * If the record is already flagged as 'deleted' we don't delete it again
		 */
		if (!PHALCON_IS_EQUAL(&actual_value, &value)) {
			/**
			 * Clone the current model to make a clean new operation
			 */
			if (phalcon_clone(&update_model, model) == FAILURE) {
				return;
			}

			PHALCON_CALL_METHOD(NULL, &update_model, "writeattribute", &field, &value);

			/**
			 * Update the cloned model
			 */
			PHALCON_CALL_METHOD(&status, &update_model, "save");
			if (!zend_is_true(&status)) {
				/**
				 * Transfer the messages from the cloned model to the original model
				 */
				PHALCON_CALL_METHOD(&messages, &update_model, "getmessages");

				ZEND_HASH_FOREACH_VAL(Z_ARRVAL(messages), message) {
					PHALCON_CALL_METHOD(NULL, model, "appendmessage", message);
				} ZEND_HASH_FOREACH_END();

				RETURN_FALSE;
			}

			/**
			 * Update the original model too
			 */
			PHALCON_CALL_METHOD(NULL, model, "writeattribute", &field, &value);

			/**
			 * Fire the beforeDelete event
			 */
 			ZVAL_STRING(&event_name, "afterDelete");
			PHALCON_CALL_METHOD(NULL, model, "fireevent", &event_name);
			zval_ptr_dtor(&event_name);
		}
	}
}
