/*
*/

#ifndef __CMD_BASE_H
#define __CMD_BASE_H

#include <string>
#include <vector>
using namespace std;

#include "xcontext.h"

/* base class of command */
class cmd_base {
public:
	cmd_base() { ctx = NULL; }
	cmd_base(const string &cmd_name, xcontext *n_ctx) : name(cmd_name), ctx(n_ctx) {}
	virtual ~cmd_base() { }

	void set_name(const string &n_name)
	{
		// it must be deep copy
		name = n_name;
	}

	void set_context(xcontext *p_ctx)
	{
		ctx = p_ctx;
	}

	const string & get_name() { return name; }

	virtual int exec(const vector<string> &arg, string &out) { }

private:
	string 		name;
	xcontext 	*ctx;
};

#endif // __CMD_BASE_H
