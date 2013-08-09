/*
*/

#ifndef __XCAP_H
#define __XCAP_H

#include <pcap.h>

typedef int (*pkt_cb)(void *pkt, int pkt_size, void *ctx);
typedef struct _pkt_cb_list_s {
	int size;
	int num;
	PVOID * ctxs;
	pkt_cb *cbs;
}pkt_cb_list_t, *pkt_cb_list_p;

class xcap {
public:
	xcap(int xcap_type);
	~xcap();

	int init(char *dev, char *filter, int promisc, int to_ms);

	/* int add_dev(*dev, char *filter); */

	void set_pkt_proc(pcap_handler p_h, void *ctx) {
		p_pkt_proc = p_h;
		pkt_proc_ctx = ctx;
	}

	const pkt_cb_list_p get_pkt_cb_list(int pkt_type);

	void set_vlan() { with_vlan = 1; }
	bool is_vlan() { return with_vlan==1?true:false; }

	int add_pkt_cb(pkt_cb pkt_fun, int pkt_cap_type, void *ctx);

	void set_app_pkt_cb(pkt_cb pkt_fun, int pkt_cap_type, PVOID ctx)
	{
		app_pkt_cb = pkt_fun;
		app_pkt_cb_ctx = ctx;
		pkt_type = pkt_cap_type;
	}

	void get_app_pkt_cb(pkt_cb & pkt_fun, PVOID &ctx) {
		ctx = app_pkt_cb_ctx;
		pkt_fun = app_pkt_cb;
	}

	int xcap_run();

	void xcap_stop() { pcap_breakloop(dev_hd); }

public:
	typedef enum {
		XCAP_FILE = 0x00,
		XCAP_ETH,
	}XCAP_TYPE;

	typedef enum {
		PKT_LOOP = 0x00,
		PKT_IP,
		PKT_ARP,
		PKT_RARP,
		PKT_IPV6,
		PKT_ALL,
		PKT_CAP_SIZE
	}PKT_CAP_TYPE;

private:
	int	type;
	int pkt_type;
	int with_vlan;
	pcap_t *dev_hd; // device handler
	pcap_handler p_pkt_proc;
	void *pkt_proc_ctx;
	pkt_cb_list_p pkt_cbs[PKT_CAP_SIZE];

	pkt_cb app_pkt_cb; // now only support one callback for processing application pkt.	
	PVOID app_pkt_cb_ctx;
};

#endif // __XCAP_H
