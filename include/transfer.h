/*
 * @file name: transfer.h
 * @encoding : utf-8
 * @author   : huang chunping
 * @date	 : 2013-06-10
 * @history  :
 *				1. 2013-06-10: support udp and tcp, ipv4
 */
#ifndef __TRANSFER_H
#define __TRANSFER_H

#include <sys/un.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include "tutil.h"
#include "buff.h"

enum {
	IS_SERVER = 0x00,
	IS_CLIENT,
};

class transfer {
public:
	transfer(int role, int proto_type, char *bind_ip, unsigned short bind_port);
	~transfer();

	int init(int role, int proto_type, char *bind_ip, unsigned short bind_port);

	bool is_ok() { return status==ST_OK?true:false; }

	int nsend(char *dat, int dat_len, char *dst_ip = NULL, int port = -1);

	int nsend(char *dat, int dat_len, struct sockaddr_in *sa = NULL);

	int nrecv(struct sockaddr_in * sa = NULL, char *buf = NULL, int size = -1);
	
	buff & get_recv_buf() { return recv_buf; }

private:
	int fd;
	int status;
	int role;			// client or server
	int proto_type;		// UDP or tcp
	buff recv_buf;
	buff send_buf;
	struct sockaddr_in s_addr;  //server address
};

#endif // __TRANSFER_H
