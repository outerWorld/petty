
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <time.h>

#include <errno.h>
#include <signal.h>

#include "tutil.h"
#include "cmdline.h"
#include "xcap.h"

enum {
	OPT_DEVID = 0x01,
	OPT_FILTER,
	OPT_OUTFILE,
	OPT_HELP,
};

static opt_profile_t opt_prof[] = {
	{(char*)"-i", OPT_ARG_CONSTANT, 1, OPT_DEVID},
	{(char*)"-f", OPT_ARG_OPTIONAL, 64, OPT_FILTER},
	{(char*)"-o", OPT_ARG_CONSTANT, 1, OPT_OUTFILE},
	{(char*)"-h", OPT_ARG_CONSTANT, 0, OPT_HELP},
	{NULL, -1, -1, -1},
};

static int run = 0;
static xcap * xx = NULL;

typedef struct _pkt_proc_ctx_s {
	int o_fd;
}pkt_proc_ctx_t, *pkt_proc_ctx_p; 

static int pkt_proc(void *data, int data_len, PVOID ctx)
{
	pkt_proc_ctx_p p_ctx = (pkt_proc_ctx_p)ctx;
	int fd = p_ctx->o_fd;
	if (data_len <= 0) return 0;

	//fprintf(stdout, "data len = %d\n", data_len);
	write(fd, (char*)data, data_len);

	return 0;
}

static void sig_proc(int sig_no)
{
	fprintf(stdout, "%d sigint = %d\n", sig_no, SIGINT);
	if (sig_no == SIGINT) {
		run = 0;
		fprintf(stdout, "run = %d\n", run);
		xx->xcap_stop();
	}
}

static int sig_reg()
{
	signal(SIGKILL, sig_proc);
	signal(SIGINT, sig_proc);
	signal(SIGPIPE, sig_proc);

	return 0;
}

static int usage()
{
	fprintf(stdout, "Usage: pkt_log -i network_card -f filter_rules -o logfile -h\n");

	return 0;
}

/*
 * usage: pkt_log -i network_card -f filter_rules -o logfile -h
 */
int main(int argc, char *argv[])
{
	char *dev = (char*)"eth0";
	void *arglist = NULL;
	char filter[128] = { 0 };
	char outfile[128] = { 0 };
	pkt_proc_ctx_t ctx = { 0 };
	char now_time[64] = { 0 };
	time_t now;
	struct tm tm_now;
	int opt_idx = 0, opt_id = 0, arg_num = 0;

	if (argc < 2) {
		fprintf(stderr, "invalid arguments!\n");
		return 1;
	}

#if 1
	time(&now);
	localtime_r(&now, &tm_now);
	snprintf(now_time, sizeof(now_time), "%04d%02d%02d.%02d%02d%02d", tm_now.tm_year+1900, tm_now.tm_mon + 1, tm_now.tm_mday, tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);
#else
	snprintf(now_time, sizeof(now_time), "%s", "123");
#endif

	if (cmdline_init() != 0) {
		fprintf(stderr, "cmdline_init failed!\n");
		return 1;
	}

	if (0 != cmdline_parse(argc-1, (char**)&argv[1], (opt_profile_p)opt_prof)) {
		return 1;
	}

	while (-1 != cmdline_result_loop(&opt_idx, &opt_id, &arglist, &arg_num)) {
		switch (opt_id) {
			case OPT_HELP:
				return usage();

				break;
			case OPT_DEVID:
				if (arg_num <= 0) dev = strdup("eth0");
				else dev = strdup(cmdline_get_arg(arglist, 0));

				break;
			case OPT_FILTER:
			{
				int i = 0;
				fprintf(stdout, "arg_num = %d\n", arg_num);
				//aggregate_cmd(filter, sizeof(filter), arg_num, (char**)&argv[opt_idx + 2]);
				for (i = 0; i < arg_num; i++) {
					strcat(filter, cmdline_get_arg(arglist, i));
					strcat(filter, " ");
				}
				fprintf(stdout, "filter = [%s]\n", filter);
			}

				break;
			case OPT_OUTFILE:
				if (arg_num <= 0) snprintf(outfile, sizeof(outfile), "%s_pkt.log", now_time);
				else snprintf(outfile, sizeof(outfile), "%s_%s", now_time, cmdline_get_arg(arglist, 0));

				break;
			default:
				break;
		}
	}

	cmdline_show();
	cmdline_destroy();

	sig_reg();

	ctx.o_fd = open(outfile, O_WRONLY|O_CREAT|O_APPEND, S_IRWXU|S_IRWXO|S_IRWXG);
	if (ctx.o_fd < 0) {
		fprintf(stderr, "open [%s] failed: <%d,%s>\n", outfile, errno, strerror(errno));
		return 1;
	}

	xx = new xcap(xcap::XCAP_ETH);
	if (!xx) {
		fprintf(stderr, "new xcap failed!\n");
		return 1;
	}
	
	fprintf(stdout, "dev = %s, filter = %s\n", dev, filter);
	if (0 != xx->init(dev, filter, 0, 2)) {
		fprintf(stdout, "failed to init xcap object!\n");
		return 1;
	}
	xx->set_app_pkt_cb(pkt_proc, xcap::PKT_IP, &ctx);

	run = 1;
	while (run) {
		if (xx->xcap_run() < 0) break;
	}

	fprintf(stdout, "----quit from pkt_log\n");

	close(ctx.o_fd);
	xx->xcap_stop();

	delete xx;

	return 0;
}
