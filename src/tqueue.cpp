// -*- encoding=utf-8 -*-
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pthread.h>

#include "tutil.h"
#include "tqueue.h"

tqueue::tqueue(int e_nsize, int e_nnum, int mt_support)
{
	status = ST_SUC;
	if (init(e_nsize, e_nnum, mt_support) != 0) status = ST_FAIL;
}

tqueue::~tqueue()
{
	if (que_t_mutex) {
		pthread_mutex_destroy(que_t_mutex);
		MEM_FREE(que_t_mutex);
	}
	if (que_f_mutex) {
		pthread_mutex_destroy(que_f_mutex);
		MEM_FREE(que_f_mutex);
	}

	MEM_FREE(blk);
}

int tqueue::init(int e_size, int e_num, int mt_support)
{
	status = ST_FAIL;
	front = 0;
	tail = 0;
	this->e_size = e_size;
	this->e_num = e_num;

	que_f_mutex = 0;
	que_t_mutex = 0;
	td_mode = mt_support;
	if (mt_support == MT_SUPPORT) {
		MEM_ALLOC(que_f_mutex, pthread_mutex_t*, sizeof(pthread_mutex_t), -1);
		if (pthread_mutex_init(que_f_mutex, NULL) != 0) return -1;

		MEM_ALLOC(que_t_mutex, pthread_mutex_t*, sizeof(pthread_mutex_t), -1);
		if (pthread_mutex_init(que_t_mutex, NULL) != 0) return -1;
	}

	blk_size = this->e_size + sizeof(blk_header_t);
	MEM_ALLOC(blk, char *, blk_size*this->e_num, -1);

	status = ST_SUC;
	return 0;
}

int tqueue::front_unlock()
{
	return (que_f_mutex?pthread_mutex_unlock(que_f_mutex):0);
}

int tqueue::front_lock()
{
	return (que_f_mutex?pthread_mutex_trylock(que_f_mutex):0);
}


int tqueue::get_front_data(char **o_blk, int *len)
{
	char *addr = NULL;
	
	if (front_lock() != 0 || !o_blk) return -1;
	if (is_empty()) {
		front_unlock();
		return -2;	
	}

	// Attention: multiplication may consume some performance, and also, this function may be called frequently, so here can be optimized.
	addr = blk + front * blk_size;
	if (o_blk) *o_blk = addr + sizeof(blk_header_t);

	if (len) *len = ((blk_header_p)addr)->len;

	return 0;
}

int tqueue::pop()
{
	front = (front + 1) % e_num;

	return front_unlock();
}

int tqueue::tail_unlock()
{
	return (que_t_mutex ? pthread_mutex_unlock(que_t_mutex) : 0);
}

int tqueue::tail_lock()
{
	return (que_t_mutex ? pthread_mutex_trylock(que_t_mutex) : 0);
}

int tqueue::fetch_blk(char **o_blk)
{
	char *addr = NULL;

	if (tail_lock() != 0) return -1;
	if (is_full()) {
		tail_unlock();
		return -2;
	}

	// Attention: multiplication may consume some performance, and also, this function may be called frequently, so here can be optimized.
	addr = blk + tail * blk_size;
	if (o_blk) *o_blk = addr + sizeof(blk_header_t);

	return this->e_size;
}

int tqueue::push(char *in_blk, int blk_len)
{
	char *addr = NULL;

	addr = blk + tail * blk_size;
	((blk_header_p)addr)->len = blk_len;

	tail = (tail + 1) % e_num;
	
	return tail_unlock();
}
