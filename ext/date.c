
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

#include "date.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/string.h"
#include "kernel/array.h"
#include "kernel/operators.h"
#include "kernel/concat.h"
#include "kernel/exception.h"

/**
 * Phalcon\Date
 *
 * Provides utilities to work with dates
 */
zend_class_entry *phalcon_date_ce;

PHP_METHOD(Phalcon_Date, offset);
PHP_METHOD(Phalcon_Date, seconds);
PHP_METHOD(Phalcon_Date, minutes);
PHP_METHOD(Phalcon_Date, hours);
PHP_METHOD(Phalcon_Date, ampm);
PHP_METHOD(Phalcon_Date, adjust);
PHP_METHOD(Phalcon_Date, days);
PHP_METHOD(Phalcon_Date, months);
PHP_METHOD(Phalcon_Date, years);
PHP_METHOD(Phalcon_Date, span);
PHP_METHOD(Phalcon_Date, span2);
PHP_METHOD(Phalcon_Date, fuzzy_span);
PHP_METHOD(Phalcon_Date, fuzzy_span2);
PHP_METHOD(Phalcon_Date, unix2dos);
PHP_METHOD(Phalcon_Date, dos2unix);
PHP_METHOD(Phalcon_Date, formatted_time);
PHP_METHOD(Phalcon_Date, intervalToSeconds);
PHP_METHOD(Phalcon_Date, createDateTimeZone);
PHP_METHOD(Phalcon_Date, filter);
PHP_METHOD(Phalcon_Date, valid);
PHP_METHOD(Phalcon_Date, diff);
PHP_METHOD(Phalcon_Date, add);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_offset, 0, 0, 1)
	ZEND_ARG_INFO(0, remote)
	ZEND_ARG_INFO(0, local)
	ZEND_ARG_INFO(0, now)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_seconds, 0, 0, 0)
	ZEND_ARG_INFO(0, step)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, end)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_minutes, 0, 0, 0)
	ZEND_ARG_INFO(0, step)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_hours, 0, 0, 0)
	ZEND_ARG_INFO(0, step)
	ZEND_ARG_INFO(0, long)
	ZEND_ARG_INFO(0, start)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_ampm, 0, 0, 1)
	ZEND_ARG_INFO(0, hour)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_adjust, 0, 0, 2)
	ZEND_ARG_INFO(0, hour)
	ZEND_ARG_INFO(0, ampm)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_days, 0, 0, 1)
	ZEND_ARG_INFO(0, month)
	ZEND_ARG_INFO(0, year)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_months, 0, 0, 0)
	ZEND_ARG_INFO(0, format)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_years, 0, 0, 0)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, end)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_span, 0, 0, 1)
	ZEND_ARG_INFO(0, remote)
	ZEND_ARG_INFO(0, local)
	ZEND_ARG_INFO(0, output)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_span2, 0, 0, 1)
	ZEND_ARG_INFO(0, remote)
	ZEND_ARG_INFO(0, output)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_fuzzy_span, 0, 0, 1)
	ZEND_ARG_INFO(0, timestamp)
	ZEND_ARG_INFO(0, local_timestamp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_fuzzy_span2, 0, 0, 1)
	ZEND_ARG_INFO(0, timestamp)
	ZEND_ARG_INFO(0, output)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_unix2dos, 0, 0, 0)
	ZEND_ARG_INFO(0, timestamp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_dos2unix, 0, 0, 0)
	ZEND_ARG_INFO(0, timestamp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_formatted_time, 0, 0, 0)
	ZEND_ARG_INFO(0, datetime_str)
	ZEND_ARG_INFO(0, timestamp_format)
	ZEND_ARG_INFO(0, timezone)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_intervaltoseconds, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, interval, DateInterval, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_createdatetimezone, 0, 0, 0)
	ZEND_ARG_INFO(0, timezone)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_filter, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, date, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, format, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, delimiter, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_valid, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, date, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, format, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_diff, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, date1, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, date2, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, diffFormat, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, format, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_date_add, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, date1, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, interval, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, format, IS_STRING, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_date_method_entry[] = {
	PHP_ME(Phalcon_Date, offset, arginfo_phalcon_date_offset, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Date, seconds, arginfo_phalcon_date_seconds, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Date, minutes, arginfo_phalcon_date_minutes, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Date, hours, arginfo_phalcon_date_hours, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Date, adjust, arginfo_phalcon_date_adjust, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Date, ampm, arginfo_phalcon_date_ampm, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Date, days, arginfo_phalcon_date_days, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Date, months, arginfo_phalcon_date_months, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Date, years, arginfo_phalcon_date_years, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Date, span, arginfo_phalcon_date_span, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Date, span2, arginfo_phalcon_date_span2, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Date, fuzzy_span, arginfo_phalcon_date_fuzzy_span, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Date, fuzzy_span2, arginfo_phalcon_date_fuzzy_span2, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Date, unix2dos, arginfo_phalcon_date_unix2dos, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Date, dos2unix, arginfo_phalcon_date_dos2unix, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Date, formatted_time, arginfo_phalcon_date_formatted_time, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Date, intervalToSeconds, arginfo_phalcon_date_intervaltoseconds, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Date, createDateTimeZone, arginfo_phalcon_date_createdatetimezone, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Date, filter, arginfo_phalcon_date_filter, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Date, valid, arginfo_phalcon_date_valid, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Date, diff, arginfo_phalcon_date_diff, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Phalcon_Date, add, arginfo_phalcon_date_add, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 * Phalcon\Date initializer
 */
PHALCON_INIT_CLASS(Phalcon_Date){

	PHALCON_REGISTER_CLASS(Phalcon, Date, date, phalcon_date_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_class_constant_long(phalcon_date_ce, SL("YEAR"),   PHALCON_DATE_YEAR);
	zend_declare_class_constant_long(phalcon_date_ce, SL("MONTH"),  PHALCON_DATE_MONTH);
	zend_declare_class_constant_long(phalcon_date_ce, SL("WEEK"),   PHALCON_DATE_WEEK);
	zend_declare_class_constant_long(phalcon_date_ce, SL("DAY"),    PHALCON_DATE_DAY);
	zend_declare_class_constant_long(phalcon_date_ce, SL("HOUR"),   PHALCON_DATE_HOUR);
	zend_declare_class_constant_long(phalcon_date_ce, SL("MINUTE"), PHALCON_DATE_MINUTE);

	zend_declare_class_constant_stringl(phalcon_date_ce, SL("MONTHS_LONG"),  SL(PHALCON_DATE_MONTHS_LONG));
	zend_declare_class_constant_stringl(phalcon_date_ce, SL("MONTHS_SHORT"), SL(PHALCON_DATE_MONTHS_SHORT));

	zend_declare_property_string(phalcon_date_ce, SL("timestamp_format"), "Y-m-d H:i:s", ZEND_ACC_PUBLIC|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_date_ce, SL("timezone"), ZEND_ACC_PUBLIC|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_date_ce, SL("_months"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);

	return SUCCESS;
}

/**
 * Returns the offset (in seconds) between two time zones. Use this to
 * display dates to users in different time zones.
 *
 *     $seconds = Phalcon\Date::offset('America/Chicago', 'GMT');
 *
 * @param string $remote
 * @param string $local
 * @param mixed $date
 * @return int
 */
PHP_METHOD(Phalcon_Date, offset){

	zval *remote, *local = NULL, *date = NULL, new_date = {}, format = {}, zone_remote = {}, time_remote = {}, zone_local = {}, time_local = {}, offset_remote = {}, offset_local = {};
	zend_class_entry *ce0, *ce1;

	phalcon_fetch_params(0, 1, 2, &remote, &local, &date);

	ce0 = phalcon_fetch_str_class(SL("DateTime"), ZEND_FETCH_CLASS_AUTO);
	ce1 = phalcon_fetch_str_class(SL("DateTimeZone"), ZEND_FETCH_CLASS_AUTO);

	if (!date) {
		ZVAL_STRING(&new_date, "now");
	} else if (Z_TYPE_P(date) == IS_LONG) {
		phalcon_get_class_constant(&format, ce0, SL("RFC2822"));

		PHALCON_CALL_FUNCTION(&new_date, "date", &format, date);
	} else {
		ZVAL_COPY_VALUE(&new_date, date);
	}

	object_init_ex(&zone_remote, ce1);
	PHALCON_CALL_METHOD(NULL, &zone_remote, "__construct", remote);

	object_init_ex(&time_remote, ce0);
	PHALCON_CALL_METHOD(NULL, &time_remote, "__construct", &new_date, &zone_remote);

	if (local) {
		object_init_ex(&zone_local, ce1);
		PHALCON_CALL_METHOD(NULL, &zone_local, "__construct", local);

		object_init_ex(&time_local, ce0);
		PHALCON_CALL_METHOD(NULL, &time_local, "__construct", &new_date, &zone_local);
	} else {
		object_init_ex(&time_local, ce0);
		PHALCON_CALL_METHOD(NULL, &time_local, "__construct", &new_date);
	}

	PHALCON_CALL_METHOD(&offset_remote, &zone_remote, "getOffset", &time_remote);
	PHALCON_CALL_METHOD(&offset_local, &zone_local, "getOffset", &time_local);

	ZVAL_LONG(return_value, phalcon_get_intval(&offset_remote)-phalcon_get_intval(&offset_local));
}

/**
 * Number of seconds in a minute, incrementing by a step. Typically used as
 * a shortcut for generating a list that can used in a form.
 *
 *     $seconds = Phalcon\Date::seconds(); // 00, 01, 02, 03, ..., 58, 59
 *
 * @param int $step
 * @param int $start
 * @param int $end
 * @return array
 */
PHP_METHOD(Phalcon_Date, seconds){

	zval *step = NULL, *start = NULL, *end = NULL;
	char buf[2];
	int i, p, s, e;

	phalcon_fetch_params(0, 0, 3, &step, &start, &end);

	if (!step) {
		p = 1;
	} else {
		p = phalcon_get_intval(step);
		if (p < 1) {
			p = 1;
		}
	}

	if (!start) {
		s = 0;
	} else {
		s = phalcon_get_intval(start);
	}

	if (!end) {
		e = 60;
	} else {
		e = phalcon_get_intval(end);
	}

	array_init(return_value);

	for (i = s; i < e; i += p) {
		sprintf(buf, "%02d", i);
		phalcon_array_update_long_string(return_value, i, buf, 2, PH_COPY);
	}
}

/**
 * Number of minutes in an hour, incrementing by a step. Typically used as
 * a shortcut for generating a list that can be used in a form.
 *
 *     $minutes = Phalcon\Date::minutes(); // 00, 05, 10, 15, ..., 50, 55
 *
 * @param int $step
 * @return array
 */
PHP_METHOD(Phalcon_Date, minutes){

	zval *step = NULL, s = {};

	phalcon_fetch_params(0, 0, 1, &step);

	if (!step) {
		ZVAL_LONG(&s, 5);
		PHALCON_RETURN_CALL_SELF("seconds", &s);
	} else {
		PHALCON_RETURN_CALL_SELF("seconds", step);
	}
}

/**
 * Number of hours in a day. Typically used as a shortcut for generating a
 * list that can be used in a form.
 *
 *     $hours = Phalcon\Date::hours(); // 01, 02, 03, ..., 10, 11, 12
 *
 * @param int $step
 * @param boolean $is_long
 * @param int $start
 * @return array
 */
PHP_METHOD(Phalcon_Date, hours){

	zval *step = NULL, *is_long = NULL, *start = NULL;
	char buf[2];
	int i, p, s, e;

	phalcon_fetch_params(0, 0, 3, &step, &is_long, &start);

	if (!step) {
		p = 1;
	} else {
		p = phalcon_get_intval(step);
		if (p < 1) {
			p = 1;
		}
	}

	if (!start || Z_TYPE_P(start) == IS_NULL) {
		if (is_long && zend_is_true(is_long)) {
			s = 0;
		} else {
			s = 1;
		}
	} else {
		s = phalcon_get_intval(start);
	}

	e = is_long && zend_is_true(is_long) ? 23 : 12;

	array_init(return_value);

	for (i = s; i <= e; i += p) {
		sprintf(buf, "%02d", i);
		phalcon_array_update_long_string(return_value, i, buf, 2, PH_COPY);
	}
}

/**
 * Returns AM or PM, based on a given hour (in 24 hour format).
 *
 *     $type = Phalcon\Date::ampm(12); // PM
 *     $type = Phalcon\Date::ampm(1);  // AM
 *
 * @param int $hour
 * @return string
 */
PHP_METHOD(Phalcon_Date, ampm){

	zval *hour;

	phalcon_fetch_params(0, 1, 0, &hour);

	if (phalcon_get_intval(hour) > 11) {
		ZVAL_STRING(return_value, "PM");
	} else {
		ZVAL_STRING(return_value, "AM");
	}
}

/**
 * Adjusts a non-24-hour number into a 24-hour number.
 *
 *     $hour = Phalcon\Date::adjust(3, 'pm'); // 15
 *
 * @param int $hour
 * @param string $ampm
 * @return string
 */
PHP_METHOD(Phalcon_Date, adjust){

	zval *hour, *ampm, lower_ampm = {};
	char buf[2];
	int h;

	phalcon_fetch_params(0, 2, 0, &hour, &ampm);
	h = phalcon_get_intval(hour);

	phalcon_fast_strtolower(&lower_ampm, ampm);

	if (PHALCON_IS_STRING(&lower_ampm, "am")) {
		if (h == 12) {
			h = 0;
		}
	} else if (PHALCON_IS_STRING(&lower_ampm, "pm")) {
		if (h < 12) {
			h += 12;
		}
	}

	sprintf(buf, "%02d", h);

	ZVAL_STRING(return_value, buf);
}

/**
 * Number of days in a given month and year. Typically used as a shortcut
 * for generating a list that can be used in a form.
 *
 *     Phalcon\Date::days(4, 2010); // 1, 2, 3, ..., 28, 29, 30
 *
 * @param int $month
 * @param int $year
 * @return array
 */
PHP_METHOD(Phalcon_Date, days){

	zval *month, *year = NULL, year2 = {}, months = {}, year_months = {}, format = {}, total = {}, tmp = {}, tmp1 = {}, tmp2 = {};
	char buf[2];
	int y, m, i, t;

	phalcon_fetch_params(0, 1, 1, &month, &year);

	phalcon_read_static_property_ce(&months, phalcon_date_ce, SL("_months"), PH_READONLY);

	m = phalcon_get_intval(month);

	if (!year || PHALCON_IS_FALSE(year)) {
		ZVAL_STRING(&tmp, "Y");

		PHALCON_CALL_FUNCTION(&year2, "date", &tmp);

		y = phalcon_get_intval(&year2);
	} else {
		ZVAL_COPY_VALUE(&year2, year);
	}

	if (Z_TYPE(months) == IS_ARRAY) {
		if (phalcon_array_isset_fetch_long(&year_months, &months, y, PH_READONLY)) {
			if (phalcon_array_isset_fetch_long(return_value, &year_months, m, PH_COPY)) {
				return;
			}
		}
	} else {
		array_init(&months);
	}

	ZVAL_LONG(&tmp1, 1);
	ZVAL_LONG(&tmp2, 0);

	PHALCON_CALL_FUNCTION(&tmp, "mktime", &tmp1, &tmp2, &tmp2, month, &tmp1, &year2);

	ZVAL_STRING(&format, "t");

	PHALCON_CALL_FUNCTION(&total, "date", &format, &tmp);

	t = phalcon_get_intval(&total) + 1;

	array_init(&tmp);

	phalcon_array_update_long(&months, y, &tmp, PH_COPY);

	for (i = 1; i < t; i++) {
		sprintf(buf, "%02d", i);
		phalcon_array_update_long_string(&tmp, i, buf, 2, PH_COPY);
	}

	phalcon_array_update_long_long_multi_2(&months, y, m, &tmp, PH_COPY);

	phalcon_update_static_property_ce(phalcon_date_ce, SL("_months"), &months);

	RETURN_CTOR(&tmp);
}

/**
 * Number of months in a year. Typically used as a shortcut for generating
 * a list that can be used in a form.
 *
 * By default a mirrored array of $month_number => $month_number is returned
 *
 *     Phalcon\Date::months();
 *     // aray(1 => 1, 2 => 2, 3 => 3, ..., 12 => 12)
 *
 * But you can customise this by passing in either PHALCON_DATE_MONTHS_LONG
 *
 *     Phalcon\Date::months(Phalcon\Utils\PHALCON_DATE_MONTHS_LONG);
 *     // array(1 => 'January', 2 => 'February', ..., 12 => 'December')
 *
 * Or PHALCON_DATE_MONTHS_SHORT
 *
 *     Phalcon\Date::months(Phalcon\Date::PHALCON_DATE_MONTHS_SHORT);
 *     // array(1 => 'Jan', 2 => 'Feb', ..., 12 => 'Dec')
 *
 * @uses PHALCON_DATE_hours
 * @param string $format
 * @return array
 */
PHP_METHOD(Phalcon_Date, months){

	zval *format = NULL, tmp1 = {}, tmp2 = {};
	int i;

	phalcon_fetch_params(0, 0, 1, &format);

	if (format && (PHALCON_IS_STRING(format, PHALCON_DATE_MONTHS_LONG) || PHALCON_IS_STRING(format, PHALCON_DATE_MONTHS_SHORT))) {
		array_init(return_value);

		ZVAL_LONG(&tmp1, 0);
		ZVAL_LONG(&tmp2, 1);

		for (i = 1; i <= 12; ++i) {
			zval tmp3 = {}, time = {}, strftime = {};
			ZVAL_LONG(&tmp3, i);

			PHALCON_CALL_FUNCTION(&time, "mktime", &tmp1, &tmp1, &tmp1, &tmp3, &tmp2);
			PHALCON_CALL_FUNCTION(&strftime, "strftime", format, &time);

			phalcon_array_update_long(return_value, i, &strftime, PH_COPY);
		}
	} else {
		PHALCON_RETURN_CALL_SELF("hours");
	}
}

/**
 * Returns an array of years between a starting and ending year. By default,
 * the the current year - 5 and current year + 5 will be used. Typically used
 * as a shortcut for generating a list that can be used in a form.
 *
 *     $years = Phalcon\Date::years(2000, 2010); // 2000, 2001, ..., 2009, 2010
 *
 * @param int $start
 * @param int $end
 * @return array
 */
PHP_METHOD(Phalcon_Date, years){

	zval *start = NULL, *end = NULL, year = {}, tmp = {};
	int i, s, e;

	phalcon_fetch_params(0, 0, 2, &start, &end);

	if (!start || PHALCON_IS_FALSE(start)) {
		ZVAL_STRING(&tmp, "Y");

		PHALCON_CALL_FUNCTION(&year, "date", &tmp);

		s = phalcon_get_intval(&year) - 5;
	} else {
		s = phalcon_get_intval(start);
	}

	if (!end || PHALCON_IS_FALSE(end)) {
		ZVAL_STRING(&tmp, "Y");

		PHALCON_CALL_FUNCTION(&year, "date", &tmp);

		e = phalcon_get_intval(&year) + 5;
	} else {
		e = phalcon_get_intval(end);
	}

	array_init(return_value);

	for (i = s; i <= e; i++) {
		phalcon_array_update_long_long(return_value, i, i, PH_COPY);
	}
}

/**
 * Returns time difference between two timestamps, in human readable format.
 * If the second timestamp is not given, the current time will be used.
 * Also consider using [Phalcon\Date::fuzzy_span] when displaying a span.
 *
 *     $span = Phalcon\Date::span(60, 182, 'minutes,seconds'); // array('minutes' => 2, 'seconds' => 2)
 *     $span = Phalcon\Date::span(60, 182, 'minutes'); // 2
 *
 * @param int $remote
 * @param int $local
 * @param string $output
 * @return string/array
 */
PHP_METHOD(Phalcon_Date, span){

	zval *remote, *local = NULL, *output = NULL, lowercased_output = {}, pattern = {}, output_arr = {}, count_output = {}, tmp = {}, tmp1 = {};
	long remote_time, local_time, timespan;
	int i;

	phalcon_fetch_params(0, 1, 2, &remote, &local, &output);

	if (!output) {
		array_init(&output_arr);
		phalcon_array_append_str(&output_arr, SL("years"), 0);
		phalcon_array_append_str(&output_arr, SL("months"), 0);
		phalcon_array_append_str(&output_arr, SL("weeks"), 0);
		phalcon_array_append_str(&output_arr, SL("days"), 0);
		phalcon_array_append_str(&output_arr, SL("hours"), 0);
		phalcon_array_append_str(&output_arr, SL("minutes"), 0);
		phalcon_array_append_str(&output_arr, SL("seconds"), 0);
	} else if (Z_TYPE_P(output) != IS_ARRAY) {
		ZVAL_STR(&tmp, phalcon_trim(output, NULL, PHALCON_TRIM_BOTH));

		phalcon_fast_strtolower(&lowercased_output, &tmp);

		if (PHALCON_IS_EMPTY(&lowercased_output)) {
			RETURN_FALSE;
		}

		ZVAL_STRING(&pattern, "/[^a-z]+/");
		PHALCON_CALL_FUNCTION(&output_arr, "preg_split", &pattern, &lowercased_output);
	} else {
		ZVAL_COPY_VALUE(&output_arr, output);
	}

	ZVAL_LONG(&tmp1, 0);

	phalcon_fast_count(&count_output, &output_arr);

	PHALCON_CALL_FUNCTION(&tmp, "array_fill", &tmp1, &count_output, &tmp1);

	ZVAL_COPY_VALUE(&tmp1, &output_arr);

	PHALCON_CALL_FUNCTION(&output_arr, "array_combine", &tmp1, &tmp);

	remote_time = phalcon_get_intval(remote);

	if (!local || Z_TYPE_P(local) == IS_NULL) {
		local_time = (long) time(NULL);
	} else {
		local_time = phalcon_get_intval(local);
	}

	// Calculate timespan (seconds)
	timespan = remote_time - local_time;
	if (timespan < 0) {
		timespan *= -1;
	}

	if (phalcon_array_isset_str(&output_arr, SL("years"))) {
		i = (int) floor(timespan / PHALCON_DATE_YEAR);
		phalcon_array_update_str_long(&output_arr, SL("years"), i, 0);
		timespan -= PHALCON_DATE_YEAR * i;
	}

	if (phalcon_array_isset_str(&output_arr, SL("months"))) {
		i = (int) floor(timespan / PHALCON_DATE_MONTH);
		phalcon_array_update_str_long(&output_arr, SL("months"), i, 0);
		timespan -= PHALCON_DATE_MONTH * i;
	}

	if (phalcon_array_isset_str(&output_arr, SL("weeks"))) {
		i = (int) floor(timespan / PHALCON_DATE_WEEK);
		phalcon_array_update_str_long(&output_arr, SL("weeks"), i, 0);
		timespan -= PHALCON_DATE_WEEK * i;
	}

	if (phalcon_array_isset_str(&output_arr, SL("days"))) {
		i = (int) floor(timespan / PHALCON_DATE_DAY);
		phalcon_array_update_str_long(&output_arr, SL("days"), i, 0);
		timespan -= PHALCON_DATE_DAY * i;
	}

	if (phalcon_array_isset_str(&output_arr, SL("hours"))) {
		i = (int) floor(timespan / PHALCON_DATE_HOUR);
		phalcon_array_update_str_long(&output_arr, SL("hours"), i, 0);
		timespan -= PHALCON_DATE_HOUR * i;
	}

	if (phalcon_array_isset_str(&output_arr, SL("minutes"))) {
		i = (int) floor(timespan / PHALCON_DATE_MINUTE);
		phalcon_array_update_str_long(&output_arr, SL("minutes"), i, 0);
		timespan -= PHALCON_DATE_MINUTE * i;
	}

	if (phalcon_array_isset_str(&output_arr, SL("seconds"))) {
		phalcon_array_update_str_long(&output_arr, SL("seconds"), timespan, 0);
	}

	if (Z_LVAL(count_output) == 1) {
		PHALCON_RETURN_CALL_FUNCTION("array_pop", &output_arr);
		return;
	}

	RETURN_CTOR(&output_arr);
}

/**
 * Returns time, in human readable format.
 * Also consider using [Phalcon\Date::fuzzy_span2] when displaying a span.
 *
 *     $span = Phalcon\Date::span(182, 'minutes,seconds'); // array('minutes' => 3, 'seconds' => 2)
 *     $span = Phalcon\Date::span(182, 'minutes'); // 3
 *
 * @param int $time
 * @param string $output
 * @return string/array
 */
PHP_METHOD(Phalcon_Date, span2){

	zval *time, *output = NULL, lower_output = {}, pattern = {}, output_arr = {}, count_output = {}, tmp = {}, tmp1 = {};
	long timespan;
	int i;

	phalcon_fetch_params(0, 1, 1, &time, &output);

	if (!output) {
		ZVAL_STRING(&lower_output, "years,months,weeks,days,hours,minutes,seconds");
	} else {
		ZVAL_STR(&tmp, phalcon_trim(output, NULL, PHALCON_TRIM_BOTH));

		phalcon_fast_strtolower(&lower_output, &tmp);

		if (PHALCON_IS_EMPTY(&lower_output)) {
			RETURN_FALSE;
		}
	}

	ZVAL_STRING(&pattern, "/[^a-z]+/");

	PHALCON_CALL_FUNCTION(&output_arr, "preg_split", &pattern, &lower_output);

	ZVAL_LONG(&tmp1, 0);

	phalcon_fast_count(&count_output, &output_arr);

	PHALCON_CALL_FUNCTION(&tmp, "array_fill", &tmp1, &count_output, &tmp1);

	ZVAL_COPY_VALUE(&tmp1, &output_arr);

	PHALCON_CALL_FUNCTION(&output_arr, "array_combine", &tmp1, &tmp);

	timespan = phalcon_get_intval(time);

	if (timespan < 0) {
		timespan *= -1;
	}

	if (phalcon_array_isset_str(&output_arr, SL("years"))) {
		i = (int) floor(timespan / PHALCON_DATE_YEAR);
		phalcon_array_update_str_long(&output_arr, SL("years"), i, 0);
		timespan -= PHALCON_DATE_YEAR * i;
	}

	if (phalcon_array_isset_str(&output_arr, SL("months"))) {
		i = (int) floor(timespan / PHALCON_DATE_MONTH);
		phalcon_array_update_str_long(&output_arr, SL("months"), i, 0);
		timespan -= PHALCON_DATE_MONTH * i;
	}

	if (phalcon_array_isset_str(&output_arr, SL("weeks"))) {
		i = (int) floor(timespan / PHALCON_DATE_WEEK);
		phalcon_array_update_str_long(&output_arr, SL("weeks"), i, 0);
		timespan -= PHALCON_DATE_WEEK * i;
	}

	if (phalcon_array_isset_str(&output_arr, SL("days"))) {
		i = (int) floor(timespan / PHALCON_DATE_DAY);
		phalcon_array_update_str_long(&output_arr, SL("days"), i, 0);
		timespan -= PHALCON_DATE_DAY * i;
	}

	if (phalcon_array_isset_str(&output_arr, SL("hours"))) {
		i = (int) floor(timespan / PHALCON_DATE_HOUR);
		phalcon_array_update_str_long(&output_arr, SL("hours"), i, 0);
		timespan -= PHALCON_DATE_HOUR * i;
	}

	if (phalcon_array_isset_str(&output_arr, SL("minutes"))) {
		i = (int) floor(timespan / PHALCON_DATE_MINUTE);
		phalcon_array_update_str_long(&output_arr, SL("minutes"), i, 0);
		timespan -= PHALCON_DATE_MINUTE * i;
	}

	if (phalcon_array_isset_str(&output_arr, SL("seconds"))) {
		phalcon_array_update_str_long(&output_arr, SL("seconds"), timespan, 0);
	}

	if (Z_LVAL(count_output) == 1) {
		ZVAL_MAKE_REF(&output_arr);
		PHALCON_CALL_FUNCTION(return_value, "array_pop", &output_arr);
		ZVAL_UNREF(&output_arr);
		return;
	}

	RETURN_CTOR(&output_arr);
}

/**
 * Returns the difference between a time and now in a "fuzzy" way.
 * Displaying a fuzzy time instead of a date is usually faster to read and understand.
 *
 *     $span = Phalcon\Date::fuzzy_span(time() - 10); // "moments ago"
 *     $span = Phalcon\Date::fuzzy_span(time() + 20); // "in moments"
 *
 * A second parameter is available to manually set the "local" timestamp,
 * however this parameter shouldn't be needed in normal usage and is only
 * included for unit tests
 *
 * @param int $timestamp
 * @param int $local_timestamp
 * @return string
 */
PHP_METHOD(Phalcon_Date, fuzzy_span)
{
	zval *timestamp, *local_timestamp = NULL, span = {};
	long now_time, local_time, offset;

	phalcon_fetch_params(0, 1, 1, &timestamp, &local_timestamp);

	now_time = phalcon_get_intval(timestamp);

	if (!local_timestamp || Z_TYPE_P(local_timestamp) == IS_NULL) {
		local_time = (long) time(NULL);
	} else {
		local_time = phalcon_get_intval(local_timestamp);
	}

	// Determine the difference in seconds
	offset = local_time - now_time;
	if (offset < 0) {
		offset *= -1;
	}

	if (offset <= PHALCON_DATE_MINUTE) {
		ZVAL_STRING(&span, "moments");
	} else if (offset < (PHALCON_DATE_MINUTE * 20)) {
		ZVAL_STRING(&span, "a few minutes");
	} else if (offset <  PHALCON_DATE_HOUR) {
		ZVAL_STRING(&span, "less than an hour");
	} else if (offset < (PHALCON_DATE_HOUR * 4)) {
		ZVAL_STRING(&span, "a couple of hours");
	} else if (offset < PHALCON_DATE_DAY) {
		ZVAL_STRING(&span, "less than a day");
	} else if (offset < (PHALCON_DATE_DAY * 2)) {
		ZVAL_STRING(&span, "about a day");
	} else if (offset < (PHALCON_DATE_DAY * 4)) {
		ZVAL_STRING(&span, "a couple of days");
	} else if (offset < PHALCON_DATE_WEEK) {
		ZVAL_STRING(&span, "less than a week");
	} else if (offset < (PHALCON_DATE_WEEK * 2)) {
		ZVAL_STRING(&span, "about a week");
	} else if (offset < PHALCON_DATE_MONTH) {
		ZVAL_STRING(&span, "less than a month");
	} else if (offset < (PHALCON_DATE_MONTH * 2)) {
		ZVAL_STRING(&span, "about a month");
	} else if (offset < (PHALCON_DATE_MONTH * 4)) {
		ZVAL_STRING(&span, "a couple of months");
	} else if (offset < PHALCON_DATE_YEAR) {
		ZVAL_STRING(&span, "less than a year");
	} else if (offset < (PHALCON_DATE_YEAR * 2)) {
		ZVAL_STRING(&span, "about a year");
	} else if (offset < (PHALCON_DATE_YEAR * 4)) {
		ZVAL_STRING(&span, "a couple of years");
	} else if (offset < (PHALCON_DATE_YEAR * 8)) {
		ZVAL_STRING(&span, "a few years");
	} else if (offset < (PHALCON_DATE_YEAR * 12)) {
		ZVAL_STRING(&span, "about a decade");
	} else if (offset < (PHALCON_DATE_YEAR * 24)) {
		ZVAL_STRING(&span, "a couple of decades");
	} else if (offset < (PHALCON_DATE_YEAR * 64)) {
		ZVAL_STRING(&span, "several decades");
	} else {
		ZVAL_STRING(&span, "a long time");
	}

	if (now_time <= local_time) {
		// This is in the past
		PHALCON_CONCAT_VS(return_value, &span, " ago");
	} else {
		// This in the future
		PHALCON_CONCAT_SV(return_value, "in ", &span);
	}
}

/**
 * Returns the time in a "fuzzy" way.
 * Displaying a fuzzy time instead of a date is usually faster to read and understand.
 *
 *     $span = Phalcon\Date::fuzzy_span2(60); // "1 minutes"
 *     $span = Phalcon\Date::fuzzy_span2(10); // "10 seconds"
 *
 * A second parameter is available to manually set the "local" timestamp,
 * however this parameter shouldn't be needed in normal usage and is only
 * included for unit tests
 *
 * @param int $timestamp
 * @param string $output
 * @return string
 */
PHP_METHOD(Phalcon_Date, fuzzy_span2)
{
	zval *timestamp, *lables = NULL, output = {}, label = {}, span = {}, ret = {};
	long offset, hours, minutes, seconds;

	phalcon_fetch_params(1, 1, 1, &timestamp, &lables);

	if (!lables) {
		array_init_size(&output, 7);
		PHALCON_MM_ADD_ENTRY(&output);
		phalcon_array_append_str(&output, SL(" years"), 0);
		phalcon_array_append_str(&output, SL(" months"), 0);
		phalcon_array_append_str(&output, SL(" weeks"), 0);
		phalcon_array_append_str(&output, SL(" days"), 0);
		phalcon_array_append_str(&output, SL(" hours"), 0);
		phalcon_array_append_str(&output, SL(" minutes"), 0);
		phalcon_array_append_str(&output, SL(" seconds"), 0);
	} else {
		ZVAL_COPY_VALUE(&output, lables);
	}

	offset = phalcon_get_intval(timestamp);
	if (offset < 0) {
		offset *= -1;
	}

	hours = offset / PHALCON_DATE_HOUR;
	minutes = (offset % PHALCON_DATE_HOUR) / PHALCON_DATE_MINUTE;
	seconds = offset % PHALCON_DATE_MINUTE;

	if (hours > 0 && phalcon_array_isset_fetch_long(&label, &output, 4, PH_READONLY)) {
		ZVAL_LONG(&span, hours);
		PHALCON_SCONCAT_VV(&ret, &span, &label);
		PHALCON_MM_ADD_ENTRY(&ret);
	}

	if (minutes > 0) {
		ZVAL_LONG(&span, minutes);
		phalcon_array_fetch_long(&label, &output, 5, PH_NOISY|PH_READONLY);

		PHALCON_SCONCAT_VV(&ret, &span, &label);
		PHALCON_MM_ADD_ENTRY(&ret);
	}

	if (seconds > 0) {
		ZVAL_LONG(&span, seconds);
		phalcon_array_fetch_long(&label, &output, 6, PH_NOISY|PH_READONLY);

		PHALCON_CONCAT_VVV(return_value, &ret, &span, &label);
	}
	RETURN_MM();
}

/**
 * Converts a UNIX timestamp to DOS format. There are very few cases where
 * this is needed, but some binary formats use it (eg: zip files.)
 * Converting the other direction is done using {@link PHALCON_DATE_dos2unix}.
 *
 *     $dos = Phalcon\Date::unix2dos($unix);
 *
 * @param int $timestamp
 * @return int
 */
PHP_METHOD(Phalcon_Date, unix2dos){

	zval *timestamp = NULL, day = {}, year = {}, mon = {}, mday = {}, hours = {}, minutes = {}, seconds = {};
	int y, m, d, h, min, sec;

	phalcon_fetch_params(0, 0, 1, &timestamp);

	if (!timestamp || PHALCON_IS_FALSE(timestamp)) {
		PHALCON_CALL_FUNCTION(&day, "getdate");
	} else {
		PHALCON_CALL_FUNCTION(&day, "getdate", timestamp);
	}

	phalcon_array_fetch_str(&year, &day, SL("year"), PH_NOISY|PH_READONLY);
	phalcon_array_fetch_str(&mon, &day, SL("mon"), PH_NOISY|PH_READONLY);
	phalcon_array_fetch_str(&mday, &day, SL("mday"), PH_NOISY|PH_READONLY);
	phalcon_array_fetch_str(&hours, &day, SL("hours"), PH_NOISY|PH_READONLY);
	phalcon_array_fetch_str(&minutes, &day, SL("minutes"), PH_NOISY|PH_READONLY);
	phalcon_array_fetch_str(&seconds, &day, SL("seconds"), PH_NOISY|PH_READONLY);

	y = phalcon_get_intval(&year);
	m = phalcon_get_intval(&mon);
	d = phalcon_get_intval(&mday);
	h = phalcon_get_intval(&hours);
	min = phalcon_get_intval(&minutes);
	sec = phalcon_get_intval(&seconds);

	if (y < 1980) {
		ZVAL_LONG(return_value, (1 << 21 | 1 << 16));
	} else {
		y -= 1980;
		ZVAL_LONG(return_value, (y << 25 | m  << 21 | d  << 16 | h  << 11 | min << 5 | sec >> 1));
	}
}

/**
 * Converts a DOS timestamp to UNIX format.There are very few cases where
 * this is needed, but some binary formats use it (eg: zip files.)
 *
 *     $unix = Phalcon\Date::dos2unix($dos);
 *
 * @param int $timestamp
 * @return int
 */
PHP_METHOD(Phalcon_Date, dos2unix){

	zval *timestamp = NULL, hrs = {}, min = {}, sec = {}, mon = {}, day = {}, year = {};
	long t = 0;

	phalcon_fetch_params(0, 0, 1, &timestamp);

	if (timestamp) {
		t = phalcon_get_intval(timestamp);
	}

	ZVAL_LONG(&sec, (2 * (t & 0x1f)));
	ZVAL_LONG(&min, ((t >>  5) & 0x3f));
	ZVAL_LONG(&hrs, ((t >> 11) & 0x1f));
	ZVAL_LONG(&day, ((t >> 16) & 0x1f));
	ZVAL_LONG(&mon, ((t >> 21) & 0x0f));
	ZVAL_LONG(&year, (((t >> 25) & 0x7f)+1980));

	PHALCON_RETURN_CALL_FUNCTION("mktime", &hrs, &min, &sec, &mon, &day, &year);
}

/**
 * Returns a date/time string with the specified timestamp format
 *
 *     $time = Phalcon\Date::formatted_time('5 minutes ago');
 *
 * @param string $datetime_str
 * @param string $timestamp_format
 * @param string $timezone
 * @return string
 */
PHP_METHOD(Phalcon_Date, formatted_time){

	zval *datetime = NULL, *format = NULL, *zone = NULL, datetime_str = {}, timestamp_format = {}, timezone = {}, tz = {}, dt = {}, tmp = {}, tmp1 = {};
	zend_class_entry *ce0, *ce1;

	phalcon_fetch_params(0, 0, 3, &datetime, &format, &zone);

	if (!datetime) {
		ZVAL_STRING(&datetime_str, "now");
	} else {
		ZVAL_COPY_VALUE(&datetime_str, datetime);
	}

	if (!format || Z_TYPE_P(format) == IS_NULL) {
		 phalcon_read_static_property_ce(&timestamp_format, phalcon_date_ce, SL("timestamp_format"), PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&timestamp_format, format);
	}

	if (!zone || Z_TYPE_P(zone) == IS_NULL) {
		phalcon_read_static_property_ce(&timezone, phalcon_date_ce, SL("timezone"), PH_READONLY);
	} else {
		ZVAL_COPY_VALUE(&timezone, zone);
	}
	if (PHALCON_IS_EMPTY(&timezone)) {
		PHALCON_CALL_FUNCTION(&timezone, "date_default_timezone_get");
	}

	ce0 = phalcon_fetch_str_class(SL("DateTimeZone"), ZEND_FETCH_CLASS_AUTO);
	ce1 = phalcon_fetch_str_class(SL("DateTime"), ZEND_FETCH_CLASS_AUTO);

	object_init_ex(&tz, ce0);
	PHALCON_CALL_METHOD(NULL, &tz, "__construct", &timezone);

	object_init_ex(&dt, ce1);
	PHALCON_CALL_METHOD(NULL, &dt, "__construct", &datetime_str, &tz);

	PHALCON_CALL_METHOD(&tmp, &dt, "getTimeZone");
	PHALCON_CALL_METHOD(&tmp1, &tmp, "getName");
	PHALCON_CALL_METHOD(&tmp, &tz, "getName");

	if (PHALCON_IS_EQUAL(&tmp1, &tmp)) {
		PHALCON_CALL_METHOD(NULL, &dt, "setTimeZone", &tz);
	}

	PHALCON_RETURN_CALL_METHOD(&dt, "format", &timestamp_format);
}

/**
 * @param \DateInterval $interval
 * @return int|string Returns usually integer, but string if result is too big (> PHP_INT_MAX)
 */
PHP_METHOD(Phalcon_Date, intervalToSeconds){

	zval *interval, s = {}, i = {}, h = {}, d = {}, m = {}, y = {};
	int seconds;

	phalcon_fetch_params(0, 1, 0, &interval);

	phalcon_read_property(&s, interval, SL("s"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&i, interval, SL("i"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&h, interval, SL("h"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&d, interval, SL("d"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&m, interval, SL("m"), PH_NOISY|PH_READONLY);
	phalcon_read_property(&y, interval, SL("y"), PH_NOISY|PH_READONLY);
	seconds = Z_LVAL(s);
    if (Z_LVAL(i) > 0) {
        seconds += Z_LVAL(i) * PHALCON_DATE_MINUTE;
    }
    if (Z_LVAL(h) > 0) {
        seconds += Z_LVAL(h) * PHALCON_DATE_HOUR;
    }
    if (Z_LVAL(d) > 0) {
        seconds += Z_LVAL(d) * PHALCON_DATE_DAY;
    }
    if (Z_LVAL(m) > 0) {
        seconds += Z_LVAL(m) * PHALCON_DATE_MONTH;
    }
    if (Z_LVAL(y) > 0) {
        seconds += Z_LVAL(y) * PHALCON_DATE_YEAR;
    }
    RETURN_LONG(seconds);
}

/**
 * @param \DateTimeZone|string|int|null $zone
 * @return int|string Returns usually integer, but string if result is too big (> PHP_INT_MAX)
 */
PHP_METHOD(Phalcon_Date, createDateTimeZone){

	zval *zone = NULL, timezone = {}, tzname = {}, gmtoffset = {};
	zend_class_entry *ce0;

	phalcon_fetch_params(0, 0, 1, &zone);

	ce0 = phalcon_fetch_str_class(SL("DateTimeZone"), ZEND_FETCH_CLASS_AUTO);

	if (!zone || Z_TYPE_P(zone) == IS_NULL) {
		phalcon_read_static_property_ce(&timezone, phalcon_date_ce, SL("timezone"), PH_READONLY);
	} else {
		PHALCON_SEPARATE_PARAM(zone);
		ZVAL_COPY_VALUE(&timezone, zone);
	}
	if (PHALCON_IS_EMPTY(&timezone)) {
		PHALCON_CALL_FUNCTION(&timezone, "date_default_timezone_get");
	} else {
		if (Z_TYPE(timezone) == IS_OBJECT) {
			if (instanceof_function(Z_OBJCE(timezone), ce0)) {
				RETURN_CTOR(&timezone);
			}
		} else if (Z_TYPE(timezone) == IS_LONG) {
			ZVAL_LONG(&gmtoffset, Z_LVAL(timezone) * 3600);
			PHALCON_CALL_FUNCTION(&tzname, "timezone_name_from_abbr", &PHALCON_GLOBAL(z_null), &gmtoffset, &PHALCON_GLOBAL(z_true));
			if (PHALCON_IS_FALSE(&tzname)) {
				PHALCON_THROW_EXCEPTION_FORMAT(spl_ce_InvalidArgumentException, "Unknown or bad timezone (%s)", Z_STRVAL(timezone));
				return;
			}
			ZVAL_COPY_VALUE(&timezone, &tzname);
		} else {
			convert_to_string_ex(&timezone);
		}
	}
	object_init_ex(return_value, ce0);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", &timezone);
}

/**
 *  Filters a variable
 *
 *     $ret = Phalcon\Date::filter('2012-01-22');
 *     $ret = Phalcon\Date::filter('2012-01-22 11:00:00', 'Y-m-d H:i:s');
 *
 * @param string $date_str
 * @param string $date_format
 * @param string $delimiter
 * @return string|array
 */
PHP_METHOD(Phalcon_Date, filter){

	zval *date = NULL, *format = NULL, *delimiter = NULL, date_format = {}, format_date = {}, errors = {}, warning_count = {}, error_count = {};
	zend_class_entry *ce0;

	phalcon_fetch_params(0, 1, 2, &date, &format, &delimiter);

	if (!format || Z_TYPE_P(format) == IS_NULL) {
		ZVAL_STRING(&date_format, "Y-m-d");
	} else {
		ZVAL_COPY(&date_format, format);
	}

	ce0 = phalcon_fetch_str_class(SL("DateTime"), ZEND_FETCH_CLASS_AUTO);

	if (delimiter && Z_STRLEN_P(delimiter) > 0) {
		zval dates = {}, *str;
		phalcon_fast_explode(&dates, delimiter, date);

		array_init(return_value);
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(dates), str) {
			zval value = {};

			if (PHALCON_IS_NOT_EMPTY_STRING(str)) {
				PHALCON_CALL_CE_STATIC(&value, ce0, "createfromformat", &date_format, str);
				if (Z_TYPE(value) != IS_OBJECT) {
					phalcon_array_append(return_value, &PHALCON_GLOBAL(z_null), 0);
				} else {
					zval d = {};
					PHALCON_CALL_METHOD(&d, &value, "format", &date_format);
					phalcon_array_append(return_value, &d, 0);
				}
				zval_ptr_dtor(&value);

			}
		} ZEND_HASH_FOREACH_END();

	} else {
		PHALCON_CALL_CE_STATIC(&format_date, ce0, "createfromformat", &date_format, date);
		PHALCON_CALL_CE_STATIC(&errors, ce0, "getlasterrors");

		if (Z_TYPE(errors) == IS_ARRAY) {
			if (phalcon_array_isset_fetch_str(&warning_count, &errors, SL("warning_count"), PH_READONLY)
				&& PHALCON_GT_LONG(&warning_count, 0)) {
				zval_ptr_dtor(&errors);
				zval_ptr_dtor(&date_format);
				RETURN_NULL();
			}
			if (phalcon_array_isset_fetch_str(&error_count, &errors, SL("error_count"), PH_READONLY) && PHALCON_GT_LONG(&error_count, 0)) {
				zval_ptr_dtor(&errors);
				zval_ptr_dtor(&date_format);
				RETURN_NULL();
			}
		}

		PHALCON_CALL_METHOD(return_value, &format_date, "format", &date_format);
	}
	zval_ptr_dtor(&date_format);
}

/**
 * Checks if a value is a valid date
 *
 *     $ret = Phalcon\Date::valid('2012-01-22');
 *     $ret = Phalcon\Date::valid('2012-01-22 11:00:00', 'Y-m-d H:i:s');
 *
 * @param string $date_str
 * @param string $date_format
 * @return boolean
 */
PHP_METHOD(Phalcon_Date, valid){

	zval *date = NULL, *format = NULL, date_format = {}, errors = {}, warning_count = {}, error_count = {};
	zend_class_entry *ce0;

	phalcon_fetch_params(0, 1, 1, &date, &format);

	if (!format || Z_TYPE_P(format) == IS_NULL) {
		if (Z_STRLEN_P(date) > 10) {
			ZVAL_STRING(&date_format, "Y-m-d H:i:s");
		} else {
			ZVAL_STRING(&date_format, "Y-m-d");
		}
	} else {
		ZVAL_COPY(&date_format, format);
	}

	ce0 = phalcon_fetch_str_class(SL("DateTime"), ZEND_FETCH_CLASS_AUTO);

	PHALCON_CALL_CE_STATIC(NULL, ce0, "createfromformat", &date_format, date);
	zval_ptr_dtor(&date_format);
	PHALCON_CALL_CE_STATIC(&errors, ce0, "getlasterrors");

	if (Z_TYPE(errors) == IS_ARRAY) {
		if (phalcon_array_isset_fetch_str(&warning_count, &errors, SL("warning_count"), PH_READONLY)
			&& PHALCON_GT_LONG(&warning_count, 0)) {
			zval_ptr_dtor(&errors);
			RETURN_FALSE;
		}
		if (phalcon_array_isset_fetch_str(&error_count, &errors, SL("error_count"), PH_READONLY) && PHALCON_GT_LONG(&error_count, 0)) {
			zval_ptr_dtor(&errors);
			RETURN_FALSE;
		}
		zval_ptr_dtor(&errors);
	}

	RETURN_TRUE;
}

/**
 * Gets two date difference.
 *
 *     $ret = Phalcon\Date::diff('2012-01-22', '2018-05-10');
 *
 * @param string $date1
 * @param string $date2
 * @param string $differenceFormat
 * @param string $format
 * @return int
 */
PHP_METHOD(Phalcon_Date, diff){

	zval *date1, *date2, *diff_format = NULL, *format = NULL, date_format = {}, format_date1 = {}, format_date2 = {}, interval = {};
	zend_class_entry *ce0;

	phalcon_fetch_params(0, 2, 2, &date1, &date2, &diff_format, &format);

	if (!format || Z_TYPE_P(format) == IS_NULL) {
		if (Z_STRLEN_P(date1) > 10) {
			ZVAL_STRING(&date_format, "Y-m-d H:i:s");
		} else {
			ZVAL_STRING(&date_format, "Y-m-d");
		}
	} else {
		ZVAL_COPY(&date_format, format);
	}

	ce0 = phalcon_fetch_str_class(SL("DateTime"), ZEND_FETCH_CLASS_AUTO);

	PHALCON_CALL_CE_STATIC(&format_date1, ce0, "createfromformat", &date_format, date1);
	if (!zend_is_true(&format_date1)) {
		zval_ptr_dtor(&date_format);
		RETURN_FALSE;
	}
	PHALCON_CALL_CE_STATIC(&format_date2, ce0, "createfromformat", &date_format, date2);
	if (!zend_is_true(&format_date2)) {
		zval_ptr_dtor(&format_date1);
		zval_ptr_dtor(&date_format);
		RETURN_FALSE;
	}
	zval_ptr_dtor(&date_format);

	if (!diff_format || Z_TYPE_P(diff_format) == IS_NULL) {
		zval time1 = {}, time2 = {};
		PHALCON_CALL_METHOD(&time1, &format_date1, "gettimestamp");
		PHALCON_CALL_METHOD(&time2, &format_date2, "gettimestamp");
		phalcon_sub_function(return_value, &time1, &time2);
		zval_ptr_dtor(&time1);
		zval_ptr_dtor(&time2);
	} else {
		PHALCON_CALL_METHOD(&interval, &format_date1, "diff", &format_date2);
		PHALCON_CALL_METHOD(return_value, &interval, "format", diff_format);
		zval_ptr_dtor(&interval);
	}
	zval_ptr_dtor(&format_date1);
	zval_ptr_dtor(&format_date2);
}

/**
 * Gets two date difference.
 *
 *     $ret = Phalcon\Date::add('2012-01-22', '1 days');
 *
 * @param string $date1
 * @param string $interval
 * @param string $format
 * @return string
 */
PHP_METHOD(Phalcon_Date, add){

	zval *date1, *interval_string = NULL, *format = NULL, date_interval = {}, interval = {}, date_format = {}, format_date1 = {};
	zend_class_entry *ce0, *ce1;

	phalcon_fetch_params(0, 1, 2, &date1, &interval_string, &format);

	if (!interval_string || Z_TYPE_P(interval_string) == IS_NULL) {
		if (Z_STRLEN_P(date1) > 10) {
			ZVAL_STRING(&date_interval, "1 seconds");
		} else {
			ZVAL_STRING(&date_interval, "1 days");
		}
	} else {
		ZVAL_COPY(&date_interval, interval_string);
	}

	if (!format || Z_TYPE_P(format) == IS_NULL) {
		if (Z_STRLEN_P(date1) > 10) {
			ZVAL_STRING(&date_format, "Y-m-d H:i:s");
		} else {
			ZVAL_STRING(&date_format, "Y-m-d");
		}
	} else {
		ZVAL_COPY(&date_format, format);
	}

	ce0 = phalcon_fetch_str_class(SL("DateInterval"), ZEND_FETCH_CLASS_AUTO);
	PHALCON_CALL_CE_STATIC(&interval, ce0, "createfromdatestring", &date_interval);
	zval_ptr_dtor(&date_interval);

	ce1 = phalcon_fetch_str_class(SL("DateTime"), ZEND_FETCH_CLASS_AUTO);

	PHALCON_CALL_CE_STATIC(&format_date1, ce1, "createfromformat", &date_format, date1);
	if (!zend_is_true(&format_date1)) {
		zval_ptr_dtor(&interval);
		zval_ptr_dtor(&date_format);
		RETURN_FALSE;
	}

	PHALCON_CALL_METHOD(NULL, &format_date1, "add", &interval);
	zval_ptr_dtor(&interval);

	PHALCON_CALL_METHOD(return_value, &format_date1, "format", &date_format);
	zval_ptr_dtor(&format_date1);
	zval_ptr_dtor(&date_format);
}
