/*
 * @file name: tqueue.h
 * @encoding : utf-8
 * @author	 : huang chunping
 * @date	 : 2013-06-14
 * @history	 : 1. 2013-06-14, created
 * @		   2. 2013-06-15, loop buffer
 */

#ifndef __TQUEUE_H
#define __TQUEUE_H

typedef struct _blk_header_s {
	int len;	// length of data in block
}blk_header_t, *blk_header_p;

class tqueue {
public:
	tqueue(int e_size, int e_num, int mt_support);
	~tqueue();

	int init(int e_size, int e_num, int mt_support);

	bool is_empty() { return (front==tail); }
	bool is_full() { return ((tail+1)%e_num == front); }

	int front_lock();
	int front_unlock();
	int get_front_data(char **blk, int *len);
	int pop();

	int get_esize() { return e_size; }
	int tail_lock();
	int tail_unlock();
	int fetch_blk(char **blk);
	int push(char *blk, int blk_len);

public:
	enum {
		ST_SUPPORT = 0x00,
		MT_SUPPORT
	};

private:
	int status;
	int	td_mode;	// multi-thread or single thread supported	
	pthread_mutex_t * que_f_mutex; // mutex for front block
	pthread_mutex_t * que_t_mutex; // mutex for tail block
	int	front;
	int	tail;
	int	e_size;
	int	e_num;
	int	blk_size; // blk_size = e_size + sizeof(blk_header_t)
	char *blk;
};

#endif // __TQUEUE_H
