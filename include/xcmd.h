/*
 */

#ifndef __XCMD_H
#define __XCMD_H


#include <vector>
#include <string>
#include <map>
using namespace std;

#include "buff.h"
#include "cmd_base.h"

class xcmd {
public:
	/* command type */
	enum {
		UNKNOWN = 0x00,
		APP_CMD,	// run in application.
		SH_CMD,		// call the shell to run the command.
	};

public:
	xcmd();
	~xcmd();

	/* read command data from socket fd or file fd */
	int xread(int fd);

	/* when the command has been read completely, it can be executed */
	const string & exec(int fd);

	int clear(int fd)
	{
		map<int, buff>::iterator ib ;

		if (fd >= 0) {
			ib = chn_stream_buf.find(fd);
			if (ib != chn_stream_buf.end()) ib->second.clear();
			return 0;
		}

		for (ib = chn_stream_buf.begin(); ib != chn_stream_buf.end(); ) {
			ib->second.clear();
			ib++;
		}

		return 0;
	}

private:
	int get_next_cmd(buff &);

private:
	/* command stream buffer, multiple channles supported */
	map<int, buff> chn_stream_buf;	// channel: can be socket fd 

	/* command parsed from data stream received */
	unsigned int cmd_type;
	string cur_cmd;	// current command name parsed from chn_stream_buf.
	vector<string> cur_para;	// current command parameter parsed from chn_stream_buf.
	string cmd_out;	// result of command execution.

	map<string, cmd_base *> cmds;
};

#endif // __XCMD_H
