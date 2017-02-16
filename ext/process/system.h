
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


#ifndef PHALCON_PROCESS_SYSTEM_H
#define PHALCON_PROCESS_SYSTEM_H

#include <php.h>
#include <Zend/zend.h>

long int phalcon_get_system_uptime();
long int phalcon_get_proc_starttime(unsigned int pid);

/* structs */
typedef struct _phalcon_process {
	zend_bool has_commandline;
	unsigned int pid;
	int iopriority;

	/* cmdline */
	zval args;

	/* stat */
    char name[256];
	char state;
	unsigned int ppid;
	unsigned int pgrp;
	unsigned int session;
	unsigned int tty_nr;
	unsigned int tpgid;
	unsigned int rchar;
	unsigned int wchar;
	unsigned int flags;
	unsigned long minflt;
	unsigned long cminflt;
	unsigned long majflt;
	unsigned long cmajflt;
	unsigned long utime;
	unsigned long stime;
	long int cutime;
	long int cstime;
	long int priority;
	long int nice;
	long int numthreads;
	unsigned long long starttime;
	unsigned long vsize;
	long int rss;
	unsigned long rsslim;
	unsigned long startcode;
	unsigned long endcode;
	unsigned long startstack;
	unsigned long kstkesp;
	unsigned long kstkeip;
	unsigned long wchan;
	int exit_signal;
	int processor;
	unsigned int rt_priority;
	unsigned int policy;
	unsigned long long delayacct_blkio_ticks;
	unsigned long guest_time;
	long int cguest_time;

	/* status */
	int ruid;
	int euid;
	int ssuid;
	int fsuid;
	int rgid;
	int egid;
	int ssgid;
	int fsgid;

	/* statm */
	unsigned int size;
	unsigned int resident;
	unsigned int share;
	unsigned int text;
	unsigned int data;

	/* io */
	unsigned int syscr;
	unsigned int syscw;
	unsigned int read_bytes;
	unsigned int write_bytes;
	unsigned int cancelled_write_bytes;
} phalcon_process;

int phalcon_get_proc_info(phalcon_process *proc, unsigned int pid);
int phalcon_proc_parse_stat(phalcon_process *proc, unsigned int pid);
int phalcon_proc_parse_statm(phalcon_process *proc, unsigned int pid);
int phalcon_proc_parse_status(phalcon_process *proc, unsigned int pid);
int phalcon_proc_parse_io(phalcon_process* proc, unsigned int pid);
int phalcon_proc_parse_cmdline(phalcon_process *proc, unsigned int pid);
int phalcon_proc_get_iopriority(unsigned int pid);

#endif /* PHALCON_PROCESS_SYSTEM_H */
