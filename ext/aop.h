
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

#ifndef PHALCON_AOP_H
#define PHALCON_AOP_H

#include "php_phalcon.h"

#include <ext/pcre/php_pcre.h>

#define PHALCON_AOP_KIND_AROUND		1
#define PHALCON_AOP_KIND_BEFORE		2
#define PHALCON_AOP_KIND_AFTER		4
#define PHALCON_AOP_KIND_READ		8
#define PHALCON_AOP_KIND_WRITE 		16
#define PHALCON_AOP_KIND_PROPERTY	32
#define PHALCON_AOP_KIND_METHOD		64
#define PHALCON_AOP_KIND_FUNCTION	128
#define PHALCON_AOP_KIND_CATCH		256
#define PHALCON_AOP_KIND_RETURN		512

#define PHALCON_AOP_KIND_AROUND_READ_PROPERTY  (PHALCON_AOP_KIND_AROUND + PHALCON_AOP_KIND_READ + PHALCON_AOP_KIND_PROPERTY)
#define PHALCON_AOP_KIND_AROUND_WRITE_PROPERTY (PHALCON_AOP_KIND_AROUND + PHALCON_AOP_KIND_WRITE + PHALCON_AOP_KIND_PROPERTY)
#define PHALCON_AOP_KIND_BEFORE_READ_PROPERTY  (PHALCON_AOP_KIND_BEFORE + PHALCON_AOP_KIND_READ + PHALCON_AOP_KIND_PROPERTY)
#define PHALCON_AOP_KIND_BEFORE_WRITE_PROPERTY (PHALCON_AOP_KIND_BEFORE + PHALCON_AOP_KIND_WRITE + PHALCON_AOP_KIND_PROPERTY)
#define PHALCON_AOP_KIND_AFTER_READ_PROPERTY   (PHALCON_AOP_KIND_AFTER + PHALCON_AOP_KIND_READ + PHALCON_AOP_KIND_PROPERTY)
#define PHALCON_AOP_KIND_AFTER_WRITE_PROPERTY  (PHALCON_AOP_KIND_AFTER + PHALCON_AOP_KIND_WRITE + PHALCON_AOP_KIND_PROPERTY)

#define PHALCON_AOP_KIND_AROUND_METHOD   (PHALCON_AOP_KIND_AROUND + PHALCON_AOP_KIND_METHOD)
#define PHALCON_AOP_KIND_AROUND_FUNCTION (PHALCON_AOP_KIND_AROUND + PHALCON_AOP_KIND_FUNCTION)
#define PHALCON_AOP_KIND_BEFORE_METHOD   (PHALCON_AOP_KIND_BEFORE + PHALCON_AOP_KIND_METHOD)
#define PHALCON_AOP_KIND_BEFORE_FUNCTION (PHALCON_AOP_KIND_BEFORE + PHALCON_AOP_KIND_FUNCTION)
#define PHALCON_AOP_KIND_AFTER_METHOD    (PHALCON_AOP_KIND_AFTER + PHALCON_AOP_KIND_METHOD)
#define PHALCON_AOP_KIND_AFTER_FUNCTION  (PHALCON_AOP_KIND_AFTER + PHALCON_AOP_KIND_FUNCTION)

typedef struct {
	int scope;
	int static_state;

	zend_string *class_name;
	int class_jok;
	
	zend_string *method;
	int method_jok;
	
	zend_string *selector;
	int kind_of_advice;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;
#if PHP_VERSION_ID < 70300
	pcre *re_method;
	pcre *re_class;
#else
	pcre2_code *re_method;
	pcre2_code *re_class;
#endif
} phalcon_aop_pointcut;

typedef struct {
    zend_array *ht;
    int version;
    zend_class_entry *ce;
} phalcon_aop_pointcut_cache;

ZEND_API void phalcon_aop_execute_internal(zend_execute_data *execute_data, zval *return_value);
ZEND_API void phalcon_aop_execute_ex(zend_execute_data *execute_data);

void phalcon_aop_do_func_execute(HashPosition pos, zend_array *pointcut_table, zend_execute_data *ex, zval *object);
void phalcon_aop_do_read_property(HashPosition pos, zend_array *pointcut_table, zval *aop_object);
void phalcon_aop_do_write_property(HashPosition pos, zend_array *pointcut_table, zval *aop_object);

zval *phalcon_aop_read_property(zval *object, zval *member, int type, void **cache_slot, zval *rv);
#if PHP_VERSION_ID >= 70400
zval *phalcon_aop_write_property(zval *object, zval *member, zval *value, void **cache_slot);
#else
void phalcon_aop_write_property(zval *object, zval *member, zval *value, void **cache_slot);
#endif
zval *phalcon_aop_get_property_ptr_ptr(zval *object, zval *member, int type, void **cache_slot);

void phalcon_aop_free_pointcut(zval *elem);
void phalcon_aop_free_pointcut_cache(zval *elem);

extern zend_object_read_property_t    original_zend_std_read_property;
extern zend_object_write_property_t   original_zend_std_write_property;
extern zend_object_get_property_ptr_ptr_t	original_zend_std_get_property_ptr_ptr;

extern zend_class_entry *phalcon_aop_ce;

PHALCON_INIT_CLASS(Phalcon_Aop);

#endif /* PHALCON_AOP_H */
