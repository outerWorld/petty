
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netinet/ip.h>

#include "tutil.h"
#include "xcap.h"

#define ERR(info) fprintf(stderr, "err: %s [%d:%s]\n", info, errno, strerror(errno))

typedef struct _context_s {
	int fd;
	char buf[2048];
}context_t, *context_p;

int ipv4_pkt_proc(void *pkt, int pkt_size, void *ctx)
{
	int len = 0;
	struct iphdr *p_ip = NULL;
	char src_ip[48], dst_ip[48];
	char tl_type[12];
	context_p p_ctx = (context_p)ctx;

	if (pkt_size < sizeof(struct iphdr)) {
		return 1;
	}
	p_ip = (struct iphdr*)pkt;

	if (!inet_ntop(AF_INET, (char*)&p_ip->saddr, src_ip, sizeof(src_ip))) {
		return 1;
	}

	if (!inet_ntop(AF_INET, (char*)&p_ip->daddr, dst_ip, sizeof(dst_ip))) {
		return 1;
	}

	if (p_ctx->fd > 0) {
		len = sprintf(p_ctx->buf, "%s %s\n", src_ip, dst_ip);
		write(p_ctx->fd, p_ctx->buf, len);
	}

	return 0;
}

int ipv6_pkt_proc(void *pkt, int pkt_size, void *ctx)
{
	int len = 0;
	struct ip6_hdr *p_ip = NULL;
	char src_ip[48], dst_ip[48];
	char tl_type[12];
	context_p p_ctx = (context_p)ctx;

	if (pkt_size < sizeof(struct ip6_hdr)) {
		return 1;
	}
	p_ip = (struct ip6_hdr*)pkt;

	if (!inet_ntop(AF_INET6, (char*)&p_ip->ip6_src, src_ip, sizeof(src_ip))) {
		return 1;
	}

	if (!inet_ntop(AF_INET6, (char*)&p_ip->ip6_dst, dst_ip, sizeof(dst_ip))) {
		return 1;
	}

	if (p_ctx->fd > 0) {
		len = sprintf(p_ctx->buf, "%s %s\n", src_ip, dst_ip);
		write(p_ctx->fd, p_ctx->buf, len);
	}

	return 0;
}


static int get_file_fd(char *file)
{
	int fd;

	fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

	return fd;
}

static void usage()
{
	fprintf(stdout, "getpkt_ip dev-type dev-name filter\n");
	fprintf(stdout, "dev-type: 0, dev-name is pcap-file; 1, dev-name is ethernet-device-name\n");
	fprintf(stdout, "filter: for example, tcp dst port 80\n");
}

// test_pkt_ip type dev_name filter
int main(int argc, char *argv[])
{
	int cap_type = 0;
	char *dev = NULL;
	char filter[1024];
	xcap * p_x = NULL;
	context_p p_ctx = NULL;

	if (argc < 3) {
		usage();
		return 1;
	}

	cap_type = atoi(argv[1]);
	dev = strdup(argv[2]);

	if (0 != aggregate_cmd(filter, sizeof(filter), argc-3, &argv[3])) {
		usage();
		return 1;
	}

	p_x = new xcap(cap_type);
	if (!p_x) return 1;

	if (0 != p_x->init(dev, filter, 0, 0)) {
		return 1;
	}

	p_ctx = (context_p)malloc(sizeof(context_t));
	if (!p_ctx) return 1;

	p_ctx->fd = get_file_fd("ip_list");
	if (p_ctx->fd <= 0) {
		return 1;
	}

	p_x->add_pkt_cb(ipv4_pkt_proc, xcap::PKT_IP, p_ctx);
	p_x->add_pkt_cb(ipv6_pkt_proc, xcap::PKT_IPV6, p_ctx);

	if (0 != p_x->xcap_run()) {
		return 0;
	}

	delete p_x;
	p_x = NULL;

	free(p_ctx);
	close(p_ctx->fd);

	return 0;
}
