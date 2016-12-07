/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2015 Phalcon Team (http://www.phalconphp.com)       |
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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef PHP_WIN32
#include "win32/time.h"
#elif defined(NETWARE)
#include <sys/timeval.h>
#include <sys/time.h>
#else
#include <sys/time.h>
#endif

#include <ctype.h>

#include <php.h>

#include "kernel/main.h"
#include "kernel/time.h"
#include "kernel/operators.h"

void phalcon_time(zval *return_value)
{
	RETURN_LONG(time(NULL));
}

int phalcon_get_time()
{
	return time(NULL);
}

#ifdef HAVE_GETTIMEOFDAY
void phalcon_microtime(zval *return_value, zval *get_as_float)
{
	struct timeval tp = {0};
	char ret[100];

	if (gettimeofday(&tp, NULL)) {
		RETURN_FALSE;
	}

	if (get_as_float && PHALCON_IS_TRUE(get_as_float)) {
		RETURN_DOUBLE((double)(tp.tv_sec + tp.tv_usec / MICRO_IN_SEC));
	}

	snprintf(ret, 100, "%.8F %ld", tp.tv_usec / MICRO_IN_SEC, tp.tv_sec);
	RETURN_STRING(ret);
}

double phalcon_get_microtime()
{
	struct timeval tp = {0};

	if (gettimeofday(&tp, NULL)) {
		return -1;
	}

	return (double)(tp.tv_sec + tp.tv_usec / MICRO_IN_SEC);
}
#endif

long int phalcon_get_system_uptime()
{
    FILE	*fp;
    char    temp[128];
    time_t  system_boot_time;
    int     btime_found;

    fp = fopen("/proc/stat", "r");

    if(fp == NULL){
        return -1;
    }

    btime_found = 0;
    while(fscanf(fp, "%s %ld", temp, &system_boot_time) != EOF){
        if(!strncmp(temp, "btime", sizeof("btime"))){
            btime_found = 1;
            break;
        }
    }

    fclose(fp);

    if(!btime_found){
        return -1;
    }

    return system_boot_time;
}

long int phalcon_get_proc_starttime(unsigned int pid)
{
    FILE	*fp;
    char    readbuf[1024];
    char    filename[128], *pc, *pt;
    int     column_on, column;
    time_t  proc_start_time;
    long long tmp_long_long;


    snprintf(filename, sizeof(filename), "/proc/%lu/stat", pid);

    fp = fopen(filename, "r");
    if(!fp){
        return -1;
    }

    memset(readbuf, 0x00, sizeof(readbuf));

    if(!fgets(readbuf, sizeof(readbuf), fp)){
        return -1;
    }

    fclose(fp);

	column = 0;
    column_on = 0;
    pc = readbuf;
    while(*pc){
		if(*pc == ' ' || *pc == '\t'){
            column_on = 0;
		}else{
            if(!column_on){
                column++;
            }
            column_on = 1;
		}

        if(column == 22)
            break;
        pc++;
    }

	pt = pc;
    while(*pc) {
		if (*pc != ' ' && *pc != '\t') {
			pc++;
		} else {
			*pc = 0x00;
			break;
		}
	}

	tmp_long_long = strtoll(pt, NULL, 10);
#ifdef __LINUX__
	proc_start_time = tmp_long_long / HZ;
#else
	proc_start_time = tmp_long_long / 100;
#endif
	proc_start_time += phalcon_get_system_uptime();

	return proc_start_time;
}
