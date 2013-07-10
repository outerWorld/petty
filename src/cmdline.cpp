
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <stdarg.h>
#include "tutil.h"
#include "cmdline.h"

#define SET_ERR(buf, sz, fmt, ...) snprintf(buf, sz, fmt, __VA_ARGS__)

static opt_result_t g_opt_result = { 0 };

static int loop_seq = 0;

int cmdline_init()
{
	memset(&g_opt_result, 0x00, sizeof(opt_result_t));

	return 0;
}

// before calling this function, it must be sure p_prof is ended by {NULL, -1, -1}
static int opt_search(char *opt_name, opt_profile_p p_prof)
{
	int i = 0;

	while (p_prof[i].opt_name) {
		if (strncmp(opt_name, p_prof[i].opt_name, strlen(opt_name)) == 0) {
			return i;
		}
		i++;
	}

	return -1;
}

int cmdline_parse(int argc, char **argv, opt_profile_p p_profile)
{
	int len = 0;
	if (argc < 1) return 0;

	g_opt_result.opt_num = 0;
	g_opt_result.val_num = 0;
	g_opt_result.size = argc;
	MEM_ALLOC(g_opt_result.p_opts, opt_node_p, sizeof(opt_node_t)*g_opt_result.size, -1);
	MEM_ALLOC(g_opt_result.p_vals, opt_node_p, sizeof(opt_node_t)*g_opt_result.size, -1);

	opt_node_p p_opts = g_opt_result.p_opts;
	opt_node_p p_vals = g_opt_result.p_vals;
	opt_node_p p_cur_opt = NULL;
	int arg_needed = 0;
	int	arg_coll_num = 0;
	int optional = 0;
	int	opt_num = 0;
	int val_num = 0;
	int	opt_idx = -1;

	for (int idx = 0; idx < argc; idx++) {
		len = strlen(argv[idx]);

		// think it: deal with optional arguments for option.
		switch (argv[idx][0]) {
			case '-':
				// if it is the first or last option in commandline, error will occured.
				if (optional) {
					p_cur_opt->arg_num = arg_coll_num;
					optional = 0;
				} else {
					// check whether it has collected enough values for current option
					if (arg_coll_num < arg_needed) return -1;
					p_cur_opt ? p_cur_opt->arg_num = arg_coll_num : 0;
					arg_needed = 0;
				}

				opt_idx = opt_search(argv[idx], p_profile);
				if (-1 == opt_idx) return -1;

				p_cur_opt = NULL;
				p_opts[opt_num].raw_idx = idx;
				p_opts[opt_num].opt_type = OPT_OPT;
				p_opts[opt_num].kv.opt_name = strdup(argv[idx]);
				p_opts[opt_num].id = p_profile[opt_idx].id;
				p_opts[opt_num].arg_size = p_profile[opt_idx].arg_num;

				optional = p_profile[opt_idx].optional;
				if (optional) p_cur_opt = &p_opts[opt_num];
				arg_needed = p_profile[opt_idx].arg_num;
				arg_coll_num = 0;
				if (arg_needed > 0) {
					MEM_ALLOC(p_opts[opt_num].arglist, opt_node_p, sizeof(opt_node_t)*arg_needed, -1);
					p_cur_opt = &p_opts[opt_num];
				}
				opt_num++;

				break;
			default:
				// if optional is still effective, then added value to current option
				if (optional) {
					if (p_cur_opt) {
						if (arg_coll_num >= arg_needed) {
							p_cur_opt->arg_num = arg_coll_num;	
							optional = 0;
							p_cur_opt = NULL;
#if 0
							p_vals[val_num].opt_type = OPT_VAL;
							p_vals[val_num].kv.opt_val = strdup(argv[idx]);
							p_vals[val_num].raw_idx = idx;
							val_num++;
#else
							g_opt_result.p_opts[opt_num].opt_type = OPT_VAL;
							g_opt_result.p_opts[opt_num].kv.opt_val = strdup(argv[idx]);
							g_opt_result.p_opts[opt_num].raw_idx = idx;
						g_opt_result.p_opts[opt_num].id = -1;
							opt_num++;
#endif
						} else {
							p_cur_opt->arglist[arg_coll_num].raw_idx = idx;
							p_cur_opt->arglist[arg_coll_num].opt_type = OPT_VAL;
							p_cur_opt->arglist[arg_coll_num].kv.opt_val = strdup(argv[idx]);
							arg_coll_num++;
							if (arg_coll_num >= arg_needed) {
								p_cur_opt->arg_num = arg_coll_num;	
								optional = 0;
								p_cur_opt = NULL;
							}
						}
					} else return -1;
				} else {
					if (arg_needed > 0 && p_cur_opt) {
						if (arg_coll_num >= arg_needed) {
							p_cur_opt->arg_num = arg_coll_num;
							p_cur_opt = NULL;
#if 0
							p_vals[val_num].opt_type = OPT_VAL;
							p_vals[val_num].kv.opt_val = strdup(argv[idx]);
							p_vals[val_num].raw_idx = idx;
							val_num++;
#else
						g_opt_result.p_opts[opt_num].opt_type = OPT_VAL;
						g_opt_result.p_opts[opt_num].kv.opt_val = strdup(argv[idx]);
						g_opt_result.p_opts[opt_num].raw_idx = idx;
						g_opt_result.p_opts[opt_num].id = -1;
						opt_num++;
#endif
						}
						if (p_cur_opt) {
							p_cur_opt->arglist[arg_coll_num].raw_idx = idx;
							p_cur_opt->arglist[arg_coll_num].opt_type = OPT_VAL;
							p_cur_opt->arglist[arg_coll_num].kv.opt_val = strdup(argv[idx]);
							arg_coll_num++;
							if (arg_coll_num >= arg_needed) {
								p_cur_opt->arg_num = arg_coll_num;	
								optional = 0;
								p_cur_opt = NULL;
							}
						}
					} else {
#if 0
						p_vals[val_num].opt_type = OPT_VAL;
						p_vals[val_num].kv.opt_val = strdup(argv[idx]);
						p_vals[val_num].raw_idx = idx;
						val_num++;
#else
						g_opt_result.p_opts[opt_num].opt_type = OPT_VAL;
						g_opt_result.p_opts[opt_num].id = -1;
						g_opt_result.p_opts[opt_num].kv.opt_val = strdup(argv[idx]);
						g_opt_result.p_opts[opt_num].raw_idx = idx;
						opt_num++;
						
							
#endif
					}
				}
				break;
		}
	}

	if (p_cur_opt && arg_coll_num > 0) {
		p_cur_opt->arg_num = arg_coll_num;	
	}
	
	g_opt_result.opt_num = opt_num;
	g_opt_result.val_num = val_num;

	return 0;
}


int cmdline_parse(char *cmd_line)
{
	return 0;
}


void cmdline_error()
{
}

// the caller must make sure the seq is less than the size of arglist
char* cmdline_get_arg(void *arglist, int seq)
{
	opt_node_p p_arglist = (opt_node_p)arglist;

	return p_arglist?p_arglist[seq].kv.opt_val:NULL;
}

int cmdline_result_loop(int *opt_idx, int *opt_id, void **arglist, int *arg_num)
{
	opt_node_p p_arglist = NULL;

	if (loop_seq >= g_opt_result.opt_num) {
		loop_seq = 0;  // reset to the begin
		return -1;
	}

	*opt_idx = g_opt_result.p_opts[loop_seq].raw_idx;
	*opt_id = g_opt_result.p_opts[loop_seq].id;
	*arg_num = g_opt_result.p_opts[loop_seq].arg_num;
	*arglist = (void*)g_opt_result.p_opts[loop_seq].arglist;

	loop_seq++;

	return 0;
}


void cmdline_show()
{
	for (int i = 0; i < g_opt_result.opt_num; i++) {
		fprintf(stdout, "%s ", g_opt_result.p_opts[i].kv.opt_name);
		for (int j = 0; j < g_opt_result.p_opts[i].arg_num; j++) {
			fprintf(stdout, "%s ", g_opt_result.p_opts[i].arglist[j].kv.opt_val);
		}
		fprintf(stdout, "\n");
	}

	for (int i = 0; i < g_opt_result.val_num; i++) {
		fprintf(stdout, "%s ", g_opt_result.p_vals[i].kv.opt_val);
		fprintf(stdout, "\n");
	}


}

void cmdline_destroy()
{
	if (g_opt_result.p_opts) {
		for (int i = 0; i < g_opt_result.size; i++) {
			for (int j = 0; j < g_opt_result.p_opts[i].arg_num; j++) {
				if (g_opt_result.p_opts[i].arglist[j].kv.opt_val) free(g_opt_result.p_opts[i].arglist[j].kv.opt_val);
			}
		}
	}
}

