// -*- encoding = utf-8 -*-
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/resource.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "tutil.h"

/*
 * valid format:xxx.xxx.xxx.xxx
 */
int check_ip_valid(char *ip)
{
	char *pb, *pe;
	int len = 0;
	int seg_num = 4;
	int value = 0;

	pb = ip;
	pe = ip;
	while ('\0' != *pe) {
		while ('\0' != (*pe) && (*pe) != '.') {
			if ((*pe) < 0x30 || (*pe) > 0x39) return 0;
			pe++;
		}
		len = pe - pb;
		if (len == 0 || len > 3) return 0;
		value = 0;
		while (pb < pe) {
			value = value * 10 + (*(pb++)) - 0x30;
		}
		if (value < 0 || value > 255) return 0;
		seg_num--;
		if ('\0' == *pe) break;	
		pb++;
		pe++;
	}

	if (seg_num < 0 || seg_num >= 1) return 0;

	return 1;
}


int aggregate_cmd(char *buf, int buf_size, int argc, char *argv[])
{
	int i = 0;
	int len = 0;
	int buf_len = 0;
	
	buf[0] = '\0';

	for (i = 0; i < argc; i++) {
		len = snprintf(buf + buf_len, buf_size - buf_len, "%s ", argv[i]);
		if (len >= buf_size - buf_len) {
			return -1;
		}
		buf_len += len;
	}

	return 0;
}

int set_daemon()
{
	int i = 0;
	pid_t pid;
	struct rlimit rl;
	struct sigaction sa;
	int fd0, fd1, fd2;
	
	umask(0);

	if (getrlimit(RLIMIT_NOFILE, &rl) < 0) return -1;

	if ((pid = fork()) < 0) {
		return -1;
	} else if (pid != 0) exit(0);
	else { /* Child process: do nothing */}

	setsid();

	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) < 0) return -1;

	if (rl.rlim_max == RLIM_INFINITY) rl.rlim_max = 1024;
	for (i = 0; i < rl.rlim_max; i++) close(i);
	
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

	return 0;
}


unsigned int hash_calc(char *data, int len)
{
	unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;
	int i = 0;

    while (i < len) {
        hash = hash * seed + data[i];
		i++;
    }

    return (hash & 0x7FFFFFFF);
}

// "\b01010101" "\o07" "\x7A"
char *str_esc(const char *str, int str_len, char **key, int *key_len)
{
	char *p = NULL;
	int num = 0;
	int k_num = 0;
	int esc_type = 0x00;
	int esc_flag = 0;
	unsigned char value = 0;

	enum {
		ESC_BASE = 0x00,
		ESC_HEX,
		ESC_OCT,
		ESC_BIN
	};

	// after character escape, the length of result should be less than str_len, so capacity of str_len for *key is enough.
	MEM_ALLOC(*key, char *, str_len + 1, NULL);
	p = *key;
	
	memcpy(p, str, str_len);
	p[str_len] = '\0';
	*key_len = str_len;

	return p;

	// cases: "\b11111111" "\o377" "\xff"
	// cases: "\b111111"   "\o476" "\xfm"
	// cases: "\b112"

	for (int i = 0; i < str_len; i++) {
		switch (str[i]) {
			case '\\':
				if (esc_flag) {
					p[num++] = '\\';
					esc_flag = 0;
				} else {
					esc_flag = 1;
				}

				break;
			case 'x': case 'X': // hex
				if (esc_flag) {
				} else {
				}
				break;
			case 'o': case 'O': //octo

				break;
			case 'b': case 'B': // binary

				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				if (esc_flag) {
					if (k_num == 0) {
						
					} else if (k_num == 1) {
					}
				} else {
				}

				break;
			case 'a': /* case 'b': */ case 'c': case 'd': case 'e': case 'f':
			case 'A': /* case 'B': */case 'C': case 'D': case 'E': case 'F':

				break;
			default:
				break;
		}
	}

	return *key;
}
