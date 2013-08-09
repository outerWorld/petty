
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tutil.h"
#include "xcap.h"
#include "radius.h"
#include "radius_dpi.h"

static int g_num = 0;

static int radius_cb(void *un_pkt, int len, void *ctx)
{
	radius_dpi * p_rad_dpi = (radius_dpi*)ctx;
	rad_attr_info_p p_name = NULL;
	rad_attr_info_p p_ip = NULL;
	rad_attr_info_p p_status = NULL;

	if (0 != p_rad_dpi->parse((char *)un_pkt + 8, len -8)) {
		//fprintf(stdout, "pkt_len = %d, parse radius failed\n", len);
	} else {
		//fprintf(stdout, "ok!\n");
	}

	p_name = p_rad_dpi->get_attr(RAD_ATTR_USER_NAME);
	p_ip = p_rad_dpi->get_attr(RAD_ATTR_FRAMED_IP_ADDR);
	p_status = p_rad_dpi->get_attr(RAD_ATTR_ACCOUNT_STATUS);
	if (p_name->addr) fprintf(stdout, "%p:[%.*s] %d\n", p_name->addr, p_name->len, p_name->addr, p_name->len);
	if (p_ip->addr) fprintf(stdout, "%p:[%08x] %d\n", p_ip->addr, p_ip->addr ? *(unsigned int*)p_ip->addr : 0, p_ip->len);
	if (p_status->addr) fprintf(stdout, "%p:[%08x] %d\n", p_status->addr, p_status->addr ? *(unsigned int*)p_status->addr : 0, p_status->len);

	if (p_name) {
		fprintf(stdout, "name = %p[%.*s], %d\n", p_name->addr, p_name->len, p_name->addr, p_name->len);
	}
	if (p_ip) {
		fprintf(stdout, "ip = %p, %d\n", p_ip->addr, p_ip->len);
	}
	if (p_status) {
		fprintf(stdout, "status = %p, %d\n", p_status->addr, p_status->len);
	}

	if (p_name->addr  && p_ip->addr && p_status->addr) {
		fprintf(stdout, "ok = %d\n", g_num++);
	}

	p_rad_dpi->attr_reset();

	return 0;
}

// test_radius_dpi pcapfile filter
int main(int argc, char *argv[])
{
	char *dev = NULL;
	char filter[256] =  { 0 };
	xcap * xx = new xcap(xcap::XCAP_FILE);
	radius_dpi *p_rad_dpi;

	if (argc > 1) dev = argv[1];
	if (argc > 2) {
		aggregate_cmd(filter, sizeof(filter), argc-2, (char**)&argv[2]);
	}
	p_rad_dpi = new radius_dpi();
	p_rad_dpi->enable_code(RAD_ACCOUNT_REQ);
	xx->set_app_pkt_cb(radius_cb, xcap::PKT_IP, (PVOID)p_rad_dpi);
	if (xx->init(dev, filter, 0, 0) != 0) {
		return -1;
	}

	xx->xcap_run();

	xx->xcap_stop();

	return 0;
}
