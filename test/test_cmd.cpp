
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cmdline.h"

enum {
	OPT_NAME_TYPE_ID = 0x01,
	OPT_NAME_TYPE_FILTER_ID,
};

static const opt_profile_t opt_profile[] = {
	{"-t", 0, 1, OPT_NAME_TYPE_ID},
	{"-f", 1, 64, OPT_NAME_TYPE_FILTER_ID},
	{"--filter", 1, 64, OPT_NAME_TYPE_FILTER_ID},
	{"--type", 0, 1, OPT_NAME_TYPE_ID},
	{NULL, -1, -1, -1}
};

int main(int argc, char *argv[])
{
	int opt_idx = 0;
	int opt_id = 0;
	int arg_num = 0;
	void *arglist = NULL;

	if (argc < 2) return 0;
	if (0 != cmdline_parse(argc-1, &argv[1], (opt_profile_t*)&opt_profile)) {
		return 1;
	}

	while (-1 != cmdline_result_loop(&opt_idx, &opt_id, &arglist, &arg_num)) {
		switch (opt_id) {
			case OPT_NAME_TYPE_ID:
				fprintf(stdout, "argv[%d] = %s, ", opt_idx + 1, argv[opt_idx+1]);
				fprintf(stdout, "args = ");
				for (int i = 0; i < arg_num; i++) {
					fprintf(stdout, "%s ", cmdline_get_arg(arglist, i));
				}
				fprintf(stdout, "\n");

				break;
			case OPT_NAME_TYPE_FILTER_ID:
				fprintf(stdout, "argv[%d] = %s, ", opt_idx + 1, argv[opt_idx + 1]);
				fprintf(stdout, "args = ");
				for (int i = 0; i < arg_num; i++) {
					fprintf(stdout, "%s ", cmdline_get_arg(arglist, i));
				}
				fprintf(stdout, "\n");
				
				break;
			case -1:
				fprintf(stdout, "Not option: argv[%d] %s\n", opt_idx + 1, argv[opt_idx + 1]);
				break;
			default:
				break;
		}
	}

	//cmdline_show();

	

	cmdline_destroy();

	return 0;		
}
