/*
 */

#ifndef __CMDLINE_H
#define __CMDLINE_H

#define ERR_BUF_SZ	1024

#define OPT_PROFILE_END {NULL, -1, -1, -1}
typedef struct _opt_profile_s {
	char	*opt_name;
	int		optional;	// if it is optional, then the arguments will not be set constant, but it must set the maximum number
	int		arg_num;	//
	int		id; //opt name id, so "-" or "--" can be identified only as id number
}opt_profile_t, *opt_profile_p;

enum {
	OPT_OPT = 0x00,
	OPT_VAL,
	OPT_MAX_NUM
};

enum {
	OPT_ARG_CONSTANT = 0x00,
	OPT_ARG_OPTIONAL
};

typedef struct _opt_node_s {
	union {
		char 		*opt_name;  // if opt_type = OPT_OPT, this takes effect
		char		*opt_val;	// if opt_type = OPT_VAL, this takes effect
	}kv;
	int			opt_type;
	int					id;
	int					raw_idx;	// index in argv[]
	int					arg_size;
	int					arg_num;
	struct _opt_node_s	*arglist;
}opt_node_t, *opt_node_p;

typedef struct _opt_result_s {
	int			size;
	int			opt_num;
	opt_node_p	p_opts;	// options start by -- or -
	int			val_num;
	opt_node_p	p_vals; // not options, pure values
}opt_result_t, *opt_result_p;

int cmdline_init();

// the prof must be ended with a value {NULL, -1, -1}
int cmdline_parse(int argc, char **argv, opt_profile_p p_prof);

int cmdline_result_loop(int *opt_idx, int *opt_id, void **arglist, int *arg_num);

char* cmdline_get_arg(void *arglist, int seq);

int cmdline_parse(char *cmd_line);

void cmdline_error();

void cmdline_show();

void cmdline_destroy();
#endif // __CMDLINE_H
