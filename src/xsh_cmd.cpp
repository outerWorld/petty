
#include "xsh_cmd.h"

int xsh_cmd::exec(const vector<string> &arg, string &out)
{
	out += "xsh>>\n";
	out += "\tplease input your name";

	return 0;
}
