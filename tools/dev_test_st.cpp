
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pthread.h>
#include <sched.h>

#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <netinet/ip.h>
#include <netinet/tcp.h>

#include "tutil.h"
#include "transfer.h"
#include "cmdline.h"

enum {
	PKT_PKT = 0x00,
	PKT_HTTP_GET,
	PKT_TYPE_NUM
};

typedef struct _pkt_info_s {
	/* int					type; */
	unsigned long long 	num;
	unsigned long long 	bytes;
}pkt_info_t, *pkt_info_p;

static int pkt_st_inc(pkt_info_p pkt_info, int type, int bytes)
{
	pkt_info[type].num++;
	pkt_info[type].bytes += bytes;

	return 0;
}

static int pkt_st_clean(pkt_info_p pkt_info, int type)
{
	if (type >= PKT_TYPE_NUM) {
		memset(pkt_info, 0x00, sizeof(pkt_info));
	} else {
		pkt_info[type].num = 0;
		pkt_info[type].bytes = 0;
	}
}

static int pkt_st_vary(const pkt_info_p pkt_info1, const pkt_info_p pkt_info2)
{
	if (pkt_info1->num != pkt_info2->num || pkt_info1->bytes != pkt_info2->bytes) {
		return 1;
	}

	return 0;
}

enum {
	OPT_ATTACH = 0x00,
	OPT_INTERVAL,
	OPT_CAP_NUM,
	OPT_DEST,
	OPT_HELP
};

static opt_profile_t opt_prof[] = {
	{(char*)"--attach", OPT_ARG_CONSTANT, 1, OPT_ATTACH},
	{(char*)"--interval", OPT_ARG_CONSTANT, 1, OPT_INTERVAL},
	{(char*)"--cap-num", OPT_ARG_CONSTANT, 1, OPT_CAP_NUM},
	{(char*)"-d", OPT_ARG_CONSTANT, 2, OPT_DEST},
	{(char*)"-h", OPT_ARG_CONSTANT, 1, OPT_HELP},
	{(char*)NULL, -1, -1, -1},
};

typedef struct _pkt_cap_ctx_s {
	int			id;
	int			num;
	pkt_info_p 	p_pkt_info;
	int			forward_id;	// select what device to forward the packets captured from other devices.
	struct timeval tm_beg;
	struct timeval tm_end;
}pkt_cap_ctx_t, *pkt_cap_ctx_p;

static int pkt_cap_ctx_log(pkt_cap_ctx_p p_ctx, int num, char *logbuf, int logbuf_sz, struct timeval now)
{
	int len = 0;

	for (int i = 0; i < num; i++) {
		int ret = snprintf(logbuf + len, logbuf_sz - len, "%ld.%ld|pkt_info|%d|%ld.%ld|%ld.%ld|%llu, %llu bytes|%llu, %llu bytes\n",
						now.tv_sec, now.tv_usec, i, p_ctx[i].tm_beg.tv_sec, p_ctx[i].tm_beg.tv_usec,
						p_ctx[i].tm_end.tv_sec, p_ctx[i].tm_end.tv_usec, 
						p_ctx[i].p_pkt_info[PKT_PKT].num, p_ctx[i].p_pkt_info[PKT_PKT].bytes,
						p_ctx[i].p_pkt_info[PKT_HTTP_GET].num, p_ctx[i].p_pkt_info[PKT_HTTP_GET].bytes
						);
		if (ret >= (logbuf_sz-len)) {
			return 0;
		}
		len += ret;
	}

	return len;
}

static int pkt_cap_ctx_clear(pkt_cap_ctx_p p_ctx, int num)
{
	for (int i = 0; i < num; i++) {
		memset(p_ctx[i].p_pkt_info, 0x00, sizeof(pkt_info_t) *p_ctx[i].num);
		/* p_ctx[i].num = 0; */ // do not to reset it!!!
		/* p_ctx[i].forward_id = -1; */
		memset(&p_ctx[i].tm_beg, 0x00, sizeof(p_ctx[i].tm_beg));
		memset(&p_ctx[i].tm_end, 0x00, sizeof(p_ctx[i].tm_end));
	}

	return 0;
}

static pkt_cap_ctx_p pkt_cap_ctx_new(int num)
{
	pkt_cap_ctx_p snap = NULL;
	
	MEM_ALLOC(snap, pkt_cap_ctx_p, sizeof(pkt_cap_ctx_t) * num, NULL);
	for (int i = 0; i < num; i++) {
		MEM_ALLOC(snap[i].p_pkt_info, pkt_info_p, sizeof(pkt_info_t) * PKT_TYPE_NUM, NULL);
		snap[i].num = PKT_TYPE_NUM;
	}

	pkt_cap_ctx_clear(snap, num);	

	return snap;
}

static pkt_cap_ctx_p pkt_cap_ctx_snap(const pkt_cap_ctx_p p_ctx, int num, pkt_cap_ctx_p p_snap)
{
	pkt_cap_ctx_p snap = NULL;

	if (!p_snap) {
		snap = pkt_cap_ctx_new(num);
	} else {
		snap = p_snap;
	}

	for (int i = 0; i < num; i++) {
		memcpy(snap[i].p_pkt_info, p_ctx[i].p_pkt_info, sizeof(pkt_info_t) * snap[i].num);
		// todo: other members don't copy now.
	}

	return snap;
}

static int is_http_get_pkt(char *data, int data_len)
{
	struct iphdr *p_ip = NULL;
	struct tcphdr *p_tcp = NULL;
	unsigned short dst_port = 0;
	char *p = data;
	int app_len = 0;

	// if link-layer is in data
	//p = p + 14;
	
	if (data_len <= sizeof(struct iphdr) + sizeof(struct tcphdr)) {
		return 0;
	}

	p_ip = (struct iphdr*)p;
	if (p_ip->protocol != IPPROTO_TCP) {
		return 0;
	}

	p_tcp = (struct tcphdr*)(p + (p_ip->ihl<<2));
	dst_port = ntohs(p_tcp->dest);
	if (dst_port != 80 && dst_port != 8080) return 0;

	p = ((char*)p_tcp) + (p_tcp->doff << 2);
	app_len = data_len - ((p_ip->ihl + p_tcp->doff) << 2);
	if (app_len <= sizeof("GET / HTTP/1.1\r\n")-1) // "GET / HTTP/1.1"
		return 0;

	if (strncasecmp(p, "GET ", sizeof("GET ")-1) != 0) {
		return 0;
	}

	if (!strstr(p, " HTTP/1.1\r\n")) {
		return 0;
	}

	return 1;
}

static void * pkt_cap_cb(void *arg)
{
	pkt_cap_ctx_p ctx = (pkt_cap_ctx_p)arg;
	char *data = NULL;
	int data_len = 0;
	int is_pkt_first = 1;
	cpu_set_t mask;
	struct timeval init_tm;
	memset(&init_tm, 0x00, sizeof(init_tm));
	memset(&ctx->tm_beg, 0x00, sizeof(ctx->tm_beg));

	if (ctx->id >= 0) {
		CPU_ZERO(&mask);
		CPU_SET(ctx->id, &mask);
		if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
			fprintf(stderr, "%s pthread_setaffinity_np on %d failed!\n", __FUNCTION__, ctx->id);
			return NULL;
		}
	}

	while (1) {
		// todo: capture data from device.
		// 

		if (memcmp(&ctx->tm_beg, &init_tm, sizeof(init_tm)) == 0) {
			fprintf(stdout, "is pkt first!\n");
			gettimeofday(&ctx->tm_beg, NULL);
		}
		pkt_st_inc(ctx->p_pkt_info, PKT_PKT, data_len);

		// if packet is http get
		if (is_http_get_pkt(data, data_len)) {
			pkt_st_inc(ctx->p_pkt_info, PKT_HTTP_GET, data_len);
		}

		// todo: send packet.
		//

		gettimeofday(&ctx->tm_end, NULL);
	}

	return NULL;
}

static int read_meminfo(int fd, char *logbuf, int logbuf_sz, struct timeval now)
{
	struct stat st;
	int buf_sz = 1024 * 4;
	char buf[1024*4];
	char *pb, *p = NULL;
	int log_len = 0;
	int i = 0;
	int ret = 0;
	
	ret = read(fd, buf, buf_sz-1);
	if (ret < 0) {
		fprintf(stderr, "read %d failed!\n", fd);
		return -1;
	}
	buf[ret] = '\0';
	
	ret = snprintf(logbuf + log_len, logbuf_sz - log_len, "%ld.%ld|meminfo|", now.tv_sec, now.tv_usec);
	if (ret >= logbuf_sz-log_len) return -1;
	log_len += ret;

	pb = buf;
	p = (char*)strstr(buf, (char*)"\n");
	while (p) {
		int len = p - pb;
		memcpy(logbuf + log_len, pb, len);
		log_len += len;
		logbuf[log_len] = '|';
		log_len += 1;
		pb = p + 1;
		p = (char*)strstr(pb, (char*)"\n");
	}
	logbuf[log_len] = '\n';	
	logbuf[log_len+1] = '\0';	
	log_len += 2;

	return log_len;
}

static int read_cpuinfo(int fd, char *logbuf, int logbuf_sz, struct timeval now)
{
	int buf_sz = 1024*4;
	char buf[1024*4];
	int log_len = 0;
	int i = 0;
	int ret = 0;

	ret = read(fd, buf, buf_sz - 1);
	if (ret < 0) {
		fprintf(stderr, "read %d failed!\n", fd);
		return -1;
	}
	buf[ret] = '\0';
	//fprintf(stdout, "ret = %ld, cpuinfo = %s\n", ret, (char*)buf);

	ret = snprintf(logbuf + log_len, logbuf_sz - log_len, "%ld.%ld|cpuinfo|", now.tv_sec, now.tv_usec);
	if (ret >= logbuf_sz-log_len) return -1;
	log_len += ret;

	char *pb = buf;
	char *p = strstr(buf, (char*)"\n");
	while (p) {
		int len = p - pb;
		if (i != 3) { // do not need intr line.
			memcpy(logbuf + log_len, pb, len);
			log_len += len;
			logbuf[log_len] = '|';
			log_len += 1;
		}
		pb = p + 1;
		p = strstr((char*)pb, (char*)"\n");
		i++;
	}
	logbuf[log_len] = '\n';	
	logbuf[log_len+1] = '\0';	
	log_len += 2;

	return log_len;
}

typedef struct _st_ctx_s {
	int				id;
	int				chk_interval;
	int				num;	
	pkt_cap_ctx_p 	p_cap_ctx;
	transfer		*sender;
}st_ctx_t, *st_ctx_p;

static void * st_cb(void *arg)
{
	int num = 0;
	st_ctx_p ctx = (st_ctx_p)arg;
	pkt_cap_ctx_p p_snap = NULL;
	transfer *sender = ctx->sender;
	int log_size = 10240, log_len = 0;
	char *log;

	struct timeval now;
	cpu_set_t mask;
	int cpu_fd = 0; // read from /proc/stat
	int mem_fd = 0; // read from /proc/meminfo

	MEM_ALLOC(log, char*, log_size, NULL);

	if (ctx->id >= 0) {
		CPU_ZERO(&mask);
		CPU_SET(ctx->id, &mask);
		if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
			fprintf(stderr, "%s pthread_setaffinity_np on %d failed!\n", __FUNCTION__, ctx->id);
			return NULL;
		}
	}

	p_snap = pkt_cap_ctx_snap(ctx->p_cap_ctx, ctx->num, NULL);

	sleep(5);

	while (1) {
		int all_stop = 1;
		for (int i = 0; i < ctx->num; i++) {
			if (pkt_st_vary(&p_snap[i].p_pkt_info[PKT_PKT], &ctx->p_cap_ctx[i].p_pkt_info[PKT_PKT])) {
				all_stop = 0;
				break;
			}
		}
		cpu_fd = open("/proc/stat", O_RDONLY, S_IRUSR);
		if (cpu_fd < 0) {
			fprintf(stderr, "read [%s] failed! [%d,%s]\n", "proc/stat", errno, strerror(errno));
			return NULL;
		}
		mem_fd = open("/proc/meminfo", O_RDONLY, S_IRUSR);
		if (cpu_fd < 0) {
			fprintf(stderr, "read [%s] failed! [%d,%s]\n", "proc/meminfo", errno, strerror(errno));
			return NULL;
		}

		// send log to log-server
		gettimeofday(&now, NULL);
		log_len = pkt_cap_ctx_log(ctx->p_cap_ctx, ctx->num, log, log_size, now);
		if (log_len > 0) sender->nsend(log, log_len, NULL, -1);
		log_len = read_cpuinfo(cpu_fd, log, log_size, now);
		if (log_len > 0) sender->nsend(log, log_len, NULL, -1);
		else fprintf(stderr, "read_cpuinfo %d\n", log_len);
		log_len = read_meminfo(mem_fd, log, log_size, now);
		if (log_len > 0) sender->nsend(log, log_len, NULL, -1);
		else fprintf(stderr, "read_meminfo %d\n", log_len);

		if (all_stop) {
			fprintf(stdout, "pkt_cap_ctx_clear: clear pkt statis info\n");
			pkt_cap_ctx_clear(p_snap, p_snap->num);	
			pkt_cap_ctx_clear(ctx->p_cap_ctx, ctx->num);
		} else {
			pkt_cap_ctx_snap(ctx->p_cap_ctx, ctx->num, p_snap);
		}
		close(mem_fd);
		close(cpu_fd);

		sleep(ctx->chk_interval);
	}

	return NULL;
}

static int usage()
{
	fprintf(stdout, "dev_test_st --attach {0|1} --interval 10 --cap-num {pkt_capture_num} -d dst_ip dst_port -h\n");

	return 0;
}

int main(int argc, char *argv[])
{
	transfer * st_sender =NULL;
	int chk_interval = 0, attach = 0, cap_num = 0;
	char *dst_ip = NULL;
	unsigned short dst_port = 0;
	int opt_idx = 0, opt_id = 0, arg_num = 0;
	void *arglist = NULL;
	pkt_cap_ctx_p p_cap_ctx = NULL;
	st_ctx_p p_st_ctx = NULL;
	cpu_set_t set;

	if (argc < 2) return 1;

	if (0 != cmdline_init()) {
		fprintf(stderr, "cmdline_init failed!\n");
		return 1;
	}

	if (0 != cmdline_parse(argc-1, &argv[1], opt_prof)) {
		fprintf(stderr, "cmdline_parse failed!\n");
		return 1;
	}

	while (-1 != cmdline_result_loop(&opt_idx, &opt_id, &arglist, &arg_num)) {
		switch (opt_id) {
			case OPT_HELP:
				return usage();

				break;
			case OPT_DEST:
				dst_ip = strdup(cmdline_get_arg(arglist, 0));
				dst_port = atoi(cmdline_get_arg(arglist, 1));

				break;
			case OPT_INTERVAL:
				chk_interval = atoi(cmdline_get_arg(arglist, 0));

				break;
			case OPT_CAP_NUM:
				cap_num = atoi(cmdline_get_arg(arglist, 0));

				break;
			case OPT_ATTACH:
				attach = atoi(cmdline_get_arg(arglist, 0));

				break;
			default:
				break;
		}
	}

	cmdline_show();
	cmdline_destroy();

	st_sender = new transfer(IS_CLIENT, SOCK_DGRAM, dst_ip, dst_port);
	if (!st_sender || true != st_sender->is_ok()) {
		return -1;
	}

	p_cap_ctx = pkt_cap_ctx_new(cap_num);
	MEM_ALLOC(p_st_ctx, st_ctx_p, sizeof(st_ctx_t), 1);

	int cpu_num = sysconf(_SC_NPROCESSORS_CONF);
	fprintf(stdout, "cpu num = %d\n", cpu_num);
	p_st_ctx->num = cap_num;
	p_st_ctx->chk_interval = chk_interval;
	p_st_ctx->p_cap_ctx = p_cap_ctx;
	p_st_ctx->sender = st_sender;
	if (attach) p_st_ctx->id = cap_num > cpu_num ? (cpu_num-1) : cap_num;
	else p_st_ctx->id = -1;

	pthread_t st_thd;
	if (0 != pthread_create(&st_thd, NULL, st_cb, (void*)p_st_ctx)) {
		fprintf(stderr, "create st_cb failed!\n");
		return 1;
	}

	pthread_t cap_thd;
	for (int i = 0; i < cap_num; i++) {
		if (attach) p_cap_ctx[i].id = cap_num > cpu_num ? (i%(cpu_num-1)) : i;
		else p_cap_ctx[i].id = -1;
		if (0 != pthread_create(&cap_thd, NULL, pkt_cap_cb, &p_cap_ctx[i])) {
			fprintf(stderr, "create pkt_cap_cb failed!\n");
			return 1;
		}
	}

	void *ret = NULL;
	pthread_join(st_thd, &ret);

	return 0;
}
