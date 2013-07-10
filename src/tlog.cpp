// -*- encoding = utf-8 -*-
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <pthread.h>

#include "tlog.h"

static tlog * g_log = NULL;

tlog::tlog(char *fpath, char *logname, int e_size, int e_num)
{
	log_q_esize = e_size;
	log_q_enum = e_num;
	fetch_empty = 0;
	fetch_lock = 0;
	push_full = 0;
	push_lock = 0;
	memset(log_fds, -1, LOG_TYPE_MAX*sizeof(int));
	memset(log_ques, 0x00, sizeof(tqueue*)*LOG_TYPE_MAX);
	for (int i = 0; i < LOG_TYPE_MAX; i++) {
		snprintf(logf_name[i], LOGF_NAME_SIZE, "%s/%s.%s", fpath, logname, log_type_desc[i]);
	}
}

tlog::~tlog()
{
	for (int i = 0; i < LOG_TYPE_MAX; i++) {
		if (log_fds[i] > 0) close(log_fds[i]);
		log_fds[i] = -1;
		
		if (log_ques[i]) delete log_ques[i];
		log_ques[i] = NULL;
	}
}

int tlog::start()
{
	status = LOGGER_RUNNING;
	return 0;	
}

int tlog::store(int type)
{
	int ret = 0;
	char *blk = NULL;
	int data_len = 0;

	// Attention: log_ques created when log_push called as it is really needed.
	if (!log_ques[type]) return 0;

	if (log_fds[type] == -1) {
		log_fds[type] = open(logf_name[type], O_CREAT |O_APPEND | O_WRONLY, S_IRWXU);
		if (log_fds[type] < 0) {
			fprintf(stderr, "open %s fail! [%d, %s]\n", logf_name[type], errno, strerror(errno));
			return -1;
		}
	}
	ret = log_ques[type]->get_front_data(&blk, &data_len);
	if (0 != ret) {
		//fprintf(stderr, "get_front_data fail [%d, %s]\n", errno, strerror(errno));
		usleep(5);
		(ret == -1) ? (fetch_lock++) : (fetch_empty++);
		return 0;
	}

	//fprintf(stdout, "[%s] %d\n", log_type_desc[type], data_len);
	if (data_len > 0) {
		write(log_fds[type], blk, data_len);
	}

	log_ques[type]->pop();

	return 0;
}

int tlog::stop()
{
	status = LOGGER_STOP;
	int t = 50;
	while (t-- > 0) {
		if (status == LOGGER_STOPPED) break;
		usleep(20);
	}
	
	usleep(500);

	return 0;
}

int tlog::log_it(unsigned long id, int type, char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	log_it(id, type, fmt, va);
	va_end(va);

	return 0;
}

int tlog::log_it(unsigned long id, int type, char *fmt, va_list va)
{
	int	ret = 0;
	int len = 0;
	int log_len = 0;
	struct timeval now;
	char *log_blk = NULL;
	struct tm t_now = { 0 };

	if (type < LOG_TYPE_BASE || type >= LOG_TYPE_MAX) {
		return -1;
	}

	gettimeofday(&now, NULL);
	localtime_r(&now.tv_sec, &t_now);

	// delay initialize.
	if (!log_ques[type]) {
		log_ques[type] = new tqueue(g_log->log_q_esize, g_log->log_q_enum, tqueue::MT_SUPPORT);
		if (!log_ques[type]) return -1;
	}

	// get one block buffer from queue, and lock it.
	ret = log_ques[type]->fetch_blk(&log_blk);
	if (ret < 0) {
		(ret == -1) ? (push_lock++) : (push_full++);
		return -1;
	}

	len = snprintf(log_blk, g_log->log_q_esize, "<%4d-%02d-%02d %02d:%02d:%02d.%ld, %lu, %s>\01",
			t_now.tm_year + 1900, t_now.tm_mon, t_now.tm_mday, t_now.tm_hour, t_now.tm_min, t_now.tm_sec, now.tv_usec, id, log_type_desc[type]);
	if (len >= g_log->log_q_esize) {
		log_ques[type]->tail_unlock();
		return -1;
	}
	log_len = len;

	len = vsnprintf(log_blk + log_len, g_log->log_q_esize-log_len, fmt, va);
	if (len >= g_log->log_q_esize - log_len) {
		log_ques[type]->tail_unlock();
		return -1;
	}
	log_len += len;

	log_ques[type]->push(log_blk, log_len);

	return 0;
}

int tlog::sync()
{
	for (int i = 0; i < LOG_TYPE_MAX; i++)
		log_fds[i] > 0 ? fsync(log_fds[i]) : 1;

	return 0;
}

static void * log_proc(void *arg)
{
	tlog * log = (tlog*)arg;

	log->start();

	while (log->is_running()) {
		for (int i = 0; i < LOG_TYPE_MAX; i++) {
			log->store(i);
		}
	}

	log->sync();
	log->set_status(LOGGER_STOPPED);

	return NULL;
}

int log_init(char *fpath, char *logname, int e_size, int e_num)
{
	pthread_t thd;
	g_log = new tlog(fpath, logname, e_size, e_num);
	if (!g_log) return -1;

	if (0 != pthread_create(&thd, NULL, log_proc, (void*)g_log)) {
		return -1;
	}

	return 0;
}


int log_push(unsigned long id, int type, char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	g_log->log_it(id, type, fmt, va);
	va_end(va);

	return 0;
}

int log_destroy()
{
	if (g_log) {
		g_log->show_status();
		g_log->stop();
		delete g_log;
	}
	g_log = NULL;

	return 0;
}

