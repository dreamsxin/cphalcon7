
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

#include "annotations/annotation.h"
#include "annotations/exception.h"
#include "annotations/scanner.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/operators.h"
#include "kernel/concat.h"


/**
 * Phalcon\Annotations\Annotation
 *
 * Represents a single annotation in an annotations collection
 */
zend_class_entry *phalcon_annotations_annotation_ce;

PHP_METHOD(Phalcon_Annotations_Annotation, __construct);
PHP_METHOD(Phalcon_Annotations_Annotation, getName);
PHP_METHOD(Phalcon_Annotations_Annotation, getExpression);
PHP_METHOD(Phalcon_Annotations_Annotation, getExprArguments);
PHP_METHOD(Phalcon_Annotations_Annotation, getArguments);
PHP_METHOD(Phalcon_Annotations_Annotation, numberArguments);
PHP_METHOD(Phalcon_Annotations_Annotation, getArgument);
PHP_METHOD(Phalcon_Annotations_Annotation, hasArgument);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_annotations_annotation___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, reflectionData, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_annotations_annotation_getexpression, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, expr, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_annotations_annotation_getargument, 0, 0, 1)
	ZEND_ARG_INFO(0, position)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_annotations_annotation_hasargument, 0, 0, 1)
	ZEND_ARG_INFO(0, position)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_annotations_annotation_method_entry[] = {
	PHP_ME(Phalcon_Annotations_Annotation, __construct, arginfo_phalcon_annotations_annotation___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Annotations_Annotation, getName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Annotations_Annotation, getExpression, arginfo_phalcon_annotations_annotation_getexpression, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Annotations_Annotation, getExprArguments, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Annotations_Annotation, getArguments, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Annotations_Annotation, numberArguments, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Annotations_Annotation, getArgument, arginfo_phalcon_annotations_annotation_getargument, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Annotations_Annotation, hasArgument, arginfo_phalcon_annotations_annotation_hasargument, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Annotations_Annotation, getNamedArgument, getArgument, arginfo_phalcon_annotations_annotation_getargument, ZEND_ACC_PUBLIC)
	PHP_MALIAS(Phalcon_Annotations_Annotation, getNamedParameter, getArgument, arginfo_phalcon_annotations_annotation_getargument, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
	PHP_MALIAS(Phalcon_Annotations_Annotation, hasNamedArgument, hasArgument, arginfo_phalcon_annotations_annotation_hasargument, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Annotations\Annotation initializer
 */
PHALCON_INIT_CLASS(Phalcon_Annotations_Annotation){

	PHALCON_REGISTER_CLASS(Phalcon\\Annotations, Annotation, annotations_annotation, phalcon_annotations_annotation_method_entry, 0);

	zend_declare_property_null(phalcon_annotations_annotation_ce, SL("_name"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_annotations_annotation_ce, SL("_arguments"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_annotations_annotation_ce, SL("_exprArguments"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Phalcon\Annotations\Annotation constructor
 *
 * @param array $reflectionData
 */
PHP_METHOD(Phalcon_Annotations_Annotation, __construct){

	zval *reflection_data, name = {}, arguments = {}, expr_arguments = {}, *argument;

	phalcon_fetch_params(0, 1, 0, &reflection_data);

	phalcon_array_fetch_str(&name, reflection_data, SL("name"), PH_NOISY|PH_READONLY);
	phalcon_update_property(getThis(), SL("_name"), &name);

	/**
	 * Process annotation arguments
	 */
	if (phalcon_array_isset_fetch_str(&expr_arguments, reflection_data, SL("arguments"), PH_READONLY)) {
		array_init(&arguments);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(expr_arguments), argument) {
			zval expr = {}, resolved_argument = {}, name = {};

			phalcon_array_fetch_str(&expr, argument, SL("expr"), PH_NOISY|PH_READONLY);

			PHALCON_CALL_METHOD(&resolved_argument, getThis(), "getexpression", &expr);
			if (phalcon_array_isset_fetch_str(&name, argument, SL("name"), PH_READONLY)) {
				phalcon_array_update(&arguments, &name, &resolved_argument, 0);
			} else {
				phalcon_array_append(&arguments, &resolved_argument, 0);
			}
		} ZEND_HASH_FOREACH_END();

		phalcon_update_property(getThis(), SL("_arguments"), &arguments);
		phalcon_update_property(getThis(), SL("_exprArguments"), &expr_arguments);
		zval_ptr_dtor(&arguments);
	}
}

/**
 * Returns the annotation's name
 *
 * @return string
 */
PHP_METHOD(Phalcon_Annotations_Annotation, getName){


	RETURN_MEMBER(getThis(), "_name");
}

/**
 * Resolves an annotation expression
 *
 * @param array $expr
 * @return mixed
 */
PHP_METHOD(Phalcon_Annotations_Annotation, getExpression){

	zval *expr, type = {}, items = {}, *item, exception_message = {};

	phalcon_fetch_params(0, 1, 0, &expr);

	phalcon_array_fetch_str(&type, expr, SL("type"), PH_NOISY|PH_READONLY);

	switch (phalcon_get_intval(&type)) {
		case PHANNOT_T_INTEGER:
		case PHANNOT_T_DOUBLE:
		case PHANNOT_T_STRING:
		case PHANNOT_T_IDENTIFIER:
			phalcon_array_fetch_str(&items, expr, SL("value"), PH_NOISY|PH_READONLY);
			RETURN_CTOR(&items);
			/* no break because of implicit return */

		case PHANNOT_T_NULL:
			RETURN_NULL();
			/* no break because of implicit return */

		case PHANNOT_T_FALSE:
			RETURN_FALSE;
			/* no break because of implicit return */

		case PHANNOT_T_TRUE:
			RETURN_TRUE;
			/* no break because of implicit return */

		case PHANNOT_T_ARRAY:
			phalcon_array_fetch_str(&items, expr, SL("items"), PH_NOISY|PH_READONLY);

			array_init(return_value);

			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(items), item) {
				zval name = {}, item_expr = {}, resolved_item = {};

				phalcon_array_fetch_str(&item_expr, item, SL("expr"), PH_NOISY|PH_READONLY);

				PHALCON_CALL_METHOD(&resolved_item, getThis(), "getexpression", &item_expr);
				if (phalcon_array_isset_fetch_str(&name, item, SL("name"), PH_READONLY)) {
					phalcon_array_update(return_value, &name, &resolved_item, 0);
				} else {
					phalcon_array_append(return_value, &resolved_item, 0);
				}
			} ZEND_HASH_FOREACH_END();

			return;
			/* no break because of implicit return */

		case PHANNOT_T_ANNOTATION:
			object_init_ex(return_value, phalcon_annotations_annotation_ce);
			PHALCON_CALL_METHOD(NULL, return_value, "__construct", expr);
			return;
			/* no break because of implicit return */

		default:
			PHALCON_CONCAT_SVS(&exception_message, "The expression ", &type, " is unknown");
			PHALCON_THROW_EXCEPTION_ZVAL(phalcon_annotations_exception_ce, &exception_message);
			return;
	}
}

/**
 * Returns the expression arguments without resolving
 *
 * @return array
 */
PHP_METHOD(Phalcon_Annotations_Annotation, getExprArguments){


	RETURN_MEMBER(getThis(), "_exprArguments");
}

/**
 * Returns the expression arguments
 *
 * @return array
 */
PHP_METHOD(Phalcon_Annotations_Annotation, getArguments){


	RETURN_MEMBER(getThis(), "_arguments");
}

/**
 * Returns the number of arguments that the annotation has
 *
 * @return int
 */
PHP_METHOD(Phalcon_Annotations_Annotation, numberArguments){

	zval arguments = {};

	phalcon_read_property(&arguments, getThis(), SL("_arguments"), PH_NOISY|PH_READONLY);
	phalcon_fast_count(return_value, &arguments);
}

/**
 * Returns an argument in a specific position
 *
 * @return mixed
 */
PHP_METHOD(Phalcon_Annotations_Annotation, getArgument){

	zval *position, arguments = {};

	phalcon_fetch_params(0, 1, 0, &position);

	phalcon_read_property(&arguments, getThis(), SL("_arguments"), PH_NOISY|PH_READONLY);
	if (!phalcon_array_isset_fetch(return_value, &arguments, position, PH_COPY)) {
		RETURN_NULL();
	}
}

/**
 * Checks if the annotation has a specific argument
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Annotations_Annotation, hasArgument){

	zval *position, arguments = {};

	phalcon_fetch_params(0, 1, 0, &position);

	phalcon_read_property(&arguments, getThis(), SL("_arguments"), PH_NOISY|PH_READONLY);
	RETURN_BOOL(phalcon_array_isset(&arguments, position));
}

/**
 * Returns a named argument
 *
 * @param string $name
 * @return mixed
 */
PHALCON_DOC_METHOD(Phalcon_Annotations_Annotation, getNamedArgument);

/**
 * Returns a named argument (deprecated)
 *
 * @deprecated
 * @param string $name
 * @return mixed
 */
PHALCON_DOC_METHOD(Phalcon_Annotations_Annotation, getNamedParameter);

/**
 * Checks if the annotation has a specific named argument
 *
 * @return boolean
 */
PHALCON_DOC_METHOD(Phalcon_Annotations_Annotation, hasNamedArgument);
