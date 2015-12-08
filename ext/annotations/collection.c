
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

#include "annotations/collection.h"
#include "annotations/annotation.h"
#include "annotations/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/object.h"
#include "kernel/concat.h"
#include "kernel/operators.h"

/**
 * Phalcon\Annotations\Collection
 *
 * Represents a collection of annotations. This class allows to traverse a group of annotations easily
 *
 *<code>
 * //Traverse annotations
 * foreach ($classAnnotations as $annotation) {
 *     echo 'Name=', $annotation->getName(), PHP_EOL;
 * }
 *
 * //Check if the annotations has a specific
 * var_dump($classAnnotations->has('Cacheable'));
 *
 * //Get an specific annotation in the collection
 * $annotation = $classAnnotations->get('Cacheable');
 *</code>
 */
zend_class_entry *phalcon_annotations_collection_ce;

PHP_METHOD(Phalcon_Annotations_Collection, __construct);
PHP_METHOD(Phalcon_Annotations_Collection, count);
PHP_METHOD(Phalcon_Annotations_Collection, rewind);
PHP_METHOD(Phalcon_Annotations_Collection, current);
PHP_METHOD(Phalcon_Annotations_Collection, key);
PHP_METHOD(Phalcon_Annotations_Collection, next);
PHP_METHOD(Phalcon_Annotations_Collection, valid);
PHP_METHOD(Phalcon_Annotations_Collection, getAnnotations);
PHP_METHOD(Phalcon_Annotations_Collection, get);
PHP_METHOD(Phalcon_Annotations_Collection, getAll);
PHP_METHOD(Phalcon_Annotations_Collection, has);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_annotations_collection___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, reflectionData)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_annotations_collection_get, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_annotations_collection_getall, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_annotations_collection_has, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_annotations_collection_method_entry[] = {
	PHP_ME(Phalcon_Annotations_Collection, __construct, arginfo_phalcon_annotations_collection___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Annotations_Collection, count, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Annotations_Collection, rewind, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Annotations_Collection, current, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Annotations_Collection, key, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Annotations_Collection, next, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Annotations_Collection, valid, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Annotations_Collection, getAnnotations, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Annotations_Collection, get, arginfo_phalcon_annotations_collection_get, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Annotations_Collection, getAll, arginfo_phalcon_annotations_collection_getall, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Annotations_Collection, has, arginfo_phalcon_annotations_collection_has, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


/**
 * Phalcon\Annotations\Collection initializer
 */
PHALCON_INIT_CLASS(Phalcon_Annotations_Collection){

	PHALCON_REGISTER_CLASS(Phalcon\\Annotations, Collection, annotations_collection, phalcon_annotations_collection_method_entry, 0);

	zend_declare_property_long(phalcon_annotations_collection_ce, SL("_position"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_annotations_collection_ce, SL("_annotations"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_annotations_collection_ce, 2, zend_ce_iterator, spl_ce_Countable);

	return SUCCESS;
}

/**
 * Phalcon\Annotations\Collection constructor
 *
 * @param array $reflectionData
 */
PHP_METHOD(Phalcon_Annotations_Collection, __construct){

	zval *reflection_data = NULL, *annotations, *annotation_data;
	zval *annotation = NULL;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 0, 1, &reflection_data);

	if (reflection_data && Z_TYPE_P(reflection_data) != IS_NULL) {
		if (Z_TYPE_P(reflection_data) != IS_ARRAY) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_annotations_exception_ce, "Reflection data must be an array");
			return;
		}

		PHALCON_INIT_VAR(annotations);
		array_init(annotations);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(reflection_data), annotation_data) {
			PHALCON_INIT_NVAR(annotation);
			object_init_ex(annotation, phalcon_annotations_annotation_ce);
			PHALCON_CALL_METHOD(NULL, annotation, "__construct", annotation_data);
			phalcon_array_append(annotations, annotation, PH_COPY);
		} ZEND_HASH_FOREACH_END();

		phalcon_update_property_this(getThis(), SL("_annotations"), annotations);
	}

	PHALCON_MM_RESTORE();
}

/**
 * Returns the number of annotations in the collection
 *
 * @return int
 */
PHP_METHOD(Phalcon_Annotations_Collection, count){

	zval *annotations;

	annotations = phalcon_read_property(getThis(), SL("_annotations"), PH_NOISY);
	if (Z_TYPE_P(annotations) == IS_ARRAY) {
		RETURN_LONG(zend_hash_num_elements(Z_ARRVAL_P(annotations)));
	}

	RETURN_LONG(0);
}

/**
 * Rewinds the internal iterator
 */
PHP_METHOD(Phalcon_Annotations_Collection, rewind){


	phalcon_update_property_long(getThis(), SL("_position"), 0);

}

/**
 * Returns the current annotation in the iterator
 *
 * @return Phalcon\Annotations\Annotation
 */
PHP_METHOD(Phalcon_Annotations_Collection, current){

	zval *position, *annotations, *annotation;

	position    = phalcon_read_property(getThis(), SL("_position"), PH_NOISY);
	annotations = phalcon_read_property(getThis(), SL("_annotations"), PH_NOISY);
	if (phalcon_array_isset_fetch(&annotation, annotations, position)) {
		RETURN_ZVAL(annotation, 1, 0);
	}

	RETURN_NULL();
}

/**
 * Returns the current position/key in the iterator
 *
 * @return int
 */
PHP_METHOD(Phalcon_Annotations_Collection, key){


	RETURN_MEMBER(getThis(), "_position");
}

/**
 * Moves the internal iteration pointer to the next position
 *
 */
PHP_METHOD(Phalcon_Annotations_Collection, next){


	phalcon_property_incr(getThis(), SL("_position"));

}

/**
 * Check if the current annotation in the iterator is valid
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Annotations_Collection, valid){

	zval *position, *annotations;

	PHALCON_MM_GROW();

	position = phalcon_read_property(getThis(), SL("_position"), PH_NOISY);

	annotations = phalcon_read_property(getThis(), SL("_annotations"), PH_NOISY);
	if (phalcon_array_isset(annotations, position)) {
		RETURN_MM_TRUE;
	}

	RETURN_MM_FALSE;
}

/**
 * Returns the internal annotations as an array
 *
 * @return Phalcon\Annotations\Annotation[]
 */
PHP_METHOD(Phalcon_Annotations_Collection, getAnnotations){


	RETURN_MEMBER(getThis(), "_annotations");
}

/**
 * Returns the first annotation that match a name
 *
 * @param string $name
 * @return Phalcon\Annotations\Annotation
 */
PHP_METHOD(Phalcon_Annotations_Collection, get){

	zval *name, *annotations, *annotation = NULL, *annotation_name = NULL;
	zval exception_message;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 0, &name);

	annotations = phalcon_read_property(getThis(), SL("_annotations"), PH_NOISY);
	if (Z_TYPE_P(annotations) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(annotations), annotation) {
			PHALCON_CALL_METHOD(&annotation_name, annotation, "getname");
			if (PHALCON_IS_EQUAL(name, annotation_name)) {
				RETURN_CCTOR(annotation);
			}
		} ZEND_HASH_FOREACH_END();

	}

	PHALCON_CONCAT_SVS(&exception_message, "The collection doesn't have an annotation called '", name, "'");
	PHALCON_THROW_EXCEPTION_ZVAL(phalcon_annotations_exception_ce, &exception_message);
	return;
}

/**
 * Returns all the annotations that match a name
 *
 * @param string $name
 * @return Phalcon\Annotations\Annotation[]
 */
PHP_METHOD(Phalcon_Annotations_Collection, getAll){

	zval *name, *found, *annotations, *annotation = NULL, *annotation_name = NULL;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 0, &name);

	PHALCON_INIT_VAR(found);
	array_init(found);

	annotations = phalcon_read_property(getThis(), SL("_annotations"), PH_NOISY);
	if (Z_TYPE_P(annotations) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(annotations), annotation) {
			PHALCON_CALL_METHOD(&annotation_name, annotation, "getname");
			if (PHALCON_IS_EQUAL(name, annotation_name)) {
				phalcon_array_append(found, annotation, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();
	}

	RETURN_CTOR(found);
}

/**
 * Check if an annotation exists in a collection
 *
 * @param string $name
 * @return boolean
 */
PHP_METHOD(Phalcon_Annotations_Collection, has){

	zval *name, *annotations, *annotation = NULL, *annotation_name = NULL;

	PHALCON_MM_GROW();

	phalcon_fetch_params(1, 1, 0, &name);

	annotations = phalcon_read_property(getThis(), SL("_annotations"), PH_NOISY);
	if (Z_TYPE_P(annotations) == IS_ARRAY) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(annotations), annotation) {
			PHALCON_CALL_METHOD(&annotation_name, annotation, "getname");
			if (PHALCON_IS_EQUAL(name, annotation_name)) {
				RETURN_MM_TRUE;
			}
		} ZEND_HASH_FOREACH_END();
	}

	RETURN_MM_FALSE;
}

