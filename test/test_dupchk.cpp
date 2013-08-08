#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>

#include <openssl/md5.h>

#include "tutil.h"
#include "xcap.h"

typedef struct _context_s {
	int fd;
	int times;
}context_t, *context_p;

static context_t ctx = { -1, 0 };

static char hex_map[] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'A', 'B', 'C', 'D', 'E', 'F'
};

typedef struct _http_user_info_s {
	unsigned int src_ip;
	unsigned int dst_ip;
	unsigned short src_port;
	unsigned short dst_port;
	unsigned int tcp_seq;
	unsigned int tcp_ack;
}http_user_info_t, *http_user_info_p;

int pkt_process(void *pkt, int pkt_sz, void *ctx)
{
	unsigned char md5_val[16] = { 0 };
	unsigned char md5_val_str[36] = { 0 };
	unsigned char *u_pkt = (unsigned char*)pkt;
	char src_ip_str[24], dst_ip_str[24];
	unsigned int src_ip = 0, dst_ip = 0;
	context_p p_ctx = (context_p)ctx;
	http_user_info_p p_user = NULL;

	MD5(u_pkt, pkt_sz, md5_val);

	for (int i = 0; i < 16; i++) {
		md5_val_str[i*2] = hex_map[md5_val[i]&0x0F];
		md5_val_str[i*2 + 1] = hex_map[md5_val[i] >> 4];
	}
	md5_val_str[32] = '\n';

	write(p_ctx->fd, md5_val_str, 33);
	p_ctx->times++;
	if (p_ctx->times % 100 == 0) {
		fsync(p_ctx->fd);
	}

#if 0
	if (pkt_sz < sizeof(http_user_info_t)) {
		return 0;
	}
	p_user = (http_user_info_p)pkt;
	src_ip = htonl(p_user->src_ip);
	dst_ip = htonl(p_user->dst_ip);
	inet_ntop(AF_INET, (char*)&src_ip, src_ip_str, sizeof(src_ip_str));
	inet_ntop(AF_INET, (char*)&dst_ip, dst_ip_str, sizeof(dst_ip_str));
	fprintf(stdout, "%s %s\n", src_ip_str, dst_ip_str);
#endif

	return 0;
}

#define ERR(info) fprintf(stderr, "err: %s\n", info)

// test_dupchk type pcap_file
// type-->0, ip
// type-->1, application data
int main(int argc, char *argv[])
{
	xcap * p_x = NULL;
	int type = 0;

	p_x = new xcap(xcap::XCAP_FILE);
	if (!p_x) return 1;

	type = atoi(argv[1]);

	if (0 != p_x->init(argv[2], "", 0, 0)) {
		ERR("xcap init failed!");	
		return 1;
	}

	ctx.fd = open("pkt_md5", O_TRUNC|O_CREAT|O_WRONLY, S_IRWXU);
	if (ctx.fd < 0) {
		ERR("open pkt_md5 failed!");
		goto __EXIT;
	}

	if (type == 0) {
		p_x->add_pkt_cb(pkt_process, xcap::PKT_IP, (void*)&ctx);
	} else {
		p_x->set_app_pkt_cb(pkt_process, xcap::PKT_IP, (void*)&ctx);
	}
	if (0 != p_x->xcap_run()) {
		goto __EXIT;
	}

	close(ctx.fd);
	p_x->xcap_stop();

__EXIT:
	delete p_x;

	return 0;
}
