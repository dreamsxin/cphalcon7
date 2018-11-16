
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
  |          Julien Salleyron <julien.salleyron@gmail.com>                 |
  |          <pangudashu@gmail.com>                                        |
  +------------------------------------------------------------------------+
*/

#include "aop.h"
#include "aop/lexer.h"
#include "aop/joinpoint.h"
#include "aop/exception.h"

#include <ext/standard/php_string.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/exception.h"

extern ZEND_API void (*original_zend_execute_internal)(zend_execute_data *execute_data, zval *return_value);
extern ZEND_API void (*original_zend_execute_ex)(zend_execute_data *execute_data);

zend_object_read_property_t    original_zend_std_read_property;
zend_object_write_property_t   original_zend_std_write_property;
zend_object_get_property_ptr_ptr_t	original_zend_std_get_property_ptr_ptr;

/**
 * Phalcon\Aop
 *
 * AOP aims to allow separation of cross-cutting concerns (cache, log, security, transactions, ...)
 */
zend_class_entry *phalcon_aop_ce;

PHP_METHOD(Phalcon_Aop, addBefore);
PHP_METHOD(Phalcon_Aop, addAfter);
PHP_METHOD(Phalcon_Aop, addAfterReturning);
PHP_METHOD(Phalcon_Aop, addAfterThrowing);
PHP_METHOD(Phalcon_Aop, addAround);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_aop_addbefore, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, pointcut, IS_STRING, 0)
	ZEND_ARG_CALLABLE_INFO(0, advice, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_aop_addafter, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, pointcut, IS_STRING, 0)
	ZEND_ARG_CALLABLE_INFO(0, advice, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_aop_addafterreturning, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, pointcut, IS_STRING, 0)
	ZEND_ARG_CALLABLE_INFO(0, advice, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_aop_addafterthrowing, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, pointcut, IS_STRING, 0)
	ZEND_ARG_CALLABLE_INFO(0, advice, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_aop_addaround, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, pointcut, IS_STRING, 0)
	ZEND_ARG_CALLABLE_INFO(0, advice, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_aop_method_entry[] = {
	PHP_ME(Phalcon_Aop, addBefore, arginfo_phalcon_aop_addbefore, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Aop, addAfter, arginfo_phalcon_aop_addafter, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Aop, addAfterReturning, arginfo_phalcon_aop_addafterreturning, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Aop, addAfterThrowing, arginfo_phalcon_aop_addafterthrowing, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Aop, addAround, arginfo_phalcon_aop_addaround, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Aop initializer
 */
PHALCON_INIT_CLASS(Phalcon_Aop){

	PHALCON_REGISTER_CLASS(Phalcon, Aop, aop, phalcon_aop_method_entry, 0);

	zend_declare_class_constant_long(phalcon_aop_ce, SL("KIND_BEFORE"),			PHALCON_AOP_KIND_BEFORE);
	zend_declare_class_constant_long(phalcon_aop_ce, SL("KIND_AFTER"),			PHALCON_AOP_KIND_AFTER);
	zend_declare_class_constant_long(phalcon_aop_ce, SL("KIND_AROUND"),			PHALCON_AOP_KIND_AROUND);
	zend_declare_class_constant_long(phalcon_aop_ce, SL("KIND_PROPERTY"),		PHALCON_AOP_KIND_PROPERTY);
	zend_declare_class_constant_long(phalcon_aop_ce, SL("KIND_FUNCTION"),		PHALCON_AOP_KIND_FUNCTION);
	zend_declare_class_constant_long(phalcon_aop_ce, SL("KIND_METHOD"),			PHALCON_AOP_KIND_METHOD);
	zend_declare_class_constant_long(phalcon_aop_ce, SL("KIND_READ"),			PHALCON_AOP_KIND_READ);
	zend_declare_class_constant_long(phalcon_aop_ce, SL("KIND_WRITE"),			PHALCON_AOP_KIND_WRITE);
	zend_declare_class_constant_long(phalcon_aop_ce, SL("KIND_AROUND_WRITE_PROPERTY"),		PHALCON_AOP_KIND_AROUND_WRITE_PROPERTY);
	zend_declare_class_constant_long(phalcon_aop_ce, SL("KIND_AROUND_READ_PROPERTY"),		PHALCON_AOP_KIND_AROUND_READ_PROPERTY);
	zend_declare_class_constant_long(phalcon_aop_ce, SL("KIND_BEFORE_WRITE_PROPERTY"),		PHALCON_AOP_KIND_BEFORE_WRITE_PROPERTY);
	zend_declare_class_constant_long(phalcon_aop_ce, SL("KIND_BEFORE_READ_PROPERTY"),		PHALCON_AOP_KIND_BEFORE_READ_PROPERTY);
	zend_declare_class_constant_long(phalcon_aop_ce, SL("KIND_AFTER_WRITE_PROPERTY"),		PHALCON_AOP_KIND_AFTER_WRITE_PROPERTY);
	zend_declare_class_constant_long(phalcon_aop_ce, SL("KIND_AFTER_READ_PROPERTY"),		PHALCON_AOP_KIND_AFTER_READ_PROPERTY);
	zend_declare_class_constant_long(phalcon_aop_ce, SL("KIND_BEFORE_METHOD"),		PHALCON_AOP_KIND_BEFORE_METHOD);
	zend_declare_class_constant_long(phalcon_aop_ce, SL("KIND_AFTER_METHOD"),		PHALCON_AOP_KIND_AFTER_METHOD);
	zend_declare_class_constant_long(phalcon_aop_ce, SL("KIND_AROUND_METHOD"),		PHALCON_AOP_KIND_AROUND_METHOD);
	zend_declare_class_constant_long(phalcon_aop_ce, SL("KIND_BEFORE_FUNCTION"),	PHALCON_AOP_KIND_BEFORE_FUNCTION);
	zend_declare_class_constant_long(phalcon_aop_ce, SL("KIND_AFTER_FUNCTION"),		PHALCON_AOP_KIND_AFTER_FUNCTION);
	zend_declare_class_constant_long(phalcon_aop_ce, SL("KIND_AROUND_FUNCTION"),		PHALCON_AOP_KIND_AROUND_FUNCTION);

	return SUCCESS;
}

static int strcmp_with_joker_case(char *str_with_jok, char *str, int case_sensitive) /*{{{*/
{
	if (str_with_jok[0] == '*') {
		if (str_with_jok[1] == '\0') {
			return 1;
		}
	}
	if (str_with_jok[0] == '*') {
		if (case_sensitive) {
			return !strcmp(str_with_jok+1, str+(strlen(str)-(strlen(str_with_jok)-1)));
		} else {
			return !strcasecmp(str_with_jok+1, str+(strlen(str)-(strlen(str_with_jok)-1)));
		}
	}
	if (str_with_jok[strlen(str_with_jok)-1] == '*') {
		if (case_sensitive) {
			return !strncmp(str_with_jok, str, strlen(str_with_jok)-1);
		} else {
			return !strncasecmp(str_with_jok, str, strlen(str_with_jok)-1);
		}
	}
	if (case_sensitive) {
		return !strcmp(str_with_jok, str);
	} else {
		return !strcasecmp(str_with_jok, str);
	}
}
/*}}}*/

static int pointcut_match_zend_class_entry(phalcon_aop_pointcut *pc, zend_class_entry *ce) /*{{{*/
{
	int i, matches;

#if PHP_VERSION_ID >= 70300
	matches = pcre2_match(pc->re_class, (PCRE2_SPTR)ZSTR_VAL(ce->name), ZSTR_LEN(ce->name), 0, 0, NULL, 0);
#else
	matches = pcre_exec(pc->re_class, NULL, ZSTR_VAL(ce->name), ZSTR_LEN(ce->name), 0, 0, NULL, 0);
#endif
	if (matches >= 0) {
		return 1;
	}

	for (i = 0; i < (int) ce->num_interfaces; i++) {
#if PHP_VERSION_ID >= 70300
		matches = pcre2_match(pc->re_class, (PCRE2_SPTR)ZSTR_VAL(ce->interfaces[i]->name), ZSTR_LEN(ce->interfaces[i]->name), 0, 0, NULL, 0);
#else
		matches = pcre_exec(pc->re_class, NULL, ZSTR_VAL(ce->interfaces[i]->name), ZSTR_LEN(ce->interfaces[i]->name), 0, 0, NULL, 0);
#endif
		if (matches >= 0) {
			return 1;
		}
	}

	for (i = 0; i < (int) ce->num_traits; i++) {
#if PHP_VERSION_ID >= 70400
		matches = pcre2_match(pc->re_class, (PCRE2_SPTR)ZSTR_VAL(ce->trait_names[i].name), ZSTR_LEN(ce->trait_names[i].name), 0, 0, NULL, 0);
#elif PHP_VERSION_ID >= 70300
		matches = pcre2_match(pc->re_class, (PCRE2_SPTR)ZSTR_VAL(ce->traits[i]->name), ZSTR_LEN(ce->traits[i]->name), 0, 0, NULL, 0);
#else
		matches = pcre_exec(pc->re_class, NULL, ZSTR_VAL(ce->traits[i]->name), ZSTR_LEN(ce->traits[i]->name), 0, 0, NULL, 0);
#endif
		if (matches>=0) {
			return 1;
		}
	}

	ce = ce->parent;
	while (ce != NULL) {
#if PHP_VERSION_ID >= 70300
		matches = pcre2_match(pc->re_class, (PCRE2_SPTR)ZSTR_VAL(ce->name), ZSTR_LEN(ce->name), 0, 0, NULL, 0);
#else
		matches = pcre_exec(pc->re_class, NULL, ZSTR_VAL(ce->name), ZSTR_LEN(ce->name), 0, 0, NULL, 0);
#endif
		if (matches >= 0) {
			return 1;
		}
		ce = ce->parent;
	}
	return 0;
}
/*}}}*/

static zend_array *calculate_class_pointcuts(zend_class_entry *ce, int kind_of_advice) /*{{{*/
{
	phalcon_aop_pointcut *pc;
	zend_array *ht;
	zval *pc_value;

	ALLOC_HASHTABLE(ht);
	zend_hash_init(ht, 16, NULL, NULL , 0);

	ZEND_HASH_FOREACH_VAL(PHALCON_GLOBAL(aop).pointcuts_table, pc_value) {
		pc = (phalcon_aop_pointcut *)Z_PTR_P(pc_value);
		if (!(pc->kind_of_advice & kind_of_advice)) {
			continue;
		}
		if ((ce == NULL && pc->kind_of_advice & PHALCON_AOP_KIND_FUNCTION)
			|| (ce != NULL && pointcut_match_zend_class_entry(pc, ce))) {
			zend_hash_next_index_insert(ht, pc_value);
		}
	} ZEND_HASH_FOREACH_END();

	return ht;
}
/*}}}*/

static int pointcut_match_zend_function(phalcon_aop_pointcut *pc, zend_execute_data *ex) /*{{{*/
{
	int comp_start = 0;
	zend_function *curr_func = ex->func;

	//check static
	if (pc->static_state != 2) {
		if (pc->static_state) {
			if (!(curr_func->common.fn_flags & ZEND_ACC_STATIC)) {
				return 0;
			}
		} else {
			if ((curr_func->common.fn_flags & ZEND_ACC_STATIC)) {
				return 0;
			}
		}
	}
	//check public/protect/private
	if (pc->scope != 0 && !(pc->scope & (curr_func->common.fn_flags & ZEND_ACC_PPP_MASK))) {
		return 0;
	}

	if (pc->class_name == NULL && ZSTR_VAL(pc->method)[0] == '*' && ZSTR_VAL(pc->method)[1] == '\0') {
		return 1;
	}
	if (pc->class_name == NULL && curr_func->common.scope != NULL) {
		return 0;
	}
	if (pc->method_jok) {
#if PHP_VERSION_ID >= 70300
		int matches = pcre2_match(pc->re_method, (PCRE2_SPTR)ZSTR_VAL(curr_func->common.function_name), ZSTR_LEN(curr_func->common.function_name), 0, 0, NULL, 0);
#else
		int matches = pcre_exec(pc->re_method, NULL, ZSTR_VAL(curr_func->common.function_name), ZSTR_LEN(curr_func->common.function_name), 0, 0, NULL, 0);
#endif
		if (matches < 0) {
			return 0;
		}
	} else {
		if (ZSTR_VAL(pc->method)[0] == '\\') {
			comp_start = 1;
		}
		if (strcasecmp(ZSTR_VAL(pc->method) + comp_start, ZSTR_VAL(curr_func->common.function_name))) {
			return 0;
		}
	}
	return 1;
}
/*}}}*/

static zend_array *calculate_function_pointcuts(zend_execute_data *ex) /*{{{*/
{
	zend_object *object = NULL;
	zend_class_entry *ce = NULL;
	zend_array *class_pointcuts;
	zval *pc_value;
	phalcon_aop_pointcut *pc;
	zend_ulong h;

#if PHP_MINOR_VERSION < 1
	object = Z_OBJ(ex->This);
#else
	if (Z_TYPE(ex->This) == IS_OBJECT) {
		object = Z_OBJ(ex->This);
	}
#endif
	if (object != NULL) {
		ce = Z_OBJCE(ex->This);
	}

	if (ce == NULL && ex->func->common.fn_flags & ZEND_ACC_STATIC) {
		ce = ex->func->common.scope;
	}

	class_pointcuts = calculate_class_pointcuts(ce, PHALCON_AOP_KIND_FUNCTION | PHALCON_AOP_KIND_METHOD);

	ZEND_HASH_FOREACH_NUM_KEY_VAL(class_pointcuts, h, pc_value) {
		pc = (phalcon_aop_pointcut *)Z_PTR_P(pc_value);
		if (pointcut_match_zend_function(pc, ex)) {
			continue;
		}
		//delete unmatch element
		zend_hash_index_del(class_pointcuts, h);
	} ZEND_HASH_FOREACH_END();

	return class_pointcuts;
}
/*}}}*/

static int test_property_scope(phalcon_aop_pointcut *current_pc, zend_class_entry *ce, zend_string *member_str) /*{{{*/
{
	zval *property_info_val;
	zend_property_info *property_info = NULL;

	property_info_val = zend_hash_find(&ce->properties_info, member_str);
	if (property_info_val) {
		property_info = (zend_property_info *)Z_PTR_P(property_info_val);
		if (current_pc->static_state != 2) {
			if (current_pc->static_state) {
				if (!(property_info->flags & ZEND_ACC_STATIC)) {
					return 0;
				}
			} else {
				if ((property_info->flags & ZEND_ACC_STATIC)) {
					return 0;
				}
			}
		}
		if (current_pc->scope != 0 && !(current_pc->scope & (property_info->flags & ZEND_ACC_PPP_MASK))) {
			return 0;
		}
	} else {
		if (current_pc->scope != 0 && !(current_pc->scope & ZEND_ACC_PUBLIC)) {
			return 0;
		}
		if (current_pc->static_state == 1) {
			return 0;
		}
	}
	return 1;
}
/*}}}*/

static zend_array *calculate_property_pointcuts(zval *object, zend_string *member_str, int kind) /*{{{*/
{
	zend_array *class_pointcuts;
	zval *pc_value;
	phalcon_aop_pointcut *pc;
	zend_ulong h;

	class_pointcuts = calculate_class_pointcuts(Z_OBJCE_P(object), kind);
   
	ZEND_HASH_FOREACH_NUM_KEY_VAL(class_pointcuts, h, pc_value) {
		pc = (phalcon_aop_pointcut *)Z_PTR_P(pc_value);
		if (ZSTR_VAL(pc->method)[0] != '*') {
			if (!strcmp_with_joker_case(ZSTR_VAL(pc->method), ZSTR_VAL(member_str), 1)) {
				zend_hash_index_del(class_pointcuts, h);
				continue;
			}
		}
		//Scope
		if (pc->static_state != 2 || pc->scope != 0) {
			if (!test_property_scope(pc, Z_OBJCE_P(object), member_str)) {
				zend_hash_index_del(class_pointcuts, h);
				continue;
			}
		}
	} ZEND_HASH_FOREACH_END();

	return class_pointcuts;
}
/*}}}*/

phalcon_aop_object_cache *get_object_cache (zend_object *object) /*{{{*/
{
	int i;
	uint32_t handle;

	handle = object->handle;
	if (handle >= PHALCON_GLOBAL(aop).object_cache_size) {
		PHALCON_GLOBAL(aop).object_cache = erealloc(PHALCON_GLOBAL(aop).object_cache, sizeof(phalcon_aop_object_cache)*handle + 1);
		for (i = PHALCON_GLOBAL(aop).object_cache_size; i <= handle; i++) {
			PHALCON_GLOBAL(aop).object_cache[i] = NULL;
		}
		PHALCON_GLOBAL(aop).object_cache_size = handle+1;
	}
	if (PHALCON_GLOBAL(aop).object_cache[handle] == NULL) {
		PHALCON_GLOBAL(aop).object_cache[handle] = emalloc(sizeof(phalcon_aop_object_cache));
		PHALCON_GLOBAL(aop).object_cache[handle]->write = NULL;
		PHALCON_GLOBAL(aop).object_cache[handle]->read = NULL;
		PHALCON_GLOBAL(aop).object_cache[handle]->func = NULL;
	}
	return PHALCON_GLOBAL(aop).object_cache[handle];
}
/*}}}*/

/*{{{ get_object_cache_func/read/write*/
zend_array *get_object_cache_func(zend_object *object)
{
	phalcon_aop_object_cache *cache;
	cache = get_object_cache(object);
	if (cache->func == NULL) {
		ALLOC_HASHTABLE(cache->func);
		zend_hash_init(cache->func, 16, NULL, phalcon_aop_free_pointcut_cache , 0);
	}
	return cache->func;
}

zend_array *get_object_cache_read(zend_object *object)
{
	phalcon_aop_object_cache *cache;
	cache = get_object_cache(object);
	if (cache->read == NULL) {
		ALLOC_HASHTABLE(cache->read);
		zend_hash_init(cache->read, 16, NULL, phalcon_aop_free_pointcut_cache, 0);
	}
	return cache->read;
}

zend_array *get_object_cache_write(zend_object *object)
{
	phalcon_aop_object_cache *cache;
	cache = get_object_cache(object);
	if (cache->write == NULL) {
		ALLOC_HASHTABLE(cache->write);
		zend_hash_init(cache->write, 16, NULL, phalcon_aop_free_pointcut_cache , 0);
	}
	return cache->write;
}
/*}}}*/

static zend_array *get_cache_func(zend_execute_data *ex) /*{{{*/
{
	zend_array *ht_object_cache = NULL;
	zend_object *object = NULL;
	zend_class_entry *ce;
	zend_string *cache_key;
	zval *cache = NULL;
	zval pointcut_cache_value;
	phalcon_aop_pointcut_cache *_pointcut_cache = NULL;

#if PHP_MINOR_VERSION < 1
	object = Z_OBJ(ex->This);
#else
	if (Z_TYPE(ex->This) == IS_OBJECT) {
		object = Z_OBJ(ex->This);
	}
#endif
	//1.search cache
	if (object == NULL) { //function or static method
		ht_object_cache = PHALCON_GLOBAL(aop).function_cache;
		if (ex->func->common.fn_flags & ZEND_ACC_STATIC) {
			ce = ex->func->common.scope;//ex->called_scope;
			cache_key = zend_string_init("", ZSTR_LEN(ex->func->common.function_name) + ZSTR_LEN(ce->name) + 2, 0);
			sprintf((char *)ZSTR_VAL(cache_key), "%s::%s", ZSTR_VAL(ce->name), ZSTR_VAL(ex->func->common.function_name));
		} else {
			cache_key = zend_string_copy(ex->func->common.function_name);
		}
	} else { //method
		ce = ex->func->common.scope;//ex->called_scope;
		cache_key = zend_string_copy(ex->func->common.function_name);
		ht_object_cache = get_object_cache_func(object);
	}

	cache = zend_hash_find(ht_object_cache, cache_key);

	if (cache != NULL) {
		_pointcut_cache = (phalcon_aop_pointcut_cache *)Z_PTR_P(cache);
		if (_pointcut_cache->version != PHALCON_GLOBAL(aop).pointcut_version || (object != NULL && _pointcut_cache->ce != ce)) {
			//cache lost
			_pointcut_cache = NULL;
			zend_hash_del(ht_object_cache, cache_key);
		}
	}

	//2.calculate function hit pointcut
	if (_pointcut_cache == NULL) {
		_pointcut_cache = (phalcon_aop_pointcut_cache *)emalloc(sizeof(phalcon_aop_pointcut_cache));
		_pointcut_cache->ht = calculate_function_pointcuts(ex);
		_pointcut_cache->version = PHALCON_GLOBAL(aop).pointcut_version;

		if (object == NULL) {
			_pointcut_cache->ce = NULL;
		} else {
			_pointcut_cache->ce = ce;
		}
		ZVAL_PTR(&pointcut_cache_value, _pointcut_cache);

		zend_hash_add(ht_object_cache, cache_key, &pointcut_cache_value);
	}
	zend_string_release(cache_key);
	return _pointcut_cache->ht;
}
/*}}}*/

static zend_array *get_cache_property(zval *object, zval *member, int type) /*{{{*/
{
	zend_array *ht_object_cache = NULL;
	zval *cache = NULL;
	phalcon_aop_pointcut_cache *_pointcut_cache = NULL;
	zval pointcut_cache_value;
	zend_string *member_str = NULL;

	if (type & PHALCON_AOP_KIND_READ) {
		ht_object_cache = get_object_cache_read(Z_OBJ_P(object));
	} else {
		ht_object_cache = get_object_cache_write(Z_OBJ_P(object));
	}

	if (Z_TYPE_P(member) != IS_STRING ) {
		member_str = zval_get_string(member);
	} else {
		member_str = Z_STR_P(member);
	}
	cache = zend_hash_find(ht_object_cache, member_str);

	if (cache != NULL) {
		_pointcut_cache = (phalcon_aop_pointcut_cache *)Z_PTR_P(cache);
		if (_pointcut_cache->version != PHALCON_GLOBAL(aop).pointcut_version || _pointcut_cache->ce != Z_OBJCE_P(object)) {
			//cache lost
			_pointcut_cache = NULL;
			zend_hash_del(ht_object_cache, member_str);
		}
	}
	if (_pointcut_cache == NULL) {
		_pointcut_cache = (phalcon_aop_pointcut_cache *)emalloc(sizeof(phalcon_aop_pointcut_cache));
		_pointcut_cache->ht = calculate_property_pointcuts(object, member_str, type);
		_pointcut_cache->version = PHALCON_GLOBAL(aop).pointcut_version;
		_pointcut_cache->ce = Z_OBJCE_P(object);

		ZVAL_PTR(&pointcut_cache_value, _pointcut_cache);
		zend_hash_add(ht_object_cache, member_str, &pointcut_cache_value);
	}
	if (member_str != Z_STR_P(member)) {
		zend_string_release(member_str);
	}
	return _pointcut_cache->ht;
}
/*}}}*/

static void execute_pointcut(phalcon_aop_pointcut *pc, zval *arg, zval *retval) /*{{{*/
{
	zval params[1];

	ZVAL_COPY_VALUE(&params[0], arg);

	pc->fci.retval = retval;
	pc->fci.param_count = 1;
	pc->fci.params = params;

	if (zend_call_function(&pc->fci, &pc->fci_cache) == FAILURE) {
		zend_error(E_ERROR, "Problem in AOP Callback");
	}
}
/*}}}*/

static void execute_context(zend_execute_data *execute_data, zval *args) /*{{{*/
{
	if (EG(exception)) {
		return ;
	}

	//overload arguments
	if (args != NULL) {
		uint32_t i, first_extra_arg, call_num_args;
		zval *original_args_value;
		zval *overload_args_value;
		zend_op_array *op_array = &EX(func)->op_array;

		first_extra_arg = op_array->num_args;
		call_num_args = zend_hash_num_elements(Z_ARR_P(args));//ZEND_CALL_NUM_ARGS(execute_data);

		if (call_num_args <= first_extra_arg) {
			for (i = 0; i < call_num_args; i++){
				original_args_value = ZEND_CALL_VAR_NUM(execute_data, i);
				overload_args_value = zend_hash_index_find(Z_ARR_P(args), (zend_ulong)i);

				zval_ptr_dtor(original_args_value);
				ZVAL_COPY(original_args_value, overload_args_value);
			}
		} else {
			//1) overload common params
			for (i = 0; i < first_extra_arg; i++){
				original_args_value = ZEND_CALL_VAR_NUM(execute_data, i);
				overload_args_value = zend_hash_index_find(Z_ARR_P(args), (zend_ulong)i);

				zval_ptr_dtor(original_args_value);
				ZVAL_COPY(original_args_value, overload_args_value);
			}

			//2) overload extra params
			if (op_array->fn_flags & ZEND_ACC_VARIADIC) {
				for (i = 0; i < call_num_args - first_extra_arg; i++) {
					original_args_value = ZEND_CALL_VAR_NUM(execute_data, op_array->last_var + op_array->T + i);
					overload_args_value = zend_hash_index_find(Z_ARR_P(args), (zend_ulong)(i + first_extra_arg));

					zval_ptr_dtor(original_args_value);
					ZVAL_COPY(original_args_value, overload_args_value);
				}
			}
		}
		ZEND_CALL_NUM_ARGS(execute_data) = call_num_args;
	}

	EG(current_execute_data) = execute_data;

	if (execute_data->func->common.type == ZEND_USER_FUNCTION) {
		original_zend_execute_ex(execute_data);
	} else if (execute_data->func->common.type == ZEND_INTERNAL_FUNCTION) {
		//zval return_value;
		if (original_zend_execute_internal) {
			original_zend_execute_internal(execute_data, execute_data->return_value);
		}else{
			execute_internal(execute_data, execute_data->return_value);
		}
	} else { /* ZEND_OVERLOADED_FUNCTION */
		//this will never happend,becase there's no hook for overload function
#if PHP_VERSION_ID >= 70200
		zend_do_fcall_overloaded(execute_data, execute_data->return_value);
#endif
	}
}
/*}}}*/

void phalcon_aop_do_func_execute(HashPosition pos, zend_array *pointcut_table, zend_execute_data *ex, zval *aop_object) /*{{{*/
{
	phalcon_aop_pointcut *current_pc = NULL;
	zval *current_pc_value = NULL;
	zval pointcut_ret = {};
	zend_object *exception = NULL;
#if PHP_MINOR_VERSION < 1
	zend_class_entry *current_scope = NULL;
#endif
	phalcon_aop_joinpoint_object *joinpoint = phalcon_aop_joinpoint_object_from_obj(Z_OBJ_P(aop_object));

	while(1){
		current_pc_value = zend_hash_get_current_data_ex(pointcut_table, &pos);
		if (current_pc_value == NULL || Z_TYPE_P(current_pc_value) != IS_UNDEF) {
			break;
		} else {
			zend_hash_move_forward_ex(pointcut_table, &pos);
		}
	}

	if (current_pc_value == NULL) {
#if PHP_MINOR_VERSION < 1
		if (EG(scope) != ex->called_scope) {
			current_scope = EG(scope);
			EG(scope) = ex->called_scope;
		}
#endif
		PHALCON_GLOBAL(aop).overloaded = 0;
		execute_context(ex, joinpoint->args);
		PHALCON_GLOBAL(aop).overloaded = 1;

		joinpoint->is_ex_executed = 1;

#if PHP_MINOR_VERSION < 1
		if (current_scope != NULL) {
			EG(scope) = current_scope;
		}
#endif

		return;
	}
	zend_hash_move_forward_ex(pointcut_table, &pos);

	current_pc = (phalcon_aop_pointcut *)Z_PTR_P(current_pc_value);

	joinpoint->current_pointcut = current_pc;
	joinpoint->pos = pos;
	joinpoint->kind_of_advice = current_pc->kind_of_advice;

	if (current_pc->kind_of_advice & PHALCON_AOP_KIND_BEFORE) {
		if (!EG(exception)) {
			execute_pointcut(current_pc, aop_object, &pointcut_ret);
			if (Z_TYPE(pointcut_ret) > IS_NULL) {
				zval_ptr_dtor(&pointcut_ret);
			}
		}
	}
	if (current_pc->kind_of_advice & PHALCON_AOP_KIND_AROUND) {
		if (!EG(exception)) {
			execute_pointcut(current_pc, aop_object, &pointcut_ret);
			if (Z_TYPE(pointcut_ret) > IS_NULL) {
				if (ex->return_value != NULL) {
					zval_ptr_dtor(ex->return_value);
					ZVAL_COPY_VALUE(ex->return_value, &pointcut_ret);
				} else {
					zval_ptr_dtor(&pointcut_ret);
				}
			} else if (joinpoint->return_value != NULL) {
				if (ex->return_value != NULL) {
					zval_ptr_dtor(ex->return_value);
					ZVAL_COPY(ex->return_value, joinpoint->return_value);
				}
			}
		}
	} else {
		phalcon_aop_do_func_execute(pos, pointcut_table, ex, aop_object);
	}
	//PHALCON_AOP_KIND_AFTER
	if (current_pc->kind_of_advice & PHALCON_AOP_KIND_AFTER) {
		if (current_pc->kind_of_advice & PHALCON_AOP_KIND_CATCH && EG(exception)) {
			exception = EG(exception);
			joinpoint->exception = exception;
			EG(exception) = NULL;
			execute_pointcut(current_pc, aop_object, &pointcut_ret);
			if (EG(exception)) {
				zend_object_release(EG(exception));
			}
			EG(exception) = exception;

			if (Z_TYPE(pointcut_ret) > IS_NULL) {
				if (ex->return_value != NULL) {
					zval_ptr_dtor(ex->return_value);
					ZVAL_COPY_VALUE(ex->return_value, &pointcut_ret);
				} else {
					zval_ptr_dtor(&pointcut_ret);
				}
			} else if (joinpoint->return_value != NULL) {
				if (ex->return_value != NULL) {
					zval_ptr_dtor(ex->return_value);
					ZVAL_COPY(ex->return_value, joinpoint->return_value);
				}
			}

		} else if (current_pc->kind_of_advice & PHALCON_AOP_KIND_RETURN && !EG(exception)) {
			execute_pointcut(current_pc, aop_object, &pointcut_ret);
			if (Z_TYPE(pointcut_ret) > IS_NULL) {
				if (ex->return_value != NULL) {
					zval_ptr_dtor(ex->return_value);
					ZVAL_COPY_VALUE(ex->return_value, &pointcut_ret);
				} else {
					zval_ptr_dtor(&pointcut_ret);
				}
			} else if (joinpoint->return_value != NULL) {
				if (ex->return_value != NULL) {
					zval_ptr_dtor(ex->return_value);
					ZVAL_COPY(ex->return_value, joinpoint->return_value);
				}
			}
		}
	}
}
/*}}}*/

void func_pointcut_and_execute(zend_execute_data *ex) /*{{{*/
{
	zval aop_object;
	phalcon_aop_joinpoint_object *joinpoint;
	zend_array *pointcut_table = NULL;
	HashPosition pos;
	zval *real_return_value;

	//find pointcut of current call function
	pointcut_table = get_cache_func (ex);
	if (pointcut_table == NULL || zend_hash_num_elements(pointcut_table) == 0) {
		PHALCON_GLOBAL(aop).overloaded = 0;
		execute_context(ex, NULL);
		PHALCON_GLOBAL(aop).overloaded = 1;
		return;
	}
	zend_hash_internal_pointer_reset_ex(pointcut_table, &pos);

	object_init_ex(&aop_object, phalcon_aop_joinpoint_ce);
	joinpoint = phalcon_aop_joinpoint_object_from_obj(Z_OBJ(aop_object));
	joinpoint->ex = ex;
	joinpoint->is_ex_executed = 0;
	joinpoint->advice = pointcut_table;
	joinpoint->exception = NULL;
	joinpoint->args = NULL;
	joinpoint->return_value = NULL;

	ZVAL_UNDEF(&joinpoint->property_value);

	if (EG(current_execute_data) == ex){
		//dely to execute call function, execute pointcut first
		EG(current_execute_data) = ex->prev_execute_data;
	}

	int no_ret = 0;
	if (ex->return_value == NULL) {
		no_ret = 1;
		ex->return_value = emalloc(sizeof(zval));
		ZVAL_UNDEF(ex->return_value);
	}

	phalcon_aop_do_func_execute(pos, pointcut_table, ex, &aop_object);

	if (no_ret == 1){
		zval_ptr_dtor(ex->return_value);
		efree(ex->return_value);
	} else {
		if (joinpoint->return_value_changed && Z_ISREF_P(ex->return_value)) {
			real_return_value = Z_REFVAL_P(ex->return_value);
			Z_TRY_ADDREF_P(real_return_value);
			zval_ptr_dtor(ex->return_value);
			ZVAL_COPY_VALUE(ex->return_value, real_return_value);
		}
	}

	if (joinpoint->is_ex_executed == 0) {
		uint32_t i;
		zval *original_args_value;

		 for (i = 0; i < ex->func->common.num_args; i++) {
			original_args_value = ZEND_CALL_VAR_NUM(ex, i);
			zval_ptr_dtor(original_args_value);
		 }
	}

	zval_ptr_dtor(&aop_object);
	return;
}
/*}}}*/

ZEND_API void phalcon_aop_execute_internal(zend_execute_data *ex, zval *return_value) /*{{{*/
{
	zend_function *fbc = NULL;

	fbc = ex->func;

	if (!PHALCON_GLOBAL(aop).enable_aop || fbc == NULL || PHALCON_GLOBAL(aop).overloaded || EG(exception) || fbc->common.function_name == NULL) {
		if (original_zend_execute_internal) {
			return original_zend_execute_internal(ex, return_value);
		}else{
			return execute_internal(ex, return_value);
		}
	}

	ex->return_value = return_value;

	PHALCON_GLOBAL(aop).overloaded = 1;
	func_pointcut_and_execute(ex);
	PHALCON_GLOBAL(aop).overloaded = 0;
}
/*}}}*/

//execute_ex overload
ZEND_API void phalcon_aop_execute_ex(zend_execute_data *ex) /*{{{*/
{
	zend_function *fbc = NULL;

	fbc = ex->func;

	if (!PHALCON_GLOBAL(aop).enable_aop || fbc == NULL || PHALCON_GLOBAL(aop).overloaded || EG(exception) || fbc->common.function_name == NULL || fbc->common.type == ZEND_EVAL_CODE || fbc->common.fn_flags & ZEND_ACC_CLOSURE) {
		return original_zend_execute_ex(ex);
	}

	PHALCON_GLOBAL(aop).overloaded = 1;
	func_pointcut_and_execute(ex);
	PHALCON_GLOBAL(aop).overloaded = 0;
}
/*}}}*/

void phalcon_aop_do_read_property(HashPosition pos, zend_array *pointcut_table, zval *aop_object) /*{{{*/
{
	phalcon_aop_pointcut *current_pc = NULL;
	zval *current_pc_value = NULL;
	zval pointcut_ret;
	phalcon_aop_joinpoint_object *joinpoint = phalcon_aop_joinpoint_object_from_obj(Z_OBJ_P(aop_object));
	zval *property_value;
	zend_class_entry *current_scope = NULL;

	while(1){
		current_pc_value = zend_hash_get_current_data_ex(pointcut_table, &pos);
		if (current_pc_value == NULL || Z_TYPE_P(current_pc_value) != IS_UNDEF) {
			break;
		} else {
			zend_hash_move_forward_ex(pointcut_table, &pos);
		}
	}

	if (current_pc_value == NULL) {
#if PHP_MINOR_VERSION < 1
		if (EG(scope) != joinpoint->ex->called_scope) {
			current_scope = EG(scope);
			EG(scope) = joinpoint->ex->called_scope;
		}
#else
		if (EG(fake_scope) != joinpoint->ex->func->common.scope) {
			current_scope = EG(fake_scope);
			EG(fake_scope) = joinpoint->ex->func->common.scope;
		}
#endif

		property_value = original_zend_std_read_property(joinpoint->object, joinpoint->member, joinpoint->type, joinpoint->cache_slot, joinpoint->rv);
		ZVAL_COPY_VALUE(PHALCON_GLOBAL(aop).property_value, property_value);

		if (current_scope != NULL) {
#if PHP_MINOR_VERSION < 1
			EG(scope) = current_scope;
#else
			EG(fake_scope) = current_scope;
#endif
		}
		return;
	}
	zend_hash_move_forward_ex(pointcut_table, &pos);

	current_pc = (phalcon_aop_pointcut *)Z_PTR_P(current_pc_value);

	joinpoint->current_pointcut = current_pc;
	joinpoint->pos = pos;
	joinpoint->kind_of_advice = (current_pc->kind_of_advice&PHALCON_AOP_KIND_WRITE) ? (current_pc->kind_of_advice - PHALCON_AOP_KIND_WRITE) : current_pc->kind_of_advice;

	if (current_pc->kind_of_advice & PHALCON_AOP_KIND_BEFORE) {
		execute_pointcut(current_pc, aop_object, &pointcut_ret);
		if (Z_TYPE(pointcut_ret) > IS_NULL) {
			zval_ptr_dtor(&pointcut_ret);
		}
	}

	if (current_pc->kind_of_advice & PHALCON_AOP_KIND_AROUND) {
		execute_pointcut(current_pc, aop_object, &pointcut_ret);
		if (Z_TYPE(pointcut_ret) > IS_NULL) {
			ZVAL_COPY_VALUE(PHALCON_GLOBAL(aop).property_value, &pointcut_ret);
		} else if (joinpoint->return_value != NULL) {
			ZVAL_COPY(PHALCON_GLOBAL(aop).property_value, joinpoint->return_value);
		}
	} else {
		phalcon_aop_do_read_property(pos, pointcut_table, aop_object);
	}

	if (current_pc->kind_of_advice & PHALCON_AOP_KIND_AFTER) {
		execute_pointcut(current_pc, aop_object, &pointcut_ret);
		if (Z_TYPE(pointcut_ret) > IS_NULL) {
			ZVAL_COPY_VALUE(PHALCON_GLOBAL(aop).property_value, &pointcut_ret);
		} else if (joinpoint->return_value != NULL) {
			ZVAL_COPY(PHALCON_GLOBAL(aop).property_value, joinpoint->return_value);
		}
	}
}
/*}}}*/

zval *phalcon_aop_read_property(zval *object, zval *member, int type, void **cache_slot, zval *rv) /*{{{*/
{
	zval aop_object;
	phalcon_aop_joinpoint_object *joinpoint;
	zend_array *pointcut_table = NULL;
	HashPosition pos;

	if (PHALCON_GLOBAL(aop).lock_read_property > 25) {
		zend_error(E_ERROR, "Too many level of nested advices. Are there any recursive call ?");
	}

	pointcut_table = get_cache_property(object, member, PHALCON_AOP_KIND_READ);
	if (pointcut_table == NULL || zend_hash_num_elements(pointcut_table) == 0) {
		return original_zend_std_read_property(object,member,type,cache_slot,rv);
	}
	zend_hash_internal_pointer_reset_ex(pointcut_table, &pos);

	object_init_ex(&aop_object, phalcon_aop_joinpoint_ce);
	joinpoint = phalcon_aop_joinpoint_object_from_obj(Z_OBJ(aop_object));
	joinpoint->advice = pointcut_table;
	joinpoint->ex = EG(current_execute_data);
	joinpoint->args = NULL;
	joinpoint->return_value = NULL;
	joinpoint->object = object;
	joinpoint->member = member;
	joinpoint->type = type;
	//To avoid use runtime cache
	joinpoint->cache_slot = NULL;//cache_slot;
	joinpoint->rv = rv;

	ZVAL_UNDEF(&joinpoint->property_value);

	if (PHALCON_GLOBAL(aop).property_value == NULL) {
		PHALCON_GLOBAL(aop).property_value = emalloc(sizeof(zval));
	}
	ZVAL_NULL(PHALCON_GLOBAL(aop).property_value);

	PHALCON_GLOBAL(aop).lock_read_property++;
	phalcon_aop_do_read_property(0, pointcut_table, &aop_object);
	PHALCON_GLOBAL(aop).lock_read_property--;

	zval_ptr_dtor(&aop_object);
	return PHALCON_GLOBAL(aop).property_value;
}
/*}}}*/

void phalcon_aop_do_write_property(HashPosition pos, zend_array *pointcut_table, zval *aop_object) /*{{{*/
{
	phalcon_aop_pointcut *current_pc = NULL;
	zval *current_pc_value = NULL;
	zval pointcut_ret;
	phalcon_aop_joinpoint_object *joinpoint = phalcon_aop_joinpoint_object_from_obj(Z_OBJ_P(aop_object));
	zend_class_entry *current_scope = NULL;

	while(1){
		current_pc_value = zend_hash_get_current_data_ex(pointcut_table, &pos);
		if (current_pc_value == NULL || Z_TYPE_P(current_pc_value) != IS_UNDEF) {
			break;
		} else {
			zend_hash_move_forward_ex(pointcut_table, &pos);
		}
	}

	if (current_pc_value == NULL) {
#if PHP_MINOR_VERSION < 1
		if (EG(scope) != joinpoint->ex->called_scope) {
			current_scope = EG(scope);
			EG(scope) = joinpoint->ex->called_scope;
		}
#else
		if (EG(fake_scope) != joinpoint->ex->func->common.scope) {
			current_scope = EG(fake_scope);
			EG(fake_scope) = joinpoint->ex->func->common.scope;
		}
#endif
		original_zend_std_write_property(joinpoint->object, joinpoint->member, &joinpoint->property_value, joinpoint->cache_slot);
		if (current_scope != NULL) {
#if PHP_MINOR_VERSION < 1
			EG(scope) = current_scope;
#else
			EG(fake_scope) = current_scope;
#endif
		}
		return;
	}
	zend_hash_move_forward_ex(pointcut_table, &pos);

	current_pc = (phalcon_aop_pointcut *)Z_PTR_P(current_pc_value);

	joinpoint->current_pointcut = current_pc;
	joinpoint->pos = pos;
	joinpoint->kind_of_advice = (current_pc->kind_of_advice&PHALCON_AOP_KIND_READ) ? (current_pc->kind_of_advice - PHALCON_AOP_KIND_READ) : current_pc->kind_of_advice;

	if (current_pc->kind_of_advice & PHALCON_AOP_KIND_BEFORE) {
		execute_pointcut(current_pc, aop_object, &pointcut_ret);
		if (Z_TYPE(pointcut_ret) > IS_NULL) {
			zval_ptr_dtor(&pointcut_ret);
		}
	}

	if (current_pc->kind_of_advice & PHALCON_AOP_KIND_AROUND) {
		execute_pointcut(current_pc, aop_object, &pointcut_ret);
	} else {
		phalcon_aop_do_write_property(pos, pointcut_table, aop_object);
	}

	if (current_pc->kind_of_advice & PHALCON_AOP_KIND_AFTER) {
		execute_pointcut(current_pc, aop_object, &pointcut_ret);
	}
}
/*}}}*/

void phalcon_aop_write_property(zval *object, zval *member, zval *value, void **cache_slot) /*{{{*/
{
	zval aop_object;
	phalcon_aop_joinpoint_object *joinpoint;
	zend_array *pointcut_table = NULL;
	HashPosition pos;

	if (PHALCON_GLOBAL(aop).lock_write_property > 25) {
		zend_error(E_ERROR, "Too many level of nested advices. Are there any recursive call ?");
	}

	pointcut_table = get_cache_property(object, member, PHALCON_AOP_KIND_WRITE);
	if (pointcut_table == NULL || zend_hash_num_elements(pointcut_table) == 0) {
		original_zend_std_write_property(object, member, value, cache_slot);
		return ;
	}
	zend_hash_internal_pointer_reset_ex(pointcut_table, &pos);

	object_init_ex(&aop_object, phalcon_aop_joinpoint_ce);
	joinpoint = phalcon_aop_joinpoint_object_from_obj(Z_OBJ(aop_object));
	joinpoint->advice = pointcut_table;
	joinpoint->ex = EG(current_execute_data);
	joinpoint->args = NULL;
	joinpoint->return_value = NULL;
	joinpoint->object = object;
	joinpoint->member = member;
	//To avoid use runtime cache
	joinpoint->cache_slot = NULL;//cache_slot;

	ZVAL_COPY(&joinpoint->property_value, value);

	PHALCON_GLOBAL(aop).lock_write_property++;
	phalcon_aop_do_write_property(0, pointcut_table, &aop_object);
	PHALCON_GLOBAL(aop).lock_write_property--;

	zval_ptr_dtor(&aop_object);
}
/*}}}*/

zval *phalcon_aop_get_property_ptr_ptr(zval *object, zval *member, int type, void **cache_slot)
{
	zend_execute_data *ex = EG(current_execute_data);
	if (ex->opline == NULL || (ex->opline->opcode != ZEND_PRE_INC_OBJ && ex->opline->opcode != ZEND_POST_INC_OBJ && ex->opline->opcode != ZEND_PRE_DEC_OBJ && ex->opline->opcode != ZEND_POST_DEC_OBJ)) {
		return original_zend_std_get_property_ptr_ptr(object, member, type, cache_slot);
	}
	// Call original to not have a notice
	//original_zend_std_get_property_ptr_ptr(object, member, type, cache_slot);
	return NULL;
}

void phalcon_aop_make_regexp_on_pointcut (phalcon_aop_pointcut *pc) /*{{{*/
{
#if PHP_VERSION_ID >= 70300
	uint32_t *pcre_extra = NULL;
	uint32_t preg_options = 0;
#else
	pcre_extra *pcre_extra = NULL;
	int preg_options = 0;
#endif
	zend_string *regexp;
	zend_string *regexp_buffer = NULL;
	zend_string *regexp_tmp = NULL;
	char tempregexp[500];

	pc->method_jok = (strchr(ZSTR_VAL(pc->method), '*') != NULL);

	regexp_buffer = php_str_to_str(ZSTR_VAL(pc->method), ZSTR_LEN(pc->method), "**\\", 3, "[.#}", 4);
	
	regexp_tmp = regexp_buffer;
	regexp_buffer = php_str_to_str(ZSTR_VAL(regexp_tmp), ZSTR_LEN(regexp_buffer), "**", 2, "[.#]", 4);
	zend_string_release(regexp_tmp);

	regexp_tmp = regexp_buffer;
	regexp_buffer = php_str_to_str(ZSTR_VAL(regexp_tmp), ZSTR_LEN(regexp_buffer), "\\", 1, "\\\\", 2);
	zend_string_release(regexp_tmp);
	
	regexp_tmp = regexp_buffer;
	regexp_buffer = php_str_to_str(ZSTR_VAL(regexp_tmp), ZSTR_LEN(regexp_buffer), "*", 1, "[^\\\\]*", 6);
	zend_string_release(regexp_tmp);

	regexp_tmp = regexp_buffer;
	regexp_buffer = php_str_to_str(ZSTR_VAL(regexp_tmp), ZSTR_LEN(regexp_buffer), "[.#]", 4, ".*", 2);
	zend_string_release(regexp_tmp);
	
	regexp_tmp = regexp_buffer;
	regexp_buffer = php_str_to_str(ZSTR_VAL(regexp_tmp), ZSTR_LEN(regexp_buffer), "[.#}", 4, "(.*\\\\)?", 7);
	zend_string_release(regexp_tmp);

	if (ZSTR_VAL(regexp_buffer)[0] != '\\') {
		sprintf((char *)tempregexp, "/^%s$/i", ZSTR_VAL(regexp_buffer));
	} else {
		sprintf((char *)tempregexp, "/^%s$/i", ZSTR_VAL(regexp_buffer) + 2);
	}
	zend_string_release(regexp_buffer);

	regexp = zend_string_init(tempregexp, strlen(tempregexp), 0);
#if PHP_VERSION_ID >= 70300
	pc->re_method = pcre_get_compiled_regex(regexp, pcre_extra, &preg_options);
#else
	pc->re_method = pcre_get_compiled_regex(regexp, &pcre_extra, &preg_options);
#endif
	zend_string_release(regexp);	

	if (!pc->re_method) {
		php_error_docref(NULL, E_WARNING, "Invalid expression");
	}

	if (pc->class_name != NULL) {
		regexp_buffer = php_str_to_str(ZSTR_VAL(pc->class_name), ZSTR_LEN(pc->class_name), "**\\", 3, "[.#}", 4);
		
		regexp_tmp = regexp_buffer;
		regexp_buffer = php_str_to_str(ZSTR_VAL(regexp_tmp), ZSTR_LEN(regexp_buffer), "**", 2, "[.#]", 4);
		zend_string_release(regexp_tmp);
		
		regexp_tmp = regexp_buffer;
		regexp_buffer = php_str_to_str(ZSTR_VAL(regexp_tmp), ZSTR_LEN(regexp_buffer), "\\", 1, "\\\\", 2);
		zend_string_release(regexp_tmp);
		
		regexp_tmp = regexp_buffer;
		regexp_buffer = php_str_to_str(ZSTR_VAL(regexp_tmp), ZSTR_LEN(regexp_buffer), "*", 1, "[^\\\\]*", 6);
		zend_string_release(regexp_tmp);
		
		regexp_tmp = regexp_buffer;
		regexp_buffer = php_str_to_str(ZSTR_VAL(regexp_tmp), ZSTR_LEN(regexp_buffer), "[.#]", 4, ".*", 2);
		zend_string_release(regexp_tmp);
		
		regexp_tmp = regexp_buffer;
		regexp_buffer = php_str_to_str(ZSTR_VAL(regexp_tmp), ZSTR_LEN(regexp_buffer), "[.#}", 4, "(.*\\\\)?", 7);
		zend_string_release(regexp_tmp);

		if (ZSTR_VAL(regexp_buffer)[0] != '\\') {
			sprintf((char *)tempregexp, "/^%s$/i", ZSTR_VAL(regexp_buffer));
		} else {
			sprintf((char *)tempregexp, "/^%s$/i", ZSTR_VAL(regexp_buffer) + 2);
		}
		zend_string_release(regexp_buffer);

		regexp = zend_string_init(tempregexp, strlen(tempregexp), 0);
#if PHP_VERSION_ID >= 70300
		pc->re_class = pcre_get_compiled_regex(regexp, pcre_extra, &preg_options);
#else
		pc->re_class = pcre_get_compiled_regex(regexp, &pcre_extra, &preg_options);
#endif
		zend_string_release(regexp);

		if (!pc->re_class) {
			php_error_docref(NULL, E_WARNING, "Invalid expression");
		}
	}
}
/*}}}*/

static phalcon_aop_pointcut *phalcon_aop_alloc_pointcut() /*{{{*/
{
	phalcon_aop_pointcut *pc = (phalcon_aop_pointcut *)emalloc(sizeof(phalcon_aop_pointcut));

	pc->scope = 0;
	pc->static_state = 2;
	pc->method_jok = 0;
	pc->class_jok = 0;
	pc->class_name = NULL;
	pc->method = NULL;
	pc->selector = NULL;
	pc->kind_of_advice = 0;
	//pc->fci = NULL;
	//pc->fcic = NULL;
	pc->re_method = NULL;
	pc->re_class = NULL;
	return pc;
}
/*}}}*/

void phalcon_aop_free_pointcut(zval *elem)
{
	phalcon_aop_pointcut *pc = (phalcon_aop_pointcut *)Z_PTR_P(elem);

	if (pc == NULL) {
		return;
	}

	if (&(pc->fci.function_name)) {
		zval_ptr_dtor(&pc->fci.function_name);
	}

	if (pc->method != NULL) {
		zend_string_release(pc->method);
	}
	if (pc->class_name != NULL) {
		zend_string_release(pc->class_name);
	}
	efree(pc);
}

void phalcon_aop_free_pointcut_cache(zval *elem)
{
	phalcon_aop_pointcut_cache *cache = (phalcon_aop_pointcut_cache *)Z_PTR_P(elem);
	if (cache->ht != NULL) {
		zend_hash_destroy(cache->ht);
		FREE_HASHTABLE(cache->ht);
	}
	efree(cache);
}

static void phalcon_aop_add_pointcut (zend_fcall_info fci, zend_fcall_info_cache fci_cache, zend_string *selector, int cut_type) /*{{{*/
{
	zval pointcut_val;
	phalcon_aop_pointcut *pc = NULL;
	char *temp_str = NULL;
	int is_class = 0;
	scanner_state *state;
	scanner_token *token;

	if (ZSTR_LEN(selector) < 2) {
		zend_error(E_ERROR, "The given pointcut is invalid. You must specify a function call, a method call or a property operation");
	}

	pc = phalcon_aop_alloc_pointcut();
	pc->selector = selector;
	pc->fci = fci;
	pc->fci_cache = fci_cache;
	pc->kind_of_advice = cut_type;

	state = (scanner_state *)emalloc(sizeof(scanner_state));
	token = (scanner_token *)emalloc(sizeof(scanner_token));

	state->start = ZSTR_VAL(selector);
	state->end = state->start;
	while(0 <= scan(state, token)) {
		//		php_printf("TOKEN %d \n", token->TOKEN);
		switch (token->TOKEN) {
			case TOKEN_STATIC:
				pc->static_state=token->int_val;
				break;
			case TOKEN_SCOPE:
				pc->scope |= token->int_val;
				break;
			case TOKEN_CLASS:
				pc->class_name = zend_string_init(temp_str, strlen(temp_str), 0);//estrdup(temp_str);
				efree(temp_str);
				temp_str=NULL;
				is_class=1;
				break;
			case TOKEN_PROPERTY:
				pc->kind_of_advice |= PHALCON_AOP_KIND_PROPERTY | token->int_val;
				break;
			case TOKEN_FUNCTION:
				if (is_class) {
					pc->kind_of_advice |= PHALCON_AOP_KIND_METHOD;
				} else {
					pc->kind_of_advice |= PHALCON_AOP_KIND_FUNCTION;
				}
				break;
			case TOKEN_TEXT:
				if (temp_str!=NULL) {
					efree(temp_str);
				}
				temp_str=estrdup(token->str_val);
				efree(token->str_val);
				break;
			default:
				break;
		}
	}
	if (temp_str != NULL) {
		//method or property
		pc->method = zend_string_init(temp_str, strlen(temp_str), 0);
		efree(temp_str);
	}
	efree(state);
	efree(token);

	//add("class::property", xxx)
	if (pc->kind_of_advice == cut_type) {
		pc->kind_of_advice |= PHALCON_AOP_KIND_READ | PHALCON_AOP_KIND_WRITE | PHALCON_AOP_KIND_PROPERTY;
	}

	phalcon_aop_make_regexp_on_pointcut(pc);

	ZVAL_PTR(&pointcut_val, pc);
	zend_hash_next_index_insert(PHALCON_GLOBAL(aop).pointcuts_table, &pointcut_val);
	PHALCON_GLOBAL(aop).pointcut_version++;
}
/*}}}*/

/**
 * Adds an advice to be run before the matching join points
 *
 *<code>
 *	\Phalcon\Aop::addBefore('mytest::test()', function(Phalcon\Aop\Joinpoint $obj){
 *		echo "before";
 *	})
 *</code>
 *
 * @param string $selector
 * @param callable $handler
 */
PHP_METHOD(Phalcon_Aop, addBefore){

	zend_string *selector;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;

	if (!PHALCON_GLOBAL(aop).enable_aop) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_aop_exception_ce, "Change phalcon.aop.enable_aop in php.ini.");
		return;
	}

	//parse prameters
	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_STR(selector)
		Z_PARAM_FUNC(fci, fci_cache)
	ZEND_PARSE_PARAMETERS_END_EX(
		PHALCON_THROW_EXCEPTION_STR(phalcon_aop_exception_ce, "Expects a string for the pointcut as a first argument and a callback as a second argument");
		return;
	);

	if (&(fci.function_name)) {
		Z_TRY_ADDREF(fci.function_name);
	}
	phalcon_aop_add_pointcut(fci, fci_cache, selector, PHALCON_AOP_KIND_BEFORE);
}

/**
 * Adds an advice to be run after the matching join points
 *
 *<code>
 *	\Phalcon\Aop::addAfter('mytest::test()', function(Phalcon\Aop\Joinpoint $obj){
 *		echo "after";
 *	})
 *</code>
 *
 * @param string $selector
 * @param callable $handler
 */
PHP_METHOD(Phalcon_Aop, addAfter){

	zend_string *selector;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;

	if (!PHALCON_GLOBAL(aop).enable_aop) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_aop_exception_ce, "Change phalcon.aop.enable_aop in php.ini.");
		return;
	}

	//parse prameters
	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_STR(selector)
		Z_PARAM_FUNC(fci, fci_cache)
	ZEND_PARSE_PARAMETERS_END_EX(
		PHALCON_THROW_EXCEPTION_STR(phalcon_aop_exception_ce, "Expects a string for the pointcut as a first argument and a callback as a second argument");
		return;
	);

	if (&(fci.function_name)) {
		Z_TRY_ADDREF(fci.function_name);
	}
	phalcon_aop_add_pointcut(fci, fci_cache, selector, PHALCON_AOP_KIND_AFTER | PHALCON_AOP_KIND_CATCH | PHALCON_AOP_KIND_RETURN);
}

/**
 * Links advices that becomes active after the target normally returns from execution (no exception).
 *
 *<code>
 *	\Phalcon\Aop::addAfterReturning('mytest::test()', function(Phalcon\Aop\Joinpoint $obj){
 *		if ($obj->getReturnedValue() === null) {
 *			echo "I'm updating {$obj->getMethodName()} in {$obj->getClassName()}, now returning this";
 *			$obj->setReturnedValue($obj->getObject());
 *		}
 *	})
 *</code>
 *
 * @param string $selector
 * @param callable $handler
 */
PHP_METHOD(Phalcon_Aop, addAfterReturning){

	zend_string *selector;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;

	if (!PHALCON_GLOBAL(aop).enable_aop) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_aop_exception_ce, "Change phalcon.aop.enable_aop in php.ini.");
		return;
	}

	//parse prameters
	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_STR(selector)
		Z_PARAM_FUNC(fci, fci_cache)
	ZEND_PARSE_PARAMETERS_END_EX(
		PHALCON_THROW_EXCEPTION_STR(phalcon_aop_exception_ce, "Expects a string for the pointcut as a first argument and a callback as a second argument");
		return;
	);

	if (&(fci.function_name)) {
		Z_TRY_ADDREF(fci.function_name);
	}
	phalcon_aop_add_pointcut(fci, fci_cache, selector, PHALCON_AOP_KIND_AFTER | PHALCON_AOP_KIND_RETURN);
}

/**
 * Links advices that becomes active if the target raise an (uncaught) exception.
 *
 * @param string $selector
 * @param callable $handler
 */
PHP_METHOD(Phalcon_Aop, addAfterThrowing){

	zend_string *selector;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;

	if (!PHALCON_GLOBAL(aop).enable_aop) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_aop_exception_ce, "Change phalcon.aop.enable_aop in php.ini.");
		return;
	}

	//parse prameters
	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_STR(selector)
		Z_PARAM_FUNC(fci, fci_cache)
	ZEND_PARSE_PARAMETERS_END_EX(
		PHALCON_THROW_EXCEPTION_STR(phalcon_aop_exception_ce, "Expects a string for the pointcut as a first argument and a callback as a second argument");
		return;
	);

	if (&(fci.function_name)) {
		Z_TRY_ADDREF(fci.function_name);
	}
	phalcon_aop_add_pointcut(fci, fci_cache, selector, PHALCON_AOP_KIND_AFTER | PHALCON_AOP_KIND_CATCH);
}

/**
 * Adds an advice to be run around the matching join points
 *
 *<code>
 *	\Phalcon\Aop::addAround('mytest::test()', function(Phalcon\Aop\Joinpoint $obj){
 *		return $obj->process();
 *	})
 *</code>
 *
 * @param string $selector
 * @param callable $handler
 */
PHP_METHOD(Phalcon_Aop, addAround){

	zend_string *selector;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;

	if (!PHALCON_GLOBAL(aop).enable_aop) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_aop_exception_ce, "Change phalcon.aop.enable_aop in php.ini.");
		return;
	}

	//parse prameters
	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_STR(selector)
		Z_PARAM_FUNC(fci, fci_cache)
	ZEND_PARSE_PARAMETERS_END_EX(
		PHALCON_THROW_EXCEPTION_STR(phalcon_aop_exception_ce, "Expects a string for the pointcut as a first argument and a callback as a second argument");
		return;
	);

	if (&(fci.function_name)) {
		Z_TRY_ADDREF(fci.function_name);
	}
	phalcon_aop_add_pointcut(fci, fci_cache, selector, PHALCON_AOP_KIND_AROUND);
}
