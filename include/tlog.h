/*
 * @file name: tlog.h
 * @encoding : utf-8
 * @author	 : huang chunping
 * @date	 : 2013-06-10
 * @history	 : 1. 2013-06-10, created
 */

#ifndef __TLOG_H
#define __TLOG_H

#include "tqueue.h"

typedef enum {
	LOG_TYPE_BASE = 0x00,
	LOG_INFO = 0x00,
	LOG_ERR,
	LOG_WARN,
	LOG_NOTICE,
	LOG_TYPE_MAX
}LOG_TYPE;

enum {
	LOGGER_INIT	= 0x00,
	LOGGER_RUNNING,
	LOGGER_STOP,
	LOGGER_STOPPED
};

#define LOGF_NAME_SIZE 256

static char * log_type_desc[] = {
	(char*)"INFO", (char*)"ERR", (char*)"WARN", (char*)"NOTICE", (char*)"UNKNOWN"
};

class tlog {
public:
	tlog(char *fpath, char *logname, int e_size, int e_num);
	~tlog();

	bool is_running() { return status==LOGGER_RUNNING; }
	
	void set_status(int st) { status = st; }
	
	int store(int type);
	
	int	sync();

	int log_it(unsigned long int id, int type, char *fmt, ...);
	int log_it(unsigned long int id, int type, char *fmt, va_list va);

	int start();
	int stop(); 

	void show_status() { fprintf(stdout, "push_lock = %d, push_full = %d, fetch_lock = %d, fetch_empty = %d\n", push_lock, push_full, fetch_lock, fetch_empty); }

public:
	int		log_q_esize;
	int		log_q_enum;

private:
	int		push_full;	// failed times of writing log: for queue is full
	int		push_lock;	// failed times of writing log: for queue is locked
	int		fetch_lock;	// failed times of fetching log: for queue is locked
	int		fetch_empty;	// failed times of fetching log: for queue is empty.
	int		status;
	char	logf_name[LOG_TYPE_MAX][LOGF_NAME_SIZE];
	int		log_fds[LOG_TYPE_MAX];
	tqueue	*log_ques[LOG_TYPE_MAX];
};

int log_init(char *fpath, char *logname, int e_size, int e_num);

int log_push(unsigned long id, int type, char *fmt, ...);

int log_destroy();

#endif // __TLOG_H
