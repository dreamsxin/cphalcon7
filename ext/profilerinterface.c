
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

#include "profilerinterface.h"
#include "kernel/main.h"

zend_class_entry *phalcon_profilerinterface_ce;

static const zend_function_entry phalcon_profilerinterface_method_entry[] = {
	PHP_ABSTRACT_ME(Phalcon_ProfilerInterface, startProfile, arginfo_phalcon_profilerinterface_startprofile)
	PHP_ABSTRACT_ME(Phalcon_ProfilerInterface, stopProfile, arginfo_phalcon_profilerinterface_stopprofile)
	PHP_ABSTRACT_ME(Phalcon_ProfilerInterface, getTotalElapsedSeconds, NULL)
	PHP_ABSTRACT_ME(Phalcon_ProfilerInterface, getTotalUsageMemory, NULL)
	PHP_ABSTRACT_ME(Phalcon_ProfilerInterface, getProfiles, NULL)
	PHP_ABSTRACT_ME(Phalcon_ProfilerInterface, getLastProfile, NULL)
	PHP_ABSTRACT_ME(Phalcon_ProfilerInterface, reset, NULL)
	PHP_FE_END
};

/**
 * Phalcon\ProfilerInterface initializer
 */
PHALCON_INIT_CLASS(Phalcon_ProfilerInterface){

	PHALCON_REGISTER_INTERFACE(Phalcon, ProfilerInterface, profilerinterface, phalcon_profilerinterface_method_entry);

	return SUCCESS;
}

/**
 * Starts the profile
 *
 * @param string $name
 * @param array $data
 * @return Phalcon\ProfilerInterface
 */
PHALCON_DOC_METHOD(Phalcon_ProfilerInterface, startProfile);

/**
 * Stops the active profile
 *
 * @param string $name
 * @return Phalcon\ProfilerInterface
 */
PHALCON_DOC_METHOD(Phalcon_ProfilerInterface, stopProfile);

/**
 * Returns the total time in seconds spent by the profiles
 *
 * @return double
 */
PHALCON_DOC_METHOD(Phalcon_ProfilerInterface, getTotalElapsedSeconds);

/**
 * Returns the amount of memory allocated spent by the profiles
 *
 * @return int
 */
PHALCON_DOC_METHOD(Phalcon_ProfilerInterface, getTotalUsageMemory);

/**
 * Returns all the processed profiles
 *
 * @return Phalcon\Profiler\ItemInterface[]
 */
PHALCON_DOC_METHOD(Phalcon_ProfilerInterface, getProfiles);

/**
 * Returns the last profile executed in the profiler
 *
 * @return Phalcon\Profiler\ItemInterface
 */
PHALCON_DOC_METHOD(Phalcon_ProfilerInterface, getLastProfile);

/**
 * Resets the profiler, cleaning up all the profiles
 *
 * @return Phalcon\ProfilerInterface
 */
PHALCON_DOC_METHOD(Phalcon_ProfilerInterface, reset);
