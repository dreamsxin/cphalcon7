
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

#ifndef PHALCON_DATE_H
#define PHALCON_DATE_H

#include "php_phalcon.h"

#define PHALCON_DATE_YEAR          31556926
#define PHALCON_DATE_MONTH         2629744
#define PHALCON_DATE_WEEK          604800
#define PHALCON_DATE_DAY           86400
#define PHALCON_DATE_HOUR          3600
#define PHALCON_DATE_MINUTE        60

#define PHALCON_YEARS_PER_CENTURY   100
#define PHALCON_YEARS_PER_DECADE    10
#define PHALCON_MONTHS_PER_YEAR     12
#define PHALCON_MONTHS_PER_QUARTER  3
#define PHALCON_WEEKS_PER_YEAR      52
#define PHALCON_DAYS_PER_WEEK       7
#define PHALCON_HOURS_PER_DAY       24
#define PHALCON_MINUTES_PER_HOUR    60
#define PHALCON_SECONDS_PER_MINUTE  60

#define PHALCON_DATE_MONTHS_LONG   "%B"
#define PHALCON_DATE_MONTHS_SHORT  "%b"

extern zend_class_entry *phalcon_date_ce;

PHALCON_INIT_CLASS(Phalcon_Date);

#endif /* PHALCON_DATE_H */
