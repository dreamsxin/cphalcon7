
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

#include "di/service/builder.h"
#include "di/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/concat.h"
#include "kernel/exception.h"
#include "kernel/array.h"
#include "kernel/fcall.h"
#include "kernel/object.h"
#include "kernel/operators.h"
#include "kernel/hash.h"

/**
 * Phalcon\Di\Service\Builder
 *
 * This class builds instances based on complex definitions
 */
zend_class_entry *phalcon_di_service_builder_ce;

PHP_METHOD(Phalcon_Di_Service_Builder, _buildParameter);
PHP_METHOD(Phalcon_Di_Service_Builder, _buildParameters);
PHP_METHOD(Phalcon_Di_Service_Builder, build);


static const zend_function_entry phalcon_di_service_builder_method_entry[] = {
	PHP_ME(Phalcon_Di_Service_Builder, _buildParameter, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Di_Service_Builder, _buildParameters, NULL, ZEND_ACC_PROTECTED)
	PHP_ME(Phalcon_Di_Service_Builder, build, arginfo_phalcon_di_service_builder_build, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Di\Service\Builder initializer
 */
PHALCON_INIT_CLASS(Phalcon_Di_Service_Builder){

	PHALCON_REGISTER_CLASS(Phalcon\\Di\\Service, Builder, di_service_builder, phalcon_di_service_builder_method_entry, 0);

	return SUCCESS;
}

/**
 * Resolves a constructor/call parameter
 *
 * @param Phalcon\DiInterface $dependencyInjector
 * @param int $position
 * @param array $argument
 * @return mixed
 */
PHP_METHOD(Phalcon_Di_Service_Builder, _buildParameter){

	zval *dependency_injector, *position, *argument, exception_message = {}, type = {}, name = {}, value = {}, instance_arguments = {};

	phalcon_fetch_params(0, 3, 0, &dependency_injector, &position, &argument);

	/**
	 * All the arguments must be an array
	 */
	if (Z_TYPE_P(argument) != IS_ARRAY) {
		PHALCON_CONCAT_SVS(&exception_message, "Argument at position ", position, " must be an array");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_di_exception_ce, &exception_message);
		zval_ptr_dtor(&exception_message);
		return;
	}

	if (Z_TYPE_P(dependency_injector) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_di_exception_ce, "The dependency injector container is not valid");
		return;
	}

	/**
	 * All the arguments must have a type
	 */
	if (!phalcon_array_isset_fetch_str(&type, argument, SL("type"), PH_READONLY)) {
		PHALCON_CONCAT_SVS(&exception_message, "Argument at position ", position, " must have a type");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_di_exception_ce, &exception_message);
		zval_ptr_dtor(&exception_message);
		return;
	}

	/**
	 * If the argument type is 'service', we obtain the service from the DI
	 */
	if (PHALCON_IS_STRING(&type, "service")) {
		if (!phalcon_array_isset_fetch_str(&name, argument, SL("name"), PH_READONLY)) {
			PHALCON_CONCAT_SV(&exception_message, "Service 'name' is required in parameter on position ", position);
			PHALCON_THROW_EXCEPTION_ZVAL(phalcon_di_exception_ce, &exception_message);
			zval_ptr_dtor(&exception_message);
			return;
		}

		PHALCON_RETURN_CALL_METHOD(dependency_injector, "get", &name);
		return;
	}

	/**
	 * If the argument type is 'parameter', we assign the value as it is
	 */
	if (PHALCON_IS_STRING(&type, "parameter")) {
		if (!phalcon_array_isset_fetch_str(&value, argument, SL("value"), PH_READONLY)) {
			PHALCON_CONCAT_SV(&exception_message, "Service 'value' is required in parameter on position ", position);
			PHALCON_THROW_EXCEPTION_ZVAL(phalcon_di_exception_ce, &exception_message);
			zval_ptr_dtor(&exception_message);
			return;
		}

		RETURN_CTOR(&value);
	}

	/**
	 * If the argument type is 'instance', we assign the value as it is
	 */
	if (PHALCON_IS_STRING(&type, "instance")) {
		if (!phalcon_array_isset_fetch_str(&name, argument, SL("className"), PH_READONLY)) {
			PHALCON_CONCAT_SV(&exception_message, "Service 'className' is required in parameter on position ", position);
			PHALCON_THROW_EXCEPTION_ZVAL(phalcon_di_exception_ce, &exception_message);
			zval_ptr_dtor(&exception_message);
			return;
		}

		if (!phalcon_array_isset_str(argument, SL("arguments"))) {
			/**
			 * The instance parameter does not have arguments for its constructor
			 */
			PHALCON_CALL_METHOD(&value, dependency_injector, "get", &name);
		} else {
			/**
			 * Build the instance with arguments
			 */
			PHALCON_RETURN_CALL_METHOD(dependency_injector, "get", &name, &instance_arguments);
			return;
		}

		RETURN_CTOR(&value);
	}

	/**
	 * Unknown parameter type
	 */
	PHALCON_CONCAT_SV(&exception_message, "Unknown service type in parameter on position ", position);
	PHALCON_THROW_EXCEPTION_ZVAL(phalcon_di_exception_ce, &exception_message);
	zval_ptr_dtor(&exception_message);
	return;
}

/**
 * Resolves an array of parameters
 *
 * @param Phalcon\DiInterface $dependencyInjector
 * @param array $arguments
 * @return array
 */
PHP_METHOD(Phalcon_Di_Service_Builder, _buildParameters){

	zval *dependency_injector, *arguments, build_arguments = {}, *argument;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 2, 0, &dependency_injector, &arguments);

	/**
	 * The arguments group must be an array of arrays
	 */
	if (Z_TYPE_P(arguments) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_di_exception_ce, "Definition arguments must be an array");
		return;
	}

	array_init(&build_arguments);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(arguments), idx, str_key, argument) {
		zval position = {}, value = {};
		if (str_key) {
			ZVAL_STR(&position, str_key);
		} else {
			ZVAL_LONG(&position, idx);
		}

		PHALCON_CALL_METHOD(&value, getThis(), "_buildparameter", dependency_injector, &position, argument);
		phalcon_array_append(&build_arguments, &value, PH_COPY);
	} ZEND_HASH_FOREACH_END();

	RETURN_CTOR(&build_arguments);
}

/**
 * Builds a service using a complex service definition
 *
 * @param Phalcon\DiInterface $dependencyInjector
 * @param array $definition
 * @param array $parameters
 * @return mixed
 */
PHP_METHOD(Phalcon_Di_Service_Builder, build){

	zval *dependency_injector, *definition, *parameters = NULL, class_name = {}, instance = {}, arguments = {}, build_arguments = {};
	zval param_calls = {}, *method, exception_message = {}, *property;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 2, 1, &dependency_injector, &definition, &parameters);

	if (!parameters) {
		parameters = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(definition) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_di_exception_ce, "The service definition must be an array");
		return;
	}

	/**
	 * The class name is required
	 */
	if (!phalcon_array_isset_str(definition, SL("className"))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_di_exception_ce, "Invalid service definition. Missing 'className' parameter");
		return;
	}

	phalcon_array_fetch_str(&class_name, definition, SL("className"), PH_NOISY|PH_READONLY);
	if (Z_TYPE_P(parameters) == IS_ARRAY) {
		/**
		 * Build the instance overriding the definition constructor parameters
		 */
		if (phalcon_create_instance_params(&instance, &class_name, parameters) == FAILURE) {
			RETURN_FALSE;
		}
	} else {
		/**
		 * Check if the argument has constructor arguments
		 */
		if (phalcon_array_isset_fetch_str(&arguments, definition, SL("arguments"), PH_READONLY)) {
			/**
			 * Resolve the constructor parameters
			 */
			PHALCON_CALL_METHOD(&build_arguments, getThis(), "_buildparameters", dependency_injector, &arguments);

			/**
			 * Create the instance based on the parameters
			 */
			if (phalcon_create_instance_params(&instance, &class_name, &build_arguments) == FAILURE) {
				RETURN_FALSE;
			}
		} else {
			if (phalcon_create_instance(&instance, &class_name) == FAILURE) {
				RETURN_FALSE;
			}
		}
	}

	/**
	 * The definition has calls?
	 */
	if (phalcon_array_isset_fetch_str(&param_calls, definition, SL("calls"), PH_READONLY)) {
		if (Z_TYPE(instance) != IS_OBJECT) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_di_exception_ce, "The definition has setter injection parameters but the constructor didn't return an instance");
			return;
		}

		if (Z_TYPE(param_calls) != IS_ARRAY) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_di_exception_ce, "Setter injection parameters must be an array");
			return;
		}

		/**
		 * The method call has parameters
		 */
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(param_calls), idx, str_key, method) {
			zval method_position = {}, method_name = {}, method_call = {}, method_args = {}, build_arguments = {}, status = {};
			if (str_key) {
				ZVAL_STR(&method_position, str_key);
			} else {
				ZVAL_LONG(&method_position, idx);
			}

			/**
			 * The call parameter must be an array of arrays
			 */
			if (Z_TYPE_P(method) != IS_ARRAY) {
				PHALCON_CONCAT_SV(&exception_message, "Method call must be an array on position ", &method_position);
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_di_exception_ce, &exception_message);
				zval_ptr_dtor(&exception_message);
				return;
			}

			/**
			 * A param 'method' is required
			 */
			if (!phalcon_array_isset_str(method, SL("method"))) {
				PHALCON_CONCAT_SV(&exception_message, "The method name is required on position ", &method_position);
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_di_exception_ce, &exception_message);
				zval_ptr_dtor(&exception_message);
				return;
			}

			phalcon_array_fetch_str(&method_name, method, SL("method"), PH_NOISY|PH_READONLY);

			/**
			 * Create the method call
			 */
			array_init_size(&method_call, 2);
			phalcon_array_append(&method_call, &instance, PH_COPY);
			phalcon_array_append(&method_call, &method_name, PH_COPY);
			if (phalcon_array_isset_fetch_str(&method_args, method, SL("arguments"), PH_READONLY)) {
				if (Z_TYPE(method_args) != IS_ARRAY) {
					PHALCON_CONCAT_SV(&exception_message, "Call arguments must be an array ", &method_position);
					PHALCON_THROW_EXCEPTION_ZVAL(phalcon_di_exception_ce, &exception_message);
					zval_ptr_dtor(&exception_message);
					zval_ptr_dtor(&method_call);
					return;
				}

				if (phalcon_fast_count_ev(&method_args)) {
					/**
					 * Resolve the constructor parameters
					 */
					PHALCON_CALL_METHOD(&build_arguments, getThis(), "_buildparameters", dependency_injector, &method_args);

					/**
					 * Call the method on the instance
					 */
					PHALCON_CALL_USER_FUNC_ARRAY(&status, &method_call, &build_arguments);
					zval_ptr_dtor(&build_arguments);
					zval_ptr_dtor(&method_call);
					continue;
				}
			}

			/**
			 * Call the method on the instance without arguments
			 */
			PHALCON_CALL_USER_FUNC(&status, &method_call);
			zval_ptr_dtor(&method_call);

		} ZEND_HASH_FOREACH_END();

	}

	/**
	 * The definition has properties?
	 */
	if (phalcon_array_isset_fetch_str(&param_calls, definition, SL("properties"), PH_READONLY)) {
		if (Z_TYPE(instance) != IS_OBJECT) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_di_exception_ce, "The definition has properties injection parameters but the constructor didn't return an instance");
			return;
		}

		if (Z_TYPE(param_calls) != IS_ARRAY) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_di_exception_ce, "Setter injection parameters must be an array");
			return;
		}

		/**
		 * The method call has parameters
		 */
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(param_calls), idx, str_key, property) {
			zval property_position = {}, property_name = {}, property_value = {}, value = {};
			if (str_key) {
				ZVAL_STR(&property_position, str_key);
			} else {
				ZVAL_LONG(&property_position, idx);
			}

			/**
			 * The call parameter must be an array of arrays
			 */
			if (Z_TYPE_P(property) != IS_ARRAY) {
				PHALCON_CONCAT_SV(&exception_message, "Property must be an array on position ", &property_position);
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_di_exception_ce, &exception_message);
				zval_ptr_dtor(&exception_message);
				return;
			}

			/**
			 * A param 'name' is required
			 */
			if (!phalcon_array_isset_str(property, SL("name"))) {
				PHALCON_CONCAT_SV(&exception_message, "The property name is required on position ", &property_position);
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_di_exception_ce, &exception_message);
				zval_ptr_dtor(&exception_message);
				return;
			}

			/**
			 * A param 'value' is required
			 */
			if (!phalcon_array_isset_str(property, SL("value"))) {
				PHALCON_CONCAT_SV(&exception_message, "The property value is required on position ", &property_position);
				PHALCON_THROW_EXCEPTION_ZVAL(phalcon_di_exception_ce, &exception_message);
				zval_ptr_dtor(&exception_message);
				return;
			}

			phalcon_array_fetch_str(&property_name, property, SL("name"), PH_NOISY|PH_READONLY);
			phalcon_array_fetch_str(&property_value, property, SL("value"), PH_NOISY|PH_READONLY);

			/**
			 * Resolve the parameter
			 */
			PHALCON_CALL_METHOD(&value, getThis(), "_buildparameter", dependency_injector, &property_position, &property_value);

			/**
			 * Update the public property
			 */
			phalcon_update_property_zval_zval(&instance, &property_name, &value);
			zval_ptr_dtor(&value);
		} ZEND_HASH_FOREACH_END();
	}

	RETURN_CTOR(&instance);
}
