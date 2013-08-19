
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <net/if_arp.h>

#include "tutil.h"
#include "xcap.h"

typedef struct _arp_context_s {
	int fd;
}arp_context_t, *arp_context_p;

static int show_arphdr(struct arphdr *p_arp)
{
	fprintf(stdout, "%x %x %x %x %x\n", ntohs(p_arp->ar_hrd), ntohs(p_arp->ar_pro), p_arp->ar_hln, p_arp->ar_pln, ntohs(p_arp->ar_op));
	
	return 0;
}

int arp_pkt_proc(void *pkt, int pkt_sz, void *ctx)
{
	struct arphdr * p_arp = NULL;

	if (pkt_sz < sizeof(struct arphdr)) {
		return 1;
	}
	p_arp = (struct arphdr*)pkt;
	show_arphdr(p_arp);

	return 0;
}

// getarp dev-type dev-name filter
int main(int argc, char *argv[])
{
	int dev_type = 0;
	char *dev_name = NULL;
	char filter[1024] = { 0 };
	xcap * p_x = NULL;
	arp_context_p p_ctx = NULL;

	if (argc < 3) return 0;

	dev_type = atoi(argv[1]);
	dev_name = strdup(argv[2]);
	aggregate_cmd(filter, sizeof(filter), argc-3, &argv[3]);

	p_x = new xcap(dev_type);
	if (!p_x) return 1;

	if (0 != p_x->init(dev_name, filter, 1, 0)) {
		return 1;
	}

	p_ctx = (arp_context_p)malloc(sizeof(arp_context_t));
	if (!p_ctx) return 1;
	
	p_x->add_pkt_cb(arp_pkt_proc, xcap::PKT_ARP, p_ctx);

	if (-1 == p_x->xcap_run()) {
		return 1;
	}

	if (p_x) delete p_x;
	p_x = NULL;

	return 0;
}
