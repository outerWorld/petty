// -*- encoding = utf-8 -*-
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<arpa/inet.h>
#include <sys/un.h>
#include <time.h>

#include <errno.h>

#include "tutil.h"
#include "transfer.h"

transfer::transfer(int new_role, int new_proto_type, char *bind_ip, unsigned short bind_port)
{
	status = ST_OK;
	
	if (0 != init(new_role, new_proto_type, bind_ip, bind_port)) {
		status = ST_FAIL;
	}
}

transfer::~transfer()
{
	close(fd);
}

int transfer::init(int new_role, int new_proto_type, char *bind_ip, unsigned short bind_port)
{
	status = ST_OK;
	memset(&s_addr, 0x00, sizeof(s_addr));	
	role = new_role;
	proto_type = new_proto_type;

	s_addr.sin_family = AF_INET;
	if (bind_ip ) {
		if (! check_ip_valid(bind_ip)) return -1;
		if (inet_pton(AF_INET, bind_ip, (void*)&s_addr.sin_addr.s_addr) < 0) {
			status = ST_FAIL;
			return -1;
		}
	}

	s_addr.sin_port = htons(bind_port); 
	
	// ipv6 will be supported later
	fd = socket(AF_INET, proto_type, 0);
	if (fd < 0) {
		status = ST_FAIL;
		return -1;
	}

	if (IS_SERVER == role && 0 != bind(fd, (struct sockaddr*)&s_addr, sizeof(s_addr))) {
		status = ST_FAIL;
		return -1;
	}

	if (IS_CLIENT == role && proto_type == SOCK_STREAM && 0 != connect(fd, (struct sockaddr*)&s_addr, sizeof(s_addr))) {
		status = ST_FAIL;
		return -1;
	}

	if (IS_SERVER == role && proto_type == SOCK_STREAM && 0 != listen(fd, 5)) {
		status = ST_FAIL;
		return -1;
	}

	recv_buf.create(1024 * 10);
	send_buf.create(1024 * 10);

	return 0;
}


int transfer::nsend(char *dat, int dat_len, struct sockaddr_in *in_sa)
{
	int len = 0;
	int sent = 0;
	int remain = dat_len;
	int retry_count = 0;
	struct sockaddr_in sa;

	if (!in_sa) {
		memcpy(&sa, &s_addr, sizeof(sa));
	} else {
		memcpy(&sa, in_sa, sizeof(sa));
	}

	while (remain > 0) {
		len = sendto(fd, dat + sent, remain, 0, (struct sockaddr*)&sa, sizeof(sa));
		if (len == 0) {
			return -1;
		} else if (len < 0) {
			switch (errno) {
				case EAGAIN:
				case EINTR:
					usleep(1);
					if (retry_count >= 3) {
						status = ST_FAIL;
						return -1;
					}
					retry_count++;
					break;
				case EPIPE:
					close(fd);
					fd = -1;
				default:
					status = ST_FAIL;
					return -1;
			}
		}

		sent += len;
		remain -= len;
	}

	return 0;
}

int transfer::nsend(char *dat, int dat_len, char *dst_ip, int port)
{
	struct sockaddr_in sa;
	
	sa.sin_family = AF_INET;
	if (dst_ip && check_ip_valid(dst_ip)) {
		if (0 != inet_pton(AF_INET, dst_ip, (void*)&sa.sin_addr.s_addr)) {
			return -1;
		}
	} else {
		sa.sin_addr.s_addr = s_addr.sin_addr.s_addr;
	}

	if (port > 0) {
		sa.sin_port = htons(port);
	} else {
		sa.sin_port = s_addr.sin_port;
	}

	return nsend(dat, dat_len, &sa);
}

int transfer::nrecv(struct sockaddr_in * sa, char *buf, int size)
{
	int len = 0;
	int r_size = 0;
	char *r_buf = NULL;
	socklen_t addr_len = 0;


	if (buf) {
		r_buf = buf;
		r_size = size;
	} else {
		r_buf = recv_buf.get_buf();
		r_size = recv_buf.get_cap();
	}

	addr_len = sizeof(struct sockaddr_in);
	if (sa) {
		sa->sin_family = AF_INET;
		len = recvfrom(fd, r_buf, r_size, 0, (struct sockaddr*)sa, &addr_len);
	} else {
		len = recv(fd, r_buf, r_size, 0);
	}
	if (len == 0) {
		close(fd);
		fd = -1;
		status = ST_FAIL;
		return -1;
	} else if (len < 0) {
		switch (errno) {
			case EAGAIN:
			case EINTR:
				return 0;
			default:
				return -1;
		}
	}

	recv_buf.add_len(len);

	return 0;
}
