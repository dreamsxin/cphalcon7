
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

#include "profiler/iteminterface.h"
#include "kernel/main.h"

zend_class_entry *phalcon_profiler_iteminterface_ce;

static const zend_function_entry phalcon_profiler_iteminterface_method_entry[] = {
	PHP_ABSTRACT_ME(Phalcon_ProfilerInterface, getName, NULL)
	PHP_ABSTRACT_ME(Phalcon_ProfilerInterface, setData, arginfo_phalcon_profiler_iteminterface_setdata)
	PHP_ABSTRACT_ME(Phalcon_ProfilerInterface, getData, NULL)
	PHP_ABSTRACT_ME(Phalcon_ProfilerInterface, setInitialTime, arginfo_phalcon_profiler_iteminterface_setinitialtime)
	PHP_ABSTRACT_ME(Phalcon_ProfilerInterface, getInitialTime, NULL)
	PHP_ABSTRACT_ME(Phalcon_ProfilerInterface, setFinalTime, arginfo_phalcon_profiler_iteminterface_setfinaltime)
	PHP_ABSTRACT_ME(Phalcon_ProfilerInterface, getFinalTime, NULL)
	PHP_ABSTRACT_ME(Phalcon_ProfilerInterface, getTotalElapsedSeconds, NULL)
	PHP_ABSTRACT_ME(Phalcon_ProfilerInterface, setStartMemory, arginfo_phalcon_profiler_iteminterface_setstartmemory)
	PHP_ABSTRACT_ME(Phalcon_ProfilerInterface, getStartMemory, NULL)
	PHP_ABSTRACT_ME(Phalcon_ProfilerInterface, setEndMemory, arginfo_phalcon_profiler_iteminterface_setendmemory)
	PHP_ABSTRACT_ME(Phalcon_ProfilerInterface, getEndMemory, NULL)
	PHP_ABSTRACT_ME(Phalcon_ProfilerInterface, getTotalUsageMemory, NULL)
	PHP_FE_END
};

/**
 * Phalcon\Profiler\ItemInterface initializer
 */
PHALCON_INIT_CLASS(Phalcon_Profiler_ItemInterface){

	PHALCON_REGISTER_INTERFACE(Phalcon\\Profile, ItemInterface, profiler_iteminterface, phalcon_profiler_iteminterface_method_entry);

	return SUCCESS;
}

/**
 * Returns the name
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_Profiler_ItemInterface, getName);

/**
 * Sets the data related to the profile
 *
 * @param array $data
 */
PHALCON_DOC_METHOD(Phalcon_Profiler_ItemInterface, setData);

/**
 * Returns the data related to the profile
 *
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_Profiler_ItemInterface, getData);

/**
 * Sets the timestamp on when the profile started
 *
 * @param double $initialTime
 */
PHALCON_DOC_METHOD(Phalcon_Profiler_ItemInterface, setInitialTime);

/**
 * Returns the initial time in milseconds on when the profile started
 *
 * @return double
 */
PHALCON_DOC_METHOD(Phalcon_Profiler_ItemInterface, getInitialTime);

/**
 * Sets the timestamp on when the profile ended
 *
 * @param double $finalTime
 */
PHALCON_DOC_METHOD(Phalcon_Profiler_ItemInterface, setFinalTime);

/**
 * Returns the initial time in milseconds on when the profile ended
 *
 * @return double
 */
PHALCON_DOC_METHOD(Phalcon_Profiler_ItemInterface, getFinalTime);

/**
 * Returns the total time in seconds spent by the profile
 *
 * @return double
 */
PHALCON_DOC_METHOD(Phalcon_Profiler_ItemInterface, getTotalElapsedSeconds);

/**
 * Sets the amount of memory allocated on when the profile started
 *
 * @param int $startMemory
 */
PHALCON_DOC_METHOD(Phalcon_Profiler_ItemInterface, setStartMemory);

/**
 * Returns the amount of memory allocated on when the profile started
 *
 * @return int
 */
PHALCON_DOC_METHOD(Phalcon_Profiler_ItemInterface, getStartMemory);

/**
 * Sets the amount of memory allocated on when the profile ended
 *
 * @param int $endMemory
 */
PHALCON_DOC_METHOD(Phalcon_Profiler_ItemInterface, setEndMemory);

/**
 * Returns the the amount of memory allocated on when the profile ended
 *
 * @return int
 */
PHALCON_DOC_METHOD(Phalcon_Profiler_ItemInterface, getEndMemory);

/**
 * Returns the amount of memory allocated spent by the profile
 *
 * @return int
 */
PHALCON_DOC_METHOD(Phalcon_Profiler_ItemInterface, getTotalUsageMemory);
