
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <errno.h>

/*
 * test_socket type server_ip server_port
 */
#define BUF_SZ 1024
static char *sendbuf;
static char *recvbuf;

enum {
	SERVER = 0x00,
	CLIENT
};

#define _ERR(err) fprintf(stderr, "%s failed! [%d,%s]\n", err, errno, strerror(errno))

int main(int argc, char *argv[])
{
	int type;
	int sock = -1;
	int sock_rem = -1;
	int ret = 0;
	char *serv_ip;
	unsigned short serv_port;
	int recvbuf_sz = 0;
	int sendbuf_sz = 0;
	socklen_t s_len = 0;
	struct sockaddr_in s_addr = { 0 };
	struct sockaddr_in rem_addr = { 0 };

	if (argc < 4) return 0;
	type = atoi(argv[1]);
	serv_ip = strdup(argv[2]);
	serv_port = atoi(argv[3]);

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) return 1;

	s_len = sizeof(recvbuf_sz);
	if (-1 == getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recvbuf_sz, &s_len)) {
		_ERR("getsockopt SO_RCVBUF");
		return 1;
	}
	fprintf(stdout, "default recvbuf_sz = %d\n", recvbuf_sz);

	s_len = sizeof(sendbuf_sz);
	if (-1 == getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendbuf_sz, &s_len)) {
		_ERR("getsockopt SO_SNDBUF");
		return 1;
	}
	fprintf(stdout, "default sendbuf_sz = %d\n", sendbuf_sz);

	sendbuf_sz = 1024*10;
	recvbuf_sz = 1024 * 2;
	if (-1 == setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recvbuf_sz, sizeof(recvbuf_sz))) {
		_ERR("setsockopt SO_RCVBUF");
		return 1;
	}
	if (-1 == setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendbuf_sz, sizeof(sendbuf_sz))) {
		_ERR("setsockopt SO_SNDBUF");
		return 1;
	}

	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(serv_port);
	inet_pton(AF_INET, serv_ip, &s_addr.sin_addr.s_addr);
	if (type == SERVER) {
		//fcntl(sock, F_SETFL, O_NONBLOCK);
		if (0 != bind(sock, (struct sockaddr*)&s_addr, sizeof(s_addr))) {
			close(sock);
			_ERR("bind ");
			return 0;
		}
		listen(sock, 5);
		s_len = sizeof(rem_addr);
		sock_rem = accept(sock, (struct sockaddr*)&rem_addr, &s_len);
		sendbuf = (char*)malloc(sendbuf_sz + 1024);
		if (!sendbuf) return 1;
		memset(sendbuf, 'a', sendbuf_sz + 1024);
		sendbuf[recvbuf_sz] = 'b';
		sendbuf[recvbuf_sz + 1] = '\0';
		ret = send(sock_rem, sendbuf, sendbuf_sz + 1024, MSG_DONTWAIT);
		fprintf(stdout, "server send [%d] bytes, tried [%d] bytes\n", ret, sendbuf_sz);
		close(sock_rem);
	} else {
		if (0 != connect(sock, (struct sockaddr*)&s_addr, sizeof(s_addr))) {
			close(sock);
			_ERR("connect ");
			return 1;
		}
		recvbuf = (char*)malloc(recvbuf_sz - 512);
		if (!recvbuf) return 1;

		do {
			ret = recv(sock, recvbuf, recvbuf_sz - 512, 0);
			fprintf(stdout, "client recv [%d] bytes\n", ret);
		} while(ret > 0);
	}

	while (1) {
		sleep(1);
	}

	close(sock);

	return 0;
}
