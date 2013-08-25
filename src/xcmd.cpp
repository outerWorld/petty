
#include <errno.h>
#include <unistd.h>

#include <iostream>
using namespace std;

#include "xsh_cmd.h"
#include "xcmd.h"


xcmd::xcmd()
{
	chn_stream_buf.clear();
	cur_cmd.clear();
	cur_para.clear();
	cmds.clear();
	cmd_out.clear();
	
	//cmds.insert(pair<string, cmd_base*>(xsh_cmd::get_name(), xsh_cmd::get_inst()));
	cmds.insert(pair<string, cmd_base*>(string("xsh"), xsh_cmd::get_inst()));

}

xcmd::~xcmd()
{
}

int xcmd::xread(int fd)
{
	int len = 0, size = 0, ret = 0, f_ret = 0;
	int try_times = 0, read_on = 0;
	char *buf = NULL;
	buff * buf_str = NULL;
	map<int , buff>::iterator f_iter = chn_stream_buf.find(fd);
	pair<map<int, buff>::iterator, bool> ret_iter;

	if (f_iter == chn_stream_buf.end()) {
		ret_iter = chn_stream_buf.insert(pair<int, buff>(fd, buff()));
		if (ret_iter.second == false) return -1;
		f_iter = ret_iter.first;
	}

	buf_str = &(f_iter->second);
	read_on = 1;
	while (read_on) {
		size = buf_str->get_cap();
		if (size < 512) {
			buf_str->resize(buf_str->get_size() + 512 - size);
			size = 512;
		}
		buf = (char*)buf_str->get_buf() + buf_str->get_data_len();
		ret = read(fd, buf, size);
		if (ret > 0) {
			// re-length the string object.
			buf_str->add_len(ret);
		} else if (ret == 0) {
			break;
		} else {
			// error process
			switch (errno) {
				case EAGAIN:
					// try 3 times.
					if (try_times++ >= 3) {
						read_on = 0;
						f_ret = -1;
					}
					// continue to read.
					break;

				/* case EWOULDBLOCK: break;*/
				case EINTR:
					if (try_times++ >= 3) {
						read_on = 0;
						f_ret = -1;
					}
					// continue to read.
					break;

				case EBADF:
				case EINVAL:
				case EIO:
					read_on = 0;
					f_ret = -1;
					// return error.
					break;
				default:
					read_on = 0;
					f_ret = -1;
					// unknow error, return error.
					break;
			}
		}
	}

	return f_ret;
}

/*
command format: "GODBLESS\01type\01command\02parameter1\02parameter2\02\00"
			\00: start with byte value 0x00
			GODBLESS:
			\01:seperator of items
			\02:seperator of sub-items
		and note that, if command has no parameters, it must set a empty parameter like "command\02\02\00"; also, if the data in command or parameters has seperator '\01' or '\02', it may has problems occured.

exceptions:
	1.
	2.
	3.	
*/
#define MAGIC_STR	"GODBLESS"
#define MAGIC_STR_LEN (sizeof(MAGIC_STR) - 1)
#define CMD_PROTO_MIN_SZ 15
int xcmd::get_next_cmd(buff & cmdbuf)
{
	unsigned char *d = NULL;
	unsigned char type = 0;
	int i = 0, j = 0, len = 0;

	d = cmdbuf.get_data();
	len = cmdbuf.get_data_len();

	if (len < CMD_PROTO_MIN_SZ) {
		return -1;
	}

	enum {
			MAGIC = 0x00,
			COMMAND_TYPE,
			COMMAND,
			PARA
	}next_state = MAGIC;
	i = 0; j = 0;
	while (j < len && d[j] != '\00') {
		if (d[j] != '\02' && d[j] != '\01') {
			j++;
			continue;
		}
		switch (next_state) {
			case MAGIC:
				if (MAGIC_STR_LEN == (j-i) && memcmp(MAGIC_STR, d + i, MAGIC_STR_LEN) == 0) {
					next_state = COMMAND_TYPE;
				} else {
					// continue to find the next COMMAND.
					// skip '\00' for next command
					while (j < len && d[j] != '\00') {
						j++;
					}
				}
				break;

			case COMMAND_TYPE:
				if (1 != (j-i) || (d[j] != APP_CMD && d[j] != SH_CMD)) {
					// invalid command type, so omit this command and find the next command.
					// continue to find the next COMMAND.
					// skip '\00' for next command
					while (j < len && d[j] != '\00') {
						j++;
					}
					next_state = MAGIC;
				} else {
					cmd_type = d[j];	
					next_state = COMMAND;
				}
				break;

			case COMMAND:
				if (j - i > 0) {
					cur_cmd = string((char*)(d + i), j-i);
					next_state = PARA;
				} else {
					// invalid.
					// continue to find the next COMMAND.
					// skip '\00' for next command
					while (j < len && d[j] != '\00') {
						j++;
					}
					next_state = MAGIC;
				}
				break;

			case PARA:
				if (j - i > 0) {
					cur_para.push_back(string((char*)(d + i), j - i));
				} else {
				}
				break;
			default:
				break;
		}

		j++;
		i = j;
	}
	// check the end flag of command
	if (d[j] == '\0') {
		// cut the data parsed.

		if (next_state != PARA) {
			return 0;
		}
	} else {
	}

	return 0;
}

const string & xcmd::exec(int fd)
{
	map<int, buff>::iterator f_iter;
	map<string, cmd_base*>::iterator f_cmd_iter;
	string __out;

	cmd_out.clear();

	f_iter = chn_stream_buf.find(fd);
	if (f_iter == chn_stream_buf.end()) return string("");

	f_iter->second.show();
	while (-1 != get_next_cmd(f_iter->second)) {
		f_cmd_iter = cmds.find(cur_cmd);
		if (f_cmd_iter == cmds.end()) {
			continue;
		}
		f_cmd_iter->second->exec(cur_para, __out);
		cmd_out += __out;
		__out.clear();
	}

	return cmd_out;
}
