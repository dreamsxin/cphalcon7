#ifndef PHALCON_KERNEL_REFLECTION_H
#define PHALCON_KERNEL_REFLECTION_H

#include "php_phalcon.h"

PHALCON_ATTR_NONNULL static inline const char* phalcon_get_class_doc_comment(const zend_class_entry *ce, const char **comment, uint32_t *len)
{
	if (ce->type == ZEND_USER_CLASS) {
		*comment = ce->info.user.doc_comment;
		*len     = ce->info.user.doc_comment_len;
		return ce->info.user.doc_comment;
	}

	*comment = NULL;
	*len     = 0;
	return NULL;
}

PHALCON_ATTR_NONNULL static inline const char* phalcon_get_class_filename(const zend_class_entry *ce)
{
	if (ce->type == ZEND_USER_CLASS) {
#if PHP_VERSION_ID >= 50400
		return ce->info.user.filename;
#else
		return ce->filename;
#endif
	}

	return NULL;
}

PHALCON_ATTR_NONNULL static inline zend_uint phalcon_get_class_startline(const zend_class_entry *ce)
{
	if (ce->type == ZEND_USER_CLASS) {
		return ce->info.user.line_start;
	}

	return 0;
}

static inline const char* phalcon_get_property_doc_comment(const zend_property_info *prop, const char **comment, uint32_t *len)
{
	*comment = prop->doc_comment;
	*len     = prop->doc_comment_len;
	return prop->doc_comment;
}

static inline const char* phalcon_get_function_doc_comment(const zend_function *func, const char **comment, uint32_t *len)
{
	if (func->type == ZEND_USER_FUNCTION) {
		*comment = func->op_array.doc_comment;
		*len     = func->op_array.doc_comment_len;
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
