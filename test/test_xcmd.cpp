
#include <iostream>
#include <string>
using namespace std;

#include "xcmd.h"

int main(int argc, char *argv[])
{
	xcmd x_cmd;

	x_cmd.xread(0);
	string out = x_cmd.exec(0);
	
	cout << "execute = " << out << endl;

	return 0;
}

