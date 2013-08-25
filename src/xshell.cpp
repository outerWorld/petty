
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>

#include <iostream>
#include <exception>

#include "xcmd.h"
#include "xshell.h"

static int xshell::addr_size[xshell::IPv_MAX] = {sizeof(struct sockaddr_in), sizeof(struct sockaddr_in6)};

class xshell_except : public exception {
public:
	xshell_except(const string & msg) throw() {}
	~xshell_except() throw() { }

private:
	string e_msg;
};
static xshell_except xshell_e("");

xshell::xshell(char *s_ip, unsigned short s_port, unsigned short iptype) throw()
{
	struct sockaddr_in *sa4 = NULL;
	struct sockaddr_in6 *sa6 = NULL;

	srv_port = htons(s_port);
	// prepare buffer for the socket address.
	addr = (unsigned char*)malloc(sizeof(struct sockaddr_in6));
	if (!addr) throw xshell_e;
	memset(addr, 0x00, sizeof(struct sockaddr_in6));

	// address
	if (iptype == IPv4) {
		sa4 = (struct sockaddr_in*)addr;
		sa4->sin_family = AF_INET;
		sa4->sin_port = htons(s_port);
		if (inet_pton(AF_INET, s_ip, &sa4->sin_addr.s_addr) <= 0) throw xshell_e;

		s_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (s_fd) throw xshell_e;

	} else if (iptype == IPv6) {
		sa6 = (struct sockaddr_in6*)addr;
		sa6->sin6_family = AF_INET6;
		sa6->sin6_port = htons(s_port);
		if (inet_pton(AF_INET6, s_ip, &sa6->sin6_addr) <= 0) throw xshell_e;
		s_fd = socket(AF_INET6, SOCK_STREAM, 0);
		if (s_fd) throw xshell_e;

	} else throw xshell_e;


	srv_iptype = iptype;
}

xshell::~xshell()
{
	if (addr) {
		free(addr);
	}

	if (s_fd > 0) {
		close(s_fd);
	}
}

typedef struct cmd_proc_ctx_s {
	int 			proc_id;	// id of command processor.
	msg_vector_t	*msg_que;	// message queue
	xshell			*caller;
}cmd_proc_ctx_t, *cmd_proc_ctx_p;

int xshell::init()
{
	int yes = 1;

	if (setsockopt(s_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		return -1;
	}

	bind(s_fd, (struct sockaddr*)addr, addr_size[srv_iptype]);
	listen(s_fd, 5);

	cmd_proc_ctx_t ctxs[CMD_PROC_NUM];
	for (int i = 0; i < CMD_PROC_NUM; i++) {
		ctxs[i].proc_id = i;
		ctxs[i].caller = this;
		ctxs[i].msg_que = &cmd_msg_que[i];
		if (pthread_create(&cmd_procs[i], NULL, cmd_proc, &ctxs[i]) != 0) {
			return -1;
		}
	}

	return 0;
}

/*
format for command msg:
	\0<ip encrypt code>command para0 para1 .... paran\r\n\0
	<ip encrypt code> = MD5(ip), 16 bytes. ip is in network order bytes
	command = , seperated by \01
	para0,para1.... = parameters of command, seperated by ' ' 
	the completed command is ended with \r\n\0
*/

static void * xshell::cmd_proc(PVOID arg)
{
	cmd_proc_ctx_p p_ctx = (cmd_proc_ctx_p)arg;
	xshell *p_sh = p_ctx->caller;
	msg_vector_t *msg_que = p_sh->cmd_msg_que;
	msg_vector_t::iterator ibeg;
	char *ip_data = NULL;
	int ip_len = 0;
	int buf_sz = 0;
	char *buf = NULL;
	char addr[ADDR_SIZE] = { 0 };
	int addr_len = 0;
	struct sockaddr_in *sa4 = (struct sockaddr_in*)addr;
	struct sockaddr_in6 *sa6 = (struct sockaddr_in6*)addr;
	xcmd x_cmd;

	while (p_sh->running) {
		// recv data from queue.
		if (msg_que->empty()) {
			usleep(10);
			continue;
		}

		ibeg = msg_que->begin();

		switch (ibeg->op) {
			case SOP_DEL:
				// find command buffer for this fd
				x_cmd.clear(ibeg->s_info.fd);
				close(ibeg->s_info.fd);
				break;
			case SOP_RCV:
			{
				x_cmd.xread(ibeg->s_info.fd);
				const string &out = x_cmd.exec(ibeg->s_info.fd);
				write(ibeg->s_info.fd, out.c_str(), out.length());	
			}
				break;
			case SOP_SEND:
				break;
			default:
				break;
		}
		msg_que->erase(ibeg);
	}

	return p_sh;
}

static void* xshell::shell_proc(PVOID arg)
{
	int i = 0;
	int ret = 0;
	fd_set rfds, efds;
	int fds_max = 0;
	int fds_min = 0;
	struct timeval tm_o;
	char cli_addr[ADDR_SIZE] = { 0 };
	xshell * p_shell = NULL;

	p_shell = (xshell*)arg;

	FD_ZERO(&rfds);
	FD_SET(p_shell->s_fd, &rfds);
	if (p_shell->s_fd > fds_max) {
		fds_max = p_shell->s_fd;
	}
	if (fds_max > FD_SETSIZE) {
		return NULL;
	}

	if (fds_min > p_shell->s_fd) {
		fds_min = p_shell->s_fd;
	}

	tm_o.tv_usec = 100;
	tm_o.tv_sec = 0;
	while (1) {
		ret = select(fds_max, &rfds, &efds, NULL, &tm_o);
		if (ret == 0) {
			continue;
		} else if (ret == -1) {
			switch (errno) {
				case EBADF:
					break;
				case EINTR:
					break;
				case EINVAL:
					break;
				case ENOMEM:
					break;
				default:
					break;
			}
		} else {
			for (i = fds_min; i < fds_max; i++) {
				if (!FD_ISSET(i, &rfds)) {
					continue;
				}
				if (p_shell->s_fd == i) {
					int addr_len = ADDR_SIZE;
					int cli_fd = accept(i, (struct sockaddr*)cli_addr, &addr_len);
					if (-1 == cli_fd) {
						switch (errno) {
							case EMFILE:
							case ENFILE:
								break;
							default:
								break;
						}
					} else {
						// the new fd must not be large than the capacity of rfds;
						if (cli_fd >= FD_SETSIZE) {
							close(cli_fd);
							continue;
						}
						if (cli_fd < fds_min) {
							fds_min = cli_fd;
						}
						if (cli_fd > fds_max) {
							fds_max = cli_fd;
						}
						FD_SET(cli_fd, &rfds);
						p_shell->conn_count += 1;
					}
				} else {
					// read data from client and send it to processors.
				}
			}
		}
	}

	return p_shell;
}

int xshell::run(int new_instance)
{
	pthread_t tid;

	if (new_instance) {
		if (0 != pthread_create(&tid, NULL, shell_proc, this)) {
			return -1;
		}
	} else {
		if (NULL == shell_proc(this)) return -1;
	}

	return 0;
}
