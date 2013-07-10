// -*- encoding = utf-8 -*-
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <linux/if_ether.h>

#include "tutil.h"
#include "xcap.h"

static void xcap_default_pkt_proc(unsigned char* user_data, const struct pcap_pkthdr * pkt_hdr, const unsigned char *pkt_data);

xcap::xcap(int xcap_type)
{
	p_pkt_proc = xcap_default_pkt_proc;
	pkt_proc_ctx = this;
	dev_hd = NULL;
	this->type = xcap_type;
	with_vlan = 0;
	
	memset(pkt_cbs, 0x00, sizeof(pkt_cb_list_p)*PKT_CAP_SIZE);
	app_pkt_cb = NULL;
}

xcap::~xcap()
{
	if (dev_hd) {
		pcap_close(dev_hd);
		dev_hd = NULL;
	}
}


int xcap::init(char *dev, char *filter, int promisc, int to_ms)
{
	char errbuf[PCAP_ERRBUF_SIZE] = { 0 };
	bpf_program bpf_prg;
	bpf_u_int32 net_id = 0, mask_id = 0;
	int blk = 0;

	if (type == XCAP_FILE)
			dev_hd = pcap_open_offline(dev, errbuf);
	else if (type == XCAP_ETH)
			dev_hd = pcap_open_live(dev, 65535, promisc, to_ms, errbuf);
	else return -1;

	if (!dev_hd) {
		fprintf(stderr, "%s failed, error [%s]\n", type==XCAP_FILE ? "pcap_open_offline":"pcap_open_live", errbuf);
		return -1;
	}

	if (type == XCAP_ETH) {
#if 0
		if (0 != pcap_lookupnet(dev, &net_id, &mask_id, errbuf)) {
			fprintf(stderr, "pcap_lookupnet failed, error [%s]\n", errbuf);
			pcap_close(dev_hd);
			dev_hd = 0;
			return -1;
		}
#endif
	}

	fprintf(stdout, "pcap_getnonblock = %d\n", pcap_getnonblock(dev_hd, errbuf));

#if 0
	if (type == XCAP_ETH && 0 != pcap_set_timeout(dev_hd, 1)) {
		fprintf(stderr, "pcap_set_timeout failed, error [%s]\n", errbuf);
		pcap_close(dev_hd);
		dev_hd = 0;
		return -1;
	}
#endif

	if (0 != pcap_compile(dev_hd, &bpf_prg, filter, 0, mask_id)) {
		fprintf(stderr, "pcap_compile failed, error [%s]\n", errbuf);
		pcap_close(dev_hd);
		dev_hd = 0;
		return -1;
	}

	if (0 != pcap_setfilter(dev_hd, &bpf_prg)) {
		fprintf(stderr, "pcap_setfilter failed, error [%s]\n", errbuf);
		return -1;
	}

	return 0;
}

int xcap::xcap_run()
{
	int ret = 0;

	while (ret >= 0) {
		ret = pcap_dispatch(dev_hd, -1, p_pkt_proc, (unsigned char*)pkt_proc_ctx);
		if (0 == ret) {
			//fprintf(stdout, "pcap_dispatch returns 0\n");
			usleep(5);
		}
	}

	return ret;
}

int xcap::add_pkt_cb(pkt_cb pkt_fun, int pkt_cap_type, void *ctx)
{
	pkt_cb_list_p p_cbs = NULL;
	pkt_cb * p_cb = NULL;
	PVOID * p_ctx = NULL;
	if (pkt_cap_type >= PKT_CAP_SIZE) return -1;

	p_cbs = pkt_cbs[pkt_cap_type];
	if (!pkt_cbs[pkt_cap_type]) {
		MEM_ALLOC(pkt_cbs[pkt_cap_type], pkt_cb_list_p, sizeof(pkt_cb_list_t), -1);
		p_cbs = pkt_cbs[pkt_cap_type];
	}

	if (p_cbs->num >= p_cbs->size) {
		MEM_ALLOC(p_cb, pkt_cb*, sizeof(pkt_cb)*2*(p_cbs->size + 1), -1);
		MEM_ALLOC(p_ctx, PVOID *, sizeof(PVOID)*2*(p_cbs->size + 1), -1);

		(p_cbs->num > 0) ? memcpy(p_cb, p_cbs->cbs, sizeof(pkt_cb)*p_cbs->num) : 0;
		(p_cbs->num > 0) ? memcpy(p_ctx, p_cbs->cbs, sizeof(void*)*p_cbs->num) : 0;

		if (p_cbs->cbs) free(p_cbs->cbs);
		if (p_cbs->ctxs) free(p_cbs->ctxs);

		p_cbs->cbs = p_cb;
		p_cbs->ctxs = p_ctx;
		p_cbs->size = 2 * (p_cbs->size + 1);
	}

	p_cbs->cbs[p_cbs->num] = pkt_fun;
	p_cbs->ctxs[p_cbs->num] = ctx;

	p_cbs->num++;

	return 0;
}

const pkt_cb_list_p xcap::get_pkt_cb_list(int pkt_type)
{
	switch (pkt_type) {
		case ETH_P_IP:
			return pkt_cbs[PKT_IP];
		case ETH_P_IPV6:
			return pkt_cbs[PKT_IPV6];
		case ETH_P_ARP:
			return pkt_cbs[PKT_ARP];
		case ETH_P_RARP:
			return pkt_cbs[PKT_RARP];
		case ETH_P_LOOP:
			return pkt_cbs[PKT_LOOP];
		default:
			return NULL;
	}

	return NULL;
}
int fetch_app_data(unsigned short eth_proto, char *pkt, int pkt_len, char **app_data, int *app_data_len)
{
	struct ip6_hdr *p_ip6 = NULL;
	struct iphdr *p_ip = NULL;
	struct udphdr *p_udp = NULL;
	struct tcphdr *p_tcp = NULL;

	*app_data = NULL;
	*app_data_len = 0;
	switch (eth_proto) {
		case ETH_P_IP:
			p_ip = (struct iphdr*)pkt;
			if (pkt_len <= sizeof(struct iphdr)) return -1;

			if (p_ip->protocol == IPPROTO_TCP) {
				if (pkt_len < p_ip->ihl*4 + sizeof(struct tcphdr)) return -1;
				p_tcp = (struct tcphdr*)(pkt + p_ip->ihl * 4);
				
				if (pkt_len < p_ip->ihl * 4 + p_tcp->doff * 4) return -1;
				*app_data = (pkt + p_ip->ihl * 4 + p_tcp->doff * 4);
				*app_data_len = pkt_len - p_ip->ihl * 4 - p_tcp->doff * 4;
			} else if (p_ip->protocol == IPPROTO_UDP) {
				if (pkt_len < p_ip->ihl*4 + sizeof(struct udphdr)) return -1;
				*app_data = (pkt + p_ip->ihl * 4 + sizeof(struct udphdr));
				*app_data_len = pkt_len - p_ip->ihl * 4 - sizeof(struct udphdr);
			} else {
			}
			break;
		case ETH_P_ARP:
			break;
		case ETH_P_LOOP:
			break;
		case ETH_P_RARP:
			break;
		case ETH_P_IPV6:
			break;
		default:
			break;
	}

	return 0;
}

static void xcap_default_pkt_proc(unsigned char* user_data, const struct pcap_pkthdr * pkt_hdr, const unsigned char *pkt_data)
{
	int i = 0;
	int pkt_len = 0;
	int eth_size = 0;
	pkt_cb_list_p p_pkt_cbs = NULL;
	unsigned short eth_proto = 0;
	struct ethhdr * p_eth = NULL;
	xcap * p_cap = (xcap*)user_data;

	pkt_len = pkt_hdr->caplen;
	if (p_cap->is_vlan()) {
		eth_proto = *(unsigned short*)(pkt_data + ETH_HLEN + 2); // src6_dst6_vlan4_proto2
		eth_size = ETH_HLEN + 4;
	} else {
		eth_proto = *(unsigned short*)(pkt_data + ETH_HLEN - 2); // src6_dst6_proto2
		eth_size = ETH_HLEN;
	}
	eth_proto = ntohs(eth_proto);

	if (eth_proto != ETH_P_IP && eth_proto != ETH_P_ARP && eth_proto != ETH_P_LOOP && eth_proto != ETH_P_RARP && eth_proto != ETH_P_IPV6) {
		return;
	}

	p_pkt_cbs = p_cap->get_pkt_cb_list(eth_proto);

	// with protocol headers
	for (i = 0; p_pkt_cbs && i < p_pkt_cbs->num; i++) {
		p_pkt_cbs->cbs[i]((PVOID)(pkt_data + eth_size), pkt_len - eth_size, p_pkt_cbs->ctxs[i]);
	}

	// callback for processing application data
	pkt_cb app_pkt_cb = NULL;
	PVOID app_pkt_cb_ctx = NULL;
	p_cap->get_app_pkt_cb(app_pkt_cb, app_pkt_cb_ctx);
	if (app_pkt_cb) {
		char *app_data = NULL;
		int app_data_len = 0;
		if (0 != fetch_app_data(eth_proto, (char*)pkt_data + eth_size, pkt_len - eth_size, &app_data, &app_data_len)) return;
		app_pkt_cb(app_data, app_data_len, app_pkt_cb_ctx);
	}

}
