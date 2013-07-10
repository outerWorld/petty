/*
*/

#ifndef __RADIUS_H
#define __RADIUS_H

// support 128 english letters or 42 chinese letters
#define USER_NAME_SZ	128

#define RADIUS_AUTH_LEN 16

// radius code
enum {
	RAD_ACCESS_REQ = 0x01,
	RAD_ACCESS_OK_RSP = 0x02,
	RAD_ACCESS_DENY_RSP = 0x03,
	RAD_ACCOUNT_REQ = 0x04,
	RAD_ACCOUNT_RSP = 0x05,
	RAD_ACESS_CHALLENGE = 0x0b,
	RAD_STATUS_SERVER = 0x0c,
	RAD_STATUS_CLIENT = 0x0d,
	RAD_CODE_MAX = 0x0e,	
	RAD_CODE_NUM = 8, // the number of code
};

// radius attribute types
enum {
	RAD_ATTR_BASE = 0x00,
	RAD_ATTR_USER_NAME = 0x01,
	RAD_ATTR_USER_PASSWD = 0x02,
	RAD_ATTR_CHAP_PASSWD = 0x03,
	RAD_ATTR_NAS_IP_ADDR = 0x04,
	RAD_ATTR_NAS_PORT = 0x05,
	RAD_ATTR_SERVICE_TYPE = 0x06,
	RAD_ATTR_FRAMED_PROTO = 0x07,
	RAD_ATTR_FRAMED_IP_ADDR = 0x08,
	RAD_ATTR_FRAMED_IP_NMASK = 0x09,
	RAD_ATTR_FRAMED_ROUTING = 0x0a,
	RAD_ATTR_FILTER_ID = 0x0b,
	RAD_ATTR_FRAMED_MTU = 0x0c,
	RAD_ATTR_FRAMED_COMPRESS = 0x0d,
	RAD_ATTR_LOGIN_IP_HOST = 0x0e,
	RAD_ATTR_LOGIN_SERVICE = 0x0f,
	RAD_ATTR_LOGIN_TCP_PORT = 0x10,
	/* RAD_ATTR_UNUSED = 0x11, */
	RAD_ATTR_REPLY_MSG = 0x12,
	RAD_ATTR_CALLBACK_NUM = 0x13,
	RAD_ATTR_CALLBACK_ID = 0x14,
	/* RAD_ATTR_UNUSED = 0x15, */
	RAD_ATTR_FRAMED_ROUT = 0x16,
	RAD_ATTR_FRAMED_IPX_NETWORK = 0x17,
	RAD_ATTR_STATE = 0x18,
	RAD_ATTR_CLASS = 0x19,
	RAD_ATTR_VENDOR_SPEC = 0x1a,
	RAD_ATTR_SESSION_TIMEOUT = 0x1b,
	RAD_ATTR_IDLE_TIMEOUT = 0x1c,
	RAD_ATTR_TERMIN_ACTION = 0x1d,
	RAD_ATTR_CALLED_STATION_ID = 0x1e,
	RAD_ATTR_CALLING_STATION_ID = 0x1f,
	RAD_ATTR_NSA_IDENTIFIER = 0x20,
	RAD_ATTR_PROXY_STATE = 0x21,
	RAD_ATTR_LOGIN_LAT_SERVICE = 0x22,
	RAD_ATTR_LOGIN_LAT_NODE = 0x23,
	RAD_ATTR_LOGIN_LAT_GROUP = 0x24,
	RAD_ATTR_FRAMED_APPLETALK_LINK = 0x25,
	RAD_ATTR_FRAMED_APPLETALK_NETWORK = 0x26,
	RAD_ATTR_FRAMED_APPLETALK_ZONE = 0x27,

	/*reserved for account: 0x28 ~ 0x3b */
	RAD_ATTR_ACCOUNT_STATUS = 0x028,

	RAD_ATTR_CHAP_CHALLENGE = 0x3c,
	RAD_ATTR_NAS_PORT_TYPE = 0x3d,
	RAD_ATTR_PORT_LIMIT = 0x3e,
	RAD_ATTR_LOGIN_LAT_PORT = 0x3f,
	RAD_ATTR_MAX
};

enum {
	RAD_USER_STATUS_BASE = 0x00,
	RAD_USER_STATUS_START = 0x01,
	RAD_USER_STATUS_STOP = 0x02,
	RAD_USER_STATUS_UPDATE = 0x03,
	RAD_USER_STATUS_MAX
};

typedef struct radius_header_s {
	unsigned char code;
	unsigned char identifier;
	unsigned short len;
	unsigned char  auth[RADIUS_AUTH_LEN];
}radius_header_t, *radius_header_p;

#define RAD_HEADER_SZ	sizeof(radius_header_t)

typedef struct radius_proto_header_s {
	unsigned char code;
	unsigned char identifier;
	unsigned short len;
	unsigned char  auth[0];
}radius_proto_header_t, *radius_proto_header_p;

typedef struct radius_attr_s {
	unsigned char type;
	unsigned char len;
}radius_attr_t, *radius_attr_p;

#define RAD_ATTR_HEAD_SZ	sizeof(radius_attr_t)

typedef struct blk_info_s {
	int len;
	char *addr;
}blk_info_t, *blk_info_p;

#endif // __RADIUS_H
