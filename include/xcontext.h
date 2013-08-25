/*
 */
#ifndef __XCONTEXT_H
#define __XCONTEXT_H

#include "tutil.h"

#include <string.h>
#include <stdlib.h>

class xcontext {
public:
	typedef int (*ctx_cleanup_func)(void *cxt, int sz);
public:
	xcontext(void *n_ctx, int sz, void *clean_up)
	{
		set_ctx(n_ctx, sz, clean_up);		
	}

	virtual ~xcontext()
	{
		if (p_cleanup) {
			p_cleanup(ctx, size);
		}	
	}

	int set_ctx(void *n_ctx, int sz, void *clean_up)
	{
		ctx = n_ctx;
		size = sz;
		p_cleanup = (ctx_cleanup_func)clean_up;

		return 0;
	}

	int dup_ctx(void *n_ctx, int n_size, void *clean_up)
	{
		MEM_ALLOC(ctx, void*, n_size, -1);
		memcpy(ctx, n_ctx, n_size);
		size = n_size;
		p_cleanup = (ctx_cleanup_func)clean_up;
	
		return 0;
	} 

	void * get_ctx(int &sz)
	{
		sz = size;
		return ctx;
	}

	/* virtual int destroy() = NULL; */

private:
	int 				size;
	void				*ctx;
	ctx_cleanup_func 	p_cleanup;
};

#endif // __XCONTEXT_H
