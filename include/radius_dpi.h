/*
 */

#ifndef __RADIUS_DPI_H
#define __RADIUS_DPI_H

#include "radius.h"

typedef blk_info_t rad_attr_info_t;
typedef blk_info_t *rad_attr_info_p;

class radius_dpi {
public:
	radius_dpi();
	~radius_dpi();

	// choose the attributes you want to dpi, if type equals to RAD_ATTR_MAX, then all the attributes will be enabled.
	int enable_attr_dpi(int type);

	// reversed function as enable_attr_dpi
	int disable_attr_dpi(int type);

	int enable_code(int code_type);
	int disable_code(int code_type);

	int set_attr_dpi(int type, int val)
	{
		if (type >= RAD_ATTR_MAX) return -1;
		dpi_map[type] = (val == 1 ? 1 : 0);
		return 0;
	}

	int set_code_dpi(int type, int val)
	{
		if (type >= RAD_CODE_MAX) return -1;
		dpi_code_map[type] = (val == 1 ? 1 : 0);
		return 0;
	}

	bool is_code_needed(int code_type)
	{
		return code_type>=RAD_CODE_MAX ? false: (dpi_code_map[code_type] == 1 ? true: false);
	}

	int parse(char *radius_pkt, int pkt_len);

	rad_attr_info_p get_attr(int type);

	//
	int attr_reset()
	{
		memset(rad_attrs, 0x00, sizeof(rad_attr_info_t)*RAD_ATTR_MAX);
		memset(&rad_head, 0x00, sizeof(radius_header_t));
		status = DPI_INIT;
	}

public:
	enum {
		DPI_INIT = 0x00,
		DPI_OK,
		DPI_FAIL,
	};

private:
	int				status;
	radius_header_t rad_head;
	rad_attr_info_t rad_attrs[RAD_ATTR_MAX];
	unsigned char	dpi_map[RAD_ATTR_MAX];
	unsigned char	dpi_code_map[RAD_CODE_MAX];
};

#endif // __RADIUS_DPI_H
