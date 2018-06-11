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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <unistd.h>

#include "process/system.h"
#include "kernel/array.h"

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


    snprintf(filename, sizeof(filename), "/proc/%lu/stat", (unsigned long)pid);

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

int phalcon_get_proc_info(phalcon_process *proc, unsigned int pid) {

	proc->pid = pid;
    memset(proc->name, 0, sizeof(proc->name));

	phalcon_proc_parse_stat(proc, pid);
	phalcon_proc_parse_statm(proc, pid);
	phalcon_proc_parse_status(proc, pid);
	phalcon_proc_parse_io(proc, pid);
	phalcon_proc_parse_cmdline(proc, pid);

	proc->iopriority = phalcon_proc_get_iopriority(pid);

	return 0;
}

int phalcon_proc_parse_stat(phalcon_process *proc, unsigned int pid) {
	char path[256];
	int fd;
	char readbuf[1024];

    sprintf(path, "/proc/%i/stat", pid);
	fd = open(path, O_RDONLY);
	memset(&readbuf, 0, sizeof(readbuf));
	if (read(fd, readbuf, sizeof(readbuf)) != -1) {
		sscanf(readbuf, "%d %256s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %*d %llu %lu %ld %lu %lu %lu %lu %lu %lu %*u %*u %*u %*u %lu %*u %*u %d %d %u %u %llu %lu %ld",
					&proc->pid,
					proc->name,
					&proc->state,
					&proc->ppid,
					&proc->pgrp,
					&proc->session,
					&proc->tty_nr,
					&proc->tpgid,
					&proc->flags,
					&proc->minflt,
					&proc->cminflt,
					&proc->majflt,
					&proc->cmajflt,
					&proc->utime,
					&proc->stime,
					&proc->cutime,
					&proc->cstime,
					&proc->priority,
					&proc->nice,
					&proc->numthreads,
					&proc->starttime,
					&proc->vsize,
					&proc->rss,
					&proc->rsslim,
					&proc->startcode,
					&proc->endcode,
					&proc->startstack,
					&proc->kstkesp,
					&proc->kstkeip,
					&proc->wchan,
					&proc->exit_signal,
					&proc->processor,
					&proc->rt_priority,
					&proc->policy,
					&proc->delayacct_blkio_ticks,
					&proc->guest_time,
					&proc->cguest_time);
	}
    close(fd);
	return 0;
}

int phalcon_proc_parse_statm(phalcon_process *proc, unsigned int pid) {
	char path[256];
	int fd;
	char buf[1024];

    sprintf(path, "/proc/%i/statm", pid);
	fd = open(path, O_RDONLY);
	memset(&buf, 0, sizeof(buf));
	if (read(fd, buf, sizeof(buf)) != -1) {
		sscanf(buf, "%u %u %u %u %*u %u %*u",
		&proc->size,
		&proc->resident,
		&proc->share,
		&proc->text,
		&proc->data);
	}
    close(fd);
	return 0;
}

int phalcon_proc_parse_status(phalcon_process *proc, unsigned int pid) {
	char path[256];
	int fd;
	char buf[1024];

    sprintf(path, "/proc/%i/status", pid);
    fd = open(path, O_RDONLY);
	memset(&buf, 0, sizeof(buf));
	if (read(fd, buf, sizeof(buf)) != -1) {
		sscanf(buf, "Name: %*s\nState: %*[^\n]\nTgid: %*d\nPid: %*d\nPPid: %*d\nTracerPid: %*d\nUid: %d %d %d %d\nGid: %d %d %d %d\n",
		&proc->ruid,
		&proc->euid,
		&proc->ssuid,
		&proc->fsuid,
		&proc->rgid,
		&proc->egid,
		&proc->ssgid,
		&proc->fsgid);
	}
    close(fd);
	return 0;
}

int phalcon_proc_parse_io(phalcon_process* proc, unsigned int pid) {
	char path[256];
	int fd;
	char buf[1024];

    sprintf(path, "/proc/%i/io", pid);
    fd = open(path, O_RDONLY);
	memset(&buf, 0, sizeof(buf));
	if (read(fd, buf, sizeof(buf)) != -1) {
		sscanf(buf, "rchar: %d\nwchar: %d\nsyscr: %d\nsyscw: %d\nread_bytes: %d\nwrite_bytes: %d\ncancelled_write_bytes: %d",
					&proc->rchar,
					&proc->wchar,
					&proc->syscr,
					&proc->syscw,
					&proc->read_bytes,
					&proc->write_bytes,
					&proc->cancelled_write_bytes);
	}
    close(fd);

	return 0;
}

int phalcon_proc_parse_cmdline(phalcon_process *proc, unsigned int pid) {
	char path[256];
	int fd;
	char buf[1024];
	unsigned int buflen, i, argc;
	char *c;

    sprintf(path, "/proc/%i/cmdline", pid);
    fd = open(path, O_RDONLY);
	buflen = read(fd, buf, sizeof(buf));

	if(buflen == 0) {
			return 0;
	}
	proc->has_commandline = 1;

	/* cmdline is not null terminated if there are no command line arguments. */
	if (buf[buflen-1] != '\0') {
		buf[buflen++] = '\0';
	}

	/* figure out argc */
	argc=0;
	for (i=0; i < buflen; i++) {
		if (buf[i] == '\0'){
			argc++;
		}
		c++;
	}

	/* One for each pointer plus null */
	array_init_size(&(proc->args), argc);

	/* And the char arrays */
	c = buf;
	for (i=0; i < argc; i++) {
		phalcon_array_append_str(&(proc->args), c, strlen(c)+1, 0);
		while (*c != '\0'){
				c++;
		}
		c++;
	}
    close(fd);
	return 0;
}

int phalcon_proc_get_iopriority(unsigned int pid) {
	
#ifdef SYS_ioprio_get
	return syscall(SYS_ioprio_get, 1, pid);
#else 
	return -1; 
#endif 
}
