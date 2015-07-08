#ifndef PHALCON_KERNEL_REFLECTION_H
#define PHALCON_KERNEL_REFLECTION_H

#include "php_phalcon.h"

PHALCON_ATTR_NONNULL static inline zend_string* phalcon_get_class_doc_comment(const zend_class_entry *ce)
{
	if (ce->type == ZEND_USER_CLASS) {
		return ce->info.user.doc_comment;
	}

	return NULL;
}

PHALCON_ATTR_NONNULL static inline zend_string* phalcon_get_class_filename(const zend_class_entry *ce)
{
	if (ce->type == ZEND_USER_CLASS) {
		return ce->info.user.filename;
	}

	return NULL;
}

PHALCON_ATTR_NONNULL static inline uint32_t phalcon_get_class_startline(const zend_class_entry *ce)
{
	if (ce->type == ZEND_USER_CLASS) {
		return ce->info.user.line_start;
	}

	return 0;
}

static inline zend_string* phalcon_get_property_doc_comment(const zend_property_info *prop)
{
	return prop->doc_comment;
}

static inline zend_string* phalcon_get_function_doc_comment(const zend_function *func)
{
	if (func->type == ZEND_USER_FUNCTION) {
		return func->op_array.doc_comment;
	}

	return NULL;
}

static inline uint32_t phalcon_get_function_startline(const zend_function *func)
{
	if (func->type == ZEND_USER_FUNCTION) {
		return func->op_array.line_start;
	}

	return 0;
}

#endif /* PHALCON_KERNEL_REFLECTION_H */
