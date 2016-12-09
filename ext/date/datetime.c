/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2013 Phalcon Team (http://www.phalconphp.com)       |
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

#include "date/datetime.h"
#include "date.h"

#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/hash.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/output.h"


/**
 * Phalcon\Date\DateTime
 *
 *<code>
 * $datetime = new \Phalcon\Date\DateTime();
 * $start = $datetime->startOfDay();
 * $end = $datetime->endOfDay();
 *</code>
 */
zend_class_entry *phalcon_date_datetime_ce;

PHP_METHOD(Phalcon_Date_DateTime, __construct);
PHP_METHOD(Phalcon_Date_DateTime, setDateTime);
PHP_METHOD(Phalcon_Date_DateTime, startOfDay);
PHP_METHOD(Phalcon_Date_DateTime, endOfDay);
PHP_METHOD(Phalcon_Date_DateTime, startOfMonth);
PHP_METHOD(Phalcon_Date_DateTime, endOfMonth);
PHP_METHOD(Phalcon_Date_DateTime, startOfQuarter);
PHP_METHOD(Phalcon_Date_DateTime, endOfQuarter);
PHP_METHOD(Phalcon_Date_DateTime, startOfYear);
PHP_METHOD(Phalcon_Date_DateTime, endOfYear);
PHP_METHOD(Phalcon_Date_DateTime, __get);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_datetime___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, font)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_datetime_setdatetime, 0, 0, 5)
	ZEND_ARG_TYPE_INFO(0, year, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, month, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, day, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, hour, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, minute, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, second, IS_LONG, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_date_datetime_method_entry[] = {
	PHP_ME(Phalcon_Date_DateTime, __construct, arginfo_phalcon_date_datetime___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Date_DateTime, setDateTime, arginfo_phalcon_date_datetime_setdatetime, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Date_DateTime, startOfDay, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Date_DateTime, endOfDay, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Date_DateTime, startOfMonth, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Date_DateTime, endOfMonth, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Date_DateTime, startOfQuarter, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Date_DateTime, endOfQuarter, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Date_DateTime, startOfYear, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Date_DateTime, endOfYear, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Date_DateTime, __get, arginfo___get, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Date\DateTime initializer
 */
PHALCON_INIT_CLASS(Phalcon_Date_DateTime){

	zend_class_entry *datetime_ce;

	datetime_ce = phalcon_fetch_str_class(SL("DateTime"), ZEND_FETCH_CLASS_AUTO);

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Date, DateTime, date_datetime, datetime_ce, phalcon_date_datetime_method_entry, 0);

	return SUCCESS;
}

/**
 * Create a new instance
 *
 * @param string|null $time
 * @param \DateTimeZone|string|null $tz
 */
PHP_METHOD(Phalcon_Date_DateTime, __construct){

	zval *time = NULL, *tz = NULL, timezone = {};

	phalcon_fetch_params(0, 0, 2, &time, &tz);

	if (!time) {
		time = &PHALCON_GLOBAL(z_null);
	}

	if (!tz) {
		tz = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_CE_STATICW(&timezone, phalcon_date_ce, "createdatetimezone", tz);
	PHALCON_CALL_PARENTW(NULL, phalcon_date_datetime_ce, getThis(), "__construct", time, &timezone);
}

/**
 * Set the date and time all together
 *
 * @param int $year
 * @param int $month
 * @param int $day
 * @param int $hour
 * @param int $minute
 * @param int $second
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, setDateTime){

	zval *year, *month, *day, *hour, *minute, *second = NULL, datetime = {};

	phalcon_fetch_params(0, 5, 1, &year, &month, &day, &hour, &minute, &second);

	if (!second) {
		second = &PHALCON_GLOBAL(z_zero);
	}

	PHALCON_CALL_METHODW(&datetime, getThis(), "setdate", year, month, day);
	PHALCON_CALL_METHODW(return_value, datetime, "settime", hour, minute, second);
}

/**
 * Resets the time to 00:00:00
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, startOfDay){

	PHALCON_CALL_METHODW(return_value, getThis(), "settime", &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));
}

/**
 * Resets the time to 23:59:59
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, endOfDay){

	zval hour = {}, minute = {}, second = {};

	ZVAL_LONG(&hour, 23);
	ZVAL_LONG(&minute, 59);
	ZVAL_LONG(&second, 59);

	PHALCON_CALL_METHODW(return_value, getThis(), "settime", &hour, &minute, &second);
}

/**
 * Resets the date to the first day of the month and the time to 00:00:00
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, endOfDay){

	zval hour = {}, minute = {}, second = {};

	ZVAL_LONG(&year, 23);
	ZVAL_LONG(&month, 59);
	ZVAL_LONG(&day, 59);

	PHALCON_CALL_METHODW(return_value, getThis(), "setdatetime", &year, &month, &day, &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));
}

/**
 * Get a part of the Carbon object
 *
 * @param string $name
 *
 * @throws \InvalidArgumentException
 *
 * @return string|int|\DateTimeZone
 */
PHP_METHOD(Phalcon_Date_DateTime, __get){

}

