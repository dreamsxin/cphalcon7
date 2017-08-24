
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
  +------------------------------------------------------------------------+
*/

#include "sync/event.h"
#include "sync/exception.h"

#include "kernel/main.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/string.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/exception.h"
#include "kernel/time.h"

/**
 * Phalcon\Sync\Event
 *
 */
zend_class_entry *phalcon_sync_event_ce;

PHP_METHOD(Phalcon_Sync_Event, __construct);
PHP_METHOD(Phalcon_Sync_Event, wait);
PHP_METHOD(Phalcon_Sync_Event, fire);
PHP_METHOD(Phalcon_Sync_Event, reset);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_sync_event___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, manual, _IS_BOOL, 1)
	ZEND_ARG_TYPE_INFO(0, prefire, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_sync_event_wait, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, wait, IS_LONG, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_sync_event_method_entry[] = {
	PHP_ME(Phalcon_Sync_Event, __construct, arginfo_phalcon_sync_event___construct, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Sync_Event, wait, arginfo_phalcon_sync_event_wait, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Sync_Event, fire, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Sync_Event, reset, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_object_handlers phalcon_sync_event_object_handlers;
zend_object* phalcon_sync_event_object_create_handler(zend_class_entry *ce)
{
	phalcon_sync_event_object *intern = ecalloc(1, sizeof(phalcon_sync_event_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_sync_event_object_handlers;

	intern->MxNamed = 0;
	intern->MxMem = NULL;

	return &intern->std;
}

void phalcon_sync_event_object_free_handler(zend_object *object)
{
	phalcon_sync_event_object *intern = phalcon_sync_event_object_from_obj(object);

	if (intern->MxMem != NULL)
	{
		if (intern->MxNamed) {
			phalcon_namedmem_unmap(intern->MxMem, phalcon_event_getsize());
		} else {
			phalcon_event_free(&intern->MxPthreadEvent);

			efree(intern->MxMem);
		}
	}
}

/**
 * Phalcon\Sync\Event initializer
 */
PHALCON_INIT_CLASS(Phalcon_Sync_Event){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT(Phalcon\\Sync, Event, sync_event, phalcon_sync_event_method_entry, 0);

	return SUCCESS;
}

/**
 * Phalcon\Sync\Event constructor
 *
 * @param string $name
 * @param boolean $manual
 * @param boolean $prefire
 */
PHP_METHOD(Phalcon_Sync_Event, __construct){

	zval *name = NULL, *manual = NULL, *prefire = NULL;
	phalcon_sync_event_object *intern;
	size_t Pos, TempSize;
	int Result;

	phalcon_fetch_params(0, 0, 3, &name, &manual, &prefire);

	intern = phalcon_sync_event_object_from_obj(Z_OBJ_P(getThis()));

	if (!name || PHALCON_IS_EMPTY(name)) {
		intern->MxNamed = 0;
	} else {
		intern->MxNamed = 1;
	}

	TempSize = phalcon_event_getsize();
	Result = phalcon_namedmem_init(&intern->MxMem, &Pos, "/Sync_Event", intern->MxNamed ? Z_STRVAL_P(name) : NULL, TempSize);

	if (Result < 0) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_sync_exception_ce, "Event object could not be created");
		return;
	}

	phalcon_event_get(&intern->MxPthreadEvent, intern->MxMem + Pos);

	/* Handle the first time this event has been opened. */
	if (Result == 0) {
		phalcon_event_init(&intern->MxPthreadEvent, intern->MxNamed, (manual ? 1 : 0), (prefire ? 1 : 0));
		if (intern->MxNamed) phalcon_namedmem_ready(intern->MxMem);
	}
}

/**
 * Waits for an event object to fire
 *
 * @param int $wait
 * @return boolean
 */
PHP_METHOD(Phalcon_Sync_Event, wait){

	zval *_wait = NULL;
	uint32_t wait = -1;
	phalcon_sync_event_object *intern;

	phalcon_fetch_params(0, 0, 1, &_wait);

	if (_wait && Z_TYPE_P(_wait) == IS_LONG) {
		wait = (uint32_t)Z_TYPE_P(_wait);
	}
	if (wait < 0) {
		wait = INFINITE;
	}

	intern = phalcon_sync_event_object_from_obj(Z_OBJ_P(getThis()));

	if (!phalcon_event_wait(&intern->MxPthreadEvent, wait)) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

/**
 * Lets a thread through that is waiting.  Lets multiple threads through that are waiting if the event object is 'manual'
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Sync_Event, fire){

	phalcon_sync_event_object *intern;

	intern = phalcon_sync_event_object_from_obj(Z_OBJ_P(getThis()));

	if (!phalcon_event_fire(&intern->MxPthreadEvent)) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

/**
 * Resets the event object state.  Only use when the event object is 'manual'
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Sync_Event, reset){

	phalcon_sync_event_object *intern;

	intern = phalcon_sync_event_object_from_obj(Z_OBJ_P(getThis()));

	if (!phalcon_event_reset(&intern->MxPthreadEvent)) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}