/*
 * @file name: tutil.h
 * @encoding : utf-8
 * @author	 : huang chunping
 * @date	 : 2013-06-10
 */

#ifndef __TUTIL_H
#define __TUTIL_H

typedef void * PVOID;

#define IP_V4_SIZE 4

#define MEM_ALLOC(paddr, type, size, err) \
paddr = (type)malloc(size);\
if (!(paddr)) return (err); \
memset(paddr, 0x00, size);

#define MEM_FREE(paddr) free(paddr)

typedef enum {
	ST_SUC = 0x00,
	ST_OK,
	ST_FAIL
}STATUS;

int check_ip_valid(char *ip);

int aggregate_cmd(char *buf, int buf_size, int argc, char *argv[]);

int set_daemon();

unsigned int hash_calc(char *data, int len);

// it is to escape the characters like '\0b' to 0x0b
char *str_esc(const char *str, int str_len, char **key, int *key_len);

#endif // __TUTIL_H
