/*
*/

#ifndef __XSH_CMD_H
#define __XSH_CMD_H

#include <string>
#include <vector>
using namespace std;

#include "cmd_base.h"

class xsh_cmd : public cmd_base {
public:
	xsh_cmd() { }
	xsh_cmd(const string & name) : cmd_base(name, NULL) { }

	virtual ~xsh_cmd() { }

	virtual int exec(const vector<string> &arg, string &out);

	static xsh_cmd * get_inst()
	{
		static xsh_cmd * p_cmd = new xsh_cmd("xsh");

		return p_cmd;
	}

	static const string & get_name() { return "xsh"; }

private:
};

#endif // __XSH_CMD_H
