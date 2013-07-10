
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tutil.h"
#include "xcap.h"

static int ip_cb1(void *pkt, int pkt_size, void *ctx)
{
	fprintf(stdout, "ip[0], size = %d\n", pkt_size);
	return 0;
}

static int ip_cb2(void *pkt, int pkt_size, void *ctx)
{
	fprintf(stdout, "ip[1], size = %d\n", pkt_size);
	return 0;
}

static int ip_cb3(void *pkt, int pkt_size, void *ctx)
{
	fprintf(stdout, "ip[2], size = %d\n", pkt_size);
	return 0;
}

int main(int argc, char *argv[])
{
	int type = 0;
	const int filter_pos = 3;
	xcap *xx;
	char *dev;
	char filter[1024];

	if (argc < filter_pos) {
		return 1;
	}
	
	type = atoi(argv[1]);
	dev = argv[2];

	if (0 != aggregate_cmd(filter, sizeof(filter), argc - filter_pos, &argv[filter_pos])) {
		return 1;
	}

	xx = new xcap(type);

	if (0 != xx->init(dev, filter, 0, 0)) {
		delete xx;
		return 1;
	}

	xx->add_pkt_cb(ip_cb1, xcap::PKT_IP, NULL);
	xx->add_pkt_cb(ip_cb2, xcap::PKT_IP, NULL);
	xx->add_pkt_cb(ip_cb3, xcap::PKT_IP, NULL);

	xx->xcap_run();

	delete xx;

	return 0;
}
