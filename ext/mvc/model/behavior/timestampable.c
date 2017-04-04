
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

#include "mvc/model/behavior/timestampable.h"
#include "mvc/model/behavior.h"
#include "mvc/model/behaviorinterface.h"
#include "mvc/model/exception.h"

#include <Zend/zend_closures.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"
#include "kernel/array.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/string.h"

/**
 * Phalcon\Mvc\Model\Behavior\Timestampable
 *
 * Allows to automatically update a model’s attribute saving the
 * datetime when a record is created or updated
 */
zend_class_entry *phalcon_mvc_model_behavior_timestampable_ce;

PHP_METHOD(Phalcon_Mvc_Model_Behavior_Timestampable, notify);

static const zend_function_entry phalcon_mvc_model_behavior_timestampable_method_entry[] = {
	PHP_ME(Phalcon_Mvc_Model_Behavior_Timestampable, notify, arginfo_phalcon_mvc_model_behaviorinterface_notify, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Mvc\Model\Behavior\Timestampable initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_Behavior_Timestampable){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Mvc\\Model\\Behavior, Timestampable, mvc_model_behavior_timestampable, phalcon_mvc_model_behavior_ce, phalcon_mvc_model_behavior_timestampable_method_entry, 0);

	zend_class_implements(phalcon_mvc_model_behavior_timestampable_ce, 1, phalcon_mvc_model_behaviorinterface_ce);

	return SUCCESS;
}

/**
 * Listens for notifications from the models manager
 *
 * @param string $type
 * @param Phalcon\Mvc\ModelInterface $model
 */
PHP_METHOD(Phalcon_Mvc_Model_Behavior_Timestampable, notify){

	zval *type, *model, take_action = {}, options = {}, field = {}, format = {}, timestamp = {}, generator = {}, *single_field;

	phalcon_fetch_params(0, 2, 0, &type, &model);

	/**
	 * Check if the developer decided to take action here
	 */
	PHALCON_CALL_METHOD(&take_action, getThis(), "musttakeaction", type);
	if (PHALCON_IS_NOT_TRUE(&take_action)) {
		RETURN_NULL();
	}

	PHALCON_CALL_METHOD(&options, getThis(), "getoptions", type);
	if (Z_TYPE(options) == IS_ARRAY) {

		/**
		 * The field name is required in this behavior
		 */
		if (!phalcon_array_isset_fetch_str(&field, &options, SL("field"), PH_READONLY)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The option 'field' is required");
			return;
		}

		if (phalcon_array_isset_fetch_str(&format, &options, SL("format"), PH_READONLY)) {
			/**
			 * Format is a format for date()
			 */
			phalcon_date(&timestamp, &format, NULL);
		} else if (phalcon_array_isset_fetch_str(&generator, &options, SL("generator"), PH_READONLY)) {
			/**
			 * A generator is a closure that produce the correct timestamp value
			 */
			if (Z_TYPE(generator) == IS_OBJECT) {
				if (instanceof_function(Z_OBJCE(generator), zend_ce_closure)) {
					PHALCON_CALL_USER_FUNC(&timestamp, &generator);
				}
			}
		}

		/**
		 * Last resort call time()
		 */
		if (Z_TYPE(timestamp) <= IS_NULL) {
			ZVAL_LONG(&timestamp, (long) time(NULL));
		}

		/**
		 * Assign the value to the field, use writeattribute if the property is protected
		 */
		if (unlikely(Z_TYPE(field) == IS_ARRAY)) {
			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(field), single_field) {
				PHALCON_CALL_METHOD(NULL, model, "writeattribute", single_field, &timestamp);
			} ZEND_HASH_FOREACH_END();
		} else {
			PHALCON_CALL_METHOD(NULL, model, "writeattribute", &field, &timestamp);
		}
	}
}
