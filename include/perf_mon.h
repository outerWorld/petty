/*
 * @file    : perf_mon.h
 * @encoding: utf-8
 * @author  : huang chunping
 * @date    : 2013-04-13
 *
 */

#ifndef __PERF_MON_H
#define __PERF_MON_H

// preset_times --> initialized executed times
int perf_mon_init(int max_point, int preset_times);

int perf_mon_add(int id, char *name);

// status --> start or end
enum { ST_START = 0x01, ST_END = 0x02 };
int perf_mon_here(int id, int status);

//int perf_statis(int sig_no);

int perf_mon_destroy();

#endif // __PERF_MON_H
