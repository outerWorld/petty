
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include "tutil.h"
#include "tlog.h"

int main(int argc, char *argv[])
{
	int is_daemon = 0;
	char *log_fpath, *log_fname;

	if (argc < 4) return 1;
	is_daemon = atoi(argv[1]);
	if (is_daemon) set_daemon();
	log_fpath = argv[2];
	log_fname = argv[3];
	
	if (0 != log_init(log_fpath, log_fname, 1024, 1024)) return 1;
	log_push(getpid(), LOG_INFO, "log name [%s%s]", log_fpath, log_fname);
	//log_push(getpid(), LOG_INFO, "log name\n");

	log_destroy();
	
	return 0;
}
