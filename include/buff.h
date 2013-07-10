/*
 * @file name: buff.h
 * @encoding : utf-8
 * @author	 : huang chunping
 * @date	 : 2013-06-10
 * @history	 : 1. 2013-06-10, created
 */
#ifndef __BUFF_H
#define __BUFF_H

class buff {
public:
	buff() {
		size = 0;
		len = 0;
		buf = 0;
	}
	~buff() {
		if (buf) delete []buf;
		buf = 0;
	}

	int create(int new_size) {
		char *p = NULL;

		// if the new size is lower to old size, return
		if (new_size <= 0) return -1;
		if (new_size <= size) return 0;

		p = new char[new_size];
		if (!p) return -1;
		if (len > 0) memcpy(p, buf, len);

		if (buf) delete []buf;

		buf = p;
		size = new_size;

		return 0;
	}

	char* get_data() { return buf; }
	int get_data_len() { return len; }
	int	get_size() { return size; }
	int get_cap() { return (size - len); }
	char* get_buf() { return (buf + len); }
	void add_len(int new_len) { len += new_len; }

public:
	int	size;
	int	len;
	char *buf;
};

#endif // __BUF_H
