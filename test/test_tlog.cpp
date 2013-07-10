// -*- encoding = utf-8 -*-

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>

#include "perf_mon.h"
#include "tlog.h"

void * tlog_proc(void *arg)
{
	int i = 0;

	while (i < 102400) {
		log_push(0, LOG_INFO, (char*)"a\n");
		log_push(0, LOG_ERR, (char*)"b\n");
		log_push(0, LOG_WARN, (char*)"c\n");
		log_push(0, LOG_NOTICE, (char*)"d\n");
		//usleep(2);
		//if (i % 1024) usleep(20);
		i++;
	}
}

#undef __PERF_MON_H
int main(int argc, char *argv[])
{
	pthread_t thd;
	int i = 0;
	void * ret = NULL;

	if (argc < 3) return 1;

	if (0 != log_init(argv[1], argv[2], 2048, 1024)) return 1;

#if defined(__PERF_MON_H)
	perf_mon_init(10, 10);
	perf_mon_add(0, (char*)"log_push");

	perf_mon_here(0, ST_START);
#endif // __PERF_MON_H
	pthread_create(&thd, NULL, tlog_proc, NULL);
	pthread_create(&thd, NULL, tlog_proc, NULL);
	pthread_join(thd, (void**)&ret);
//	while (i < 102400) {
//		log_push(0, LOG_INFO, (char*)"a\n");
//		log_push(0, LOG_ERR, (char*)"b\n");
//		log_push(0, LOG_WARN, (char*)"c\n");
//		log_push(0, LOG_NOTICE, (char*)"d\n");
//		//usleep(2);
//		//if (i % 1024) usleep(20);
//		i++;
//	}
#if defined(__PERF_MON_H)
	perf_mon_here(0, ST_END);
#endif // __PERF_MON_H

	log_destroy();

#if defined(__PERF_MON_H)
	raise(SIGUSR1);
#endif // __PERF_MON_H
	//sleep(10);

	return 0;
}
