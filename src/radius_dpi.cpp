
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "arpa/inet.h"

#include "radius.h"
#include "radius_dpi.h"

radius_dpi::radius_dpi()
{
	memset(&rad_head, 0x00, sizeof(rad_head));
	memset(rad_attrs, 0x00, sizeof(rad_attr_info_t) * RAD_ATTR_MAX);
	memset(dpi_map, 0x00, sizeof(char) * RAD_ATTR_MAX);
	memset(dpi_code_map, 0x00, sizeof(char) * RAD_CODE_MAX);
	
	status = DPI_INIT;
}

radius_dpi::~radius_dpi()
{
}

int radius_dpi::enable_attr_dpi(int type)
{
	if (type > RAD_ATTR_MAX) {
		for (int i = 0; i < RAD_ATTR_MAX; i++) {
			set_attr_dpi(i, 1);
		}
	} else {
		set_attr_dpi(type, 1);
	}

	return 0;
}

int radius_dpi::disable_attr_dpi(int type)
{
	if (type >= RAD_ATTR_MAX) {
		for (int i = 0; i < RAD_ATTR_MAX; i++) {
			set_attr_dpi(i, 0);
		}
	} else {
		set_attr_dpi(type, 0);
	}
	return 0;
}

int radius_dpi::enable_code(int code_type)
{
	if (code_type >= RAD_CODE_MAX) {
		for (int i = 0; i < RAD_CODE_MAX; i++) {
			set_code_dpi(i, 1);
		}
	} else {
		set_code_dpi(code_type, 1);
	}

	return 0;
}

int radius_dpi::disable_code(int code_type)
{
	if (code_type >= RAD_CODE_MAX) {
		for (int i = 0; i < RAD_CODE_MAX; i++) {
			set_code_dpi(i, 0);
		}
	} else {
		set_code_dpi(code_type, 0);
	}
	return 0;
}


rad_attr_info_p radius_dpi::get_attr(int type)
{
	if (type >= RAD_ATTR_MAX) return NULL;
	
	return &rad_attrs[type];
}


int radius_dpi::parse(char *radius_pkt, int pkt_len)
{
	int remain_len = 0;
	char *attr_addr = NULL;
	radius_attr_p p_attr = NULL;
	radius_proto_header_p p_head = NULL;

	if (pkt_len < RAD_HEADER_SZ) {
		status = DPI_FAIL;
		return -1;
	}

	p_head = (radius_proto_header_p)radius_pkt;
	if (! is_code_needed(p_head->code)) {
		status = DPI_FAIL;
		return -1;
	}

	p_head->len = ntohs(p_head->len);
	if (p_head->len > pkt_len) {
		status = DPI_FAIL;
		return -1;
	}
	// store the header
	memcpy(&rad_head, p_head, RAD_HEADER_SZ);

	//remain_len = pkt_len - RAD_HEADER_SZ;
	attr_addr = radius_pkt + RAD_HEADER_SZ;
	remain_len = pkt_len - RAD_HEADER_SZ;
	while (remain_len >= RAD_ATTR_HEAD_SZ) {
		int attr_len = 0;
		p_attr = (radius_attr_p)attr_addr;
		attr_len = p_attr->len;
		if (attr_len > remain_len) {
			status = DPI_FAIL;
			return -1;
		}
		
		if (p_attr->type < RAD_ATTR_MAX) {
			rad_attrs[p_attr->type].len = attr_len - RAD_ATTR_HEAD_SZ;
			rad_attrs[p_attr->type].addr = attr_addr + RAD_ATTR_HEAD_SZ;
		}

		remain_len -= attr_len;
		attr_addr += attr_len;
	}

	return 0;
}
