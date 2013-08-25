/*
 *	support IPv4 and IPv6
 *	support multi-threads
 */

#ifndef __XSHELL_H
#define __XSHELL_H

#include <vector>
#include <map>
#include <string>

#include "tutil.h"

#define ADDR_SIZE	sizeof(struct sockaddr_in6)

// socket information
typedef struct _sock_info_s {
	int fd;
	int	 addr_len;
	char addr[ADDR_SIZE];
}sock_info_t, *sock_info_p;

//socket operation
enum {
	SOP_DEL,	// error occurs on socket, deletion is needed.
	SOP_RCV,	// data comes on socket, recv is needed.
	SOP_SEND,	// send op is available on socket
	SOP_MAX
};
typedef struct _in_msg_s {
	int			op;
	sock_info_t s_info;
}in_msg_t, *in_msg_p;

typedef std::vector<in_msg_t> msg_vector_t;

class xshell {
public:
	#define CMD_PROC_NUM	5	// number of command processors
	#define DFT_SRV_PORT 19862
	enum {
		IPv4 = 0x00,
		IPv6,
		IPv_MAX
	};
	static int addr_size[IPv_MAX];

public:
	xshell(char *s_ip = "0.0.0.0", unsigned short s_port = DFT_SRV_PORT, unsigned short iptype = IPv4) throw();
	~xshell();

	int init();

	static void *cmd_proc(PVOID arg);
	static void *shell_proc(PVOID arg);

	int run(int new_instance = 0);

	int stop() { running = 0; }

	int wait();

	int execute(const char *cmdname, const char *paras);

private:
	int				conn_count;	// count of connection
	int				s_fd;		// server socket file descriptor
	unsigned char 	*addr;		//  buffer size = sizeof(sockaddr_in6)
	unsigned short 	srv_port;	// service port
	unsigned short	srv_iptype;	// IPv4 or IPv6
	msg_vector_t	cmd_msg_que[CMD_PROC_NUM];	// message queues between command processors and shell_proc which monitor status of socket.
	pthread_t 		cmd_procs[CMD_PROC_NUM];
	int 			running;
};

#endif // __XSHELL_H
