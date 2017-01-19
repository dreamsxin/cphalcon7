
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

#include "db/profiler.h"
#include "db/profiler/item.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/object.h"
#include "kernel/operators.h"

/**
 * Phalcon\Db\Profiler
 *
 * Instances of Phalcon\Db can generate execution profiles
 * on SQL statements sent to the relational database. Profiled
 * information includes execution time in miliseconds.
 * This helps you to identify bottlenecks in your applications.
 *
 *<code>
 *
 *	$profiler = new Phalcon\Db\Profiler();
 *
 *	//Set the connection profiler
 *	$connection->setProfiler($profiler);
 *
 *	$sql = "SELECT buyer_name, quantity, product_name
 *	FROM buyers LEFT JOIN products ON
 *	buyers.pid=products.id";
 *
 *	//Execute a SQL statement
 *	$connection->query($sql);
 *
 *	//Get the last profile in the profiler
 *	$profile = $profiler->getLastProfile();
 *
 *	echo "SQL Statement: ", $profile->getSQLStatement(), "\n";
 *	echo "Start Time: ", $profile->getInitialTime(), "\n";
 *	echo "Final Time: ", $profile->getFinalTime(), "\n";
 *	echo "Total Elapsed Time: ", $profile->getTotalElapsedSeconds(), "\n";
 *
 *</code>
 *
 */
zend_class_entry *phalcon_db_profiler_ce;

PHP_METHOD(Phalcon_Db_Profiler, startProfile);
PHP_METHOD(Phalcon_Db_Profiler, stopProfile);
PHP_METHOD(Phalcon_Db_Profiler, getNumberTotalStatements);
PHP_METHOD(Phalcon_Db_Profiler, getTotalElapsedSeconds);
PHP_METHOD(Phalcon_Db_Profiler, getProfiles);
PHP_METHOD(Phalcon_Db_Profiler, reset);
PHP_METHOD(Phalcon_Db_Profiler, getLastProfile);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_profiler_startprofile, 0, 0, 1)
	ZEND_ARG_INFO(0, sqlStatement)
	ZEND_ARG_INFO(0, sqlVariables)
	ZEND_ARG_INFO(0, sqlBindTypes)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_db_profiler_method_entry[] = {
	PHP_ME(Phalcon_Db_Profiler, startProfile, arginfo_phalcon_db_profiler_startprofile, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Profiler, stopProfile, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Profiler, getNumberTotalStatements, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Profiler, getTotalElapsedSeconds, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Profiler, getProfiles, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Profiler, reset, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Profiler, getLastProfile, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Db\Profiler initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Profiler){

	PHALCON_REGISTER_CLASS(Phalcon\\Db, Profiler, db_profiler, phalcon_db_profiler_method_entry, 0);

	zend_declare_property_null(phalcon_db_profiler_ce, SL("_allProfiles"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_profiler_ce, SL("_activeProfile"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_db_profiler_ce, SL("_totalSeconds"), 0, ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Starts the profile of a SQL sentence
 *
 * @param string $sqlStatement
 * @param $sqlVariables
 * @param $sqlBindTypes
 * @return Phalcon\Db\Profiler
 */
PHP_METHOD(Phalcon_Db_Profiler, startProfile){

	zval *sql_statement, *sql_variables = NULL, *sql_bindtypes = NULL, active_profile = {}, time = {};

	phalcon_fetch_params(0, 1, 2, &sql_statement, &sql_variables, &sql_bindtypes);

	object_init_ex(&active_profile, phalcon_db_profiler_item_ce);
	PHALCON_CALL_METHOD(NULL, &active_profile, "setsqlstatement", sql_statement);

	if (sql_variables) {
	    PHALCON_CALL_METHOD(NULL, &active_profile, "setsqlvariables", sql_variables);
	}

	if (sql_bindtypes) {
	    PHALCON_CALL_METHOD(NULL, &active_profile, "setsqlbindtypes", sql_bindtypes);
	}

	PHALCON_CALL_FUNCTION(&time, "microtime", &PHALCON_GLOBAL(z_true));
	PHALCON_CALL_METHOD(NULL, &active_profile, "setinitialtime", &time);

	if (phalcon_method_exists_ex(getThis(), SL("beforestartprofile")) == SUCCESS) {
		PHALCON_CALL_METHOD(NULL, getThis(), "beforestartprofile", &active_profile);
	}

	phalcon_update_property_zval(getThis(), SL("_activeProfile"), &active_profile);

	RETURN_THIS();
}

/**
 * Stops the active profile
 *
 * @return Phalcon\Db\Profiler
 */
PHP_METHOD(Phalcon_Db_Profiler, stopProfile){

	zval active_profile = {}, final_time = {}, initial_time = {}, difference = {}, total_seconds = {}, new_total_seconds = {};

	phalcon_read_property(&active_profile, getThis(), SL("_activeProfile"), PH_NOISY);

	PHALCON_CALL_FUNCTION(&final_time, "microtime", &PHALCON_GLOBAL(z_true));
	PHALCON_CALL_METHOD(NULL, &active_profile, "setfinaltime", &final_time);

	PHALCON_CALL_METHOD(&initial_time, &active_profile, "getinitialtime");

	phalcon_sub_function(&difference, &final_time, &initial_time);

	phalcon_read_property(&total_seconds, getThis(), SL("_totalSeconds"), PH_NOISY);

	phalcon_add_function(&new_total_seconds, &total_seconds, &difference);

	phalcon_update_property_zval(getThis(), SL("_totalSeconds"), &new_total_seconds);
	phalcon_update_property_array_append(getThis(), SL("_allProfiles"), &active_profile);

	if (phalcon_method_exists_ex(getThis(), SL("afterendprofile")) == SUCCESS) {
		PHALCON_CALL_METHOD(NULL, getThis(), "afterendprofile", &active_profile);
	}

	RETURN_THIS();
}

/**
 * Returns the total number of SQL statements processed
 *
 * @return integer
 */
PHP_METHOD(Phalcon_Db_Profiler, getNumberTotalStatements){

	zval all_profiles = {};
	phalcon_read_property(&all_profiles, getThis(), SL("_allProfiles"), PH_NOISY);
	phalcon_fast_count(return_value, &all_profiles);
}

/**
 * Returns the total time in seconds spent by the profiles
 *
 * @return double
 */
PHP_METHOD(Phalcon_Db_Profiler, getTotalElapsedSeconds){


	RETURN_MEMBER(getThis(), "_totalSeconds");
}

/**
 * Returns all the processed profiles
 *
 * @return Phalcon\Db\Profiler\Item[]
 */
PHP_METHOD(Phalcon_Db_Profiler, getProfiles){


	RETURN_MEMBER(getThis(), "_allProfiles");
}

/**
 * Resets the profiler, cleaning up all the profiles
 *
 * @return Phalcon\Db\Profiler
 */
PHP_METHOD(Phalcon_Db_Profiler, reset){

	phalcon_update_property_empty_array(getThis(), SL("_allProfiles"));
	RETURN_THIS();
}

/**
 * Returns the last profile executed in the profiler
 *
 * @return Phalcon\Db\Profiler\Item
 */
PHP_METHOD(Phalcon_Db_Profiler, getLastProfile){


	RETURN_MEMBER(getThis(), "_activeProfile");
}
