
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

#include "process/sharedmemory.h"

#include "kernel/main.h"
#include "kernel/shm.h"
#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/operators.h"

/**
 * Phalcon\Process\Sharedmemory
 *
 * This class defines sharedmemory entity and its description
 *
 */
zend_class_entry *phalcon_process_sharedmemory_ce;

PHP_METHOD(Phalcon_Process_Sharedmemory, __construct);
PHP_METHOD(Phalcon_Process_Sharedmemory, open);
PHP_METHOD(Phalcon_Process_Sharedmemory, create);
PHP_METHOD(Phalcon_Process_Sharedmemory, lock);
PHP_METHOD(Phalcon_Process_Sharedmemory, unlock);
PHP_METHOD(Phalcon_Process_Sharedmemory, read);
PHP_METHOD(Phalcon_Process_Sharedmemory, write);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_process_sharedmemory___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_process_sharedmemory_method_entry[] = {
	PHP_ME(Phalcon_Process_Sharedmemory, __construct, arginfo_phalcon_process_sharedmemory___construct, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Process_Sharedmemory, open, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Sharedmemory, create, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Sharedmemory, lock, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Sharedmemory, unlock, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Sharedmemory, read, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Process_Sharedmemory, write, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};
