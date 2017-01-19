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

#include <ext/date/php_date.h>

#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/hash.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/output.h"

#include "internal/arginfo.h"


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
PHP_METHOD(Phalcon_Date_DateTime, startOfDecade);
PHP_METHOD(Phalcon_Date_DateTime, endOfDecade);
PHP_METHOD(Phalcon_Date_DateTime, startOfCentury);
PHP_METHOD(Phalcon_Date_DateTime, endOfCentury);
PHP_METHOD(Phalcon_Date_DateTime, modifyYear);
PHP_METHOD(Phalcon_Date_DateTime, modifyQuarter);
PHP_METHOD(Phalcon_Date_DateTime, modifyMonth);
PHP_METHOD(Phalcon_Date_DateTime, modifyDay);
PHP_METHOD(Phalcon_Date_DateTime, modifyHour);
PHP_METHOD(Phalcon_Date_DateTime, modifyMinute);
PHP_METHOD(Phalcon_Date_DateTime, modifySecond);
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_datetime_modifyyear, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, year, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_datetime_modifyquarter, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, quarter, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_datetime_modifymonth, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, month, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_datetime_modifyday, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, day, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_datetime_modifyhour, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, hour, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_datetime_modifyminute, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, minute, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_datetime_modifysecond, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, second, IS_LONG, 0)
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
	PHP_ME(Phalcon_Date_DateTime, startOfDecade, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Date_DateTime, endOfDecade, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Date_DateTime, startOfCentury, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Date_DateTime, endOfCentury, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Date_DateTime, modifyYear, arginfo_phalcon_date_datetime_modifyyear, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Date_DateTime, modifyQuarter, arginfo_phalcon_date_datetime_modifyquarter, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Date_DateTime, modifyMonth, arginfo_phalcon_date_datetime_modifymonth, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Date_DateTime, modifyDay, arginfo_phalcon_date_datetime_modifyday, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Date_DateTime, modifyHour, arginfo_phalcon_date_datetime_modifyhour, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Date_DateTime, modifyMinute, arginfo_phalcon_date_datetime_modifyminute, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Date_DateTime, modifySecond, arginfo_phalcon_date_datetime_modifysecond, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Date_DateTime, __get, arginfo___get, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Date\DateTime initializer
 */
PHALCON_INIT_CLASS(Phalcon_Date_DateTime){

	zend_class_entry *datetime_ce;

	datetime_ce = php_date_get_date_ce();

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

	PHALCON_CALL_CE_STATIC(&timezone, phalcon_date_ce, "createdatetimezone", tz);
	PHALCON_CALL_PARENT(NULL, phalcon_date_datetime_ce, getThis(), "__construct", time, &timezone);
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

	PHALCON_CALL_METHOD(&datetime, getThis(), "setdate", year, month, day);
	PHALCON_CALL_METHOD(return_value, &datetime, "settime", hour, minute, second);
}

/**
 * Resets the time to 00:00:00
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, startOfDay){

	PHALCON_CALL_METHOD(return_value, getThis(), "settime", &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));
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

	PHALCON_CALL_METHOD(return_value, getThis(), "settime", &hour, &minute, &second);
}

/**
 * Resets the date to the first day of the month and the time to 00:00:00
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, startOfMonth){

	zval p = {}, year = {}, month = {};

	ZVAL_STRING(&p, "year");
	PHALCON_CALL_METHOD(&year, getThis(), "__get", &p);

	ZVAL_STRING(&p, "month");
	PHALCON_CALL_METHOD(&month, getThis(), "__get", &p);

	PHALCON_CALL_METHOD(return_value, getThis(), "setdatetime", &year, &month, &PHALCON_GLOBAL(z_one), &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));
}

/**
 * Resets the date to end of the month and time to 23:59:59
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, endOfMonth){

	zval p = {}, year = {}, month = {}, daysinmonth = {}, hour = {}, minute = {}, second = {};

	ZVAL_STRING(&p, "year");
	PHALCON_CALL_METHOD(&year, getThis(), "__get", &p);

	ZVAL_STRING(&p, "month");
	PHALCON_CALL_METHOD(&month, getThis(), "__get", &p);

	ZVAL_STRING(&p, "daysInMonth");
	PHALCON_CALL_METHOD(&daysinmonth, getThis(), "__get", &p);

	ZVAL_LONG(&hour, 23);
	ZVAL_LONG(&minute, 59);
	ZVAL_LONG(&second, 59);

	PHALCON_CALL_METHOD(return_value, getThis(), "setdatetime", &year, &month, &daysinmonth, &hour, &minute, &second);
}

/**
 * Resets the date to the first day of the quarter and the time to 00:00:00
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, startOfQuarter){

	zval p = {}, year = {}, quarter = {}, month = {};

	ZVAL_STRING(&p, "year");
	PHALCON_CALL_METHOD(&year, getThis(), "__get", &p);

	ZVAL_STRING(&p, "quarter");
	PHALCON_CALL_METHOD(&quarter, getThis(), "__get", &p);

	ZVAL_LONG(&month, ((Z_LVAL(quarter) - 1) * PHALCON_MONTHS_PER_QUARTER + 1));

	PHALCON_CALL_METHOD(return_value, getThis(), "setdatetime", &year, &month, &PHALCON_GLOBAL(z_one), &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));
}

/**
 * Resets the date to end of the quarter and time to 23:59:59
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, endOfQuarter){

	zval p = {}, year = {}, quarter = {}, month = {}, daysinmonth = {}, hour = {}, minute = {}, second = {};

	ZVAL_STRING(&p, "year");
	PHALCON_CALL_METHOD(&year, getThis(), "__get", &p);

	ZVAL_STRING(&p, "quarter");
	PHALCON_CALL_METHOD(&quarter, getThis(), "__get", &p);

	ZVAL_LONG(&month, (Z_LVAL(quarter) * PHALCON_MONTHS_PER_QUARTER));

	ZVAL_STRING(&p, "daysInMonth");
	PHALCON_CALL_METHOD(&daysinmonth, getThis(), "__get", &p);

	ZVAL_LONG(&hour, 23);
	ZVAL_LONG(&minute, 59);
	ZVAL_LONG(&second, 59);

	PHALCON_CALL_METHOD(return_value, getThis(), "setdatetime", &year, &month, &daysinmonth, &hour, &minute, &second);
}

/**
 * Resets the date to the first day of the year and the time to 00:00:00
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, startOfYear){

	zval p = {}, year = {};

	ZVAL_STRING(&p, "year");
	PHALCON_CALL_METHOD(&year, getThis(), "__get", &p);

	PHALCON_CALL_METHOD(return_value, getThis(), "setdatetime", &year, &PHALCON_GLOBAL(z_one), &PHALCON_GLOBAL(z_one), &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));
}

/**
 * Resets the date to end of the year and time to 23:59:59
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, endOfYear){

	zval p = {}, year = {}, month = {}, day = {}, hour = {}, minute = {}, second = {};

	ZVAL_STRING(&p, "year");
	PHALCON_CALL_METHOD(&year, getThis(), "__get", &p);

	ZVAL_LONG(&month, 12);
	ZVAL_LONG(&day, 31);

	ZVAL_LONG(&hour, 23);
	ZVAL_LONG(&minute, 59);
	ZVAL_LONG(&second, 59);

	PHALCON_CALL_METHOD(return_value, getThis(), "setdatetime", &year, &month, &day, &hour, &minute, &second);
}

/**
 * Resets the date to the first day of the year and the time to 00:00:00
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, startOfDecade){

	zval p = {}, year = {}, decade = {};

	ZVAL_STRING(&p, "year");
	PHALCON_CALL_METHOD(&year, getThis(), "__get", &p);

	ZVAL_LONG(&decade, (Z_LVAL(year) - (Z_LVAL(year) - 1) % PHALCON_YEARS_PER_DECADE));

	PHALCON_CALL_METHOD(return_value, getThis(), "setdatetime", &decade, &PHALCON_GLOBAL(z_one), &PHALCON_GLOBAL(z_one), &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));
}

/**
 * Resets the date to end of the decade and time to 23:59:59
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, endOfDecade){

	zval p = {}, year = {}, decade = {}, month = {}, day = {}, hour = {}, minute = {}, second = {};

	ZVAL_STRING(&p, "year");
	PHALCON_CALL_METHOD(&year, getThis(), "__get", &p);

	ZVAL_LONG(&decade, (Z_LVAL(year) - (Z_LVAL(year) - 1) % PHALCON_YEARS_PER_DECADE + PHALCON_YEARS_PER_DECADE - 1));

	ZVAL_LONG(&month, 12);
	ZVAL_LONG(&day, 31);

	ZVAL_LONG(&hour, 23);
	ZVAL_LONG(&minute, 59);
	ZVAL_LONG(&second, 59);

	PHALCON_CALL_METHOD(return_value, getThis(), "setdatetime", &decade, &month, &day, &hour, &minute, &second);
}

/**
 * Resets the date to the first day of the century and the time to 00:00:00
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, startOfCentury){

	zval p = {}, year = {}, century = {};

	ZVAL_STRING(&p, "year");
	PHALCON_CALL_METHOD(&year, getThis(), "__get", &p);

	ZVAL_LONG(&century, (Z_LVAL(year) - (Z_LVAL(year) - 1) % PHALCON_YEARS_PER_CENTURY));

	PHALCON_CALL_METHOD(return_value, getThis(), "setdatetime", &century, &PHALCON_GLOBAL(z_one), &PHALCON_GLOBAL(z_one), &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero), &PHALCON_GLOBAL(z_zero));
}

/**
 * Resets the date to end of the century and time to 23:59:59
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, endOfCentury){

	zval p = {}, year = {}, century = {}, month = {}, day = {}, hour = {}, minute = {}, second = {};

	ZVAL_STRING(&p, "year");
	PHALCON_CALL_METHOD(&year, getThis(), "__get", &p);

	ZVAL_LONG(&century, (Z_LVAL(year) - (Z_LVAL(year) - 1) % PHALCON_YEARS_PER_CENTURY + PHALCON_YEARS_PER_CENTURY - 1));

	ZVAL_LONG(&month, 12);
	ZVAL_LONG(&day, 31);

	ZVAL_LONG(&hour, 23);
	ZVAL_LONG(&minute, 59);
	ZVAL_LONG(&second, 59);

	PHALCON_CALL_METHOD(return_value, getThis(), "setdatetime", &century, &month, &day, &hour, &minute, &second);
}

/**
 * Add or Remove years from the instance
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, modifyYear){

	zval *year, v = {};

	phalcon_fetch_params(0, 1, 0, &year);

	PHALCON_CONCAT_VS(&v, year, " year");

	PHALCON_CALL_METHOD(NULL, getThis(), "modify", &v);

	RETURN_THIS();
}

/**
 * Add or Remove quarters from the instance
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, modifyQuarter){

	zval *quarter, month = {}, v = {};

	phalcon_fetch_params(0, 1, 0, &quarter);

	ZVAL_LONG(&month, Z_LVAL_P(quarter) * PHALCON_MONTHS_PER_QUARTER);
	PHALCON_CONCAT_VS(&v, &month, " month");

	PHALCON_CALL_METHOD(NULL, getThis(), "modify", &v);

	RETURN_THIS();
}

/**
 * Add or Remove months from the instance
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, modifyMonth){

	zval *month, v = {};

	phalcon_fetch_params(0, 1, 0, &month);

	PHALCON_CONCAT_VS(&v, month, " month");

	PHALCON_CALL_METHOD(NULL, getThis(), "modify", &v);

	RETURN_THIS();
}

/**
 * Add or Remove days from the instance
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, modifyDay){

	zval *day, v = {};

	phalcon_fetch_params(0, 1, 0, &day);

	PHALCON_CONCAT_VS(&v, day, " day");

	PHALCON_CALL_METHOD(NULL, getThis(), "modify", &v);

	RETURN_THIS();
}

/**
 * Add or Remove hours from the instance
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, modifyHour){

	zval *hour, v = {};

	phalcon_fetch_params(0, 1, 0, &hour);

	PHALCON_CONCAT_VS(&v, hour, " hour");

	PHALCON_CALL_METHOD(NULL, getThis(), "modify", &v);

	RETURN_THIS();
}

/**
 * Add or Remove minutes from the instance
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, modifyMinute){

	zval *minute, v = {};

	phalcon_fetch_params(0, 1, 0, &minute);

	PHALCON_CONCAT_VS(&v, minute, " minute");

	PHALCON_CALL_METHOD(NULL, getThis(), "modify", &v);

	RETURN_THIS();
}

/**
 * Add or Remove seconds from the instance
 *
 * @return \DateTime
 */
PHP_METHOD(Phalcon_Date_DateTime, modifySecond){

	zval *second, v = {};

	phalcon_fetch_params(0, 1, 0, &second);

	PHALCON_CONCAT_VS(&v, second, " second");

	PHALCON_CALL_METHOD(NULL, getThis(), "modify", &v);

	RETURN_THIS();
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

	zval *property, lower_property = {}, format = {}, p = {}, v = {};

	phalcon_fetch_params(0, 1, 0, &property);
	PHALCON_ENSURE_IS_STRING(property);

	phalcon_fast_strtolower(&lower_property, property);

	if (PHALCON_IS_STRING(&lower_property, "year")) {
		ZVAL_STRING(&format, "Y");
		PHALCON_CALL_METHOD(return_value, getThis(), "format", &format);
		convert_to_long(return_value);
		return;
	} else if (PHALCON_IS_STRING(&lower_property, "month")) {
		ZVAL_STRING(&format, "n");
		PHALCON_CALL_METHOD(return_value, getThis(), "format", &format);
		convert_to_long(return_value);
		return;
	} else if (PHALCON_IS_STRING(&lower_property, "day")) {
		ZVAL_STRING(&format, "i");
		PHALCON_CALL_METHOD(return_value, getThis(), "format", &format);
		convert_to_long(return_value);
		return;
	} else if (PHALCON_IS_STRING(&lower_property, "hour")) {
		ZVAL_STRING(&format, "G");
		PHALCON_CALL_METHOD(return_value, getThis(), "format", &format);
		convert_to_long(return_value);
		return;
	} else if (PHALCON_IS_STRING(&lower_property, "minute")) {
		ZVAL_STRING(&format, "i");
		PHALCON_CALL_METHOD(return_value, getThis(), "format", &format);
		convert_to_long(return_value);
		return;
	} else if (PHALCON_IS_STRING(&lower_property, "second")) {
		ZVAL_STRING(&format, "s");
		PHALCON_CALL_METHOD(return_value, getThis(), "format", &format);
		convert_to_long(return_value);
		return;
	} else if (PHALCON_IS_STRING(&lower_property, "micro")) {
		ZVAL_STRING(&format, "u");
		PHALCON_CALL_METHOD(return_value, getThis(), "format", &format);
		convert_to_long(return_value);
		return;
	} else if (PHALCON_IS_STRING(&lower_property, "dayofweek")) {
		ZVAL_STRING(&format, "w");
		PHALCON_CALL_METHOD(return_value, getThis(), "format", &format);
		convert_to_long(return_value);
		return;
	} else if (PHALCON_IS_STRING(&lower_property, "dayofyear")) {
		ZVAL_STRING(&format, "z");
		PHALCON_CALL_METHOD(return_value, getThis(), "format", &format);
		convert_to_long(return_value);
		return;
	} else if (PHALCON_IS_STRING(&lower_property, "weekofyear")) {
		ZVAL_STRING(&format, "W");
		PHALCON_CALL_METHOD(return_value, getThis(), "format", &format);
		convert_to_long(return_value);
		return;
	} else if (PHALCON_IS_STRING(&lower_property, "daysinmonth")) {
		ZVAL_STRING(&format, "t");
		PHALCON_CALL_METHOD(return_value, getThis(), "format", &format);
		convert_to_long(return_value);
		return;
	} else if (PHALCON_IS_STRING(&lower_property, "timestamp")) {
		ZVAL_STRING(&format, "U");
		PHALCON_CALL_METHOD(return_value, getThis(), "format", &format);
		convert_to_long(return_value);
		return;
	} else if (PHALCON_IS_STRING(&lower_property, "weekofmonth")) {
		ZVAL_STRING(&p, "day");
		PHALCON_CALL_METHOD(&v, getThis(), "__get", &p);
		RETURN_LONG(ceil((float)Z_LVAL(v) / PHALCON_DAYS_PER_WEEK));
	} else if (PHALCON_IS_STRING(&lower_property, "quarter")) {
		ZVAL_STRING(&p, "month");
		PHALCON_CALL_METHOD(&v, getThis(), "__get", &p);
		RETURN_LONG(ceil((float)Z_LVAL(v) / PHALCON_MONTHS_PER_QUARTER));
	} else if (PHALCON_IS_STRING(&lower_property, "offset")) {
		PHALCON_CALL_METHOD(return_value, getThis(), "getoffset");
	} else if (PHALCON_IS_STRING(&lower_property, "offsethours")) {
		PHALCON_CALL_METHOD(&v, getThis(), "getoffset");
		RETURN_LONG(Z_LVAL(v) / PHALCON_SECONDS_PER_MINUTE / PHALCON_MINUTES_PER_HOUR);
	} else if (PHALCON_IS_STRING(&lower_property, "dst")) {
		ZVAL_STRING(&format, "I");
		PHALCON_CALL_METHOD(&v, getThis(), "format", &format);
		convert_to_long(&v);
		RETURN_BOOL(Z_LVAL(v));
	}
	PHALCON_THROW_EXCEPTION_FORMAT(spl_ce_InvalidArgumentException, "Unknown getter '%s'", Z_STRVAL_P(property));
	return;
}
